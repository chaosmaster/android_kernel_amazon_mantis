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
#define LOG_TAG "DOVI_CORE3_HAL"

#include "disp_type.h"
#include "dovi_log.h"
#include "dovi_table.h"

#include "disp_hw_mgr.h"

#include "dovi_core3_hal.h"
#include "dovi_core3_hw.h"
#include "disp_hw_log.h"

union core3_rap_reg_u *p_core3_rap_hw_reg;
union core3_cfg_reg_u *p_core3_cfg_hw_reg;
union core3_dpm_reg_u *p_core3_dpm_hw_reg;

union core3_rap_reg_u core3_rap_sw_reg;
union core3_cfg_reg_u core3_cfg_sw_reg;
union core3_dpm_reg_u core3_dpm_sw_reg;

uint8_t core3_rap_update_mode[CORE3_RAP_REG_NUM];
uint8_t core3_cfg_update_mode[CORE3_CFG_REG_NUM];
uint8_t core3_dpm_update_mode[CORE3_DPM_REG_NUM];

char *core3_reg_base;
int core3_ctrl_sw_reset;

BOOL _fgDoviSwPathEnable;
BOOL _fgDoviHwPathEnable;

int core3_hal_enable;
int core3_hal_pre_enable;
int core3_hal_init;
bool dovi_core3_isr_md_enable = 1;
bool core3_bypass_dither;
uint8_t core3_version = 4;
bool core3_adjust_hstart;

struct disp_hw_resolution core3_res;

#define GET_CORE3_RAP_PTR(reg, mode)     \
do {                                      \
	reg = &core3_rap_sw_reg;         \
	mode = core3_rap_update_mode;         \
} while (0)

#define GET_CORE3_CFG_PTR(reg, mode)     \
do {                                      \
	reg = &core3_cfg_sw_reg;         \
	mode = core3_cfg_update_mode;         \
} while (0)

#define GET_CORE3_DPM_PTR(reg, mode)     \
do {                                      \
	reg = &core3_dpm_sw_reg;         \
	mode = core3_dpm_update_mode;         \
} while (0)

int dovi_core3_hal_init(char *reg_base)
{
	if (core3_hal_init) {
		dovi_info("core3 hal already inited\n");
		return 0;
	}

	/* dovi_func(); */

	core3_reg_base = reg_base;

	p_core3_dpm_hw_reg = (union core3_dpm_reg_u *)(reg_base + CORE3_DPM_REG_BASE);
	p_core3_cfg_hw_reg = (union core3_cfg_reg_u *)(reg_base + CORE3_CFG_REG_BASE);
	p_core3_rap_hw_reg = (union core3_rap_reg_u *)(reg_base + CORE3_RAP_REG_BASE);

	/* dovi_core3_hal_reg_base_status(); */

	core3_hal_init = 1;
	return 1;
}

