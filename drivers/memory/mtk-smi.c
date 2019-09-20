/*
 * Copyright (c) 2015-2016 MediaTek Inc.
 * Author: Yong Wu <yong.wu@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/clk.h>
#include <linux/component.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <soc/mediatek/smi.h>
#include <dt-bindings/memory/mt2701-larb-port.h>

/* mt8173 */
#define SMI_LARB_MMU_EN		0xf00
#define MTK_SMI_LARB_NR_MAX	9

/* mt2701 */
#define REG_SMI_SECUR_CON_BASE		0x5c0

/* every register control 8 port, register offset 0x4 */
#define REG_SMI_SECUR_CON_OFFSET(id)	(((id) >> 3) << 2)
#define REG_SMI_SECUR_CON_ADDR(id)	\
	(REG_SMI_SECUR_CON_BASE + REG_SMI_SECUR_CON_OFFSET(id))

/*
 * every port have 4 bit to control, bit[port + 3] control virtual or physical,
 * bit[port + 2 : port + 1] control the domain, bit[port] control the security
 * or non-security.
 */
#define SMI_SECUR_CON_VAL_MSK(id)	(~(0xf << (((id) & 0x7) << 2)))
#define SMI_SECUR_CON_VAL_VIRT(id)	BIT((((id) & 0x7) << 2) + 3)
/* mt2701 domain should be set to 3 */
#define SMI_SECUR_CON_VAL_DOMAIN(id)	(0x3 << ((((id) & 0x7) << 2) + 1))

/* mt2712 */
#define SMI_LARB_NONSEC_CON(id)	(0x380 + (id * 4))
#define F_MMU_EN		BIT(0)

/* mt8695 larb8 */
#define CFG_DRAM_AXI	0x380
#define ARMMU		BIT(23)
#define AWMMU		BIT(7)

#define SMI_LARB_SLP_CON		0xC
#define SLP_PROT_EN			BIT(0)
#define SLP_PROT_RDY			BIT(16)

#define SMI_BUS_SEL		0x220
#define SMI_BUS_SEL_DEFAULT	0x1

#define SMI_COMMON_CLAMP_EN		0x3C0
#define SMI_COMMON_CLAMP_EN_SET		0x3C4
#define SMI_COMMON_CLAMP_EN_CLR		0x3C8

struct mtk_smi_larb_gen {
	int port_in_larb[MTK_LARB_NR_MAX + 1];
	void (*config_port)(struct device *);
};

struct smi_common_suspend_reg {
	u32		l1len; /* always be 0xb. 0x100 */
	u32		l1arb0; /* 0x104 */
	u32		l1arb1; /* 0x108 */
	u32		l1arb5; /* 0x118 */
	u32		l1arb7; /* 0x120 */
	u32		bus_sel; /* 0x220 */
	bool		skip_frst_resume;
};

struct smi_larb_suspend_reg {
	u32		cmd_throt_con; /* 0x24 */
	u32		starv_con; /* 0x28 */
	u32		force_ultra; /* 0x78 */
	u32		force_preultra; /* 0x7c */
	u32		weighting_val[32]; /* 0x100 */
	u32		ostdl_port[32]; /* 0x200 */
	bool		skip_frst_resume;
};

struct mtk_smi {
	struct device			*dev;
	/* larb1 need one more clk axi_asif for mt8695 */
	struct clk			*clk_apb, *clk_smi, *clk_asif;
	struct clk			*clk_async; /*only needed by mt2701*/
	void __iomem			*smi_ao_base;
	void __iomem		*base;
	struct smi_common_suspend_reg suspend_reg;
};

struct mtk_smi_larb { /* larb: local arbiter */
	struct mtk_smi			smi;
	void __iomem			*base;
	struct device			*smi_common_dev;
	const struct mtk_smi_larb_gen	*larb_gen;
	int				larbid;
	u32				*mmu;
	int				ref_cnt;
	struct smi_larb_suspend_reg	suspend_reg;
};

struct mtk_larb_dev {
	struct device *dev;
	int larbid;
};

static struct mtk_larb_dev gmtk_larb_dev[MTK_SMI_LARB_NR_MAX];
static struct mtk_smi_larb *gmtk_larb[MTK_SMI_LARB_NR_MAX];
static struct mtk_smi *gmtk_smi;

