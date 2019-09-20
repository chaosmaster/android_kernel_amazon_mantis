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

#ifndef __DISP_DOVI_COMMON_H__
#define __DISP_DOVI_COMMON_H__


enum dovi_signal_format_t {
	DOVI_FORMAT_DOVI = 0,
	DOVI_FORMAT_HDR10 = 1,
	DOVI_FORMAT_SDR = 2,
	DOVI_FORMAT_DOVI_LOW_LATENCY = 3,
};

enum dovi_enable_type_t {
	DOVI_INOUT_FORMAT_CHANGE = 2,
	DOVI_PRIORITY_MODE_CHANGE = 3,
	DOVI_RESOLUTION_CHANGE = 4,
};

struct dovi_info_t {
	enum dovi_signal_format_t out_format;
	bool is_graphic_mode;
	bool is_low_latency;
	unsigned char *vsvdb_edid;
};

enum dovi_signal_range_t {
	DOVI_SIG_RANGE_SMPTE = 0,	/* head range */
	DOVI_SIG_RANGE_FULL = 1,	/* full range */
	DOVI_SIG_RANGE_SDI = 2	/* PQ */
};

struct dovi_video_info_t {
	int width;   /**<@brief Width of the video frame   */
	int height;  /**<@brief Height of the video frame   */
	enum dovi_signal_format_t input_format;
	enum dovi_signal_range_t src_yuv_range;
	int src_chroma_format;
	int src_bit_depth;
	int src_fps;
	int orignalsrc_width;	/* for scaling case */
	int orignalsrc_heigth;
};

struct dovi_graphic_info_t {
	int graphic_min_lum, graphic_max_lum;
	int f_graphic_on;
};

#endif				/* __DOVI_COMMON_H__ */
