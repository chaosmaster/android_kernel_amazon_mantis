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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "clk-mtk.h"
#include "clk-gate.h"

#include <dt-bindings/clock/mt8695-clk.h>

static DEFINE_SPINLOCK(mt8695_clk_lock);

static const struct mtk_fixed_clk top_early_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_DMPLL, "dmpll_ck", NULL, 400000000),
};

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_CLKRTC_INT, "clkrtc_int", NULL, 32000),
};

static const struct mtk_fixed_factor top_divs[] = {
	FACTOR(CLK_TOP_ARMCA35PLL_600M, "armca35pll_600m", "armpll", 1, 2),
	FACTOR(CLK_TOP_ARMCA35PLL_400M, "armca35pll_400m", "armpll", 1, 3),
	FACTOR(CLK_TOP_SYSPLL, "syspll_ck", "mainpll", 1, 1),
	FACTOR(CLK_TOP_SYSPLL_D2, "syspll_d2", "syspll_ck", 1, 2),
	FACTOR(CLK_TOP_SYSPLL1_D2, "syspll1_d2", "syspll_d2", 1, 2),
	FACTOR(CLK_TOP_SYSPLL1_D4, "syspll1_d4", "syspll_d2", 1, 4),
	FACTOR(CLK_TOP_SYSPLL1_D16, "syspll1_d16", "syspll_d2", 1, 16),
	FACTOR(CLK_TOP_SYSPLL_D3, "syspll_d3", "syspll_ck", 1, 3),
	FACTOR(CLK_TOP_SYSPLL2_D2, "syspll2_d2", "syspll_d3", 1, 2),
	FACTOR(CLK_TOP_SYSPLL2_D4, "syspll2_d4", "syspll_d3", 1, 4),
	FACTOR(CLK_TOP_SYSPLL_D5, "syspll_d5", "syspll_ck", 1, 5),
	FACTOR(CLK_TOP_SYSPLL3_D2, "syspll3_d2", "syspll_d5", 1, 2),
	FACTOR(CLK_TOP_SYSPLL3_D4, "syspll3_d4", "syspll_d5", 1, 4),
	FACTOR(CLK_TOP_SYSPLL_D7, "syspll_d7", "syspll_ck", 1, 7),
	FACTOR(CLK_TOP_SYSPLL4_D2, "syspll4_d2", "syspll_d7", 1, 2),
	FACTOR(CLK_TOP_SYSPLL4_D4, "syspll4_d4", "syspll_d7", 1, 4),
	FACTOR(CLK_TOP_UNIVPLL, "univpll", "univ2pll", 1, 2),
	FACTOR(CLK_TOP_UNIVPLL_D7, "univpll_d7", "univpll", 1, 7),
	FACTOR(CLK_TOP_UNIVPLL_D26, "univpll_d26", "univpll", 1, 26),
	FACTOR(CLK_TOP_UNIVPLL_D52, "univpll_d52", "univpll", 1, 52),
	FACTOR(CLK_TOP_UNIVPLL_D2, "univpll_d2", "univpll", 1, 2),
	FACTOR(CLK_TOP_UNIVPLL1_D2, "univpll1_d2", "univpll_d2", 1, 2),
	FACTOR(CLK_TOP_UNIVPLL1_D4, "univpll1_d4", "univpll_d2", 1, 4),
	FACTOR(CLK_TOP_UNIVPLL1_D8, "univpll1_d8", "univpll_d2", 1, 8),
	FACTOR(CLK_TOP_UNIVPLL_D3, "univpll_d3", "univpll", 1, 3),
	FACTOR(CLK_TOP_UNIVPLL2_D2, "univpll2_d2", "univpll_d3", 1, 2),
	FACTOR(CLK_TOP_UNIVPLL2_D4, "univpll2_d4", "univpll_d3", 1, 4),
	FACTOR(CLK_TOP_UNIVPLL2_D8, "univpll2_d8", "univpll_d3", 1, 8),
	FACTOR(CLK_TOP_UNIVPLL_D5, "univpll_d5", "univpll", 1, 5),
	FACTOR(CLK_TOP_UNIVPLL3_D2, "univpll3_d2", "univpll_d5", 1, 2),
	FACTOR(CLK_TOP_UNIVPLL3_D4, "univpll3_d4", "univpll_d5", 1, 4),
	FACTOR(CLK_TOP_UNIVPLL3_D8, "univpll3_d8", "univpll_d5", 1, 8),
	FACTOR(CLK_TOP_F_MP0_PLL1, "f_mp0_pll1_ck", "univpll_d2", 1, 1),
	FACTOR(CLK_TOP_F_MP0_PLL2, "f_mp0_pll2_ck", "univpll1_d2", 1, 1),
	FACTOR(CLK_TOP_APLL1, "apll1_ck", "apll1", 1, 1),
	FACTOR(CLK_TOP_APLL1_D2, "apll1_d2", "apll1_ck", 1, 2),
	FACTOR(CLK_TOP_APLL1_D4, "apll1_d4", "apll1_ck", 1, 4),
	FACTOR(CLK_TOP_APLL1_D8, "apll1_d8", "apll1_ck", 1, 8),
	FACTOR(CLK_TOP_APLL1_D16, "apll1_d16", "apll1_ck", 1, 16),
	FACTOR(CLK_TOP_APLL2, "apll2_ck", "apll2", 1, 1),
	FACTOR(CLK_TOP_APLL2_D2, "apll2_d2", "apll2_ck", 1, 2),
	FACTOR(CLK_TOP_APLL2_D4, "apll2_d4", "apll2_ck", 1, 4),
	FACTOR(CLK_TOP_APLL2_D8, "apll2_d8", "apll2_ck", 1, 8),
	FACTOR(CLK_TOP_APLL2_D16, "apll2_d16", "apll2_ck", 1, 16),
	FACTOR(CLK_TOP_OSDPLL, "osdpll_ck", "osdpll", 1, 1),
	FACTOR(CLK_TOP_OSDPLL_D2, "osdpll_d2", "osdpll", 1, 2),
	FACTOR(CLK_TOP_OSDPLL_D3, "osdpll_d3", "osdpll", 1, 3),
	FACTOR(CLK_TOP_OSDPLL_D4, "osdpll_d4", "osdpll", 1, 4),
	FACTOR(CLK_TOP_OSDPLL_D6, "osdpll_d6", "osdpll", 1, 6),
	FACTOR(CLK_TOP_OSDPLL_D12, "osdpll_d12", "osdpll", 1, 12),
	FACTOR(CLK_TOP_ETHERPLL_125M, "etherpll_125m", "etherpll", 1, 1),
	FACTOR(CLK_TOP_ETHERPLL_50M, "etherpll_50m", "etherpll", 1, 1),
	FACTOR(CLK_TOP_SYS_26M, "sys_26m", "clk26m", 1, 1),
	FACTOR(CLK_TOP_MMPLL, "mmpll_ck", "mmpll", 1, 1),
	FACTOR(CLK_TOP_MMPLL_D2, "mmpll_d2", "mmpll_ck", 1, 2),
	FACTOR(CLK_TOP_VDECPLL, "vdecpll_ck", "vdecpll", 1, 1),
	FACTOR(CLK_TOP_TVDPLL, "tvdpll_ck", "tvdpll", 1, 1),
	FACTOR(CLK_TOP_TVDPLL_D2, "tvdpll_d2", "tvdpll_ck", 1, 2),
	FACTOR(CLK_TOP_TVDPLL_D4, "tvdpll_d4", "tvdpll_ck", 1, 4),
	FACTOR(CLK_TOP_TVDPLL_D8, "tvdpll_d8", "tvdpll_ck", 1, 8),
	FACTOR(CLK_TOP_MSDCPLL, "msdcpll_ck", "msdcpll", 1, 1),
	FACTOR(CLK_TOP_MSDCPLL_D2, "msdcpll_d2", "msdcpll_ck", 1, 2),
	FACTOR(CLK_TOP_MSDCPLL_D4, "msdcpll_d4", "msdcpll_ck", 1, 4),
	FACTOR(CLK_TOP_CLK26M_D2, "clk26m_d2", "sys_26m", 1, 2),
	FACTOR(CLK_TOP_NFI1X, "nfi1x", "nfi2x_sel", 1, 2),
};