enum mtk_smi_gen {
	MTK_SMI_GEN1,
	MTK_SMI_GEN2
};

static int mtk_smi_clk_enable(const struct mtk_smi *smi)
{
	int ret;

	ret = clk_prepare_enable(smi->clk_apb);
	if (ret)
		return ret;

	ret = clk_prepare_enable(smi->clk_smi);
	if (ret)
		goto err_disable_apb;

	ret = clk_prepare_enable(smi->clk_asif);
	if (ret)
		goto err_disable_smi;

	return 0;

err_disable_smi:
	clk_disable_unprepare(smi->clk_smi);
err_disable_apb:
	clk_disable_unprepare(smi->clk_apb);
	return ret;
}

static void mtk_smi_clk_disable(const struct mtk_smi *smi)
{
	clk_disable_unprepare(smi->clk_asif);
	clk_disable_unprepare(smi->clk_smi);
	clk_disable_unprepare(smi->clk_apb);
}

/* specific for mt8695 larb8 */
static void mtk_smi_config_port_larb8(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	u32 reg;

	/* read iommu enable */
	if (*larb->mmu & BIT(0)) {
		reg = readl_relaxed(larb->base + CFG_DRAM_AXI);
		reg |= ARMMU;
		writel(reg, larb->base + CFG_DRAM_AXI);

	}

	/* write iommu enable */
	if (*larb->mmu & BIT(1)) {
		reg = readl_relaxed(larb->base + CFG_DRAM_AXI);
		reg |= AWMMU;
		writel(reg, larb->base + CFG_DRAM_AXI);
	}
}

int mtk_smi_larb_get(struct device *larbdev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(larbdev);
	int ret;

	ret = pm_runtime_get_sync(larbdev);
	if (ret < 0)
		return ret;

	larb->ref_cnt++;
	return 0;
}
EXPORT_SYMBOL_GPL(mtk_smi_larb_get);

void mtk_smi_larb_put(struct device *larbdev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(larbdev);

	/*
	 * Don't de-configure the iommu info for this larb since there may be
	 * several modules in this larb.
	 * The iommu info will be reset after power off.
	 */
	pm_runtime_put_sync(larbdev);

	larb->ref_cnt--;
	if (larb->ref_cnt < 0) {
		dev_warn_ratelimited(larbdev, "warn ref_cnt %d.",
				     larb->ref_cnt);
		larb->ref_cnt = 0; /* recovery. */
	}
}
EXPORT_SYMBOL_GPL(mtk_smi_larb_put);

int mtk_smi_larb_clock_on(int larbid, bool pm)
{
	struct device *dev = gmtk_larb_dev[larbid].dev;

	if (!dev)
		return -1;
	if (larbid == 2) /* larb2 nouse in mt8695*/
		return 0;

	/*
	 * for mt8695 larb1 and larb7 need vdec power and clock,
	 * in vdec_power_on/off, vdec will call mtk_smi_larb_get
	 * to enable both larb1 and larb7's power and clock.
	 * this change is only used for MET tools, other drivers
	 * should not call this interface.
	 */
	if (larbid != 1 && larbid != 7)
		return mtk_smi_larb_get(dev);

#ifdef CONFIG_MTK_VIDEOCODEC_DRIVER
	vdec_power_on_for_smi();
	return 0;
#else
	return mtk_smi_larb_get(dev);
#endif
}
EXPORT_SYMBOL_GPL(mtk_smi_larb_clock_on);

int mtk_smi_larb_ready(int larbid)
{
	struct device *dev = gmtk_larb_dev[larbid].dev;

	if (dev)
		return 1;

	return 0;
}
EXPORT_SYMBOL_GPL(mtk_smi_larb_ready);

void mtk_smi_larb_clock_off(int larbid, bool pm)
{
	struct device *dev = gmtk_larb_dev[larbid].dev;

	if (!dev)
		return;

	if (larbid == 2) /* larb2 nouse in mt8695*/
		return;

#ifdef CONFIG_MTK_VIDEOCODEC_DRIVER
	if (larbid != 1 && larbid != 7)
		mtk_smi_larb_put(dev);
	else
		vdec_power_off_for_smi();
#else
	mtk_smi_larb_put(dev);
#endif
}
EXPORT_SYMBOL_GPL(mtk_smi_larb_clock_off);

