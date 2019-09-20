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

#ifndef _DOVI_CORE1_HAL_H_
#define _DOVI_CORE1_HAL_H_

#include "dovi_core1_hw.h"
#include "disp_hw_mgr.h"

extern int pp_is_enable;
extern int hdmi_delay;
extern unsigned int g_force_dolby;

int dovi_core1_hal_config_reg(uint32_t *p_core1_reg);
int dovi_core1_hal_config_lut(UINT32 lut_addr);
int dovi_core1_hal_set_enable(UINT32 enable);
int dovi_core1_hal_control_config(struct disp_hw_resolution *info);
int dovi_core1_hal_isr(void);
int dovi_core1_hal_init(char *reg_base);
int dovi_core1_hal_uninit(void);
int dovi_core1_hal_set_composer_mode(BOOL fgCompEL);

int dovi_core1_hal_status(void);
int dovi_core1_hal_reg_base_status(void);

int dovi_core1_hal_bypass_csc(uint32_t bypass);
int dovi_core1_hal_bypass_cvm(uint32_t bypass);
int dovi_core1_hal_set_out_fix_pattern_enable(bool enable,
	uint32_t pattern_i, uint32_t pattern_cp, uint32_t pattern_ct);
#endif				/*  */
