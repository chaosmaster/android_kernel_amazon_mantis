#include <linux/module.h>
#include <linux/preempt.h>
#include <linux/ioport.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/init.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/soc/mediatek/infracfg.h>

#include "mt_freqhopping.h"

#include "mtk_mfg.h"

/*
 * This will disable any clk/pll/regulator setting operations.
 * We use this to ensure HW is using default value from preloader.
 */
/*#define MFG_BRING_UP_ALL_ON*/

/*
 * Handle mtcmos controlling by mfg driver, to make sure HW order
 * and HW APM.
 */
#define MFG_DIRECT_POWER_CTRL

#define CG_MFG_AXI BIT(0)
#define CG_MFG_MEM BIT(1)
#define CG_MFG_G3D BIT(2)
#define CG_MFG_26M BIT(3)
#define CG_MFG_ALL (CG_MFG_AXI | CG_MFG_MEM | CG_MFG_G3D | CG_MFG_26M)

#define REG_MFG_CG_STA 0x00
#define REG_MFG_CG_SET 0x04
#define REG_MFG_CG_CLR 0x08

#define MASK_BITS(l, r) (((1 << ((l) - (r) + 1)) - 1) << (r))
#define READ_BITS(x, l, r) (((x) & MASK_BITS((l), (r))) >> (r))
#define MODIFY_BITS(v, n, l, r) (((v) & ~MASK_BITS((l), (r))) | ((n) << (r)))

#define MFG_READ32(r) __raw_readl(mfg_start + (r))
#define MFG_WRITE32(v, r) __raw_writel((v), mfg_start + (r))
#define UPDATE_MFG_REG_BITS(reg, val, l, r) \
do {\
	u32 v;\
	v = MFG_READ32(reg);\
	v = MODIFY_BITS(v, val, l, r);\
	MFG_WRITE32(v, reg);\
} while (0)

static struct platform_device *mfg_dev;

#define MFG_CLK_SLOW 0
#define MFG_CLK_AXI 1
#define MFG_CLK_CORE 2

static const char * const top_mfg_clk_sel_name[] = {
	"mfg_slow_in_sel",
	"mfg_axi_in_sel",
	"mfg_sel",
};

static const char * const top_mfg_clk_sel_parent_name[] = {
	"slow",
	"axi",
	"pll",
};

#define MFG_CLK_COUNT ARRAY_SIZE(top_mfg_clk_sel_name)

static struct clk *clk_sel[MFG_CLK_COUNT];
static struct clk *clk_parent[MFG_CLK_COUNT];
static struct clk *clk_pll;

static struct regulator *vgpu;

static DEFINE_MUTEX(enable_lock);
static int enable_count;

static void __iomem *mfg_start;

static bool enable_hwapm = true;
static bool bw_ultra_disable;

#ifdef CONFIG_MTK8695_FREQ_HOPPING
static bool pll_support_hopping = true;
#else
static bool pll_support_hopping;
#endif
/*
 * Store freq set by GPU driver.
 * They are for 'snap' getters .
 */
static unsigned long current_freq;
static int current_volt;

enum {
	PD_ASYNC = 0,
	PD_2D,
	PD_3D,
	PD_MAX
};

#ifdef MFG_DIRECT_POWER_CTRL
static void __iomem *scp_start;
static struct regmap *infracfg_reg;

struct scp_pd_t {
	u32 offs;
	u32 sta_mask;
	u32 pdn_mask;
	u32 ack_mask;
	u32 bus_prot_mask;
};

static struct scp_pd_t scp_pd[] = {
	{
		.offs = 0x0290,
		.sta_mask = BIT(18),
		.pdn_mask = GENMASK(11, 8),
		.ack_mask = 0,/* GENMASK(15, 12), */
		.bus_prot_mask = 0,/*BIT(14) | BIT(21) | BIT(22) | BIT(23),*/
	},
	{
		.offs = 0x0294,
		.sta_mask = BIT(19),
		.pdn_mask = GENMASK(11, 8),
		.ack_mask = GENMASK(15, 12),
		.bus_prot_mask = BIT(14) | BIT(21) | BIT(22) | BIT(23),
	},
	{
		.offs = 0x0298,
		.sta_mask = BIT(20),
		.pdn_mask = GENMASK(13, 8),
		.ack_mask = GENMASK(19, 16),
		.bus_prot_mask = 0,
	},
};

