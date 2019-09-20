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

#ifndef __DOVI_CONTROL_PATH_DRV_H
#define __DOVI_CONTROL_PATH_DRV_H

#define MAX_FILENAME_LENGTH 256

enum signal_fmt_t {
	SIGNAL_FORMAT_INVALID = -1,
	SIGNAL_FORMAT_DOVI = 0,
	SIGNAL_FORMAT_HDR10 = 1,
	SIGNAL_FORMAT_SDR = 2
};

enum signal_range_t {
	SIGNAL_RANGE_SMPTE = 0,
	SIGNAL_RANGE_FULL = 1,
	SIGNAL_RANGE_SDI = 2
};

enum graphic_format_t {
	GRAPHIC_SDR_YUV = 0,	/* BT.709 YUV BT1886 */
	GRAPHIC_SDR_RGB = 1,	/* BT.709 RGB BT1886 */
	GRAPHIC_HDR_YUV = 2,	/* BT.2020 YUV PQ */
	GRAPHIC_HDR_RGB = 3	/* BT.2020 RGB PQ */
};

enum chroma_format_t {
	CHROMA_FORMAT_P420 = 0,
	CHROMA_FORMAT_UYVY = 1,
	CHROMA_FORMAT_P444 = 2
};

enum pri_mode_t {
	V_PRIORITY = 0,
	G_PRIORITY = 1
};

/**
*    @typedef cp_cli_param_t
*    @brief Command line parameters.
*/
struct cp_param_t {
	int width;   /**<@brief Width of the video frame   */
	int height;  /**<@brief Height of the video frame   */
	enum signal_fmt_t input_format;
	enum signal_fmt_t output_format;
	int md_filter;
	int target_min_lum, target_max_lum;
	int graphic_min_lum, graphic_max_lum;
	enum signal_range_t src_yuv_range;
	int src_chroma_format;
	int src_bit_depth;
	int f_graphic_on;
	int g_bit_depth;
	enum graphic_format_t g_format;
	enum chroma_format_t g_cf;
	enum pri_mode_t priority_mode;
	int src_fps;
	int use_ll;
	int ll_rgb_desired;
	int dovi2hdr10_mapping;
	char vsvdb_file[MAX_FILENAME_LENGTH];
	unsigned char vsvdb_hdmi[0x1A];
};

#endif
