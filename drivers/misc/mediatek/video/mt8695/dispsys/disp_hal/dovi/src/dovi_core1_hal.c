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

/*****************************************************************************
*  Video Plane: Interface
*****************************************************************************/

#ifndef _DOVI_CORE1_HAL_C_
#define _DOVI_CORE1_HAL_C_

#include <linux/module.h>

#include "disp_hw_mgr.h"

#include "disp_type.h"
#include "dovi_log.h"
#include "dovi_common_hw.h"
#include "dovi_core1_hw.h"
#include "dovi_core1_hal.h"
/******************************************************************************
* local definition
******************************************************************************/



/******************************************************************************
* Function prototype
******************************************************************************/


/******************************************************************************
* Local variable
******************************************************************************/
union core1_dpm_reg_u *p_core1_dpm_hw_reg;
union core1_cfg_reg_u *p_core1_cfg_hw_reg;
union core1_rap_reg_u *p_core1_rap_hw_reg;

union core1_dpm_reg_u core1_dpm_sw_reg;
union core1_cfg_reg_u core1_cfg_sw_reg;
union core1_rap_reg_u core1_rap_sw_reg;

uint8_t core1_dpm_update_mode[CORE1_DPM_REG_NUM];
uint8_t core1_cfg_update_mode[CORE1_CFG_REG_NUM];
uint8_t core1_rap_update_mode[CORE1_RAP_REG_NUM];

char *core1_reg_base;

int core1_ctrl_sw_reset;
int core1_enable;
int core1_pre_enable;
int core1_enable_state;
int core1_hal_init;


int pp_is_enable;
int hdmi_delay;

struct disp_hw_resolution core1_out_res;
bool core1_bypass_cvm;
bool core1_bypass_csc;

/******************************************************************************
* local macro
******************************************************************************/
int dovi_core1_hal_init(char *reg_base)
{
	if (core1_hal_init) {
		dovi_info("core1 hal already inited\n");
		return 0;
	}

	/* dovi_func(); */

	core1_reg_base = reg_base;

	p_core1_dpm_hw_reg = (union core1_dpm_reg_u *)(reg_base + CORE1_DPM_REG_BASE);
	p_core1_cfg_hw_reg = (union core1_cfg_reg_u *)(reg_base + CORE1_CFG_REG_BASE);
	p_core1_rap_hw_reg = (union core1_rap_reg_u *)(reg_base + CORE1_RAP_REG_BASE);

	/* dovi_core1_hal_reg_base_status(); */

	core1_hal_init = 1;
	return 1;
}

int dovi_core1_hal_uninit(void)
{
	if (!core1_hal_init) {
		dovi_info("core1 hal already uninited\n");
		return 0;
	}

	dovi_func();

	core1_hal_init = 0;
	return 1;
}

/* config 0x41800 dovi core1 hw register*/
int dovi_core1_hal_config_reg(uint32_t *p_core1_reg)
{
	int idx = 0;

	if (((p_core1_reg[(0xCC - 0x18)/4] & 0x1fff) != core1_out_res.width) ||
	(((p_core1_reg[(0xCC - 0x18)/4] >> 13) & 0x1fff) != core1_out_res.height)) {
		dovi_default("core1_res 0x%X -> 0x%X 0x%X\n",
		p_core1_reg[(0xCC - 0x18)/4],
		core1_out_res.width,
		core1_out_res.height);

		p_core1_reg[(0xCC - 0x18)/4] = (core1_out_res.height << 13)
		| core1_out_res.width;
	}

	for (idx = 0; idx < CORE1_DPM_REG_NUM; idx++) {
		core1_dpm_sw_reg.reg[idx] = p_core1_reg[idx];
		core1_dpm_update_mode[idx] = 1;
	}

	dovi_core1_hal_set_composer_mode(false);

	return 1;
}

int dovi_core1_hal_config_lut(UINT32 lut_addr)
{
	if (lut_addr) {
		core1_rap_sw_reg.fld.dma_start_addr = ((lut_addr) >> 4);

		core1_rap_update_mode[0x10 / 4] = 1;
	} else {
		dovi_error("core1_lut_addr is not allocated !!!!");
	}

	return 1;
}

int dovi_core1_hal_set_composer_mode(BOOL fgCompEL)
{
	if (fgCompEL) {
		core1_dpm_sw_reg.fld.core1_dpm_reg[(0xC8 - 0x18) / 4] = 0;
		core1_dpm_update_mode[(0xC8 - 0x18) / 4] = 1;
	} else {
		core1_dpm_sw_reg.fld.core1_dpm_reg[(0xC8 - 0x18) / 4] = 1;
		core1_dpm_update_mode[(0xC8 - 0x18) / 4] = 1;
	}

	return 1;
}


