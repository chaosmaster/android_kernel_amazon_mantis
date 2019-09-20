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


#include "disp_pp_if.h"
#include "pp_hal.h"
#include "pp_drv.h"
#include "pp_hw.h"
#include "disp_type.h"
#include "disp_hw_mgr.h"
#include "disp_info.h"



/******************************************************************************
*Function: disp_pp_vdieo_proc(enum POST_UI_ITEM_ENUM proc_item, int16_t ui_min,
								int16_t ui_max, int16_t ui_dft, int16_t ui_cur)
*Description: PostProcess or video  setting
*Parameter: proc_item, ui_min, ui_max, ui_dft, ui_cur
*Return: PP_OK/PP_FAIL
******************************************************************************/
int32_t disp_pp_vdieo_proc(enum POST_UI_ITEM_ENUM proc_item, int16_t ui_min, int16_t ui_max,
			   int16_t ui_dft, int16_t ui_cur)
{
	uint16_t qty_item;

	PP_INFO("Set Video%d, %d, %d, %d, %d\n", (uint32_t) proc_item, ui_min, ui_max, ui_dft,
		ui_cur);
	switch (proc_item) {
	case POST_VIDEO_SHARPNESS:
		if (ui_min == 0)
			ui_min = 1;
		sharp_ui_min = ui_min;
		sharp_ui_max = ui_max;
		sharp_dft = ui_dft;
		sharp_cur = ui_cur;
		for (qty_item = QUALITY_TDSHARP_BEGIN; qty_item <= QUALITY_TDSHARP_END; qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		for (qty_item = QUALITY_PRE_SHARP_BEGIN; qty_item <= QUALITY_PRE_SHARP_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		if (ui_cur == 0) {
			pp_hal_sharp_enable(SV_OFF);
			pp_hal_update_sharp();
		} else {
			pp_hal_sharp_enable(SV_ON);
			pp_hal_update_sharp();
		}
		PP_INFO("Set Sharpness, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_CTI:
		for (qty_item = QUALITY_CTI_BEGIN; qty_item <= QUALITY_CTI_END; qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		if (ui_cur == 0) {
			pp_hal_cti_enable(SV_OFF);
			pp_hal_cti_update();
		}

		else {
			pp_hal_cti_enable(SV_ON);
			pp_hal_cti_update();
		}
		PP_INFO("SetCTI, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_CONTRAST:
		write_qty_table(QUALITY_CONTRAST,
				pp_qty_mapping(QUALITY_CONTRAST, ui_min, ui_max, ui_dft, ui_cur));
		pp_hal_update_main_con(read_qty_table(QUALITY_CONTRAST));
		pp_hal_color_proc_bypass(0);
		PP_INFO("SetContrast, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_BRIGHTNESS:
		write_qty_table(QUALITY_BRIGHTNESS,
				pp_qty_mapping(QUALITY_BRIGHTNESS, ui_min, ui_max, ui_dft, ui_cur));
		pp_hal_update_main_bri(read_qty_table(QUALITY_BRIGHTNESS));
		pp_hal_color_proc_bypass(0);
		PP_INFO("SetBrightness, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_SATURATION:
		write_qty_table(QUALITY_SATURATION,
				pp_qty_mapping(QUALITY_SATURATION, ui_min, ui_max, ui_dft, ui_cur));
		pp_hal_update_main_sat(read_qty_table(QUALITY_SATURATION));
		pp_hal_color_proc_bypass(0);
		PP_INFO("SetSaturation, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_HUE:
		write_qty_table(QUALITY_HUE,
				pp_qty_mapping(QUALITY_HUE, ui_min, ui_max, ui_dft, ui_cur));
		pp_hal_update_main_hue(read_qty_table(QUALITY_HUE));
		pp_hal_color_proc_bypass(0);
		PP_INFO("SetHue, _level=%d\n", ui_cur);
		break;
	case POST_VIDEO_SCE_ONOFF:

		/*pp_hal_set_sce_table(ui_cur);  */
		break;
	case POST_VIDEO_CDS:	/*patentissue, cannotuse */
		for (qty_item = QUALITY_CDS_BEGIN; qty_item <= QUALITY_CDS_END; qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		if (ui_cur == 0) {
			pp_hal_cds_enable(SV_OFF);
		}

		else {
			pp_hal_cds_enable(SV_ON);
			pp_hal_cds_update();
		}
		break;
	case POST_VIDEO_ADAPTIVE_LUMA_ONOFF:
		if (ui_min == 0)
			ui_min = 1;

		for (qty_item = QUALITY_ADAPTIVE_LUMA_BEGIN;
		     qty_item <= QUALITY_ADAPTIVE_LUMA_END; qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_auto_con_enable(SV_OFF);
		pp_auto_con_set_dft();
		pp_hal_auto_con_enable(ui_cur); /*ui_cur==0 means turn off. */
		PP_INFO("SetAutoContrastOn/Off=%d\n", ui_cur);
		break;
	case POST_VIDEO_COLOR_RED_Y:	/*RED_LUMA */
		for (qty_item = QUALITY_COLOR_R_Y_BEGIN; qty_item <= QUALITY_COLOR_R_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scered_y4(read_qty_table(QUALITY_R_Y04));
		pp_hal_set_scered_y5(read_qty_table(QUALITY_R_Y05));
		break;
	case POST_VIDEO_COLOR_RED_S:	/*RED_SAT */
		for (qty_item = QUALITY_COLOR_R_S_BEGIN; qty_item <= QUALITY_COLOR_R_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scered_s4(read_qty_table(QUALITY_R_S04));
		pp_hal_set_scered_s5(read_qty_table(QUALITY_R_S05));
		break;
	case POST_VIDEO_COLOR_RED_H:	/*RED_HUE */
		for (qty_item = QUALITY_COLOR_R_H_BEGIN; qty_item <= QUALITY_COLOR_R_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scered_h4(read_qty_table(QUALITY_R_H04));
		pp_hal_set_scered_h5(read_qty_table(QUALITY_R_H05));
		break;
	case POST_VIDEO_COLOR_GREEN_Y:
		for (qty_item = QUALITY_COLOR_G_Y_BEGIN; qty_item <= QUALITY_COLOR_G_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scegreen_y10(read_qty_table(QUALITY_G_Y10));
		pp_hal_set_scegreen_y11(read_qty_table(QUALITY_G_Y11));
		break;
	case POST_VIDEO_COLOR_GREEN_S:
		for (qty_item = QUALITY_COLOR_G_S_BEGIN; qty_item <= QUALITY_COLOR_G_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scegreen_s10(read_qty_table(QUALITY_G_S10));
		pp_hal_set_scegreen_s11(read_qty_table(QUALITY_G_S11));
		break;
	case POST_VIDEO_COLOR_GREEN_H:
		for (qty_item = QUALITY_COLOR_G_H_BEGIN; qty_item <= QUALITY_COLOR_G_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scegreen_h10(read_qty_table(QUALITY_G_H10));
		pp_hal_set_scegreen_h11(read_qty_table(QUALITY_G_H11));
		break;
	case POST_VIDEO_COLOR_BLUE_Y:
		for (qty_item = QUALITY_COLOR_B_Y_BEGIN; qty_item <= QUALITY_COLOR_B_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceblue_y14(read_qty_table(QUALITY_B_Y14));
		pp_hal_set_sceblue_y15(read_qty_table(QUALITY_B_Y15));
		break;
	case POST_VIDEO_COLOR_BLUE_S:
		for (qty_item = QUALITY_COLOR_B_S_BEGIN; qty_item <= QUALITY_COLOR_B_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceblue_s14(read_qty_table(QUALITY_B_S14));
		pp_hal_set_sceblue_s15(read_qty_table(QUALITY_B_S15));
		break;
	case POST_VIDEO_COLOR_BLUE_H:
		for (qty_item = QUALITY_COLOR_B_H_BEGIN; qty_item <= QUALITY_COLOR_B_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceblue_h14(read_qty_table(QUALITY_B_H14));
		pp_hal_set_sceblue_H15(read_qty_table(QUALITY_B_H15));
		break;
	case POST_VIDEO_COLOR_YELLOW_Y:
		for (qty_item = QUALITY_COLOR_Y_Y_BEGIN; qty_item <= QUALITY_COLOR_Y_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceyellow_y7(read_qty_table(QUALITY_Y_Y07));
		pp_hal_set_sceyellow_y8(read_qty_table(QUALITY_Y_Y08));
		break;
	case POST_VIDEO_COLOR_YELLOW_S:
		for (qty_item = QUALITY_COLOR_Y_S_BEGIN; qty_item <= QUALITY_COLOR_Y_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceyellow_s7(read_qty_table(QUALITY_Y_S07));
		pp_hal_set_sceyellow_s8(read_qty_table(QUALITY_Y_S08));
		break;
	case POST_VIDEO_COLOR_YELLOW_H:
		for (qty_item = QUALITY_COLOR_Y_H_BEGIN; qty_item <= QUALITY_COLOR_Y_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_sceyellow_h7(read_qty_table(QUALITY_Y_H07));
		pp_hal_set_sceyellow_h8(read_qty_table(QUALITY_Y_H08));
		break;
	case POST_VIDEO_COLOR_CYAN_Y:
		for (qty_item = QUALITY_COLOR_C_Y_BEGIN; qty_item <= QUALITY_COLOR_C_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scecyan_y12(read_qty_table(QUALITY_C_Y12));
		pp_hal_set_scecyan_y13(read_qty_table(QUALITY_C_Y13));
		break;
	case POST_VIDEO_COLOR_CYAN_S:
		for (qty_item = QUALITY_COLOR_C_S_BEGIN; qty_item <= QUALITY_COLOR_C_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scecyan_s12(read_qty_table(QUALITY_C_S12));
		pp_hal_set_scecyan_s13(read_qty_table(QUALITY_C_S13));
		break;
	case POST_VIDEO_COLOR_CYAN_H:
		for (qty_item = QUALITY_COLOR_C_H_BEGIN; qty_item <= QUALITY_COLOR_C_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scecyan_h12(read_qty_table(QUALITY_C_H12));
		pp_hal_set_scecyan_h13(read_qty_table(QUALITY_C_H13));
		break;
	case POST_VIDEO_COLOR_MAGENTA_Y:
		for (qty_item = QUALITY_COLOR_M_Y_BEGIN; qty_item <= QUALITY_COLOR_M_Y_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scemagenta_y2(read_qty_table(QUALITY_M_Y02));
		pp_hal_set_scemagenta_y3(read_qty_table(QUALITY_M_Y03));
		break;
	case POST_VIDEO_COLOR_MAGENTA_S:
		for (qty_item = QUALITY_COLOR_M_S_BEGIN; qty_item <= QUALITY_COLOR_M_S_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scemagenta_s2(read_qty_table(QUALITY_M_S02));
		pp_hal_set_scemagenta_s3(read_qty_table(QUALITY_M_S03));
		break;
	case POST_VIDEO_COLOR_MAGENTA_H:
		for (qty_item = QUALITY_COLOR_M_H_BEGIN; qty_item <= QUALITY_COLOR_M_H_END;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, ui_min, ui_max, ui_dft, ui_cur));
		}
		pp_hal_set_scemagenta_h2(read_qty_table(QUALITY_M_H02));
		pp_hal_set_scemagenta_h3(read_qty_table(QUALITY_M_H03));
		break;
	default:
		return PP_FAIL;
	}
	return PP_OK;
}


/*******************************************************
*Function: vPostChgRes(HDMI_VIDEO_RESOLUTION e_res)
*Description: Post Process or change resolution
*Parameter: e_res
*Return: none
******************************************************************************/
static int disp_pp_chg_res(const struct disp_hw_resolution *info)
{
	pp_hal_chg_res(info->res_mode);
	return PP_OK;
}

/******************************************************************************
*Function: disp_pp_request_vdp_src_region(uint32_t src_width, uint32_t output_width)
*Description: PostProcess or request VDP SourceRegion
*Parameter: src_width, output_width
*Return: none
******************************************************************************/
void disp_pp_request_vdp_src_region(uint32_t src_width, uint32_t output_width)
{
	pp_hal_src_region(src_width, output_width);
}

/******************************************************************************
*Function: disp_pp_chg_input_cs(DISP_VIDEO_TYPE e_cs)
*Description: PostProcess or request PMX colorspace
*Parameter: fgIsHD
*Return: none
******************************************************************************/
void disp_pp_chg_input_cs(enum DISP_VIDEO_TYPE e_cs)
{
	pp_hal_chg_input_cs((uint8_t) e_cs);
}

/******************************************************************************
*Function: disp_pp_set_mode(uint8_t mode)
*Description: Set PostProcess or Standard mode(bypasscolorPengine)
*Parameter: b_on_off
*Return: none
******************************************************************************/
void disp_pp_set_mode(uint8_t mode)
{
	pp_hal_set_video_mode((enum POST_VIDEO_MODE_ENUM)mode);
	PP_LOG("Set SCE video mode table=%d\n", mode);
}

/*for smooth fast search*/
#define DSTEP 1
#define SCALER 10000
#define LSCALER (100000000)
#define DELTA 10000
#define DSCALER 128
#define ASCALER 255
#define GAIN_COUNTER_THRESHOD 60

/*------------------------------------------------*/


/*Initial Alpha Max/Min_gain*/
uint16_t cur_max = SCALER;
uint16_t pre_max_2 = SCALER;
uint16_t pre_max_1 = SCALER;
uint16_t cur_min = SCALER;
uint16_t pre_min_2 = SCALER;
uint16_t pre_min_1 = SCALER;

/*Initial Y/CbCr_gain*/
uint16_t cur_y = SCALER;
uint16_t pre_y_2 = SCALER;
uint16_t pre_y_1 = SCALER;
uint16_t cur_cbcr = SCALER;
uint16_t pre_cbcr_2 = SCALER;
uint16_t pre_cbcr_1 = SCALER;
uint32_t pmode_cur_max = LSCALER;
uint32_t pmode_pre_max_2 = LSCALER;
uint32_t pmode_pre_max_1 = LSCALER;
uint32_t pmode_cur_min = LSCALER;
uint32_t pmode_pre_min_2 = LSCALER;
uint32_t pmode_pre_min_1 = LSCALER;

/*Initial Y/CbCr_gain*/
uint32_t pmode_cur_y = LSCALER;
uint32_t pmode_pre_y_2 = LSCALER;
uint32_t pmode_pre_y_1 = LSCALER;
uint32_t pmode_cur_cbcr = LSCALER;
uint32_t pmode_pre_cbcr_2 = LSCALER;
uint32_t pmode_pre_cbcr_1 = LSCALER;


/*----------------------*/
uint32_t rec_mode;
uint32_t rec_alpha_max = 0xff;
uint32_t rec_alpha_min = 0xff;
uint32_t rec_y_gain = 0x80;
uint32_t rec_cbcr_gain = 0x80;
uint32_t gain_same_couner;
uint32_t same_gain_return;

/*----------------------*/
static uint16_t _pp_calculate_next_value(uint16_t tar, uint16_t v_2, uint16_t v_1, uint16_t b)
{
	uint32_t cur = 0;

	cur = v_1 * SCALER - (v_2 - v_1) * b;
	PP_LOG("_pp_calculate_next_valuecur=%3d, v_1=%3d, b=%3d\n", cur, v_1, b);
	cur = cur / SCALER;
	if (v_1 > tar && (uint16_t) cur <= tar)
		cur = tar;
	/*
	*else {
	*v_2=v_1;
	*v_1=cur;
	*}
	*/
	PP_LOG("_pp_calculate_next_valuecur=%3d, v_1=%3d, tar=%3d\n", cur, v_1, tar);
	return (uint16_t) cur;
}


/*Initial Y/CbCr_gain*/
uint32_t sfs_cur_max = 0x2710;
uint32_t sfs_pre_max_2 = 0x2710;
uint32_t sfs_pre_max_1 = 0x2710;
uint32_t sfs_cur_min = 0x2710;
uint32_t sfs_pre_min_2 = 0x2710;
uint32_t sfs_pre_min_1 = 0x2710;
/*Initial Y/CbCr_gain*/
uint16_t pmode_sfs_cur_max = 1;
uint16_t pmode_sfs_pre_max_2 = 1;
uint16_t pmode_sfs_pre_max_1 = 1;
uint16_t pmode_sfs_cur_min = 1;
uint16_t pmode_sfs_pre_min_2 = 1;
uint16_t pmode_sfs_pre_min_1 = 1;
bool _fgPmode_SFS_maxTar;
bool _fgPmode_SFS_minTar;
/*----------------------*/
uint32_t sfs_max = 0x80;
uint32_t sfs_min = 0x80;


/*Set fast smooth search y gain value and cbcr gain value*/

/*Initial Y/CbCr_gain*/
uint32_t sfs_cur_y = 0x2710;
uint32_t pre_sfs_y_2 = 0x2710;
uint32_t pre_sfs_y_1 = 0x2710;
uint32_t sfs_cur_cbcr = 0x2710;
uint32_t sfs_pre_cbcr_2 = 0x2710;
uint32_t sfs_pre_cbcr_1 = 0x2710;
/*Initial Y/CbCr_gain*/
uint16_t pmode_sfs_cur_y = 1;
uint16_t pmode_sfs_pre_y_2 = 1;
uint16_t pmode_sfs_pre_y_1 = 1;
uint16_t pmode_sfs_cur_cbcr = 1;
uint16_t pmode_sfs_pre_cbcr_2 = 1;
uint16_t pmode_sfs_pre_cbcr_1 = 1;
bool _fg_pmode_sfs_y_tar;
bool _fg_pmode_sfs_cbcr_tar;
/*----------------------*/
uint32_t sfs_y = 0x80;
uint32_t sfs_cbcr = 0x80;
/*----------------------*/
void disp_pp_set_sfs_curve(uint32_t mode, uint32_t frame, uint32_t syncounter,
			   uint32_t y_b, uint32_t y_tar, uint32_t y_d,
			   uint32_t cbcr_b, uint32_t cbcr_tar, uint32_t cbcr_d)
{
	uint32_t b_y, tar_y;
	uint32_t b_cbcr, tar_cbcr;
	uint32_t temp;
	/*get y gain value and b value*/
	tar_y = y_tar;
	b_y = y_b;
	/*get cb cr gain value and b value*/
	tar_cbcr = cbcr_tar;
	b_cbcr = cbcr_b;
	switch (mode) {
	case FF1_FR1_MODE:
	case FF2_FR2_MODE:
	case FF3_FR3_MODE:
		if (syncounter == 1 && mode == FF1_FR1_MODE) {
			temp = (u4RegRd4B(G_PIC_ADJ_MAIN) & 0x0000ff00) >> 8;
			sfs_cur_y = (((temp == 0) ? 0x80 : (0x100 - temp)) * SCALER) >> 7;
			sfs_cur_cbcr =
			    (((u4RegRd4B(G_PIC_ADJ_MAIN) & 0x00ff0000) >> 16) * SCALER) >> 7;
		}
		pp_hal_color_proc_bypass(false);
		if (syncounter == 1) {

	/*Initial Y/CbCr_gain at first time*/
			pre_sfs_y_2 = sfs_cur_y;
			pre_sfs_y_1 = sfs_cur_y;
			sfs_pre_cbcr_2 = sfs_cur_cbcr;
			sfs_pre_cbcr_1 = sfs_cur_cbcr;
		}

		else if (syncounter == 2) {

	/*Calc Y/CbCr_gain at next sync*/
			if (pre_sfs_y_2 > tar_y) {
				pre_sfs_y_1 = pre_sfs_y_2 - y_d;	/*DSTEP;  */
				sfs_cur_y = (pre_sfs_y_1 > tar_y) ? pre_sfs_y_1 : tar_y;
			}
			if (sfs_pre_cbcr_2 > tar_cbcr) {
				sfs_pre_cbcr_1 = sfs_pre_cbcr_2 - cbcr_d;	/*DSTEP;  */
				sfs_cur_cbcr =
				    (sfs_pre_cbcr_1 > tar_cbcr) ? sfs_pre_cbcr_1 : tar_cbcr;
			}
		}

		else {

	/*Calc next y/cbcr gain*/
			sfs_cur_y = _pp_calculate_next_value(tar_y, pre_sfs_y_2, pre_sfs_y_1, b_y);
			pre_sfs_y_2 = pre_sfs_y_1;
			pre_sfs_y_1 = sfs_cur_y;
			if (sfs_cur_y <= tar_y)
				pre_sfs_y_2 = pre_sfs_y_1;
			sfs_cur_cbcr =
			    _pp_calculate_next_value(tar_cbcr, sfs_pre_cbcr_2, sfs_pre_cbcr_1,
						     b_cbcr);
			sfs_pre_cbcr_2 = sfs_pre_cbcr_1;
			sfs_pre_cbcr_1 = sfs_cur_cbcr;
			if (sfs_cur_cbcr <= tar_cbcr)
				sfs_pre_cbcr_2 = sfs_pre_cbcr_1;
		}
		break;
	case PLAY_MODE:
	default:
		if (syncounter == 1) {

	/*Initial Y/CbCr_gain at first time*/
			pmode_sfs_pre_y_2 = (uint16_t) sfs_cur_y / SCALER;
			pmode_sfs_pre_y_1 = (uint16_t) sfs_cur_y / SCALER;
			pmode_sfs_cur_y = (uint16_t) sfs_cur_y / SCALER;
			pmode_sfs_pre_cbcr_2 = (uint16_t) sfs_cur_cbcr / SCALER;
			pmode_sfs_pre_cbcr_1 = (uint16_t) sfs_cur_cbcr / SCALER;
			pmode_sfs_cur_cbcr = (uint16_t) sfs_cur_cbcr / SCALER;
		}

		else if (syncounter == 2) {

	/*Calc Y/CbCr_gain at next sync*/
			if (pmode_sfs_pre_y_2 < (uint16_t) tar_y / SCALER) {
				pmode_sfs_pre_y_1 = pmode_sfs_pre_y_2 + ((uint16_t) y_d / SCALER);
				pmode_sfs_cur_y =
				    pmode_sfs_pre_y_1 < (uint16_t) tar_y / SCALER ?
				    pmode_sfs_pre_y_1 : (uint16_t) tar_y / SCALER;
				sfs_cur_y = (uint32_t) (pmode_sfs_cur_y * SCALER);
			}
			if (pmode_sfs_pre_cbcr_2 < (uint16_t) tar_cbcr / SCALER) {
				pmode_sfs_pre_cbcr_1 =
				    pmode_sfs_pre_cbcr_2 + ((uint16_t) cbcr_d / SCALER);
				pmode_sfs_cur_cbcr =
				    pmode_sfs_pre_cbcr_1 <
				    (uint16_t) tar_cbcr /
				    SCALER ? pmode_sfs_pre_cbcr_1 : (uint16_t) tar_cbcr / SCALER;
				sfs_cur_cbcr = (uint32_t) (pmode_sfs_cur_cbcr * SCALER);
			}
		}

		else {

	/*Y gain*/
			pmode_sfs_cur_y = pmode_sfs_pre_y_1 +
			    (pmode_sfs_pre_y_1 - pmode_sfs_pre_y_2) * ((uint16_t) b_y / SCALER);
			if (pmode_sfs_cur_y > (uint16_t) tar_y / SCALER
			    && pmode_sfs_pre_y_1 < (uint16_t) tar_y / SCALER) {
				pmode_sfs_cur_y = (uint16_t) tar_y / SCALER;
				sfs_cur_y = tar_y;
				_fg_pmode_sfs_y_tar = 1;
			}

			else {
				sfs_cur_y = (uint32_t) (pmode_sfs_cur_y * SCALER);
				_fg_pmode_sfs_y_tar = 0;
			}
			pmode_sfs_pre_y_2 = pmode_sfs_pre_y_1;
			pmode_sfs_pre_y_1 = pmode_sfs_cur_y;
			if (_fg_pmode_sfs_y_tar)
				pmode_sfs_pre_y_2 = pmode_sfs_pre_y_1;
	/*CbCr_gain*/
			pmode_sfs_cur_cbcr = pmode_sfs_pre_cbcr_1 +
			    (pmode_sfs_pre_cbcr_1 -
			     pmode_sfs_pre_cbcr_2) * ((uint16_t) b_cbcr / SCALER);
			if (pmode_sfs_cur_cbcr > (uint16_t) tar_cbcr / SCALER
			    && pmode_sfs_pre_cbcr_1 < (uint16_t) tar_cbcr / SCALER) {
				pmode_sfs_cur_cbcr = (uint16_t) tar_cbcr / SCALER;
				sfs_cur_cbcr = tar_cbcr;
				_fg_pmode_sfs_cbcr_tar = 1;
			}

			else {
				sfs_cur_cbcr = (uint32_t) (pmode_sfs_cur_cbcr * SCALER);
				_fg_pmode_sfs_cbcr_tar = 0;
			}
			pmode_sfs_pre_cbcr_2 = pmode_sfs_pre_cbcr_1;
			pmode_sfs_pre_cbcr_1 = pmode_sfs_cur_cbcr;
			if (_fg_pmode_sfs_cbcr_tar)
				pmode_sfs_pre_cbcr_2 = pmode_sfs_pre_cbcr_1;
		}
		break;
	}
	if ((mode == FF1_FR1_MODE) || (mode == FF2_FR2_MODE) || (mode == FF3_FR3_MODE)) {
		sfs_y = ((sfs_cur_y << 7) / SCALER) & 0xff;
		sfs_cbcr = ((sfs_cur_cbcr << 7) / SCALER) & 0xff;
	}

	else {
		sfs_y = (uint32_t) (pmode_sfs_cur_y * DSCALER) & 0xff;
		sfs_cbcr = (uint32_t) (pmode_sfs_cur_cbcr * DSCALER) & 0xff;
	}
	PP_INFO("mode=%d, syncounter=%3d, Y__gain=%3d, CbCr__gain=%3d\n", mode,
		syncounter, sfs_y, sfs_cbcr);
	/*write Y_gain&CbCr_gain to postproc register*/
	pp_hal_update_main_bri(sfs_y);
	pp_hal_update_main_sat(sfs_cbcr);
	rec_mode = mode;
}

void dis_pp_set_sfs_curve_for_gain(uint32_t mode, uint32_t syncounter,
				   uint32_t y_b, uint32_t y_tar, uint32_t y_d,
				   uint32_t cbcr_b, uint32_t cbcr_tar, uint32_t cbcr_d)
{
	uint32_t frame;

	if (pp_is_video_60hz(src_res))
		frame = FRAM_RATE_60HZ;
	else if (pp_is_video_50hz(src_res))
		frame = FRAM_RATE_50HZ;
	else if (pp_is_video_24hz(src_res))
		frame = FRAM_RATE_24HZ;
	else
		frame = FRAM_RATE_60HZ;

	disp_pp_set_sfs_curve(mode, frame, syncounter, y_b, y_tar, y_d, cbcr_b, cbcr_tar, cbcr_d);
}

/******************************************************************************
*Function: disp_pp_init
*Description: Drv initial PostProcessor
*Parameter: None
*Return: None
******************************************************************************/
static int disp_pp_init(struct disp_hw_common_info *info)
{
	PP_FUNC();
	if (pp_hal_init() == PP_FAIL)
		return PP_FAIL;
	if (pp_drv_init() == PP_FAIL)
		return PP_FAIL;
	disp_pp_chg_res(info->resolution);
	return PP_OK;
}


/******************************************************************************
*Function: i4Post_UnInit
*Description: Drv Uninitial PostProcessor
*Parameter: None
*Return: None
******************************************************************************/
static int disp_pp_deinit(void)
{
	if (pp_drv_uninit() == PP_FAIL)
		return PP_FAIL;
	return PP_OK;
}

static int disp_pp_suspend(void)
{
	PP_FUNC();
	pp_enable(false);
	/*pp_hal_sharp_enable(SV_OFF);*/
	return PP_OK;
}

static int disp_pp_resume(void)
{
	PP_FUNC();
	pp_enable(true);
	/*pp_hal_sharp_enable(SV_ON);*/
	return PP_OK;
}


/*****************driver************/
struct disp_hw disp_pp_driver = {
	.name = PP_DRV_NAME,
	.init = disp_pp_init,
	.deinit = disp_pp_deinit,
	.start = NULL,
	.stop = NULL,
	.suspend = disp_pp_suspend,
	.resume = disp_pp_resume,
	.get_info = NULL,
	.change_resolution = NULL,
	.config = NULL,
	.irq_handler = NULL,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = NULL,
};

struct disp_hw *disp_pp_get_drv(void)
{
	return NULL;
}