static int
mtk_smi_larb_bind(struct device *dev, struct device *master, void *data)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	struct mtk_smi_iommu *smi_iommu = data;
	unsigned int         i;

	for (i = 0; i < MTK_LARB_NR_MAX; i++) {
		if (dev == smi_iommu->larb_imu[i].dev) {
			/* The 'mmu' may be updated in iommu-attach/detach. */
			larb->mmu = &smi_iommu->larb_imu[i].mmu;
			return 0;
		}
	}
	return -ENODEV;
}

static void mtk_smi_larb_config_port_mt8173(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);

	writel(*larb->mmu, larb->base + SMI_LARB_MMU_EN);
}

static void mtk_smi_larb_config_port_mt2712(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	u32 reg;
	int i;

	for (i = 0; i < 32; i++) {
		if (*larb->mmu & BIT(i)) {
			reg = readl_relaxed(larb->base + SMI_LARB_NONSEC_CON(i));
			reg |= F_MMU_EN;
			writel(reg, larb->base + SMI_LARB_NONSEC_CON(i));
		}
	}
}

static void mtk_smi_larb_config_port_gen1(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	const struct mtk_smi_larb_gen *larb_gen = larb->larb_gen;
	struct mtk_smi *common = dev_get_drvdata(larb->smi_common_dev);
	int i, m4u_port_id, larb_port_num;
	u32 sec_con_val, reg_val;

	m4u_port_id = larb_gen->port_in_larb[larb->larbid];
	larb_port_num = larb_gen->port_in_larb[larb->larbid + 1]
			- larb_gen->port_in_larb[larb->larbid];

	for (i = 0; i < larb_port_num; i++, m4u_port_id++) {
		if (*larb->mmu & BIT(i)) {
			/* bit[port + 3] controls the virtual or physical */
			sec_con_val = SMI_SECUR_CON_VAL_VIRT(m4u_port_id);
		} else {
			/* do not need to enable m4u for this port */
			continue;
		}
		reg_val = readl(common->smi_ao_base
			+ REG_SMI_SECUR_CON_ADDR(m4u_port_id));
		reg_val &= SMI_SECUR_CON_VAL_MSK(m4u_port_id);
		reg_val |= sec_con_val;
		reg_val |= SMI_SECUR_CON_VAL_DOMAIN(m4u_port_id);
		writel(reg_val,
			common->smi_ao_base
			+ REG_SMI_SECUR_CON_ADDR(m4u_port_id));
	}
}

static void
mtk_smi_larb_unbind(struct device *dev, struct device *master, void *data)
{
	/* Do nothing as the iommu is always enabled. */
}

