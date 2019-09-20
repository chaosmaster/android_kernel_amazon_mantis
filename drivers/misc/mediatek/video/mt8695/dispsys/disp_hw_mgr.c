/*
* disp_hw_mgr.c - display hardware manager
*
*  Copyright (C) 2015-2016 MediaTek Inc.
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

#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>

#define LOG_TAG "HW"
#include "hdmitx.h"
#include "disp_hw_mgr.h"
#include "disp_hw_log.h"
#include "disp_reg.h"
#include "disp_hw_debug.h"
#include "disp_clk.h"
#include "disp_path.h"
#include "disp_irq.h"
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
#include "hdmiedid.h"
#endif

static struct disp_hw_manager disp_hw_mgr = {
	.irq_num = 0,
	.sequence.init = {
		DISP_MODULE_PMX,
		DISP_MODULE_OSD,
		DISP_MODULE_VDP,
		DISP_MODULE_DOVI,
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_VIDEOIN,
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
	},
	.sequence.config = {
		DISP_MODULE_OSD,
		DISP_MODULE_DOVI,
		DISP_MODULE_PMX,
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_VDP,
		DISP_MODULE_VIDEOIN,
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
	},
	.sequence.change_res = {
		DISP_MODULE_PMX,
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
		DISP_MODULE_OSD,
		DISP_MODULE_VDP,
		DISP_MODULE_DOVI,
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_VIDEOIN,
	},
	.sequence.suspend = {
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
		DISP_MODULE_OSD,
		DISP_MODULE_VDP,
		DISP_MODULE_DOVI,
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_VIDEOIN,
		DISP_MODULE_PMX,
	},
	.sequence.resume = {
		DISP_MODULE_PMX,
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
		DISP_MODULE_OSD,
		DISP_MODULE_VDP,
		DISP_MODULE_DOVI,
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_VIDEOIN,
	},

	.sequence.handle_irq = {
		DISP_MODULE_HDR_BT2020,
		DISP_MODULE_OSD,
		DISP_MODULE_DOVI,
		DISP_MODULE_PMX,
		DISP_MODULE_VDP,
		DISP_MODULE_VIDEOIN,
		DISP_MODULE_P2I,
		DISP_MODULE_PP,
	},

	.get_drv[DISP_MODULE_OSD] = disp_osd_get_drv,
	.get_drv[DISP_MODULE_VDP] = disp_vdp_get_drv,
	.get_drv[DISP_MODULE_PMX] = disp_pmx_get_drv,
	.get_drv[DISP_MODULE_HDR_BT2020] = disp_hdr_bt2020_get_drv,
	.get_drv[DISP_MODULE_VIDEOIN] = disp_videoin_get_drv,
	.get_drv[DISP_MODULE_P2I] = disp_p2i_get_drv,
	.get_drv[DISP_MODULE_PP] = disp_pp_get_drv,
	.get_drv[DISP_MODULE_DOVI] = disp_dovi_get_drv,

};

static struct disp_hw_manager *mgr = &disp_hw_mgr;
struct device *hw_mgr_dev;


/*
* htotal, vtotal, width , height,  freq, progressive,  hd,  resolution
*/
static const struct disp_hw_resolution disp_resolution_table[] = {
	{858, 525, 720, 480, 60, false, false, HDMI_VIDEO_720x480i_60Hz, false},
	{864, 625, 720, 576, 50, false, false, HDMI_VIDEO_720x576i_50Hz, false},
	{858, 525, 720, 480, 60, true, false, HDMI_VIDEO_720x480p_60Hz, false},
	{864, 625, 720, 576, 50, true, false, HDMI_VIDEO_720x576p_50Hz, false},
	{1650, 750, 1280, 720, 60, true, true, HDMI_VIDEO_1280x720p_59_94Hz, true},
	{1650, 750, 1280, 720, 60, true, true, HDMI_VIDEO_1280x720p_60Hz, false},
	{1980, 750, 1280, 720, 50, true, true, HDMI_VIDEO_1280x720p_50Hz, false},
	{2200, 1125, 1920, 1080, 60, false, true, HDMI_VIDEO_1920x1080i_60Hz, false},
	{2640, 1125, 1920, 1080, 50, false, true, HDMI_VIDEO_1920x1080i_50Hz, false},
	{2200, 1125, 1920, 1080, 30, true, true, HDMI_VIDEO_1920x1080p_30Hz, false},
	{2640, 1125, 1920, 1080, 25, true, true, HDMI_VIDEO_1920x1080p_25Hz, false},
	{2750, 1125, 1920, 1080, 24, true, true, HDMI_VIDEO_1920x1080p_24Hz, false},
	{2750, 1125, 1920, 1080, 24, true, true, HDMI_VIDEO_1920x1080p_23Hz, true},
	{2200, 1125, 1920, 1080, 30, true, true, HDMI_VIDEO_1920x1080p_29Hz, true},
	{2200, 1125, 1920, 1080, 60, true, true, HDMI_VIDEO_1920x1080p_59_94Hz, true},
	{2200, 1125, 1920, 1080, 60, true, true, HDMI_VIDEO_1920x1080p_60Hz, false},
	{2640, 1125, 1920, 1080, 50, true, true, HDMI_VIDEO_1920x1080p_50Hz, false},
	{5500, 2250, 3840, 2160, 24, true, true, HDMI_VIDEO_3840x2160P_23_976HZ, true},
	{5500, 2250, 3840, 2160, 24, true, true, HDMI_VIDEO_3840x2160P_24HZ, false},
	{5280, 2250, 3840, 2160, 25, true, true, HDMI_VIDEO_3840x2160P_25HZ, false},
	{4400, 2250, 3840, 2160, 30, true, true, HDMI_VIDEO_3840x2160P_29_97HZ, true},
	{4400, 2250, 3840, 2160, 30, true, true, HDMI_VIDEO_3840x2160P_30HZ, false},
	{5280, 2250, 3840, 2160, 50, true, true, HDMI_VIDEO_3840x2160P_50HZ, false},
	{4400, 2250, 3840, 2160, 60, true, true, HDMI_VIDEO_3840x2160P_59_94HZ, true},
	{4400, 2250, 3840, 2160, 60, true, true, HDMI_VIDEO_3840x2160P_60HZ, false},
	{5500, 2250, 4096, 2160, 24, true, true, HDMI_VIDEO_4096x2160P_24HZ, false},
	{5280, 2250, 4096, 2160, 50, true, true, HDMI_VIDEO_4096x2160P_50HZ, false},
	{4400, 2250, 4096, 2160, 60, true, true, HDMI_VIDEO_4096x2160P_60HZ, false},
};

