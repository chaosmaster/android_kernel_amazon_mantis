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

#ifndef __DISP_INFO_H__
#define __DISP_INFO_H__

#include "linux/types.h"
#include "hdmitx.h"

#ifndef DOVI_MD_MAX_LEN
#define DOVI_MD_MAX_LEN 1024
#endif


#define DISP_BUFFER_MAX 4


enum MTK_DISPIF_TYPE {
	DISPIF_TYPE_DBI,
	DISPIF_TYPE_DPI,
	DISPIF_TYPE_DSI,
	DISPIF_TYPE_DPI0,
	DISPIF_TYPE_DPI1,
	DISPIF_TYPE_DSI0,
	DISPIF_TYPE_DSI1,
	HDMI,
	HDMI_SMARTBOOK,
	MHL,
	DISPIF_TYPE_CVBS,
	SLIMPORT
};

enum MTK_DISPIF_DEVICE_TYPE {
	MTK_DISPIF_PRIMARY_LCD = 0,
	MTK_DISPIF_HDMI,
	MTK_DISPIF_CVBS,
	MTK_MAX_DISPLAY_COUNT
};

enum MTK_HDR_TYPE_FLAG {
	MTK_HDR_TYPE_DOLBY_VISION = 1 << 0,
	MTK_HDR_TYPE_HDR10 = 1 << 1,
	MTK_HDR_TYPE_HLG = 1 << 2
};

struct mtk_disp_info {
	uint32_t display_id;
	uint32_t isHwVsyncAvailable;
	enum MTK_DISPIF_TYPE displayType;
	uint32_t displayWidth;
	uint32_t displayHeight;
	uint32_t displayFormat;
	uint32_t vsyncFPS;
	uint32_t physicalWidth;
	uint32_t physicalHeight;
	uint32_t isConnected;
	uint32_t lcmOriginalWidth;
	uint32_t lcmOriginalHeight;
	uint32_t disp_resolution;
};

struct mtk_disp_hdr_info_t {
	uint32_t colorPrimaries; /* colour_primaries emum */
	uint32_t transformCharacter; /* transfer_characteristics emum */
	uint32_t matrixCoeffs; /* matrix_coeffs emum */
	uint32_t displayPrimariesX[3]; /* display_primaries_x */
	uint32_t displayPrimariesY[3]; /* display_primaries_y */
	uint32_t whitePointX; /* white_point_x */
	uint32_t whitePointY; /* white_point_y */
	uint32_t maxDisplayMasteringLuminance; /* max_display_mastering_luminance */
	uint32_t minDisplayMasteringLuminance; /* min_display_mastering_luminance */
	uint32_t maxCll;
	uint32_t maxFall;
};

enum DISP_LAYER_TYPE {
	DISP_LAYER_VDP,
	DISP_LAYER_OSD,
	DISP_LAYER_SIDEBAND,
	DISP_LAYER_AEE,
	DISP_LAYER_UNKNOW
};

enum DISP_BUFFER_SOURCE {
	/* ion buffer */
	DISP_BUFFER_ION = 0,
	/* dim layer, const alpha */
	DISP_BUFFER_ALPHA = 1,
	/* mva buffer */
	DISP_BUFFER_MVA = 2,
};


enum DISP_MGR_USER {
	DISP_USER_HWC,
	DISP_USER_AVSYNC,
	DISP_USER_AEE,
	DISP_USER_PANDISP,
	DISP_USER_INVALID
};

#define MAKE_DISP_HW_FORMAT_ID(id, bpp)  (((id) << 8) | (bpp))

