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

#define LOG_TAG "drv_osd_if"

#include <linux/kthread.h>

#include <linux/vmalloc.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/types.h>


#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/file.h>
#include <mt-plat/sync_write.h>
#include <linux/dma-mapping.h>
#include <linux/memory.h>

#include "disp_osd_if.h"
#include "osd_sw.h"
#include "disp_osd_fence.h"
#include "osd_hw.h"

#include "disp_osd_ion.h"

#include "disp_osd_log.h"
#include "sw_sync.h"
#include "sync.h"
#include "disp_clk.h"
#include "disp_irq.h"

#include "disp_hw_mgr.h"
#include "disp_pmx_if.h"
#include "disp_hw_log.h"
#include "disp_path.h"

/*#include "mtk_sync.h"*/
struct task_struct *osd_irq_task[MAX_OSD_INPUT_CONFIG];
wait_queue_head_t osd_irq_wq[MAX_OSD_INPUT_CONFIG];
atomic_t osd_irq_event[MAX_OSD_INPUT_CONFIG] = {ATOMIC_INIT(0)};

struct task_struct *osd_config_task;
wait_queue_head_t osd_config_wq;
atomic_t osd_config_event = ATOMIC_INIT(0);

uint32_t osd_layer[MAX_OSD_INPUT_CONFIG] = {OSD_PLANE_1, OSD_PLANE_2};
unsigned long vsync_cnt, vsync_cnt1;
bool find_current[MAX_OSD_INPUT_CONFIG];
unsigned long config_cnt[MAX_OSD_INPUT_CONFIG];
unsigned long release_cnt[MAX_OSD_INPUT_CONFIG];

static struct list_head OSD_Buffer_Head[MAX_OSD_INPUT_CONFIG];
static struct list_head OSD_Configed_Head[MAX_OSD_INPUT_CONFIG];

Osd_buffer_list *pview[MAX_OSD_INPUT_CONFIG] = { NULL };

static BOOL fgupdate[MAX_OSD_INPUT_CONFIG];
static BOOL fg_config_update[MAX_OSD_INPUT_CONFIG];
static BOOL fgupdate_ex[MAX_OSD_INPUT_CONFIG];
static BOOL fg_alpha_det_update[MAX_OSD_INPUT_CONFIG];
bool is_stop_plane_done = true;

bool fg_debug_update;
UINT32 m_uptimes;
bool fg_osd_alpha_detect_en;
int  fg_osd_always_update_frame[MAX_OSD_INPUT_CONFIG];
bool fg_update_after_init;
bool fg_osd_debug_dump_en[MAX_OSD_INPUT_CONFIG];
bool fg_dovi_idk_test;
struct osd_context_t osd;
bool trigger[MAX_OSD_INPUT_CONFIG];
struct mtk_disp_buffer *osd_stop_config;
/* -------------------------plane config------------------------------------------ */
static int _osd_plane_map(unsigned int lay_id)
{
	switch (lay_id) {
	case 0:
		return OSD_PLANE_2;/*osd 3*/
	case 1:
		return OSD_PLANE_1;/*osd 2*/
	default:
		return OSD_PLANE_1;
	}
}
static int _osd_parse_dev_node(void)
{
	struct device_node *np;
	unsigned int reg_value;
	unsigned int irq_value;
	struct disp_hw *osd_drv = disp_osd_get_drv();
#if !OSD_IOMMU_SUPPORT
	/*amy:for fpga test*/
	uintptr_t mmu_reg;
#endif
	/*unsigned int irq_value; */

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-osd");
	if (np == NULL) {
		OSDERR("dts error, no osd device node.\n");
		return OSD_RET_DTS_FAIL;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);
	of_property_read_u32_index(np, "interrupts", 1, &irq_value);

	/*todo: osd register for 8695*/
	osd.osd_reg.osd_fmt_reg_base[0] = (uintptr_t) of_iomap(np, 0);	/*osd2: 0x14004300*/
	osd.osd_reg.osd_pln_reg_base[0] = osd.osd_reg.osd_fmt_reg_base[0] + 0x100;
	osd.osd_reg.osd_scl_reg_base[0] = osd.osd_reg.osd_fmt_reg_base[0] + 0x200;

	osd.osd_reg.osd_fmt_reg_base[1] = (uintptr_t) of_iomap(np, 1);	/*osd3: 0x14003000 */
	osd.osd_reg.osd_pln_reg_base[1] = osd.osd_reg.osd_fmt_reg_base[1] + 0x100;
	osd.osd_reg.osd_scl_reg_base[1] = osd.osd_reg.osd_fmt_reg_base[1] + 0x200;

	osd.osd_reg.osd_pmx_reg_base = (uintptr_t) of_iomap(np, 2);	/*0x14001c00 */

#if !OSD_IOMMU_SUPPORT
	mmu_reg = (uintptr_t) of_iomap(np, 3);	/*0x1400c380 */
	mt_reg_sync_writew(0x0, mmu_reg);
	mt_reg_sync_writew(0x0, mmu_reg + 4);
#endif

	osd_drv->irq[0].value = irq_of_parse_and_map(np, 0);  /* osd3 irq*/
	if (osd_drv->irq[0].value == 0)
		OSD_LOG_E("OSD get irq from dts fail 1\n");
	else {
		osd_drv->irq[0].irq = DISP_IRQ_OSD3_VSYNC;
		osd_drv->irq_num = 1;
	}

	OSD_LOG_D("osd reg base:0x%p,0x%p,0x%p\n", (void *)osd.osd_reg.osd_fmt_reg_base[0],
		(void *)osd.osd_reg.osd_fmt_reg_base[1], (void *)osd.osd_reg.osd_pmx_reg_base);
	return OSD_RET_OK;
}

long int got_current_time_us(void)
{
	struct timeval t;

	do_gettimeofday(&t);
	return (t.tv_sec & 0xFFF) * 1000000 + t.tv_usec;
}

static int disp_osd_irq_handle(uint32_t irq)
{
	if (irq == DISP_IRQ_FMT_VSYNC) {
		if (vsync_cnt == 0xFFFFFFFFFFFFFFFF)
			vsync_cnt = 0;
		else
			vsync_cnt++;

		if (osd.osd_in_suspend[OSD_PLANE_1] &&  osd.osd_in_suspend[OSD_PLANE_2])
			return OSD_RET_OK;

		if (!osd.osd_initial)
			return OSD_RET_OK;

		/*check trigger setting(atomic)*/
		/*set config vsync cnt according to trigger info*/
		if (trigger[OSD_PLANE_1])
			osd_pla_non_shadow_update(OSD_PLANE_1);
		if (trigger[OSD_PLANE_2])
			osd_pla_non_shadow_update(OSD_PLANE_2);

		/*disp_osd_update_register();*/
		if (fg_osd_always_update_frame[OSD_PLANE_1] > 0)
			_OSD_AlwaysUpdateReg(OSD_PLANE_1, 1);

		if (fg_osd_always_update_frame[OSD_PLANE_2] > 0)
			_OSD_AlwaysUpdateReg(OSD_PLANE_2, 1);

		atomic_set(&osd_irq_event[OSD_PLANE_1], 1);
		wake_up_interruptible(&osd_irq_wq[OSD_PLANE_1]);

		atomic_set(&osd_irq_event[OSD_PLANE_2], 1);
		wake_up_interruptible(&osd_irq_wq[OSD_PLANE_2]);

		/*clear osd3 plane irq flag*/
		atomic_set(&osd.irq_res_flag, 0);
		atomic_set(&osd.config_cmd_count, 0);
		trigger[OSD_PLANE_1] = false;
		trigger[OSD_PLANE_2] = false;
	} else if (irq == DISP_IRQ_OSD3_VSYNC) {
		/*wake up config thread*/
		if (vsync_cnt1 != vsync_cnt) {
			atomic_set(&osd.irq_res_flag, 1);
	#if 0
			wake_up_interruptible(&osd.irq_res);
	#endif
		}
		vsync_cnt1 = vsync_cnt;
	}

	return OSD_RET_OK;
}

static void osd_remove_update_buffer_list(uint32_t layer)
{
	int i = layer;

	mutex_lock(&(osd.osd_config_queue_lock[i]));
	while (!list_empty(&(OSD_Configed_Head[i]))) {
		Osd_buffer_list *pBuffList = NULL;

		pBuffList = list_first_entry(&OSD_Configed_Head[i],
						 Osd_buffer_list, list);
		if (pBuffList) {
			if (pBuffList->fence_fd > -1) {
				if (pBuffList->fences != NULL)
					sync_fence_put(pBuffList->fences);
			}

			OSD_LOG_I("[%d]rel update fence fd:%d, idx:%d\n", i, pBuffList->fence_fd,
			pBuffList->buffer_info.config.index);
			release_cnt[i]++;
			osd_release_all_fence(i,  pBuffList->buffer_info.config.index,
				pBuffList->buffer_info.config.pre_index);

			if (pBuffList->buffer_info.config.ion_handle != NULL)
				osd_ion_free_handle(osd_ion_client,
										pBuffList->buffer_info.config.ion_handle);

			list_del_init(&(pBuffList->list));
			vfree(pBuffList);
		}
	}
	mutex_unlock(&(osd.osd_config_queue_lock[i]));

}

static void osd_remove_config_buffer_list(uint32_t layer)
{
	int i = layer;

	mutex_lock(&(osd.osd_release_buf_lock[OSD_PLANE_2]));
	while (!list_empty(&(OSD_Buffer_Head[i]))) {
		Osd_buffer_list *pBuffList = NULL;

		pBuffList = list_first_entry(&OSD_Buffer_Head[i],
						 Osd_buffer_list, list);
		if (pBuffList) {
			if (pBuffList->list_buf_state != list_useless) {
				if (pBuffList->fence_fd > -1) {
					if (pBuffList->fences != NULL)
						sync_fence_put(pBuffList->fences);
				}

				OSD_LOG_I("[%d]rel config fence fd:%d, idx:%d\n", i, pBuffList->fence_fd,
				pBuffList->buffer_info.config.index);
				release_cnt[i]++;
				osd_release_all_fence(i,  pBuffList->buffer_info.config.index,
					pBuffList->buffer_info.config.pre_index);

				if (pBuffList->buffer_info.config.ion_handle != NULL)
					osd_ion_free_handle(osd_ion_client,
										pBuffList->buffer_info.config.ion_handle);
			}
			list_del_init(&(pBuffList->list));
			vfree(pBuffList);
		}
	}

	mutex_unlock(&(osd.osd_release_buf_lock[OSD_PLANE_2]));
}


void osd_engine_clk_enable(HDMI_VIDEO_RESOLUTION res_mode, bool enable)
{
	if (enable) {
		/*disp_clock_set_pll(DISP_CLK_OSDPLL, 648000000);*/
		disp_clock_enable(DISP_CLK_OSDPLL, true);
		disp_clock_enable(DISP_CLK_OSD_FHD, true);
		disp_clock_enable(DISP_CLK_OSD_UHD, true);
		disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
		disp_clock_enable(DISP_CLK_OSD_SEL, true);
		disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL);
	} else {
		disp_clock_enable(DISP_CLK_OSDPLL, false);
		disp_clock_enable(DISP_CLK_OSD_SEL, false);
		disp_clock_enable(DISP_CLK_OSD_PREMIX, false);
		disp_clock_enable(DISP_CLK_OSD_UHD, false);
		disp_clock_enable(DISP_CLK_OSD_FHD, false);
	}

}

