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


#ifndef _PP_HAL_H_
#define _PP_HAL_H_

#include "disp_reg.h"
#include "disp_type.h"
#include "disp_pp_if.h"
#include "disp_hw_mgr.h"


#define PP_ERR(fmt, arg...) pr_err("[PP]error:"fmt, ##arg)
#define PP_WARN(fmt, arg...) pr_warn("[PP]:"fmt, ##arg)
#define PP_INFO(fmt, arg...) pr_info("[PP]:"fmt, ##arg)
#define PP_LOG(fmt, arg...) pr_debug("[PP]:"fmt, ##arg)
#define PP_DEBUG(fmt, arg...) pr_debug("[PP]:"fmt, ##arg)
#define PP_FUNC() pr_debug("[PP]func:%s, line:%d\n", __func__, __LINE__)

enum PP_COLOR_PROCESSOR_ENUM {
	Y_SLOPE_MAIN = 0,
	S_G1_MAIN,
	S_G2_MAIN,
	S_G3_MAIN,
	S_P1_MAIN,
	S_P2_MAIN,
	S_Y0_G_MAIN,
	S_Y64_G_MAIN,
	S_Y128_G_MAIN,
	S_Y192_G_MAIN,
	S_Y256_G_MAIN,
	H_FTN_MAIN
};

enum PP_QUALITY_ITEM_ENUM {
	QUALITY_TDSHARP_GAIN1 = 0,
	QUALITY_TDSHARP_GAIN2,
	QUALITY_TDSHARP_GAIN3,
	QUALITY_TDSHARP_GAIN4,
	QUALITY_TDSHARP_GAIN5,
	QUALITY_TDSHARP_GAIN6,

	QUALITY_TDSHARP_GAIN_NEG1,
	QUALITY_TDSHARP_GAIN_NEG2,
	QUALITY_TDSHARP_GAIN_NEG3,
	QUALITY_TDSHARP_GAIN_NEG4,
	QUALITY_TDSHARP_GAIN_NEG5,
	QUALITY_TDSHARP_GAIN_NEG6,

	QUALITY_TDSHARP_CORING1,
	QUALITY_TDSHARP_CORING2,
	QUALITY_TDSHARP_CORING3,
	QUALITY_TDSHARP_CORING4,
	QUALITY_TDSHARP_CORING5,
	QUALITY_TDSHARP_CORING6,

	QUALITY_TDSHARP_LIMIT_POS_ALL,
	QUALITY_TDSHARP_LIMIT_NEG_ALL,

	QUALITY_TDSHARP_LIMIT_POS1,
	QUALITY_TDSHARP_LIMIT_POS2,
	QUALITY_TDSHARP_LIMIT_POS3,
	QUALITY_TDSHARP_LIMIT_POS4,
	QUALITY_TDSHARP_LIMIT_POS5,
	QUALITY_TDSHARP_LIMIT_POS6,

	QUALITY_TDSHARP_LIMIT_NEG1,
	QUALITY_TDSHARP_LIMIT_NEG2,
	QUALITY_TDSHARP_LIMIT_NEG3,
	QUALITY_TDSHARP_LIMIT_NEG4,
	QUALITY_TDSHARP_LIMIT_NEG5,
	QUALITY_TDSHARP_LIMIT_NEG6,

	QUALITY_TDSHARP_CLIP_EN1,
	QUALITY_TDSHARP_CLIP_EN2,
	QUALITY_TDSHARP_CLIP_EN3,
	QUALITY_TDSHARP_CLIP_EN4,
	QUALITY_TDSHARP_CLIP_EN5,
	QUALITY_TDSHARP_CLIP_EN6,