static const char * const hdmitx20_cksel_parents[] = {
	"tvdpll_d8",
	"osdpll_d12"
};

static struct mtk_composite apmixed_muxes[] = {
	/* HDMITX20_CKSEL */
	MUX(CLK_APMIXED_HDMITX20_CKSEL, "hdmitx20_cksel", hdmitx20_cksel_parents, 0x48, 16, 1),
};

static const char * const axi_parents[] = {
	"clk26m",
	"syspll1_d2",
	"syspll_d5",
	"syspll1_d4",
	"univpll_d5",
	"univpll2_d2",
	"msdcpll_ck"
};

static const char * const mem_parents[] = {
	"clk26m",
	"dmpll_ck"
};

static const char * const sd_parents[] = {
	"clk26m",
	"osdpll_d6"
};

static const char * const mm_parents[] = {
	"clk26m",
	"msdcpll_ck",
	"syspll_d3",
	"syspll1_d2",
	"syspll_d5",
	"syspll1_d4",
	"univpll1_d2",
	"univpll2_d2"
};

static const char * const vdec_lae_parents[] = {
	"clk26m",
	"vdecpll_ck",
	"osdpll_d2",
	"univpll_d3",
	"msdcpll_ck",
	"syspll_d3",
	"univpll1_d2",
	"mmpll_d2",
	"syspll3_d2",
	"tvdpll_ck"
};

static const char * const venc_parents[] = {
	"clk26m",
	"univpll1_d2",
	"mmpll_d2",
	"tvdpll_d2",
	"syspll1_d2",
	"univpll_d5",
	"syspll_d5",
	"univpll2_d2",
	"syspll3_d2"
};

static const char * const mfg_parents[] = {
	"clk26m",
	"mmpll_ck",
	"dmpll_ck",
	"syspll_d2",
	"univpll_d2",
	"vdecpll_ck",
	"tvdpll_ck",
	"osdpll_ck",
	"clk26m",
	"syspll_d3",
	"syspll1_d2",
	"syspll_d5",
	"univpll_d3",
	"univpll1_d2",
	"univpll_d5",
	"univpll2_d2"
};

static const char * const rsz_parents[] = {
	"clk26m",
	"syspll_d2",
	"mmpll_ck",
	"univpll_d3",
	"osdpll_d2",
	"univpll_d5"
};

static const char * const uart_parents[] = {
	"clk26m",
	"univpll2_d8"
};

static const char * const spi_parents[] = {
	"clk26m",
	"univpll2_d4",
	"univpll1_d4",
	"univpll2_d2",
	"univpll3_d2",
	"univpll1_d8"
};

static const char * const usb20_parents[] = {
	"clk26m",
	"univpll1_d8",
	"univpll3_d4"
};

static const char * const usb30_parents[] = {
	"clk26m",
	"univpll3_d2",
	"univpll3_d4",
	"univpll2_d4"
};

static const char * const msdc50_0_h_parents[] = {
	"clk26m",
	"syspll1_d2",
	"syspll2_d2",
	"msdcpll_d4",
	"univpll1_d4",
	"syspll4_d2",
	"msdcpll_d2",
	"univpll1_d4"
};

static const char * const msdc50_0_parents[] = {
	"clk26m",
	"msdcpll_ck",
	"msdcpll_d2",
	"osdpll_d2",
	"syspll2_d2",
	"msdcpll_d4",
	"univpll2_d2",
	"univpll1_d2"
};

static const char * const msdc30_1_parents[] = {
	"clk26m",
	"univpll2_d2",
	"msdcpll_d2",
	"univpll1_d4",
	"syspll2_d2",
	"msdcpll_d4",
	"syspll1_d4",
	"univpll1_d8"
};

static const char * const msdc30_2_parents[] = {
	"clk26m",
	"msdcpll_ck",
	"msdcpll_d2",
	"univpll1_d4",
	"syspll2_d2",
	"univpll_d7",
	"univpll_d3",
	"univpll2_d2"
};

static const char * const msdc0p_aes_parents[] = {
	"clk26m",
	"msdcpll_ck",
	"univpll_d3",
	"univpll2_d2"
};

static const char * const intdir_parents[] = {
	"clk26m",
	"univpll_d3",
	"syspll_d2",
	"tvdpll_ck",
	"univpll_d2"
};