void osd_clk_enable(HDMI_VIDEO_RESOLUTION res_mode, bool enable, unsigned int layer_id)
{
	if (enable) {
		/* disp_clock_enable(DISP_CLK_SDR2HDR, true); */
		disp_clock_enable(DISP_CLK_OSDPLL, true);
		if (layer_id == OSD_PLANE_2) {
			disp_clock_smi_larb_en(DISP_SMI_LARB4, true);
			disp_clock_enable(DISP_CLK_OSD_FHD, true);
		} else {
			disp_clock_smi_larb_en(DISP_SMI_LARB0, true);
			disp_clock_enable(DISP_CLK_OSD_UHD, true);
		}
		disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
		disp_clock_enable(DISP_CLK_OSD_SEL, true);
	#if 0
		if (res_mode <= HDMI_VIDEO_1920x1080p_50Hz)
			disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL_D2);
		else
			disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL);
	#endif
		disp_clock_select_pll(DISP_CLK_OSD_SEL, DISP_CLK_OSDPLL);
	} else {
		disp_clock_enable(DISP_CLK_OSDPLL, false);
		disp_clock_enable(DISP_CLK_OSD_SEL, false);
		disp_clock_enable(DISP_CLK_OSD_PREMIX, false);
		if (layer_id == OSD_PLANE_2) {
			disp_clock_smi_larb_en(DISP_SMI_LARB4, false);
			disp_clock_enable(DISP_CLK_OSD_FHD, false);
		} else {
			disp_clock_smi_larb_en(DISP_SMI_LARB0, false);
			disp_clock_enable(DISP_CLK_OSD_UHD, false);
		}
		/* disp_clock_enable(DISP_CLK_SDR2HDR, false); */
	}

}

static int osd_init(struct disp_hw_common_info *info)
{
	UINT32 u4Plane = 0;
	struct sched_param param = {2};
	struct sched_param irq_param = { .sched_priority = MAX_RT_PRIO - 2 };

#if	CONFIG_OSD_TEST
	INT32 ret = 0;
#endif
	_osd_parse_dev_node();
	_OSD_BASE_GET_REG_BASE(osd.osd_reg.osd_fmt_reg_base, osd.osd_reg.osd_pmx_reg_base);
	_OSD_PLA_GET_REG_BASE(osd.osd_reg.osd_pln_reg_base);
	_OSD_SC_GET_REG_BASE(osd.osd_reg.osd_scl_reg_base);

	for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++) {
		osd.res_chg[u4Plane].osd_res_mode = info->resolution->res_mode;
		osd.res_chg[u4Plane].cur_res_mode = info->resolution->res_mode;
		osd.res_chg[u4Plane].fg_need_change_res = false;
		osd.res_chg[u4Plane].height = info->resolution->height;
		osd.res_chg[u4Plane].width = info->resolution->width;
		osd.res_chg[u4Plane].htotal = info->resolution->htotal;
		osd.res_chg[u4Plane].vtotal = info->resolution->vtotal;
		osd.res_chg[u4Plane].is_hd = info->resolution->is_hd;
		osd.res_chg[u4Plane].is_progressive = info->resolution->is_progressive;
		osd.update_tl_idx[u4Plane] = -1;

		init_waitqueue_head(&osd.res_chg[u4Plane].event_res);
		atomic_set(&osd.res_chg[u4Plane].event_res_flag, 0);

		/*init res change event*/
		init_waitqueue_head(&osd.suspend_event_res[u4Plane]);
		atomic_set(&osd.suspend_event_res_flag[u4Plane], 0);

		mutex_init(&(osd.osd_queue_lock[u4Plane]));
		mutex_init(&(osd.osd_config_queue_lock[u4Plane]));
		mutex_init(&(osd.osd_release_buf_lock[u4Plane]));

		mutex_init(&osd.stop_sync_lock[u4Plane]);
		mutex_init(&osd.config_setting_lock[u4Plane]);

		osd.osd_stop_ctl.osd_stop_pts[u4Plane] = 0;
		osd.update_tl_idx[u4Plane] = 0;
		osd.plug_out[u4Plane].fg_hdmi_plug_out = false;
	}

	init_waitqueue_head(&osd.irq_res);
	atomic_set(&osd.irq_res_flag, 0);

	/* clk manager*/
	/*osd_engine_clk_enable(info->resolution->res_mode, true);*/
	/*default use FHD-OSD*/
	osd_clk_enable(info->resolution->res_mode, true, OSD_PLANE_2);
	osd_clk_enable(info->resolution->res_mode, true, OSD_PLANE_1);

	osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_1] = OSD_LAYER_START;
	osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_2] = OSD_LAYER_START;

	/*todo: base0, base1, fastlogo layer get reg from hw setting,  the other layer still need reset*/
	#if CONFIG_DRV_FAST_LOGO
	/*enter driver ,default not change resolution, res should be same as fastlogo res*/
	/*osd1:get osd info from hw setting*/
		for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++) {
			_OSD_BASE_SetReg(u4Plane, NULL);
			_OSD_PLA_SetReg(u4Plane, NULL);
			OSD_PLA_Reset(u4Plane);
		}

		/*todo: fastlogo layer(UHD do not need res change.)*/
		/*todo: FHD layer need be set to the same res info,*/
		/*incase this layer been used without any res change */
		i4Osd_BaseSetFmt(OSD_PLANE_1, OSD_MAIN_PATH, &(osd.res_chg[OSD_PLANE_1]), true);
	#else
		for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++) {
			_OSD_BASE_SetReg(u4Plane, NULL);
			i4Osd_BaseSetFmt(u4Plane, OSD_MAIN_PATH, &(osd.res_chg[u4Plane]), true);
			OSD_PLA_Reset(u4Plane);
			OSDDBG("OSD_PLA_Reset %d\n", u4Plane);
		}
	#endif
	/*premix config*/
	osd_premix_config(&(osd.res_chg[OSD_PLANE_2]));
	_OSD_BASE_SetPremixUpdate(OSD_PLANE_2, 1);
	_OSD_PLA_SetUpdateStatus(OSD_PLANE_2, 1);
	osd_plane_set_extend_update(OSD_PLANE_1, 1);
	osd_plane_set_extend_update(OSD_PLANE_2, 1);
	fgupdate[OSD_PLANE_1] = true;
	fgupdate[OSD_PLANE_2] = true;

#if 0
	for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++) {
		OSD_PLA_Reset(u4Plane);
		OSD_LOG_I("OSD_PLA_Reset %d\n", u4Plane);
	}
	for (u4Scaler = OSD_SCALER_1; u4Scaler < OSD_SCALER_MAX_NUM; u4Scaler++) {
		IGNORE_RET(OSD_SC_Scale(u4Scaler, true, 0, 0, 0, 0));
		IGNORE_RET(OSD_SC_SetLpf(u4Scaler, false));
		/* _OSD_SC_SetReg(u4Scaler, NULL); */
		OSD_LOG_I("_OSD_SC_SetReg %d\n", u4Scaler);
	}
	fgupdate = true;
#endif
	osd_region_init(); /*todo: new region */
	OSDDBG("osd_region_init done\n");

	osd_ion_init();

	for (u4Plane = 0; u4Plane < MAX_OSD_INPUT_CONFIG; u4Plane++) {
		osd_sync_init(u4Plane);
		INIT_LIST_HEAD(&(OSD_Buffer_Head[u4Plane]));
		INIT_LIST_HEAD(&(OSD_Configed_Head[u4Plane]));
	}

	disp_osd_debug_init();
	disp_osd_dim_layer_buffer_init();

	init_waitqueue_head(&osd_config_wq);
	if (!osd_config_task) {
		osd_config_task = kthread_create(disp_osd_config_kthread,
					 (void *)&(osd_layer[OSD_PLANE_2]), "disp_osd2_config_kthread");
		wake_up_process(osd_config_task);
		OSDDBG("kthread_create disp_osd2_engine_kthread\n");
	}
	sched_setscheduler(osd_config_task, SCHED_RR, &param);

	init_waitqueue_head(&osd_irq_wq[OSD_PLANE_1]);
	if (!osd_irq_task[OSD_PLANE_1]) {
		osd_irq_task[OSD_PLANE_1] = kthread_create(disp_osd_irq_kthread,
					 (void *)&(osd_layer[OSD_PLANE_1]), "disp_osd1_irq_kthread");
		wake_up_process(osd_irq_task[OSD_PLANE_1]);
		OSDDBG("kthread_create disp_osd1_irq_kthread\n");
	}
	sched_setscheduler(osd_irq_task[OSD_PLANE_1], SCHED_RR, &irq_param);

	init_waitqueue_head(&osd_irq_wq[OSD_PLANE_2]);
	if (!osd_irq_task[OSD_PLANE_2]) {
		osd_irq_task[OSD_PLANE_2] = kthread_create(disp_osd_irq_kthread,
					 (void *)&(osd_layer[OSD_PLANE_2]), "disp_osd2_irq_kthread");
		wake_up_process(osd_irq_task[OSD_PLANE_2]);
		OSDDBG("kthread_create disp_osd2_irq_kthread\n");
	}
	sched_setscheduler(osd_irq_task[OSD_PLANE_2], SCHED_RR, &irq_param);

	osd_stop_config = vmalloc(sizeof(struct mtk_disp_buffer));
	memset(osd_stop_config, 0, sizeof(struct mtk_disp_buffer));

	osd.osd_in_suspend[OSD_PLANE_1] = false;
	osd.osd_in_suspend[OSD_PLANE_2] = false;
	fg_osd_alpha_detect_en =  false;
	osd.osd_swap = false;
	osd.osd_initial = true;
	fg_osd_always_update_frame[OSD_PLANE_1] = 2;
	fg_osd_always_update_frame[OSD_PLANE_2] = 2;
	fg_update_after_init =  true;

	OSDDBG("osd_init done\n");
	return OSD_RET_OK;
}

static int osd_uninit(void)
{
	unsigned int u4Plane = 0;

	OSD_LOG_I("disp_osd_uninit\n");
	disp_osd_debug_deinit();

	_osd_region_uninit();
	osd_dim_layer_uninit();
	vfree(osd_stop_config);

	/*make sure all command be done!*/

	for (u4Plane = 0; u4Plane < MAX_OSD_INPUT_CONFIG; u4Plane++) {
		osd_sync_destroy(u4Plane);
		osd.res_chg[u4Plane].fg_need_change_res = false;
		osd_clk_enable(osd.res_chg[u4Plane].osd_res_mode, false, u4Plane);
		fgupdate[u4Plane] = false;
	}

	return OSD_RET_OK;

}

uint32_t osd_config_cnt[2];
uint32_t osd_update_cnt[2];
unsigned long osd_start_vsync;
void disp_osd_vsync_stastic(unsigned int idx)
{
	if (idx == 1) {
		osd_config_cnt[0] = 0;
		osd_update_cnt[0] = 0;
		osd_config_cnt[1] = 0;
		osd_update_cnt[1] = 0;
		osd_start_vsync = vsync_cnt;
	}

	if (idx == 2) {
		OSD_LOG_I("osd[%ld-%ld]:%d,%d\n", osd_start_vsync, vsync_cnt,
			osd_config_cnt[0], osd_update_cnt[0]);

		OSD_LOG_I("osd[%ld-%ld]:%d,%d\n", osd_start_vsync, vsync_cnt,
			osd_config_cnt[1], osd_update_cnt[1]);
	}
}

