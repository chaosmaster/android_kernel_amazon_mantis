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



#define LOG_TAG "VDP"

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>

#include "vdp_hw.h"
#include "vdp_hal.h"
#include "disp_info.h"
#include "disp_vdp_if.h"
#include "disp_hw_mgr.h"
#include "disp_hw_log.h"
#include "mtk_sync.h"
#include "disp_clk.h"
#include "disp_path.h"
#include "disp_irq.h"

/*#include "vdp_fence.h"*/
/*#include "vdp_ion.h"*/
/*for kzalloc */
/*#include <linux/vmalloc.h>*/
#include <linux/slab.h>
#include "ion_drv.h"

/*#include "mtk_sync.h"*/

#include "disp_vdp_debug.h"
#include "disp_vdp_vsync.h"
#include "fmt_hal.h"
#include "fmt_def.h"
/*#include "disp_drv_platform.h"*/
#include "disp_vdp_cli.h"
#include "disp_dovi_tz_client.h"
#include "disp_dovi_main.h"

#define FENCE_TIMEOUT 1000
#define INVALID_PTS 0xffffffff

/*
** we don't have active start signal.
** but VSYNC IRQ is a little earlier before Active Start.
** So we delay 2ms in VSYNC IRQ to simulate Active Start.
*/
#define USE_FAKE_ACTIVE_START 0	/* set timer in VSYNC IRQ to simulate HW Active start */

unsigned int vdp_dbg_level = (0
/* | DDP_FUNC_LOG */
/* | DDP_FLOW_LOG */
/* | DDP_COLOR_FORMAT_LOG */
/* | DDP_FB_FLOW_LOG */
/* | DDP_RESOLUTION_LOG */
/* | DDP_OVL_FB_LOG */
	| VDP_FENCE_LOG
/* | DDP_TVE_FENCE_LOG */
/* | DDP_FENCE1_LOG */
/* | DDP_FENCE2_LOG */
);

struct list_head  vdp_buffer_lis;
static DEFINE_MUTEX(_disp_fence_mutex);

struct ion_client *vdp_ion_client;

static struct workqueue_struct *release_buffer_wq[2];

/* whether vdp init is OK.
** not allow to handle irq if disp_vdp_init() not run.
*/
static bool vdp_init_done;
static bool hdmi_resolution_changed;
HDMI_VIDEO_RESOLUTION current_resolution;

struct task_struct *vdp_set_input_buffer_worker_task;
wait_queue_head_t vdp_set_input_buffer_irq_wq;
atomic_t vdp_set_input_buffer_irq_event = ATOMIC_INIT(0);
struct task_struct *vdp_get_input_buffer_worker_task;
struct video_layer_info vdp_video_layer[VIDEO_LAYER_MAX_COUNT];
struct video_layer_info *video_layer = vdp_video_layer;
struct disp_hw_common_info disp_common_info;

enum DISP_DR_TYPE_T force_dr_range;

uint32_t vdp_disp_test;

void vdp_set_HDMI_BT2020_signal(bool enable_bt2020)
{
	/* store last time bt2020 signal status. */
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	static bool bt2020_enabled;

	if (enable_bt2020 && disp_common_info.tv.is_support_bt2020) {
		/* enable bt2020 */
		if (!bt2020_enabled) {
			DISP_LOG_N("enable BT2020 signal\n");
			vBT2020Enable(true);
		}
		bt2020_enabled = true;
	} else {
		/* disable bt2020 */
		if (bt2020_enabled) {
			DISP_LOG_N("disable BT2020 signal\n");
			vBT2020Enable(false);
		}
		bt2020_enabled = false;
	}
#endif
}

static void vdp_ion_init(void)
{
	if (!vdp_ion_client && g_ion_device)
		vdp_ion_client = ion_client_create(g_ion_device, "vdp");


	if (!vdp_ion_client) {
		DISP_LOG_E("create ion client failed!\n");
		return;
	}

	DISP_LOG_D("create ion client 0x%p\n", vdp_ion_client);
}

static struct ion_handle *vdp_ion_import_handle(struct ion_client *client, int fd)
{
	struct ion_handle *handle = NULL;
	struct ion_mm_data mm_data;
	/* If no need Ion support, do nothing! */
	if (fd == VDP_NO_ION_FD) {
		DISP_LOG_E("NO NEED ion support\n");
		return handle;
	}

	if (!client) {
		DISP_LOG_E("invalid ion client!\n");
		return handle;
	}
	if (fd == VDP_INVALID_ION_FD) {
		return handle;
	}
	handle = ion_import_dma_buf(client, fd);
	if (IS_ERR_OR_NULL(handle)) {
		DISP_LOG_E("import ion handle failed!\n");
		return handle;
	}
	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	mm_data.config_buffer_param.kernel_handle = handle;
	mm_data.config_buffer_param.module_id = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.coherent = 0;

	if (ion_kernel_ioctl(client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data))
		DISP_LOG_E("configure ion buffer failed!\n");

	return handle;
}


static size_t vdp_ion_phys_mmu_addr(struct ion_client *client, struct ion_handle *handle,
				      unsigned int *mva)
{
	size_t size;

	if (!client) {
		DISP_LOG_E("invalid ion client!\n");
		return 0;
	}
	if (IS_ERR_OR_NULL(handle))
		return 0;

	ion_phys(client, handle, (ion_phys_addr_t *) mva, &size);

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (vdp_cli_get()->enable_mva_debug)
		DISP_LOG_E("mva[0x%08x] va:[%p]\n", *mva, ion_map_kernel(client, handle));
#endif
	return size;
}


#if 0
int vdp_clear_incoming_buffer(disp_video_buffer_info *buffer_info)
{
	int i, ret = 0;
	struct sync_fence *fence;

	for (i = 0; i < MAX_VIDEO_INPUT_CONFIG; i++) {
		disp_video_layer_config *layer = &buffer_info->layers[i];

		if (layer->fence_fd < 0)
			continue; /* Nothing to wait on */

		if (layer->fence_fd == 0) {
			DDPERR(" invalid fence fd %d\n", layer->fence_fd);
			continue; /* Nothing to wait on */
		}
		fence = sync_fence_fdget(layer->fence_fd);
		if (!fence) {
			/* This is bad, but we just log an error and continue */
			DDPERR(" failed to import sync fence\n");
			fence = NULL;
			continue;
		}

		sync_fence_put(fence);
	}
	return ret;
}
#endif

int vdp_set_output_resolution(uint32_t res)
{
	unsigned int ret = 0;

	return ret;
}

