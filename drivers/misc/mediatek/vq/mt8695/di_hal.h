/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Linjie Zhang <Linjie.Zhang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DI_HAL_H
#define DI_HAL_H

#include <linux/ioctl.h>
#include "mtk_vq_info.h"

#include "di_util.h"
#include "di_table.h"

#define MTK_DI_CLK_CNT 7


enum DI_HW_CLK {
	CLK_VDO_DI = 0,
	CLK_DISP_DI,
	CLK_WCH,
	CLK_DISP_DRAMC_LARB8,
	CLK_TOP_DI_SEL,
	CLK_TOP_TVDPLL_D2,
	CLK_TOP_OSDPLL_D3,
};

struct VQ_PROCESS_PARAM_T {
	/* if -1, ion address is invalid and should use mva address */
	unsigned int u4InputAddrIonYPrev;
	unsigned int u4InputAddrIonYCurr;
	unsigned int u4InputAddrIonYNext;
	unsigned int u4InputAddrIonCbcrPrev;
	unsigned int u4InputAddrIonCbcrCurr;
	unsigned int u4InputAddrIonCbcrNext;
	unsigned int u4OutputAddrIonY;
	unsigned int u4OutputAddrIonCbcr;

	unsigned int u4InputAddrIonYSizePrev;
	unsigned int u4InputAddrIonYSizeCurr;
	unsigned int u4InputAddrIonYSizeNext;
	unsigned int u4OutputAddrIonYSize;

	unsigned int u4InputAddrMvaYPrev;
	unsigned int u4InputAddrMvaYCurr;
	unsigned int u4InputAddrMvaYNext;
	unsigned int u4InputAddrMvaCbcrPrev;
	unsigned int u4InputAddrMvaCbcrCurr;
	unsigned int u4InputAddrMvaCbcrNext;
	unsigned int u4OutputAddrMvaY;
	unsigned int u4OutputAddrMvaCbcr;

	unsigned int u4InputAddrVaYPrev;
	unsigned int u4InputAddrVaYCurr;
	unsigned int u4InputAddrVaYNext;
	unsigned int u4InputAddrVaCbcrPrev;
	unsigned int u4InputAddrVaCbcrCurr;
	unsigned int u4InputAddrVaCbcrNext;
	unsigned int u4OutputAddrVaY;
	unsigned int u4OutputAddrVaCbcr;

	unsigned short u2SrcPicWidth;
	unsigned short u2SrcPicHeight;
	unsigned short u2SrcFrmWidth;
	unsigned short u2SrcFrmHeight;

	enum VQ_DI_MODE eDiMode;
	enum VQ_FIELD_TYPE eCurrField;
	enum VQ_COLOR_FMT eSrcColorFmt;
	enum VQ_COLOR_FMT eDstColorFmt;

	unsigned char u1TopFieldFirst;

	unsigned int u4MnrLevel;
	unsigned int u4BnrLevel;

};

struct di_info {
	uintptr_t dispfmt_reg_base;
	uintptr_t vdo_reg_base;
	uintptr_t wch_reg_base;
	uintptr_t disp_top_reg_base;

	uintptr_t disp_top_di_reg_base;

	uintptr_t disp_top_pclk_reg_base;

	int wrch_frame_end_irq;
	int di_vsync_irq;
	int di_disp_end_irq;
	int di_under_run_irq;


	struct clk *clks[MTK_DI_CLK_CNT];
	struct device *pm_dev;

	wait_queue_head_t wait_di_irq_wq;
	atomic_t wait_di_irq_flag;

	void *fusionbuf_va;
	dma_addr_t fusionbuf_mva;


#if DI_ASYNC_SUPPORT
	struct task_struct *di_config_task;

	wait_queue_head_t di_config_wq;
	atomic_t di_config_event;

	struct semaphore di_sem;

	struct list_head di_list;
	struct mutex di_timeline_lock;
	struct sw_sync_timeline *di_timeline;
	int di_timeline_max;
#endif

	uint8_t di_reg_irq_st;
	wait_queue_head_t di_event_irq_handle;
	uint32_t di_event_irq_timeout_count;
	atomic_t di_event_Irq_flag;

	void *wch_va_y;
	void *wch_va_c;
	unsigned long long count;

};


int di_hal_power_on(struct di_info *di);
int di_hal_power_off(struct di_info *di);

int di_hal_hw_init(struct platform_device *dev, struct di_info *di);
int di_hal_config(struct mtk_vq_config_info *config_info);
void di_hal_free_irq(struct di_info *di);
void VDP_HalSetDeintParam (MA_THRESHOLD_T * pThr);

#endif				/* DI_HAL_H */