	QUALITY_TDSHARP_CLIP_THPOS1,
	QUALITY_TDSHARP_CLIP_THNEG1,
	QUALITY_TDSHARP_CLIP_THPOS2,
	QUALITY_TDSHARP_CLIP_THNEG2,
	QUALITY_TDSHARP_CLIP_THPOS3,
	QUALITY_TDSHARP_CLIP_THNEG3,
	QUALITY_TDSHARP_CLIP_THPOS4,
	QUALITY_TDSHARP_CLIP_THNEG4,
	QUALITY_TDSHARP_CLIP_THPOS5,
	QUALITY_TDSHARP_CLIP_THNEG5,
	QUALITY_TDSHARP_CLIP_THPOS6,
	QUALITY_TDSHARP_CLIP_THNEG6,

	QUALITY_TDSHARP_SOFT_CLIP_GAIN1,
	QUALITY_TDSHARP_SOFT_CLIP_GAIN2,
	QUALITY_TDSHARP_SOFT_CLIP_GAIN3,
	QUALITY_TDSHARP_SOFT_CLIP_GAIN4,
	QUALITY_TDSHARP_SOFT_CLIP_GAIN5,
	QUALITY_TDSHARP_SOFT_CLIP_GAIN6,

	QUALITY_H1_1_SOFT_CLIP_GAIN,
	QUALITY_H2_1_SOFT_CLIP_GAIN,

	QUALITY_H1_1_GAIN,
	QUALITY_H2_1_GAIN,
	QUALITY_H1_1_GAIN_NEG,
	QUALITY_H2_1_GAIN_NEG,
	QUALITY_H1_1_CORING,
	QUALITY_H2_1_CORING,
	QUALITY_H1_1_LIMIT_POS,
	QUALITY_H2_1_LIMIT_POS,
	QUALITY_H1_1_LIMIT_NEG,
	QUALITY_H2_1_LIMIT_NEG,
	QUALITY_H1_1_CLIP_EN,
	QUALITY_H2_1_CLIP_EN,
	QUALITY_H1_1_CLIP_THPOS,
	QUALITY_H1_1_CLIP_THNEG,
	QUALITY_H2_1_CLIP_THPOS,
	QUALITY_H2_1_CLIP_THNEG,


	QUALITY_H1_SOFT_COR_GAIN,
	QUALITY_H2_SOFT_COR_GAIN,
	QUALITY_V1_SOFT_COR_GAIN,
	QUALITY_V2_SOFT_COR_GAIN,
	QUALITY_X1_SOFT_COR_GAIN,
	QUALITY_X2_SOFT_COR_GAIN,
	QUALITY_H1_1_SOFT_COR_GAIN,
	QUALITY_H2_1_SOFT_COR_GAIN,

    /*CDS*/
	QUALITY_CDS_GAIN_SIGN_1,	/*Index=82 */
	QUALITY_CDS_GAIN_1,
	QUALITY_CDS_CORING_1,
	QUALITY_CDS_UP_BOUND_1,
	QUALITY_CDS_LOW_BOUND_1,

	QUALITY_CDS_GAIN_SIGN_2,
	QUALITY_CDS_GAIN_2,
	QUALITY_CDS_CORING_2,
	QUALITY_CDS_UP_BOUND_2,
	QUALITY_CDS_LOW_BOUND_2,

    /*CTI*/
	QUALITY_ECTI_CLP_SZ,
	QUALITY_ECTI_VWGT,
	QUALITY_ECTI_UWGT,
	QUALITY_ECTI_LPF3_SEL,
	QUALITY_ECTI_LPF2_SEL,
	QUALITY_ECTI_LPF1_SEL,
	QUALITY_ECTI_FIX_SZ,
	QUALITY_ECTI_SGN_TH,
	QUALITY_ECTI_U_STB_OFST2,
	QUALITY_ECTI_U_STB_GAIN,
	QUALITY_ECTI_U_STB_OFST1,
	QUALITY_ECTI_U_WND_OFST,
	QUALITY_ECTI_U_WND_SZ,
	QUALITY_ECTI_V_STB_OFST2,
	QUALITY_ECTI_V_STB_GAIN,
	QUALITY_ECTI_V_STB_OFST1,
	QUALITY_ECTI_V_WND_OFST,
	QUALITY_ECTI_V_WND_SZ,
	QUALITY_ECTI_FLAT_OFST2,
	QUALITY_ECTI_FLAT_GAIN,
	QUALITY_ECTI_FLAT_OFST1,
	QUALITY_ECTI_FLAT_SZ,
	QUALITY_ECTI_COR,
	QUALITY_ECTI_V_LMT,
	QUALITY_ECTI_U_LMT,
/*
*QUALITY_ECTI_HMSK_END,
*QUALITY_ECTI_HMSK_START,
*QUALITY_ECTI_VMSK_END,
*QUALITY_ECTI_VMSK_START,
*QUALITY_ECTI_HDEMO_END,
*QUALITY_ECTI_HDEMO_START,
*QUALITY_ECTI_VDEMO_END,
*QUALITY_ECTI_VDEMO_START,
*/