static const char * const audio_parents[] = {
	"clk26m",
	"syspll3_d4",
	"syspll4_d4",
	"syspll1_d16"
};

static const char * const aud_intbus_parents[] = {
	"clk26m",
	"syspll1_d4",
	"syspll4_d2",
	"univpll3_d2",
	"univpll2_d8",
	"syspll3_d2",
	"syspll3_d4"
};

static const char * const osd_parents[] = {
	"clk26m",
	"osdpll_ck",
	"osdpll_d2",
	"osdpll_d4",
	"osdpll_d6",
	"osdpll_d12",
	"univpll_d2",
	"univpll1_d2"
};

static const char * const vdo3_parents[] = {
	"clk26m",
	"osdpll_ck",
	"tvdpll_ck",
	"tvdpll_d2",
	"osdpll_d2",
	"osdpll_d3",
	"osdpll_d4"
};

static const char * const vdo4_parents[] = {
	"clk26m",
	"osdpll_d3",
	"osdpll_d4"
};

static const char * const hd_parents[] = {
	"clk26m",
	"tvdpll_ck"
};

static const char * const nr_parents[] = {
	"clk26m",
	"univpll1_d4",
	"syspll2_d2",
	"syspll1_d4",
	"univpll1_d8",
	"univpll3_d2",
	"univpll2_d2",
	"syspll_d5"
};

static const char * const cci400_parents[] = {
	"clk26m",
	"univpll2_d2",
	"armca35pll_600m",
	"armca35pll_400m",
	"univpll_d2",
	"syspll_d2",
	"msdcpll_ck",
	"univpll_d3"
};

static const char * const aud_1_parents[] = {
	"clk26m",
	"apll1_ck",
	"univpll2_d4",
	"univpll2_d8"
};

static const char * const aud_2_parents[] = {
	"clk26m",
	"apll2_ck",
	"univpll2_d4",
	"univpll2_d8"
};

static const char * const mem_mfg_parents[] = {
	"clk26m",
	"mmpll_ck",
	"univpll_d3"
};

static const char * const slow_mfg_parents[] = {
	"clk26m",
	"univpll2_d4",
	"univpll2_d8"
};

static const char * const axi_mfg_parents[] = {
	"clk26m",
	"univpll1_d2",
	"axi_sel",
	"mmpll_ck"
};

static const char * const hdcp_parents[] = {
	"clk26m",
	"syspll4_d2",
	"syspll3_d4",
	"univpll2_d4"
};

static const char * const hdcp_24m_parents[] = {
	"clk26m",
	"univpll_d26",
	"univpll_d52",
	"univpll2_d8"
};

static const char * const clk32k_parents[] = {
	"clkrtc_int",
	"clkrtc_int",
	"clk26m",
	"univpll3_d8"
};

static const char * const spinor_parents[] = {
	"clk26m",
	"clk26m_d2",
	"syspll4_d4",
	"univpll2_d8",
	"univpll3_d4",
	"syspll4_d2",
	"syspll2_d4",
	"univpll2_d4",
	"univpll3_d2",
	"syspll1_d4"
};

static const char * const apll_parents[] = {
	"clk26m",
	"apll1_ck",
	"apll1_d2",
	"apll1_d4",
	"apll1_d8",
	"apll1_d16",
	"apll2_ck",
	"apll2_d2",
	"apll2_d4",
	"apll2_d8",
	"apll2_d16",
	"clk26m",
	"clk26m"
};

static const char * const a1sys_hp_parents[] = {
	"clk26m",
	"apll1_ck",
	"apll1_d2",
	"apll1_d4",
	"apll1_d8"
};

static const char * const a2sys_hp_parents[] = {
	"clk26m",
	"apll2_ck",
	"apll2_d2",
	"apll2_d4",
	"apll2_d8"
};

static const char * const asm_l_parents[] = {
	"clk26m",
	"univpll2_d4",
	"univpll2_d2",
	"syspll_d5"
};

static const char * const i2so1_parents[] = {
	"clk26m",
	"apll1_ck",
	"apll2_ck"
};

static const char * const ether_125m_parents[] = {
	"clk26m",
	"etherpll_125m",
	"univpll3_d2"
};

static const char * const ether_50m_parents[] = {
	"clk26m",
	"etherpll_50m",
	"univpll_d26"
};

static const char * const spislv_parents[] = {
	"clk26m",
	"univpll2_d4",
	"univpll1_d4",
	"univpll2_d2",
	"univpll3_d2",
	"univpll1_d8",
	"univpll1_d2",
	"univpll_d5"
};

static const char * const i2c_parents[] = {
	"clk26m",
	"univpll_d26",
	"univpll2_d4",
	"univpll3_d2",
	"univpll1_d4"
};

static const char * const pwm_infra_parents[] = {
	"clk26m",
	"univpll2_d4",
	"univpll3_d2",
	"univpll1_d4"
};

static const char * const gcpu_parents[] = {
	"clk26m",
	"syspll_d3",
	"syspll1_d2",
	"univpll1_d2",
	"univpll_d5",
	"univpll3_d2",
	"univpll_d3"
};

static const char * const ecc_parents[] = {
	"clk26m",
	"univpll2_d2",
	"univpll1_d2",
	"univpll_d3",
	"syspll_d2",
	"univpll_d2",
	"syspll_d3",
	"univpll_d3"
};

static const char * const di_parents[] = {
	"clk26m",
	"tvdpll_d2",
	"tvdpll_d4",
	"tvdpll_d8",
	"osdpll_d3",
	"osdpll_d4",
	"osdpll_d6",
	"osdpll_d12"
};

static const char * const dmx_parents[] = {
	"clk26m",
	"osdpll_d2",
	"tvdpll_d2",
	"syspll1_d2",
	"mmpll_d2",
	"osdpll_d3",
	"msdcpll_d2",
	"apll2_ck"
};

static const char * const nfi2x_parents[] = {
	"clk26m",
	"univpll2_d8",
	"univpll1_d4",
	"syspll2_d2",
	"msdcpll_d2",
	"mmpll_d2",
	"syspll1_d2",
	"syspll_d3"
};

