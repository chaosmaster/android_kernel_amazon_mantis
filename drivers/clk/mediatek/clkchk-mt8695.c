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
#include <linux/syscore_ops.h>
#include <linux/version.h>

#define WARN_ON_CHECK_PLL_FAIL		0
#define CLKDBG_CCF_API_4_4	1

#define TAG	"[clkchk] "

#define clk_warn(fmt, args...)	pr_warn(TAG fmt, ##args)

#if !CLKDBG_CCF_API_4_4

/* backward compatible */

static const char *clk_hw_get_name(const struct clk_hw *hw)
{
	return __clk_get_name(hw->clk);
}

static bool clk_hw_is_prepared(const struct clk_hw *hw)
{
	return __clk_is_prepared(hw->clk);
}

static bool clk_hw_is_enabled(const struct clk_hw *hw)
{
	return __clk_is_enabled(hw->clk);
}

#endif /* !CLKDBG_CCF_API_4_4 */

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

static const char *ccf_state(struct clk_hw *hw)
{
	if (__clk_get_enable_count(hw->clk))
		return "enabled";

	if (clk_hw_is_prepared(hw))
		return "prepared";

	return "disabled";
}

static void print_enabled_clks(void)
{
	const char * const *cn = get_all_clk_names();

	clk_warn("enabled clks:\n");

	for (; *cn; cn++) {
		struct clk *c = __clk_lookup(*cn);
		struct clk_hw *c_hw = __clk_get_hw(c);
		struct clk_hw *p_hw;

		if (IS_ERR_OR_NULL(c) || !c_hw)
			continue;

		p_hw = clk_hw_get_parent(c_hw);

		if (!p_hw)
			continue;

		if (!clk_hw_is_prepared(c_hw) && !__clk_get_enable_count(c))
			continue;

		clk_warn("[%-17s: %8s, %3d, %3d, %10ld, %17s]\n",
			clk_hw_get_name(c_hw),
			ccf_state(c_hw),
			clk_hw_is_prepared(c_hw),
			__clk_get_enable_count(c),
			clk_hw_get_rate(c_hw),
			p_hw ? clk_hw_get_name(p_hw) : "- ");
	}
}

static void check_pll_off(void)
{
	static const char * const off_pll_names[] = {
		"univpll",
		"mmpll",
		"msdcpll",
		"tvdpll",
		"etherpll",
		"vdecpll",
		"osdpll",
		"apll1",
		"apll2",
		NULL
	};

	static struct clk *off_plls[ARRAY_SIZE(off_pll_names)];

	struct clk **c;
	int invalid = 0;
	char buf[128] = {0};
	int n = 0;

	if (!off_plls[0]) {
		const char * const *pn;

		for (pn = off_pll_names, c = off_plls; *pn; pn++, c++)
			*c = __clk_lookup(*pn);
	}

	for (c = off_plls; *c; c++) {
		struct clk_hw *c_hw = __clk_get_hw(*c);

		if (!c_hw)
			continue;

		if (!clk_hw_is_prepared(c_hw) && !clk_hw_is_enabled(c_hw))
			continue;

		n += snprintf(buf + n, sizeof(buf) - n, "%s ",
				clk_hw_get_name(c_hw));

		invalid++;
	}

	if (invalid) {
		clk_warn("unexpected unclosed PLL: %s\n", buf);
		print_enabled_clks();

#if WARN_ON_CHECK_PLL_FAIL
		WARN_ON(1);
#endif
	}
}

static int clkchk_syscore_suspend(void)
{
	check_pll_off();

	return 0;
}

static void clkchk_syscore_resume(void)
{
}

static struct syscore_ops clkchk_syscore_ops = {
	.suspend = clkchk_syscore_suspend,
	.resume = clkchk_syscore_resume,
};

static int __init clkchk_init(void)
{
	if (!of_machine_is_compatible("mediatek,mt8695"))
		return -ENODEV;

	register_syscore_ops(&clkchk_syscore_ops);

	return 0;
}
subsys_initcall(clkchk_init);
