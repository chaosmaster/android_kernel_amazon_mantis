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
#define LOG_TAG "VDOUT_SYS_HAL"

#include "hdmitx.h"

#include "disp_type.h"
#include "disp_def.h"
#include "vdout_sys_hw.h"
#include "vdout_sys_hal.h"
#include "disp_clk.h"
#include "disp_hw_log.h"

struct vdout_context_t vdout;

void vdout_sys_hal_4k2k_clock_enable(uint32_t res)
{
	VDOUT_LOG_D("set 4k2k clock enable, res=%d\n", res);

	if (fgResIs4k2k(res)) {
		if ((res == HDMI_VIDEO_3840x2160P_60HZ) || (res == HDMI_VIDEO_3840x2160P_50HZ) ||
			(res == HDMI_VIDEO_4096x2160P_60HZ) || (res == HDMI_VIDEO_4096x2160P_50HZ)) {
			vWriteVDOUTMsk(VDTCLK_CFG3, 0, VDO4_296M_EN | FMT_296M_EN);
			vWriteVDOUTMsk(HDMI_CONFIG_SUB, 0, RGB2HDMI_296M_EN_SUB);
			vWriteVDOUT2Msk(VDTCLK_CONFIG4, 0, VDO3_296M_EN);
			vWriteVDOUT2Msk(VDTCLK_CONFIG4,
				RGB2HDMI_594M_EN | FMT_594M_EN | VDO3_594M_EN  | VDO4_594M_EN,
				RGB2HDMI_594M_EN | FMT_594M_EN | VDO3_594M_EN  | VDO4_594M_EN);
		} else {
			vWriteVDOUTMsk(VDTCLK_CFG3, VDO4_296M_EN | FMT_296M_EN,
				VDO4_296M_EN | FMT_296M_EN);
			vWriteVDOUT2Msk(VDTCLK_CONFIG4, VDO3_296M_EN, VDO3_296M_EN);
			vWriteVDOUTMsk(HDMI_CONFIG_SUB, RGB2HDMI_296M_EN_SUB, RGB2HDMI_296M_EN_SUB);
			vWriteVDOUT2Msk(VDTCLK_CONFIG4, 0,
				RGB2HDMI_594M_EN | FMT_594M_EN | VDO3_594M_EN  | VDO4_594M_EN);
		}
	} else {
		vWriteVDOUTMsk(VDTCLK_CFG3, 0, VDO4_296M_EN | FMT_296M_EN);
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, 0, RGB2HDMI_296M_EN_SUB);
		vWriteVDOUT2Msk(VDTCLK_CONFIG4, 0,
			VDO3_296M_EN | RGB2HDMI_594M_EN | FMT_594M_EN | VDO3_594M_EN | VDO4_594M_EN);
	}
}

void vdout_sys_hal_set_hdmi(HDMI_VIDEO_RESOLUTION res)
{
	if (fgIsHDRes(res))
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, SELF_OPT_HDMI_SUB | VDOUT_CLK_HDMI_HD_SUB,
			SELF_OPT_HDMI_SUB | VDOUT_CLK_HDMI_HD_SUB | HDMI_PRGS27M_SUB); /*bit18,19*/
	else
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, HDMI_PRGS27M_SUB,
			SELF_OPT_HDMI_SUB | VDOUT_CLK_HDMI_HD_SUB | HDMI_PRGS27M_SUB); /*bit24*/

	if (res == HDMI_VIDEO_1920x1080p_50Hz || res == HDMI_VIDEO_1920x1080p_60Hz)
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, VDOUT_CLK_HDMI_1080P_SUB,
			VDOUT_CLK_HDMI_1080P_SUB);/*bit21*/
	else
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, 0, VDOUT_CLK_HDMI_1080P_SUB);

	/*vdout 74m*/
	if ((res == HDMI_VIDEO_1280x720p_60Hz) || (res == HDMI_VIDEO_1280x720p_50Hz) ||
			(res == HDMI_VIDEO_1920x1080p_30Hz) || (res == HDMI_VIDEO_1920x1080p_25Hz) ||
			(res == HDMI_VIDEO_1920x1080p_24Hz) || (res == HDMI_VIDEO_1920x1080p_23Hz) ||
			(res == HDMI_VIDEO_1920x1080p_29Hz))
		vWriteVDOUTMsk(RW_VDOUT_CLK, HD_HALF, HD_HALF);
	else
		vWriteVDOUTMsk(RW_VDOUT_CLK, 0, HD_HALF);

	vdout_sys_hal_4k2k_clock_enable(res);
}

