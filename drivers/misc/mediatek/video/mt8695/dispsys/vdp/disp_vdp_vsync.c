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



#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#define LOG_TAG "VDP_VSYNC"

#include "disp_vdp_vsync.h"
#include "disp_vdp_if.h"
#include "vdp_hw.h"
#include "vdp_hal.h"
/*#include "mt85xx.h"*/
#include "mtk_disp_mgr.h"
#include "disp_hw_mgr.h"
#include "ion_drv.h"
#include "disp_hw_log.h"
#include "mtk_sync.h"
#include "fmt_hal.h"
#include "disp_clk.h"
#include "vdout_sys_hal.h"
#include "disp_path.h"
#include "disp_vdp_sec.h"
#include "disp_dovi_common.h"
#include "disp_vdp_cli.h"
#include "disp_dovi_main.h"
#include "disp_dovi_common_if.h"
#include "disp_dovi_io.h"
#include "videoin_hal.h"
#include "dovi_core3_hal.h"
#include "dovi_core2_hal.h"
#include "dovi_core1_hal.h"
#include "disp_sys_hal.h"
#include "disp_hdr_core.h"



UINT32 _u4VdpISRCount;  /* VDP ISR count */
UINT32 u4VsyncLineTest;
enum HDR10_TYPE_ENUM hdr_enable;	/* record buffer HDR enable status */


/*for vdp main thread wake up */
static struct task_struct *disp_vdp_thread0, *disp_vdp_thread1;
static wait_queue_head_t disp_vdp_wq0, disp_vdp_wq1;
static unsigned int gWakeupVdpSwThread0, gWakeupVdpSwThread1;

static LIST_HEAD(video_info_pool_main);
static LIST_HEAD(video_info_pool_sub);

static DEFINE_MUTEX(video_info_pool_main_mutex);
static DEFINE_MUTEX(video_info_pool_sub_mutex);

static DEFINE_MUTEX(fence_buffer_mutex);

static int vdp_routine(void *data);

bool print_video_fence_history;

struct mtk_disp_hdr_md_info_t vdp_hdr_info[VDP_MAX];
/* record whether current main path is playing dolby video. */
uint32_t dolby_path_enable;
bool netflix_dolby_path_enable;
bool netflix_dolby_osd_no_ready;
uint32_t dolby_path_enable_cnt;
uint32_t dolby_path_start;
bool hdr10_plus_enable;
uint32_t dolby_path_enable_vdp_id;
enum dovi_signal_format_t dolby_out_format;
bool dolby_force_output;
enum dovi_signal_format_t dolby_force_out_format;
enum dovi_signal_format_t dolby_out_format_res_change;
bool dolby_vdp_pause;
struct dovi_info_t vdp_dovi_info;
bool dolby_path_disable;
uint32_t dolby_path_disable_cnt;
HDMI_VIDEO_RESOLUTION old_resolution;
bool debug_dovi_path_full_vs10;
bool dovi_vs10_disable_delay;
uint32_t dovi_vs10_path_enable_cnt;
static struct mutex disp_dovi_mutex;
/*
	*IDK Cert special flow
	*In IDK Cert, we must set one buf show idk_dump_vsync_cnt vsyncs
	*for dump data from dolby core3 by video in.
	*/
bool dovi_idk_dump;
bool dovi_vs10_force;
uint32_t dovi_idk_disp_cnt;
int32_t idk_dump_vsync_cnt;
bool dovi_idk_dump_set_vin;
bool force_dolby_off;
bool dolby_path_restart = false;

uint32_t dovi_ipcore_version = 4;

#define DOLBY_FMT_DELAY 0x33
#define DISPF_UHD_DELAY 0x42
#define HDR10_PLUS_METADATA_SIZE 512
#define DOVI_RES4K60_TMDS_RATE 590

int debug_frame_count[2];	/* vdp show fps */

uint32_t g_dma_read_empty_threshold;
uint32_t g_dma_write_full_threshold;

struct video_buffer_info *disp_video_init_buf_info(struct video_buffer_info *buf)
{
	memset(buf, 0, sizeof(struct video_buffer_info));
	INIT_LIST_HEAD(&buf->list);

	buf->release_fence_fd = VDP_INVALID_FENCE_FD;
	buf->ion_hnd = NULL;
	buf->layer_id = 0xFF;
	buf->src_phy_addr = 0;
	buf->source_duration = 0;
	return buf;
}

int vdp_get_list_count(struct list_head *head)
{
	int count = 0;
	struct list_head *pos, *n;

	if (head == NULL || list_empty(head))
		return count;

	list_for_each_safe(pos, n, head) {
		count++;
	}

	return count;
}

struct video_buffer_info *vdp_alloc_buf_info(void)
{
	static int allocated_count = 0;
	int value[4] = {0};

	mutex_lock(&video_info_pool_main_mutex);
	value[0] = vdp_get_list_count(&video_info_pool_main);
	mutex_unlock(&video_info_pool_main_mutex);

	mutex_lock(&video_info_pool_sub_mutex);
	value[1] = vdp_get_list_count(&video_info_pool_sub);
	mutex_unlock(&video_info_pool_sub_mutex);

	mutex_lock(&video_layer[0].sync_lock);
	value[2] = vdp_get_list_count(&video_layer[0].buf_list);
	mutex_unlock(&video_layer[0].sync_lock);


	mutex_lock(&video_layer[1].sync_lock);
	value[3] = vdp_get_list_count(&video_layer[1].buf_list);
	mutex_unlock(&video_layer[1].sync_lock);

	DISP_LOG_I("create new buf node, main_pool:%d, sub_pool:%d, main_working_pool:%d, sub_working_pool:%d total:%d\n",
				value[0],
				value[1],
				value[2],
				value[3],
				allocated_count++);

	return kzalloc(sizeof(struct video_buffer_info), GFP_KERNEL);
}

/**
	* Query a @mtkfb_fence_buf_info node from @info_pool_head, if empty create a new one
	*/
struct video_buffer_info *vdp_get_buf_info(enum VIDEO_LAYER_ID id)
{
	struct video_buffer_info *info = NULL;

	if (id == DISP_MAIN_VIDEO) {
		mutex_lock(&video_info_pool_main_mutex);
		if (!list_empty(&video_info_pool_main)) {
			info = list_first_entry(&video_info_pool_main, struct video_buffer_info, list);
			list_del_init(&info->list);
			mutex_unlock(&video_info_pool_main_mutex);
			disp_video_init_buf_info(info);
		} else {
			mutex_unlock(&video_info_pool_main_mutex);
			info = vdp_alloc_buf_info();
			disp_video_init_buf_info(info);
		}
	} else if (id == DISP_SUB_VIDEO) {
		mutex_lock(&video_info_pool_sub_mutex);
		if (!list_empty(&video_info_pool_sub)) {
			info = list_first_entry(&video_info_pool_sub, struct video_buffer_info, list);
			list_del_init(&info->list);
			mutex_unlock(&video_info_pool_sub_mutex);
			disp_video_init_buf_info(info);
		} else {
			mutex_unlock(&video_info_pool_sub_mutex);
			info = vdp_alloc_buf_info();
			disp_video_init_buf_info(info);
		}
	} else
		DISP_LOG_E("vdp_get_buf_info fail\n");
	return info;
}

void release_buf_info(struct list_head *list, unsigned int layer_id)
{
	if (layer_id == MAIN_VIDEO_INDEX) {
		mutex_lock(&video_info_pool_main_mutex);
		list_add_tail(list, &video_info_pool_main);
		mutex_unlock(&video_info_pool_main_mutex);
	} else if (layer_id == SUB_VIDEO_INDEX){
		mutex_lock(&video_info_pool_sub_mutex);
		list_add_tail(list, &video_info_pool_sub);
		mutex_unlock(&video_info_pool_sub_mutex);
	} else
		DISP_LOG_E("release invalid layer_id:%d buffer\n", layer_id);

}

void vdp_ion_free_handle(struct ion_client *client, struct ion_handle *handle)
{
	if (!client) {
		DISP_LOG_E("invalid ion client!\n");
		return;
	}
	if (IS_ERR_OR_NULL(handle))
		return;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (vdp_cli_get()->enable_mva_debug)
		ion_unmap_kernel(client, handle);
#endif

	ion_free(client, handle);
	/*DISP_LOG_I("free ion handle 0x%p\n", handle);*/
}


void vdp_release_buffer(unsigned int layer_id)
{
	struct video_buffer_info *buf = NULL;
	struct video_buffer_info *temp = NULL;
	struct video_layer_info *layer_info = NULL;
	bool is_Y_C_independent = false;

	if (layer_id == MAIN_VIDEO_INDEX)
		layer_info = &video_layer[0];
	else
		layer_info = &video_layer[1];

	list_for_each_entry_safe(buf, temp, &(layer_info->buf_list), list) {
		if (((layer_info->timeline_idx - 1) == buf->current_fence_index)
			|| (layer_info->state == VDP_LAYER_STOPPED)) {
			/*signal the fence */
			if ((buf->ion_fd & 0xFFFF000) > 0)
				is_Y_C_independent = true;
			if (layer_info->timeline != NULL) {
				timeline_inc((struct sw_sync_timeline *)layer_info->timeline, 1);
				buf->time_fence_signal = sched_clock();
				if (print_video_fence_history)
					DISP_LOG_N("video fence%d alive time 0x%llx, wait disp 0x%llx\n",
					buf->current_fence_index,
					(buf->time_fence_signal - buf->time_fence_create),
					(buf->time_start_display - buf->time_fence_create));
			}
			layer_info->release_idx++;
			/*free ion handle */
			vdp_ion_free_handle(vdp_ion_client, buf->ion_hnd);
			if (is_Y_C_independent)
				vdp_ion_free_handle(vdp_ion_client, buf->ion_hnd2);

			list_del_init(&buf->list);
			release_buf_info(&buf->list, layer_id);
			break;
		}
	}
}


void vdp_isr(void)
{
	if (video_layer[0].display_duration >= video_layer[0].vsync_duration)
		video_layer[0].display_duration -= video_layer[0].vsync_duration;
	if (video_layer[1].display_duration >= video_layer[1].vsync_duration)
		video_layer[1].display_duration -= video_layer[1].vsync_duration;
#if !VIDEO_USE_HW_SHADOW
	vdp_hal_isr(!video_layer[0].secure_en, !video_layer[1].secure_en);
#endif
}

void vdp_vsync_init(void)
{
	/*INT32 i;*/
	/*x_os_isr_fct pfnOldIsr;*/
	/*DDPFUNC();*/

	/*RAI_Init();*/
	mutex_init(&disp_dovi_mutex);
	init_waitqueue_head(&disp_vdp_wq0);
	init_waitqueue_head(&disp_vdp_wq1);
	gWakeupVdpSwThread0 = 0;
	gWakeupVdpSwThread1 = 0;

	disp_vdp_thread0 =
		kthread_create(vdp_routine, (void *)(&(video_layer[0].layer_id)), "video_disp0");


	disp_vdp_thread1 =
		kthread_create(vdp_routine, (void *)(&(video_layer[1].layer_id)), "video_disp1");
	wake_up_process(disp_vdp_thread0);
	wake_up_process(disp_vdp_thread1);
	print_video_fence_history = false;
	hdr_enable = HDR10_TYPE_NONE;

	idk_dump_vsync_cnt = -1;

	/*VDP_FrcInit ();*/

	/*VPQ_Init();*/
	/*clear message queue */
	/*VDP_MEMSET ((void *) _arMsgQ, 0, sizeof (VDP_MSG_Q_T) * VDP_MAX_NS);*/
	/*VDP_MEMSET ((void *) VdpCmdQ, 0, sizeof (VDP_CMD_Q_T));*/

	/*for (i = 0; i < VDP_MAX_NS; i++)*/
	/*VdpProcSequence[i] = i;*/

	/*reg ISR (move to HAL ? Since VECTOR is platform dependent!)*/
	/*VERIFY(x_reg_isr(VECTOR_DISP_VSYNC, _VDPVsyncIsr, &pfnOldIsr) == OSR_OK);*/

}



void vdp_update_registers(void)
{
}


void vdp_wakeup_routine(void)
{
	gWakeupVdpSwThread0 = 1;
	wake_up(&disp_vdp_wq0);
	gWakeupVdpSwThread1 = 1;
	wake_up(&disp_vdp_wq1);
}

/*convert the input information to hw structure */
void vdp_set_hal_config(struct video_buffer_info *buf1,
	struct video_buffer_info *buf2,
	struct video_buffer_info *buf3,
	struct vdp_hal_config_info *hal_config)
{
	bool is_Y_C_independent = false;

	if (buf1 == NULL) {
		DISP_LOG_E("current buffer info is NULL\n");
		return;
	}
	if (((buf1->ion_fd & 0xFFFF0000) > 0) && (!buf1->secruity_en))
		is_Y_C_independent = true;

	/* hdr10 plus metadata copy */
	hal_config->cur_fb_info.is_hdr10plus = (buf1->hdr10_type == HDR10_TYPE_PLUS);
	hal_config->cur_fb_info.metadata_sec_handle = buf1->hdr_info.metadata_info.dovi_metadata.sec_handle;
	hal_config->cur_fb_info.metadata_size = buf1->hdr_info.metadata_info.dovi_metadata.len;

	hal_config->is_el_exist = false;
	hal_config->enable = true;
	hal_config->is_second_field = false;
	hal_config->cur_fb_info.is_secruity = buf1->secruity_en;
	if (buf1->secruity_en || video_layer[buf1->layer_id].secure2normal) {
		hal_config->cur_fb_info.fb_addr.sec_handle = buf1->src_phy_addr;
		hal_config->cur_fb_info.fb_addr.addr_y = buf1->ofst_y;
		hal_config->cur_fb_info.fb_addr.addr_c = buf1->ofst_c;
		hal_config->cur_fb_info.fb_addr.buffer_size =
			buf1->buffer_size;
	} else {
		if (is_Y_C_independent) {
			hal_config->cur_fb_info.fb_addr.addr_y =
				buf1->src_phy_addr + buf1->ofst_y;
			hal_config->cur_fb_info.fb_addr.addr_c =
				buf1->src_phy_addr2 + buf1->ofst_c;
		} else {
			hal_config->cur_fb_info.fb_addr.addr_y =
				buf1->src_phy_addr + buf1->ofst_y;
			hal_config->cur_fb_info.fb_addr.addr_c =
				buf1->src_phy_addr + buf1->ofst_c;
		}
	}

	hal_config->cur_fb_info.pts = buf1->pts;

	hal_config->cur_fb_info.is_10bit = buf1->is_10bit;
	hal_config->cur_fb_info.is_10bit_tile_mode = buf1->is_10bit_lbs2bit_tile_mode;
	hal_config->cur_fb_info.is_dolby = buf1->is_dolby;
	hal_config->cur_fb_info.is_interlace = buf1->is_interlace;
	hal_config->cur_fb_info.is_ufo = buf1->is_ufo;
	hal_config->cur_fb_info.is_jumpmode = buf1->is_jumpmode;

	hal_config->cur_fb_info.fb_size.pic_w = buf1->crop.width;
	hal_config->cur_fb_info.fb_size.pic_h = buf1->crop.height;
	hal_config->cur_fb_info.fb_size.buff_w = buf1->src.pitch;
	hal_config->cur_fb_info.fb_size.buff_h = buf1->src.height;
	hal_config->cur_fb_info.src_region.x = buf1->src.x;
	hal_config->cur_fb_info.src_region.y = buf1->src.y;
	hal_config->cur_fb_info.src_region.width = buf1->crop.width;
	hal_config->cur_fb_info.src_region.height = buf1->crop.height;
	hal_config->cur_fb_info.out_region.x = buf1->tgt.x;
	hal_config->cur_fb_info.out_region.y = buf1->tgt.y;
	hal_config->cur_fb_info.out_region.width = buf1->tgt.width;
	hal_config->cur_fb_info.out_region.height = buf1->tgt.height;
	if ((buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV420_BLOCK) ||
		(buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV422_BLOCK))
		hal_config->cur_fb_info.is_scan_line = false;
	else if ((buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV420_RASTER) ||
		(buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV422_RASTER))
		hal_config->cur_fb_info.is_scan_line = true;
	else {
		DISP_LOG_E("VDP -- invalid color format:%d\n", buf1->src_fmt);
		hal_config->cur_fb_info.is_scan_line = true;
	}

	if ((buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV420_BLOCK) ||
		(buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV420_RASTER))
		hal_config->cur_fb_info.is_yuv422 = false;
	else if ((buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV422_BLOCK) ||
		(buf1->src_fmt == DISP_HW_COLOR_FORMAT_YUV422_RASTER))
		hal_config->cur_fb_info.is_yuv422 = true;
	else {
		DISP_LOG_E("VDP -- invalid color format:%d\n", buf1->src_fmt);
		hal_config->cur_fb_info.is_yuv422 = true;
	}