int mtk_smi_vp_setting(bool osd_4k)
{
	int i;
	void __iomem *addr;

	if (!gmtk_larb[0] || !gmtk_smi)
		return -EINVAL;

	for (i = 0; i < 8; i++)
		mtk_smi_larb_clock_on(i, true);

	/* SMI common */
	addr = gmtk_smi->base;
	writel_relaxed(0xb, addr + 0x100);
	if (osd_4k)
		writel_relaxed(0x0, addr + 0x104);
	else
		writel_relaxed(0x1141, addr + 0x104); /* normal video.*/
	writel_relaxed(0x1840, addr  + 0x108);
	writel_relaxed(0x244000, addr + 0x118);
	writel_relaxed(0x1040, addr + 0x120);
	writel(0x501, addr + 0x220); /* adjust axi channel */

	addr = gmtk_larb[0]->base;
	writel_relaxed(0x10012, addr + 0x28);
	if (osd_4k)
		writel_relaxed(0x16, addr + 0x200); /* OSTD */
	else
		writel_relaxed(0x3f, addr + 0x200);/* defaulty.*/

	addr = gmtk_larb[1]->base;
	writel_relaxed(0x27, addr + 0x200);/* mc */
	writel_relaxed(0x4, addr + 0x204);
	writel_relaxed(0x4, addr + 0x208);
	writel_relaxed(0x4, addr + 0x20c);
	writel_relaxed(0x4, addr + 0x210);
	writel_relaxed(0x4, addr + 0x214);
	writel_relaxed(0x4, addr + 0x218);
	writel_relaxed(0x8, addr + 0x21c);
	writel_relaxed(0x8, addr + 0x220);
	writel_relaxed(0x8, addr + 0x224);
	writel_relaxed(0xf, addr + 0x228);

	addr = gmtk_larb[4]->base;
	writel_relaxed(0x10012, addr + 0x28);
	writel_relaxed(0x0, addr + 0x78);
	writel_relaxed(0x3f, addr + 0x200);/* fhd_osd defaulty.*/
	writel_relaxed(0x1, addr + 0x204); /* dobly */
	writel_relaxed(0x1, addr + 0x208); /* dobly */

	/* larb5's default for bit[18:16] is zero */
	addr = gmtk_larb[5]->base;
	if (osd_4k)/* osd4k always force ultra */
		writel_relaxed(0x3ff, addr + 0x78);
	writel_relaxed(0x1, addr + 0x108);
	writel_relaxed(0x1, addr + 0x10c);
	writel_relaxed(0x1, addr + 0x110);
	writel_relaxed(0x1, addr + 0x114);

	writel_relaxed(0x4, addr + 0x200);
	writel_relaxed(0x4, addr + 0x204);
	writel_relaxed(0x8, addr + 0x208);
	writel_relaxed(0x8, addr + 0x20c);
	writel_relaxed(0x8, addr + 0x210);
	writel_relaxed(0x8, addr + 0x214);
	writel_relaxed(0x4, addr + 0x218);
	writel_relaxed(0x4, addr + 0x21c);
	writel_relaxed(0x4, addr + 0x220);
	writel_relaxed(0x4, addr + 0x224);

	addr = gmtk_larb[6]->base;
	writel_relaxed(0x3f, addr + 0x7c);

	addr = gmtk_larb[7]->base; /* vdec lat */
	writel_relaxed(0x8, addr + 0x200);
	writel_relaxed(0x8, addr + 0x204);
	writel_relaxed(0x8, addr + 0x208);
	writel_relaxed(0x4, addr + 0x20c);
	writel_relaxed(0x4, addr + 0x210);
	writel_relaxed(0x8, addr + 0x214);

	for (i = 7; i >= 0; i--)
		mtk_smi_larb_clock_off(i, true);

	return 0;
}
EXPORT_SYMBOL_GPL(mtk_smi_vp_setting);

int mtk_smi_init_setting(void)
{
	void __iomem *addr;
	int i;

	if (!gmtk_larb[0] || !gmtk_smi)
		return -EINVAL;

	for (i = 0; i < 8; i++)
		mtk_smi_larb_clock_on(i, true);

	addr = gmtk_larb[0]->base;
	writel_relaxed(0x0, addr + 0x28);
	writel_relaxed(0x3f, addr + 0x200);

	addr = gmtk_larb[4]->base;
	writel_relaxed(0x0, addr + 0x28);
	writel_relaxed(0x0, addr + 0x78);
	writel_relaxed(0x3f, addr + 0x204); /* dobly */
	writel_relaxed(0x3f, addr + 0x208);

	addr = gmtk_smi->base;
	writel_relaxed(0xb, addr + 0x100);
	writel_relaxed(0x0, addr + 0x104);
	writel_relaxed(0x0, addr + 0x118);
	writel_relaxed(0x0, addr + 0x108);
	writel_relaxed(0x0, addr + 0x120);

	/* larb0 and larb4/5 use two channel. */
	writel(SMI_BUS_SEL_DEFAULT, addr + SMI_BUS_SEL);

	addr = gmtk_larb[5]->base;
	writel_relaxed(0x0, addr + 0x78);
	writel_relaxed(0x0, addr + 0x7c);
	writel_relaxed(0x0, addr + 0x108);
	writel_relaxed(0x0, addr + 0x10c);
	writel_relaxed(0x0, addr + 0x110);
	writel_relaxed(0x0, addr + 0x114);
	for (i = 0; i < 10; i++)
		writel_relaxed(0x3f, addr + 0x200 + i * 4);

	addr = gmtk_larb[1]->base;
	for (i = 0; i < 11; i++)
		writel_relaxed(0x3f, addr + 0x200 + i * 4);
	addr = gmtk_larb[7]->base;
	for (i = 0; i < 6; i++)
		writel_relaxed(0x3f, addr + 0x200 + i * 4);

	for (i = 7; i >= 0; i--)
		mtk_smi_larb_clock_off(i, true);

	return 0;
}
EXPORT_SYMBOL_GPL(mtk_smi_init_setting);

