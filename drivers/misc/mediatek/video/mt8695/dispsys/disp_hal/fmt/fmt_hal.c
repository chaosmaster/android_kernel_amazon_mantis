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
#define LOG_TAG "FMT_HAL"

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/string.h>

#include "fmt_hw.h"
#include "fmt_hal.h"
#include "fmt_def.h"
#include "disp_def.h"
#include "disp_reg.h"
#include "disp_clk.h"
#include "disp_hw_log.h"

struct fmt_context fmt;

#define SUB_VDOUT_FMT_OFFSET 0x300

const struct fmt_delay_info dispfmt_delay_table[] = {
	{1, 2, 0x334, HDMI_VIDEO_720x480i_60Hz},
	{1, 2, 0x35a, HDMI_VIDEO_720x576i_50Hz},
	{1, 2, 0x334, HDMI_VIDEO_720x480p_60Hz},
	{1, 2, 0x35a, HDMI_VIDEO_720x576p_50Hz},
	{1, 2, 0x64a, HDMI_VIDEO_1280x720p_60Hz},
	{1, 2, 0x794, HDMI_VIDEO_1280x720p_50Hz},
	{0, 0x8C8, 0x86F, HDMI_VIDEO_1920x1080i_60Hz},
	{0, 0x8C8, 0xA28, HDMI_VIDEO_1920x1080i_50Hz},
	{0, 0x8C8, 0x870, HDMI_VIDEO_1920x1080p_30Hz},
	{0, 0x8C8, 0xA28, HDMI_VIDEO_1920x1080p_25Hz},
	{0, 0x8C8, 0xA96, HDMI_VIDEO_1920x1080p_24Hz},
	{0, 0x8C8, 0xA96, HDMI_VIDEO_1920x1080p_23Hz},
	{0, 0x8C8, 0x870, HDMI_VIDEO_1920x1080p_29Hz},
	{0, 0x8C8, 0x885, HDMI_VIDEO_1920x1080p_60Hz},
	{0, 0x8C8, 0xA28, HDMI_VIDEO_1920x1080p_50Hz},
	{0, 0x1192, 0x1553, HDMI_VIDEO_3840x2160P_23_976HZ},
	{0, 0x1192, 0x1553, HDMI_VIDEO_3840x2160P_24HZ},
	{0, 0x1192, 0x1477, HDMI_VIDEO_3840x2160P_25HZ},
	{0, 0x1192, 0x1107, HDMI_VIDEO_3840x2160P_29_97HZ},
	{0, 0x1192, 0x10ED, HDMI_VIDEO_3840x2160P_30HZ},
	{0, 0x1192, 0x1553, HDMI_VIDEO_4096x2160P_24HZ},
	{0, 0x1192, 0x1477, HDMI_VIDEO_3840x2160P_50HZ},
	{0, 0x1192, 0x10ED, HDMI_VIDEO_3840x2160P_60HZ},
	{0, 0x1192, 0x1477, HDMI_VIDEO_4096x2160P_50HZ},
	{0, 0x1192, 0x10ED, HDMI_VIDEO_4096x2160P_60HZ},
};

const uint32_t scale_16_pahse_coef_y[] = {
	0x00000000,
	0xF913D2A2,
	0xFFF205FE,
	0xFF03FA10,
	0xFBE70AFC,
	0xFE05F322,
	0xF3DE0EFA,
	0xFD08ED35,
	0xE8D711F9,
	0xFC0BE64A,
	0xD9D313F8,
	0xFB0EE060,
	0xCAD014F8,
	0xFA10DA76,
	0xB8D014F8,
	0xF912D58C,
	0x00000001,
};

static void _fmt_mutex_init(void)
{
	mutex_init(&(fmt.lock));
}

static void _fmt_mutex_lock(void)
{
	mutex_lock(&(fmt.lock));
}

static void _fmt_mutex_unlock(void)
{
	mutex_unlock(&(fmt.lock));
}

static uint32_t _fmt_get_vsync_width(HDMI_VIDEO_RESOLUTION resolution)
{
	uint32_t width = 0;

	switch (resolution) {
	case HDMI_VIDEO_1920x1080p_23Hz:
		width = 24;
		break;
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080i_60Hz:
		width = 31;
		break;
	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
		width = 17;
		break;
	default:
		width = 20;
		break;
	}

	return width;
}

static uint32_t _fmt_get_dispfmt_vsync_width(HDMI_VIDEO_RESOLUTION resolution)
{
	uint32_t width = 0;

	switch (resolution) {
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
	case HDMI_VIDEO_4096x2160P_60HZ:
	case HDMI_VIDEO_4096x2160P_50HZ:
		width = 0x15;
		break;
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_4096x2160P_24HZ:
		width = 0x8;
		break;
	default:
		width = 2;
		break;
	}

	return width;
}

static void _fmt_hal_reset(uint32_t fmt_id)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	if (fmt_id >= VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);

		if (vdout_fmt && vdout_fmt->status == FMT_STA_USED) {
			WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (0 << 10),
				      (1 << 10));
			WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (1 << 10),
				      (1 << 10));
			WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (0 << 10),
				      (1 << 10));
		}
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);

		if (disp_fmt->is_sec)
			return;

		if (disp_fmt && disp_fmt->status == FMT_STA_USED) {
			WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_VDO_FMT_CTRL), (0 << 10),
				      (1 << 10));
			WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_VDO_FMT_CTRL), (1 << 10),
				      (1 << 10));
			WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_VDO_FMT_CTRL), (0 << 10),
				      (1 << 10));
		}
	}
}

static void _fmt_dispfmt_setting(uint32_t fmt_id)
{
	uint32_t u4RegIdx;
	uint64_t regMask;

	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB) {
		FMT_LOG_E("DISP_PMX_ID %d is invalid please check\n", fmt_id);
		return;
	}

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->is_sec)
		return;

	if (disp_fmt->status == FMT_STA_USED) {
		for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_DISP_FMT_MAIN_REG_NUM;
		     u4RegIdx++, regMask <<= 1) {
			if (IS_REG_SET(*disp_fmt->reg_mode, regMask)
			    && (u4RegIdx != DISP_FMT_REG_ACTIVE_H / 4)) {
				WriteREG32((disp_fmt->hw_fmt_base + u4RegIdx * 4),
					   disp_fmt->sw_fmt_base->au4Reg[u4RegIdx]);

				FMT_LOG_D("[DISP_FMT_REG %d] Reg= 0x%x, Val= 0x%x\n", fmt_id,
					  (u4RegIdx * 4), disp_fmt->sw_fmt_base->au4Reg[u4RegIdx]);
				disp_fmt->shadow_trigger = 1;
			}
		}
		*(disp_fmt->reg_mode) = 0x00;
	}
}

static void _fmt_dispfmt_trigger(uint32_t fmt_id)
{
	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB) {
		FMT_LOG_E("DISP_FMT_ID %d is invalid please check\n", fmt_id);
		return;
	}

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);

	if ((disp_fmt->status == FMT_STA_USED)
	    && (disp_fmt->shadow_trigger == 1 && disp_fmt->shadow_en == 1)) {
		FMT_LOG_D("[DISP_FMT_REG %d] trigger disp fmt shadow register..\n", fmt_id);

		if (!disp_fmt->is_sec)
			WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_ACTIVE_H), (1 << 31), (1 << 31));

		disp_fmt->shadow_trigger = 0;
	}
}

static void _fmt_vdoutfmt_setting(uint32_t fmt_id)
{
	uint32_t u4RegIdx;
	uint64_t regMask;

	struct vdout_fmt_t *fmt_info = NULL;

	fmt_info = fmt_hal_get_vdoutfmt_reg(fmt_id);

	for (u4RegIdx = 0, regMask = 1; u4RegIdx < HAL_VDOUT_FMT_REG_NUM; u4RegIdx++, regMask <<= 1) {
		if (IS_REG_SET(*fmt_info->reg_mode, regMask)
		    && (u4RegIdx != (VDOUT_FMT_REG_FMT_CTRL / 4))) {
			WriteREG32((fmt_info->hw_fmt_base + u4RegIdx * 4),
				   fmt_info->sw_fmt_base->au4Reg[u4RegIdx]);

			fmt.vdout_fmt.shadow_trigger = 1;
			FMT_LOG_D("[VDOUT_FMT_REG %d] Reg= 0x%x, Val= 0x%x\n", fmt_id,
				  (u4RegIdx * 4), fmt_info->sw_fmt_base->au4Reg[u4RegIdx]);
		}
	}

	*fmt_info->reg_mode = 0;

}

