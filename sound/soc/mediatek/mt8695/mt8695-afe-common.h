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
#ifndef MT8695_AFE_COMMON_H_
#define MT8695_AFE_COMMON_H_

#include <linux/clk.h>
#include <linux/regmap.h>
#include <sound/asound.h>

#define MT8695_AUDIO_CCF	1

enum {
	MTK_AFE_MEMIF_HDMI,
	MTK_AFE_MEMIF_SPDIF,
	MTK_AFE_MEMIF_DL2,
	MTK_AFE_MEMIF_I2S,
	MTK_AFE_MEMIF_TDM_IN,
	MTK_AFE_MEMIF_DMIC,
	MTK_AFE_MEMIF_UL5,
	MTK_AFE_MEMIF_NUM,
	MTK_AFE_BACKEND_BASE = MTK_AFE_MEMIF_NUM,
	MTK_AFE_IO_MOD_PCM1 = MTK_AFE_BACKEND_BASE,
	MTK_AFE_IO_HDMI,
	MTK_AFE_IO_SPDIF,
	MTK_AFE_IO_I2S,
	MTK_AFE_IO_TDM_IN,
	MTK_AFE_IO_DMIC,
	MTK_AFE_IO_BTSCO,
	MTK_AFE_BACKEND_END,
	MTK_AFE_BACKEND_NUM = (MTK_AFE_BACKEND_END - MTK_AFE_BACKEND_BASE),
};

enum clk_type {
	MTK_CLK_APMIXED_APLL1,
	MTK_CLK_APMIXED_APLL2,
	MTK_CLK_TOP_SYS_26M,
	MTK_CLK_TOP_APLL1,
	MTK_CLK_TOP_APLL1_D2,
	MTK_CLK_TOP_APLL1_D4,
	MTK_CLK_TOP_APLL1_D8,
	MTK_CLK_TOP_APLL1_D16,
	MTK_CLK_TOP_APLL2,
	MTK_CLK_TOP_APLL2_D2,
	MTK_CLK_TOP_APLL2_D4,
	MTK_CLK_TOP_APLL2_D8,
	MTK_CLK_TOP_APLL2_D16,
	MTK_CLK_TOP_UNIVPLL2_D4,
	MTK_CLK_TOP_UNIVPLL2_D2,
	MTK_CLK_TOP_SYSPLL_D5,
	MTK_CLK_TOP_UNIVPLL_D3,
	MTK_CLK_TOP_SYSPLL_D2,
	MTK_CLK_TOP_TVDPLL,
	MTK_CLK_TOP_UNIVPLL_D2,
	MTK_CLK_TOP_SYSPLL1_D4,
	MTK_CLK_TOP_SYSPLL2_D4,
	MTK_CLK_TOP_UNIVPLL3_D2,
	MTK_CLK_TOP_UNIVPLL2_D8,
	MTK_CLK_TOP_SYSPLL3_D2,
	MTK_CLK_TOP_SYSPLL3_D4,
	MTK_CLK_TOP_I2SI1_SEL,
	MTK_CLK_TOP_APLL_DIV5,
	MTK_CLK_TOP_APLL_DIV_PDN5,
	MTK_CLK_TOP_I2SI2_SEL,
	MTK_CLK_TOP_APLL_DIV6,
	MTK_CLK_TOP_APLL_DIV_PDN6,
	MTK_CLK_TOP_I2SO1_SEL,
	MTK_CLK_TOP_APLL_DIV0,
	MTK_CLK_TOP_APLL_DIV_PDN0,
	MTK_CLK_TOP_I2SO2_SEL,
	MTK_CLK_TOP_TDMIN_SEL,
	MTK_CLK_TOP_APLL_DIV1,
	MTK_CLK_TOP_APLL_DIV_PDN1,
	MTK_CLK_TOP_APLL_SEL,
	MTK_CLK_TOP_APLL2_SEL,
	MTK_CLK_TOP_A1SYS_HP_SEL,
	MTK_CLK_TOP_A2SYS_HP_SEL,
	MTK_CLK_TOP_ASM_L_SEL,
	MTK_CLK_TOP_ASM_M_SEL,
	MTK_CLK_TOP_ASM_H_SEL,
	MTK_CLK_TOP_INTDIR_SEL,
	MTK_CLK_TOP_AUD_INTBUS_SEL,
	MTK_CLK_NUM
};

