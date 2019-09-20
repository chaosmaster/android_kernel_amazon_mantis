/*
 * mt8695-afe-util.h  --  Mediatek audio utility
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
#ifndef _MT8695_AFE_UTILITY_H_
#define _MT8695_AFE_UTILITY_H_

#include "mt8695-afe-common.h"

int mt8695_clock_init(struct mtk_afe *afe);

int mt8695_clock_power(struct mtk_afe *afe, enum clk_type clk, int en);

int mt8695_set_fapll_ck_src(struct mtk_afe *afe, int parent);

int mt8695_set_fapll2_ck_src(struct mtk_afe *afe, int parent);

int mt8695_set_a1sys_clk_src(struct mtk_afe *afe, int parent);

int mt8695_set_a2sys_clk_src(struct mtk_afe *afe, int parent);

int mt8695_set_intbus_clk_src(struct mtk_afe *afe, int parent);

int mt8695_afe_enable_top_cg(struct mtk_afe *afe, unsigned int cg_type);

int mt8695_afe_disable_top_cg(struct mtk_afe *afe, unsigned cg_type);

int mt8695_afe_enable_i2so1_mclk(struct mtk_afe *afe);

int mt8695_afe_disable_i2so1_mclk(struct mtk_afe *afe);

int mt8695_afe_enable_i2so2_mclk(struct mtk_afe *afe);

int mt8695_afe_disable_i2so2_mclk(struct mtk_afe *afe);


int mt8695_afe_enable_afe_on(struct mtk_afe *afe);

int mt8695_afe_disable_afe_on(struct mtk_afe *afe);

int mt8695_asys_timing_on(struct mtk_afe *afe);

int mt8695_asys_timing_off(struct mtk_afe *afe);

int mt8695_i2so1_set_mclk(struct mtk_afe *afe, unsigned int rate);

int mt8695_i2so2_set_mclk(struct mtk_afe *afe, unsigned int rate);

int mt8695_hdmi_set_clock(struct mtk_afe *afe, unsigned int rate);

int mt8695_tdmin_set_clock(struct mtk_afe *afe, unsigned int rate, unsigned int channels, unsigned int bit_width);

int mt8695_iec_set_clock(struct mtk_afe *afe, unsigned int rate);

int mt8695_iec_set_config(struct mtk_afe *afe);

enum afe_sampling_rate rate_convert_enum(unsigned int rate);

int mt8695_i2so_set_config(struct mtk_afe *afe, struct mt8695_i2s_out_config *config);

extern u64 mtk_timer_get_cnt(u8 timer);

#endif
