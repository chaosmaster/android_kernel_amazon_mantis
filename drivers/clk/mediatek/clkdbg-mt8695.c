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
#include <linux/io.h>

#include "clkdbg.h"

#define DUMP_INIT_STATE		0

/*
 * clkdbg dump_regs
 */

enum {
	topckgen,
	infracfg,
	pericfg,
	scpsys,
	apmixed,
	fhctl,
	mfgsys,
	mmsys,
	dispsys,
	vdsocsys,
	vdcoresys,
	vencsys,
	demuxsys,
};

#define REGBASE_V(_phys, _id_name) { .phys = _phys, .name = #_id_name }

/*
 * checkpatch.pl ERROR:COMPLEX_MACRO
 *
 * #define REGBASE(_phys, _id_name) [_id_name] = REGBASE_V(_phys, _id_name)
 */

static struct regbase rb[] = {
	[topckgen]  = REGBASE_V(0x10000000, topckgen),
	[infracfg]  = REGBASE_V(0x10001000, infracfg),
	[pericfg]   = REGBASE_V(0x10003000, pericfg),
	[scpsys]    = REGBASE_V(0x10006000, scpsys),
	[apmixed]   = REGBASE_V(0x10209000, apmixed),
	[fhctl]     = REGBASE_V(0x10209e00, fhctl),
	[mfgsys]    = REGBASE_V(0x13ffe000, mfgsys),
	[mmsys]     = REGBASE_V(0x14000000, mmsys),
	[dispsys]    = REGBASE_V(0x15000000, dispsys),
	[vdsocsys]   = REGBASE_V(0x1600f000, vdsocsys),
	[vdcoresys]    = REGBASE_V(0x1602f000, vdcoresys),
	[vencsys]   = REGBASE_V(0x18000000, vencsys),
	[demuxsys] = REGBASE_V(0x19000000, demuxsys),
};