enum afe_irq_mode {
	MTK_AFE_IRQ_1 = 0,
	MTK_AFE_IRQ_2,
	MTK_AFE_IRQ_5, /* dedicated for HDMI */
	MTK_AFE_IRQ_6, /* dedicated for SPDIF */
	MTK_AFE_IRQ_NUM
};

enum asys_irq_mode { /* common irq */
	MTK_ASYS_IRQ_1 = 0,
	MTK_ASYS_IRQ_2,
	MTK_ASYS_IRQ_3,
	MTK_ASYS_IRQ_4,
	MTK_ASYS_IRQ_NUM
};

enum afe_top_clock_gate {
	MTK_AFE_CG_AFE,
	MTK_AFE_CG_LRCK_CNT,
	MTK_AFE_CG_SPDIFIN_TUNER_APLL_CK,
	MTK_AFE_CG_SPDIFIN_TUNER_DBG_CK,
	MTK_AFE_CG_HDMI_CK,
	MTK_AFE_CG_SPDIF_CK,
	MTK_AFE_CG_APLL,
	MTK_AFE_CG_TML,
	MTK_AFE_CG_I2SIN1_CK,
	MTK_AFE_CG_I2SIN2_CK,
	MTK_AFE_CG_I2SIN3_CK,
	MTK_AFE_CG_I2SIN4_CK,
	MTK_AFE_CG_I2SO1_CK,
	MTK_AFE_CG_I2SO2_CK,
	MTK_AFE_CG_I2SO3_CK,
	MTK_AFE_CG_I2SO4_CK,
	MTK_AFE_CG_I2SO5_CK,
	MTK_AFE_CG_I2SO6_CK,
	MTK_AFE_CG_ASRCI1_CK,
	MTK_AFE_CG_ASRCI2_CK,
	MTK_AFE_CG_MULTI_IN_CK,
	MTK_AFE_CG_INTDIR_CK,
	MTK_AFE_CG_A1SYS_CK,
	MTK_AFE_CG_A2SYS_CK,
	MTK_AFE_CG_AFE_CONN,
	MTK_AFE_CG_PCMIF,
	MTK_AFE_CG_ASRCI3_CK,
	MTK_AFE_CG_ASRCI4_CK,
	MTK_AFE_CG_MEMIF_UL1,
	MTK_AFE_CG_MEMIF_UL2,
	MTK_AFE_CG_MEMIF_UL3,
	MTK_AFE_CG_MEMIF_UL4,
	MTK_AFE_CG_MEMIF_UL5,
	MTK_AFE_CG_MEMIF_DL1,
	MTK_AFE_CG_MEMIF_DL2,
	MTK_AFE_CG_MEMIF_DL6,
	MTK_AFE_CG_MEMIF_DLMCH,
	MTK_AFE_CG_NUM
};

enum afe_hdmi_inter {
	MTK_AFE_HDMI_CONN_I20,
	MTK_AFE_HDMI_CONN_I21,
	MTK_AFE_HDMI_CONN_I22,
	MTK_AFE_HDMI_CONN_I23,
	MTK_AFE_HDMI_CONN_I24,
	MTK_AFE_HDMI_CONN_I25,
	MTK_AFE_HDMI_CONN_I26,
	MTK_AFE_HDMI_CONN_I27
};

enum afe_sampling_rate {
	FS_8000HZ		= 0x0,
	FS_12000HZ		= 0x1,
	FS_16000HZ		= 0x2,
	FS_24000HZ		= 0x3,
	FS_32000HZ		= 0x4,
	FS_48000HZ		= 0x5,
	FS_96000HZ		= 0x6,
	FS_192000HZ		= 0x7,
	FS_384000HZ		= 0x8,
	FS_I2S1			= 0x9,
	FS_I2S2			= 0xA,
	FS_I2S3			= 0xB,
	FS_I2S4			= 0xC,
	FS_I2S5			= 0xD,
	FS_I2S6			= 0xE,
	FS_7350HZ		= 0x10,
	FS_11025HZ		= 0x11,
	FS_14700HZ		= 0x12,
	FS_22050HZ		= 0x13,
	FS_29400HZ		= 0x14,
	FS_44100HZ		= 0x15,
	FS_88200HZ		= 0x16,
	FS_176400HZ		= 0x17,
	FS_352800HZ		= 0x18,
};