#define DOWN_SEM() \
do { \
	if (down_interruptible(&mgr->mgr_sem)) { \
		DISP_LOG_E("semaphore down fail at %s:%s line%d\n", \
				__FILE__, __func__, __LINE__); \
	} \
} while (0)

#define UP_SEM() up(&mgr->mgr_sem)

#define IS_DISP_SUSPEND()	(atomic_read(&mgr->status) == DISP_STATUS_SUSPEND)
#define IS_DISP_RESUME()	(atomic_read(&mgr->status) == DISP_STATUS_RESUME)
#define IS_DISP_NORMAL()	(atomic_read(&mgr->status) == DISP_STATUS_NORMAL)
#define IS_DISP_DEINIT()	(atomic_read(&mgr->status) == DISP_STATUS_DEINIT)


/*
* After realize, please delete it.
*/
#define DISP_TODO
#ifdef DISP_TODO


struct disp_hw *disp_hdr_bt2020_get_drv(void);


#endif

static void _disp_lock_init(void)
{
	mutex_init(&(mgr->lock));
}

static void _disp_mutex_lock(void)
{
	mutex_lock(&(mgr->lock));
}

static void _disp_mutex_unlock(void)
{
	mutex_unlock(&(mgr->lock));
}


static const struct disp_hw_resolution *_get_resolution(HDMI_VIDEO_RESOLUTION res_mode)
{
	int i;
	const struct disp_hw_resolution *res;
	uint32_t size = ARRAY_SIZE(disp_resolution_table);

	for (i = 0; i < size; i++) {
		res = &disp_resolution_table[i];
		if (res_mode == res->res_mode)
			break;
	}

	memcpy(&mgr->current_res, res, sizeof(struct disp_hw_resolution));
	if (res->res_mode == HDMI_VIDEO_1280x720p_59_94Hz)
		mgr->current_res.res_mode = HDMI_VIDEO_1280x720p_60Hz;
	else if (res->res_mode == HDMI_VIDEO_1920x1080p_59_94Hz)
		mgr->current_res.res_mode = HDMI_VIDEO_1920x1080p_60Hz;
	else if (res->res_mode == HDMI_VIDEO_3840x2160P_59_94HZ)
		mgr->current_res.res_mode = HDMI_VIDEO_3840x2160P_60HZ;

	DISP_LOG_D("_get_resolution: %d\n", res->res_mode);

	return &mgr->current_res;
}

static struct disp_hw_irq *_get_irq(uint32_t value)
{
	int i;

	for (i = 0; i < mgr->irq_num; i++) {
		if (value == mgr->hw_irq[i].value)
			return &mgr->hw_irq[i];
	}

	return NULL;
}