static void _fmt_vdoutfmt_trigger(bool is_force)
{
	struct vdout_fmt_t *fmt_info = NULL;
	struct vdout_fmt_t *sub_fmt_info = NULL;

	/*just trigger main vdout fmt*/
	fmt_info = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	sub_fmt_info = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT_SUB);

	if (((sub_fmt_info->shadow_trigger == 1 || fmt_info->shadow_trigger == 1)
		&& fmt_info->shadow_en == 1) || is_force) {
		FMT_LOG_D("[VDOUT_FMT_REG] trigger vdout fmt shadow register..\n");
		WriteREG32Msk((fmt_info->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (1 << 31),
			      (1 << 31));

		fmt_info->shadow_trigger = 0;
		sub_fmt_info->shadow_trigger = 0;
	}
}

static int32_t _fmt_write_reg(uint32_t fmt_id, void *data, uint32_t reg, bool hw_shadow)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	if (fmt_id > VDOUT_FMT_SUB)
		return -1;

	if (!data) {
		FMT_LOG_I("_fmt_write_reg data is null,fmt_id=%d, reg=0x%x.\n",
			fmt_id, reg);
		return -1;
	}

	if (fmt_id >= VDOUT_FMT) {
		vdout_fmt = (struct vdout_fmt_t *)data;

		if (hw_shadow) {
			WriteREG32(vdout_fmt->hw_fmt_base + reg,
				vdout_fmt->sw_fmt_base->au4Reg[reg/4]);
			vdout_fmt->shadow_trigger = 1;
			FMT_LOG_D("[VDOUT_REG %d] Write Reg= 0x%x, Val= 0x%x\n", fmt_id,
					  reg, vdout_fmt->sw_fmt_base->au4Reg[reg / 4]);
			_fmt_vdoutfmt_trigger(false);
		} else {
			REG_SET(*vdout_fmt->reg_mode, REG_MASK(reg / 4));
		}
	} else {
		disp_fmt = (struct disp_fmt_t *)data;
		/* don't write hw register when secure playback*/
		if (disp_fmt->is_sec)
			hw_shadow = false;

		if (hw_shadow) {
			WriteREG32(disp_fmt->hw_fmt_base + reg,
				disp_fmt->sw_fmt_base->au4Reg[reg/4]);
			disp_fmt->shadow_trigger = 1;
			FMT_LOG_D("[DISP_FMT_REG %d] Write Reg= 0x%x, Val= 0x%x\n", fmt_id,
					  reg, disp_fmt->sw_fmt_base->au4Reg[reg / 4]);
			_fmt_dispfmt_trigger(fmt_id);
		} else {
			REG_SET(*disp_fmt->reg_mode, REG_MASK(reg / 4));
		}
	}

	return 0;
}

static void _fmt_select_vdout_cconv(bool select_cconv)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);

	if (select_cconv)
		vdout_fmt->sw_fmt_base->rField.fgVDOUT_CCONV = 1;
	else
		vdout_fmt->sw_fmt_base->rField.fgVDOUT_CCONV = 0;

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_PLANE, false);
}

static void _fmt_set_vdoutfmt_mode(HDMI_VIDEO_RESOLUTION resolution, bool config_hw)
{
	struct vdout_fmt_t *vdout_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt_sub = NULL;

	FMT_LOG_D("set vdout fmt mode: %d, config: %d\n", resolution, config_hw);

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	vdout_fmt_sub = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT_SUB);

	switch (resolution) {
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x480p_60Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 0;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_720x576i_50Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 0;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_1920x1080p_24Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0xABE;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_1920x1080p_25Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0xA50;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_1920x1080p_29Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x898;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:

		if ((resolution == HDMI_VIDEO_3840x2160P_23_976HZ)
		    || (resolution == HDMI_VIDEO_3840x2160P_24HZ))
			vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x157C;
		else if ((resolution == HDMI_VIDEO_3840x2160P_25HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_50HZ))
			vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x14A0;
		else if ((resolution == HDMI_VIDEO_3840x2160P_29_97HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_30HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_60HZ))
			vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x1130;

		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_4096x2160P_24HZ:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x157c;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_4096x2160P_50HZ:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x14A0;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;

	case HDMI_VIDEO_4096x2160P_60HZ:
		vdout_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		vdout_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		vdout_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		vdout_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x1130;
		vdout_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		vdout_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		vdout_fmt->sw_fmt_base->rField.fgMAS_CNT_SYNC = 0;
		break;


	default:
		FMT_LOG_E("the resolution is wrong , res: %d\n", resolution);

		break;
	}

	vdout_fmt->sw_fmt_base->rField.u1Prgs_for_osd_3d = 0;
	vdout_fmt->sw_fmt_base->rField.fgPLN3_PRE_MODE = 1;


	if (HDMI_VIDEO_1920x1080i_60Hz == resolution || HDMI_VIDEO_1920x1080i_50Hz == resolution)
		vdout_fmt->sw_fmt_base->rField.fgRST_PHASE = 1;
	else
		vdout_fmt->sw_fmt_base->rField.fgRST_PHASE = 0;

	vdout_fmt->sw_fmt_base->rField.fgPRGS = 1;
	vdout_fmt->sw_fmt_base->rField.u1VSYNWIDTH = _fmt_get_vsync_width(resolution);
	vdout_fmt->sw_fmt_base->rField.u1HSYNWIDTH = 32;

	vdout_fmt->sw_fmt_base->rField.fgFMTM = 1;

	vdout_fmt->status = FMT_STA_USED;
	vdout_fmt_sub->status = FMT_STA_USED;

	if (config_hw == true) {
		/*write hw register */
		_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_FMT_HV_TOTAL, true);
		_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_MULTI, true);
		_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_FMT_MODE, true);
		_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_FMT_CTRL, true);
		_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_PREMIX, true);
		_fmt_hal_reset(VDOUT_FMT);
	}
	fmt_hal_enable(VDOUT_FMT, true);
	fmt.res = resolution;
}

static void _fmt_reset_fmt_info(uint32_t fmt_id)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_LOG_D("reset fmt id: %d\n", fmt_id);

	if (fmt_id > VDOUT_FMT_SUB)
		return;

	if (fmt_id >= VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt == NULL) {
			FMT_LOG_E("get vdout fmt register info error\n");
			return;
		}

		vdout_fmt->status = FMT_STA_UNUSED;
		*(vdout_fmt->reg_mode) = 0;
		memset((void *)(vdout_fmt->sw_fmt_base), 0, sizeof(union vdout_fmt_union_t));
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt == NULL) {
			FMT_LOG_E("get display fmt register info error\n");
			return;
		}

		disp_fmt->status = FMT_STA_UNUSED;
		*(disp_fmt->reg_mode) = 0;
		memset((void *)(disp_fmt->sw_fmt_base), 0, sizeof(union disp_fmt_union_t));
	}
}
static void _fmt_set_dispfmt_mode(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_LOG_D("set disp_fmt mode: %d, fmt_id: %d\n", resolution, fmt_id);
	_fmt_reset_fmt_info(fmt_id);

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);

	switch (resolution) {
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x480p_60Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 0;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;
	case HDMI_VIDEO_720x576i_50Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 0;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 1;
		break;

	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 0;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		if ((resolution == HDMI_VIDEO_1920x1080i_60Hz)
		    || (resolution == HDMI_VIDEO_1920x1080i_50Hz))
			disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 1;
		else
			disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_1920x1080p_24Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0xABE;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0xABE;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_1920x1080p_25Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0xA50;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0xA50;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_1920x1080p_29Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x898;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x465;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0x898;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;

		if ((resolution == HDMI_VIDEO_3840x2160P_23_976HZ)
		    || (resolution == HDMI_VIDEO_3840x2160P_24HZ))
			disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x157C;
		else if ((resolution == HDMI_VIDEO_3840x2160P_25HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_50HZ))
			disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x14A0;
		else if ((resolution == HDMI_VIDEO_3840x2160P_29_97HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_30HZ)
			 || (resolution == HDMI_VIDEO_3840x2160P_60HZ))
			disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x1130;

		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = disp_fmt->sw_fmt_base->rField.u2V_TOTAL;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = disp_fmt->sw_fmt_base->rField.u2H_TOTAL;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 1;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_4096x2160P_24HZ:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x157c;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0x157c;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 1;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_4096x2160P_50HZ:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x14A0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0x14A0;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	case HDMI_VIDEO_4096x2160P_60HZ:
		disp_fmt->sw_fmt_base->rField.fgHD_ON = 1;
		disp_fmt->sw_fmt_base->rField.fgHD_TP = 0;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL = 0x1130;
		disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = 0x8ca;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = 0x1130;
		disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX = 1;
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		disp_fmt->sw_fmt_base->rField.fgEN_FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.u1FIRST_PXL_LEAD = 0;
		disp_fmt->sw_fmt_base->rField.fgHD_C_POS = 0;
		break;

	default:
		FMT_LOG_E("the resolution is wrong , res: %d\n", resolution);

		break;
	}

	disp_fmt->sw_fmt_base->rField.u2C4Default = 0x10;
	disp_fmt->sw_fmt_base->rField.fgFACT_PREC = 1;

	switch (resolution) {
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_4096x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_4096x2160P_50HZ:
	case HDMI_VIDEO_4096x2160P_60HZ:
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 1;

		break;
	default:
		disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 0;
		break;
	}

	if (fgIsHDRes(resolution) && (!fgIsFullHDRes(resolution)))
		disp_fmt->sw_fmt_base->rField.fgHD_C_FIR_EN = 1;
	else
		disp_fmt->sw_fmt_base->rField.fgHD_C_FIR_EN = 0;

	/*set Vsync & Hsync */
	disp_fmt->sw_fmt_base->rField.fgPRGS = 1;
	/*disp_fmt VsysncW need reduce because add ufo */
	disp_fmt->sw_fmt_base->rField.u1VSYNWIDTH
	= _fmt_get_dispfmt_vsync_width(resolution);
	disp_fmt->sw_fmt_base->rField.u1HSYNWIDTH = 32;

	disp_fmt->sw_fmt_base->rField.FMT_VP_RESET_SELECT = 1;

	disp_fmt->sw_fmt_base->rField.fgPFOFF = 0;
	disp_fmt->sw_fmt_base->rField.fgFMTM = 0;

	/*write hw register */
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_PIXEL_LENGTH, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HV_TOTAL_MIX, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HV_TOTAL, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_Y_LIMITATION, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_RATIO, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_MODE_CTRL, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_VDO_FMT_CTRL, true);

	disp_fmt->status = FMT_STA_USED;

	fmt_hal_set_delay_by_res(fmt_id, resolution);
}