	 /*ADAPTIVELUMA*/
	QUALITY_AL_GAIN,
	QUALITY_AL_OFFSET,
	QUALITY_AL_LIMIT,
	QUALITY_AL_METHOD,
	QUALITY_AL_SCENE_CHANGE_MAX_THD,
	QUALITY_AL_SCENE_CHANGE_TOTAL_THD,
	QUALITY_BWS_ON_1_OFF_0,
	QUALITY_BWS_BLACK_ON_1_OFF_0,
	QUALITY_BWS_WHITE_ON_1_OFF_0,
	QUALITY_BWS_BLACK_LEVEL,
	QUALITY_BWS_BLACK_GAIN,
	QUALITY_BWS_BLACK_OFFSET,
	QUALITY_BWS_BLACK_RATIO,
	QUALITY_BWS_BLACK_LIMIT,
	QUALITY_BWS_WHITE_LEVEL,
	QUALITY_BWS_WHITE_GAIN,
	QUALITY_BWS_WHITE_OFFSET,
	QUALITY_BWS_WHITE_RATIO,
	QUALITY_BWS_WHITE_LIMIT,

	QUALITY_CONTRAST,
	QUALITY_BRIGHTNESS,
	QUALITY_SATURATION,
	QUALITY_HUE,

	QUALITY_M_Y02,
	QUALITY_M_Y03,
	QUALITY_R_Y04,
	QUALITY_R_Y05,
	QUALITY_Y_Y07,
	QUALITY_Y_Y08,
	QUALITY_G_Y10,
	QUALITY_G_Y11,
	QUALITY_C_Y12,
	QUALITY_C_Y13,
	QUALITY_B_Y14,
	QUALITY_B_Y15,

	QUALITY_M_S02,
	QUALITY_M_S03,
	QUALITY_R_S04,
	QUALITY_R_S05,
	QUALITY_Y_S07,
	QUALITY_Y_S08,
	QUALITY_G_S10,
	QUALITY_G_S11,
	QUALITY_C_S12,
	QUALITY_C_S13,
	QUALITY_B_S14,
	QUALITY_B_S15,

	QUALITY_M_H02,
	QUALITY_M_H03,
	QUALITY_R_H04,
	QUALITY_R_H05,
	QUALITY_Y_H07,
	QUALITY_Y_H08,
	QUALITY_G_H10,
	QUALITY_G_H11,
	QUALITY_C_H12,
	QUALITY_C_H13,
	QUALITY_B_H14,
	QUALITY_B_H15,

	QUALITY_CONTRAST_STD,
	QUALITY_BRIGHTNESS_STD,
	QUALITY_SATURATION_STD,
	QUALITY_HUE_STD,

	QUALITY_CONTRAST_VIVID,
	QUALITY_BRIGHTNESS_VIVID,
	QUALITY_SATURATION_VIVID,
	QUALITY_HUE_VIVID,

	QUALITY_CONTRAST_CINEMA,
	QUALITY_BRIGHTNESS_CINEMA,
	QUALITY_SATURATION_CINEMA,
	QUALITY_HUE_CINEMA,

	QUALITY_PRE_SHARP_CORING_B1,	/*Index=188 */
	QUALITY_PRE_SHARP_LIMIT_NEG_B1,
	QUALITY_PRE_SHARP_LIMIT_POS_B1,
	QUALITY_PRE_SHARP_GAIN_B1,

