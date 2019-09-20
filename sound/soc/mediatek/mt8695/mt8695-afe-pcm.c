/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/pm_runtime.h>
#include <linux/types.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <linux/mfd/syscon.h>
#include <asm/div64.h>
#include "mt8695-afe-regs.h"
#include "mt8695-afe-util.h"
#include "mt8695_aud_debug.h"

#define CREATE_TRACE_POINTS
#include <trace/events/amz_atrace.h>

#define MT8695_FPGA_EARLY_PORTING 0
#define BYTES_OF_INT64 (8)
int hdmi_dac_state = 0;
int64_t hdmi_dac_timestamp = 0;

static const struct regmap_config mt8695_afe_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = AFE_SECURE_MASK_END_ADR_MSB,
	.cache_type = REGCACHE_NONE,
};

static const struct snd_pcm_hardware mt8695_afe_hardware = {
	.info = (SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP_VALID),
	.buffer_bytes_max = 1024 * 1024,
	.period_bytes_min = 256,
	.period_bytes_max = 512 * 1024,
	.periods_min = 2,
	.periods_max = 256,
	.fifo_size = 0,
};

static const struct mt8695_afe_memif_data memif_data[MTK_AFE_MEMIF_NUM] = {
	{
		.name = "HDMI",
		.id = MTK_AFE_MEMIF_HDMI,
		.reg_ofs_base = AFE_HDMI_OUT_BASE,
		.reg_ofs_end = AFE_HDMI_OUT_END,
		.reg_ofs_cur = AFE_HDMI_OUT_CUR,
		.fs_shift = -1,
		.hd_shift = -1,
		.mono_shift = -1,
		.lsb_mode_shift = -1,
		.dsd_width_shift = -1,
		.enable_shift = -1,
		.irq_reg_cnt = AFE_IRQ_CNT5,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_AFE_IRQ_5,
		.irq_fs_reg = -1,
		.irq_fs_shift = -1,
		.asys_irq_fs_shift = -1,
		.irq_clr_shift = 4,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = AFE_MEMIF_PBUF_SIZE,
		.pbuf_shift = 18,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 0,
		.buffer_align_bytes = 64,
	},
	{
		.name = "SPDIF",
		.id = MTK_AFE_MEMIF_SPDIF,
		.reg_ofs_base = AFE_SPDIF_BASE,
		.reg_ofs_end = AFE_SPDIF_END,
		.reg_ofs_cur = AFE_SPDIF_CUR,
		.fs_shift = -1,
		.hd_shift = -1,
		.mono_shift = -1,
		.lsb_mode_shift = -1,
		.dsd_width_shift = -1,
		.enable_shift = -1,
		.irq_reg_cnt = -1,
		.irq_cnt_shift = -1,
		.irq_mode = MTK_AFE_IRQ_6,
		.irq_fs_reg = -1,
		.irq_fs_shift = -1,
		.asys_irq_fs_shift = -1,
		.irq_clr_shift = 5,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = AFE_MEMIF_PBUF_SIZE,
		.pbuf_shift = 20,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 0,
		.buffer_align_bytes = 64,
	},
	{
		.name = "DL2",
		.id = MTK_AFE_MEMIF_DL2,
		.reg_ofs_base = AFE_DL2_BASE,
		.reg_ofs_end = AFE_DL2_END,
		.reg_ofs_cur = AFE_DL2_CUR,
		.fs_shift = 5,
		.hd_shift = 2,
		.mono_shift = 17,
		.lsb_mode_shift = 25,
		.dsd_width_shift = 2,
		.enable_shift = 2,
		.irq_reg_cnt = ASYS_IRQ2_CON,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_ASYS_IRQ_2,
		.irq_fs_reg = ASYS_IRQ2_CON,
		.irq_fs_shift = -1,
		.asys_irq_fs_shift = 24,
		.irq_clr_shift = 1,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = AFE_MEMIF_PBUF_SIZE,
		.pbuf_shift = 2,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 0,
		.buffer_align_bytes = 64,
	},
	{
		.name = "DL1",
		.id = MTK_AFE_MEMIF_I2S,
		.reg_ofs_base = AFE_DL1_BASE,
		.reg_ofs_end = AFE_DL1_END,
		.reg_ofs_cur = AFE_DL1_CUR,
		.fs_shift = 0,
		.hd_shift = 0,
		.mono_shift = 16,
		.lsb_mode_shift = 24,
		.dsd_width_shift = 0,
		.enable_shift = 1,
		.irq_reg_cnt = AFE_IRQ_CNT1,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_AFE_IRQ_1,
		.irq_fs_reg = AFE_IRQ_CON,
		.irq_fs_shift = 4,
		.asys_irq_fs_shift = -1,
		.irq_clr_shift = 0,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = AFE_MEMIF_PBUF_SIZE,
		.pbuf_shift = 0,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 0,
		.buffer_align_bytes = 64,
	},
	{
		.name = "TDM_IN",
		.id = MTK_AFE_MEMIF_TDM_IN,
		.reg_ofs_base = AFE_TDM_IN_BASE,
		.reg_ofs_end = AFE_TDM_IN_END,
		.reg_ofs_cur = AFE_TDM_IN_CUR,
		.fs_shift = -1,
		.hd_shift = 18,
		.mono_shift = -1,
		.lsb_mode_shift = -1,
		.dsd_width_shift = -1,
		.enable_shift = 25,
		.irq_reg_cnt = ASYS_IRQ1_CON,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_ASYS_IRQ_1,
		.irq_fs_reg = ASYS_IRQ1_CON,
		.irq_fs_shift = -1,
		.asys_irq_fs_shift = 24,
		.irq_clr_shift = 0,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = -1,
		.pbuf_shift = -1,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 1,
		.buffer_align_bytes = 64,
	},
	{
		.name = "DMIC",
		.id = MTK_AFE_MEMIF_DMIC,
		.reg_ofs_base = AFE_UL3_BASE,
		.reg_ofs_end = AFE_UL3_END,
		.reg_ofs_cur = AFE_UL3_CUR,
		.fs_shift = 10,
		.hd_shift = 4,
		.mono_shift = -1,
		.lsb_mode_shift = -1,
		.dsd_width_shift = -1,
		.enable_shift = 12,
		.irq_reg_cnt = AFE_IRQ_CNT2,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_AFE_IRQ_2,
		.irq_fs_reg = AFE_IRQ_CON,
		.irq_fs_shift = 8,
		.asys_irq_fs_shift = -1,
		.irq_clr_shift = 1,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = -1,
		.pbuf_shift = -1,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 1,
		.buffer_align_bytes = 64,
	},
	{
		.name = "UL5",
		.id = MTK_AFE_MEMIF_UL5,
		.reg_ofs_base = AFE_UL5_BASE,
		.reg_ofs_end = AFE_UL5_END,
		.reg_ofs_cur = AFE_UL5_CUR,
		.fs_shift = 20,
		.hd_shift = 8,
		.mono_shift = 8,
		.lsb_mode_shift = 28,
		.dsd_width_shift = -1,
		.enable_shift = 14,
		.irq_reg_cnt = ASYS_IRQ3_CON,
		.irq_cnt_shift = 0,
		.irq_mode = MTK_ASYS_IRQ_3,
		.irq_fs_reg = ASYS_IRQ3_CON,
		.irq_fs_shift = -1,
		.asys_irq_fs_shift = 24,
		.irq_clr_shift = 2,
		.max_sram_size = 0,
		.sram_offset = 0,
		.pbuf_reg = -1,
		.pbuf_shift = -1,
		.conn_format_mask = -1,
		.prealloc_size = 0,
		.direction = 1,
		.buffer_align_bytes = 64,
	},
};

enum dmic_wire_mode {
	DMIC_ONE_WIRE = 0,
	DMIC_TWO_WIRE,
};

static unsigned int channels_2_4_8[] = {
	2, 4, 8
};

static unsigned int channels_2_4[] = {
	2, 4
};

static unsigned int channels_1_2[] = {
	1, 2
};

static struct snd_pcm_hw_constraint_list constraints_channels_tdm_in = {
		.count = ARRAY_SIZE(channels_2_4_8),
		.list = channels_2_4_8,
		.mask = 0,
};

static struct snd_pcm_hw_constraint_list constraints_channels_dmic = {
		.count = ARRAY_SIZE(channels_2_4),
		.list = channels_2_4,
		.mask = 0,
};

static struct snd_pcm_hw_constraint_list constraints_channels_btsco = {
		.count = ARRAY_SIZE(channels_1_2),
		.list = channels_1_2,
		.mask = 0,
};

struct gnt_memif_info {
	int dai_id;
	char dai_name[32];
	unsigned int pbuf_size_reg;
	unsigned int pbuf_size_shift;
	unsigned int pbuf_size_mask;
	unsigned int pbuf_size_unit;
	unsigned int irq_mon_reg;
	int cur_tolerance;
};

struct gnt_memif_info gnt_memif_data[] = {
	{
		.dai_id = MTK_AFE_MEMIF_HDMI,
		.dai_name = "HDMI",
		.pbuf_size_reg = AFE_MEMIF_PBUF_SIZE,
		.pbuf_size_shift = 18,
		.pbuf_size_mask = 0x3,
		.pbuf_size_unit = 32,
		.irq_mon_reg = AFE_IRQ5_MCU_CNT_MON,
		.cur_tolerance = 1,
	},
};

static void hdmi_output_from_i2s_set(struct mtk_afe *afe, bool flags)
{
	if (flags) {
		regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG,
			S_I2S1_OUT_BCK_WS_SEL_MASK, 0x6 << S_I2S1_OUT_BCK_WS_SEL_POS);
		regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG,
			S_I2S1_OUT_DAT_SEL_MASK, 0x6 << S_I2S1_OUT_DAT_SEL_POS);
	} else {
		regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG,
			S_I2S1_OUT_BCK_WS_SEL_MASK, 0x0 << S_I2S1_OUT_BCK_WS_SEL_POS);
		regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG,
			S_I2S1_OUT_DAT_SEL_MASK, 0x0 << S_I2S1_OUT_DAT_SEL_POS);
	}
}

static int mt8695_asys_enable_irq(struct mtk_afe *afe, struct mt8695_afe_memif *memif)
{
	int irq_mode = memif->data->irq_mode;
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->asys_irq_mode_ref_cnt[irq_mode]++;
		if (afe->asys_irq_mode_ref_cnt[irq_mode] > 1) {
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	}

	switch (irq_mode) {
	case MTK_ASYS_IRQ_1:
		regmap_update_bits(afe->regmap, ASYS_IRQ1_CON, 1 << 31, 1 << 31);
		break;
	case MTK_ASYS_IRQ_2:
		regmap_update_bits(afe->regmap, ASYS_IRQ2_CON, 1 << 31, 1 << 31);
		break;
	case MTK_ASYS_IRQ_3:
		regmap_update_bits(afe->regmap, ASYS_IRQ3_CON, 1 << 31, 1 << 31);
		break;
	case MTK_ASYS_IRQ_4:
		regmap_update_bits(afe->regmap, ASYS_IRQ4_CON, 1 << 31, 1 << 31);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

static int mt8695_asys_disable_irq(struct mtk_afe *afe, struct mt8695_afe_memif *memif)
{
	int irq_mode = memif->data->irq_mode;
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->asys_irq_mode_ref_cnt[irq_mode]--;
	if (afe->asys_irq_mode_ref_cnt[irq_mode] > 0) {
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	} else if (afe->asys_irq_mode_ref_cnt[irq_mode] < 0) {
		afe->asys_irq_mode_ref_cnt[irq_mode] = 0;
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	}

	switch (irq_mode) {
	case MTK_ASYS_IRQ_1:
		regmap_update_bits(afe->regmap, ASYS_IRQ1_CON, 1 << 31, 0 << 31);
		regmap_write(afe->regmap, ASYS_IRQ_CLR, 1 << 0);
		break;
	case MTK_ASYS_IRQ_2:
		regmap_update_bits(afe->regmap, ASYS_IRQ2_CON, 1 << 31, 0 << 31);
		regmap_write(afe->regmap, ASYS_IRQ_CLR, 1 << 1);
		break;
	case MTK_ASYS_IRQ_3:
		regmap_update_bits(afe->regmap, ASYS_IRQ3_CON, 1 << 31, 0 << 31);
		regmap_write(afe->regmap, ASYS_IRQ_CLR, 1 << 2);
		break;
	case MTK_ASYS_IRQ_4:
		regmap_update_bits(afe->regmap, ASYS_IRQ4_CON, 1 << 31, 0 << 31);
		regmap_write(afe->regmap, ASYS_IRQ_CLR, 1 << 3);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}

static int mt8695_afe_enable_irq(struct mtk_afe *afe, struct mt8695_afe_memif *memif)
{
	int irq_mode = memif->data->irq_mode;
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->irq_mode_ref_cnt[irq_mode]++;
	if (afe->irq_mode_ref_cnt[irq_mode] > 1) {
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	}

	switch (irq_mode) {
	case MTK_AFE_IRQ_1:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 0, 1 << 0);
		break;
	case MTK_AFE_IRQ_2:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 1, 1 << 1);
		break;
	case MTK_AFE_IRQ_5:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 12, 1 << 12);
		break;
	case MTK_AFE_IRQ_6:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 13, 1 << 13);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}