#define REGNAME(_base, _ofs, _name)	\
	{ .base = &rb[_base], .ofs = _ofs, .name = #_name }

static struct regname rn[] = {
	REGNAME(topckgen, 0x040, CLK_CFG_0),
	REGNAME(topckgen, 0x050, CLK_CFG_1),
	REGNAME(topckgen, 0x060, CLK_CFG_2),
	REGNAME(topckgen, 0x070, CLK_CFG_3),
	REGNAME(topckgen, 0x080, CLK_CFG_4),
	REGNAME(topckgen, 0x090, CLK_CFG_5),
	REGNAME(topckgen, 0x0a0, CLK_CFG_6),
	REGNAME(topckgen, 0x0b0, CLK_CFG_7),
	REGNAME(topckgen, 0x0c0, CLK_CFG_8),
	REGNAME(topckgen, 0x0d0, CLK_CFG_9),
	REGNAME(topckgen, 0x500, CLK_CFG_10),
	REGNAME(topckgen, 0x510, CLK_CFG_11),
	REGNAME(topckgen, 0x520, CLK_CFG_12),
	REGNAME(topckgen, 0x530, CLK_CFG_13),
	REGNAME(topckgen, 0x540, CLK_CFG_14),
	REGNAME(scpsys, 0x258, SPM_VDE_PWR_CON),
	REGNAME(scpsys, 0x25c, SPM_DIS_M_PWR_CON),
	REGNAME(scpsys, 0x260, SPM_DIS_S_PWR_CON),
	REGNAME(scpsys, 0x264, SPM_DIS_C_PWR_CON),
	REGNAME(scpsys, 0x26c, SPM_MM_PWR_CON),
	REGNAME(scpsys, 0x270, SPM_DOLBY_PWR_CON),
	REGNAME(scpsys, 0x274, SPM_VEN_PWR_CON),
	REGNAME(scpsys, 0x290, SPM_MFG_ASYNC_PWR_CON),
	REGNAME(scpsys, 0x294, SPM_MFG_2D_PWR_CON),
	REGNAME(scpsys, 0x298, SPM_MFG_3D_PWR_CON),
	REGNAME(scpsys, 0x2b4, SPM_USB_PWR_CON),
	REGNAME(scpsys, 0x2bc, SPM_ETH_PWR_CON),
	REGNAME(scpsys, 0x60c, SPM_PWR_STATUS),
	REGNAME(scpsys, 0x610, SPM_PWR_STATUS_2ND),
	REGNAME(apmixed, 0x004, AP_PLL_CON1),
	REGNAME(apmixed, 0x008, AP_PLL_CON2),
	REGNAME(apmixed, 0x00C, AP_PLL_CON3),
	REGNAME(apmixed, 0x010, AP_PLL_CON4),
	REGNAME(apmixed, 0x0110, ARMPLL_CON0),
	REGNAME(apmixed, 0x0114, ARMPLL_CON1),
	REGNAME(apmixed, 0x0118, ARMPLL_CON2),
	REGNAME(apmixed, 0x011C, ARMPLL_PWR_CON0),
	REGNAME(apmixed, 0x0120, MAINPLL_CON0),
	REGNAME(apmixed, 0x0124, MAINPLL_CON1),
	REGNAME(apmixed, 0x0128, MAINPLL_CON2),
	REGNAME(apmixed, 0x012C, MAINPLL_PWR_CON0),
	REGNAME(apmixed, 0x0130, UNIVPLL_CON0),
	REGNAME(apmixed, 0x0134, UNIVPLL_CON1),
	REGNAME(apmixed, 0x0138, UNIVPLL_CON2),
	REGNAME(apmixed, 0x013C, UNIVPLL_PWR_CON0),
	REGNAME(apmixed, 0x0140, MMPLL_CON0),
	REGNAME(apmixed, 0x0144, MMPLL_CON1),
	REGNAME(apmixed, 0x0148, MMPLL_CON2),
	REGNAME(apmixed, 0x014C, MMPLL_PWR_CON0),
	REGNAME(apmixed, 0x0150, MSDCPLL_CON0),
	REGNAME(apmixed, 0x0154, MSDCPLL_CON1),
	REGNAME(apmixed, 0x0158, MSDCPLL_CON2),
	REGNAME(apmixed, 0x015C, MSDCPLL_PWR_CON0),
	REGNAME(apmixed, 0x0170, TVDPLL_CON0),
	REGNAME(apmixed, 0x0174, TVDPLL_CON1),
	REGNAME(apmixed, 0x0178, TVDPLL_CON2),
	REGNAME(apmixed, 0x017C, TVDPLL_PWR_CON0),
	REGNAME(apmixed, 0x0180, ETHERPLL_CON0),
	REGNAME(apmixed, 0x0184, ETHERPLL_CON1),
	REGNAME(apmixed, 0x0188, ETHERPLL_CON2),
	REGNAME(apmixed, 0x018C, ETHERPLL_PWR_CON0),
	REGNAME(apmixed, 0x0190, VDECPLL_CON0),
	REGNAME(apmixed, 0x0194, VDECPLL_CON1),
	REGNAME(apmixed, 0x0198, VDECPLL_CON2),
	REGNAME(apmixed, 0x019C, VDECPLL_PWR_CON0),
	REGNAME(apmixed, 0x0200, OSDPLL_CON0),
	REGNAME(apmixed, 0x0204, OSDPLL_CON1),
	REGNAME(apmixed, 0x0208, OSDPLL_CON2),
	REGNAME(apmixed, 0x020C, OSDPLL_PWR_CON0),
	REGNAME(apmixed, 0x0210, APLL1_CON0),
	REGNAME(apmixed, 0x0214, APLL1_CON1),
	REGNAME(apmixed, 0x0218, APLL1_CON2),
	REGNAME(apmixed, 0x021C, APLL1_CON3),
	REGNAME(apmixed, 0x0220, APLL1_PWR_CON0),
	REGNAME(apmixed, 0x0224, APLL2_CON0),
	REGNAME(apmixed, 0x0228, APLL2_CON1),
	REGNAME(apmixed, 0x022C, APLL2_CON2),
	REGNAME(apmixed, 0x0230, APLL2_CON3),
	REGNAME(apmixed, 0x0234, APLL2_PWR_CON0),
	REGNAME(topckgen, 0x120, CLK_AUDDIV_0),
	REGNAME(topckgen, 0x424, CLK_CG_EN_CFG),
	REGNAME(infracfg, 0x048, INFRA_PDN_STA),
	REGNAME(pericfg, 0x018, PERI_PDN0_STA),
	REGNAME(pericfg, 0x01c, PERI_PDN1_STA),
	REGNAME(pericfg, 0x42c, PERI_MSDC_CLK_EN),
	REGNAME(mfgsys, 0x000, MFG_CG_STA),
	REGNAME(mmsys, 0x100, MMSYS_CG0_STA),
	REGNAME(mmsys, 0x884, MMSYS_CG1_STA),
	REGNAME(dispsys, 0x00c, DISPSYS_CG_STA),
	REGNAME(vdsocsys, 0x0a0, VDEC_SOC_CKEN),
	REGNAME(vdsocsys, 0x0a8, VDEC_LAT_CKEN),
	REGNAME(vdsocsys, 0x0b0, VDEC_LARB7_CKGEN),
	REGNAME(vdcoresys, 0x000, VDEC_CKEN),
	REGNAME(vdcoresys, 0x008, VDEC_LARB1_CKGEN),
	REGNAME(vencsys, 0x000, VENC_CG_STA),
	REGNAME(demuxsys, 0x000, DEMUX_CG_STA),
	{}
};

static const struct regname *get_all_regnames(void)
{
	return rn;
}

static void __init init_regbase(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rb); i++)
		rb[i].virt = ioremap(rb[i].phys, PAGE_SIZE);
}