	QUALITY_PRE_SHARP_CLIP_NEG_B1,
	QUALITY_PRE_SHARP_CLIP_POS_B1,
	QUALITY_PRE_SHARP_LIMIT_NEG,
	QUALITY_PRE_SHARP_LIMIT_POS,

	/*QUALITY_PRE_SHARP_EN_B1, */
	QUALITY_PRE_SHARP_SHRINK_SEL_B1,

	QUALITY_PRE_SHARP_CLIP_SEL_B1,
	QUALITY_PRE_SHARP_CLIP_EN_B1,
	/*QUALITY_PRE_SHARP_CLIP_NEG_B1, */
	/*QUALITY_PRE_SHARP_CLIP_POS_B1, */

	/*QUALITY_PRE_SHARP_BYPASS_B1, */
	QUALITY_PRE_SHARP_FILTER_SEL_B1,
	QUALITY_PRE_SHARP_SHIFT_B1,
	QUALITY_PRE_SHARP_PREC_B1,
	/*QUALITY_PRE_SHARP_LIMIT_NEG, */
	/*QUALITY_PRE_SHARP_LIMIT_POS, */

	QUALITY_MAX
};

/*For flash PQ architecture*/

#define MT8580_PARA_FROM_TV 0
struct pp_dft_qty {
	uint8_t qty_dft_min;
	uint8_t qty_dft_max;
	uint8_t qty_dft_dft;
	uint16_t qty_dft_refenence;
	enum PP_QUALITY_ITEM_ENUM qty_dft_item;

};

enum POST_DRV_PATH_ENUM {
	SV_MAIN = 0,
	SV_PIP,
	SV_MIXER,
	SV_BYPASS,

};

extern uint8_t pp_qty_tbl[QUALITY_MAX];

#define read_qty_table(addr) pp_qty_tbl[addr]
#define write_qty_table(addr, data) (pp_qty_tbl[addr] = data)


#define QUALITY_TDSHARP_BEGIN  QUALITY_TDSHARP_GAIN1
/*add for presharpness*/
#define QUALITY_TDSHARP_END QUALITY_H2_1_SOFT_COR_GAIN
#define QUALITY_PRE_SHARP_BEGIN  QUALITY_PRE_SHARP_CORING_B1
#define QUALITY_PRE_SHARP_END QUALITY_PRE_SHARP_PREC_B1


#define QUALITY_CDS_BEGIN  QUALITY_CDS_GAIN_SIGN_1
#define QUALITY_CDS_END QUALITY_CDS_LOW_BOUND_2

#define QUALITY_CTI_BEGIN  QUALITY_ECTI_CLP_SZ
#define QUALITY_CTI_END QUALITY_ECTI_U_LMT

#define QUALITY_ADAPTIVE_LUMA_BEGIN  QUALITY_AL_GAIN
#define QUALITY_ADAPTIVE_LUMA_END QUALITY_BWS_WHITE_LIMIT

#define QUALITY_COLOR_M_Y_BEGIN QUALITY_M_Y02
#define QUALITY_COLOR_M_Y_END QUALITY_M_Y03

#define QUALITY_COLOR_R_Y_BEGIN QUALITY_R_Y04
#define QUALITY_COLOR_R_Y_END QUALITY_R_Y05

#define QUALITY_COLOR_Y_Y_BEGIN QUALITY_Y_Y07
#define QUALITY_COLOR_Y_Y_END QUALITY_Y_Y08

#define QUALITY_COLOR_G_Y_BEGIN QUALITY_G_Y10
#define QUALITY_COLOR_G_Y_END QUALITY_G_Y11

#define QUALITY_COLOR_C_Y_BEGIN QUALITY_C_Y12
#define QUALITY_COLOR_C_Y_END QUALITY_C_Y13

#define QUALITY_COLOR_B_Y_BEGIN QUALITY_B_Y14
#define QUALITY_COLOR_B_Y_END QUALITY_B_Y15

