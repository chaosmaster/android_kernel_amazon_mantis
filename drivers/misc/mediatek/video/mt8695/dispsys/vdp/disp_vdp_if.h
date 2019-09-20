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



#ifndef __VDP_IF_H__
#define __VDP_IF_H__

#include "disp_info.h"

#include <linux/mutex.h>
#include <linux/list.h>

#include "hdmitx.h"
#include "disp_hw_mgr.h"

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
#define VIDEO_DISPLAY_SECURE_ENABLE 1
#else
#define VIDEO_DISPLAY_SECURE_ENABLE 0
#endif

#define VIDEO_USE_HW_SHADOW	1
#define VIDEO_REDUCE_BUFFER 1

#define VIDEO_LAYER_MAX_COUNT	2
#define VIDEO_DISP_TIMELINE_COUNT VIDEO_LAYER_MAX_COUNT
#define VDP_MEMSET(addr, value, len)  memset((addr), value, len)

/* Maximum number of video plane */
#define VDP_MAX_NS				3
#define VDP_HW_NS               2
#define VDP_NS                  3


#define MAIN_VIDEO_INDEX			0
#define SUB_VIDEO_INDEX			1

#define TIME_BASE              599400
extern unsigned int vdp_dbg_level;
#define VDP_FUNC_LOG (1 << 0)
#define VDP_FLOW_LOG (1 << 1)
#define VDP_COLOR_FORMAT_LOG (1 << 2)
#define VDP_FB_FLOW_LOG (1 << 3)
#define VDP_RESOLUTION_LOG (1 << 4)
#define VDP_AVSYNC_LOG (1 << 6)
#define VDP_FENCE_LOG (1 << 7)
#define VDP_FENCE1_LOG (1 << 8)
#define VDP_FENCE2_LOG (1 << 9)
#define VDP_CAPTURE_LOG (1 << 10)
#define VDP_DOVI_LOG (1 << 11)
#define VDP_HDR10PLUS_LOG (1 << 12)


/* debug level define */
#define VDB_VSYNC_DISP_LOG            0
#define VDB_TELECINE_LOG              1
#define VDB_ISR_DISP_LOG              2
#define VDB_CHK_32_LOG                3
#define VDB_TELECINE_PARAM_LOG        4
#define VDB_TELECINE_22_LOG           5
#define VDB_AVSYNC_LOG                6
#define VDB_CC_LOG                    7
#define VDB_PLANE_ENABLE_LOG          8
#define VDB_CPS_LOG                   9
#define VDB_REGION_SET_LOG            10
#define VDB_CAPTURE_LOG               11
#define VDB_INTERLACE_LOG             12
#define VDB_AVSYNC_LOG1               13
#define VDB_ISR1_DISP_LOG             14
#define VDB_STC_ISR_LOG               15
#define VDB_ISR_DISP_LOG2             16
#define VDB_MA_LOG                    17
#define VDB_DEINT_LOG                 18
#define VDB_PAL_32_LOG                19
#define VDB_PICINFO_LOG               20
#define VDB_FRC_LOG                   21
#define VDB_CNT_HIST_LOG              22
#define VDB_RESIZER_LOG               23
#define VDB_TMP1_LOG                  24
#define VDB_PIP_METADATA_LOG          25
#define VDB_TELECINE_32_LOG           26
#define VDB_MA_HD_LOG                 27
#define VDB_IF_LOG                    28
#define VDB_ISR_TIME_LOG              29
#define VDB_LETTERBOX_LOG_1           30
#define VDB_LETTERBOX_LOG_2           31
#define VDB_TEN_THOUSAND_CENT_LOG     32
#define VDB_INTERFACE_LOG             33
#define VDB_MW_INTERFACE_LOG          34
#define VDB_VERIFY_ISR_TEST           35
#define VDB_VERIFY_ROUTINE_TEST       36
#define VDB_VERIFY_ADDRESS_FLOW       37
#define VDB_REGION_SET_LOG_2          39
#define VDB_EX_REGION_SET_LOG         40
#define VDB_DESIRED_RESOLUTION_LOG    41
#define VDB_GET_FB_LOG                42
#define VDB_RELEASE_FB_LOG            43
#define VDB_RESIZER_LOG_2             44
#define VDB_AVSYNC_LOG2               45
#define VDB_AVSYNC_LOG3               46
#define VDB_CMD_LOG                   47
#define VDB_DROP_FB_LOG               48
#define VDB_SHOW_AU_FB_COUNT_LOG      49
#define VDB_MVDO_POSITION_LOG         51
#define VDB_START_LINE_LOG            52
#define VDB_VFY_INTERFACE_LOG         50
#define VDB_VFY_AVSYNC_LOG            51
#define VDB_START_LINE_LOG            52
#define VDB_AVSYNC_LOG4               53
#define VDB_AVSYNC_LOG5               54
#define VDB_DIRECT_DISPLAY_LOG        55
#define VDB_AVSYNC_LOG6               56
#define VDB_INTERLACE_LOG2            57
#define VDB_INTERLACE_LOG3            58
#define VDB_VDO3_TEST                 59
#define VDB_DEFAULT_LOG               60
#define VDB_KEEP_QUALITY_AFTER_FLUSH_LOG 61
#define VDB_LOG_SIMPLFY               62
#define VDB_MAX_LOG                   63

