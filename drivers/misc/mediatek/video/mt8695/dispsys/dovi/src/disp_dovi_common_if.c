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
#define LOG_TAG "DOVI_COMMON_IF"

#include "disp_type.h"
#include "disp_info.h"

#include "disp_dovi_md_parser.h"
#include "disp_dovi_control_if.h"
#include "disp_dovi_common.h"
#include "disp_dovi_common_if.h"

#include "disp_hw_mgr.h"
#include "dovi_core1_hal.h"
#include "dovi_core2_hal.h"
#include "dovi_core3_hal.h"

#include "disp_dovi_core1_if.h"
#include "disp_dovi_core2_if.h"
#include "disp_dovi_core3_if.h"

#include "dovi_log.h"

#include "disp_dovi_tz_client.h"

#include "dovi_table.h"
#include "disp_dovi_io.h"
#include "disp_hw_log.h"

#define START_CODE_LEN_MAX 5
/* 0x7C 0x01 0x19 */
#define NAL_TYPE_LEN_MAX 3


UINT32 _u4VSyncCount;
uint32_t dovi_common_test;

UINT32 dovi_enable;
UINT32 dovi_log_enable;
int gmin;
int gmax;
/*dovi_enable enable*/
/*
* 0 0 already disable   0
* 0 1 disable to enable 1
* 1 0 enable to disable 2
* 1 1 already enable    3
*/
UINT32 dovi_proc_state;

UINT32 dovi_rpu_total_len;
UINT32 dovi_out_res;
UINT32 dovi_out_width;
UINT32 dovi_out_height;

BOOL dovi_md_parser_enable;
BOOL dovi_cp_enable;
BOOL dovi_core3_md_enable;

struct mtk_disp_hdr10_md_t *p_hdr_md;

struct cp_param_t *p_cp_param;
uint32_t *p_comp_md;
uint32_t *p_orig_md;

uint32_t *p_core1_reg;
uint32_t *p_core1_lut;

uint32_t *p_core2_reg;
uint32_t *p_core2_lut;

uint32_t *p_core3_reg;
uint32_t *p_hdmi_md;

struct mtk_disp_hdr10_md_t *p_hdr10_md;

unsigned char *rpu_bs_buffer;
bool *p_svp;
uint32_t *p_sec_handle;
unsigned int *p_rpu_bs_len;
enum DISP_DR_TYPE_T *p_dr_type;
uint32_t *p_frame_num;

uint32_t *p_orig_md_len;
uint32_t *p_hdmi_md_len;

uint32_t dump_md_enable;
uint32_t reg_test_enable;
bool *p_profile4;
bool set_graphic_max_lum_enable;
bool set_video_max_lum_enable;

int graphic_max_lum;
int video_max_lum;



UINT32 dovi_set_video_info(struct mtk_disp_hdr_md_info_t *hdr_metadata)
{
	dovi_func();

	p_cp_param->width = dovi_out_width;
	p_cp_param->height = dovi_out_height;

	switch (hdr_metadata->dr_range) {
	case DISP_DR_TYPE_DOVI:
		p_cp_param->input_format = SIGNAL_FORMAT_DOVI;
		break;
	case DISP_DR_TYPE_HDR10:
		p_cp_param->input_format = SIGNAL_FORMAT_HDR10;
		break;
	default:
		p_cp_param->input_format = SIGNAL_FORMAT_SDR;
		break;
	}

	p_cp_param->src_yuv_range = SIGNAL_RANGE_SMPTE;
	p_cp_param->src_bit_depth = 10;
	p_cp_param->src_fps = 60;


	dovi_default("set video info:W:%d x H:%d, input_format:%d\n",
	p_cp_param->width, p_cp_param->height, p_cp_param->input_format);

	return DOVI_RET_OK;
}