static uint64_t _get_current_time_us(void)
{
	struct timeval t;

	do_gettimeofday(&t);
	return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

static void _disp_change_resolution_work(struct work_struct *work)
{
	unsigned long start, end, time;
	struct disp_hw *drv = container_of(work, struct disp_hw, change_res_work);

	DOWN_SEM();
	DISP_LOG_HW("%s start to change resolution\n", drv->name);
	start = _get_current_time_us();
	drv->change_resolution(mgr->common_info.resolution);
	end = _get_current_time_us();
	time = (end - start) / 1000;
	DISP_LOG_HW("%s change resolution done, spend %dms\n",
			drv->name, (uint32_t)time);
	DISP_MMP(MMP_DISP_HW_CHANGE_RES, MMP_PULSE, drv->module, time);

	mgr->change_res_module &= (~(1 << drv->module));
	UP_SEM();
	if (mgr->change_res_module == 0) {
		atomic_set(&mgr->change_res_done_flag, 1);
		wake_up(&mgr->change_res_wq);
	}
}

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
static int _get_hdmi_cap(struct disp_hw_tv_capbility *cap)
{
	HDMI_EDID_T edid_info = {0};

	hdmi_AppGetEdidInfo(&edid_info);
	if (edid_info.ui1_sink_support_static_hdr & EDID_SUPPORT_SMPTE_ST_2084)
		cap->is_support_hdr = true;
	else
		cap->is_support_hdr = false;

	if (edid_info.ui1_sink_support_static_hdr & EDID_SUPPORT_FUTURE_EOTF)
		cap->is_support_hlg = true;
	else
		cap->is_support_hlg = false;

	if (edid_info.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR)
		cap->is_support_dolby = true;
	else
		cap->is_support_dolby = false;

	if (edid_info.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_HDR10_PLUS)
		cap->is_support_hdr10_plus = true;
	else
		cap->is_support_hdr10_plus = false;

	if (edid_info.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR_2160P60)
		cap->is_support_dolby_2160p60 = true;
	else
		cap->is_support_dolby_2160p60 = false;

	cap->is_support_601 = true;
	cap->is_support_709 = true;

	if ((edid_info.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_CYCC) ||
		(edid_info.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_YCC) ||
		(edid_info.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_RGB))
		cap->is_support_bt2020 = true;
	else
		cap->is_support_bt2020 = false;

	cap->hdr_content_max_luminance = edid_info.ui1_sink_hdr_content_max_luminance;
	cap->hdr_content_max_frame_average_luminance =
		edid_info.ui1_sink_hdr_content_max_frame_average_luminance;
	cap->hdr_content_min_luminance = edid_info.ui1_sink_hdr_content_min_luminance;

	memcpy((void *)cap->vsvdb_edid, (void *)edid_info.ui1_sink_dolbyvision_block, 0x1A);
	cap->is_support_dolby_low_latency =
		edid_info.ui4_sink_dolbyvision_vsvdb_low_latency_support;
	cap->dolbyvision_vsvdb_version =
		edid_info.ui4_sink_dolbyvision_vsvdb_version;
	cap->dolbyvision_vsvdb_v2_interface =
		edid_info.ui4_sink_dolbyvision_vsvdb_v2_interface;

	cap->hdr10_plus_app_ver = edid_info.ui1_sink_hdr10plus_app_version;

	DISP_LOG_I("get hdmi cap, hdr static 0x%X dynamic 0x%X colorimetry 0x%X appversion:%d\n",
	edid_info.ui1_sink_support_static_hdr,
	edid_info.ui1_sink_support_dynamic_hdr,
	edid_info.ui2_sink_colorimetry,
	edid_info.ui1_sink_hdr10plus_app_version);

	DISP_LOG_I("get hdmi cap, hdr low_latency 0x%X vsvdb_version 0x%X v2_interface 0x%X\n",
	cap->is_support_dolby_low_latency,
	cap->dolbyvision_vsvdb_version,
	cap->dolbyvision_vsvdb_v2_interface);

	cap->supported_resolution = vDispGetHdmiResolution();
	cap->screen_width = edid_info.ui1_Display_Horizontal_Size;
	cap->screen_height = edid_info.ui1_Display_Vertical_Size;
	/* maximum TMDS rate supported over HDMI */
	if (edid_info.ui2_sink_max_tmds_character_rate != 0)
		cap->max_tmds_rate = edid_info.ui2_sink_max_tmds_character_rate;
	else
		cap->max_tmds_rate = edid_info.ui1_sink_max_tmds_clock;

	return 0;
}
#endif

static void _disp_queue_change_res(void)
{
	int i;
	struct disp_hw *drv;
	enum DISP_MODULE_ENUM module;

	_disp_mutex_lock();
	/*set resolution information to disp_path*/
	disp_path_set_res((struct disp_path_resolution *)mgr->common_info.resolution);

	DOWN_SEM();
	DISP_MMP(MMP_DISP_HW_CHANGE_RES, MMP_START,
			mgr->common_info.resolution->res_mode, 0);
	DISP_LOG_HW("all sub module change resolution start\n");
	mgr->change_res_module = 0;
	mgr->common_info.hw_mgr_status = DISP_STATUS_CHANGE_RES;
	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.change_res[i];
		drv = mgr->disp_hw_drv[module];

		if (drv && drv->change_resolution) {
			mgr->change_res_module |= (1<<module);

			INIT_WORK(&drv->change_res_work,
				_disp_change_resolution_work);
			queue_work(mgr->change_res_workq,
				&drv->change_res_work);
		}
	}
	UP_SEM();

	/* wait for all sub module */
	wait_event(mgr->change_res_wq,
			atomic_read(&mgr->change_res_done_flag));
	atomic_set(&mgr->change_res_done_flag, 0);
	mgr->common_info.hw_mgr_status = DISP_STATUS_NORMAL;

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	_get_hdmi_cap(&mgr->common_info.tv);
#endif

	DISP_MMP(MMP_DISP_HW_CHANGE_RES, MMP_END,
			mgr->common_info.resolution->res_mode, 0);

	/* notify HWC resolution change done */
	/* switch_set_state(&mgr->hdmi_res_switch, mgr->common_info.resolution->res_mode); */

	_disp_mutex_unlock();
	DISP_LOG_HW("all sub module change resolution finish\n");
}