	if (buf1->is_ufo) {
		if (is_Y_C_independent) {
			hal_config->cur_fb_info.fb_addr.addr_y_len =
				buf1->src_phy_addr + buf1->ofst_y_len;
			hal_config->cur_fb_info.fb_addr.addr_c_len =
				buf1->src_phy_addr2 + buf1->ofst_c_len;
		} else {
			hal_config->cur_fb_info.fb_addr.addr_y_len =
				buf1->src_phy_addr + buf1->ofst_y_len;
			hal_config->cur_fb_info.fb_addr.addr_c_len =
				buf1->src_phy_addr + buf1->ofst_c_len;
		}
		if (buf1->secruity_en || video_layer[buf1->layer_id].secure2normal) {
			hal_config->cur_fb_info.fb_addr.addr_y_len =
				buf1->ofst_y_len;
			hal_config->cur_fb_info.fb_addr.addr_c_len =
				buf1->ofst_c_len;
		}
	}

	if (buf1->is_interlace) {
		if ((buf2 == NULL) || (buf3 == NULL)) {
			DISP_LOG_E("reference buffer info is NULL\n");
			return;
		}
	}

	vdp_printf(VDP_AVSYNC_LOG,
	"cfg hal pts %lld fd %d %p 0x%X (0x%X 0x%X 0x%X 0x%X)\n",
	buf1->pts,
	buf1->ion_fd,
	buf1->ion_hnd,
	buf1->src_phy_addr,
	buf1->ofst_y,
	buf1->ofst_c,
	buf1->ofst_y_len,
	buf1->ofst_c_len);

	vdp_printf(VDP_AVSYNC_LOG,
	"addr (0x%X 0x%X 0x%X 0x%X)\n",
	hal_config->cur_fb_info.fb_addr.addr_y,
	hal_config->cur_fb_info.fb_addr.addr_c,
	hal_config->cur_fb_info.fb_addr.addr_y_len,
	hal_config->cur_fb_info.fb_addr.addr_c_len);

}

#if 0
static void vdp_set_disp_fmt_info(struct fmt_active_info *active_info,
	struct vdp_hal_disp_info *vdp_hal_info)
{
	active_info->h_begine = vdp_hal_info->h_start;
	active_info->h_end = vdp_hal_info->h_end;
	active_info->v_odd_begine = vdp_hal_info->v_odd_start;
	active_info->v_odd_end = vdp_hal_info->v_odd_end;
	active_info->v_even_begine = vdp_hal_info->v_even_start;
	active_info->v_even_end = vdp_hal_info->v_even_end;
}
#endif

char *vdp_print_hdr_type(enum HDR10_TYPE_ENUM hdr_type)
{
	switch (hdr_type) {
	case HDR10_TYPE_NONE:
		return "HDR10_TYPE_NONE";
	case HDR10_TYPE_ST2084:
		return "HDR10_TYPE_ST2084";
	case HDR10_TYPE_HLG:
		return "HDR10_TYPE_HLG";
	case HDR10_TYPE_PLUS:
		return "HDR10_TYPE_PLUS";
	default:
		return NULL;
	}
	return NULL;
}

static void vdp_config_hdmi_signal(enum HDR10_TYPE_ENUM hdr_type, VID_STATIC_HDMI_MD_T *metadata, bool enable)
{
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT

	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};
	static bool hdr_enabled; /* avoid enable/disable hdr st2084 twice */
	static bool hlg_enabled; /* avoid enable/disable hlg twice */

	switch (hdr_type) {
	case HDR10_TYPE_ST2084:
		if (enable && disp_common_info.tv.is_support_hdr) {
			/* enable ST2084 */
			if (!hdr_enabled) {
				rHdr.fgIsMetadata = true;
				rHdr.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10;
				memcpy(&rHdr.metadata_info.hdr10_metadata,
					metadata,
					sizeof(VID_STATIC_HDMI_MD_T));
				vSetStaticHdrType(GAMMA_ST2084);
				vVdpSetHdrMetadata(true, rHdr);
				vHdrEnable(true);
				DISP_LOG_N("hdmi signal: %s enable:%d\n", vdp_print_hdr_type(hdr_type), true);
			}
			hdr_enabled = true;
		} else {
			/* disable ST2084 */
			if (hdr_enabled) {
				vHdrEnable(false);
				DISP_LOG_N("hdmi signal: %s enable:%d\n", vdp_print_hdr_type(hdr_type), false);
			}
			hdr_enabled = false;
		}
		break;
	case HDR10_TYPE_HLG:
		if (enable && disp_common_info.tv.is_support_hlg) {
			/* enable hlg */
			if (!hlg_enabled) {
				rHdr.fgIsMetadata = true;
				rHdr.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10;
				memcpy(&rHdr.metadata_info.hdr10_metadata,
					metadata,
					sizeof(VID_STATIC_HDMI_MD_T));
				vSetStaticHdrType(GAMMA_HLG);
				vVdpSetHdrMetadata(true, rHdr);
				vHdrEnable(true);
				DISP_LOG_N("hdmi signal: %s enable:%d\n", vdp_print_hdr_type(hdr_type), true);
			}
			hlg_enabled = true;
		} else {
			/* disable hlg */
			if (hlg_enabled) {
				vHdrEnable(false);
				DISP_LOG_N("hdmi signal: %s enable:%d\n", vdp_print_hdr_type(hdr_type), false);
			}
			hlg_enabled = false;
		}
		break;
	case HDR10_TYPE_PLUS:
		if (disp_common_info.tv.is_support_hdr10_plus) {
			DISP_LOG_N("hdmi signal: %s app_version:0x%x enable:%d\n",
				vdp_print_hdr_type(hdr_type),
				disp_common_info.tv.hdr10_plus_app_ver,
				enable);
			DISP_LOG_E("tv support HDR10 plus, also use HDR st2084 in parallel\n");

			if (enable) {
				rHdr.fgIsMetadata = true;
				rHdr.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10;
				memcpy(&rHdr.metadata_info.hdr10_metadata,
					metadata,
					sizeof(VID_STATIC_HDMI_MD_T));
				vSetStaticHdrType(GAMMA_ST2084);
				vVdpSetHdrMetadata(true, rHdr);
			}

			if (disp_common_info.tv.hdr10_plus_app_ver != 0xFF)
				vHdr10PlusVSIFEnable(enable, ui_force_hdr_type, dolby_out_format);
			else
				vHdr10PlusEnable(enable);
		} else {
			DISP_LOG_E("tv not support HDR10 plus, use HDR st2084 instead\n");
			vdp_config_hdmi_signal(HDR10_TYPE_ST2084, metadata, enable);
		}
		break;
	default:
		/* for HDR10_TYPE_NONE, do nothing no matter enable or disable */
		break;
	}
#endif
}

static void vdp_state_switch(enum VDP_LAYER_STATE src_state,
	enum VDP_LAYER_STATE dst_state,
	struct video_layer_info *layer_info)
{
	if (layer_info->state == src_state)
		layer_info->state = dst_state;
}

static bool vdp_check_2160p_timing(HDMI_VIDEO_RESOLUTION res_mode)
{
	switch (res_mode) {
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
		return true;
	default:
		return false;
	}

	return false;
}

static bool vdp_check_2160p60_timing(HDMI_VIDEO_RESOLUTION res_mode)
{
	if ((res_mode == HDMI_VIDEO_3840x2160P_60HZ) ||
		(res_mode == HDMI_VIDEO_3840x2160P_50HZ) ||
		(res_mode == HDMI_VIDEO_4096x2160P_60HZ) ||
		(res_mode == HDMI_VIDEO_4096x2160P_50HZ))
		return true;
	else
		return false;
}

#if 0
static int vdp_check_info_update(unsigned char vdp_id, struct vdp_hal_config_info *hal_config,
	bool check_only)
{
	struct video_layer_info *layer_info;
	int ret = 0;

	layer_info = &video_layer[vdp_id];
	if (layer_info->enable != hal_config->enable) {
		ret |= VDP_INFO_EN_UPDATE;
		if (!check_only)
			layer_info->enable = hal_config->enable;
	}
	if ((layer_info->src_rgn.x != hal_config->cur_fb_info.src_region.x) ||
		(layer_info->src_rgn.y != hal_config->cur_fb_info.src_region.y) ||
		(layer_info->src_rgn.width != hal_config->cur_fb_info.src_region.width) ||
		(layer_info->src_rgn.height != hal_config->cur_fb_info.src_region.height)) {
		ret |= VDP_INFO_IN_REGION_UPDATE;
		if (!check_only) {
			layer_info->src_rgn.x = hal_config->cur_fb_info.src_region.x;
			layer_info->src_rgn.y = hal_config->cur_fb_info.src_region.y;
			layer_info->src_rgn.width = hal_config->cur_fb_info.src_region.width;
			layer_info->src_rgn.height = hal_config->cur_fb_info.src_region.height;
		}
	}
	if ((layer_info->tgt_rgn.x != hal_config->cur_fb_info.out_region.x) ||
		(layer_info->tgt_rgn.y != hal_config->cur_fb_info.out_region.y) ||
		(layer_info->tgt_rgn.width != hal_config->cur_fb_info.out_region.width) ||
		(layer_info->tgt_rgn.height != hal_config->cur_fb_info.out_region.height)) {
		ret |= VDP_INFO_OUT_REGION_UPDATE;
		if (!check_only) {
			layer_info->tgt_rgn.x = hal_config->cur_fb_info.out_region.x;
			layer_info->tgt_rgn.y = hal_config->cur_fb_info.out_region.y;
			layer_info->tgt_rgn.width = hal_config->cur_fb_info.out_region.width;
			layer_info->tgt_rgn.height = hal_config->cur_fb_info.out_region.height;
		}
	}
	return ret;
}
#endif

int vdp_check_dsd_available(unsigned char vdp_id, struct vdp_hal_config_info *hal_config)
{
	if ((hal_config->cur_fb_info.src_region.height > disp_common_info.resolution->height) ||
		(hal_config->cur_fb_info.src_region.width > disp_common_info.resolution->width)) {
		if ((disp_common_info.resolution->width > hal_config->cur_fb_info.out_region.width * 8) ||
			(disp_common_info.resolution->height > hal_config->cur_fb_info.out_region.height * 2))
			return 0;
		else {
			if ((disp_common_info.resolution->width == 1280) &&
				(disp_common_info.resolution->height == 720) &&
				(hal_config->cur_fb_info.src_region.height <= 1080) &&
				(hal_config->cur_fb_info.src_region.width <= 1920))
				return 0;
		else
			return 1;
		}
	} else
		return 0;
}

static enum DSD_CASE_E vdp_set_dsd_case(uint16_t src_width, uint16_t src_height)
{
	enum DSD_CASE_E dsd_case = DSD_NONE;

	switch (disp_common_info.resolution->res_mode) {
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
		if ((src_width > 1920) || (src_height > 1080))
			dsd_case = DSD_4K_TO_1080P;
		break;
	case HDMI_VIDEO_720x480p_60Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		if ((src_width > 1920) || (src_height > 1080))
			dsd_case = DSD_4K_TO_480P;
		else if ((src_width > 1280) || (src_height > 720))
			dsd_case = DSD_1080P_TO_480P;
		else if ((src_width > 720) || (src_height > 480))
			dsd_case = DSD_720P_TO_480P;
		break;
	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
		if ((src_width > 1920) || (src_height > 1080))
			dsd_case = DSD_4K_TO_720P;
		else
			dsd_case = DSD_1080P_TO_720P;
		break;
	default:
		dsd_case = DSD_NONE;
		break;
	}
	return dsd_case;
}

static void vdp_select_dsd_pll(unsigned char vdp_id,
	enum DSD_CASE_E dsd_type)
{
	switch (dsd_type) {
	case DSD_4K_TO_480P:
		/*648M */
		disp_clock_select_pll(DISP_CLK_VDO3_SEL, DISP_CLK_OSDPLL);
		break;
	case DSD_4K_TO_720P:
		/*594 */
		disp_clock_select_pll(DISP_CLK_VDO3_SEL, DISP_CLK_TVDPLL);
		break;
	case DSD_4K_TO_1080P:
		/*594 */
		disp_clock_select_pll(DISP_CLK_VDO3_SEL, DISP_CLK_TVDPLL);
		break;
	case DSD_1080P_TO_480P:
	case DSD_720P_TO_480P:
		if (vdp_id == 0)
			disp_clock_select_pll(DISP_CLK_VDO3_SEL, DISP_CLK_OSDPLL_D4);
		else
			disp_clock_select_pll(DISP_CLK_VDO4_SEL, DISP_CLK_OSDPLL_D4);
		break;
	case DSD_1080P_TO_720P:
		break;
	default:
		break;
	}
}

static void vdp_update_fmt_setting(unsigned char vdp_id,
	struct vdp_hal_config_info *config_info,
	enum HDR10_TYPE_ENUM buffer_type)
{
	int32_t status = FMT_OK;
	uint8_t dsd_en = 0;
	struct fmt_dsd_scl_info fmt_dsd_scl_info = {0};
	uint32_t h_factor = 0;
	uint32_t src_width = 0;
	uint32_t src_height = 0;
	uint32_t dst_width = 0;
	uint32_t dst_height = 0;

	/* store HDMI h start / v start */
	int h_start = 0;
	int v_start_odd = 0;
	int v_start_even = 0;

	struct fmt_active_info dispfmt_active_info = {0};	/*0A0/0A4/0A8 */
	struct fmt_active_info vdoutfmt_active_info = {0};	/*3A0/3A4/3A8 */
	struct fmt_hd_scl_info info = {0};	/*070/074/078/07C */

	dsd_en = vdp_check_dsd_available(vdp_id, config_info);

	video_layer[vdp_id].enable = config_info->enable;

	/*calculate the h factor for dispfmt */
	src_width = config_info->cur_fb_info.src_region.width;
	src_height = config_info->cur_fb_info.src_region.height;
	dst_width = config_info->cur_fb_info.out_region.width;
	dst_height = config_info->cur_fb_info.out_region.height;
	h_factor = src_width * DISPFMT_H_FACTOR / dst_width;

	/*get the active zone information */
	disp_path_get_active_zone(vdp_id, disp_common_info.resolution->res_mode,
				&h_start, &v_start_odd, &v_start_even);

	/* fill DISPFMT active zone */
	if (src_width > dst_width) {	/* use hdown scale or dsd */
		/* in scale down case, the dispfmt zoom is for input buffer
		** v_begin & end is special, need add display y offset.
		*/
		dispfmt_active_info.h_begine = h_start;
		dispfmt_active_info.h_end = dispfmt_active_info.h_begine + src_width - 1;
		dispfmt_active_info.v_even_begine = v_start_even + config_info->cur_fb_info.out_region.y;
		dispfmt_active_info.v_even_end = dispfmt_active_info.v_even_begine + dst_height - 1;
		dispfmt_active_info.v_odd_begine = dispfmt_active_info.v_even_begine;
		dispfmt_active_info.v_odd_end = dispfmt_active_info.v_even_end;
	} else {	/* no hdown scale or dsd */
		/* in scale up case, the dispfmt zoom is for display area */
		dispfmt_active_info.h_begine = h_start + config_info->cur_fb_info.out_region.x;
		dispfmt_active_info.h_end = dispfmt_active_info.h_begine + dst_width - 1;
		dispfmt_active_info.v_even_begine = v_start_even + config_info->cur_fb_info.out_region.y;
		dispfmt_active_info.v_even_end = dispfmt_active_info.v_even_begine + dst_height - 1;
		dispfmt_active_info.v_odd_begine = dispfmt_active_info.v_even_begine;
		dispfmt_active_info.v_odd_end = dispfmt_active_info.v_even_end;
	}

	/* fild VDOUT FMT active zone: display area */
	vdoutfmt_active_info.h_begine = h_start + config_info->cur_fb_info.out_region.x;
	vdoutfmt_active_info.h_end = vdoutfmt_active_info.h_begine + dst_width - 1;
	vdoutfmt_active_info.v_even_begine = v_start_even + config_info->cur_fb_info.out_region.y;
	vdoutfmt_active_info.v_even_end = vdoutfmt_active_info.v_even_begine + dst_height - 1;
	vdoutfmt_active_info.v_odd_begine = vdoutfmt_active_info.v_even_begine;
	vdoutfmt_active_info.v_odd_end = vdoutfmt_active_info.v_even_end;