static const char * const tdmin_parents[] = {
	"apll1_d4",
	"apll2_d4",
	"apll1_d8",
	"apll2_d8",
	"apll1_d16",
	"apll2_d16",
	"i2si1_sel",
	"i2si2_sel",
	"i2so1_sel",
	"i2so2_sel"
};

static struct mtk_composite top_early_muxes[] = {
	MUX_GATE(CLK_TOP_MEM_SEL, "mem_sel", mem_parents, 0x040, 8, 1, 15),
};

static struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE(CLK_TOP_AXI_SEL, "axi_sel", axi_parents, 0x040, 0, 3, 7),
	MUX_GATE(CLK_TOP_SD_SEL, "sd_sel", sd_parents, 0x040, 16, 1, 23),
	MUX_GATE(CLK_TOP_MM_SEL, "mm_sel", mm_parents, 0x040, 24, 3, 31),
	/* CLK_CFG_1 */
	MUX_GATE(CLK_TOP_VDEC_LAE_SEL, "vdec_lae_sel", vdec_lae_parents, 0x050, 0, 4, 7),
	MUX_GATE(CLK_TOP_VDEC_SEL, "vdec_sel", vdec_lae_parents, 0x050, 8, 4, 15),
	MUX_GATE(CLK_TOP_VDEC_SLOW_SEL, "vdec_slow_sel", vdec_lae_parents, 0x050, 16, 4, 23),
	MUX_GATE(CLK_TOP_VENC_SEL, "venc_sel", venc_parents, 0x050, 24, 4, 31),
	/* CLK_CFG_2 */
	MUX_GATE(CLK_TOP_MFG_SEL, "mfg_sel", mfg_parents, 0x060, 0, 4, 7),
	MUX_GATE(CLK_TOP_RSZ_SEL, "rsz_sel", rsz_parents, 0x060, 8, 3, 15),
	MUX_GATE(CLK_TOP_UART_SEL, "uart_sel", uart_parents, 0x060, 16, 1, 23),
	MUX_GATE(CLK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x060, 24, 3, 31),
	/* CLK_CFG_3 */
	MUX_GATE(CLK_TOP_USB20_SEL, "usb20_sel", usb20_parents, 0x070, 0, 2, 7),
	MUX_GATE(CLK_TOP_USB30_SEL, "usb30_sel", usb30_parents, 0x070, 8, 2, 15),
	MUX_GATE(CLK_TOP_MSDC50_0_HCLK_SEL, "msdc50_0_h_sel", msdc50_0_h_parents, 0x070, 16, 3, 23),
	MUX_GATE(CLK_TOP_MSDC50_0_SEL, "msdc50_0_sel", msdc50_0_parents, 0x070, 24, 3, 31),
	/* CLK_CFG_4 */
	MUX_GATE(CLK_TOP_MSDC30_1_SEL, "msdc30_1_sel", msdc30_1_parents, 0x080, 0, 3, 7),
	MUX_GATE(CLK_TOP_MSDC30_2_SEL, "msdc30_2_sel", msdc30_2_parents, 0x080, 8, 3, 15),
	MUX_GATE(CLK_TOP_MSDC50_2_HCLK_SEL, "msdc50_2_h_sel", msdc50_0_h_parents, 0x080, 16, 3, 23),
	MUX_GATE(CLK_TOP_MSDC0P_AES_SEL, "msdc0p_aes_sel", msdc0p_aes_parents, 0x080, 24, 2, 31),
	/* CLK_CFG_5 */
	MUX_GATE(CLK_TOP_INTDIR_SEL, "intdir_sel", intdir_parents, 0x090, 0, 3, 7),
	MUX_GATE(CLK_TOP_AUDIO_SEL, "audio_sel", audio_parents, 0x090, 8, 2, 15),
	MUX_GATE(CLK_TOP_AUD_INTBUS_SEL, "aud_intbus_sel", aud_intbus_parents, 0x090, 16, 3, 23),
	MUX_GATE(CLK_TOP_OSD_SEL, "osd_sel", osd_parents, 0x090, 24, 3, 31),
	/* CLK_CFG_6 */
	MUX_GATE(CLK_TOP_VDO3_SEL, "vdo3_sel", vdo3_parents, 0x0a0, 0, 3, 7),
	MUX_GATE(CLK_TOP_VDO4_SEL, "vdo4_sel", vdo4_parents, 0x0a0, 8, 2, 15),
	MUX_GATE(CLK_TOP_HD_SEL, "hd_sel", hd_parents, 0x0a0, 16, 1, 23),
	MUX_GATE(CLK_TOP_NR_SEL, "nr_sel", nr_parents, 0x0a0, 24, 3, 31),
	/* CLK_CFG_7 */
	MUX_GATE(CLK_TOP_CCI400_SEL, "cci400_sel", cci400_parents, 0x0b0, 0, 3, 7),
	MUX_GATE(CLK_TOP_AUD_1_SEL, "aud_1_sel", aud_1_parents, 0x0b0, 8, 2, 15),
	MUX_GATE(CLK_TOP_AUD_2_SEL, "aud_2_sel", aud_2_parents, 0x0b0, 16, 2, 23),
	MUX_GATE(CLK_TOP_MEM_MFG_IN_AS_SEL, "mem_mfg_sel", mem_mfg_parents, 0x0b0, 24, 2, 31),
	/* CLK_CFG_8 */
	MUX_GATE(CLK_TOP_SLOW_MFG_SEL, "slow_mfg_sel", slow_mfg_parents, 0x0c0, 0, 2, 7),
	MUX_GATE(CLK_TOP_AXI_MFG_IN_AS_SEL, "axi_mfg_sel", axi_mfg_parents, 0x0c0, 8, 2, 15),
	MUX_GATE(CLK_TOP_HDCP_SEL, "hdcp_sel", hdcp_parents, 0x0c0, 16, 2, 23),
	MUX_GATE(CLK_TOP_HDCP_24M_SEL, "hdcp_24m_sel", hdcp_24m_parents, 0x0c0, 24, 2, 31),
	/* CLK_CFG_9 */
	MUX_GATE(CLK_TOP_CLK32K, "clk32k", clk32k_parents, 0x0d0, 0, 2, 7),
	MUX_GATE(CLK_TOP_SPINOR_SEL, "spinor_sel", spinor_parents, 0x0d0, 8, 4, 15),
	MUX_GATE(CLK_TOP_APLL_SEL, "apll_sel", apll_parents, 0x0d0, 16, 4, 23),
	MUX_GATE(CLK_TOP_APLL2_SEL, "apll2_sel", apll_parents, 0x0d0, 24, 4, 31),
	/* CLK_CFG_10 */
	MUX_GATE(CLK_TOP_A1SYS_HP_SEL, "a1sys_hp_sel", a1sys_hp_parents, 0x500, 0, 3, 7),
	MUX_GATE(CLK_TOP_A2SYS_HP_SEL, "a2sys_hp_sel", a2sys_hp_parents, 0x500, 8, 3, 15),
	MUX_GATE(CLK_TOP_ASM_L_SEL, "asm_l_sel", asm_l_parents, 0x500, 16, 2, 23),
	MUX_GATE(CLK_TOP_ASM_M_SEL, "asm_m_sel", asm_l_parents, 0x500, 24, 2, 31),
	/* CLK_CFG_11 */
	MUX_GATE(CLK_TOP_ASM_H_SEL, "asm_h_sel", asm_l_parents, 0x510, 0, 2, 7),
	MUX_GATE(CLK_TOP_I2SO1_SEL, "i2so1_sel", i2so1_parents, 0x510, 8, 2, 15),
	MUX_GATE(CLK_TOP_I2SO2_SEL, "i2so2_sel", i2so1_parents, 0x510, 16, 2, 23),
	/* CLK_CFG_12 */
	MUX_GATE(CLK_TOP_I2SI1_SEL, "i2si1_sel", i2so1_parents, 0x520, 0, 2, 7),
	MUX_GATE(CLK_TOP_I2SI2_SEL, "i2si2_sel", i2so1_parents, 0x520, 8, 2, 15),
	MUX_GATE(CLK_TOP_ETHER_125M_SEL, "ether_125m_sel", ether_125m_parents, 0x520, 16, 2, 23),
	MUX_GATE(CLK_TOP_ETHER_50M_SEL, "ether_50m_sel", ether_50m_parents, 0x520, 24, 2, 31),
	/* CLK_CFG_13 */
	MUX_GATE(CLK_TOP_SPISLV_SEL, "spislv_sel", spislv_parents, 0x530, 0, 3, 7),
	MUX_GATE(CLK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x530, 8, 3, 15),
	MUX_GATE(CLK_TOP_PWM_INFRA_SEL, "pwm_infra_sel", pwm_infra_parents, 0x530, 16, 2, 23),
	MUX_GATE(CLK_TOP_GCPU_SEL, "gcpu_sel", gcpu_parents, 0x530, 24, 3, 31),
	/* CLK_CFG_14 */
	MUX_GATE(CLK_TOP_ECC_SEL, "ecc_sel", ecc_parents, 0x540, 0, 3, 7),
	MUX_GATE(CLK_TOP_DI_SEL, "di_sel", di_parents, 0x540, 8, 3, 15),
	MUX_GATE(CLK_TOP_DMX_SEL, "dmx_sel", dmx_parents, 0x540, 16, 3, 23),
	MUX_GATE(CLK_TOP_NFI2X_SEL, "nfi2x_sel", nfi2x_parents, 0x540, 24, 3, 31),
	/* CLK_AUDDIV_2 */
	MUX(CLK_TOP_TDMIN_SEL, "tdmin_sel", tdmin_parents, 0x128, 4, 4),
};