/* config 0x41c00 dovi core1 wrap control register*/
int dovi_core1_hal_control_config(struct disp_hw_resolution *info)
{
	int pixel_rate = 0;
	int operating_mode = 0;
	uint32_t core1_cfg_reg_004 = 0;
	UINT32 frame_rate = info->frequency;

	switch (frame_rate) {
	case 23:
		pixel_rate = 0;
		break;
	case 24:
		pixel_rate = 1;
		break;
	case 25:
		pixel_rate = 2;
		break;
	case 29:
		pixel_rate = 3;
		break;
	case 30:
		pixel_rate = 4;
		break;
	case 50:
		pixel_rate = 5;
		break;
	case 59:
		pixel_rate = 6;
		break;
	case 60:
		pixel_rate = 7;
		break;
	default:
		pixel_rate = 7;
		break;

	}

	core1_out_res = *info;

	if (g_force_dolby > 0)
		core1_ctrl_sw_reset = 0;
	else
		core1_ctrl_sw_reset = 1;

	/* 0x04 */
	core1_rap_sw_reg.fld.dma_trigger = 1;
	core1_rap_sw_reg.fld.dma_trigger_select = 1;

	/* 0x08 */
	core1_rap_sw_reg.fld.dma_en = 1;
	core1_rap_sw_reg.fld.dma_dram_en = 1;
	core1_rap_sw_reg.fld.hsync_select = 1;
	core1_rap_sw_reg.fld.vsync_select = 1;
	core1_rap_sw_reg.fld.de_select = 1;

	/* 0x18 */
	if (g_dma_read_empty_threshold != 0)
		core1_rap_sw_reg.fld.dma_read_empty_threshold = g_dma_read_empty_threshold;
	else
		core1_rap_sw_reg.fld.dma_read_empty_threshold = 0x3F;
	if (g_dma_write_full_threshold != 0)
		core1_rap_sw_reg.fld.dma_write_full_threshold = g_dma_write_full_threshold;
	else
		core1_rap_sw_reg.fld.dma_write_full_threshold = 0x3F;

	/* 0x1C */
	core1_rap_sw_reg.fld.bl_hsync_de_swap = 1;
	core1_rap_sw_reg.fld.bl_de_pol_ctrl = 1;

	core1_cfg_reg_004 = (operating_mode << CORE1_OPERATING_MODE);
	core1_cfg_reg_004 |= (pixel_rate << CORE1_PIXEL_RATE);

	if (core1_bypass_cvm)
		core1_cfg_reg_004 |= (1 << CORE1_BYPASS_CVM);

	if (core1_bypass_csc)
		core1_cfg_reg_004 |= (1 << CORE1_BYPASS_CSC);

	core1_cfg_sw_reg.fld.core1_cfg_reg[CORE1_CFG_REG_004] = core1_cfg_reg_004;

	core1_rap_update_mode[0x04 / 4] = 1;
	core1_rap_update_mode[0x08 / 4] = 1;
	core1_rap_update_mode[0x18 / 4] = 1;
	core1_rap_update_mode[0x1C / 4] = 1;

	core1_cfg_update_mode[CORE1_CFG_REG_004] = 1;

	return 1;
}

/* core1 path switch */
/* 0x42530[7] = 1 core1 path switch for mix */
/* 0x42538[5] = 1 core1 share vdo dram i/f */
/* dispfmt3 0x450f0[24]  output444 */
/* dispfmt1 0x420f0[24]  output444 */
int dovi_core1_hal_set_enable(UINT32 enable)
{
	/* disp fmt color space */

	/* path switch */
	core1_enable = enable;

	return 1;
}

int dovi_core1_hal_is_support(void)
{
	return (p_core1_cfg_hw_reg
	&& !(p_core1_cfg_hw_reg->fld.core1_cfg_reg[CORE1_CFG_REG_004]
	& (1 << CORE1_EFUSE_DISABLE)));
}

int dovi_core1_hal_bypass_csc(uint32_t bypass)
{
	dovi_default("core1 bypass csc : %d!\n", bypass);

	if (bypass) {
		core1_cfg_sw_reg.fld.core1_cfg_reg[CORE1_CFG_REG_004]
		|= (1 << CORE1_BYPASS_CSC);
		core1_bypass_csc = true;
	} else {
		core1_cfg_sw_reg.fld.core1_cfg_reg[CORE1_CFG_REG_004]
		&= !(1 << CORE1_BYPASS_CSC);
		core1_bypass_csc = false;
	}

	core1_cfg_update_mode[CORE1_CFG_REG_004] = 1;

	return 1;
}
int dovi_core1_hal_bypass_cvm(uint32_t bypass)
{
	dovi_default("core1 bypass cvm : %d!\n", bypass);

	if (bypass) {
		core1_cfg_sw_reg.fld.core1_cfg_reg[CORE1_CFG_REG_004]
		|= (1 << CORE1_BYPASS_CVM);
		core1_bypass_cvm = true;
	} else {
		core1_cfg_sw_reg.fld.core1_cfg_reg[CORE1_CFG_REG_004]
		&= !(1 << CORE1_BYPASS_CVM);
		core1_bypass_cvm = false;
	}

	core1_cfg_update_mode[CORE1_CFG_REG_004] = 1;

	return 1;
}