	/*get the down scaler information */
	if (src_width > dst_width) {
		info.hd_scl_on = true;
		info.in_x_pos = h_start;
		info.out_x_offset = config_info->cur_fb_info.out_region.x;
		info.out_y_odd_pos = v_start_odd + config_info->cur_fb_info.out_region.y;
		info.out_y_odd_pos_e = info.out_y_odd_pos + dst_height;
		info.out_y_even_pos = info.out_y_odd_pos;
		info.out_y_even_pos_e = info.out_y_odd_pos_e;
		info.res = disp_common_info.resolution->res_mode;
		info.src_w = src_width;
		info.out_w = dst_width;
	} else
		info.hd_scl_on = false;

	if (info.hd_scl_on)
		fmt_hal_set_pixel_factor(vdp_id, src_width, 0x1000, 1, false);
	else {
		uint32_t factor_linear_coef = 0;

		/* conditions for using linear coef when dispfmt do scale.
		** 1. current signal is HDR
		** 2. sub path is sdr.
		** then we must go though sdr2hdr, in this case, need to change scale from factor to linear.
		*/
		if ((hdr_enable != HDR10_TYPE_NONE) && (vdp_id == 1) && (buffer_type == HDR10_TYPE_NONE))
			factor_linear_coef = 1;

		fmt_hal_set_pixel_factor(vdp_id, src_width, h_factor, 1, factor_linear_coef);
		fmt_hal_h_down_disable(vdp_id);
	}

	fmt_hal_set_active_zone(vdp_id, &dispfmt_active_info);


	/* dispfmt 70/74/78 register setting
	** and adjust vdout active zone
	*/
	if ((info.hd_scl_on) && (dsd_en == 0)) {
		status = fmt_hal_h_down_enable(vdp_id, &info);
		if (status == FMT_OK) {
			vdoutfmt_active_info.h_begine = info.vdout_out_h_start;
			vdoutfmt_active_info.h_end = info.vdout_out_h_end;
		} else {
			fmt_hal_set_pixel_factor(vdp_id, src_width, h_factor, 1, false);
			DISP_LOG_E("get fmt h_down info fail\n");
		}
	}

	/* odd data,  & is dolby cb cr swap */
	if (config_info->cur_fb_info.is_dolby || dolby_path_enable) {
		#if 0
		/* For Plan C2 */
		fmt_hal_set_output_444((vdp_id == DISP_FMT_MAIN) ? DISP_FMT_MAIN : DISP_FMT_SUB, false);
		if ((info.hd_scl_on) && (dsd_en == 0)) {
			if (status == FMT_OK) {
				if ((info.dispfmt_out_h_start % 2) == 0)
					fmt_hal_set_uv_swap((vdp_id == DISP_FMT_MAIN) ? DISP_FMT_MAIN : DISP_FMT_SUB, true);
				else
					fmt_hal_set_uv_swap((vdp_id == DISP_FMT_MAIN) ? DISP_FMT_MAIN : DISP_FMT_SUB, false);
			}
		} else {
			if ((dispfmt_active_info.h_begine % 2) == 1)
				fmt_hal_set_uv_swap((vdp_id == DISP_FMT_MAIN) ? DISP_FMT_MAIN : DISP_FMT_SUB, true);
			else
				fmt_hal_set_uv_swap((vdp_id == DISP_FMT_MAIN) ? DISP_FMT_MAIN : DISP_FMT_SUB, false);
		}
		#else

		if (vdp_id == DISP_FMT_MAIN)
			fmt_hal_set_output_444(DISP_FMT_MAIN, false);

		if ((info.hd_scl_on) && (dsd_en == 0)) {
			if (status == FMT_OK) {
				if ((info.dispfmt_out_h_start % 2) == 0)
					fmt_hal_set_uv_swap(DISP_FMT_MAIN, true);
				else
					fmt_hal_set_uv_swap(DISP_FMT_MAIN, false);
			}
		} else {
			if ((dispfmt_active_info.h_begine % 2) == 1)
				fmt_hal_set_uv_swap(DISP_FMT_MAIN, true);
			else
				fmt_hal_set_uv_swap(DISP_FMT_MAIN, false);
		}

		#endif
	}


	/* adjust vdout active zone sepecial for 2160P */
	if (vdp_check_2160p_timing(disp_common_info.resolution->res_mode)) {
		/* 2160P normal & scale up case */
		vdoutfmt_active_info.h_begine += 0x62;
		vdoutfmt_active_info.h_end += 0x62;

		/* 2160P scale down case */
		if (((info.hd_scl_on) && (dsd_en == 0))) {
			vdoutfmt_active_info.h_begine += 0x3;
			vdoutfmt_active_info.h_end += 0x3;
		}
	}

	/* set vdout active zone */
	fmt_hal_set_active_zone((vdp_id == DISP_FMT_MAIN) ? VDOUT_FMT : VDOUT_FMT_SUB,
		&vdoutfmt_active_info);

	if (dsd_en == 1) {
		fmt_dsd_scl_info.dsd_case =
			vdp_set_dsd_case(config_info->cur_fb_info.src_region.width,
				config_info->cur_fb_info.src_region.height);
		fmt_dsd_scl_info.fg_dsd_scl_on = true;
		fmt_dsd_scl_info.src_x = config_info->cur_fb_info.src_region.x;
		fmt_dsd_scl_info.src_y = config_info->cur_fb_info.src_region.y;
		fmt_dsd_scl_info.src_w = config_info->cur_fb_info.src_region.width;
		fmt_dsd_scl_info.src_h = config_info->cur_fb_info.src_region.height;
		fmt_dsd_scl_info.out_x = config_info->cur_fb_info.out_region.x;
		fmt_dsd_scl_info.out_y = config_info->cur_fb_info.out_region.y;
		fmt_dsd_scl_info.out_w = config_info->cur_fb_info.out_region.width;
		fmt_dsd_scl_info.out_h = config_info->cur_fb_info.out_region.height;
		fmt_dsd_scl_info.original_active_start = h_start;
		fmt_dsd_scl_info.active_start = vdoutfmt_active_info.h_begine;
		fmt_dsd_scl_info.active_end = vdoutfmt_active_info.h_end;
		fmt_hal_set_active_zone(vdp_id, &vdoutfmt_active_info);
		fmt_hal_dsd_enable(vdp_id, &fmt_dsd_scl_info);
		if ((vdp_id == 0) && (dolby_path_enable == 0))
			disp_path_set_dsd_delay(DISP_PATH_MVDO_OUT,
				disp_common_info.resolution->res_mode);
		else if (vdp_id == 1)
			disp_path_set_dsd_delay(DISP_PATH_SVDO_OUT,
				disp_common_info.resolution->res_mode);
		fmt_hal_dsd_set_mode(vdp_id, fmt_dsd_scl_info.dsd_case,
			disp_common_info.resolution->htotal,
			disp_common_info.resolution->vtotal);
		vdp_select_dsd_pll(vdp_id, fmt_dsd_scl_info.dsd_case);
		vdout_sys_hal_select_dsd_clk(true);
	} else
		vdout_sys_hal_select_dsd_clk(false);

	if (vdp_id == MAIN_VIDEO_INDEX)
		fmt_hal_enable(DISP_FMT_MAIN, true);
	else if (vdp_id == SUB_VIDEO_INDEX) {
		fmt_hal_enable(DISP_FMT_SUB, true);
		fmt_hal_mix_plane(FMT_HW_PLANE_2);
	}
}

void vdp_update_dovi_setting_res_change(struct disp_hw_tv_capbility *tv_cap,
	const struct disp_hw_resolution *resolution)
{
	bool is_2160p60_out = vdp_check_2160p60_timing(resolution->res_mode);
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};

	vdp_dovi_info.vsvdb_edid = tv_cap->vsvdb_edid;

	DISP_LOG_N("vdp_update_dovi_setting_res_change\n");
	DISP_LOG_N("support hdr %d, dolby %d dolby_2160p60 %d, dolby_low_latency %d, is_2160p60_out %d\n",
		tv_cap->is_support_hdr,
		tv_cap->is_support_dolby,
		tv_cap->is_support_dolby_2160p60,
		tv_cap->is_support_dolby_low_latency,
		is_2160p60_out);

	/* dolby hw not support 4096 dovi output */

	if ((tv_cap->is_support_dolby_2160p60 ||
		(tv_cap->is_support_dolby && !is_2160p60_out))
		&& (resolution->width <= 3840)) {
		if (tv_cap->is_support_dolby_low_latency) {
			/* if tv support ll and ui set ll, wo set output LL
			* if tv only support ll, we set ooutput LL
			*/
			if (tv_cap->dolbyvision_vsvdb_version == 0x2) {
				if ((tv_cap->dolbyvision_vsvdb_v2_interface == 0x0)
					|| (tv_cap->dolbyvision_vsvdb_v2_interface == 0x1))
					vdp_dovi_info.is_low_latency = true;
			} else
				vdp_dovi_info.is_low_latency = false;
		} else
			vdp_dovi_info.is_low_latency = false;

		if (vdp_dovi_info.is_low_latency)
			dolby_out_format_res_change = DOVI_FORMAT_DOVI_LOW_LATENCY;
		else
			dolby_out_format_res_change = DOVI_FORMAT_DOVI;
	} else if (tv_cap->is_support_dolby && is_2160p60_out
			&& (tv_cap->dolbyvision_vsvdb_version == 0x2)
			&& ((tv_cap->dolbyvision_vsvdb_v2_interface == 0x2)
			|| (tv_cap->dolbyvision_vsvdb_v2_interface == 0x3))) {
		vdp_dovi_info.is_low_latency = false;
		dolby_out_format_res_change = DOVI_FORMAT_DOVI;
	} else if (tv_cap->is_support_dolby_low_latency && is_2160p60_out) {
		vdp_dovi_info.is_low_latency = true;
		dolby_out_format_res_change = DOVI_FORMAT_DOVI_LOW_LATENCY;
	} else if (tv_cap->is_support_hdr)
		dolby_out_format_res_change = DOVI_FORMAT_HDR10;
	else
		dolby_out_format_res_change = DOVI_FORMAT_SDR;

	/* make sure chosen format can be truly supported - if clock/tmds rate is
	   not enough, move to a lower format */
	if ((tv_cap->max_tmds_rate < DOVI_RES4K60_TMDS_RATE) && is_2160p60_out){
		if ((dolby_out_format_res_change == DOVI_FORMAT_DOVI) ||
		    (dolby_out_format_res_change == DOVI_FORMAT_DOVI_LOW_LATENCY))
			DISP_LOG_N("dolby vision 2160p60 can not be supported: max_tmds_rate=%d\n",
				tv_cap->max_tmds_rate);
		if (tv_cap->is_support_hdr)
			dolby_out_format_res_change = DOVI_FORMAT_HDR10;
		else
			dolby_out_format_res_change = DOVI_FORMAT_SDR;
	}

	vdp_dovi_info.out_format = dolby_out_format_res_change;
	if (osd_enable)
		vdp_dovi_info.is_graphic_mode = true;
	else
		vdp_dovi_info.is_graphic_mode = false;

	if (dolby_out_format_res_change != dolby_out_format) {
		DISP_LOG_N("out_format change from %d to %d, %d\n",
			dolby_out_format, dolby_out_format_res_change, dolby_vdp_pause);

		vdp_drv->drv_call(DISP_CMD_DOVI_OUT_FORMAT_UPDATE, &vdp_dovi_info);
		if (dolby_out_format == DOVI_FORMAT_DOVI)
			vDolbyHdrEnable(false);
		else if (dolby_out_format == DOVI_FORMAT_HDR10) {
			vHdrEnable(false);
			vdp_set_HDMI_BT2020_signal(false);
		} else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
			vLowLatencyDolbyVisionEnable(false);

		#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if (dolby_out_format_res_change == DOVI_FORMAT_DOVI)
			vDolbyHdrEnable(true);
		else if (dolby_out_format_res_change == DOVI_FORMAT_HDR10) {
			vSetStaticHdrType(GAMMA_ST2084);
			vHdrEnable(true);
			if (tv_cap->is_support_bt2020)
				vdp_set_HDMI_BT2020_signal(true);
		} else if (dolby_out_format_res_change == DOVI_FORMAT_DOVI_LOW_LATENCY)
			vLowLatencyDolbyVisionEnable(true);
		#endif
	} else if (dolby_out_format_res_change == dolby_out_format) {
		DISP_LOG_I("out_format is same as before %d, %d\n",
			dolby_out_format, dolby_vdp_pause);

		if ((dolby_out_format == DOVI_FORMAT_HDR10)
			|| (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)) {
			vdp_drv->drv_call(DISP_CMD_DOVI_OUT_FORMAT_UPDATE, &vdp_dovi_info);
			/* if it is hdr10/LOW LATENCE, we must enable again. it will disbale when hot plug out */
			#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
			if (dolby_out_format == DOVI_FORMAT_HDR10) {
				vSetStaticHdrType(GAMMA_ST2084);
				vHdrEnable(true);
				if (tv_cap->is_support_bt2020)
					vdp_set_HDMI_BT2020_signal(true);
			} else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
				vLowLatencyDolbyVisionEnable(true);
			#endif
		}
	}
	if ((old_resolution != resolution->res_mode)
		|| (dolby_out_format_res_change != dolby_out_format)) {
		if ((resolution->res_mode == HDMI_VIDEO_3840x2160P_23_976HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_24HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_25HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_29_97HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_30HZ)
			|| (resolution->res_mode == HDMI_VIDEO_4096x2160P_24HZ)) {
			g_dma_read_empty_threshold = 0x38;
			g_dma_write_full_threshold = 0x38;
		} else if((resolution->res_mode == HDMI_VIDEO_3840x2160P_60HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_50HZ)
			|| (resolution->res_mode == HDMI_VIDEO_4096x2160P_60HZ)
			|| (resolution->res_mode == HDMI_VIDEO_4096x2160P_50HZ)
			|| (resolution->res_mode == HDMI_VIDEO_3840x2160P_59_94HZ)
			|| (resolution->res_mode == HDMI_VIDEO_4096x2160P_59_94HZ)) {
			g_dma_read_empty_threshold = 0x5F;
			g_dma_write_full_threshold = 0x1F;
		} else {
			g_dma_read_empty_threshold = 0x3F;
			g_dma_write_full_threshold = 0x3F;
		}
		vdp_hdr_info[dolby_path_enable_vdp_id].enable = DOVI_RESOLUTION_CHANGE;
		vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE_RES_CHANGE,
			&vdp_hdr_info[dolby_path_enable_vdp_id]);

		if (dolby_out_format_res_change == DOVI_FORMAT_HDR10) {
			memcpy(&rHdr,
				&vdp_hdr_info[0].hdr10_info,
				sizeof(VID_PLA_HDR_METADATA_INFO_T));
			vVdpSetHdrMetadata(true, rHdr);
			DISP_LOG_E("res change hdr10 info %d,%d,%d!\n",
				rHdr.e_DynamicRangeType,
				rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[0],
				rHdr.metadata_info.hdr10_metadata.ui2_MaxDisplayMasteringLuminance);
		}
	}
	dolby_out_format = dolby_out_format_res_change;
	old_resolution = resolution->res_mode;
	vdp_update_dovi_path_delay(false);
}