void osd_release_buffer(unsigned int layer_id)
{
	/*new buffer comes, free last buffer*/
	/*stop free last buffer*/
	/*no new buffer, keep last buffer*/
	uint32_t i = layer_id;

	mutex_lock(&(osd.osd_config_queue_lock[i]));

	if (!list_empty(&(OSD_Configed_Head[i]))) {
		Osd_buffer_list *pBuffList = NULL;
		Osd_buffer_list *pBuffList_Temp = NULL;

		list_for_each_entry_safe(pBuffList, pBuffList_Temp, &(OSD_Configed_Head[i]), list) {
			if (pBuffList->list_buf_state == list_updated) {
				if ((pBuffList->buffer_info.config.index < (osd.update_tl_idx[i])) ||
					(osd.update_tl_idx[i] == -1)) {
					if (pBuffList->buffer_info.config.index != -1)
						osd_signal_fence(i, pBuffList->buffer_info.config.index);
					/*else*/
						/*osd_signal_fence(i, osd.update_tl_idx[i]);*/
					release_cnt[i]++;
					OSDDBG("free idx[%ld]:%d; cnt: %ld:%ld\n", vsync_cnt,
					pBuffList->buffer_info.config.index, config_cnt[i], release_cnt[i]);

					if (pBuffList->buffer_info.config.ion_handle != NULL)
						osd_ion_free_handle(osd_ion_client,
							pBuffList->buffer_info.config.ion_handle);

					if (pBuffList->fence_fd > -1) {
					if (pBuffList->fences != NULL)
						sync_fence_put(pBuffList->fences);
					}

					list_del_init(&(pBuffList->list));
					vfree(pBuffList);
					pBuffList = NULL;
				}
			}

		}
	}
	mutex_unlock(&(osd.osd_config_queue_lock[i]));
}

int disp_osd_irq_kthread(void *data)
{
	int wait_ret = 0;
	int i = 0;
	uint32_t plane_id = *(uint32_t *)data;

#if 0
	bool is_last_cmd = false;
	is_need_force_free_cmd = false;
#endif

	while (1) {
		wait_ret = wait_event_interruptible(osd_irq_wq[plane_id], atomic_read(&osd_irq_event[plane_id]));
		atomic_set(&osd_irq_event[plane_id], 0);

		_OSD_AlwaysUpdateReg(plane_id, 0);

		if (fg_osd_always_update_frame[i] > 0)
			fg_osd_always_update_frame[i]--;

		if (osd.osd_in_suspend[plane_id]) { /*not handle cmd  before resume*/
			continue;
		}

		if (osd.res_chg[plane_id].fg_need_change_res) {
			if (osd.res_chg[plane_id].fg_res_vsync3) {
				_Osd_ReleaseReset_Plane(plane_id);
				osd.res_chg[plane_id].cur_res_mode = osd.res_chg[plane_id].osd_res_mode;
				/*inform pmx/hdmi: osd resolution change done! */
				osd_remove_config_buffer_list(plane_id);
				osd_remove_update_buffer_list(plane_id);
				osd.res_chg[plane_id].fg_need_change_res = false;
				atomic_set(&osd.res_chg[plane_id].event_res_flag, 1);
				wake_up(&osd.res_chg[plane_id].event_res);
				osd.res_chg[plane_id].fg_res_vsync3 = false;
				osd.osd_current_frame[i] = NULL;
			}

			if (osd.res_chg[plane_id].fg_res_vsync2) {
				osd_clk_enable(osd.res_chg[plane_id].osd_res_mode, true, plane_id);
				osd_clk_enable(osd.res_chg[plane_id].cur_res_mode, false, plane_id);
				/*todo: osd base change*/
				i4Osd_BaseSetFmt(plane_id, OSD_MAIN_PATH, &(osd.res_chg[plane_id]), true);
				/*todo:add osd premix setting*/
				osd_premix_config(&(osd.res_chg[plane_id]));
				_OSD_BASE_SetPremixUpdate(plane_id, 1);
				osd.res_chg[plane_id].fg_res_vsync3 = true;
				osd.res_chg[plane_id].fg_res_vsync2 = false;
				fg_osd_always_update_frame[i] = 3;
				fgupdate[plane_id] = true;
			}

			if (osd.res_chg[plane_id].fg_res_vsync1) {
				OSD_PLA_DisableAllPlane(plane_id);
				osd.res_chg[plane_id].fg_res_vsync2 = true;
				osd.res_chg[plane_id].fg_res_vsync1 = false;
				fgupdate[plane_id] = true;
			}

			/*goto update;*/
		}

		i = plane_id;
		{
			find_current[i] = false;
			/*get next and current buffer*/
			mutex_lock(&(osd.osd_config_queue_lock[i]));
			if (!list_empty(&(OSD_Configed_Head[i]))) {
				Osd_buffer_list *pBuffList = NULL;

				list_for_each_entry(pBuffList, &OSD_Configed_Head[i], list) {
				if (pBuffList->list_buf_state == list_configed) {
					if (pBuffList->vsync_cnt < vsync_cnt) {
						pBuffList->list_buf_state = list_updated;
					/*if (pBuffList->buffer_info.config.index != -1)*/
							osd.update_tl_idx[i] = pBuffList->buffer_info.config.index;
						find_current[i] = true;
						osd.osd_current_frame[i] = pBuffList;
						OSDDBG("%d:[%ld]osd current idx:%d cv:%ld\n", i, vsync_cnt,
						osd.update_tl_idx[i], pBuffList->vsync_cnt);
						if (pBuffList->buffer_info.config.pre_index != -1)
							osd_signal_pre_fence(i, pBuffList->buffer_info.config.pre_index);
						OSDDBG("free pre-fence idx[%ld]:%d", vsync_cnt, pBuffList->buffer_info.config.pre_index);
					} else {
						OSDDBG("%d:[%ld] idx:%d config vsync:%ld\n", i, vsync_cnt,
							pBuffList->buffer_info.config.index, pBuffList->vsync_cnt);
					}
				}
				}
			}
			mutex_unlock(&(osd.osd_config_queue_lock[i]));

	#if 0
			if ((osd.update_tl_idx[i] ==
					osd.osd_stop_ctl.osd_stop_rel_fence_idx[i]) ||
				(osd.osd_stop_ctl.osd_stop_rel_fence_idx[i] == -1)) {
				is_last_cmd = true;
			} else {
				is_last_cmd = false;
			}
	#endif

			if (find_current[i])
				osd_release_buffer(i);

	#if 0
		if (is_need_force_free_cmd) {
			osd.update_tl_idx[i] = osd.update_tl_idx[i] + 1;
			osd_release_buffer(i);
			is_need_force_free_cmd = false;
			osd.update_tl_idx[i] = osd.update_tl_idx[i] - 1;
		}
	#endif

			if (true) {
				mutex_lock(&osd.stop_sync_lock[i]);
				if (osd.osd_stop_ctl.osd_stop_status[i] == OSD_LAYER_STOPPING) {
					if (osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_2] == OSD_LAYER_STOPPED) {
						osd_clk_enable(osd.res_chg[i].cur_res_mode, false, i);
						osd.osd_stop_ctl.osd_stop_status[i] = OSD_LAYER_STOPPED;
						OSD_LOG_D("[%ld]osd clk disable\n", vsync_cnt);
					}

					if (i == OSD_PLANE_2) {
						osd_clk_enable(osd.res_chg[i].cur_res_mode, false, i);
						osd.osd_stop_ctl.osd_stop_status[i] = OSD_LAYER_STOPPED;
						OSD_LOG_D("[%ld]osd clk disable\n", vsync_cnt);
					}
					osd.osd_stop_ctl.osd_stop_pts[i] = 0;
				}
				if (osd.osd_stop_ctl.osd_stop_status[i] ==	OSD_LAYER_DISABLE) {
					if (osd.osd_stop_ctl.osd_stop_pts[i]++ > 30)
						osd.osd_stop_ctl.osd_stop_status[i] = OSD_LAYER_STOPPING;
				#if 0
					is_need_force_free_cmd =  true;
					is_stop_plane_done = true;
				#endif
				}

				if (osd.osd_stop_ctl.osd_stop_status[i] ==	OSD_LAYER_STOP) {
					osd.osd_stop_ctl.osd_stop_status[i] = OSD_LAYER_DISABLE;
					osd.osd_stop_ctl.osd_stop_pts[i] = 1;
					/*osd_plane_enable(i, false);*/ /*plane disable not finish before next vsync*/
					/*OSD_LOG_D("[%ld]0-osd list empty i =%d\n", vsync_cnt, i);*/
					/*fgupdate[i] = true;*/
				}

#if 0
				/*in case: status already change to start, but cmd still handle stop cmd*/
				if (osd.osd_stop_ctl.osd_stop_status[i] ==	OSD_LAYER_START &&
						is_stop_plane_done == false) {
					osd_plane_enable(i, false);
					OSD_LOG_D("[%ld]1-osd list empty i =%d\n", vsync_cnt, i);
					fgupdate_ex[i] = true;
					is_need_force_free_cmd =  true;
					is_stop_plane_done = true;
				}
#endif
				mutex_unlock(&osd.stop_sync_lock[i]);
			}

			if (find_current[i] == false) {
				if (osd.osd_stop_ctl.osd_stop_status[i] ==	OSD_LAYER_START)
					osd_reassemble_multi_region(i);
			}
		}

		if (osd.osd_try_in_suspend[i] == 2) {
			atomic_set(&osd.suspend_event_res_flag[i], 1);
			wake_up(&osd.suspend_event_res[i]);
			osd.osd_try_in_suspend[i]++;
		}

		if (osd.osd_try_in_suspend[i] == 1) {
			osd_plane_enable(i, false);
			fgupdate[i] = true;
			OSD_LOG_D("osd_try_in_suspend close osd plane %d\n", i);
			osd.osd_try_in_suspend[i]++;
			osd.osd_current_frame[i] = NULL;
		}

		if (osd.plug_out[i].fg_hdmi_plug_out) {
			osd_remove_config_buffer_list(i);
			osd_remove_update_buffer_list(i);
			osd.osd_current_frame[i] = NULL;
			if (OSD_Is_PLA_Enabled(i)) {
				osd_plane_enable(i, false);
				OSD_LOG_D("hdmi plug out release buf %d", i);
				fgupdate[i] = true;
			} else {
				fgupdate[i] = false;
			}
		}

#if 0
		if (fgupdate[i]) {
			osd.drop_check_info.last_buffer_addr[i] = 0;
			osd.drop_check_info.last_alpha_en[i] = 0;
			osd.drop_check_info.last_alpha[i] = 0;
		}
#endif

		mutex_lock(&osd.config_setting_lock[plane_id]);
		disp_osd_update_register(plane_id);
		/*disp_osd_update_register_for_alpha_det(plane_id);*/
		mutex_unlock(&osd.config_setting_lock[plane_id]);

		if (kthread_should_stop())
			break;

	}
	return 0;
}

