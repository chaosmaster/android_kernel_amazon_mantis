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



#define LOG_TAG "DOVI_MAIN"

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "disp_info.h"
#include "disp_hw_mgr.h"
#include "disp_clk.h"
#include "dovi_type.h"
#include "dovi_log.h"

#include "disp_dovi_main.h"

#include "dovi_core1_hal.h"
#include "dovi_core2_hal.h"
#include "dovi_core3_hal.h"
#include "dovi_common_hal.h"

#include "disp_dovi_cmd.h"
#include "disp_dovi_core1_if.h"
#include "disp_dovi_core2_if.h"
#include "disp_dovi_core3_if.h"

#include "disp_dovi_io.h"

#include "dovi_table.h"

#include "fmt_hal.h"
#include "disp_dovi_common.h"
#include "disp_dovi_common_if.h"

#include "vdout_sys_hal.h"
#include "disp_path.h"

#include "disp_irq.h"
#include "disp_vdp_if.h"
#include "disp_vdp_vsync.h"
#include "disp_hw_log.h"

struct disp_hw_common_info dovi_disp_common_info;
struct disp_hw_resolution dovi_res = {
	.res_mode = 0xff,
};

char dovi_dts_nd_name[DOVI_DTS_MAX_ND_SIZE] = "mediatek,mt8695-dovi_core";

char *dovi_reg_base[DOVI_CORE_MAX];


struct device *dovi_dev;

static struct task_struct *disp_dovi_thread;
static wait_queue_head_t disp_dovi_wq;
static unsigned int disp_dovi_wakeup_thread;
static struct mutex disp_dovi_mutex;

#define DOVI_PATH_TEST_MASK 0x100

static bool dovi_init_done;
static struct task_struct *disp_dovi_vsync_thread;
static wait_queue_head_t disp_dovi_vsync_wq;
static unsigned int disp_dovi_wakeup_vsync_thread;
static struct mutex disp_dovi_vsync_mutex;
uint32_t disp_dovi_event;
unsigned long disp_dovi_vsync_cnt;
#define DOVI_EVENT_UPDATE_ISR (0x1 << 0)
#define DOVI_EVENT_DISABLE_CLK (0x1 << 1)

uint32_t dovi_option;
uint32_t osd_enable;
uint32_t dolby_core2_enable;
bool dovi_mute;
struct dovi_info_t dovi_info;
bool priority_mode_change;
uint32_t ui_force_hdr_type;

uint32_t dovi_idk_test;
uint32_t dovi_idk_file_id;
int ll_rgb_desired;
bool dovi_logo_show;
uint32_t f_graphic_on;
char *uhd_fmt_reg_base;
char *uhd_pla_reg_base;
char *uhd_scl_reg_base;
char *fhd_fmt_reg_base;
char *fhd_pla_reg_base;
char *fhd_scl_reg_base;
char *osd_premix_reg_base;
uint32_t uhd_active_zone;
uint32_t fhd_active_zone;
bool fhd_scale_to_uhd;
int ll_format;
uint32_t enable;

uint32_t dovi_sdk_test;
uint32_t dovi_sdk_file_id;

static int _dovi_parse_dev_node(void)
{
	struct device_node *np;
	unsigned int reg_value;
	char nd_name[DOVI_DTS_MAX_ND_SIZE];

	sprintf(nd_name, "%s", dovi_dts_nd_name);

	np = of_find_compatible_node(NULL, NULL, nd_name);
	if (np == NULL) {
		dovi_error("dts error, no vdo device node %s.\n", nd_name);
		return DOVI_STATUS_ERROR;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);

	/* get core1 reg base 0x14009000 */
	dovi_reg_base[DOVI_CORE1] = (char *) of_iomap(np, 0);
	/* get core2 reg base 0x1400a000 */
	dovi_reg_base[DOVI_CORE2] = (char *) of_iomap(np, 1);
	/* get core3 reg base 0x1400b000 */
	dovi_reg_base[DOVI_CORE3] = (char *) of_iomap(np, 2);

    /* for idk cert */
	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-osd");
	if (np == NULL) {
		dovi_error("dts error, no osd device node.\n");
		return DOVI_STATUS_ERROR;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);
	uhd_fmt_reg_base = (char *) of_iomap(np, 0);	/*osd2: uhd 0x14004300*/
	uhd_pla_reg_base = uhd_fmt_reg_base + 0x100;
	uhd_scl_reg_base = uhd_fmt_reg_base + 0x200;

	fhd_fmt_reg_base = (char *) of_iomap(np, 1);	/*osd3: 0x14003000 */
	fhd_pla_reg_base = fhd_fmt_reg_base + 0x100;
	fhd_scl_reg_base = fhd_fmt_reg_base + 0x200;
	osd_premix_reg_base = (char *) of_iomap(np, 2);	/*0x14001c00 */

	return DOVI_STATUS_OK;
}

int disp_dovi_resolution_change(const struct disp_hw_resolution *info)
{
	/* dovi_func(); */
	if (dovi_res.res_mode != info->res_mode)
		dovi_res = *info;
	dovi_set_out_res(info->res_mode, info->width, info->height);
	return DOVI_STATUS_OK;
}