static void vdp_update_dovi_output_setting_playing(struct disp_hw_common_info *info)
{
	struct disp_hw_tv_capbility *tv_cap = &info->tv;
	struct disp_hw *vdp_drv = disp_vdp_get_drv();

	if (!dolby_force_output) {
		DISP_LOG_E("dolby_force_output is false!\n");
		return;
	}

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	if (dolby_out_format == DOVI_FORMAT_DOVI)
		vDolbyHdrEnable(false);
	else if (dolby_out_format == DOVI_FORMAT_HDR10) {
		vHdrEnable(false);
		vdp_set_HDMI_BT2020_signal(false);
	} else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
		vLowLatencyDolbyVisionEnable(false);
#endif
	vdp_dovi_info.is_graphic_mode = false;
	vdp_dovi_info.is_low_latency = false;
	vdp_dovi_info.vsvdb_edid = tv_cap->vsvdb_edid;

	DISP_LOG_N("vdp_update_dovi_output_setting_playing!\n");

	if (dolby_force_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
		vdp_dovi_info.is_low_latency = true;
	else
		vdp_dovi_info.is_low_latency = false;
	dolby_out_format = dolby_force_out_format;

	vdp_dovi_info.out_format = dolby_out_format;

	if (!dovi_idk_dump)
		vdp_drv->drv_call(DISP_CMD_DOVI_OUT_FORMAT_UPDATE, &vdp_dovi_info);
	else {
		dolby_out_format = dovi_get_output_format();
		if (dovi_get_low_latency_mode() == 1)
			dolby_out_format = DOVI_FORMAT_DOVI_LOW_LATENCY;
		DISP_LOG_N("idk dump output %d!\n", dolby_out_format);
	}

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	if (dolby_out_format == DOVI_FORMAT_DOVI)
		vDolbyHdrEnable(true);
	else if (dolby_out_format == DOVI_FORMAT_HDR10) {
		vSetStaticHdrType(GAMMA_ST2084);
		vHdrEnable(true);
		if (tv_cap->is_support_bt2020)
			vdp_set_HDMI_BT2020_signal(true);
	} else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
		vLowLatencyDolbyVisionEnable(true);
#endif
	vdp_update_dovi_path_delay(false);
}


static void vdp_update_dovi_output_setting(struct disp_hw_common_info *info,
	struct video_buffer_info *buf)
{
	struct disp_hw_tv_capbility *tv_cap = &info->tv;

	vdp_dovi_set_out_format(info, buf);

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	if (dolby_out_format == DOVI_FORMAT_DOVI)
		vDolbyHdrEnable(true);
	else if (dolby_out_format == DOVI_FORMAT_HDR10) {
		vSetStaticHdrType(GAMMA_ST2084);
		vHdrEnable(true);
		if (tv_cap->is_support_bt2020)
			vdp_set_HDMI_BT2020_signal(true);
	} else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
		vLowLatencyDolbyVisionEnable(true);
#endif
}

void vdp_dovi_set_out_format(struct disp_hw_common_info *info,
	struct video_buffer_info *buf)
{
	struct disp_hw_tv_capbility *tv_cap = &info->tv;
	const struct disp_hw_resolution *resolution = info->resolution;
	bool is_2160p60_out = vdp_check_2160p60_timing(resolution->res_mode);
	struct disp_hw *vdp_drv = disp_vdp_get_drv();

	if (buf != NULL) {
		vdp_dovi_info.is_graphic_mode = buf->is_dolby_graphic_mode;
		vdp_dovi_info.is_low_latency = buf->is_dolby_low_latency;
	} else {
		vdp_dovi_info.is_graphic_mode = false;
		vdp_dovi_info.is_low_latency = false;
	}
	vdp_dovi_info.vsvdb_edid = tv_cap->vsvdb_edid;

	DISP_LOG_N("vdp_update_dovi_output_setting\n");
	DISP_LOG_N("support hdr %d, dolby %d dolby_2160p60 %d, dolby_low_latency %d, is_2160p60_out %d\n",
		tv_cap->is_support_hdr,
		tv_cap->is_support_dolby,
		tv_cap->is_support_dolby_2160p60,
		tv_cap->is_support_dolby_low_latency,
		is_2160p60_out);

	/* dolby hw not support 4096 dovi output */
	if ((tv_cap->is_support_dolby_2160p60 ||
		(tv_cap->is_support_dolby && !is_2160p60_out))
		&& (resolution->width <= 3840)) {
		if (tv_cap->is_support_dolby_low_latency) {
			/* if tv support ll and ui set ll, wo set output LL
			* if tv only support ll, we set ooutput LL
			*/
			if ((buf != NULL) && (buf->is_dolby_low_latency)) {
			    DISP_LOG_N("buf->is_dolby_low_latency true\n");
				vdp_dovi_info.is_low_latency = true;
			} else if (tv_cap->dolbyvision_vsvdb_version == 0x2) {
				if ((tv_cap->dolbyvision_vsvdb_v2_interface == 0x0)
					|| (tv_cap->dolbyvision_vsvdb_v2_interface == 0x1))
					vdp_dovi_info.is_low_latency = true;
			} else
				vdp_dovi_info.is_low_latency = false;
		}
		if (vdp_dovi_info.is_low_latency)
			dolby_out_format = DOVI_FORMAT_DOVI_LOW_LATENCY;
		else
			dolby_out_format = DOVI_FORMAT_DOVI;
	} else if (tv_cap->is_support_dolby && is_2160p60_out
			&& (tv_cap->dolbyvision_vsvdb_version == 0x2)
			&& ((tv_cap->dolbyvision_vsvdb_v2_interface == 0x2)
			|| (tv_cap->dolbyvision_vsvdb_v2_interface == 0x3))) {
		vdp_dovi_info.is_low_latency = false;
		dolby_out_format = DOVI_FORMAT_DOVI;
	} else if (tv_cap->is_support_dolby_low_latency && is_2160p60_out) {
		vdp_dovi_info.is_low_latency = true;
		dolby_out_format = DOVI_FORMAT_DOVI_LOW_LATENCY;
	} else if (tv_cap->is_support_hdr)
		dolby_out_format = DOVI_FORMAT_HDR10;
	else
		dolby_out_format = DOVI_FORMAT_SDR;

	/* make sure chosen format can be truly supported - if clock/tmds rate is
	   not enough, move to a lower format */
	if ((tv_cap->max_tmds_rate < DOVI_RES4K60_TMDS_RATE) && is_2160p60_out){
		if ((dolby_out_format == DOVI_FORMAT_DOVI) ||
		    (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY))
			DISP_LOG_N("dolby vision 2160p60 can not be supported: max_tmds_rate=%d\n",
				tv_cap->max_tmds_rate);
		if (tv_cap->is_support_hdr)
			dolby_out_format = DOVI_FORMAT_HDR10;
		else
			dolby_out_format = DOVI_FORMAT_SDR;
	}

	if (dolby_force_output) {
		if (dolby_force_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
			vdp_dovi_info.is_low_latency = true;
		else
			vdp_dovi_info.is_low_latency = false;
		dolby_out_format = dolby_force_out_format;
		dolby_force_output = false;
	}

	vdp_dovi_info.out_format = dolby_out_format;
	if (osd_enable)
		vdp_dovi_info.is_graphic_mode = true;
	else
		vdp_dovi_info.is_graphic_mode = false;

	if (!dovi_idk_dump)
		vdp_drv->drv_call(DISP_CMD_DOVI_OUT_FORMAT_UPDATE, &vdp_dovi_info);
	else {
		dolby_out_format = dovi_get_output_format();
		if (dovi_get_low_latency_mode() == 1)
			dolby_out_format = DOVI_FORMAT_DOVI_LOW_LATENCY;
		DISP_LOG_I("idk dump output %d!\n", dolby_out_format);
	}
}

void vdp_update_dovi_path_delay(int dsd_en)
{
	/* if output is dovi_ll, we must delay 4 bypass core3 dither
	* if do dovi_idk_dump, we must delay 4 bypass core2 yuv2rgb
	*/
	if (dovi_ipcore_version == 4) {
		if (dovi_idk_dump) {
			if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, -4, -4);
				else
					disp_path_set_dolby_by_shift(true, -4, -4);
			} else if (dolby_out_format == DOVI_FORMAT_DOVI) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, 37, -4);
				else
					disp_path_set_dolby_by_shift(true, 37, -4);
			} else {
				if (core3_bypass_dither) {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, -4, -4);
					else
						disp_path_set_dolby_by_shift(true, -4, -4);
				} else {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, 0, -4);
					else
						disp_path_set_dolby_by_shift(true, 0, -4);
				}
			}
		} else {
			if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, -4, -4);
				else
					disp_path_set_dolby_by_shift(true, -4, -4);
			} else if (dolby_out_format == DOVI_FORMAT_DOVI) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, 37, -4);
				else
					disp_path_set_dolby_by_shift(true, 37, -4);
			} else {
				if (core3_bypass_dither) {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, -4, -4);
					else
						disp_path_set_dolby_by_shift(true, -4, -4);
				} else {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, 0, -4);
					else
						disp_path_set_dolby_by_shift(true, 0, -4);
				}
			}
		}
	} else if (dovi_ipcore_version == 1) {
		if (dovi_idk_dump) {
			if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, -4, -4);
				else
					disp_path_set_dolby_by_shift(true, -4, -4);
			} else {
				if (core3_bypass_dither) {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, -4, -4);
					else
						disp_path_set_dolby_by_shift(true, -4, -4);
				} else {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, 0, -4);
					else
						disp_path_set_dolby_by_shift(true, 0, -4);
				}
			}
		} else {
			if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
				if (dsd_en)
					disp_path_set_dsd_dolby_by_shift(true, -4, 0);
				else
					disp_path_set_dolby_by_shift(true, -4, 0);
			} else {
				if (core3_bypass_dither) {
					if (dsd_en)
						disp_path_set_dsd_dolby_by_shift(true, -4, 0);
					else
						disp_path_set_dolby_by_shift(true, -4, 0);
				} else {
					if (dsd_en)
						disp_path_set_dsd_dolby(true);
					else
						disp_path_set_dolby(true);
				}
			}
		}
	}
}

void vdp_dovi_path_enable(void)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};
	uint32_t tmp_dolby_core2_enable = 0;

	if (netflix_dolby_path_enable) {
		DISP_LOG_E("dobly path is already enabled!\n");
		return;
	}
	if (osd_enable == 0) {
		DISP_LOG_E("dobly path is not already for enabled because no ui!\n");
		osd_enable = 1;
	}

	netflix_dolby_osd_no_ready = false;

	if (!g_force_dolby) {
		vdout_sys_hal_dolby_mix_on(true);
		disp_clock_enable(DISP_CLK_DOLBY1, true);
		disp_clock_enable(DISP_CLK_DOLBY3, true);
		disp_clock_enable(DISP_CLK_DOLBY_MIX, true);
    }
	disp_clock_smi_larb_en(DISP_SMI_LARB4, true);
	disp_hw_mgr_get_info(&disp_common_info);
	fmt_hal_set_output_444(DISP_FMT_MAIN, false);
	fmt_hal_set_output_444(DISP_FMT_SUB, false);
	fmt_hal_set_uv_swap(DISP_FMT_MAIN, true);
	fmt_hal_set_uv_swap(DISP_FMT_SUB, true);
	dovi_set_video_input_format(DOVI_FORMAT_SDR);
	vdp_update_dovi_output_setting(&disp_common_info, NULL);
	if ((video_layer[0].state == VDP_LAYER_IDLE)
		|| (video_layer[0].state == VDP_LAYER_STOPPING)
		|| (video_layer[0].state == VDP_LAYER_STOPPED))
		dovi_core1_hal_set_out_fix_pattern_enable(true, 0x04, 0x200, 0x200);
	tmp_dolby_core2_enable = 1;
	if (!g_force_dolby)
		vdp_drv->drv_call(DISP_CMD_DOVI_CORE2_UPDATE, &tmp_dolby_core2_enable);
	else
		dolby_core2_enable = 1;

	DISP_LOG_E("dobly path enable not mix graphic!\n");

	if (!g_force_dolby) {
		vdp_hdr_info[0].enable = 1;
		vdp_hdr_info[0].dr_range = DISP_DR_TYPE_SDR;
		vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[0]);
	} else {
		vdp_hdr_info[0].enable = 1;
		vdp_hdr_info[0].dr_range = DISP_DR_TYPE_SDR;
		DISP_LOG_E("dobly path enable process!\n");
		disp_dovi_process(1, &vdp_hdr_info[0]);
	}
	/* vdp_update_dovi_path_delay(false); delay to next vsync*/

	dolby_path_enable = 1;
	netflix_dolby_path_enable = true;
	dovi_vs10_path_enable_cnt = 0;

	if (!g_force_dolby) {
		fmt_hal_not_mix_plane(FMT_HW_PLANE_3);

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if ((dolby_path_enable) && (dolby_out_format == DOVI_FORMAT_HDR10)) {
			memcpy(&rHdr,
				&vdp_hdr_info[0].hdr10_info,
				sizeof(VID_PLA_HDR_METADATA_INFO_T));
			rHdr.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10;
			rHdr.fgIsMetadata = true;
			rHdr.metadata_info.hdr10_metadata.fgNeedUpdStaticMeta = true;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[0] = 35400;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[1] = 8500;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[2] = 6550;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[0] = 14600;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[1] = 39850;
			rHdr.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[2] = 2300;
			rHdr.metadata_info.hdr10_metadata.ui2_WhitePointX = 15635;
			rHdr.metadata_info.hdr10_metadata.ui2_WhitePointY = 16450;
			rHdr.metadata_info.hdr10_metadata.ui2_MaxCLL = 0;
			rHdr.metadata_info.hdr10_metadata.ui2_MaxFALL = 0;
			rHdr.metadata_info.hdr10_metadata.ui2_MaxDisplayMasteringLuminance = 1000;
			rHdr.metadata_info.hdr10_metadata.ui2_MinDisplayMasteringLuminance = 50;
			vVdpSetHdrMetadata(true, rHdr);
		}
#endif
	}
}

void vdp_dovi_path_disable(void)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();

	if (netflix_dolby_path_enable || dolby_path_enable) {
		dolby_path_enable = 0;
		netflix_dolby_path_enable = false;
		g_force_dolby = false;

		DISP_LOG_N("%s#%d set dobly path disable\n", __func__,__LINE__);
		disp_path_set_dolby(false);
		dovi_core1_hal_set_out_fix_pattern_enable(false, 0x0, 0x0, 0x0);
		fmt_hal_set_output_444(DISP_FMT_MAIN, true);
		fmt_hal_set_output_444(DISP_FMT_SUB, true);
		fmt_hal_set_uv_swap(DISP_FMT_MAIN, false);
		fmt_hal_set_uv_swap(DISP_FMT_SUB, false);
		vdout_sys_hal_dolby_mix_on(false);
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if (dolby_out_format == DOVI_FORMAT_DOVI)
			vDolbyHdrEnable(false);
		else if (dolby_out_format == DOVI_FORMAT_HDR10) {
			vHdrEnable(false);
			vdp_set_HDMI_BT2020_signal(false);
		}
		else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
			vLowLatencyDolbyVisionEnable(false);
#endif
		vdp_hdr_info[0].dr_range = DISP_DR_TYPE_PHLP_RESVERD;
		vdp_hdr_info[0].enable = 0;
		vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[0]);
	} else if (g_force_dolby) {
		DISP_LOG_E("%s#%d for g_force_dolby true, g_hdr_type %u\n", __func__, __LINE__, g_hdr_type);
		disp_path_set_dolby(false);
		dovi_core1_hal_set_enable(false);
		dovi_core2_hal_set_enable(false);
		dovi_core3_hal_set_enable(false);
		if (g_hdr_type == LK_HDR_TYPE_HDR10_ST2084) {
			vHdrEnable(false);
			vBT2020Enable(false);
		} else if (g_hdr_type == LK_HDR_TYPE_DOVI_STD)
			vDolbyHdrEnable(false);
		else if (g_hdr_type == LK_HDR_TYPE_DOVI_LOWLATENCY)
			vLowLatencyDolbyVisionEnable(false);
		dolby_path_enable = 0;
		g_force_dolby = 0;
		g_hdr_type = 0;
		g_out_format = 2;
	} else {
		DISP_LOG_E("dobly path is not enable!\n");
		return;
	}
}