enum DISP_HW_COLOR_FORMAT {
	DISP_HW_COLOR_FORMAT_UNKNOWN = 0,
	DISP_HW_COLOR_FORMAT_RGB565 = MAKE_DISP_HW_FORMAT_ID(1, 0),
	DISP_HW_COLOR_FORMAT_RGB888 = MAKE_DISP_HW_FORMAT_ID(1, 1),
	DISP_HW_COLOR_FORMAT_BGR888 = MAKE_DISP_HW_FORMAT_ID(1, 2),
	DISP_HW_COLOR_FORMAT_ARGB8888 = MAKE_DISP_HW_FORMAT_ID(1, 3),
	DISP_HW_COLOR_FORMAT_ABGR8888 = MAKE_DISP_HW_FORMAT_ID(1, 4),
	DISP_HW_COLOR_FORMAT_RGBA8888 = MAKE_DISP_HW_FORMAT_ID(1, 5),
	DISP_HW_COLOR_FORMAT_BGRA8888 = MAKE_DISP_HW_FORMAT_ID(1, 6),
	DISP_HW_COLOR_FORMAT_YUV420_BLOCK = MAKE_DISP_HW_FORMAT_ID(2, 0),
	DISP_HW_COLOR_FORMAT_YUV422_BLOCK = MAKE_DISP_HW_FORMAT_ID(2, 1),
	DISP_HW_COLOR_FORMAT_YUV420_RASTER = MAKE_DISP_HW_FORMAT_ID(2, 2),
	DISP_HW_COLOR_FORMAT_YUV422_RASTER = MAKE_DISP_HW_FORMAT_ID(2, 3),
	DISP_HW_COLOR_FORMAT_BPP_MASK = 0xFF,
};

struct mtk_disp_range {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
};

enum DISP_VIDEO_TYPE {
	VIDEO_YUV_BT601_FULL = 0,
	VIDEO_YUV_BT601	= 1,
	VIDEO_YUV_BT709  = 2
};

enum DISP_ASPECT_RATIO {
	DISP_ASPECT_RATIO_4_3 = 0,
	DISP_ASPECT_RATIO_16_9
};

enum DISP_ALPHA_TYPE {
	DISP_ALPHA_ONE = 0,
	DISP_ALPHA_SRC = 1,
	DISP_ALPHA_SRC_INVERT = 2,
	DISP_ALPHA_INVALID = 3,
};

enum VIDEO_LAYER_ID {
	DISP_MAIN_VIDEO			= 0,
	DISP_SUB_VIDEO			= 1,
	DISP_MAX_VIDEO			= 2
};

enum DISP_DR_TYPE_T {
	DISP_DR_TYPE_SDR = 0,
	DISP_DR_TYPE_HDR10,
	DISP_DR_TYPE_DOVI,
	DISP_DR_TYPE_PHLP,
	DISP_DR_TYPE_PHLP_RESVERD
};

struct mtk_disp_hdr10_md_t {
	uint32_t ui2_DisplayPrimariesX[3];
	uint32_t ui2_DisplayPrimariesY[3];
	uint32_t ui2_WhitePointX;
	uint32_t ui2_WhitePointY;
	uint32_t ui2_MaxDisplayMasteringLuminance;
	uint32_t ui2_MinDisplayMasteringLuminance;
	uint32_t ui2_MaxCLL;
	uint32_t ui2_MaxFALL;
};

struct mtk_disp_dovi_md_t {
	uint64_t pts;
	uint32_t len;
	bool svp;
	uint32_t sec_handle;
	void *addr;
};

struct mtk_vdp_dovi_md_t {
	uint64_t pts;
	uint32_t len;
	bool svp;
	uint32_t sec_handle;
	uint8_t buff[DOVI_MD_MAX_LEN];
};

union mtk_disp_hdr_md_t {
	struct mtk_disp_hdr10_md_t hdr10_metadata;
	struct mtk_vdp_dovi_md_t dovi_metadata;
};

struct mtk_disp_hdr_md_info_t {
	enum DISP_DR_TYPE_T dr_range;
	union mtk_disp_hdr_md_t metadata_info;
	uint32_t enable;
	VID_PLA_HDR_METADATA_INFO_T hdr10_info;
};

struct mtk_disp_buffer {
	unsigned int layer_order;
	unsigned int layer_id;
	unsigned int layer_enable;

	enum DISP_LAYER_TYPE type;
	enum DISP_BUFFER_SOURCE buffer_source;
	enum DISP_HW_COLOR_FORMAT src_fmt;

	bool color_key_en;
	int src_color_key;

	bool alpha_en;
	int alpha;