static int mt8695_afe_disable_irq(struct mtk_afe *afe, struct mt8695_afe_memif *memif)
{
	int irq_mode = memif->data->irq_mode;
	unsigned long flags;

	spin_lock_irqsave(&afe->afe_ctrl_lock, flags);

	afe->irq_mode_ref_cnt[irq_mode]--;
	if (afe->irq_mode_ref_cnt[irq_mode] > 0) {
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	} else if (afe->irq_mode_ref_cnt[irq_mode] < 0) {
		afe->irq_mode_ref_cnt[irq_mode] = 0;
		spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);
		return 0;
	}

	switch (irq_mode) {
	case MTK_AFE_IRQ_1:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 0, 0 << 0);
		regmap_write(afe->regmap, AFE_IRQ_CLR, 1 << 0);
		break;
	case MTK_AFE_IRQ_2:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 1, 0 << 1);
		regmap_write(afe->regmap, AFE_IRQ_CLR, 1 << 1);
		break;
	case MTK_AFE_IRQ_5:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 12, 0 << 12);
		regmap_write(afe->regmap, AFE_IRQ_CLR, 1 << 4);
		break;
	case MTK_AFE_IRQ_6:
		regmap_update_bits(afe->regmap, AFE_IRQ_CON, 1 << 13, 0 << 13);
		regmap_write(afe->regmap, AFE_IRQ_CLR, 1 << 5);
		break;
	default:
		break;
	}

	spin_unlock_irqrestore(&afe->afe_ctrl_lock, flags);

	return 0;
}


static int mt8695_afe_suspend(struct device *dev)
{
	struct mtk_afe *afe = dev_get_drvdata(dev);
	unsigned int i = 0;

	afe->suspended = true;
	for ( ; i < MTK_AFE_CG_NUM; i++)
		afe->top_cg_ref_cnt[i] = 0;
	mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL1, 0);
	mt8695_clock_power(afe, MTK_CLK_TOP_APLL_SEL, 0);
	mt8695_clock_power(afe, MTK_CLK_TOP_A1SYS_HP_SEL, 0);

	return 0;
}

static int mt8695_afe_resume(struct device *dev)
{
	struct mtk_afe *afe = dev_get_drvdata(dev);

	afe->suspended = false;
	mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL1, 1);
	mt8695_clock_power(afe, MTK_CLK_TOP_APLL_SEL, 1);
	mt8695_clock_power(afe, MTK_CLK_TOP_A1SYS_HP_SEL, 1);
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_A1SYS_CK);
	regmap_write(afe->regmap, AUDIO_TOP_CON5, 0x18DF);

	return 0;
}

static int mt8695_afe_dai_suspend(struct snd_soc_dai *dai)
{
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);

	dev_dbg(afe->dev, "%s id %d suspended %d\n",
		__func__, dai->id, afe->suspended);

	if (afe->suspended)
		return 0;

	return mt8695_afe_suspend(afe->dev);
}

static int mt8695_afe_dai_resume(struct snd_soc_dai *dai)
{
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);

	dev_dbg(afe->dev, "%s id %d suspended %d\n",
		__func__, dai->id, afe->suspended);

	if (!afe->suspended)
		return 0;

	mt8695_afe_resume(afe->dev);

	return 0;
}

static int mt8695_afe_dais_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	int ret;

	dev_info(afe->dev, "%s\n", __func__);
	snd_pcm_hw_constraint_step(runtime, 0,
		SNDRV_PCM_HW_PARAM_BUFFER_BYTES, memif->data->buffer_align_bytes);

	snd_soc_set_runtime_hwparams(substream, &mt8695_afe_hardware);

	ret = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);

	if (ret < 0) {
		dev_info(afe->dev, "snd_pcm_hw_constraint_integer failed!\n");
		return ret;
	}
	if (rtd->cpu_dai->id == MTK_AFE_MEMIF_I2S)
		mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_MEMIF_DL1);
	else if (rtd->cpu_dai->id == MTK_AFE_MEMIF_DL2)
		mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_MEMIF_DL2);
	else if (rtd->cpu_dai->id == MTK_AFE_MEMIF_UL5)
		mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_MEMIF_UL5);

	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_AFE);
	mt8695_asys_timing_on(afe);
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_AFE_CONN);
	mt8695_afe_enable_afe_on(afe);

	memif->substream = substream;

	return 0;
}

static void mt8695_afe_dais_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	struct snd_pcm_runtime * const runtime = substream->runtime;

	dev_dbg(afe->dev, "%s %s\n", __func__, memif->data->name);

	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_AFE);
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_AFE_CONN);
	mt8695_asys_timing_off(afe);
	mt8695_afe_disable_afe_on(afe);
	if (memif->prepared) {
		if (!afe->force_clk_on && afe->is_apll_related_clks_on) {
			if (runtime->rate % 8000) {
				mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_A2SYS_CK);
				mt8695_clock_power(afe, MTK_CLK_TOP_A2SYS_HP_SEL, 0);
				if ((rtd->cpu_dai->id == MTK_AFE_MEMIF_SPDIF) ||
						(rtd->cpu_dai->id == MTK_AFE_MEMIF_HDMI) ||
						(rtd->cpu_dai->id == MTK_AFE_MEMIF_TDM_IN))
					mt8695_clock_power(afe, MTK_CLK_TOP_APLL2_SEL, 0);
				mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL2, 0);
			}
		}
		memif->prepared = false;
	}
	if (rtd->cpu_dai->id == MTK_AFE_MEMIF_I2S)
		mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_MEMIF_DL1);
	else if (rtd->cpu_dai->id == MTK_AFE_MEMIF_DL2)
		mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_MEMIF_DL2);
	else if (rtd->cpu_dai->id == MTK_AFE_MEMIF_UL5)
		mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_MEMIF_UL5);
	memif->substream = NULL;
	memif->avsync_mode = false;
}

static int mt8695_afe_dais_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,
					struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	const struct mt8695_afe_memif_data *data = memif->data;
	const size_t request_size = params_buffer_bytes(params);
	int ret;

	dev_info(afe->dev, "%s\n", __func__);
	dev_info(afe->dev,
		"%s[%s] period = %u, rate = %u, channels = %u, fmt = %u\n",
		__func__, dai->name, params_period_size(params), params_rate(params),
		params_channels(params), params_format(params));

	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));

	if (ret < 0) {
		dev_info(afe->dev,
				"%s %s malloc pages %zu bytes failed %d\n",
				__func__, data->name, request_size, ret);
		return ret;
	}

	memif->phys_buf_addr = substream->runtime->dma_addr;
	memif->buffer_size = substream->runtime->dma_bytes;
	if (rtd->cpu_dai->id == MTK_AFE_MEMIF_SPDIF) {
		afe->iec.buf.buf_sa = afe->iec.buf.buf_nsadr = memif->phys_buf_addr;
		afe->iec.buf.buf_ea = afe->iec.buf.buf_sa + memif->buffer_size;
	}

	if (rtd->cpu_dai->id == MTK_AFE_MEMIF_DMIC) {
		if ((afe->dmic_wire_mode == DMIC_TWO_WIRE) && (substream->runtime->channels) == 4)
			return -EINVAL;
	}
	return 0;
}

static int mt8695_afe_dais_hw_free(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	int ret = 0;

	dev_dbg(afe->dev, "%s %s\n", __func__, memif->data->name);

	ret = snd_pcm_lib_free_pages(substream);

	return ret;
}