static const struct component_ops mtk_smi_larb_component_ops = {
	.bind = mtk_smi_larb_bind,
	.unbind = mtk_smi_larb_unbind,
};

static const struct mtk_smi_larb_gen mtk_smi_larb_mt8173 = {
	/* mt8173 do not need the port in larb */
	.config_port = mtk_smi_larb_config_port_mt8173,
};

static const struct mtk_smi_larb_gen mtk_smi_larb_mt2712 = {
	/* mt2712 do not need the port in larb */
	.config_port = mtk_smi_larb_config_port_mt2712,
};

static const struct mtk_smi_larb_gen mtk_smi_larb_mt2701 = {
	.port_in_larb = {
		LARB0_PORT_OFFSET, LARB1_PORT_OFFSET,
		LARB2_PORT_OFFSET, LARB3_PORT_OFFSET
	},
	.config_port = mtk_smi_larb_config_port_gen1,
};

static const struct of_device_id mtk_smi_larb_of_ids[] = {
	{
		.compatible = "mediatek,mt8173-smi-larb",
		.data = &mtk_smi_larb_mt8173
	},
	{
		.compatible = "mediatek,mt2701-smi-larb",
		.data = &mtk_smi_larb_mt2701
	},
	{
		.compatible = "mediatek,mt2712-smi-larb",
		.data = &mtk_smi_larb_mt2712
	},
	/* most mt8695 smi common and larb are same with mt2712 */
	{
		.compatible = "mediatek,mt8695-smi-larb",
		.data = &mtk_smi_larb_mt2712
	},
	{}
};

static int mtk_smi_larb_probe(struct platform_device *pdev)
{
	struct mtk_smi_larb *larb;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct device_node *smi_node;
	struct platform_device *smi_pdev;
	const struct of_device_id *of_id;
	int ret;

	if (!dev->pm_domain)
		return -EPROBE_DEFER;

	of_id = of_match_node(mtk_smi_larb_of_ids, pdev->dev.of_node);
	if (!of_id)
		return -EINVAL;

	larb = devm_kzalloc(dev, sizeof(*larb), GFP_KERNEL);
	if (!larb)
		return -ENOMEM;

	larb->larb_gen = of_id->data;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	larb->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(larb->base))
		return PTR_ERR(larb->base);

	larb->smi.clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(larb->smi.clk_apb))
		return PTR_ERR(larb->smi.clk_apb);

	larb->smi.clk_smi = devm_clk_get(dev, "smi");
	if (IS_ERR(larb->smi.clk_smi))
		return PTR_ERR(larb->smi.clk_smi);
	larb->smi.dev = dev;

	smi_node = of_parse_phandle(dev->of_node, "mediatek,smi", 0);
	if (!smi_node)
		return -EINVAL;

	ret = of_property_read_u32(dev->of_node, "mediatek,larbidx",
				   &larb->larbid);
	/* There may be no this property in mt8173 while the others have it. */
	if (ret &&
	    !of_device_is_compatible(dev->of_node, "mediatek,mt8173-smi-larb"))
		return ret;

	/* do not need to check the return value since clk is ready here. */
	if (of_device_is_compatible(dev->of_node, "mediatek,mt8695-smi-larb") &&
	    (larb->larbid == 1 || larb->larbid == 7))
		larb->smi.clk_asif = devm_clk_get(dev, "asif");
	else
		larb->smi.clk_asif = NULL;

	smi_pdev = of_find_device_by_node(smi_node);
	of_node_put(smi_node);
	if (smi_pdev) {
		if (!platform_get_drvdata(smi_pdev))
			return -EPROBE_DEFER;
		larb->smi_common_dev = &smi_pdev->dev;
	} else {
		dev_err(dev, "Failed to get the smi_common device\n");
		return -EINVAL;
	}

	pm_runtime_enable(dev);
	gmtk_larb_dev[larb->larbid].dev = &pdev->dev;
	gmtk_larb_dev[larb->larbid].larbid = larb->larbid;
	gmtk_larb[larb->larbid] = larb;

	platform_set_drvdata(pdev, larb);

	return component_add(dev, &mtk_smi_larb_component_ops);
}

