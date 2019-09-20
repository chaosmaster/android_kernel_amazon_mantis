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

#ifndef __DISP_PATH_H__
#define __DISP_PATH_H__

#include <linux/device.h>
#include <linux/types.h>
#include "hdmitx.h"

#define FULL_SCREEN_IDX 2  /*0-default,  1-dolby*/
#define DISP_PATH_SHADOW_ENABLE

struct full_screen_param {
	int h_start;
	int v_odd_start;
	int v_even_start;
	int h_delay;
	int v_delay;
	HDMI_VIDEO_RESOLUTION res;
};

enum DISP_PATH_TYPE {
	DISP_PATH_DEFAULT = 0,
	DISP_PATH_DOLBY = (1 << 0),
	DISP_PATH_P2I = (1 << 1),
	DISP_PATH_DOLBY_P2I = (1 << 1) | (1 << 0),
	DISP_PATH_UNKNOWN
};

enum DISP_PATH_HW_ID {
	DISP_PATH_MVDO,
	DISP_PATH_SVDO,
	DISP_PATH_SVDO_DOLBY_CORE2,
	DISP_PATH_SVDO_SDR2HDR,
	DISP_PATH_OSD1,
	DISP_PATH_OSD2,
	DISP_PATH_MVDO_OUT,
	DISP_PATH_SVDO_OUT,
	DISP_PATH_DOLBY_CORE1,
	DISP_PATH_DOLBY_CORE2,
	DISP_PATH_DOLBY_CORE3,
};

struct disp_path_resolution {
	uint16_t htotal;
	uint16_t vtotal;
	uint16_t width;
	uint16_t height;
	uint16_t frequency;
	bool is_progressive;
	bool is_hd;
	HDMI_VIDEO_RESOLUTION res;
};

struct disp_path_context {
	struct disp_path_resolution resolution;
	enum DISP_PATH_TYPE path;
	bool is_dolby;
	struct full_screen_param fs_param;
};



int disp_path_get_active_zone(enum DISP_PATH_HW_ID id, HDMI_VIDEO_RESOLUTION res,
	int *h_start, int *v_odd_start, int *v_even_start);

int disp_path_get_delay_info(enum DISP_PATH_HW_ID id, HDMI_VIDEO_RESOLUTION res,
	int *h_delay, int *v_delay);

int disp_path_set_delay(enum DISP_PATH_HW_ID id, HDMI_VIDEO_RESOLUTION res);

int disp_path_set_dsd_delay(enum DISP_PATH_HW_ID id, HDMI_VIDEO_RESOLUTION res);
int disp_path_set_delay_by_shift(enum DISP_PATH_HW_ID id, HDMI_VIDEO_RESOLUTION res,
	bool h_left, int h_shift_cnt, bool v_up, int v_shift_cnt);

int disp_path_set_dolby(bool is_dolby);
int disp_path_set_dsd_dolby(bool is_dolby);

int disp_path_set_dolby_by_shift(bool is_dolby, int h_shift_cnt_all, int h_shift_cnt_ui);
int disp_path_set_dsd_dolby_by_shift(bool is_dolby, int h_shift_cnt_all, int h_shift_cnt_ui);

int disp_path_set_res(struct disp_path_resolution *resolution);

int disp_path_reset(void);

int disp_path_init(struct disp_path_resolution *resolution);

extern unsigned int g_hdmi_res;
extern unsigned int g_force_dolby;
extern unsigned int g_hdr_type;
extern unsigned int g_out_format;


#endif