static void vdp_update_dovi_setting(unsigned char vdp_id,
	struct video_buffer_info *buf, int dsd_en)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
	bool hasupdate = false;
	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};
	bool update_hdrinfo = false;

	mutex_lock(&disp_dovi_mutex);
	if (disp_common_info.tv.force_hdr > 1)
		buf->is_dolby_vs10 = true;
	if (dolby_path_start && !netflix_dolby_path_enable) {
		if ((buf->is_dolby || buf->is_dolby_vs10 || dovi_idk_dump || dovi_vs10_force)
				&& dovi_core1_hal_is_support()) {
			dolby_path_enable = 1;
			dolby_path_enable_cnt = 0;
			dolby_path_enable_vdp_id = vdp_id;
			DISP_LOG_N("set dobly path enable, pts %lld\n", buf->pts);

			DISP_LOG_N("vdp dr_range %d buf dr_range %d\n",
				vdp_hdr_info[vdp_id].dr_range,
				buf->hdr_info.dr_range);

			if (!dovi_idk_dump) {
				disp_dovi_set_mute_unmute(true);
				fmt_hal_not_mix_plane(FMT_HW_PLANE_3);
			}

			fmt_hal_set_output_444(vdp_id, false);
			fmt_hal_set_uv_swap(vdp_id, true);
			vdout_sys_hal_dolby_mix_on(true);

			disp_clock_smi_larb_en(DISP_SMI_LARB4, true);
			disp_clock_enable(DISP_CLK_DOLBY1, true);
			disp_clock_enable(DISP_CLK_DOLBY3, true);
			disp_clock_enable(DISP_CLK_DOLBY_MIX, true);
			dovi_core1_hal_set_out_fix_pattern_enable(false, 0x0, 0x0, 0x0);

			vdp_update_dovi_output_setting(&disp_common_info, buf);

			if (dovi_idk_dump)
				vdp_drv->drv_call(DISP_CMD_OSD_UPDATE, &dovi_idk_dump);

			buf->hdr_info.enable = 1;
			vdp_hdr_info[vdp_id] = buf->hdr_info;
			if (buf->hdr_info.dr_range == DISP_DR_TYPE_HDR10) {
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[0] =
					buf->hdr10_info.ui2_DisplayPrimariesX[0];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[1] =
					buf->hdr10_info.ui2_DisplayPrimariesX[1];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[2] =
					buf->hdr10_info.ui2_DisplayPrimariesX[2];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[0] =
					buf->hdr10_info.ui2_DisplayPrimariesY[0];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[1] =
					buf->hdr10_info.ui2_DisplayPrimariesY[1];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[2] =
					buf->hdr10_info.ui2_DisplayPrimariesY[2];
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_WhitePointX =
					buf->hdr10_info.ui2_WhitePointX;
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_WhitePointY =
					buf->hdr10_info.ui2_WhitePointY;
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxDisplayMasteringLuminance =
					buf->hdr10_info.ui2_MaxDisplayMasteringLuminance;
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MinDisplayMasteringLuminance =
					buf->hdr10_info.ui2_MinDisplayMasteringLuminance;
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxCLL =
					buf->hdr10_info.ui2_MaxCLL;
				vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxFALL =
					buf->hdr10_info.ui2_MaxFALL;
			}
			vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[vdp_id]);
			hasupdate = true;

			vdp_update_dovi_path_delay(dsd_en);
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
			if ((dolby_path_enable) && (dolby_out_format == DOVI_FORMAT_HDR10)) {
				memcpy(&rHdr,
					&vdp_hdr_info[vdp_id].hdr10_info,
					sizeof(VID_PLA_HDR_METADATA_INFO_T));
				vVdpSetHdrMetadata(true, rHdr);
			}
#endif
			dolby_path_start = 0;
		} else
			vdp_hdr_info[vdp_id] = buf->hdr_info;
	} else if (dolby_path_start && netflix_dolby_path_enable) {
		fmt_hal_set_output_444(vdp_id, false);
		fmt_hal_set_uv_swap(vdp_id, true);
		vdp_update_dovi_output_setting(&disp_common_info, buf);
		vdp_update_dovi_path_delay(dsd_en);
		dovi_core1_hal_set_out_fix_pattern_enable(false, 0x0, 0x0, 0x0);
		if (buf->hdr_info.dr_range == DISP_DR_TYPE_SDR) {
			buf->hdr_info.enable = 1;
			vdp_hdr_info[vdp_id] = buf->hdr_info;
			vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[vdp_id]);
			hasupdate = true;
		} else if ((buf->hdr_info.dr_range == DISP_DR_TYPE_HDR10)
			&& (vdp_hdr_info[vdp_id].dr_range == buf->hdr_info.dr_range)) {
			buf->hdr_info.enable = 1;
			vdp_hdr_info[vdp_id] = buf->hdr_info;
			vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[vdp_id]);
			hasupdate = true;
		}
		dolby_path_start = 0;
	}

	if (dolby_force_output)
		vdp_update_dovi_output_setting_playing(&disp_common_info);

	if ((dolby_path_enable) && (!hasupdate)) {
		dolby_path_enable_cnt++;
		if ((dolby_path_enable_cnt == 2) && (!dovi_idk_dump)
			&& (!netflix_dolby_path_enable)) {
			disp_dovi_set_mute_unmute(false);
			fmt_hal_mix_plane(FMT_HW_PLANE_3);
		}
	}
	if ((dolby_path_enable) && (!hasupdate)
		&& ((buf->hdr_info.dr_range == DISP_DR_TYPE_DOVI)
		|| (vdp_hdr_info[vdp_id].dr_range != buf->hdr_info.dr_range))) {
		if ((vdp_hdr_info[vdp_id].dr_range != buf->hdr_info.dr_range)
			|| (dolby_force_output)) {
			buf->hdr_info.enable = DOVI_INOUT_FORMAT_CHANGE;
			dolby_force_output = false;
			update_hdrinfo = true;
		} else
			buf->hdr_info.enable = 1;
		vdp_hdr_info[vdp_id] = buf->hdr_info;
		if (buf->hdr_info.dr_range == DISP_DR_TYPE_HDR10) {
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[0] =
				buf->hdr10_info.ui2_DisplayPrimariesX[0];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[1] =
				buf->hdr10_info.ui2_DisplayPrimariesX[1];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[2] =
				buf->hdr10_info.ui2_DisplayPrimariesX[2];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[0] =
				buf->hdr10_info.ui2_DisplayPrimariesY[0];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[1] =
				buf->hdr10_info.ui2_DisplayPrimariesY[1];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[2] =
				buf->hdr10_info.ui2_DisplayPrimariesY[2];
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_WhitePointX =
				buf->hdr10_info.ui2_WhitePointX;
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_WhitePointY =
				buf->hdr10_info.ui2_WhitePointY;
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxDisplayMasteringLuminance =
				buf->hdr10_info.ui2_MaxDisplayMasteringLuminance;
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MinDisplayMasteringLuminance =
				buf->hdr10_info.ui2_MinDisplayMasteringLuminance;
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxCLL =
				buf->hdr10_info.ui2_MaxCLL;
			vdp_hdr_info[vdp_id].metadata_info.hdr10_metadata.ui2_MaxFALL =
				buf->hdr10_info.ui2_MaxFALL;
		}
		vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[vdp_id]);
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if ((dolby_path_enable) && (dolby_out_format == DOVI_FORMAT_HDR10)) {
			memcpy(&rHdr,
				&vdp_hdr_info[vdp_id].hdr10_info,
				sizeof(VID_PLA_HDR_METADATA_INFO_T));
			vVdpSetHdrMetadata(true, rHdr);
		}
#endif
	}
	mutex_unlock(&disp_dovi_mutex);
}

static void vdp_disable_dolby_path(void)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
	bool dovi_idk_dump_osd = false;
	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};
	enum DISP_DR_TYPE_T dovi_input_dr_type = DISP_DR_TYPE_SDR;

	if ((dolby_path_enable == 1) && (!netflix_dolby_path_enable)) {
		dolby_path_enable = 0;
		dolby_path_disable = true;
		dolby_path_disable_cnt = 0;
		DISP_LOG_I("%s#%d set dobly path disable\n", __func__, __LINE__);
		disp_path_set_dolby(false);
		fmt_hal_set_output_444(DISP_FMT_MAIN, true);
		fmt_hal_set_output_444(DISP_FMT_SUB, true);
		fmt_hal_set_uv_swap(DISP_FMT_MAIN, false);
		fmt_hal_set_uv_swap(DISP_FMT_SUB, false);
		vdout_sys_hal_dolby_mix_on(false);
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if (dolby_out_format == DOVI_FORMAT_DOVI)
			vDolbyHdrEnable(false);
		else if (dolby_out_format == DOVI_FORMAT_HDR10) {
			vHdrEnable(false);
			vdp_set_HDMI_BT2020_signal(false);
		}
		else if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
			vLowLatencyDolbyVisionEnable(false);
#endif

		if (dovi_idk_dump) {
			vdp_drv->drv_call(DISP_CMD_OSD_UPDATE, &dovi_idk_dump_osd);
			idk_dump_vsync_cnt = -1;
			dovi_idk_dump_set_vin = false;
			dovi_idk_dump = false;
		}
	} else if ((dolby_path_enable == 1) && netflix_dolby_path_enable) {
		vdp_hdr_info[0].enable = DOVI_INOUT_FORMAT_CHANGE;
		vdp_hdr_info[0].dr_range = DISP_DR_TYPE_SDR;
		dovi_core1_hal_set_out_fix_pattern_enable(true, 0x04, 0x200, 0x200);
		dovi_get_input_format(&dovi_input_dr_type);
		vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[0]);
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
		if ((dovi_input_dr_type != DISP_DR_TYPE_SDR) && (dolby_out_format == DOVI_FORMAT_HDR10)) {
			memcpy(&rHdr,
				&vdp_hdr_info[0].hdr10_info,
				sizeof(VID_PLA_HDR_METADATA_INFO_T));
			vVdpSetHdrMetadata(true, rHdr);
		}
#endif
	} else if ((debug_dovi_path_full_vs10 || (ui_force_hdr_type > 1))
		&& (dolby_path_enable == 0)) {
		dolby_path_restart = true;
		DISP_LOG_I("set dobly path enable because of vs10\n");
		vdp_config_hdmi_signal(hdr_enable, NULL, false);
		hdr_enable =  HDR10_TYPE_NONE;
		DISP_LOG_I("[VDP] set pre hdr disable %d\n", hdr_enable);
		vdp_set_HDMI_BT2020_signal(false);
		vdp_dovi_path_enable();
	}
}

static void vdp_disable_display_path(struct video_layer_info *layer_info)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
	struct vdp_hal_config_info config_info;
	uint32_t *reg_addr = (uint32_t *)(DISPSYS_BASE + 8);
	uint32_t value = *reg_addr;

	DISP_LOG_N("%s\n", __func__);

	layer_info->last_pts = 0xffffffff;

	memset(&layer_info->src_rgn, 0, sizeof(struct mtk_disp_range));
	memset(&layer_info->tgt_rgn, 0, sizeof(struct mtk_disp_range));
	layer_info->enable = false;
	layer_info->res_change = false;
	layer_info->src_phy_addr = 0;
	layer_info->buffer_size = 0;

	/* disable VDO */
	memset(&config_info, 0, sizeof(struct vdp_hal_config_info));
	vdp_hal_config(layer_info->layer_id, &config_info);
	/* disable disp_fmt. */
	fmt_hal_enable(layer_info->layer_id, false);
	video_layer[layer_info->layer_id].sdr2hdr = false;

		/* stop HDR10 signal */
	if ((layer_info->layer_id == 0)
		&& (dolby_path_enable != 1)
		&& (!netflix_dolby_path_enable)) {
		UINT32 hdr_osd_path = 2;

		vdp_config_hdmi_signal(hdr_enable, NULL, false);
		hdr_enable = HDR10_TYPE_NONE;
		DISP_LOG_I("[VDP%d] set hdr enable %d\n", layer_info->layer_id, hdr_enable);
		/*when TV had stopped, set the current TV play source to sdr */
		vdp_drv->drv_call(DISP_CMD_PLAY_HDR_SOURCE,
			&hdr_enable);
		/*stopping using sdrhdr&bt2020 module if video is stopped */
		vdp_drv->drv_call(DISP_CMD_STOP_SDR2HDR_BT2020,
			&hdr_osd_path);

		vdp_set_HDMI_BT2020_signal(false);
	} else if ((layer_info->layer_id == 0)
		&& (dolby_path_enable == 1)
		&& (netflix_dolby_path_enable)) {
		UINT32 hdr_osd_path = 2;
		/*stopping using sdrhdr&bt2020 module if video is stopped */
		vdp_drv->drv_call(DISP_CMD_STOP_SDR2HDR_BT2020,
			&hdr_osd_path);
	}


	if (layer_info->layer_id == 0) {
		fmt_hal_clock_on_off(DISP_FMT_MAIN, false);
		disp_clock_enable(DISP_CLK_VDO3, false);
		vdout_sys_hal_select_dsd_clk(false);

		if (layer_info->secure_en) {
			layer_info->secure_en = false;
			layer_info->secure2normal= false;
			disp_vdp_sec_deinit(layer_info->layer_id);
		}

		disp_clock_smi_larb_en(DISP_SMI_LARB5, false);
		vdp_drv->drv_call(DISP_CMD_STOP_HDR2SDR_BT2020,
			&(layer_info->layer_id));
	} else if (layer_info->layer_id == 1) {
		disp_clock_enable(DISP_CLK_VDO4, false);
		fmt_hal_clock_on_off(DISP_FMT_SUB, false);
		vdout_sys_hal_select_dsd_clk(false);

		if (layer_info->secure_en) {
			layer_info->secure_en = false;
			layer_info->secure2normal= false;
			disp_vdp_sec_deinit(layer_info->layer_id);
		}

		disp_clock_smi_larb_en(DISP_SMI_LARB6, false);

		#if 1
		/* Plan A */
		/* if play dolby video in main path, need to enable premix clock.
		** and also disable premix after stop vdo4.
		*/
		//disp_vdp_enable_premix_clock(false);
		if (disp_vdp_get_osd_premix()) {
			disp_clock_enable(DISP_CLK_OSD_PREMIX, false);
			disp_vdp_set_osd_premix(false);
			disp_vdp_enable_premix_clock(disp_vdp_get_osd_premix());
		}
		#else
		/* Plan C2 */
		if (disp_vdp_get_main_sub_swap_status()) {
			uint32_t enable_vdo4 = 1;
			/* enable vdo4 output to pre-mix */
			disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &enable_vdo4);

			/* enable YUV444 */
			fmt_hal_set_output_444(DISP_FMT_SUB, true);
			disp_vdp_set_main_sub_swap(0);
		}
		#endif
		DISP_LOG_I("stop sub path hdr2sdr_bt2020\n");
		vdp_drv->drv_call(DISP_CMD_STOP_HDR2SDR_BT2020,
			&(layer_info->layer_id));

		/* if stop sub video, need to set vdout_fmt not mix sub plane. */
		fmt_hal_not_mix_plane(FMT_HW_PLANE_2);
	}

	/* reset VDP: write 0 for reset, when stopping video. */
	/* write 0x15000008[0] = 0 for layer 0 */
	/* write 0x15000008[7] = 0 for layer 1 */
	if (layer_info->layer_id == 0) {
		value &= 0xFFFFFFFE;
		*(reg_addr) = value;
	} else if (layer_info->layer_id == 1) {
		value &= 0xFFFFFF7F;
		*(reg_addr) = value;
	} else
		DISP_LOG_E("invalid layer id:%d\n", layer_info->layer_id);

	layer_info->state = VDP_LAYER_IDLE;
	wake_up(&layer_info->wait_queue);
}

/* for start */
void vdp_disable_active_zone(struct video_layer_info *layer_info)
{
	struct fmt_active_info active_info;
	uint32_t *reg_addr = (uint32_t *)(DISPSYS_BASE + 8);
	uint32_t value = *reg_addr;

	/* only 1 display buffer is in	buffer list. */
	memset(&active_info, 0, sizeof(struct fmt_active_info));
	fmt_hal_set_active_zone(layer_info->layer_id == 0 ?
		VDOUT_FMT : VDOUT_FMT_SUB,
		&active_info);
	fmt_hal_shadow_update();
	fmt_hal_set_active_zone(layer_info->layer_id == 0 ?
		DISP_FMT_MAIN : DISP_FMT_SUB,
		&active_info);



	/* reset VDP: write 1 for de-reset when start playing. */
	/* write 0x15000008[0] = [1] for layer 0 */
	/* write 0x15000008[7] = [1] for layer 1 */
	if (layer_info->layer_id == 0) {
		value |= 0x00000001;
		*(reg_addr) = value;
	} else if (layer_info->layer_id == 1) {
		value |= 0x00000080;
		*(reg_addr) = value;
	} else
		DISP_LOG_E("vdp_disable_active_zone invalid layer id:%d\n", layer_info->layer_id);
}

/* for stop */
void vdp_disable_vdout_active_zone(struct video_layer_info *layer_info)
{
	struct fmt_active_info active_info;

	/* only 1 display buffer is in	buffer list. */
	memset(&active_info, 0, sizeof(struct fmt_active_info));
	if (layer_info->secure_en)
		disp_vdp_sec_disable_active_zone(layer_info->layer_id);
	else
		fmt_hal_set_active_zone(layer_info->layer_id == 0 ?
			DISP_FMT_MAIN : DISP_FMT_SUB,
			&active_info);
	fmt_hal_set_active_zone(layer_info->layer_id == 0 ?
		VDOUT_FMT : VDOUT_FMT_SUB,
		&active_info);
	fmt_hal_shadow_update();
}


#define MAX_NUM_WINDOWS 3
#define MULBASE 10

uint16_t vsif_min(uint16_t sei_value, uint16_t mocecule_de,
	uint32_t mocecule_max, uint16_t denominater, uint16_t max_value)
{
	uint32_t sei_value_new;
	uint32_t temp = 0;
	uint16_t vsif_value = 0;

	temp = (sei_value * MULBASE) / mocecule_de;
	temp = (temp > mocecule_max * MULBASE) ? mocecule_max * MULBASE:temp;
	temp = temp/denominater;
	if ((temp % 10) >= 5)
		temp = 1;
	else
		temp = 0;

	sei_value_new = sei_value/mocecule_de;
	sei_value_new = (sei_value_new > mocecule_max) ? mocecule_max : sei_value_new;
	vsif_value = (uint16_t)(sei_value_new / denominater + temp);
	vsif_value = (vsif_value > max_value) ? max_value : vsif_value;
	return vsif_value;
}

