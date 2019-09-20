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


#ifndef DISP_PMX_IF_H
#define DISP_PMX_IF_H

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/device.h>

#include "hdmitx.h"

/** maximum number of plane mixer */
#define PMX_MAX_NS                  1
#define PMX_1                       0

#define PMX_OK                  0
#define PMX_PARAM_ERR           -1
#define PMX_SET_FAIL            -2
#define PMX_GET_FAIL			-3
#define PMX_DTS_FAIL			-4

#define PMX_MAIN 0
#define PMX_PIP  1
#define PMX_OSD1 2
#define PMX_OSD2 3
#define PMX_OSD3 4

#define PMX_DRV_NAME  "disp_drv_pmx"

#define PMX_ERR(fmt, arg...)  pr_err("[PMX] error:"fmt, ##arg)
#define PMX_WARN(fmt, arg...)  pr_warn("[PMX]:"fmt, ##arg)
#define PMX_INFO(fmt, arg...)  pr_info("[PMX]:"fmt, ##arg)
#define PMX_LOG(fmt, arg...)  pr_debug("[PMX]:"fmt, ##arg)
#define PMX_DEBUG(fmt, arg...) pr_debug("[PMX]:"fmt, ##arg)

#define PMX_FUNC() pr_debug("[PMX] func: %s\n", __func__)

extern int hdmi_internal_video_config(HDMI_VIDEO_RESOLUTION vformat);
extern int hdmi_internal_audio_config(HDMI_AUDIO_FORMAT aformat);

#endif