int disp_dovi_init(struct disp_hw_common_info *info)
{
	/* dovi_func_start(); */
	dovi_printf("%s enable dolby clock\n", __func__);
	if (g_force_dolby) {
		disp_clock_enable(DISP_CLK_DOLBY1, true);
		disp_clock_enable(DISP_CLK_DOLBY2, true);
		disp_clock_enable(DISP_CLK_DOLBY3, true);
		disp_clock_enable(DISP_CLK_DOLBY_MIX, true);
	}
	dovi_dev = disp_hw_mgr_get_dev();

	disp_dovi_resolution_change(info->resolution);

	/*parser dts file for register base */
	_dovi_parse_dev_node();

	memset((void *)&dovi_disp_common_info, 0, sizeof(struct disp_hw_common_info));

	dovi_disp_common_info = *info;

	dovi_debug_init();

	dovi_core1_init(dovi_reg_base[DOVI_CORE1]);
	dovi_core2_init(dovi_reg_base[DOVI_CORE2]);
	dovi_core3_init(dovi_reg_base[DOVI_CORE3]);

	dovi_sec_init();

	disp_dovi_common_init();

	disp_dovi_vsync_init();

	if (g_force_dolby) {
		uint32_t *p_core1_sdr_sdr_lut;
		uint32_t *p_core2_sdr_sdr_lut;

		dolby_path_enable = 1;

		p_core1_sdr_sdr_lut = dovi_v241_1080p_yuv_yuv_core1_lut;

		if (g_out_format == DOVI_FORMAT_DOVI) {
			p_core2_sdr_sdr_lut = dovi_1080p_sdr_ipt_core2_lut;
		} else if (g_out_format == DOVI_FORMAT_HDR10) {
			p_core2_sdr_sdr_lut = dovi_1080p_sdr_hdr10_core2_lut;
		} else if (g_out_format == DOVI_FORMAT_DOVI_LOW_LATENCY) {
			p_core2_sdr_sdr_lut = dovi_1080p_sdr_ll_core2_lut;
		} else if (g_out_format == DOVI_FORMAT_SDR) {
			p_core2_sdr_sdr_lut = dovi_1080p_sdr_sdr_core2_lut;
		}

		dovi_core1_hal_set_enable(true);
		dovi_core2_hal_set_enable(true);
		dovi_core3_hal_set_enable(true);
		dovi_core1_config_lut(p_core1_sdr_sdr_lut);
		dovi_core2_config_lut(p_core2_sdr_sdr_lut);
		disp_dovi_set_event(DOVI_EVENT_UPDATE_ISR);
	}

	osd_enable = 0;
	dolby_core2_enable = 0;
	dovi_idk_test = 0;
	dovi_idk_file_id = 0;
	dovi_init_done = true;

	/* dovi_func_end(); */
	return DOVI_STATUS_OK;
}

void disp_dovi_isr(void)
{
	/* core1 isr */
	dovi_core1_hal_isr();

	/* core2 isr */
	dovi_core2_hal_isr();

	/* core3 isr */
	dovi_core3_hal_isr();

}

int disp_dovi_start(struct disp_hw_common_info *info, unsigned int layer_id)
{
	dovi_func();
	dovi_disp_common_info = *info;
	disp_dovi_show_tv_cap(&info->tv);
	disp_dovi_show_res_status(dovi_res.res_mode);

	return DOVI_STATUS_OK;
}

int disp_dovi_stop(unsigned int layer_id)
{
	dovi_func();

	return DOVI_STATUS_OK;
}

int disp_dovi_deinit(void)
{
	dovi_func();

	return DOVI_STATUS_OK;
}

int disp_dovi_show_tv_cap(struct disp_hw_tv_capbility *cap)
{
	if (cap) {
		dovi_default("support_hdr %d\n", cap->is_support_hdr);
		dovi_default("support_dolby %d\n", cap->is_support_dolby);
		dovi_default("support_dolby_4k60p %d\n", cap->is_support_dolby_2160p60);
		dovi_default("support_601 %d\n", cap->is_support_601);
		dovi_default("support_709 %d\n", cap->is_support_709);
		dovi_default("support_2020 %d\n", cap->is_support_bt2020);
	} else
		dovi_error("cap pointer is null %p\n", cap);

	return DOVI_STATUS_OK;
}

int disp_dovi_show_res_status(HDMI_VIDEO_RESOLUTION res)
{
	dovi_default("htotal %d vtotal %d\n",
		dobly_resolution_table[res].htotal, dobly_resolution_table[res].vtotal);
	dovi_default("width %d height %d\n",
		dobly_resolution_table[res].width, dobly_resolution_table[res].height);
	dovi_default("frequency %d progressive %d\n",
		dobly_resolution_table[res].frequency,
		dobly_resolution_table[res].is_progressive);
	dovi_default("hd %d mod %d\n",
		dobly_resolution_table[res].is_hd,
		dobly_resolution_table[res].res_mode);
	dovi_default("output resolution = %s\n", dobly_resstr[res]);

	return DOVI_STATUS_OK;
}

int disp_dovi_show_dev_status(void)
{
	dovi_default("dovi_dev %p\n", dovi_dev);

	dovi_default("dovi device node name %s\n", dovi_dts_nd_name);

	dovi_default("core%d base=0x%p\n", DOVI_CORE1, dovi_reg_base[DOVI_CORE1]);
	dovi_default("core%d base=0x%p\n", DOVI_CORE2, dovi_reg_base[DOVI_CORE2]);
	dovi_default("core%d base=0x%p\n", DOVI_CORE3, dovi_reg_base[DOVI_CORE3]);

	return DOVI_STATUS_OK;
}

int disp_dovi_status(uint32_t level)
{
	dovi_func();

	disp_dovi_show_tv_cap(&dovi_disp_common_info.tv);
	disp_dovi_show_res_status(dovi_res.res_mode);

	disp_dovi_show_dev_status();

	dovi_core1_status();
	dovi_core2_status();
	dovi_core3_status();

	return DOVI_STATUS_OK;
}

void disp_dovi_wakeup_routine(void)
{
	if (dovi_option & DOVI_PATH_TEST_MASK) {
		wake_up(&disp_dovi_wq);
		disp_dovi_wakeup_thread = 1;

		dovi_info("wakeup thread\n");
	}
}

static int dovi_routine(void *data)
{
	while (1) {
		wait_event_interruptible(disp_dovi_wq, disp_dovi_wakeup_thread);
		disp_dovi_wakeup_thread = 0;

		mutex_lock(&disp_dovi_mutex);

		if (!(dovi_option & DOVI_PATH_TEST_MASK)) {
			dovi_default("dovi_option 0x%X is invalid\n", dovi_option);
			mutex_unlock(&disp_dovi_mutex);
			continue;
		}

		dovi_option &= ~DOVI_PATH_TEST_MASK;

		disp_dovi_default_path(dovi_option);

		disp_dovi_isr();

		mutex_unlock(&disp_dovi_mutex);
	}

	return DOVI_STATUS_OK;
}

int disp_dovi_default_path_init(uint32_t option)
{
	if (!disp_dovi_thread) {
		mutex_init(&disp_dovi_mutex);
		dovi_default("mutex init\n");

		init_waitqueue_head(&disp_dovi_wq);
		disp_dovi_wakeup_thread = 0;
		dovi_default("wq init\n");

		disp_dovi_thread =
			kthread_create(dovi_routine, NULL, "disp_dovi_thread");
		wake_up_process(disp_dovi_thread);
		dovi_default("thread %p init\n", disp_dovi_thread);
	}

	mutex_lock(&disp_dovi_mutex);

	dovi_option = option | DOVI_PATH_TEST_MASK;

	mutex_unlock(&disp_dovi_mutex);

	return DOVI_STATUS_OK;
}

