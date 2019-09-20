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

#ifndef __DOVI_VFY_DRV_H__
#define __DOVI_VFY_DRV_H__


enum dovi_idk_version {
	DOVI_IDK_230 = 0,
	DOVI_IDK_241 = 1,
	DOVI_IDK_242 = 2,
	DOVI_IDK_MAX
};

enum dovi_resolution {
	DOVI_480P = 0,
	DOVI_576P = 1,
	DOVI_720P = 2,
	DOVI_1080P,
	DOVI_2160P,
	DOVI_RESOLUTION_MAX
};

enum dovi_format {
	DOVI_SDR = 0,
	DOVI_HDR10 = 1,
	DOVI_DOVI = 2,
	DOVI_FORMAT_MAX
};

enum dovi_unit_id {
	DOVI_REGISTER_TEST = 0,
	DOVI_DOUBLE_LAYER_BYPASS_CSC_CVM = 1,
	DOVI_SIGNLE_LAYER_BYPASS_CSC_CVM = 2,
	DOVI_UHD_DOUBLE_LAYER,
	DOVI_UHD_SIGNLE_LAYER,
	DOVI_FHD_DOUBLE_LAYER, /* 5 */
	DOVI_SD_DOUBLE_LAYER,
	DOVI_SHADOW_TEST,
	DOVI_EFUSE_TEST,
	DOVI_SWITCH_ON_TEST,
	DOVI_SWITCH_OFF_TEST, /* 10 */
	DOVI_SD_IPT_BYPASS_MODE,
	DOVI_SD_IPT_TUNNELED_MODE,
	DOVI_SD_HDR10_MODE,
	DOVI_SD_SDR10_MODE,
	DOVI_SD_DITHER_BYPASS, /* 15 */
	DOVI_FHD_24HZ_DOUBLE_LAYER,
	DOVI_HD_DOUBLE_LAYER,
	DOVI_SD_IPT_TUNNELED_MODE_MUTE_TEST,
	DOVI_UHD_IPT_TUNNELED_MODE, /* 19 */
	DOVI_UNIT_ID_MAX
};

struct dovi_unit_info_t {
	enum dovi_idk_version idk_version; /* 2.3, 2.4.1, 2.4.2*/
	enum dovi_resolution resolution; /* 0 480p, 1 576p, 2 720p, 3 4080p, 4 2160p */
	enum dovi_format in_format;
	enum dovi_format out_format;
	uint32_t graphic_on;
	enum dovi_unit_id unit_id;
};

uint32_t dovi_unit_register_test(void);
int dovi_unit_set_path(struct dovi_unit_info_t *p_dovi_unit_info);
int dovi_unit_test(enum dovi_unit_id unit_id);


extern void disp_dovi_isr(void);
#endif
