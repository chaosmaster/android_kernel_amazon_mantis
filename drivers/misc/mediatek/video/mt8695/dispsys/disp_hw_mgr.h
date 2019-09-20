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

#ifndef __DISP_HW_MGR_H__
#define __DISP_HW_MGR_H__

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/switch.h>

#include "disp_info.h"
#include "hdmitx.h"
#include "disp_clk.h"

extern struct disp_hw_common_info disp_common_info;

/*
* If add, at last and before DISP_MODULE_NUM.
* then modify disp_hw_mgr.sequence at disp_hw_mgr.c
* then add DISP_EVENT
*/

enum DISP_MODULE_ENUM {
	DISP_MODULE_OSD,
	DISP_MODULE_VDP,
	DISP_MODULE_PMX,
	DISP_MODULE_HDR_BT2020,
	DISP_MODULE_VIDEOIN,
	DISP_MODULE_P2I,
	DISP_MODULE_PP,
	DISP_MODULE_DOVI,
	DISP_MODULE_NUM,
};

enum DISP_HW_LAYER {
	DISP_VDP_LAYER1 = 0,
	DISP_VDP_LAYER2,
	DISP_OSD_LAYER1,
	DISP_OSD_LAYER2,
	DISP_LAYER_NUM,
};

/*
* the low level event is as change resolution done of every sub module:
* 1 << module
*/
enum DISP_EVENT {
	DISP_EVENT_PLUG_IN					= 1 << 0,
	DISP_EVENT_PLUG_OUT					= 1 << 1,
	DISP_EVENT_CHANGE_RES		= 1 << 2,
	DISP_EVENT_ACT_START			= 1 << 3,
	DISP_EVENT_VSYNC				= 1 << 4,
	DISP_EVENT_CHANGE_RES_DONE	= 1 << 5,
	DISP_EVENT_VIDEO_VSYNC          = 1 << 6,
	DISP_EVENT_FORCE_HDR			= 1 << 7,
};

enum DISP_HW_MGR_STATUS {
	DISP_STATUS_DEINIT,
	DISP_STATUS_INIT,
	DISP_STATUS_NORMAL,
	DISP_STATUS_CONFIG,
	DISP_STATUS_CHANGE_RES,
	DISP_STATUS_SUSPEND,
	DISP_STATUS_RESUME,
};

struct disp_hw_resolution {
	uint16_t htotal;
	uint16_t vtotal;
	uint16_t width;
	uint16_t height;
	uint16_t frequency;
	char is_progressive;
	char is_hd;
	HDMI_VIDEO_RESOLUTION res_mode;
	bool is_fractional;
};

struct disp_hw_tv_capbility {
	char is_support_hdr;
	char is_support_hlg;
	char is_support_dolby;
	char is_support_dolby_2160p60;
	char is_support_601;
	char is_support_709;
	char is_support_bt2020;
	char is_support_dolby_low_latency;
	char is_support_hdr10_plus;
	long long supported_resolution;
	unsigned int hdr_content_max_luminance;
	unsigned int hdr_content_max_frame_average_luminance;
	unsigned int hdr_content_min_luminance;
	unsigned int dolbyvision_vsvdb_version;
	unsigned int dolbyvision_vsvdb_v2_interface;
	unsigned char vsvdb_edid[0x1A];
	unsigned char hdr10_plus_app_ver;
	unsigned int force_hdr;
	unsigned char screen_width;
	unsigned char screen_height;
	unsigned short max_tmds_rate;
};

struct disp_hw_common_info {
	struct disp_hw_tv_capbility tv;
	const struct disp_hw_resolution *resolution;
	struct mtk_disp_vdp_cap vdp_cap;
	int hw_mgr_status;
	int osd_swap;
};

struct disp_hw_sequence {
	enum DISP_MODULE_ENUM init[DISP_MODULE_NUM];
	enum DISP_MODULE_ENUM config[DISP_MODULE_NUM];
	enum DISP_MODULE_ENUM change_res[DISP_MODULE_NUM];
	enum DISP_MODULE_ENUM suspend[DISP_MODULE_NUM];
	enum DISP_MODULE_ENUM resume[DISP_MODULE_NUM];
	enum DISP_MODULE_ENUM handle_irq[DISP_MODULE_NUM];
};

struct disp_hw_irq {
	uint32_t value;   /* virtual irq value */
	uint32_t irq;
};