int disp_dovi_default_path(uint32_t option)
{
	uint32_t *p_core1_reg;
	uint32_t *p_core2_reg;
	uint32_t *p_core3_reg;

	uint32_t *p_core1_lut;
	uint32_t *p_core2_lut;

	uint32_t core1_reg_size = sizeof(struct core1_dpm_reg_t);
	uint32_t core2_reg_size = sizeof(struct core2_dpm_reg_t);
	uint32_t core3_reg_size = sizeof(struct core3_dpm_reg_t);
	uint32_t lut_size = DOVI_LUT_SIZE;

	static bool dolby_enable;

	if ((dolby_enable && option) || (!dolby_enable && !option)) {
		dovi_info("option %d dobly already enable %d\n",
		option, dolby_enable);
		return DOVI_STATUS_OK;
	}

	if (option)
		dolby_enable = true;
	else
		dolby_enable = false;

	dovi_info("option %d dobly_enable %d\n", option, dolby_enable);

	dovi_info("core_reg_size %d %d %d lut_size %d\n",
		    core1_reg_size, core2_reg_size, core3_reg_size, lut_size);

	if (dolby_enable) {
		dovi_info("enable dobly clock\n");
		disp_clock_enable(DISP_CLK_DOLBY1, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY2, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY3, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY_MIX, dolby_enable);

		dovi_core1_init(dovi_reg_base[DOVI_CORE1]);
		dovi_core2_init(dovi_reg_base[DOVI_CORE2]);
		dovi_core3_init(dovi_reg_base[DOVI_CORE3]);

		dovi_core1_hal_set_enable(dolby_enable);
		dovi_core2_hal_set_enable(dolby_enable);
		dovi_core3_hal_set_enable(dolby_enable);

		dovi_core1_hal_control_config(&dovi_res);
		dovi_core2_hal_control_config(&dovi_res);
		dovi_core3_hal_control_config(&dovi_res);

		if (option == 1) {
			/* idk 2.4.1 5200, core1 yuv input, core3 yuv output, 1080p */
			p_core1_reg = dovi_v241_1080p_yuv_yuv_core1_reg;
			p_core2_reg = dovi_v241_1080p_ipt_yuv_core2_reg;
			p_core3_reg = dovi_v241_1080p_yuv_yuv_core3_reg;
			p_core1_lut = dovi_v241_1080p_yuv_yuv_core1_lut;
			p_core2_lut = dovi_v241_1080p_ipt_yuv_core2_lut;
		} else if (option == 2) {
			/* idk 2.4.1 5000, core1 ipt input, core3 yuv output, 1080p */
			p_core1_reg = dovi_v241_1080p_ipt_yuv_core1_reg;
			p_core2_reg = dovi_v241_1080p_ipt_yuv_core2_reg;
			p_core3_reg = dovi_v241_1080p_ipt_yuv_core3_reg;
			p_core1_lut = dovi_v241_1080p_ipt_yuv_core1_lut;
			p_core2_lut = dovi_v241_1080p_ipt_yuv_core2_lut;
		} else if (option == 3 || option == 5) {
			/* idk 2.4.1 5201, core1 yuv input, core3 dovi output, 1080p */
			p_core1_reg = dovi_v241_1080p_yuv_ipt_core1_reg;
			p_core2_reg = dovi_v241_1080p_ipt_yuv_core2_reg;
			p_core3_reg = dovi_v241_1080p_yuv_ipt_core3_reg;
			p_core1_lut = dovi_v241_1080p_yuv_ipt_core1_lut;
			p_core2_lut = dovi_v241_1080p_ipt_yuv_core2_lut;
		} else {
			/* idk 2.4.1 5045, core1 ipt input, core3 dovi output, 1080p */
			p_core1_reg = dovi_v241_1080p_ipt_ipt_core1_reg;
			p_core2_reg = dovi_v241_1080p_ipt_ipt_core2_reg;
			p_core3_reg = dovi_v241_1080p_ipt_ipt_core3_reg;
			p_core1_lut = dovi_v241_1080p_ipt_ipt_core1_lut;
			p_core2_lut = dovi_v241_1080p_ipt_ipt_core2_lut;
		}

		dovi_info("p_core_reg 0x%p 0x%p 0x%p lut 0x%p 0x%p\n",
			    p_core1_reg, p_core2_reg,
			    p_core3_reg, p_core1_lut,
			    p_core2_lut);

		dovi_core1_hal_config_reg(p_core1_reg);
		dovi_core2_hal_config_reg(p_core2_reg);
		dovi_core3_hal_config_reg(p_core3_reg);

		dovi_core1_config_lut(p_core1_lut);
		dovi_core2_config_lut(p_core2_lut);

		vdout_sys_hal_videoin_source_sel(VIDEOIN_SRC_SEL_DOLBY3);
	}

	disp_path_set_dolby(dolby_enable);
	vdout_sys_hal_dolby_mix_on(dolby_enable);

	disp_path_set_delay(DISP_PATH_MVDO, dovi_res.res_mode);
	disp_path_set_delay(DISP_PATH_MVDO_OUT, dovi_res.res_mode);

	fmt_hal_set_output_444(DISP_FMT_MAIN, !dolby_enable);
	fmt_hal_set_uv_swap(DISP_FMT_MAIN, dolby_enable);

	if (!dolby_enable) {
		disp_clock_enable(DISP_CLK_DOLBY_MIX, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY3, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY2, dolby_enable);
		disp_clock_enable(DISP_CLK_DOLBY1, dolby_enable);

		dovi_info("disable dobly clock\n");
	}

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	if (option == 3 || option == 4)
		vDolbyHdrEnable(true);
	else
		vDolbyHdrEnable(false);
#endif

	return DOVI_STATUS_OK;
}

void disp_dovi_set_event(int event)
{
	mutex_lock(&disp_dovi_vsync_mutex);
	dovi_default("dovi set event 0x%X %lu\n", event, disp_dovi_vsync_cnt);
	disp_dovi_event |= event;
	mutex_unlock(&disp_dovi_vsync_mutex);
}

void disp_dovi_wakeup_vsync_routine(void)
{
	disp_dovi_wakeup_vsync_thread = 1;
	wake_up(&disp_dovi_vsync_wq);
}

