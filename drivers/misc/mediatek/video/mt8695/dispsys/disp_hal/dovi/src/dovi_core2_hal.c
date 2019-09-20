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

/* ----------------------------------------------------------------------------- */
/* Include files */
/* ----------------------------------------------------------------------------- */
#include "dovi_type.h"
#include "dovi_log.h"

#include "disp_hw_mgr.h"


#include "dovi_core2_hw.h"
#include "dovi_core2_hal.h"

#include "dovi_table.h"

/* ----------------------------------------------------------------------------- */
/* Macro definitions */
/* ----------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------- */
/* Static variables */
/* ----------------------------------------------------------------------------- */
/* software register */
union core2_dpm_reg_u *p_core2_dpm_hw_reg;
union core2_cfg_reg_u *p_core2_cfg_hw_reg;
union core2_rap_reg_u *p_core2_rap_hw_reg;

union core2_dpm_reg_u core2_dpm_sw_reg;
union core2_cfg_reg_u core2_cfg_sw_reg;
union core2_rap_reg_u core2_rap_sw_reg;

uint8_t core2_dpm_update_mode[CORE2_DPM_REG_NUM];
uint8_t core2_cfg_update_mode[CORE2_CFG_REG_NUM];
uint8_t core2_rap_update_mode[CORE2_RAP_REG_NUM];

char *core2_reg_base;

UINT16 m_u2path = 0xff;
struct disp_hw_resolution  core2_out_res;
int core2_ctrl_sw_reset;
int core2_CTRL_enable;
UINT32 core2_hal_init;
int core2_enable;
int core2_pre_enable;

uint32_t core2_reset_all;

/* ----------------------------------------------------------------------------- */
/* Inter-file functions */
/* ----------------------------------------------------------------------------- */
int dovi_core2_hal_init(char *reg_base)
{
	if (core2_hal_init) {
		dovi_info("core2 hal already inited\n");
		return 0;
	}

	/* dovi_func(); */

	core2_reset_all = CORE2_RESET_ALL;

	core2_reg_base = reg_base;

	p_core2_dpm_hw_reg = (union core2_dpm_reg_u *)(reg_base + CORE2_DPM_REG_BASE);
	p_core2_cfg_hw_reg = (union core2_cfg_reg_u *)(reg_base + CORE2_CFG_REG_BASE);
	p_core2_rap_hw_reg = (union core2_rap_reg_u *)(reg_base + CORE2_RAP_REG_BASE);

	/* dovi_core2_hal_reg_base_status(); */

	core2_hal_init = 1;
	return 1;
}

UINT32 dovi_core2_hal_is_support(void)
{
	return (p_core2_cfg_hw_reg && !p_core2_cfg_hw_reg->fld.fgEfuse);
}