int disp_osd_config_kthread(void *data)
{
	int wait_ret = 0;
#if 0
	int wait_plane_ret = 0;
#endif
	int i = 0;

	while (1) {
		wait_ret = wait_event_interruptible(osd_config_wq, atomic_read(&osd_config_event));
		atomic_set(&osd_config_event, 0);

		/*wait 2 layer fence*/
		while (((!osd .osd_swap &&!list_empty(&(OSD_Buffer_Head[OSD_PLANE_2]))) ||
				(osd .osd_swap && !list_empty(&(OSD_Buffer_Head[OSD_PLANE_1])))) &&
					!(osd.res_chg[OSD_PLANE_2].fg_need_change_res)) {

			mutex_lock(&(osd.osd_release_buf_lock[OSD_PLANE_2]));
			for (i = OSD_PLANE_2; i >= OSD_PLANE_1; i--) {
				if (!list_empty(&(OSD_Buffer_Head[i]))) {
					Osd_buffer_list *pBuffList = NULL;

					pBuffList = list_first_entry(&(OSD_Buffer_Head[i]), Osd_buffer_list, list);
					if (pBuffList) {
					if (pBuffList->list_buf_state == list_new) {
						OSDDBG("vsync%ld:[%d]wait handle fence idx=%d\n",
							vsync_cnt, i, pBuffList->buffer_info.config.index);
					if (osd_config_sw_register(
						&(pBuffList->buffer_info.config)) < 0) {
						OSD_LOG_E("osd_config_sw_register Failed\n");
						osd_reset_plane_region_state(
						pBuffList->buffer_info.config.layer_id);
					if (pBuffList->buffer_info.config.sbs != 0)
						osd_reset_plane_region_state(
							pBuffList->buffer_info.config.layer_id + 1);
					}

					if ((pBuffList->fence_fd) > -1) {
					if (pBuffList->fences != NULL) {
						/*osd wait acquire fence */
					if (sync_fence_wait(pBuffList->fences, 1000) < 0) {
						OSD_LOG_I("vsync%ld, %d wait fence nxt vsync\n", vsync_cnt, i);
						/*continue;*/
						}
					} else {
						OSDERR("get fence from fd fail\n");
					}
					}

					pview[i] = pBuffList;
							#if 0
							if (pBuffList->buffer_paired == 0)
								break;
							#endif
						}
					}/*pBufferList*/
				} else {
					pview[i] = NULL;
				}
			}


			/*debug*/
	#if 0
			if (osd_pla_get_osd_en_effect(OSD_PLANE_2)) {
				wait_plane_ret = wait_event_interruptible_timeout(osd.irq_res,
					atomic_read(&osd.irq_res_flag), msecs_to_jiffies(45));
				atomic_set(&osd.irq_res_flag, 0);
				if (wait_plane_ret < 0)
					OSD_LOG_E("wait plane cnt timeout\n");
			}
	#endif
			if ((atomic_read(&osd.config_cmd_count) > 0) &&
				atomic_read(&osd.irq_res_flag) == 1) {
				usleep_range(400, 700);
			}

			for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
				if (pview[i] != NULL) {
					mutex_lock(&osd.config_setting_lock[i]);
					fg_config_update[i] = true;
					disp_osd_update_register(i);
					mutex_unlock(&osd.config_setting_lock[i]);
				}
			}

			for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
				if (pview[i] != NULL) {
					disp_osd_trigger_hw(i);
					pview[i]->list_buf_state = list_configed;
					pview[i]->vsync_cnt = vsync_cnt;
					config_cnt[i]++;
					trigger[i] = true;
				}
			}

			for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
				if (pview[i] != NULL) {
					mutex_lock(&(osd.osd_queue_lock[i]));
					list_del_init(&(pview[i]->list));
					mutex_unlock(&(osd.osd_queue_lock[i]));

					mutex_lock(&(osd.osd_config_queue_lock[i]));
					list_add_tail(&(pview[i]->list), &(OSD_Configed_Head[i]));
					mutex_unlock(&(osd.osd_config_queue_lock[i]));
					pview[i] = NULL;
				}
			}


			/*trigger and record this vsync's record info*/
			mutex_unlock(&(osd.osd_release_buf_lock[OSD_PLANE_2]));
			atomic_add(1, &osd.config_cmd_count);
		}

		if (kthread_should_stop())
			break;

	}

	return 0;
}

int osd_color_fmt_remap(enum DISP_HW_COLOR_FORMAT disp_clr_fmt, uint32_t *osd_fmt, uint32_t *fmt_order)
{
	if (disp_clr_fmt == DISP_HW_COLOR_FORMAT_ARGB8888) {
		*osd_fmt = OSD_CM_ARGB8888_DIRECT32;
		*fmt_order = 0;
	} else if (disp_clr_fmt == DISP_HW_COLOR_FORMAT_ABGR8888) {
		*osd_fmt = OSD_CM_ARGB8888_DIRECT32;
		*fmt_order = 1;
	} else if (disp_clr_fmt == DISP_HW_COLOR_FORMAT_RGBA8888) {
		*osd_fmt = OSD_CM_ARGB8888_DIRECT32;
		*fmt_order = 2;
	} else if (disp_clr_fmt == DISP_HW_COLOR_FORMAT_BGRA8888) {
		*osd_fmt = OSD_CM_ARGB8888_DIRECT32;
		*fmt_order = 3;
	} else if (disp_clr_fmt == DISP_HW_COLOR_FORMAT_RGB565) {
		*osd_fmt = OSD_CM_RGB565_DIRECT16;
		*fmt_order = 1;
	} else {
		return -1;
	}
	return 0;
}

int osd_config_sw_register(struct disp_osd_layer_config *osd_overlay_buffer_info)
{
	INT32 i4Ret = 0;
	UINT32 u4Region;
	UINT32 u4SBS = osd_overlay_buffer_info->sbs;
	BOOL fgValidReg = osd_overlay_buffer_info->layer_enable;
	unsigned int pvBitmap = osd_overlay_buffer_info->src_phy_addr;
	UINT32 u4SrcHeight = osd_overlay_buffer_info->src_height;
	UINT32 u4SrcWidth = osd_overlay_buffer_info->src_width;
	UINT32 u4SrcDispX = osd_overlay_buffer_info->src_offset_x;
	UINT32 u4SrcDispY = osd_overlay_buffer_info->src_offset_y;
	UINT32 u4tgtDispX = osd_overlay_buffer_info->tgt_offset_x;
	UINT32 u4tgtDispY = osd_overlay_buffer_info->tgt_offset_y;
	UINT32 u4tgtWidth = osd_overlay_buffer_info->tgt_width;
	UINT32 u4tgtheight = osd_overlay_buffer_info->tgt_height;
	UINT32 u4src_pitch = osd_overlay_buffer_info->src_pitch;
	/*UINT32 u4RgnWidth =osd_overlay_buffer_info->tgt_width; */
	UINT32 u4Plane = osd_overlay_buffer_info->layer_id; /*osd2, osd3*/
	UINT32 u4Scaler = u4Plane;
	UINT32 u4AlphaEn = osd_overlay_buffer_info->alpha_enable;
	uint32_t alpha = osd_overlay_buffer_info->alpha;
	bool osd_swap = osd_overlay_buffer_info->osd_swap;

	struct ion_handle *ion_handle_1 = osd_overlay_buffer_info->ion_handle;
	unsigned long va;
	unsigned int u4src_fmt, fmt_order;
	uint32_t mix_layer_id;
	uint32_t osd_hstart, osd_vodd, osd_veven;

	OSD_PRINTF(OSD_CONFIG_SW_LOG, "osd_config_sw_register u4Plane=%d ,layer_enable =%d, src_fmt=%x, alpha:%d, alpha_en:%d\n",
		   u4Plane, fgValidReg, osd_overlay_buffer_info->src_fmt, alpha, u4AlphaEn);

	if (fgValidReg) { /*plane enable*/
	if (osd_color_fmt_remap(osd_overlay_buffer_info->src_fmt, &u4src_fmt, &fmt_order) < 0)
		return -2;

	/*cal pitch*/
	if (u4src_pitch == 0) {
		if ((u4src_fmt == (UINT32) OSD_CM_AYCBCR8888_DIRECT32) ||
		    (u4src_fmt == (UINT32) OSD_CM_ARGB8888_DIRECT32))
			u4src_pitch = u4SrcWidth << 2;
		else
			u4src_pitch = u4SrcWidth << 1;
	} else {
		if ((u4src_fmt == (UINT32) OSD_CM_AYCBCR8888_DIRECT32) ||
		    (u4src_fmt == (UINT32) OSD_CM_ARGB8888_DIRECT32))
			u4src_pitch = u4src_pitch << 2;
		else
			u4src_pitch = u4src_pitch << 1;
	}

	if (u4SBS == 1) {
		/*todo: sbs will be fail here*/
		_OSD_BASE_SetUpdate(0, true);
		u4tgtWidth = osd_overlay_buffer_info->tgt_width / 2;
		OSD_PRINTF(OSD_CONFIG_SW_LOG, "u4SBS=%d,u4tgtWidth =%d\n", u4SBS, u4tgtWidth);
		i4Ret = osd_get_free_region(u4Plane + 1, &u4Region);
		i4Ret = _osd_region_reset(u4Region);
		if (i4Ret < 0)
			return i4Ret;

		i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, u4SrcDispY, u4SrcWidth, u4SrcHeight,
					  pvBitmap, u4src_fmt, u4src_pitch,
					  0, 0,
					  u4SrcWidth, u4SrcHeight, u4Plane + 1, 0, u4AlphaEn, alpha, fmt_order);
		osd_set_region_state(u4Plane + 1);
		if (i4Ret < 0)
			return i4Ret;

		i4Ret =
			OSD_SC_Scale(u4Scaler + 1, fgValidReg, u4SrcWidth, u4SrcHeight,
			u4tgtDispX, u4tgtDispY, u4tgtWidth, u4tgtheight);
		if (i4Ret < 0)
			return i4Ret;

		i4Ret = OsdFlipPlaneRightNow(u4Plane + 1, fgValidReg, u4Region, 0);
		if (i4Ret < 0)
			return i4Ret;

	}
	i4Ret = osd_get_free_region(u4Plane, &u4Region);
	i4Ret = _osd_region_reset(u4Region);
	if (i4Ret < 0)
		return i4Ret;

	osd_overlay_buffer_info->region_id = u4Region;

	/*todo: rgn alpha detect control*/
	i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, u4SrcDispY, u4SrcWidth, u4SrcHeight,
				  pvBitmap, u4src_fmt, u4src_pitch,
				  0, 0, u4SrcWidth, u4SrcHeight, u4Plane, 0, u4AlphaEn, alpha, fmt_order);
	if (fg_osd_alpha_detect_en) {
		if (u4SrcHeight > 127) {
			osd_alpha_detect_enable(u4Plane, true, u4SrcHeight);
			osd.rgn.osd_rgn_idx[u4Plane] = u4Region;
		} else {
			osd_alpha_detect_enable(u4Plane, false, u4SrcHeight);
			osd.rgn.osd_rgn_idx[u4Plane] = u4Region;
		}
	}

	#if 0
	if (fg_osd_alpha_detect_en) {
		if (u4SrcHeight > 127) {
			for (i = 0; i < OSD_RGN_REG_NUM - 1; i++) {
				region_height0 = u4SrcHeight / OSD_RGN_REG_NUM;/*there may be a problem*/
				i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, (u4SrcDispY + i*region_height0),
					u4SrcWidth, region_height0, pvBitmap, u4src_fmt, u4src_pitch,
					0, (i * region_height0), u4SrcWidth, region_height0, u4Plane, (i+1),
					u4AlphaEn, alpha, fmt_order);
			}
			region_height1 = (u4SrcHeight - i*region_height0);
			i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, (u4SrcDispY + i*region_height0),
				u4SrcWidth, region_height1, pvBitmap, u4src_fmt, u4src_pitch,
				0, (i * region_height0), u4SrcWidth, region_height1, u4Plane, (i+1),
				u4AlphaEn, alpha, fmt_order);

			osd_alpha_detect_enable(u4Plane, true, u4SrcHeight);
			mutex_lock(&osd.alpha_detect_lock);
			osd.rgn.osd_rgn_idx[u4Plane] = u4Region;
			osd.rgn.osd_multi_region_rdy[u4Plane] = 0;
			mutex_unlock(&osd.alpha_detect_lock);
		} else {
			osd_alpha_detect_enable(u4Plane, false, u4SrcHeight);
			mutex_lock(&osd.alpha_detect_lock);
			osd.rgn.osd_rgn_idx[u4Plane] = u4Region;
			osd.rgn.osd_multi_region_rdy[u4Plane] = 2;
			mutex_unlock(&osd.alpha_detect_lock);
		}
	} else {
		osd_alpha_detect_enable(u4Plane, false, u4SrcHeight);
	}