static int mt8695_afe_dais_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	const struct mt8695_afe_memif_data *data = memif->data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	enum afe_sampling_rate fs;
	const unsigned int bit_width = snd_pcm_format_width(runtime->format);
	unsigned int irq_fs;
	bool turn_on_clk = false;
	unsigned int reg = 0;

	dev_info(afe->dev, "%s %s\n", __func__, memif->data->name);

	if ((afe->cached_sample_rate != runtime->rate) || (afe->cached_channels != runtime->channels)) {
		if (afe->is_apll_related_clks_on && (afe->cached_sample_rate % 8000)) {
			mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_A2SYS_CK);
			mt8695_clock_power(afe, MTK_CLK_TOP_A2SYS_HP_SEL, 0);
			if ((rtd->cpu_dai->id == MTK_AFE_MEMIF_SPDIF) ||
				(rtd->cpu_dai->id == MTK_AFE_MEMIF_HDMI) ||
				(rtd->cpu_dai->id == MTK_AFE_MEMIF_TDM_IN))
				mt8695_clock_power(afe, MTK_CLK_TOP_APLL2_SEL, 0);
			mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL2, 0);
			afe->is_apll_related_clks_on = false;
			}
			turn_on_clk = true;
		} else if (!memif->prepared) {
			turn_on_clk = true;
	}

	if (turn_on_clk && !afe->is_apll_related_clks_on && (runtime->rate % 8000)) {
		mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL2, 1);
		if ((rtd->cpu_dai->id == MTK_AFE_MEMIF_SPDIF) || (rtd->cpu_dai->id == MTK_AFE_MEMIF_HDMI) ||
				(rtd->cpu_dai->id == MTK_AFE_MEMIF_TDM_IN))
			mt8695_clock_power(afe, MTK_CLK_TOP_APLL2_SEL, 1);
		mt8695_clock_power(afe, MTK_CLK_TOP_A2SYS_HP_SEL, 1);
		mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_A2SYS_CK);
		afe->is_apll_related_clks_on = true;
	}

	if (!memif->prepared) {
		if (rtd->cpu_dai->id != MTK_AFE_MEMIF_SPDIF) {
			/* start */
			regmap_write(afe->regmap, data->reg_ofs_base, memif->phys_buf_addr);
			/* end */
			regmap_write(afe->regmap, data->reg_ofs_end, memif->phys_buf_addr + memif->buffer_size - 1);
		} else {
			/* start */
			dev_info(afe->dev, "%d %d %d\n", data->reg_ofs_base, memif->phys_buf_addr, memif->buffer_size);
			regmap_write(afe->regmap, data->reg_ofs_base, memif->phys_buf_addr);
			/* end */
			regmap_write(afe->regmap, data->reg_ofs_end, memif->phys_buf_addr + memif->buffer_size);
		}

		/* set channel */
		if (data->mono_shift >= 0) {
			unsigned int mono = (runtime->channels == 1) ? 1 : 0;

			if (data->direction == 0)
				reg = AFE_DAC_CON3;
			else if (data->direction == 1)
				reg = AFE_DAC_CON4;
			regmap_update_bits(afe->regmap, reg, 1 << data->mono_shift,
						mono << data->mono_shift);
		}
		/* set lsb format */
		if (data->lsb_mode_shift >= 0) {
			if (data->direction == 0)
				reg = AFE_DAC_CON3;
			else if (data->direction == 1)
				reg = AFE_DAC_CON4;
			regmap_update_bits(afe->regmap, reg, 1 << data->lsb_mode_shift, 0);

		}

		/* set pbuf size */
		if (data->pbuf_reg >= 0)
			regmap_update_bits(afe->regmap, data->pbuf_reg, 3 << data->pbuf_shift, 3 << data->pbuf_shift);

		/* set rate */
		if (data->fs_shift >= 0) {
			fs = rate_convert_enum(runtime->rate);
			if (data->direction == 0)
				regmap_update_bits(afe->regmap, AFE_DAC_CON1, 0x1F << data->fs_shift,
					fs << data->fs_shift);
			else if (data->direction == 1)
				regmap_update_bits(afe->regmap, AFE_DAC_CON2, 0x1F << data->fs_shift,
					fs << data->fs_shift);
		}

		if ((data->hd_shift >= 0) && (data->direction == 0)) {
			if (bit_width == 32)
				regmap_update_bits(afe->regmap, AFE_MEMIF_HD_CON0,
							0x3 << data->hd_shift, 1 << data->hd_shift);
			else
				regmap_update_bits(afe->regmap, AFE_MEMIF_HD_CON0, 0x3 << data->hd_shift, 0);
		}

		if ((data->hd_shift >= 0) && (data->direction == 1)) {
			if ((bit_width == 32) || ((rtd->cpu_dai->id == MTK_AFE_MEMIF_DMIC) && (runtime->channels == 4)))
				regmap_update_bits(afe->regmap, AFE_MEMIF_HD_CON1,
							0x1 << data->hd_shift, 1 << data->hd_shift);
			else
				regmap_update_bits(afe->regmap, AFE_MEMIF_HD_CON1, 0x1 << data->hd_shift, 0);
		}

		if (data->dsd_width_shift >= 0)
			regmap_update_bits(afe->regmap, AFE_DAC_CON5, 0x3 << data->dsd_width_shift, 0);


		/* set irq counter */
		if (data->irq_reg_cnt >= 0)
			regmap_update_bits(afe->regmap, data->irq_reg_cnt, 0x3ffff << data->irq_cnt_shift,
					runtime->period_size << data->irq_cnt_shift);

		/* set irq fs */
		if ((data->asys_irq_fs_shift >= 0) && (data->irq_fs_reg >= 0)) {
			switch (runtime->rate) {
			case 8000:
				irq_fs = 0;
				break;
			case 12000:
				irq_fs = 1;
				break;
			case 16000:
				irq_fs = 2;
				break;
			case 24000:
				irq_fs = 3;
				break;
			case 32000:
				irq_fs = 4;
				break;
			case 48000:
				irq_fs = 5;
				break;
			case 96000:
				irq_fs = 6;
				break;
			case 192000:
				irq_fs = 7;
				break;
			case 11025:
				irq_fs = 0x11;
				break;
			case 22050:
				irq_fs = 0x13;
				break;
			case 44100:
				irq_fs = 0x15;
				break;
			case 88200:
				irq_fs = 0x16;
				break;
			case 176400:
				irq_fs = 0x17;
				break;
			case 352800:
				irq_fs = 0x18;
				break;
			default:
				irq_fs = 0;
				dev_info(afe->dev, "%s irq fs(%d) is not supported!", __func__, runtime->rate);
				break;
			}
			regmap_update_bits(afe->regmap, data->irq_fs_reg, 0x1F << data->asys_irq_fs_shift,
									irq_fs << data->asys_irq_fs_shift);
		}
		if ((data->irq_fs_shift >= 0) && (data->irq_fs_reg >= 0)) {
			switch (runtime->rate) {
			case 8000:
				irq_fs = 0;
				break;
			case 12000:
				irq_fs = 1;
				break;
			case 16000:
				irq_fs = 2;
				break;
			case 24000:
				irq_fs = 3;
				break;
			case 32000:
				irq_fs = 4;
				break;
			case 48000:
				irq_fs = 5;
				break;
			case 96000:
				irq_fs = 6;
				break;
			case 192000:
				irq_fs = 7;
				break;
			case 11025:
				irq_fs = 9;
				break;
			case 22050:
				irq_fs = 0xb;
				break;
			case 44100:
				irq_fs = 0xd;
				break;
			case 88200:
				irq_fs = 0xe;
				break;
			case 176400:
				irq_fs = 0xf;
				break;
			default:
				irq_fs = 0;
				dev_info(afe->dev, "%s irq fs(%d) is not supported!", __func__, runtime->rate);
				break;
			}

			regmap_update_bits(afe->regmap, data->irq_fs_reg, 0xF << data->irq_fs_shift,
									irq_fs << data->irq_fs_shift);
		}

		memif->prepared = true;
		memif->buf_frame_size = runtime->frame_bits / 8;
		afe->cached_sample_rate = runtime->rate;
		afe->cached_channels = runtime->channels;
	}
	return 0;
}

static int mt8695_afe_dais_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	unsigned long flags;

	dev_dbg(afe->dev, "%s %s cmd = %d\n", __func__, memif->data->name, cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		if (memif->data->enable_shift >= 0)
			regmap_update_bits(afe->regmap, AFE_DAC_CON0,
						1 << memif->data->enable_shift,
						1 << memif->data->enable_shift);

		mt8695_afe_enable_irq(afe, memif);
		mt8695_asys_enable_irq(afe, memif);
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		if (memif->data->enable_shift >= 0)
			regmap_update_bits(afe->regmap, AFE_DAC_CON0,
						1 << memif->data->enable_shift, 0);

		mt8695_afe_disable_irq(afe, memif);
		mt8695_asys_disable_irq(afe, memif);
		if (memif->avsync_mode) {
			spin_lock_irqsave(&memif->buf_info_lock, flags);
			memif->buf_read = memif->buf_write = 0;
			memif->buf_reset = true;
			spin_unlock_irqrestore(&memif->buf_info_lock, flags);
		}

		return 0;
	default:
		return -EINVAL;
	}
}

static int mt8695_afe_hdmi_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

#if MT8695_FPGA_EARLY_PORTING
	regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG,
				S_I2S1_OUT_BCK_WS_SEL_MASK
				| S_I2S1_OUT_DAT_SEL_MASK
				| S_I2S2_OUT_DAT_SEL_MASK
				| S_I2S3_OUT_DAT_SEL_MASK
				| S_I2S4_OUT_DAT_SEL_MASK,
				(6 << S_I2S1_OUT_BCK_WS_SEL_POS)
				| (6 << S_I2S1_OUT_DAT_SEL_POS)
				| (7 << S_I2S2_OUT_DAT_SEL_POS)
				| (8 << S_I2S3_OUT_DAT_SEL_POS)
				| (9 << S_I2S4_OUT_DAT_SEL_POS));

#endif
	/* O20~O27: L/R/LS/RS/C/LFE/CH7/CH8 */
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O26_POS, MTK_AFE_HDMI_CONN_I26 << HDMI_INTER_O26_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O27_POS, MTK_AFE_HDMI_CONN_I27 << HDMI_INTER_O27_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O24_POS, MTK_AFE_HDMI_CONN_I24 << HDMI_INTER_O24_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O25_POS, MTK_AFE_HDMI_CONN_I25 << HDMI_INTER_O25_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O22_POS, MTK_AFE_HDMI_CONN_I22 << HDMI_INTER_O22_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O23_POS, MTK_AFE_HDMI_CONN_I23 << HDMI_INTER_O23_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O20_POS, MTK_AFE_HDMI_CONN_I20 << HDMI_INTER_O20_POS);
	regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
						7 << HDMI_INTER_O21_POS, MTK_AFE_HDMI_CONN_I21 << HDMI_INTER_O21_POS);

	return 0;
}

static void mt8695_afe_hdmi_shutdown(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;

	dev_info(afe->dev, "%s\n", __func__);
	if (be->prepared[stream]) {
		/* disable HDMI */
		regmap_update_bits(afe->regmap, AFE_HDMI_CONN0,
							(7 << HDMI_INTER_O26_POS)
							| (7 << HDMI_INTER_O27_POS)
							| (7 << HDMI_INTER_O24_POS)
							| (7 << HDMI_INTER_O25_POS)
							| (7 << HDMI_INTER_O22_POS)
							| (7 << HDMI_INTER_O23_POS)
							| (7 << HDMI_INTER_O20_POS)
							| (7 << HDMI_INTER_O21_POS), 0);
		be->prepared[stream] = false;
	}
}

static int mt8695_afe_hdmi_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int rate = runtime->rate;
	const unsigned int channels = runtime->channels;
	const int bit_width = snd_pcm_format_width(runtime->format);
	const unsigned int stream = substream->stream;
	unsigned int val = 0;

	dev_info(afe->dev, "%s rate %d channels %d bit_width %d\n", __func__, rate, channels, bit_width);

	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}

	if (afe->force_clk_on) {
		if ((afe->cached_hdmi_audio_format == afe->hdmi_audio_format) &&
			(afe->cached_hdmi_audio_sample_rate == rate) &&
			(afe->cached_hdmi_audio_channels == channels) &&
			(afe->cached_hdmi_audio_bit_depth == bit_width)) {
			afe->need_hdmi_toggle = false;
		} else {
			/* disable Out control */
			regmap_update_bits(afe->regmap, AFE_8CH_I2S_OUT_CON, HDMI_8CH_I2S_CON_EN_MASK, 0);
			mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_HDMI_CK);
			/* TODO - check if some hdmi sink need delay between
			 * disable and enable hdmi out control */
			afe->need_hdmi_toggle = true;
			afe->cached_hdmi_audio_format = afe->hdmi_audio_format;
			afe->cached_hdmi_audio_sample_rate = rate;
			afe->cached_hdmi_audio_channels = channels;
			afe->cached_hdmi_audio_bit_depth = bit_width;
		}
	}
	/* Set HDMI Clock */
	mt8695_hdmi_set_clock(afe, rate);

	val |= (channels << HDMI_OUT_CH_NUM_POS);

	if (bit_width == 32)
		val |= HDMI_OUT_BIT_WIDTH_32;

	regmap_write(afe->regmap, AFE_HDMI_OUT_CON0, val);

	val = 0;

	val |= HDMI_8CH_I2S_CON_I2S_32BIT;

	if (be->fmt_mode == SND_SOC_DAIFMT_I2S)
		val |= HDMI_8CH_I2S_CON_I2S_DELAY;
	else
		val |= HDMI_8CH_I2S_CON_LRCK_INV;

	val |= HDMI_8CH_I2S_CON_BCK_INV;

	if (!afe->force_clk_on || afe->need_hdmi_toggle)
		regmap_write(afe->regmap, AFE_8CH_I2S_OUT_CON, val);

	be->prepared[stream] = true;

	return 0;
}

static int mt8695_afe_hdmi_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct timespec ts;

	dev_info(afe->dev, "%s cmd=%d %s\n", __func__, cmd, dai->name);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		if (!afe->force_clk_on || afe->need_hdmi_toggle) {
			/* enable Out control */
			mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_HDMI_CK);
			regmap_update_bits(afe->regmap, AFE_8CH_I2S_OUT_CON, HDMI_8CH_I2S_CON_EN_MASK, HDMI_8CH_I2S_CON_EN);
		}
		/*enable hdmi agent */
		regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, HDMI_OUT_ON_MASK, HDMI_OUT_ON);

		getrawmonotonic(&ts);
		hdmi_dac_timestamp = timespec_to_ns(&ts);
		hdmi_dac_state = 1;

		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/*disable hdmi agent */
		regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, HDMI_OUT_ON_MASK, 0);
		/* disable Out control */
		if (!afe->force_clk_on) {
			regmap_update_bits(afe->regmap, AFE_8CH_I2S_OUT_CON, HDMI_8CH_I2S_CON_EN_MASK, 0);
			mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_HDMI_CK);
		}

		getrawmonotonic(&ts);
		hdmi_dac_timestamp = timespec_to_ns(&ts);
		hdmi_dac_state = 0;

		return 0;
	default:
		return -EINVAL;
	}
}