UINT32 dovi_set_video_input_format(enum dovi_signal_format_t e_input_format)
{
	if (p_cp_param != NULL) {
		p_cp_param->input_format = e_input_format;

		DISP_LOG_DEBUG("[DOVI]Re-Update HDR Type Input:%d\n", e_input_format);
	}

	return DOVI_RET_OK;
}

UINT32 dovi_get_input_format(enum DISP_DR_TYPE_T *dovi_input_dr)
{
	if ((p_cp_param != NULL) && (dovi_input_dr != NULL)) {
		switch (p_cp_param->input_format) {
		case SIGNAL_FORMAT_DOVI:
			*dovi_input_dr = DISP_DR_TYPE_DOVI;
			break;
		case SIGNAL_FORMAT_HDR10:
			*dovi_input_dr = DISP_DR_TYPE_HDR10;
			break;
		case SIGNAL_FORMAT_SDR:
			*dovi_input_dr = DISP_DR_TYPE_SDR;
			break;
		default:
			*dovi_input_dr = SIGNAL_FORMAT_SDR;
			break;
		}
	}
	return DOVI_RET_OK;
}

UINT32 dovi_set_graphic_format(UINT32 g_format)
{
	dovi_func();
	DISP_LOG_DEBUG("set_graphic_format %d\n", g_format);

	p_cp_param->g_format = g_format;

	return DOVI_RET_OK;
}

UINT32 dovi_set_graphic_info(UINT32 ucOn)
{
	dovi_func();
	DISP_LOG_DEBUG("set_graphic_on %d\n", ucOn);

	p_cp_param->f_graphic_on = ucOn;


	return DOVI_RET_OK;
}

UINT32 dovi_set_graphic_priority_mode(BOOL fgOn)
{
	DISP_LOG_DEBUG("set_graphic_on %d\n", fgOn);

	p_cp_param->priority_mode = fgOn;


	return DOVI_RET_OK;
}

UINT32 dovi_set_priority_mode(UINT32 mode)
{
	dovi_func();
	dovi_default("set_priority_mode %d\n", mode);

	if (force_priority_mode)
		p_cp_param->priority_mode = (enum pri_mode_t)priority_mode;
	else
		p_cp_param->priority_mode = mode;

	return DOVI_RET_OK;
}


UINT32 dovi_get_priority_mode(UINT32 *mode)
{
	dovi_func();
	dovi_info("get_priority_mode %d\n", p_cp_param->priority_mode);
	*mode = p_cp_param->priority_mode;

	return DOVI_RET_OK;
}
UINT32 dovi_update_graphic_info(void)
{
	dovi_func();
	DISP_LOG_DEBUG("update_graphic_info\n");

	if (gmin != 0)
		p_cp_param->graphic_max_lum = gmin;
	else
		p_cp_param->graphic_min_lum = 50;

	if (gmax != 0)
		p_cp_param->graphic_max_lum = gmax;
	else if (p_cp_param->output_format == SIGNAL_FORMAT_SDR)
		p_cp_param->graphic_max_lum = 1000000;	/* align SDR input lum */
	else if ((p_cp_param->input_format == SIGNAL_FORMAT_SDR)
		&& (p_cp_param->output_format == SIGNAL_FORMAT_HDR10))
		p_cp_param->graphic_max_lum = 3000000;	/* align Dolby tuned input lum */
	else if ((p_cp_param->input_format == SIGNAL_FORMAT_SDR)
		&& (p_cp_param->output_format == SIGNAL_FORMAT_DOVI))
		p_cp_param->graphic_max_lum = 3000000;	/* tuned by customer wanted*/
	else
		p_cp_param->graphic_max_lum = 10000000;	/* align 2.4 C model */

	if (set_graphic_max_lum_enable)
		p_cp_param->graphic_max_lum = graphic_max_lum * 10000;

	dovi_default("set graphic_max_lum %d!\n", p_cp_param->graphic_max_lum);
	p_cp_param->g_bit_depth = 8;
	/*if (!dovi_idk_test)*/
	p_cp_param->g_format = GRAPHIC_SDR_RGB;
	p_cp_param->g_cf = CHROMA_FORMAT_P444;

	return DOVI_RET_OK;
}