#endif
	osd_set_region_state(u4Plane);
	if (i4Ret < 0)
		return i4Ret;

	i4Ret = OSD_SC_Scale(u4Scaler, fgValidReg, u4SrcWidth, u4SrcHeight,
			u4tgtDispX, u4tgtDispY, u4tgtWidth, u4tgtheight);
	if (i4Ret < 0)
		return i4Ret;

	if (u4Plane == OSD_PLANE_2)
		osd_premix_window_config(osd_overlay_buffer_info);

	if (u4Plane == OSD_PLANE_1)
		mix_layer_id = DISP_PATH_OSD1;/*UHD*/
	else
		mix_layer_id = DISP_PATH_OSD2;/*FHD*/

	if (osd_swap) {
		osd_pmx_set_swap_1(2);
		osd_pmx_set_swap_2(1);
		/*get active info*/
		disp_path_get_active_zone(mix_layer_id, osd.res_chg[u4Plane].cur_res_mode,
			&osd_hstart, &osd_vodd, &osd_veven);
		/*set active info*/
		if (u4Plane == OSD_PLANE_1)
			_OSD_BASE_SetScrnHStartOsd2(u4Plane, osd_hstart - 2);
		else
			_OSD_BASE_SetScrnHStartOsd3(u4Plane, osd_hstart + 2);
	} else {
		osd_pmx_set_swap_1(1);
		osd_pmx_set_swap_2(2);
		/*get active info*/
		disp_path_get_active_zone(mix_layer_id, osd.res_chg[u4Plane].cur_res_mode,
			&osd_hstart, &osd_vodd, &osd_veven);
		/*set active info*/
		if (u4Plane == OSD_PLANE_1)
			_OSD_BASE_SetScrnHStartOsd2(u4Plane, osd_hstart);
		else
			_OSD_BASE_SetScrnHStartOsd3(u4Plane, osd_hstart);
	}

	if (u4AlphaEn) {
		_OSD_PLA_SetFading(u4Plane, alpha);
		if (alpha != 0xFF) {
			if (u4Plane == OSD_PLANE_1)
				osd_pmx_set_mix2_pre_mul_a(0);
			if (u4Plane == OSD_PLANE_2)
				osd_pmx_set_mix1_pre_mul_a(0);
		} else {
			if (u4Plane == OSD_PLANE_1)
				osd_pmx_set_mix2_pre_mul_a(1);
			if (u4Plane == OSD_PLANE_2)
				osd_pmx_set_mix1_pre_mul_a(1);
		}
	} else {
		_OSD_PLA_SetFading(u4Plane, (uint32_t) 0xFF);
		if (u4Plane == OSD_PLANE_1)
			osd_pmx_set_mix2_pre_mul_a(0);
		if (u4Plane == OSD_PLANE_2)
			osd_pmx_set_mix1_pre_mul_a(0);
	}
	_OSD_BASE_SetPremixUpdate(u4Plane, 1);

	i4Ret = OsdFlipPlaneRightNow(u4Plane, fgValidReg, u4Region, 0);

#if 1
	if (ion_handle_1 != NULL) {
		va = (unsigned long)ion_map_kernel(osd_ion_client, ion_handle_1);
		if (u4Plane == OSD_PLANE_1)
			DISP_MMP_DUMP(MMP_DISP_BITMAP_OSD1, va, u4src_pitch*u4SrcHeight,
			osd_overlay_buffer_info->src_fmt, u4SrcWidth, u4SrcHeight);
		else
			DISP_MMP_DUMP(MMP_DISP_BITMAP_OSD2, va, u4src_pitch*u4SrcHeight,
			osd_overlay_buffer_info->src_fmt, u4SrcWidth, u4SrcHeight);
	}
#endif
	} else {/*plane disable*/
		i4Ret = OsdFlipPlaneRightNow(u4Plane, fgValidReg, 0, 0);
	}
	return i4Ret;
}

int osd_reconfig_sw_register(uint32_t plane_id, uint32_t region_id,
		struct disp_osd_layer_config *osd_overlay_buffer_info)
{
	INT32 i4Ret = 0;
	UINT32 u4Region;
	BOOL fgValidReg = osd_overlay_buffer_info->layer_enable;
	unsigned int pvBitmap = osd_overlay_buffer_info->src_phy_addr;
	UINT32 u4SrcHeight = osd_overlay_buffer_info->src_height;
	UINT32 u4SrcWidth = osd_overlay_buffer_info->src_width;
	UINT32 u4SrcDispX = osd_overlay_buffer_info->src_offset_x;
	UINT32 u4SrcDispY = osd_overlay_buffer_info->src_offset_y;
	UINT32 u4src_pitch = osd_overlay_buffer_info->src_pitch;
	/*UINT32 u4RgnWidth =osd_overlay_buffer_info->tgt_width; */
	UINT32 u4Plane = osd_overlay_buffer_info->layer_id; /*osd2, osd3*/
	UINT32 u4AlphaEn = osd_overlay_buffer_info->alpha_enable;
	uint32_t alpha = osd_overlay_buffer_info->alpha;
	unsigned int u4src_fmt, fmt_order, region_height0, region_height1;
	unsigned int i;

	OSD_PRINTF(OSD_CONFIG_SW_LOG, "osd_config_sw_register u4Plane=%d ,layer_enable =%d, src_fmt=%x\n",
		   u4Plane, fgValidReg, osd_overlay_buffer_info->src_fmt);
	if (osd_color_fmt_remap(osd_overlay_buffer_info->src_fmt, &u4src_fmt, &fmt_order) < 0)
		return -2;

	/*cal pitch*/
	if (u4src_pitch == 0) {
		if ((u4src_fmt == (UINT32) OSD_CM_AYCBCR8888_DIRECT32) ||
		    (u4src_fmt == (UINT32) OSD_CM_ARGB8888_DIRECT32))
			u4src_pitch = u4SrcWidth << 2;
		else
			u4src_pitch = u4SrcWidth << 1;
	} else {
		if ((u4src_fmt == (UINT32) OSD_CM_AYCBCR8888_DIRECT32) ||
		    (u4src_fmt == (UINT32) OSD_CM_ARGB8888_DIRECT32))
			u4src_pitch = u4src_pitch << 2;
		else
			u4src_pitch = u4src_pitch << 1;
	}

	u4Region = region_id;

	/*todo: rgn alpha detect control*/
	if (u4SrcHeight > 127) {
		for (i = 0; i < OSD_RGN_REG_NUM - 1; i++) {
			region_height0 = u4SrcHeight / OSD_RGN_REG_NUM;/*there may be a problem*/
			i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, (u4SrcDispY + i*region_height0),
				u4SrcWidth, region_height0, pvBitmap, u4src_fmt, u4src_pitch,
				0, (i * region_height0), u4SrcWidth, region_height0, u4Plane, (i+1),
				u4AlphaEn, alpha, fmt_order);
		}
		region_height1 = (u4SrcHeight - i*region_height0);
		i4Ret = OSD_RGN_Create_EX(u4Region, u4SrcDispX, (u4SrcDispY + i*region_height0),
			u4SrcWidth, region_height1, pvBitmap, u4src_fmt, u4src_pitch,
			0, (i * region_height0), u4SrcWidth, region_height1, u4Plane, (i+1),
			u4AlphaEn, alpha, fmt_order);

		osd.rgn.osd_multi_region_rdy[u4Plane] = 1;
	} else {
		osd.rgn.osd_multi_region_rdy[u4Plane] = 0;
	}
	return i4Ret;
}


int osd_reassemble_multi_region(uint32_t plane)
{
	unsigned int region_alpha = 0xffffffff;
	unsigned int region_id;
	unsigned int header_id = 0;
	unsigned int sub_header_addr = 0;
	unsigned int i = plane;

	if (fg_osd_alpha_detect_en) {
		/*make sure alpha value is right for current frame*/
		if (osd.osd_current_frame[i] != NULL) {
			/*get  plane region alpha info*/
			osd_pla_get_alpha_region(plane, &region_alpha);
				/* plane multi-region setting*/
			if ((region_alpha & 0xff) == 0xff) {
				return 0;
			} else if ((region_alpha & 0xff) == 0x0) {
				OSDDBG("no region is visible\n");
				/*osd_plane_enable(plane, false);*/ /*no visible region*/
			} else {
				region_id = osd.osd_current_frame[i]->buffer_info.config.region_id;

				osd_reconfig_sw_register(plane, region_id,
					&(osd.osd_current_frame[i]->buffer_info.config));

				if (osd.rgn.osd_multi_region_rdy[i] == 1) {
					OSDDBG("region id is:%x,%d\n", plane, region_id);
					for (i = OSD_RGN_CLUSTER_NUM-1; i > 0; i--) {
					if (region_alpha & (0x1 << (i-1))) {
						header_id = i;

					if (sub_header_addr != 0) {
						_OSD_RGN_SetNextRegion(region_id, i, sub_header_addr);
						_OSD_RGN_SetNextEnable(region_id, i, 1);
					}
						OSDDBG("get region is visible:0x%x\n", sub_header_addr);
						_OSD_RGN_GetAddress(region_id, i, &sub_header_addr);

					}
					}
					/*reflip new rgn to osd plane*/
					OsdFlipPlaneRightNow(plane, true, region_id, header_id);
					fg_alpha_det_update[plane] = true;
				}
			}
		}
	}
	return 0;
}

int disp_osd_update_register(uint32_t plane)
{
	if (!fg_dovi_idk_test) {
		if (fg_config_update[plane]) {
			if (osd.osd_irq_thread_update[plane] == vsync_cnt) {
				OSD_LOG_D("[%ld]osd%d can't rewirte irq setting\n", vsync_cnt, plane);
				fg_config_update[plane] = false;
				return -OSD_RET_UPDATE_FAIL;
			}
		}

		if (fgupdate[plane])
			osd.osd_irq_thread_update[plane] = vsync_cnt;

		if (fg_config_update[plane])
			osd.osd_config_thread_update[plane] = vsync_cnt;

		osd_update_cnt[plane]++;

		/*if alpha detect update, could rewite by config thread*/
		if ((fgupdate[plane] || fg_config_update[plane] || fg_debug_update) || fgupdate_ex[plane]) {
			if (osd.osd_stop_ctl.osd_stop_status[plane] != OSD_LAYER_STOPPED) {
				_OSD_UpdateReg(plane, 0);
				_OSD_BASE_Update(plane);
				OSD_PLA_Update(plane);
				OSD_SC_Update(plane);
				OSD_PREMIX_Update(plane);
				if (!fg_config_update[plane])
					_OSD_UpdateReg(plane, 1);
			}
			fgupdate[plane] = false;
			fgupdate_ex[plane] = false;
			fg_config_update[plane] = false;
			fg_debug_update = false;
			/*m_uptimes++;*/
			/*OSD_PRINTF(OSD_VYSNC_LOG, "osd_update_register m_uptimes=%d\n", m_uptimes);*/
		}

	}
	/*time4 = got_current_time_us(); */
	/*OSD_PRINTF(OSD_VYSNC_LOG, "time3 =%ld time4 =%ld,inter =%ld\n", time3,  time4,  (time4 - time3)); */
	return OSD_RET_OK;
}

