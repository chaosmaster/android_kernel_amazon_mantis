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


#include <linux/workqueue.h>
#include <linux/vmalloc.h>

#include "disp_hdr_def.h"
#include "disp_hdr_core.h"
#include "disp_hdr_util.h"
#include "disp_hdr_device.h"
#include "disp_bt2020.h"
#include "disp_sdr2hdr.h"
#include "disp_hdr2sdr.h"
#include "disp_hw_mgr.h"
#include "disp_hdr_cli.h"
#include "disp_hdr_sec.h"
#include "disp_irq.h"
#include "vdout_sys_hal.h"
#include "disp_sys_hal.h"
#include "disp_path.h"
#include "fmt_hal.h"
#include <linux/mutex.h>



static struct workqueue_struct *gHandleHdrClockPathThred;
static struct workqueue_struct *gHdrThread[HDR_PATH_MAX] = {NULL};
static struct list_head gConfigListHead[HDR_PATH_MAX] = { {0} };
static bool gConfigListHeadInit; /* check if HDR module ready to use */

static struct HDR_BUFFER_INFO gDispBufferInfo[HDR_PATH_MAX]; /* store last frame buffer info */
static struct disp_hw_tv_capbility gTVInfo[HDR_PATH_MAX]; /* store last frame tv info */

/* checking if it configured hdr2sdr/bt2020/sdr2hsdr at the first time*/
static bool gFirstConfigure[HDR_PATH_MAX];

static bool osd_enable_sdr2hdr; /* checking if osd is showing when playing hdr source */
static bool gOsdHadConfigure; /* checking if osd path had configured*/

/*if main path is hdr, sub path is sdr, TV supports hdr, need to reconfigure sdr2hdr&bt2020 for sub path */
static bool gOsdHadConfigureForSubPath;
static bool gSubPathHavestopped;
static bool gOsbPathHaveReconfiguedForSubVideo;


static bool gOsdPathSuspend; /* checking if osd path had suspended*/

static bool gVideoIsPlaying; /* checking if it is playing video*/
static bool gVideoIsPlayingForSubPathDolbypip;
static bool gVideoIsPlayingForSubPathDolbypipHadUsed;
/**/
static bool gSubVideoCurrentPlayingState;
struct mutex sync_lock_for_sub_path;

/*
when plays hdr10+ soure on hdr10+ tv, then play pip video, it needs to new sdr2hdr register for sub video.
when press key-home, hwc stopped sub video firstly, but then HWC gives a osd graphic,it needs to reconfigured
sdr2hdr register for netflix certification if sub video stopped, but cpus do not complete reconfigure sdr2hdr register,
cpu which is used to for vdp stops osd path graphic and main video, after clear osd path sdr2hdr register, then cpus continue
complete to sdr2hdr register. So sdr2hdr register will not clear finally.
It uses mutex to make sure display configure and stop work independendly.
*/

struct mutex sync_lock_for_disp_configure_and_stop;

/*hdr2sdr bt2020 & sdr2hdr clock manager*/
static bool clockFlag[HDR_CLOCK_MODULE_MAX] = {0};


static const struct disp_hdr_dynamic_clock_on_off hdr_dynamic_table[] = {
	{2, 5, 2, 5, 1, 1, HDMI_VIDEO_720x480i_60Hz},
	{2, 5, 2, 5, 1, 1, HDMI_VIDEO_720x576i_50Hz},
	{2, 5, 2, 5, 1, 1, HDMI_VIDEO_720x480p_60Hz},
	{2, 5, 2, 5, 1, 1, HDMI_VIDEO_720x576p_50Hz},
	{2, 16, 2, 16, 1, 1, HDMI_VIDEO_1280x720p_60Hz},
	{2, 16, 2, 16, 1, 1, HDMI_VIDEO_1280x720p_50Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080i_60Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080i_50Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_30Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_25Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_24Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_23Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_29Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_60Hz},
	{2, 30, 2, 30, 1, 1, HDMI_VIDEO_1920x1080p_50Hz},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_23_976HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_24HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_25HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_29_97HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_30HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_50HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_3840x2160P_60HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_4096x2160P_24HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_4096x2160P_50HZ},
	{25, 60, 25, 60, 1, 1, HDMI_VIDEO_4096x2160P_60HZ},
};

static const struct disp_sdr2hdr_osd_path_delay sdr2hdr_osd_delay_table[] = {
	{26, HDMI_VIDEO_720x480i_60Hz},
	{26, HDMI_VIDEO_720x576i_50Hz},
	{26, HDMI_VIDEO_720x480p_60Hz},
	{26, HDMI_VIDEO_720x576p_50Hz},
	{26, HDMI_VIDEO_1280x720p_60Hz},
	{26, HDMI_VIDEO_1280x720p_50Hz},
	{26, HDMI_VIDEO_1920x1080i_60Hz},
	{26, HDMI_VIDEO_1920x1080i_50Hz},
	{26, HDMI_VIDEO_1920x1080p_30Hz},
	{26, HDMI_VIDEO_1920x1080p_25Hz},
	{26, HDMI_VIDEO_1920x1080p_24Hz},
	{26, HDMI_VIDEO_1920x1080p_23Hz},
	{26, HDMI_VIDEO_1920x1080p_29Hz},
	{26, HDMI_VIDEO_1920x1080p_60Hz},
	{26, HDMI_VIDEO_1920x1080p_50Hz},
	{26, HDMI_VIDEO_3840x2160P_23_976HZ},
	{26, HDMI_VIDEO_3840x2160P_24HZ},
	{26, HDMI_VIDEO_3840x2160P_25HZ},
	{26, HDMI_VIDEO_3840x2160P_29_97HZ},
	{26, HDMI_VIDEO_3840x2160P_30HZ},
	{26, HDMI_VIDEO_3840x2160P_50HZ},
	{26, HDMI_VIDEO_3840x2160P_60HZ},
	{26, HDMI_VIDEO_4096x2160P_24HZ},
	{26, HDMI_VIDEO_4096x2160P_50HZ},
	{26, HDMI_VIDEO_4096x2160P_60HZ},
};


