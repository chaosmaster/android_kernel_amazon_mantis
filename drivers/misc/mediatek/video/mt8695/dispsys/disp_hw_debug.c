/*
* disp_hw_debug.c - display hardware manager
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

#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <kree/mem.h>
#include <kree/system.h>
#include <tz_cross/ta_mem.h>
#include <tz_cross/trustzone.h>
#include <linux/slab.h>
#define LOG_TAG "DEBUG"
#include "disp_hw_log.h"
#include "disp_hw_debug.h"
#include "disp_hw_mgr.h"
#include "disp_clk.h"
#include "ion_drv.h"
#include "mtk_ion.h"
#include "disp_sys_hal.h"
#include "fmt_hal.h"
/* #define DISP_HWMGR_SUPPORT_ION */

static int debug_init;

static struct dentry *debugfs;
static struct ion_client *client;
static struct ion_handle *handle;
static void *ion_va;
static int ion_fd = -1;

static char dbg_buf[2048];
static char STR_HELP[] =
	"USAGE:\n"
	"       echo [ACTION]>/d/hwmgr\n"
	"ACTION:\n"
	"      trigger_ui:1 > /d/hwmgr     1:support ion\n"
	"      trigger_vdo:1 > /d/hwmgr    1: support ion\n"
	"      sram_pd:1 > /d/hwmgr\n"
	"              0: open all sram power\n"
	"              1: power down all sram power\n"
	"              2: home screen scenario, keep osd_fhd/hdmi power\n"
	"              3: home screen scenario - interlace, keep osd_fhd/hdmi/p2i power\n"
	"              4: uhd mvdo scenario - keep osd_fhd/hdmi/vdo3 power\n"
	"              5: uhd mvdo(no ui) scenario - keep hdmi/vdo3 power\n"
	"              6: uhd dolby scenario - keep osd_fhd/hdmi/vdo3/dolby power\n"
	"              7: uhd dolby scenario(no ui) - keep hdmi/vdo3/dolby power\n";