/*
 * clkdbg fmeter
 */

#include <linux/delay.h>

#ifndef GENMASK
#define GENMASK(h, l)	(((1U << ((h) - (l) + 1)) - 1) << (l))
#endif

#define ALT_BITS(o, h, l, v) \
	(((o) & ~GENMASK(h, l)) | (((v) << (l)) & GENMASK(h, l)))

#define clk_readl(addr)		readl(addr)
#define clk_writel(addr, val)	\
	do { writel(val, addr); wmb(); } while (0) /* sync write */
#define clk_writel_mask(addr, mask, val)	\
	clk_writel(addr, (clk_readl(addr) & ~(mask)) | (val))

#define ABS_DIFF(a, b)	((a) > (b) ? (a) - (b) : (b) - (a))

enum FMETER_TYPE {
	FT_NULL,
	ABIST,
	CKGEN
};

#define FMCLK(_t, _i, _n) { .type = _t, .id = _i, .name = _n }

static const struct fmeter_clk fclks[] = {
	FMCLK(ABIST,  4, "AD_MAIN_H546M_CK"),
	FMCLK(ABIST,  5, "AD_MAIN_H364M_CK"),
	FMCLK(ABIST,  6, "AD_MAIN_H218P4M_CK"),
	FMCLK(ABIST,  7, "AD_MAIN_H156M_CK"),
	FMCLK(ABIST,  8, "AD_UNIV_178P3M_CK"),
	FMCLK(ABIST,  9, "AD_UNIVPLL_UNIV_48M_CK"),
	FMCLK(ABIST, 10, "AD_UNIV_624M_CK"),
	FMCLK(ABIST, 11, "AD_UNIV_416M_CK"),
	FMCLK(ABIST, 12, "AD_UNIV_249P6M_CK"),
	FMCLK(ABIST, 13, "AD_APLL1_CK"),
	FMCLK(ABIST, 14, "AD_APLL2_CK"),
	FMCLK(ABIST, 15, "AD_LTEPLL_FS26M_CK"),
	FMCLK(ABIST, 16, "clkrtc_int_pre"),
	FMCLK(ABIST, 17, "AD_MMPLL_500M_CK"),
	FMCLK(ABIST, 19, "AD_VDECPLL_546M_CK"),
	FMCLK(ABIST, 20, "AD_TVDPLL_594M_CK"),
	FMCLK(ABIST, 22, "AD_MSDCPLL_400M_CK"),
	FMCLK(ABIST, 23, "AD_ETHERPLL_50M_CK"),
	FMCLK(ABIST, 24, "clkph_MCK_o"),
	FMCLK(ABIST, 25, "AD_USB_48M_CK"),
	FMCLK(ABIST, 30, "AD_TVDPLL_594M_CK"),
	FMCLK(ABIST, 34, "AD_ETHERPLL_125M_CK"),
	FMCLK(ABIST, 35, "AD_ARMPLL_D2_CK"),
	FMCLK(ABIST, 36, "AD_ARMPLL_D3_CK"),
	FMCLK(ABIST, 37, "AD_HDMITX20PLL_MONFBK_CK_CKSYS"),
	FMCLK(ABIST, 38, "AD_HDMITX20PLL_MONREF_CK_CKSYS"),
	FMCLK(ABIST, 39, "AD_HDMITX20PLL_MONPLL_CK_CKSYS"),
	FMCLK(ABIST, 40, "AD_HDMITX20_MONCLK_CKSYS"),
	FMCLK(ABIST, 41, "AD_USB20_CLK60M_P0"),
	FMCLK(ABIST, 42, "AD_USB20_CLK60M_P1"),
	FMCLK(ABIST, 43, "AD_UNIVPLL_UNIV_48M_CK"),
	FMCLK(ABIST, 44, "AD_UNIVPLL_SSUSB_48M_CK"),
	FMCLK(ABIST, 45, "occ_gated_armpll_occ_mon"),
	FMCLK(ABIST, 46, "armpll_occ_mon_cksys"),
	FMCLK(ABIST, 47, "DA_HDMITX20_REF_CK"),
	FMCLK(ABIST, 48, "DA_ARMCPU_MON_CK"),
	FMCLK(ABIST, 49, "AD_PLLGP_MON_FM_CK"),
	FMCLK(ABIST, 50, "AD_PLLGP_MON_FM_CK_CKSYS"),
	FMCLK(ABIST, 51, "trng_freq_debug_out0"),
	FMCLK(ABIST, 52, "trng_freq_debug_out1"),
	FMCLK(ABIST, 53, "mcusys_arm_clk_out_all"),
	FMCLK(ABIST, 54, "AD_UNIVPLL_192M_CK"),
	FMCLK(CKGEN,  1, "hf_faxi_ck"),
	FMCLK(CKGEN,  2, "hd_faxi_ck"),
	FMCLK(CKGEN,  5, "hf_fmm_ck"),
	FMCLK(CKGEN,  7, "hf_fvdec_ck"),
	FMCLK(CKGEN,  8, "hf_fvenc_ck"),
	FMCLK(CKGEN,  9, "hf_fmfg_ck"),
	FMCLK(CKGEN, 11, "f_fuart_ck"),
	FMCLK(CKGEN, 12, "hf_fspi_ck"),
	FMCLK(CKGEN, 13, "f_fusb20_ck"),
	FMCLK(CKGEN, 14, "f_fusb30_ck"),
	FMCLK(CKGEN, 15, "hf_fmsdc50_0_hclk_ck"),
	FMCLK(CKGEN, 16, "hf_fmsdc50_0_ck"),
	FMCLK(CKGEN, 17, "hf_fmsdc30_1_ck"),
	FMCLK(CKGEN, 18, "hf_fmsdc30_2_ck"),
	FMCLK(CKGEN, 20, "hf_faudio_ck"),
	FMCLK(CKGEN, 21, "hf_faud_intbus_ck"),
	FMCLK(CKGEN, 25, "hf_fnr_ck"),
	FMCLK(CKGEN, 27, "hf_fcci400_ck"),
	FMCLK(CKGEN, 28, "hf_faud_1_ck"),
	FMCLK(CKGEN, 29, "hf_faud_2_ck"),
	FMCLK(CKGEN, 30, "hf_fmem_mfg_in_as_ck"),
	FMCLK(CKGEN, 31, "hf_faxi_mfg_in_as_ck"),
	FMCLK(CKGEN, 32, "f_f26m_ck"),
	FMCLK(CKGEN, 39, "f_ffpc_ck"),
	FMCLK(CKGEN, 40, "f_fckbus_ck_scan"),
	FMCLK(CKGEN, 48, "hf_fhdcp_ck"),
	FMCLK(CKGEN, 49, "hf_fhdcp_24m_ck"),
	FMCLK(CKGEN, 50, "hf_fmsdc0p_aes_ck"),
	FMCLK(CKGEN, 51, "hf_fgcpu_ck"),
	FMCLK(CKGEN, 52, "hf_fmem_ck"),
	FMCLK(CKGEN, 53, "hf_fi2so1_mck"),
	FMCLK(CKGEN, 54, "hf_fnfi1x_ck"),
	FMCLK(CKGEN, 55, "hf_fether_125m_ck"),
	FMCLK(CKGEN, 56, "hf_fapll2_ck"),
	FMCLK(CKGEN, 57, "hf_fa2sys_hp_ck"),
	FMCLK(CKGEN, 58, "hf_fasm_l_ck"),
	FMCLK(CKGEN, 59, "hf_fspislv_ck"),
	FMCLK(CKGEN, 60, "hf_fasm_h_ck"),
	FMCLK(CKGEN, 61, "hf_fa1sys_hp_ck"),
	FMCLK(CKGEN, 65, "hf_fasm_m_ck"),
	FMCLK(CKGEN, 66, "hf_fapll_ck"),
	FMCLK(CKGEN, 67, "hf_fspinor_ck"),
	FMCLK(CKGEN, 69, "hf_fnfi2x_ck"),
	FMCLK(CKGEN, 70, "hf_fpwm_infra_ck"),
	FMCLK(CKGEN, 72, "hf_fether_50m_rmii_ck"),
	FMCLK(CKGEN, 73, "hf_fi2c_ck"),
	FMCLK(CKGEN, 78, "etherpll_50m_ck"),
	FMCLK(CKGEN, 79, "hf_fi2si2_mck"),
	FMCLK(CKGEN, 83, "hf_fi2si1_mck"),
	FMCLK(CKGEN, 84, "hf_fi2so2_mck"),
	FMCLK(CKGEN, 85, "tst_sel_1[25]"),
	FMCLK(CKGEN, 86, "tst_sel_1[26]"),
	FMCLK(CKGEN, 87, "tst_sel_1[27]"),
	FMCLK(CKGEN, 88, "tst_sel_1[28]"),
	FMCLK(CKGEN, 89, "tst_sel_1[29]"),
	FMCLK(CKGEN, 90, "tst_sel_1[30]"),
	FMCLK(CKGEN, 91, "tst_sel_1[31]"),
	FMCLK(CKGEN, 92, "tst_sel_1[0]"),
	FMCLK(CKGEN, 93, "tst_sel_1[1]"),
	FMCLK(CKGEN, 94, "tst_sel_1[2]"),
	FMCLK(CKGEN, 95, "tst_sel_1[3]"),
	{}
};

