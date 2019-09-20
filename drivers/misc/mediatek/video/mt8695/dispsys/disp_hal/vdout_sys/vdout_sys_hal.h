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


#ifndef _VDOUT_HAL_H_
#define _VDOUT_HAL_H_

#include <linux/types.h>
#include <linux/printk.h>

#include "vdout_sys_hw.h"
#include "disp_reg.h"

extern struct vdout_context_t vdout;

/* temp define, it will be modified */
#define VDOUT_SYS_BASE		vdout.init_param.sys1_reg_base
#define VDOUT_SYS2_BASE		vdout.init_param.sys2_reg_base


/**********************************************************************
*  VDOUT System & Clock Macros
*********************************************************************/

#define vWriteVDOUT(dAddr, dVal)  WriteREG32((VDOUT_SYS_BASE + dAddr), dVal)
#define dReadVDOUT(dAddr)         ReadREG32(VDOUT_SYS_BASE + dAddr)
#define vWriteVDOUTMsk(dAddr, dVal, dMsk) vWriteVDOUT((dAddr), (dReadVDOUT(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

#define vWriteVDOUT2(dAddr, dVal)  WriteREG32((VDOUT_SYS2_BASE + dAddr), dVal)
#define dReadVDOUT2(dAddr)         ReadREG32(VDOUT_SYS2_BASE + dAddr)
#define vWriteVDOUT2Msk(dAddr, dVal, dMsk) vWriteVDOUT2((dAddr), (dReadVDOUT2(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

struct vdout_init_param {
	uintptr_t sys1_reg_base;
	uintptr_t sys2_reg_base;
	uintptr_t mmsys_reg_base;
};

struct vdout_context_t {
	bool inited;
	struct vdout_init_param init_param;
};

enum VDOUT_SYS_IRQ_BIT {
	VDOUT_SYS_IRQ_OSD2_UNDERFLOW	= 0x1 << 1,
	VDOUT_SYS_IRQ_OSD3_UNDERFLOW	= 0x1 << 2,
	VDOUT_SYS_IRQ_OSD5_UNDERFLOW	= 0x1 << 3,
	VDOUT_SYS_IRQ_OSD2_VSYNC		= 0x1 << 4,
	VDOUT_SYS_IRQ_OSD3_VSYNC		= 0x1 << 5,
	VDOUT_SYS_IRQ_OSD5_VSYNC		= 0x1 << 6,
	VDOUT_SYS_IRQ_FMT_ACTIVE_START	= 0x1 << 7,
	VDOUT_SYS_IRQ_FMT_ACTIVE_END	= 0x1 << 8,
	VDOUT_SYS_IRQ_FMT_VSYNC			= 0x1 << 9,
	VDOUT_SYS_IRQ_VDOIN				= 0x1 << 10,
	VDOUT_SYS_IRQ_VM				= 0x1 << 11,
	VDOUT_SYS_IRQ_SCLER				= 0x1 << 12,
	VDOUT_SYS_IRQ_DOLBY1			= 0x1 << 13,
	VDOUT_SYS_IRQ_DOLBY2			= 0x1 << 14,
	VDOUT_SYS_IRQ_DOLBY3			= 0x1 << 15
};

enum VIDEOIN_SRC_SEL {
	VIDEOIN_SRC_SEL_FMT = 1,
	VIDEOIN_SRC_SEL_FMT_VDO_ONLY,
	VIDEOIN_SRC_SEL_P2I,
	VIDEOIN_SRC_SEL_SDR2HDR,
	VIDEOIN_SRC_SEL_OSD,
	VIDEOIN_SRC_SEL_VM,
	VIDEOIN_SRC_SEL_HDMI,
	VIDEOIN_SRC_SEL_DOLBY1,
	VIDEOIN_SRC_SEL_VDO_MAIN,
	VIDEOIN_SRC_SEL_VDO_SUB,
	VIDEOIN_SRC_SEL_DOLBY2,
	VIDEOIN_SRC_SEL_DOLBY3,
};

enum VM_SRC_SEL {
	VM_SRC_SEL_FMT = 0,
	VM_SRC_SEL_OSD5,
	VM_SRC_SEL_P2I,
	VM_SRC_SEL_DOLBY3
};

