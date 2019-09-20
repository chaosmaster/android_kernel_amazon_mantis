/*
 * Mediatek audio utility
 *
 * Copyright (c) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "mt8695-afe-regs.h"
#include "mt8695-afe-common.h"
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of.h>

#define APLL_48K_BASE	196608000
#define APLL_44K_BASE	180633600

static char *mt8695_clk_name[MTK_CLK_NUM] = {
	"apll1",
	"apll2",
	"sys_26m",
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
	"univpll2_d4",
	"univpll2_d2",
	"syspll_d5",
	"univpll_d3",
	"syspll_d2",
	"tvdpll_ck",
	"univpll_d2",
	"syspll1_d4",
	"syspll2_d4",
	"univpll3_d2",
	"univpll2_d8",
	"syspll3_d2",
	"syspll3_d4",
	"i2si1_sel",
	"apll_div5",
	"apll_div_pdn5",
	"i2si2_sel",
	"apll_div6",
	"apll_div_pdn6",
	"i2so1_sel",
	"apll_div0",
	"apll_div_pdn0",
	"i2so2_sel",
	"tdmin_sel",
	"apll_div1",
	"apll_div_pdn1",
	"apll_sel",
	"apll2_sel",
	"a1sys_hp_sel",
	"a2sys_hp_sel",
	"asm_l_sel",
	"asm_m_sel",
	"asm_h_sel",
	"intdir_sel",
	"aud_intbus_sel",
};

static int get_top_cg_mask(unsigned int cg_type)
{
	switch (cg_type) {
	case MTK_AFE_CG_AFE:
		return AUD_TCON0_PDN_AFE;
	case MTK_AFE_CG_LRCK_CNT:
		return AUD_TCON0_PDN_LRCK_CNT;
	case MTK_AFE_CG_HDMI_CK:
		return AUD_TCON0_PDN_HDMI_CK;
	case MTK_AFE_CG_SPDIF_CK:
		return AUD_TCON0_PDN_SPDF_CK;
	case MTK_AFE_CG_APLL:
		return AUD_TCON0_PDN_APLL;
	case MTK_AFE_CG_TML:
		return AUD_TCON0_PDN_TML;
	case MTK_AFE_CG_I2SIN1_CK:
		return AUD_TCON4_PDN_I2SIN1;
	case MTK_AFE_CG_I2SIN2_CK:
		return AUD_TCON4_PDN_I2SIN2;
	case MTK_AFE_CG_I2SIN3_CK:
		return AUD_TCON4_PDN_I2SIN3;
	case MTK_AFE_CG_I2SIN4_CK:
		return AUD_TCON4_PDN_I2SIN4;
	case MTK_AFE_CG_I2SO1_CK:
		return AUD_TCON4_PDN_I2SO1;
	case MTK_AFE_CG_I2SO2_CK:
		return AUD_TCON4_PDN_I2SO2;
	case MTK_AFE_CG_I2SO3_CK:
		return AUD_TCON4_PDN_I2SO3;
	case MTK_AFE_CG_I2SO4_CK:
		return AUD_TCON4_PDN_I2SO4;
	case MTK_AFE_CG_I2SO5_CK:
		return AUD_TCON4_PDN_I2SO5;
	case MTK_AFE_CG_I2SO6_CK:
		return AUD_TCON4_PDN_I2SO6;
	case MTK_AFE_CG_ASRCI1_CK:
		return AUD_TCON4_PDN_ASRCI1;
	case MTK_AFE_CG_ASRCI2_CK:
		return AUD_TCON4_PDN_ASRCI2;
	case MTK_AFE_CG_MULTI_IN_CK:
		return AUD_TCON4_PDN_MULTI_IN;
	case MTK_AFE_CG_INTDIR_CK:
		return AUD_TCON4_PDN_INTDIR;
	case MTK_AFE_CG_A1SYS_CK:
		return AUD_TCON4_PDN_A1SYS;
	case MTK_AFE_CG_A2SYS_CK:
		return AUD_TCON4_PDN_A2SYS;
	case MTK_AFE_CG_AFE_CONN:
		return AUD_TCON4_PDN_AFE_CONN;
	case MTK_AFE_CG_PCMIF:
		return AUD_TCON4_PDN_PCMIF;
	case MTK_AFE_CG_ASRCI3_CK:
		return AUD_TCON4_PDN_ASRCI3;
	case MTK_AFE_CG_ASRCI4_CK:
		return AUD_TCON4_PDN_ASRCI4;
	case MTK_AFE_CG_MEMIF_UL1:
		return AUD_TCON5_PDN_MEMIF_UL1;
	case MTK_AFE_CG_MEMIF_UL2:
		return AUD_TCON5_PDN_MEMIF_UL2;
	case MTK_AFE_CG_MEMIF_UL3:
		return AUD_TCON5_PDN_MEMIF_UL3;
	case MTK_AFE_CG_MEMIF_UL4:
		return AUD_TCON5_PDN_MEMIF_UL4;
	case MTK_AFE_CG_MEMIF_UL5:
		return AUD_TCON5_PDN_MEMIF_UL5;
	case MTK_AFE_CG_MEMIF_DL1:
		return AUD_TCON5_PDN_MEMIF_DL1;
	case MTK_AFE_CG_MEMIF_DL2:
		return AUD_TCON5_PDN_MEMIF_DL2;
	case MTK_AFE_CG_MEMIF_DL6:
		return AUD_TCON5_PDN_MEMIF_DL6;
	case MTK_AFE_CG_MEMIF_DLMCH:
		return AUD_TCON5_PDN_MEMIF_DLMCH;
	default:
		return -1;
	}
}

int mt8695_clock_init(struct mtk_afe *afe)
{
	int i;
#if MT8695_AUDIO_CCF
	for (i = 0; i < MTK_CLK_NUM; i++) {
		afe->clocks[i] = devm_clk_get(afe->dev, mt8695_clk_name[i]);
		if (IS_ERR(afe->clocks[i])) {
			dev_err(afe->dev, "%s devm_clk_get %s fail\n", __func__, mt8695_clk_name[i]);
			return PTR_ERR(afe->clocks[i]);
		}
	}
#endif
	return 0;
}

int mt8695_clock_power(struct mtk_afe *afe, enum clk_type clk, int en)
{
	int ret = 0;

#if MT8695_AUDIO_CCF
	if (en)
		ret = clk_prepare_enable(afe->clocks[clk]);
	else
		clk_disable_unprepare(afe->clocks[clk]);
#endif
	if (ret)
		dev_err(afe->dev, "%s %s power %s failed\n", __func__, mt8695_clk_name[clk], en ? "on" : "off");

	return ret;
}

int mt8695_set_fapll_ck_src(struct mtk_afe *afe, int parent)
{
	int ret = 0;

	if ((parent < MTK_CLK_TOP_SYS_26M) || (parent > MTK_CLK_TOP_APLL2_D16)) {
		dev_err(afe->dev, "%s parent(%d) is invalid\n", __func__, parent);
		return -EINVAL;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_APLL_SEL], afe->clocks[parent]);
	if (ret)
		dev_err(afe->dev, "[%s]clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_APLL_SEL], mt8695_clk_name[parent]);
#else
	regmap_update_bits(afe->topregmap, 0xD0, (0xF << 16), ((parent - MTK_CLK_TOP_SYS_26M) << 16));
#endif
	return ret;
}

int mt8695_set_fapll2_ck_src(struct mtk_afe *afe, int parent)
{
	int ret = 0;

	if ((parent < MTK_CLK_TOP_SYS_26M) || (parent > MTK_CLK_TOP_APLL2_D16)) {
		dev_err(afe->dev, "%s parent(%d) is invalid\n", __func__, parent);
		return -EINVAL;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_APLL2_SEL], afe->clocks[parent]);
	if (ret)
		dev_err(afe->dev, "[%s]clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_APLL2_SEL], mt8695_clk_name[parent]);
#else
	regmap_update_bits(afe->topregmap, 0xD0, (0xF << 24), ((parent - MTK_CLK_TOP_SYS_26M) << 24));
#endif
	return ret;
}

int mt8695_set_a1sys_clk_src(struct mtk_afe *afe, int parent)
{
	int ret = 0;

	if ((parent < MTK_CLK_TOP_SYS_26M) || (parent > MTK_CLK_TOP_APLL1_D8)) {
		dev_err(afe->dev, "%s parent(%d) is invalid\n", __func__, parent);
		return -EINVAL;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_A1SYS_HP_SEL], afe->clocks[parent]);
	if (ret)
		dev_err(afe->dev, "[%s]clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_A1SYS_HP_SEL], mt8695_clk_name[parent]);
#else
	regmap_update_bits(afe->topregmap, 0x500, 0x7, (parent - MTK_CLK_TOP_SYS_26M));
#endif
	return ret;
}

int mt8695_set_a2sys_clk_src(struct mtk_afe *afe, int parent)
{
	int ret = 0;

	if (((parent < MTK_CLK_TOP_APLL2) || (parent > MTK_CLK_TOP_APLL2_D8)) &&
		(parent != MTK_CLK_TOP_SYS_26M)) {
		dev_err(afe->dev, "%s parent(%d) is invalid\n", __func__, parent);
		return -EINVAL;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_A2SYS_HP_SEL], afe->clocks[parent]);
	if (ret)
		dev_err(afe->dev, "[%s]clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_A2SYS_HP_SEL], mt8695_clk_name[parent]);
#else
	if (parent != MTK_CLK_TOP_SYS_26M)
		regmap_update_bits(afe->topregmap, 0x500, 0x7 << 8, (parent - MTK_CLK_TOP_APLL2 + 1) << 8);
	else
		regmap_update_bits(afe->topregmap, 0x500, 0x7 << 8, 0);
#endif
	return ret;
}

int mt8695_set_intbus_clk_src(struct mtk_afe *afe, int parent)
{
	int ret = 0;

	if (((parent < MTK_CLK_TOP_SYSPLL1_D4) || (parent > MTK_CLK_TOP_SYSPLL3_D4))
		&& (parent != MTK_CLK_TOP_SYS_26M)) {
		dev_err(afe->dev, "%s parent(%d) is invalid\n", __func__, parent);
		return -EINVAL;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_AUD_INTBUS_SEL], afe->clocks[parent]);
	if (ret)
		dev_err(afe->dev, "[%s]clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_AUD_INTBUS_SEL], mt8695_clk_name[parent]);
#else
	if (parent != MTK_CLK_TOP_SYS_26M)
		regmap_update_bits(afe->topregmap, 0x90, 0x7 << 16, (parent - MTK_CLK_TOP_SYSPLL1_D4 + 1) << 8);
	else
		regmap_update_bits(afe->topregmap, 0x90, 0x7 << 16,  0);
#endif
	return ret;
}

int mt8695_afe_enable_top_cg(struct mtk_afe *afe, unsigned int cg_type)
{
	int mask = get_top_cg_mask(cg_type);
	unsigned long flags;

	if (mask < 0) {
		dev_dbg(afe->dev, "%s cg_type(%d) is invalid!\n", __func__, cg_type);
		return -1;
	}
	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->top_cg_ref_cnt[cg_type]++;
	if (afe->top_cg_ref_cnt[cg_type] == 1) {
		if (cg_type <= MTK_AFE_CG_TML)
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, mask, 0x0);
		else if (cg_type <= MTK_AFE_CG_ASRCI4_CK)
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON4, mask, 0x0);
		else
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON5, mask, 0x0);
	}

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}


int mt8695_afe_disable_top_cg(struct mtk_afe *afe, unsigned cg_type)
{
	int mask = get_top_cg_mask(cg_type);
	unsigned long flags;

	if (mask < 0) {
		dev_dbg(afe->dev, "%s cg_type(%d) is invalid!\n", __func__, cg_type);
		return -1;
	}
	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->top_cg_ref_cnt[cg_type]--;
	if (afe->top_cg_ref_cnt[cg_type] == 0) {
		if (cg_type <= MTK_AFE_CG_TML)
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, mask, mask);
		else if (cg_type <= MTK_AFE_CG_ASRCI4_CK)
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON4, mask, mask);
		else
			regmap_update_bits(afe->regmap, AUDIO_TOP_CON5, mask, mask);
	} else if (afe->top_cg_ref_cnt[cg_type] < 0)
		afe->top_cg_ref_cnt[cg_type] = 0;

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

int mt8695_afe_enable_i2so1_mclk(struct mtk_afe *afe)
{
#if	MT8695_AUDIO_CCF
	clk_prepare_enable(afe->clocks[MTK_CLK_TOP_I2SO1_SEL]);
	clk_prepare_enable(afe->clocks[MTK_CLK_TOP_APLL_DIV_PDN0]);
#else
	regmap_update_bits(afe->topregmap, 0x510, (0x1 << 15), 0);
	regmap_update_bits(afe->topregmap, 0x120, 1, 0);
#endif
	return 0;
}

int mt8695_afe_disable_i2so1_mclk(struct mtk_afe *afe)
{
#if	MT8695_AUDIO_CCF
	clk_disable_unprepare(afe->clocks[MTK_CLK_TOP_I2SO1_SEL]);
	clk_disable_unprepare(afe->clocks[MTK_CLK_TOP_APLL_DIV_PDN0]);
#else
	regmap_update_bits(afe->topregmap, 0x510, (0x1 << 15), (0x1 << 15));
	regmap_update_bits(afe->topregmap, 0x120, 1, 1);
#endif
	return 0;
}

int mt8695_afe_enable_i2so2_mclk(struct mtk_afe *afe)
{
		clk_prepare_enable(afe->clocks[MTK_CLK_TOP_I2SO2_SEL]);
		clk_prepare_enable(afe->clocks[MTK_CLK_TOP_APLL_DIV_PDN1]);
		return 0;
}

int mt8695_afe_disable_i2so2_mclk(struct mtk_afe *afe)
{
		clk_disable_unprepare(afe->clocks[MTK_CLK_TOP_I2SO2_SEL]);
		clk_disable_unprepare(afe->clocks[MTK_CLK_TOP_APLL_DIV_PDN1]);
		return 0;
}


int mt8695_afe_enable_afe_on(struct mtk_afe *afe)
{
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->afe_on_ref_cnt++;
	if (afe->afe_on_ref_cnt == 1)
		regmap_update_bits(afe->regmap, AFE_DAC_CON0, 0x1, 0x1);

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}


int mt8695_afe_disable_afe_on(struct mtk_afe *afe)
{
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->afe_on_ref_cnt--;
	if (afe->afe_on_ref_cnt == 0)
		regmap_update_bits(afe->regmap, AFE_DAC_CON0, 0x1, 0x0);
	else if (afe->afe_on_ref_cnt < 0)
		afe->afe_on_ref_cnt = 0;

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

int mt8695_asys_timing_on(struct mtk_afe *afe)
{
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);
	afe->asys_timing_ref_cnt++;
	if (afe->asys_timing_ref_cnt == 1)
		regmap_update_bits(afe->regmap, ASYS_TOP_CON, A1SYS_TIMING_ON_MASK, A1SYS_TIMING_ON);
	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

int mt8695_asys_timing_off(struct mtk_afe *afe)
{
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);
	afe->asys_timing_ref_cnt--;
	if (afe->asys_timing_ref_cnt == 0)
		regmap_update_bits(afe->regmap, ASYS_TOP_CON, A1SYS_TIMING_ON_MASK, A1SYS_TIMING_ON);
	else if (afe->asys_timing_ref_cnt < 0)
		afe->asys_timing_ref_cnt = 0;
	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

int mt8695_i2so1_set_mclk(struct mtk_afe *afe, unsigned int rate)
{
	unsigned int apll;
	int ret = 0;
#if !MT8695_AUDIO_CCF
	unsigned int div;
#endif

	if ((rate % 8000) == 0) {
#if MT8695_AUDIO_CCF
		apll = MTK_CLK_TOP_APLL1;
#else
		apll = APLL_48K_BASE;
		regmap_update_bits(afe->topregmap, 0x510, (0x3 << 8), (0x1 << 8));
#endif
	} else if ((rate % 11025) == 0) {
#if MT8695_AUDIO_CCF
		apll = MTK_CLK_TOP_APLL2;
#else
		apll = APLL_44K_BASE;
		regmap_update_bits(afe->topregmap, 0x510, (0x3 << 8), (0x2 << 8));
#endif
	} else {
		dev_err(afe->dev, "%s not support rate(%d)\n", __func__, rate);
		return -1;
	}
#if MT8695_AUDIO_CCF
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_I2SO1_SEL], afe->clocks[apll]);
	if (ret) {
		dev_err(afe->dev, "%s clock(%s) set parent(%s) failed!\n",
				__func__, mt8695_clk_name[MTK_CLK_TOP_I2SO1_SEL], mt8695_clk_name[apll]);
	}
	clk_set_rate(afe->clocks[MTK_CLK_TOP_APLL_DIV0], 256 * rate);
#else
	div = apll / (256 * rate) - 1;
	regmap_update_bits(afe->topregmap, 0x124, (0xFF), div);
#endif
	dev_err(afe->dev, "%s rate(%d)\n", __func__, rate);
	return 0;
}

int mt8695_i2so2_set_mclk(struct mtk_afe *afe, unsigned int rate)
{
	unsigned int apll;
	int ret = 0;

	if ((rate % 8000) == 0) {
		apll = MTK_CLK_TOP_APLL1;
	} else if ((rate % 11025) == 0) {
		apll = MTK_CLK_TOP_APLL2;
	} else {
		dev_err(afe->dev, "%s not support rate(%d)\n", __func__, rate);
		return -1;
	}
	ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_I2SO2_SEL], afe->clocks[apll]);
	if (ret) {
		dev_err(afe->dev, "%s clock(%s) set parent(%s) failed!\n",
				__func__, mt8695_clk_name[MTK_CLK_TOP_I2SO2_SEL], mt8695_clk_name[apll]);
	}
	clk_set_rate(afe->clocks[MTK_CLK_TOP_APLL_DIV1], 256 * rate);
	dev_err(afe->dev, "%s rate(%d)\n", __func__, rate);
	return 0;
}

int mt8695_hdmi_set_clock(struct mtk_afe *afe, unsigned int rate)
{
	unsigned int div, apll;
#if 0
	if ((rate % 8000) == 0)
		apll = 24576000;
	else if ((rate % 11025) == 0)
		apll = 22579200;
	else {
		dev_info(afe->dev, "The input rate(%d) is invalid!\n", rate);
		return -1;
	}
#else
	if ((rate % 8000) == 0) {
		apll = APLL_48K_BASE;
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, AUD_IEC_8CHI2S_CK_SEL_MASK, AUD_IEC_8CHI2S_CK_SEL_APLL);
	} else if ((rate % 11025) == 0) {
		apll = APLL_44K_BASE;
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, AUD_IEC_8CHI2S_CK_SEL_MASK, AUD_IEC_8CHI2S_CK_SEL_APL2);
	} else {
		dev_info(afe->dev, "The input rate(%d) is invalid!\n", rate);
		return -1;
	}
#endif
	/* bck = 64fs */
	div = ((apll / rate) >> 7) - 1;
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON3, HDMI_BCK_DIV_MASK, (div << HDMI_BCK_DIV_POS));

	return 0;
}