void vdout_sys_hal_videoin_source_sel(enum VIDEOIN_SRC_SEL src_point)
{
	if (src_point == VIDEOIN_SRC_SEL_DOLBY1 ||
		src_point == VIDEOIN_SRC_SEL_DOLBY2 ||
		src_point == VIDEOIN_SRC_SEL_DOLBY3) {
		vWriteVDOUTMsk(HDMI_CONFIG0_SUB, VIDEOIN_SRC_SEL_DOLBY1, VIDEO_IN_SRC_SEL);

		if (src_point == VIDEOIN_SRC_SEL_DOLBY1)
			vWriteVDOUT2Msk(SYS_2_CFG_34, VIDEO_IN_DOLBY1, VIDEO_IN_DOLBY_CONFIG);
		else if (src_point == VIDEOIN_SRC_SEL_DOLBY2)
			vWriteVDOUT2Msk(SYS_2_CFG_34, VIDEO_IN_DOLBY2, VIDEO_IN_DOLBY_CONFIG);
		else
			vWriteVDOUT2Msk(SYS_2_CFG_34, VIDEO_IN_DOLBY3, VIDEO_IN_DOLBY_CONFIG);

	} else {
		vWriteVDOUTMsk(HDMI_CONFIG0_SUB, src_point, VIDEO_IN_SRC_SEL);
	}
}

void vdout_sys_hal_select_dsd_clk(bool en)
{
	if (en)
		vWriteVDOUT2Msk(VDTCLK_CONFIG4, VDO3_150HZ_EN, VDO3_150HZ_EN);
	else
		vWriteVDOUT2Msk(VDTCLK_CONFIG4, 0, VDO3_150HZ_EN);
}


/************************************/
/* vdout sys path select */
/************************************/
int vdout_sys_hal_vm_source_sel(enum VM_SRC_SEL src_sel)
{
	VDOUT_LOG_D("vm src_sel: %d\n", src_sel);

	if (src_sel == VM_SRC_SEL_DOLBY3) {
		vWriteVDOUTMsk(SYS_CFG_CC, VM_SEL_DOLBY3, VM_SEL_DOLBY3);
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, 0, VM_SEL);
	} else {
		vWriteVDOUTMsk(SYS_CFG_CC, 0, VM_SEL_DOLBY3);
		if (src_sel == VM_SRC_SEL_OSD5)
			vWriteVDOUTMsk(HDMI_CONFIG_SUB, VM_SEL_OSD5, VM_SEL);
		else if (src_sel == VM_SRC_SEL_P2I)
			vWriteVDOUTMsk(HDMI_CONFIG_SUB, VM_SEL_P2I, VM_SEL);
		else
			vWriteVDOUTMsk(HDMI_CONFIG_SUB, VM_SEL_FMT, VM_SEL);
	}

	return 0;
}

int vdout_sys_hal_dolby_mix_on(bool on)
{
	VDOUT_LOG_D("dolby_mix_enable: %d\n", on);

	if (on)
		vWriteVDOUT2Msk(SYS_2_CFG_34, DOLBY_MIX_ON, DOLBY_MIX_ON);
	else
		vWriteVDOUT2Msk(SYS_2_CFG_34, 0, DOLBY_MIX_ON);

	return 0;
}

int vdout_sys_hal_sdppf_source_sel(bool is_mix)
{
	VDOUT_LOG_D("sdppf src_sel: %d\n", is_mix);

	if (is_mix)
		vWriteVDOUTMsk(OSD5BG, SDPPF_SRC_SEL_MIX, SDPPF_SRC_SEL);
	else
		vWriteVDOUTMsk(OSD5BG, SDPPF_SRC_SEL_DOLBY3, SDPPF_SRC_SEL);

	return 0;
}

int vdout_sys_hal_p2i_source_sel(bool is_sdppf)
{
	if (is_sdppf)
		vWriteVDOUT2Msk(SYS_2_CFG_34, P2I_SRC_SEL, P2I_SRC_SEL);
	else
		vWriteVDOUT2Msk(SYS_2_CFG_34, 0, P2I_SRC_SEL);

	return 0;
}

