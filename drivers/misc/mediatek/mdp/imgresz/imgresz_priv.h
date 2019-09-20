/*
 * Copyright (c) 2016-2017 MediaTek Inc.
 * Author: Yong Wu <yong.wu@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef MTK_IMGRESZ_PRIV_H
#define MTK_IMGRESZ_PRIV_H

#define DRV_VERIFY_SUPPORT	1

#include "imgresz.h"

#define loginfo(lvl, string, args...) do {\
		if (imgresz_log_level & lvl)\
			pr_info("[ImgResz]"string, ##args);\
		} while (0)
#define logwarn(string, args...) pr_info("[ImgResz]"string, ##args)

enum IMGRESZ_CHIP_VER {
	IMGRESZ_CURR_CHIP_VER_BASE = 0,
	IMGRESZ_CURR_CHIP_VER_8697,
	IMGRESZ_CURR_CHIP_VER_8695,
	IMGRESZ_CURR_CHIP_VER_MAX
};

enum IMGRESZ_RESAMPLE_METHOD_T {
	IMGRESZ_RESAMPLE_METHOD_M_TAP,/* multi-tap */
	IMGRESZ_RESAMPLE_METHOD_4_TAP,
	IMGRESZ_RESAMPLE_METHOD_8_TAP
};

enum IMGRESZ_BUF_MAIN_FORMAT_T {
	IMGRESZ_BUF_MAIN_FORMAT_Y_C,
	IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR,
	IMGRESZ_BUF_MAIN_FORMAT_INDEX,
	IMGRESZ_BUF_MAIN_FORMAT_ARGB,
	IMGRESZ_BUF_MAIN_FORMAT_AYUV,
};

enum IMGRESZ_YUV_FORMAT_T {
	IMGRESZ_YUV_FORMAT_420,
	IMGRESZ_YUV_FORMAT_422,
	IMGRESZ_YUV_FORMAT_444
};

enum IMGRESZ_ARGB_FORMAT_T {
	IMGRESZ_ARGB_FORMAT_0565,
	IMGRESZ_ARGB_FORMAT_1555,
	IMGRESZ_ARGB_FORMAT_4444,
	IMGRESZ_ARGB_FORMAT_8888,
};

struct imgresz_buf_format {
	enum IMGRESZ_BUF_MAIN_FORMAT_T mainformat;
	enum IMGRESZ_YUV_FORMAT_T yuv_format;
	enum IMGRESZ_ARGB_FORMAT_T argb_format;

	unsigned int h_sample[3];
	unsigned int v_sample[3];
	bool block;
	bool progressive;
	bool top_field;
	bool bit10;
	bool jump_10bit;
};

#endif