UINT32 dovi_set_output_format(enum dovi_signal_format_t out_format)
{
	dovi_func();

	dovi_default("set output format %d\n", out_format);
	/* The output are all dovi, when we set dovi or dovi_ll.
	*  dolby core will convert dovi to dovi_ll according the vsvdb and use_ll.
	*/
	if (out_format == DOVI_FORMAT_DOVI_LOW_LATENCY)
		out_format = DOVI_FORMAT_DOVI;

	p_cp_param->output_format = out_format;

	return DOVI_RET_OK;
}

UINT32 dovi_get_output_format(void)
{
	dovi_func();
	DISP_LOG_DEBUG("get output format %d\n", p_cp_param->output_format);

	return (UINT32)p_cp_param->output_format;
}

bool dovi_get_profile4(void)
{
	return *p_profile4;
}

UINT32 dovi_set_target_lum(int target_min_lum, int target_max_lum)
{
	dovi_func();

	p_cp_param->target_min_lum = target_min_lum;
	p_cp_param->target_max_lum = target_max_lum;

	return DOVI_RET_OK;
}

UINT32 dovi_update_target_lum(void)
{
	int target_min_lum;
	int target_max_lum;

	dovi_func();

	target_min_lum = 50;
	if (p_cp_param->input_format == SIGNAL_FORMAT_DOVI) {
		switch (p_cp_param->output_format) {
		case SIGNAL_FORMAT_DOVI:
			target_max_lum = 5000000;	/* 40000000; */
			break;
		case SIGNAL_FORMAT_HDR10:
			target_max_lum = 10000000;	/* 40000000; */
			break;
		default:
			target_max_lum = 1000000;
			break;
		}
	} else if (p_cp_param->input_format == SIGNAL_FORMAT_HDR10) {
		switch (p_cp_param->output_format) {
		case SIGNAL_FORMAT_DOVI:
			target_max_lum = 5000000;
			break;
		case SIGNAL_FORMAT_HDR10:
			target_max_lum = 10000000;
			break;
		default:
			target_max_lum = 1000000;
			break;
		}
	} else {
		switch (p_cp_param->output_format) {
		case SIGNAL_FORMAT_DOVI:
			target_max_lum = 5000000;
			break;
		case SIGNAL_FORMAT_HDR10:
			target_max_lum = 10000000;
			break;
		default:
			target_max_lum = 1000000;
			break;
		}
	}

	if (p_cp_param->output_format == SIGNAL_FORMAT_SDR)
		p_cp_param->graphic_max_lum = 1000000;

	/* adjust video lum with cli*/
	if (set_video_max_lum_enable)
		target_max_lum = video_max_lum * 10000;

	p_cp_param->target_min_lum = target_min_lum;
	p_cp_param->target_max_lum = target_max_lum;

	dovi_info("out_format %d, target luam %d %d\n",
	p_cp_param->output_format,
	target_min_lum, target_max_lum);

	return DOVI_RET_OK;
}


uint32_t dovi_set_out_res(uint32_t out_res, uint16_t width,	uint16_t height)
{
	/* dovi_func(); */
	dovi_out_res = out_res;
	dovi_out_width = width;
	dovi_out_height = height;

	return DOVI_RET_OK;
}

UINT32 dovi_set_composer_mode(BOOL fgComposerEL)
{
	dovi_core1_hal_set_composer_mode(fgComposerEL);
	return DOVI_RET_OK;
}

UINT32 dovi_set_low_latency_mode(int use_ll, int ll_rgb_desired)
{
	dovi_func();

	dovi_default("set use_ll %d ll_rgb_desired %d\n", use_ll, ll_rgb_desired);

	p_cp_param->use_ll = use_ll;
	p_cp_param->ll_rgb_desired = ll_rgb_desired;

	return DOVI_RET_OK;
}