enum HDR_STATUS hdr_core_config_ext_yuv2rgb_coef(struct yuv2rgb_ext_coef_struct *pYuv2rgbExtCoef,
	const MATRIX_PARAM *pMatrix)
{
	if (pYuv2rgbExtCoef == NULL) {
		HDR_ERR("invalid input param\n");
		return HDR_STATUS_INVALID_PARAM;
	}

	pYuv2rgbExtCoef->c_cf_rmx = hdr_util_translate_s3p13(pMatrix->c00);
	pYuv2rgbExtCoef->c_cf_rmy = hdr_util_translate_s3p13(pMatrix->c01);
	pYuv2rgbExtCoef->c_cf_rmz = hdr_util_translate_s3p13(pMatrix->c02);

	pYuv2rgbExtCoef->c_cf_gmx = hdr_util_translate_s3p13(pMatrix->c10);
	pYuv2rgbExtCoef->c_cf_gmy = hdr_util_translate_s3p13(pMatrix->c11);
	pYuv2rgbExtCoef->c_cf_gmz = hdr_util_translate_s3p13(pMatrix->c12);

	pYuv2rgbExtCoef->c_cf_bmx = hdr_util_translate_s3p13(pMatrix->c20);
	pYuv2rgbExtCoef->c_cf_bmy = hdr_util_translate_s3p13(pMatrix->c21);
	pYuv2rgbExtCoef->c_cf_bmz = hdr_util_translate_s3p13(pMatrix->c22);

	pYuv2rgbExtCoef->c_cf_rax = pMatrix->pre_add_0;
	pYuv2rgbExtCoef->c_cf_gax = pMatrix->pre_add_0;
	pYuv2rgbExtCoef->c_cf_bax = pMatrix->pre_add_0;

	pYuv2rgbExtCoef->c_cf_ra = 0;
	pYuv2rgbExtCoef->c_cf_ga = 0;
	pYuv2rgbExtCoef->c_cf_ba = 0;

	pYuv2rgbExtCoef->c_cf_rmz_c02_sp = pMatrix->c02_sp;
	pYuv2rgbExtCoef->c_cf_bmy_c21_sp = pMatrix->c21_sp;
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_constant_luma_input(struct yuv2rgb_config_struct *pConfig)
{
	MATRIX_PARAM matrix = {0};

	if (pConfig == NULL) {
		HDR_ERR("invalid param\n");
		return HDR_STATUS_INVALID_PARAM;
	}


	pConfig->en_yuv2rgb = ENABLE_YUV2RGB;
	pConfig->constant_luma = CONSTANT_LUMA;
	pConfig->yuv2rgb_coef = YUV2RGB_COEF_FROM_EXT_REGISTER;

	matrix.c00 = 9539;
	matrix.c01 = 0;
	matrix.c02 = 16025;
	matrix.c10 = 9539;
	matrix.c11 = 0;
	matrix.c12 = 0;
	matrix.c20 = 9539;
	matrix.c21 = 18096;
	matrix.c22 = 0;
	matrix.pre_add_0 = -256;
	matrix.pre_add_1 = -2048;
	matrix.pre_add_2 = -2048;
	matrix.c02_sp = 9266;
	matrix.c21_sp = 14750;
	hdr_core_config_ext_yuv2rgb_coef(&pConfig->yuv2rgb_ext_coef, &matrix);
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_yuv2rgb(const struct data_type_struct *pInputType,
	struct yuv2rgb_config_struct *pConfig)
{
	pConfig->update = true;
	pConfig->en_yuv2rgb = ENABLE_YUV2RGB;
	switch (pInputType->colorFormat) {
	case COLOR_FORMAT_LIMIT_709:
		pConfig->yuv2rgb_coef = LIMIT_BT709_FULL_RGB;
		break;
	case COLOR_FORMAT_LIMIT_601:
		pConfig->yuv2rgb_coef = LIMIT_BT601_FULL_RGB;
		break;
	case COLOR_FORMAT_FULL_709:
		pConfig->yuv2rgb_coef = FULL_BT709_FULL_RGB;
		break;
	case COLOR_FORMAT_FULL_601:
		pConfig->yuv2rgb_coef = FULL_BT601_FULL_RGB;
		break;

	case COLOR_FORMAT_LIMIT_2020:
	case COLOR_FORMAT_FULL_2020: /* We don't have full BT2020 format input */
		if (!pInputType->constant_luma_bt2020_input)
			pConfig->yuv2rgb_coef = LIMIT_BT2020_FULL_RGB;
		else
			hdr_core_config_constant_luma_input(pConfig);
		break;

	case COLOR_FORMAT_RGB:
		pConfig->en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->update = false;
		break;
	default:
		HDR_ERR("invalid input format:0x%x\n", pInputType->colorFormat);
		return HDR_STATUS_INVALID_INPUT_FORMAT;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_degamma(const struct data_type_struct *pInputType,
	struct degamma_config_struct *pConfig)
{
	pConfig->update = true;
	pConfig->en_degamma = ENABLE_DEGAMMA;

	switch (pInputType->gammaType) {
	case GAMMA_TYPE_2p4:
		pConfig->degamma_curve = DEGAMMA_CURVE_2p4;
		break;
	case GAMMA_TYPE_709:
		if (pInputType->colorFormat == COLOR_FORMAT_RGB)
			pConfig->degamma_curve = DEGAMMA_CURVE_SRGB;
		else
			pConfig->degamma_curve = DEGAMMA_CURVE_709;
		break;
	case GAMMA_TYPE_ST2084:
		pConfig->degamma_curve = DEGAMMA_CURVE_ST2084;
		break;
	case GAMMA_TYPE_HLG:
		pConfig->degamma_curve = DEGAMMA_CURVE_HLG;
		break;
	case GAMMA_TYPE_BYPASS:
		pConfig->en_degamma = DISABLE_DEGAMMA;
		pConfig->update = false;
		break;
	default:
		HDR_ERR("invalid gamma type:0x%x\n", pInputType->gammaType);
		return HDR_STATUS_INVALID_GAMMA_TYPE;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_gamma(const struct data_type_struct *pOutputType, struct gamma_config_struct *pConfig)
{
	pConfig->update = true;
	pConfig->en_gamma = ENABLE_GAMMA;
	switch (pOutputType->gammaType) {
	case GAMMA_TYPE_2p4:
		pConfig->gamma_curve = GAMMA_CURVE_2p4;
		break;
	case GAMMA_TYPE_709:
		pConfig->gamma_curve = GAMMA_CURVE_709;
		break;
	case GAMMA_TYPE_BYPASS:
		pConfig->en_gamma = DISABLE_GAMMA;
		pConfig->update = false;
		break;
	default:
		HDR_ERR("invalid gamma type:0x%x\n", pOutputType->gammaType);
		return HDR_STATUS_INVALID_GAMMA_TYPE;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_3x3matrix(const struct data_type_struct *pInputType,
	const struct data_type_struct *pOutputType,
	struct matrix_config_struct *pConfig)
{
	pConfig->update = true;
	pConfig->en_3x3matrix = ENABLE_3X3MATRIX;
	switch (pInputType->colorFormat << 8 | pOutputType->colorFormat) {
	case (COLOR_FORMAT_FULL_709 << 8 | COLOR_FORMAT_FULL_2020):
	case (COLOR_FORMAT_FULL_709 << 8 | COLOR_FORMAT_LIMIT_2020):
	case (COLOR_FORMAT_LIMIT_709 << 8 | COLOR_FORMAT_FULL_2020):
	case (COLOR_FORMAT_LIMIT_709 << 8 | COLOR_FORMAT_LIMIT_2020):
	case (COLOR_FORMAT_FULL_601 << 8 | COLOR_FORMAT_FULL_2020):
	case (COLOR_FORMAT_FULL_601 << 8 | COLOR_FORMAT_LIMIT_2020):
	case (COLOR_FORMAT_LIMIT_601 << 8 | COLOR_FORMAT_FULL_2020):
	case (COLOR_FORMAT_LIMIT_601 << 8 | COLOR_FORMAT_LIMIT_2020):
		pConfig->matrix_coef = MATRIX_709_2020;
		break;
	case (COLOR_FORMAT_FULL_2020 << 8 | COLOR_FORMAT_FULL_709):
	case (COLOR_FORMAT_LIMIT_2020 << 8 | COLOR_FORMAT_FULL_709):
	case (COLOR_FORMAT_FULL_2020 << 8 | COLOR_FORMAT_LIMIT_709):
	case (COLOR_FORMAT_LIMIT_2020 << 8 | COLOR_FORMAT_LIMIT_709):
	case (COLOR_FORMAT_FULL_2020 << 8 | COLOR_FORMAT_FULL_601):
	case (COLOR_FORMAT_LIMIT_2020 << 8 | COLOR_FORMAT_FULL_601):
	case (COLOR_FORMAT_FULL_2020 << 8 | COLOR_FORMAT_LIMIT_601):
	case (COLOR_FORMAT_LIMIT_2020 << 8 | COLOR_FORMAT_LIMIT_601):
		pConfig->matrix_coef = MATRIX_2020_709;
		break;
	default:
		pConfig->en_3x3matrix = DISABLE_3X3MATRIX;
		pConfig->update = false;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_config_rgb2yuv(const struct data_type_struct *pOutputType,
	struct rgb2yuv_config_struct *pConfig)
{
	pConfig->update = true;
	pConfig->en_rgb2yuv = ENABLE_RGB2YUV;
	switch (pOutputType->colorFormat) {
	case COLOR_FORMAT_LIMIT_601:
		pConfig->rgb2yuv_coef = FULL_RGB_LIMIT_BT601;
		break;
	case COLOR_FORMAT_LIMIT_709:
		pConfig->rgb2yuv_coef = FULL_RGB_LIMIT_BT709;
		break;
	case COLOR_FORMAT_LIMIT_2020:
		pConfig->rgb2yuv_coef = FULL_RGB_LIMIT_BT2020;
		break;
	case COLOR_FORMAT_FULL_601:
		pConfig->rgb2yuv_coef = FULL_RGB_FULL_BT601;
		break;
	case COLOR_FORMAT_FULL_709:
		pConfig->rgb2yuv_coef = FULL_RGB_FULL_BT709;
		break;
	case COLOR_FORMAT_FULL_2020:
		pConfig->rgb2yuv_coef = FULL_RGB_FULL_BT2020;
		break;
	case COLOR_FORMAT_RGB:
		pConfig->en_rgb2yuv = DISABLE_RGB2YUV;
		pConfig->update = false;
		break;
	default:
		HDR_ERR("invalid output format:0x%x\n", pOutputType->colorFormat);
		return HDR_STATUS_INVALID_OUTPUT_FORMAT;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_write_yuv2rgb_ext_register(int plane,
	const struct yuv2rgb_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc)
{
	struct write_cmd_struct data;

	data.plane = plane;
	if (pConfig == NULL || pCallBackFunc == NULL) {
		HDR_ERR("hdr_core_write_yuv2rgb_register: invalid NULL pointer\n");
		return HDR_STATUS_NULL_POINTER;
	}
	if ((pConfig->en_yuv2rgb == ENABLE_YUV2RGB) &&
		(pConfig->update) &&
		(pConfig->yuv2rgb_coef == YUV2RGB_COEF_FROM_EXT_REGISTER)) {
		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_rmx << 0) |
					(pConfig->yuv2rgb_ext_coef.c_cf_rmy << 16);
		data.offset = 0x40;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_rmz << 0) |
					(pConfig->yuv2rgb_ext_coef.c_cf_gmx << 16);
		data.offset = 0x08;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_gmy << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_gmz << 16);
		data.offset = 0x0C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_bmx << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_bmy << 16);
		data.offset = 0x10;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_bmz << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_rax << 16);
		data.offset = 0x14;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
#if 0
		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_ray << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_raz << 16);
		data.offset = 0x18;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_gax << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_gay << 16);
		data.offset = 0x1C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_gaz << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_bax << 16);
		data.offset = 0x20;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_bay << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_baz << 16);
		data.offset = 0x24;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
#else
		/* do not config  c_cf_ray c_cf_raz c_cf_gay c_cf_gaz c_cf_bay c_cf_baz */
		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_gax << 0);
		data.offset = 0x1C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_bax << 16);
		data.offset = 0x20;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
#endif
		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_ra << 0) |
				(pConfig->yuv2rgb_ext_coef.c_cf_ga << 16);
		data.offset = 0x28;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->yuv2rgb_ext_coef.c_cf_ba << 0);
		data.offset = 0x2C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		pCallBackFunc(WRITE_CMD_GET_MODULE_NAME, &data);
		if (strcmp(data.module_name, "BT2020") == 0) {
			data.value = (pConfig->yuv2rgb_ext_coef.c_cf_rmz_c02_sp << 0) |
					(pConfig->yuv2rgb_ext_coef.c_cf_bmy_c21_sp << 16);
			data.offset = 0x78;
			pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
		} else if (strcmp(data.module_name, "HDR2SDR") == 0) {
			data.value = (pConfig->yuv2rgb_ext_coef.c_cf_rmz_c02_sp << 0) |
					(pConfig->yuv2rgb_ext_coef.c_cf_bmy_c21_sp << 16);
			data.offset = 0x98;
			pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
		} else if (strcmp(data.module_name, "SDR2HDR") == 0) {
			/* do nothing, SDR2HDR has no constant luma */
		} else {
			HDR_ERR("invalid module name:%s\n", data.module_name);
		}
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_write_gamma4096_register(int plane,
	const struct gamma_max_4096_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc)
{
	struct write_cmd_struct data = {0};

	data.plane = plane;

	if (pConfig == NULL || pCallBackFunc == NULL) {
		HDR_ERR("hdr_core_write_gamma4096_register NULL pointer error\n");
		return HDR_STATUS_NULL_POINTER;
	}

	if (!pConfig->update)
		return HDR_STATUS_OK;

	data.value = (pConfig->gamma_r_sram_4096 << 0) |
			(pConfig->gamma_g_sram_4096 << 16);
	data.offset = 0x54;
	pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

	data.value = (pConfig->gamma_b_sram_4096 << 0);
	data.offset = 0x58;
	pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_write_matrix_ext_register(int plane,
	const struct matrix_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc)
{
	struct write_cmd_struct data = {0};

	if (!(pConfig->matrix_coef == MATRIX_COEF_FROM_EXT_REGISTER &&
		pConfig->update &&
		pConfig->en_3x3matrix == ENABLE_3X3MATRIX))
		return HDR_STATUS_OK;

	data.plane = plane;
	pCallBackFunc(WRITE_CMD_GET_MODULE_NAME, &data);
	if (strcmp(data.module_name, "BT2020") == 0) {
		data.value = (pConfig->matrix_ext_coef.matrix01 << 0) |
				(pConfig->matrix_ext_coef.matrix02 << 16);
		data.offset = 0x40;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->matrix_ext_coef.matrix11 << 0) |
				(pConfig->matrix_ext_coef.matrix12 << 16);
		data.offset = 0x44;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->matrix_ext_coef.matrix21 << 0) |
				(pConfig->matrix_ext_coef.matrix22 << 16);
		data.offset = 0x48;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->matrix_ext_coef.matrix00 << 0) |
				(pConfig->matrix_ext_coef.matrix10 << 16);
		data.offset = 0x4C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->matrix_ext_coef.matrix20 << 0);
		data.offset = 0x50;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
	} else if (strcmp(data.module_name, "HDR2SDR") == 0) {
		data.value = pConfig->matrix_ext_coef.matrix01 << 16;
		data.mask = 0xFFFF0000;
		data.offset = 0x40;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER_WITH_MASK, &data);

		data.value = pConfig->matrix_ext_coef.matrix02 << 0 |
				pConfig->matrix_ext_coef.matrix11 << 16;
		data.offset = 0x44;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = pConfig->matrix_ext_coef.matrix12 << 0 |
				pConfig->matrix_ext_coef.matrix21 << 16;
		data.offset = 0x48;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = pConfig->matrix_ext_coef.matrix22 << 0 |
				pConfig->matrix_ext_coef.matrix00 << 16;
		data.offset = 0x4C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = pConfig->matrix_ext_coef.matrix10 << 0 |
				pConfig->matrix_ext_coef.matrix20 << 16;
		data.offset = 0x50;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
	} else {
		HDR_ERR("invalid module name:%s\n", data.module_name);
		return HDR_STATUS_OK;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_core_write_rgb2yuv_ext_register(int plane,
	const struct rgb2yuv_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc)
{
	struct write_cmd_struct data = {0};

	data.plane = plane;

	if (pConfig == NULL || pCallBackFunc == NULL) {
		HDR_ERR("hdr_core_write_rgb2yuv_ext_register NULL pointer error\n");
		return HDR_STATUS_NULL_POINTER;
	}

	if ((pConfig->rgb2yuv_coef == RGB2YUV_COEF_FROM_EXT_REGISTER) &&
		(pConfig->update) &&
		(pConfig->en_rgb2yuv == ENABLE_RGB2YUV)) {
		data.value = (pConfig->rgb2yuv_ext_coef.c_cf_y_mul_r << 0) |
				(pConfig->rgb2yuv_ext_coef.c_cf_y_mul_g << 16);
		data.offset = 0x5C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->rgb2yuv_ext_coef.c_cf_y_mul_b << 0) |
				(pConfig->rgb2yuv_ext_coef.c_cf_cb_mul_r << 16);
		data.offset = 0x60;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->rgb2yuv_ext_coef.c_cf_cb_mul_g << 0) |
				(pConfig->rgb2yuv_ext_coef.c_cf_cb_mul_b << 16);
		data.offset = 0x64;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->rgb2yuv_ext_coef.c_cf_cr_mul_r << 0) |
				(pConfig->rgb2yuv_ext_coef.c_cf_cr_mul_g << 16);
		data.offset = 0x68;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);

		data.value = (pConfig->rgb2yuv_ext_coef.c_cf_cr_mul_b << 0);
		data.offset = 0x6C;
		pCallBackFunc(WRITE_CMD_WRITE_REGISTER, &data);
	}
	return HDR_STATUS_OK;
}

/*
** colorPrimaries: source color space: bt601/bt709/bt2020
** 0	Reserved
** 1	BT709
** 2	Unspecified
** 3	Reserved
** 4	BT470M
** 5	BT470BG, BT601-6 625
** 6	SMPTE 170M, BT601-6 525 (same with 7)
** 7	SMPTE 240M (same with 6)
** 8	FILM
** 9	BT2020-2
** 10	SMPTE ST428
** 11	SMPTE RP 431
** 12	SMPTE EG 432
** 13 ¨C 21	Reserved
** 22	EBU 3213
** 23-255	Reserved
*/

COLOR_FORMAT_ENUM _hdr_core_fill_input_format(const struct mtk_disp_buffer *pBuffer)
{
	WARN_ON(pBuffer == NULL);

	if (pBuffer->hdr_info.colorPrimaries == 9)
		return COLOR_FORMAT_LIMIT_2020;
	if (pBuffer->hdr_info.colorPrimaries == 1 ||
		pBuffer->hdr_info.colorPrimaries == 0)
		return COLOR_FORMAT_FULL_709;
	if (pBuffer->hdr_info.colorPrimaries == 2)
		return COLOR_FORMAT_LIMIT_709;
	if (pBuffer->hdr_info.colorPrimaries == 5 ||
		pBuffer->hdr_info.colorPrimaries == 6 ||
		pBuffer->hdr_info.colorPrimaries == 7)
		return COLOR_FORMAT_LIMIT_601;

	return COLOR_FORMAT_LIMIT_601;
#if 0

	/* always limit BT2020, we rare encounter full BT2020 color space */
	if (pBuffer->is_bt2020)
		return COLOR_FORMAT_LIMIT_2020;

	switch (pBuffer->video_type) {
	case VIDEO_YUV_BT601_FULL:
		return COLOR_FORMAT_FULL_601;
	case VIDEO_YUV_BT601:
		return COLOR_FORMAT_LIMIT_601;
	case VIDEO_YUV_BT709:
		return COLOR_FORMAT_LIMIT_709;
	case VIDEO_YUV_BT709_FULL:
		return COLOR_FORMAT_FULL_709;
	case VIDEO_YUV_BT2020:
		return COLOR_FORMAT_LIMIT_2020;
	case VIDEO_YUV_BT2020_FULL:
		return COLOR_FORMAT_FULL_2020;
	default:
		HDR_ERR("invalid video type:%d\n", pBuffer->video_type);
	}
	WARN_ON(1);
	return COLOR_FORMAT_LIMIT_601;
#endif
}