#define FHCTL_HP_EN		(rb[fhctl].virt + 0x000)
#define CLK_CFG_M0		(rb[topckgen].virt + 0x100)
#define CLK_CFG_M1		(rb[topckgen].virt + 0x104)
#define CLK_MISC_CFG_1	(rb[topckgen].virt + 0x214)
#define CLK_MISC_CFG_2	(rb[topckgen].virt + 0x218)
#define CLK26CALI_0		(rb[topckgen].virt + 0x220)
#define CLK26CALI_1		(rb[topckgen].virt + 0x224)
#define CLK26CALI_2		(rb[topckgen].virt + 0x228)
#define PLL_TEST_CON0	(rb[apmixed].virt + 0x040)

#define RG_FRMTR_WINDOW     1023

static void set_fmeter_divider_arm(u32 k1)
{
	u32 v = clk_readl(CLK_MISC_CFG_1);

	v = ALT_BITS(v, 15, 8, k1);
	clk_writel(CLK_MISC_CFG_1, v);
}

static void set_fmeter_divider(u32 k1)
{
	u32 v = clk_readl(CLK_MISC_CFG_1);

	v = ALT_BITS(v, 7, 0, k1);
	v = ALT_BITS(v, 31, 24, k1);
	clk_writel(CLK_MISC_CFG_1, v);
}