void disp_osd_trigger_hw(uint32_t plane)
{
	_OSD_UpdateReg(plane, 1);
}

int disp_osd_update_register_for_alpha_det(uint32_t plane)
{
	if (!fg_dovi_idk_test) {
		if (fg_alpha_det_update[plane]) {
			if (osd.osd_config_thread_update[plane] == vsync_cnt) {
				OSD_LOG_D("osd%d can't set alpha detect setting\n", plane);
				return OSD_RET_OK;
			}

			if (osd.osd_stop_ctl.osd_stop_status[plane] != OSD_LAYER_STOPPED) {
				_OSD_UpdateReg(plane, 0);
				OSD_PLA_Update(plane);
				_OSD_UpdateReg(plane, 1);
			}

			fg_alpha_det_update[plane] = false;

		}
	}
	return OSD_RET_OK;
}

bool osd_drop_command_check(uint32_t plane_id, disp_osd_input_config *buffer_info)
{
	if (buffer_info->config.layer_enable) {
		if (buffer_info->config.res_mode != osd.res_chg[plane_id].cur_res_mode)
			return true;
	}

#if 0
	/* same resolution buffers - check other conditions to drop or not */
	if (osd.drop_check_info.last_buffer_addr[plane_id] == buffer_info->config.src_phy_addr) {
		if ((osd.drop_check_info.last_alpha_en[plane_id] == buffer_info->config.alpha_enable) &&
			(osd.drop_check_info.last_alpha[plane_id] == buffer_info->config.alpha) &&
			(osd.drop_check_info.last_tgt_x_offset[plane_id] == buffer_info->config.tgt_offset_x) &&
			(osd.drop_check_info.last_tgt_y_offset[plane_id] == buffer_info->config.tgt_offset_y) &&
			(osd.drop_check_info.last_tgt_width[plane_id] == buffer_info->config.tgt_width) &&
			(osd.drop_check_info.last_tgt_height[plane_id] == buffer_info->config.tgt_height) &&
			(osd.drop_check_info.last_src_x_offset[plane_id] == buffer_info->config.src_offset_x) &&
			(osd.drop_check_info.last_src_y_offset[plane_id] == buffer_info->config.src_offset_y) &&
			(osd.drop_check_info.last_src_width[plane_id] == buffer_info->config.src_width) &&
			(osd.drop_check_info.last_src_height[plane_id] == buffer_info->config.src_height))
			return true;
		else
			return false;
	}
#endif
	return false;
}

static int disp_mgr_post_osd_buffer(disp_osd_input_config *buffer_info)
{
	int ret = OSD_RET_OK;
	Osd_buffer_list *pBuffList = NULL;
	int value = MTK_OSD_NO_FENCE_FD;
	int value_present = MTK_OSD_NO_FENCE_FD;
	int fenceFd = MTK_OSD_NO_FENCE_FD;
	int pre_fencefd = MTK_OSD_NO_FENCE_FD;

	int i = buffer_info->config.layer_id;

	if (osd_drop_command_check(i, buffer_info)) {
		buffer_info->config.ion_handle = NULL;
		buffer_info->config.layer_enable = false;
		buffer_info->config.acquire_fence_fd = -1;
		OSD_LOG_I("invalid layer res info\n");
	#if 0
		buffer_info->fence_fd = MTK_OSD_NO_FENCE_FD;
		buffer_info->config.release_fence_fd = MTK_OSD_NO_FENCE_FD;
		buffer_info->config.present_fence_fd = MTK_OSD_NO_FENCE_FD;
		osd_ion_free_handle(osd_ion_client, buffer_info->config.ion_handle);
		if (i == 0) {
			osd.osd_layer_paired = 0;
			goto out;
		} else {
			if (osd.osd_layer_paired == 1) {
				pBuffList = vmalloc(sizeof(Osd_buffer_list));
				if (!pBuffList) {
					ret = -OSD_RET_INV_ARG;
					OSD_LOG_E("could not allocate buffer_list\n");
					goto err;
				}
				pBuffList->list_buf_state = list_useless;
				goto insert;
			} else {
				goto trigger;
			}
		}
	#endif
	}

	pBuffList = vmalloc(sizeof(Osd_buffer_list));
	if (!pBuffList) {
		ret = -OSD_RET_INV_ARG;
		OSD_LOG_E("could not allocate buffer_list\n");
		goto err;
	}

	memcpy(&pBuffList->buffer_info, buffer_info, sizeof(disp_osd_input_config));

	if (buffer_info->config.ion_handle != NULL) {
		ret = osd_create_fence(&pre_fencefd, &fenceFd, &value_present, &value, i);
		if (ret < 0)
			goto err;
	}

	OSDDBG("osd_create_fence done %ld: lay:%d, fenceFd=%d, value=%d,%d src=0x%lx\n", vsync_cnt, i,
		fenceFd, value, value_present, (unsigned long)buffer_info->config.src_phy_addr);

	pBuffList->list_buf_state = list_new;
	pBuffList->buffer_info.config.release_fence_fd = fenceFd;
	pBuffList->buffer_info.config.present_fence_fd = pre_fencefd;
	pBuffList->buffer_info.config.index = value;
	pBuffList->buffer_info.config.pre_index = value_present;
	osd.last_cmd_fence_idx[i] = value;
#if 0
	osd.drop_check_info.last_buffer_addr[i] = buffer_info->config.src_phy_addr;
	osd.drop_check_info.last_alpha_en[i] = buffer_info->config.alpha_enable;
	osd.drop_check_info.last_alpha[i] = buffer_info->config.alpha;
	osd.drop_check_info.last_tgt_x_offset[i] = buffer_info->config.tgt_offset_x;
	osd.drop_check_info.last_tgt_y_offset[i] = buffer_info->config.tgt_offset_y;
	osd.drop_check_info.last_tgt_width[i] = buffer_info->config.tgt_width;
	osd.drop_check_info.last_tgt_height[i] = buffer_info->config.tgt_height;
	osd.drop_check_info.last_src_x_offset[i] = buffer_info->config.src_offset_x;
	osd.drop_check_info.last_src_y_offset[i] = buffer_info->config.src_offset_y;
	osd.drop_check_info.last_src_width[i] = buffer_info->config.src_width;
	osd.drop_check_info.last_src_height[i] = buffer_info->config.src_height;
#endif

	if (buffer_info->config.acquire_fence_fd > -1)
		pBuffList->fences = sync_fence_fdget(buffer_info->config.acquire_fence_fd);

	pBuffList->fence_fd = buffer_info->config.acquire_fence_fd;

	buffer_info->config.release_fence_fd = fenceFd;
	buffer_info->config.present_fence_fd = pre_fencefd;
#if 0
	if ((i == 1) && (osd.osd_layer_paired == 1))
		pBuffList->buffer_paired = 1;
	else
		pBuffList->buffer_paired = 0;

	if (i == 0)
		osd.osd_layer_paired = 1;
	else
		osd.osd_layer_paired = 0;

	pBuffList->cmd_cnt = cmd_cnt;
#endif

#if 0
insert:
#endif
	INIT_LIST_HEAD(&pBuffList->list);
	mutex_lock(&(osd.osd_queue_lock[i]));
	list_add_tail(&pBuffList->list, &(OSD_Buffer_Head[i]));
	mutex_unlock(&(osd.osd_queue_lock[i]));

	if ((osd.osd_try_in_suspend[i] != 0) || (osd.res_chg[i].fg_need_change_res)) {
		OSD_LOG_I("osd enter suspend or resolution change :%d\n", (osd.res_chg[i].fg_need_change_res));
		goto out;
	}
#if 0
trigger:
#endif
	goto out;

err:
	OSD_LOG_E("fence_fd=%d failed\n", buffer_info->fence_fd);
	buffer_info->fence_fd = MTK_OSD_NO_FENCE_FD;
	buffer_info->config.release_fence_fd = MTK_OSD_NO_FENCE_FD;
	buffer_info->config.present_fence_fd = MTK_OSD_NO_FENCE_FD;
	vfree(pBuffList);

out:

	return ret;

}

static int osd_set_displaymode(const struct disp_hw_resolution *info)
{
	int ret = -1;
	int i = 0;

	if (osd.res_chg[OSD_PLANE_2].osd_res_mode != info->res_mode) {
		for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
			osd.res_chg[i].fg_need_change_res = true;
			osd.res_chg[i].fg_res_vsync1 = true;
			osd.res_chg[i].fg_res_vsync2 = false;
			osd.res_chg[i].fg_res_vsync3 = false;
			osd.res_chg[i].osd_res_mode = info->res_mode;
			osd.res_chg[i].cur_res_mode = HDMI_VIDEO_RESOLUTION_NUM;
			osd.res_chg[i].frequency = info->frequency;
			osd.res_chg[i].height = info->height;
			osd.res_chg[i].width = info->width;
			osd.res_chg[i].htotal = info->htotal;
			osd.res_chg[i].vtotal = info->vtotal;
			osd.res_chg[i].is_hd = info->is_hd;
			osd.res_chg[i].is_progressive = info->is_progressive;
		}
		ret = (int)wait_event_timeout(osd.res_chg[OSD_PLANE_1].event_res,
			atomic_read(&osd.res_chg[OSD_PLANE_1].event_res_flag), msecs_to_jiffies(150));
			atomic_set(&osd.res_chg[OSD_PLANE_1].event_res_flag, 0);

		/*wait res change event*/
		if (ret == 0)
			OSD_LOG_E("wait osd1 reg change event timeout\n");

		ret = (int)wait_event_timeout(osd.res_chg[OSD_PLANE_2].event_res,
		atomic_read(&osd.res_chg[OSD_PLANE_2].event_res_flag), msecs_to_jiffies(150));
		atomic_set(&osd.res_chg[OSD_PLANE_2].event_res_flag, 0);

		/*wait res change event*/
		if (ret == 0)
			OSD_LOG_E("wait osd2 reg change event timeout\n");
	}

	return OSD_RET_OK;
}

/* test code */
int test_osd_buffer(disp_osd_input_config *buffer_info)
{
	int ret = OSD_RET_OK;

	return ret;
}

void disp_osd_dim_layer_buffer_init(void)
{
	struct device *dev;
	unsigned int size;
	unsigned int width, height;
	dma_addr_t mva_address = 0;
	unsigned long temp_address = 0;
	int i = 0, j = 0;

	dev = disp_hw_mgr_get_dev();
	width = 8;
	height  = 4;
	size = width * height * 4;

	osd.dim_layer_virt_addr = dma_alloc_coherent(dev, size, &mva_address, GFP_KERNEL);
	osd.dim_layer_phy_addr = mva_address;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			temp_address = (unsigned long)(osd.dim_layer_virt_addr + i * width * 4 + j * 4);
			*(uint8_t *)temp_address = 0x00;
			*(uint8_t *)(temp_address + 1) = 0x00;
			*(uint8_t *)(temp_address + 2) = 0x00;
			*(uint8_t *)(temp_address + 3) = 0xFF;
		}
	}

	OSD_LOG_I("dim layer address 0x%p, 0x%x\n", osd.dim_layer_virt_addr, osd.dim_layer_phy_addr);
}