static int _vdp_parse_dev_node(void)
{
	struct device_node *np;
	unsigned int reg_value;
	unsigned int irq_value;
	struct disp_hw *vdp_drv = disp_vdp_get_drv();


	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-vdo");
	if (np == NULL) {
		DISP_LOG_E("dts error, no vdo device node.\n");
		return VDP_DTS_ERROR;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);
	of_property_read_u32_index(np, "interrupts", 1, &irq_value);

	disp_vdo_reg_base[0] = (char *)of_iomap(np, 0); /*vdo3, 15001000 */
	disp_vdo_reg_base[1] = (char *)of_iomap(np, 1); /*vdo4, 15008000 */

	vdp_drv->irq[0].value = irq_of_parse_and_map(np, 0);

	if (vdp_drv->irq[0].value == 0)
		DISP_LOG_E("VDP get irq from dts fail\n");
	else {
		vdp_drv->irq[0].irq = DISP_IRQ_DISP3_VSYNC;
		vdp_drv->irq_num = 1;
	}

	if (vdp_drv->irq[0].value == 0)
		return VDP_DTS_ERROR;

	return VDP_OK;


}

#if 0
static int _h_down_cap(uint32_t layer_id, uint32_t src_w, uint32_t out_w, uint32_t out_h)
{
	int factor = 0;
	int in_h_limit = 0;

	DISP_LOG_D("layer_id: %d, src_w: %d, out_w: %d, out_h: %d\n",
		   layer_id, src_w, out_w, out_h);

	factor = (src_w * 0x1000) / out_w;

	if (out_h == 2160)
		in_h_limit = 4096;
	else if (out_h == 1080)
		in_h_limit = 1920;
	else if (out_h == 720)
		in_h_limit = 1280;
	else if (out_h < 720)
		in_h_limit = 720;
	else
		in_h_limit = 0;

	if ((out_w > src_w) || (out_w == 0) || (src_w == 0) || (factor > 0xFFFF)
	    || (out_w & 1) || (src_w > in_h_limit)
		) {
		DISP_LOG_D("Not able to downscale\n");
		return 0;
	}

	DISP_LOG_D("Be able to downscale\n");
	return 1;
}
#endif

bool force_dsd_off;
static int _vdp_dsd_cap(struct mtk_disp_vdp_cap *vdp_cap, uint32_t out_w, uint32_t out_h)
{
	if (force_dsd_off)
		return 0;

	if (((vdp_cap->src.width > out_w) ||
		(vdp_cap->src.height > out_h))) {
		if (out_h < vdp_cap->tgt.height * 2)
			return 1;
		else
			return 0;
	} else
		return 0;
}

void disp_vdp_enable_premix_clock(bool enable)
{
	static uint32_t clock_enabled;

	if (enable && !clock_enabled) {
		clock_enabled = true;
		DISP_LOG_I("enable OSD premix\n");
		//disp_clock_enable(DISP_CLK_OSD_PREMIX, clock_enabled);
		#if 1
		disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &clock_enabled);
		//disp_vdp_get_drv()->drv_call(DISP_CMD_DOVI_CORE2_UPDATE, &clock_enabled);
		#endif
		return;
	}

	if (!enable && clock_enabled) {
		clock_enabled = false;
		DISP_LOG_I("disable OSD premix\n");
		//disp_clock_enable(DISP_CLK_OSD_PREMIX, clock_enabled);
		disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &clock_enabled);
		//disp_vdp_get_drv()->drv_call(DISP_CMD_DOVI_CORE2_UPDATE, &clock_enabled);
		return;
	}

}

#if (VIDEO_USE_HW_SHADOW && VIDEO_REDUCE_BUFFER)
void disp_vdp_release_buffer(struct work_struct *work)
{
	struct video_layer_info *layer_info = NULL;

	layer_info = container_of(work, struct video_layer_info, task_work);
	if (layer_info == NULL) {
		DISP_LOG_E("get VDP layer info NULL\n");
		return;
	}

	mutex_lock(&(layer_info->sync_lock));
	do {
		struct video_buffer_info *buf = NULL;
		struct video_buffer_info *temp = NULL;

		list_for_each_entry_safe(buf, temp, &(layer_info->buf_list), list) {
			if (layer_info->release_timeline_idx > buf->current_fence_index) {
				/* signal the fence */
				if (layer_info->timeline != NULL) {
					timeline_inc((struct sw_sync_timeline *)layer_info->timeline, 1);
					buf->time_fence_signal = sched_clock();
					if (print_video_fence_history)
						DISP_LOG_N("video fence%d alive time %lld, wait disp 0x%llx\n",
						buf->current_fence_index,
						(buf->time_fence_signal - buf->time_fence_create),
						(buf->time_start_display - buf->time_fence_create));
				}

				layer_info->release_idx++;
				/*free ion handle */
				vdp_ion_free_handle(vdp_ion_client, buf->ion_hnd);

				/* add buffer list to free buffer pool */
				list_del_init(&buf->list);
				release_buf_info(&buf->list, layer_info->layer_id);
				break;
			}
		}

	} while (0);
	mutex_unlock(&(layer_info->sync_lock));
}
#endif

bool enable_frame_drop_log;

int disp_vdp_init(struct disp_hw_common_info *info)
{
	int i = 0;
	char name[16] = {0};

	DISP_LOG_D("disp_vdp_init start\n");
	/*parser dts file for register base & irq */
	_vdp_parse_dev_node();
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	vdp_cli_init();
#endif
	if (g_hdmi_res == HDMI_VIDEO_1280x720p_59_94Hz)
		old_resolution = HDMI_VIDEO_1280x720p_60Hz;
	else if (g_hdmi_res == HDMI_VIDEO_1920x1080p_59_94Hz)
		old_resolution = HDMI_VIDEO_1920x1080p_60Hz;
	else if (g_hdmi_res == HDMI_VIDEO_3840x2160P_59_94HZ)
		old_resolution = HDMI_VIDEO_3840x2160P_60HZ;
	else
		old_resolution = g_hdmi_res;

	dolby_out_format = g_out_format;
	DISP_LOG_N("%s get from lk: old_resolution = %d, dolby_out_format = %d\n",
		__func__, old_resolution, dolby_out_format);
	/*Init variables */
	for (i = 0; i < VIDEO_LAYER_MAX_COUNT; i++) {
		memset((void *)(&video_layer[i]), 0, sizeof(struct video_layer_info));
		video_layer[i].layer_id = i;
		sprintf(name, "vdp_timeline%d", i);
		video_layer[i].timeline =
			(struct sw_sync_timeline *)timeline_create(name);
		INIT_LIST_HEAD(&video_layer[i].buf_list);
		mutex_init(&(video_layer[i].sync_lock));
		video_layer[i].inited = 1;
		video_layer[i].timeline_idx = 0;
		video_layer[i].release_timeline_idx = 0;
		video_layer[i].release_idx = 0;
		video_layer[i].last_pts = 0xffffffff;
		video_layer[i].fence_idx = 0;
		video_layer[i].display_duration = 0;
		video_layer[i].vsync_duration = 0;
		video_layer[i].enable = false;
		video_layer[i].layer_start = false;
		video_layer[i].dsd_en = false;
		video_layer[i].sdr2hdr = false;
		video_layer[i].secure_en = false;
		video_layer[i].secure2normal = false;
		video_layer[i].res_change = false;
		video_layer[i].state = VDP_LAYER_IDLE;

		#if (VIDEO_USE_HW_SHADOW && VIDEO_REDUCE_BUFFER)
		INIT_WORK(&video_layer[i].task_work, disp_vdp_release_buffer);
		#endif
		init_waitqueue_head(&video_layer[i].wait_queue);
	}

	memset((void *)&disp_common_info, 0, sizeof(struct disp_hw_common_info));
	vdp_ion_init();

	/* create work queue for release buffer */
	release_buffer_wq[0] = create_singlethread_workqueue("vdp0_release_buffer");
	release_buffer_wq[1] = create_singlethread_workqueue("vdp1_release_buffer");

	/*Create semaphore */

	/*Create thread */

	vdp_debug_init();


	vdp_vsync_init();

	/*vdp_hal_init();*/

	/*vdp_kthread_init();*/

	force_dsd_off = true;
	vdp_init_done = true;
	enable_frame_drop_log = false;
	DISP_LOG_D("disp_vdp_init end\n");
	return VDP_OK;
}