	/* display region */
	struct mtk_disp_range src;
	struct mtk_disp_range crop;
	struct mtk_disp_range tgt;

	/* buffer address and fence */
	int ion_fd;
	bool secruity_en;
	int acquire_fence_fd;
	int release_fence_fd;
	int present_fence_fd;

	/* for pre-multiple alpha */
	enum DISP_ALPHA_TYPE src_alpha;
	enum DISP_ALPHA_TYPE dst_alpha;

	/* if buffer_info is video plane, should fill video info */
	bool is_ufo;
	bool is_dolby;
	bool is_progressive;
	bool is_10bit;
	bool is_10bit_lbs2bit_tile_mode;
	bool is_hdr;
	bool is_hdr10plus;
	bool is_pack_mode;
	bool is_bt2020;
	bool is_seamless;
	bool is_jumpmode;
	uint64_t pts;
	unsigned int fps;
	enum DISP_ASPECT_RATIO aspect_ratio;
	enum DISP_VIDEO_TYPE video_type;
	struct mtk_disp_hdr_info_t hdr_info;
	enum DISP_DR_TYPE_T dr_range;

	/*video buffer offset*/
	unsigned int ofst_y;
	unsigned int ofst_c;
	unsigned int ofst_c_len;
	unsigned int ofst_y_len;
	uint32_t buffer_size;

	unsigned int meta_data_size;
	struct mtk_disp_dovi_md_t dolby_info;

	void *meta_data;
	void *src_base_addr;
	void *src_phy_addr;
	uint32_t res_mode;
};

struct mtk_disp_config {
	enum DISP_MGR_USER user;
	unsigned int buffer_num;
	struct mtk_disp_buffer buffer_info[DISP_BUFFER_MAX];
};

struct mtk_disp_vdp_cap {
	struct mtk_disp_range src;
	struct mtk_disp_range crop;
	struct mtk_disp_range tgt;
	enum DISP_HW_COLOR_FORMAT src_fmt;
	bool is_progressive;
	bool is_10bit;
	bool need_resizer;
};

struct mtk_disp_hdmi_cap {
	unsigned int hdr_type;
	unsigned int hdr_content_max_luminance;
	unsigned int hdr_content_max_frame_average_luminance;
	unsigned int hdr_content_min_luminance;
	long long supported_resolution;
	unsigned int disp_resolution;
	unsigned char screen_width;
	unsigned char screen_height;
};

struct mtk_disp_layer_info {
	unsigned int display_id;
	unsigned int supported_ui_num;
	unsigned int supported_video_num;
};

#define MTK_DISP_IOW(num, dtype)     _IOW('O', num, dtype)
#define MTK_DISP_IOR(num, dtype)     _IOR('O', num, dtype)
#define MTK_DISP_IOWR(num, dtype)    _IOWR('O', num, dtype)
#define MTK_DISP_IO(num)             _IO('O', num)

#define MTK_DISP_IOCTL_GET_INFO		MTK_DISP_IOR(0xd0, struct mtk_disp_info)
#define MTK_DISP_IOCTL_WAIT_VSYNC	MTK_DISP_IO(0xd1)
#define MTK_DISP_IOCTL_SET_INPUT_BUFFER	MTK_DISP_IOWR(0xd2, struct mtk_disp_config)
#define MTK_DISP_IOCTL_PLAY			MTK_DISP_IO(0xd3)
#define MTK_DISP_IOCTL_STOP			MTK_DISP_IO(0xd4)
#define MTK_DISP_IOCTL_SUSPEND		MTK_DISP_IO(0xd5)
#define MTK_DISP_IOCTL_RESUME		MTK_DISP_IO(0xd6)
#define MTK_DISP_IOCTL_VDP_CAP		MTK_DISP_IOWR(0xd7, struct mtk_disp_vdp_cap)
#define MTK_DISP_IOCTL_HDMI_CAP		MTK_DISP_IOR(0xd8, struct mtk_disp_hdmi_cap)
#define MTK_DISP_IOCTL_GET_LAYER_INFO	MTK_DISP_IOR(0xd9, struct mtk_disp_layer_info)
#endif
