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

#ifndef _FMT_DEF_H_
#define _FMT_DEF_H_

/* ----------------------------------------------------------------------------- */
/* Include files */
/* ----------------------------------------------------------------------------- */
#include <linux/types.h>
#include <linux/printk.h>

#include "fmt_hw.h"

#define FMT_OK 0
#define FMT_PARAR_ERR -1
#define FMT_SET_ERR -2
#define FMT_GET_ERR -3
#define FMT_STATE_ERR -4
#define FMT_ALLOC_FAIL -5

#define DISP_FMT_MAIN 0
#define DISP_FMT_SUB 1
#define VDOUT_FMT (DISP_FMT_SUB + 1)
#define VDOUT_FMT_SUB (VDOUT_FMT + 1)

#define FMT_HW_PLANE_1    0
#define FMT_HW_PLANE_2    1
#define FMT_HW_PLANE_3    2

/* Resolution */
#define FMT_480I_WIDTH		720
#define FMT_480I_HEIGHT	480
#define FMT_480P_WIDTH		720
#define FMT_480P_HEIGHT	480
#define FMT_576I_WIDTH		720
#define FMT_576I_HEIGHT	576
#define FMT_576P_WIDTH		720
#define FMT_576P_HEIGHT	576
#define FMT_720P_WIDTH		1280
#define FMT_720P_HEIGHT	720
#define FMT_1080I_WIDTH	1920
#define FMT_1080I_HEIGHT	1080
#define FMT_1080P_WIDTH	1920
#define FMT_1080P_HEIGHT	1080
#define FMT_2160P_WIDTH  3840
#define FMT_2160P_HEIGHT	2160
#define FMT_2161P_WIDTH  4096
#define FMT_2161P_HEIGHT	2160



#define FMT_768P_WIDTH		1366
#define FMT_768P_HEIGHT	768
#define FMT_540P_WIDTH		1920
#define FMT_540P_HEIGHT	540
#define FMT_768I_WIDTH		1366
#define FMT_768I_HEIGHT	768
#define FMT_1536I_WIDTH	1366
#define FMT_1536I_HEIGHT	1536
#define FMT_720I_WIDTH		1280
#define FMT_720I_HEIGHT	720
#define FMT_1440I_WIDTH	1280
#define FMT_1440I_HEIGHT	1440

#define FMT_PANEL_AUO_B089AW01_WIDTH  1024
#define FMT_PANEL_AUO_B089AW01_HEIGHT  600

#define FMT_640_480_WIDTH	640
#define FMT_640_480_HEIGHT	480

enum FMT_TV_TYPE {
	FMT_TV_TYPE_NTSC = 0,
	FMT_TV_TYPE_PAL_M,
	FMT_TV_TYPE_PAL_N,
	FMT_TV_TYPE_PAL,
	FMT_TV_TYPE_PAL_1080P_24,
	FMT_TV_TYPE_PAL_1080P_25,
	FMT_TV_TYPE_PAL_1080P_30,
	FMT_TV_TYPE_NTSC_1080P_23_9,
	FMT_TV_TYPE_NTSC_1080P_29_9,
	FMT_TV_TYPE_720P3D,
	FMT_TV_TYPE_1080i3D
};


enum FMT_OUTPUT_FREQ_T {
	FMT_OUTPUT_FREQ_60 = 60,
	FMT_OUTPUT_FREQ_50 = 50,
	FMT_OUTPUT_FREQ_30 = 30,
	FMT_OUTPUT_FREQ_25 = 25,
	FMT_OUTPUT_FREQ_24 = 24,
	FMT_OUTPUT_FREQ_23_976 = 23,
	FMT_OUTPUT_FREQ_29_97 = 29,
};

enum FMT_3D_TYPE_T {
	FMT_3D_NO = 0,
	FMT_3D_FP,
	FMT_3D_SBS,
	FMT_3D_TAB,
	FMT_3D_SBSF,
	FMT_3D_MAX
};

enum FMT_LOG_LEVEL {
	FMT_LL_ERROR,
	FMT_LL_WARNING,
	FMT_LL_INFO,
	FMT_LL_DEBUG,
	FMT_LL_VERBOSE
};

enum FMT_STATUS {
	FMT_STA_UNUSED,
	FMT_STA_USED
};

enum DISP_FMT {
	DISP_FMT1 = 0,
	DISP_FMT2 = 1,
	DISP_FMT_CNT = 2
};

struct vdout_fmt_t {
	enum FMT_STATUS status;
	bool reset_in_vsync;
	bool shadow_en;
	bool shadow_trigger;
	uint64_t *reg_mode;
	uintptr_t hw_fmt_base;
	union vdout_fmt_union_t *sw_fmt_base;
};

struct disp_fmt_t {
	enum FMT_STATUS status;
	bool reset_in_vsync;
	bool shadow_en;
	bool shadow_trigger;
	bool is_sec;
	uint64_t *reg_mode;
	uintptr_t hw_fmt_base;
	union disp_fmt_union_t *sw_fmt_base;
};

struct fmt_context {
	uintptr_t vdout_fmt_reg_base;
	uintptr_t disp_fmt_reg_base[DISP_FMT_CNT];	/*fmt1, fmt2 */
	uintptr_t io_reg_base;

	struct vdout_fmt_t vdout_fmt;
	struct vdout_fmt_t sub_vdout_fmt;
	struct disp_fmt_t main_disp_fmt;
	struct disp_fmt_t sub_disp_fmt;

	struct mutex lock;
	bool inited;
	uint32_t fmt_log_level;
	HDMI_VIDEO_RESOLUTION res;
};


extern struct fmt_context fmt;

#define FMT_LOG(level, format...) \
do { \
	if (level <= fmt.fmt_log_level) \
		pr_err("[FMT] "format); \
} while (0)

#define FMT_LOG_E(format...) FMT_LOG(FMT_LL_ERROR, "error: "format)
#define FMT_LOG_W(format...) FMT_LOG(FMT_LL_WARNING, format)
#define FMT_LOG_I(format...) FMT_LOG(FMT_LL_INFO, format)
#define FMT_LOG_D(format...) FMT_LOG(FMT_LL_DEBUG, format)
#define FMT_LOG_V(format...) FMT_LOG(FMT_LL_VERBOSE, format)

#define FMT_FUNC()  FMT_LOG_D("%s LINE:%d\n", __func__, __LINE__)

#define CHECK_STATE_RET_VOID(state) \
do { \
	if (state == FMT_STA_UNUSED) { \
		FMT_LOG_E("%s state error\n", __func__); \
	} \
} while (0)

#define CHECK_STATE_RET_INT(state) \
do { \
	if (state == FMT_STA_UNUSED) { \
		FMT_LOG_E("%s state error\n", __func__); \
	} \
} while (0)


#endif