/* config 0x41c00 dovi core2 wrap control register*/
UINT32 dovi_core2_hal_control_config(struct disp_hw_resolution *info)
{
	UINT32 out_res = info->res_mode;

	if (out_res >= HDMI_VIDEO_RESOLUTION_NUM) {
		dovi_error("resolution set error!!!\n");
		return 0;
	}

	dovi_info("CORE2_RESET_ALL 0x%X 0x%X\n",
	CORE2_RESET_ALL, core2_reset_all);

	core2_out_res = *info;

	if (g_force_dolby > 0)
		core2_ctrl_sw_reset = 0;
	else
		core2_ctrl_sw_reset = 1;

	/* 0x04 */
	core2_rap_sw_reg.fld.fgDmaTrigger = 1;
	core2_rap_sw_reg.fld.fgDmaVsTrigger = 1;

	/* 0x08 */
	core2_rap_sw_reg.fld.fgDmaVsPol = 1;
	core2_rap_sw_reg.fld.fgEnDma = 1;
	core2_rap_sw_reg.fld.fgEnDmaDRAM = 1;

	if (true) {
		/* 0xA408 = 0xC */
		core2_rap_sw_reg.fld.fgHsync_Sel_Ext = 0;
		core2_rap_sw_reg.fld.fgVsync_Set_Ext = 0;
		core2_rap_sw_reg.fld.fgDe_Sel_Ext = 0;

		/* 0xA41C = 0x63 */
		core2_rap_sw_reg.fld.fgHsync_De_Swap = 1;
		core2_rap_sw_reg.fld.fgVsync_Sel_Vde = 1;

		core2_rap_sw_reg.fld.fgVsync_Pol_N = 1;
		core2_rap_sw_reg.fld.fgDe_Pol_N = 1;

		/* 0xA44C = 0x1 */
		core2_rap_sw_reg.fld.u4Use_Tg_New = 1;

		/* 0xA450 = 0x020D035A */
		core2_rap_sw_reg.fld.u4Tg_New_H_Total = dobly_resolution_table[out_res].htotal;
		core2_rap_sw_reg.fld.u4Tg_New_V_Total = dobly_resolution_table[out_res].vtotal;

		/* 0xA454 = 0x01E002D0 */
		core2_rap_sw_reg.fld.u4Tg_New_H_Active = dobly_resolution_table[out_res].width;
		core2_rap_sw_reg.fld.u4Tg_New_V_Active = dobly_resolution_table[out_res].height;

		/* 0xA458 = 0x00040004 */
		core2_rap_sw_reg.fld.u4Tg_New_H_Width = 4;
		core2_rap_sw_reg.fld.u4Tg_New_V_Width = 4;

		/* 0xA45c = 0x00070007 */
		core2_rap_sw_reg.fld.u4Tg_New_H_Front = 7;
		core2_rap_sw_reg.fld.u4Tg_New_V_Front = 7;
	} else {
		/* 0xA408 = 0x7C */
		core2_rap_sw_reg.fld.fgHsync_Sel_Ext = 1;
		core2_rap_sw_reg.fld.fgVsync_Set_Ext = 1;
		core2_rap_sw_reg.fld.fgDe_Sel_Ext = 1;

		/* 0xxA41C = 0x41 */
		core2_rap_sw_reg.fld.fgHsync_De_Swap = 1;
		core2_rap_sw_reg.fld.fgVsync_Sel_Vde = 0;

		core2_rap_sw_reg.fld.fgVsync_Pol_N = 0;
		core2_rap_sw_reg.fld.fgDe_Pol_N = 1;

		/* 0xA44C = 0x1 */
		core2_rap_sw_reg.fld.u4Use_Tg_New = 0;
	}

	/* 0x14 */
	core2_rap_sw_reg.fld.u4DmaAddrNum = 0x140;

	/* 0x18 */
	core2_rap_sw_reg.fld.u4DmaReadThrd = 0x3F;
	core2_rap_sw_reg.fld.u4DmaWriteThrd = 0x3F;

	/* 0x20 */
	core2_rap_sw_reg.fld.u4Hsync_Width = 0xa;
	core2_rap_sw_reg.fld.u4Vsync_Width = 0xa;

	/* 0x48 = 0x15 */
	core2_rap_sw_reg.fld.u4InputRGB = 0x2A;

	core2_rap_sw_reg.reg[0x20 / 4] = core2_timing_info[out_res].sync_width;
	core2_rap_sw_reg.reg[0x24 / 4] = core2_timing_info[out_res].x_zone;
	core2_rap_sw_reg.reg[0x28 / 4] = core2_timing_info[out_res].y_zone;
	core2_rap_sw_reg.reg[0x2C / 4] = core2_timing_info[out_res].sync_delay;

	if (dovi_mute) {
		core2_cfg_sw_reg.fld.fgYUV2RGB = 0x0;

		/* 0xA468 = 0xFF000000 */
		if (!dovi_idk_test) {
			core2_rap_sw_reg.fld.u4In_Fix_A = 0x3F;
			core2_rap_sw_reg.fld.fgIn_Fix_Pattern_A = 1;
			core2_rap_sw_reg.fld.fgIn_Fix_Pattern = 1;
		}
	} else {
		if (dovi_idk_test)
			core2_cfg_sw_reg.fld.fgYUV2RGB = 0x0;
		else
			core2_cfg_sw_reg.fld.fgYUV2RGB = 0x0;

		/* 0xA468 = 0xEF000000 */
		core2_rap_sw_reg.fld.u4In_Fix_A = 0x3F;
	}

	core2_rap_update_mode[0x04 / 4] = 1;
	core2_rap_update_mode[0x08 / 4] = 1;
	core2_rap_update_mode[0x18 / 4] = 1;
	core2_rap_update_mode[0x1C / 4] = 1;
	core2_rap_update_mode[0x20 / 4] = 1;
	core2_rap_update_mode[0x24 / 4] = 1;
	core2_rap_update_mode[0x28 / 4] = 1;
	core2_rap_update_mode[0x2C / 4] = 1;
	core2_rap_update_mode[0x48 / 4] = 1;

	core2_rap_update_mode[0x4C / 4] = 1;
	core2_rap_update_mode[0x50 / 4] = 1;
	core2_rap_update_mode[0x54 / 4] = 1;
	core2_rap_update_mode[0x58 / 4] = 1;
	core2_rap_update_mode[0x5C / 4] = 1;
	core2_rap_update_mode[0x68 / 4] = 1;

	core2_cfg_update_mode[0x04 / 4] = 1;

	return 1;
}

