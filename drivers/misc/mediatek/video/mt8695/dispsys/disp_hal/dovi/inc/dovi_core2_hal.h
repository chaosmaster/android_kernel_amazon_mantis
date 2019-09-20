/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DOVI_CORE2_HAL_H_
#define _DOVI_CORE2_HAL_H_

#include "dovi_core2_hw.h"

int dovi_core2_hal_init(char *reg_base);
int dovi_core2_hal_status(void);
int dovi_core2_hal_reg_base_status(void);
int dovi_core2_hal_isr(void);
int dovi_core2_hal_set_enable(UINT32 enable);
UINT32 dovi_core2_hal_control_config(struct disp_hw_resolution *info);
int dovi_core2_hal_config_lut(UINT32 lut_addr);
int dovi_core2_hal_config_reg(uint32_t *p_core2_reg);
int dovi_core2_hal_show_black(bool enable);
int dovi_core2_set_black_pattern(bool enable);

#define IS_DISP_WITH_EXT(mode) ( \
	(mode == HDMI_VIDEO_720x480i_60Hz) || \
	(mode == HDMI_VIDEO_720x480p_60Hz) \
	)

extern uint32_t dovi_idk_test;
extern bool dovi_mute;
extern unsigned int g_force_dolby;
#endif				/*  */
