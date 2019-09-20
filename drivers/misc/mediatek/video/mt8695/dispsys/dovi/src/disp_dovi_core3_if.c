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
#include "dovi_type.h"
#include "dovi_log.h"

#include "disp_dovi_core3_if.h"
#include "disp_hw_mgr.h"
#include "dovi_core3_hal.h"
#include "dovi_core3_hw.h"

int core3_init;


int dovi_core3_init(char *reg_base)
{
	INT32 i4Ret = 0;

	if (core3_init) {
		dovi_info("core3 already inited\n");
		return 0;
	}

	dovi_core3_hal_init(reg_base);

	core3_init = 1;

	return i4Ret;
}

int dovi_core3_uninit(void)
{
	dovi_info(" [CORE3] Drv uninit ok\n");

	return 1;
}

int dovi_core3_status(void)
{
	dovi_default("core init %d\n", core3_init);

	dovi_core3_hal_status();

	return 1;
}
