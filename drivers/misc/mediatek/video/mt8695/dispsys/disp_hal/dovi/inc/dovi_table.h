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

#ifndef _DOVI_TABLE_H_
#define _DOVI_TABLE_H_
#include "hdmitx.h"
#include "dovi/inc/disp_dovi_control_if.h"

#define CORE3_MAX_TIMING 6
#define MAX_FILE_NAME_LEN 70
#define MAX_DOVI_TEST_CASE_ID 150
#define OUT_FORMAT_DOVI 0
#define OUT_FORMAT_HDR10 1
#define OUT_FORMAT_SDR 2

struct dobly_resolution {
	uint16_t htotal;
	uint16_t vtotal;
	uint16_t width;
	uint16_t height;
	uint16_t frequency;
	bool is_progressive;
	bool is_hd;
	HDMI_VIDEO_RESOLUTION res_mode;
};

struct core3_timing_info_t {
	uint16_t width;
	uint16_t height;
	uint16_t x_start;
	uint16_t y_start;
};

struct dovi_timing_info {
	uint32_t sync_width;
	uint32_t x_zone;
	uint32_t y_zone;
	uint32_t sync_delay;
};

struct dovi_out_format_table_t {
	uint32_t test_case_id;
	uint32_t out_format;
};

struct dovi2hdr10_out_map_table_t {
	uint32_t test_case_id;
	uint32_t dovi2hdr10_mapping;
};


struct dovi_case_info_t {
	uint32_t test_case_id;
	char vsvdb_file_name[MAX_FILE_NAME_LEN];
	uint32_t use_ll;
	uint32_t ll_rgb_desired;
};

struct dovi_graphic_info_table_t {
	uint32_t test_case_id;
	char graphic_file_name[MAX_FILE_NAME_LEN];
	int f_graphic_on;
	enum pri_mode_t priority_mode;
	enum graphic_format_t g_format;
};

extern unsigned int core3_select_external_timing;

void dovi_get_core3_x_start(uint16_t width, uint16_t height, uint16_t *x_start, uint16_t *y_start);

extern uint32_t dovi_v23_480p_yuv_yuv_core1_reg[];
extern uint32_t dovi_v23_480p_yuv_yuv_core2_reg[];
extern uint32_t dovi_v23_480p_yuv_yuv_core3_reg[];
extern uint32_t dovi_v23_480p_yuv_yuv_core1_lut[];
extern uint32_t dovi_v23_480p_yuv_yuv_core2_lut[];

extern uint32_t dovi_v23_480p_yuv_ipt_core1_reg[];
extern uint32_t dovi_v23_480p_yuv_ipt_core2_reg[];
extern uint32_t dovi_v23_480p_yuv_ipt_core3_reg[];
extern uint32_t dovi_v23_480p_yuv_ipt_core1_lut[];
extern uint32_t dovi_v23_480p_yuv_ipt_core2_lut[];

extern uint32_t dovi_v23_480p_yuv_hdr_core1_reg[];
extern uint32_t dovi_v23_480p_yuv_hdr_core2_reg[];
extern uint32_t dovi_v23_480p_yuv_hdr_core3_reg[];
extern uint32_t dovi_v23_480p_yuv_hdr_core1_lut[];
extern uint32_t dovi_v23_480p_yuv_hdr_core2_lut[];

extern uint32_t dovi_v23_720p_yuv_yuv_core1_reg[];
extern uint32_t dovi_v23_720p_yuv_yuv_core2_reg[];
extern uint32_t dovi_v23_720p_yuv_yuv_core3_reg[];
extern uint32_t dovi_v23_720p_yuv_yuv_core1_lut[];
extern uint32_t dovi_v23_720p_yuv_yuv_core2_lut[];

extern uint32_t dovi_v23_1080p_yuv_yuv_core1_reg[];
extern uint32_t dovi_v23_1080p_yuv_yuv_core2_reg[];
extern uint32_t dovi_v23_1080p_yuv_yuv_core3_reg[];
extern uint32_t dovi_v23_1080p_yuv_yuv_core1_lut[];