static int32_t _fmt_get_dispfmt_id(uint32_t fmt_id)
{
	FMT_FUNC();
	DISP_LOG_DEBUG("get_dispfmt_id: %d\n", fmt_id);

	if (fmt_id == DISP_FMT_MAIN)
		return DISP_FMT1;  /* fmt1 */
	else if (fmt_id == DISP_FMT_SUB)
		return DISP_FMT2;  /* fmt2 */

	return DISP_FMT_MAIN;
}

static int32_t _fmt_hd_scl_enable(uint32_t fmt_id, uint32_t factor, bool en, bool fifo_en)
{
	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.fgHDWN_EN = en;
	disp_fmt->sw_fmt_base->rField.fgFULL_FIFO = fifo_en;
	disp_fmt->sw_fmt_base->rField.fgHDWN_444 = en;
	disp_fmt->sw_fmt_base->rField.u2HDWN_FAC = factor;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_CTRL, true);

	return FMT_OK;
}

static int32_t _fmt_set_in_region(uint32_t fmt_id, uint32_t h_start, uint32_t h_end,
	uint32_t odd_start, uint32_t odd_end, uint32_t even_start, uint32_t even_end)
{
	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN = h_start;
	disp_fmt->sw_fmt_base->rField.u2HDWN_HEND = h_end;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VOBGN = odd_start;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VOEND = odd_end;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VEBGN = even_start;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VEEND = even_end;
	disp_fmt->sw_fmt_base->rField.u2HDWN_2F = 1;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_H, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_VO, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_VE, true);

	return FMT_OK;
}

static int32_t _fmt_set_out_region(uint32_t fmt_id, uint32_t h_start, uint32_t h_end)
{
	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.u2HDWN_HO_BGN = h_start;
	disp_fmt->sw_fmt_base->rField.u2HDWN_HO_END = h_end;
	disp_fmt->sw_fmt_base->rField.fgFULL_FIFO = 1;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_OUTPUT, true);

	return FMT_OK;
}

static int32_t _fmt_set_buildin_color(bool en)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}
	vdout_fmt->sw_fmt_base->rField.fgCB_ON = en;

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_MIXER_MODE, true);

	return FMT_OK;
}

void fmt_hal_enable(uint32_t fmt_id, bool enable)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();

	if (fmt_id > VDOUT_FMT)
		return;

	FMT_LOG_D("enable fmt, fmt_id: %d\n", fmt_id);

	if (fmt_id == VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s vdout fmt state error\n", __func__);
			return;
		}

		if (enable) {
			WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (1 << 0), (1 << 0));

			vdout_fmt->sw_fmt_base->rField.fgFTRST = 0;
			vdout_fmt->sw_fmt_base->rField.fgFMT_ON = true;
		} else {
			WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (0 << 0), (1 << 0));

			vdout_fmt->sw_fmt_base->rField.fgFMT_ON = false;
		}
		vdout_fmt->shadow_trigger = 1;
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s disp fmt state error\n", __func__);
			return;
		}

		if (enable) {
			disp_fmt->sw_fmt_base->rField.fgVDO_EN = true;

			if (!disp_fmt->is_sec)
				WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_VDO_FMT_CTRL), (1 << 0), (1 << 0));
			else
				REG_SET(*disp_fmt->reg_mode, REG_MASK(DISP_FMT_REG_VDO_FMT_CTRL / 4));
		} else {
			disp_fmt->sw_fmt_base->rField.fgVDO_EN = false;

			if (!disp_fmt->is_sec)
				WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_VDO_FMT_CTRL), (0 << 0), (1 << 0));
			else
				REG_SET(*disp_fmt->reg_mode, REG_MASK(DISP_FMT_REG_VDO_FMT_CTRL / 4));
		}
		disp_fmt->shadow_trigger = 1;
	}

}

struct disp_fmt_t *fmt_hal_get_dispfmt_reg(uint32_t fmt_id)
{
	if (fmt_id == DISP_FMT_MAIN)
		return &fmt.main_disp_fmt;
	return &fmt.sub_disp_fmt;
}

struct vdout_fmt_t *fmt_hal_get_vdoutfmt_reg(uint32_t fmt_id)
{
	if (fmt_id == VDOUT_FMT)
		return &fmt.vdout_fmt;
	else if (fmt_id == VDOUT_FMT_SUB)
		return &fmt.sub_vdout_fmt;
	else
		return NULL;
}

void fmt_hal_set_mode(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution, bool config_hw)
{
	_fmt_mutex_lock();

	if (fmt_id == VDOUT_FMT)
		_fmt_set_vdoutfmt_mode(resolution, config_hw);
	else
		_fmt_set_dispfmt_mode(fmt_id, resolution);

	_fmt_mutex_unlock();
}

void fmt_hal_reset_in_vsync(uint32_t fmt_id)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	if (fmt_id > VDOUT_FMT)
		return;

	if (fmt_id == VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s vdout fmt state error\n", __func__);
			return;
		}

		vdout_fmt->reset_in_vsync = true;
		_fmt_select_vdout_cconv(true);
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s disp fmt state error\n", __func__);
			return;
		}

		disp_fmt->reset_in_vsync = true;
	}
}

void fmt_hal_hw_shadow_enable(uint32_t fmt_id)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	if (fmt_id > VDOUT_FMT)
		return;

	_fmt_mutex_lock();

	if (fmt_id == VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s vdout fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		vdout_fmt->sw_fmt_base->rField.fgLayer3_En = 1;
		vdout_fmt->shadow_en = 1;
		WriteREG32Msk((vdout_fmt->hw_fmt_base + VDOUT_FMT_REG_FMT_CTRL), (1 << 28), (1 << 28));
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s disp fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		disp_fmt->sw_fmt_base->rField.fgLayer3_En = 1;
		disp_fmt->shadow_en = 1;
		if (!disp_fmt->is_sec)
			WriteREG32Msk((disp_fmt->hw_fmt_base + DISP_FMT_REG_ACTIVE_H), (1 << 29), (1 << 29));
		else
			REG_SET(*disp_fmt->reg_mode, REG_MASK(DISP_FMT_REG_ACTIVE_H / 4));
	}

	_fmt_mutex_unlock();
}

void fmt_hal_update_hw_register(void)
{
	/*update register */
	_fmt_dispfmt_setting(DISP_FMT_MAIN);
	_fmt_dispfmt_setting(DISP_FMT_SUB);
	_fmt_vdoutfmt_setting(VDOUT_FMT);
	_fmt_vdoutfmt_setting(VDOUT_FMT_SUB);
	/*trigger register */
	_fmt_dispfmt_trigger(DISP_FMT_MAIN);
	_fmt_dispfmt_trigger(DISP_FMT_SUB);
	_fmt_vdoutfmt_trigger(false);
}

void fmt_hal_reset(uint32_t fmt_id)
{
	_fmt_mutex_lock();

	_fmt_hal_reset(fmt_id);

	_fmt_mutex_unlock();
}