static const char * const mcu_bus_parents[] = {
	"clk26m",
	"armpll",
	"f_mp0_pll1_ck",
	"f_mp0_pll2_ck"
};

static struct mtk_composite mcu_muxes[] = {
	/* bus_pll_divider_cfg */
	MUX(CLK_MCU_BUS_SEL, "mcu_bus_sel", mcu_bus_parents, 0x7C0, 9, 2),
};

static const char * const per_uart0_parents[] = {
	"clk26m",
	"uart_sel"
};

static struct mtk_composite peri_muxes[] = {
	/* PERI_UART_CK_SOURCE_SEL */
	MUX(CLK_PERI_UART0_SEL, "per_uart0_sel", per_uart0_parents, 0x40C, 0, 1),
	MUX(CLK_PERI_UART1_SEL, "per_uart1_sel", per_uart0_parents, 0x40C, 1, 1),
	MUX(CLK_PERI_UART2_SEL, "per_uart2_sel", per_uart0_parents, 0x40C, 2, 1),
};

static const struct mtk_clk_divider top_adj_divs[] = {
	DIV_ADJ_FLAGS(CLK_TOP_APLL_DIV0, "apll_div0", "i2so1_sel", 0x124, 0, 8, CLK_DIVIDER_ROUND_CLOSEST),
	DIV_ADJ_FLAGS(CLK_TOP_APLL_DIV1, "apll_div1", "i2so2_sel", 0x124, 8, 8, CLK_DIVIDER_ROUND_CLOSEST),
	DIV_ADJ_FLAGS(CLK_TOP_APLL_DIV5, "apll_div5", "i2si1_sel", 0x128, 8, 8, CLK_DIVIDER_ROUND_CLOSEST),
	DIV_ADJ_FLAGS(CLK_TOP_APLL_DIV6, "apll_div6", "i2si2_sel", 0x128, 16, 8, CLK_DIVIDER_ROUND_CLOSEST),
};

static const struct mtk_gate_regs top0_cg_regs = {
	.set_ofs = 0x120,
	.clr_ofs = 0x120,
	.sta_ofs = 0x120,
};

static const struct mtk_gate_regs top1_cg_regs = {
	.set_ofs = 0x424,
	.clr_ofs = 0x424,
	.sta_ofs = 0x424,
};

#define GATE_TOP0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &top0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr,	\
	}

#define GATE_TOP1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &top1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate top_clks[] = {
	/* TOP0 */
	GATE_TOP0(CLK_TOP_APLL_DIV_PDN0, "apll_div_pdn0", "i2so1_sel", 0),
	GATE_TOP0(CLK_TOP_APLL_DIV_PDN1, "apll_div_pdn1", "i2so2_sel", 1),
	GATE_TOP0(CLK_TOP_APLL_DIV_PDN5, "apll_div_pdn5", "i2si1_sel", 5),
	GATE_TOP0(CLK_TOP_APLL_DIV_PDN6, "apll_div_pdn6", "i2si2_sel", 6),
	/* TOP1 */
	GATE_TOP1(CLK_TOP_NFI2X_EN, "nfi2x_en", "nfi2x_sel", 0),
	GATE_TOP1(CLK_TOP_NFI1X_EN, "nfi1x_en", "nfi1x", 2),
};