enum DISP_CMD {
	DISP_CMD_STOP_HDR2SDR_BT2020,
	DISP_CMD_STOP_SDR2HDR_BT2020,
	DISP_CMD_OSD_ENABLE_SDR2HDR_BT2020,
	DISP_CMD_PLAY_HDR_SOURCE,
	DISP_CMD_DOVI_OUT_FORMAT_UPDATE,
	DISP_CMD_METADATA_UPDATE,
	DISP_CMD_DOVI_CORE2_UPDATE,
	DISP_CMD_DOVI_SET_VIDEO_IN,
	DISP_CMD_DOVI_DUMP_FRAME,
	DISP_CMD_OSD_PREMIX_ENABLE_VDO4,
	DISP_CMD_OSD_UPDATE,
	DISP_CMD_GET_HDMI_CAP,
	DISP_CMD_METADATA_UPDATE_RES_CHANGE,
	DISP_CMD_HDMITX_PLUG_OUT,
	DISP_CMD_HDMITX_PLUG_IN,
	DISP_CMD_DOVI_FULL_VS10,
	DISP_CMD_OSD_RGB_TO_BGR,
	DISP_CMD_NUM
};

typedef int (*event_callback) (enum DISP_EVENT event, void *data);

struct disp_hw {
	const char *name;
	enum DISP_MODULE_ENUM module;
	struct work_struct change_res_work;
	uint32_t log_level;

	uint32_t irq_num;
	struct disp_hw_irq irq[8]; /* max=8 */

	int (*init)(struct disp_hw_common_info *info);
	int (*deinit)(void);
	int (*start)(struct disp_hw_common_info *info, unsigned int layer_id);
	int (*stop)(unsigned int layer_id);
	int (*suspend)(void);
	int (*resume)(void);
	int (*get_info)(struct disp_hw_common_info *info);
	int (*change_resolution)(const struct disp_hw_resolution *info);
	int (*config)(struct mtk_disp_buffer *config, struct disp_hw_common_info *info);
	int (*irq_handler)(uint32_t irq);
	int (*set_listener)(event_callback callback);
	int (*wait_event)(enum DISP_EVENT event);
	int (*dump)(uint32_t level);
	int (*drv_call)(enum DISP_CMD cmd, void *data);
	int (*set_cmd)(enum DISP_CMD cmd, void *data);
	int (*config_ex)(struct mtk_disp_config *config, struct disp_hw_common_info *info);
};

struct disp_hw_manager {
	atomic_t status;
	struct disp_hw_sequence sequence;

	struct disp_hw *disp_hw_drv[DISP_MODULE_NUM];
	struct disp_hw *(*get_drv[DISP_MODULE_NUM])(void);

	struct disp_hw_common_info common_info;

	struct task_struct *wait_event_task;
	wait_queue_head_t event_wq;
	atomic_t event_flag;

	struct workqueue_struct *change_res_workq;
	wait_queue_head_t change_res_wq;
	atomic_t change_res_done_flag;
	uint32_t change_res_module;
	struct switch_dev hdmi_res_switch;

	wait_queue_head_t wait_vsync_wq;
	atomic_t wait_vsync_flag;
	unsigned long long vsync_ts;

	struct task_struct *disp_irq_polling_task; /*for early porting*/

	struct semaphore mgr_sem;
	bool is_secrity;
	uint32_t log_level;

	uint32_t irq_num;
	struct disp_hw_irq hw_irq[8];

	bool hw_playing_status[DISP_LAYER_NUM]; /* 0: main video, 1: sub video, 2: main osd, 3: sub osd */
	bool hdmi_plug_out;
	struct disp_hw_resolution current_res;

	struct mutex lock;
	bool is_tunnel_playback;
};

extern struct disp_hw *disp_osd_get_drv(void);
extern struct disp_hw *disp_vdp_get_drv(void);
extern struct disp_hw *disp_pmx_get_drv(void);
extern struct disp_hw *disp_hdr_bt2020_get_drv(void);
extern struct disp_hw *disp_videoin_get_drv(void);
extern struct disp_hw *disp_p2i_get_drv(void);
extern struct disp_hw *disp_pp_get_drv(void);
extern struct disp_hw *disp_dovi_get_drv(void);

extern struct device *disp_hw_mgr_get_dev(void);

extern uint32_t g_dma_read_empty_threshold;
extern uint32_t g_dma_write_full_threshold;

/*
* For display manager, not display sub module
*/
int disp_hw_mgr_set_pm_dev(struct device *dev, enum DISP_PM_TYPE type);
int disp_hw_mgr_init(struct device *dev, unsigned int resolution);
int disp_path_shadow_enable(void);
int disp_hw_mgr_get_info(struct disp_hw_common_info *info);
int disp_hw_get_vdp_cap(struct mtk_disp_vdp_cap *vdp_cap);
int disp_hw_get_hdmi_cap(struct mtk_disp_hdmi_cap *hdmi_cap);
int disp_hw_mgr_wait_vsync(unsigned long long *ts);
int disp_hw_mgr_config(struct mtk_disp_config *config);
int disp_hw_mgr_dump(uint32_t level);
int disp_hw_mgr_send_event(enum DISP_EVENT event, void *data);
int disp_hw_mgr_resume(void);
int disp_hw_mgr_suspend(void);
bool disp_hw_mgr_is_slept(void);
int disp_hw_mgr_deinit(void);

int disp_hw_mgr_status(void);

#endif