void osd_dim_layer_uninit(void)
{
	unsigned int size = 0;
	struct device *dev;

	size = 8 * 4 * 4;
	dev = disp_hw_mgr_get_dev();
	if (osd.dim_layer_phy_addr)
		dma_free_coherent(dev, size, osd.dim_layer_virt_addr, osd.dim_layer_phy_addr);
}

#ifdef min
#undef min
#define min(x, y)   (x < y ? x : y)
#endif
#ifdef max
#undef max
#define max(x, y)   (x > y ? x : y)
#endif

/*need release fd before return*/
static int disp_osd_config(struct mtk_disp_buffer *config, struct disp_hw_common_info *info)
{
	disp_osd_input_config osd_plane_config = { 0 };
	struct ion_handle *osd_buf_ion_handle = NULL;
	unsigned int mva;
	unsigned int j;
	int ret = OSD_RET_OK;

	/*insert  buffer to queue */
	if (config->type == DISP_LAYER_OSD) {
		if (osd.plug_out[OSD_PLANE_1].fg_hdmi_plug_out || osd.plug_out[OSD_PLANE_2].fg_hdmi_plug_out) {
			//OSD_LOG_I("osd drop UI during hdmi plug out layer_enable=%d\n", config->layer_enable);
			config->release_fence_fd = MTK_OSD_NO_FENCE_FD;
			config->present_fence_fd = MTK_OSD_NO_FENCE_FD;
			goto error;
		}

		if (config->layer_id > MAX_OSD_INPUT_CONFIG) {
			OSD_LOG_E("disp osd config lay id error\n");
			ret = -OSD_RET_INV_ARG;
			goto error;
		}

		osd_plane_config.config.layer_id = _osd_plane_map(config->layer_id);
		osd_plane_config.config.layer_enable = config->layer_enable;
		if (config->layer_enable) {
			if (config->buffer_source != DISP_BUFFER_ALPHA) {
				osd_plane_config.config.src_fmt = config->src_fmt;
				osd_plane_config.config.src_pitch = config->src.pitch;
				osd_plane_config.config.src_offset_x = config->src.x;
				osd_plane_config.config.src_offset_y = config->src.y;
				osd_plane_config.config.src_width = config->src.width;
				osd_plane_config.config.src_height = config->src.height;
				osd_plane_config.config.src_phy_addr =
					(unsigned int)((unsigned long)config->src_phy_addr);
				if (info->osd_swap == 1) {
					osd_plane_config.config.osd_swap = true;
					osd.osd_swap = true;
				} else {
					osd.osd_swap = false;
				}

				if (config->ion_fd > 0) {
					osd_buf_ion_handle = osd_ion_import_handle(osd_ion_client, config->ion_fd);
					if (osd_buf_ion_handle == NULL) {
						OSD_LOG_E("disp osd config get ion handle err\n");
						ret = -OSD_RET_INV_ARG;
						goto error;
					}
					osd_ion_phys_mmu_addr(osd_ion_client, osd_buf_ion_handle, &mva);
					if (mva == 0) {
						OSD_LOG_E("disp osd config ion_phys err\n");
						ret = -OSD_RET_INV_ARG;
						goto error;
					}
				} else {
					if (osd_plane_config.config.src_phy_addr < 0) {
						OSD_LOG_E("disp osd config src_phy_addr err\n");
						ret = -OSD_RET_INV_ARG;
						goto error;
					}
					mva = osd_plane_config.config.src_phy_addr;
				}

			} else {
				osd_plane_config.config.src_fmt = DISP_HW_COLOR_FORMAT_RGBA8888;
				osd_plane_config.config.src_pitch = 0;
				osd_plane_config.config.src_offset_x = 0;
				osd_plane_config.config.src_offset_y = 0;
				osd_plane_config.config.src_width = 8;
				osd_plane_config.config.src_height = 4;
				osd_plane_config.config.src_phy_addr = osd.dim_layer_phy_addr;
				mva = osd_plane_config.config.src_phy_addr;
			}

			osd_plane_config.config.ion_handle = osd_buf_ion_handle;
			osd_plane_config.config.src_phy_addr = mva;

			osd_plane_config.config.tgt_offset_x = config->tgt.x;
			osd_plane_config.config.tgt_offset_y = config->tgt.y;
			osd_plane_config.config.tgt_width = config->tgt.width;
			osd_plane_config.config.tgt_height = config->tgt.height;
			osd_plane_config.config.alpha_enable = config->alpha_en;
			osd_plane_config.config.alpha = config->alpha;
			osd_plane_config.config.res_mode = config->res_mode;

			osd_plane_config.config.acquire_fence_fd = config->acquire_fence_fd;

			if (osd.win_cnt > 0) {
				for (j = 0; j < osd.win_cnt; j++) {
					uint32_t win_x = max(osd.win[j].x, config->tgt.x);
					uint32_t win_y = max(osd.win[j].y, config->tgt.y);
					uint32_t win_x_end = min((config->tgt.x + config->tgt.width - 1),
						(osd.win[j].x + osd.win[j].width - 1));
					uint32_t win_y_end = min((config->tgt.y + config->tgt.height - 1),
						(osd.win[j].y + osd.win[j].height - 1));

					if ((win_x_end > win_x) && (win_y_end > win_y)) {
						osd_plane_config.config.win[j].active = true;
						osd_plane_config.config.win[j].x = win_x - config->tgt.x;
						osd_plane_config.config.win[j].y = win_y - config->tgt.y;
						osd_plane_config.config.win[j].width = win_x_end - win_x + 1;
						osd_plane_config.config.win[j].height = win_y_end - win_y + 1;
						OSDDBG("win[%d,%d,%d,%d]\n", osd.win[j].x, osd.win[j].y,
							osd.win[j].width, osd.win[j].height);
					}
				}
			}

		} else {
			/*layer disable*/
			osd_plane_config.config.ion_handle = NULL;
			osd_plane_config.config.acquire_fence_fd = -1;
		}

		disp_mgr_post_osd_buffer(&osd_plane_config);

		config->release_fence_fd = osd_plane_config.config.release_fence_fd;
		config->present_fence_fd = osd_plane_config.config.present_fence_fd;

	} else if (config->type == DISP_LAYER_VDP && config->layer_order > 0) {
		/*record win info*/
		if (osd.win_cnt > (OSD_PLANE_MAX_WINDOW-1)) {
			OSD_LOG_E("osd not support win cnt > 2\n");
			ret = -OSD_RET_INV_ARG;
			goto error;
		}

		osd.win[osd.win_cnt].x = config->tgt.x;
		osd.win[osd.win_cnt].y = config->tgt.y;
		osd.win[osd.win_cnt].width = config->tgt.width;
		osd.win[osd.win_cnt].height = config->tgt.height;
		osd.win[osd.win_cnt].active = false;
		osd.win_cnt++;
		OSD_PRINTF(OSD_FLOW_LOG, "vdp layer:%d\n", osd.win_cnt);
	}

error:
	return ret;
}

static int disp_osd_config_ex(struct mtk_disp_config *config, struct disp_hw_common_info *info)
{
	int i = 0;
	int ret = 0;
	int ui_layer_cnt = 0;

	if (config->user == DISP_USER_AVSYNC)
		return 0;

	/*insert stop info when UI layer stopped*/
	for (i = 0; i < 4; i++) {
		if (config->buffer_info[i].layer_enable &&
			(config->buffer_info[i].type == DISP_LAYER_OSD ||
			config->buffer_info[i].type == DISP_LAYER_AEE))
			ui_layer_cnt++;
	}

	for (i = 3; i >= 0; i--) {
		if (ui_layer_cnt == 2) {
			if (config->buffer_info[i].layer_enable)
				disp_osd_config(&config->buffer_info[i], info);
		} else if (ui_layer_cnt == 1) {
			if (config->buffer_info[i].layer_enable) {
				if ((config->buffer_info[i].type == DISP_LAYER_OSD ||
					config->buffer_info[i].type == DISP_LAYER_AEE)) {
					if (config->buffer_info[i].layer_id == 0) {
						/*insert layer 1 stop buffer*/
						osd_stop_config->type = DISP_LAYER_OSD;
						osd_stop_config->layer_enable = false;
						osd_stop_config->layer_id = 1;
						disp_osd_config(osd_stop_config, info);
						/*insert layer 0 buffer*/
						disp_osd_config(&config->buffer_info[i], info);
					} else if (config->buffer_info[i].layer_id == 1) {
						/*insert layer 1 buffer*/
						disp_osd_config(&config->buffer_info[i], info);

						/*insert layer 0 stop buffer*/
						osd_stop_config->type = DISP_LAYER_OSD;
						osd_stop_config->layer_enable = false;
						osd_stop_config->layer_id = 0;
						disp_osd_config(osd_stop_config, info);

					} else {
						OSD_LOG_E("err layer id:%d\n", config->buffer_info[i].layer_id);
					}
				} else {
					disp_osd_config(&config->buffer_info[i], info);
				}
			}
		} else if (ui_layer_cnt == 0) {
			if (osd.osd_stop_ctl.osd_stop_status[0] == OSD_LAYER_STOPPED &&
				osd.osd_stop_ctl.osd_stop_status[1] == OSD_LAYER_STOPPED) {
				goto OUT;
			} else {
				/*insert stop buffer for UI*/
				osd_stop_config->type = DISP_LAYER_OSD;
				osd_stop_config->layer_enable = false;
				osd_stop_config->layer_id = 1;
				disp_osd_config(osd_stop_config, info);

				osd_stop_config->type = DISP_LAYER_OSD;
				osd_stop_config->layer_enable = false;
				osd_stop_config->layer_id = 0;
				disp_osd_config(osd_stop_config, info);
				break;
			}
		} else {
			OSD_LOG_I("ui_layer_cnt = %d error\n", ui_layer_cnt);
		}
	}

	atomic_set(&osd_config_event, 1);
	wake_up_interruptible(&osd_config_wq);

	osd.win_cnt = 0; /*clear win info*/

OUT:
	return ret;
}

int disp_osd_start(struct disp_hw_common_info *info, unsigned int layer_id)
{
	unsigned int osd_layer_id;
	struct disp_hw *osd_drv = disp_osd_get_drv();
	uint32_t dolby_core2_enable = 0;
	uint32_t osd_enable_sdr2hdr = 1;

	osd_layer_id = _osd_plane_map(layer_id);
	/* call dovi to enable core2 clk*/
	dolby_core2_enable = 1;
	osd_drv->drv_call(DISP_CMD_DOVI_CORE2_UPDATE, &dolby_core2_enable);
	osd_drv->drv_call(DISP_CMD_OSD_ENABLE_SDR2HDR_BT2020, &osd_enable_sdr2hdr);
	OSD_PRINTF(OSD_FLOW_LOG, "[%ld]disp_osd_start\n", vsync_cnt);

	mutex_lock(&osd.stop_sync_lock[osd_layer_id]);
	if (osd.osd_stop_ctl.osd_stop_status[osd_layer_id] == OSD_LAYER_STOPPED) {
		osd_clk_enable(osd.res_chg[osd_layer_id].cur_res_mode, true, osd_layer_id);
		fg_osd_always_update_frame[osd_layer_id] = 3;
	}

	if (fg_update_after_init && osd_layer_id == OSD_PLANE_1) {
		fg_osd_always_update_frame[osd_layer_id] = 3;
		fg_update_after_init = false;
	}

	osd.osd_stop_ctl.osd_stop_status[osd_layer_id] = OSD_LAYER_START;
	osd.osd_stop_ctl.osd_stop_pts[osd_layer_id] = 0;
	mutex_unlock(&osd.stop_sync_lock[osd_layer_id]);

	return OSD_RET_OK;
}