void fmt_hal_set_pllgp_hdmidds(uint32_t eRes, bool en, bool set_pll, bool fractional)
{
	bool fgHd = false;

	_fmt_mutex_lock();

	fgHd = fgIsHDRes(eRes);
	if (en) {
		if (set_pll) {
			if (fractional)
				disp_clock_set_pll(DISP_CLK_TVDPLL, 593400000);
			else
				disp_clock_set_pll(DISP_CLK_TVDPLL, 594000000);
		}
		disp_clock_enable(DISP_CLK_TVDPLL, true);
		if (fgHd)
			disp_clock_enable(DISP_CLK_HD_SEL, true);
		else
			disp_clock_enable(DISP_CLK_SD_SEL, true);
	} else {
		disp_clock_enable(DISP_CLK_TVDPLL, false);
		if (fgHd)
			disp_clock_enable(DISP_CLK_HD_SEL, false);
		else
			disp_clock_enable(DISP_CLK_SD_SEL, false);
	}

	_fmt_mutex_unlock();
}

void fmt_hal_set_tv_type(uint32_t fmt_id, uint32_t tv_type)
{
	uint32_t tv_mode;
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_D("fmt_id: %d, tv_type: %d\n", fmt_id, tv_type);

	if (fmt_id > VDOUT_FMT)
		return;

	_fmt_mutex_lock();

	tv_mode = 0;
	if (tv_type == FMT_TV_TYPE_NTSC)
		tv_mode = 0;
	else if (tv_type == FMT_TV_TYPE_PAL_M)
		tv_mode = 1;
	else if (tv_type == FMT_TV_TYPE_PAL_N)
		tv_mode = 2;
	else if (tv_type == FMT_TV_TYPE_PAL)
		tv_mode = 3;

	if (fmt_id == VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s vdout fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		vdout_fmt->sw_fmt_base->rField.u1TVMODE = tv_mode;

		_fmt_write_reg(fmt_id, (void *)vdout_fmt, VDOUT_FMT_REG_FMT_MODE, true);
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s disp fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		disp_fmt->sw_fmt_base->rField.u1TVMODE = tv_mode;

		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_MODE_CTRL, true);
	}

	_fmt_mutex_unlock();
}

void fmt_hal_set_background(uint32_t fmt_id, uint32_t background)
{
	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_D("fmt_id: %d, background: 0x%x\n", fmt_id, background);

	if (fmt_id > VDOUT_FMT)
		return;

	_fmt_mutex_lock();

	if (fmt_id == VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s vdout fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		vdout_fmt->sw_fmt_base->rField.u1BGY = (background & 0xF00000) >> 20;
		vdout_fmt->sw_fmt_base->rField.u1BGCB = (background & 0x00F000) >> 12;
		vdout_fmt->sw_fmt_base->rField.u1BGCR = (background & 0x0000F0) >> 4;

		_fmt_write_reg(fmt_id, (void *)vdout_fmt, VDOUT_FMT_REG_BG_COLOR, false);
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s disp fmt state error\n", __func__);
			_fmt_mutex_unlock();
			return;
		}

		disp_fmt->sw_fmt_base->rField.u1BGY = (background & 0xF00000) >> 20;
		disp_fmt->sw_fmt_base->rField.u1BGCB = (background & 0x00F000) >> 12;
		disp_fmt->sw_fmt_base->rField.u1BGCR = (background & 0x0000F0) >> 4;

		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_BG_COLOR, false);
	}

	_fmt_mutex_unlock();
}

void fmt_hal_not_mix_plane(uint32_t plane)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();

	_fmt_mutex_lock();

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return;
	}

	switch (plane) {
	case FMT_HW_PLANE_2:
		vdout_fmt->sw_fmt_base->rField.fgPLN2_OFF = 1;
		break;
	case FMT_HW_PLANE_3:
		vdout_fmt->sw_fmt_base->rField.fgPLN3_OFF = 1;
		break;
	default:
		break;
	}

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_PLANE, true);

	_fmt_mutex_unlock();
}


void fmt_hal_mix_plane(uint32_t plane)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();

	_fmt_mutex_lock();

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return;
	}

	switch (plane) {
	case FMT_HW_PLANE_2:
		vdout_fmt->sw_fmt_base->rField.fgPLN2_OFF = 0;
		break;
	case FMT_HW_PLANE_3:
		vdout_fmt->sw_fmt_base->rField.fgPLN3_OFF = 0;
		break;
	default:
		break;
	}

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_PLANE, true);

	_fmt_mutex_unlock();
}

void fmt_hal_disable_video_plane(uint32_t fmt_id)
{
	FMT_FUNC();

	if (fmt_id == DISP_FMT_SUB)
		fmt_hal_not_mix_plane(FMT_HW_PLANE_2);
}

bool fmt_hal_plane_is_mix(uint32_t plane)
{
	bool fgMixPlane = false;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();

	_fmt_mutex_lock();

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return false;
	}

	switch (plane) {
	case FMT_HW_PLANE_1:
		fgMixPlane = true;
		break;
	case FMT_HW_PLANE_2:
		fgMixPlane = (vdout_fmt->sw_fmt_base->rField.fgPLN2_OFF == 0) ? true : false;
		break;
	case FMT_HW_PLANE_3:
		fgMixPlane = (vdout_fmt->sw_fmt_base->rField.fgPLN3_OFF == 0) ? true : false;
		break;
	default:
		break;
	}

	_fmt_mutex_unlock();

	return fgMixPlane;

}

void fmt_hal_disable_plane(enum FMT_PLANE plane)
{
	FMT_FUNC();

	if (plane == SUB_VIDEO_PLANE)
		fmt_hal_not_mix_plane(FMT_HW_PLANE_2);
	else if (plane == OSD_PRE_MIX_PLANE)
		fmt_hal_not_mix_plane(FMT_HW_PLANE_3);
}

void fmt_hal_set_alpha(uint32_t fmt_id, uint32_t alpha)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, alpha: 0x%x\n", fmt_id, alpha);

	if (fmt_id > VDOUT_FMT)
		return;

	_fmt_mutex_lock();

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return;
	}

	vdout_fmt->sw_fmt_base->rField.u1VDO1_RATIO = 1;

	if (fmt_id == DISP_FMT_MAIN) {
		vdout_fmt->sw_fmt_base->rField.fgVDO1_R_EN = 1;
		vdout_fmt->sw_fmt_base->rField.u1VDO1_RATIO = alpha;
	} else if (fmt_id == DISP_FMT_SUB) {
		vdout_fmt->sw_fmt_base->rField.fgVDO2_R_EN = 1;
		vdout_fmt->sw_fmt_base->rField.u1VDO2_RATIO = alpha;
	}

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_RATIO, false);

	_fmt_mutex_unlock();
}

int32_t fmt_hal_set_active_zone_by_res(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution,
				       uint16_t width, uint16_t height)
{
	int32_t x_offset;
	int32_t y_odd_offset;
	int32_t y_even_offset;
	struct fmt_active_info active_info = { 0 };

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, resolution: %d, width: %d, height: %d\n", fmt_id, resolution, width,
		  height);

	if (fmt_id > VDOUT_FMT)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	if (height) {
		switch (height) {
		case 480:

			x_offset = 0x63;
			y_odd_offset = 0x31;
			y_even_offset = 0x31;
			break;

		case 576:

			x_offset = 0x6D;
			y_odd_offset = 0x31;
			y_even_offset = 0x31;
			break;

		case 720:

			x_offset = 0xC5;
			y_odd_offset = 0x26;
			y_even_offset = 0x26;
			break;

		case 1080:

			x_offset = 0xC1;
			y_odd_offset = 0x2A;
			y_even_offset = 0x2A;
			break;

		case 2160:
			x_offset = 0x181;
			y_odd_offset = 0x53;
			y_even_offset = 0x53;
			break;

		default:
			x_offset = 0x63;
			y_odd_offset = 0x31;
			y_even_offset = 0x31;
			break;

		}
	} else {
		switch (resolution) {
		case HDMI_VIDEO_720x480i_60Hz:
		case HDMI_VIDEO_720x480p_60Hz:
			x_offset = 0x63;
			y_odd_offset = 0x31;
			y_even_offset = 0x31;
			width = 720;
			height = 480;
			break;
		case HDMI_VIDEO_720x576i_50Hz:
		case HDMI_VIDEO_720x576p_50Hz:
			x_offset = 0x6D;
			y_odd_offset = 0x31;
			y_even_offset = 0x31;
			width = 720;
			height = 576;
			break;
		case HDMI_VIDEO_1280x720p_60Hz:
		case HDMI_VIDEO_1280x720p_50Hz:
			x_offset = 0xC5;
			y_odd_offset = 0x26;
			y_even_offset = 0x26;
			width = 1280;
			height = 720;
			break;
		case HDMI_VIDEO_1920x1080i_60Hz:
		case HDMI_VIDEO_1920x1080i_50Hz:
		case HDMI_VIDEO_1920x1080p_60Hz:
		case HDMI_VIDEO_1920x1080p_50Hz:
		case HDMI_VIDEO_1920x1080p_24Hz:
		case HDMI_VIDEO_1920x1080p_23Hz:
		case HDMI_VIDEO_1920x1080p_25Hz:
		case HDMI_VIDEO_1920x1080p_29Hz:
		case HDMI_VIDEO_1920x1080p_30Hz:
			x_offset = 0xC1;
			y_odd_offset = 0x2A;
			y_even_offset = 0x2A;
			width = 1920;
			height = 1080;
			break;
		case HDMI_VIDEO_3840x2160P_23_976HZ:
		case HDMI_VIDEO_3840x2160P_24HZ:
		case HDMI_VIDEO_3840x2160P_25HZ:
		case HDMI_VIDEO_3840x2160P_29_97HZ:
		case HDMI_VIDEO_3840x2160P_30HZ:
		case HDMI_VIDEO_3840x2160P_60HZ:
		case HDMI_VIDEO_3840x2160P_50HZ:
			x_offset = 0x181;
			y_odd_offset = 0x53;
			y_even_offset = 0x53;
			width = 3840;
			height = 2160;
			break;
		default:
			x_offset = 0xC1;
			y_odd_offset = 0x2A;
			y_even_offset = 0x2A;
			width = 1920;
			height = 1080;
			break;
		}
	}

	active_info.h_begine = x_offset;
	active_info.h_end = x_offset + width - 1;
	active_info.v_even_begine = y_odd_offset;
	active_info.v_even_end = y_odd_offset + height - 1;
	active_info.v_odd_begine = y_even_offset;
	active_info.v_odd_end = y_even_offset + height - 1;

	_fmt_mutex_unlock();

	return fmt_hal_set_active_zone(fmt_id, &active_info);
}