int dovi_core2_hal_config_reg(uint32_t *p_core2_reg)
{
	int idx = 0;

	for (idx = 0; idx < CORE2_DPM_REG_NUM; idx++) {
		core2_dpm_sw_reg.reg[idx] = p_core2_reg[idx];
		core2_dpm_update_mode[idx] = 1;
	}

	return 1;
}

int dovi_core2_hal_config_lut(UINT32 lut_addr)
{
	if (lut_addr) {
		core2_rap_sw_reg.fld.u4DmaAddr = ((lut_addr) >> 4);

		core2_rap_update_mode[0x10 / 4] = 1;
	} else {
		dovi_error("core2_lut_addr is not allocated !!!!");
	}

	return 1;
}

int dovi_core2_hal_isr(void)
{
	int idx = 0;

	if (core2_enable) {
		if (core2_ctrl_sw_reset) {
			p_core2_rap_hw_reg->reg[idx] = core2_reset_all;
			p_core2_rap_hw_reg->reg[idx] = CORE2_RESET_CLEAR;

			core2_ctrl_sw_reset = 0;
		}

		for (idx = 0; idx < CORE2_RAP_REG_NUM; idx++) {
			if (core2_rap_update_mode[idx] == 1) {
				p_core2_rap_hw_reg->reg[idx] = core2_rap_sw_reg.reg[idx];
				core2_rap_update_mode[idx] = 0;
			}
		}

		/* core2 trigger */
		WriteREG(core2_reg_base + CORE2_PROG_START, 0x1);

		for (idx = 0; idx < CORE2_CFG_REG_NUM; idx++) {
			if (core2_cfg_update_mode[idx] == 1) {
				p_core2_cfg_hw_reg->reg[idx] = core2_cfg_sw_reg.reg[idx];
				core2_cfg_update_mode[idx] = 0;
			}
		}

		for (idx = 0; idx < CORE2_DPM_REG_NUM; idx++) {
			if (core2_dpm_update_mode[idx] == 1) {
				p_core2_dpm_hw_reg->reg[idx] = core2_dpm_sw_reg.reg[idx];
				core2_dpm_update_mode[idx] = 0;
			}
		}

		/* core2 trigger */
		WriteREG(core2_reg_base + CORE2_PROG_FINISH, 0x1);
	}

	if (core2_pre_enable != core2_enable && dovi_core2_hal_is_support()) {

		core2_pre_enable = core2_enable;
	}

	return 1;
}

int dovi_core2_hal_set_enable(UINT32 enable)
{
	/* disp fmt color space */

	/* path switch */
	core2_enable = enable;

	return 1;
}

int dovi_core2_hal_show_black(bool enable)
{
	if (enable) {
		/* 0xA468 = 0xFF000000 */
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern_A = 1;
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern = 1;
		core2_cfg_sw_reg.fld.fgYUV2RGB = 0x0;
	} else {
		/* 0xA468 = 0x3F000000 */
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern_A = 0;
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern = 0;
		core2_cfg_sw_reg.fld.fgYUV2RGB = 0x1;
	}
	core2_rap_update_mode[0x68 / 4] = 1;
	core2_cfg_update_mode[0x04 / 4] = 1;
	return 1;
}

int dovi_core2_hal_reg_base_status(void)
{
	dovi_default("core2_reg_base		0x%p\n", core2_reg_base);
	dovi_default("p_core2_cfg_hw_reg 0x%p\n", p_core2_cfg_hw_reg);
	dovi_default("p_core2_dpm_hw_reg 0x%p\n", p_core2_dpm_hw_reg);
	dovi_default("p_core2_rap_hw_reg 0x%p\n", p_core2_rap_hw_reg);

	return 1;
}

int dovi_core2_hal_status(void)
{
	dovi_default("core2 support %d\n", dovi_core2_hal_is_support());

	if (core2_reg_base)
		dovi_default("core2 efuse[0x%x] 0x%p = 0x%x\n",
			    CORE2_EFUSE_REG,
			    core2_reg_base + CORE2_EFUSE_REG,
			    ReadREG(core2_reg_base + CORE2_EFUSE_REG));

	dovi_core2_hal_reg_base_status();

	return 1;
}

int dovi_core2_set_black_pattern(bool enable)
{
	core2_rap_sw_reg.fld.u4In_Fix_Y = 0x0;
	core2_rap_sw_reg.fld.u4In_Fix_C = 0x0;
	core2_rap_sw_reg.fld.u4In_Fix_V = 0x0;
	core2_rap_sw_reg.fld.u4In_Fix_A = 0x3F;
	if (enable) {
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern_A = 1;
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern = 1;
	} else {
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern_A = 0;
		core2_rap_sw_reg.fld.fgIn_Fix_Pattern = 0;
	}
	core2_rap_update_mode[0x068/4] = 1;
	return 1;
}
