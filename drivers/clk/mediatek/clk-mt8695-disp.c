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

static const struct mtk_gate_regs disp_cg_regs = {
	.set_ofs = 0xC,
	.clr_ofs = 0xC,
	.sta_ofs = 0xC,
};

#define GATE_DISP(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &disp_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate disp_clks[] = {
	GATE_DISP(CLK_DISP_VDO3, "disp_vdo3", "vdo3_sel", 0),
	GATE_DISP(CLK_DISP_FMT3, "disp_fmt3", "mm_sel", 1),
	GATE_DISP(CLK_DISP_MV_HDR2SDR, "disp_mv_hdr2sdr", "mm_sel", 2),
	GATE_DISP(CLK_DISP_MV_BT2020, "disp_mv_bt2020", "mm_sel", 3),
	GATE_DISP(CLK_DISP_SMI_LARB5, "disp_smi_larb5", "mm_sel", 4),
	GATE_DISP(CLK_DISP_SMI_LARB6, "disp_smi_larb6", "mm_sel", 5),
	GATE_DISP(CLK_DISP_DRAMC_LARB8, "disp_dc_larb8", "mm_sel", 6),
	GATE_DISP(CLK_DISP_VDO4, "disp_vdo4", "vdo4_sel", 7),
	GATE_DISP(CLK_DISP_FMT4, "disp_fmt4", "mm_sel", 8),
	GATE_DISP(CLK_DISP_SV_HDR2SDR, "disp_sv_hdr2sdr", "mm_sel", 9),
	GATE_DISP(CLK_DISP_SV_BT2020, "disp_sv_bt2020", "mm_sel", 10),
	GATE_DISP(CLK_DISP_IRT_DMA, "disp_irt_dma", "mm_sel", 11),
	GATE_DISP(CLK_DISP_RSZ0, "disp_rsz0", "rsz_sel", 12),
	GATE_DISP(CLK_DISP_VDO_DI, "disp_vdo_di", "di_sel", 14),
	GATE_DISP(CLK_DISP_FMT_DI, "disp_fmt_di", "di_sel", 15),
	GATE_DISP(CLK_DISP_NR, "disp_nr", "nr_sel", 16),
	GATE_DISP(CLK_DISP_WR_CHANNEL, "disp_wr_channel", "mm_sel", 17),
};

static int clk_mt8695_disp_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;

	clk_data = mtk_alloc_clk_data(CLK_DISP_NR_CLK);

	mtk_clk_register_gates(node, disp_clks, ARRAY_SIZE(disp_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static const struct of_device_id of_match_clk_mt8695_disp[] = {
	{ .compatible = "mediatek,mt8695-disp", },
	{}
};

static struct platform_driver clk_mt8695_disp_drv = {
	.probe = clk_mt8695_disp_probe,
	.driver = {
		.name = "clk-mt8695-disp",
		.of_match_table = of_match_clk_mt8695_disp,
	},
};

builtin_platform_driver(clk_mt8695_disp_drv);