int32_t fmt_hal_set_active_zone(uint32_t fmt_id, struct fmt_active_info *active_info)
{

	struct disp_fmt_t *disp_fmt = NULL;
	struct vdout_fmt_t *vdout_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_D("h_b: 0x%x, odd_b: 0x%x, even_b: 0x%x\n",
		  active_info->h_begine, active_info->v_even_begine, active_info->v_odd_begine);

	if (fmt_id > VDOUT_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	if (fmt_id >= VDOUT_FMT) {
		vdout_fmt = fmt_hal_get_vdoutfmt_reg(fmt_id);
		if (vdout_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s state error\n", __func__);
			_fmt_mutex_unlock();
			return FMT_STATE_ERR;
		}

		vdout_fmt->sw_fmt_base->rField.u2HACTBGN = active_info->h_begine;
		vdout_fmt->sw_fmt_base->rField.u2HACTEND = active_info->h_end;

		vdout_fmt->sw_fmt_base->rField.u2VOACTBGN = active_info->v_even_begine;
		vdout_fmt->sw_fmt_base->rField.u2VOACTEND = active_info->v_even_end;

		vdout_fmt->sw_fmt_base->rField.u2VEACTBGN = active_info->v_odd_begine;
		vdout_fmt->sw_fmt_base->rField.u2VEACTEND = active_info->v_odd_end;

		_fmt_write_reg(fmt_id, (void *)vdout_fmt, VDOUT_FMT_REG_ACTIVE_H, true);
		_fmt_write_reg(fmt_id, (void *)vdout_fmt, VDOUT_FMT_REG_ODD_ACTIVE_V, true);
		_fmt_write_reg(fmt_id, (void *)vdout_fmt, VDOUT_FMT_REG_EVEN_ACTIVE_V, true);
	} else {
		disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
		if (disp_fmt->status == FMT_STA_UNUSED) {
			FMT_LOG_E("%s state error\n", __func__);
			_fmt_mutex_unlock();
			return FMT_STATE_ERR;
		}

		disp_fmt->sw_fmt_base->rField.u2HACTBGN = active_info->h_begine;
		disp_fmt->sw_fmt_base->rField.u2HACTEND = active_info->h_end;

		disp_fmt->sw_fmt_base->rField.u2VOACTBGN = active_info->v_even_begine;
		disp_fmt->sw_fmt_base->rField.u2VOACTEND = active_info->v_even_end;

		disp_fmt->sw_fmt_base->rField.u2VEACTBGN = active_info->v_odd_begine;
		disp_fmt->sw_fmt_base->rField.u2VEACTEND = active_info->v_odd_end;

		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_ACTIVE_H, true);
		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_ODD_ACTIVE_V, true);
		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_EVEN_ACTIVE_V, true);
	}

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_set_delay_by_res(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution)
{
	int i = 0;
	const struct fmt_delay_info *delay;
	uint32_t size = 0;

	if (fmt_id != DISP_FMT_MAIN)
		return FMT_PARAR_ERR;

	size = ARRAY_SIZE(dispfmt_delay_table);

	for (i = 0; i < size; i++) {
		delay = &dispfmt_delay_table[i];
		if (resolution == delay->res)
			break;
	}

	if (i == size) {
		FMT_LOG_E("can not find out the delay info\n");
		return 0;
	}

	return fmt_hal_set_delay(fmt_id, delay->adj_f, delay->h_delay, delay->v_delay);
}

int32_t fmt_hal_set_delay(uint32_t fmt_id, uint32_t adj_f, uint32_t h_delay, uint32_t v_delay)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	DISP_LOG_DEBUG("fmt_id: %d, h_delay: %d, v_delay: %d\n", fmt_id, h_delay, v_delay);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}


	disp_fmt->sw_fmt_base->rField.fgADJ_FWD = adj_f;
	disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY = h_delay;
	disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY = v_delay;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DELAY, false);

	return FMT_OK;
}

int32_t fmt_hal_set_h_scale_coef(uint32_t fmt_id, uint32_t enable_16phase)
{
	struct disp_fmt_t *disp_fmt = NULL;
	int i = 0;

	FMT_FUNC();

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	if (enable_16phase) {
		for (i = 0; i < 16; i ++) {
			disp_fmt->sw_fmt_base->au4Reg[i] = scale_16_pahse_coef_y[i];
			_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_Y_H_COFF0 + 0x4 * i, true);
		}

		disp_fmt->sw_fmt_base->rField.u4YHCOEF19 = scale_16_pahse_coef_y[16];
		_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_Y_H_COFF19, true);
	}

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_set_pixel_factor(uint32_t fmt_id, uint32_t pixel_len, uint32_t hsfactor, uint32_t enable, uint32_t factor_linear_coef)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_D("fmt_id: %d, pixel_len: %d, hsfactor: %d\n", fmt_id, pixel_len, hsfactor);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.u2PXLLEN = pixel_len;
	disp_fmt->sw_fmt_base->rField.u2HSFACTOR = hsfactor;
	disp_fmt->sw_fmt_base->rField.fgHSON = enable;

	if (enable && (hsfactor == 0x1000))
		factor_linear_coef = 1;
	disp_fmt->sw_fmt_base->rField.fgHSLR = factor_linear_coef;

	disp_fmt->sw_fmt_base->rField.fgPHASE_16 = 1;
	disp_fmt->sw_fmt_base->rField.fgDERING_EN_Y = 1;
	disp_fmt->sw_fmt_base->rField.fgDERING_EN_C = 1;
	disp_fmt->sw_fmt_base->rField.u1DERING_TRANS = 0x2;
	disp_fmt->sw_fmt_base->rField.u2DERING_THRESHOLD_Y = 0x1C;
	disp_fmt->sw_fmt_base->rField.u2DERING_THRESHOLD_C = 0x1C;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DERING, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_PIXEL_LENGTH, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HORIZONTAL_SCALING, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}