#endif

static int enable_count;

#ifdef CONFIG_OF
static const struct of_device_id mfg_dt_ids[] = {
	{.compatible = "mediatek,mt8695-mfg"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mfg_dt_ids);

#if !defined(MFG_DIRECT_POWER_CTRL)

static const struct of_device_id mfg_async_dt_ids[] = {
	{.compatible = "mediatek,mt8695-mfg-async"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mfg_async_dt_ids);

static const struct of_device_id mfg_2d_dt_ids[] = {
	{.compatible = "mediatek,mt8695-mfg-2d"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mfg_2d_dt_ids);

static const struct of_device_id mfg_3d_dt_ids[] = {
	{.compatible = "mediatek,mt8695-mfg-3d"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mfg_3d_dt_ids);

#endif

#endif /* CONFIG_OF */

#ifdef MFG_DIRECT_POWER_CTRL

#define SCP_READ32(r) readl(scp_start + (r))
#define SCP_WRITE32(v, r) writel((v), scp_start + (r))

/*
 * > 0: enabled
 * ==0: disabled,
 * < 0: error state.
 */
static int power_state(struct scp_pd_t *c)
{
	u32 status;
	u32 status2;

	status = SCP_READ32(0x60c) & c->sta_mask;
	status2 = SCP_READ32(0x610) & c->sta_mask;

	if (status && status2)
		return 1;
	if (!status && !status2)
		return 0;

	return -EINVAL;
}

#define PWR_RST_B_BIT		BIT(0)
#define PWR_ISO_BIT			BIT(1)
#define PWR_ON_BIT			BIT(2)
#define PWR_ON_2ND_BIT		BIT(3)
#define PWR_CLK_DIS_BIT		BIT(4)

static int scp_enable(int idx)
{
	unsigned long timeout;
	struct scp_pd_t *c = &scp_pd[idx];
	u32 pdn_ack = c->ack_mask;
	u32 val;
	int ret;

	val = SCP_READ32(c->offs);
	val |= PWR_ON_BIT;
	SCP_WRITE32(val, c->offs);
	val |= PWR_ON_2ND_BIT;
	SCP_WRITE32(val, c->offs);

	timeout = jiffies + HZ;
	while (power_state(c) <= 0) {
		cpu_relax();
		if (time_after(jiffies, timeout)) {
			dev_err(&mfg_dev->dev, "timeout of power on %d\n", idx);
			break;
		}
	}

	val &= ~PWR_CLK_DIS_BIT;
	SCP_WRITE32(val, c->offs);

	val &= ~PWR_ISO_BIT;
	SCP_WRITE32(val, c->offs);

	val |= PWR_RST_B_BIT;
	SCP_WRITE32(val, c->offs);

	val &= ~c->pdn_mask;
	SCP_WRITE32(val, c->offs);

	/* wait until SRAM_PDN_ACK all 0 */
	timeout = jiffies + HZ;
	while (pdn_ack && (SCP_READ32(c->offs) & pdn_ack)) {
		cpu_relax();
		if (time_after(jiffies, timeout)) {
			dev_err(&mfg_dev->dev, "power on %d timeout, pdn_ack %x, %x\n", idx, pdn_ack, val);
			goto error_out;
		}
	}

	if (c->bus_prot_mask) {
		ret = mtk_infracfg_clear_bus_protection(
				infracfg_reg,
				c->bus_prot_mask);
		if (ret)
			goto error_out;
	}

	return 0;

error_out:
	dev_err(&mfg_dev->dev, "Failed to power on %d\n", idx);
	return ret;
}

static int scp_disable(int idx)
{
	int ret;
	u32 val;
	struct scp_pd_t *c = &scp_pd[idx];
	unsigned long timeout;
	u32 pdn_ack = c->ack_mask;

	if (c->bus_prot_mask) {
		ret = mtk_infracfg_set_bus_protection(
				infracfg_reg,
				c->bus_prot_mask);
		if (ret)
			dev_err(&mfg_dev->dev, "*** bus prot set error %d\n", ret);
	}

	val = SCP_READ32(c->offs);
	val |= c->pdn_mask;
	SCP_WRITE32(val, c->offs);

	/* wait until SRAM_PDN_ACK all 1 */
	timeout = jiffies + HZ;
	while (pdn_ack && (SCP_READ32(c->offs) & pdn_ack) != pdn_ack) {
		cpu_relax();
		if (time_after(jiffies, timeout)) {
			dev_err(&mfg_dev->dev, "power off %d timeout, pdn_ack %x, %x\n", idx, pdn_ack, val);
			break;
		}
	}

	val |= PWR_ISO_BIT;
	SCP_WRITE32(val, c->offs);
	val &= ~PWR_RST_B_BIT;
	SCP_WRITE32(val, c->offs);
	val |= PWR_CLK_DIS_BIT;
	SCP_WRITE32(val, c->offs);

	val &= ~PWR_ON_BIT;
	SCP_WRITE32(val, c->offs);
	val &= ~PWR_ON_2ND_BIT;
	SCP_WRITE32(val, c->offs);

	timeout = jiffies + HZ;
	while (power_state(c) != 0) {
		cpu_relax();
		if (time_after(jiffies, timeout)) {
			dev_err(&mfg_dev->dev, "power off %d timeout\n", idx);
			break;
		}
	}

	return 0;
}
static int mtcmos_enable(int idx)
{
	return scp_enable(idx);
}

static int mtcmos_disable(int idx)
{
	return scp_disable(idx);
}


#else
struct device *mtcmos_dev[3];

static int mtcmos_enable(int idx)
{
	pm_runtime_get_sync(mtcmos_dev[idx]);
}

static int mtcmos_disable(int idx)
{
	pm_runtime_put_sync(mtcmos_dev[idx]);
}

static int mfg_device_async_probe(struct platform_device *pdev)
{
	mtcmos_dev[PD_ASYNC] = pdev;
	pm_runtime_enable(&pdev->dev);
	return 0;
}

static int mfg_device_async_remove(struct platform_device *pdev)
{
	mtcmos_dev[PD_ASYNC] = NULL;
	pm_runtime_disable(&pdev->dev);
	return 0;
}


static struct platform_driver mtk_mfg_async_driver = {
	.probe = mfg_device_async_probe,
	.remove = mfg_device_async_remove,
	.driver = {
		   .name = "mfg_async",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(mfg_async_dt_ids),
		   },
};

static int mfg_device_2d_probe(struct platform_device *pdev)
{
	mtcmos_dev[PD_2D] = pdev;
	pm_runtime_enable(&pdev->dev);
	return 0;
}

static int mfg_device_2d_remove(struct platform_device *pdev)
{
	mtcmos_dev[PD_2D] = NULL;
	pm_runtime_disable(&pdev->dev);
	return 0;
}
static struct platform_driver mtk_mfg_2d_driver = {
	.probe = mfg_device_2d_probe,
	.remove = mfg_device_2d_remove,
	.driver = {
		   .name = "mfg_2d",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(mfg_2d_dt_ids),
		   },
};


static int mfg_device_3d_probe(struct platform_device *pdev)
{
	mtcmos_dev[PD_3D] = pdev;
	pm_runtime_enable(&pdev->dev);
	return 0;
}

static int mfg_device_3d_remove(struct platform_device *pdev)
{
	mtcmos_dev[PD_3D] = NULL;
	pm_runtime_disable(&pdev->dev);
	return 0;
}
static struct platform_driver mtk_mfg_3d_driver = {
	.probe = mfg_device_3d_probe,
	.remove = mfg_device_3d_remove,
	.driver = {
		   .name = "mfg_3d",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(mfg_3d_dt_ids),
		   },
};

#endif

static void mfg_set_cg(void)
{
	MFG_WRITE32(CG_MFG_ALL, REG_MFG_CG_SET);
}

static void mfg_clr_cg(void)
{
	MFG_WRITE32(CG_MFG_ALL, REG_MFG_CG_CLR);
}

static void mfg_power_off(void)
{
	int i;

	mfg_set_cg();

	if (!enable_hwapm)
		mtcmos_disable(PD_3D);

	clk_disable_unprepare(clk_sel[MFG_CLK_CORE]);

	mtcmos_disable(PD_2D);

	mtcmos_disable(PD_ASYNC);

	clk_disable_unprepare(clk_sel[MFG_CLK_SLOW]);
	clk_disable_unprepare(clk_sel[MFG_CLK_AXI]);

	for (i = 0; i < MFG_CLK_COUNT; i++)
		clk_disable_unprepare(clk_parent[i]);

	if (vgpu)
		regulator_disable(vgpu);
}

static void mfg_power_on(void)
{
	int i;
	int ret;

	if (vgpu) {
		ret = regulator_enable(vgpu);
		if (ret)
			pr_err("regulator enable failed %d\n", ret);
	}

	for (i = 0; i < MFG_CLK_COUNT; i++) {
		clk_prepare_enable(clk_parent[i]);
		clk_set_parent(clk_sel[i], clk_parent[i]);
	}

	clk_prepare_enable(clk_sel[MFG_CLK_SLOW]);
	clk_prepare_enable(clk_sel[MFG_CLK_AXI]);

	mtcmos_enable(PD_ASYNC);
	mtcmos_enable(PD_2D);

	clk_prepare_enable(clk_sel[MFG_CLK_CORE]);

	if (!enable_hwapm)
		mtcmos_enable(PD_3D);
	else
		/* NOTE, this is to ensure the 3D state is off! */
		mtcmos_disable(PD_3D);
	mfg_clr_cg();
}

void mfg_dump_regs_seq(struct seq_file *m, const char *prefix)
{
	int i;

	if (m) {
		if (vgpu)
			seq_printf(m, "regulator: %d, %s\n",
				regulator_get_voltage(vgpu),
				regulator_is_enabled(vgpu)?"enabled":"disabled");
		else
			seq_puts(m, "regulator: static\n");

		for (i = 0; i < PD_MAX; i++)
			seq_printf(m, "Power: %d %d\n", i, power_state(&scp_pd[i]));

		for (i = 0; i < MFG_CLK_COUNT; i++) {
			struct clk *clk = clk_sel[i];
			struct clk *parent;

			if (clk) {
				seq_printf(m, "Clk: %s %d, rate: %lu\n",
					__clk_get_name(clk),
					__clk_get_enable_count(clk),
					clk_get_rate(clk));

				parent = clk_get_parent(clk);
				if (parent)
					seq_printf(m, "--Parent Clk: %s %d, rate :%lu\n\n",
						__clk_get_name(parent),
						__clk_get_enable_count(clk),
						clk_get_rate(parent));
				else
					seq_puts(m, "--Parent Clk : NONE\n\n");

			}
		}

		seq_hex_dump(m, "MFG ", DUMP_PREFIX_OFFSET, 4, 4, mfg_start, 0x1000, false);

	} else {
		if (vgpu)
			dev_info(&mfg_dev->dev, "%s regulator: %d, %s\n", prefix,
				regulator_get_voltage(vgpu),
				regulator_is_enabled(vgpu)?"enabled":"disabled");
		else
			dev_info(&mfg_dev->dev, "%s regulator: static\n", prefix);

		for (i = 0; i < PD_MAX; i++)
			dev_info(&mfg_dev->dev, "%s Power: %d %d\n", prefix, i, power_state(&scp_pd[i]));

		for (i = 0; i < MFG_CLK_COUNT; i++) {
			struct clk *clk = clk_sel[i];
			struct clk *parent;

			if (clk) {
				dev_info(&mfg_dev->dev, "%s Clk: %s %d, rate: %lu\n",
					prefix,
					__clk_get_name(clk),
					__clk_get_enable_count(clk),
					clk_get_rate(clk));

				parent = clk_get_parent(clk);
				if (parent)
					dev_info(&mfg_dev->dev, "%s--Parent Clk: %s %d, rate :%lu\n",
						prefix,
						__clk_get_name(parent),
						__clk_get_enable_count(clk),
						clk_get_rate(parent));
				else
					dev_info(&mfg_dev->dev, "%s--Parent Clk : NONE\n", prefix);

			}
		}
		print_hex_dump(KERN_INFO, prefix, DUMP_PREFIX_OFFSET, 4, 4, mfg_start, 0x1000, false);
	}
}

void mtk_mfg_dump_regs(const char *prefix)
{
	mfg_dump_regs_seq(NULL, prefix);
}

void mfg_write(u32 off, u32 val)
{
	MFG_WRITE32(val, off);
}

static void mfg_setup_hwapm(void)
{
	MFG_WRITE32(0x01a80000, 0x504);
	MFG_WRITE32(0x00080010, 0x508);
	MFG_WRITE32(0x00080010, 0x50c);
	MFG_WRITE32(0x00b800b8, 0x510);
	MFG_WRITE32(0x00b000b0, 0x514);
	MFG_WRITE32(0x00c000c8, 0x518);
	MFG_WRITE32(0x00c000c8, 0x51c);
	MFG_WRITE32(0x00d000d8, 0x520);
	MFG_WRITE32(0x00d800d8, 0x524);
	MFG_WRITE32(0x00d800d8, 0x528);
	MFG_WRITE32(0x9000001b, 0x24);
	MFG_WRITE32(0x8000001b, 0x24);
}

static void mfg_config_bw_ultra(void)
{
	if (bw_ultra_disable)
		MFG_WRITE32(0x1, 0x6c0);
}

void mtk_mfg_enable_gpu(void)
{
	mutex_lock(&enable_lock);

	enable_count++;
	/* If there is no error, this should not happen */
	if (enable_count != 1) {
		pr_err("ERR: MFG enable %d", enable_count);
		mutex_unlock(&enable_lock);
		return;
	}

#ifndef MFG_BRING_UP_ALL_ON
	mfg_power_on();

	mfg_config_bw_ultra();

	if (enable_hwapm)
		mfg_setup_hwapm();

#endif

	mutex_unlock(&enable_lock);
}

void mtk_mfg_disable_gpu(void)
{
	mutex_lock(&enable_lock);

	enable_count--;
	/* This should not happen */
	if (enable_count != 0) {
		pr_err("ERR: MFG disable %d", enable_count);
		mutex_unlock(&enable_lock);
		return;
	}

	/* Empty disable for all on */
#ifndef MFG_BRING_UP_ALL_ON
	mfg_power_off();
#endif /* MFG_BRING_UP_ALL_ON */

	mutex_unlock(&enable_lock);
}


bool mtk_mfg_is_ready(void)
{
	return (mfg_start != NULL);
}

unsigned long mtk_mfg_get_snap_freq(void)
{
	return current_freq;
}

unsigned long mtk_mfg_get_freq(void)
{
	if (clk_pll)
		return clk_get_rate(clk_pll);
	return current_freq;
}

void mtk_mfg_set_freq(unsigned long freq, bool hopping)
{
#ifndef MFG_BRING_UP_ALL_ON
	if (pll_support_hopping && hopping) {
		unsigned long vco;

		vco = div64_u64((u64)freq * 4 * 0x4000UL, 26000000UL);
		mtk_fhctl_hopping_by_id(FH_MM_PLLID,  vco);
	}

	current_freq = freq;

	/* For HW view, we don't need set rate here since hopping did it.
	 * But we want get rate value from CCF , and don't modify its no-cache flag
	 * currently.
	 */
	if (clk_pll)
		clk_set_rate(clk_pll, freq);
 #endif
}

bool mtk_mfg_dvfs_idle(void)
{
	return !pll_support_hopping;
}

int mtk_mfg_get_volt(void)
{
	if (vgpu)
		return regulator_get_voltage(vgpu);
	/* Volt is a fixed value from vcore. */
	return 800000;
}

int mtk_mfg_get_snap_volt(void)
{
	return current_volt;
}


int mtk_mfg_set_volt(int volt)
{
	int ret = 0;
#ifndef MFG_BRING_UP_ALL_ON
	if (vgpu)
		ret = regulator_set_voltage(vgpu, volt, volt);
#endif
	current_volt = volt;
	return ret;
}

bool mtk_mfg_can_set_volt(void)
{
	return vgpu != NULL;
}

bool mtk_mfg_is_hwapm(void)
{
	return enable_hwapm;
}

#define MFG_POWER_DYNAMIC 484 /* mw */
#define MFG_POWER_REF_FREQ 8000000  /* hz * 100 */
#define MFG_POWER_REF_VOLT 9000 /* mV * 100 */
#define MFG_POWER_LEAK  4 /* FIXME */
unsigned long mtk_mfg_calculate_power(unsigned long volt, unsigned long freq)
{
	return MFG_POWER_LEAK + MFG_POWER_DYNAMIC *
		(freq / MFG_POWER_REF_FREQ) *
		(volt / MFG_POWER_REF_VOLT) * (volt / MFG_POWER_REF_VOLT) /
		(100 * 10000);
}

void mtk_mfg_disable_bw_ultra(void)
{
	/* Set state, and apply directly to HW if power is on */
	mutex_lock(&enable_lock);
	bw_ultra_disable = true;
	if (enable_count > 0)
		mfg_config_bw_ultra();

	mutex_unlock(&enable_lock);

}

void mtk_mfg_enable_bw_ultra(void)
{
	mutex_lock(&enable_lock);
	bw_ultra_disable = false;
	if (enable_count > 0)
		mfg_config_bw_ultra();

	mutex_unlock(&enable_lock);
}

bool mtk_mfg_is_bw_ultra(void)
{
	return !bw_ultra_disable;
}

static int mfg_device_probe(struct platform_device *pdev)
{
	int i;

	pr_info("MFG device start probe, hwapm %d, hopping %d\n", enable_hwapm, pll_support_hopping);

	vgpu = devm_regulator_get_optional(&pdev->dev, "reg-vgpu");
	if (IS_ERR_OR_NULL(vgpu)) {
		if (PTR_ERR(vgpu) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		vgpu = NULL;
	}

	infracfg_reg = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
			"infracfg");

	mfg_dev = pdev;

	for (i = 0; i < MFG_CLK_COUNT; i++) {
		clk_parent[i] = devm_clk_get(&pdev->dev, top_mfg_clk_sel_parent_name[i]);
		if (IS_ERR(clk_parent[i])) {
			dev_err(&pdev->dev, "devm_clk_get %s failed !!!\n", top_mfg_clk_sel_parent_name[i]);
			goto error_out;
		}
	}

	for (i = 0; i < MFG_CLK_COUNT; i++) {
		clk_sel[i] = devm_clk_get(&pdev->dev, top_mfg_clk_sel_name[i]);
		if (IS_ERR(clk_sel[i])) {
			dev_err(&pdev->dev, "devm_clk_get %s failed !!!\n", top_mfg_clk_sel_name[i]);
			goto error_out;
		}
	}

	clk_pll = devm_clk_get(&pdev->dev, "pll");
	if (IS_ERR(clk_pll)) {
		dev_err(&pdev->dev, "pll clk get failed\n");
		clk_pll = NULL;
	}

	mfg_start = of_iomap(pdev->dev.of_node, 0);
	if (IS_ERR_OR_NULL(mfg_start)) {
		mfg_start = NULL;
		goto error_out;
	}

	scp_start = of_iomap(pdev->dev.of_node, 1);
	if (IS_ERR_OR_NULL(scp_start)) {
		scp_start = NULL;
		goto error_out;
	}

	pr_info("MFG start is mapped %p, scp %p\n", mfg_start, scp_start);

	return 0;

error_out:

	for (i = 0 ; i < MFG_CLK_COUNT; i++) {
		if (clk_parent[i])
			devm_clk_put(&pdev->dev, clk_parent[i]);
		if (clk_sel[i])
			devm_clk_put(&pdev->dev, clk_sel[i]);
	}

	if (mfg_start)
		iounmap(mfg_start);
	return -1;
}

static int mfg_device_remove(struct platform_device *pdev)
{
	int i;

	for (i = 0 ; i < MFG_CLK_COUNT; i++) {
		if (clk_parent[i])
			devm_clk_put(&pdev->dev, clk_parent[i]);
		if (clk_sel[i])
			devm_clk_put(&pdev->dev, clk_sel[i]);
	}

	if (vgpu && PTR_ERR(vgpu) != -EPROBE_DEFER)
		devm_regulator_put(vgpu);

	if (mfg_start)
		iounmap(mfg_start);

	mfg_dev = NULL;

	return 0;
}
static struct platform_driver mtk_mfg_driver = {
	.probe = mfg_device_probe,
	.remove = mfg_device_remove,
	.driver = {
		   .name = "mfg",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(mfg_dt_ids),
		   },
};

static int __init mfg_driver_init(void)
{
	int ret;

	pr_info("init mfg driver\n");

	ret = platform_driver_register(&mtk_mfg_driver);

#if !defined(MFG_DIRECT_POWER_CTRL)
	ret = platform_driver_register(&mtk_mfg_3d_driver);
	ret = platform_driver_register(&mtk_mfg_2d_driver);
	ret = platform_driver_register(&mtk_mfg_async_driver);
#endif

	return ret;
}

/* device_initcall(mfg_driver_init); */
module_init(mfg_driver_init)