static int mt8695_afe_hdmi_set_fmt(struct snd_soc_dai *dai,
				unsigned int fmt)
{
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];

	dev_dbg(afe->dev, "%s %s\n", __func__, dai->name);

	be->fmt_mode = 0;
	/* set DAI format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
		be->fmt_mode |= fmt & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		dev_info(afe->dev, "invalid dai format %u\n", fmt);
		return -EINVAL;
	}
	return 0;
}

static int mt8695_afe_spdif_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	dev_info(afe->dev, "%s %s\n", __func__, dai->name);

	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_SPDIF_CK);

	return 0;
}

static void mt8695_afe_spdif_shutdown(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;

	dev_info(afe->dev, "%s %s\n", __func__, dai->name);

	if (be->prepared[stream]) {
		/* disable SPDIF */
		regmap_update_bits(afe->regmap, AFE_SPDIF_OUT_CON0,
							SPDIF_OUT_CLOCK_ON_OFF_MASK | SPDIF_OUT_MEMIF_ON_OFF_MASK,
							SPDIF_OUT_CLOCK_OFF | SPDIF_OUT_MEMIF_OFF);
		be->prepared[stream] = false;
	}
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_SPDIF_CK);
}

static int mt8695_afe_spdif_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	const unsigned int rate = runtime->rate;
	const int bit_width = snd_pcm_format_width(runtime->format);
	const unsigned int stream = substream->stream;
	const snd_pcm_uframes_t period_size = runtime->period_size;
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];

	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}

	afe->iec.bit_width = bit_width;
	afe->iec.nsnum = ((period_size >> 1) << 16) | period_size;
	afe->iec.period_bytes = period_size * 2 * (bit_width >> 3);
	switch (rate) {
	case 32000:
		afe->iec.force_update_size = 2;
		break;
	case 44100:
		afe->iec.force_update_size = 3;
		break;
	case 48000:
		afe->iec.force_update_size = 3;
		break;
	case 88200:
		afe->iec.force_update_size = 5;
		break;
	case 96000:
		afe->iec.force_update_size = 5;
		break;
	case 176400:
		afe->iec.force_update_size = 9;
		break;
	case 192000:
		afe->iec.force_update_size = 0xa;
		break;
	default:
		afe->iec.force_update_size = 0;
		break;
	}
	mt8695_iec_set_clock(afe, rate);
	regmap_update_bits(afe->regmap, AFE_SPDIF_OUT_CON0,
							SPDIF_OUT_CLOCK_ON_OFF_MASK | SPDIF_OUT_MEMIF_ON_OFF_MASK,
							SPDIF_OUT_CLOCK_ON | SPDIF_OUT_MEMIF_ON);
	mt8695_iec_set_config(afe);
	be->prepared[stream] = true;
	return 0;
}

static int mt8695_afe_spdif_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	dev_info(afe->dev, "%s cmd=%d %s\n", __func__, cmd, dai->name);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* enable Out control */
		regmap_update_bits(afe->regmap, AFE_IEC_BURST_INFO, IEC_BURST_NOT_READY, IEC_BURST_NOT_READY_MASK);
		udelay(2000);
		regmap_update_bits(afe->regmap, AFE_IEC_CFG, IEC_ENABLE, IEC_EN_MASK);
		return 0;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* disable Out control */
		regmap_update_bits(afe->regmap, AFE_IEC_CFG, IEC_DISABLE, IEC_EN_MASK);
		return 0;
	default:
		return -EINVAL;
	}
}

static int mt8695_afe_i2s_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG, S_I2S1_OUT_BCK_WS_SEL_MASK, 0);
	regmap_update_bits(afe->regmap, AFE_I2SOUT_CH_CFG, S_I2S1_OUT_DAT_SEL_MASK, 0);
	mt8695_afe_enable_i2so1_mclk(afe);
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_I2SO1_CK);
	return 0;
}

static void mt8695_afe_i2s_shutdown(struct snd_pcm_substream *substream,
				  struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;

	if (be->prepared[stream]) {
		/* disable I2S */
		be->prepared[stream] = false;
	}
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_I2SO1_CK);
	mt8695_afe_disable_i2so1_mclk(afe);
}

static int mt8695_afe_i2s_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct mt8695_i2s_out_config config;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	const unsigned int stream = substream->stream;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const int bit_width = snd_pcm_format_width(runtime->format);

	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}
	config.dsd_mode = 0;
	config.dsd_use = I2S_OUT_DSD_USE_NORMAL;
	config.couple_mode = 0;
	config.one_heart_mode = 0;
	config.slave = 0;
	if (bit_width == 16) {
		if (be->fmt_mode == SND_SOC_DAIFMT_I2S)
			config.fmt = FMT_64CYCLE_16BIT_I2S;
		else if (be->fmt_mode == SND_SOC_DAIFMT_LEFT_J)
			config.fmt = FMT_64CYCLE_16BIT_LJ;
		else if (be->fmt_mode == SND_SOC_DAIFMT_RIGHT_J)
			config.fmt = FMT_64CYCLE_16BIT_RJ;
	} else if (bit_width == 32) {
		if (be->fmt_mode == SND_SOC_DAIFMT_I2S)
			config.fmt = FMT_64CYCLE_32BIT_I2S;
		else if (be->fmt_mode == SND_SOC_DAIFMT_LEFT_J)
			config.fmt = FMT_64CYCLE_32BIT_LJ;
		else if (be->fmt_mode == SND_SOC_DAIFMT_RIGHT_J)
			config.fmt = FMT_64CYCLE_32BIT_LJ;
	}
	config.mclk = 0;
	config.use_asrc = 0;
	config.mode = rate_convert_enum(runtime->rate);

	mt8695_i2so_set_config(afe, &config);
	mt8695_i2so1_set_mclk(afe, runtime->rate);
	be->prepared[stream] = true;

	return 0;
}

static int mt8695_afe_i2s_trigger(struct snd_pcm_substream *substream, int cmd,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	dev_info(afe->dev, "%s cmd=%d %s\n", __func__, cmd, dai->name);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* enable Out control */
		regmap_update_bits(afe->regmap, ASYS_I2SO1_CON, I2S_SW_RESET_MASK, I2S_SW_RESET_EN);
		udelay(1);
		regmap_update_bits(afe->regmap, ASYS_I2SO1_CON, I2S_SW_RESET_MASK, I2S_SW_RESET_DIS);
		udelay(1);
		regmap_update_bits(afe->regmap, ASYS_I2SO1_CON, I2S_ENABLE_MASK, I2S_ENABLE);
		return 0;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* disable Out control */
		regmap_update_bits(afe->regmap, ASYS_I2SO1_CON, I2S_ENABLE_MASK, I2S_DISABLE);
		return 0;
	default:
		return -EINVAL;
	}
}