int32_t fmt_hal_dsd_set_mode(uint32_t fmt_id, enum DSD_CASE_E dsd_case, int h_total, int v_total)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, dsd_case: %d, h_total: %d, v_total: %d\n", fmt_id, dsd_case, h_total, v_total);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.u2V_TOTAL = v_total;
	disp_fmt->sw_fmt_base->rField.u2H_TOTAL = h_total;
	disp_fmt->sw_fmt_base->rField.fgADJ_T = 1;
	switch (dsd_case) {
	case DSD_4K_TO_1080P:
	disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = v_total * 2;
	disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = h_total * 2;
		break;
	case DSD_4K_TO_720P:
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = v_total * 2;

		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = h_total * 4;
		break;
	case DSD_4K_TO_480P:
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = v_total * 4;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = h_total * 6;
		break;
	case DSD_1080P_TO_480P:
	case DSD_720P_TO_480P:
		disp_fmt->sw_fmt_base->rField.u2V_TOTAL_MIX = v_total * 2;
		disp_fmt->sw_fmt_base->rField.u2H_TOTAL_MIX = h_total * 3;
		break;
	default:
		break;
	}

	disp_fmt->sw_fmt_base->rField.fgADJ_T_MIX  = 1;
	disp_fmt->sw_fmt_base->rField.fgNEW_SD_MODE = 1;
	disp_fmt->sw_fmt_base->rField.fgDSDCLK_EANBLE = 1;
	if (dsd_case == DSD_4K_TO_480P) {
		disp_fmt->sw_fmt_base->rField.fgNEW_SD_USE_LOW = 1;
		disp_fmt->sw_fmt_base->rField.fgNEW_SD_4SEL1_EN = 1;
	} else {
		disp_fmt->sw_fmt_base->rField.fgNEW_SD_USE_LOW = 0;
		disp_fmt->sw_fmt_base->rField.fgNEW_SD_4SEL1_EN = 0;
	}
	disp_fmt->sw_fmt_base->rField.fgNEW_4K_DOWN_MODE = 1;
	disp_fmt->sw_fmt_base->rField.fgNOT_PST_D2 = 1;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HV_TOTAL_MIX, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HV_TOTAL, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_MODE_CTRL, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_PIXEL_LENGTH, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_RATIO, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_dsd_enable(uint32_t fmt_id, struct fmt_dsd_scl_info *info)
{
	struct disp_fmt_t *disp_fmt = NULL;
	int pixel_advance1 = 24 * 4;
	int pixel_advance2 = 20;
	int ratio = 4;
	int start = 0;
	int start1 = 0;

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, src xy(%d,%d) wh(%d,%d), dst xy(%d,%d) wh(%d,%d)\n", fmt_id,
		info->src_x, info->src_y, info->src_w, info->src_h,
		info->out_x, info->out_y, info->out_w, info->out_h);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	/*0x6c H scale factor  Hdownscale need change scale factor by srcw and outw*/
	disp_fmt->sw_fmt_base->rField.fgHDWN_EN = 1;
	disp_fmt->sw_fmt_base->rField.fgFULL_FIFO = 1;
	disp_fmt->sw_fmt_base->rField.fgHDWN_444 = 1;
	disp_fmt->sw_fmt_base->rField.u2HDWN_FAC = (info->src_w > info->out_w) ?
		(0x1000 * info->src_w / info->out_w) : 0x1000;

	/**0xc0 is the same as the 0xA0 in vdout fmt*/
	disp_fmt->sw_fmt_base->rField.u2DowndScaleStart = info->active_start;

	disp_fmt->sw_fmt_base->rField.u2DowndScaleEnd = info->active_end;

	/*0x7c*/
	start = info->active_start;
	start1 = info->original_active_start;
	switch (info->dsd_case) {
	case DSD_4K_TO_720P:
		ratio = 8; /*8fs*/
		/*7c_start = c0 start x 8 - 24x4x8*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_BGN =
			(start * ratio - pixel_advance1 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBEG_14_13 =
			((start * ratio - pixel_advance1 * ratio) & 0x6FFF) >> 13;
		/*7c_end = c0_start+active w * 8*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_END =
			(start * ratio - pixel_advance1 * ratio + 1280 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND_14_13 =
			((start * ratio - pixel_advance1 * ratio + 1280 * ratio) & 0x6FFF) >> 13;
		disp_fmt->sw_fmt_base->rField.u2HDWN_CYCLE_2_0 = 7;
		disp_fmt->sw_fmt_base->rField.u2Cycle_4_3 = 0;


		info->out_h_start = disp_fmt->sw_fmt_base->rField.u2DowndScaleStart;
		info->out_h_end = disp_fmt->sw_fmt_base->rField.u2DowndScaleEnd - 5;

		start = start1 * ratio - pixel_advance1 * ratio;
		break;
	case DSD_4K_TO_1080P:
		ratio = 4; /*4fs*/
		/*7c_start = c0 start x 4 - 24x4x4*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_BGN =
			(start * ratio - pixel_advance1 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBEG_14_13 =
			((start * ratio - pixel_advance1 * ratio) & 0x6FFF) >> 13;
		/*7c_end = c0_start+active w * 4*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_END =
			(start * ratio - pixel_advance1 * ratio + 1920 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND_14_13 =
			((start * ratio - pixel_advance1 * ratio + 1920 * ratio) & 0x6FFF) >> 13;
		disp_fmt->sw_fmt_base->rField.u2HDWN_CYCLE_2_0 = 3;
		disp_fmt->sw_fmt_base->rField.u2Cycle_4_3 = 0;


		info->out_h_start = disp_fmt->sw_fmt_base->rField.u2DowndScaleStart;
		info->out_h_end = disp_fmt->sw_fmt_base->rField.u2DowndScaleEnd - 5;
		start = start1 * ratio - pixel_advance1 * ratio;
		break;
	case DSD_4K_TO_480P:
		ratio = 24; /*24fs*/
		/*7c_start = c0 start x 24 - 24x4x24*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_BGN =
			(start * ratio - pixel_advance1 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBEG_14_13 =
			((start * ratio - pixel_advance1 * ratio) & 0x6FFF) >> 13;
		/*7c_end = c0_start+active w * 24*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_END =
			(start * ratio - pixel_advance1 * ratio + 720 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND_14_13 =
			((start * ratio - pixel_advance1 * ratio + 720 * ratio) & 0x6FFF) >> 13;
		disp_fmt->sw_fmt_base->rField.u2HDWN_CYCLE_2_0 = 7;
		disp_fmt->sw_fmt_base->rField.u2Cycle_4_3 = 2;
		start = start1 * ratio - pixel_advance1 * ratio;
		break;
	case DSD_1080P_TO_480P:
	case DSD_720P_TO_480P:
		ratio = 6; /*6fs*/
		pixel_advance1 = 12*4;
		/*7c_start = c0 start x ratio - 12*4*ratio*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_BGN =
			(start * ratio - pixel_advance1 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBEG_14_13 =
			((start * ratio - pixel_advance1 * ratio) & 0x6FFF) >> 13;
		/*7c_end = c0_start+active w * ratio*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HO_END =
			(start * ratio - pixel_advance1 * ratio + 720 * ratio) & 0x1FFF;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND_14_13 =
			((start * ratio - pixel_advance1 * ratio + 720 * ratio) & 0x6FFF) >> 13;
		disp_fmt->sw_fmt_base->rField.u2HDWN_CYCLE_2_0 = 5;
		disp_fmt->sw_fmt_base->rField.u2Cycle_4_3 = 0;
		start = start1 * ratio - pixel_advance1 * ratio;
		break;
	default:
		FMT_LOG_E("unsupport case: %d\n", info->dsd_case);
		_fmt_mutex_unlock();
		return -1;
	}
	switch (info->dsd_case) {
	case DSD_4K_TO_720P:
	case DSD_4K_TO_1080P:
	case DSD_4K_TO_480P:
		/*0x70*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN =
			start - pixel_advance2 * 4 + info->src_x;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN + info->src_w - 1;
		disp_fmt->sw_fmt_base->rField.u2HACTBGN =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN - 5;
		disp_fmt->sw_fmt_base->rField.u2HACTEND =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HEND - 5;
		FMT_LOG_I("0xA0 s:0x%x |e:0x%x\n", disp_fmt->sw_fmt_base->rField.u2HACTBGN,
			disp_fmt->sw_fmt_base->rField.u2HACTEND);
		break;
	case DSD_1080P_TO_480P:
	case DSD_720P_TO_480P:
		pixel_advance2 = 21;
		/*0x70*/
		disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN =
			start - pixel_advance2 * 4 + info->src_x;
		disp_fmt->sw_fmt_base->rField.u2HDWN_HEND =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN + info->src_w - 1;
		/*0xA0*/
		disp_fmt->sw_fmt_base->rField.u2HACTBGN =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HBGN - 5;
		disp_fmt->sw_fmt_base->rField.u2HACTEND =
			disp_fmt->sw_fmt_base->rField.u2HDWN_HEND - 5;
		FMT_LOG_I("0xA0 s:0x%x |e:0x%x\n", disp_fmt->sw_fmt_base->rField.u2HACTBGN,
			disp_fmt->sw_fmt_base->rField.u2HACTEND);
		break;
	default:
		FMT_LOG_E("unsupport case: %d\n", info->dsd_case);
		_fmt_mutex_unlock();
		return -1;
	}
	/*0x74*/
	disp_fmt->sw_fmt_base->rField.u2HDWN_VOBGN = disp_fmt->sw_fmt_base->rField.u2VOACTBGN;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VOEND = disp_fmt->sw_fmt_base->rField.u2VEACTEND;
	/*0x78*/
	disp_fmt->sw_fmt_base->rField.u2HDWN_VEBGN = disp_fmt->sw_fmt_base->rField.u2VEACTBGN;
	disp_fmt->sw_fmt_base->rField.u2HDWN_VEEND = disp_fmt->sw_fmt_base->rField.u2VEACTEND;
	/*0xb0, reset factor*/
	disp_fmt->sw_fmt_base->rField.u2HSFACTOR = 0x1000;

	/*0xcc*/
	disp_fmt->sw_fmt_base->rField.u2DemoModeWidth = 1;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_CTRL, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_H, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_VO, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_VE, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_RNG_OUTPUT, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DOWN_CONFIG, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_CONFIG, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_ACTIVE_H, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_HORIZONTAL_SCALING, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_dsd_set_delay(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, resolution: %d\n", fmt_id, resolution);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;


	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		return FMT_STATE_ERR;
	}

	_fmt_mutex_lock();

	disp_fmt->sw_fmt_base->rField.u2DSDHSYN_DELAY = disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY;
	disp_fmt->sw_fmt_base->rField.u2DSDVSYN_DELAY = disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY;
	disp_fmt->sw_fmt_base->rField.fgADJ_FWD = disp_fmt->sw_fmt_base->rField.fgADJ_FWD;

	switch (resolution) {
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
	case HDMI_VIDEO_1920x1080p_25Hz:
	case HDMI_VIDEO_1920x1080p_24Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
	case HDMI_VIDEO_1920x1080p_29Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
		disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY =
			disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY * 2;
		disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY =
			disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY * 2;
		break;
	case HDMI_VIDEO_1920x1080p_60Hz:
		/*temp solution*/
		FMT_LOG_I("set dsd delay for 1080p60\n");
		disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY = 0xFBC;
		disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY = 0x1188;
		disp_fmt->sw_fmt_base->rField.u2DSDHSYN_DELAY = 0x82E;
		disp_fmt->sw_fmt_base->rField.u2DSDVSYN_DELAY = 0x8C4;
		break;
	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
		disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY =
			disp_fmt->sw_fmt_base->rField.u2HSYN_DELAY * 4;
		disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY =
			disp_fmt->sw_fmt_base->rField.u2VSYN_DELAY * 2;
		break;
	default:
		FMT_LOG_E("unsupport resolution=%d\n", resolution);
	}

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DELAY, true);
	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_DSD_DELAY, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_h_down_enable(uint32_t fmt_id, struct fmt_hd_scl_info *info)
{
	int factor = 0;
	int in_h_start = 0;
	int in_h_end = 0;
	int out_h_start = 0;
	int out_h_end = 0;
	int in_h_limit = 0;

	FMT_FUNC();

	if (!info)
		return FMT_PARAR_ERR;

	FMT_LOG_D("[DOWNSCL]fmt_id: %d, src_w: %d, out_w: %d, x_pos: %d, out_x_oft: %d\n",
		fmt_id, info->src_w, info->out_w, info->in_x_pos, info->in_x_pos);
	FMT_LOG_D("[DOWNSCL]odd_s: %d, odd_e: %d, even_s: %d, even_e: %d, res: %d\n",
		info->out_y_odd_pos, info->out_y_odd_pos_e, info->out_y_even_pos,
		info->out_y_even_pos_e, info->res);

	_fmt_mutex_lock();

	factor = (info->src_w * 0x1000) / info->out_w;

	switch (info->res) {
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x576i_50Hz:
	case HDMI_VIDEO_720x480p_60Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		in_h_start = info->in_x_pos + 2;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 3;
		out_h_start = info->in_x_pos + info->out_x_offset + 3;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 3;

		in_h_limit = 720;

		break;
	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
	case HDMI_VIDEO_1280x720p3d_60Hz:
	case HDMI_VIDEO_1280x720p3d_50Hz:
		in_h_start = info->in_x_pos + 5;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 6;
		out_h_start = info->in_x_pos + info->out_x_offset + 5;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 7;

		in_h_limit = 1280;

		break;
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
	case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080i3d_50Hz:
		in_h_start = info->in_x_pos + 5;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 6;
		out_h_start = info->in_x_pos + info->out_x_offset + 5;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 5;

		in_h_limit = 1920;

		break;

	case HDMI_VIDEO_1920x1080p_24Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
	case HDMI_VIDEO_1920x1080p_25Hz:
	case HDMI_VIDEO_1920x1080p_29Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
	case HDMI_VIDEO_1920x1080p3d_23Hz:
		in_h_start = info->in_x_pos + 4;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 5;
		out_h_start = info->in_x_pos + info->out_x_offset + 5;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 6;

		in_h_limit = 1920;

		break;
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
		in_h_start = info->in_x_pos + 4;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 5;
		out_h_start = info->in_x_pos + info->out_x_offset + 3;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 5;

		in_h_limit = 4096;

		break;
	case HDMI_VIDEO_4096x2160P_24HZ:
	case HDMI_VIDEO_4096x2160P_60HZ:
	case HDMI_VIDEO_4096x2160P_50HZ:
		in_h_start = info->in_x_pos + 4;
		in_h_end = info->in_x_pos + (info->src_w - 1) + 5;
		out_h_start = info->in_x_pos + info->out_x_offset + 8;
		out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1) + 8;

		in_h_limit = 4096;

		break;
	default:
		break;
	};
	FMT_LOG_D("[DOWNSCL]in_h_start: %d, in_h_end: %d, out_h_start: %d, out_h_end: %d\n",
		in_h_start, in_h_end, out_h_start, out_h_end);

	if (out_h_start >= out_h_end || info->out_y_even_pos >= info->out_y_even_pos_e
		|| info->out_y_odd_pos >= info->out_y_odd_pos_e
		|| info->out_w >= info->src_w || info->out_w == 0 || info->src_w == 0
		|| factor > 0xFFFF /*|| info->out_w & 1*/|| info->src_w > in_h_limit) {
		FMT_LOG_I("parameter error\n");
		_fmt_mutex_unlock();
		return FMT_PARAR_ERR;
	}

	_fmt_set_in_region(fmt_id, in_h_start, in_h_end, info->out_y_odd_pos, info->out_y_odd_pos_e,
			info->out_y_even_pos, info->out_y_odd_pos_e);
	_fmt_set_out_region(fmt_id, out_h_start, out_h_end);

	info->dispfmt_out_h_start = out_h_start;
	info->dispfmt_out_h_end = out_h_end;
	info->vdout_out_h_start = info->in_x_pos + info->out_x_offset;
	info->vdout_out_h_end = info->in_x_pos + info->out_x_offset + (info->out_w - 1);

	FMT_LOG_D("[DOWNSCL]out_h_s: %d, out_h_e: %d\n", info->vdout_out_h_start, info->vdout_out_h_end);

	if (info->src_w <= 1920)
		_fmt_hd_scl_enable(fmt_id, factor, true, false);
	else
		_fmt_hd_scl_enable(fmt_id, factor, true, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_h_down_disable(uint32_t fmt_id)
{
	FMT_FUNC();
	_fmt_mutex_lock();

	_fmt_hd_scl_enable(fmt_id, 0x1000, false, true);

	_fmt_mutex_unlock();

	return FMT_OK;
}

int32_t fmt_hal_h_down_cap(uint32_t fmt_id, uint32_t src_w, uint32_t out_w, uint32_t out_h)
{
	int factor = 0;
	int in_h_limit = 0;

	FMT_FUNC();
	FMT_LOG_D("fmt_id: %d, src_w: %d, out_w: %d, out_h: %d\n",
		fmt_id, src_w, out_w, out_h);
	_fmt_mutex_lock();

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
		|| ((out_h < 1000) && ((out_w << 1) >= src_w))) {
		FMT_LOG_I("Not able to downscale\n");
		_fmt_mutex_unlock();
		return 0;
	}

	FMT_LOG_I("Be able to downscale\n");
	_fmt_mutex_unlock();
	return 1;
}