#define VDP_DRV_NAME  "disp_drv_vdp"

/*Error code define */
#define VDP_OK				0
#define VDP_FAIL			-1
#define VDP_DTS_ERROR			-2
#define VDP_CREATE_FENCE_FAIL		-3
#define VDP_GET_ION_HND_FAIL		-4
#define VDP_GET_PHY_FAIL		-5
#define VDP_INVALID_INDEX		-6
#define VDP_NOT_START			-7
#define VDP_CREATE_SESSION_FAIL		-8
#define VDP_CREATE_MEMROY_SESSION_FAIL	-9
#define VDP_SESSION_NOT_CREATE		-10
#define VDP_SERVICE_CALL_FAIL		-11
#define VDP_DESTROY_SESSION_FAIL	-12
#define VDP_COPY_HDR10_PLUS_META_FAIL	-13


#define VDP_NO_ION_FD			((int)(~0U>>1))

#define VDP_INVALID_ION_FD		(-1)
#define VDP_INVALID_FENCE_FD		(-1)


#define VDP_INFO_EN_UPDATE		(1 << 0)
#define VDP_INFO_IN_REGION_UPDATE	(1 << 1)
#define VDP_INFO_OUT_REGION_UPDATE	(1 << 2)

#define vdp_printf(level, string, args...) do { \
	if (vdp_dbg_level & (level)) { \
		DISP_LOG_I(string, ##args); \
	} \
} while (0)


#define VDP_HDR10PLUS_GRAPHIC_OVERLAY_FLAG  (1 << 0)
#define VDP_HDR10PLUS_VISF_TIMING_MODE      (1 << 1)

enum BUFFER_STATE {
	BUFFER_CREATE,
	BUFFER_INSERT,
	BUFFER_REG_CONFIGED,
	BUFFER_REG_UPDATED,
	BUFFER_READ_DONE,
	BUFFER_DROPPED
};
enum VDP_LAYER_STATE {
	VDP_LAYER_IDLE,
	VDP_LAYER_RUNNING,
	VDP_LAYER_STOPPING,
	VDP_LAYER_STOPPED,
	VDP_LAYER_START,
};

enum HDR10_TYPE_ENUM {
	HDR10_TYPE_NONE = 0,
	HDR10_TYPE_ST2084 = 1,
	HDR10_TYPE_HLG = 2,
	HDR10_TYPE_PLUS = 3,
	HDR10_TYPE_MAX,
};



struct video_buffer_info {
	struct list_head list;
	/* buffer displayed on main path or sub path */
	unsigned int layer_id;

	/* passed from HWC */
	unsigned int layer_enable;

	/* secure buffer */
	bool secruity_en;

	enum BUFFER_STATE buf_state;

	/* no use. could used to dump vdp buffer. */
	void *src_base_addr;

	/* for normal buffer: value = MVA.
	** for secure buffer: value = secure handle.
	*/
	unsigned int src_phy_addr;

	/* for YC separate case. no use any more. */
	unsigned int src_phy_addr2;

	/* buffer format: not include UFO/10bit */
	enum DISP_HW_COLOR_FORMAT src_fmt;

	/* source buffer size */
	struct mtk_disp_range src;

	/* crop region */
	struct mtk_disp_range crop;

	/* display region */
	struct mtk_disp_range tgt;

	/* buffer ion handle converted from ion_fd */
	struct ion_handle *ion_hnd;

	/* for YC separate case. no use any more. */
	struct ion_handle *ion_hnd2;

	/* buffer ion_fd pass from HWC */
	int ion_fd;

	/* passed from HWC, VDP need to wait before use this buffer */
	int acquire_fence_fd;

	/* return to HWC, VDP should signal after release this buffer */
	int release_fence_fd;

	/* current buffer fence value */
	int current_fence_index;

	/* if buffer_info is video plane, should fill video info */
	bool is_ufo;
	bool is_dolby;
	bool is_dolby_graphic_mode;
	bool is_dolby_low_latency;
	bool is_dolby_vs10;
	bool is_interlace;
	bool is_10bit;
	bool is_10bit_lbs2bit_tile_mode;
	enum HDR10_TYPE_ENUM hdr10_type;
	bool is_pack_mode;
	bool is_bt2020;
	bool is_seamless;
	bool is_jumpmode;
	uint64_t pts;
	unsigned int fps;
	enum DISP_ASPECT_RATIO aspect_ratio;
	enum DISP_VIDEO_TYPE video_type;