int dovi_core3_hal_control_config(struct disp_hw_resolution *info)
{
	uint16_t width;
	uint16_t height;

	uint16_t x_start = 0x081;
	uint16_t x_end;
	uint16_t y_start = 0x2a;
	uint16_t y_end;

	core3_res = *info;
	width = info->width;
	height = info->height;

	dovi_get_core3_x_start(width, height, &x_start, &y_start);
	if ((core3_version == 4) && (core3_adjust_hstart))
		x_start = x_start - 0x25;
	x_end = x_start + width - 1;
	y_end = y_start + height - 1;


	dovi_info("core3 ctrl config %d x(0x%x 0x%x) y(0x%x 0x%x)\n",
		    core3_res.res_mode, x_start, x_end, y_start, y_end);

	if (g_force_dolby > 0)
		core3_ctrl_sw_reset = 0;
	else
		core3_ctrl_sw_reset = 1;

	if ((core3_select_external_timing) || (dovi_idk_test)) {
		/* use video timing */
		/* 0x08 */
		core3_rap_sw_reg.fld.Hsync_Sel_Ext = 1;
		core3_rap_sw_reg.fld.Vsync_Set_Ext = 1;
		core3_rap_sw_reg.fld.De_Sel_Ext = 1;

		/* 0x1C = 0x63 */
		core3_rap_sw_reg.fld.Hsync_De_Swap = 0;
		core3_rap_sw_reg.fld.Vsync_Sel_Vde = 0;

		core3_rap_sw_reg.fld.Vsync_Pol_N = 0;
		core3_rap_sw_reg.fld.De_Pol_N = 0;

		/* 0x20 */
		core3_rap_sw_reg.fld.Hsync_Width = 0;
		core3_rap_sw_reg.fld.Vsync_Width = 0;

		/* 0x24 */
		core3_rap_sw_reg.fld.X_Active_Start = 0;
		core3_rap_sw_reg.fld.X_Active_End = 0;

		/* 0x28 */
		core3_rap_sw_reg.fld.Y_Active_Start = 0;
		core3_rap_sw_reg.fld.Y_Active_End = 0;

		/* 0x2C */
		core3_rap_sw_reg.fld.Hsync_Delay = 0;
		core3_rap_sw_reg.fld.Vsync_Delay = 0;
	} else {
		/* 0x08 */
		core3_rap_sw_reg.fld.Hsync_Sel_Ext = 0;
		core3_rap_sw_reg.fld.Vsync_Set_Ext = 0;
		core3_rap_sw_reg.fld.De_Sel_Ext = 0;

		/* 0x1C = 0x63 */
		core3_rap_sw_reg.fld.Hsync_De_Swap = 1;
		core3_rap_sw_reg.fld.Vsync_Sel_Vde = 0;

		core3_rap_sw_reg.fld.Vsync_Pol_N = 0;
		core3_rap_sw_reg.fld.De_Pol_N = 1;

		/* 0x20 */
		core3_rap_sw_reg.fld.Hsync_Width = 0xA;
		core3_rap_sw_reg.fld.Vsync_Width = 0xA;

		dovi_info("111 core3 ctrl config %d x(0x%x 0x%x) y(0x%x 0x%x)\n",
				core3_res.res_mode, x_start, x_end, y_start, y_end);
		/* 0x24 */
		core3_rap_sw_reg.fld.X_Active_Start = x_start;
		core3_rap_sw_reg.fld.X_Active_End = x_end;

		/* 0x28 */
		core3_rap_sw_reg.fld.Y_Active_Start = y_start;
		core3_rap_sw_reg.fld.Y_Active_End = y_end;
		dovi_info("222 core3 ctrl config %d x(0x%x 0x%x) y(0x%x 0x%x)\n",
				core3_res.res_mode, x_start, x_end, y_start, y_end);
		/* 0x2C */
		core3_rap_sw_reg.fld.Hsync_Delay = 0x17;
		core3_rap_sw_reg.fld.Vsync_Delay = 0x1;
	}


	core3_rap_update_mode[0x08 / 4] = 1;
	core3_rap_update_mode[0x1C / 4] = 1;
	core3_rap_update_mode[0x20 / 4] = 1;
	core3_rap_update_mode[0x24 / 4] = 1;
	core3_rap_update_mode[0x28 / 4] = 1;
	core3_rap_update_mode[0x2C / 4] = 1;

	return 1;
}

int dovi_core3_hal_config_reg(uint32_t *p_core3_reg)
{
	int idx = 0;

	for (idx = 0; idx < CORE3_DPM_REG_NUM; idx++) {
		core3_dpm_sw_reg.reg[idx] = p_core3_reg[idx];
		core3_dpm_update_mode[idx] = 1;
	}
	if (core3_bypass_dither) {
		idx = 0x3E0/4;
		core3_dpm_sw_reg.reg[idx] |= (1 << 5);
		core3_dpm_update_mode[idx] = 1;
	}
	return 1;
}