static int mtk_smi_larb_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);
	component_del(&pdev->dev, &mtk_smi_larb_component_ops);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int __maybe_unused mtk_smi_larb_suspend(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	struct mtk_smi *common = dev_get_drvdata(larb->smi_common_dev);
	struct smi_larb_suspend_reg *backup = &larb->suspend_reg;
	void __iomem *base = larb->base;
	u32 larbid = larb->larbid;
	u32 tmp;
	int i, ret;

	if (larbid != 8) {
		backup->cmd_throt_con = readl_relaxed(base + 0x24);
		backup->starv_con = readl_relaxed(base + 0x28);
		backup->force_ultra = readl_relaxed(base + 0x78);
		backup->force_preultra = readl_relaxed(base + 0x7c);

		for (i = 0; i < 32; i++)
			backup->weighting_val[i] = readl_relaxed(
							base + 0x100 + i * 4);
		for (i = 0; i < 32; i++)
			backup->ostdl_port[i] = readl_relaxed(
							base + 0x200 + i * 4);
	}

	if (larbid != 8) {
		/* larb sleep control enable. wait outstanding is 0. */
		writel_relaxed(SLP_PROT_EN, base + SMI_LARB_SLP_CON);
		if (readl_poll_timeout_atomic(base + SMI_LARB_SLP_CON,
					tmp, !!(tmp & SLP_PROT_RDY),
					10, 10000))
			dev_warn(dev, "sleep con is not ready(%d).", tmp);
	}

	/* Clamp this larb in smi_common. the smi-common clk is enabled here. */
	if (larbid == 8) /* bdpsys larbid remap.*/
		larbid = 2;
	writel(BIT(larbid), common->base + SMI_COMMON_CLAMP_EN_SET);
	ret = readl_poll_timeout_atomic(common->base + SMI_COMMON_CLAMP_EN,
					tmp, !!(tmp & BIT(larbid)),
					10, 10000);
	if (ret)
		dev_warn(dev, "suspend clamp enable 0x%x failed(%d)\n",
			 readl_relaxed(common->base + SMI_COMMON_CLAMP_EN),
			 ret);

	mtk_smi_clk_disable(&larb->smi);
	pm_runtime_put_sync(larb->smi_common_dev);

	return 0;
}

static int __maybe_unused mtk_smi_larb_resume(struct device *dev)
{
	struct mtk_smi_larb *larb = dev_get_drvdata(dev);
	struct mtk_smi *common = dev_get_drvdata(larb->smi_common_dev);
	struct smi_larb_suspend_reg *backup = &larb->suspend_reg;
	const struct mtk_smi_larb_gen *larb_gen = larb->larb_gen;
	void __iomem *base = larb->base;
	u32 larbid = larb->larbid, tmp;
	int ret, i;

	/* Power on smi-common. */
	ret = pm_runtime_get_sync(larb->smi_common_dev);
	if (ret < 0) {
		dev_err(dev, "smi-common pm get failed(%d).\n", ret);
		return ret;
	}

	ret = mtk_smi_clk_enable(&larb->smi);
	if (ret) {
		dev_err(dev, "Failed to enable clk(%d) in runtime resume\n", ret);
		return ret;
	}

	if (larbid != 8) {
		/* larb sleep control disable. */
		writel_relaxed(0, base + SMI_LARB_SLP_CON);
	}

	/* Clamp clr this larb in smi_common */
	if (larbid == 8) /* bdpsys larbid remap.*/
		larbid = 2;
	writel(BIT(larbid), common->base + SMI_COMMON_CLAMP_EN_CLR);
	ret = readl_poll_timeout_atomic(common->base + SMI_COMMON_CLAMP_EN,
					tmp, !(tmp & BIT(larbid)),
					10, 10000);
	if (ret)
		dev_warn(dev, "resume clamp clear 0x%x failed(%d)\n",
			readl_relaxed(common->base + SMI_COMMON_CLAMP_EN),
			ret);

	/* Avoid change the 0x24 for display larb(0+4) who is show fastlogo */
	if (larbid != 0 && larbid != 4 && larb->larbid != 8 &&
	    larbid != 1 && larbid != 7)
		writel_relaxed(0x370246, base + 0x24);/* always enable bit16/17/18 */

	/* Configure the iommu info for this larb, only 8691 have larb8 */
	if (larb->larbid != 8)
		larb_gen->config_port(dev);
	else
		mtk_smi_config_port_larb8(dev);

	/* Skip the frst resume as the data below is 0 defaultly. */
	if (!backup->skip_frst_resume) {
		backup->skip_frst_resume = true;
		return 0;
	}

	if (larb->larbid == 8)
		return 0;

	writel_relaxed(backup->starv_con, base + 0x28);
	writel_relaxed(backup->force_ultra, base + 0x78);
	writel_relaxed(backup->force_preultra, base + 0x7c);

	for (i = 0; i < 32; i++)
		writel_relaxed(backup->weighting_val[i], base + 0x100 + i * 4);

	for (i = 0; i < 32; i++)
		writel_relaxed(backup->ostdl_port[i], base + 0x200 + i * 4);

	return 0;
}
#endif