static void _disp_wakeup_event(enum DISP_EVENT event)
{
	if (event) {
		atomic_set(&mgr->event_flag, event);
		wake_up(&mgr->event_wq);
	}
}

static void _disp_wakeup_vsync_event(enum DISP_EVENT event)
{
	if (event) {
		mgr->vsync_ts = ktime_to_ns(ktime_get());
		atomic_set(&mgr->wait_vsync_flag, event);
		wake_up(&mgr->wait_vsync_wq);
	}
}

static int _disp_set_cmd(enum DISP_CMD cmd, void *data)
{
	struct disp_hw *drv;
	int i = 0;
	enum DISP_MODULE_ENUM module;

	if (cmd == DISP_CMD_GET_HDMI_CAP) {
	#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		_get_hdmi_cap(&mgr->common_info.tv);
	#endif
		if (data)
			memcpy(data, (void *)(&mgr->common_info.tv), sizeof(struct disp_hw_tv_capbility));
		return 0;
	}

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.config[i];
		drv = mgr->disp_hw_drv[module];

		if (drv == NULL || drv->set_cmd == NULL)
			continue;

		drv->set_cmd(cmd, data);
	}
	return 0;
}

static int _disp_event_callback(enum DISP_EVENT event, void *data)
{
	HDMI_VIDEO_RESOLUTION res_mode = HDMI_VIDEO_RESOLUTION_NUM;
	const struct disp_hw_resolution *resolution;

	switch (event) {
	case DISP_EVENT_CHANGE_RES:
		res_mode = *(HDMI_VIDEO_RESOLUTION *)data;
	#if (0)
		if (res_mode == mgr->common_info.resolution->res_mode) {
		#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
			_get_hdmi_cap(&mgr->common_info.tv);
		#endif
			break;
		}
	#endif

		resolution = _get_resolution(res_mode);
		if (resolution != NULL) {
			_get_hdmi_cap(&mgr->common_info.tv);
			mgr->common_info.resolution = resolution;
			disp_hw_mgr_get_info(&disp_common_info);
			if (0)
				_disp_wakeup_event(DISP_EVENT_CHANGE_RES);
			else
				_disp_queue_change_res();
		}
		break;
	case DISP_EVENT_FORCE_HDR:
		_get_hdmi_cap(&mgr->common_info.tv);
		disp_hw_mgr_get_info(&disp_common_info);
		mgr->common_info.tv.force_hdr = *(unsigned int *)data;
		_disp_set_cmd(DISP_CMD_DOVI_FULL_VS10, data);
		break;

	case DISP_EVENT_PLUG_OUT:
		disp_osd_get_drv()->drv_call(DISP_CMD_HDMITX_PLUG_OUT, NULL);
		disp_vdp_get_drv()->stop(0);
		disp_vdp_get_drv()->stop(1);
		mgr->hw_playing_status[0] = false;
		mgr->hw_playing_status[1] = false;
		mgr->hdmi_plug_out = true;
		break;

	case DISP_EVENT_PLUG_IN:
		mgr->hdmi_plug_out = false;
		_get_hdmi_cap(&mgr->common_info.tv);
		disp_hw_mgr_get_info(&disp_common_info);
		disp_osd_get_drv()->drv_call(DISP_CMD_HDMITX_PLUG_IN, NULL);
		break;

	default:
		break;
	}

	return 0;
}

static int _disp_hw_wait_event_kthread(void *data)
{
	uint32_t event;

	while (1) {
		wait_event(mgr->event_wq,
			(event = (uint32_t)atomic_read(&mgr->event_flag)));
		atomic_set(&mgr->event_flag, 0);

		if (event & DISP_EVENT_ACT_START) {
			/* SOF */
			DISP_MMP(MMP_DISP_FMT, MMP_START, 0, 0);
		} else if (event & DISP_EVENT_VSYNC) {
			/* EOF */
			DISP_MMP(MMP_DISP_FMT, MMP_END, 0, 0);
		} else if (event & DISP_EVENT_CHANGE_RES) {
			_disp_queue_change_res();
		}

		if (kthread_should_stop()) {
			DISP_LOG_HW("_wait_event_kthread stop\n");
			break;
		}
	}

	return 0;
}

static irqreturn_t _disp_irq_handler(int value, void *dev_id)
{
	int i;
	struct disp_hw *drv = (struct disp_hw *)dev_id;
	struct disp_hw_irq *hw_irq = _get_irq(value);

	DISP_LOG_IRQ("_disp_irq_handler irq num: %d\n", value);

	if (!hw_irq) {
		DISP_LOG_E("hw_irq is NULL, irq num: %d\n", value);
		return IRQ_NONE;
	}

	/*clear irq and printk some logs*/
	disp_irq_manager(hw_irq->irq);

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		drv = mgr->disp_hw_drv[mgr->sequence.handle_irq[i]];
		if (drv && drv->irq_handler)
			drv->irq_handler(hw_irq->irq);
	}

	if (hw_irq->irq == DISP_IRQ_FMT_VSYNC)
		_disp_wakeup_vsync_event(DISP_EVENT_VSYNC);

	return IRQ_HANDLED;
}