int dovi_core3_hal_dither_bypass(uint32_t bypass)
{
	int idx = 0x3E0/4;

	if (bypass) {
		core3_dpm_sw_reg.reg[idx] |= (1 << 5);
		core3_bypass_dither = true;
	} else {
		core3_dpm_sw_reg.reg[idx] &= !(1 << 5);
		core3_bypass_dither = false;
	}

	core3_dpm_update_mode[idx] = 1;

	return 1;
}

int dovi_core3_hal_set_out_mode(enum dovi_core3_out_mode mode)
{
	int idx = 0x004/4;

	core3_dpm_sw_reg.reg[idx] = mode;

	core3_dpm_update_mode[idx] = 1;

	return 1;
}

int dovi_core3_hal_set_out_fix_pattern_enable(bool enable)
{
	int idx = 0x06C/4;

	core3_rap_sw_reg.fld.fgOut_Fix_Pattern = enable;

	core3_rap_update_mode[idx] = 1;

	return 1;
}

int dovi_core3_hal_isr(void)
{
	int idx = 0;

	if (core3_hal_enable) {
		if (core3_ctrl_sw_reset) {
			p_core3_rap_hw_reg->reg[idx] = CORE3_RESET_ALL;
			p_core3_rap_hw_reg->reg[idx] = CORE3_RESET_CLEAR;

			core3_ctrl_sw_reset = 0;
		}

		for (idx = 0; idx < CORE3_RAP_REG_NUM; idx++) {
			if (core3_rap_update_mode[idx] == 1) {
				p_core3_rap_hw_reg->reg[idx] = core3_rap_sw_reg.reg[idx];
				core3_rap_update_mode[idx] = 0;
			}
		}

		/* core1 trigger */
		WriteREG(core3_reg_base + CORE3_PROG_START, 0x1);

		for (idx = 0; idx < CORE3_DPM_REG_NUM; idx++) {
			if (core3_dpm_update_mode[idx] == 1) {
				p_core3_dpm_hw_reg->reg[idx] = core3_dpm_sw_reg.reg[idx];
				core3_dpm_update_mode[idx] = 0;
			}
		}

		/* core1 trigger */
		WriteREG(core3_reg_base + CORE3_PROG_FINISH, 0x1);
	}

	return 1;
}

int dovi_core3_hal_set_enable(UINT32 enable)
{
	/* disp fmt color space */

	/* path switch */
	core3_hal_enable = enable;

	return 1;
}

int dovi_core3_hal_is_support(void)
{
	return (p_core3_cfg_hw_reg && !p_core3_cfg_hw_reg->fld.Efuse_disable);
}

int dovi_core3_hal_reg_base_status(void)
{
	dovi_default("core3_reg_base		0x%p\n", core3_reg_base);
	dovi_default("p_core3_cfg_hw_reg 0x%p\n", p_core3_cfg_hw_reg);
	dovi_default("p_core3_dpm_hw_reg 0x%p\n", p_core3_dpm_hw_reg);
	dovi_default("p_core3_rap_hw_reg 0x%p\n", p_core3_rap_hw_reg);

	return 1;
}

int dovi_core3_hal_status(void)
{
	dovi_default("core3 support %d\n", dovi_core3_hal_is_support());

	if (core3_reg_base)
		dovi_default("core3 efuse[0x%x] 0x%p = 0x%x\n",
			    CORE3_EFUSE_REG,
			    core3_reg_base + CORE3_EFUSE_REG,
			    ReadREG(core3_reg_base + CORE3_EFUSE_REG));

	dovi_core3_hal_reg_base_status();

	return 1;
}

void dovi_core3_hal_adjust_hstart(bool adjust)
{
	DISP_LOG_DEBUG("dovi_core3_hal_adjust_hstart %d\n", adjust);

	/* for core3 set timing */
	core3_adjust_hstart = adjust;
}