static int disp_hw_get_ion_buffer(void)
{
	if (!client)
		client = ion_client_create(g_ion_device, "hwmgr_debug");

	if (ion_fd != -1)
		return 0;

	handle = ion_alloc(client, (1920 * 1080 * 5), 0, ION_HEAP_MULTIMEDIA_MASK, 0);
	if (IS_ERR(handle)) {
		DISP_LOG_I("allocate ion handle fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	ion_va = ion_map_kernel(client, handle);
	if (ion_va == NULL) {
		DISP_LOG_I("map va fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}
	ion_fd = ion_share_dma_buf_fd(client, handle);

	return 0;
}

static int disp_hw_debug_trigger_ui(bool support_ion)
{
	int i = 0;
	int j = 0;
	unsigned int *fill_point = NULL;
	unsigned long paStart = 0;
	void *vaStart = NULL;
	int ret = 0;
	int fd = -1;
	struct mtk_disp_config config = {0};
	struct disp_hw_common_info hw_info;

	DISP_LOG_I("disp_hw_debug_trigger_ui be call.\n");

	ret = disp_hw_mgr_get_info(&hw_info);
	if (ret != 0)
		return -1;

	if (support_ion) {
		paStart = 0;
		if (ion_va == NULL || ion_fd == -1)
			disp_hw_get_ion_buffer();

		vaStart = ion_va;
		fd = ion_fd;
	} else {
		paStart = fb_bus_addr;
		vaStart = fb_va_base;
	}

	DISP_LOG_I("fd= %d, paStart= 0x%x, vaStart= %p\n", fd, (unsigned int)paStart, vaStart);
	fill_point = (unsigned int *)vaStart;

	/*fill a color bar to ui buffer*/
	for (i = 0; i < 1080; i++) {
		if (i < 270) {
			for (j = 0; j < 1920; j++) {
				*fill_point = 0xFFFF0000;
				fill_point++;
			}
		} else if (i >= 270 && i < 540) {
			for (j = 0; j < 1920; j++) {
				*fill_point = 0xFF00FF00;
				fill_point++;
			}
		} else if (i >= 540 && i < 810) {
			for (j = 0; j < 1920; j++) {
				*fill_point = 0xFF0000FF;
				fill_point++;
			}
		} else if (i >= 810) {
			for (j = 0; j < 1920; j++) {
				*fill_point = 0xFFFF00FF;
				fill_point++;
			}
		}
	}

	config.buffer_num = 1;
	config.buffer_info[0].layer_id = 0;
	config.buffer_info[0].layer_enable = 1;
	config.buffer_info[0].src_base_addr = vaStart;
	config.buffer_info[0].src_phy_addr = (void *)paStart;
	config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_BGRA8888;
	config.buffer_info[0].type = DISP_LAYER_OSD;
	config.buffer_info[0].src.x = 0;
	config.buffer_info[0].src.y = 0;
	config.buffer_info[0].src.width = 1920;
	config.buffer_info[0].src.height = 1080;
	config.buffer_info[0].tgt.x = 0;
	config.buffer_info[0].tgt.y = 0;
	config.buffer_info[0].tgt.width = hw_info.resolution->width;
	config.buffer_info[0].tgt.height = hw_info.resolution->height;
	config.buffer_info[0].alpha_en = 0;
	config.buffer_info[0].ion_fd = fd;
	config.buffer_info[0].acquire_fence_fd = -1;

	ret = disp_hw_mgr_config(&config);

	return ret;
}

static int disp_hw_debug_trigger_vdo(unsigned int support_ion)
{
	unsigned long paStart = 0;
	void *vaStart = NULL;
	int ret = 0;
	int fd = -1;
	struct mtk_disp_config config = {0};
	struct disp_hw_common_info hw_info;

	DISP_LOG_I("disp_hw_debug_trigger_vdo be call support_ion %d.\n", support_ion);

	ret = disp_hw_mgr_get_info(&hw_info);
	if (ret != 0)
		return -1;

	if (support_ion) {
		paStart = 0;
		if (ion_va == NULL || ion_fd == -1)
			disp_hw_get_ion_buffer();

		vaStart = ion_va;
		fd = ion_fd;
	} else {
		paStart = fb_bus_addr;
		vaStart = fb_va_base;
	}

	DISP_LOG_I("fd= %d, paStart= 0x%x, vaStart= %p\n", fd, (unsigned int)paStart, vaStart);

	config.buffer_num = 1;
	config.buffer_info[0].layer_id = 0;
	config.buffer_info[0].layer_enable = 1;
	config.buffer_info[0].src_base_addr = vaStart;
	config.buffer_info[0].src_phy_addr = (void *)paStart;
	config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_YUV420_BLOCK;
	config.buffer_info[0].type = DISP_LAYER_VDP;
	config.buffer_info[0].is_progressive = 1;
	if (support_ion == 2) {
		config.buffer_info[0].is_10bit = 1;
		DISP_LOG_I("is_10bit = %d\n", config.buffer_info[0].is_10bit);
	}

	if (support_ion == 3) {
		config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_YUV420_RASTER;
		config.buffer_info[0].dr_range = DISP_DR_TYPE_DOVI;
		DISP_LOG_I("dr_range = %d\n",
		config.buffer_info[0].dr_range);
	}

	config.buffer_info[0].src.x = 0;
	config.buffer_info[0].src.y = 0;
	config.buffer_info[0].src.width = hw_info.resolution->width;
	config.buffer_info[0].src.height = hw_info.resolution->height;
	config.buffer_info[0].src.pitch = hw_info.resolution->width;
	config.buffer_info[0].tgt.x = 0;
	config.buffer_info[0].tgt.y = 0;
	config.buffer_info[0].tgt.width = hw_info.resolution->width;
	config.buffer_info[0].tgt.height = hw_info.resolution->height;
	config.buffer_info[0].ofst_c = 0x200000;
	config.buffer_info[0].alpha_en = 0;
	config.buffer_info[0].ion_fd = fd;
	config.buffer_info[0].acquire_fence_fd = -1;

	ret = disp_hw_mgr_config(&config);

	return ret;
}

static int disp_hw_debug_trigger_pip(bool support_ion)
{
	unsigned long paStart = 0;
	void *vaStart = NULL;
	int ret = 0;
	int fd = -1;
	int i = 0;
	struct mtk_disp_config config = {0};
	struct disp_hw_common_info hw_info;

	DISP_LOG_I("disp_hw_debug_trigger_vdo be call.\n");

	ret = disp_hw_mgr_get_info(&hw_info);
	if (ret != 0)
		return -1;

	if (support_ion) {
		paStart = 0;
		if (ion_va == NULL || ion_fd == -1)
			disp_hw_get_ion_buffer();

		vaStart = ion_va;
		fd = ion_fd;
	} else {
		paStart = fb_bus_addr;
		vaStart = fb_va_base;
	}

	DISP_LOG_I("fd= %d, paStart= 0x%x, vaStart= %p\n", fd, (unsigned int)paStart, vaStart);

	config.buffer_num = 2;
	for (i = 0; i < config.buffer_num; i++) {
		config.buffer_info[i].layer_id = i;
		config.buffer_info[i].layer_enable = 1;
		config.buffer_info[i].src_base_addr = vaStart;
		config.buffer_info[i].src_phy_addr = (void *)paStart;
		config.buffer_info[i].src_fmt = DISP_HW_COLOR_FORMAT_YUV420_BLOCK;
		config.buffer_info[i].type = DISP_LAYER_VDP;
		config.buffer_info[i].is_progressive = 1;
		config.buffer_info[i].src.x = 0;
		config.buffer_info[i].src.y = 0;
		config.buffer_info[i].src.width = 1920;
		config.buffer_info[i].src.height = 1080;
		config.buffer_info[i].src.pitch = 1920;
		config.buffer_info[i].tgt.x = 0;
		config.buffer_info[i].tgt.y = 0;
		config.buffer_info[i].tgt.width = hw_info.resolution->width;
		config.buffer_info[i].tgt.height = hw_info.resolution->height;
		config.buffer_info[i].ofst_c = 0x200000;
		config.buffer_info[i].alpha_en = 0;
		config.buffer_info[i].ion_fd = fd;
		config.buffer_info[i].acquire_fence_fd = -1;
	}

	ret = disp_hw_mgr_config(&config);

	return ret;
}
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
static KREE_SESSION_HANDLE vdp_test_session;
#endif

static int disp_hw_debug_trigger_secure_vdo(unsigned int vdp_id)
{
	int ret = 0;
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	struct mtk_disp_config config = {0};
	struct disp_hw_common_info hw_info;
	KREE_SECUREMEM_HANDLE sec_handle;

	ret = disp_hw_mgr_get_info(&hw_info);
	if (ret != 0)
		return -1;
	DISP_LOG_E("disp_hw_debug_trigger_secure_vdo start\n");
	if (vdp_test_session == 0) {
		ret = KREE_CreateSession(TZ_TA_MEM_UUID, &vdp_test_session);
		if (ret != TZ_RESULT_SUCCESS) {
			DISP_LOG_E("create vdp_test_session fail:%d\n", ret);
			return -1;
		}
	}
	KREE_AllocSecurechunkmem(vdp_test_session, &sec_handle, 0x400, 0x400000);
	DISP_LOG_I("buffer handle = 0x%x\n", sec_handle);
	config.buffer_num = 1;
	config.buffer_info[0].layer_id = vdp_id;
	config.buffer_info[0].layer_enable = 1;
	config.buffer_info[0].src_base_addr = 0;
	config.buffer_info[0].src_phy_addr = 0;
	config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_YUV420_BLOCK;
	config.buffer_info[0].type = DISP_LAYER_VDP;
	config.buffer_info[0].is_progressive = 1;
	config.buffer_info[0].src.x = 0;
	config.buffer_info[0].src.y = 0;
	config.buffer_info[0].src.width = hw_info.resolution->width;
	config.buffer_info[0].src.height = hw_info.resolution->height;
	config.buffer_info[0].src.pitch = hw_info.resolution->width;
	config.buffer_info[0].tgt.x = 0;
	config.buffer_info[0].tgt.y = 0;
	config.buffer_info[0].tgt.width = hw_info.resolution->width;
	config.buffer_info[0].tgt.height = hw_info.resolution->height;
	config.buffer_info[0].ofst_c = 0x200000;
	config.buffer_info[0].alpha_en = 0;
	config.buffer_info[0].ion_fd = (int)sec_handle;
	config.buffer_info[0].acquire_fence_fd = -1;
	config.buffer_info[0].secruity_en = true;
	config.buffer_info[0].buffer_size = 0x400000;
	ret = disp_hw_mgr_config(&config);
	DISP_LOG_E("disp_hw_debug_trigger_secure_vdo end\n");
#endif
	return ret;
}
static int disp_hw_debug_sram_pd(int scenario)
{
	DISP_LOG_I("sram pd scenario=%d\n", scenario);

	if (scenario == 0) {
		DISP_LOG_I("open all sram power\n");
		disp_sys_sram_pd_all(false, 0);
	} else if (scenario == 1) {
		DISP_LOG_I("power down all sram power\n");
		disp_sys_sram_pd_all(true, 0);
	} else if (scenario == 2) {
		DISP_LOG_I("home screen scenario, keep osd_fhd/hdmi power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_OSD_FHD | DISP_SYS_SRAM_HDMI);
	} else if (scenario == 3) {
		DISP_LOG_I("home screen scenario - interlace, keep osd_fhd/hdmi/p2i power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_OSD_FHD | DISP_SYS_SRAM_HDMI
			| DISP_SYS_SRAM_P2I);
	} else if (scenario == 4) {
		DISP_LOG_I("uhd mvdo scenario - keep osd_fhd/hdmi/vdo3 power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_OSD_FHD | DISP_SYS_SRAM_HDMI
			| DISP_SYS_SRAM_MVDO_0 | DISP_SYS_SRAM_MVDO_1
			| DISP_SYS_SRAM_MVDO_2 | DISP_SYS_SRAM_MVDO_3);
	} else if (scenario == 5) {
		DISP_LOG_I("uhd mvdo(no ui) scenario - keep hdmi/vdo3 power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_HDMI
			| DISP_SYS_SRAM_MVDO_0 | DISP_SYS_SRAM_MVDO_1
			| DISP_SYS_SRAM_MVDO_2 | DISP_SYS_SRAM_MVDO_3);
	} else if (scenario == 6) {
		DISP_LOG_I("uhd dolby scenario - keep osd_fhd/hdmi/vdo3/dolby power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_OSD_FHD | DISP_SYS_SRAM_HDMI
			| DISP_SYS_SRAM_MVDO_0 | DISP_SYS_SRAM_MVDO_1
			| DISP_SYS_SRAM_MVDO_2 | DISP_SYS_SRAM_MVDO_3 | DISP_SYS_SRAM_DOLBY);
	} else if (scenario == 7) {
		DISP_LOG_I("uhd dolby scenario(no ui) - keep hdmi/vdo3/dolby power\n");
		disp_sys_sram_pd_all(true, DISP_SYS_SRAM_HDMI
			| DISP_SYS_SRAM_MVDO_0 | DISP_SYS_SRAM_MVDO_1 | DISP_SYS_SRAM_DOLBY);
	}

	return 0;
}

static uint64_t _get_current_time_us(void)
{
	struct timeval t;

	do_gettimeofday(&t);
	return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

static int disp_hw_calc_vsync(int vsync_cnt)
{
	int i = 0;
	unsigned long start, end;
	int time;

	start = _get_current_time_us();
	while (i < vsync_cnt) {
		disp_hw_mgr_wait_vsync(NULL);
		i++;
	}
	end = _get_current_time_us();
	time =  (int)(end - start);
	DISP_LOG_I("vsync cnt=%d, total time=%d (ms), vysnc time=%d (us).\n",
		vsync_cnt, (time / 1000), (time / vsync_cnt));

	return (time / vsync_cnt);
}

static void disp_hw_process_dbg_opt(const char *opt)
{
	int ret = 0;
	char *p;

	if (strncmp(opt, "log_level:", 10) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 10;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (value == 1)
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG1);
		else if (value == 2)
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG2);
		else if (value == 3)
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG3);
		else if (value == 4)
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG4);
		else if (value == 5)
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG5);
		else
			disp_log_set_level(DISP_LOG_LEVEL_DEBUG2);

	} else if (strncmp(opt, "debug:", 6) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 6;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (value == 1)
			DISP_LOG_I("test debug cmd pass.\n");
	} else if (strncmp(opt, "change_res:", 11) == 0) {
		HDMI_VIDEO_RESOLUTION res_mode = HDMI_VIDEO_RESOLUTION_NUM;

		p = (char *)opt + 11;
		ret = kstrtoul(p, 10, (unsigned long int *)&res_mode);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (res_mode != HDMI_VIDEO_RESOLUTION_NUM)
			disp_hw_mgr_send_event(DISP_EVENT_CHANGE_RES, (void *)&res_mode);
	} else if (strncmp(opt, "dump_clk:", 9) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_clock_dump(value);
	} else if (strncmp(opt, "en_vdo_timelog:", 15) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 15;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);

		if (value == 0) {
			print_video_fence_history = false;
			DISP_LOG_E("disable video timer log\n");
		} else {
			print_video_fence_history = true;
			DISP_LOG_E("enable video timer log\n");
		}
	} else if (strncmp(opt, "trigger_ui:", 11) == 0) {
		unsigned int support_ion = 0;

		p = (char *)opt + 11;
		ret = kstrtoul(p, 10, (unsigned long int *)&support_ion);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		disp_hw_debug_trigger_ui(support_ion);
	} else if (strncmp(opt, "trigger_vdo:", 12) == 0) {
		unsigned int support_ion = 0;

		p = (char *)opt + 12;
		ret = kstrtoul(p, 10, (unsigned long int *)&support_ion);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_hw_debug_trigger_vdo(support_ion);
	} else if (strncmp(opt, "trigger_pip:", 12) == 0) {
		unsigned int support_ion = 0;

		p = (char *)opt + 12;
		ret = kstrtoul(p, 10, (unsigned long int *)&support_ion);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_hw_debug_trigger_pip(support_ion);
	} else if (strncmp(opt, "calc_vsync:", 11) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 12;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_hw_calc_vsync(value);
	} else if (strncmp(opt, "get_fps", 7) == 0) {
		int fps = 0;
		int vsync = disp_hw_calc_vsync(600);

		fps = 1 * 1000 * 1000 * 100 / vsync;
		DISP_LOG_I("fps=%d.%d\n", (fps / 100), (fps % 100));
	} else if (strncmp(opt, "suspend", 7) == 0) {
		disp_hw_mgr_suspend();
	} else if (strncmp(opt, "resume", 6) == 0) {
		disp_hw_mgr_resume();
	} else if (strncmp(opt, "status", 6) == 0) {
		DISP_LOG_I("printf hw mgr status\n");
		disp_hw_mgr_status();
	} else if (strncmp(opt, "dump_layer", 10) == 0) {
		int enable;
		int sample_x;
		int sample_y;

		ret = sscanf(opt, "dump_layer %d %d %d\n", &enable, &sample_x, &sample_y);
		if (ret != 3) {
			DISP_LOG_I("dump_layer ret=%d\n", ret);
			goto Error;
		}

		dump_enable = enable;
		down_sample_x = sample_x;
		down_sample_y = sample_y;

	} else if (strncmp(opt, "sram_pd:", 8) == 0) {
		unsigned int scenario = 0;

		p = (char *)opt + 8;
		ret = kstrtoul(p, 10, (unsigned long int *)&scenario);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_hw_debug_sram_pd(scenario);
	} else if (strncmp(opt, "-h", 2) == 0) {
		DISP_LOG_I("%s\n", STR_HELP);
	} else if (strncmp(opt, "trigger_sec_vdo:", 16) == 0) {
		unsigned int vdp_id = 0;

		p = (char *)opt + 16;
		ret = kstrtoul(p, 10, (unsigned long int *)&vdp_id);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_hw_debug_trigger_secure_vdo(vdp_id);
	} else if (strncmp(opt, "colorbar:", 9) == 0) {
		unsigned int en = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 10, (unsigned long int *)&en);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		fmt_hal_show_colorbar(en);
	} else if (strncmp(opt, "reset_fmt:", 10) == 0) {
		unsigned int fmt_id = 0;

		p = (char *)opt + 10;
		ret = kstrtoul(p, 10, (unsigned long int *)&fmt_id);
		if (ret) {
			DISP_LOG_E("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		fmt_hal_reset(fmt_id);
	}
	return;

Error:
	DISP_LOG_E("parse command error!\n%s", STR_HELP);
}

static void disp_hw_process_dbg_cmd(char *cmd)
{
	DISP_LOG_I("cmd: %s\n", cmd);
	memset(dbg_buf, 0, sizeof(dbg_buf));
	disp_hw_process_dbg_opt(cmd);
}

/* --------------------------------------------------------------------------- */
/* Debug FileSystem Routines */
/* --------------------------------------------------------------------------- */

static int disp_hw_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static char cmd_buf[512];

static ssize_t disp_hw_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(dbg_buf))
		return simple_read_from_buffer(ubuf, count, ppos, dbg_buf, strlen(dbg_buf));
	else
		return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));

}

static ssize_t disp_hw_debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&cmd_buf, ubuf, count))
		return -EFAULT;

	cmd_buf[count] = 0;

	disp_hw_process_dbg_cmd(cmd_buf);

	return ret;
}

static const struct file_operations debug_fops = {
	.read = disp_hw_debug_read,
	.write = disp_hw_debug_write,
	.open = disp_hw_debug_open,
};

void disp_hw_debug_init(void)
{
	if (!debug_init) {
		debug_init = 1;
		debugfs = debugfs_create_file("hwmgr",
					      S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);

		DISP_LOG_D("disp hw debug init, fs= %p\n", debugfs);

	}
}

void disp_hw_debug_deinit(void)
{
	debugfs_remove(debugfs);
}

