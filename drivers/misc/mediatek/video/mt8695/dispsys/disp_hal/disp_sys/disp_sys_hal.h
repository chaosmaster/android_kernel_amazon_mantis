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


#ifndef _DISP_SYS_HAL_H_
#define _DISP_SYS_HAL_H_

#include <linux/types.h>
#include <linux/printk.h>
#include "disp_reg.h"

extern struct disp_sys_context_t disp_sys;

#define DISPSYS_BASE		disp_sys.init_param.dispsys_reg_base
#define MMSYS_BASE			disp_sys.init_param.mmsys_reg_base

enum DISP_SYS_IRQ_BIT {
	DISP_SYS_IRQ_DISP3_END		= 0x1 << 0,
	DISP_SYS_IRQ_VDO3_UNDERRUN	= 0x1 << 1,
	DISP_SYS_IRQ_DISP3_VSYNC	= 0x1 << 4,
	DISP_SYS_IRQ_DISP4_END		= 0x1 << 8,
	DISP_SYS_IRQ_VDO4_UNDERRUN	= 0x1 << 9,
	DISP_SYS_IRQ_DISP4_VSYNC	= 0x1 << 10,
};

enum DISP_SYS_SRAM_PD {
	/*MMSYS SRAM PD*/
	DISP_SYS_SRAM_DRAM2AXI			= 0x1 << 0,
	DISP_SYS_SRAM_SDR2HDR_LUTA		= 0x1 << 1,
	DISP_SYS_SRAM_SDR2HDR_LUTB		= 0x1 << 2,
	DISP_SYS_SRAM_VM				= 0x1 << 3,
	DISP_SYS_SRAM_OSD_FHD			= 0x1 << 4,
	DISP_SYS_SRAM_OSD_UHD			= 0x1 << 5,
	DISP_SYS_SRAM_OSD_TVE			= 0x1 << 6,
	DISP_SYS_SRAM_VIDEOIN			= 0x1 << 7,
	DISP_SYS_SRAM_HDMI				= 0x1 << 8,
	DISP_SYS_SRAM_P2I				= 0x1 << 9,
	DISP_SYS_SRAM_DOLBY				= 0x1 << 10,
	/*DISPSYS SRAM PD*/
	DISP_SYS_SRAM_MVDO_HDR2SDR_LUTA	= 0x1 << 11,
	DISP_SYS_SRAM_MVDO_HDR2SDR_LUTB	= 0x1 << 12,
	DISP_SYS_SRAM_DISPFMT3			= 0x1 << 13,
	DISP_SYS_SRAM_MVDO_0			= 0x1 << 14,
	DISP_SYS_SRAM_MVDO_1			= 0x1 << 15,
	DISP_SYS_SRAM_MVDO_2			= 0x1 << 16,
	DISP_SYS_SRAM_MVDO_3			= 0x1 << 17,
	DISP_SYS_SRAM_SVDO_0			= 0x1 << 18,
	DISP_SYS_SRAM_SVDO_1			= 0x1 << 19,
	DISP_SYS_SRAM_DISPFMT4			= 0x1 << 20,
	DISP_SYS_SRAM_SVDO_HDR2SDR_LUTA	= 0x1 << 21,
	DISP_SYS_SRAM_SVDO_HDR2SDR_LUTB	= 0x1 << 22,
	DISP_SYS_SRAM_IMGRZ				= 0x1 << 23,
	DISP_SYS_SRAM_WC				= 0x1 << 24,
	DISP_SYS_SRAM_NR_DL				= 0x1 << 25,
	DISP_SYS_SRAM_NR_COMMON			= 0x1 << 26,
	DISP_SYS_SRAM_VDO2				= 0x1 << 27,
};

struct disp_sys_init_param {
	uintptr_t dispsys_reg_base;
	uintptr_t mmsys_reg_base;
};

struct disp_sys_context_t {
	bool inited;
	struct disp_sys_init_param init_param;
};

#define DISP_SYS_LOG_D(format...) pr_debug("[DISPSYS] "format)
#define DISP_SYS_LOG_I(format...) pr_info("[DISPSYS] "format)

enum DISP_TYPE {
	DISP_MAIN,
	DISP_SUB
};

enum DISP_OUT_SEL {
	DISP_OUT_SEL_VDO = (0x0 << 16),
	DISP_OUT_SEL_HDR2SDR = (0x1 << 16),
	DISP_OUT_SEL_BT2020 = (0x2 << 16),
	DISP_OUT_SEL_FIXED = (0x3 << 16)
};

void disp_sys_hal_set_disp_out(enum DISP_TYPE type, enum DISP_OUT_SEL out_sel);
void disp_sys_hal_shadow_en(bool en);
void disp_sys_hal_shadow_update(void);
void disp_sys_clear_irq(enum DISP_SYS_IRQ_BIT irq);
void disp_sys_clear_irq_all(void);
void disp_sys_sram_pd(enum DISP_SYS_SRAM_PD pd, bool en);
void disp_sys_sram_pd_all(bool en, int except_flags);
int disp_sys_hal_init(struct disp_sys_init_param *param);
int disp_sys_hal_video_preultra_en(enum DISP_TYPE type, bool en);
int disp_sys_hal_video_ultra_en(enum DISP_TYPE type, bool en);
void disp_sys_hal_hdr2sdr_dynamic_clock_ctrl(uint32_t value, bool en);
void disp_sys_hal_set_main_sub_swap(uint32_t swap);


#endif