int disp_vdp_deinit(void)
{
	ion_client_destroy(vdp_ion_client);
	return VDP_OK;
}

bool disp_vdp_check_layer_id(const int layer_id, const int line_num)
{
	if (layer_id >= VIDEO_LAYER_MAX_COUNT) {
		DISP_LOG_E("video layer index[%d] invalid, line[%d]\n",
			layer_id, line_num);
		return false;
	}
	return true;
}

int disp_vdp_get_vsync_duration(const struct disp_hw_resolution *res)
{
	int vsync_duration = TIME_BASE / res->frequency;

	if (res->is_fractional) {
		if (res->frequency == 60)
			vsync_duration = TIME_BASE * 100 / (res->frequency * 100 - 6);
		else if (res->frequency == 30)
			vsync_duration = TIME_BASE * 100 / (res->frequency * 100 - 3);
		else if (res->frequency == 24)
			vsync_duration = TIME_BASE * 1000 / (res->frequency * 1000 - 24);
	}
	DISP_LOG_N("the vsync_duration is %d\n", vsync_duration);

	return vsync_duration;
}

uint32_t disp_vdp_get_source_duration(uint32_t fps, const struct disp_hw_resolution *info)
{
	uint32_t source_duration = TIME_BASE * 100 / 5994; /* 59.94 fps */
	if (fps != 0) {
		if (fps == 2397)
			source_duration = TIME_BASE * 1000 / 23976;
		else
			source_duration = TIME_BASE * 100 / fps;
	}

	/* adjust 60fps, must not adjust 59.94 fps resolution. will cause av not sync */
	if (info->frequency == 60 && !info->is_fractional) {
		/* 60fps */
		switch (fps) {
		case 2397:
		case 2400:
			/*output: 2 3 2 3 2 3 ... */
			source_duration = TIME_BASE * 100 / 2400;
			break;
		case 2997:
		case 3000:
			/* output: 2 2 2 2 ... */
			source_duration = TIME_BASE * 100 / 3000;
			break;
		case 5994:
		case 6000:
			/* output: 1 1 1 1 ... */
			source_duration = TIME_BASE * 100 / 6000;
			break;
		case 2500:
		case 5000:
		default:
			break;
		}
	}

	return source_duration;
}


/*Open the power & clock */
int disp_vdp_start(struct disp_hw_common_info *info, unsigned int layer_id)
{
	enum FMT_TV_TYPE tv_type;
	struct video_layer_info *layer_info = NULL;
	int ret = 0;

	/*vdo init */
	if (!disp_vdp_check_layer_id(layer_id, __LINE__))
		return VDP_INVALID_INDEX;

	layer_info = &video_layer[layer_id];

	/* VDP is already started bypass. */
	if (layer_info->layer_start)
		return VDP_OK;
	layer_info->layer_start = true;

	/* if we stop, then start in next few vsync, maybe stop flow has not finished yet.
	** we need to wait last stop flow finished, then run start flow.
	*/
	ret = wait_event_timeout(layer_info->wait_queue,
			layer_info->state == VDP_LAYER_IDLE,
			msecs_to_jiffies(1000));

	if (ret == 0) {
		DISP_LOG_E("wait layer state idle timeout\n");
		return 0;
	}

	vdp_hal_init(layer_id);
	memcpy((void *)&disp_common_info, (const void *)info, sizeof(struct disp_hw_common_info));
	if (info->resolution->frequency == 25 || info->resolution->frequency == 50)
		tv_type = FMT_TV_TYPE_PAL;
	else
		tv_type = FMT_TV_TYPE_NTSC;

	DISP_LOG_I("disp_vdp_start%d, res %d\n", layer_id, info->resolution->res_mode);
	/*enable disp_fmt */
	if (layer_id == 0) {
		fmt_hal_clock_on_off(DISP_FMT_MAIN, true);
		fmt_hal_set_mode(DISP_FMT_MAIN, info->resolution->res_mode, true);
		if ((ui_force_hdr_type > 1) && info->tv.is_support_dolby) {
			vdp_dovi_set_out_format(info, NULL);
			vdp_update_dovi_path_delay(false);
		} else
			disp_path_set_delay(DISP_PATH_MVDO, info->resolution->res_mode);
		fmt_hal_set_tv_type(DISP_FMT_MAIN, tv_type);
		fmt_hal_set_uv_swap(DISP_FMT_MAIN, false);
		fmt_hal_set_h_scale_coef(DISP_FMT_MAIN, 1);
		fmt_hal_set_output_444(DISP_FMT_MAIN, true);
		fmt_hal_hw_shadow_enable(DISP_FMT_MAIN);
		disp_clock_enable(DISP_CLK_VDO3, true);
		disp_sys_hal_video_ultra_en(DISP_MAIN, true);
		disp_clock_smi_larb_en(DISP_SMI_LARB5, true);
		vdp_disable_active_zone(layer_info);
		video_layer[0].vsync_duration = disp_vdp_get_vsync_duration(info->resolution);
		video_layer[0].display_duration = 0;
		dolby_path_start = 1;
		hdr10_plus_enable = false;
	} else if (layer_id == 1) {
		fmt_hal_clock_on_off(DISP_FMT_SUB, true);
		fmt_hal_set_mode(DISP_FMT_SUB, info->resolution->res_mode, true);
		#if 0
		if (dolby_path_enable) {
			vdp_update_dovi_path_delay(false);
		} else
			disp_path_set_delay(DISP_PATH_SVDO, info->resolution->res_mode);
		#endif
		fmt_hal_set_tv_type(DISP_FMT_SUB, tv_type);
		fmt_hal_set_uv_swap(DISP_FMT_SUB, false);
		fmt_hal_set_h_scale_coef(DISP_FMT_SUB, 1);
		fmt_hal_set_output_444(DISP_FMT_SUB, true);
		fmt_hal_hw_shadow_enable(DISP_FMT_SUB);
		disp_clock_enable(DISP_CLK_VDO4, true);
		disp_sys_hal_video_preultra_en(DISP_SUB, true);
		disp_clock_smi_larb_en(DISP_SMI_LARB6, true);
		video_layer[1].vsync_duration = disp_vdp_get_vsync_duration(info->resolution);
		video_layer[1].display_duration = 0;

		vdp_disable_active_zone(layer_info);

		DISP_LOG_D("call sub path dolby_enable%d\n", dolby_path_enable);
		#if 1
		/* Plan A */
		/* main path is playing dolby video, sub path should enable osd premix. */
		if (dolby_path_enable) {
			disp_path_set_delay(DISP_PATH_SVDO_DOLBY_CORE2, info->resolution->res_mode);
		} else
			disp_path_set_delay(DISP_PATH_SVDO, info->resolution->res_mode);
		#else
		/* Plan C2 */
		/* main path is playing dolby video, sub path should disable and swap with main path in roution. */
		if (dolby_path_enable) {
			uint32_t disable = 0;
			/* disable vdo4 output to pre-mix */
			disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &disable);

			/* disable YUV444 */
			fmt_hal_set_output_444(1, 0);
			vdp_update_dovi_path_delay(false);
		} else
			disp_path_set_delay(DISP_PATH_SVDO, info->resolution->res_mode);
		#endif
	} else {
		DISP_LOG_E("Invalid video id %d\n", layer_id);
		return VDP_INVALID_INDEX;
	}
	video_layer[layer_id].state = VDP_LAYER_START;
	video_layer[layer_id].dsd_en = false;
	video_layer[layer_id].type = DISP_LAYER_UNKNOW;

	/* vdp_disable_active_zone(layer_info); */

	/*dispfmt setting? */
	return VDP_OK;
}