static int mt8695_afe_i2s_set_fmt(struct snd_soc_dai *dai,
				unsigned int fmt)
{
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];

	be->fmt_mode = 0;
	/* set DAI format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
	case SND_SOC_DAIFMT_RIGHT_J:
		be->fmt_mode |= fmt & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		dev_info(afe->dev, "invalid dai format %u\n", fmt);
		return -EINVAL;
	}
	return 0;
}

static int mt8695_iec_chstatus_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 4;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x7FFFFFFF;
	uinfo->value.integer.step = 1;
	return 0;
}

static int mt8695_iec_chstatus_set(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	afe->iec.ch_status.chl_stat0 = ucontrol->value.integer.value[0];
	afe->iec.ch_status.chl_stat1 = ucontrol->value.integer.value[1];
	afe->iec.ch_status.chr_stat0 = ucontrol->value.integer.value[2];
	afe->iec.ch_status.chr_stat1 = ucontrol->value.integer.value[3];

	return 0;
}

static int mt8695_hdmi_audio_format_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x7FFFFFFF;
	uinfo->value.integer.step = 1;
	return 0;
}

static int mt8695_hdmi_audio_format_get(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = afe->hdmi_audio_format;

	return 0;
}

static int mt8695_hdmi_audio_format_set(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	afe->hdmi_audio_format = ucontrol->value.integer.value[0];

	return 0;
}

static int mt8695_avsync_mode_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	uinfo->value.integer.step = 1;
	return 0;
}

static int mt8695_avsync_mode_set(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt8695_afe_memif *memif = &afe->memif[MTK_AFE_MEMIF_HDMI];
	unsigned long flags;

	if (ucontrol->value.integer.value[0]) {
		if (!memif->avsync_mode) {
			memif->avsync_mode = true;
			spin_lock_irqsave(&memif->buf_info_lock, flags);
			memif->buf_reset = true;
			memif->buf_read = memif->buf_write = 0;
			spin_unlock_irqrestore(&memif->buf_info_lock, flags);
		}
	} else
		memif->avsync_mode = false;

	return 0;
}

static int mt8695_avsync_mode_get(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	if (afe->memif[MTK_AFE_MEMIF_HDMI].avsync_mode)
		ucontrol->value.integer.value[0] = 1;
	else
		ucontrol->value.integer.value[0] = 0;

	return 0;
}

static int mt8695_buf_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 3;
	return 0;
}

static int mt8695_buf_info_set(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);
	unsigned long flags = 0;
	struct mt8695_afe_memif *memif = &afe->memif[MTK_AFE_MEMIF_HDMI];

	spin_lock_irqsave(&memif->buf_info_lock, flags);
	if (ucontrol->value.integer.value[2]) {
		memif->buf_read = memif->buf_write = 0;
		memif->buf_reset = true;
	} else
		memif->buf_reset = false;
	spin_unlock_irqrestore(&memif->buf_info_lock, flags);
	return 0;
}

static int mt8695_buf_info_get(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	unsigned int buf_read_cur, buf_read_bef, buf_addition;
	unsigned long flags = 0;
	unsigned long buf_cmp;
	unsigned long long temp_read;
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt8695_afe_memif *memif = &afe->memif[MTK_AFE_MEMIF_HDMI];

	if (memif->avsync_mode) {
		spin_lock_irqsave(&memif->buf_info_lock, flags);
		temp_read = memif->buf_read;
		buf_read_bef = do_div(temp_read, memif->buffer_size);
		regmap_read(afe->regmap, AFE_HDMI_OUT_CUR, &buf_read_cur);
		if (buf_read_cur == 0)
			buf_read_cur = memif->phys_buf_addr;

		if (memif->prepared)
			buf_read_cur -= memif->phys_buf_addr;
		else
			buf_read_cur = 0;

		if (buf_read_cur >= buf_read_bef)
			buf_addition = buf_read_cur - buf_read_bef;
		else
			buf_addition = buf_read_cur + memif->buffer_size - buf_read_bef;

		buf_cmp = memif->buf_read + buf_addition;
		if (memif->buf_read >= memif->buf_write) {
			dev_err(afe->dev, "%s abnormal case underrun happened!\n", __func__);
			dev_err(afe->dev, "%s memif->buf_read 0x%llx, memif->buf_write 0x%llx!\n", __func__,
				memif->buf_read, memif->buf_write);
			memif->buf_read = buf_addition;
			memif->buf_reset = true;
		} else {
			memif->buf_read += buf_addition;
		}
		temp_read = memif->buf_read;
		do_div(temp_read, memif->buf_frame_size);
		ucontrol->value.integer.value[0] = (long)(do_div(temp_read, 0xffffffff));
		temp_read = memif->buf_write;
		do_div(temp_read, memif->buf_frame_size);
		ucontrol->value.integer.value[1] = (long)(do_div(temp_read, 0xffffffff));
		ucontrol->value.integer.value[2] = (long)memif->buf_reset;
		spin_unlock_irqrestore(&memif->buf_info_lock, flags);
	} else {
		ucontrol->value.integer.value[0] = 0;
		ucontrol->value.integer.value[1] = 0;
		ucontrol->value.integer.value[2] = true;
	}
	return 0;
}

static int mt8695_hdmi_force_clk_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	uinfo->value.integer.step = 1;
	return 0;
}

static int mt8695_hdmi_force_clk_set(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	if (!ucontrol->value.integer.value[0]) {
		if (afe->force_clk_on) {
			if (afe->is_apll_related_clks_on) {
				if (afe->cached_sample_rate % 8000) {
					mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_A2SYS_CK);
					mt8695_clock_power(afe, MTK_CLK_TOP_A2SYS_HP_SEL, 0);
					mt8695_clock_power(afe, MTK_CLK_TOP_APLL2_SEL, 0);
					mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL2, 0);
				}
				afe->is_apll_related_clks_on = false;
			}

			regmap_update_bits(afe->regmap, AFE_8CH_I2S_OUT_CON, HDMI_OUT_ON_MASK, 0);
			regmap_update_bits(afe->regmap, AFE_HDMI_OUT_CON0, HDMI_8CH_I2S_CON_EN_MASK, 0);
			mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_HDMI_CK);
			afe->force_clk_on = false;
		}
		afe->hdmi_force_clk_switch = ucontrol->value.integer.value[0];
	} else {
		afe->hdmi_force_clk_switch = ucontrol->value.integer.value[0];
		afe->force_clk_on = true;
	}

	return 0;
}

static int mt8695_hdmi_force_clk_get(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct mtk_afe *afe = snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = afe->hdmi_force_clk_switch;

	return 0;
}

static long long mt8695_afe_get_next_write_timestamp(struct snd_soc_card *card,
				    struct gnt_memif_info info)
{
	struct snd_pcm_substream *substream;
	struct snd_soc_pcm_runtime *rtd = NULL;
	struct snd_pcm_runtime *runtime;
	struct mtk_afe *afe;
	struct mt8695_afe_memif *memif;
	unsigned long flag;
	unsigned int pbuf_size = 0;
	int i;
	int ret;
	long long timestamp = 0;
	long long temp = 0;
	int remain_size = 0;
	int real_remain_size;
	int delta = 0;
	int irq_count;
	int rptr_offset = 0;
	int max_prefetch_size;
	int period = 0;
	int rate;

	/* search target substream */
	for (i = 0; i < card->num_rtd; i++) {
		rtd = &(card->rtd[i]);
		if (rtd->cpu_dai->id == info.dai_id &&
		    !strcmp(rtd->cpu_dai->name, info.dai_name)) {
			break;
		}
	}

	if (i == card->num_rtd) {
		dev_err(afe->dev, "%s can not find the target substream\n", __func__);
		timestamp = -1;
		return timestamp;
	}

	substream = rtd->pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	afe = snd_soc_platform_get_drvdata(rtd->platform);
	memif = &afe->memif[rtd->cpu_dai->id];

	/* get target sub stream information */
	snd_pcm_stream_lock_irqsave(substream, flag);
	runtime = substream->runtime;
	if (runtime == NULL) {
		dev_err(afe->dev, "%s() playback substream is not opened(%d)\n",
		    __func__, substream->hw_opened);
		snd_pcm_stream_unlock_irqrestore(substream, flag);
		timestamp = -1;
		return timestamp;
	}

	if ((runtime->status->state == SNDRV_PCM_STATE_OPEN) ||
	    (runtime->status->state == SNDRV_PCM_STATE_DISCONNECTED) ||
	    (runtime->status->state == SNDRV_PCM_STATE_SUSPENDED)) {
		dev_err(afe->dev, "%s() playback state(%d) is not right\n",
		    __func__, runtime->status->state);
		snd_pcm_stream_unlock_irqrestore(substream, flag);
		timestamp = -1;
		return timestamp;
	}

	period = (int)(runtime->period_size);
	rate = (int)(runtime->rate);

	if (runtime->status->state == SNDRV_PCM_STATE_XRUN) {
		real_remain_size = 0;
		timestamp = (long long)mtk_timer_get_cnt(6);
		snd_pcm_stream_unlock_irqrestore(substream, flag);
		dev_err(afe->dev, "%s() playback state is xrun state\n", __func__);
		goto output_cal;
	}

	if ((runtime->status->state != SNDRV_PCM_STATE_XRUN) &&
		  (!snd_pcm_running(substream))) {
		real_remain_size =
		    (int)(runtime->buffer_size - snd_pcm_playback_avail(runtime) + runtime->delay);
		timestamp = (long long)mtk_timer_get_cnt(6);
		snd_pcm_stream_unlock_irqrestore(substream, flag);
		goto output_cal;
	}

	regmap_read(afe->regmap, info.pbuf_size_reg, &pbuf_size);
	pbuf_size = ((pbuf_size>>info.pbuf_size_shift) & info.pbuf_size_mask);
	pbuf_size = (info.pbuf_size_unit)<<pbuf_size;
	max_prefetch_size = bytes_to_frames(runtime, pbuf_size);

	timestamp = (long long)mtk_timer_get_cnt(6);
	regmap_read(afe->regmap, info.irq_mon_reg, &irq_count);
	dev_dbg(afe->dev, "%s() max_prefetch_size = %d timestamp = 0x%16llx\n", __func__, max_prefetch_size, timestamp);
	dev_dbg(afe->dev, "%s() irq_count = %d\n", __func__, irq_count);
	/*update the hw_ptr now*/
	ret = snd_pcm_update_hw_ptr(substream);

	if (ret < 0) {
		real_remain_size = 0;
		snd_pcm_stream_unlock_irqrestore(substream, flag);
		goto output_cal;
	}

	remain_size =
	    (int)(runtime->buffer_size - snd_pcm_playback_avail(runtime) + runtime->delay);
	delta = (int)(runtime->control->appl_ptr % runtime->period_size);
	if (delta != 0)
		delta = period - delta;

	rptr_offset = (int)(runtime->status->hw_ptr % runtime->period_size);
	snd_pcm_stream_unlock_irqrestore(substream, flag);

	if (remain_size + delta <= irq_count) {
		real_remain_size = irq_count;
		real_remain_size -= delta;
	} else {
		int tmp;

		tmp = remain_size + delta - irq_count;
		real_remain_size = irq_count + (tmp / period + ((tmp%period) ? 1 : 0)) * period;
		real_remain_size -= delta;
	}

	/* unormal case check */
	if ((real_remain_size > (remain_size + max_prefetch_size + info.cur_tolerance)) ||
	    (real_remain_size < (remain_size - info.cur_tolerance))) {
		dev_err(afe->dev, "%s() invalid real_remain_size(%d), irq_count(%d), rptr_offset(%d), delta(%d), periods(%d), remain(%d)\n",
		    __func__, real_remain_size, irq_count,
		    rptr_offset, delta, period, remain_size);
		real_remain_size = remain_size;
	}

output_cal:
	temp = (13000000LL) * (long long)real_remain_size;
	do_div(temp, (long long)rate);
	timestamp += temp;
	return timestamp;
}

static int mt8695_hdmi_dai_state_get(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = hdmi_dac_state;
	return 0;
}

static int mt8695_hdmi_dai_state_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0xFFFFFFFF;

	return 0;
}

static int mt8695_afe_hdmi_dai_timestamp_get(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer64.value[0] = hdmi_dac_timestamp;
	return 0;
}

static int mt8695_afe_hdmi_timestamp_get(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_platform *platform = snd_soc_kcontrol_platform(kcontrol);
	struct snd_soc_card *card = platform->component.card;

	ucontrol->value.integer64.value[0] =
	    mt8695_afe_get_next_write_timestamp(card, gnt_memif_data[0]);
	return 0;
}



static int mt8695_afe_tdm_in_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);

	snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS,
			&constraints_channels_tdm_in);
	mt8695_afe_enable_i2so2_mclk(afe);

	return 0;
}

static void mt8695_afe_tdm_in_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;
		if (be->prepared[stream]) {
		/* disable tdmin */
		be->prepared[stream] = false;
	}
	mt8695_afe_disable_i2so2_mclk(afe);
}

static int mt8695_afe_tdm_in_prepare(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int rate = runtime->rate;
	const unsigned int channels = runtime->channels;
	const int bit_width = snd_pcm_format_width(runtime->format);
	const unsigned int stream = substream->stream;
	unsigned int val, val2;

	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}
	dev_info(afe->dev, "%s\n", __func__);
	/* Set TDM IN  Clock */
	mt8695_tdmin_set_clock(afe, rate, channels, bit_width);

	val = 0;

	if ((be->fmt_mode & SND_SOC_DAIFMT_FORMAT_MASK) == SND_SOC_DAIFMT_I2S)
		val |= AFE_TDM_IN_CON1_I2S | AFE_TDM_IN_CON1_LRCK_DELAY;

	/* bck&lrck phase */
	switch (be->fmt_mode & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_IF:
		val |= AFE_TDM_IN_CON1_LRCK_INV |
		AFE_TDM_IN_CON1_BCK_INV;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		val |= AFE_TDM_IN_CON1_LRCK_INV;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		val |= AFE_TDM_IN_CON1_BCK_INV;
		break;
	default:
		break;
	}

	/* bit width related */
	if (bit_width > 16) {
		val2 = 32;
		val |= AFE_TDM_IN_CON1_WLEN_32BIT |
			AFE_TDM_IN_CON1_FAST_LRCK_CYCLE_32BCK;
		val |= AFE_TDM_IN_CON1_LRCK_WIDTH(val2);
	} else {
		val2 = 16;
		val |= AFE_TDM_IN_CON1_WLEN_16BIT |
			AFE_TDM_IN_CON1_FAST_LRCK_CYCLE_16BCK;
		val |= AFE_TDM_IN_CON1_LRCK_WIDTH(val2);
	}

	switch (channels) {
	case 2:
		val |= AFE_TDM_IN_CON1_2CH_PER_SDATA;
		break;
	case 4:
		val |= AFE_TDM_IN_CON1_4CH_PER_SDATA;
		break;
	case 8:
		val |= AFE_TDM_IN_CON1_8CH_PER_SDATA;
		break;
	default:
		break;
	}

	regmap_update_bits(afe->regmap, AFE_TDM_IN_CON1,
		~(u32)AFE_TDM_IN_CON1_EN, val);

	be->prepared[stream] = true;
	return 0;
}

static int mt8695_afe_tdm_in_set_fmt(struct snd_soc_dai *dai,
				unsigned int fmt)
{
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];

	be->fmt_mode = 0;
	dev_info(afe->dev, "mt8695_afe_tdm_in_set_fmt\n");
	/* set DAI format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
		be->fmt_mode |= fmt & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		dev_err(afe->dev, "invalid dai format %u\n", fmt);
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
	case SND_SOC_DAIFMT_NB_IF:
	case SND_SOC_DAIFMT_IB_NF:
	case SND_SOC_DAIFMT_IB_IF:
		be->fmt_mode |= fmt & SND_SOC_DAIFMT_INV_MASK;
		break;
	default:
		dev_err(afe->dev, "invalid dai format %u\n", fmt);
		return -EINVAL;
	}

	return 0;
}

static int mt8695_afe_tdm_in_trigger(struct snd_pcm_substream *substream, int cmd,
								struct snd_soc_dai *dai)
{
		struct snd_soc_pcm_runtime *rtd = substream->private_data;
		struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

		switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
				dev_err(afe->dev, "mt8695_afe_tdm_in_trigger start\n");
				/* enable tdm in */
				regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_PDN_M | TDMIN_PDN_B, 0);
				regmap_update_bits(afe->regmap, AFE_TDM_IN_CON1, TDM_IN_EN, TDM_IN_EN);
				return 0;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
				dev_err(afe->dev, "mt8695_afe_tdm_in_trigger stop\n");
				/* disable tdm in */
				regmap_update_bits(afe->regmap, AFE_DAC_CON0, TDM_IN_ON_MASK, TDM_IN_OFF);
				regmap_update_bits(afe->regmap, AFE_TDMIN_CLKDIV_CFG, TDMIN_PDN_M | TDMIN_PDN_B, 0x3);
				return 0;
		default:
				return -EINVAL;
		}

		return 0;
}

static int mt8695_afe_dmic_startup(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);

	dev_info(afe->dev, "%s\n", __func__);
	snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS,
							&constraints_channels_dmic);
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_MEMIF_UL3);

	return 0;
}

static void mt8695_afe_dmic_shutdown(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;

	dev_info(afe->dev, "%s\n", __func__);
	if (be->prepared[stream]) {
		/* disable tdmin */
		be->prepared[stream] = false;
	}
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_MEMIF_UL3);
}