/* according mtk_disp_buffer calculate which HDR path:main/sub/osd */
enum HDR_PATH_ENUM _hdr_core_get_hdr_path(const struct mtk_disp_buffer *pBuffer)
{
	enum HDR_PATH_ENUM path = HDR_PATH_MAX;

	if (pBuffer->type == DISP_LAYER_SIDEBAND || pBuffer->type == DISP_LAYER_VDP) {
		if (pBuffer->layer_id == 0)
			path = HDR_PATH_MAIN;
		else if (pBuffer->layer_id == 1)
			path = HDR_PATH_SUB;
		else {
			HDR_ERR("invalid display layer_id = %d\n", pBuffer->layer_id);
			WARN_ON(1);
		}
	} else
		path = HDR_PATH_OSD;

	return path;
}

/* parse HDR buffer info from display buffer info
** hdr metadata
** matrixCoeffs: source is constant bt2020 / non constant bt2020.
** 0	GBR
** 1	BT709
** 2	Unspecified
** 3	Reserved
** 4	FCC
** 5	BT470BG, BT601-6 625  (same with 6)
** 6	SMPTE 170M,  BT601-6 525  (same with 5)
** 7	SMPTE 240M
** 8	YCgCo
** 9	BT2020 non constant
** 10	BT2020 constant
** 11	YDzDx
** 12	N/A
** 13	N/A
** 14	ICtCp
** 15-255	Reserved

** transformCharacter: source gamma info ST2084 / HLG
** 0	Reserved
** 1	ITUR BT709 (same with 6,14, 15)
** 2	Unspecified
** 3	Reserved
** 4	Gamma 2.2
** 5	Gamma 2.8
** 6	SMPTE 170M (same with 1, 14, 15)
** 7	SMPTE 240M
** 8	Linear
** 9	Log
** 10	Sqrt
** 11	IEC 61966-2-4
** 12	ITUR BT1361
** 13	IEC 61966-2-1
** 14	BT2020-2 (same with 1, 6, 15)
** 15	BT2020-2 (same with 1, 6, 14)
** 16	SMPTE ST2084, PQ
** 17	SMPTE ST428-1
** 18	ARIB STDB67, HLG
** 19-255	Reserved

** colorPrimaries: source color space: bt601/bt709/bt2020
** 0	Reserved
** 1	BT709
** 2	Unspecified
** 3	Reserved
** 4	BT470M
** 5	BT470BG, BT601-6 625
** 6	SMPTE 170M, BT601-6 525 (same with 7)
** 7	SMPTE 240M (same with 6)
** 8	FILM
** 9	BT2020-2
** 10	SMPTE ST428
** 11	SMPTE RP 431
** 12	SMPTE EG 432
** 13 ¨C 21	Reserved
** 22	EBU 3213
** 23-255	Reserved
*/

bool _hdr_core_parse_disp_buffer_info(const struct mtk_disp_buffer *pConfig, struct HDR_BUFFER_INFO *pBuffer,
	struct disp_hw_common_info *pInfo)
{
	pBuffer->is_hdr = pConfig->is_hdr;
	pBuffer->is_dolby = pConfig->is_dolby;
	pBuffer->is_hdr10plus = pConfig->is_hdr10plus;
	pBuffer->color_format = _hdr_core_fill_input_format(pConfig);
	pBuffer->constant_bt2020 = (pConfig->hdr_info.matrixCoeffs == 10);
	if (pConfig->hdr_info.transformCharacter == 16)
		pBuffer->gamma_type = GAMMA_TYPE_ST2084;
	else if (pConfig->hdr_info.transformCharacter == 18)
		pBuffer->gamma_type = GAMMA_TYPE_HLG;
	else
		pBuffer->gamma_type = GAMMA_TYPE_2p4;
	memcpy(&pBuffer->resolution, pInfo->resolution, sizeof(pBuffer->resolution));
	return true;
}


/* cli setting change hdr config */
enum HDR_STATUS _hdr_core_handle_cli_setting(struct config_info_struct *pConfig)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	/* bypass module in cli */
	if (hdr_cli_get_info()->bypassModule & 0x001) {
		pConfig->BT2020Config.inputType.bypassModule = true;
		HDR_LOG("cli bypass BT2020\n");
	}

	if (hdr_cli_get_info()->bypassModule & 0x010) {
		pConfig->SDR2HDRConfig.inputType.bypassModule = true;
		HDR_LOG("cli bypass SDR2HDR\n");
	}

	if (hdr_cli_get_info()->bypassModule & 0x100) {
		pConfig->HDR2SDRConfig.inputType.bypassModule = true;
		HDR_LOG("cli bypass HDR2SDR\n");
	}

	if (hdr_cli_get_info()->gainValue != 0xFFFFFFFF)
		pConfig->SDR2HDRConfig.inputType.sdr2hdrGain = hdr_cli_get_info()->gainValue;
#endif
	return HDR_STATUS_OK;
}



#if 0
enum HDR_STATUS _hdr_core_fill_BT2020_info(struct config_info_struct *pConfig,
	const struct mtk_disp_buffer *pBuffer, const struct disp_hw_common_info *pTVInfo)
{
	/* we use HDR2SDR 3x3matrix do color space convert, if we have used HDR2SDR, don't need to use BT2020 */

	/* harry:TODO we need to check TV support capability */
	pConfig->BT2020Config.inputType.bypassModule = pBuffer->is_hdr ? true : false;
	if (pConfig->BT2020Config.inputType.bypassModule == true)
		return HDR_STATUS_OK;

	/* fill BT2020 input dataType */
	pConfig->BT2020Config.inputType.colorFormat = _hdr_core_fill_input_format(pBuffer);
	/* harry TODO: get from vdec */
	pConfig->BT2020Config.inputType.constant_luma_bt2020_input = false;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */
	pConfig->BT2020Config.inputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->BT2020Config.inputType.hdrType = HDR_TYPE_SDR;

	/* fill BT2020 output dataType */
	/* harry:TODO need to change according to TV capability */
	pConfig->BT2020Config.outputType.colorFormat = COLOR_FORMAT_LIMIT_709;
	/* only use in inputType */
	pConfig->BT2020Config.outputType.constant_luma_bt2020_input = false;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */;
	pConfig->BT2020Config.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->BT2020Config.outputType.hdrType = HDR_TYPE_SDR;

	return HDR_STATUS_OK;
}

enum HDR_STATUS _hdr_core_fill_SDR2HDR_info(struct config_info_struct *pConfig,
	const struct mtk_disp_buffer *pBuffer, const struct disp_hw_common_info *pTVInfo)
{
	/* buffer is sdr source & TV support HDR use sdr2hdr */
	pConfig->SDR2HDRConfig.inputType.bypassModule = (!pBuffer->is_hdr && pTVInfo->tv.is_support_hdr);
	if (pConfig->SDR2HDRConfig.inputType.bypassModule)
		return HDR_STATUS_OK;

	/* fill SDR2HDR input info */
	pConfig->SDR2HDRConfig.inputType.colorFormat = _hdr_core_fill_input_format(pBuffer);
	/* matrixCoeffs == 9: non constant BT2020. matrixCoeffs == 10: constant BT2020 */
	/*harry:TODO need to query from HWC */
	pConfig->SDR2HDRConfig.inputType.constant_luma_bt2020_input = false;
	/* SDR2HDR don't have degamma & gamma, don't need to set gamma type. */
	pConfig->SDR2HDRConfig.inputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->SDR2HDRConfig.inputType.hdrType = HDR_TYPE_SDR;
	/* TV gain value OK to change */
	pConfig->SDR2HDRConfig.inputType.sdr2hdrGain = 50;

	/* fill SDR2HDR output info */
	/* harry:TODO need to change according to TV capability */
	pConfig->SDR2HDRConfig.outputType.colorFormat = COLOR_FORMAT_LIMIT_709;
	/* only use in inputType */
	pConfig->SDR2HDRConfig.outputType.constant_luma_bt2020_input = false;
	/* SDR2HDR don't have degamma & gamma, don't need to set gamma type. */
	pConfig->SDR2HDRConfig.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->SDR2HDRConfig.outputType.hdrType = HDR_TYPE_HDR10;
	/* only use in sdr2hdr input type. */
	pConfig->SDR2HDRConfig.outputType.sdr2hdrGain = 0;

	return HDR_STATUS_OK;
}

enum HDR_STATUS _hdr_core_fill_HDR2SDR_info(struct config_info_struct *pConfig,
	const struct mtk_disp_buffer *pBuffer, const struct disp_hw_common_info *pTVInfo)
{
	/* if buffer source is HDR source but TV only support SDR, use HDR2SDR*/
	pConfig->HDR2SDRConfig.inputType.bypassModule = (pBuffer->is_hdr && !pTVInfo->tv.is_support_hdr) ? false : true;
	if (pConfig->HDR2SDRConfig.inputType.bypassModule)
		return HDR_STATUS_OK;

	pConfig->HDR2SDRConfig.inputType.colorFormat = _hdr_core_fill_input_format(pBuffer);
	/* matrixCoeffs == 9: non constant BT2020. matrixCoeffs == 10: constant BT2020 */
	/*harry:TODO need to query from HWC */
	pConfig->HDR2SDRConfig.inputType.constant_luma_bt2020_input = false;
	/*harry:TODO need to query from HWC */
	pConfig->HDR2SDRConfig.inputType.gammaType = GAMMA_TYPE_ST2084;
	pConfig->HDR2SDRConfig.inputType.hdrType = HDR_TYPE_HDR10;

	/* fill HDR2SDR output dataType */
	/* harry:TODO need to change according to TV capability */
	pConfig->HDR2SDRConfig.outputType.colorFormat = COLOR_FORMAT_LIMIT_709;
	/* only use in inputType */
	pConfig->HDR2SDRConfig.outputType.constant_luma_bt2020_input = false;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */
	pConfig->HDR2SDRConfig.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->HDR2SDRConfig.outputType.hdrType = HDR_TYPE_SDR;
	/* only use for SDR2HDR */
	pConfig->HDR2SDRConfig.outputType.sdr2hdrGain = 0;

	return HDR_STATUS_OK;
}
#endif


#if 1
enum COLOR_FORMAT_ENUM _hdr_core_get_tv_max_support_color_sapce(const struct disp_hw_common_info *pTVInfo)
{
	enum COLOR_FORMAT_ENUM color_format;

	if (pTVInfo->tv.is_support_bt2020)
		color_format = COLOR_FORMAT_LIMIT_2020;
	else if (pTVInfo->tv.is_support_709)
		color_format = COLOR_FORMAT_LIMIT_709;
	else
		color_format = COLOR_FORMAT_LIMIT_601;
	return color_format;
}

enum HDR_STATUS _hdr_core_fill_BT2020_info(struct config_info_struct *pConfig,
	struct HDR_BUFFER_INFO *pBuffer, const struct disp_hw_common_info *pTVInfo, enum HDR_PATH_ENUM path)
{
	/*
	** we use HDR2SDR 3x3matrix do color space convert, if we have used HDR2SDR, don't need to use BT2020
	**
	*/
	if (((pBuffer->color_format == COLOR_FORMAT_FULL_2020) ||
		(pBuffer->color_format == COLOR_FORMAT_LIMIT_2020)) &&
		(pTVInfo->tv.is_support_bt2020 == false))
		pConfig->BT2020Config.inputType.bypassModule = false;
	else if (((pBuffer->color_format == COLOR_FORMAT_FULL_709) ||
		(pBuffer->color_format == COLOR_FORMAT_LIMIT_709)) &&
		(pTVInfo->tv.is_support_709 == false))
		pConfig->BT2020Config.inputType.bypassModule = false;
	else if (((pBuffer->color_format == COLOR_FORMAT_FULL_601) ||
		(pBuffer->color_format == COLOR_FORMAT_LIMIT_601)) &&
		(pTVInfo->tv.is_support_601 == false))
		pConfig->BT2020Config.inputType.bypassModule = false;
	else
		pConfig->BT2020Config.inputType.bypassModule = true;

	if (path == HDR_PATH_OSD) {
		pConfig->BT2020Config.inputType.bypassModule = false;
	}