void user_data_registered_inu_t_t35(unsigned char *data, uint32_t size, uint8_t *vsif)
{
	uint8_t country_code = 0;

	uint8_t application_identifier = 0;
	uint8_t application_version = 0;
	uint8_t num_windows = 0;

	uint8_t num_idx = 0;
	uint8_t idx = 0;
	uint32_t sel_tsdmaxl = 0;
	uint8_t vsif_tsdmaxl = 0;
	uint32_t sei_avgmaxrgb = 0;
	uint8_t vsif_avgmaxrgb = 0;
	uint8_t num_sei_dmaxrgb = 0;
	uint32_t sei_dmaxrgb[16];/* distributtion */
	uint8_t sei_dmaxrgb_idx[16];/* distributtion */
	uint8_t vsif_dmaxrgb[16];
	uint8_t tone_mapping_flag = 0;
	uint32_t sei_kpx = 0;
	uint8_t vsif_kpx = 0;
	uint32_t sei_kpy = 0;
	uint8_t vsif_kpy = 0;
	uint8_t num_sei_bz_a = 0;
	uint32_t sei_bz_a[9];
	uint8_t vsif_bz_a[9];
	int remained_bits;
	uint32_t value;
	unsigned char *metadata = NULL;

	if (data == NULL || vsif == NULL)
		return;

	metadata = data;
	country_code = *metadata;
	/* skip country code, privider_code, provider_oriented_code(u(8)+u(16)+u(16)) */
	metadata += 5;

	application_identifier = *metadata;
	application_version = *(metadata+1);

	metadata += 2;
	num_windows = (((*metadata) & 0xC0) >> 6);

	/* 27 bit */
	sel_tsdmaxl = (((*(metadata)) << 24) |
		((*(metadata + 1)) << 16) |
		((*(metadata + 2)) << 8) |
		(*(metadata + 3)));
	sel_tsdmaxl = (sel_tsdmaxl & 0x3FFFFFFF) >> 3;
	metadata += 3; /* 2bit */
	vdp_printf(VDP_HDR10PLUS_LOG, "country_code:0x%x num_windows:%d sel_tsdmaxl:0x%x *metadata = 0x%x\n",
		country_code, num_windows, sel_tsdmaxl, *metadata);

	remained_bits = 2;
	for (num_idx = 0; num_idx < num_windows; ++num_idx) {
	/*remained_bits +17bit(maxscl)*3 = min:51bit.max:58 */

	vdp_printf(VDP_HDR10PLUS_LOG, "num_idx[%d] *(metadata) = 0x%x remained_bits = %d\n",
		num_idx, *(metadata), remained_bits);

	/* skip maxsel */

	if ((remained_bits == 0) || (remained_bits > 3)) {
		metadata += 6;
		remained_bits = 8 + 6 * 8 - (51 - remained_bits);
	} else {
		metadata += 7;
		remained_bits = 8 + 6 * 8 - (51 - remained_bits);
	}



	vdp_printf(VDP_HDR10PLUS_LOG, "after checkmaxScl *(metadata) = 0x%x remained_bits = %d\n",
		*(metadata), remained_bits);

	remained_bits = 8 - (17 - (remained_bits + 8));

	vdp_printf(VDP_HDR10PLUS_LOG, "after avgmaxrgb  *(metadata) = 0x%x remained_bits = %d\n",
		*(metadata), remained_bits);


	sei_avgmaxrgb = ((((*(metadata)) << 16) |
		((*(metadata + 1)) << 8) |
		(*(metadata + 2))) >> remained_bits) & 0x1FF;
	vdp_printf(VDP_HDR10PLUS_LOG, "sei_avgmaxrgb:%d, remained_bits = %d *metadata = 0x%x, 0x%x, 0x%x\n",
		sei_avgmaxrgb, remained_bits, *metadata, *(metadata + 1), *(metadata + 2));

	metadata += 2;
	if (remained_bits >= 4) {
		remained_bits -= 4;
		num_sei_dmaxrgb = ((*metadata) >> remained_bits) & 0xF;
	} else {
		remained_bits = 8 - remained_bits;
		num_sei_dmaxrgb = ((((*metadata) << 8) | (*(metadata + 1))) >> remained_bits) & 0xF;
		metadata++;
	}
	if (remained_bits == 0)
		metadata++;

	vdp_printf(VDP_HDR10PLUS_LOG, "num_sei_dmaxrgb:%d *(metadata) = 0x%x remained_bits = %d\n",
		num_sei_dmaxrgb, *(metadata), remained_bits);
	memset(sei_dmaxrgb, 0, sizeof(sei_dmaxrgb));
	for (idx = 0; idx < num_sei_dmaxrgb; ++idx) {
		if (remained_bits)
			sei_dmaxrgb[idx] = ((((*metadata) << 24) |
				((*(metadata + 1)) << 16) |
				((*(metadata + 2)) << 8) |
				(*(metadata + 3))) >> remained_bits) & 0xFFFFFF;
		else
			sei_dmaxrgb[idx] = ((((*(metadata)) << 16) |
				((*(metadata + 1)) << 8) |
				(*(metadata + 2)))) >> remained_bits;
		sei_dmaxrgb_idx[idx] = (sei_dmaxrgb[idx] & 0xFE0000) >> 17;
		sei_dmaxrgb[idx] = sei_dmaxrgb[idx] & 0x1FFFF;
		vdp_printf(VDP_HDR10PLUS_LOG, "sei_dmaxrgb_idx:%d, value; %d (*metadata) = 0x%x\n",
			sei_dmaxrgb_idx[idx], sei_dmaxrgb[idx], (*metadata));
		metadata += 3;
	}

	vdp_printf(VDP_HDR10PLUS_LOG, "after dmxrgb: remained_bits = %d, *metadata = 0x%x\n",
		remained_bits, *metadata);
	/* 10bit */

	if (remained_bits == 0) {
		metadata += 1;
		remained_bits = 8 + 8 - 10;/* 3 */
	} else if (remained_bits <= 2) {
		metadata += 2;
		if (remained_bits == 2)
			remained_bits = remained_bits + 8 - 10;/* 3 */
		else
			remained_bits = remained_bits + 8 - 10 + 8;/* 3 */
	} else {
		metadata += 1;
		remained_bits = remained_bits + 8 - 10;/* 3 */
	}

	vdp_printf(VDP_HDR10PLUS_LOG, "after bright_pixel: remained_bits = %d, *metadata = 0x%x\n",
		remained_bits, *metadata);
	}

	vdp_printf(VDP_HDR10PLUS_LOG, "before tone_mapping_flag: remained_bits = %d, *metadata = 0x%x\n",
		remained_bits, *metadata);

	/*skip luminance flag 1bit */
	if (remained_bits == 0) {
		metadata += 1;
		remained_bits = 7;
	} else if (remained_bits == 1) {
		metadata += 1;
		remained_bits = 0;
	} else
		remained_bits -= 1;

	for (num_idx = 0; num_idx < num_windows; ++num_idx) {
		/*1bit */
		if (remained_bits) {
			remained_bits--;
			tone_mapping_flag = (((*metadata)) >> remained_bits) & 0x1;
			if (remained_bits == 0)
				metadata += 1;
		} else {
			remained_bits = 7;
			tone_mapping_flag = (((*metadata)) >> remained_bits) & 0x1;
		}

		vdp_printf(VDP_HDR10PLUS_LOG, "[%d, %d]tone_mapping_flag:%d *metadata = 0x%x remained_bits = %d\n",
			num_idx, num_windows, tone_mapping_flag, *metadata, remained_bits);
		if (tone_mapping_flag) {

			if (remained_bits)
				value = (((*metadata << 24) |
					(*(metadata + 1) << 16) |
					(*(metadata + 2) << 8) |
					*(metadata + 3)) >> remained_bits) & 0xFFFFFF;
			else
				value = ((*metadata << 16) |
					((*(metadata + 1)) << 8) |
					((*(metadata + 2)))) >> remained_bits;

			sei_kpx = (value & 0xFFF000) >> 12;
			sei_kpy = (value & 0xFFF);
			vdp_printf(VDP_HDR10PLUS_LOG, "sei_kpx:%d sei_kpy:%d *metadata = 0x%x remained_bits = %d\n",
				sei_kpx, sei_kpy, *metadata, remained_bits);
			metadata += 3;

			if (remained_bits >= 4) {
				remained_bits -= 4;
				num_sei_bz_a = ((*metadata) >> remained_bits) & 0xF;

				if (remained_bits == 0)
					metadata += 1;
			} else if (remained_bits == 0) {
				remained_bits = 4;
				num_sei_bz_a = ((*metadata) >> remained_bits) & 0xF;
			} else {
				remained_bits = 8 - 4 - remained_bits;
				num_sei_bz_a = (((*metadata << 8) | (*(metadata + 1))) >> remained_bits) & 0xF;
			}
			vdp_printf(VDP_HDR10PLUS_LOG, "num_sei_bz_a:%d *metadata = 0x%x remained_bits = %d\n",
				num_sei_bz_a, *metadata, remained_bits);
			memset(sei_bz_a, 0, sizeof(sei_bz_a));

			for (idx = 0; idx < num_sei_bz_a; ++idx) {
				vdp_printf(VDP_HDR10PLUS_LOG, "*metadata = 0x%x remained_bits = %d\n",
					*metadata, remained_bits);
				if (remained_bits == 1) {
					remained_bits = 16 - (1 + 8);
					sei_bz_a[idx] = (((*metadata << 16) |
						(*(metadata + 1) << 8 |
						(*metadata))) >> remained_bits) & 0x3FF;
					metadata += 1;
				} else if (remained_bits == 2) {
					remained_bits = remained_bits + 8 - 10;
					sei_bz_a[idx] = (((*metadata << 8) |
						(*(metadata + 1))) >> remained_bits) & 0x3FF;
					metadata += 2;
				} else if (remained_bits == 0) {
					remained_bits = 8 + 8 - 10;
					sei_bz_a[idx] = (((*metadata << 8) |
						(*(metadata + 1))) >> remained_bits) & 0x3FF;
					metadata += 1;
				} else {
					remained_bits = remained_bits + 8 - 10;
					sei_bz_a[idx] = (((*metadata << 8) |
						(*(metadata + 1))) >> remained_bits) & 0x3FF;
					metadata += 1;
				}
				vdp_printf(VDP_HDR10PLUS_LOG, "sei_bz_a[%d] = %d *metadata = 0x%x remained_bits = %d\n",
					idx, sei_bz_a[idx], *metadata, remained_bits);
			}
		}

		if (remained_bits == 0) {
			metadata += 1;
			remained_bits = 7;
		} else if (remained_bits == 1) {
			metadata += 1;
			remained_bits = 0;
		} else
			remained_bits -= 1;
	}


	vsif_tsdmaxl  = vsif_min(sel_tsdmaxl, 1, 1024, 32, 31);
	vsif_avgmaxrgb = vsif_min(sei_avgmaxrgb, 10, 4096, 16, 255);

	vdp_printf(VDP_HDR10PLUS_LOG, "vsif_tsdmaxl = %d vsif_avgmaxrgb = %d\n", vsif_tsdmaxl, vsif_avgmaxrgb);

	vsif[0] = (0x40) | (vsif_tsdmaxl << 1);
	vsif[1] = vsif_avgmaxrgb;
	for (idx = 0; idx < num_sei_bz_a; ++idx) {
		if (idx != 2)
			vsif_dmaxrgb[idx] = vsif_min(sei_dmaxrgb[idx], 10, 4096, 16, 255);
		else
			vsif_dmaxrgb[idx] = sei_dmaxrgb[idx];
		vsif[2 + idx] = vsif_dmaxrgb[idx];
	}

	vsif_kpx = vsif_min(sei_kpx, 1, 0xFFFF, 4, 1023);

	vsif_kpy = vsif_min(sei_kpy, 1, 0xFFFF, 4, 1023);


	vdp_printf(VDP_HDR10PLUS_LOG, "sei_kpx = %d, vsif_kpx = %d, sei_kpy = %d, vsif_kpy = %d\n",
		sei_kpx, vsif_kpx, sei_kpy, vsif_kpy);

	vsif[11] = (num_sei_bz_a << 4) | ((vsif_kpx & 0x3C0) >> 6);
	vsif[12] = ((vsif_kpx & 0x3F) << 2) | ((vsif_kpy & 0x300) >> 8);
	vsif[13] = (vsif_kpy & 0xFF);
	for (idx = 0; idx < 9; ++idx) {
		vsif_bz_a[idx] = vsif_min(sei_bz_a[idx], 1, 0xFFFF, 4, 255);
		vsif[14 + idx] = vsif_bz_a[idx];
		vdp_printf(VDP_HDR10PLUS_LOG, "vsif_bz_a = %d, sei_bz_a = %d\n", sei_bz_a[idx], vsif_bz_a[idx]);
	}

	if (osd_enable)
		vsif[23] = 0x80;
	else
	vsif[23] = 0x00;
}


uint32_t vdp_bitshift(int32_t needbits)
{
	uint32_t retValue = 1;
	int32_t temp = 0;

	temp = needbits;
	while (needbits) {
		retValue *= 2;
		needbits--;
	}
	return (retValue - 1);
}


UINT32 vdp_bitvalue_get(uint8_t *hdr_metadata, int32_t needbits, int32_t *remained_bits, uint32_t *addBytes)
{
	int32_t diff = 0;
	int32_t i = 0;
	uint32_t retValue = 0;
	*addBytes = 0;
	diff = needbits - (*remained_bits);
	if (diff < 0) {
		(*remained_bits) -= needbits;
		retValue = ((*hdr_metadata) >> (*remained_bits)) & vdp_bitshift(needbits);
	} else {
		if (diff == 0) {
			retValue = ((*hdr_metadata) >> 0) & vdp_bitshift(needbits);
			*addBytes = 1;
			*remained_bits = 8;
		} else {
			if (diff - (diff / 8 * 8) > 0) {
				*addBytes = diff / 8 + 1;
				while (i <= (*addBytes)) {
					retValue += (*(hdr_metadata + i)) << (8 * ((*addBytes) - i));
					i++;
				}
			*remained_bits = (*addBytes) * 8 - diff;
			retValue = (retValue >> (*remained_bits)) & vdp_bitshift(needbits);
			} else {
				(*addBytes) = diff / 8;
				while (i <= (*addBytes)) {
					retValue += (*(hdr_metadata + i)) << (8 * ((*addBytes) - i));
					i++;
				}

				retValue = (retValue >> 0) & vdp_bitshift(needbits);
				*remained_bits = 8;
				(*addBytes)++;
			}
		}
	}
	vdp_printf(VDP_HDR10PLUS_LOG,
		"[vdp] after update addbytes is %d value is 0x%x, remained_bits is %d\n",
		(*addBytes),
		retValue,
		*remained_bits);
	return retValue;
}

