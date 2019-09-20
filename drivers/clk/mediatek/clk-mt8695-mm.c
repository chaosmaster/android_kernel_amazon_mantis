/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Chen Zhong <chen.zhong@mediatek.com>
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

#include <linux/clk-provider.h>
#include <linux/platform_device.h>

#include "clk-mtk.h"
#include "clk-gate.h"

#include <dt-bindings/clock/mt8695-clk.h>

static const struct mtk_gate_regs mm0_cg_regs = {
	.set_ofs = 0x104,
	.clr_ofs = 0x108,
	.sta_ofs = 0x100,
};

static const struct mtk_gate_regs mm1_cg_regs = {
	.set_ofs = 0x884,
	.clr_ofs = 0x884,
	.sta_ofs = 0x884,
};

#define GATE_MM0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &mm0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_MM1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &mm1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate mm_clks[] = {
	/* MM0 */
	GATE_MM0(CLK_MM_SMI_COMMON, "mm_smi_common", "mm_sel", 0),
	GATE_MM0(CLK_MM_SMI_LARB0, "mm_smi_larb0", "mm_sel", 1),
	GATE_MM0(CLK_MM_DRAM_LARB, "mm_d_larb", "mm_sel", 2),
	GATE_MM0(CLK_MM_FAKE_ENG, "mm_fake_eng", "mm_sel", 3),
	GATE_MM0(CLK_MM_SMI_LARB4, "mm_smi_larb4", "mm_sel", 4),
	GATE_MM0(CLK_MM_SMI_LARB1, "mm_smi_larb1", "mm_sel", 5),
	GATE_MM0(CLK_MM_SMI_LARB5, "mm_smi_larb5", "mm_sel", 6),
	GATE_MM0(CLK_MM_SMI_LARB6, "mm_smi_larb6", "mm_sel", 7),
	GATE_MM0(CLK_MM_SMI_LARB7, "mm_smi_larb7", "mm_sel", 8),
	GATE_MM0(CLK_MM_VDEC2IMG, "mm_vdec2img", "mm_sel", 9),
	GATE_MM0(CLK_MM_VDOUT, "mm_vdout", "mm_sel", 10),
	/* MM1 */
	GATE_MM1(CLK_MM_FMT_TG, "mm_fmt_tg", "sd_sel", 0),
	GATE_MM1(CLK_MM_FMT_DGO, "mm_fmt_dgo", "sd_sel", 1),
	GATE_MM1(CLK_MM_TVE, "mm_tve", "sd_sel", 2),
	GATE_MM1(CLK_MM_CRC, "mm_crc", "sd_sel", 3),
	GATE_MM1(CLK_MM_OSD_TVE, "mm_osd_tve", "osd_sel", 4),
	GATE_MM1(CLK_MM_OSD_FHD, "mm_osd_fhd", "osd_sel", 5),
	GATE_MM1(CLK_MM_OSD_UHD, "mm_osd_uhd", "osd_sel", 6),
	GATE_MM1(CLK_MM_P2I, "mm_p2i", "sd_sel", 7),
	GATE_MM1(CLK_MM_HDMITX, "mm_hdmitx", "sd_sel", 8),
	GATE_MM1(CLK_MM_RGB2HDMI, "mm_rgb2hdmi", "sd_sel", 9),
	GATE_MM1(CLK_MM_SCLER, "mm_scler", "sd_sel", 10),
	GATE_MM1(CLK_MM_SDPPF, "mm_sdppf", "sd_sel", 11),
	GATE_MM1(CLK_MM_VDOIN, "mm_vdoin", "sd_sel", 12),
	GATE_MM1(CLK_MM_DOLBY1, "mm_dolby1", "sd_sel", 13),
	GATE_MM1(CLK_MM_DOLBY2, "mm_dolby2", "sd_sel", 14),
	GATE_MM1(CLK_MM_DOLBY3, "mm_dolby3", "sd_sel", 15),
	GATE_MM1(CLK_MM_OSD_SDR2HDR, "mm_osd_sdr2hdr", "sd_sel", 16),
	GATE_MM1(CLK_MM_OSD_PREMIX, "mm_osd_premix", "sd_sel", 17),
	GATE_MM1(CLK_MM_DOLBY_MIX, "mm_dolby_mix", "sd_sel", 18),
	GATE_MM1(CLK_MM_VM, "mm_vm", "sd_sel", 19),
};

static int clk_mt8695_mm_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;

	clk_data = mtk_alloc_clk_data(CLK_MM_NR_CLK);

	mtk_clk_register_gates(node, mm_clks, ARRAY_SIZE(mm_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static const struct of_device_id of_match_clk_mt8695_mm[] = {
	{ .compatible = "mediatek,mt8695-mmsys", },
	{}
};

static struct platform_driver clk_mt8695_mm_drv = {
	.probe = clk_mt8695_mm_probe,
	.driver = {
		.name = "clk-mt8695-mm",
		.of_match_table = of_match_clk_mt8695_mm,
	},
};

static int __init clk_mt8695_mm_init(void)
{
	int ret;

	ret = platform_driver_register(&clk_mt8695_mm_drv);
	if (ret != 0) {
		pr_err("Failed to register SMI driver\n");
		return ret;
	}

	return 0;
}
arch_initcall(clk_mt8695_mm_init);
/* builtin_platform_driver(clk_mt8695_mm_drv);*/