/*Close the power & clock*/
int disp_vdp_stop(unsigned int layer_id)
{
	/* struct vdp_hal_config_info config_info = {0}; */
	if (!disp_vdp_check_layer_id(layer_id, __LINE__))
		return VDP_INVALID_INDEX;

	/* VDP is already stopped bypass. */
	if (!video_layer[layer_id].layer_start) {
		DISP_LOG_N("vdp not start, so no need do stop\n");
		return VDP_OK;
	}
	video_layer[layer_id].layer_start = false;

	DISP_LOG_N("disp_vdp_stop %d\n", layer_id);

	if ((dolby_path_enable == 1) && (!netflix_dolby_path_enable)
		&& (!dovi_idk_dump) && (layer_id == 0)
		&& (dolby_path_enable_cnt >= 2)) {
		disp_dovi_set_mute_unmute(true);
		fmt_hal_not_mix_plane(FMT_HW_PLANE_3);
	}

	video_layer[layer_id].type = DISP_LAYER_UNKNOW;
	video_layer[layer_id].state = VDP_LAYER_STOPPING;
	video_layer[layer_id].last_pts = INVALID_PTS;

	return VDP_OK;
}

int disp_vdp_suspend(void)
{
	int layer_enable = video_layer[0].layer_start | (video_layer[1].layer_start << 1);
	int ret = 0;
	int i = 0;

	for (i = 0; i < 2; i++) {
		if (layer_enable & BIT(i))
			disp_vdp_stop(i);
	}

	for (i = 0; i < 2; i++) {
		if (layer_enable & BIT(i)) {
			ret = wait_event_timeout(video_layer[i].wait_queue,
						video_layer[i].state == VDP_LAYER_IDLE,
						msecs_to_jiffies(1000));

			if (ret == 0) {
				DISP_LOG_E("wait layer[%d] state idle timeout\n", i);
				return -1;
			}
		}
	}

	return VDP_OK;
}

int disp_vdp_resume(void)
{
	return VDP_OK;
}

bool disp_vdp_check_dsd_condition(HDMI_VIDEO_RESOLUTION res_mode,
	uint32_t src_width, uint32_t src_height)
{
	if ((res_mode == HDMI_VIDEO_1920x1080p_60Hz) ||
		(res_mode == HDMI_VIDEO_1920x1080p_50Hz) ||
		(res_mode == HDMI_VIDEO_1280x720p_60Hz) ||
		(res_mode == HDMI_VIDEO_1280x720p_50Hz) ||
		(res_mode == HDMI_VIDEO_720x480p_60Hz) ||
		(res_mode == HDMI_VIDEO_720x576p_50Hz)) {
		/*1080p to 720p can't use dsd solution */
		if ((res_mode == HDMI_VIDEO_1280x720p_60Hz) ||
			(res_mode == HDMI_VIDEO_1280x720p_50Hz)) {
			if ((src_width <= 1920) && (src_height <= 1080))
				return false;
			else
				return true;
		} else
			return true;
	} else
		return false;
}

int disp_vdp_get_info(struct disp_hw_common_info *info)
{
	bool use_crop = false;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t dst_width = 0;
	uint32_t dst_height = 0;
	struct mtk_disp_vdp_cap *vdp_cap;

	vdp_cap = &(info->vdp_cap);

	vdp_cap->need_resizer = false;	/*set default value */
	if (((vdp_cap->src.width != vdp_cap->crop.width) ||
	    (vdp_cap->src.height != vdp_cap->crop.height)) &&
	    (vdp_cap->crop.width != 0) && (vdp_cap->crop.height != 0)) {
		use_crop = true;
		width = vdp_cap->crop.width;
		height = vdp_cap->crop.height;
	} else {
		use_crop = false;
		width = vdp_cap->src.width;
		height = vdp_cap->src.height;
	}
	dst_width = vdp_cap->tgt.width;
	dst_height = vdp_cap->tgt.height;

	DISP_LOG_MSG("src(%d %d %d %d) crop(%d %d %d %d) tgt(%d %d %d %d) res(%d %d)\n",
		   vdp_cap->src.x, vdp_cap->src.y, vdp_cap->src.width,
		   vdp_cap->src.height, vdp_cap->crop.x, vdp_cap->crop.y,
		   vdp_cap->crop.width, vdp_cap->crop.height,
		   vdp_cap->tgt.x, vdp_cap->tgt.y, vdp_cap->tgt.width,
		   vdp_cap->tgt.height, info->resolution->width,
		   info->resolution->height);

	do {
		/* for vdp dump buffer debug:
		** force use MDP to dump MDP buffer
		** the MDP output buffer is VDP input buffer
		*/
		#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
		if (vdp_cli_get()->enable_force_use_mdp) {
			DISP_LOG_E("force use MDP for debug\n");
			vdp_cap->need_resizer = true;
			break;
		}
		#endif

		/* buffer & HDMI resolution are not the same.
		** in this case, can't use H Down Scale
		** only can use DSD or imgresz.
		*/
		if ((width > info->resolution->width) ||
			(height > info->resolution->height)) {
			if (!_vdp_dsd_cap(vdp_cap, info->resolution->width, info->resolution->height))
				vdp_cap->need_resizer = true;	/* DSD not available */
			else {
				if (disp_vdp_check_dsd_condition(info->resolution->res_mode,
					width, height))
					vdp_cap->need_resizer = false;	/* DSD available */
				else
					vdp_cap->need_resizer = true;
			}

			break;
		}

		/* buffer & HDMI resolution are the same.
		** in this case, we can use H Down scale
		*/
		if ((width >= dst_width * 8) || (height >= dst_height * 2))
			vdp_cap->need_resizer = true;
	} while (0);

	DISP_LOG_I("use_resizer = %d\n", vdp_cap->need_resizer);
	return VDP_OK;
}