	/*if osd path was dolby/vs10 source, it needs bypass bt2020 even if source changed  or tv capbility changed*/
	if (path == HDR_PATH_OSD) {
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((pTVInfo->tv.force_hdr > 1) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && pTVInfo->tv.is_support_hdr10_plus)) &&
			(gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)))
			pConfig->BT2020Config.inputType.bypassModule = true;
	}

	/*if main/sub path was dolby/vs10 source, it needs bypass bt2020 even if source changed  or tv capbility changed*/
	if (path == HDR_PATH_MAIN || path == HDR_PATH_SUB) {
		if (!(pBuffer->is_hdr) || (pTVInfo->tv.is_support_hdr || pTVInfo->tv.is_support_hlg) ||
			pBuffer->is_dolby ||
			((pTVInfo->tv.force_hdr > 1) && (pBuffer->gamma_type != GAMMA_TYPE_HLG)))
			pConfig->BT2020Config.inputType.bypassModule = true;
	}
	if (path == HDR_PATH_SUB) {
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus))) ||
			(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[path].is_support_hdr && !gDispBufferInfo[path].is_hdr) ||
			(!(gTVInfo[path].force_hdr > 1) && gDispBufferInfo[path].is_hdr && !gTVInfo[path].is_support_hdr))
			pConfig->BT2020Config.inputType.bypassModule = false;
		else {
			HDR_DBG("_hdr_core_fill_BT2020_info, sub path[%d],bypass hdr2sdr & bt2020\n",path);
			pConfig->BT2020Config.inputType.bypassModule = true;
		}
		#if 0
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((pTVInfo->tv.force_hdr > 1) &&
			(gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)))
			pConfig->BT2020Config.inputType.bypassModule = false;
		if ((pTVInfo->tv.force_hdr > 1) && gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && pTVInfo->tv.is_support_hdr10_plus)
			pConfig->BT2020Config.inputType.bypassModule = true;
		#endif
	}
	if (pConfig->BT2020Config.inputType.bypassModule == true)
		return HDR_STATUS_OK;

	pConfig->BT2020Config.inputType.colorFormat = pBuffer->color_format;
	pConfig->BT2020Config.inputType.constant_luma_bt2020_input = pBuffer->constant_bt2020;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */
	pConfig->BT2020Config.inputType.gammaType = pBuffer->gamma_type;
	pConfig->BT2020Config.inputType.hdrType = HDR_TYPE_SDR;

	/* fill BT2020 output dataType */
	/* change according to TV capability */
	pConfig->BT2020Config.outputType.colorFormat = _hdr_core_get_tv_max_support_color_sapce(pTVInfo);

	/* only use in inputType */
	pConfig->BT2020Config.outputType.constant_luma_bt2020_input = false;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */;
	pConfig->BT2020Config.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->BT2020Config.outputType.hdrType = HDR_TYPE_SDR;

	/* fill HDR_BUFFER_INFO for next hardware use. */
	pBuffer->color_format = pConfig->BT2020Config.outputType.colorFormat;
	pBuffer->constant_bt2020 = pConfig->BT2020Config.outputType.constant_luma_bt2020_input;
	pBuffer->gamma_type = pConfig->BT2020Config.outputType.gammaType;
	pBuffer->is_hdr = false;

	/* dump BT2020 input & output for debug. */
	HDR_DBG("dump BT2020 info begin\n");
	HDR_DBG("input: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d]\n",
		pConfig->BT2020Config.inputType.colorFormat,
		pConfig->BT2020Config.inputType.constant_luma_bt2020_input,
		pConfig->BT2020Config.inputType.gammaType,
		pConfig->BT2020Config.inputType.hdrType);
	HDR_DBG("output: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d]\n",
		pConfig->BT2020Config.outputType.colorFormat,
		pConfig->BT2020Config.outputType.constant_luma_bt2020_input,
		pConfig->BT2020Config.outputType.gammaType,
		pConfig->BT2020Config.outputType.hdrType);
	HDR_DBG("dump BT2020 info end\n");
	return HDR_STATUS_OK;
}

enum HDR_STATUS _hdr_core_fill_SDR2HDR_info(struct config_info_struct *pConfig,
	struct HDR_BUFFER_INFO *pBuffer, const struct disp_hw_common_info *pTVInfo)
{
	/* buffer is sdr source & TV support HDR use sdr2hdr */
	if (pBuffer->is_hdr == false && pTVInfo->tv.is_support_hdr == true
		&& gDispBufferInfo[HDR_PATH_MAIN].is_hdr)
		pConfig->SDR2HDRConfig.inputType.bypassModule = false;
	else
		pConfig->SDR2HDRConfig.inputType.bypassModule = false;

	/*if main path was dolby/vs10 source, it needs bypass sdr2hdr even if source changed  or tv capbility changed*/
	if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
		((pTVInfo->tv.force_hdr > 1) && (!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && pTVInfo->tv.is_support_hdr10_plus))
		&& (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)))
		pConfig->SDR2HDRConfig.inputType.bypassModule = true;

	if (pConfig->SDR2HDRConfig.inputType.bypassModule)
		return HDR_STATUS_OK;

	/* fill SDR2HDR input info */
	pConfig->SDR2HDRConfig.inputType.colorFormat = pBuffer->color_format;
	/* constant bt2020 always false, cause we use bt2020 handle */
	WARN_ON(pBuffer->constant_bt2020 != false); /* SDR2HDR input should not be constant bt2020 source. */
	pConfig->SDR2HDRConfig.inputType.constant_luma_bt2020_input = pBuffer->constant_bt2020;
	/* SDR2HDR don't have degamma & gamma, don't need to set gamma type. */
	WARN_ON(pBuffer->gamma_type != GAMMA_TYPE_2p4);
	pConfig->SDR2HDRConfig.inputType.gammaType = pBuffer->gamma_type;
	pConfig->SDR2HDRConfig.inputType.hdrType = HDR_TYPE_SDR;
	/* TV gain value OK to change */
	pConfig->SDR2HDRConfig.inputType.sdr2hdrGain = 13;

	/* fill SDR2HDR output info */
	/* SDR2HDR don't do color space convert, so input & output color space are same */
	pConfig->SDR2HDRConfig.outputType.colorFormat = pBuffer->color_format;
	/* only use in inputType */
	pConfig->SDR2HDRConfig.outputType.constant_luma_bt2020_input = false;
	/* SDR2HDR don't have degamma & gamma, don't need to set gamma type. */
	pConfig->SDR2HDRConfig.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->SDR2HDRConfig.outputType.hdrType = HDR_TYPE_HDR10;
	/* only use in sdr2hdr input type. */
	pConfig->SDR2HDRConfig.outputType.sdr2hdrGain = 0;

	/* fill HDR_BUFFER_INFO for next hardware use. nouse for SDR2HDR. */
	pBuffer->color_format = pConfig->SDR2HDRConfig.outputType.colorFormat;
	pBuffer->constant_bt2020 = pConfig->SDR2HDRConfig.outputType.constant_luma_bt2020_input;
	pBuffer->gamma_type = pConfig->SDR2HDRConfig.outputType.gammaType;
	pBuffer->is_hdr = true;

	/* dump SDR2HDR input & output info for debug. */
	HDR_DBG("dump SDR2HDR info begin\n");
	HDR_DBG("input: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d] gain[%d]\n",
		pConfig->SDR2HDRConfig.inputType.colorFormat,
		pConfig->SDR2HDRConfig.inputType.constant_luma_bt2020_input,
		pConfig->SDR2HDRConfig.inputType.gammaType,
		pConfig->SDR2HDRConfig.inputType.hdrType,
		pConfig->SDR2HDRConfig.inputType.sdr2hdrGain);
	HDR_DBG("output: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d]\n",
		pConfig->SDR2HDRConfig.outputType.colorFormat,
		pConfig->SDR2HDRConfig.outputType.constant_luma_bt2020_input,
		pConfig->SDR2HDRConfig.outputType.gammaType,
		pConfig->SDR2HDRConfig.outputType.hdrType);
	HDR_DBG("dump SDR2HDR info end\n");
	return HDR_STATUS_OK;
}

enum HDR_STATUS _hdr_core_fill_HDR2SDR_info(struct config_info_struct *pConfig,
	struct HDR_BUFFER_INFO *pBuffer, const struct disp_hw_common_info *pTVInfo, enum HDR_PATH_ENUM path)
{
	/* if buffer source is HDR source but TV only support SDR, use HDR2SDR*/
	pConfig->HDR2SDRConfig.inputType.bypassModule = (pBuffer->is_hdr && !pTVInfo->tv.is_support_hdr) ? false : true;
	/*for dolby/vs10 source, it needs bypass hdr2sdr even if source changed  or tv capbility changed*/
	if (pBuffer->is_dolby || ((pTVInfo->tv.force_hdr > 1) && (pBuffer->gamma_type != GAMMA_TYPE_HLG)))
		pConfig->HDR2SDRConfig.inputType.bypassModule = true;

#if 1
	if (path == HDR_PATH_SUB) {
		pConfig->HDR2SDRConfig.inputType.bypassModule = (pBuffer->is_hdr && !pTVInfo->tv.is_support_hdr) ? false : true;
		#if 0
		if ((gDispBufferInfo[HDR_PATH_MAIN].is_dolby
		|| ((pTVInfo->tv.force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)))) {
			pConfig->HDR2SDRConfig.inputType.bypassModule =
				(pBuffer->is_hdr && !pTVInfo->tv.is_support_hdr) ? false : true;
		}
		if ((pTVInfo->tv.force_hdr > 1) && gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && pTVInfo->tv.is_support_hdr10_plus)
			pConfig->HDR2SDRConfig.inputType.bypassModule = true;
		#endif
	}
#endif

	if (pConfig->HDR2SDRConfig.inputType.bypassModule)
		return HDR_STATUS_OK;

	pConfig->HDR2SDRConfig.inputType.colorFormat = pBuffer->color_format;
	/* fill constant bt2020 info */
	pConfig->HDR2SDRConfig.inputType.constant_luma_bt2020_input = pBuffer->constant_bt2020;
	/*harry:TODO need to query from HWC */
	pConfig->HDR2SDRConfig.inputType.gammaType = pBuffer->gamma_type;
	pConfig->HDR2SDRConfig.inputType.hdrType = HDR_TYPE_HDR10;

	/* fill HDR2SDR output dataType */
	/* harry:TODO need to change according to TV capability */
	pConfig->HDR2SDRConfig.outputType.colorFormat = _hdr_core_get_tv_max_support_color_sapce(pTVInfo);
	/* only use in inputType */
	pConfig->HDR2SDRConfig.outputType.constant_luma_bt2020_input = false;
	/* force SDR use gamma2.4 cause HWC doesn't pass this info */
	pConfig->HDR2SDRConfig.outputType.gammaType = GAMMA_TYPE_2p4;
	pConfig->HDR2SDRConfig.outputType.hdrType = HDR_TYPE_SDR;
	/* only use for SDR2HDR */
	pConfig->HDR2SDRConfig.outputType.sdr2hdrGain = 0;

	/*modify HDR_BUFFER_INFO for next hardware use. */
	pBuffer->color_format = pConfig->HDR2SDRConfig.outputType.colorFormat;
	pBuffer->constant_bt2020 = pConfig->HDR2SDRConfig.outputType.constant_luma_bt2020_input;
	pBuffer->gamma_type = pConfig->HDR2SDRConfig.outputType.gammaType;
	pBuffer->is_hdr = false;

	/*dump input & output info for debug. */
	HDR_DBG("dump HDR2SDR setting begin\n");
	HDR_DBG("input: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d]\n",
		pConfig->HDR2SDRConfig.inputType.colorFormat,
		pConfig->HDR2SDRConfig.inputType.constant_luma_bt2020_input,
		pConfig->HDR2SDRConfig.inputType.gammaType,
		pConfig->HDR2SDRConfig.inputType.hdrType);
	HDR_DBG("output: format[%d] constant_bt2020[%d] gammaType[%d] hdrType[%d]\n",
		pConfig->HDR2SDRConfig.outputType.colorFormat,
		pConfig->HDR2SDRConfig.outputType.constant_luma_bt2020_input,
		pConfig->HDR2SDRConfig.outputType.gammaType,
		pConfig->HDR2SDRConfig.outputType.hdrType);
	HDR_DBG("dump HDR2SDR setting end\n");

	return HDR_STATUS_OK;
}
#endif


/* harry TODO: fill config info struct with pBuffer & pTVInfo.
** HDR2SDR/BT2020/SDR2HDR HW selection.
*/
enum HDR_STATUS _hdr_core_fill_config_info(struct config_info_struct *pConfig,
	const struct mtk_disp_buffer *pBuffer, const struct disp_hw_common_info *pTVInfo)
{
	struct HDR_BUFFER_INFO hdr_buffer_info;
	/* fill HDR_PATH */
	pConfig->path = _hdr_core_get_hdr_path(pBuffer);
	/* fill disp hardware resolution info */
	memcpy(&pConfig->resolution, pTVInfo->resolution, sizeof(pConfig->resolution));
	HDR_LOG("_hdr_core_fill_config_info,res_mode: %d\n", pConfig->resolution.res_mode);

	memcpy(&hdr_buffer_info, &gDispBufferInfo[pConfig->path], sizeof(hdr_buffer_info));

	/* fill buffer info & HDMI info */
	if (pConfig->path == HDR_PATH_MAIN || pConfig->path == HDR_PATH_SUB) {
		/* handle HDR2SDR -> BT2020 path config */

		/* fill HDR2SDR input dataType */
		_hdr_core_fill_HDR2SDR_info(pConfig, &hdr_buffer_info, pTVInfo, pConfig->path);

		/* fill BT2020 input & output dataType */
		_hdr_core_fill_BT2020_info(pConfig, &hdr_buffer_info, pTVInfo, pConfig->path);

	} else {
		/* handle BT2020 -> SDR2HDR path config */

		/* fill BT2020 input & output dataType */
		_hdr_core_fill_BT2020_info(pConfig, &hdr_buffer_info, pTVInfo, pConfig->path);

		/* fill SDR2HDR input & output dataType */
		_hdr_core_fill_SDR2HDR_info(pConfig, &hdr_buffer_info, pTVInfo);
	}

	/* cli setting can affect current hdr config */
	_hdr_core_handle_cli_setting(pConfig);
	return HDR_STATUS_OK;
}