enum VSYNC_PULSE_TYPE {
	VSYNC_PULSE_TYPE_VDO1_OUT,
	VSYNC_PULSE_TYPE_VDO2_OUT,
	VSYNC_PULSE_TYPE_DOLBY1,
	VSYNC_PULSE_TYPE_DOLBY2,
	VSYNC_PULSE_TYPE_DOLBY3,
	VSYNC_PULSE_TYPE_OSD_UHD,
	VSYNC_PULSE_TYPE_OSD_FHD,
	VSYNC_PULSE_TYPE_VDO1,
	VSYNC_PULSE_TYPE_VDO2,
	VSYNC_PULSE_TYPE_VM
};

#define VDOUT_LOG_E(format...) pr_info("[VDOUT]error: "format)
#define VDOUT_LOG_I(format...) pr_info("[VDOUT] "format)
#define VDOUT_LOG_D(format...) pr_debug("[VDOUT] "format)

/************************************************************************
*    Function : void vdout_sys_hal_4k2k_clock_enable(uint32_t res)
*    Description : set clock mux for 4k resolution
*                res: HDMI output resolution
*    Return      : None
************************************************************************/
void vdout_sys_hal_4k2k_clock_enable(uint32_t res);


/************************************************************************
*    Function : void vdout_sys_hal_set_hdmi(uint32_t res)
*    Description : Setting HDMI vdout data path and clock Mux
*                res: HDMI output resolution
*    Return      : None
************************************************************************/
void vdout_sys_hal_set_hdmi(HDMI_VIDEO_RESOLUTION res);


/************************************************************************
*    Function : void vdout_sys_hal_videoin_source_sel(enum VIDEOIN_SRC_SEL src_point)
*    Description : select video-in source point
*                src_point: source point of video-in
*    Return      : None
************************************************************************/
void vdout_sys_hal_videoin_source_sel(enum VIDEOIN_SRC_SEL src_point);


/************************************************************************
*    Function : int vdout_sys_hal_vm_source_sel(enum VM_SRC_SEL src_sel)
*    Description : video mark select source
*                src_sel: which source
*    Return      : int
************************************************************************/
int vdout_sys_hal_vm_source_sel(enum VM_SRC_SEL src_sel);


/************************************************************************
*    Function : int vdout_sys_hal_sdppf_source_sel(bool is_mix)
*    Description : sdppf select source
*                is_mix: source is mix or not
*    Return      : int
************************************************************************/
int vdout_sys_hal_sdppf_source_sel(bool is_mix);


/************************************************************************
*    Function : int vdout_sys_hal_p2i_source_sel(bool is_sdppf)
*    Description : p2i select source
*                is_sdppf: source is sdppf or not
*    Return      : int
************************************************************************/
int vdout_sys_hal_p2i_source_sel(bool is_sdppf);

/************************************************************************
*    Function : vdout_sys_hal_set_vp(...)
*    Description : set vsync pulse to adjust delay
*                type: vsync type
*                h_counter: horizontal delay
*                v_counter: vertical delay
*    Return      : int
************************************************************************/
int vdout_sys_hal_set_vp(enum VSYNC_PULSE_TYPE type, int h_counter, int v_counter);


/************************************************************************
*    Function : vdout_sys_hal_vp_shadow_en(bool enable)
*    Description : enable vsync pulse HW shadow
*                enable: true or false
*    Return      : none
************************************************************************/
void vdout_sys_hal_vp_shadow_en(bool enable);


/************************************************************************
*    Function : vdout_sys_hal_mux_shadow_en(bool enable)
*    Description : enable MUX HW shadow
*                enable: true or false
*    Return      : none
************************************************************************/
void vdout_sys_hal_mux_shadow_en(bool enable);


/************************************************************************
*    Function : vdout_sys_hal_clock_on_off(bool en)
*    Description : clock on/off
*                en: enable or nor
*    Return      : none
************************************************************************/
void vdout_sys_hal_clock_on_off(bool en);
void vdout_sys_hal_select_dsd_clk(bool en);
void vdout_sys_clear_irq(enum VDOUT_SYS_IRQ_BIT irq);
void vdout_sys_clear_irq_all(void);
void vdout_sys_422_to_420(bool en);
void vdout_sys_hal_vm_bypass(bool bypass);
void vdout_sys_hal_sdr2hdr_bypass(bool bypass);
void vdout_sys_hal_set_new_sd_sel(bool en);
int vdout_sys_hal_init(struct vdout_init_param *param);

int vdout_sys_hal_dolby_mix_on(bool on);
#endif