static int disp_dovi_vsync_routine(void *data)
{
	while (1) {
		wait_event_interruptible(disp_dovi_vsync_wq, disp_dovi_wakeup_vsync_thread);
		disp_dovi_wakeup_vsync_thread = 0;

		mutex_lock(&disp_dovi_vsync_mutex);

		++disp_dovi_vsync_cnt;

		if (disp_dovi_event & DOVI_EVENT_UPDATE_ISR) {
			disp_dovi_event &= ~DOVI_EVENT_UPDATE_ISR;
			dovi_default("dovi clear isr event %X %X %lu\n",
				disp_dovi_event, ~DOVI_EVENT_UPDATE_ISR,
				disp_dovi_vsync_cnt);
			disp_dovi_isr();
		}

		if (disp_dovi_event & DOVI_EVENT_DISABLE_CLK) {
			disp_dovi_event &= ~DOVI_EVENT_DISABLE_CLK;
			dovi_default("dovi clear disable clk event %X %X %lu\n",
				disp_dovi_event, ~DOVI_EVENT_DISABLE_CLK,
				disp_dovi_vsync_cnt);
			disp_dovi_set_clk_disable(enable);
		}

		mutex_unlock(&disp_dovi_vsync_mutex);
	}

	return DOVI_STATUS_OK;
}

int disp_dovi_vsync_init(void)
{
	dovi_default("%s\n", __func__);

	if (!disp_dovi_vsync_thread) {
		mutex_init(&disp_dovi_vsync_mutex);
		dovi_default("mutex init\n");

		init_waitqueue_head(&disp_dovi_vsync_wq);
		disp_dovi_wakeup_vsync_thread = 0;
		dovi_default("wq init\n");

		disp_dovi_vsync_thread =
			kthread_create(disp_dovi_vsync_routine, NULL, "disp_dovi_vsync_thread");
		wake_up_process(disp_dovi_vsync_thread);
		dovi_default("thread %p init\n", disp_dovi_vsync_thread);
	}

	return DOVI_STATUS_OK;
}

int disp_dovi_dump_hdr_info(struct mtk_disp_hdr_md_info_t *hdr_info)
{
	struct mtk_vdp_dovi_md_t *dolby_info = NULL;
	bool dump_enable = false;
	unsigned char *buff = NULL;
	uint32_t rpu_len = 0;

	if (hdr_info->dr_range == DISP_DR_TYPE_DOVI) {
		dolby_info = &hdr_info->metadata_info.dovi_metadata;
		buff = dolby_info->buff;
		rpu_len = dolby_info->len;

		dovi_info("dump info rpu pts %lld len %d addr %p\n",
			    dolby_info->pts, dolby_info->len, dolby_info->buff);

		if (dolby_info->len != 0) {
			uint32_t *addr = (uint32_t *) dolby_info->buff;

			dovi_info("dovi rpu value 0x%X 0x%X 0x%X 0x%X\n",
				    addr[0], addr[1], addr[2], addr[3]);
		}

		dump_enable = true;

	}

	disp_dovi_dump_rpu(dump_enable, buff, rpu_len);

	return DOVI_STATUS_OK;
}

void disp_dovi_set_hdr_enable(uint32_t enable, uint32_t outformat)
{
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
	if (outformat == DOVI_FORMAT_DOVI)
		vDolbyHdrEnable(enable);
	else if (outformat == DOVI_FORMAT_HDR10) {
		vSetStaticHdrType(GAMMA_ST2084);
		vHdrEnable(enable);
	}
#endif
}

uint32_t disp_dovi_set_idk_info(void)
{
	if (dovi_idk_test) {
		uint32_t out_format;
		int use_ll = 0;
		uint32_t dovi2hdr10_mapping = 0;
		char *vsvdb_file_name;
		enum pri_mode_t priority_mode;
		enum graphic_format_t g_format;
		char *graphic_file_name;
		uint32_t graphic_file_size = 0;

		out_format = get_dovi_out_format(dovi_idk_file_id);
		use_ll = get_dovi_use_ll(dovi_idk_file_id);
		ll_rgb_desired = get_dovi_ll_rgb_desired(dovi_idk_file_id);
		dovi2hdr10_mapping = get_dovi2hdr10_mapping_type(dovi_idk_file_id);
		vsvdb_file_name = get_dovi_vsvdb_file_name(dovi_idk_file_id);
		graphic_file_name = get_dovi_graphic_file_name(dovi_idk_file_id);

		/* HDR10 dump 10bit */
		if (out_format == DOVI_FORMAT_HDR10) {
			idk_dump_bpp = VIDEOIN_BITMODE_10;
		} else {
			if ((dovi_idk_file_id == 5042) ||
				(dovi_idk_file_id == 5043) ||
				(dovi_idk_file_id == 5044) ||
				get_dovi_use_ll(dovi_idk_file_id))
				/* low latency mode, use hdr10 output dump */
				idk_dump_bpp = VIDEOIN_BITMODE_12;
			else
				idk_dump_bpp = VIDEOIN_BITMODE_8;
		}
		if (core3_bypass_dither)
			idk_dump_bpp = VIDEOIN_BITMODE_12;

		dovi_set_output_format(out_format);
		dovi_set_low_latency_mode(use_ll, ll_rgb_desired);
		dovi_set_dovi2hdr10_mapping(dovi2hdr10_mapping);
		dovi_set_vsvdb_file_name(vsvdb_file_name);

		f_graphic_on = get_dovi_graphic_on(dovi_idk_file_id);
		priority_mode = get_dovi_priority_mode(dovi_idk_file_id);
		g_format = get_dovi_g_format(dovi_idk_file_id);
		dovi_set_graphic_info(f_graphic_on);
		dovi_set_priority_mode(priority_mode);
		dovi_set_graphic_format(g_format);

		disp_dovi_idk_dump_frame_start(dovi_idk_file_id);

		/*set osd load dobly graphic for show */
		graphic_file_size = dobly_resolution_table[dovi_res.res_mode].width
			* dobly_resolution_table[dovi_res.res_mode].height * 4;

		if (f_graphic_on) {
			disp_dovi_set_osd_clk_enable(true);
			disp_dovi_load_buffer(graphic_file_name,
				(unsigned char *)graphic_idk_load_addr_va,
				graphic_file_size);
			disp_dovi_set_graphic_header(graphic_header_idk_load_addr_va,
			graphic_idk_dump_load_pa);

		}
	}
	return DOVI_STATUS_OK;
}