#define QUALITY_COLOR_M_S_BEGIN QUALITY_M_S02
#define QUALITY_COLOR_M_S_END QUALITY_M_S03

#define QUALITY_COLOR_R_S_BEGIN QUALITY_R_S04
#define QUALITY_COLOR_R_S_END QUALITY_R_S05

#define QUALITY_COLOR_Y_S_BEGIN QUALITY_Y_S07
#define QUALITY_COLOR_Y_S_END QUALITY_Y_S08

#define QUALITY_COLOR_G_S_BEGIN QUALITY_G_S10
#define QUALITY_COLOR_G_S_END QUALITY_G_S11

#define QUALITY_COLOR_C_S_BEGIN QUALITY_C_S12
#define QUALITY_COLOR_C_S_END QUALITY_C_S13

#define QUALITY_COLOR_B_S_BEGIN QUALITY_B_S14
#define QUALITY_COLOR_B_S_END QUALITY_B_S15

#define QUALITY_COLOR_M_H_BEGIN QUALITY_M_H02
#define QUALITY_COLOR_M_H_END QUALITY_M_H03

#define QUALITY_COLOR_R_H_BEGIN QUALITY_R_H04
#define QUALITY_COLOR_R_H_END QUALITY_R_H05

#define QUALITY_COLOR_Y_H_BEGIN QUALITY_Y_H07
#define QUALITY_COLOR_Y_H_END QUALITY_Y_H08

#define QUALITY_COLOR_G_H_BEGIN QUALITY_G_H10
#define QUALITY_COLOR_G_H_END QUALITY_G_H11

#define QUALITY_COLOR_C_H_BEGIN QUALITY_C_H12
#define QUALITY_COLOR_C_H_END QUALITY_C_H13

#define QUALITY_COLOR_B_H_BEGIN QUALITY_B_H14
#define QUALITY_COLOR_B_H_END QUALITY_B_H15

extern uintptr_t pp_reg_base;
extern uint8_t sharp_ui_min, sharp_ui_max, sharp_dft, sharp_cur;
extern uint32_t src_res;

int pp_hal_init(void);
/*****************************************************************************************/
/*************************************CTI******************************************/
/*****************************************************************************************/
void pp_hal_cti_enable(uint8_t b_on_off);
void pp_hal_cti_update(void);

/*****************************************************************************************/
/************************************2DSHARP*******************************************/
/*****************************************************************************************/
void pp_hal_sharp_enable(uint8_t b_on_off);
void pp_hal_update_sharp(void);
void pp_hal_set_sharp_param(struct POST_SHN_BAND_PARA band_param);
void pp_hal_set_pre_sharp_param(struct POST_SHN_PRE_BAND1_PARA band_param);	/*addforpresharpness */
struct POST_SHN_PRE_BAND1_PARA pp_hal_get_pre_sharp_param(void);	/*addforpresharpness */
struct POST_SHN_BAND_PARA pp_get_sharp_param(enum POST_SHN_BAND_ENUM band);
void pp_hal_set_sharp_ctrl(struct POST_SHN_CTRL_PARA ctrl_param);
struct POST_SHN_CTRL_PARA pp_hal_get_sharp_ctrl(void);

/*****************************************************************************************/
/*************************************CDS******************************************/
/*****************************************************************************************/
void pp_hal_cds_enable(uint8_t b_on_off);
void pp_hal_cds_update(void);

/*****************************************************************************************/
/***********************************CONTRAST****************************************/
/*****************************************************************************************/
void pp_hal_update_main_con(uint8_t value);

/*****************************************************************************************/
/**********************************BRIGHTNESS***************************************/
/*****************************************************************************************/
void pp_hal_update_main_bri(uint8_t value);

/*****************************************************************************************/
/**********************************SATURATION***************************************/
/*****************************************************************************************/
void pp_hal_update_main_sat(uint8_t value);

/*****************************************************************************************/
/*************************************HUE*******************************************/
/*****************************************************************************************/
void pp_hal_update_main_hue(uint8_t value);