int disp_osd_stop(unsigned int layer_id)
{
	unsigned int osd_layer_id;
	struct disp_hw *osd_drv = disp_osd_get_drv();
	uint32_t dolby_core2_enable = 0;
	uint32_t osd_enable_sdr2hdr = 0;
	int ret =  OSD_RET_OK;

	osd_layer_id = _osd_plane_map(layer_id);

	OSD_PRINTF(OSD_FLOW_LOG, "disp_osd_stop in L956\n");

	if (osd.osd_stop_ctl.osd_stop_status[osd_layer_id] == OSD_LAYER_START) {
		OSD_PRINTF(OSD_FLOW_LOG, "disp_osd_stop\n");
		osd.osd_stop_ctl.osd_stop_status[osd_layer_id] = OSD_LAYER_STOP;
		ret = OSD_RET_OK;
	} else {
		OSD_LOG_I("err stop status: %d\n",
			osd.osd_stop_ctl.osd_stop_status[osd_layer_id]);
		ret = -OSD_RET_INV_ARG;
	}
	osd.osd_stop_ctl.osd_stop_rel_fence_idx[osd_layer_id] =
		osd.last_cmd_fence_idx[osd_layer_id];
	is_stop_plane_done = false;

	/* call dovi to disable core2 clk */
	if ((osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_1] != OSD_LAYER_START) &&
		(osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_2] != OSD_LAYER_START)) {
		dolby_core2_enable = 0;
		osd_drv->drv_call(DISP_CMD_DOVI_CORE2_UPDATE, &dolby_core2_enable);
	}

	/* call sdr2hdr to disable osd path clk */
	if ((osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_1] != OSD_LAYER_START) &&
		(osd.osd_stop_ctl.osd_stop_status[OSD_PLANE_2] != OSD_LAYER_START)) {
		osd_enable_sdr2hdr = 0;
		osd_drv->drv_call(DISP_CMD_OSD_ENABLE_SDR2HDR_BT2020, &osd_enable_sdr2hdr);
	}
	return ret;
}

#define ALIGN_TO_256(x)  ((x + 0x255) & ~(0x255))

static int disp_osd_suspend(void)
{
	int i;
	char *osd_suspend_reg_info = NULL;
	unsigned int osd_suspend_reg_info_size = 0;

	osd_suspend_reg_info_size = 0x700;

	osd_suspend_reg_info = vmalloc(osd_suspend_reg_info_size);
	osd.osd_suspend_save_info = osd_suspend_reg_info;

	for (i = 0; i < MAX_OSD_INPUT_CONFIG; i++) {
		memcpy(osd_suspend_reg_info, (void *)osd.osd_reg.osd_fmt_reg_base[i], sizeof(OSD_BASE_UNION_T));
		osd_suspend_reg_info += 0x100;
	}

	memcpy(osd_suspend_reg_info, (void *)(osd.osd_reg.osd_pmx_reg_base), sizeof(OSD_PREMIX_UNION_T));

	/*osd push reset*/
	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		osd.osd_try_in_suspend[i] = 1;
		wait_event(osd.suspend_event_res[i], atomic_read(&osd.suspend_event_res_flag[i]));
		atomic_set(&osd.suspend_event_res_flag[i], 0);
	}

	/*remove all buffer*/
	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		osd_remove_config_buffer_list(i);
		osd_remove_update_buffer_list(i);
	}

	/*osd_engine_clk_enable(osd.res_chg.cur_res_mode, false);*/
	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		mutex_lock(&osd.stop_sync_lock[i]);
		if (osd.osd_stop_ctl.osd_stop_status[i] != OSD_LAYER_STOPPED)
			osd_clk_enable(osd.res_chg[i].cur_res_mode, false, i);
		mutex_unlock(&osd.stop_sync_lock[i]);

		osd.osd_in_suspend[i] = true;
		osd.osd_try_in_suspend[i] = 0;
	}

	OSD_LOG_I("osd suspend 0x%p\n", (void *)osd.osd_suspend_save_info);
	return OSD_RET_OK;
}

static int disp_osd_resume(void)
{
	int i;
	char *osd_suspend_reg_info = NULL;

	OSD_LOG_I("osd resume 0x%p\n", (void *)osd.osd_suspend_save_info);

	osd_suspend_reg_info = osd.osd_suspend_save_info;

	/*restore osd hw  register*/
	for (i = 0; i < MAX_OSD_INPUT_CONFIG; i++) {
		memcpy((void *)(osd.osd_reg.osd_fmt_reg_base[i] + 8), osd_suspend_reg_info + 8,
			sizeof(OSD_BASE_UNION_T) - 8);
		osd_suspend_reg_info += 0x100;
	}

	/*todo: resume osd setting recovery*/
	memcpy((void *)(osd.osd_reg.osd_pmx_reg_base), osd_suspend_reg_info, sizeof(OSD_PREMIX_UNION_T));

	for (i = 0; i < MAX_OSD_INPUT_CONFIG; i++) {
		/*reset full screen info*/
		memset((void *)osd.osd_reg.osd_fmt_reg_base[i], 9, sizeof(uintptr_t));
		i4Osd_BaseSetFmt(i, OSD_MAIN_PATH, &(osd.res_chg[i]), true);
		_OSD_BASE_SetUpdate(i, true);
		_OSD_AlwaysUpdateReg(i, true);
		_OSD_UpdateReg(i, true);
	}

	OSDMSG("0xf0020000:%x,%x,%x,%x\n", *((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[0])),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[0] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[0] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[0] + 0xc)));

	OSDMSG("0xf0020100:%x,%x,%x,%x\n", *((unsigned int *)(osd.osd_reg.osd_pln_reg_base[OSD_PLANE_1])),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[OSD_PLANE_1] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[OSD_PLANE_1] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[OSD_PLANE_1] + 0xc)));

	OSDMSG("0xf0020400:%x,%x,%x,%x\n", *((unsigned int *)(osd.osd_reg.osd_scl_reg_base[OSD_SCALER_1])),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[OSD_SCALER_1] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[OSD_SCALER_1] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[OSD_SCALER_1] + 0xc)));

	/*enable osd clk*/
	/*osd_engine_clk_enable(osd.res_chg.cur_res_mode, true);*/
	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		mutex_lock(&osd.stop_sync_lock[i]);
		if (osd.osd_stop_ctl.osd_stop_status[i] != OSD_LAYER_STOPPED) {
			osd_clk_enable(osd.res_chg[i].cur_res_mode, true, i);
			fg_osd_always_update_frame[i] = 3;
		}
		mutex_unlock(&osd.stop_sync_lock[i]);

		osd.osd_in_suspend[i] = false;
	}

	vfree(osd.osd_suspend_save_info);
	osd.osd_suspend_save_info = NULL;

	return OSD_RET_OK;
}

static int disp_osd_get_info(struct disp_hw_common_info *info)
{
	return OSD_RET_OK;
}

static int disp_osd_dump(uint32_t level)
{
	return OSD_RET_OK;
}

static int disp_osd_hdmi_plug_out(void)
{
	int i;

	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		if (osd.plug_out[i].fg_hdmi_plug_out == false) {
			osd.plug_out[i].fg_hdmi_plug_out = true;
			osd.plug_out[i].fg_vsync1 = true;
			osd.plug_out[i].fg_vsync2 = false;
		}
	}
	OSD_LOG_I("disp_osd_hdmi_plug_out:%d\n",
		osd.plug_out[OSD_PLANE_1].fg_hdmi_plug_out);

	return OSD_RET_OK;
}

static int disp_osd_hdmi_plug_in(void)
{
	int i;

	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		if (osd.plug_out[i].fg_hdmi_plug_out == true)
			osd.plug_out[i].fg_hdmi_plug_out = false;
	}

	OSD_LOG_I("disp_osd_hdmi_plug_in:%d\n",
		osd.plug_out[OSD_PLANE_1].fg_hdmi_plug_out);

	return OSD_RET_OK;
}

static int disp_osd_premix_vdo4_enable(enum DISP_CMD cmd, void *data)
{
	uint32_t vdo4_premix_enable;
	uint32_t rgb2bgr;

	switch (cmd) {
	case DISP_CMD_OSD_PREMIX_ENABLE_VDO4:
		vdo4_premix_enable = *((uint32_t *)data);
		if (vdo4_premix_enable) {
			osd_pmx_set_plane0_y_sel(0);
			osd_pmx_set_plane0_cb_sel(1);
			osd_pmx_set_plane0_cr_sel(2);

			osd_pmx_set_pln0_msk(0);
			osd_pmx_set_vdo4_alpha(0xFF);
		} else {
			osd_pmx_set_pln0_msk(1);
			osd_pmx_set_vdo4_alpha(0);
		}

		#if 1
		OSD_PREMIX_Update_vdo4();
		#else
		fgupdate[OSD_PLANE_1] = 1;
		fgupdate[OSD_PLANE_2] = 1;
		osd_pmx_set_shadow_update(1);
		_OSD_BASE_SetPremixUpdate(OSD_PLANE_1, 1);
		_OSD_BASE_SetPremixUpdate(OSD_PLANE_2, 1);
		#endif
		break;
	case DISP_CMD_OSD_UPDATE:
		fg_dovi_idk_test = *((bool *)data);
		OSD_LOG_I("fg_dovi_idk_test %d\n", fg_dovi_idk_test);
		break;
	case DISP_CMD_HDMITX_PLUG_OUT:
		disp_osd_hdmi_plug_out();
		break;
	case DISP_CMD_HDMITX_PLUG_IN:
		disp_osd_hdmi_plug_in();
		break;
	case DISP_CMD_OSD_RGB_TO_BGR:
		rgb2bgr = *((uint32_t *)data);
		OSD_LOG_I("RGB_TO_BGR: %d\n", rgb2bgr);
		osd_premix_rgb2bgr(rgb2bgr);
		osd_pmx_set_shadow_update(1);
		_OSD_BASE_SetPremixUpdate(OSD_PLANE_1, 1);
		_OSD_BASE_SetPremixUpdate(OSD_PLANE_2, 1);
		break;
	default:
		break;
	}
	return OSD_RET_OK;
}

/*****************osd driver****************/
struct disp_hw disp_osd_driver = {
	.name = OSD_DRV_NAME,
	.init = osd_init,
	.deinit = osd_uninit,
	.start = disp_osd_start,
	.stop = disp_osd_stop,
	.suspend = disp_osd_suspend,
	.resume = disp_osd_resume,
	.get_info = disp_osd_get_info,
	.change_resolution = osd_set_displaymode,
	.config = NULL,
	.config_ex = disp_osd_config_ex,
	.irq_handler = disp_osd_irq_handle,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = disp_osd_dump,
	.set_cmd = disp_osd_premix_vdo4_enable,
};


struct disp_hw *disp_osd_get_drv(void)
{
	return &disp_osd_driver;
}
