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

#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/sched.h>

#define LOG_TAG "DOVI_VFY_DEBUG"
#include "dovi_type.h"
#include "dovi_log.h"
#include "disp_dovi_cmd.h"
#include "dovi_core1_hal.h"
#include "dovi_unit_drv.h"

int dovi_unit_process_dbg_opt(const char *opt)
{
	int ret = 0;
	char *p;

	if (strncmp(opt, "register_test", 13) == 0) {
		dovi_unit_register_test();
	} else if (strncmp(opt, "core1_bypass_csc:", 13) == 0) {
		unsigned int bypass = 0;

		p = (char *)opt + 13;
		ret = kstrtoul(p, 10, (unsigned long int *)&bypass);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_core1_hal_bypass_csc(bypass);
	} else if (strncmp(opt, "core1_bypass_cvm:", 13) == 0) {
		unsigned int bypass = 0;

		p = (char *)opt + 13;
		ret = kstrtoul(p, 10, (unsigned long int *)&bypass);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_core1_hal_bypass_cvm(bypass);
	} else if (strncmp(opt, "set_path:", 9) == 0) {
		enum dovi_resolution resolution;
		struct dovi_unit_info_t dovi_unit_info;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 10, (unsigned long int *)&resolution);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dovi_unit_info.idk_version = DOVI_IDK_230;
		dovi_unit_info.resolution = resolution;

		dovi_unit_set_path(&dovi_unit_info);
	} else if (strncmp(opt, "unit_id:", 8) == 0) {
		enum dovi_unit_id unit_id;

		p = (char *)opt + 8;
		ret = kstrtoul(p, 10, (unsigned long int *)&unit_id);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dovi_unit_test(unit_id);
	} else {
		dovi_error("test debug cmd pass.\n");
		goto Error;
	}

	return 1;
Error:
	return 0;
}