/*****************************************************************************************/
/*************************************SCE*******************************************/
/*****************************************************************************************/
void pp_hal_main_sec_on_off(uint8_t b_on_off);
void pp_hal_set_sce_table(enum POST_VIDEO_MODE_ENUM emode);
void pp_hal_sce_init(void);

/*****************************************************************************************/
/*************************************LTI&HLTI*******************************************/
/*****************************************************************************************/
void pp_hal_lti_init(HDMI_VIDEO_RESOLUTION e_res);

/*****************************************************************************************/
/*************************************HVBand*******************************************/
/*****************************************************************************************/
void pp_hal_hv_band_init(HDMI_VIDEO_RESOLUTION e_res);

/*****************************************************************************************/
/*************************************Adap*******************************************/
/*****************************************************************************************/
void pp_hal_adap_init(HDMI_VIDEO_RESOLUTION e_res);

/*****************************************************************************************/
/*************************************Misc********************************************/
/*****************************************************************************************/
void pp_hal_yc_proc_init(void);
/*#define POST_LC_YLEV*/

/*****************************************************************************************/
/*************************************AdaptiveLuma***********************************/
/*****************************************************************************************/

uint32_t pp_get_hist_cur(uint8_t addr);
void pp_set_hist_cur(uint8_t addr, uint32_t ddata);
uint32_t pp_get_hist_rec(uint8_t addr);
void pp_set_hist_rec(uint8_t addr, uint32_t ddata);
void pp_hal_boost_chroma(uint8_t b_on_off);

#define BLACK_STRETCH_ENABLE 1
#define WHITE_STRETCH_ENABLE 1

#define BLACK_STRETCH_LEVEL 5
#define BLACK_STRETCH_GAIN 0x40
#define BLACK_STRETCH_OFFSET 0x20
#define BLACK_STRETCH_RATIO 0x90
#define BLACK_STRETCH_LIMIT 0x10

#define WHITE_STRETCH_LEVEL 5
#define WHITE_STRETCH_GAIN 0x40
#define WHITE_STRETCH_OFFSET 0x10
#define WHITE_STRETCH_RATIO 0x68
#define WHITE_STRETCH_LIMIT 0x08

#define AL_GAIN 0x40
#define AL_OFFSET 0
#define AL_LIMIT 0xFF
#define MAX_DIFF_THRESHOLD 0x0A
#define TOTAL_DIFF_THRESHOLD 0x30
#define MAX_NOISE_THRESHOLD 5

#define LUMA_HIST_LEVEL 16
#define LUMA_HIST_NUM 32

#define Y_TFN_LEN 33

/*----Histogram------*/
void pp_get_hist_info(void);
uint8_t pp_get_hist(uint32_t *p_hist);
uint8_t pp_get_apl_value(void);
uint8_t pp_get_luma_max(void);
uint8_t pp_get_luma_min(void);

/*----Adaptive luma-----*/
void pp_auto_con_init(void);
void pp_hal_auto_con_enable(uint8_t b_on_off);
void pp_update_luma_curve(void);
void pp_auto_contrast(void);
void pp_freeze_auto_con(uint8_t freeze);
void pp_hal_set_con_dft(void);
void pp_set_luma_curve(void);
void pp_set_luma_sw_reg(uint16_t *luma_array);
void pp_hal_set_al_param(uint8_t item, uint8_t value);
void pp_hal_set_bs_param(uint8_t item, uint8_t value);
void pp_hal_set_ws_param(uint8_t item, uint8_t value);

/*----Adaptive SCE-----*/
void pp_hal_auto_sat(void);
void pp_hal_sat_hist_win(uint32_t win_x, uint32_t win_y);
void pp_hal_main_video_mode(uint8_t mode);