UINT32 dovi_get_low_latency_mode(void)
{
	dovi_func();

	dovi_default("get use_ll  %d\n", p_cp_param->use_ll);

	return (UINT32)p_cp_param->use_ll;
}

UINT32 dovi_set_dovi2hdr10_mapping(int dovi2hdr10_mapping)
{
	dovi_func();

	p_cp_param->dovi2hdr10_mapping = dovi2hdr10_mapping;

	dovi_default("set dovi2hdr10_mapping %d\n", dovi2hdr10_mapping);

	return DOVI_RET_OK;
}

UINT32 dovi_set_vsvdb_file_name(char *vsvdb_file_name)
{
	dovi_func();

	memset(p_cp_param->vsvdb_file, 0, MAX_FILENAME_LENGTH);
	sprintf(p_cp_param->vsvdb_file, "/sdcard/vsvdb/%s", vsvdb_file_name);
	disp_dovi_load_buffer(p_cp_param->vsvdb_file,
		(unsigned char *)p_cp_param->vsvdb_hdmi, 0x1A);
	dovi_default("set vsvdb_file_name %s\n", vsvdb_file_name);

	return DOVI_RET_OK;
}

UINT32 dovi_set_vsvdb_hdmi(char *vsvdb_edid, int len)
{
	unsigned int i = 0;

	dovi_func();

	if ((vsvdb_edid != NULL) && (len <= 0x1A)) {
		memset(p_cp_param->vsvdb_hdmi, 0, 0x1A);
		memcpy((void *)p_cp_param->vsvdb_hdmi, (void *)vsvdb_edid, len);
	}
	dovi_vsvdb("vsvdb info below:\n");
	for (i = 0; i < 0x1A; i++)
		dovi_vsvdb("0x%x ", p_cp_param->vsvdb_hdmi[i]);
	return DOVI_RET_OK;
}

int dovi_remove_rpu_nal_type(unsigned int first_frame,
			     unsigned char *src_rpu, unsigned char *dst_rpu, unsigned int len)
{
	unsigned int idx = 0;
	unsigned int dst_len = 0;
	unsigned int rpu_len = 0;
	unsigned int nal_len = START_CODE_LEN_MAX;
	unsigned int start_idx = nal_len;
	unsigned char start_code[START_CODE_LEN_MAX] = { 0x00, 0x00, 0x00, 0x01, 0x19 };

	/* remove start code and find out the nal type start idx */
	for (idx = 0; idx < len; idx++) {
		if ((idx >= 3) &&
		    (src_rpu[idx - 3] == 0x0) &&
		    (src_rpu[idx - 2] == 0x0) && (src_rpu[idx - 1] == 0x1)) {
			break;
		}
	}

	if (idx >= len) {
		dovi_error("can not find out start code\n");
		return -1;
	}

	/* keep the start code for the first frame */
	if (first_frame) {
		start_idx = nal_len;
		memcpy((VOID *) (dst_rpu), (VOID *) (start_code), nal_len);
	} else
		start_idx = 0;

	dst_len += start_idx;

	/* copy the real rpu to dst bufffer */
	rpu_len = (len - idx - NAL_TYPE_LEN_MAX);
	memcpy((VOID *) (dst_rpu + start_idx), (VOID *) (src_rpu + idx + NAL_TYPE_LEN_MAX),
	       rpu_len);

	/* copy the start code to the end of the rpu for md parser */
	dst_len += rpu_len;
	memcpy((VOID *) (dst_rpu + dst_len), (VOID *) (start_code), nal_len);

	dst_len += nal_len;


	return dst_len;
}