static int mt8695_afe_dmic_prepare(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int rate = runtime->rate;
	const unsigned int channels = runtime->channels;
	const unsigned int stream = substream->stream;
	unsigned int val = 0;
	unsigned int reg, reg_t;

	dev_info(afe->dev, "%s\n", __func__);
	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}

	if (channels == 2) {
		if (afe->dmic_wire_mode == DMIC_ONE_WIRE) {
			/* one wired mode*/
			regmap_update_bits(afe->regmap, AFE_MUX_SEL_CFG, DMIC1_IN_L_SEL_MASK, DMIC1_IN_L_SEL_DMIC0_DAT);
		} else if (afe->dmic_wire_mode == DMIC_TWO_WIRE) {
			/* two wired mode*/
			regmap_update_bits(afe->regmap, AFE_MUX_SEL_CFG, DMIC1_IN_L_SEL_MASK, DMIC1_IN_L_SEL_DMIC0_DAT);
			regmap_update_bits(afe->regmap, AFE_MUX_SEL_CFG, DMIC1_IN_R_SEL_MASK,
									DMIC1_IN_R_SEL_DMIC0_DAT_R);
		}
		reg = AFE_DMIC_TOP_CON;
	} else if (channels == 4) {
		regmap_update_bits(afe->regmap, AFE_MUX_SEL_CFG, DMIC1_IN_L_SEL_MASK, DMIC1_IN_L_SEL_DMIC0_DAT);
		regmap_update_bits(afe->regmap, AFE_MUX_SEL_CFG, DMIC2_IN_L_SEL_MASK, DMIC2_IN_L_SEL_DMIC0_DAT_R);
		reg = AFE_DMIC_TOP_CON;
		reg_t = AFE_DMIC2_TOP_CON;
	}

	if (afe->dmic_wire_mode == DMIC_TWO_WIRE)
		val |= DMIC_TWO_WIRE_MODE;

		/* digital mic ch1/2 phase selection*/
		val |= (0 << DMIC_CH1_CK_PHASE_POS);
		val |= (4 << DMIC_CH2_CK_PHASE_POS);

	switch (rate) {
	case 8000:
		val |= (DMIC_VOID_MODE_8K | DMIC_DOMAIN_48K);
		break;
	case 16000:
		val |= (DMIC_VOID_MODE_16K | DMIC_DOMAIN_48K);
		break;
	case 32000:
		val |= (DMIC_VOID_MODE_32K | DMIC_DOMAIN_48K);
		break;
	case 48000:
		val |= (DMIC_VOID_MODE_48_44K | DMIC_DOMAIN_48K);
		break;
	case 44100:
		val |= (DMIC_VOID_MODE_48_44K | DMIC_DOMAIN_44K);
		break;
	}

	regmap_update_bits(afe->regmap, reg, ~(u32)(DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK), val);
	if (reg_t == AFE_DMIC2_TOP_CON)
		regmap_update_bits(afe->regmap, reg_t, ~(u32)(DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK), val);

	be->prepared[stream] = true;
	return 0;
}

static int mt8695_afe_dmic_trigger(struct snd_pcm_substream *substream, int cmd,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		dev_info(afe->dev, "mt8695_afe_tdm_in_trigger start\n");
		/* enable tdm in */
		if (substream->runtime->channels == 2) {
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC1_MASK, 0);
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, (DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK),
				(DMIC_TIMING_ON|DMIC_CIC_ON));
		} else {
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, DMIC_4CH_MODE_MASK, DMIC_4CH_MODE_ON);
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC1_MASK, 0);
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC2_MASK, 0);
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, (DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK),
				(DMIC_TIMING_ON|DMIC_CIC_ON));
		}
		return 0;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		dev_info(afe->dev, "mt8695_afe_tdm_in_trigger stop\n");
		/* disable tdm in */
		if (substream->runtime->channels == 2) {
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC1_MASK, (1 << PDN_DMIC1_POS));
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, (DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK),
				(DMIC_TIMING_OFF|DMIC_CIC_OFF));
		} else {
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, DMIC_4CH_MODE_MASK, DMIC_4CH_MODE_OFF);
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC1_MASK, (1 << PDN_DMIC1_POS));
			regmap_update_bits(afe->regmap, PWR2_TOP_CON, PDN_DMIC2_MASK, (1 << PDN_DMIC2_POS));
			regmap_update_bits(afe->regmap, AFE_DMIC_TOP_CON, (DMIC_TIMING_ON_MASK|DMIC_CIC_ON_MASK),
				(DMIC_TIMING_OFF|DMIC_CIC_OFF));
		}
		return 0;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mt8695_afe_btsco_startup(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_dai_get_drvdata(dai);

	dev_info(afe->dev, "%s\n", __func__);
	snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS,
							&constraints_channels_btsco);

	return 0;
}


static void mt8695_afe_btsco_shutdown(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int stream = substream->stream;

	dev_info(afe->dev, "%s\n", __func__);
	if (be->prepared[stream]) {
		/* disable pcmif */
		be->prepared[stream] = false;
	}
}

static int mt8695_afe_btsco_prepare(struct snd_pcm_substream *substream,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_pcm_runtime * const runtime = substream->runtime;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_be_dai_data *be = &afe->be_data[dai->id - MTK_AFE_BACKEND_BASE];
	const unsigned int rate = runtime->rate;
	const unsigned int channels = runtime->channels;
	const unsigned int stream = substream->stream;
	const int bit_width = snd_pcm_format_width(runtime->format);
	unsigned int val = 0;
	unsigned int val2 = 0;

	dev_info(afe->dev, "%s %s\n", __func__, dai->name);
	if (be->prepared[stream]) {
		dev_info(afe->dev, "%s prepared already\n", __func__);
		return 0;
	}

	/*0: i2s mode 2: mode A 3: mode B*/
	val |= PCM_FMT_I2S;

	/* sampling rate */
	switch (rate) {
	case 8000:
		val |= PCM_8K;
		val2 = 0x0;
		break;
	case 16000:
		val |= PCM_16K;
		val2 = 0x2;
		break;
	case 32000:
		val |= PCM_32K;
		val2 = 0x4;
		break;
	case 48000:
		val |= PCM_48K;
		val2 = 0x5;
		break;
	}

	/* bit width related */
	if (bit_width > 16)
		val |= PCM_BCK_64 | PCM_WLEN_32BIT;
	else
		val |= PCM_BCK_32 | PCM_WLEN_16BIT;

	/* channel mask*/
	if (channels == 1)
		val |= BT_MODE_SINGLE;
	else
		val |= BT_MODE_DUAL;

	/*default master mode for mt7668 and don't go asrc*/
	val |= PCM_MASTER_MODE | PCM_GO_ASYNC_FIFO | PCM_EXT_MD;

	regmap_update_bits(afe->regmap, PCM_INTF_CON1, ~(u32)(PCM_EN_MASK), val);
	regmap_update_bits(afe->regmap, ASMI_TIMING_CON0, ASMI0_MODE_MASK, val2 << ASMI0_MODE_POS);
	be->prepared[stream] = true;

	return 0;
}

static int mt8695_afe_btsco_trigger(struct snd_pcm_substream *substream, int cmd,
								struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		/* enable  control */
		regmap_update_bits(afe->regmap, PCM_INTF_CON1, PCM_EN_MASK, 1 << PCM_EN_POS);
		mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_PCMIF);

		return 0;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		/* disable control */
		regmap_update_bits(afe->regmap, PCM_INTF_CON1, PCM_EN_MASK, 0 << PCM_EN_POS);
		mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_PCMIF);
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct snd_kcontrol_new mt8695_afe_controls[] = {
	{
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "iec_channel_status_set",
	 .put = mt8695_iec_chstatus_set,
	 .info = mt8695_iec_chstatus_info,
	 .access = SNDRV_CTL_ELEM_ACCESS_WRITE,
	 },
	 {
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "hdmi_output_mode",
	 .info = mt8695_avsync_mode_info,
	 .put = mt8695_avsync_mode_set,
	 .get = mt8695_avsync_mode_get,
	 .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	 },
	 {
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "hdmi_buf_info",
	 .info = mt8695_buf_info,
	 .put = mt8695_buf_info_set,
	 .get = mt8695_buf_info_get,
	 .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	 },
	 {
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "hdmi_force_clk_switch",
	 .info = mt8695_hdmi_force_clk_info,
	 .put = mt8695_hdmi_force_clk_set,
	 .get = mt8695_hdmi_force_clk_get,
	 .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	 },
	 {
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "hdmi_audio_format",
	 .put = mt8695_hdmi_audio_format_set,
	 .info = mt8695_hdmi_audio_format_info,
	 .get = mt8695_hdmi_audio_format_get,
	 .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	 },
	 {
	 .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	 .name = "hdmi_dai_state",
	 .info = mt8695_hdmi_dai_state_info,
	 .get = mt8695_hdmi_dai_state_get,
	 .access = SNDRV_CTL_ELEM_ACCESS_READ,
	 },
	 SND_SOC_BYTES_EXT("hdmi_dai_timestamp", BYTES_OF_INT64,
		mt8695_afe_hdmi_dai_timestamp_get, NULL),
	 SND_SOC_BYTES_EXT("hdmi_hrt_timestamp", BYTES_OF_INT64,
		mt8695_afe_hdmi_timestamp_get, NULL),
};

/* FE DAIs */
static const struct snd_soc_dai_ops mt8695_afe_dai_ops = {
	.startup	= mt8695_afe_dais_startup,
	.shutdown	= mt8695_afe_dais_shutdown,
	.hw_params	= mt8695_afe_dais_hw_params,
	.hw_free	= mt8695_afe_dais_hw_free,
	.prepare	= mt8695_afe_dais_prepare,
	.trigger	= mt8695_afe_dais_trigger,
};

/* BE DAIs */
static const struct snd_soc_dai_ops mt8695_afe_hdmi_ops = {
	.startup	= mt8695_afe_hdmi_startup,
	.shutdown	= mt8695_afe_hdmi_shutdown,
	.prepare	= mt8695_afe_hdmi_prepare,
	.trigger	= mt8695_afe_hdmi_trigger,
	.set_fmt	= mt8695_afe_hdmi_set_fmt,
};

static const struct snd_soc_dai_ops mt8695_afe_spdif_ops = {
	.startup = mt8695_afe_spdif_startup,
	.shutdown = mt8695_afe_spdif_shutdown,
	.prepare = mt8695_afe_spdif_prepare,
	.trigger = mt8695_afe_spdif_trigger,
};

static const struct snd_soc_dai_ops mt8695_afe_i2s_ops = {
	.startup = mt8695_afe_i2s_startup,
	.shutdown = mt8695_afe_i2s_shutdown,
	.prepare = mt8695_afe_i2s_prepare,
	.trigger = mt8695_afe_i2s_trigger,
	.set_fmt = mt8695_afe_i2s_set_fmt,
};

static const struct snd_soc_dai_ops mt8695_afe_tdm_in_ops = {
	.startup	= mt8695_afe_tdm_in_startup,
	.shutdown	= mt8695_afe_tdm_in_shutdown,
	.prepare	= mt8695_afe_tdm_in_prepare,
	.trigger	= mt8695_afe_tdm_in_trigger,
	.set_fmt	= mt8695_afe_tdm_in_set_fmt,
};

static const struct snd_soc_dai_ops mt8695_afe_dmic_ops = {
	.startup	= mt8695_afe_dmic_startup,
	.shutdown	= mt8695_afe_dmic_shutdown,
	.prepare	= mt8695_afe_dmic_prepare,
	.trigger	= mt8695_afe_dmic_trigger,
};

static const struct snd_soc_dai_ops mt8695_afe_btsco_ops = {
	.startup = mt8695_afe_btsco_startup,
	.shutdown = mt8695_afe_btsco_shutdown,
	.prepare = mt8695_afe_btsco_prepare,
	.trigger = mt8695_afe_btsco_trigger,
};