uint32_t disp_dovi_set_sdk_info(void)
{
	if (dovi_sdk_test) {
		enum pri_mode_t priority_mode;
		char *graphic_file_name;
		uint32_t graphic_file_size = 0;

		graphic_file_name = dovi_graphic_name[dovi_idk_file_id];

		f_graphic_on = 1;
		priority_mode = G_PRIORITY;
		dovi_set_graphic_info(f_graphic_on);
		dovi_set_priority_mode(priority_mode);

		disp_dovi_alloc_graphic_buffer();

		/*set osd load dobly graphic for show */
		graphic_file_size = dobly_resolution_table[dovi_res.res_mode].width
			* dobly_resolution_table[dovi_res.res_mode].height * 4;

		if (f_graphic_on) {
			disp_dovi_set_osd_clk_enable(true);
			disp_dovi_load_buffer(graphic_file_name,
				(unsigned char *)graphic_idk_load_addr_va,
				graphic_file_size);
			disp_dovi_set_graphic_header(graphic_header_idk_load_addr_va,
			graphic_idk_dump_load_pa);

		}
	}

	disp_dovi_sdk_handle(dovi_sdk_test);

	return DOVI_STATUS_OK;
}


uint32_t idk_graphic_header[] = {
	0xE0000000, /* color mode */
	0xFFC00000, /* 23:0 -> 27:4*/
	0xE40001E0, /* 1080p */
	0x00000000,
	0x00001000,
	0x00001000,
	0x00000001, /* 3:2 -> 31:30  */
	0x51800000, /* 24:23 -> 29:28  */
	0x00000000,
	0x04380780,
	0x00000438,
	0x00000780,
};

void disp_dovi_set_graphic_header(uint32_t *va, dma_addr_t graphic_pa)
{
	uint32_t graphic_header_size = 48;
	uint32_t idx = 0;
	uint32_t width = dobly_resolution_table[dovi_res.res_mode].width;
	uint32_t height = dobly_resolution_table[dovi_res.res_mode].height;


	if (fhd_scale_to_uhd) {
		width = 1920;
		height = 1080;
	}
	dovi_default("gen header va %p pa 0x%x\n", va, (uint32_t)graphic_pa);

	memcpy(va, idk_graphic_header, graphic_header_size);

	va[1] &= 0xFF000000;
	va[1] |= ((graphic_pa & 0x0FFFFFFF) >> 4);

	va[2] &= 0xFFFFF800;
	va[2] |= ((width * 4) >> 4);

	va[6] &= (~(0x3 << 2));
	va[6] |= (((graphic_pa & 0xC0000000) >> 30) << 2);

	va[7] &= (~(0x3 << 23));
	va[7] |= (((graphic_pa & 0x30000000) >> 28) << 23);

	va[9] &= 0x0;
	va[9] |= height << 16;
	va[9] |= width;

	va[10] &= 0x0;
	va[10] |= height;

	va[11] &= 0x0;
	va[11] |= width;

	for (idx = 0; idx < graphic_header_size/4; idx++)
		dovi_default("va[%d] 0x%x\n", idx, va[idx]);
}

void disp_dovi_set_osd_clk_enable(bool enable)
{
	if (enable) {
		disp_clock_enable(DISP_CLK_OSDPLL, true);
		disp_clock_smi_larb_en(DISP_SMI_LARB0, true);
		disp_clock_enable(DISP_CLK_OSD_UHD, true);
		disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
		disp_clock_enable(DISP_CLK_OSD_SEL, true);
		if (dovi_res.res_mode <= HDMI_VIDEO_1920x1080p_50Hz)
			disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL_D2);
		else
			disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL);
	} else {
		disp_clock_enable(DISP_CLK_OSDPLL, false);
		disp_clock_enable(DISP_CLK_OSD_SEL, false);
		disp_clock_enable(DISP_CLK_OSD_PREMIX, false);
		disp_clock_smi_larb_en(DISP_SMI_LARB0, false);
		disp_clock_enable(DISP_CLK_OSD_UHD, false);
	}
}

void disp_dovi_set_osd_all_black(void)
{
	/* fgUpdate = 0 */
	WriteREGMsk(uhd_fmt_reg_base, 0x0, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x0, 0x1);
	/* active zone = 0 */
	uhd_active_zone = ReadREG(uhd_fmt_reg_base + 0x1c);
	fhd_active_zone = ReadREG(fhd_fmt_reg_base + 0x1c);
	dovi_default("read uhd/fhd_active_zone 0x%x 0x%x!\n", uhd_active_zone, fhd_active_zone);
	WriteREG(uhd_fmt_reg_base + 0x1c, 0x0);
	WriteREG(fhd_fmt_reg_base + 0x1c, 0x0);
	/* fgUpdate = 1 */
	WriteREGMsk(uhd_fmt_reg_base, 0x1, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x1, 0x1);
}

void disp_dovi_set_osd_active_zone(void)
{
	/* fgUpdate = 0 */
	WriteREGMsk(uhd_fmt_reg_base, 0x0, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x0, 0x1);
	/* set active zone */
	dovi_default("write uhd/fhd_active_zone 0x%x 0x%x!\n", uhd_active_zone, fhd_active_zone);
	WriteREG(uhd_fmt_reg_base + 0x1c, uhd_active_zone);
	WriteREG(fhd_fmt_reg_base + 0x1c, fhd_active_zone);
	/* fgUpdate = 1 */
	WriteREGMsk(uhd_fmt_reg_base, 0x1, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x1, 0x1);
}

void disp_dovi_set_osd_enable(bool enable)
{
	/* fgUpdate = 0 */
	WriteREGMsk(uhd_fmt_reg_base, 0x0, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x0, 0x1);
	if (enable == true) {
		dovi_default("open uhd for idk cert\n");
		disp_dovi_set_osd_showdoblylogo();
		/* fgOsdEn = 1 */
		WriteREGMsk(uhd_pla_reg_base, 0x1, 0x1);
		WriteREGMsk(fhd_pla_reg_base, 0x0, 0x1);
		WriteREGMsk(uhd_pla_reg_base + 0x4,
			graphic_header_idk_dump_load_pa >> 4, 0xFFFFFFFF);
		/* mix2_de_sel = 1 */
		WriteREGMsk(osd_premix_reg_base + 0x28, 0x1 << 31, 0x1 << 31);
		WriteREGMsk(osd_premix_reg_base + 0x28, 0x1 << 5, 0x1 << 5);
		WriteREGMsk(osd_premix_reg_base + 0x24, 0x1 << 5, 0x1 << 5);
	} else {
		dovi_default("close uhd for idk cert\n");
		/* fgOsdEn = 0 */
		WriteREGMsk(uhd_pla_reg_base, 0x0, 0x1);
		WriteREGMsk(fhd_pla_reg_base, 0x0, 0x1);
		/* mix2_de_sel = 1 */
		WriteREGMsk(osd_premix_reg_base + 0x28, 0x0 << 31, 0x1 << 31);
		WriteREGMsk(osd_premix_reg_base + 0x28, 0x0 << 5, 0x1 << 5);
		WriteREGMsk(osd_premix_reg_base + 0x24, 0x0 << 5, 0x1 << 5);
	}
	/* fgUpdate = 1 */
	WriteREGMsk(uhd_fmt_reg_base, 0x1, 0x1);
	WriteREGMsk(fhd_fmt_reg_base, 0x1, 0x1);
}