int disp_dovi_common_init(void)
{
	struct dovi_share_memory_info_t *p_share_mem = dovi_share_mem;

	p_dr_type = &p_share_mem->dr_type;
	p_hdr_md = &p_share_mem->hdr10_md;
	p_rpu_bs_len = &p_share_mem->rpu_bs_len;
	p_frame_num = &p_share_mem->frame_num;
	p_svp = &p_share_mem->svp;
	p_sec_handle = &p_share_mem->sec_handle;
	rpu_bs_buffer = p_share_mem->rpu_bs_buffer;
	p_cp_param = &p_share_mem->cp_param;
	p_comp_md = p_share_mem->comp_md;
	p_orig_md = p_share_mem->orig_md;
	p_hdmi_md = p_share_mem->hdmi_md;
	p_hdr10_md = &p_share_mem->hdr10_info_frame;

	p_orig_md_len = &p_share_mem->orig_md_len;
	p_hdmi_md_len = &p_share_mem->hdmi_md_len;

	p_core1_reg = p_share_mem->core1_reg;
	p_core2_reg = p_share_mem->core2_reg;
	p_core3_reg = p_share_mem->core3_reg;

	p_core1_lut = p_share_mem->core1_lut;
	p_core2_lut = p_share_mem->core2_lut;

	p_profile4 = &p_share_mem->profile4;
#if DOVI_TZ_OK
#else
	disp_dovi_common_test(2);
#endif

	return DOVI_RET_OK;
}

int disp_dovi_common_test(uint32_t option)
{
	struct dovi_share_memory_info_t *p_share_mem = dovi_share_mem;

	/* dovi_func(); */

	/* dovi_default("common test option %d\n", option); */

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
	} else if (option == 3) {
		/* idk 2.4.1 5200, core1 yuv input, core3 dovi output, 1080p */
		p_core1_reg = dovi_v241_1080p_yuv_ipt_core1_reg;
		p_core2_reg = dovi_v241_1080p_ipt_yuv_core2_reg;
		p_core3_reg = dovi_v241_1080p_yuv_ipt_core3_reg;
		p_core1_lut = dovi_v241_1080p_yuv_ipt_core1_lut;
		p_core2_lut = dovi_v241_1080p_ipt_yuv_core2_lut;
	} else if (option == 4) {
		/* idk 2.4.1 5000, core1 ipt input, core3 dovi output, 1080p */
		p_core1_reg = dovi_v241_1080p_ipt_ipt_core1_reg;
		p_core2_reg = dovi_v241_1080p_ipt_ipt_core2_reg;
		p_core3_reg = dovi_v241_1080p_ipt_ipt_core3_reg;
		p_core1_lut = dovi_v241_1080p_ipt_ipt_core1_lut;
		p_core2_lut = dovi_v241_1080p_ipt_ipt_core2_lut;
	} else {
		p_core1_reg = p_share_mem->core1_reg;
		p_core2_reg = p_share_mem->core2_reg;
		p_core3_reg = p_share_mem->core3_reg;

		p_core1_lut = p_share_mem->core1_lut;
		p_core2_lut = p_share_mem->core2_lut;
	}

	return DOVI_RET_OK;
}

int disp_dovi_md(void)
{
	if (dump_md_enable) {
		if (p_cp_param->input_format == SIGNAL_FORMAT_DOVI) {
			disp_dovi_dump_comp(*p_frame_num,
			(unsigned char *)p_comp_md, DOVI_COMP_SIZE);
			disp_dovi_dump_orig_md(*p_frame_num,
			(unsigned char *)p_comp_md, *p_orig_md_len);
		}

		disp_dovi_dump_core_reg(1, *p_frame_num,
		(unsigned char *)p_core1_reg, CORE1_DPM_REG_NUM*4);

		disp_dovi_dump_core_reg(2, *p_frame_num,
		(unsigned char *)p_core2_reg, CORE2_DPM_REG_NUM*4);

		disp_dovi_dump_core_reg(3, *p_frame_num,
		(unsigned char *)p_core3_reg, CORE3_DPM_REG_NUM*4);

		disp_dovi_dump_core_lut(1, *p_frame_num,
		(unsigned char *)p_core1_lut, DOVI_LUT_SIZE);

		disp_dovi_dump_core_lut(2, *p_frame_num,
		(unsigned char *)p_core2_lut, DOVI_LUT_SIZE);
	}

	return DOVI_RET_OK;
}