static int _is_irq_need_enable(enum DISP_MODULE_ENUM module)
{
	switch (module) {
	case DISP_MODULE_PMX:
	case DISP_MODULE_VDP:
	case DISP_MODULE_OSD:
		return 1;
	default:
		return 0;
	}
}

#if 0
static struct disp_hw_irq *_get_irq_by_irqbit(enum DISP_IRQ_E irq)
{
	int i;

	for (i = 0; i < mgr->irq_num; i++) {
		if (irq == mgr->hw_irq[i].irq)
			return &mgr->hw_irq[i];
	}

	return NULL;
}

/* polling irq status to call irq_handler */
static int disp_irq_polling_kthread_func(void *data)
{
	struct disp_hw_irq *irq = NULL;

	while (1) {
		/*read irq status register*/
		mdelay(1000);
		irq = _get_irq_by_irqbit(DISP_IRQ_FMT_VSYNC);
		if (irq)
			_disp_irq_handler(irq->value, NULL);

		mdelay(1);
		irq = _get_irq_by_irqbit(DISP_IRQ_FMT_ACTIVE_START);
		if (irq)
			_disp_irq_handler(irq->value, NULL);
	}

	return 0;
}

#endif

static int _disp_request_irq(struct disp_hw *drv)
{
	int i, ret = 0;

	for (i = 0; i < drv->irq_num; i++) {
		if (_is_irq_need_enable(drv->module)
			&& drv->irq[i].value) {
			mgr->hw_irq[mgr->irq_num].value = drv->irq[i].value;
			mgr->hw_irq[mgr->irq_num].irq = drv->irq[i].irq;
			mgr->irq_num++;

			ret = request_irq(drv->irq[i].value,
					(irq_handler_t)_disp_irq_handler,
					IRQF_TRIGGER_NONE, drv->name, (void *)drv);

			if (ret) {
				DISP_LOG_E("request %s irq_%d fail\n",
						drv->name,
						drv->irq[i].value);
				ret = -EFAULT;
				break;
			}

			DISP_LOG_D("request %s irq_%d hw_irq %d success\n",
					drv->name, drv->irq[i].value, drv->irq[i].irq);
		}
	}

	return ret;
}

static int _init_common_info(struct disp_hw_common_info *common_info, unsigned int resolution)
{
	HDMI_VIDEO_RESOLUTION res_mode = resolution;

	common_info->resolution = _get_resolution(res_mode);

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	_get_hdmi_cap(&mgr->common_info.tv);
#endif

	return 0;
}

int disp_hw_mgr_set_pm_dev(struct device *dev, enum DISP_PM_TYPE type)
{
	return disp_clock_set_pm_dev(dev, type);
}

struct device *disp_hw_mgr_get_dev(void)
{
	return hw_mgr_dev;
}

static void disp_hw_mgr_set_dev(struct device *dev)
{
	 hw_mgr_dev = dev;
}


int disp_hw_mgr_init(struct device *dev, unsigned int resolution)
{
	int i, ret = 0;
	struct disp_hw *hw_drv;
	enum DISP_MODULE_ENUM module;
	struct disp_hw_common_info *common_info;

	if (atomic_read(&mgr->status) != DISP_STATUS_DEINIT) {
		DISP_LOG_D("already inited.\n");
		return 0;
	}
	disp_hw_mgr_set_dev(dev);
	disp_log_init(dev);
	disp_hw_debug_init();
	_disp_lock_init();

	sema_init(&mgr->mgr_sem, 1);
	init_waitqueue_head(&mgr->event_wq);
	init_waitqueue_head(&mgr->change_res_wq);
	init_waitqueue_head(&mgr->wait_vsync_wq);
	atomic_set(&mgr->event_flag, 0);
	atomic_set(&mgr->change_res_done_flag, 0);
	atomic_set(&mgr->wait_vsync_flag, 0);
	mgr->vsync_ts = 0;
	mgr->change_res_module = 0;
	mgr->hdmi_plug_out = false;

	atomic_set(&mgr->status, DISP_STATUS_INIT);
	common_info = &mgr->common_info;
	_init_common_info(common_info, resolution);
	disp_path_init((struct disp_path_resolution *)common_info->resolution);
	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.init[i];
		mgr->disp_hw_drv[module] = mgr->get_drv[module]();
		hw_drv = mgr->disp_hw_drv[module];
		if (hw_drv) {
			hw_drv->module = module;

			if (hw_drv->init)
				ret = hw_drv->init(common_info);
			if (ret != 0) {
				DISP_LOG_E("%s init fail, module=%d\n", hw_drv->name, module);
				break;
			}

			if (hw_drv->get_info)
				ret = hw_drv->get_info(common_info);
			if (ret != 0) {
				DISP_LOG_E("%s get info fail\n", hw_drv->name);
				break;
			}

			if (hw_drv->set_listener)
				ret = hw_drv->set_listener(_disp_event_callback);
			if (ret != 0) {
				DISP_LOG_E("%s set listener fail\n", hw_drv->name);
				break;
			}

			hw_drv->drv_call = _disp_set_cmd;

			ret = _disp_request_irq(hw_drv);
			if (ret != 0)
				break;

			DISP_LOG_HW("module %s init done\n", hw_drv->name);
		}
	}

	disp_path_shadow_enable();
