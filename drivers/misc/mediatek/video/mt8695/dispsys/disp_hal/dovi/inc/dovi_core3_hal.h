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


#ifndef _DOVI_CORE3_HAL_H_
#define _DOVI_CORE3_HAL_H_

#include "dovi_core3_hw.h"
#include "disp_hw_mgr.h"

enum dovi_core3_out_mode {
	DOVI_CORE3_IPT444_BYPASS,
	DOVI_CORE3_IPT422_TUNNELED,
	DOVI_CORE3_HDR10,
	DOVI_CORE3_SDR10,
	DOVI_CORE3_SDR,
	DOVI_CORE3_YCC422_TUNNELED
};

int dovi_core3_hal_init(char *reg_base);
int dovi_core3_hal_is_support(void);
int dovi_core3_hal_isr(void);
int dovi_core3_hal_control_config(struct disp_hw_resolution *info);
int dovi_core3_hal_status(void);
int dovi_core3_hal_reg_base_status(void);
int dovi_core3_hal_config_reg(uint32_t *p_core3_reg);
int dovi_core3_hal_set_enable(UINT32 enable);
int dovi_core3_hal_dither_bypass(uint32_t bypass);
int dovi_core3_hal_set_out_mode(enum dovi_core3_out_mode mode);
int dovi_core3_hal_set_out_fix_pattern_enable(bool enable);
void dovi_core3_hal_adjust_hstart(bool adjust);

extern uint32_t dovi_idk_test;
extern bool core3_bypass_dither;
extern unsigned int g_force_dolby;

#endif