int disp_dovi_process_core2(uint32_t enable)
{
	if (1) {
		/* core2 init */
		dovi_info("set core2 hal enable %d\n", enable);
		dovi_core2_hal_set_enable(enable);
	}

	return DOVI_RET_OK;
}

int disp_dovi_set_hdr10_param(
	struct mtk_disp_hdr10_md_t *p_hdr10_param,
	struct mtk_disp_hdr_md_info_t *hdr_metadata)
{
	struct mtk_disp_hdr10_md_t *p_hdr10_metadata = &hdr_metadata->metadata_info.hdr10_metadata;

	if ((p_hdr10_param != NULL) && (p_hdr10_metadata != NULL)) {
		memcpy((void *)p_hdr10_param,
		(void *)p_hdr10_metadata, sizeof(struct mtk_disp_hdr10_md_t));
	}

	return DOVI_RET_OK;
}

int disp_dovi_update_hdr10_info_frame(
	struct mtk_disp_hdr10_md_t *p_hdr10_info_frame,
	struct mtk_disp_hdr_md_info_t *hdr_metadata)
{
	VID_STATIC_HDMI_MD_T *p_hdr10_metadata = &hdr_metadata->hdr10_info.metadata_info.hdr10_metadata;

	hdr_metadata->hdr10_info.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10;
	hdr_metadata->hdr10_info.fgIsMetadata = true;

	p_hdr10_metadata->ui2_DisplayPrimariesX[0] = p_hdr10_info_frame->ui2_DisplayPrimariesX[0];
	p_hdr10_metadata->ui2_DisplayPrimariesX[1] = p_hdr10_info_frame->ui2_DisplayPrimariesX[1];
	p_hdr10_metadata->ui2_DisplayPrimariesX[2] = p_hdr10_info_frame->ui2_DisplayPrimariesX[2];
	p_hdr10_metadata->ui2_DisplayPrimariesY[0] = p_hdr10_info_frame->ui2_DisplayPrimariesY[0];
	p_hdr10_metadata->ui2_DisplayPrimariesY[1] = p_hdr10_info_frame->ui2_DisplayPrimariesY[1];
	p_hdr10_metadata->ui2_DisplayPrimariesY[2] = p_hdr10_info_frame->ui2_DisplayPrimariesY[2];
	p_hdr10_metadata->ui2_WhitePointX = p_hdr10_info_frame->ui2_WhitePointX;
	p_hdr10_metadata->ui2_WhitePointY = p_hdr10_info_frame->ui2_WhitePointY;
	p_hdr10_metadata->ui2_MaxDisplayMasteringLuminance = p_hdr10_info_frame->ui2_MaxDisplayMasteringLuminance;
	p_hdr10_metadata->ui2_MinDisplayMasteringLuminance = p_hdr10_info_frame->ui2_MinDisplayMasteringLuminance;
	p_hdr10_metadata->ui2_MaxCLL = p_hdr10_info_frame->ui2_MaxCLL;
	p_hdr10_metadata->ui2_MaxFALL = p_hdr10_info_frame->ui2_MaxFALL;
	p_hdr10_metadata->fgNeedUpdStaticMeta = 1;
	dovi_info("p_hdr10_metadata ui2_DisplayPrimariesX[%d %d %d]\n",
			p_hdr10_metadata->ui2_DisplayPrimariesX[0],
			p_hdr10_metadata->ui2_DisplayPrimariesX[1],
			p_hdr10_metadata->ui2_DisplayPrimariesX[2]);
	dovi_info("p_hdr10_metadata ui2_DisplayPrimariesY[%d %d %d]\n",
			p_hdr10_metadata->ui2_DisplayPrimariesY[0],
			p_hdr10_metadata->ui2_DisplayPrimariesY[1],
			p_hdr10_metadata->ui2_DisplayPrimariesY[2]);
	dovi_info("p_hdr10_metadata ui2_WhitePointX Y[%d %d]\n",
			p_hdr10_metadata->ui2_WhitePointX,
			p_hdr10_metadata->ui2_WhitePointY);
	dovi_info("p_hdr10_metadata max min[%d %d]\n",
			p_hdr10_metadata->ui2_MaxDisplayMasteringLuminance,
			p_hdr10_metadata->ui2_MinDisplayMasteringLuminance);
	dovi_info("p_hdr10_metadata ui2_MaxCLL ui2_MaxFALL[%d %d]\n",
			p_hdr10_metadata->ui2_MaxCLL,
			p_hdr10_metadata->ui2_MaxFALL);
	return DOVI_RET_OK;
}



