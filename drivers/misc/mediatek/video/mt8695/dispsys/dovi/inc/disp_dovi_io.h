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

#ifndef __DISP_DOVI_IO_H__
#define __DISP_DOVI_IO_H__

#include "hdmitx.h"
#include "videoin_hal.h"
#include "vdout_sys_hal.h"

enum dovi_clr_fmt {
	BLK_8B_420_NON_UFO,
	BLK_8B_420_UFO,
	RST_8B_420_NON_UFO,
	BLK_10B_420_NON_UFO,
	BLK_10B_420_UFO,
	RST_10B_420_NON_UFO,
};

void disp_dovi_idk_dump_frame_end(void);
void disp_dovi_idk_dump_frame_start(uint32_t file_id);
void disp_dovi_alloc_graphic_buffer(void);
void disp_dovi_free_graphic_buffer(void);
void disp_dovi_idk_dump_frame(void);
void disp_dovi_idk_dump_vin(bool enable, enum VIDEOIN_SRC_SEL src,
	enum VIDEOIN_YCbCr_FORMAT fmt);

void disp_dovi_load_buffer(char *file_name, unsigned char *buff, uint32_t len);

void disp_dovi_dump_rpu(bool dump_enable, unsigned char *buff, uint32_t rpu_len);

void disp_dovi_dump_comp(unsigned int frame_num,
unsigned char *buff, uint32_t len);

void disp_dovi_dump_orig_md(unsigned int frame_num,
unsigned char *buff, uint32_t len);

void disp_dovi_dump_hdmi_md(unsigned int frame_num,
unsigned char *buff, uint32_t len);

void disp_dovi_dump_core_reg(unsigned int core_id,
unsigned int frame_num,
unsigned char *buff, uint32_t len);

void disp_dovi_dump_core_lut(unsigned int core_id,
unsigned int frame_num,
unsigned char *buff, uint32_t len);

extern struct disp_hw_resolution dovi_res;
extern struct device *dovi_dev;

void disp_dovi_dump_vin(bool enable, enum VIDEOIN_SRC_SEL src,
enum VIDEOIN_YCbCr_FORMAT fmt, enum VIDEO_BIT_MODE bpp);

void disp_dovi_alloc_input_buf(uint32_t vdp_id, bool enable);

void disp_dovi_load_video_pattern(uint32_t vdp_id, uint32_t pattern,
enum dovi_clr_fmt color_fmt);

void disp_dovi_enable_vdp(uint32_t vdp_id, uint32_t pattern);
extern int ll_rgb_desired;
extern int ll_format;
#endif