static u8 wait_fmeter_done(u32 tri_bit)
{
	static int max_wait_count;
	int wait_count = (max_wait_count > 0) ? (max_wait_count * 2 + 2) : 100;
	int i;

	/* wait fmeter */
	for (i = 0; i < wait_count && (clk_readl(CLK26CALI_0) & tri_bit); i++)
		udelay(20);

	if (!(clk_readl(CLK26CALI_0) & tri_bit)) {
		max_wait_count = max(max_wait_count, i);
		return 1;
	}

	return 0;
}
static u32 fmeter_freq(enum FMETER_TYPE type, int k1, int clk)
{
	void __iomem *clk_cfg_reg = (type == CKGEN) ? CLK_CFG_M1 : CLK_CFG_M0;
	void __iomem *cnt_reg = (type == CKGEN) ? CLK26CALI_2 : CLK26CALI_1;
	u32 cksw_mask = (type == CKGEN) ? GENMASK(22, 16) : GENMASK(14, 8);
	u32 cksw_val = (type == CKGEN) ? (clk << 16) : (clk << 8);
	u32 tri_bit = (type == CKGEN) ? BIT(4) : BIT(0);
	u32 clk_exc = (type == CKGEN) ? BIT(5) : BIT(2);
	u32 clk_misc_cfg_1, clk_misc_cfg_2, clk_cfg_val, cnt, freq = 0;

	/* setup fmeter */
	clk_setl(CLK26CALI_0, BIT(7));	/* enable fmeter_en */
	clk_clrl(CLK26CALI_0, clk_exc);	/* set clk_exc */
	clk_writel_mask(cnt_reg, GENMASK(25, 16), RG_FRMTR_WINDOW << 16);	/* load_cnt */

	clk_misc_cfg_1 = clk_readl(CLK_MISC_CFG_1);	/* backup CLK_MISC_CFG_1 value */
	clk_misc_cfg_2 = clk_readl(CLK_MISC_CFG_2);	/* backup CLK_MISC_CFG_2 value */
	clk_cfg_val = clk_readl(clk_cfg_reg);		/* backup clk_cfg_reg value */

	set_fmeter_divider(k1);			/* set divider (0 = /1) */
	set_fmeter_divider_arm(k1);
	clk_writel_mask(clk_cfg_reg, cksw_mask, cksw_val);	/* select cksw */

	clk_setl(CLK26CALI_0, tri_bit);	/* start fmeter */

	if (wait_fmeter_done(tri_bit)) {
		cnt = clk_readl(cnt_reg) & 0xFFFF;
		freq = (cnt * 26000) * (k1 + 1) / (RG_FRMTR_WINDOW + 1); /* (KHz) ; freq = counter * 26M / 1024 */
	}

	/* restore register settings */
	clk_writel(clk_cfg_reg, clk_cfg_val);
	clk_writel(CLK_MISC_CFG_2, clk_misc_cfg_2);
	clk_writel(CLK_MISC_CFG_1, clk_misc_cfg_1);

	clk_clrl(CLK26CALI_0, BIT(7));	/* disable fmeter_en */

	return freq;
}