static struct snd_soc_dai_driver mt8695_afe_pcm_dais[] = {
	/* FE DAIs */
	{
		.name = "HDMI",
		.id = MTK_AFE_MEMIF_HDMI,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.playback = {
			.stream_name = "HDMI",
			.channels_min = 2,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "SPDIF",
		.id = MTK_AFE_MEMIF_SPDIF,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.playback = {
			.stream_name = "SPDIF",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "DL1",
		.id = MTK_AFE_MEMIF_I2S,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.playback = {
			.stream_name = "DL1_Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "TDM_IN",
		.id = MTK_AFE_MEMIF_TDM_IN,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.capture = {
			.stream_name = "TDM_IN",
			.channels_min = 2,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S24_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "DMIC",
		.id = MTK_AFE_MEMIF_DMIC,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.capture = {
			.stream_name = "DMIC",
			.channels_min = 2,
			.channels_max = 4,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
				SNDRV_PCM_RATE_44100,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "DL2",
		.id = MTK_AFE_MEMIF_DL2,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.playback = {
			.stream_name = "DL2_Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},
	{
		.name = "UL5",
		.id = MTK_AFE_MEMIF_UL5,
		.suspend = mt8695_afe_dai_suspend,
		.resume = mt8695_afe_dai_resume,
		.capture = {
			.stream_name = "UL5_Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dai_ops,
	},

	/* BE DAIs */
	{
		.name = "8CH_I2S_OUT",
		.id = MTK_AFE_IO_HDMI,
		.playback = {
			.stream_name = "HDMI Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8695_afe_hdmi_ops,
	},
	{
		.name = "IEC",
		.id = MTK_AFE_IO_SPDIF,
		.playback = {
			.stream_name = "IEC Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
		},
		.ops = &mt8695_afe_spdif_ops,
	},
	{
		.name = "I2S",
		.id = MTK_AFE_IO_I2S,
		.playback = {
			.stream_name = "I2S Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |
				SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 |
				SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400 |
				SNDRV_PCM_RATE_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_i2s_ops,
	},
	{
		.name = "TDM_IN_IO",
		.id = MTK_AFE_IO_TDM_IN,
		.capture = {
			.stream_name = "TDM IN Capture",
			.channels_min = 2,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8695_afe_tdm_in_ops,
	},
	{
		.name = "DMIC_IO",
		.id = MTK_AFE_IO_DMIC,
		.capture = {
			.stream_name = "DMIC Capture",
			.channels_min = 2,
			.channels_max = 4,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
					SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
					SNDRV_PCM_RATE_44100,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_dmic_ops,
	},
	{
		.name = "BTSCO_IO",
		.id = MTK_AFE_IO_BTSCO,
		.playback = {
			.stream_name = "PCM Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
					SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
					SNDRV_PCM_RATE_44100,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.stream_name = "PCM Capture",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
					SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000 |
					SNDRV_PCM_RATE_44100,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.ops = &mt8695_afe_btsco_ops,
	},
};

static const struct snd_kcontrol_new mt8695_afe_o15_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I12_Switch", AFE_CONN15, 12, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o16_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I13_Switch", AFE_CONN16, 13, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o08_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I04_Switch", AFE_CONN8, 4, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o09_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I05_Switch", AFE_CONN9, 5, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o10_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I06_Switch", AFE_CONN10, 6, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o11_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I07_Switch", AFE_CONN11, 7, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o04_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I31_Switch", AFE_CONN4, 31, 1, 0),
};

static const struct snd_kcontrol_new mt8695_afe_o05_mix[] = {
	SOC_DAPM_SINGLE_AUTODISABLE("I32_Switch", AFE_CONN36, 0, 1, 0),
};


static const struct snd_soc_dapm_widget mt8695_afe_widgets[] = {
	/* inter-connections */
	SND_SOC_DAPM_MIXER("I12", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("I13", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("O15", SND_SOC_NOPM, 0, 0, mt8695_afe_o15_mix, ARRAY_SIZE(mt8695_afe_o15_mix)),
	SND_SOC_DAPM_MIXER("O16", SND_SOC_NOPM, 0, 0, mt8695_afe_o16_mix, ARRAY_SIZE(mt8695_afe_o16_mix)),
	SND_SOC_DAPM_MIXER("I31", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("I32", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("O04", SND_SOC_NOPM, 0, 0, mt8695_afe_o04_mix, ARRAY_SIZE(mt8695_afe_o04_mix)),
	SND_SOC_DAPM_MIXER("O05", SND_SOC_NOPM, 0, 0, mt8695_afe_o05_mix, ARRAY_SIZE(mt8695_afe_o05_mix)),
	SND_SOC_DAPM_MIXER("I06", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("I07", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("O10", SND_SOC_NOPM, 0, 0, mt8695_afe_o10_mix, ARRAY_SIZE(mt8695_afe_o10_mix)),
	SND_SOC_DAPM_MIXER("O11", SND_SOC_NOPM, 0, 0, mt8695_afe_o11_mix, ARRAY_SIZE(mt8695_afe_o11_mix)),
	SND_SOC_DAPM_MIXER("I04", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("I05", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("O08", SND_SOC_NOPM, 0, 0, mt8695_afe_o08_mix, ARRAY_SIZE(mt8695_afe_o08_mix)),
	SND_SOC_DAPM_MIXER("O09", SND_SOC_NOPM, 0, 0, mt8695_afe_o09_mix, ARRAY_SIZE(mt8695_afe_o09_mix)),
	SND_SOC_DAPM_OUTPUT("I2S Output"),
	SND_SOC_DAPM_INPUT("DMIC Input"),
	SND_SOC_DAPM_OUTPUT("BTSCO Output"),
	SND_SOC_DAPM_INPUT("BTSCO Input"),
};

static const struct snd_soc_dapm_route mt8695_afe_routes[] = {
	{"HDMI Playback", NULL, "HDMI"},
	{"IEC Playback", NULL, "SPDIF"},
	{"I12", NULL, "DL1_Playback"},
	{"I13", NULL, "DL1_Playback"},
	{"O15", "I12_Switch", "I12"},
	{"O16", "I13_Switch", "I13"},
	{"I2S Playback", NULL, "O15"},
	{"I2S Playback", NULL, "O16"},
	{"I2S Output", NULL, "I2S Playback"},
	/*tdmin ul*/
	{"TDM_IN", NULL, "TDM IN Capture"},
	/*dmic ul*/
	{"DMIC Capture", NULL, "DMIC Input"},
	{"I31", NULL, "DMIC Capture"},
	{"I32", NULL, "DMIC Capture"},
	{"O04", "I31_Switch", "I31"},
	{"O05", "I32_Switch", "I32"},
	{"DMIC", NULL, "O04"},
	{"DMIC", NULL, "O05"},
	/*bt sco dl*/
	{"BTSCO Output", NULL, "PCM Playback"},
	{"PCM Playback", NULL, "O10"},
	{"PCM Playback", NULL, "O11"},
	{"O10", "I06_Switch", "I06"},
	{"O11", "I07_Switch", "I07"},
	{"I06", NULL, "DL2_Playback"},
	{"I07", NULL, "DL2_Playback"},
	/*btsco ul*/
	{"PCM Capture", NULL, "BTSCO Input"},
	{"I04", NULL, "PCM Capture"},
	{"I05", NULL, "PCM Capture"},
	{"O08", "I04_Switch", "I04"},
	{"O09", "I05_Switch", "I05"},
	{"UL5_Capture", NULL, "O08"},
	{"UL5_Capture", NULL, "O09"},
};

static const struct snd_soc_component_driver mt8695_afe_dai_component = {
	.name = "mtk-afe-dai",
	.controls = mt8695_afe_controls,
	.num_controls = ARRAY_SIZE(mt8695_afe_controls),
	.dapm_widgets = mt8695_afe_widgets,
	.num_dapm_widgets = ARRAY_SIZE(mt8695_afe_widgets),
	.dapm_routes = mt8695_afe_routes,
	.num_dapm_routes = ARRAY_SIZE(mt8695_afe_routes),
};

static snd_pcm_uframes_t mt8695_afe_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	unsigned int hw_ptr, hw_bef_read_index, buf_addition;
	int ret;
	unsigned long flags;
	unsigned long buf_cmp;
	unsigned long long temp_read;

	spin_lock_irqsave(&memif->buf_info_lock, flags);
	ret = regmap_read(afe->regmap, memif->data->reg_ofs_cur, &hw_ptr);
	if (ret || (hw_ptr == 0)) {
		dev_info(afe->dev, "%s hw_ptr err ret = %d\n", __func__, ret);
		hw_ptr = memif->phys_buf_addr;
	}

	hw_ptr -= memif->phys_buf_addr;
	if (memif->avsync_mode) {
		temp_read = memif->buf_read;
		hw_bef_read_index = do_div(temp_read, memif->buffer_size);
		if (hw_ptr >= hw_bef_read_index)
			buf_addition = hw_ptr - hw_bef_read_index;
		else
			buf_addition = memif->buffer_size + hw_ptr - hw_bef_read_index;

		buf_cmp = memif->buf_read + buf_addition;
		if (memif->buf_read >= memif->buf_write) {
			dev_err(afe->dev, "%s abnormal case underrun happened!\n", __func__);
			dev_err(afe->dev, "%s memif->buf_read 0x%llx, memif->buf_write 0x%llx!\n", __func__,
				memif->buf_read, memif->buf_write);
			memif->buf_read = buf_addition;
			memif->buf_reset = true;
		} else {
			memif->buf_read += buf_addition;
		}
	}
	spin_unlock_irqrestore(&memif->buf_info_lock, flags);
	return bytes_to_frames(substream->runtime, hw_ptr);
}

int mt8695_afe_pcm_copy(struct snd_pcm_substream *substream, int channel, snd_pcm_uframes_t pos,
							void __user *buf, snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8695_afe_memif *memif = &afe->memif[rtd->cpu_dai->id];
	char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
	unsigned long flags;

	if (rtd->cpu_dai->id <= MTK_AFE_MEMIF_I2S) {
		if (copy_from_user(hwbuf, buf, frames_to_bytes(runtime, count)))
			return -EFAULT;
	} else if (rtd->cpu_dai->id >= MTK_AFE_MEMIF_TDM_IN) {
		if (copy_to_user(buf, hwbuf, frames_to_bytes(runtime, count)))
			return -EFAULT;
	}
	if (memif->avsync_mode) {
		spin_lock_irqsave(&memif->buf_info_lock, flags);
		memif->buf_write += frames_to_bytes(runtime, count);
		spin_unlock_irqrestore(&memif->buf_info_lock, flags);
	}
	ATRACE_COUNTER("afe_pcm_copy", count);
	return 0;
}

static const struct snd_pcm_ops mt8695_afe_pcm_ops = {
	.ioctl = snd_pcm_lib_ioctl,
	.pointer = mt8695_afe_pcm_pointer,
	.copy = mt8695_afe_pcm_copy,
};

static int mt8695_afe_pcm_probe(struct snd_soc_platform *platform)
{
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(platform);

	dev_info(afe->dev, "%s\n", __func__);
#if MT8695_FPGA_EARLY_PORTING
	regmap_update_bits(afe->regmap, FPGA_CFG2,
		FPFA_I2S_CK_DAT_SEL_MASK | (1 << 25),
		(0x1<<FPFA_I2SOUT1_M_CK_POS) | (0x1<<FPFA_I2SOUT1_M_DAT_POS));
#endif
	/* Power On APLL1 and APLL2*/
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_APLL);
	hdmi_output_from_i2s_set(afe, true);

	return 0;
}

int mt8695_afe_pcm_remove(struct snd_soc_platform *platform)
{
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(platform);

	/* Power Off APLL1 and APLL2*/
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_APLL);
	hdmi_output_from_i2s_set(afe, false);

	return 0;
}

static int mt8695_afe_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	struct mtk_afe *afe = snd_soc_platform_get_drvdata(rtd->platform);
	size_t size = afe->memif[rtd->cpu_dai->id].data->prealloc_size;
	struct snd_pcm_substream *substream;
	int stream;

	dev_info(afe->dev, "%s\n", __func__);
	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (substream) {
			struct snd_dma_buffer *buf = &substream->dma_buffer;

			buf->dev.type = SNDRV_DMA_TYPE_DEV;
			buf->dev.dev = card->dev;
			buf->private_data = NULL;
		}
	}

	if (size > 0)
		return snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, card->dev, size, size);

	return 0;
}

static void mt8695_afe_pcm_free(struct snd_pcm *pcm)
{
	snd_pcm_lib_preallocate_free_for_all(pcm);
}

static const struct snd_soc_platform_driver mt8695_afe_pcm_platform = {
	.probe = mt8695_afe_pcm_probe,
	.remove = mt8695_afe_pcm_remove,
	.pcm_new = mt8695_afe_pcm_new,
	.pcm_free = mt8695_afe_pcm_free,
	.ops = &mt8695_afe_pcm_ops,
};

static irqreturn_t mt8695_asys_irq_handler(int irq, void *dev_id)
{
	struct mtk_afe *afe = dev_id;
	unsigned int reg_value;
	unsigned int memif_status;
	int i, ret;

	ret = regmap_read(afe->regmap, ASYS_IRQ_STATUS, &reg_value);
	if (ret) {
		dev_info(afe->dev, "%s irq status err\n", __func__);
		reg_value = ASYS_IRQ_STATUS_BITS;
		goto err_irq;
	}

	ret = regmap_read(afe->regmap, AFE_DAC_CON0, &memif_status);
	if (ret) {
		dev_info(afe->dev, "%s memif status err\n", __func__);
		reg_value = ASYS_IRQ_STATUS_BITS;
		goto err_irq;
	}

	for (i = 0; i < MTK_AFE_MEMIF_NUM; i++) {
		struct mt8695_afe_memif *memif = &afe->memif[i];
		struct snd_pcm_substream *substream = memif->substream;

	if (!substream || !(reg_value & (1 << memif->data->irq_clr_shift)))
		continue;

	if (memif->data->enable_shift >= 0 && !((1 << memif->data->enable_shift) & memif_status))
	continue;

	snd_pcm_period_elapsed(substream);
	}

err_irq:
	/* clear irq */
	regmap_write(afe->regmap, ASYS_IRQ_CLR, reg_value & ASYS_IRQ_STATUS_BITS);

	return IRQ_HANDLED;
}

static irqreturn_t mt8695_afe_irq_handler(int irq, void *dev_id)
{
	struct mtk_afe *afe = dev_id;
	unsigned int reg_value, reg_value1;
	unsigned int memif_status;
	int i, ret;

	ret = regmap_read(afe->regmap, AFE_IRQ_STATUS, &reg_value);
	if (ret) {
		dev_info(afe->dev, "%s irq status err\n", __func__);
		reg_value = AFE_IRQ_STATUS_BITS;
		goto err_irq;
	}

	ret = regmap_read(afe->regmap, AFE_DAC_CON0, &memif_status);
	if (ret) {
		dev_info(afe->dev, "%s memif status err\n", __func__);
		reg_value = AFE_IRQ_STATUS_BITS;
		goto err_irq;
	}

	for (i = 0; i < MTK_AFE_MEMIF_NUM; i++) {
		struct mt8695_afe_memif *memif = &afe->memif[i];
		struct snd_pcm_substream *substream = memif->substream;

		if (!substream || !(reg_value & (1 << memif->data->irq_clr_shift)))
			continue;

		if (memif->data->enable_shift >= 0 &&
			!((1 << memif->data->enable_shift) & memif_status))
			continue;

		if (i == MTK_AFE_MEMIF_SPDIF) {
			regmap_read(afe->regmap, AFE_IEC_BURST_INFO, &reg_value1);
			if ((reg_value1 & IEC_BURST_NOT_READY) == IEC_BURST_NOT_READY) {
				dev_info(afe->dev, "%s iec ready status not 0!\n", __func__);
				return IRQ_HANDLED;
			}
			afe->iec.buf.buf_nsadr += afe->iec.period_bytes;
			if (afe->iec.buf.buf_nsadr >= afe->iec.buf.buf_ea)
				afe->iec.buf.buf_nsadr = afe->iec.buf.buf_sa;
			regmap_write(afe->regmap, AFE_IEC_NSADR, afe->iec.buf.buf_nsadr);
			regmap_update_bits(afe->regmap, AFE_IEC_BURST_INFO,
							IEC_BURST_NOT_READY, IEC_BURST_NOT_READY_MASK);
		}
		snd_pcm_period_elapsed(substream);
	}

err_irq:
	/* clear irq */
	regmap_write(afe->regmap, AFE_IRQ_CLR, reg_value & AFE_IRQ_STATUS_BITS);

	return IRQ_HANDLED;
}

static int mt8695_afe_pcm_dev_probe(struct platform_device *pdev)
{
	int ret, i;
	unsigned int irq_id, asys_irq_id;
	struct resource *res;
	struct mtk_afe *afe;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *node;

	afe = devm_kzalloc(&pdev->dev, sizeof(*afe), GFP_KERNEL);
	if (!afe)
		return -ENOMEM;

	spin_lock_init(&afe->afe_ctrl_lock);
	mutex_init(&afe->afe_clk_mutex);

	afe->dev = &pdev->dev;
	dev_info(afe->dev, "mt8695_afe_pcm_dev_probe\n");
	irq_id = platform_get_irq(pdev, 0);
	asys_irq_id = platform_get_irq(pdev, 1);
	dev_info(afe->dev, "platform_get_irq irq_id = %d,asys_irq_id = %d\n", irq_id, asys_irq_id);
	if (!(irq_id && asys_irq_id)) {
		dev_info(afe->dev, "np %s no irq!\n", np->name);
		return -ENXIO;
	}

	ret = devm_request_irq(afe->dev, irq_id, mt8695_afe_irq_handler, 0, "Afe_ISR_Handle", (void *)afe);
	if (ret) {
		dev_info(afe->dev, "could not request_afe_irq!\n");
		return ret;
	}

	ret = devm_request_irq(afe->dev, asys_irq_id, mt8695_asys_irq_handler, 0, "Asys_ISR_Handle", (void *)afe);
	if (ret) {
		dev_info(afe->dev, "could not request_ays_irq!\n");
		return ret;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	afe->base_addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(afe->base_addr))
		return PTR_ERR(afe->base_addr);

	afe->regmap = devm_regmap_init_mmio(&pdev->dev, afe->base_addr, &mt8695_afe_regmap_config);
	if (IS_ERR(afe->regmap))
		return PTR_ERR(afe->regmap);

	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-topckgen");
	if (!node) {
		dev_err(afe->dev, "%s (mediatek,mt8695-topckgen) fail\n", __func__);
		return -1;
	}
	afe->top_base_addr = of_iomap(node, 0);
	if (IS_ERR(afe->top_base_addr))
		return PTR_ERR(afe->top_base_addr);

	afe->topregmap = syscon_node_to_regmap(node);
	if (IS_ERR(afe->topregmap))
		return PTR_ERR(afe->topregmap);

	for (i = 0; i < MTK_AFE_MEMIF_NUM; i++) {
		afe->memif[i].data = &memif_data[i];
		spin_lock_init(&afe->memif[i].buf_info_lock);
	}

	ret = of_property_read_u32(afe->dev->of_node, "mediatek,dmic-wire-mode",
				&afe->dmic_wire_mode);
	dev_info(afe->dev, "%s debug dmic_wire_mode = %d\n", __func__, afe->dmic_wire_mode);
	if (ret) {
		dev_warn(afe->dev, "%s fail to read dmic-wire-mode in node %s\n",
			__func__, afe->dev->of_node->full_name);
		afe->dmic_wire_mode = DMIC_ONE_WIRE;
	} else if ((afe->dmic_wire_mode != DMIC_ONE_WIRE) &&
		afe->dmic_wire_mode != DMIC_TWO_WIRE) {
		afe->dmic_wire_mode = DMIC_ONE_WIRE;
	}
	/* force audio clock on*/
	afe->cached_sample_rate = 48000;
	afe->cached_channels = 2;
	afe->force_clk_on = true;
	afe->hdmi_force_clk_switch = 1;
	afe->hdmi_audio_format = 0;
	afe->cached_hdmi_audio_format = 0;
	afe->cached_hdmi_audio_sample_rate = 0;
	afe->cached_hdmi_audio_channels = 0;
	afe->cached_hdmi_audio_bit_depth = 0;
	afe->need_hdmi_toggle = true;
	/* init ck gen clock name */
	mt8695_clock_init(afe);
	platform_set_drvdata(pdev, afe);
	/* Power On APLL1 and APLL2 */
	mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL1, 1);
	mt8695_clock_power(afe, MTK_CLK_TOP_APLL_SEL, 1);
	/* Select fapll1 parent as apll1_ck and fapll2 parent as apll2_ck*/
	mt8695_set_fapll_ck_src(afe, MTK_CLK_TOP_APLL1);
	mt8695_set_fapll2_ck_src(afe, MTK_CLK_TOP_APLL2);
	/* Select a1sys parent as apll1_d4 and a2sys parent as apll2_d4 */
	mt8695_set_a1sys_clk_src(afe, MTK_CLK_TOP_APLL1_D4);
	mt8695_set_a2sys_clk_src(afe, MTK_CLK_TOP_APLL2_D4);
	/* Select audio intbus parent as syspll1_d4*/
	mt8695_set_intbus_clk_src(afe, MTK_CLK_TOP_SYSPLL1_D4);
	mt8695_clock_power(afe, MTK_CLK_TOP_AUD_INTBUS_SEL, 1);
	mt8695_init_debugfs(afe);
	mt8695_clock_power(afe, MTK_CLK_TOP_A1SYS_HP_SEL, 1);
	mt8695_afe_enable_top_cg(afe, MTK_AFE_CG_A1SYS_CK);
	regmap_write(afe->regmap, AUDIO_TOP_CON5, 0x18DF);
	ret = snd_soc_register_platform(&pdev->dev, &mt8695_afe_pcm_platform);
	if (ret)
		goto err_platform;

	ret = snd_soc_register_component(&pdev->dev, &mt8695_afe_dai_component, mt8695_afe_pcm_dais,
						ARRAY_SIZE(mt8695_afe_pcm_dais));

	if (ret)
		goto err_component;

	dev_dbg(&pdev->dev, "[3]MTK AFE driver initialized.\n");
	return 0;

err_component:
	snd_soc_unregister_platform(&pdev->dev);
err_platform:
	return ret;

}

static int mt8695_afe_pcm_dev_remove(struct platform_device *pdev)
{
	struct mtk_afe *afe;

	afe = platform_get_drvdata(pdev);
	mt8695_afe_disable_top_cg(afe, MTK_AFE_CG_A1SYS_CK);
	mt8695_clock_power(afe, MTK_CLK_TOP_A1SYS_HP_SEL, 0);
	mt8695_clock_power(afe, MTK_CLK_TOP_AUD_INTBUS_SEL, 1);
	mt8695_clock_power(afe, MTK_CLK_TOP_APLL_SEL, 0);
	mt8695_clock_power(afe, MTK_CLK_APMIXED_APLL1, 0);
	mt8695_uninit_debugfs(afe);
	snd_soc_unregister_component(&pdev->dev);
	snd_soc_unregister_platform(&pdev->dev);

	return 0;
}

static const struct of_device_id mt8695_afe_pcm_dt_match[] = {
	{ .compatible = "mediatek,mt8695-afe-pcm", },
	{ }
};
MODULE_DEVICE_TABLE(of, mt8695_afe_pcm_dt_match);

static struct platform_driver mt8695_afe_pcm_driver = {
	.driver = {
				.name = "mtk-afe-pcm",
				.of_match_table = mt8695_afe_pcm_dt_match,
	},
	.probe = mt8695_afe_pcm_dev_probe,
	.remove = mt8695_afe_pcm_dev_remove,
};

module_platform_driver(mt8695_afe_pcm_driver);

MODULE_DESCRIPTION("Mediatek ALSA SoC AFE platform driver");
MODULE_LICENSE("GPL v2");
