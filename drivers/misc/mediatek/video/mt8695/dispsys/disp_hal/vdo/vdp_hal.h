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


#ifndef __VDP_HAL_H__
#define __VDP_HAL_H__
/*#include <mach/sync_write.h>*/
/*#include <mach/mt_typedefs.h>*/

/*#include "pmx_hal.h"*/
#include "disp_type.h"
#include "disp_vdp_if.h"
#include "disp_hw_mgr.h"
#include "disp_hw_log.h"
#include "vdp_hw.h"

#define VDP_1					0
#define VDP_2					1
#define VDP_MAX					2

#define VDP_ADDR_MAX				5
#define VDP_FACTOR1				0x800
#define VDP_FACTOR2				0x800
#define VDP_H_FACTOR				0x1000

#define VDP_VERTICAL_SCALING_FACTOR(vdp_id) \
	(((vdp_id) == VDP_1) ? VDP_FACTOR1 : VDP_FACTOR2)


#define HSCALE_UNIT				0x1000

struct vdp_hal_region {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

struct vdp_hal_fb_size {
	uint32_t pic_w;
	uint32_t pic_h;
	uint32_t buff_w;
	uint32_t buff_h;
};

struct vdp_hal_fb_addr {
	uint32_t addr_y;
	uint32_t addr_c;
	uint32_t addr_y_len;
	uint32_t addr_c_len;
	int sec_handle;
	uint32_t buffer_size;
};

struct vdp_hal_fb_info {
	uint8_t is_secruity;
	uint8_t is_interlace;
	uint8_t is_dolby;
	uint8_t is_scan_line;
	uint8_t is_yuv422;
	uint8_t is_10bit;
	uint8_t is_ufo;
	uint8_t is_10bit_tile_mode;
	uint8_t is_jumpmode;


	uint8_t is_hdr10plus;

	/* for dolby or hdr10 metadata svp path */
	uint32_t metadata_size;
	uint32_t metadata_sec_handle;

	struct vdp_hal_fb_size fb_size;
	struct vdp_hal_fb_addr fb_addr;

	/* display region */
	struct vdp_hal_region src_region;
	struct vdp_hal_region out_region;


	/* if buffer_info is video plane, should fill video info */
	unsigned long long pts;
};

struct vdp_hal_config_info {
	uint8_t vdp_id;
	uint8_t enable;
	uint8_t is_el_exist;
	uint8_t is_second_field;

	struct vdp_hal_fb_info cur_fb_info;

	/*for dolby vision el */
	struct vdp_hal_fb_info el_cur_fb_info;
};

struct vdp_hal_color_format {
	uint8_t is_scan_line;
	uint8_t is_yuv422;
	uint8_t is_10bit;
	uint8_t is_ufo;
	uint8_t is_10bit_tile_mode;
	uint8_t is_jumpmode;
};

struct vdp_hal_data_info {
	uint8_t vdp_id;
	uint8_t enable;
	uint8_t is_secruity;
	uint8_t is_interlace;
	uint8_t is_dolby;
	uint8_t is_el_exist;
	uint8_t is_second_field;
	uint32_t addr_y[VDP_ADDR_MAX];
	uint32_t addr_c[VDP_ADDR_MAX];

	struct vdp_hal_color_format color_fmt;

	struct vdp_hal_fb_size fb_size;
	struct vdp_hal_fb_addr fb_addr;

	/* display region */
	struct vdp_hal_region src_region;
	struct vdp_hal_region out_region;

	struct vdp_hal_fb_info cur_fb_info;

};

struct vdp_hal_disp_info {
	uint32_t pxl_len; /* 0x4209C */
	uint32_t h_start; /* 0x420A0 */
	uint32_t h_end;   /* 0x420A0 */
	uint32_t v_odd_start; /* 0x420A4 */
	uint32_t v_odd_end;   /* 0x420A4 */
	uint32_t v_even_start; /* 0x420A8 */
	uint32_t v_even_end;   /* 0x420A8 */
	uint32_t h_factor; /* 0x420B0 */
	uint32_t h_scl_on; /* 0x420B0 */
	uint32_t h_scl_linear; /* 0x420B0 */
	uint32_t h_delay ; /* 0x420E8 */
	uint32_t v_delay; /* 0x420E8 */
};

struct vdp_hal_hd_scl_info {
	bool hd_scl_on;
	int src_w;
	int out_w;
	int in_x_pos;
	int out_x_offset;
	int out_y_odd_pos;
	int out_y_odd_pos_e;
	int out_y_even_pos;
	int out_y_even_pos_e;
	int out_h_start;
	int out_h_end;
	HDMI_VIDEO_RESOLUTION res;
};
struct vdp_hal_offset {
	int32_t x_ofst; /*x offset*/
	int32_t y_ofst_odd; /*y offset odd*/
	int32_t y_ofst_even; /*y offset even*/
};

struct vdp_offset_table {
	struct vdp_hal_offset sub_video;
	struct vdp_hal_offset main_video;
	HDMI_VIDEO_RESOLUTION res;
};

enum VDP_OUT_MODE {
	VDP_OUT_480 = 0,
	VDP_OUT_576 = 1,
	VDP_OUT_720 = 2,
	VDP_OUT_1080 = 3,
	VDP_OUT_2160 = 4
};

enum VDP_DATA_PACK_MODE {
	VDP_8BIT_UFO = 2,
	VDP_8BIT_UNPACK = 3,
	VDP_10BIT_UFO = 6,
	VDP_10BIT_UNPACK = 7,
	VDP_10BIT_UFO_JUMP = 4,
};

void vdp_hal_init(unsigned char vdp_id);
void vdp_hal_isr(bool update_main, bool update_sub);
void vdp_hal_set_enable(uint8_t vdp_id, uint8_t enable);
void vdp_hal_config(unsigned char vdp_id, struct vdp_hal_config_info *p_config);
void vdp_hal_config_timing(unsigned char vdp_id,
	const struct disp_hw_resolution *res_info,
	int h_sta, int v_sta_odd, int v_sta_even);
void vdp_hal_set_dsd_config(uint8_t vdp_id,
	uint16_t src_height,
	uint16_t dst_height,
	uint16_t res_height);
void vdp_hal_get_info(unsigned char vdp_id,
	struct vdp_hal_data_info *vdp_hal,
	struct vdp_hal_disp_info *vdp_disp,
	struct vdo_sw_shadow *vdp_sw_shadow);
void vdp_hal_set_info(unsigned char vdp_id,
	struct vdp_hal_data_info *vdp_hal,
	struct vdp_hal_disp_info *vdp_disp,
	struct vdo_sw_shadow *vdp_sw_shadow);

void vdp_hal_set_ufo_addr(uint8_t vdp_id, struct vdp_hal_fb_addr *p_fb_addr);
void vdp_hal_set_src_color_format(uint8_t vdp_id,
struct vdp_hal_color_format *p_clr_fmt);

void vdp_hal_set_src_size(uint8_t vdp_id, uint32_t width, uint32_t height);
void vdp_hal_set_disp_config(uint8_t vdp_id,
	struct vdp_hal_region *p_src,
	struct vdp_hal_region *p_out);
#endif