static u32 measure_stable_fmeter_freq(enum FMETER_TYPE type, int k1, int clk)
{
	u32 last_freq = 0;
	u32 freq = fmeter_freq(type, k1, clk);
	u32 maxfreq = max(freq, last_freq);

	while (maxfreq > 0 && ABS_DIFF(freq, last_freq) * 100 / maxfreq > 10) {
		last_freq = freq;
		freq = fmeter_freq(type, k1, clk);
		maxfreq = max(freq, last_freq);
	}

	return freq;
}

static const struct fmeter_clk *get_all_fmeter_clks(void)
{
	return fclks;
}

struct bak {
	u32 fhctl_hp_en;
};

static void *prepare_fmeter(void)
{
	static struct bak regs;

	regs.fhctl_hp_en = clk_readl(FHCTL_HP_EN);

	clk_writel(FHCTL_HP_EN, 0x0);		/* disable PLL hopping */
	udelay(10);

	return &regs;
}

static void unprepare_fmeter(void *data)
{
	struct bak *regs = data;

	/* restore old setting */
	clk_writel(FHCTL_HP_EN, regs->fhctl_hp_en);
}

static u32 fmeter_freq_op(const struct fmeter_clk *fclk)
{
	if (fclk->type)
		return measure_stable_fmeter_freq(fclk->type, 0, fclk->id);

	return 0;
}

/*
 * clkdbg dump_state
 */