void fmt_hal_shadow_update(void)
{
	_fmt_vdoutfmt_trigger(true);
}

int32_t fmt_hal_set_output_444(uint32_t fmt_id, bool is_444)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	DISP_LOG_DEBUG("fmt_id: %d, is_444: %d,\n", fmt_id, is_444);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.OUTPUT_444 = is_444;

	disp_fmt->sw_fmt_base->rField.fgLayer3_Trigger = 1;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_FORMAT_422_CTRL, true);

	_fmt_mutex_unlock();

	return 0;
}

int32_t fmt_hal_set_uv_swap(uint32_t fmt_id, bool swap)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_D("fmt_id: %d, swap: %d,\n", fmt_id, swap);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	if (disp_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return FMT_STATE_ERR;
	}

	disp_fmt->sw_fmt_base->rField.fgUVSW = swap;

	_fmt_write_reg(fmt_id, (void *)disp_fmt, DISP_FMT_REG_VDO_FMT_CTRL, true);

	_fmt_mutex_unlock();

	return 0;
}

int32_t fmt_hal_show_colorbar(bool en)
{
	if (en) {
		fmt_hal_set_active_zone_by_res(VDOUT_FMT, fmt.res, 0, 0);
		fmt_hal_not_mix_plane(FMT_HW_PLANE_2);
		fmt_hal_not_mix_plane(FMT_HW_PLANE_3);
		_fmt_set_buildin_color(true);
	} else {
		fmt_hal_mix_plane(FMT_HW_PLANE_3);
		_fmt_set_buildin_color(false);
	}

	return FMT_OK;
}

