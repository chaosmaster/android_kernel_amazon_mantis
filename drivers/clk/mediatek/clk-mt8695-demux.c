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

static const struct mtk_gate_regs demux_cg_regs = {
	.set_ofs = 0x4,
	.clr_ofs = 0x8,
	.sta_ofs = 0x0,
};

#define GATE_DEMUX(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &demux_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr_inv,	\
	}

static const struct mtk_gate demux_clks[] = {
	GATE_DEMUX(CLK_DEMUX_DMX_SMI, "demux_dmx_smi", "mm_sel", 0),
	GATE_DEMUX(CLK_DEMUX, "demux", "dmx_sel", 4),
};

static int clk_mt8695_demux_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;

	clk_data = mtk_alloc_clk_data(CLK_DEMUX_NR_CLK);

	mtk_clk_register_gates(node, demux_clks, ARRAY_SIZE(demux_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static const struct of_device_id of_match_clk_mt8695_demux[] = {
	{ .compatible = "mediatek,mt8695-demuxsys", },
	{}
};

static struct platform_driver clk_mt8695_demux_drv = {
	.probe = clk_mt8695_demux_probe,
	.driver = {
		.name = "clk-mt8695-demux",
		.of_match_table = of_match_clk_mt8695_demux,
	},
};

builtin_platform_driver(clk_mt8695_demux_drv);