static const char * const *get_all_clk_names(void)
{
	static const char * const clks[] = {
		/* plls */
		"armpll",
		"mainpll",
		"univpll",
		"mmpll",
		"msdcpll",
		"tvdpll",
		"etherpll",
		"vdecpll",
		"osdpll",
		"apll1",
		"apll2",
		/* topckgen */
		"dmpll_ck",
		"clkrtc_int",
		"armca35pll_ck",
		"armca35pll_600m",
		"armca35pll_400m",
		"syspll_ck",
		"syspll_d2",
		"syspll1_d2",
		"syspll1_d4",
		"syspll1_d16",
		"syspll_d3",
		"syspll2_d2",
		"syspll2_d4",
		"syspll_d5",
		"syspll3_d2",
		"syspll3_d4",
		"syspll_d7",
		"syspll4_d2",
		"syspll4_d4",
		"univpll_ck",
		"univpll_d7",
		"univpll_d26",
		"univpll_d52",
		"univpll_d2",
		"univpll1_d2",
		"univpll1_d4",
		"univpll1_d8",
		"univpll_d3",
		"univpll2_d2",
		"univpll2_d4",
		"univpll2_d8",
		"univpll_d5",
		"univpll3_d2",
		"univpll3_d4",
		"univpll3_d8",
		"f_mp0_pll1_ck",
		"f_mp0_pll2_ck",
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
		"osdpll_ck",
		"osdpll_d2",
		"osdpll_d3",
		"osdpll_d4",
		"osdpll_d6",
		"osdpll_d12",
		"etherpll_125m",
		"etherpll_50m",
		"sys_26m",
		"mmpll_ck",
		"mmpll_d2",
		"vdecpll_ck",
		"tvdpll_ck",
		"tvdpll_d2",
		"tvdpll_d4",
		"tvdpll_d8",
		"msdcpll_ck",
		"msdcpll_d2",
		"msdcpll_d4",
		"clk26m_d2",
		"nfi1x",
		"axi_sel",
		"mem_sel",
		"sd_sel",
		"mm_sel",
		"vdec_lae_sel",
		"vdec_sel",
		"vdec_slow_sel",
		"venc_sel",
		"mfg_sel",
		"rsz_sel",
		"uart_sel",
		"spi_sel",
		"usb20_sel",
		"usb30_sel",
		"msdc50_0_h_sel",
		"msdc50_0_sel",
		"msdc30_1_sel",
		"msdc30_2_sel",
		"msdc50_2_h_sel",
		"msdc0p_aes_sel",
		"intdir_sel",
		"audio_sel",
		"aud_intbus_sel",
		"osd_sel",
		"vdo3_sel",
		"vdo4_sel",
		"hd_sel",
		"nr_sel",
		"cci400_sel",
		"aud_1_sel",
		"aud_2_sel",
		"mem_mfg_sel",
		"slow_mfg_sel",
		"axi_mfg_sel",
		"hdcp_sel",
		"hdcp_24m_sel",
		"clk32k",
		"spinor_sel",
		"apll_sel",
		"apll2_sel",
		"a1sys_hp_sel",
		"a2sys_hp_sel",
		"asm_l_sel",
		"asm_m_sel",
		"asm_h_sel",
		"i2so1_sel",
		"i2so2_sel",
		"tdmin_sel",
		"i2si1_sel",
		"i2si2_sel",
		"ether_125m_sel",
		"ether_50m_sel",
		"spislv_sel",
		"i2c_sel",
		"pwm_infra_sel",
		"gcpu_sel",
		"ecc_sel",
		"di_sel",
		"dmx_sel",
		"nfi2x_sel",
		"apll_div0",
		"apll_div1",
		"apll_div4",
		"apll_div5",
		"apll_div6",
		"apll_div_pdn0",
		"apll_div_pdn1",
		"apll_div_pdn4",
		"apll_div_pdn5",
		"apll_div_pdn6",
		"nfi2x_en",
		"nfi1x_en",
		/* mcucfg */
		"mcu_bus_sel",
		/* infracfg */
		"infra_dbgclk",
		"infra_gce",
		"infra_m4u",
		"infra_kp",
		"infra_cec",
		/* dispsys */
		"disp_vdo3",
		"disp_fmt3",
		"disp_mv_hdr2sdr",
		"disp_mv_bt2020",
		"disp_smi_larb5",
		"disp_smi_larb6",
		"disp_dc_larb8",
		"disp_vdo4",
		"disp_fmt4",
		"disp_sv_hdr2sdr",
		"disp_sv_bt2020",
		"disp_irt_dma",
		"disp_rsz0",
		"disp_vdo_di",
		"disp_fmt_di",
		"disp_nr",
		"disp_wr_channel",
		/* mfgcfg */
		"mfg_baxi",
		"mfg_bmem",
		"mfg_bg3d",
		"mfg_b26m",
		/* mmsys */
		"mm_smi_common",
		"mm_smi_larb0",
		"mm_d_larb",
		"mm_fake_eng",
		"mm_smi_larb4",
		"mm_smi_larb1",
		"mm_smi_larb5",
		"mm_smi_larb6",
		"mm_smi_larb7",
		"mm_vdec2img",
		"mm_vdout",
		"mm_fmt_tg",
		"mm_fmt_dgo",
		"mm_tve",
		"mm_crc",
		"mm_osd_tve",
		"mm_osd_fhd",
		"mm_osd_uhd",
		"mm_p2i",
		"mm_hdmitx",
		"mm_rgb2hdmi",
		"mm_scler",
		"mm_sdppf",
		"mm_vdoin",
		"mm_dolby1",
		"mm_dolby2",
		"mm_dolby3",
		"mm_osd_sdr2hdr",
		"mm_osd_premix",
		"mm_dolby_mix",
		"mm_vm",
		/* pericfg */
		"per_nfi",
		"per_therm",
		"per_pwm0",
		"per_pwm1",
		"per_pwm2",
		"per_pwm3",
		"per_pwm4",
		"per_pwm5",
		"per_pwm6",
		"per_pwm7",
		"per_pwm",
		"per_usb",
		"per_ap_dma",
		"per_msdc30_0",
		"per_msdc30_1",
		"per_msdc30_2",
		"per_uart0",
		"per_uart1",
		"per_uart2",
		"per_uart3",
		"per_i2c0",
		"per_i2c1",
		"per_i2c2",
		"per_i2c3",
		"per_i2c4",
		"per_auxadc",
		"per_spi0",
		"per_sflash",
		"per_spi2",
		"per_haxi_sflash",
		"per_gmac",
		"per_gmac_pclk",
		"per_ptp_therm",
		"per_msdc50_0_en",
		"per_msdc30_1_en",
		"per_msdc30_2_en",
		"per_msdc50_0_h",
		"per_msdc50_2_h",
		/* vdcoresys */
		"vdcore_vdec",
		"vdcore_larb1",
		/* vdsocsys */
		"vdsoc_soc",
		"vdsoc_lat",
		"vdsoc_larb7",
		/* vencsys */
		"venc_smi",
		"venc",
		/* demuxsys */
		"demux_dmx_smi",
		"demux",
		/* end */
		NULL
	};

	return clks;
}