/* init HDR module */
static int _hdr_core_init(struct disp_hw_common_info *info)
{
	enum HDR_STATUS status = HDR_STATUS_OK;
	int i = 0;
	enum HDR_PATH_ENUM path;

	HDR_LOG("enter HDR init\n");
	status = hdr_device_init_device_info();
	if (status != HDR_STATUS_OK) {
		HDR_ERR("get device tree info fail:%d\n", status);
		return status;
	}

	gHdrThread[HDR_PATH_MAIN] = create_singlethread_workqueue("hdr_path_main");
	gHdrThread[HDR_PATH_SUB] = create_singlethread_workqueue("hdr_path_sub");
	gHdrThread[HDR_PATH_OSD] = create_singlethread_workqueue("hdr_path_osd");
	gHandleHdrClockPathThred = create_singlethread_workqueue("hdr_clock_path");

	for (i = 0; i < HDR_PATH_MAX; i++)
		INIT_LIST_HEAD(&gConfigListHead[i]);
	gConfigListHeadInit = true;

	osd_enable_sdr2hdr = false;
	gOsdHadConfigure = false;

	gOsdHadConfigureForSubPath = false;
	gSubPathHavestopped = false;
	gOsbPathHaveReconfiguedForSubVideo = false;

	gOsdPathSuspend = false;
	gVideoIsPlaying = false;
	gVideoIsPlayingForSubPathDolbypip = false;
	gVideoIsPlayingForSubPathDolbypipHadUsed = false;
	gSubVideoCurrentPlayingState = false;
	mutex_init(&sync_lock_for_sub_path);
	mutex_init(&sync_lock_for_disp_configure_and_stop);
	for (path = HDR_PATH_MAIN; path < HDR_PATH_MAX; path++)
		gFirstConfigure[path] = true;

	bt2020_init();
	sdr2hdr_init();
	hdr2sdr_init();
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	hdr_cli_init();
#endif
#ifdef HDR_SECURE_SUPPORT
	hdr_sec_init(); /* only call once */
#endif

#if CONFIG_DRV_SDR2HDR_USED_FAST_LOGO
	/*if bring up stage had used sdr2hdr's rgb2yuv,kernel init stage need open sdr2hdr clock*/
	if (!clockFlag[HDR_CLOCK_MODULE_OSD]) {
		disp_clock_enable(DISP_CLK_SDR2HDR, true);
		clockFlag[HDR_CLOCK_MODULE_OSD] = true;
		/* hdr_core_adjust_osd_delay(info->resolution->res_mode, HDR_PATH_OSD); */
	}
#if 0
	/* clock control */
	if (!clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR]) {
		disp_clock_enable(DISP_CLK_SVDO_HDR2SDR, true);
		clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR] = true;
	}
	if (!clockFlag[HDR_CLOCK_MODULE_SUB_BT2020]) {
		disp_clock_enable(DISP_CLK_SVDO_BT2020, true);
		clockFlag[HDR_CLOCK_MODULE_SUB_BT2020] = true;
	}

	/* path control */
	disp_sys_hal_set_disp_out(DISP_SUB, DISP_OUT_SEL_BT2020);
#endif
#endif

	return 0;
}

void hdr_core_enable_dynamic_clock_on(HDMI_VIDEO_RESOLUTION res_mode, enum HDR_PATH_ENUM path)
{
	int i;
	uint32_t value = 0;
	const struct disp_hdr_dynamic_clock_on_off *res = NULL;
	uint32_t size = ARRAY_SIZE(hdr_dynamic_table);

	for (i = 0; i < size; i++) {
		res = &hdr_dynamic_table[i];
		if (res_mode == res->res_mode)
			break;
	}

	HDR_LOG("_get_resolution: %d\n", res->res_mode);

	if (path == HDR_PATH_MAIN) {
		value = (res->clk_dyn_off_start_main << 0) |
				(res->clk_dyn_off_end_main << 8) |
				(res->clk_dyn_ctrl_main << 15);
	} else if (path == HDR_PATH_SUB) {
		value = (res->clk_dyn_off_start_sub << 16) |
				(res->clk_dyn_off_end_sub << 24) |
				(res->clk_dyn_ctrl_sub << 31);
	}

	disp_sys_hal_hdr2sdr_dynamic_clock_ctrl(value, true);

}

void hdr_core_adjust_osd_delay(HDMI_VIDEO_RESOLUTION res_mode, enum HDR_PATH_ENUM path)
{
	int i;
	uint32_t value = 0;
	const struct disp_sdr2hdr_osd_path_delay *res = NULL;
	uint32_t size = ARRAY_SIZE(sdr2hdr_osd_delay_table);

	for (i = 0; i < size; i++) {
		res = &sdr2hdr_osd_delay_table[i];
		if (res_mode == res->res_mode)
			break;
	}

	HDR_LOG("resolution: %d, osd_delay : %d\n", res->res_mode, res->osd_delay);

	if (path == HDR_PATH_OSD)
		value = res->osd_delay;

	/*set delay due to thruoghing hdr2sdr and bt2020 sub module*/
	disp_path_set_delay_by_shift(DISP_PATH_OSD1, res_mode, true, value, false, 0);
	disp_path_set_delay_by_shift(DISP_PATH_OSD2, res_mode, true, value, false, 0);

}


/* judge if need to through hdr2sdr, bt2020 or sdr2hdr*/
bool hdr_core_need_through_hdr2sdr_bt2020_sdr2hdr(enum HDR_PATH_ENUM path,
	struct HDR_BUFFER_INFO bufferInfo, struct disp_hw_tv_capbility tv)
{
	/* for main/sub path, if source is non-hdr or tv is support hdr */
	/* or TV timing is hdr */
	/* or app force hdr source to use dolby flow  */
	/* did not need to through hdr2sdr and bt2020*/
	if (path == HDR_PATH_MAIN) {
		if (!(bufferInfo.is_hdr) || (tv.is_support_hdr || tv.is_support_hlg) ||
			bufferInfo.is_dolby ||
			((tv.force_hdr > 1) && (bufferInfo.gamma_type != GAMMA_TYPE_HLG))) {
			HDR_TONE_LOG("The source is non-hdr or tv is support hdr.\n");
			return false;
		}
	}
	if (path == HDR_PATH_SUB) {
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((tv.force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && tv.is_support_hdr10_plus))) ||
			(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && tv.is_support_hdr && !bufferInfo.is_hdr) ||
			(!(tv.force_hdr > 1) && bufferInfo.is_hdr && !tv.is_support_hdr))
			return true;
		else {
			HDR_TONE_LOG("sub path,did not need to hdr2sdr & bt2020\n");
			return false;
		}
		#if 0
		if (!(bufferInfo.is_hdr) || (tv.is_support_hdr || tv.is_support_hlg) ||
			bufferInfo.is_dolby ||
			((tv.force_hdr > 1) && (bufferInfo.gamma_type != GAMMA_TYPE_HLG))) {
			if (!(gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
				((tv.force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)))
				|| ((tv.force_hdr > 1) && gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && tv.is_support_hdr10_plus)) {
				HDR_TONE_LOG("sub path,the source is non-hdr or tv is support hdr.\n");
				return false;
			}
		}
		#endif
	}

	/* for osd path, if source is dolby source, bypass sdr2hdr& bt2020*/
	/*or need to through bt2020 and sdr2hdr*/
	if (path == HDR_PATH_OSD) {
		if (!osd_enable_sdr2hdr || gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((tv.force_hdr > 1) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && tv.is_support_hdr10_plus)) &&
			(gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG))) {
			HDR_TONE_LOG("The source is dolby\n");
			return false;
		}
		/*If it is playing video except dolby source, need to configure sdr2hdr path*/
		/*now osd direct link sdr2hdr whether it is playing video or not except dolby source*/
		/*for hdr video, sdr2hdr  needs to output limit 709*/
		/*for sdr video, sdr2hdr  needs to output limit 709*/
		/*If video have stopped, sdr2hdr  needs to output limit709*/
		if(!gVideoIsPlaying) {
			HDR_TONE_LOG("The video is not playing, did not need to configure sdr2hdr path\n");
			return false;
		}
	}

	return true;
}

bool hdr_core_had_through_hdr2sdr_bt2020_sdr2hdr(enum HDR_PATH_ENUM path)
{
	/* for main/sub path, if source is non-hdr or tv is support hdr */
	/* or TV timing is hdr */
	/* or app force hdr source to use dolby flow  */
	/* did not need to through hdr2sdr and bt2020*/
	if (path == HDR_PATH_MAIN) {
		if (!(gDispBufferInfo[path].is_hdr) || (gTVInfo[path].is_support_hdr || gTVInfo[path].is_support_hlg)
			|| gDispBufferInfo[path].is_dolby ||
			((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[path].gamma_type != GAMMA_TYPE_HLG))) {
			HDR_DBG("The source is non-hdr or tv is support hdr, had not used hdr2sdr\n");
			return false;
		}
	}

	if (path == HDR_PATH_SUB) {
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus))) ||
			(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[path].is_support_hdr && !gDispBufferInfo[path].is_hdr) ||
			(!(gTVInfo[path].force_hdr > 1) && gDispBufferInfo[path].is_hdr && !gTVInfo[path].is_support_hdr) ||
			gVideoIsPlayingForSubPathDolbypipHadUsed)
			return true;
		else {
			HDR_DBG("sub path,had not through hdr2sdr & bt2020\n");
			return false;
		}
		#if 0
		if (!(gDispBufferInfo[path].is_hdr) || (gTVInfo[path].is_support_hdr || gTVInfo[path].is_support_hlg)
			|| gDispBufferInfo[path].is_dolby ||
			((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[path].gamma_type != GAMMA_TYPE_HLG))) {
			if (!(((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG)) ||
				gDispBufferInfo[HDR_PATH_MAIN].is_dolby)
				|| ((gTVInfo[path].force_hdr > 1) && gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus)) {
				HDR_DBG("sub path, source is non-hdr or tv is support hdr, have not to use hdr2sdr\n");
				return false;
			}
		}
		#endif
	}

	/* for osd path, if source is dolby source, bypass sdr2hdr& bt2020*/
	/*or need to through bt2020 and sdr2hdr*/
	if (path == HDR_PATH_OSD) {
		if (!osd_enable_sdr2hdr || gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
			((gTVInfo[path].force_hdr > 1) &&
			(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus)) &&
			(gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG))) {
			HDR_DBG("The source is dolby\n");
			return false;
		}
		if(!gVideoIsPlaying) {
			HDR_DBG("The video is not playing, did not configured sdr2hdr path\n");
			return false;
		}
	}

	return true;
}