	/* meta data */
	unsigned int meta_data_size;
	void *meta_data;

	/* HDR10 metadata for HDMI use. */
	VID_STATIC_HDMI_MD_T hdr10_info;

	/* dolby HDR metadata for Dolby use. */
	struct mtk_disp_hdr_md_info_t hdr_info;

	/* video buffer Addr offset
	** baseAddr is converted from ion fd / secure Handle.
	** Y buffer = baseAddr + ofst_y
	** C buffer = baseAddr + ofst_c
	** Ylen buffer = baseAddr + ofst_y_len
	** Clen buffer = baseAddr + ofst_c_len
	*/
	unsigned int ofst_y;
	unsigned int ofst_c;
	unsigned int ofst_c_len;
	unsigned int ofst_y_len;

	/* display buffer size */
	uint32_t buffer_size;

	/*for performance debug */
	uint64_t time_fence_create;
	uint64_t time_start_display;
	uint64_t time_fence_signal;

	/* buffer display time. */
	unsigned int source_duration;
};


struct video_layer_info {
	unsigned int inited;
	struct mutex sync_lock;
	unsigned int layer_id;
	/* for release buffer in irq handle */
	struct work_struct task_work;

	/* need to wait for layer state idle when start play. */
	wait_queue_head_t wait_queue;

	/* the newest config buffer index(fence value)
	** VDP use fence value as buffer index.
	*/
	unsigned int fence_idx;

	/* current display buffer index(fence value). */
	unsigned int timeline_idx;
	unsigned int release_timeline_idx;

	/* last released buffer index(fence value). */
	unsigned int release_idx;

	/* no use */
	unsigned int fence_fd;

	/* timeline on VDO HW */
	struct sw_sync_timeline *timeline;

	/* all video_buffer need to display list */
	struct list_head buf_list;

	/* record lastest display buffer pts */
	uint64_t last_pts;

	enum VDP_LAYER_STATE state;

	/* for FRC */
	unsigned int display_duration;
	unsigned int vsync_duration;

	bool enable;	/* enable VDO */
	bool layer_start; /* disp_vdp_start() this layer. */

	/* record latest display buffer source display_range */
	struct mtk_disp_range src_rgn;

	/* record latest display buffer target display_range */
	struct mtk_disp_range tgt_rgn;

	/* record latest display buffer mva/handle & size
	** for normal buffer: value = MVA.
	** for secure buffer: value = secure handle.
	*/
	unsigned int src_phy_addr;
	int buffer_size;


	bool dsd_en;	/* enable dsd feature */
	bool secure_en;	/* secure layer */
	bool secure2normal;
	bool res_change;
	bool sdr2hdr;
	enum DISP_LAYER_TYPE type;
};

struct video_playback_info {
	HDMI_VIDEO_RESOLUTION curr_res;
};


enum VID_PLA_REGION_TYPE {
	VID_PLA_REGION_TYPE_UNKNOWN = 0,
	VID_PLA_REGION_TYPE_PIXEL,
	VID_PLA_REGION_TYPE_PERMILLE,
	VID_PLA_REGION_TYPE_LETTERBOX,
	VID_PLA_REGION_TYPE_PER_TEN_THOUSAND
/*
 *  SM_VSH_REGION_TYPE_PERCENT
 */
};

struct vdp_region {
	enum VID_PLA_REGION_TYPE	eRegionType;
	unsigned int			X;
	unsigned int			Y;
	unsigned int			Width;
	unsigned int			Height;
	char				bIsFullRegion;
};

/*Save the information set by hwc */
struct vdp_display_info {
	uint32_t display_counter; /*for frc */
};

extern int vdp_set_output_resolution(uint32_t res);
void disp_vdp_enable_premix_clock(bool enable);
/* record whether main path is playing dolby video now. */
extern uint32_t dolby_path_enable;
extern uint32_t dolby_path_start;
extern bool hdr10_plus_enable;
extern void vdp_update_dovi_setting_res_change(struct disp_hw_tv_capbility *tv_cap,
	const struct disp_hw_resolution *resolution);
extern bool dovi_idk_dump;
void vdp_set_HDMI_BT2020_signal(bool enable_bt2020);
extern bool netflix_dolby_path_enable;
bool disp_vdp_get_main_sub_swap_status(void);
void disp_vdp_set_main_sub_swap(uint32_t swap);
void disp_vdp_set_osd_premix(bool enable);
bool disp_vdp_get_osd_premix(void);


extern bool netflix_dolby_osd_no_ready;

#endif