/*----Build luma curve-----*/
void pp_build_con_curve_dft(uint16_t *curve);
void pp_build_con_curve_dyn(const uint32_t *p_hist_cur, uint16_t *p_dync_curve);
void pp_build_con_curve_target(uint16_t *p_target_curve);
void pp_build_con_curve(const uint16_t *p_dync_curve, const uint16_t *p_target_curve);
uint8_t pp_get_normalized_hist(uint32_t *p_hist_cur);
void pp_detect_scene_changed(const uint32_t *p_hist_cur);
void pp_get_adaptive_gain(const uint32_t *p_hist_cur);
void pp_limit_con_param(void);
uint8_t pp_get_bs_gain(void);
uint8_t pp_get_ws_gain(void);

void pp_update_luma_curve(void);
void pp_contrast_proc(void);

uint8_t pp_get_norm_hist(uint32_t *p_hist);
void pp_hal_set_path(uint8_t path);

void pp_hal_color_proc_bypass(int16_t mode);
void pp_hal_set_sharp_lti_band_param(int16_t mode, int16_t _gain);
void pp_hal_set_sharp_band1_param(int8_t _gain, bool enable);


struct POST_CTI_CTRL_PARA pp_hal_get_ctir_param(void);
void pp_hal_set_ctir_param(struct POST_CTI_CTRL_PARA ret_param);
void pp_hal_chg_res(HDMI_VIDEO_RESOLUTION e_res);
void pp_hal_set_video_mode(enum POST_VIDEO_MODE_ENUM e_on_off);
void pp_hal_chg_input_cs(uint8_t cs);
void pp_hal_src_region(uint32_t src_width, uint32_t output_width);

uint16_t pp_search_qty_item(uint16_t qty_item);
uint16_t pp_qty_mapping(uint16_t qty_item, int16_t ui_min, int16_t ui_max,
			int16_t ui_dft, int16_t ui_cur);

void pp_hal_scene_chg_th(uint8_t change, uint8_t normal);

void pp_hal_set_scered_y4(uint8_t value);
void pp_hal_set_scered_y5(uint8_t value);
void pp_hal_set_scered_s4(uint8_t value);
void pp_hal_set_scered_s5(uint8_t value);
void pp_hal_set_scered_h4(uint8_t value);
void pp_hal_set_scered_h5(uint8_t value);

void pp_hal_set_scegreen_y10(uint8_t value);
void pp_hal_set_scegreen_y11(uint8_t value);
void pp_hal_set_scegreen_s10(uint8_t value);
void pp_hal_set_scegreen_s11(uint8_t value);
void pp_hal_set_scegreen_h10(uint8_t value);
void pp_hal_set_scegreen_h11(uint8_t value);

void pp_hal_set_sceblue_y14(uint8_t value);
void pp_hal_set_sceblue_y15(uint8_t value);
void pp_hal_set_sceblue_s14(uint8_t value);
void pp_hal_set_sceblue_s15(uint8_t value);
void pp_hal_set_sceblue_h14(uint8_t value);
void pp_hal_set_sceblue_H15(uint8_t value);

void pp_hal_set_sceyellow_y7(uint8_t value);
void pp_hal_set_sceyellow_y8(uint8_t value);
void pp_hal_set_sceyellow_s7(uint8_t value);
void pp_hal_set_sceyellow_s8(uint8_t value);
void pp_hal_set_sceyellow_h7(uint8_t value);
void pp_hal_set_sceyellow_h8(uint8_t value);

void pp_hal_set_scecyan_y12(uint8_t value);
void pp_hal_set_scecyan_y13(uint8_t value);
void pp_hal_set_scecyan_s12(uint8_t value);
void pp_hal_set_scecyan_s13(uint8_t value);
void pp_hal_set_scecyan_h12(uint8_t value);
void pp_hal_set_scecyan_h13(uint8_t value);

void pp_hal_set_scemagenta_y2(uint8_t value);
void pp_hal_set_scemagenta_y3(uint8_t value);
void pp_hal_set_scemagenta_s2(uint8_t value);
void pp_hal_set_scemagenta_s3(uint8_t value);
void pp_hal_set_scemagenta_h2(uint8_t value);
void pp_hal_set_scemagenta_h3(uint8_t value);

#endif