int vdout_sys_hal_set_vp(enum VSYNC_PULSE_TYPE type, int h_counter, int v_counter)
{
	int vp_setting = 0;

	DISP_LOG_DEBUG("vsync pulse type=%d, h_counter=0x%x, v_counter=0x%x\n",
		type, h_counter, v_counter);

	vp_setting = v_counter << 16 | h_counter;

	if (type == VSYNC_PULSE_TYPE_VDO1_OUT)
		vWriteVDOUT(VSYNC_PULSE_VDO1_OUT, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_VDO2_OUT)
		vWriteVDOUT(VSYNC_PULSE_VDO2_OUT, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_DOLBY1)
		vWriteVDOUT(VSYNC_PULSE_DOLBY1, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_DOLBY2)
		vWriteVDOUT(VSYNC_PULSE_DOLBY2, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_DOLBY3)
		vWriteVDOUT(VSYNC_PULSE_DOLBY3, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_OSD_UHD)
		vWriteVDOUT(VSYNC_PULSE_OSD_UHD, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_OSD_FHD)
		vWriteVDOUT(VSYNC_PULSE_OSD_FHD, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_VDO1)
		vWriteVDOUT(VSYNC_PULSE_VDO1, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_VDO2)
		vWriteVDOUT(VSYNC_PULSE_VDO2, vp_setting);
	else if (type == VSYNC_PULSE_TYPE_VM)
		vWriteVDOUT(VSYNC_PULSE_VM, vp_setting);
	else
		VDOUT_LOG_I("error type=%d\n", type);

	return 0;
}

void vdout_sys_hal_vp_shadow_en(bool enable)
{
	if (enable)
		vWriteVDOUTMsk(SYS_CFG_B0, VYSNC_PULSE_SHADOW_EN, VYSNC_PULSE_SHADOW_EN);
	else
		vWriteVDOUTMsk(SYS_CFG_B0, 0, VYSNC_PULSE_SHADOW_EN);
}

void vdout_sys_hal_mux_shadow_en(bool enable)
{
	if (enable)
		vWriteVDOUTMsk(SYS_CFG_B0, MUX_SHADOW_EN, MUX_SHADOW_EN);
	else
		vWriteVDOUTMsk(SYS_CFG_B0, 0, MUX_SHADOW_EN);
}

void vdout_sys_hal_clock_on_off(bool en)
{
	disp_clock_enable(DISP_CLK_VDOUT_SYS, en);
}

void vdout_sys_hal_vm_bypass(bool bypass)
{
	if (bypass)
		vWriteVDOUTMsk(HDMI_CONFIG0_SUB, VM_BYPASS, VM_BYPASS);
	else
		vWriteVDOUTMsk(HDMI_CONFIG0_SUB, 0, VM_BYPASS);
}

void vdout_sys_hal_sdr2hdr_bypass(bool bypass)
{
	if (bypass)
		vWriteVDOUT2Msk(SYS_2_CFG_34, SDR2HDR_BYPASS, SDR2HDR_BYPASS);
	else
		vWriteVDOUT2Msk(SYS_2_CFG_34, 0, SDR2HDR_BYPASS);
}

void vdout_sys_hal_set_new_sd_sel(bool en)
{
	if (en)
		vWriteVDOUTMsk(VDTCLK_CFG3, VIDEOIN_NEW_SD_SEL, VIDEOIN_NEW_SD_SEL);
	else
		vWriteVDOUTMsk(VDTCLK_CFG3, 0, VIDEOIN_NEW_SD_SEL);
}

void vdout_sys_clear_irq(enum VDOUT_SYS_IRQ_BIT irq)
{
	/*set 1 and 0 to clear interrupt*/
	vWriteVDOUT2Msk(VDOUT_INT_CLR, irq, irq);
	vWriteVDOUT2Msk(VDOUT_INT_CLR, 0, irq);
}

void vdout_sys_clear_irq_all(void)
{
	/*set 1 and 0 to clear interrupt*/
	vWriteVDOUT2(VDOUT_INT_CLR, 0xffffffff);
	vWriteVDOUT2(VDOUT_INT_CLR, 0);
}

void vdout_sys_422_to_420(bool en)
{
	if (en)
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, HDMI_422_TO_420, HDMI_422_TO_420);
	else
		vWriteVDOUTMsk(HDMI_CONFIG_SUB, 0, HDMI_422_TO_420);
}

int vdout_sys_hal_init(struct vdout_init_param *param)
{
	if (vdout.inited)
		return -1;

	vdout.inited = true;
	vdout.init_param.sys1_reg_base = param->sys1_reg_base;
	vdout.init_param.sys2_reg_base = param->sys2_reg_base;
	vdout.init_param.mmsys_reg_base = param->mmsys_reg_base;

	VDOUT_LOG_D("sys reg base: 0x%lx, sys2 reg base: 0x%lx\n",
	vdout.init_param.sys1_reg_base, vdout.init_param.sys2_reg_base);

	if (vdout.init_param.sys1_reg_base == 0 || vdout.init_param.sys2_reg_base == 0)
		return -1;

	return 0;
}

