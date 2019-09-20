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



#ifndef _DISP_VDP_VSYNC_H_
#define _DISP_VDP_VSYNC_H_
/*#include <mach/sync_write.h>*/
/*#include <mach/mt_typedefs.h>*/
#include <linux/types.h>
#include "disp_vdp_if.h"
#include "vdp_hal.h"
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
#include "hdmitx.h"

extern void vVdpSetHdrMetadata(bool enable, VID_PLA_HDR_METADATA_INFO_T hdr_metadata);
extern void vHdrEnable(bool fgEnable);
extern void vBT2020Enable(bool fgEnable);
extern void vDolbyHdrEnable(bool fgEnable);
extern void vLowLatencyDolbyVisionEnable(bool fgEnable);
extern void vHdr10PlusEnable(bool fgEnable);
extern void vSetStaticHdrType(char bType);
extern void vHdr10PlusVSIFEnable(bool fgEnable, unsigned int forcedHdrType, int dolbyOpFmt);
#endif


extern uint32_t osd_enable;
extern struct ion_client *vdp_ion_client;
extern struct video_layer_info *video_layer;
extern struct video_playback_info video_play_info;
extern struct disp_hw_common_info disp_common_info;
extern struct mtk_disp_hdr_md_info_t vdp_hdr_info[VDP_MAX];
extern uint32_t ui_force_hdr_type;

#define NORMAL_PLAYBACK_DURATION 1
#define INTERLACE_PLAYBACK_DUARATION 3

void vdp_vsync_init(void);
void vdp_wakeup_routine(void);
void vdp_update_registers(void);
void vdp_isr(void);
void vdp_disable_active_zone(struct video_layer_info *layer_info);
struct video_buffer_info *vdp_get_buf_info(enum VIDEO_LAYER_ID id);
void vdp_ion_free_handle(struct ion_client *client, struct ion_handle *handle);
void release_buf_info(struct list_head *list, unsigned int layer_id);
int vdp_check_dsd_available(unsigned char vdp_id,
	struct vdp_hal_config_info *hal_config);
void vdp_update_dovi_path_delay(int dsd_en);

extern int dovi_core1_hal_is_support(void);
extern uint32_t dolby_path_enable;
void vdp_dovi_path_disable(void);
void vdp_dovi_path_enable(void);
extern uint32_t dolby_path_enable_cnt;
void vdp_dovi_set_out_format(struct disp_hw_common_info *info,
	struct video_buffer_info *buf);
extern HDMI_VIDEO_RESOLUTION old_resolution;
extern enum dovi_signal_format_t dolby_out_format;
extern uint32_t dolby_core2_enable;

void vdp_update_dovi_path_delay(int dsd_en);

#endif