/*
 * clkdbg pwr_status
 */

static const char * const *get_pwr_names(void)
{
	static const char * const pwr_names[] = {
		[0]  = "",
		[1]  = "",
		[2]  = "",
		[3]  = "",
		[4]  = "",
		[5]  = "",
		[6]  = "",
		[7]  = "VDEC",
		[8]  = "DIS_M",
		[9]  = "DIS_S",
		[10] = "DIS_C",
		[11] = "MM",
		[12] = "DOLBY",
		[13] = "VEN",
		[14] = "DPY",
		[15] = "DPY1",
		[16] = "",
		[17] = "",
		[18] = "MFG_ASYNC",
		[19] = "MFG_2D",
		[20] = "MFG_3D",
		[21] = "INFRA",
		[22] = "ETH",
		[23] = "USB",
		[24] = "",
		[25] = "",
		[26] = "",
		[27] = "",
		[28] = "",
		[29] = "",
		[30] = "",
		[31] = "",
	};

	return pwr_names;
}

/*
 * clkdbg dump_clks
 */

void setup_provider_clk(struct provider_clk *pvdck)
{
	static const struct {
		const char *pvdname;
		u32 pwr_mask;
	} pvd_pwr_mask[] = {
		{"mfgsys",  BIT(18) | BIT(19) | BIT(20)},
		{"mmsys",  BIT(11)},
		{"dispsys", BIT(8) | BIT(9) | BIT(10)},
		{"vdsocsys", BIT(7)},
		{"vdcoresys", BIT(7)},
		{"vencsys", BIT(13)},
		{"demuxsys", BIT(13)},
	};

	int i;
	const char *pvdname = pvdck->provider_name;

	if (!pvdname)
		return;

	for (i = 0; i < ARRAY_SIZE(pvd_pwr_mask); i++) {
		if (strcmp(pvdname, pvd_pwr_mask[i].pvdname) == 0) {
			pvdck->pwr_mask = pvd_pwr_mask[i].pwr_mask;
			return;
		}
	}
}

/*
 * init functions
 */

static struct clkdbg_ops clkdbg_mt8695_ops = {
	.get_all_fmeter_clks = get_all_fmeter_clks,
	.prepare_fmeter = prepare_fmeter,
	.unprepare_fmeter = unprepare_fmeter,
	.fmeter_freq = fmeter_freq_op,
	.get_all_regnames = get_all_regnames,
	.get_all_clk_names = get_all_clk_names,
	.get_pwr_names = get_pwr_names,
	.setup_provider_clk = setup_provider_clk,
};

static void __init init_custom_cmds(void)
{
	static const struct cmd_fn cmds[] = {
		{}
	};

	set_custom_cmds(cmds);
}

static int __init clkdbg_mt8695_init(void)
{
	if (!of_machine_is_compatible("mediatek,mt8695"))
		return -ENODEV;

	init_regbase();

	init_custom_cmds();
	set_clkdbg_ops(&clkdbg_mt8695_ops);

#if DUMP_INIT_STATE
	print_regs();
	print_fmeter_all();
#endif /* DUMP_INIT_STATE */

	return 0;
}
device_initcall(clkdbg_mt8695_init);