static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x80,
	.clr_ofs = 0x84,
	.sta_ofs = 0x88,
};

#define GATE_INFRA0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &infra0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_INFRA1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &infra1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

static const struct mtk_gate infra_early_clks[] = {
	GATE_INFRA0(CLK_INFRA_M4U, "infra_m4u", "mem_sel", 8),
};

static const struct mtk_gate infra_clks[] = {
	/* INFRA0 */
	GATE_INFRA0(CLK_INFRA_DBGCLK, "infra_dbgclk", "axi_sel", 0),
	GATE_INFRA0(CLK_INFRA_GCE, "infra_gce", "axi_sel", 6),
	GATE_INFRA0(CLK_INFRA_KP, "infra_kp", "axi_sel", 16),
	GATE_INFRA0(CLK_INFRA_CEC, "infra_cec", "clk32k", 18),
	/* INFRA1 */
	GATE_INFRA1(CLK_INFRA_TRNG, "infra_trng", "axi_sel", 0),
};

static const struct mtk_gate_regs peri0_cg_regs = {
	.set_ofs = 0x8,
	.clr_ofs = 0x10,
	.sta_ofs = 0x18,
};

static const struct mtk_gate_regs peri1_cg_regs = {
	.set_ofs = 0xc,
	.clr_ofs = 0x14,
	.sta_ofs = 0x1c,
};

static const struct mtk_gate_regs peri2_cg_regs = {
	.set_ofs = 0x42c,
	.clr_ofs = 0x42c,
	.sta_ofs = 0x42c,
};

#define GATE_PERI0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &peri0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_PERI1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &peri1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_PERI1_I(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &peri1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr_inv,	\
	}

#define GATE_PERI2(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &peri2_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate peri_clks[] = {
	/* PERI0 */
	GATE_PERI0(CLK_PERI_NFI, "per_nfi", "axi_sel", 0),
	GATE_PERI0(CLK_PERI_THERM, "per_therm", "axi_sel", 1),
	GATE_PERI0(CLK_PERI_PWM0, "per_pwm0", "pwm_infra_sel", 2),
	GATE_PERI0(CLK_PERI_PWM1, "per_pwm1", "pwm_infra_sel", 3),
	GATE_PERI0(CLK_PERI_PWM2, "per_pwm2", "pwm_infra_sel", 4),
	GATE_PERI0(CLK_PERI_PWM3, "per_pwm3", "pwm_infra_sel", 5),
	GATE_PERI0(CLK_PERI_PWM4, "per_pwm4", "pwm_infra_sel", 6),
	GATE_PERI0(CLK_PERI_PWM5, "per_pwm5", "pwm_infra_sel", 7),
	GATE_PERI0(CLK_PERI_PWM6, "per_pwm6", "pwm_infra_sel", 8),
	GATE_PERI0(CLK_PERI_PWM7, "per_pwm7", "pwm_infra_sel", 9),
	GATE_PERI0(CLK_PERI_PWM, "per_pwm", "pwm_infra_sel", 10),
	GATE_PERI0(CLK_PERI_USB, "per_usb", "axi_sel", 11),
	GATE_PERI0(CLK_PERI_AP_DMA, "per_ap_dma", "axi_sel", 13),
	GATE_PERI0(CLK_PERI_MSDC30_0, "per_msdc30_0", "msdc50_0_sel", 14),
	GATE_PERI0(CLK_PERI_MSDC30_1, "per_msdc30_1", "msdc30_1_sel", 15),
	GATE_PERI0(CLK_PERI_MSDC30_2, "per_msdc30_2", "msdc30_2_sel", 16),
	GATE_PERI0(CLK_PERI_UART0, "per_uart0", "per_uart0_sel", 20),
	GATE_PERI0(CLK_PERI_UART1, "per_uart1", "per_uart1_sel", 21),
	GATE_PERI0(CLK_PERI_UART2, "per_uart2", "per_uart2_sel", 22),
	GATE_PERI0(CLK_PERI_I2C0, "per_i2c0", "axi_sel", 24),
	GATE_PERI0(CLK_PERI_I2C1, "per_i2c1", "axi_sel", 25),
	GATE_PERI0(CLK_PERI_I2C2, "per_i2c2", "axi_sel", 26),
	GATE_PERI0(CLK_PERI_I2C3, "per_i2c3", "axi_sel", 27),
	GATE_PERI0(CLK_PERI_I2C4, "per_i2c4", "axi_sel", 28),
	GATE_PERI0(CLK_PERI_AUXADC, "per_auxadc", "sys_26m", 29),
	GATE_PERI0(CLK_PERI_SPI0, "per_spi0", "spi_sel", 30),
	/* PERI1 */
	GATE_PERI1(CLK_PERI_SFLASH, "per_sflash", "spinor_sel", 1),
	GATE_PERI1(CLK_PERI_SPI2, "per_spi2", "spi_sel", 5),
	GATE_PERI1(CLK_PERI_HAXI_SFLASH, "per_haxi_sflash", "axi_sel", 11),
	GATE_PERI1(CLK_PERI_GMAC, "per_gmac", "axi_sel", 12),
	GATE_PERI1(CLK_PERI_GMAC_PCLK, "per_gmac_pclk", "axi_sel", 16),
	GATE_PERI1_I(CLK_PERI_PTP_THERM, "per_ptp_therm", "sys_26m", 17),
	/* PERI2 */
	GATE_PERI2(CLK_PERI_MSDC50_0_EN, "per_msdc50_0_en", "msdc50_0_sel", 0),
	GATE_PERI2(CLK_PERI_MSDC30_1_EN, "per_msdc30_1_en", "msdc30_1_sel", 1),
	GATE_PERI2(CLK_PERI_MSDC30_2_EN, "per_msdc30_2_en", "msdc30_2_sel", 2),
	GATE_PERI2(CLK_PERI_MSDC50_0_HCLK_EN, "per_msdc50_0_h", "msdc50_0_h_sel", 4),
	GATE_PERI2(CLK_PERI_MSDC50_2_HCLK_EN, "per_msdc50_2_h", "msdc50_2_h_sel", 5),
};

