/*
 * Copyright (C) 2016 MediaTek Inc.
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



#ifndef __DISP_VDP_DEBUG_H__
#define __DISP_VDP_DEBUG_H__

#include <linux/kernel.h>
#include "disp_info.h"
#include "disp_dovi_common.h"

void vdp_debug_init(void);
void vdp_debug_exit(void);

extern int disp_vdp_dbg_level_enable(uint32_t level, uint32_t enable);

extern enum DISP_DR_TYPE_T force_dr_range;

extern uint32_t vdp_disp_test;
extern bool dovi_idk_dump;
extern bool dovi_vs10_force;
extern bool dolby_force_output;
extern enum dovi_signal_format_t dolby_force_out_format;
extern uint32_t dovi_idk_disp_cnt;
extern bool force_dsd_off;
extern bool enable_frame_drop_log;
extern bool force_dolby_off;
extern bool debug_dovi_path_full_vs10;

#ifndef STR_CONVERT
#define STR_CONVERT(p, val, base, action)\
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
			DISP_LOG_E("[ERROR]kstrtoint/kstrtouint/kstrtoul return error: %d\n" \
				"  file : %s, line : %d\n",		\
				ret, __FILE__, __LINE__);\
			action; \
		} \
	} while (0)
#endif

#endif				/* __DISP_VDP_DEBUG_H__ */