void hdr_core_handle_clock_path(struct work_struct *pWorkItem)
{
	struct config_info_struct *pConfig;

	pConfig = container_of(pWorkItem, struct config_info_struct, workItem);
	if (pConfig == NULL) {
		HDR_ERR("handle hdr_core_handle_clock_path NULL pointer error\n");
		return;
	}

	do {
		HDR_LOG("hdr_core_handle_clock_path:%d\n", pConfig->path);

		if (pConfig->path == HDR_PATH_MAIN) { /* main path */
			if (!pConfig->BT2020Config.disable_bt2020) { /* use bt2020: open HDR2SDR & BT2020 clock */
				/* clock control */
				if (!clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR]) {
					disp_clock_enable(DISP_CLK_MVDO_HDR2SDR, true);
					clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR] = true;
				}
				if (!clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020]) {
					disp_clock_enable(DISP_CLK_MVDO_BT2020,	true);
					clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020] = true;
				}

				/* path control */
				disp_sys_hal_set_disp_out(DISP_MAIN, DISP_OUT_SEL_BT2020);
				HDR_LOG("hdr_core_handle_clock_path, res_mode:%d\n", pConfig->resolution.res_mode);

				/*set delay due to thruoghing hdr2sdr and bt2020 sub module*/
				disp_path_set_delay_by_shift(DISP_PATH_MVDO, pConfig->resolution.res_mode,
					true, 57, false, 0);

				/* disp main path dynamic clock off between vsync_width and active_start interval */
				/*note that cannot clock off between 0 and*/
				/*vsync_width which the hardware is still working*/
				hdr_core_enable_dynamic_clock_on(pConfig->resolution.res_mode, pConfig->path);

			} else if (!pConfig->HDR2SDRConfig.disable_hdr2hdr) {
				/*only use hdr2sdr: open HDR2SDR clock & disable BT2020 clock */
				/* clock control */
				if (!clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR]) {
					disp_clock_enable(DISP_CLK_MVDO_HDR2SDR, true);
					clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR] = true;
				}

				if (clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020]) {
					disp_clock_enable(DISP_CLK_MVDO_BT2020, false);
					clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020] = false;
				}

				/* path control */
				disp_sys_hal_set_disp_out(DISP_MAIN, DISP_OUT_SEL_HDR2SDR);
				/*set delay due to thruoghing hdr2sdr sub module*/
				disp_path_set_delay_by_shift(DISP_PATH_MVDO, pConfig->resolution.res_mode,
					true, 42, false, 0);

				/* disp main path dynamic clock off between vsync_width and active_start interval */
				hdr_core_enable_dynamic_clock_on(pConfig->resolution.res_mode, pConfig->path);

			} else {
				/* use none: disable HDR2SDR & BT2020 clock */
				if (clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020] || clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR]) {
					/* path control */
					disp_sys_hal_set_disp_out(DISP_MAIN, DISP_OUT_SEL_VDO);
					disp_path_set_delay_by_shift(DISP_PATH_MVDO, pConfig->resolution.res_mode,
						true, 0, false, 0);
				}
				/* clock control */
				if (clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR]) {
					disp_clock_enable(DISP_CLK_MVDO_HDR2SDR, false);
					clockFlag[HDR_CLOCK_MODULE_MAIN_HDR2SDR] = false;
				}

				if (clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020]) {
					disp_clock_enable(DISP_CLK_MVDO_BT2020, false);
					clockFlag[HDR_CLOCK_MODULE_MAIN_BT2020] = false;
				}

			}
		}
		#if 1
		if (pConfig->path == HDR_PATH_SUB) { /* sub path */
			if (pConfig->BT2020Config.disable_bt2020 && pConfig->HDR2SDRConfig.disable_hdr2hdr) {
				mutex_lock(&sync_lock_for_sub_path);
				if (gSubVideoCurrentPlayingState == true) {
					HDR_LOG("sub path current playing state is start,need to use hdr2sdr&bt2020\n");
					mutex_unlock(&sync_lock_for_sub_path);
					break;
				}
				mutex_unlock(&sync_lock_for_sub_path);
				/* use none: disable HDR2SDR & BT2020 clock */
				/* clock control */
				HDR_LOG("sub path current playing state is stopped,need to bypass hdr2sdr&bt2020\n");
				if (clockFlag[HDR_CLOCK_MODULE_SUB_BT2020] || clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR]) {
					/* path control */
					disp_sys_hal_set_disp_out(DISP_SUB, DISP_OUT_SEL_VDO);
					disp_path_set_delay_by_shift(DISP_PATH_SVDO, pConfig->resolution.res_mode,
						true, 0, false, 0);

				}
				if (clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR]) {
					disp_clock_enable(DISP_CLK_SVDO_HDR2SDR, false);
					clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR] = false;
				}

				if (clockFlag[HDR_CLOCK_MODULE_SUB_BT2020]) {
					disp_clock_enable(DISP_CLK_SVDO_BT2020, false);
					clockFlag[HDR_CLOCK_MODULE_SUB_BT2020] = false;
				}

				/* path control */
				/*disp_sys_hal_set_disp_out(DISP_SUB, DISP_OUT_SEL_VDO);*/
				/*disp_path_set_delay_by_shift(DISP_PATH_MVDO, pConfig->resolution.res_mode,
					true, 0, false, 0);*/
			}
		}
		#endif
		if (pConfig->path == HDR_PATH_OSD) {/* osd path handle */
			/* use SDR2HDR & BT2020: open SDR2HDR & BT2020 clock */
			if (!pConfig->SDR2HDRConfig.disable_sdr2hdr) {
				/* clock control */
				if (!clockFlag[HDR_CLOCK_MODULE_OSD]) {
					disp_clock_enable(DISP_CLK_SDR2HDR, true);
					clockFlag[HDR_CLOCK_MODULE_OSD] = true;
					/* path control */
					vdout_sys_hal_sdr2hdr_bypass(false);

					/*set osd path delay due to thruoghing sdr2hdr&bt220  module*/
					hdr_core_adjust_osd_delay(pConfig->resolution.res_mode, pConfig->path);
					/*fmt_hal_shadow_update();*/
				}

				/* path control */
				/*vdout_sys_hal_sdr2hdr_bypass(false);*/

				/*set osd path delay due to thruoghing sdr2hdr&bt220  module*/
				/*hdr_core_adjust_osd_delay(pConfig->resolution.res_mode, pConfig->path);*/
				/*fmt_hal_shadow_update();*/

			} else if (gOsdPathSuspend){ /* use none: disable SDR2HDR & BT2020 clock */
				/* clock control */
				if (clockFlag[HDR_CLOCK_MODULE_OSD]) {
					disp_clock_enable(DISP_CLK_SDR2HDR, false);
					clockFlag[HDR_CLOCK_MODULE_OSD] = false;
				}

				/* path control */
				vdout_sys_hal_sdr2hdr_bypass(true);
				fmt_hal_shadow_update();
			}
		}

	} while (0);

	/*free pConfig memory because of configuring and stopping hdr2sdr module which once malloc memory*/
	vfree(pConfig);
	HDR_LOG("free pConfig success\n");
}

/*now osd direct link to sdr2hdr whether it is playing video or not*/
/*for hdr video, needing to use bt2020's 3*3 matrix, degamma, gamma sub module & sdr2hdr's sdr2hdr and rgb2yuv sub module*/
/*sdr2hdr needs to output limit 709 corlor format finally*/

/*for sdr video, only needs sdr2hdr's rgb2yuv, sdr2hdr outputs limit 709 finally*/
/*If video have stopped, only needs sdr2hdr's rgb2yuv, sdr2hdr outputs limit709 finally*/
enum HDR_STATUS sdr2hdr_boot_kenel_init_stage_special_prcess(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig)
{
	if (path != HDR_PATH_OSD)
		return HDR_STATUS_INVALID_PARAM;

	if (gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[path].is_support_hdr
		&& gVideoIsPlaying) {
		/*for hdr video, neededto use bt2020's 3*3 matrix, degamma, gamma, sub module*/
		/*& sdr2hdr's sdr2hdr and rgb2yuv sub module*/
		/*degamma module selects DEGAMMA_CURVE_SRGB & gamma module selects GAMMA_CURVE_2p4*/
		/*sdr2hdr needs to output llimit709 corlor format finally*/
		pConfig->BT2020Config.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->BT2020Config.degamma_config.degamma_curve = DEGAMMA_CURVE_SRGB;
		pConfig->BT2020Config.gamma_config.gamma_curve = GAMMA_CURVE_2p4;
		pConfig->BT2020Config.rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;

		pConfig->SDR2HDRConfig.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->SDR2HDRConfig.rgb2yuv_config.rgb2yuv_coef = FULL_RGB_LIMIT_BT709;
		fmt_hal_multiple_alpha(true);
	} else if ((!gDispBufferInfo[HDR_PATH_MAIN].is_hdr ||
		(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && !gTVInfo[path].is_support_hdr)) &&
		gVideoIsPlaying) {
		/*for sdr video, only needs sdr2hdr's rgb2yuv, sdr2hdr outputs limit 709 color format finally*/
		pConfig->BT2020Config.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->BT2020Config.degamma_config.en_degamma = DISABLE_DEGAMMA;
		pConfig->BT2020Config.matrix_config.en_3x3matrix = DISABLE_3X3MATRIX;
		pConfig->BT2020Config.gamma_config.en_gamma = DISABLE_GAMMA;
		pConfig->BT2020Config.rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;

		pConfig->SDR2HDRConfig.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->SDR2HDRConfig.en_main_func = DISABLE_MAIN_FUNC;
		pConfig->SDR2HDRConfig.rgb2yuv_config.en_rgb2yuv = ENABLE_RGB2YUV;
		pConfig->SDR2HDRConfig.rgb2yuv_config.rgb2yuv_coef = FULL_RGB_LIMIT_BT709;
		fmt_hal_multiple_alpha(true);
	} else if(!gVideoIsPlaying) {
		/*If video have stopped, only needs sdr2hdr's rgb2yuv, sdr2hdr outputs limit 709 finally*/
		pConfig->BT2020Config.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->BT2020Config.degamma_config.en_degamma = DISABLE_DEGAMMA;
		pConfig->BT2020Config.matrix_config.en_3x3matrix = DISABLE_3X3MATRIX;
		pConfig->BT2020Config.gamma_config.en_gamma = DISABLE_GAMMA;
		pConfig->BT2020Config.rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;

		pConfig->SDR2HDRConfig.yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
		pConfig->SDR2HDRConfig.en_main_func = DISABLE_MAIN_FUNC;
		pConfig->SDR2HDRConfig.rgb2yuv_config.en_rgb2yuv = ENABLE_RGB2YUV;
		pConfig->SDR2HDRConfig.rgb2yuv_config.rgb2yuv_coef = FULL_RGB_LIMIT_BT709;
		fmt_hal_multiple_alpha(true);
	}
	HDR_DBG("hdr special_process done [%d %d %d %d]\n", pConfig->SDR2HDRConfig.en_main_func,
		gDispBufferInfo[HDR_PATH_MAIN].is_hdr, gVideoIsPlaying, gTVInfo[path].is_support_hdr);

	return HDR_STATUS_OK;
}

enum HDR_STATUS sdr2hdr_bt2020_for_sub_path_video(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig)
{
	if (!gVideoIsPlayingForSubPathDolbypip || (path != HDR_PATH_OSD) || (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
		((gTVInfo[path].force_hdr > 1) && ((gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
		!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus))) ||
		!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[path].is_support_hdr && !gDispBufferInfo[HDR_PATH_SUB].is_hdr))) {
		HDR_DBG("sdr2hdr_bt2020_for_sub_path_video, bypass sdr2hdr&bt2020, osd path[%d]\n",path);
		return HDR_STATUS_INVALID_PARAM;
	} else {
		HDR_DBG("sdr2hdr_bt2020_for_sub_path_video, using sdr2hdr&bt2020, osd path[%d],\n",path);
	}

	if (gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[HDR_PATH_MAIN].is_support_hdr && !gDispBufferInfo[HDR_PATH_SUB].is_hdr) {
		/*for hdr video, needed to use bt2020's 3*3 matrix, degamma, gamma, sub module*/
		/*& sdr2hdr's sdr2hdr and rgb2yuv sub module*/
		/*degamma module selects DEGAMMA_CURVE_2p4 & gamma module selects GAMMA_CURVE_709*/
		/*sdr2hdr needs to output llimit2020 corlor format finally*/
		pConfig->BT2020Config.degamma_config.degamma_curve = DEGAMMA_CURVE_SRGB;
		pConfig->BT2020Config.gamma_config.gamma_curve = GAMMA_CURVE_709;


		pConfig->SDR2HDRConfig.rgb2yuv_config.rgb2yuv_coef = FULL_RGB_LIMIT_BT2020;
		gOsbPathHaveReconfiguedForSubVideo = true;
	}
	HDR_DBG("hdr sdr2hdr_bt2020_for_sub_path_videodone [%d %d %d]\n",
		gDispBufferInfo[HDR_PATH_MAIN].is_hdr,
		gTVInfo[path].is_support_hdr, gDispBufferInfo[HDR_PATH_SUB].is_hdr);

	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr2sdr_bt2020_for_dolby_pip(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig)
{
	if ((path == HDR_PATH_SUB) && (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
		((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
		(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus))) ||
		(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[path].is_support_hdr && !gDispBufferInfo[path].is_hdr)))
		HDR_DBG("hdr2sdr_bt2020_for_dolby_pip, using yuv2rgb of bt2020,sub path[%d]\n",path);
	else {
		HDR_DBG("_hdr_core_fill_BT2020_info, bypass yuv2rgn of bt2020, sub path[%d],\n",path);
		return HDR_STATUS_INVALID_PARAM;
	}

	if (gVideoIsPlayingForSubPathDolbypip) {
		pConfig->BT2020Config.yuv2rgb_config.en_yuv2rgb = ENABLE_YUV2RGB;
		if (gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
		((gTVInfo[path].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
		(!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[path].is_support_hdr10_plus)))) {
			pConfig->BT2020Config.yuv2rgb_config.yuv2rgb_coef = LIMIT_BT709_FULL_RGB;
		} else {
			pConfig->BT2020Config.yuv2rgb_config.yuv2rgb_coef = LIMIT_BT601_LIMIT_RGB;
		}
		pConfig->BT2020Config.degamma_config.en_degamma = DISABLE_DEGAMMA;
		pConfig->BT2020Config.matrix_config.en_3x3matrix = DISABLE_3X3MATRIX;
		pConfig->BT2020Config.gamma_config.en_gamma = DISABLE_GAMMA;
		pConfig->BT2020Config.rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;
		gVideoIsPlayingForSubPathDolbypipHadUsed = true;
	}
	HDR_DBG("hdr2sdr_bt2020_for_dolby_pip [%d %d %d]\n", gDispBufferInfo[HDR_PATH_MAIN].is_dolby,
		gTVInfo[path].force_hdr, gDispBufferInfo[HDR_PATH_MAIN].gamma_type);

	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr2sdr_bt2020_sub_path_clock_path_ctrl(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig)
{
	if (path != HDR_PATH_SUB) {
			HDR_DBG("non sub path");
			return HDR_STATUS_INVALID_PARAM;
	}
	HDR_LOG("hdr2sdr_bt2020_sub_path_clock_path_ctrl, res_mode:%d, path:%d\n",
		pConfig->resolution.res_mode, path);
	if (!pConfig->BT2020Config.disable_bt2020) { /* use bt2020: open HDR2SDR & BT2020 clock */
		/* clock control */
		if (!clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR]) {
			disp_clock_enable(DISP_CLK_SVDO_HDR2SDR, true);
			clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR] = true;
		}
		if (!clockFlag[HDR_CLOCK_MODULE_SUB_BT2020]) {
			disp_clock_enable(DISP_CLK_SVDO_BT2020, true);
			clockFlag[HDR_CLOCK_MODULE_SUB_BT2020] = true;

			/* path control */
			disp_sys_hal_set_disp_out(DISP_SUB, DISP_OUT_SEL_BT2020);

			/*set delay due to thruoghing hdr2sdr and bt2020 sub module*/
			if (!gVideoIsPlayingForSubPathDolbypipHadUsed) {
				disp_path_set_delay_by_shift(DISP_PATH_SVDO, pConfig->resolution.res_mode,
				true, 57, false, 0);
				gVideoIsPlayingForSubPathDolbypipHadUsed = false;
			}
		}

	} else if (!pConfig->HDR2SDRConfig.disable_hdr2hdr) {
		/*only use hdr2sdr: open HDR2SDR clock & disable BT2020 clock */
		/* clock control */
		if (!clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR]) {
			disp_clock_enable(DISP_CLK_SVDO_HDR2SDR, true);
			clockFlag[HDR_CLOCK_MODULE_SUB_HDR2SDR] = true;
			/* path control */
			disp_sys_hal_set_disp_out(DISP_SUB, DISP_OUT_SEL_HDR2SDR);

			/*set delay due to thruoghing hdr2sdr sub module*/
			disp_path_set_delay_by_shift(DISP_PATH_SVDO, pConfig->resolution.res_mode,
				true, 42, false, 0);
		}

		if (clockFlag[HDR_CLOCK_MODULE_SUB_BT2020]) {
			disp_clock_enable(DISP_CLK_SVDO_BT2020, false);
			clockFlag[HDR_CLOCK_MODULE_SUB_BT2020] = false;
		}
	}

	return HDR_STATUS_OK;
}

