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

#ifndef _DOVI_VFY_HAL_C_
#define _DOVI_VFY_HAL_C_

#include <linux/module.h>


#include "disp_type.h"
#include "dovi_log.h"
#include "dovi_common_hw.h"
#include "dovi_core1_hw.h"
#include "dovi_core1_hal.h"

#include "dovi_unit_hal.h"

#define DOVI_REGISTER_FILL 0xFFFFFFFF
#define DOVI_REGISTER_NUM (0x400/4)

uint32_t dovi_unit_core_reg_read_only(uint32_t id, uint32_t idx)
{
	uint32_t reg = idx * 4;

	if (id == 0) {
		if ((reg == 0x000)
		|| ((reg >= 0x078) && (reg <= 0x084))
		|| ((reg >= 0x37c) && (reg <= 0x398))
		|| ((reg >= 0x3ac) && (reg <= 0x3fc)))
			return 1;
	} else if (id == 1) {
		if ((reg == 0x000)
		|| ((reg >= 0x078) && (reg <= 0x3fc)))
			return 1;
	} else {
		if ((reg == 0x000)
		|| ((reg >= 0x080) && (reg <= 0x08f))
		|| ((reg >= 0x3f0) && (reg <= 0x3fc)))
			return 1;
	}

	return 0;
}

uint32_t dovi_unit_fill_core_register(uint32_t id)
{
	uint32_t idx = 0;
	uint32_t *reg_base;

	if (id == 0)
		reg_base = (uint32_t *)core1_reg_base;
	else if (id == 1)
		reg_base = (uint32_t *)core2_reg_base;
	else
		reg_base = (uint32_t *)core3_reg_base;

	reg_base[0x08/4] = 1;

	reg_base[0x00/4] = DOVI_REGISTER_FILL;
	reg_base[0x04/4] = DOVI_REGISTER_FILL;

	for (idx = 0x10/4; idx < DOVI_REGISTER_NUM; idx++)
		reg_base[idx] = DOVI_REGISTER_FILL;

	reg_base[0x0c/4] = 1;

	return 1;
}

uint32_t dovi_unit_check_core_register(uint32_t id)
{
	uint32_t idx = 0;

	uint32_t *reg_base;
	uint32_t *reg_unit;

	if (id == 0) {
		reg_base = (uint32_t *)core1_reg_base;
		reg_unit = dovi_unit_core1_reg;
	} else if (id == 1) {
		reg_base = (uint32_t *)core2_reg_base;
		reg_unit = dovi_unit_core2_reg;
	} else {
		reg_base = (uint32_t *)core3_reg_base;
		reg_unit = dovi_unit_core3_reg;
	}

	for (idx = 0; idx < DOVI_REGISTER_NUM; idx++) {
		if ((reg_base[idx] != reg_unit[idx])
		&& !dovi_unit_core_reg_read_only(id, idx)) {
			dovi_error("core1 reg %d 0x%08X 0x%08X different\n",
			idx, reg_base[idx], reg_unit[idx]);
		}
	}

	if (idx < DOVI_REGISTER_NUM)
		return 0;
	else
		return 1;
}

uint32_t dovi_unit_register_test(void)
{
	uint32_t idx = 0;

	for (idx = 0; idx < 3; idx++)
		dovi_unit_fill_core_register(idx);

	for (idx = 0; idx < 3; idx++)
		dovi_unit_check_core_register(idx);

	return 1;
}

#endif