static uint8_t user_data_registered_inu_t_t35_new(uint8_t *hdr_metadata, uint32_t size, uint8_t *vsif,
	uint32_t ctl_info)
{
	uint8_t country_code = 0;
	uint8_t application_identifier = 0;
	uint8_t application_version = 0;
	uint8_t num_windows = 0;
	uint8_t num_idx = 0;
	uint8_t idx = 0;
	uint32_t sel_tsdmaxl = 0;
	uint8_t vsif_tsdmaxl = 0;
	uint32_t sei_avgmaxrgb = 0;
	uint8_t vsif_avgmaxrgb = 0;
	uint8_t num_sei_dmaxrgb = 0;
	uint32_t sei_dmaxrgb[16];/* distributtion */
	uint8_t sei_dmaxrgb_idx[16];/* distributtion */
	uint8_t vsif_dmaxrgb[16];
	uint8_t tone_mapping_flag = 0;
	uint32_t sei_kpx = 0;
	uint16_t vsif_kpx = 0;
	uint32_t sei_kpy = 0;
	uint16_t vsif_kpy = 0;
	uint8_t num_sei_bz_a = 0;
	uint32_t sei_bz_a[9];
	uint8_t vsif_bz_a[9];
	int32_t remained_bits;
	uint32_t addBytes;
	uint32_t value;
	uint8_t *metadata = NULL;
	uint8_t graphic_overlay_flag = 0;
	uint8_t vsif_timing_mode = 0;

	if (hdr_metadata == NULL || vsif == NULL)
		return 0;
	metadata = hdr_metadata;
	country_code = *metadata;
	/* skip country code, privider_code, provider_oriented_code(u(8)+u(16)+u(16)) */
	metadata += 5;
	application_identifier = *metadata;
	application_version = *(metadata + 1);
	metadata += 2;
	num_windows = (((*metadata) & 0xC0) >> 6);
	remained_bits = 6;

	/* 27 bit */
	sel_tsdmaxl = vdp_bitvalue_get(metadata, 27, &remained_bits, &addBytes);
	metadata += addBytes; /* 2bit */
	vdp_printf(VDP_HDR10PLUS_LOG,
		"[VDP]country_code:0x%x num_windows:%d sel_tsdmaxl:0x%x *metadata=0x%x\n",
		country_code,
		num_windows,
		sel_tsdmaxl,
		*metadata);
	remained_bits = 2; /* skip 1 bit for tsdaplflag */
	for (num_idx = 0; num_idx < num_windows; ++num_idx) {
		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]num_idx[%d] *(metadata)=0x%x remained_bits=%d\n",
			num_idx,
			*(metadata),
			remained_bits);

		/* skip maxsel */
		(void)vdp_bitvalue_get(metadata, 51, &remained_bits, &addBytes);
		metadata += addBytes;

		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]after checkmaxScl *(metadata)=0x%x remained_bits=%d\n",
			*(metadata),
			remained_bits);

		sei_avgmaxrgb = vdp_bitvalue_get(metadata, 17, &remained_bits, &addBytes);
		metadata += addBytes;
		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]sei_avgmaxrgb:%d, remained_bits=%d *metadata=0x%x, 0x%x, 0x%x\n",
			sei_avgmaxrgb,
			remained_bits,
			*metadata, *(metadata + 1),
			*(metadata + 2));

		num_sei_dmaxrgb = vdp_bitvalue_get(metadata, 4, &remained_bits, &addBytes);
		metadata += addBytes;
		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]num_sei_dmaxrgb:%d *(metadata)=0x%x remained_bits=%d\n",
			num_sei_dmaxrgb,
			*(metadata),
			remained_bits);
		memset(sei_dmaxrgb, 0, sizeof(sei_dmaxrgb));
		for (idx = 0; idx < num_sei_dmaxrgb; ++idx) {
			sei_dmaxrgb[idx] = vdp_bitvalue_get(metadata, 24, &remained_bits, &addBytes);
			metadata += addBytes;
			sei_dmaxrgb_idx[idx] = (sei_dmaxrgb[idx] & 0xFE0000) >> 17;
			sei_dmaxrgb[idx] = sei_dmaxrgb[idx] & 0x1FFFF;
			vdp_printf(VDP_HDR10PLUS_LOG,
				"[VDP]sei_dmaxrgb_idx:%d, value;%d (*metadata)=0x%x\n",
				sei_dmaxrgb_idx[idx],
				sei_dmaxrgb[idx],
				(*metadata));
		}

		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]after dmxrgb: remained_bits =%d, *metadata=0x%x\n",
			remained_bits,
			*metadata);

		/* 10bit */
		(void)vdp_bitvalue_get(metadata, 10, &remained_bits, &addBytes);
		metadata += addBytes;

		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]after bright_pixel: remained_bits =%d, *metadata=0x%x\n",
			remained_bits,
			*metadata);
	}

	vdp_printf(VDP_HDR10PLUS_LOG,
		"[VDP]before tone_mapping_flag: remained_bits =%d, *metadata=0x%x\n",
		remained_bits,
		*metadata);

	/* skip luminance flag 1bit */
	(void)vdp_bitvalue_get(metadata, 1, &remained_bits, &addBytes);
	metadata += addBytes;

	for (num_idx = 0; num_idx < num_windows; ++num_idx) {
			/*1bit */
		tone_mapping_flag = vdp_bitvalue_get(metadata, 1, &remained_bits, &addBytes);
		metadata += addBytes;
		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP][%d, %d]tone_mapping_flag:%d *metadata=0x%x remained_bits=%d\n",
			num_idx,
			num_windows,
			tone_mapping_flag,
			*metadata,
			remained_bits);
		memset(sei_bz_a, 0, sizeof(sei_bz_a));
		if (tone_mapping_flag) {
			value = vdp_bitvalue_get(metadata, 24, &remained_bits, &addBytes);
			metadata += addBytes;
			sei_kpx = (value & 0xFFF000) >> 12;
			sei_kpy = (value & 0xFFF);
			vdp_printf(VDP_HDR10PLUS_LOG,
				"[VDP]sei_kpx:%d sei_kpy:%d *metadata=0x%x remained_bits=%d\n",
				sei_kpx,
				sei_kpy,
				*metadata,
				remained_bits);
			num_sei_bz_a = vdp_bitvalue_get(metadata, 4, &remained_bits, &addBytes);
			metadata += addBytes;
			vdp_printf(VDP_HDR10PLUS_LOG,
				"[VDP]num_sei_bz_a:%d *metadata=0x%x remained_bits=%d\n",
				num_sei_bz_a,
				*metadata,
				remained_bits);

			for (idx = 0; idx < num_sei_bz_a; ++idx) {
				vdp_printf(VDP_HDR10PLUS_LOG,
					"[VDP]*metadata=0x%x remained_bits=%d\n",
					*metadata,
					remained_bits);
				sei_bz_a[idx] = vdp_bitvalue_get(metadata, 10, &remained_bits, &addBytes);
				metadata += addBytes;
				vdp_printf(VDP_HDR10PLUS_LOG,
					"[VDP]sei_bz_a[%d]=%d *metadata=0x%x remained_bits=%d\n",
					idx, sei_bz_a[idx],
					*metadata,
					remained_bits);
			}
		}
		vdp_bitvalue_get(metadata, 1, &remained_bits, &addBytes);
		metadata += addBytes;
	}
	vsif_tsdmaxl = (uint8_t)vsif_min(sel_tsdmaxl, 1, 1024, 32, 31);
	vsif_avgmaxrgb = (uint8_t)vsif_min(sei_avgmaxrgb, 10, 4096, 16, 255);
	vdp_printf(VDP_HDR10PLUS_LOG,
		"[VDP]vsif_tsdmaxl=%d vsif_avgmaxrgb=%d\n",
		vsif_tsdmaxl,
		vsif_avgmaxrgb);

	vsif[0] = (application_version << 6)|(vsif_tsdmaxl << 1);
	vsif[1] = vsif_avgmaxrgb;
	for (idx = 0; idx < 9; ++idx) {
		if ((idx != 2) || (application_version == 0)) {
			vsif_dmaxrgb[idx] = (uint8_t)vsif_min(sei_dmaxrgb[idx],
				10,
				4096,
				16,
				255);
			if ((idx == 8) && (application_version == 0) && (num_sei_dmaxrgb == 10))
				vsif_dmaxrgb[idx] = (uint8_t)vsif_min(sei_dmaxrgb[idx + 1],
					10,
					4096,
					16,
					255);
		} else
			vsif_dmaxrgb[idx] = (uint8_t)sei_dmaxrgb[idx];
		vsif[2 + idx] = vsif_dmaxrgb[idx];
	}
	vsif_kpx = vsif_min(sei_kpx, 1, 0xFFFF, 4, 1023);
	vsif_kpy = vsif_min(sei_kpy, 1, 0xFFFF, 4, 1023);
	vdp_printf(VDP_HDR10PLUS_LOG,
		"[VDP]sei_kpx=%d, vsif_kpx=%d, sei_kpy=%d, vsif_kpy=%d\n",
		sei_kpx,
		vsif_kpx,
		sei_kpy,
		vsif_kpy);
	vsif[11] = (num_sei_bz_a << 4) | ((vsif_kpx & 0x3C0) >> 6);
	vsif[12] = ((vsif_kpx & 0x3F) << 2) | ((vsif_kpy & 0x300) >> 8);
	vsif[13] = (vsif_kpy & 0xFF);
	for (idx = 0; idx < 9; ++idx) {
		vsif_bz_a[idx]	= (uint8_t)vsif_min(sei_bz_a[idx],
				1,
				0xFFFF,
				4,
				255);
		vsif[14+idx] = vsif_bz_a[idx];
		vdp_printf(VDP_HDR10PLUS_LOG,
			"[VDP]vsif_bz_a=%d, sei_bz_a=%d\n",
			sei_bz_a[idx],
			vsif_bz_a[idx]);
	}

	if (ctl_info & VDP_HDR10PLUS_GRAPHIC_OVERLAY_FLAG)
		graphic_overlay_flag = 1;
	if (ctl_info & VDP_HDR10PLUS_VISF_TIMING_MODE)
		vsif_timing_mode = 1;
	vsif[23] = (graphic_overlay_flag << 7) | (vsif_timing_mode << 6);
	return 1;
}


static void vdp_fill_metadata_info(VID_PLA_DR_TYPE_T type,
	VID_PLA_HDR_METADATA_INFO_T *rHdr,
	void *buf_add, unsigned int len)
{
	unsigned char vsif[32];
	unsigned int vsiflen = 0;
	uint32_t hdr10plusctlbit = 0;
	rHdr->fgIsMetadata = true;
	rHdr->e_DynamicRangeType = type;

	if (type == VID_PLA_DR_TYPE_HDR10_PLUS_VSIF) {
		memset(vsif, 0, sizeof(vsif));
		vsif[0] = 0x8B;
		vsif[1] = 0x84;
		vsif[2] = 0x90;

		/* add vsif control bit   */
		hdr10plusctlbit |= VDP_HDR10PLUS_VISF_TIMING_MODE;
		user_data_registered_inu_t_t35_new(buf_add, len, &(vsif[3]), hdr10plusctlbit);
		vsiflen = 27;
		memcpy(buf_add, vsif, vsiflen);

		rHdr->metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusAddr =
			(unsigned long)buf_add;
		rHdr->metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusSize =
			vsiflen;
	} else {
		rHdr->metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusAddr =
			(unsigned long)buf_add;
			rHdr->metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusSize =
			len;
	}
}

extern void mdp_print(char *pBuffer);