int disp_vdp_change_resolution(const struct disp_hw_resolution *info)
{
	int h_start, v_start_odd, v_start_even;
	struct disp_hw_tv_capbility tv_cap;
	enum FMT_TV_TYPE tv_type;
	struct disp_hw *vdp_drv = disp_vdp_get_drv();

	if (dolby_path_enable) {
		disp_dovi_resolution_change(info);
		vdp_drv->drv_call(DISP_CMD_GET_HDMI_CAP, &tv_cap);
		vdp_update_dovi_setting_res_change(&tv_cap, info);
	}

	if (current_resolution == info->res_mode)
		return VDP_OK;
	current_resolution = info->res_mode;

	hdmi_resolution_changed = true;

	if (info->frequency == 25 || info->frequency == 50)
		tv_type = FMT_TV_TYPE_PAL;
	else
		tv_type = FMT_TV_TYPE_NTSC;

	disp_path_get_active_zone(0, info->res_mode,
				&h_start, &v_start_odd, &v_start_even);

	if (video_layer[0].enable) {
		/* set HTotal & VTotal pixel for spec resolution. */
		fmt_hal_set_mode(DISP_FMT_MAIN, info->res_mode, true);
		fmt_hal_set_tv_type(DISP_FMT_MAIN, tv_type);
		fmt_hal_enable(DISP_FMT_MAIN, true);
		if (dolby_path_enable != 1)
			disp_path_set_delay(DISP_PATH_MVDO, info->res_mode);
		fmt_hal_hw_shadow_enable(DISP_FMT_MAIN);
		video_layer[0].vsync_duration = disp_vdp_get_vsync_duration(info);

		if (video_layer[0].secure_en)
			video_layer[0].res_change = true;
	}

	if (video_layer[1].enable) {
		/* set HTotal & VTotal pixel for spec resolution. */
		fmt_hal_set_mode(DISP_FMT_SUB, info->res_mode, true);
		fmt_hal_set_tv_type(DISP_FMT_SUB, tv_type);
		fmt_hal_enable(DISP_FMT_SUB, true);
		if (dolby_path_enable != 1)
			disp_path_set_delay(DISP_PATH_SVDO, info->res_mode);
		if (video_layer[1].sdr2hdr)
			disp_path_set_delay(DISP_PATH_SVDO_SDR2HDR, info->res_mode);
		else
			disp_path_set_delay(DISP_PATH_SVDO_DOLBY_CORE2, info->res_mode);

		fmt_hal_hw_shadow_enable(DISP_FMT_SUB);
		video_layer[1].vsync_duration = disp_vdp_get_vsync_duration(info);

		if (video_layer[1].secure_en)
			video_layer[1].res_change = true;
	}

	vdp_hal_config_timing(0, info, h_start, v_start_odd, v_start_even);

	return VDP_OK;
}

int disp_vdp_get_mva(int ion_fd, struct video_buffer_info *buf_info)
{
	bool is_Y_C_independent = false;
	unsigned int mva = 0;

	if ((ion_fd >> 16) > 0)
		is_Y_C_independent = true;

	/*get ion handle */
	if (is_Y_C_independent) {
		buf_info->ion_hnd =
			vdp_ion_import_handle(vdp_ion_client, ion_fd & 0xFFFF);
		buf_info->ion_hnd2 =
			vdp_ion_import_handle(vdp_ion_client, (ion_fd & 0xFFFF0000) >> 16);
	} else
		buf_info->ion_hnd = vdp_ion_import_handle(vdp_ion_client, ion_fd);

	if (IS_ERR_OR_NULL(buf_info->ion_hnd)) {
		if (is_Y_C_independent && !(IS_ERR_OR_NULL(buf_info->ion_hnd2)))
			vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd2);
		/* DISP_LOG_E("get video buffer ion handle fail\n"); */
		return VDP_GET_ION_HND_FAIL;
	}
	if (is_Y_C_independent && IS_ERR_OR_NULL(buf_info->ion_hnd2)) {
		vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd);
		/* DISP_LOG_E("get video buffer ion handle fail\n"); */
		return VDP_GET_ION_HND_FAIL;
	}


	/*get physical address */
	vdp_ion_phys_mmu_addr(vdp_ion_client, buf_info->ion_hnd, &mva);
	if (mva != 0)
		buf_info->src_phy_addr = mva;
	else {
		DISP_LOG_E("Get invalid physical memory\n");
		vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd);
		if (is_Y_C_independent)
			vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd2);
		return VDP_GET_PHY_FAIL;
	}
	if (is_Y_C_independent) {
		vdp_ion_phys_mmu_addr(vdp_ion_client, buf_info->ion_hnd2, &mva);
		if (mva != 0)
			buf_info->src_phy_addr2 = mva;
		else {
			DISP_LOG_E("Get invalid physical memory\n");
			vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd);
			vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd2);
			return VDP_GET_PHY_FAIL;
		}
	}
	return 0;

}


int config_frame_count[2];	/* debug vdp valid config */
int config_buffer_count[2]; /* debug vdp total buffer(contain invalid buffer)*/