enum afe_i2s_format {
	FMT_32CYCLE_16BIT_I2S,
	FMT_32CYCLE_16BIT_LJ, /* if you want FMT_32CYCLE_16BIT_RJ, please use this too, because they are same */
	FMT_64CYCLE_16BIT_I2S,
	FMT_64CYCLE_16BIT_LJ,
	FMT_64CYCLE_16BIT_RJ,
	FMT_64CYCLE_32BIT_I2S,
	FMT_64CYCLE_32BIT_LJ, /* if you want FMT_64CYCLE_32BIT_RJ, please use this too, because they are same */
	FMT_64CYCLE_24BIT_RJ,
	FMT_64CYCLE_24BIT_I2S
};


enum afe_i2s_out_dsd_use {
	I2S_OUT_DSD_USE_NORMAL,
	I2S_OUT_DSD_USE_SONY_IP
};

struct mt8695_iec_ch_status {
	unsigned int chl_stat0;
	unsigned int chl_stat1;
	unsigned int chr_stat0;
	unsigned int chr_stat1;
};

struct mt8695_iec_buf_ctrl {
	unsigned int buf_sa;
	unsigned int buf_ea;
	unsigned int buf_nsadr;
};

struct mt8695_iec_config {
	unsigned int bit_width;
	unsigned int nsnum;
	unsigned int period_bytes;
	unsigned int force_update_size;
	struct mt8695_iec_buf_ctrl buf;
	struct mt8695_iec_ch_status ch_status;
};

struct mt8695_i2s_out_config {
	int use_asrc;
	int dsd_mode;
	enum afe_i2s_out_dsd_use dsd_use;
	int couple_mode;
	int one_heart_mode;	/* 0: 2 channel mode, 1: multi channel mode */
	int slave;
	enum afe_i2s_format fmt;
	int mclk;
	enum afe_sampling_rate mode;
};

struct mt8695_afe_memif_data {
	int id;
	const char *name;
	int reg_ofs_base;
	int reg_ofs_end;
	int reg_ofs_cur;
	int fs_shift;
	int hd_shift;
	int mono_shift;
	int lsb_mode_shift;
	int dsd_width_shift;
	int enable_shift;
	int irq_reg_cnt;
	int irq_cnt_shift;
	int irq_mode;
	int irq_fs_reg;
	int irq_fs_shift;
	int asys_irq_fs_shift;
	int irq_clr_shift;
	int max_sram_size;
	int sram_offset;
	int pbuf_reg;
	int pbuf_shift;
	int conn_format_mask;
	int prealloc_size;
	int direction; /* 0: out 1: in*/
	unsigned long buffer_align_bytes;
};

struct mt8695_afe_memif {
	bool use_sram;
	bool prepared;
	bool avsync_mode;
	bool buf_reset; /* for avsync*/
	unsigned int phys_buf_addr;
	uint32_t buffer_size;
	struct snd_pcm_substream *substream;
	const struct mt8695_afe_memif_data *data;
	unsigned long long buf_read;	/* for avsync unit is bytes */
	unsigned long long buf_write;	/* for avsync unit is bytes */
	unsigned int buf_frame_size;
	spinlock_t buf_info_lock;
};

struct mt8695_afe_be_dai_data {
	bool prepared[SNDRV_PCM_STREAM_LAST + 1];
	unsigned int fmt_mode;
};

struct mtk_afe {
	void __iomem *base_addr;
	void __iomem *top_base_addr;
	struct device *dev;
	struct regmap *regmap;
	struct regmap *topregmap;
	struct mt8695_iec_config iec;
	struct mt8695_afe_memif memif[MTK_AFE_MEMIF_NUM];
	int top_cg_ref_cnt[MTK_AFE_CG_NUM];
	struct mt8695_afe_be_dai_data be_data[MTK_AFE_BACKEND_NUM];
	struct clk *clocks[MTK_CLK_NUM];
	bool suspended;
	bool is_apll_related_clks_on;
	bool force_clk_on;
	uint32_t hdmi_audio_format;
	uint32_t cached_hdmi_audio_format;
	uint32_t cached_hdmi_audio_sample_rate;
	uint32_t cached_hdmi_audio_channels;
	uint32_t cached_hdmi_audio_bit_depth;
	bool need_hdmi_toggle;
	int afe_on_ref_cnt;
	int i2s_out_on_ref_cnt;
	int irq_mode_ref_cnt[MTK_AFE_IRQ_NUM];
	int asys_irq_mode_ref_cnt[MTK_ASYS_IRQ_NUM];
	int asys_timing_ref_cnt;
	uint32_t hdmi_force_clk_switch;
	uint32_t cached_sample_rate;
	uint32_t cached_channels;
	uint32_t dmic_wire_mode;
	/* locks */
	spinlock_t afe_ctrl_lock;
	struct mutex afe_clk_mutex;
};

#endif