#define MT8695_PLL_FMAX		(3800UL * MHZ)

#define PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,	\
			_pcwbits, _pcwibits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,	\
			_tuner_en_bit, _pcw_reg, _pcw_shift, _div_table) {		\
		.id = _id,						\
		.name = _name,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.flags = _flags,					\
		.rst_bar_mask = BIT(_rst_bar_mask),			\
		.fmax = MT8695_PLL_FMAX,				\
		.pcwbits = _pcwbits,					\
		.pcwibits = _pcwibits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.tuner_reg = _tuner_reg,				\
		.tuner_en_reg = _tuner_en_reg,				\
		.tuner_en_bit = _tuner_en_bit,				\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
		.div_table = _div_table,				\
	}

#define PLL(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,	\
			_pcwbits, _pcwibits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,	\
			_tuner_en_bit, _pcw_reg, _pcw_shift)				\
		PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,		\
			_pcwbits, _pcwibits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,	\
			_tuner_en_bit, _pcw_reg, _pcw_shift, NULL)

static const struct mtk_pll_div_table armpll_div_table[] = {
	{ .div = 0, .freq = MT8695_PLL_FMAX },
	{ .div = 1, .freq = 1800500000 },
	{ .div = 2, .freq = 750000000 },
	{ .div = 3, .freq = 325000000 },
	{ .div = 4, .freq = 157625000 },
	{ } /* sentinel */
};

static const struct mtk_pll_div_table common_div_table[] = {
	{ .div = 0, .freq = MT8695_PLL_FMAX },
	{ .div = 1, .freq = 1601000000 },
	{ .div = 2, .freq = 949250000 },
	{ .div = 3, .freq = 250250000 },
	{ .div = 4, .freq = 125125000 },
	{ } /* sentinel */
};

static const struct mtk_pll_data plls[] = {
	PLL_B(CLK_APMIXED_ARMPLL, "armpll", 0x0110, 0x011C, 0x00000101, HAVE_RST_BAR, 25, 22, 8,
		0x0114, 24, 0, 0, 0, 0x0114, 0, armpll_div_table),
	PLL(CLK_APMIXED_MAINPLL, "mainpll", 0x0120, 0x012C, 0x00000101, HAVE_RST_BAR | PCW_CHG_SHIFT, 20, 32, 8,
		0x0120, 4, 0, 0, 0, 0x0124, 0),
	PLL(CLK_APMIXED_UNIV2PLL, "univ2pll", 0x0130, 0x013C, 0x1b000101, HAVE_RST_BAR | PCW_CHG_SHIFT, 20, 32, 8,
		0x0130, 4, 0, 0, 0, 0x0134, 0),
	PLL_B(CLK_APMIXED_MMPLL, "mmpll", 0x0140, 0x014C, 0x00000101, 0, 0, 22, 8,
		0x0144, 24, 0, 0, 0, 0x0144, 0, common_div_table),
	PLL_B(CLK_APMIXED_MSDCPLL, "msdcpll", 0x0150, 0x015C, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0150, 4, 0, 0, 0, 0x0154, 0, common_div_table),
	PLL_B(CLK_APMIXED_TVDPLL, "tvdpll", 0x0170, 0x017C, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0170, 4, 0, 0, 0, 0x0174, 0, common_div_table),
	PLL_B(CLK_APMIXED_ETHERPLL, "etherpll", 0x0180, 0x018C, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0180, 4, 0, 0, 0, 0x0184, 0, common_div_table),
	PLL_B(CLK_APMIXED_VDECPLL, "vdecpll", 0x0190, 0x019C, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0190, 4, 0, 0, 0, 0x0194, 0, common_div_table),
	PLL_B(CLK_APMIXED_OSDPLL, "osdpll", 0x0200, 0x020C, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0200, 4, 0, 0, 0, 0x0204, 0, common_div_table),
	PLL(CLK_APMIXED_APLL1, "apll1", 0x0210, 0x0220, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0210, 4, 0x0218, 0x0014, 0, 0x0214, 0),
	PLL(CLK_APMIXED_APLL2, "apll2", 0x0224, 0x0234, 0x00000101, PCW_CHG_SHIFT, 0, 32, 8,
		0x0224, 4, 0x022C, 0x0014, 1, 0x0228, 0),
};

static struct clk_onecell_data *mt8695_top_clk_data;
static struct clk_onecell_data *mt8695_mcu_clk_data;
static void __iomem *mt8695_top_base;

static void mtk_clk_enable_critical(void)
{
	if (!mt8695_top_clk_data || !mt8695_mcu_clk_data)
		return;

	clk_prepare_enable(mt8695_top_clk_data->clks[CLK_TOP_AXI_SEL]);
	clk_prepare_enable(mt8695_top_clk_data->clks[CLK_TOP_MEM_SEL]);
	clk_prepare_enable(mt8695_mcu_clk_data->clks[CLK_MCU_BUS_SEL]);
}