int disp_vdp_config(struct mtk_disp_buffer *config, struct disp_hw_common_info *info)
{
	/*put the new buffer into the buffer list */
	int ret = VDP_OK;
	struct video_buffer_info *buf_info = NULL;
	struct fence_data fence;
	bool is_Y_C_independent = false;

	config_buffer_count[config->layer_id]++;

	if ((config->ion_fd >> 16) > 0)
		is_Y_C_independent = true;

	if ((video_layer[config->layer_id].state !=  VDP_LAYER_START) &&
		(video_layer[config->layer_id].state != VDP_LAYER_RUNNING)) {
		if (config->ion_fd != VDP_INVALID_ION_FD) {
			DISP_LOG_E("not start yet, drop frame: id[%d] state[%d] fd %d\n",
				config->layer_id,
				video_layer[config->layer_id].state, config->ion_fd);
		}
		return VDP_NOT_START;
	}

	/* check buffer PTS and src/target window */
	if (config->pts == video_layer[config->layer_id].last_pts && !hdmi_resolution_changed) {
		if (!memcmp(&video_layer[config->layer_id].src_rgn, &config->src, sizeof(struct mtk_disp_range)) &&
			!memcmp(&video_layer[config->layer_id].tgt_rgn, &config->tgt, sizeof(struct mtk_disp_range))) {
			DISP_LOG_D("Same video buffer,skip,layerID[%d] pts[%lld]\n",
				config->layer_id,
				config->pts);
			return VDP_OK;
		}
	}
	hdmi_resolution_changed = false;

	/* get video_buffer_info to store information in mtk_disp_buffer */
	buf_info = vdp_get_buf_info(config->layer_id);
	if (buf_info == NULL) {
		DISP_LOG_E("get layer[%d] input buffer fail\n",
			config->layer_id);
		return VDP_FAIL;
	}

	/* convert ion_fd to MVA/secure_handle, store in buf_info->src_phy_addr
	** for normal buffer: the convert result is MVA.
	** for secure buffer: the convert result is secure handle.
	*/
	ret = disp_vdp_get_mva(config->ion_fd, buf_info);
	if (ret < 0) {
		if ((video_layer[config->layer_id].type == config->type) &&
			(video_layer[config->layer_id].type != DISP_LAYER_UNKNOW))
			DISP_LOG_E("buffer_layer_id: %d get mva fail ionfd:0x%x waitFence[0x%x]\n",
				config->layer_id,
				config->ion_fd,
				config->acquire_fence_fd);
		/* release acquired buffer info */
		/* buf_info is not add to any list yet, no need to lock. */
		list_del_init(&buf_info->list);
		release_buf_info(&buf_info->list, config->layer_id);
		return ret;
	}

	video_layer[config->layer_id].type = config->type;

	/* calculate buffer display time */
	buf_info->source_duration = disp_vdp_get_source_duration(config->fps, info->resolution);

	/* playback window change with video in paused state - refresh display asap */
	if (config->fps != 0) {
		if (memcmp(&video_layer[config->layer_id].tgt_rgn, &config->tgt, sizeof(struct mtk_disp_range))
			|| (video_layer[config->layer_id].last_pts == config->pts))
			buf_info->source_duration = 0;
	}

	/*for frame drop check */
	if (config->fps != 0) {
		if ((config->pts - video_layer[config->layer_id].last_pts) >
			buf_info->source_duration + 300) { /*300 is deviation */
			if (enable_frame_drop_log)
				DISP_LOG_E("drop frame, last[%lld], current[%lld], fps[%d]\n",
					video_layer[config->layer_id].last_pts,
					config->pts, config->fps);
			else
				DISP_LOG_D("drop frame, last[%lld], current[%lld], fps[%d]\n",
					video_layer[config->layer_id].last_pts,
					config->pts, config->fps);
		}
	}

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	buf_info->source_duration *= vdp_cli_get()->slow;
#endif

	memcpy((void *)(&(buf_info->crop)), (const void *)(&(config->crop)), sizeof(struct mtk_disp_range));
	memcpy((void *)(&(buf_info->src)), (const void *)(&(config->src)), sizeof(struct mtk_disp_range));
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (vdp_cli_get()->target_area.enable && vdp_cli_get()->target_area.layer_id == config->layer_id) {
		/* for debug use.
		** adjust buffer display (x_offset, yoffset) and width & height
		** use command: cli vdp.show 0 0 1920 1080 1920
		** offset(0, 0) width 1920 height 1080 pitch 1920
		*/
		memcpy((void *)(&(buf_info->tgt)),
			(const void *)(&(vdp_cli_get()->target_area.range)),
			sizeof(struct mtk_disp_range));
	} else
#endif
		memcpy((void *)(&(buf_info->tgt)),
			(const void *)(&(config->tgt)),
			sizeof(struct mtk_disp_range));

	if (buf_info->src.width == 0) {
		buf_info->src.width = 1920;
		buf_info->src.height = 1080;
		buf_info->src.pitch = 1920;
	}
	if (buf_info->crop.width == 0) {
		buf_info->crop.width = buf_info->src.width;
		buf_info->crop.height = buf_info->src.height;
	}

	if (buf_info->crop.width > 4096 || buf_info->crop.height > 2176) {
		DISP_LOG_E("error crop info (%d %d)\n", buf_info->crop.width, buf_info->crop.height);
		goto release_ion_handle;
	}

	/* for dolby unitTest */
	if (vdp_disp_test) {
		if (vdp_disp_test == 1) {
			buf_info->tgt.x = 0;
			buf_info->tgt.y = 0;
			buf_info->tgt.width = disp_common_info.resolution->width;
			buf_info->tgt.height = disp_common_info.resolution->height;
		}

		if (vdp_disp_test > 1) {
			buf_info->src.x = 0;
			buf_info->src.y = 0;
			buf_info->src.width = disp_common_info.resolution->width;
			buf_info->src.height = disp_common_info.resolution->height;

			buf_info->crop.x = 0;
			buf_info->crop.y = 0;
			buf_info->crop.width = disp_common_info.resolution->width;
			buf_info->crop.height = disp_common_info.resolution->height;
		}
	}


	buf_info->layer_id = config->layer_id;
	buf_info->buf_state = BUFFER_CREATE;
	buf_info->src_fmt = config->src_fmt;
	buf_info->ion_fd = config->ion_fd;
	buf_info->is_10bit = config->is_10bit;
	buf_info->is_10bit_lbs2bit_tile_mode = config->is_10bit_lbs2bit_tile_mode;
	buf_info->is_bt2020 = config->is_bt2020;
	buf_info->is_dolby = config->is_dolby;
	buf_info->is_pack_mode = config->is_pack_mode;
	buf_info->is_interlace = !config->is_progressive;
	buf_info->is_seamless = config->is_seamless;
	buf_info->is_ufo = config->is_ufo;
	buf_info->acquire_fence_fd = config->acquire_fence_fd;
	buf_info->src_fmt = config->src_fmt;
	buf_info->video_type = config->video_type;
	buf_info->ofst_y = config->ofst_y;
	buf_info->ofst_c = config->ofst_c;
	buf_info->ofst_y_len = config->ofst_y_len;
	buf_info->ofst_c_len = config->ofst_c_len;
	buf_info->buffer_size = config->buffer_size;
	buf_info->secruity_en = config->secruity_en;
	buf_info->is_jumpmode = config->is_jumpmode;
	buf_info->pts = config->pts;

	/* fill HDR10 type */
	if (config->is_hdr10plus)
		buf_info->hdr10_type = HDR10_TYPE_PLUS;
	else if (config->is_hdr) {
		switch (config->hdr_info.transformCharacter) {	/* 16 for ST2084, 18 for HLG */
		case 16:
			buf_info->hdr10_type = HDR10_TYPE_ST2084;
			break;
		case 18:
			buf_info->hdr10_type = HDR10_TYPE_HLG;
			break;
		default:
			DISP_LOG_E("is_hdr:%d invalid hdr type: %d\n",
				config->is_hdr,
				config->hdr_info.transformCharacter);
			goto release_ion_handle;
		}
	} else
		buf_info->hdr10_type = HDR10_TYPE_NONE;

	/*
	** Note: HDR10 plus also enable config->is_hdr & config->is_hdr10plus
	** for HDR10 plus we also need to copy HDR10 meta data & HDR10 plus metadata.
	** when TV don't support HDR10 plus we use HDR10 signal.
	*/
	if (config->is_hdr) {
		buf_info->hdr10_info.fgNeedUpdStaticMeta = true;
		buf_info->hdr10_info.ui2_DisplayPrimariesX[0] = (unsigned short)config->hdr_info.displayPrimariesX[0];
		buf_info->hdr10_info.ui2_DisplayPrimariesX[1] = (unsigned short)config->hdr_info.displayPrimariesX[1];
		buf_info->hdr10_info.ui2_DisplayPrimariesX[2] = (unsigned short)config->hdr_info.displayPrimariesX[2];
		buf_info->hdr10_info.ui2_DisplayPrimariesY[0] = (unsigned short)config->hdr_info.displayPrimariesY[0];
		buf_info->hdr10_info.ui2_DisplayPrimariesY[1] = (unsigned short)config->hdr_info.displayPrimariesY[1];
		buf_info->hdr10_info.ui2_DisplayPrimariesY[2] = (unsigned short)config->hdr_info.displayPrimariesY[2];
		buf_info->hdr10_info.ui2_MaxCLL = (unsigned short)config->hdr_info.maxCll;
		buf_info->hdr10_info.ui2_MaxDisplayMasteringLuminance =
			(unsigned short)config->hdr_info.maxDisplayMasteringLuminance;
		buf_info->hdr10_info.ui2_MaxFALL = (unsigned short)config->hdr_info.maxFall;
		buf_info->hdr10_info.ui2_MinDisplayMasteringLuminance =
			(unsigned short)config->hdr_info.minDisplayMasteringLuminance;
		buf_info->hdr10_info.ui2_WhitePointX = (unsigned short)config->hdr_info.whitePointX;
		buf_info->hdr10_info.ui2_WhitePointY = (unsigned short)config->hdr_info.whitePointY;
	}

	/*copy hdr10+ metadata */
	if (config->is_hdr10plus) {
		buf_info->hdr_info.metadata_info.dovi_metadata.len = config->dolby_info.len;
		if (!config->secruity_en) {	/* normal display buffer */
			if (config->dolby_info.len >= DOVI_MD_MAX_LEN) {
				DISP_LOG_E("hdr10 plus metadata size too long: %d\n", config->dolby_info.len);
				goto release_ion_handle;
			}

			if (copy_from_user(buf_info->hdr_info.metadata_info.dovi_metadata.buff,
				(void __user *)(config->dolby_info.addr),
				config->dolby_info.len)) {
				DISP_LOG_E("copy hdr10 plus metadata fail\n");
				buf_info->meta_data_size = 0;
				goto release_ion_handle;
			}
		} else {	/* secure display buffer */
			buf_info->hdr_info.metadata_info.dovi_metadata.sec_handle = config->dolby_info.sec_handle;
			buf_info->hdr_info.metadata_info.dovi_metadata.len = config->dolby_info.len;
			buf_info->hdr_info.metadata_info.dovi_metadata.svp = true;
		}
	}

	vdp_printf(VDP_AVSYNC_LOG, "dolby %d hdr_type %d\n", buf_info->is_dolby, buf_info->hdr10_type);

	vdp_printf(VDP_AVSYNC_LOG,
		"ion=%d hnd=0x%p pts=%lld rel=%d cur=%d dr=%d->%d 0x%X (0x%X 0x%X 0x%X 0x%X) release %d\n",
		buf_info->ion_fd, buf_info->ion_hnd, config->pts,
		buf_info->release_fence_fd,
		buf_info->current_fence_index,
		buf_info->hdr_info.dr_range,
		config->dr_range,
		buf_info->src_phy_addr,
		buf_info->ofst_y,
		buf_info->ofst_c,
		buf_info->ofst_y_len,
		buf_info->ofst_c_len,
		video_layer[config->layer_id].release_timeline_idx);



	/* fill Dolby HDR info */
	buf_info->hdr_info.dr_range = config->dr_range;
	if (config->dr_range == DISP_DR_TYPE_DOVI) {
		struct mtk_disp_dovi_md_t *dolby_info = &config->dolby_info;
		struct mtk_vdp_dovi_md_t *dovi_md_info = &buf_info->hdr_info.metadata_info.dovi_metadata;

		buf_info->hdr_info.dr_range = DISP_DR_TYPE_DOVI;

		dovi_md_info->pts = dolby_info->pts;
		dovi_md_info->len = dolby_info->len;
		dovi_md_info->svp = dolby_info->svp;
		memset(&dovi_md_info->buff, 0, DOVI_MD_MAX_LEN);

		if (dovi_md_info->svp) {
			dovi_md_info->sec_handle = dolby_info->sec_handle;

			vdp_printf(VDP_DOVI_LOG, "dovi frm pts %lld rpu pts %lld len %d sec_handle 0x%x\n",
			config->pts, dolby_info->pts, dolby_info->len, dolby_info->sec_handle);
		} else {
			vdp_printf(VDP_DOVI_LOG, "dovi frm pts %lld rpu pts %lld len %d addr %p\n",
			config->pts, dolby_info->pts, dolby_info->len, dolby_info->addr);
			if (copy_from_user(dovi_md_info->buff, (void __user *)(dolby_info->addr), dolby_info->len)) {
				DISP_LOG_E("dovi info copy from user fail\n");
				goto release_ion_handle;
			}

			if (dolby_info->len != 0) {
				uint32_t *addr = (uint32_t *) dovi_md_info->buff;

				vdp_printf(VDP_DOVI_LOG, "dovi rpu value 0x%X 0x%X 0x%X 0x%X\n",
					   addr[0], addr[1], addr[2], addr[3]);
			}
		}
	}


	/* for Dolby debug.
	** force set current buffer as dolby video
	*/
	if (force_dr_range) {
		buf_info->hdr_info.dr_range = force_dr_range - 1;

		if (buf_info->hdr_info.dr_range == DISP_DR_TYPE_DOVI) {
			buf_info->is_dolby = true;
			buf_info->hdr10_type = HDR10_TYPE_NONE;
		} else if (buf_info->hdr_info.dr_range == DISP_DR_TYPE_HDR10) {
			buf_info->is_dolby = false;
			buf_info->hdr10_type = HDR10_TYPE_ST2084;
		} else {
			buf_info->is_dolby = false;
			buf_info->hdr10_type = HDR10_TYPE_NONE;
		}
		vdp_printf(VDP_AVSYNC_LOG,
		"force dr range %d dolby %d hdr type %d\n",
		buf_info->hdr_info.dr_range,
		buf_info->is_dolby,
		buf_info->hdr10_type);
	}

	/*create release fence fd */
	mutex_lock(&(video_layer[config->layer_id].sync_lock));
	fence.fence = VDP_INVALID_FENCE_FD;
	fence.value = ++(video_layer[config->layer_id].fence_idx);
	mutex_unlock(&(video_layer[config->layer_id].sync_lock));
	sprintf(fence.name, "vdp_fence-%d-%d", config->layer_id, fence.value);
	ret = fence_create((struct sw_sync_timeline *)video_layer[config->layer_id].timeline, &fence);
	if (ret != 0) {
		DISP_LOG_E("fd leak, create vdp fence fail value:%d status:%d\n",
			fence.value, ret);
		goto release_ion_handle;
	}
	/*fill the release fence fd for hwc */
	config->release_fence_fd = fence.fence;
	buf_info->time_fence_create = sched_clock();
	buf_info->release_fence_fd = fence.fence;
	buf_info->current_fence_index = fence.value;

	config_frame_count[config->layer_id]++;
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (vdp_cli_get()->enable_pts_debug)
		DISP_LOG_E("config fps %d, pts %lld\n", config->fps, config->pts);
#endif

	/*insert the video buffer to the buffer list */
	buf_info->buf_state = BUFFER_INSERT;
	mutex_lock(&(video_layer[config->layer_id].sync_lock));
	list_add_tail(&buf_info->list, &(video_layer[config->layer_id].buf_list));
	video_layer[config->layer_id].last_pts =  config->pts;
	/* store source and target info to see if reconfigure is needed */
	memcpy((void *)(&(video_layer[config->layer_id].src_rgn)),
			(const void *)(&(config->src)),
			sizeof(struct mtk_disp_range));
	memcpy((void *)(&(video_layer[config->layer_id].tgt_rgn)),
			(const void *)(&(config->tgt)),
			sizeof(struct mtk_disp_range));
	mutex_unlock(&(video_layer[config->layer_id].sync_lock));

	return VDP_OK;

release_ion_handle:
	/* clean up allocated buffer */
	DISP_LOG_E("vdp layer config fail with error parameter, drop it\n");
	vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd);
	if (is_Y_C_independent)
		vdp_ion_free_handle(vdp_ion_client, buf_info->ion_hnd2);

	/* buf_info is not add to any list yet, no need to lock. */
	list_del_init(&buf_info->list);
	release_buf_info(&buf_info->list, config->layer_id);

	return VDP_FAIL;
}