static const struct dev_pm_ops smi_larb_pm_ops = {
	SET_RUNTIME_PM_OPS(mtk_smi_larb_suspend, mtk_smi_larb_resume, NULL)
};

static struct platform_driver mtk_smi_larb_driver = {
	.probe	= mtk_smi_larb_probe,
	.remove	= mtk_smi_larb_remove,
	.driver	= {
		.name = "mtk-smi-larb",
		.of_match_table = mtk_smi_larb_of_ids,
		.pm		= &smi_larb_pm_ops,
	}
};

static const struct of_device_id mtk_smi_common_of_ids[] = {
	{
		.compatible = "mediatek,mt8173-smi-common",
		.data = (void *)MTK_SMI_GEN2
	},
	{
		.compatible = "mediatek,mt2701-smi-common",
		.data = (void *)MTK_SMI_GEN1
	},
	{
		.compatible = "mediatek,mt2712-smi-common",
		.data = (void *)MTK_SMI_GEN2
	},
	{
		.compatible = "mediatek,mt8695-smi-common",
		.data = (void *)MTK_SMI_GEN2
	},
	{}
};

static int mtk_smi_common_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtk_smi *common;
	struct resource *res;
	const struct of_device_id *of_id;
	enum mtk_smi_gen smi_gen;

	if (!dev->pm_domain)
		return -EPROBE_DEFER;

	common = devm_kzalloc(dev, sizeof(*common), GFP_KERNEL);
	if (!common)
		return -ENOMEM;
	common->dev = dev;

	common->clk_apb = devm_clk_get(dev, "apb");
	if (IS_ERR(common->clk_apb))
		return PTR_ERR(common->clk_apb);

	common->clk_smi = devm_clk_get(dev, "smi");
	if (IS_ERR(common->clk_smi))
		return PTR_ERR(common->clk_smi);

	of_id = of_match_node(mtk_smi_common_of_ids, pdev->dev.of_node);
	if (!of_id)
		return -EINVAL;

	/*
	 * for mtk smi gen 1, we need to get the ao(always on) base to config
	 * m4u port, and we need to enable the aync clock for transform the smi
	 * clock into emi clock domain, but for mtk smi gen2, there's no smi ao
	 * base.
	 */
	smi_gen = (enum mtk_smi_gen)of_id->data;
	if (smi_gen == MTK_SMI_GEN1) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		common->smi_ao_base = devm_ioremap_resource(dev, res);
		if (IS_ERR(common->smi_ao_base))
			return PTR_ERR(common->smi_ao_base);

		common->clk_async = devm_clk_get(dev, "async");
		if (IS_ERR(common->clk_async))
			return PTR_ERR(common->clk_async);

		clk_prepare_enable(common->clk_async);
	} else {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		common->base = devm_ioremap_resource(dev, res);
		if (IS_ERR(common->base))
			return PTR_ERR(common->base);
	}

	gmtk_smi = common;
	platform_set_drvdata(pdev, common);
	pm_runtime_enable(dev);
	/*
	 * Without pm_runtime_get_sync(dev), the disp power domain
	 * would be turn off after pm_runtime_enable, meanwhile disp
	 * hw are still access register, this would cause system
	 * abnormal.
	 *
	 * If we do not call pm_runtime_get_sync, then system would hang
	 * in larb0's power domain attach, power domain SA and DE are
	 * still checking that. We would like to bypass this first and
	 * don't block the software flow.
	 */
	pm_runtime_get_sync(dev);

	return 0;
}