int mt8695_tdmin_set_clock(struct mtk_afe *afe, unsigned int rate, unsigned int channels, unsigned int bit_width)
{
	unsigned int apll, parent, mclk, bck, bck_div;
	int ret = 0;

	mclk = rate * 256;
	bck = ((channels == 6) ? 8 : channels) * rate * bit_width;
	if ((rate % 8000) == 0) {
		apll = APLL_48K_BASE;
		switch (apll / mclk) {
		case 4:
			parent = MTK_CLK_TOP_APLL1_D4;
			break;
		case 8:
			parent = MTK_CLK_TOP_APLL1_D8;
			break;
		case 16:
			parent = MTK_CLK_TOP_APLL1_D16;
			break;
		default:
			parent = MTK_CLK_TOP_I2SO2_SEL;
			mt8695_i2so2_set_mclk(afe, rate);
			break;
		}
		/*make tdm in mck select apll1*/
		regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_MCK_SEL_MASK, TDMIN_MCK_SEL_APLL1);
		ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_TDMIN_SEL], afe->clocks[parent]);
		if (ret) {
			dev_err(afe->dev, "%s clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_TDMIN_SEL], mt8695_clk_name[parent]);
		}
	} else if ((rate % 11025) == 0) {
		apll = APLL_44K_BASE;
		switch (apll / mclk) {
		case 4:
			parent = MTK_CLK_TOP_APLL2_D4;
			break;
		case 8:
			parent = MTK_CLK_TOP_APLL2_D8;
			break;
		case 16:
			parent = MTK_CLK_TOP_APLL2_D16;
			break;
		default:
			parent = MTK_CLK_TOP_I2SO2_SEL;
			mt8695_i2so2_set_mclk(afe, rate);
			break;
		}
		/*make tdm in mck select apll2*/
		regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_MCK_SEL_MASK, TDMIN_MCK_SEL_APLL2);
		ret = clk_set_parent(afe->clocks[MTK_CLK_TOP_TDMIN_SEL], afe->clocks[parent]);
		if (ret) {
			dev_err(afe->dev, "%s clock(%s) set parent(%s) failed!\n",
					__func__, mt8695_clk_name[MTK_CLK_TOP_TDMIN_SEL], mt8695_clk_name[parent]);
		}
	} else {
		dev_info(afe->dev, "The input rate(%d) is invalid!\n", rate);
		return -1;
	}
	bck_div = apll / bck;
	dev_info(afe->dev, "%s clock(%d) bck(%d) bck_div(%d)\n", __func__, apll, bck, bck_div);
	regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_CLK_DIV_B_MASK,
			(bck_div - 1) << TDMIN_CLK_DIV_B_POS);
	regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_CLK_DIV_M_MASK, 0);
	return 0;
}