void dovi_core1_hal_crc(void)
{
	dovi_info("core1 crc 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
		    ReadREG(core1_reg_base + CORE1_BL_CRC),
		    ReadREG(core1_reg_base + CORE1_EL_CRC),
		    ReadREG(core1_reg_base + CORE1_CMP_CRC),
		    ReadREG(core1_reg_base + CORE1_2TO4_CRC),
		    ReadREG(core1_reg_base + CORE1_CSC_CRC),
		    ReadREG(core1_reg_base + CORE1_CVM_CRC),
		    ReadREG(core1_reg_base + CORE1_LUT_CRC));
}

void dovi_core1_hal_crc_reset(void)
{
	dovi_info("core1 crc reset\n");
	WriteREG(core1_reg_base + CORE1_CRC_TRIGGER, 0x3);
}

int dovi_core1_hal_isr(void)
{
	int idx = 0;

	if (core1_enable) {
		if (core1_ctrl_sw_reset) {
			p_core1_rap_hw_reg->reg[idx] = CORE1_RESET_ALL;
			p_core1_rap_hw_reg->reg[idx] = CORE1_RESET_CLEAR;

			core1_ctrl_sw_reset = 0;
		}

		for (idx = 0; idx < CORE1_RAP_REG_NUM; idx++) {
			if (core1_rap_update_mode[idx] == 1) {
				p_core1_rap_hw_reg->reg[idx] = core1_rap_sw_reg.reg[idx];
				core1_rap_update_mode[idx] = 0;
			}
		}

		/* core1 trigger */
		WriteREG(core1_reg_base + CORE1_PROG_START, 0x1);

		for (idx = 0; idx < CORE1_CFG_REG_NUM; idx++) {
			if (core1_cfg_update_mode[idx] == 1) {
				p_core1_cfg_hw_reg->reg[idx] = core1_cfg_sw_reg.reg[idx];
				core1_cfg_update_mode[idx] = 0;
			}
		}

		for (idx = 0; idx < CORE1_DPM_REG_NUM; idx++) {
			if (core1_dpm_update_mode[idx] == 1) {
				p_core1_dpm_hw_reg->reg[idx] = core1_dpm_sw_reg.reg[idx];
				core1_dpm_update_mode[idx] = 0;
			}
		}

		/* core1 trigger */
		WriteREG(core1_reg_base + CORE1_PROG_FINISH, 0x1);
	}

	if (core1_pre_enable != core1_enable && dovi_core1_hal_is_support()) {
		/* core1 path switch for mix */
		/* WriteREGMsk(0x42530, core1_enable << 7, 0x1 << 7); */
		/* core1 share vdo dram i/f */
		/* WriteREGMsk(0x42538, core1_enable << 5, 0x1 << 5); */
		core1_pre_enable = core1_enable;
	}

	return 1;
}

int dovi_core1_hal_reg_base_status(void)
{
	dovi_default("core1_reg_base		0x%p\n", core1_reg_base);
	dovi_default("p_core1_cfg_hw_reg 0x%p\n", p_core1_cfg_hw_reg);
	dovi_default("p_core1_dpm_hw_reg 0x%p\n", p_core1_dpm_hw_reg);
	dovi_default("p_core1_rap_hw_reg 0x%p\n", p_core1_rap_hw_reg);

	return 1;
}

int dovi_core1_hal_status(void)
{
	dovi_default("core1 support %d\n", dovi_core1_hal_is_support());
	if (core1_reg_base)
		dovi_default("core1 efuse[0x%x] 0x%p = 0x%x\n",
			    CORE1_EFUSE_REG,
			    core1_reg_base + CORE1_EFUSE_REG,
			    ReadREG(core1_reg_base + CORE1_EFUSE_REG));

	dovi_core1_hal_reg_base_status();

	return 1;
}

int dovi_core1_hal_set_out_fix_pattern_enable(bool enable,
	uint32_t pattern_i, uint32_t pattern_cp, uint32_t pattern_ct)
{
	int idx = 0x06C/4;

	core1_rap_sw_reg.fld.out_patten_en = enable;
	core1_rap_sw_reg.fld.pattern_I = pattern_i;
	core1_rap_sw_reg.fld.pattern_Cp = pattern_cp;
	core1_rap_sw_reg.fld.pattern_Ct = pattern_ct;
	core1_rap_update_mode[idx] = 1;
	return 1;
}

#endif