#if 0
	mgr->hdmi_res_switch.name = "res_hdmi";
	mgr->hdmi_res_switch.index = 0;
	mgr->hdmi_res_switch.state = 0;
	ret = switch_dev_register(&mgr->hdmi_res_switch);
	if (ret) {
		DISP_LOG_E("switch_dev_register %s fail(%d)\n",
				mgr->hdmi_res_switch.name, ret);
		return -EFAULT;
	}
#endif
	mgr->change_res_workq = alloc_workqueue("disp_change_res_wq",
					WQ_HIGHPRI | WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
	if (IS_ERR_OR_NULL(mgr->change_res_workq)) {
		DISP_LOG_E("can not alloc workqueue disp_change_res_wq\n");
		return -EFAULT;
	}

	mgr->wait_event_task = kthread_run(_disp_hw_wait_event_kthread,
					NULL, "disp_hw_wait_event_kthread");
	if (IS_ERR_OR_NULL(mgr->wait_event_task)) {
		DISP_LOG_E("can not run disp_hw_wait_event_kthread\n");
		return -EFAULT;
	}
#if 0
	mgr->disp_irq_polling_task =
	    kthread_run(disp_irq_polling_kthread_func, NULL, "ddp_irq_polling_kthread");
	if (IS_ERR_OR_NULL(mgr->disp_irq_polling_task)) {
		DISP_LOG_E("can not run disp_hw_wait_event_kthread\n");
		return -EFAULT;
	}
#endif
	mgr->log_level = DISP_LOG_LEVEL_DEBUG3;
	atomic_set(&mgr->status, DISP_STATUS_NORMAL);

	mgr->hw_playing_status[DISP_OSD_LAYER1] = true;

	return 0;
}

int disp_hw_mgr_get_info(struct disp_hw_common_info *info)
{
	int ret = 0;
	struct disp_hw_common_info *common_info;

	WARN_ON(!info);

	common_info = &mgr->common_info;
	#if 0
	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.init[i];
		drv = mgr->disp_hw_drv[module];
		if (drv && drv->get_info)
			ret = drv->get_info(common_info);
		if (ret != 0)
			break;
	}
	#endif

	if (ret == 0)
		memcpy(info, common_info, sizeof(struct disp_hw_common_info));

	return ret;
}

int disp_hw_get_vdp_cap(struct mtk_disp_vdp_cap *vdp_cap)
{
	int ret = 0;
	struct disp_hw *drv;

	memcpy(&(mgr->common_info.vdp_cap), vdp_cap, sizeof(struct mtk_disp_vdp_cap));
	drv = mgr->disp_hw_drv[DISP_MODULE_VDP];
	if (drv && drv->get_info)
		ret = drv->get_info(&mgr->common_info);

	if (!ret)
		vdp_cap->need_resizer = mgr->common_info.vdp_cap.need_resizer;

	return 0;
}

int disp_hw_get_hdmi_cap(struct mtk_disp_hdmi_cap *hdmi_cap)
{
	int ret = 0;

	if (!hdmi_cap)
		return -1;

	memset(hdmi_cap, 0x00, sizeof(struct mtk_disp_hdmi_cap));

	if (mgr->common_info.tv.is_support_hdr)
		hdmi_cap->hdr_type |= MTK_HDR_TYPE_HDR10;
	if (mgr->common_info.tv.is_support_dolby)
		hdmi_cap->hdr_type |= MTK_HDR_TYPE_DOLBY_VISION;
	if (mgr->common_info.tv.is_support_hlg)
		hdmi_cap->hdr_type |= MTK_HDR_TYPE_HLG;

	hdmi_cap->hdr_content_max_frame_average_luminance =
		mgr->common_info.tv.hdr_content_max_frame_average_luminance;
	hdmi_cap->hdr_content_max_luminance =
		mgr->common_info.tv.hdr_content_max_luminance;
	hdmi_cap->hdr_content_min_luminance =
		mgr->common_info.tv.hdr_content_min_luminance;

	hdmi_cap->supported_resolution = mgr->common_info.tv.supported_resolution;
	hdmi_cap->disp_resolution = mgr->common_info.resolution->res_mode;
	hdmi_cap->screen_width = mgr->common_info.tv.screen_width;
	hdmi_cap->screen_height = mgr->common_info.tv.screen_height;

	return ret;
}