static int clk_mt8695_apmixed_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;
	void __iomem *base;
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		pr_notice("%s(): ioremap failed\n", __func__);
		return PTR_ERR(base);
	}

	clk_data = mtk_alloc_clk_data(CLK_APMIXED_NR_CLK);

	mtk_clk_register_plls(node, plls, ARRAY_SIZE(plls), clk_data);
	mtk_clk_register_composites(apmixed_muxes, ARRAY_SIZE(apmixed_muxes), base, &mt8695_clk_lock, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static void clk_mt8695_top_init_early(struct device_node *node)
{
	int r, i;

	mt8695_top_base = of_iomap(node, 0);
	if (!mt8695_top_base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return;
	}

	if (mt8695_top_clk_data == NULL) {
		mt8695_top_clk_data = mtk_alloc_clk_data(CLK_TOP_NR_CLK);

		for (i = 0; i < CLK_TOP_NR_CLK; i++)
			mt8695_top_clk_data->clks[i] = ERR_PTR(-EPROBE_DEFER);
	}

	mtk_clk_register_fixed_clks(top_early_fixed_clks, ARRAY_SIZE(top_early_fixed_clks),
			mt8695_top_clk_data);
	mtk_clk_register_composites(top_early_muxes, ARRAY_SIZE(top_early_muxes), mt8695_top_base,
			&mt8695_clk_lock, mt8695_top_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt8695_top_clk_data);
	if (r != 0)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}

CLK_OF_DECLARE(mt8695_topckgen, "mediatek,mt8695-topckgen", clk_mt8695_top_init_early);

static int clk_mt8695_top_probe(struct platform_device *pdev)
{
	int r, i;
	struct device_node *node = pdev->dev.of_node;

	if (mt8695_top_clk_data == NULL) {
		mt8695_top_clk_data = mtk_alloc_clk_data(CLK_TOP_NR_CLK);
	} else {
		for (i = 0; i < CLK_TOP_NR_CLK; i++) {
			if (mt8695_top_clk_data->clks[i] == ERR_PTR(-EPROBE_DEFER))
				mt8695_top_clk_data->clks[i] = ERR_PTR(-ENOENT);
		}
	}

	mtk_clk_register_fixed_clks(top_fixed_clks, ARRAY_SIZE(top_fixed_clks), mt8695_top_clk_data);
	mtk_clk_register_factors(top_divs, ARRAY_SIZE(top_divs), mt8695_top_clk_data);
	mtk_clk_register_composites(top_muxes, ARRAY_SIZE(top_muxes), mt8695_top_base,
			&mt8695_clk_lock, mt8695_top_clk_data);
	mtk_clk_register_dividers(top_adj_divs, ARRAY_SIZE(top_adj_divs), mt8695_top_base,
			&mt8695_clk_lock, mt8695_top_clk_data);
	mtk_clk_register_gates(node, top_clks, ARRAY_SIZE(top_clks), mt8695_top_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt8695_top_clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	mtk_clk_enable_critical();

	return r;
}

static struct clk_onecell_data *mt8695_infra_clk_data;

static void clk_mt8695_infra_init_early(struct device_node *node)
{
	int r, i;

	if (mt8695_infra_clk_data == NULL) {
		mt8695_infra_clk_data = mtk_alloc_clk_data(CLK_INFRA_NR_CLK);

		for (i = 0; i < CLK_INFRA_NR_CLK; i++)
			mt8695_infra_clk_data->clks[i] = ERR_PTR(-EPROBE_DEFER);
	}

	mtk_clk_register_gates(node, infra_early_clks, ARRAY_SIZE(infra_early_clks),
			mt8695_infra_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt8695_infra_clk_data);
	if (r != 0)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}

CLK_OF_DECLARE(mt8695_infracfg, "mediatek,mt8695-infracfg", clk_mt8695_infra_init_early);

static int clk_mt8695_infra_probe(struct platform_device *pdev)
{
	int r, i;
	struct device_node *node = pdev->dev.of_node;

	if (mt8695_infra_clk_data == NULL) {
		mt8695_infra_clk_data = mtk_alloc_clk_data(CLK_INFRA_NR_CLK);
	} else {
		for (i = 0; i < CLK_INFRA_NR_CLK; i++) {
			if (mt8695_infra_clk_data->clks[i] == ERR_PTR(-EPROBE_DEFER))
				mt8695_infra_clk_data->clks[i] = ERR_PTR(-ENOENT);
		}
	}

	mtk_clk_register_gates(node, infra_clks, ARRAY_SIZE(infra_clks), mt8695_infra_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt8695_infra_clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static int clk_mt8695_peri_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	int r;
	struct device_node *node = pdev->dev.of_node;
	void __iomem *base;
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		pr_notice("%s(): ioremap failed\n", __func__);
		return PTR_ERR(base);
	}

	clk_data = mtk_alloc_clk_data(CLK_PERI_NR_CLK);

	mtk_clk_register_composites(peri_muxes, ARRAY_SIZE(peri_muxes), base, &mt8695_clk_lock, clk_data);
	mtk_clk_register_gates(node, peri_clks, ARRAY_SIZE(peri_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	return r;
}

static int clk_mt8695_mcu_probe(struct platform_device *pdev)
{
	int r;
	struct device_node *node = pdev->dev.of_node;
	void __iomem *base;
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		pr_notice("%s(): ioremap failed\n", __func__);
		return PTR_ERR(base);
	}

	mt8695_mcu_clk_data = mtk_alloc_clk_data(CLK_MCU_NR_CLK);

	mtk_clk_register_composites(mcu_muxes, ARRAY_SIZE(mcu_muxes), base, &mt8695_clk_lock, mt8695_mcu_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt8695_mcu_clk_data);

	if (r)
		pr_notice("%s(): could not register clock provider: %d\n",
			__func__, r);

	mtk_clk_enable_critical();

	return r;
}

static const struct of_device_id of_match_clk_mt8695[] = {
	{
		.compatible = "mediatek,mt8695-apmixedsys",
		.data = clk_mt8695_apmixed_probe,
	}, {
		.compatible = "mediatek,mt8695-topckgen",
		.data = clk_mt8695_top_probe,
	}, {
		.compatible = "mediatek,mt8695-infracfg",
		.data = clk_mt8695_infra_probe,
	}, {
		.compatible = "mediatek,mt8695-pericfg",
		.data = clk_mt8695_peri_probe,
	}, {
		.compatible = "mediatek,mt8695-mcucfg",
		.data = clk_mt8695_mcu_probe,
	}, {
		/* sentinel */
	}
};

static int clk_mt8695_probe(struct platform_device *pdev)
{
	int (*clk_probe)(struct platform_device *);
	int r;

	clk_probe = of_device_get_match_data(&pdev->dev);
	if (!clk_probe)
		return -EINVAL;

	r = clk_probe(pdev);
	if (r)
		dev_notice(&pdev->dev,
			"could not register clock provider: %s: %d\n",
			pdev->name, r);

	return r;
}

static struct platform_driver clk_mt8695_drv = {
	.probe = clk_mt8695_probe,
	.driver = {
		.name = "clk-mt8695",
		.owner = THIS_MODULE,
		.of_match_table = of_match_clk_mt8695,
	},
};

static int __init clk_mt8695_init(void)
{
	return platform_driver_register(&clk_mt8695_drv);
}

arch_initcall(clk_mt8695_init);