static int mtk_smi_common_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int __maybe_unused mtk_smi_common_suspend(struct device *dev)
{
	struct mtk_smi *common = dev_get_drvdata(dev);
	struct smi_common_suspend_reg *backup = &common->suspend_reg;
	void __iomem *base = common->base;

	backup->l1len = 0xb;
	backup->l1arb0 = readl_relaxed(base + 0x104);
	backup->l1arb1 = readl_relaxed(base + 0x108);
	backup->l1arb5 = readl_relaxed(base + 0x118);
	backup->l1arb7 = readl_relaxed(base + 0x120);
	backup->bus_sel = readl_relaxed(base + SMI_BUS_SEL);

	mtk_smi_clk_disable(common);
	return 0;
}

static int __maybe_unused mtk_smi_common_resume(struct device *dev)
{
	struct mtk_smi *common = dev_get_drvdata(dev);
	struct smi_common_suspend_reg *backup = &common->suspend_reg;
	void __iomem *base = common->base;
	int ret;

	ret = mtk_smi_clk_enable(common);
	if (ret)
		return ret;

	writel_relaxed(0xb, base + 0x100);/* backup->l1len */

	/* Skip the frst resume as the data below is 0 defaultly. */
	if (!backup->skip_frst_resume) {
		backup->skip_frst_resume = true;
		writel_relaxed(SMI_BUS_SEL_DEFAULT, base + SMI_BUS_SEL);
		return 0;
	}
	writel_relaxed(backup->l1arb0, base + 0x104);
	writel_relaxed(backup->l1arb1, base + 0x108);
	writel_relaxed(backup->l1arb5, base + 0x118);
	writel_relaxed(backup->l1arb7, base + 0x120);
	writel_relaxed(backup->bus_sel, base + SMI_BUS_SEL);

	return 0;
}
#endif

static const struct dev_pm_ops smi_common_pm_ops = {
	SET_RUNTIME_PM_OPS(mtk_smi_common_suspend, mtk_smi_common_resume, NULL)
};

static struct platform_driver mtk_smi_common_driver = {
	.probe	= mtk_smi_common_probe,
	.remove = mtk_smi_common_remove,
	.driver	= {
		.name = "mtk-smi-common",
		.of_match_table = mtk_smi_common_of_ids,
		.pm		= &smi_common_pm_ops,
	}
};

static int __init mtk_smi_init(void)
{
	int ret;

	ret = platform_driver_register(&mtk_smi_common_driver);
	if (ret != 0) {
		pr_err("Failed to register SMI driver\n");
		return ret;
	}

	ret = platform_driver_register(&mtk_smi_larb_driver);
	if (ret != 0) {
		pr_err("Failed to register SMI-LARB driver\n");
		goto err_unreg_smi;
	}
	return ret;

err_unreg_smi:
	platform_driver_unregister(&mtk_smi_common_driver);
	return ret;
}

subsys_initcall(mtk_smi_init);

unsigned long get_larb_base_addr(int larbid)
{
	if (larbid < 0 || larbid > MTK_SMI_LARB_NR_MAX)
		return -EINVAL;

	if (!gmtk_larb[larbid] || !gmtk_larb[larbid]->base)
		return -EINVAL;

	return (unsigned long)gmtk_larb[larbid]->base;
}
EXPORT_SYMBOL_GPL(get_larb_base_addr);

unsigned long get_common_base_addr(void)
{
	if (!gmtk_smi || !gmtk_smi->base)
		return -EINVAL;

	return (unsigned long)gmtk_smi->base;
}
EXPORT_SYMBOL_GPL(get_common_base_addr);

/* put the disp power domain that we got in smi probe */
static int __init mtk_smi_init_late(void)
{
	struct device *dev = gmtk_smi->dev;

	if (!dev) {
		dev_err(dev, "%s, %d\n", __func__, __LINE__);
		return -1;
	}

	/*
	 * We get_sync the disp power domain in smi common probe,
	 * need to put_sync it to avoid dis-pairing of get/put_sync
	 * for the disp power domain.
	 *
	 * Use larb0's device is OK since it's in the same power
	 * domain with smi common.
	 */
	pm_runtime_put_sync(dev);

	return 0;
}

late_initcall(mtk_smi_init_late);