int32_t fmt_hal_set_secure(uint32_t fmt_id, bool is_secure)
{
	struct disp_fmt_t *disp_fmt = NULL;

	FMT_FUNC();
	FMT_LOG_I("fmt_id: %d, is_secure: %d,\n", fmt_id, is_secure);

	if (fmt_id > DISP_FMT_SUB)
		return FMT_PARAR_ERR;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);
	disp_fmt->is_sec = is_secure;

	_fmt_mutex_unlock();

	return 0;
}

char *fmt_hal_get_sw_register(uint32_t fmt_id, int *size, uint64_t **reg_mode)
{
	struct disp_fmt_t *disp_fmt = NULL;

	if (fmt_id > DISP_FMT_SUB)
		return NULL;

	_fmt_mutex_lock();

	disp_fmt = fmt_hal_get_dispfmt_reg(fmt_id);

	_fmt_mutex_unlock();

	*size = HAL_DISP_FMT_MAIN_REG_NUM * 4;
	*reg_mode = disp_fmt->reg_mode;

	return (char *)&disp_fmt->sw_fmt_base->au4Reg;
}

int32_t fmt_hal_clock_on_off(uint32_t fmt_id, bool on)
{
	int id = 0;

	FMT_FUNC();
	FMT_LOG_D("clock_on_off id: %d, on: %d\n", fmt_id, on);

	if (fmt_id > VDOUT_FMT)
		return FMT_PARAR_ERR;

	if (fmt_id == VDOUT_FMT) {
		disp_clock_enable(DISP_CLK_FMT_TG, on);
	} else {
		id = _fmt_get_dispfmt_id(fmt_id);

		if (id == DISP_FMT1) {
			disp_clock_enable(DISP_CLK_DISPFMT3, on);
		} else if (id == DISP_FMT2) {

			if (on) {
				/*need to open disp_main pm before enable disp_sub pm*/
				disp_clock_enable(DISP_CLK_DISPFMT3, on);
				disp_clock_enable(DISP_CLK_DISPFMT4, on);
			} else {
				disp_clock_enable(DISP_CLK_DISPFMT4, on);
				disp_clock_enable(DISP_CLK_DISPFMT3, on);
			}
		}
	}

	return 0;
}

int32_t fmt_hal_multiple_alpha(bool en)
{
	struct vdout_fmt_t *vdout_fmt = NULL;

	_fmt_mutex_lock();

	vdout_fmt = fmt_hal_get_vdoutfmt_reg(VDOUT_FMT);
	if (vdout_fmt->status == FMT_STA_UNUSED) {
		FMT_LOG_E("%s state error\n", __func__);
		_fmt_mutex_unlock();
		return -1;
	}

	if (en)
		vdout_fmt->sw_fmt_base->rField.fgPLN3_PRE_MODE = 0;
	else
		vdout_fmt->sw_fmt_base->rField.fgPLN3_PRE_MODE = 1;

	_fmt_write_reg(VDOUT_FMT, (void *)vdout_fmt, VDOUT_FMT_REG_PREMIX, true);

	_fmt_mutex_unlock();
	return 0;
}

int32_t fmt_hal_init(struct fmt_init_pararm *param)
{
	int32_t i = 0;

	FMT_FUNC();

	if (fmt.inited) {
		FMT_LOG_E("alreay inited.\n");
		return FMT_STATE_ERR;
	}
	fmt.fmt_log_level = FMT_LL_INFO;

	/* get vdout fmt and disp fmt1,2 register base from dts */
	fmt.vdout_fmt_reg_base = param->vdout_fmt_reg_base;
	fmt.io_reg_base = param->io_reg_base;

	FMT_LOG_D("vdout fmt reg_base: 0x%lx\n",
		  fmt.vdout_fmt_reg_base);

	if (fmt.vdout_fmt_reg_base == 0) {
		FMT_LOG_E("vdout fmt register base error\n\n");
		return FMT_PARAR_ERR;
	}

	for (i = 0; i < DISP_FMT_CNT; i++) {
		fmt.disp_fmt_reg_base[i] = param->disp_fmt_reg_base[i];
		FMT_LOG_D("disp fmt[%d] reg_base: 0x%lx\n", (i + 1), fmt.disp_fmt_reg_base[i]);

		if (fmt.disp_fmt_reg_base[i] == 0) {
			FMT_LOG_E("get disp fmt[%d] register base error\n", (i + 1));
			return FMT_PARAR_ERR;
		}
	}

	fmt.vdout_fmt.sw_fmt_base = kmalloc(sizeof(union vdout_fmt_union_t), GFP_KERNEL);
	fmt.sub_vdout_fmt.sw_fmt_base = kmalloc(sizeof(union vdout_fmt_union_t), GFP_KERNEL);
	fmt.main_disp_fmt.sw_fmt_base = kmalloc(sizeof(union disp_fmt_union_t), GFP_KERNEL);
	fmt.sub_disp_fmt.sw_fmt_base = kmalloc(sizeof(union disp_fmt_union_t), GFP_KERNEL);

	if (fmt.vdout_fmt.sw_fmt_base == 0 ||
	    fmt.main_disp_fmt.sw_fmt_base == 0 || fmt.sub_disp_fmt.sw_fmt_base == 0) {
		FMT_LOG_E("alloc buffer for sw register fail\n");
		return FMT_ALLOC_FAIL;
	}

	fmt.vdout_fmt.reg_mode = kmalloc(sizeof(uint64_t), GFP_KERNEL);
	fmt.sub_vdout_fmt.reg_mode = kmalloc(sizeof(uint64_t), GFP_KERNEL);
	fmt.main_disp_fmt.reg_mode = kmalloc(sizeof(uint64_t), GFP_KERNEL);
	fmt.sub_disp_fmt.reg_mode = kmalloc(sizeof(uint64_t), GFP_KERNEL);

	if (fmt.vdout_fmt.reg_mode == 0 ||
	    fmt.main_disp_fmt.reg_mode == 0 || fmt.sub_disp_fmt.reg_mode == 0) {
		FMT_LOG_E("alloc buffer for sw reg mode fail\n");
		return FMT_ALLOC_FAIL;
	}

	_fmt_mutex_init();

	_fmt_reset_fmt_info(VDOUT_FMT);
	_fmt_reset_fmt_info(VDOUT_FMT_SUB);
	fmt.vdout_fmt.hw_fmt_base = fmt.vdout_fmt_reg_base;
	fmt.sub_vdout_fmt.hw_fmt_base = fmt.vdout_fmt_reg_base + SUB_VDOUT_FMT_OFFSET;
	_fmt_reset_fmt_info(DISP_FMT_MAIN);
	_fmt_reset_fmt_info(DISP_FMT_SUB);

	fmt.main_disp_fmt.hw_fmt_base = fmt.disp_fmt_reg_base[DISP_FMT1];
	fmt.sub_disp_fmt.hw_fmt_base = fmt.disp_fmt_reg_base[DISP_FMT2];

	fmt.inited = true;
	return FMT_OK;
}

int32_t fmt_hal_deinit(void)
{
	if (fmt.vdout_fmt.sw_fmt_base == 0 || fmt.sub_vdout_fmt.sw_fmt_base == 0 ||
	    fmt.main_disp_fmt.sw_fmt_base == 0 || fmt.sub_disp_fmt.sw_fmt_base == 0) {
		FMT_LOG_I("already freed , uninit finished.\n");
		return FMT_OK;
	}

	kfree((void *)fmt.vdout_fmt.sw_fmt_base);
	fmt.vdout_fmt.sw_fmt_base = 0;

	kfree((void *)fmt.sub_vdout_fmt.sw_fmt_base);
	fmt.sub_vdout_fmt.sw_fmt_base = 0;

	kfree((void *)fmt.main_disp_fmt.sw_fmt_base);
	fmt.main_disp_fmt.sw_fmt_base = 0;

	kfree((void *)fmt.sub_disp_fmt.sw_fmt_base);
	fmt.sub_disp_fmt.sw_fmt_base = 0;

	kfree((void *)fmt.vdout_fmt.reg_mode);
	fmt.vdout_fmt.reg_mode = 0;

	kfree((void *)fmt.sub_vdout_fmt.reg_mode);
	fmt.sub_vdout_fmt.reg_mode = 0;

	kfree((void *)fmt.main_disp_fmt.reg_mode);
	fmt.main_disp_fmt.reg_mode = 0;

	kfree((void *)fmt.sub_disp_fmt.reg_mode);
	fmt.sub_disp_fmt.reg_mode = 0;

	return FMT_OK;
}