uint32_t disp_dovi_set_osd_showdoblylogo(void)
{
	uint32_t vtotal = dobly_resolution_table[dovi_res.res_mode].vtotal;
	uint32_t htotal = dobly_resolution_table[dovi_res.res_mode].htotal;
	uint32_t width = dobly_resolution_table[dovi_res.res_mode].width;
	uint32_t height = dobly_resolution_table[dovi_res.res_mode].height;
	unsigned int osd_hstart, osd_veven, osd_vodd;

	/* fgAlwaysUpdate = 1 */
	WriteREGMsk(uhd_fmt_reg_base, 0x1 << 1, 0x1 << 1);
	/* fgOsd2Prgs = 1 */
	WriteREGMsk(uhd_fmt_reg_base + 0x8, 0x1 << 4, 0x1 << 4);

	/* u4VsWidthMain = 6, u4HsWidthMain = 1 */
	WriteREGMsk(uhd_fmt_reg_base + 0xC, 0x6 << 12, 0x1FF << 12);
	WriteREGMsk(uhd_fmt_reg_base + 0xC, 0x1 << 22, 0x1FF << 22);

	/* u4AlphaSel = 0x1F */
	WriteREGMsk(uhd_fmt_reg_base + 0xAC, 0x1F << 20, 0x1F << 20);

	/* u4OvtMain = vtotal, u4OhtMain = htotal */
	WriteREGMsk(uhd_fmt_reg_base + 0xC, vtotal, 0xFFF);
	WriteREGMsk(uhd_fmt_reg_base + 0x64, htotal, 0xFFF);

	/* u4ScrnHSizeMain = width, u4ScrnVSizeMain = height */
	WriteREGMsk(uhd_fmt_reg_base + 0x1C, height, 0xFFF);
	WriteREGMsk(uhd_fmt_reg_base + 0x1C, width << 16, 0xFFF << 16);

	/* fgAutoSwEn = 1 */
	WriteREGMsk(uhd_fmt_reg_base + 0x8, 0x1 << 10, 0x1 << 10);

	if (dovi_idk_test)
		disp_path_set_delay(DISP_PATH_OSD1, dovi_res.res_mode);
	disp_path_get_active_zone(DISP_PATH_OSD1, dovi_res.res_mode, &osd_hstart,
		&osd_vodd, &osd_veven);

	/* u4ScrnHStartOsd2 = osd_hstart */
	WriteREGMsk(uhd_fmt_reg_base + 0x10, osd_hstart, 0x3FF);

	/* u4ScrnVStartBotMain = osd_veven, u4ScrnVStartTopMain = osd_vodd */
	WriteREGMsk(uhd_fmt_reg_base + 0x18, osd_veven, 0x1FFF);
	WriteREGMsk(uhd_fmt_reg_base + 0x18, osd_vodd << 16, 0x1FF << 16);

	/* u4Osd2HStart = 0, u4Osd2VStart = 0 */
	WriteREGMsk(uhd_fmt_reg_base + 0x24, 0, 0xFFFFFFFF);

	/* fgVsEdge = 1, fgHsEdge = 0 */
	WriteREGMsk(uhd_fmt_reg_base + 0x8, 0x1 << 1, 0x1 << 1);
	WriteREGMsk(uhd_fmt_reg_base + 0x8, 0x0, 0x1);

	/* fg_source_sync_select = 1 */
	WriteREGMsk(uhd_fmt_reg_base + 0x8, 0x1 << 3, 0x1 << 3);

	/* burst_8_enable = 1 */
	WriteREGMsk(uhd_pla_reg_base + 0xAC, 0x1 << 28, 0x1 << 28);


	if (fhd_scale_to_uhd) {
		/* fgScEn = 1 */
		WriteREGMsk(uhd_scl_reg_base + 0x0, 0x94, 0xFFFFFFFF);

		/* u4SrcHSize = width, u4SrcVSize = height */
		WriteREGMsk(uhd_scl_reg_base + 0x4, 0x438, 0x1FFF);
		WriteREGMsk(uhd_scl_reg_base + 0x4, 0x780 << 16, 0x1FFF << 16);

		/* u4DstHSize = width, u4DstVSize = height */
		WriteREGMsk(uhd_scl_reg_base + 0x8, height, 0x1FFF);
		WriteREGMsk(uhd_scl_reg_base + 0x8, width << 16, 0x1FFF << 16);

		/* u4VscHSize = width */
		WriteREGMsk(uhd_scl_reg_base + 0xC, 0x780, 0x1FFF);

		WriteREGMsk(uhd_scl_reg_base + 0x10, 0x0, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x14, 0x1FFD, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x18, 0x0, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x1C, 0x1FFC, 0xFFFFFFFF);
	} else {
		/* fgScEn = 1 */
		WriteREGMsk(uhd_scl_reg_base + 0x0, 0x0, 0xFFFFFFFF);

		/* u4SrcHSize = width, u4SrcVSize = height */
		WriteREGMsk(uhd_scl_reg_base + 0x4, height, 0x1FFF);
		WriteREGMsk(uhd_scl_reg_base + 0x4, width << 16, 0x1FFF << 16);

		/* u4DstHSize = width, u4DstVSize = height */
		WriteREGMsk(uhd_scl_reg_base + 0x8, height, 0x1FFF);
		WriteREGMsk(uhd_scl_reg_base + 0x8, width << 16, 0x1FFF << 16);

		/* u4VscHSize = width */
		WriteREGMsk(uhd_scl_reg_base + 0xC, width, 0x1FFF);

		WriteREGMsk(uhd_scl_reg_base + 0x10, 0x0, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x14, 0x0, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x18, 0x0, 0xFFFFFFFF);
		WriteREGMsk(uhd_scl_reg_base + 0x1C, 0x0, 0xFFFFFFFF);
	}
	return DOVI_STATUS_OK;
}