/*wake up condition :
*1. vsync active start , not including resolution change duration
*2. first frame inuput by HWC after resolution change
*/
static int vdp_routine(void *data)
{
	int ret = 0;
	struct video_buffer_info *buf = NULL;
	struct video_layer_info *layer_info;
	bool find_next = false;
	bool set_core2_pattern = false;
	unsigned int i;
	unsigned int duration;
	struct sync_fence *sync_fence = NULL;
	struct vdp_hal_config_info config_info;
	VID_PLA_HDR_METADATA_INFO_T rHdr = {0};
	bool is_Y_C_independent = false;
	bool not_add_timeline = false;
	int dsd_en = 0;
	int h_start, v_start_odd, v_start_even;
	uint32_t thread_id = *(uint32_t *)data;	/* layer id */
	uint32_t video_stream_number = 1;
	struct disp_hw *vdp_drv = disp_vdp_get_drv();
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	unsigned long timestap = 0;
	struct timespec ts;
	static unsigned int layer_show_count[2];
#endif
	static uint64_t pre_pts[2];
	static int display_vsync_count = 0;
	/*static bool sub_path_stop = false;*/   /*only for plan C2 pip, plan A no need */
	static bool main_path_config = false;

	while (1) {
		if (thread_id == 0) {
			ret = wait_event_interruptible(disp_vdp_wq0, gWakeupVdpSwThread0);
			gWakeupVdpSwThread0 = 0;
			display_vsync_count++;
		} else if (thread_id == 1) {
			ret = wait_event_interruptible(disp_vdp_wq1, gWakeupVdpSwThread1);
			gWakeupVdpSwThread1 = 0;
		} else {
			DISP_LOG_E("invalid thread id %d\n", thread_id);
			continue;
		}

		for (i = thread_id; i < (thread_id + video_stream_number); i++) {
			/*get buffer from input buffer list */
			layer_info = &video_layer[i];
			if (dolby_path_disable && !netflix_dolby_path_enable
				&& !dovi_idk_dump && (i == 0)) {
				dolby_path_disable_cnt++;
				if (dolby_path_disable_cnt == 3) {
					vdp_hdr_info[0].dr_range = DISP_DR_TYPE_PHLP_RESVERD;
					vdp_hdr_info[0].enable = 0;
					vdp_drv->drv_call(DISP_CMD_METADATA_UPDATE, &vdp_hdr_info[0]);
					dolby_path_disable = false;
					fmt_hal_mix_plane(FMT_HW_PLANE_3);
				}
			} else if (dolby_path_enable && netflix_dolby_path_enable && (i == 0)) {
				dovi_vs10_path_enable_cnt++;
				if ((g_force_dolby > 0) && (dovi_vs10_path_enable_cnt == 2)) {
					disp_dovi_isr();
					DISP_LOG_DEBUG("dobly path enable process isr!\n");
					g_force_dolby = 0;
				}
				if (dovi_vs10_path_enable_cnt == 3) {
					if (dolby_path_restart) {
						disp_dovi_process_core2(true);
						dovi_core2_set_black_pattern(true);
						dovi_core2_hal_isr();
						set_core2_pattern = true;
						DISP_LOG_DEBUG("enable core2 black pattern\n");
					} else {
						fmt_hal_mix_plane(FMT_HW_PLANE_3);
						DISP_LOG_DEBUG("switch adaptive by ui\n");
					}
					vdp_update_dovi_path_delay(false);
					DISP_LOG_DEBUG("dobly path enable mix graphic!\n");
				}

				if (set_core2_pattern && (dovi_vs10_path_enable_cnt == 5)) {
					dolby_path_restart = false;
					set_core2_pattern = false;
					disp_dovi_process_core2(true);
					dovi_core2_set_black_pattern(false);
					dovi_core2_hal_isr();
					fmt_hal_mix_plane(FMT_HW_PLANE_3);
					DISP_LOG_DEBUG("disable core2 black pattern\n");
				}
			}

			if (dovi_vs10_disable_delay && (i == 0)) { /* ?? disable dolby in vdp1 thread????*/
				vdp_dovi_path_disable();
				fmt_hal_mix_plane(FMT_HW_PLANE_3);
				dovi_vs10_disable_delay = false;
			}
			if (dovi_idk_dump && (i == 0)) {
				if (idk_dump_vsync_cnt >= 0)
					idk_dump_vsync_cnt++;

				if (idk_dump_vsync_cnt == 2) {
					if (!dovi_idk_dump_set_vin) {
						disp_dovi_idk_dump_vin(true,
							VIDEOIN_SRC_SEL_DOLBY3, VIDEOIN_FORMAT_444);
						dovi_idk_dump_set_vin = true;
					} else
						videoin_hal_enable(true);
				}
				if (idk_dump_vsync_cnt == 15)
					videoin_hal_enable(false);
				if (idk_dump_vsync_cnt == 20)
					disp_dovi_idk_dump_frame();
			}

			/* FRC control
			** whether the current display buffer display time is up
			*/
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
			if (vdp_cli_get()->enable_pts_debug)
				layer_show_count[i] ++;
#endif
			if (dovi_idk_dump)
				idk_dump_vsync_cnt = 0;

			mutex_lock(&(layer_info->sync_lock));
			if (!list_empty(&layer_info->buf_list)) {
				struct video_buffer_info *temp = NULL;

				/* buffer list is not empty, get one frame from buffer list */
				list_for_each_entry_safe(buf, temp, &layer_info->buf_list, list) {
					if (layer_info->state == VDP_LAYER_STOPPING) {
						vdp_disable_vdout_active_zone(layer_info);
						DISP_LOG_N("STOPPING: layer %d disable active zone\n", i);

						if (layer_info->layer_id == 0)
							vdp_disable_dolby_path();

						DISP_LOG_N("STOPPING: layer %d disable ufo and vdo\n", i);
						if (layer_info->secure_en)
							disp_vdp_sec_stop_hw(layer_info->layer_id);
						else {
							vdp_hal_set_enable(layer_info->layer_id, false);
							vdp_hal_isr((layer_info->layer_id == 0) ? true : false,
								(layer_info->layer_id == 1) ? true : false);
						}

						layer_info->state = VDP_LAYER_STOPPED;
						find_next = false;

						DISP_LOG_N("STOPPING:layer %d buf:%d tl_idx=%d, rel_idx=%d, tl_value=%d\n",
							i, buf->current_fence_index,
							layer_info->timeline_idx, layer_info->release_idx,
							layer_info->timeline->value);
						break;
					} else if (layer_info->state == VDP_LAYER_STOPPED) {
						vdp_release_buffer(layer_info->layer_id);

						layer_info->timeline_idx = buf->current_fence_index;
						find_next = false;

						DISP_LOG_N("STOPPED:layer %d buf:%d tl_idx=%d, rel_idx=%d, tl_value=%d\n",
							i, buf->current_fence_index,
							layer_info->timeline_idx, layer_info->release_idx,
							layer_info->timeline->value);
						break;
					} else if (buf->current_fence_index == (layer_info->timeline_idx + 1)) {
						find_next = true;
						vdp_state_switch(VDP_LAYER_START, VDP_LAYER_RUNNING, layer_info);
						break;
					} /*else if ((layer_info->layer_id == 0) && sub_path_stop) {
						find_next = true;
						DISP_LOG_N("layer %d sub path stop, need trigger main path again", i);
						break;
					} */else if (layer_info->res_change) {
						find_next = true;
						DISP_LOG_N("layer %d res change, need trigger again\n", i);
						break;
					} else if ((i == 1) && (dolby_path_enable == 0) && (hdr_enable) &&
						(buf->hdr10_type == HDR10_TYPE_NONE) && (!layer_info->sdr2hdr) &&
						disp_common_info.tv.is_support_hdr && main_path_config) {
						find_next = true;
						not_add_timeline = true;
						break;
					}
				}
			} else if ((layer_info->state == VDP_LAYER_STOPPED) ||
				(layer_info->state == VDP_LAYER_STOPPING)) {
				/* current buffer list is empty. */
				pre_pts[i] = 0;

				/*if ((layer_info->layer_id == 1) && dolby_path_enable) {
					DISP_LOG_N("layer %d stop dolby sub path, update dolby\n", i);
					sub_path_stop = true;
				}

				if (video_layer[0].state != VDP_LAYER_RUNNING)
					sub_path_stop = false;*/

				if (i == 0)
					main_path_config = false;

				vdp_disable_display_path(layer_info);
				DISP_LOG_N("layer %d no buffer now, clock off done\n", i);
			}

			mutex_unlock(&(layer_info->sync_lock));

			#if !(VIDEO_USE_HW_SHADOW && VIDEO_REDUCE_BUFFER)
			if (!find_next) {
				if (layer_info->state == VDP_LAYER_RUNNING) {
					mutex_lock(&(layer_info->sync_lock));
					vdp_release_buffer(i);
					mutex_unlock(&(layer_info->sync_lock));
				}
			}
			#endif

			if ((!find_next) || (buf == NULL))
				continue;

			if ((pre_pts[i] != buf->pts) &&
				(layer_info->display_duration >= layer_info->vsync_duration))
				continue;

			/* find a valid buffer */
			if ((buf->ion_fd & 0xFFFF000) > 0)
				is_Y_C_independent = true;
			if (buf->is_interlace)
				duration = INTERLACE_PLAYBACK_DUARATION;
			else
				duration = NORMAL_PLAYBACK_DURATION;

			#if !(VIDEO_USE_HW_SHADOW && VIDEO_REDUCE_BUFFER)
			if (layer_info->timeline_idx > layer_info->release_idx) {
				if ((layer_info->timeline_idx - layer_info->release_idx) > duration) {
					/*free the display */
					mutex_lock(&(layer_info->sync_lock));
					vdp_release_buffer(i);
					mutex_unlock(&(layer_info->sync_lock));
					if (layer_info->state == VDP_LAYER_STOPPED) {
						layer_info->timeline_idx++;
						DISP_LOG_DEBUG("releasing1...tl_idx=%d, rel_idx=%d, tl_value=%d\n",
							layer_info->timeline_idx,
							layer_info->release_idx,
							layer_info->timeline->value);
						continue;
					}
				} else if (layer_info->state == VDP_LAYER_STOPPED) {
					/*signal the fence */
					if (layer_info->timeline != NULL)
						timeline_inc((struct sw_sync_timeline *)layer_info->timeline, 1);
					layer_info->release_idx++;
					/*free ion handle */
					vdp_ion_free_handle(vdp_ion_client, buf->ion_hnd);
					if (is_Y_C_independent)
						vdp_ion_free_handle(vdp_ion_client, buf->ion_hnd2);

					mutex_lock(&layer_info->sync_lock);
					list_del_init(&buf->list);
					mutex_unlock(&layer_info->sync_lock);
					release_buf_info(&buf->list, i);
					DISP_LOG_DEBUG("releasing2...tl_idx=%d, rel_idx=%d, tl_value=%d\n",
								layer_info->timeline_idx,
							layer_info->release_idx, layer_info->timeline->value);
				}
			}
			#endif

			config_info.cur_fb_info.src_region.width = buf->crop.width;
			config_info.cur_fb_info.src_region.height = buf->crop.height;
			config_info.cur_fb_info.out_region.width = buf->tgt.width;
			config_info.cur_fb_info.out_region.height = buf->tgt.height;
			dsd_en = vdp_check_dsd_available(i, &config_info);
			if ((i == 0) && (!force_dolby_off)) {
				if ((buf->hdr10_type == HDR10_TYPE_HLG)
					|| ((buf->hdr10_type == HDR10_TYPE_PLUS)
					&& (disp_common_info.tv.is_support_hdr10_plus))) {
					if (netflix_dolby_path_enable || dolby_path_enable) {
						/* vdp_dovi_path_disable(); delay to next vsync*/
						disp_path_set_dolby(false);
						/*when vs10 is enable, for hlg source and tv is sdr, now hdr2sdr will set delay value firstly,*/
						/*but disable dolby will change delay value later,  so changed  the delay value back*/
						if ((buf->hdr10_type == HDR10_TYPE_HLG) && (!disp_common_info.tv.is_support_hdr)) {
							disp_path_set_delay_back_for_hdr2sdr();
						}
						fmt_hal_not_mix_plane(FMT_HW_PLANE_3);
						dovi_vs10_disable_delay = true;
						dolby_path_start = 0;
						hdr10_plus_enable = true;

						if (disp_vdp_get_main_sub_swap_status()) {
							uint32_t enable_vdo4 = 1;
							/* enable vdo4 output to pre-mix */
							disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &enable_vdo4);

							/* enable YUV444 */
							fmt_hal_set_output_444(DISP_FMT_SUB, true);
							disp_vdp_set_main_sub_swap(0);
						}
						continue;
					}
				} else if ((!disp_vdp_get_main_sub_swap_status())/* || sub_path_stop*/) {
					if (hdr10_plus_enable) {
						DISP_LOG_N("non-dolby change to dolby, need stop then restart\n");
						disp_vdp_get_drv()->stop(i);
						continue;
					}
					vdp_update_dovi_setting(i, buf, dsd_en);
				}
			}

			#if 0
			/* Dolby PIP case */
			/* Plan C: need to update buffer to dovi */
			if ((i == 1) && dolby_path_enable && (dovi_vs10_disable_delay == false)) {
				disp_vdp_set_main_sub_swap(1);
				vdp_update_dovi_setting(0, buf, dsd_en);
			}

			#endif

#if 1
			/* Dolby PIP case */
			/* Plan A: need to enable premix clock for dolby core2 */
			if ((i == 1) && dolby_path_enable && main_path_config) {
				if (disp_vdp_get_osd_premix() == false) {
					disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
					disp_path_set_delay(DISP_PATH_SVDO_DOLBY_CORE2, disp_common_info.resolution->res_mode);
					if (dolby_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
						if (dsd_en)
							disp_path_set_dsd_dolby_by_shift(true, -4, -4);
						else
							disp_path_set_dolby_by_shift(true, -4, -4);
					} else if (dolby_out_format == DOVI_FORMAT_DOVI) {
						if (dsd_en)
							disp_path_set_dsd_dolby_by_shift(true, 37, -4);
						else
							disp_path_set_dolby_by_shift(true, 37, -4);
					}
				}
				disp_vdp_set_osd_premix(true);
			} else if ((i == 1) && (!main_path_config)) {
				DISP_LOG_N("pip usecase, need first config main path, so continue\n");
				continue;
			}
#endif

			if ((vdp_hdr_info[0].dr_range == DISP_DR_TYPE_DOVI)
				&& (dovi_get_profile4() == true)) {
				/*wait for the acquired fence */
				layer_info->timeline_idx++;
				debug_frame_count[layer_info->layer_id]++;
				find_next = false;
				continue;
			}

			if ((i == 1) && (dolby_path_enable == 0) && (hdr_enable) &&
				(buf->hdr10_type == HDR10_TYPE_NONE) && (!layer_info->sdr2hdr) &&
				disp_common_info.tv.is_support_hdr && main_path_config) {
				layer_info->sdr2hdr = true;
				if (!disp_vdp_get_osd_premix()) {
					disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
					disp_vdp_set_osd_premix(true);
				}
				disp_path_set_delay(DISP_PATH_SVDO_SDR2HDR, disp_common_info.resolution->res_mode);
				not_add_timeline = false;
				find_next = false;
				DISP_LOG_N("sub path use osd_premix->sdr2hdr to show\n");
				continue;
			} else if ((i == 1) && (!main_path_config)) {
				DISP_LOG_N("pip usecase, need first config main path, so continue\n");
				continue;
			}

			/*wait for the buffer write operation done */
			if (buf->acquire_fence_fd != -1) {
				sync_fence = sync_fence_fdget(buf->acquire_fence_fd);
				if (sync_fence != NULL) {
				ret = sync_fence_wait(sync_fence, 100);
				if (ret < 0)
					DISP_LOG_E("wait fence error %d\n", ret);
				sync_fence_put(sync_fence);
				}
			}

			/*for secure buffer, should switch to TEE */
			#if VIDEO_DISPLAY_SECURE_ENABLE
			if (buf->secruity_en && !layer_info->secure_en) {
				DISP_LOG_N("switch layer[%d] from normal to secure map addr[0x%08x] size:[%d]\n",
					layer_info->layer_id,
					layer_info->src_phy_addr,
					layer_info->buffer_size);
				disp_vdp_sec_init(layer_info->layer_id,
					layer_info->src_phy_addr,
					layer_info->buffer_size);

				layer_info->secure_en = true;
			}

			if (!buf->secruity_en && layer_info->secure_en) {
				if (layer_info->secure2normal) {
					layer_info->secure_en = false;
					layer_info->secure2normal = false;
					disp_vdp_sec_deinit(layer_info->layer_id);
					DISP_LOG_N("switch from secure to normal port\n");
				} else {
					layer_info->secure2normal = true;
					DISP_LOG_N("first switch from secure to normal\n");
				}
			}
			#endif

			/*wait for the acquired fence */
			/*if ((layer_info->layer_id == 0) && sub_path_stop) {
				sub_path_stop = false;
				DISP_LOG_N("same buf, no need timeline_idx ++\n");
			} else */if (layer_info->res_change) {
				layer_info->res_change = false;
				DISP_LOG_N("layer %d res change, no need timeline_idx ++\n", i);
			} else if ((i == 1) && not_add_timeline) {
				not_add_timeline = false;
				DISP_LOG_N("trigger again for sub path to config\n");
			} else {
				layer_info->timeline_idx++;
				debug_frame_count[layer_info->layer_id]++;
			}

			if (layer_info->res_change)
				layer_info->res_change = false;

			layer_info->src_phy_addr = buf->src_phy_addr;
			layer_info->buffer_size = buf->buffer_size;

			/* config hw in TEE, use service call */

			/*
			*DISP_LOG_N("timeline_idx increase %d, buf fence_index=%d\n",
				layer_info->timeline_idx, buf->current_fence_index);
				*/
			/*set the buffer info to hal structure */
			memset(&config_info, 0, sizeof(struct vdp_hal_config_info));
			config_info.vdp_id = i;

			if (pre_pts[i] != buf->pts) {
				if (dovi_idk_dump)
					layer_info->display_duration += (layer_info->vsync_duration * dovi_idk_disp_cnt);
				else if (buf->source_duration)
					layer_info->display_duration += buf->source_duration;
				else
					layer_info->display_duration = 0;
			}

			/* get the active zone from display path */
			disp_path_get_active_zone(i, disp_common_info.resolution->res_mode,
				&h_start, &v_start_odd, &v_start_even);

			/* set the active zone information to vdp hal */
			vdp_hal_config_timing(i, disp_common_info.resolution,
				h_start, v_start_odd, v_start_even);

			/* fill vdp_hal_config_info according to video_buffer_info */
			vdp_set_hal_config(buf, NULL, NULL, &config_info);
#if 1
			/*get the buffer information & set to vdp/fmt sw shadow */
			/* set dispfmt & vdout active zoon and enable vdout. */
			vdp_update_fmt_setting(i, &config_info, buf->hdr10_type);
#endif
			#if VIDEO_DISPLAY_SECURE_ENABLE
			if (layer_info->secure_en) {
				struct dispfmt_setting dispfmt_info = {0};
				uint64_t *reg_mode = NULL;

				dispfmt_info.dispfmt_register_setting = (uint64_t)(uintptr_t)fmt_hal_get_sw_register(
					layer_info->layer_id,
					&dispfmt_info.dispfmt_register_setting_size,
					&reg_mode);

				dispfmt_info.dispfmt_register_setting_mask = *reg_mode;

				WARN_ON(dispfmt_info.dispfmt_register_setting == 0);
				/* default set VDP_HDR10PLUS_GRAPHIC_OVERLAY_FLAG false */
				disp_vdp_sec_config(&config_info, &dispfmt_info, VDP_HDR10PLUS_VISF_TIMING_MODE);
				/* clear dispfmt register mask */
				*reg_mode = 0;
			} else
			#endif
				vdp_hal_config(i, &config_info);

			if ((dsd_en == 1)) {
				/*Update vdo setting && disp_fmt */
				#if VIDEO_DISPLAY_SECURE_ENABLE
				if (!layer_info->secure_en)
				#endif
					vdp_hal_set_dsd_config(i,
						config_info.cur_fb_info.src_region.height,
						config_info.cur_fb_info.out_region.height,
						disp_common_info.resolution->height);

				layer_info->dsd_en = true;
			} else
				layer_info->dsd_en = false;


			#if VIDEO_USE_HW_SHADOW
			if (i == 0)
				vdp_hal_isr(!layer_info->secure_en, false);
			else
				vdp_hal_isr(false, !layer_info->secure_en);
			#endif

#if 1
			/* Dolby PIP case */
			/* Plan A: need to enable premix clock for dolby core2 */
			if ((i == 1) && dolby_path_enable && (!disp_vdp_get_osd_premix())) {
				disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
				disp_vdp_set_osd_premix(true);
				DISP_LOG_N("dolby pip, need enable osd premix\n");
			}
#endif

			if ((i == 0) && (dolby_path_enable == 0)) {
				/* hdr10 plus metadata need to copy every frame. */
				if ((buf->hdr10_type == HDR10_TYPE_PLUS) &&
					(disp_common_info.tv.is_support_hdr10_plus) &&
					(!buf->hdr_info.metadata_info.dovi_metadata.svp)) {

					if (disp_common_info.tv.hdr10_plus_app_ver != 0xFF) {
						vdp_fill_metadata_info(VID_PLA_DR_TYPE_HDR10_PLUS_VSIF,
							&rHdr,
							buf->hdr_info.metadata_info.dovi_metadata.buff,
							buf->hdr_info.metadata_info.dovi_metadata.len);
					} else {
						vdp_fill_metadata_info(VID_PLA_DR_TYPE_HDR10_PLUS,
							&rHdr,
							buf->hdr_info.metadata_info.dovi_metadata.buff,
							buf->hdr_info.metadata_info.dovi_metadata.len);
					}
					#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
					vVdpSetHdrMetadata(true, rHdr);
					#endif
				}
				/* need to set HDR10 signal */
				if (buf->hdr10_type != hdr_enable) {
					/* disable former tv signal */
					vdp_config_hdmi_signal(hdr_enable, NULL, false);

					/* enable current tv signal */
					vdp_config_hdmi_signal(buf->hdr10_type, &buf->hdr10_info, true);

					/*judge if the TV is playing hdr source */
					vdp_drv->drv_call(DISP_CMD_PLAY_HDR_SOURCE,
						&(buf->hdr10_type));

					hdr_enable = buf->hdr10_type;
				}
				vdp_set_HDMI_BT2020_signal(buf->is_bt2020);
			}

			if (i == 0)
				main_path_config = true;

			#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
			if (vdp_cli_get()->enable_pts_debug) {
				getrawmonotonic(&ts);
				timestap = (unsigned long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

				DISP_LOG_E("routine: layer %d, pre_pts %lld, count %d, duration %d, pts %lld, timer %ld\n",
					i, pre_pts[i], layer_show_count[i], layer_info->display_duration, buf->pts, timestap);

				layer_show_count[i] = 0;
			}
			#endif
			pre_pts[i] = buf->pts;

			buf->time_start_display = sched_clock();
			find_next = false;
		}
	}
	return VDP_OK;
}