#if USE_FAKE_ACTIVE_START
#include <linux/hrtimer.h>
#include <linux/ktime.h>

ktime_t ktime;
struct hrtimer hr_timer;

enum hrtimer_restart vdp_timer_callback(struct hrtimer *hr_timer)
{
	vdp_wakeup_routine();
	return HRTIMER_NORESTART;
}


int vdp_set_timer(void)
{
	int interval = 2000; /* us */
	int status = 0;

	do {
		hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ktime = ktime_set(interval / 1000000, (interval % 1000000) * 1000);
		hr_timer.function = &vdp_timer_callback;
		hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
	} while (0);

	return status;
}
#endif

int disp_vdp_irq_handler(uint32_t irq)
{
	unsigned int i = 0;

	if (!vdp_init_done)
		return VDP_OK;

	switch (irq) {
	case DISP_IRQ_FMT_ACTIVE_START:
		vdp_wakeup_routine(); /* process new buffer */
		break;
	case DISP_IRQ_FMT_VSYNC:
		vdp_isr();
		vdp_update_registers(); /* update shadow register to hw */

		/* non shadow register update */
		do {
			/* swap main & sub video */
			disp_sys_hal_set_main_sub_swap(disp_vdp_get_main_sub_swap_status());
			disp_vdp_enable_premix_clock(disp_vdp_get_osd_premix());
		} while(0);


		#if (VIDEO_USE_HW_SHADOW && VIDEO_REDUCE_BUFFER)
		/* release buffer in vsync irq. */
		for (i = 0; i < VIDEO_LAYER_MAX_COUNT; i++) {
			video_layer[i].release_timeline_idx = video_layer[i].timeline_idx;
			queue_work(release_buffer_wq[i], &video_layer[i].task_work);
		}
		#endif

		#if USE_FAKE_ACTIVE_START
		vdp_set_timer();
		#else
		vdp_wakeup_routine();
		#endif
		break;
	default:
		break;
	}
	return VDP_OK;
}