int disp_dovi_set_mute_unmute(uint32_t enable)
{
	if (!dovi_idk_test && dovi_mute) {
		if (enable) {
			if (!dolby_core2_enable) {
				dolby_core2_enable = 1;
				dovi_default("enable DISP_CLK_DOLBY2 for showblack\n");
				disp_clock_enable(DISP_CLK_DOLBY2, true);
				disp_dovi_process_core2(true);
			}
			dovi_default("dovi_core2_hal_show_black true\n");
			dovi_core2_hal_show_black(true);
		} else if (!enable) {
			if (!osd_enable) {
				if (dolby_core2_enable) {
					dolby_core2_enable = 0;
					dovi_default("disable DISP_CLK_DOLBY2 when mute done\n");
					disp_clock_enable(DISP_CLK_DOLBY2, false);
					disp_dovi_process_core2(false);
				}
			}
			dovi_default("dovi_core2_hal_show_black false\n");
			dovi_core2_hal_show_black(false);
		}
	}
	return DOVI_STATUS_OK;
}

int disp_dovi_set_clk_enable(uint32_t enable)
{
	/* when play dolby file done, we must close dolby clk */
	if (!dovi_idk_test) {
		/* if osd_enable is 1, we must open core2 when enable dolby  */
		if (enable && osd_enable) {
			if (!dolby_core2_enable) {
				dolby_core2_enable = 1;
				dovi_default("enable DISP_CLK_DOLBY2\n");
				disp_clock_enable(DISP_CLK_DOLBY2, true);
				if (dovi_info.out_format == DOVI_FORMAT_DOVI) {
					dovi_info.is_graphic_mode = true;
					dovi_set_priority_mode(dovi_info.is_graphic_mode);
					priority_mode_change = true;
				}
				disp_dovi_process_core2(true);
			}
		}
	}

	return DOVI_STATUS_OK;
}

int disp_dovi_set_clk_disable(uint32_t enable)
{
	if (!enable) {
		dovi_default("disable dolby clock %lu!\n", disp_dovi_vsync_cnt);
		disp_clock_enable(DISP_CLK_DOLBY_MIX, false);
		disp_clock_enable(DISP_CLK_DOLBY3, false);

		if (dolby_core2_enable) {
			disp_dovi_set_mute_unmute(false);
			dolby_core2_enable = 0;
			DISP_LOG_DEBUG("disable DISP_CLK_DOLBY2\n");
			disp_clock_enable(DISP_CLK_DOLBY2, false);
			disp_dovi_process_core2(false);
		}

		/* core1 must be disabled at last */
		disp_clock_enable(DISP_CLK_DOLBY1, false);
		disp_clock_smi_larb_en(DISP_SMI_LARB4, false);
	}
	return DOVI_STATUS_OK;
}


int disp_dovi_idk_handle(uint32_t enable)
{
    /* we must only show dobly logo when idk test */
	if (f_graphic_on) {
		if (enable && dovi_idk_test && !dovi_logo_show) {
			disp_dovi_set_osd_enable(true);
			disp_clock_enable(DISP_CLK_DOLBY2, true);
			disp_dovi_process_core2(true);
			dovi_logo_show = true;
		} else if (!enable && dovi_idk_test && dovi_logo_show) {
			disp_dovi_set_osd_enable(false);
			disp_clock_enable(DISP_CLK_DOLBY2, false);
			disp_dovi_process_core2(false);
			dovi_logo_show = false;
		}
	} else if (dovi_idk_test) {
		/* fgUpdate = 0 */
		WriteREGMsk(uhd_fmt_reg_base, 0x0, 0x1);
		WriteREGMsk(fhd_fmt_reg_base, 0x0, 0x1);
		/* fgOsdEn = 0 */
		WriteREGMsk(uhd_pla_reg_base, 0x0, 0x1);
		WriteREGMsk(fhd_pla_reg_base, 0x0, 0x1);
		/* fgUpdate = 1 */
		WriteREGMsk(uhd_fmt_reg_base, 0x1, 0x1);
		WriteREGMsk(fhd_fmt_reg_base, 0x1, 0x1);
	}

	/* when play done, we must close file and free mem */
	if (!enable && dovi_idk_test) {
		disp_dovi_idk_dump_frame_end();
		dovi_idk_test = 0;
		f_graphic_on = 0;
	}
	return DOVI_STATUS_OK;
}

int disp_dovi_sdk_handle(uint32_t enable)
{
	struct disp_hw *vdp_drv = disp_vdp_get_drv();

	dovi_default("%s enable %d dovi_sdk_test %d dovi_logo_show %d\n",
	__func__, enable, dovi_sdk_test, dovi_logo_show);

	/* we must only show dobly logo when sdk test */
	if (enable && dovi_sdk_test && !dovi_logo_show) {
		vdp_drv->drv_call(DISP_CMD_OSD_UPDATE, &enable);

		disp_dovi_set_osd_enable(true);
		disp_clock_enable(DISP_CLK_DOLBY2, true);
		dovi_logo_show = true;
	} else if (!enable && !dovi_sdk_test && dovi_logo_show) {
		disp_dovi_set_osd_enable(false);
		disp_clock_enable(DISP_CLK_DOLBY2, false);

		disp_dovi_free_graphic_buffer();
		vdp_drv->drv_call(DISP_CMD_OSD_UPDATE, &enable);
		dovi_logo_show = false;

		dovi_sdk_test = 0;
		f_graphic_on = 0;
	} else if (dovi_sdk_test) {
		/* fgUpdate = 0 */
		WriteREGMsk(uhd_fmt_reg_base, 0x0, 0x1);
		WriteREGMsk(fhd_fmt_reg_base, 0x0, 0x1);
		/* fgOsdEn = 0 */
		WriteREGMsk(uhd_pla_reg_base, 0x0, 0x1);
		WriteREGMsk(fhd_pla_reg_base, 0x0, 0x1);
		/* fgUpdate = 1 */
		WriteREGMsk(uhd_fmt_reg_base, 0x1, 0x1);
		WriteREGMsk(fhd_fmt_reg_base, 0x1, 0x1);
	}

	return DOVI_STATUS_OK;
}


