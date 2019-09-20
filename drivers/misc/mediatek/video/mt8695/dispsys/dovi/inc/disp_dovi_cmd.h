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

#ifndef __DISP_DOVI_CMD_H__
#define __DISP_DOVI_CMD_H__

extern bool dump_rpu_enable;
extern uint32_t dump_md_enable;
extern uint32_t reg_test_enable;
extern uint32_t dovi_idk_test;
extern uint32_t dovi_idk_file_id;
extern uint32_t dovi_sdk_test;
extern uint32_t dovi_sdk_file_id;
extern bool dovi_mute;
extern bool fhd_scale_to_uhd;
extern enum VIDEO_BIT_MODE idk_dump_bpp;

void dovi_debug_init(void);

int disp_dovi_status(uint32_t level);
int disp_dovi_default_path(uint32_t value);
int disp_dovi_default_path_init(uint32_t option);

extern uint32_t core2_reset_all;
extern int disp_dovi_common_test(uint32_t option);
extern int dovi_unit_process_dbg_opt(const char *opt);
extern uint32_t disp_dovi_set_idk_info(void);
extern uint32_t disp_dovi_set_sdk_info(void);
extern enum dovi_status dovi_sec_debug_level_init(uint32_t dovi_tz_level);
extern void disp_dovi_set_hdr_enable(uint32_t enable, uint32_t outformat);

#ifndef STR_CVT
#define STR_CVT(p, val, base, action)\
	do {			\
		int ret = 0;	\
		const char *tmp;	\
		tmp = strsep(p, ","); \
		if (tmp == NULL) \
			break; \
		if (strcmp(#base, "int") == 0)\
			ret = kstrtoint(tmp, 0, (int *)val); \
		else if (strcmp(#base, "uint") == 0)\
			ret = kstrtouint(tmp, 0, (unsigned int *)val); \
		else if (strcmp(#base, "ul") == 0)\
			ret = kstrtoul(tmp, 0, (unsigned long *)val); \
		if (ret != 0) {\
			dovi_error("[ERROR]kstrtoint/kstrtouint/kstrtoul return error: %d\n" \
				"  file : %s, line : %d\n",		\
				ret, __FILE__, __LINE__);\
			action; \
		} \
	} while (0)
#endif

#endif