int disp_vdp_dump(uint32_t level)
{
	return VDP_OK;
}

int disp_vdp_dbg_level_enable(uint32_t level, uint32_t enable)
{
	if (enable)
		vdp_dbg_level |= (1 << level);
	else
		vdp_dbg_level &= !(1 << level);
	DISP_LOG_I("set dbg level %d enable %d 0x%X\n", level, enable,
	vdp_dbg_level);
	return VDP_OK;
}


/* enable_main_sub_swap & dolby_path_enable &
** disp_vdp_get_main_sub_swap_status &
** disp_vdp_set_main_sub_swap
** these var & functions are for Dolby PIP
** 1 when start sub video, check if main path is playing dolby video(using dolby_path_enable).
** 2 if yes, disp_vdp_set_main_sub_swap(1) & update dovi setting in roution. otherwise, do nothing
** 3 when stop sub video, check disp_vdp_get_main_sub_swap_status()
** 4 if yes, set disp_vdp_set_main_sub_swap(0), otherwise, do nothing
**
*/
/* record whether main path & sub path are swaped. */
static bool enable_main_sub_swap;

bool disp_vdp_get_main_sub_swap_status(void)
{
	return enable_main_sub_swap;
}

void disp_vdp_set_main_sub_swap(uint32_t swap)
{
	if (swap == enable_main_sub_swap)
		return;
	DISP_LOG_N("swap main & sub video path:%d\n", swap);
#if 0
	/* disable vdo4 output to pre-mix */
	disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &disable);

	/* disable YUV444 */
	fmt_hal_set_output_444(1, disable);

	#if 0
	/* swap main/sub video path 0x15000014[19] */
	disp_sys_hal_set_main_sub_swap(swap);
	#endif
#endif
	enable_main_sub_swap = swap;
}

/*
** osd premix is not shadow register. we need to update in irq
*/
static bool enable_osd_premix;

void disp_vdp_set_osd_premix(bool enable)
{
	enable_osd_premix = enable;
}

bool disp_vdp_get_osd_premix(void)
{
	return enable_osd_premix;
}


/***************** driver ************/
struct disp_hw disp_vdp_driver = {
	.name = VDP_DRV_NAME,
	.init = disp_vdp_init,
	.deinit = disp_vdp_deinit,
	.start = disp_vdp_start,
	.stop = disp_vdp_stop,
	.suspend = disp_vdp_suspend,
	.resume = disp_vdp_resume,
	.get_info = disp_vdp_get_info,
	.change_resolution = disp_vdp_change_resolution,
	.config = disp_vdp_config,
	.irq_handler = disp_vdp_irq_handler,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = disp_vdp_dump,
};

struct disp_hw *disp_vdp_get_drv(void)
{
	return &disp_vdp_driver;
}