int disp_dovi_process_cmd(enum DISP_CMD cmd, void *data)
{
	struct mtk_disp_hdr_md_info_t *hdr_info;
	unsigned int dovi_gpm;

	switch (cmd) {
	case DISP_CMD_METADATA_UPDATE:
		hdr_info = (struct mtk_disp_hdr_md_info_t *) data;
		if (dump_rpu_enable)
			disp_dovi_dump_hdr_info(hdr_info);

		if (hdr_info->enable == 0)
			enable = 0;
		else
			enable = 1;

		disp_dovi_idk_handle(enable);
		/* core2 disable clk must be at before others,
		*  oherwise core1 will be not work(black screen).
		*/
		disp_dovi_set_clk_enable(enable);
		/* if priority_mode_change, we must reinit */
		dovi_get_priority_mode(&dovi_gpm);
		if (priority_mode_change && (hdr_info->enable != 0)) {
			if (hdr_info->enable == 1)
				hdr_info->enable = DOVI_PRIORITY_MODE_CHANGE;
			priority_mode_change = false;
		} else if ((dovi_gpm == 0) && (osd_enable == 1)) {
			dovi_set_priority_mode(1);
			if (hdr_info->enable == 1)
				hdr_info->enable = DOVI_PRIORITY_MODE_CHANGE;
		} else if ((dovi_gpm == 1) && (osd_enable == 0)) {
			dovi_set_priority_mode(0);
			if (hdr_info->enable == 1)
				hdr_info->enable = DOVI_PRIORITY_MODE_CHANGE;
		}

		disp_dovi_process(enable, hdr_info);
		disp_dovi_isr();

		if (!enable)
			disp_dovi_set_event(DOVI_EVENT_DISABLE_CLK);
		break;

	case DISP_CMD_METADATA_UPDATE_RES_CHANGE:
		hdr_info = (struct mtk_disp_hdr_md_info_t *) data;
		disp_dovi_process(1, hdr_info);
		disp_dovi_isr();
		break;

	case DISP_CMD_DOVI_OUT_FORMAT_UPDATE:
		if (!dovi_idk_test) {
			dovi_info = *((struct dovi_info_t *)data);
			dovi_set_output_format(dovi_info.out_format);
			if (dovi_info.out_format == DOVI_FORMAT_DOVI)
				dovi_core3_hal_adjust_hstart(true);
			else
				dovi_core3_hal_adjust_hstart(false);
			/* ll_format determines low latency mode is yuv(0) or rgb(1). */
			dovi_set_low_latency_mode(dovi_info.is_low_latency, ll_format);
			dovi_set_priority_mode(dovi_info.is_graphic_mode);
			dovi_update_graphic_info();
			#if 0
			if (dovi_info.out_format == DOVI_FORMAT_DOVI) {
				if (dovi_info.is_graphic_mode == true)
					dovi_core1_hal_bypass_cvm(false);
				else
					dovi_core1_hal_bypass_cvm(true);
			} else
				dovi_core1_hal_bypass_cvm(false);
			#endif
			dovi_core1_hal_bypass_cvm(false);
			dovi_set_vsvdb_hdmi(dovi_info.vsvdb_edid, 0x1A);
		}
		break;


	case DISP_CMD_DOVI_CORE2_UPDATE:
		osd_enable = *((uint32_t *)data);
		dovi_default("osd_enable %d\n", osd_enable);
		if ((ui_force_hdr_type > 1) && osd_enable
			&& !netflix_dolby_path_enable && netflix_dolby_osd_no_ready) {
			dolby_core2_enable = 1;
			disp_clock_enable(DISP_CLK_DOLBY2, true);
			disp_dovi_process_core2(true);
			vdp_dovi_path_enable();
		}
		/* open close core2 according the osd status when dolby path enable */
		if (!dovi_idk_test && enable) {
			if (osd_enable) {
				if (!dolby_core2_enable) {
					dolby_core2_enable = 1;
					dovi_default("enable DISP_CLK_DOLBY2\n");
					disp_clock_enable(DISP_CLK_DOLBY2, true);
					if (dovi_info.out_format == DOVI_FORMAT_DOVI) {
						dovi_info.is_graphic_mode = true;
						dovi_set_priority_mode(dovi_info.is_graphic_mode);
						priority_mode_change = true;
					}
					disp_dovi_process_core2(true);
				}
			} else {
				if (dolby_core2_enable && !netflix_dolby_path_enable) {
					disp_dovi_set_mute_unmute(false);
					dolby_core2_enable = 0;
					dovi_default("disable DISP_CLK_DOLBY2\n");
					disp_clock_enable(DISP_CLK_DOLBY2, false);
					if (dovi_info.out_format == DOVI_FORMAT_DOVI) {
						dovi_info.is_graphic_mode = false;
						dovi_set_priority_mode(dovi_info.is_graphic_mode);
						priority_mode_change = true;
					}
					disp_dovi_process_core2(false);
				}
			}
			disp_dovi_isr();
		}
		break;

	case DISP_CMD_DOVI_SET_VIDEO_IN:
		disp_dovi_idk_dump_vin(true, VIDEOIN_SRC_SEL_DOLBY3, VIDEOIN_FORMAT_444);
		break;

	case DISP_CMD_DOVI_DUMP_FRAME:
		disp_dovi_idk_dump_frame();
		break;

	case DISP_CMD_DOVI_FULL_VS10:
		ui_force_hdr_type = *((uint32_t *)data);
		dovi_default("DISP_CMD_DOVI_FULL_VS10 type:%d\n", ui_force_hdr_type);
		if (ui_force_hdr_type > 1)
			vdp_dovi_path_enable();
		else
			vdp_dovi_path_disable();
		break;

	default:
		break;
	}
	return DOVI_STATUS_OK;
}

int disp_dovi_irq_handler(uint32_t irq)
{
	if (!dovi_init_done)
		return DOVI_STATUS_OK;

	switch (irq) {
	case DISP_IRQ_FMT_ACTIVE_START:
		break;
	case DISP_IRQ_FMT_VSYNC:
		disp_dovi_wakeup_routine();
		disp_dovi_wakeup_vsync_routine();
		break;
	default:
		break;
	}
	return DOVI_STATUS_OK;
}


/***************** driver************/
struct disp_hw disp_dovi_drv = {
	.name = DOVI_DRV_NAME,
	.init = disp_dovi_init,
	.deinit = disp_dovi_deinit,
	.start = disp_dovi_start,
	.stop = disp_dovi_stop,
	.suspend = NULL,
	.resume = NULL,
	.get_info = NULL,
	.change_resolution = disp_dovi_resolution_change,
	.config = NULL,
	.irq_handler = disp_dovi_irq_handler,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = disp_dovi_status,
	.set_cmd = disp_dovi_process_cmd,
};

struct disp_hw *disp_dovi_get_drv(void)
{
	return &disp_dovi_drv;
}
