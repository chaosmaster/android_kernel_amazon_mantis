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

static const struct mtk_gate_regs vdsoc0_cg_regs = {
	.set_ofs = 0xA0,
	.clr_ofs = 0xA4,
	.sta_ofs = 0xA0,
};

static const struct mtk_gate_regs vdsoc1_cg_regs = {
	.set_ofs = 0xB0,
	.clr_ofs = 0xB4,
	.sta_ofs = 0xB0,
};

#define GATE_VDSOC0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &vdsoc0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr_inv,	\
	}

#define GATE_VDSOC1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &vdsoc1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr_inv,	\
	}

static const struct mtk_gate vdsoc_clks[] = {
	/* VDSOC0 */
	GATE_VDSOC0(CLK_VDSOC_SOC, "vdsoc_soc", "vdec_sel", 0),
	/* VDSOC1 */
	GATE_VDSOC1(CLK_VDSOC_LARB7, "vdsoc_larb7", "vdec_sel", 0),
};

static int clk_mt8695_vdsoc_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;

	clk_data = mtk_alloc_clk_data(CLK_VDSOC_NR_CLK);

	mtk_clk_register_gates(node, vdsoc_clks, ARRAY_SIZE(vdsoc_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static const struct of_device_id of_match_clk_mt8695_vdsoc[] = {
	{ .compatible = "mediatek,mt8695-vdecsoc", },
	{}
};

static struct platform_driver clk_mt8695_vdsoc_drv = {
	.probe = clk_mt8695_vdsoc_probe,
	.driver = {
		.name = "clk-mt8695-vdsoc",
		.of_match_table = of_match_clk_mt8695_vdsoc,
	},
};

builtin_platform_driver(clk_mt8695_vdsoc_drv);