#if 0
void hdr_core_config_path(struct work_struct *pWorkItem)
{
	struct config_info_struct *pConfig;
	enum HDR_STATUS status = HDR_STATUS_OK;
	int plane = 0;

	pConfig = container_of(pWorkItem, struct config_info_struct, workItem);
	if (pConfig == NULL) {
		HDR_ERR("handle hdr_core_config NULL pointer error\n");
		return;
	}

	do {
		HDR_LOG("handle path %d\n", pConfig->path);
		/* config bt2020 */
		if (hdr_device_map_bt2020_plane_from_path(pConfig->path, &plane)) {
			status = bt2020_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/* config sdr2hdr */
		if (hdr_device_map_sdr2hdr_plane_from_path(pConfig->path, &plane)) {
			status = sdr2hdr_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/* config hdr2sdr */
		if (hdr_device_map_hdr2sdr_plane_from_path(pConfig->path, &plane)) {
			status = hdr2sdr_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/*boot stage and kernel init stage processed and sdr2hdr stage which special processing*/
		sdr2hdr_boot_kenel_init_stage_special_prcess(pConfig->path, pConfig);
		hdr2sdr_bt2020_for_dolby_pip(pConfig->path, pConfig);
		hdr2sdr_bt2020_sub_path_clock_path_ctrl(pConfig->path, pConfig);

		/* config path end */
		/* add task to list */
		if (pConfig->BT2020Config.need_update ||
			pConfig->HDR2SDRConfig.need_update ||
			pConfig->SDR2HDRConfig.need_update) {
			HDR_LOG("update HDR register path[%d] BT2020[%d] SDR2HDR[%d] HDR2SDR[%d]\n", pConfig->path,
				pConfig->BT2020Config.need_update,
				pConfig->SDR2HDRConfig.need_update,
				pConfig->HDR2SDRConfig.need_update);
			list_add_tail(&pConfig->listEntry, &gConfigListHead[HDR_PATH_MAIN]);
		}
	} while (0);

	if (status != HDR_STATUS_OK) {
		HDR_ERR("config path error: %d\n", status);
		return;
	}
}
#endif

void hdr_core_config_path(struct config_info_struct *pConfig)
{
	enum HDR_STATUS status = HDR_STATUS_OK;
	int plane = 0;

	if (pConfig == NULL) {
		HDR_ERR("handle hdr_core_config NULL pointer error\n");
		return;
	}

	do {
		HDR_LOG("handle path %d\n", pConfig->path);
		/* config bt2020 */
		if (hdr_device_map_bt2020_plane_from_path(pConfig->path, &plane)) {
			status = bt2020_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/* config sdr2hdr */
		if (hdr_device_map_sdr2hdr_plane_from_path(pConfig->path, &plane)) {
			status = sdr2hdr_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/* config hdr2sdr */
		if (hdr_device_map_hdr2sdr_plane_from_path(pConfig->path, &plane)) {
			status = hdr2sdr_config(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/*boot stage and kernel init stage processed and sdr2hdr stage which special processing*/
		sdr2hdr_boot_kenel_init_stage_special_prcess(pConfig->path, pConfig);
		sdr2hdr_bt2020_for_sub_path_video(pConfig->path, pConfig);
		hdr2sdr_bt2020_for_dolby_pip(pConfig->path, pConfig);
		hdr2sdr_bt2020_sub_path_clock_path_ctrl(pConfig->path, pConfig);

		/* config path end */
		/* add task to list */
		if (pConfig->BT2020Config.need_update ||
			pConfig->HDR2SDRConfig.need_update ||
			pConfig->SDR2HDRConfig.need_update) {
			HDR_LOG("update HDR register path[%d] BT2020[%d] SDR2HDR[%d] HDR2SDR[%d]\n", pConfig->path,
				pConfig->BT2020Config.need_update,
				pConfig->SDR2HDRConfig.need_update,
				pConfig->HDR2SDRConfig.need_update);
			list_add_tail(&pConfig->listEntry, &gConfigListHead[HDR_PATH_MAIN]);
		}
	} while (0);

	if (status != HDR_STATUS_OK) {
		HDR_ERR("config path error: %d\n", status);
		return;
	}
}


/* check if we need to config HDR .
** when hdr source or TV info changes, we need to reconfig HDR.
*/
bool _hdr_core_hdr_need_reconfig(struct mtk_disp_buffer *pConfig, struct disp_hw_common_info *pInfo)
{
	bool status = false;
	enum HDR_PATH_ENUM path;
	struct HDR_BUFFER_INFO bufferInfo = {0};

	do {
		if (pConfig == NULL || pInfo == NULL || pInfo->resolution == NULL ||
			(pConfig->ion_fd == 0xFFFFFFFF)) {
			status = false;
			break;
		}
		path = _hdr_core_get_hdr_path(pConfig);
		/* update HDR HW one time */
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
		if (hdr_cli_get_info()->updateHDR != 0)
			status = true;
#endif
		_hdr_core_parse_disp_buffer_info(pConfig, &bufferInfo, pInfo);

		if (gFirstConfigure[path]) {
			if(path == HDR_PATH_MAIN) {
				gVideoIsPlaying = true;
			}
			if(path == HDR_PATH_SUB) {
				gVideoIsPlayingForSubPathDolbypip = true;
				mutex_lock(&sync_lock_for_sub_path);
				gSubVideoCurrentPlayingState = true;
				mutex_unlock(&sync_lock_for_sub_path);
			}
			memcpy(&gDispBufferInfo[path], &bufferInfo, sizeof(gDispBufferInfo[0]));
			memcpy(&gTVInfo[path], &pInfo->tv, sizeof(gTVInfo[0]));
			gFirstConfigure[path] = false;
			if (hdr_core_need_through_hdr2sdr_bt2020_sdr2hdr(path, bufferInfo, pInfo->tv)) {
				status = true;
				if ((path == HDR_PATH_OSD) && !gOsdHadConfigure)
					gOsdHadConfigure = true;
				break;
			}
		}

		/*changed between HDR TV and normal TV which need to update the relevant register again*/
		/*using the newest buffer info*/
		/* or if the first part of source is hdr and the second half is SDR, need to update register */
		/* or TV or buffer source changed for when is playing hdr source */
		if ((gDispBufferInfo[path].is_hdr != bufferInfo.is_hdr) ||
		    (gDispBufferInfo[path].is_hdr10plus != bufferInfo.is_hdr10plus) ||
			(gDispBufferInfo[path].constant_bt2020 != bufferInfo.constant_bt2020) ||
			(gDispBufferInfo[path].color_format != bufferInfo.color_format) ||
			(gDispBufferInfo[path].gamma_type != bufferInfo.gamma_type) ||

			(gTVInfo[path].is_support_hdr != pInfo->tv.is_support_hdr) ||
			(gTVInfo[path].is_support_601 != pInfo->tv.is_support_601) ||
			(gTVInfo[path].is_support_709 != pInfo->tv.is_support_709) ||
			(gTVInfo[path].is_support_bt2020 != pInfo->tv.is_support_bt2020)) {
			memcpy(&gDispBufferInfo[path], &bufferInfo, sizeof(gDispBufferInfo[0]));
			memcpy(&gTVInfo[path], &pInfo->tv, sizeof(gTVInfo[0]));
			status = true;
			break;
		} else {
			/*checking if need to pass hdr2sdr & bt2020 module */
			if (hdr_core_need_through_hdr2sdr_bt2020_sdr2hdr(path, bufferInfo, pInfo->tv)) {
				if ((path == HDR_PATH_OSD) && !gOsdHadConfigure) {
					memcpy(&gDispBufferInfo[path], &bufferInfo, sizeof(gDispBufferInfo[0]));
					memcpy(&gTVInfo[path], &pInfo->tv, sizeof(gTVInfo[0]));
					status = true;
					gOsdHadConfigure = true;
					break;
				}
				if ((path == HDR_PATH_OSD) && gVideoIsPlayingForSubPathDolbypip) {
					if (!(gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
						((gTVInfo[HDR_PATH_MAIN].force_hdr > 1) && ((gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
						!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[HDR_PATH_MAIN].is_support_hdr10_plus)))) &&
						(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[HDR_PATH_MAIN].is_support_hdr && !gDispBufferInfo[path].is_hdr)) {
						if (!gOsdHadConfigureForSubPath) {
							/*for non-dolby source/path, if main video is hdr and sub video is sdr, need to reconfigure osd path for sub video*/
							/*sub video need to outputs hdr source and use new sdr2hdr&bt2020 configuration for sub video */
							HDR_LOG("reconfigure osd path for osd video\n");
							memcpy(&gDispBufferInfo[path], &bufferInfo, sizeof(gDispBufferInfo[0]));
							memcpy(&gTVInfo[path], &pInfo->tv, sizeof(gTVInfo[0]));
							status = true;
							gOsdHadConfigureForSubPath = true;
							break;
						}
					}
				}
				if ((path == HDR_PATH_OSD) && gSubPathHavestopped && gOsbPathHaveReconfiguedForSubVideo) {
					if (!(gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
						((gTVInfo[HDR_PATH_MAIN].force_hdr > 1) && ((gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG) &&
						!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr10plus && gTVInfo[HDR_PATH_MAIN].is_support_hdr10_plus)))) &&
						(gDispBufferInfo[HDR_PATH_MAIN].is_hdr && gTVInfo[HDR_PATH_MAIN].is_support_hdr && !gDispBufferInfo[path].is_hdr)) {
							/*for non-dolby source/path, if sub video have stopped, need to reconfigure osd path for netflix graphic*/
							/*use new sdr2hdr&bt2020 configuration for netflix graphic certification */
							memcpy(&gDispBufferInfo[path], &bufferInfo, sizeof(gDispBufferInfo[0]));
							memcpy(&gTVInfo[path], &pInfo->tv, sizeof(gTVInfo[0]));
							status = true;
							gSubPathHavestopped = false;
							gOsbPathHaveReconfiguedForSubVideo = false;
							break;
					}
				}
			}
		}
	} while (0);

	if (status) {
		HDR_LOG("update HDR, dump info\n");
		HDR_LOG("colorPrimaries[%d] transformCharacter[%d] matrixCoeffs[%d]\n",
			pConfig->hdr_info.colorPrimaries,
			pConfig->hdr_info.transformCharacter,
			pConfig->hdr_info.matrixCoeffs);
		HDR_LOG("path:%d buffer info: is_hdr[%d] constant_bt2020[%d] color_format[%d] gamma_type[%d]\n",
			path, gDispBufferInfo[path].is_hdr,
			gDispBufferInfo[path].constant_bt2020,
			gDispBufferInfo[path].color_format,
			gDispBufferInfo[path].gamma_type);
		HDR_LOG("TV info: support hdr[%d] BT601[%d] BT709[%d] BT2020[%d]\n",
			gTVInfo[path].is_support_hdr,
			gTVInfo[path].is_support_601,
			gTVInfo[path].is_support_709,
			gTVInfo[path].is_support_bt2020);
	}
	return status;
}

/* config hdr: prepare hdr setting
** HWC set buffer to display manager
** queue a thread to handle
** three threads for MAIN/SUB/OSD task
*/
int _hdr_core_handle_disp_config(struct mtk_disp_buffer *pConfig, struct disp_hw_common_info *pInfo)
{
	struct config_info_struct *pHdrConfig = NULL;
	mutex_lock(&sync_lock_for_disp_configure_and_stop);

	/* check if hdr need reconfig */
	if (!_hdr_core_hdr_need_reconfig(pConfig, pInfo)) {
		mutex_unlock(&sync_lock_for_disp_configure_and_stop);
		return 0;
	}
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (hdr_cli_get_info()->updateHDR != 0) {
		HDR_DBG("Had updated hdr from cli, need to reset to default value\n");
		hdr_cli_get_info()->updateHDR = 0;
	}
#endif

	HDR_LOG("config hdr info\n");
	do {
		pHdrConfig = vmalloc(sizeof(struct config_info_struct));
		memset(pHdrConfig, 0, sizeof(struct config_info_struct));
		/* fill HDR config according to disp_buffer info & hdmi info */
		_hdr_core_fill_config_info(pHdrConfig, pConfig, pInfo);

		hdr_core_config_path(pHdrConfig);
		#if 0
		INIT_WORK(&pHdrConfig->workItem, hdr_core_config_path);
		queue_work(gHdrThread[pHdrConfig->path], &pHdrConfig->workItem);
		#endif
	} while (0);
	mutex_unlock(&sync_lock_for_disp_configure_and_stop);
	return 0;
}



/* update hdr setting */
static int _hdr_core_handle_irq(uint32_t irq)
{
	struct config_info_struct *pConfig = NULL;
	struct config_info_struct *pTempConfig = NULL;
	enum HDR_STATUS status;
	int plane;
	/* if had configured bt2020, disp video out needs to ot need to select bt2020 video out */
	bool bt2020_need_update = false;

	if (irq != DISP_IRQ_FMT_VSYNC)
		return 0;

	/* HDR module is not ready, don't handle IRQ */
	if (gConfigListHeadInit == false)
		return 0;

	/*  update setting to register. */
	list_for_each_entry_safe(pConfig, pTempConfig, &gConfigListHead[HDR_PATH_MAIN], listEntry) {
		HDR_LOG("write HDR path: %d\n", pConfig->path);
		/* write bt2020 */
		if (hdr_device_map_bt2020_plane_from_path(pConfig->path, &plane)) {
			status = bt2020_update(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
			bt2020_need_update = true;
		}

		/* write sdr2hdr */
		if (hdr_device_map_sdr2hdr_plane_from_path(pConfig->path, &plane)) {
			status = sdr2hdr_update(plane, pConfig);
			if (status != HDR_STATUS_OK)
				break;
		}

		/* write hdr2sdr */
		if (hdr_device_map_hdr2sdr_plane_from_path(pConfig->path, &plane)) {
			status = hdr2sdr_update(plane, pConfig, bt2020_need_update);
			if (status != HDR_STATUS_OK)
				break;
		}

		do {
			/* Config HDR clock and path */
			HDR_LOG("config hdr path and clock\n");
			INIT_WORK(&pConfig->workItem, hdr_core_handle_clock_path);
			queue_work(gHandleHdrClockPathThred, &pConfig->workItem);
		} while (0);

		list_del_init(&pConfig->listEntry);
		}

	return 0;
}

/* get TV timing from vdp*/
int hdr_core_handle_tv_display_source(enum HDR10_TYPE_ENUM TVTiming)
{
	HDR_LOG("hdr_core_handle_tv_display_source,TVTiming[%d]\n", TVTiming);
	return 0;
}


int hdr_core_handle_disp_stop(enum HDR_PATH_ENUM path)
{
	/* stop HDR HW */
	struct config_info_struct *pHdrConfig = NULL;
	mutex_lock(&sync_lock_for_disp_configure_and_stop);

	/*verify if the disp path is valid*/
	if (HDR_VERIFY_PATH(path)) {
		HDR_ERR("invalid path[%d] for HDR2SDR stopping, line[%d]\n", path, __LINE__);
		mutex_unlock(&sync_lock_for_disp_configure_and_stop);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	/* judge if need to stop hdr2sdr, bt2020 or sdr2hdr*/
	if (!hdr_core_had_through_hdr2sdr_bt2020_sdr2hdr(path)) {
		/*if the video has been stopped, it still needs to clear the main path data */
		/*even if it didn't use hdr2sdr&bt2020, because the info of video has been saved when we reconfig */
		if (path == HDR_PATH_MAIN) {
			gVideoIsPlaying = false;
			memset(&gDispBufferInfo[path], 0, sizeof(gDispBufferInfo[0]));
			memset(&gTVInfo[path], 0, sizeof(gTVInfo[0]));
			gFirstConfigure[path] = true;
		}
		if (path == HDR_PATH_SUB) {
			gVideoIsPlayingForSubPathDolbypip = false;
			mutex_lock(&sync_lock_for_sub_path);
			gSubVideoCurrentPlayingState = false;
			mutex_unlock(&sync_lock_for_sub_path);
			memset(&gDispBufferInfo[path], 0, sizeof(gDispBufferInfo[0]));
			memset(&gTVInfo[path], 0, sizeof(gTVInfo[0]));
			gFirstConfigure[path] = true;
			gSubPathHavestopped = true;
			gOsdHadConfigureForSubPath = false;
		}
		mutex_unlock(&sync_lock_for_disp_configure_and_stop);
		return 0;
	}
	if ((path == HDR_PATH_OSD) || (path == HDR_PATH_MAIN)) {
		gOsdHadConfigure = false;
		gVideoIsPlaying = false;
	}
	if (path == HDR_PATH_SUB) {
		gVideoIsPlayingForSubPathDolbypip = false;
		mutex_lock(&sync_lock_for_sub_path);
		gSubVideoCurrentPlayingState = false;
		mutex_unlock(&sync_lock_for_sub_path);
		gSubPathHavestopped = true;
		gOsdHadConfigureForSubPath = false;
	}

#ifdef HDR_SECURE_SUPPORT
		/* stop main path hdr2sdr tustzone*/
		if (path == HDR_PATH_MAIN)
			hdr_sec_get_share_memory()->use_hdr = 0;

		/* stop sub path hdr2sdr tustzone*/
		if (path == HDR_PATH_SUB)
			hdr_sec_get_share_memory()->use_hdr_sub_path = 0;

		if (path != HDR_PATH_OSD)
			hdr_sec_service_call(SERVICE_CALL_CMD_COPY_SHARE_MEMORY,
				SERVICE_CALL_DIRECTION_INPUT,
				hdr_sec_get_share_memory(), sizeof(*hdr_sec_get_share_memory()));
#else
		HDR_ERR("trustzone is not ready, can't stop hdr2sdr\n");
#endif
	HDR_LOG("stop HDR hardware, path[%d].\n", path);
	/*when file had stopped, disp buffer and TV info which need to reset*/
	memset(&gDispBufferInfo[path], 0, sizeof(gDispBufferInfo[0]));
	memset(&gTVInfo[path], 0, sizeof(gTVInfo[0]));
	gFirstConfigure[path] = true;

	pHdrConfig = vmalloc(sizeof(struct config_info_struct));
	memset(pHdrConfig, 0, sizeof(struct config_info_struct));
	/* fill HDR config info according to disp_buffer info & hdmi info */
	pHdrConfig->path = path;

	/* disable main/sub path bt2020 and hdr2sdr clock register */
	if ((path == HDR_PATH_MAIN) || (path == HDR_PATH_SUB)) {

		pHdrConfig->HDR2SDRConfig.need_update = true;
		pHdrConfig->HDR2SDRConfig.inputType.bypassModule = true;

		pHdrConfig->BT2020Config.need_update = true;
		pHdrConfig->BT2020Config.inputType.bypassModule = true;
	}

	/* disable osd path bt2020 and sdr2hdr clock register */
	if (path == HDR_PATH_OSD) {
		/*now sdr2hdr direct links osd, it did not need to bypass even if video stopped*/
		/*pHdrConfig->BT2020Config.need_update = true;*/
		pHdrConfig->BT2020Config.inputType.bypassModule = true;

		pHdrConfig->SDR2HDRConfig.need_update = true;
		/*pHdrConfig->SDR2HDRConfig.inputType.bypassModule = true;*/
	}
	hdr_core_config_path(pHdrConfig);
	#if 0
	INIT_WORK(&pHdrConfig->workItem, hdr_core_config_path);
	queue_work(gHdrThread[path], &pHdrConfig->workItem);
	#endif
	mutex_unlock(&sync_lock_for_disp_configure_and_stop);
	return 0;
}


int hdr_core_handle_disp_suspend(void)
{
	/* supend all HW */
	struct config_info_struct *pHdrConfig = NULL;
	enum HDR_PATH_ENUM path;

#ifdef HDR_SECURE_SUPPORT
		hdr_sec_get_share_memory()->use_hdr = 0;
		hdr_sec_get_share_memory()->use_hdr_sub_path = 0;
		hdr_sec_service_call(SERVICE_CALL_CMD_COPY_SHARE_MEMORY,
			SERVICE_CALL_DIRECTION_INPUT,
			hdr_sec_get_share_memory(), sizeof(*hdr_sec_get_share_memory()));
#else
		HDR_ERR("trustzone is not ready, can't suspend hdr2sdr\n");
#endif
	HDR_LOG("suspend all HDR hardware.\n");

	osd_enable_sdr2hdr = false;
	gOsdHadConfigure = false;
	gOsdHadConfigureForSubPath = false;

	gOsdPathSuspend = true;
	gVideoIsPlaying = false;
	gVideoIsPlayingForSubPathDolbypipHadUsed = false;
	gVideoIsPlayingForSubPathDolbypip = false;
	mutex_lock(&sync_lock_for_sub_path);
	gSubVideoCurrentPlayingState = false;
	mutex_unlock(&sync_lock_for_sub_path);
	gSubPathHavestopped = true;

	for (path = HDR_PATH_MAIN; path < HDR_PATH_MAX; path++) {
		memset(&gDispBufferInfo[path], 0, sizeof(gDispBufferInfo[0]));
		memset(&gTVInfo[path], 0, sizeof(gTVInfo[0]));
		gFirstConfigure[path] = true;

		pHdrConfig = vmalloc(sizeof(struct config_info_struct));
		memset(pHdrConfig, 0, sizeof(struct config_info_struct));
		/* fill HDR config according to disp_buffer info & hdmi info */
		pHdrConfig->path = path;

		pHdrConfig->BT2020Config.need_update = true;
		pHdrConfig->BT2020Config.inputType.bypassModule = true;

		pHdrConfig->SDR2HDRConfig.need_update = true;
		pHdrConfig->SDR2HDRConfig.inputType.bypassModule = true;

		pHdrConfig->HDR2SDRConfig.need_update = true;
		pHdrConfig->HDR2SDRConfig.inputType.bypassModule = true;

		hdr_core_config_path(pHdrConfig);
		#if 0
		INIT_WORK(&pHdrConfig->workItem, hdr_core_config_path);
		queue_work(gHdrThread[path], &pHdrConfig->workItem);
		#endif
	}
	return 0;
}

int hdr_core_handle_disp_resume(void)
{
	gOsdPathSuspend = false;
	return 0;
}


int hdr_core_handle_other_moudule_call(enum DISP_CMD cmd, void *data)
{
	if (data == NULL)
		return 0;
	HDR_TONE_LOG("hdr_core_handle_other_moudule_call: cmd:%d, data: %d\n", cmd, (*(int *)data));
	switch (cmd) {
	case DISP_CMD_STOP_HDR2SDR_BT2020:
		hdr_core_handle_disp_stop(*(int *)data);
		break;
	case DISP_CMD_STOP_SDR2HDR_BT2020:
		hdr_core_handle_disp_stop(*(int *)data);
		break;
	case DISP_CMD_OSD_ENABLE_SDR2HDR_BT2020:
		osd_enable_sdr2hdr = *(int *)data;
		if (osd_enable_sdr2hdr == 0)
			hdr_core_handle_disp_stop(HDR_PATH_OSD);
		break;
	case DISP_CMD_PLAY_HDR_SOURCE:
		hdr_core_handle_tv_display_source(*(enum HDR10_TYPE_ENUM *)data);
	default:
		break;
	}
	return 0;
}


struct disp_hw disp_hdr_driver = {
	.name = "hdr",
	.init = _hdr_core_init,
	.deinit = NULL,
	.start = NULL,
	.stop = hdr_core_handle_disp_stop,
	.suspend = hdr_core_handle_disp_suspend,
	.resume = hdr_core_handle_disp_resume,
	.get_info = NULL,
	.change_resolution = NULL,
	.config = _hdr_core_handle_disp_config, /* config hdr: prepare hdr setting */
	.irq_handler = _hdr_core_handle_irq, /* update hdr setting */
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = NULL,
	.set_cmd = hdr_core_handle_other_moudule_call,
};

/* register callback function to display manager */
struct disp_hw *disp_hdr_bt2020_get_drv(void)
{
	return &disp_hdr_driver;
}

/*change delay for dolby disable if hdr2sdr is enable*/
enum HDR_STATUS disp_path_set_delay_back_for_hdr2sdr(void)
{
	if (!(gDispBufferInfo[HDR_PATH_MAIN].is_hdr) ||
		(gTVInfo[HDR_PATH_MAIN].is_support_hdr || gTVInfo[HDR_PATH_MAIN].is_support_hlg)
		|| gDispBufferInfo[HDR_PATH_MAIN].is_dolby ||
		((gTVInfo[HDR_PATH_MAIN].force_hdr > 1) && (gDispBufferInfo[HDR_PATH_MAIN].gamma_type != GAMMA_TYPE_HLG))) {
		HDR_DBG("the source is non-hlg\n");
		return HDR_STATUS_INVALID_PARAM;
	}
	HDR_LOG("color_format[%d], res_mode[%d]\n", gDispBufferInfo[HDR_PATH_MAIN].color_format,
		gDispBufferInfo[HDR_PATH_MAIN].resolution.res_mode);

	/*set delay due to thruoghing hdr2sdr and bt2020 sub module*/

	disp_path_set_delay_by_shift(DISP_PATH_MVDO, gDispBufferInfo[HDR_PATH_MAIN].resolution.res_mode,
		true, 42, false, 0);

	/* path control */
	disp_sys_hal_set_disp_out(DISP_MAIN, DISP_OUT_SEL_HDR2SDR);
	return HDR_STATUS_OK;

}