int mt8695_iec_set_clock(struct mtk_afe *afe, unsigned int rate)
{
	unsigned int div, apll;
#if 0
	if ((rate % 8000) == 0)
		apll = (24576000);
	else if ((rate % 11025) == 0)
		apll = (22579200);
	else {
		dev_info(afe->dev, "The input rate(%d) is invalid!\n", rate);
		return -1;
	}
#else
	if ((rate % 8000) == 0) {
		apll = APLL_48K_BASE;
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, AUD_IEC_8CHI2S_CK_SEL_MASK, AUD_IEC_8CHI2S_CK_SEL_APLL);
	} else if ((rate % 11025) == 0) {
		apll = APLL_44K_BASE;
		regmap_update_bits(afe->regmap, AUDIO_TOP_CON0, AUD_IEC_8CHI2S_CK_SEL_MASK, AUD_IEC_8CHI2S_CK_SEL_APL2);
	} else {
		dev_info(afe->dev, "The input rate(%d) is invalid!\n", rate);
		return -1;
	}
#endif
	div = apll / (128 * rate) - 1;
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON2, SPDIF_CK_DIV_MASK, div);

	return 0;
}

int mt8695_iec_set_config(struct mtk_afe *afe)
{
	unsigned int reg_val = 0;

	regmap_update_bits(afe->regmap, AFE_MEMIF_PBUF_SIZE, AFE_MEMIF_IEC_PBUF_SIZE_MASK,
						AFE_MEMIF_IEC_PBUF_SIZE_128BYTES);

	regmap_update_bits(afe->regmap, AFE_IEC_NSNUM,
						(IEC_NEXT_SAM_NUM_MASK | IEC_INT_SAM_NUM_MASK),
						afe->iec.nsnum);
	regmap_update_bits(afe->regmap, AFE_IEC_BURST_LEN, IEC_BURST_LEN_MASK, (afe->iec.period_bytes << 3));
	regmap_update_bits(afe->regmap, AFE_IEC_BURST_INFO, IEC_BURST_INFO_MASK, 0);
	regmap_write(afe->regmap, AFE_IEC_CHL_STAT0, afe->iec.ch_status.chl_stat0);
	regmap_write(afe->regmap, AFE_IEC_CHL_STAT1, afe->iec.ch_status.chl_stat1);
	regmap_write(afe->regmap, AFE_IEC_CHR_STAT0, afe->iec.ch_status.chr_stat0);
	regmap_write(afe->regmap, AFE_IEC_CHR_STAT1, afe->iec.ch_status.chr_stat1);

	reg_val = (IEC_RAW_SEL_RAW | IEC_PCM_SEL_PCM | IEC_VALID_DATA | IEC_SW_NORST | IEC_FORCE_UPDATE);
	if (afe->iec.bit_width == 24)
		reg_val |= IEC_RAW_24BIT_MODE_ON;

	reg_val |= (afe->iec.force_update_size << 24);
	regmap_write(afe->regmap, AFE_IEC_CFG, reg_val);
	regmap_write(afe->regmap, AFE_IEC_NSADR, afe->iec.buf.buf_nsadr);

	return 0;
}