int disp_hw_mgr_config(struct mtk_disp_config *config)
{
	int i, j, k, ret = 0;
	struct disp_hw *drv;
	enum DISP_MODULE_ENUM module;
	bool layer_enable[DISP_LAYER_NUM] = {0};
	bool play_stop_en = true;
	bool is_tunnel = false;
	bool has_invalid_video[DISP_LAYER_NUM] = {false};

	_disp_mutex_lock();

	mgr->common_info.osd_swap = 0;

	DISP_MMP_STRUCT(MMP_DISP_HW, config, struct mtk_disp_config);
	/* has video layer or not */
	j = k = 0;
	if (config->user == DISP_USER_HWC) {
		for (i = 0; i < DISP_BUFFER_MAX; i++) {
			if (config->buffer_info[i].layer_enable
				&& config->buffer_info[i].type == DISP_LAYER_VDP) {
				if (config->buffer_info[i].ion_fd != -1) {
					config->buffer_info[i].layer_id = j;
					layer_enable[j] = true;
				} else {
					has_invalid_video[j] = true;
				}
				j++;
			}

			if (config->buffer_info[i].layer_enable && config->buffer_info[i].type == DISP_LAYER_OSD) {
				if (config->buffer_info[i].src.width >= 3840 && config->buffer_info[i].src.width <= 8192) {
					if (k == 0) {
						k = 1;
						mgr->common_info.osd_swap =  1;
					}
				}

				if (mgr->common_info.osd_swap == 1) {
					if (k == 2)
						k = 0;
				}
				config->buffer_info[i].layer_id = k;  /* modify the layer id */
				layer_enable[k + DISP_OSD_LAYER1] = true;
				k++;
			}

			if (config->buffer_info[i].layer_enable && config->buffer_info[i].type == DISP_LAYER_AEE) {
				config->buffer_info[i].type = DISP_LAYER_OSD;
				config->buffer_info[i].layer_id = 1;  /* modify the layer id */
				layer_enable[DISP_OSD_LAYER2] = true;
				k++;
			}
			config->buffer_info[i].layer_order = i;

		}
		if (has_invalid_video[DISP_VDP_LAYER1])
			layer_enable[DISP_VDP_LAYER1] = mgr->hw_playing_status[DISP_VDP_LAYER1];

		if (has_invalid_video[DISP_VDP_LAYER2])
			layer_enable[DISP_VDP_LAYER2] = mgr->hw_playing_status[DISP_VDP_LAYER2];
	} else if (config->user == DISP_USER_AVSYNC) {
		for (i = 0; i < DISP_BUFFER_MAX; i++) {
			if (config->buffer_info[i].layer_enable
				&& (config->buffer_info[i].type == DISP_LAYER_VDP
				|| config->buffer_info[i].type == DISP_LAYER_SIDEBAND)) {
				config->buffer_info[i].layer_id = j;
				layer_enable[j] = true;
				j++;
				is_tunnel = true;
			}
		}
                /*Don't affect sub path state when avsync operate main path*/
                layer_enable[DISP_VDP_LAYER2] = mgr->hw_playing_status[DISP_VDP_LAYER2];
		/*no ui when tunnel mode playback*/
		layer_enable[DISP_OSD_LAYER1] = mgr->hw_playing_status[DISP_OSD_LAYER1];
		layer_enable[DISP_OSD_LAYER2] = mgr->hw_playing_status[DISP_OSD_LAYER2];

		if (is_tunnel)
			mgr->is_tunnel_playback = true;
		else
			mgr->is_tunnel_playback = false;
	} else if (config->user == DISP_USER_AEE) {
		memcpy((void *)layer_enable, (void *)mgr->hw_playing_status, sizeof(bool) * DISP_LAYER_NUM);
		layer_enable[DISP_OSD_LAYER2] = config->buffer_info[0].layer_enable;
		config->buffer_info[0].layer_id = 1;
	}

	if (mgr->hdmi_plug_out) {
		layer_enable[DISP_VDP_LAYER1] = mgr->hw_playing_status[DISP_VDP_LAYER1];
		layer_enable[DISP_VDP_LAYER2] = mgr->hw_playing_status[DISP_VDP_LAYER2];
		DISP_LOG_I("hdmi plug out, drop video layer\n");
	}

	j = k = 0;

	/* play or stop vdp/osd */
	if (play_stop_en) {   /* there is no any ui info if config from av sync */
		for (i = 0; i < DISP_LAYER_NUM; i++) {
			if (i < DISP_OSD_LAYER1) {
				drv = mgr->disp_hw_drv[DISP_MODULE_VDP];
				j = i;
			} else {
				drv = mgr->disp_hw_drv[DISP_MODULE_OSD];
				j = i - DISP_OSD_LAYER1;
			}

			if (!mgr->hw_playing_status[i] && layer_enable[i] && drv && drv->start) {
				DISP_LOG_I("start() hw layer id: %d, layer id: %d\n", i, j);
				drv->start(&mgr->common_info, j);
			} else if (mgr->hw_playing_status[i] && !layer_enable[i] && drv && drv->stop) {
				DISP_LOG_I("stop() hw layer id: %d, layer id: %d\n", i, j);
				drv->stop(j);
			}

			mgr->hw_playing_status[i] = layer_enable[i];
		}
	}

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.config[i];
		drv = mgr->disp_hw_drv[module];

		if (drv == NULL || (drv->config == NULL && drv->config_ex == NULL))
			continue;

		if (module == DISP_MODULE_VDP) {
			for (j = 0; j < DISP_BUFFER_MAX; j++) {
				if (config->buffer_info[j].layer_enable &&
					(config->buffer_info[j].type == DISP_LAYER_VDP ||
					config->buffer_info[j].type == DISP_LAYER_SIDEBAND))
					drv->config(&config->buffer_info[j], &mgr->common_info);
			}
		} else if (module == DISP_MODULE_OSD) {
			ret = drv->config_ex(config, &mgr->common_info);
		} else {
			for (j = 0; j < DISP_BUFFER_MAX; j++) {
				if (config->buffer_info[j].layer_enable) /* && */
					/*(config->buffer_info[j].type == DISP_LAYER_VDP ||*/
					/*config->buffer_info[j].type == DISP_LAYER_SIDEBAND))*/
					ret = drv->config(&config->buffer_info[j], &mgr->common_info);
			}

		}

	}

	_disp_mutex_unlock();

	return ret;
}