int disp_dovi_process(uint32_t enable,
struct mtk_disp_hdr_md_info_t *hdr_metadata)
{
	enum DISP_DR_TYPE_T dovi_pre_input_dr_type = DISP_DR_TYPE_DOVI;
	dovi_proc_state = ((dovi_enable << 1) | enable);
	*p_dr_type = hdr_metadata->dr_range;

	/* enable or already enable dovi */
	if ((dovi_proc_state & 1) == 1) {
		if ((dovi_proc_state == 1)
			|| (hdr_metadata->enable == DOVI_INOUT_FORMAT_CHANGE)) {
			*p_frame_num = 0;
			dovi_get_input_format(&dovi_pre_input_dr_type);
			dovi_set_video_info(hdr_metadata);
		}

		/* hw setting generate by control path drv */
		if (hdr_metadata->dr_range == DISP_DR_TYPE_DOVI) {
			struct mtk_vdp_dovi_md_t *dovi_md_info;
			unsigned int first_frame = (dovi_proc_state == 1);

			dovi_md_info = &(hdr_metadata->metadata_info.dovi_metadata);

			/* do init process */
			if (dovi_proc_state == 1) {
				dovi_sec_md_parser_init();
				dovi_rpu_total_len = 0;
				dovi_md_parser_enable = true;
				*p_frame_num = 0;
			}

			if (dovi_md_parser_enable == false) {
				dovi_sec_md_parser_init();
				dovi_md_parser_enable = true;
				dovi_rpu_total_len = 0;
				first_frame = 1;
				*p_frame_num = 0;
			} else if ((dovi_proc_state != 1) && (dovi_pre_input_dr_type != DISP_DR_TYPE_DOVI)) {
				dovi_sec_md_parser_uninit();
				dovi_sec_md_parser_init();
				dovi_md_parser_enable = true;
				dovi_rpu_total_len = 0;
				first_frame = 1;
				*p_frame_num = 0;
				DISP_LOG_DEBUG("seamless change dovi input again\n");
			}

			if (dovi_md_info->svp) {
				*p_svp = true;
				*p_sec_handle = dovi_md_info->sec_handle;
				*p_rpu_bs_len = dovi_md_info->len;
				dovi_rpu("svp process frame %d rpu_size %d len %d\n",
					    *p_frame_num, dovi_md_info->len, dovi_md_info->len);
			} else {
				*p_svp = false;
				/* remove rpu NAL type */
				*p_rpu_bs_len = dovi_remove_rpu_nal_type(first_frame,
							       dovi_md_info->buff,
							       rpu_bs_buffer, dovi_md_info->len);

				if (*p_rpu_bs_len < 0) {
					dovi_error("rpu data is invalid, can't find start code\n");
					return DOVI_RET_ERROR;
				}
				dovi_rpu_total_len += *p_rpu_bs_len;

				dovi_rpu("process frame %d rpu_size %d len %d total %d\n",
					    *p_frame_num,
					    dovi_md_info->len, *p_rpu_bs_len, dovi_rpu_total_len);
			}
		}

		if (hdr_metadata->dr_range == DISP_DR_TYPE_HDR10)
			disp_dovi_set_hdr10_param(p_hdr_md, hdr_metadata);

		/* hw setting generate by control path drv */
		if (dovi_proc_state == 1) {
			/* do init process */
			dovi_update_target_lum();

			dovi_update_graphic_info();


			/* graphic on before init cli_param.f_graphic_on */
			if (!dovi_idk_test)
				dovi_set_graphic_info(enable);

			/* core1 init */
			dovi_core1_hal_set_enable(enable);

			/* core2 init */
			/* dovi_core2_hal_set_enable(enable); */

			/* core3 init */
			dovi_core3_hal_set_enable(enable);

			dovi_core1_hal_control_config(&dovi_res);
			dovi_core2_hal_control_config(&dovi_res);
			dovi_core3_hal_control_config(&dovi_res);

			dovi_sec_cp_test_init();

			if (reg_test_enable) {
				dovi_info("set ipcore with default setting\n");
				if (p_cp_param->output_format == SIGNAL_FORMAT_DOVI)
					disp_dovi_common_test(4);
				else
					disp_dovi_common_test(2);
			}
		}

		if (hdr_metadata->enable == DOVI_INOUT_FORMAT_CHANGE) {
			dovi_update_target_lum();
			dovi_update_graphic_info();
			dovi_sec_cp_test_init();
		} else if (hdr_metadata->enable == DOVI_RESOLUTION_CHANGE) {
			dovi_set_video_info(hdr_metadata);
			dovi_core1_hal_control_config(&dovi_res);
			dovi_core2_hal_control_config(&dovi_res);
			dovi_core3_hal_control_config(&dovi_res);
			dovi_sec_cp_test_init();
		} else if (hdr_metadata->enable == DOVI_PRIORITY_MODE_CHANGE)
			dovi_sec_cp_test_init();

		*p_profile4 = false;
		dovi_sec_cp_test_main();

		if (p_cp_param->output_format == SIGNAL_FORMAT_HDR10)
			disp_dovi_update_hdr10_info_frame(p_hdr10_md, hdr_metadata);

		disp_dovi_md();

		/* call dovi core1 hal api to set hw register */
		if (hdr_metadata->enable == DOVI_PRIORITY_MODE_CHANGE) {
			#if 0
			if (p_cp_param->priority_mode == 1)
				dovi_core1_hal_bypass_cvm(false);
			else
				dovi_core1_hal_bypass_cvm(true);
			#endif
			dovi_core1_hal_bypass_cvm(false);
		}
		dovi_core1_hal_config_reg(p_core1_reg);
		dovi_core1_config_lut(p_core1_lut);


		/* call dovi core2 hal api to set hw register */
		dovi_core2_hal_config_reg(p_core2_reg);
		dovi_core2_config_lut(p_core2_lut);

		/* call dovi core3 hal api to set hw register */
		dovi_core3_hal_config_reg(p_core3_reg);

		/* swtich display path register setting */
		++(*p_frame_num);

		if (dovi_proc_state == 1)
			dovi_enable = enable;
	} else if (dovi_proc_state == 2) {
		/* disable dovi path */
		dovi_info("dovi path disable vysnc %d\n", _u4VSyncCount);

		/* do uninit process */
		dovi_sec_md_parser_uninit();
		dovi_md_parser_enable = false;

		dovi_sec_cp_test_uninit();

		/* control path uninit */
		dovi_core1_hal_set_enable(enable);

		/* core2 path uninit */
		dovi_core2_hal_set_enable(enable);

		/* core3 path uninit */
		dovi_core3_hal_set_enable(enable);

		dovi_enable = enable;
	}

	return DOVI_RET_OK;
}