enum afe_sampling_rate rate_convert_enum(unsigned int rate)
{
		switch (rate) {
		case 8000:
				return FS_8000HZ;

		case 16000:
				return FS_16000HZ;

		case 32000:
				return FS_32000HZ;

		case 44100:
				return FS_44100HZ;

		case 48000:
				return FS_48000HZ;

		case 88200:
				return FS_88200HZ;

		case 96000:
				return FS_96000HZ;

		case 176400:
				return FS_176400HZ;

		case 192000:
				return FS_192000HZ;

		default:
				return 0;
	};
}

int mt8695_i2so_set_config(struct mtk_afe *afe, struct mt8695_i2s_out_config *config)
{
	unsigned int reg_val = 0, lr_invert;

	if (config->slave) {
		lr_invert = I2S_LRCK_NO_INV;
		reg_val |= (I2S_SLAVE_MODE);
	} else {
		lr_invert = I2S_LRCK_INV;
		reg_val |=	(I2S_MASTER_MODE);
	}

	if (config->dsd_mode)
		reg_val |= I2S_DSD_MODE;
	else {
		reg_val |= I2S_NORMAL_MODE;
		regmap_update_bits(afe->regmap, ASYS_TOP_CON, I2S1_DSD_USE_MASK, I2S1_DSD_USE_NORMAL);
	}

	if (config->one_heart_mode)
		reg_val |= I2S_OUT_MCH_MODE;
	else
		reg_val |= I2S_OUT_2CH_MODE;

	if (config->couple_mode)
		reg_val |= I2S_IN_OUT_SHARE_CLOCK;
	else
		reg_val |= I2S_IN_OUT_SELF_CLOCK;

	switch (config->fmt) {
	case FMT_32CYCLE_16BIT_I2S:
		reg_val |= (I2S_WLEN_16BIT | I2S_FMT_I2S);
		break;

	case FMT_32CYCLE_16BIT_LJ:
		reg_val |= (I2S_WLEN_16BIT | I2S_FMT_EIAJ | I2S_FMT_ADJUST_LJ | lr_invert);
		break;

	case FMT_64CYCLE_16BIT_I2S:
		reg_val |= (I2S_WLEN_32BIT | I2S_FMT_I2S | I2S_VALID_16BIT);
		break;

	case FMT_64CYCLE_16BIT_LJ:
		reg_val |= (I2S_WLEN_32BIT | I2S_FMT_EIAJ | I2S_FMT_ADJUST_LJ | I2S_VALID_16BIT | lr_invert);
		break;

	case FMT_64CYCLE_16BIT_RJ:
		reg_val |= (I2S_WLEN_32BIT | I2S_FMT_EIAJ | I2S_FMT_ADJUST_RJ | I2S_VALID_16BIT | lr_invert);
		break;

	case FMT_64CYCLE_32BIT_I2S:
		reg_val |= (I2S_WLEN_32BIT | I2S_FMT_I2S | I2S_VALID_24BIT);
		break;

	case FMT_64CYCLE_32BIT_LJ:
		reg_val |= (I2S_WLEN_32BIT | I2S_FMT_EIAJ | I2S_FMT_ADJUST_LJ | I2S_VALID_24BIT | lr_invert);
		break;
	default:
		dev_info(afe->dev, "%s fmt(%d) not supported now!\n", __func__, config->fmt);
		reg_val |= (I2S_WLEN_16BIT | I2S_FMT_I2S);
		break;
	}
	reg_val |= (config->mode << 8);

	reg_val |= (I2S_SW_RESET_DIS | I2S_LR_SWAP_DIS);

	regmap_write(afe->regmap, ASYS_I2SO1_CON, reg_val);

	/* Set Asys timing */
	regmap_update_bits(afe->regmap, ASMO_TIMING_CON1, ASMO1_MODE_MASK, config->mode);

	return 0;
}