/*
* return value is the time spend, if fail return 0
*/
int disp_hw_mgr_wait_vsync(unsigned long long *ts)
{
	int ret = 0;

	if (IS_DISP_SUSPEND()) {
		usleep_range(16000, 17000);
		DISP_LOG_I("VSYNC DISP_SLEPT Return\n");
		return 0;
	}
	ret = (int)wait_event_timeout(mgr->wait_vsync_wq,
			DISP_EVENT_VSYNC & atomic_read(&mgr->wait_vsync_flag),
			msecs_to_jiffies(100));
	atomic_set(&mgr->wait_vsync_flag, 0);
	if (ret == 0) {
		DISP_LOG_E("wait vsync timeout(>HZ/2)\n");
		if (ts != NULL)
			*ts = ktime_to_ns(ktime_get());
	} else if (ts != NULL)
		*ts = mgr->vsync_ts;

	return ret;
}

int disp_hw_mgr_dump(uint32_t level)
{
	int i;
	struct disp_hw *drv;

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		drv = mgr->disp_hw_drv[i];
		if (drv && drv->dump)
			drv->dump(level);
	}

	return 0;
}

int disp_hw_mgr_send_event(enum DISP_EVENT event, void *data)
{
	_disp_event_callback(event, data);

	return 0;
}

int disp_hw_mgr_resume(void)
{
	int i, ret = 0;
	struct disp_hw *drv;
	enum DISP_MODULE_ENUM module;

	if (!IS_DISP_SUSPEND()) {
		DISP_LOG_E("the status is not suspend.\n");
		return 0;
	}

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.resume[i];
		drv = mgr->disp_hw_drv[module];
		if (drv && drv->resume)
			ret = drv->resume();
		if (ret != 0)
			break;
	}

	disp_path_reset();

	atomic_set(&mgr->status, DISP_STATUS_RESUME);

	return ret;
}

int disp_hw_mgr_suspend(void)
{
	int i, ret = 0;
	struct disp_hw *drv;
	enum DISP_MODULE_ENUM module;

	if (IS_DISP_SUSPEND()) {
		DISP_LOG_D("the status is suspend already.\n");
		return 0;
	}

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.suspend[i];
		drv = mgr->disp_hw_drv[module];
		if (drv && drv->suspend)
			ret = drv->suspend();
		if (ret != 0)
			break;
	}

	atomic_set(&mgr->status, DISP_STATUS_SUSPEND);

	return ret;
}

bool disp_hw_mgr_is_slept(void)
{
	return IS_DISP_SUSPEND();
}

int disp_hw_mgr_deinit(void)
{
	int i;
	struct disp_hw *drv;
	enum DISP_MODULE_ENUM module;

	if (IS_DISP_DEINIT()) {
		DISP_LOG_E("the status is deinited already\n");
		return 0;
	}

	for (i = 0; i < DISP_MODULE_NUM; i++) {
		module = mgr->sequence.init[i];
		drv = mgr->disp_hw_drv[module];
		if (drv && drv->deinit)
			drv->deinit();
	}

	atomic_set(&mgr->status, DISP_STATUS_DEINIT);

	return 0;
}

int disp_hw_mgr_status(void)
{
	struct disp_hw_tv_capbility *tv_cap;
	struct disp_hw_tv_capbility tmp_tv_cap;
	const struct disp_hw_resolution *resolution;

	tv_cap = &mgr->common_info.tv;
	resolution = mgr->common_info.resolution;

	DISP_LOG_I("disp hw mgr status:\n");
	DISP_LOG_I("tv cap, hdr static %d dolby %d dolby_2160p60 %d\n",
	tv_cap->is_support_hdr,
	tv_cap->is_support_dolby,
	tv_cap->is_support_dolby_2160p60);

	DISP_LOG_I("resolution %d\n", resolution->res_mode);

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	_get_hdmi_cap(&tmp_tv_cap);
#endif
	DISP_LOG_I("tmp tv cap, hdr static %d dolby %d dolby_2160p60 %d\n",
	tmp_tv_cap.is_support_hdr,
	tmp_tv_cap.is_support_dolby,
	tmp_tv_cap.is_support_dolby_2160p60);

	return 0;
}

