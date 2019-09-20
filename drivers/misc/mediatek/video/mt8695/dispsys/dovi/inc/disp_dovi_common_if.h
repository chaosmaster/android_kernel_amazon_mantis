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

#ifndef _DISP_DOVI_COMMON_IF_H_
#define _DISP_DOVI_COMMON_IF_H_
#include "disp_dovi_common.h"
#include "disp_info.h"

int dovi_remove_rpu_nal_type(
unsigned int first_frame,
unsigned char *src_rpu,
unsigned char *dst_rpu,
unsigned int len);
uint32_t dovi_set_out_res(uint32_t out_res, uint16_t width,	uint16_t height);
UINT32 dovi_set_output_format(enum dovi_signal_format_t out_format);
UINT32 dovi_set_video_info(struct mtk_disp_hdr_md_info_t *hdr_metadata);
UINT32 dovi_set_video_input_format(enum dovi_signal_format_t e_input_format);
UINT32 dovi_get_input_format(enum DISP_DR_TYPE_T *dovi_input_dr);
UINT32 dovi_set_graphic_format(UINT32 g_format);
UINT32 dovi_set_graphic_info(UINT32 ucOn);
UINT32 dovi_set_composer_mode(BOOL fgComposerEL);
UINT32 dovi_update_graphic_info(void);

UINT32 dovi_set_priority_mode(UINT32 mode);
UINT32 dovi_get_priority_mode(UINT32 *mode);
UINT32 dovi_set_low_latency_mode(int use_ll, int ll_rgb_desired);
UINT32 dovi_set_dovi2hdr10_mapping(int dovi2hdr10_mapping);
UINT32 dovi_set_vsvdb_file_name(char *vsvdb_file_name);
UINT32 dovi_set_vsvdb_hdmi(char *vsvdb_edid, int len);

UINT32 dovi_update_target_lum(void);
int disp_dovi_common_init(void);
int disp_dovi_common_test(uint32_t option);
UINT32 dovi_get_low_latency_mode(void);
UINT32 dovi_get_output_format(void);
bool dovi_get_profile4(void);

extern struct disp_hw_resolution dovi_res;
extern uint32_t dovi_idk_test;
extern uint32_t force_priority_mode;
extern uint32_t priority_mode;
extern uint32_t dovi_idk_file_id;
extern bool set_graphic_max_lum_enable;
extern bool set_video_max_lum_enable;

extern int graphic_max_lum;
extern int video_max_lum;

#endif