extern uint32_t dovi_v23_2160p_yuv_yuv_core1_reg[];
extern uint32_t dovi_v23_2160p_yuv_yuv_core2_reg[];
extern uint32_t dovi_v23_2160p_yuv_yuv_core3_reg[];
extern uint32_t dovi_v23_2160p_yuv_yuv_core1_lut[];

extern uint32_t dovi_v23_2160p_yuv_ipt_core1_reg[];
extern uint32_t dovi_v23_2160p_yuv_ipt_core2_reg[];
extern uint32_t dovi_v23_2160p_yuv_ipt_core3_reg[];
extern uint32_t dovi_v23_2160p_yuv_ipt_core1_lut[];
extern uint32_t dovi_v23_2160p_yuv_ipt_core2_lut[];


/* case 1 yuv to yuv */
extern uint32_t dovi_v241_1080p_yuv_yuv_core1_reg[];
extern uint32_t dovi_v241_1080p_yuv_yuv_core3_reg[];
extern uint32_t dovi_v241_1080p_yuv_yuv_core1_lut[];

/* case 2 ipt to yuv */
extern uint32_t dovi_v241_1080p_ipt_yuv_core1_reg[];
extern uint32_t dovi_v241_1080p_ipt_yuv_core2_reg[];
extern uint32_t dovi_v241_1080p_ipt_yuv_core3_reg[];
extern uint32_t dovi_v241_1080p_ipt_yuv_core1_lut[];
extern uint32_t dovi_v241_1080p_ipt_yuv_core2_lut[];

/* case 3 yuv to ipt */
extern uint32_t dovi_v241_1080p_yuv_ipt_core1_reg[];
extern uint32_t dovi_v241_1080p_yuv_ipt_core3_reg[];
extern uint32_t dovi_v241_1080p_yuv_ipt_core1_lut[];

/* case 4 ipt to ipt */
extern uint32_t dovi_v241_1080p_ipt_ipt_core1_reg[];
extern uint32_t dovi_v241_1080p_ipt_ipt_core2_reg[];
extern uint32_t dovi_v241_1080p_ipt_ipt_core3_reg[];
extern uint32_t dovi_v241_1080p_ipt_ipt_core1_lut[];
extern uint32_t dovi_v241_1080p_ipt_ipt_core2_lut[];
/* lk for test */
extern uint32_t dovi_1080p_sdr_sdr_core2_lut[];
extern uint32_t dovi_1080p_sdr_hdr10_core2_lut[];
extern uint32_t dovi_1080p_sdr_ipt_core2_lut[];
extern uint32_t dovi_1080p_sdr_ll_core2_lut[];

extern char dovi_graphic_name[][MAX_FILE_NAME_LEN];
extern struct dobly_resolution dobly_resolution_table[];
extern const char *dobly_resstr[HDMI_VIDEO_RESOLUTION_NUM];
extern struct dovi_timing_info core2_timing_info[];

extern uint32_t get_dovi_out_format(uint32_t test_case_id);
extern uint32_t get_dovi_out_format_status(void);
extern uint32_t set_dovi_out_format(uint32_t test_case_id, uint32_t out_format);
extern char *get_dovi_vsvdb_file_name(uint32_t test_case_id);
extern uint32_t get_dovi2hdr10_mapping_type(uint32_t test_case_id);
extern uint32_t get_dovi2hdr10_mapping_type_Status(void);
extern uint32_t set_dovi2hdr10_mapping_type(uint32_t test_case_id, uint32_t dovi2hdr10_mapping);
extern uint32_t get_dovi_use_ll(uint32_t test_case_id);
extern uint32_t get_dovi_ll_rgb_desired(uint32_t test_case_id);
extern uint32_t set_dovi_ll_mode(uint32_t test_case_id, uint32_t use_ll, uint32_t ll_rgb_desired);
extern uint32_t get_dovi_ll_mode_status(void);
extern int get_dovi_graphic_on(uint32_t test_case_id);
extern enum pri_mode_t get_dovi_priority_mode(uint32_t test_case_id);
extern enum graphic_format_t get_dovi_g_format(uint32_t test_case_id);
extern void set_dovi_priority_mode(uint32_t force_pri_mode, uint32_t pri_mode);
extern void set_dovi_g_format(uint32_t force_gformat, uint32_t gformat);
extern char *get_dovi_graphic_file_name(uint32_t test_case_id);

#endif
