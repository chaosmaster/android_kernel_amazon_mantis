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

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "pp_hw.h"
#include "pp_hal.h"
#include "pp_drv.h"
#include "pp_qty_tbl.h"

/*compress dark(<16) & light(>240) part, extend16~240*/
#define NEW_LUMA_ALGORITHM

#define LUMA_INITIAL		0
#define LUMA_ADADTIVE_AL1	1	/*keep in algorithm1 */
#define LUMA_HYS_TO_AL2		2	/*2~3 */
#define LUMA_RESET_AL2		4

#define LUMA_ADADTIVE_AL2	5	/*keep in algorithm2 */
#define LUMA_HYS_TO_AL1		6	/*6~7 */
#define LUMA_RESET_AL1		8

#define LUMA_CHANGE_CNT		15

#define TDSRP_BASE_GAIN		0x05

#define PATH_MSK	(VDO2_TO_FMT_SEL|VDO1_TO_FMT_SEL|MIXER_TO_POST_RND|\
					MIXER_TO_POST_SEL|DISP2_TO_FMT_SEL|DISP1_TO_FMT_SEL)

#define MAIN_PATH	((1<<6)|(2<<2))
#define PIP_PATH	((1<<7)|(3<<1))
#define MIX_PATH	((1<<7)|(1<<6)|(1<<5))
#define BYPASS_ALL	0


/*Adaptive Luma*/
#define AL_LOW_START	8
#define AL_LOW_END		32
#define AL_HIGH_START	52
#define AL_HIGH_END		32
#define AL_LOW_BIN		13
#define AL_HIGH_BIN		9
#define BW_STRETCH_TH	0x60
#define B_STRETCH_TH	0x40
#define W_STRETCH_TH	0x40
#define MAX_HIST_TH		0x1E0

#define AbsDiff(a, b)	(((a) > (b))?((a)-(b)):((b)-(a)))

#define TDSHARP_ON_STATUS		(0x1<<0)
#define TDSHARP_OFF_STATUS		(0x0<<0)
#define CTI_ON_STATUS			(0x1<<1)
#define CTI_OFF_STATUS			(0x0<<1)
#define CDS_ON_STATUS			(0x1<<2)
#define CDS_OFF_STATUS			(0x0<<2)
#define SCE_ON_STATUS			(0x1<<3)
#define SCE_OFF_STATUS			(0x0<<3)
#define ADAPTIVE_LUMA_ON_STATUS (0x1<<4)
#define ADAPTIVE_LUMA_OFF_STATUS (0x0<<4)
#define LTI_ON_STATUS			(0x1<<5)
#define LTI_OFF_STATUS			(0x0<<5)
#define HV_ON_STATUS			(0x1<<6)
#define HV_OFF_STATUS			(0x0<<6)
#define HLTI_ON_STATUS			(0x1<<7)
#define HLTI_OFF_STATUS			(0x0<<7)
#define ADAP_SHP_ON_STATUS		(0x1<<8)
#define ADAP_SHP_OFF_STATUS		(0x0<<8)
#define CP_ON_STATUS			(0x1<<9)
#define CP_OFF_STATUS			(0x1<<9)

#define FIELD_GLITCH	3
/******************************************************************************
*Local variable
******************************************************************************/
static volatile union HAL_POST_LUMA_MAIN_UNION_T *_luma_main_hw_reg;
union HAL_POST_LUMA_MAIN_UNION_T _luma_main_sw_reg;

uint32_t _hist_cur[32];
uint32_t _hist_rec[32];

#ifdef NEW_LUMA_ALGORITHM
uint8_t _aps;
uint8_t _sat_gain;
#endif
uint8_t _apl;
uint8_t _adap_luma_gain;
uint8_t _adap_luma_offset;
uint8_t _adap_luma_limit;
uint8_t _max_diff_threshold;
uint8_t _total_diff_threshold;
uint8_t _bws_on_off;
uint8_t _bs_on_off;
uint8_t _bs_level;
uint8_t _bs_gain;
uint8_t _bs_offset;
uint8_t _bs_ratio;
uint8_t _bs_limit;
uint8_t _ws_on_off;
uint8_t _ws_level;
uint8_t _ws_gain;
uint8_t _ws_offset;
uint8_t _ws_ratio;
uint8_t _ws_limit;

uint8_t _adaptive_on;
uint8_t _compute_done;
uint8_t _curve_freeze;
uint8_t _Auto_contrast_status;
uint8_t _fg_scene_change;
uint32_t _luma_max;
uint32_t _luma_min;
uint8_t _trick_cnt;
uint8_t _normal_cnt;

uint32_t _scene_change;
uint32_t _scene_normal;

uint16_t _wa_luma_array[33];
uint8_t _change_cnt[33];

uint8_t _cur_gain_l;
uint8_t _cur_gain_m;
uint8_t _cur_gain_h;
uint8_t _value_l;
uint8_t _value_h;

uint32_t _pp_status;
uint8_t _input_cs;
enum POST_VIDEO_MODE_ENUM _video_mode;

uint32_t _src_width, _output_width;

uint8_t sharp_ui_min, sharp_ui_max, sharp_dft, sharp_cur;
uint32_t _max_sat;
uint8_t _dynamic_state;
bool _fg_pp_hal_init;
static const uint16_t _gamma_tbl[9][33] = {
/*gamma=1.4*/
	{0x000, 0x000, 0x040, 0x048, 0x056, 0x067, 0x07A, 0x090, 0x0A7, 0x0C0, 0x0DB, 0x0F6, 0x113,
	 0x132, 0x151, 0x172,
	 0x193, 0x1B5, 0x1D9, 0x1FD, 0x222, 0x248, 0x26F, 0x296, 0x2BF, 0x2E8, 0x312, 0x33C, 0x367,
	 0x393, 0x3C0, 0x3F0, 0x3C0},
/*gamma=1.3*/
	{0x000, 0x000, 0x040, 0x04B, 0x05C, 0x071, 0x087, 0x09F, 0x0B8, 0x0D3, 0x0EF, 0x10C, 0x12A,
	 0x149, 0x169, 0x18A,
	 0x1AB, 0x1CE, 0x1F0, 0x214, 0x238, 0x25D, 0x282, 0x2A8, 0x2CE, 0x2F5, 0x31D, 0x345, 0x36D,
	 0x396, 0x3C0, 0x3F0, 0x3C0},
/*gamma=1.2*/
	{0x000, 0x000, 0x040, 0x050, 0x065, 0x07D, 0x096, 0x0B1, 0x0CD, 0x0E9, 0x107, 0x125, 0x144,
	 0x164, 0x184, 0x1A4,
	 0x1C6, 0x1E7, 0x209, 0x22C, 0x24F, 0x272, 0x296, 0x2BA, 0x2DE, 0x303, 0x328, 0x34E, 0x373,
	 0x399, 0x3C0, 0x3F0, 0x3C0},
/*gamma=1.1*/
	{0x000, 0x000, 0x040, 0x056, 0x071, 0x08C, 0x0A9, 0x0C6, 0x0E4, 0x103, 0x121, 0x141, 0x160,
	 0x180, 0x1A0, 0x1C1,
	 0x1E1, 0x202, 0x224, 0x245, 0x267, 0x288, 0x2AA, 0x2CC, 0x2EF, 0x311, 0x334, 0x356, 0x379,
	 0x39C, 0x3C0, 0x3F0, 0x3C0},
/*gamma=1.0*/
	{0x000, 0x000, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x120, 0x140, 0x160, 0x180,
	 0x1A0, 0x1C0, 0x1E0,
	 0x200, 0x220, 0x240, 0x260, 0x280, 0x2A0, 0x2C0, 0x2E0, 0x300, 0x320, 0x340, 0x360, 0x380,
	 0x3A0, 0x3C0, 0x3F0, 0x3C0},
/*gamma=0.9*/
	{0x000, 0x000, 0x040, 0x06C, 0x093, 0x0B8, 0x0DB, 0x0FE, 0x11F, 0x141, 0x162, 0x182, 0x1A2,
	 0x1C2, 0x1E1, 0x201,
	 0x220, 0x23E, 0x25D, 0x27B, 0x29A, 0x2B8, 0x2D5, 0x2F3, 0x311, 0x32E, 0x34B, 0x369, 0x386,
	 0x3A3, 0x3C0, 0x3F0, 0x3C0},
/*gamma=0.8*/
	{0x000, 0x000, 0x040, 0x07E, 0x0AC, 0x0D6, 0x0FC, 0x121, 0x145, 0x167, 0x188, 0x1A9, 0x1C9,
	 0x1E8, 0x206, 0x224,
	 0x242, 0x25F, 0x27C, 0x299, 0x2B5, 0x2D1, 0x2EC, 0x307, 0x322, 0x33D, 0x358, 0x372, 0x38C,
	 0x3A6, 0x3C0, 0x3F0, 0x3C0},
/*gamma=0.7*/
	{0x000, 0x000, 0x040, 0x096, 0x0CD, 0x0FB, 0x125, 0x14C, 0x170, 0x193, 0x1B4, 0x1D4, 0x1F3,
	 0x211, 0x22F, 0x24B,
	 0x267, 0x282, 0x29D, 0x2B7, 0x2D1, 0x2EB, 0x303, 0x31C, 0x334, 0x34C, 0x364, 0x37B, 0x392,
	 0x3A9, 0x3C0, 0x3F0, 0x3C0},
/*gamma=0.6*/
	{0x000, 0x000, 0x040, 0x0B9, 0x0F7, 0x12A, 0x156, 0x17E, 0x1A3, 0x1C6, 0x1E6, 0x205, 0x223,
	 0x23F, 0x25A, 0x275,
	 0x28F, 0x2A8, 0x2C0, 0x2D8, 0x2EF, 0x306, 0x31C, 0x331, 0x347, 0x35C, 0x370, 0x385, 0x399,
	 0x3AC, 0x3C0, 0x3F0, 0x3C0}
};


uint32_t src_res = HDMI_VIDEO_720x480i_60Hz;
uintptr_t pp_reg_base;
uintptr_t fmt_reg_base;
uintptr_t vdo_reg_base;
uintptr_t io_reg_base;
/******************************************************************************
*Function: pp_hal_init(void)
*Description: get register base.
*Parameter: None
*Return: PP_OK or PP_FAIL
******************************************************************************/

int pp_hal_init(void)
{
	struct device_node *np;
	unsigned int reg_value;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-pp");
	if (np == NULL) {
		PP_ERR("dts error, no pp device node.\n");
		return PP_FAIL;
	}
	of_property_read_u32_index(np, "reg", 1, &reg_value);
	pp_reg_base = (uintptr_t) of_iomap(np, 0);
	fmt_reg_base = (uintptr_t) of_iomap(np, 1);
	vdo_reg_base = (uintptr_t) of_iomap(np, 2);
	io_reg_base = (uintptr_t) of_iomap(np, 3);

	_luma_main_hw_reg = (union HAL_POST_LUMA_MAIN_UNION_T *)HAL_POST_LUMA_MAIN_REG;
	return PP_OK;
}

/******************************************************************************
*Function: pp_hal_sce_init(void)
*Description:
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_sce_init(void)
{
	uint16_t bi;
	uint16_t w_offset;

	/*Note:Due to Fetch in VSync, can't use vRegWrite2B */
	for (bi = 0; bi < 16; bi++)
		vRegWt4B(Y_FTN_1_0 + 4 * bi, 0x400040 * bi + 0x200000);

	vRegWt2B(Y_FTN_32_, 0x3FF);


	w_offset = 0;

	for (bi = 0; bi < 16; bi++) {
		vRegWt1B(Y_SLOPE_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_G1_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_G2_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_G3_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_P1_1_0_MAIN + w_offset, 0x20);
		vRegWt1B(S_P2_1_0_MAIN + w_offset, 0x40);
		vRegWt1B(S_Y0_G_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_Y64_G_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_Y128_G_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_Y192_G_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(S_Y256_G_1_0_MAIN + w_offset, 0x80);
		vRegWt1B(H_FTN_1_0_MAIN + w_offset, 0x00);

		w_offset += 2;
	}
}

/******************************************************************************
*Function: pp_hal_hlti_on_off(uint8_t b_on_off)
*Description:
*Parameter:
*Return:
******************************************************************************/
void pp_hal_hlti_on_off(uint8_t b_on_off)
{
	if (b_on_off > 0)
		_pp_status |= HLTI_ON_STATUS;
	else
		_pp_status |= HLTI_OFF_STATUS;

	vRegWt4BMsk(HLTI_00, ((b_on_off > 0) ? HLTI_EN : 0), HLTI_EN);
}

/******************************************************************************
*Function: pp_hal_lti_on_off(uint8_t b_on_off)
*Description:
*Parameter:
*Return:
******************************************************************************/
void pp_hal_lti_on_off(uint8_t b_on_off)
{
	if (b_on_off > 0)
		_pp_status |= LTI_ON_STATUS;
	else
		_pp_status |= LTI_OFF_STATUS;

	vRegWt4BMsk(TDPROC_24, ((b_on_off > 0) ? TDSHARP_LTI_EN : 0), TDSHARP_LTI_EN);
}

/******************************************************************************
*Function: pp_hal_lti_init(void)
*Description:
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_lti_init(HDMI_VIDEO_RESOLUTION e_res)
{
	uint16_t bi;
/*uint16_t w_offset;  */
	if (pp_is_video_hd((uint8_t) e_res)) {
		for (bi = 0; bi < (sizeof(_qty_tbl_lti_720_1080) / 4); bi += 2)
			vRegWt4B(_qty_tbl_lti_720_1080[bi], _qty_tbl_lti_720_1080[bi + 1]);
	} else {
		for (bi = 0; bi < (sizeof(_wQtTbl_LTI_SD) / 4); bi += 2)
			vRegWt4B(_wQtTbl_LTI_SD[bi], _wQtTbl_LTI_SD[bi + 1]);
	}
}

/******************************************************************************
*Function: pp_hal_hv_band_on_off(uint8_t b_on_off)
*Description:
*Parameter:
*Return: None
******************************************************************************/
void pp_hal_hv_band_on_off(uint8_t b_on_off)
{
	if (b_on_off > 0)
		_pp_status |= HV_ON_STATUS;
	else
		_pp_status |= HV_OFF_STATUS;

	vRegWt4BMsk(TDPROC_6B, ((b_on_off > 0) ? TDSHARP_HVBAND_EN : 0), TDSHARP_HVBAND_EN);
}

/******************************************************************************
*Function: pp_hal_hv_band_init(void)
*Description:
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_hv_band_init(HDMI_VIDEO_RESOLUTION e_res)
{
	uint16_t bi;
/*uint16_t w_offset;  */

	if (pp_is_video_hd((uint8_t) e_res)) {
		for (bi = 0; bi < (sizeof(_qty_tbl_hv_band_720_1080) / 4); bi += 2)
			vRegWt4B(_qty_tbl_hv_band_720_1080[bi], _qty_tbl_hv_band_720_1080[bi + 1]);
	} else {
		for (bi = 0; bi < (sizeof(_wQtTbl_HVBand_SD) / 4); bi += 2)
			vRegWt4B(_wQtTbl_HVBand_SD[bi], _wQtTbl_HVBand_SD[bi + 1]);
	}
}

/******************************************************************************
*Function: pp_hal_adap_shp_on_off(uint8_t b_on_off)
*Description:
*Parameter:
*Return: None
******************************************************************************/
void pp_hal_adap_shp_on_off(uint8_t b_on_off)
{
	if (b_on_off > 0)
		_pp_status |= ADAP_SHP_ON_STATUS;
	else
		_pp_status |= ADAP_SHP_OFF_STATUS;

	vRegWt4BMsk(TDPROC_B0, ((b_on_off > 0) ? ADAP_SHP_EN : 0), ADAP_SHP_EN);
}

/******************************************************************************
*Function: pp_hal_adap_init(void)
*Description:
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_adap_init(HDMI_VIDEO_RESOLUTION e_res)
{
	uint16_t bi;

	if (pp_is_video_hd((uint8_t) e_res)) {
		for (bi = 0; bi < (sizeof(_qty_tbl_adap_720_1080) / 4); bi += 2)
			vRegWt4B(_qty_tbl_adap_720_1080[bi], _qty_tbl_adap_720_1080[bi + 1]);
	} else {
		for (bi = 0; bi < (sizeof(_wQtTbl_ADAP_SD) / 4); bi += 2)
			vRegWt4B(_wQtTbl_ADAP_SD[bi], _wQtTbl_ADAP_SD[bi + 1]);
	}
}


/******************************************************************************
*Function: pp_hal_sat_hist_init(void)
*Description: Set Saturation Histogram BIN bound
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_sat_hist_init(void)
{
	uint16_t bi;

	vRegWt1B(SAT_HIST_CFG_MAIN_3, 0x05);
	for (bi = 1; bi < 7; bi++)
		vRegWt1B(SAT_HIST_CFG_MAIN_3 + bi, (0x18 * bi + 5));
}

/******************************************************************************
*Function: pp_hal_sat_hist_win(uint32_t win_x, uint32_t win_y)
*Description: Set Saturation Histogram window
*Parameter: win_x=Hend, win_y=Vend
*Return: None
******************************************************************************/
void pp_hal_sat_hist_win(uint32_t win_x, uint32_t win_y)
{
	uint8_t bY_offset;

	if (win_y < 720)
		bY_offset = 2;
	else if (win_y < 1080)
		bY_offset = 1;
	else
		bY_offset = 0;


	vRegWt4BMsk(SAT_HIST_X_CFG_MAIN, 0, WINDOW_X_START);
	vRegWt4BMsk(SAT_HIST_Y_CFG_MAIN, bY_offset, WINDOW_Y_START);

	vRegWt4BMsk(SAT_HIST_X_CFG_MAIN, win_x << 16, WINDOW_X_END);
	vRegWt4BMsk(SAT_HIST_Y_CFG_MAIN, (win_y + bY_offset) << 16, WINDOW_Y_END);
	_max_sat = ((win_x * win_y) >> 5);
}

/******************************************************************************
*Function: pp_hal_yc_proc_init(void)
*Description: Initial YC parameters
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_yc_proc_init(void)
{
	_fg_pp_hal_init = false;

	/*Turn off sharpness */
	vRegWt4BMsk(TDPROC_24, 0, TDSHARP_EN | TDPPROC_BYPASS_ALL);

	/*Turnoff CTI */
	vRegWt4BMsk(ECTI_00, 0, ECTI_ENA);

	/*Rework for TV truncate 12->10bits */
	/*vRegWtFldAlign(COLOR_P_OPTION, 1, ORIGINAL_CLIP);  */
	vRegWt4BMsk(COLOR_P_OPTION, ORIGINAL_CLIP, ORIGINAL_CLIP);

	/*Not expand chroma and luma */
	/*vRegWtFldAlign(DBG_CFG_MAIN, 0, EXPAND);  */
	vRegWt4BMsk(DBG_CFG_MAIN, 0, EXPAND);

	/*Global picture value */
	vRegWt4B(G_PIC_ADJ_MAIN, 0x80808080);

	/*Init Sat Histogram */
	pp_hal_sat_hist_init();

	/*Init SCE */
	pp_hal_sce_init();
	pp_hal_set_sce_table(POST_VIDEO_MODE_STD);

	/*Bypass color engine */
	/*vRegWtFldAlign(CFG_MAIN, 0x80, COLORBP);  */
	vRegWt4BMsk(CFG_MAIN, 0x80, COLORBP);

	pp_hal_set_con_dft();
	pp_auto_con_init();


	_pp_status = 0;
	_scene_change = 2;
	_scene_normal = 3;
	_src_width = 0;
	_output_width = 0;
	_fg_pp_hal_init = TRUE;
}

void pp_hal_scene_chg_th(uint8_t change, uint8_t normal)
{
	_scene_change = change;
	_scene_normal = normal;
}

struct pp_active_zone_setting {
	uint32_t h_active;
	uint32_t v_odd_active;
	uint32_t v_even_active;
	uint32_t delay;
	HDMI_VIDEO_RESOLUTION res;
};


struct pp_active_zone_setting pp_active[HDMI_VIDEO_RESOLUTION_NUM] = {

	/*0x3000,       0x3004,                 0x3008,                 0x3020 */
	{0x007B034A, 0x002B020A, 0x002B020A, 0x041602E3, HDMI_VIDEO_720x480i_60Hz},	/*0 */
	{0x00850354, 0x002D026C, 0x002D026C, 0x04DE02E9, HDMI_VIDEO_720x576i_50Hz},	/*1 */
	{0x007B034A, 0x002B020A, 0x002B020A, 0x041602E3, HDMI_VIDEO_720x480p_60Hz},	/*2 */
	{0x00850354, 0x002D026C, 0x002D026C, 0x04DE02E9, HDMI_VIDEO_720x576p_50Hz},	/*3 */
	{0x01050604, 0x001A02E9, 0x001A02E9, 0x05D805FA, HDMI_VIDEO_1280x720p_60Hz},	/*4 */
	{0x01050604, 0x001A02E9, 0x001A02E9, 0x05D80745, HDMI_VIDEO_1280x720p_50Hz},	/*5 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60821, HDMI_VIDEO_1920x1080i_60Hz},	/*6 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C609D9, HDMI_VIDEO_1920x1080i_50Hz},	/*7 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60821, HDMI_VIDEO_1920x1080p_30Hz},	/*8 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C609D9, HDMI_VIDEO_1920x1080p_25Hz},	/*9 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60A47, HDMI_VIDEO_1920x1080p_24Hz},	/*a */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60A47, HDMI_VIDEO_1920x1080p_23Hz},	/*b */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60821, HDMI_VIDEO_1920x1080p_29Hz},	/*c */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60821, HDMI_VIDEO_1920x1080p_60Hz},	/*d */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C609D9, HDMI_VIDEO_1920x1080p_50Hz},	/*e */
	{0x01050604, 0x001A02E9, 0x001A02E9, 0x05D805FB, HDMI_VIDEO_1280x720p3d_60Hz},	/*f */
	{0x01050604, 0x001A02E9, 0x001A02E9, 0x05D80745, HDMI_VIDEO_1280x720p3d_50Hz},	/*10 */
	{0, 0x0026045E, 0x0026045E, 0x00020325, HDMI_VIDEO_1920x1080i3d_60Hz},	/*11 */
	{0, 0x0026045E, 0x0026045E, 0x00020325, HDMI_VIDEO_1920x1080i3d_50Hz},	/*12 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60A47, HDMI_VIDEO_1920x1080p3d_24Hz},	/*13 */
	{0x00C10840, 0x002A0461, 0x002A0461, 0x08C60A47, HDMI_VIDEO_1920x1080p3d_23Hz},	/*14 */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1576, HDMI_VIDEO_3840x2160P_23_976HZ},	/*15 */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1576, HDMI_VIDEO_3840x2160P_24HZ},	/*16 */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1576, HDMI_VIDEO_3840x2160P_25HZ},	/*17 */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1082, HDMI_VIDEO_3840x2160P_29_97HZ},	/*18 */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1082, HDMI_VIDEO_3840x2160P_30HZ},	/*19 */
	{0x01811080, 0x005408C3, 0x005408C3, 0x118E1505, HDMI_VIDEO_4096x2160P_24HZ},	/*1a */
	{0x00D9110F, 0x005408C3, 0x005408C3, 0x118E1086, HDMI_VIDEO_3840x2160P_60HZ},	/*1b */
	{0x01811180, 0x005408C3, 0x005408C3, 0x118E1381, HDMI_VIDEO_3840x2160P_50HZ},	/*1c */
	{0x01811080, 0x005408C3, 0x005408C3, 0x118E10B9, HDMI_VIDEO_4096x2160P_60HZ},	/*1d */
	{0x01811080, 0x005408C3, 0x005408C3, 0x118E1429, HDMI_VIDEO_4096x2160P_50HZ},	/*1e */
};

void pp_hal_set_fmt(HDMI_VIDEO_RESOLUTION e_res)
{
	uint8_t i;

	PP_FUNC();
	for (i = 0; i < HDMI_VIDEO_RESOLUTION_NUM; i++) {
		if (pp_active[i].res == e_res) {
			PP_INFO("set pp fmt for res id:%d", i);
			vRegWt4B(fmt_reg_base + 0x00, pp_active[i].h_active);
			vRegWt4B(fmt_reg_base + 0x04, pp_active[i].v_odd_active);
			vRegWt4B(fmt_reg_base + 0x08, pp_active[i].v_even_active);
			vRegWt4B(fmt_reg_base + 0x20, pp_active[i].delay);
			pp_hal_set_path(SV_BYPASS);
			/*guiwu: bypass first, should be enable when debug OK*/
			/*pp_hal_set_path(SV_BYPASS);*/
			break;
		}
	}
	if (i == HDMI_VIDEO_RESOLUTION_NUM) {
		PP_INFO("unknown resolution\n");
		pp_hal_set_path(SV_BYPASS);

	}
}

/******************************************************************************
*Function: pp_hal_chg_res(HDMI_VIDEO_RESOLUTION e_res)
*Description: setting fmt screen for all resolution
*Parameter: value of res
*Return: None
******************************************************************************/
void pp_hal_chg_res(HDMI_VIDEO_RESOLUTION e_res)
{

	pp_hal_set_fmt(e_res);
	vRegWt4B((fmt_reg_base + 0xAC), u4RegRd4B(fmt_reg_base + 0xAC) | 0x80000000);
	switch (e_res) {
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x480p_60Hz:
		pp_hal_sat_hist_win(720, 480);
		break;
	case HDMI_VIDEO_720x576i_50Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		pp_hal_sat_hist_win(720, 576);
		break;
	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
	case HDMI_VIDEO_1280x720p3d_60Hz:
	case HDMI_VIDEO_1280x720p3d_50Hz:
		pp_hal_sat_hist_win(1280, 720);
		break;
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
	case HDMI_VIDEO_1920x1080p_25Hz:	/*15 */
	case HDMI_VIDEO_1920x1080p_24Hz:	/*18 */
	case HDMI_VIDEO_1920x1080p_23Hz:	/*19, 1080P23.976hz */
	case HDMI_VIDEO_1920x1080p_29Hz:	/*20, 1080P29.97hz */
	case HDMI_VIDEO_1920x1080p3d_23Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
	case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080i3d_50Hz:
		pp_hal_sat_hist_win(1920, 1080);
		break;
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
		pp_hal_sat_hist_win(3840, 2160);
		break;

	case HDMI_VIDEO_4096x2160P_24HZ:
	case HDMI_VIDEO_4096x2160P_60HZ:
	case HDMI_VIDEO_4096x2160P_50HZ:
		pp_hal_sat_hist_win(4096, 2160);
		break;
	default:
		pp_hal_sat_hist_win(0, 0);
		break;
	}

	switch (e_res) {
	case HDMI_VIDEO_1920x1080p3d_23Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
	case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080i3d_50Hz:
		/*disable TDSHARP */
		vRegWt4BMsk(TDPROC_24, 0, TDSHARP_EN);
		/*disable CTI */
		vRegWt4BMsk(ECTI_00, 0, ECTI_ENA);
		/*disablead aptive luma */
		pp_hal_set_con_dft();
		pp_auto_con_init();
		/*info DCM module turn off */
		/*bypass Color-P */
		/*vRegWtFldAlign(CFG_MAIN, 1, ALLBP);  */
		vRegWt4BMsk(CFG_MAIN, ALLBP, ALLBP);
		vRegWt4BMsk(COLOR_P_OPTION, 0, C2P_SX2 | ORIGINAL_CLIP);
		break;
	default:
		pp_hal_set_video_mode(_video_mode);
		break;
	}


	src_res = e_res;
	PP_INFO("DrvFmtScreene_res=%d\n", e_res);
}

/******************************************************************************
*Function: pp_hal_chg_input_cs(uint8_t cs)
*Description: ColorspaceforSCEtuning
*Parameter: valueofcs
*Return: None
******************************************************************************/
void pp_hal_chg_input_cs(uint8_t cs)
{

	_input_cs = cs;
}

/******************************************************************************
*Function: pp_hal_src_region(uint32_t src_width, uint32_t output_width)
*Description: Source image size for sharpness reference
*Parameter: src_width, output_width
*Return: None
******************************************************************************/
void pp_hal_src_region(uint32_t src_width, uint32_t output_width)
{

	uint16_t qty_item;
	uint8_t u1_gain_ratio1, u1_gain_ratio2,
	    u1_gain_ratio3, u1_gain_ratio4, u1_gain_ratio5, u1_gain_ratio6;
#ifdef NEW_LUMA_ALGORITHM
	uint8_t u1Coring1, u1Coring2, u1Coring3, u1Coring4, u1Coring5, u1Coring6;
#endif
	/*source change or output change */
	if ((_src_width != src_width)
	    || (_output_width != output_width)) {
		_src_width = src_width;
		_output_width = output_width;
		PP_INFO("SrcWidth=%d, OutputWidth=%d\n", src_width, output_width);
		/*according to UI reset Gians */
		if (_src_width < 720) {	/*Src less thanSD */
			if (_output_width <= 720) {	/*output 480/576 */
				u1_gain_ratio1 = 10;
				u1_gain_ratio2 = 10;
				u1_gain_ratio3 = 12;
				u1_gain_ratio4 = 16;
				u1_gain_ratio5 = 20;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x10;
				u1Coring2 = 0x10;
				u1Coring3 = 0x10;
				u1Coring4 = 0x10;
				u1Coring5 = 0x10;
				u1Coring6 = 0x10;
#endif
			} else if (_output_width <= 1280) {	/*output 720 */
				u1_gain_ratio1 = 14;
				u1_gain_ratio2 = 14;
				u1_gain_ratio3 = 16;
				u1_gain_ratio4 = 20;
				u1_gain_ratio5 = 25;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x0A;
				u1Coring2 = 0x0A;
				u1Coring3 = 0x0A;
				u1Coring4 = 0x0A;
				u1Coring5 = 0x10;
				u1Coring6 = 0x10;
#endif
			} else {	/*output 1080 */

				u1_gain_ratio1 = 25;
				u1_gain_ratio2 = 25;
				u1_gain_ratio3 = 25;
				u1_gain_ratio4 = 30;
				u1_gain_ratio5 = 40;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x08;
				u1Coring2 = 0x08;
				u1Coring3 = 0x08;
				u1Coring4 = 0x08;
				u1Coring5 = 0x08;
				u1Coring6 = 0x10;
#endif
			}
		} else if (_src_width < 1280) {	/*Src is 480/576 */
			if (_output_width <= 720) {	/*output 480/576 */
				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 2;
				u1_gain_ratio4 = 4;
				u1_gain_ratio5 = 5;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x10;
				u1Coring2 = 0x10;
				u1Coring3 = 0x10;
				u1Coring4 = 0x10;
				u1Coring5 = 0x10;
				u1Coring6 = 0x10;
#endif
			} else if (_output_width <= 1280) {	/*output 720 */
				u1_gain_ratio1 = 10;
				u1_gain_ratio2 = 10;
				u1_gain_ratio3 = 12;
				u1_gain_ratio4 = 12;
				u1_gain_ratio5 = 30;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x0C;
				u1Coring2 = 0x0C;
				u1Coring3 = 0x0C;
				u1Coring4 = 0x0C;
				u1Coring5 = 0x0C;
				u1Coring6 = 0x10;
#endif
			} else {	/*output 1080 */

				u1_gain_ratio1 = 16;
				u1_gain_ratio2 = 16;
				u1_gain_ratio3 = 16;
				u1_gain_ratio4 = 20;
				u1_gain_ratio5 = 40;
				u1_gain_ratio6 = 0;
#ifdef NEW_LUMA_ALGORITHM
				u1Coring1 = 0x0A;
				u1Coring2 = 0x0A;
				u1Coring3 = 0x0A;
				u1Coring4 = 0x0A;
				u1Coring5 = 0x0A;
				u1Coring6 = 0x10;
#endif
			}
		} else if (_src_width < 1920) {	/*Src is 720 */
			if (_output_width <= 720) {	/*output 480/576 */
				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 2;
				u1_gain_ratio4 = 4;
				u1_gain_ratio5 = 5;
				u1_gain_ratio6 = 0;
			} else if (_output_width <= 1280) {	/*output 720 */
				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 2;
				u1_gain_ratio4 = 4;
				u1_gain_ratio5 = 15;
				u1_gain_ratio6 = 0;
			} else {	/*output 1080 */

				u1_gain_ratio1 = 5;
				u1_gain_ratio2 = 5;
				u1_gain_ratio3 = 1;
				u1_gain_ratio4 = 4;
				u1_gain_ratio5 = 1;
				u1_gain_ratio6 = 0;
			}
#ifdef NEW_LUMA_ALGORITHM
			u1Coring1 = 0x10;
			u1Coring2 = 0x10;
			u1Coring3 = 0x10;
			u1Coring4 = 0x10;
			u1Coring5 = 0x10;
			u1Coring6 = 0x10;
#endif
		} else {	/*Src is 1080 */

			if (_output_width <= 720) {	/*output 480/576 */
				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 1;
				u1_gain_ratio4 = 2;
				u1_gain_ratio5 = 2;
				u1_gain_ratio6 = 0;
			} else if (_output_width <= 1280) {	/*output 720 */
				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 1;
				u1_gain_ratio4 = 2;
				u1_gain_ratio5 = 2;
				u1_gain_ratio6 = 0;
			} else {	/*output 1080 */

				u1_gain_ratio1 = 1;
				u1_gain_ratio2 = 1;
				u1_gain_ratio3 = 1;
				u1_gain_ratio4 = 3;
				u1_gain_ratio5 = 1;
				u1_gain_ratio6 = 0;
			}
#ifdef NEW_LUMA_ALGORITHM
			u1Coring1 = 0x10;
			u1Coring2 = 0x10;
			u1Coring3 = 0x10;
			u1Coring4 = 0x10;
			u1Coring5 = 0x10;
			u1Coring6 = 0x10;
#endif
		}
		PP_DEBUG("u1_gain_ratio1=0x%x, u1_gain_ratio2=0x%x, u1_gain_ratio3=0x%x,",
			u1_gain_ratio1, u1_gain_ratio2, u1_gain_ratio3);
		PP_DEBUG("u1_gain_ratio4=0x%x, u1_gain_ratio5=0x%x, u1_gain_ratio6=0x%x\n",
			u1_gain_ratio4, u1_gain_ratio5, u1_gain_ratio6);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN1].qty_dft_max =
		    (u1_gain_ratio1 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio1 + 0x40);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN2].qty_dft_max =
		    (u1_gain_ratio2 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio2 + 0x40);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN3].qty_dft_max =
		    (u1_gain_ratio3 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio3 + 0x40);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN4].qty_dft_max =
		    (u1_gain_ratio4 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio4 + 0x40);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN5].qty_dft_max =
		    (u1_gain_ratio5 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio5 + 0x40);
		dft_qty_tbl[QUALITY_TDSHARP_GAIN6].qty_dft_max =
		    (u1_gain_ratio6 > 0x26) ? (0xFF) : (TDSRP_BASE_GAIN * u1_gain_ratio6 + 0x40);
#ifdef NEW_LUMA_ALGORITHM
		dft_qty_tbl[QUALITY_TDSHARP_CORING1].qty_dft_max = u1Coring1;
		dft_qty_tbl[QUALITY_TDSHARP_CORING2].qty_dft_max = u1Coring2;
		dft_qty_tbl[QUALITY_TDSHARP_CORING3].qty_dft_max = u1Coring3;
		dft_qty_tbl[QUALITY_TDSHARP_CORING4].qty_dft_max = u1Coring4;
		dft_qty_tbl[QUALITY_TDSHARP_CORING5].qty_dft_max = u1Coring5;
		dft_qty_tbl[QUALITY_TDSHARP_CORING6].qty_dft_max = u1Coring6;
		for (qty_item = QUALITY_TDSHARP_GAIN1; qty_item <= QUALITY_TDSHARP_CORING6;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, sharp_ui_min, sharp_ui_max,
						       sharp_dft, sharp_cur));
		}

		vRegWt4BMsk(TDPROC_00,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN1) << 24) |
			    read_qty_table
			    (QUALITY_TDSHARP_CORING1), TDSHARP_GAIN1 | TDSHARP_CORING1);
		vRegWt4BMsk(TDPROC_04,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN2) << 24) |
			    read_qty_table
			    (QUALITY_TDSHARP_CORING2), TDSHARP_GAIN2 | TDSHARP_CORING2);
		vRegWt4BMsk(TDPROC_08,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN3) << 24) |
			    read_qty_table
			    (QUALITY_TDSHARP_CORING3), TDSHARP_GAIN3 | TDSHARP_CORING3);
		vRegWt4BMsk(TDPROC_0C,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN4) << 24) |
			    read_qty_table
			    (QUALITY_TDSHARP_CORING4), TDSHARP_GAIN4 | TDSHARP_CORING4);
		vRegWt4BMsk(TDPROC_10,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN5) << 24) |
			    read_qty_table
			    (QUALITY_TDSHARP_CORING5), TDSHARP_GAIN5 | TDSHARP_CORING5);
		vRegWt4BMsk(TDPROC_14,
			    (read_qty_table
			     (QUALITY_TDSHARP_GAIN6) << 24) |
			    read_qty_table(QUALITY_TDSHARP_CORING6),
			    TDSHARP_GAIN6 | TDSHARP_CORING6);
#else
		for (qty_item = QUALITY_TDSHARP_GAIN1; qty_item <= QUALITY_TDSHARP_GAIN6;
		     qty_item++) {
			write_qty_table(qty_item,
					pp_qty_mapping(qty_item, sharp_ui_min, sharp_ui_max,
						       sharp_dft, sharp_cur));
		}
		vRegWt4BMsk(TDPROC_00, read_qty_table(QUALITY_TDSHARP_GAIN1), TDSHARP_GAIN1);
		vRegWt4BMsk(TDPROC_04, read_qty_table(QUALITY_TDSHARP_GAIN2), TDSHARP_GAIN2);
		vRegWt4BMsk(TDPROC_08, read_qty_table(QUALITY_TDSHARP_GAIN3), TDSHARP_GAIN3);
		vRegWt4BMsk(TDPROC_0C, read_qty_table(QUALITY_TDSHARP_GAIN4), TDSHARP_GAIN4);
		vRegWt4BMsk(TDPROC_10, read_qty_table(QUALITY_TDSHARP_GAIN5), TDSHARP_GAIN5);
		vRegWt4BMsk(TDPROC_14, read_qty_table(QUALITY_TDSHARP_GAIN6), TDSHARP_GAIN6);
#endif
	}
}

/*******************
*modeuleOn/Off*
********************/

/******************************************************************************
*Function: pp_hal_set_video_mode(enum POST_VIDEO_MODE_ENUM emode)
*Description: enable/Disable All post engine
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_video_mode(enum POST_VIDEO_MODE_ENUM emode)
{
	_video_mode = emode;
	switch (emode) {
	case POST_VIDEO_MODE_STD:
	default:
		/*disable TDSHARP */
		vRegWt4BMsk(TDPROC_24, ((_pp_status & TDSHARP_ON_STATUS) << 31), TDSHARP_EN);
		/*disable CTI */
		vRegWt4BMsk(ECTI_00, ((_pp_status & CTI_ON_STATUS) >> 1), ECTI_ENA);
		/*disable adaptive luma */
		pp_hal_set_con_dft();
		pp_auto_con_init();
		/*bypassColor-P */
		/*vRegWtFldAlign(CFG_MAIN, 1, ALLBP);  */
		if ((_pp_status & CP_ON_STATUS) == 0)
			vRegWt4BMsk(CFG_MAIN, 1 << 7, ALLBP);
		else
			vRegWt4BMsk(CFG_MAIN, 0 << 7, ALLBP);
		vRegWt4BMsk(COLOR_P_OPTION, 0, C2P_SX2 | ORIGINAL_CLIP);
		vRegWt4BMsk(VDO_SHARP_CTRL_02, (1 << 23), SHARP_EN_B1);
		/*vRegWt4BMsk(VDO_SHARP_CTRL_03, (0<<24), BYPASS_SHARP);  */
		vRegWt4BMsk(TDPROC_C0, (0 << 31), LC_EN);
		break;
	case POST_VIDEO_MODE_VIVID:
	case POST_VIDEO_MODE_CINEMA:
	case POST_VIDEO_MODE_CUSTOMER:
		vRegWt4BMsk(TDPROC_10, ((_pp_status & TDSHARP_ON_STATUS) << 31), TDSHARP_EN);
		/*process Color */
		if ((_pp_status & CP_ON_STATUS) == 0)
			vRegWt4BMsk(CFG_MAIN, 1 << 7, ALLBP);
		else
			vRegWt4BMsk(CFG_MAIN, 0 << 7, ALLBP);
		vRegWt4BMsk(COLOR_P_OPTION, C2P_SX2 | ORIGINAL_CLIP, C2P_SX2 | ORIGINAL_CLIP);
		vRegWt4BMsk(VDO_SHARP_CTRL_02, (1 << 23), SHARP_EN_B1);
		break;
	}
	/*load table */
	pp_hal_set_sce_table(emode);
}

/******************************************************************************
*Function: pp_search_qty_item(uint16_t qty_item)
*Description: Search table index fromb Default QtyTbl
*Parameter: qty_item QUALITY_xxx
*Return: Search Index/0xFFFF=invalid.
******************************************************************************/

uint16_t pp_search_qty_item(uint16_t qty_item)
{

	uint16_t u2SearchIndex;
	/*Search the item index DefaultQty Tbl. */
	for (u2SearchIndex = 0; u2SearchIndex < QUALITY_MAX; u2SearchIndex++) {
		/*Find a match entry. */
		if (dft_qty_tbl[u2SearchIndex].qty_dft_item == qty_item)
			return u2SearchIndex;

		/*Search to the end of table.Nofound. */
		if ((dft_qty_tbl[u2SearchIndex].qty_dft_min == 0xFF)
		    && (dft_qty_tbl[u2SearchIndex].qty_dft_max == 0xFF)
		    && (dft_qty_tbl[u2SearchIndex].qty_dft_dft == 0xFF)) {
			return 0xFFFF;	/*Can not find valid quality value. */
		}
	}
	return 0xFFFF;
}

/******************************************************************************
*Function: pp_qty_mapping(uint16_t qty_item, int16_t ui_min, int16_t ui_max,
int16_t ui_dft, int16_t ui_cur)
*Description: Mapping from UImin, max, dft, curtoHWmin, max, dft, cur
*Parameter: qty_item, i2Min, i2Max, i2Dft, i2Cur:UIvalues.
*Return: mapping value
******************************************************************************/

uint16_t pp_qty_mapping(uint16_t qty_item,
			int16_t ui_min, int16_t ui_max, int16_t ui_dft, int16_t ui_cur)
{

	/*search correct item by timing & index first */
	uint16_t u2HwMin;
	uint16_t u2HwMax;
	uint16_t u2HwDft;
	uint16_t u2SearchIndex;

	u2SearchIndex = pp_search_qty_item(qty_item);
	/*Not found in default quality table!!! Quality table should be updated!!! */
	if (u2SearchIndex == 0xFFFF)
		return 0;

	u2HwMin = dft_qty_tbl[u2SearchIndex].qty_dft_min;
	u2HwMax = dft_qty_tbl[u2SearchIndex].qty_dft_max;
	u2HwDft = dft_qty_tbl[u2SearchIndex].qty_dft_dft;
	PP_DEBUG("HWMin=0x%x, HWmax=0x%x, HWDtf=0x%x\n", u2HwMin, u2HwMax, u2HwDft);
	/*Some qty_item does not need mapping, return defaultHW value. */
	/*or UI_cur==UI_dft, return default HW value. */
	if (ui_min == ui_max)
		return u2HwDft;

	if ((u2HwMin == u2HwMax) || (ui_cur == ui_dft))
		return u2HwDft;

	if (ui_cur > ui_max)
		return u2HwMax;

	if (ui_cur < ui_min)
		return u2HwMin;

	/*Some qty_item use 2-level setting as min/max/default, don't mapping. */
	if (ui_max == 2) {	/*UI_max=2 means UI optionis:off, low, high. */
		if (ui_cur == 0)
			return u2HwMin;
		else if (ui_cur == 1)
			return u2HwDft;
		else if (ui_cur == 2)
			return u2HwMax;
	}
	/*Some qty_item use 3-level setting as min/max/default, don't mapping. */
	if (ui_max == 3) {	/*UI_max=3 means UI option is: off, low, mid, hi. */
		if (ui_cur == 1)
			return u2HwMin;
		else if (ui_cur == 2)
			return u2HwDft;
		else if (ui_cur == 3)
			return u2HwMax;
	}
	/*Get HW value for UI_current > UI_default. */
	if (ui_cur > ui_dft) {
		/*Prevent UI_max, UI_dft miss setting, and div=0. */
		if ((ui_max - ui_dft) <= 0) {
			return u2HwDft;
		} else {
			return (uint16_t) ((ui_cur - ui_dft) * (u2HwMax -
						u2HwDft) / (ui_max - ui_dft) + u2HwDft);
		}
	}
	/*Get HW value for UI_current < UI_default. */
	else {
		/*Prevent UI_dft,  UI_min miss setting, and div=0. */
		if ((ui_dft - ui_min) <= 0) {
			return ui_dft;
		} else {
			return (uint16_t) ((ui_cur - ui_min) * (u2HwDft -
						u2HwMin) / (ui_dft - ui_min) + u2HwMin);
		}
	}
}

/******************************************************************************
*Function: pp_hal_cti_enable(uint8_t b_on_off)
*Description: enable/DisableCTI
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_cti_enable(uint8_t b_on_off)
{
	if (b_on_off > 0)
		_pp_status |= CTI_ON_STATUS;
	else
		_pp_status &= ~CTI_ON_STATUS;

	vRegWt4BMsk(ECTI_00, ((b_on_off > 0) ? ECTI_ENA : 0), ECTI_ENA);
}

/******************************************************************************
*Function: pp_hal_cti_update(void)
*Description: Resetting CTI parameters from quality table
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_cti_update(void)
{

	vRegWt4BMsk(ECTI_00, (read_qty_table(QUALITY_ECTI_CLP_SZ) << 29) |
		    ECTI_HUE_TIE | (read_qty_table(QUALITY_ECTI_VWGT) << 24) |
		    (read_qty_table(QUALITY_ECTI_UWGT) << 20) | ECTI_CLP_ENA |
		    ECTI_VMASKSP | ECTI_PRT_ENA | ECTI_SGN_PRT | ECTI_HD,
		    ECTI_CLP_SZ | ECTI_HUE_TIE | ECTI_VWGT | ECTI_UWGT | ECTI_CLP_ENA | ECTI_FLPF |
		    ECTI_FLPF_SEL | ECTI_Dx_SGN | ECTI_MASK_EN | ECTI_VMASKSP | ECTI_PRT_ENA |
		    ECTI_SGN_PRT | ECTI_HD);
	vRegWt4BMsk(ECTI_01,
		    (read_qty_table(QUALITY_ECTI_LPF3_SEL) << 24) |
		    (read_qty_table(QUALITY_ECTI_LPF2_SEL) << 20) |
		    (read_qty_table(QUALITY_ECTI_LPF1_SEL) << 16) |
		    (read_qty_table(QUALITY_ECTI_FIX_SZ)
		     << 12) | read_qty_table(QUALITY_ECTI_SGN_TH),
		    ECTI_LPF3 | ECTI_LPF3_SEL | ECTI_LPF2 | ECTI_LPF2_SEL | ECTI_LPF1 |
		    ECTI_LPF1_SEL | ECTI_FIX_SZ | ECTI_FIX | ECTI_SEXT_LARGE | ECTI_SEXT | ECTI_SGN
		    | ECTI_SGN_TH);
	vRegWt4BMsk(ECTI_02,
		    (read_qty_table(QUALITY_ECTI_U_STB_OFST2) << 24) |
		    (read_qty_table(QUALITY_ECTI_U_STB_GAIN) << 16) |
		    (read_qty_table(QUALITY_ECTI_U_STB_OFST1) << 8) |
		    (read_qty_table(QUALITY_ECTI_U_WND_OFST) << 4) |
		    read_qty_table(QUALITY_ECTI_U_WND_SZ),
		    ECTI_U_STB_OFST2 | ECTI_U_STB_OFST2 | ECTI_U_STB_OFST1 | ECTI_U_STB_BYPASS |
		    ECTI_U_WND_OFST | ECTI_U_WND_SZ);
	vRegWt4BMsk(ECTI_03,
		    (read_qty_table(QUALITY_ECTI_V_STB_OFST2) << 24) |
		    (read_qty_table(QUALITY_ECTI_V_STB_GAIN) << 16) |
		    (read_qty_table(QUALITY_ECTI_V_STB_OFST1) << 8) |
		    (read_qty_table(QUALITY_ECTI_V_WND_OFST) << 4) |
		    read_qty_table(QUALITY_ECTI_V_WND_SZ),
		    ECTI_V_STB_OFST2 | ECTI_V_STB_OFST2 | ECTI_V_STB_OFST1 | ECTI_V_STB_BYPASS |
		    ECTI_V_WND_OFST | ECTI_V_WND_SZ);
	vRegWt4BMsk(ECTI_04,
		    (read_qty_table(QUALITY_ECTI_FLAT_OFST2) << 24) |
		    (read_qty_table(QUALITY_ECTI_FLAT_GAIN) << 16) |
		    (read_qty_table(QUALITY_ECTI_FLAT_OFST1) << 8) |
		    read_qty_table(QUALITY_ECTI_FLAT_SZ),
		    ECTI_FLAT_OFST2 | ECTI_FLAT_GAIN | ECTI_FLAT_OFST1 | ECTI_FLAT_MODE |
		    ECTI_FLAT_TIE | ECTI_FLAT_BYPASS | ECTI_FLAT_SZ);
	vRegWt4BMsk(ECTI_05, read_qty_table(QUALITY_ECTI_COR), ECTI_COR);
	vRegWt4BMsk(ECTI_06, (read_qty_table(QUALITY_ECTI_V_LMT)
			      << 12) |
		    read_qty_table(QUALITY_ECTI_U_LMT),
		    ECTI_V_LMT_ENA | ECTI_V_LMT | ECTI_U_LMT_ENA | ECTI_U_LMT);
	vRegWt4BMsk(ECTI_07, (0xFFF << 12) | (0 << 0), ECTI_HMSK_END | ECTI_HMSK_START);
	vRegWt4BMsk(ECTI_08, (0xFFF << 12) | (0 << 0), ECTI_VMSK_END | ECTI_VMSK_START);
	vRegWt4BMsk(ECTI_09, (0xFFF << 12) | (0 << 0), ECTI_HDEMO_END | ECTI_HDEMO_START);
	vRegWt4BMsk(ECTI_0A, (0xFFF << 12) | (0 << 0),
		    ECTI_DEMO_SX | ECTI_DEMO_ENA | ECTI_VDEMO_END | ECTI_VDEMO_START);
}

struct POST_CTI_CTRL_PARA pp_hal_get_ctir_param(void)
{
	struct POST_CTI_CTRL_PARA ret_param;
	uint32_t dwTmp;

	dwTmp = u4RegRd4B(ECTI_00);

	ret_param.fgCtiEn = (dwTmp & 0x00000001) ? TRUE : FALSE;
	ret_param.bECTIVwgt = (dwTmp & 0x0F000000) >> 24;
	ret_param.bECTIUwgt = (dwTmp & 0x00F00000) >> 20;
	ret_param.fgECTIFlpfEn = (dwTmp & 0x00001000) >> 12;
	ret_param.bECTIFlpfSel = (dwTmp & 0x00000800) >> 11;
	PP_INFO
	    ("VWEIGHT=0x%x, UWEIGHT=0x%x, CTILpfSel=0x%x, CTILpfEn=%d, CTIenable=%d\n",
	     ret_param.bECTIVwgt, ret_param.bECTIUwgt,
	     ret_param.bECTIFlpfSel, ret_param.fgECTIFlpfEn, ret_param.fgCtiEn);
	return ret_param;
}

void pp_hal_set_ctir_param(struct
			   POST_CTI_CTRL_PARA ret_param)
{

	vRegWt4BMsk(ECTI_00,
		    (ret_param.fgCtiEn ? ECTI_ENA : 0) |
		    (ret_param.bECTIFlpfSel << 11) |
		    (ret_param.fgECTIFlpfEn ? ECTI_FLPF : (0 << 12)) |
		    (ret_param.bECTIVwgt << 24) | (ret_param.bECTIUwgt << 20),
		    ECTI_ENA | ECTI_FLPF_SEL | ECTI_FLPF | ECTI_VWGT | ECTI_UWGT);
	PP_INFO("VWEIGHT=0x%x, UWEIGHT=0x%x, CTILpfSel=0x%x, CTILpfEn=%d, CTIenable=%d\n",
		ret_param.bECTIVwgt, ret_param.bECTIUwgt, ret_param.bECTIFlpfSel,
		ret_param.fgECTIFlpfEn, ret_param.fgCtiEn);
}

/******************************************************************************
*Function: pp_hal_cds_enable(uint8_t b_on_off)
*Description: enable/DisableCDS
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_cds_enable(uint8_t b_on_off)
{

	if (b_on_off > 0)
		_pp_status |= CDS_ON_STATUS;
	else
		_pp_status |= CDS_OFF_STATUS;

	vRegWt4BMsk(ARB_SHP1_01, ((b_on_off > 0) ? 0 : 0), ARB_SHP1_ENABLE);
	vRegWt4BMsk(ARB_SHP2_01, ((b_on_off > 0) ? 0 : 0), ARB_SHP2_ENABLE);
}

/******************************************************************************
*Function: pp_hal_cds_update(void)
*Description: Resetting CDS parameters from quality table
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_cds_update(void)
{

	vRegWt4BMsk(ARB_SHP1_01,
		    (read_qty_table(QUALITY_CDS_GAIN_SIGN_1) << 31) |
		    (read_qty_table(QUALITY_CDS_GAIN_1) << 16) |
		    (read_qty_table(QUALITY_CDS_CORING_1) << 8) | ARB_SHP1_MODE,
		    ARB_SHP1_GAIN_SIGN | ARB_SHP1_GAIN | ARB_SHP1_CORING | ARB_SHP1_MODE);
	vRegWt4BMsk(ARB_SHP1_02,
		    (read_qty_table(QUALITY_CDS_LOW_BOUND_1) << 16) |
		    read_qty_table(QUALITY_CDS_UP_BOUND_1),
		    ARB_SHP1_LOWER_BOUND | ARB_SHP1_UPPER_BOUND);
	vRegWt4BMsk(ARB_SHP1_03, (0x10 << 16) | (0x40 << 0),
		    ARB_SHP1_VALID_RANGE_INV | ARB_SHP1_VALID_RANGE);
	vRegWt4BMsk(ARB_SHP1_04,
			(0x1C8 << 16) | (0x1C8 << 0), ARB_SHP1_CR_CENTER | ARB_SHP1_CB_CENTER);	/*green color */
	vRegWt4BMsk(ARB_SHP2_01,
		    (read_qty_table(QUALITY_CDS_GAIN_SIGN_2) << 31) |
		    (read_qty_table(QUALITY_CDS_GAIN_2) << 16) |
		    (read_qty_table(QUALITY_CDS_CORING_2) << 8) | (ARB_SHP2_MODE),
		    ARB_SHP2_GAIN_SIGN | ARB_SHP2_GAIN | ARB_SHP2_CORING | ARB_SHP2_MODE);
	vRegWt4BMsk(ARB_SHP2_02,
		    (read_qty_table(QUALITY_CDS_LOW_BOUND_2) << 16) |
		    (read_qty_table(QUALITY_CDS_UP_BOUND_2)),
		    ARB_SHP2_LOWER_BOUND | ARB_SHP2_UPPER_BOUND);
	vRegWt4BMsk(ARB_SHP2_03, (0x10 << 16) | (0x40 << 0),
		    ARB_SHP2_VALID_RANGE_INV | ARB_SHP2_VALID_RANGE);
	vRegWt4BMsk(ARB_SHP2_04, (0x07C << 16) | 0x248, ARB_SHP2_CR_CENTER | ARB_SHP2_CB_CENTER);	/*blue color */
}

/******************************************************************************
*Function: struct  POST_SHN_CTRL_PARA pp_hal_get_sharp_ctrl(void)
*Description: Get Sharpness control parameters
*Parameter: void
*Return: struct POST_SHN_CTRL_PARA
******************************************************************************/
struct POST_SHN_CTRL_PARA pp_hal_get_sharp_ctrl(void)
{

	struct POST_SHN_CTRL_PARA ret_param;
	uint32_t dwTmp = u4RegRd4B(TDPROC_24);

	ret_param.fgShnEn = (dwTmp & 0x80000000) ? TRUE : FALSE;
	ret_param.b_limitAllPos = (dwTmp & 0x003FF000) >> 12;
	ret_param.b_limitAllNeg = (dwTmp & 0x000003FF) >> 0;
	return ret_param;
}

/******************************************************************************
*Function: pp_hal_set_sharp_ctrl(struct POST_SHN_CTRL_PARA ctrl_param)
*Description: Set Sharpness control parameters
*Parameter: enum  POST_SHN_CTRL_PARA
*Return: None
******************************************************************************/
void pp_hal_set_sharp_ctrl(struct
			   POST_SHN_CTRL_PARA ctrl_param)
{
	vRegWt4BMsk(TDPROC_24,
		    (ctrl_param.fgShnEn ? TDSHARP_EN : 0) |
		    (ctrl_param.b_limitAllPos << 12) | (ctrl_param.b_limitAllNeg
							<< 0),
		    TDSHARP_EN | TDSHARP_LIMIT_POS_ALL | TDSHARP_LIMIT_NEG_ALL);
}

/******************************************************************************
*Function: pp_get_sharp_param(enum POST_SHN_BAND_ENUM UNEband)
*Description: Get Sharpness band parameters
*Parameter: enum POST_SHN_BAND_ENUM
*Return: struct POST_SHN_BAND_PARA
******************************************************************************/
struct POST_SHN_BAND_PARA pp_get_sharp_param(enum POST_SHN_BAND_ENUM band)
{
	uint32_t dwTmp;
	struct POST_SHN_BAND_PARA ret_param;

	ret_param.eShnBand = band;
	ret_param.b_gain = 0;
	ret_param.bCoring = 0;
	ret_param.b_limitNeg = 0;
	ret_param.b_limitPos = 0;
	ret_param.bClipEn = 0;
	ret_param.bClipThPos = 0;
	ret_param.bClipThNeg = 0;
	ret_param.bSoftCore_gain = 0;
	switch (ret_param.eShnBand) {
	case SHN_BAND_H1:
		dwTmp = u4RegRd4B(TDPROC_00);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_01);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_H2:
		dwTmp = u4RegRd4B(TDPROC_04);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_05);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_V1:

		dwTmp = u4RegRd4B(TDPROC_08);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_09);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_V2:

		dwTmp = u4RegRd4B(TDPROC_0C);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_0D);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_X1:

		dwTmp = u4RegRd4B(TDPROC_10);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_11);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_X2:

		dwTmp = u4RegRd4B(TDPROC_14);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_15);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 6) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_LTI1:
		dwTmp = u4RegRd4B(LTI_00);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(LTI_01);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_LTI2:
		dwTmp = u4RegRd4B(LTI_04);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(LTI_05);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_H1_1:
		dwTmp = u4RegRd4B(TDPROC_20);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_21);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	case SHN_BAND_H2_1:
		dwTmp = u4RegRd4B(TDPROC_34);
		ret_param.b_gain = (dwTmp & 0xFF000000) >> 24;
		ret_param.bCoring = (dwTmp & 0x000000FF) >> 0;
		ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;
		ret_param.b_limitPos = (dwTmp & 0x00FF0000) >> 16;
		dwTmp = u4RegRd4B(TDPROC_35);
		ret_param.bClipEn = ((dwTmp & 0x000000C0) >> 7) > 0 ? 1 : 0;
		ret_param.bClipThPos = (dwTmp & 0xFF000000) >> 24;
		ret_param.bClipThNeg = (dwTmp & 0x00FF0000) >> 16;
		break;
	default:
		break;
	}
	return ret_param;
}

void pp_hal_set_pre_sharp_param(struct
				POST_SHN_PRE_BAND1_PARA band_param)
{

	vRegWt4BMsk(VDO_SHARP_CTRL_01,
		    (band_param.bCoringB1 << 24) |
		    (band_param.b_limitNegB1 << 16) |
		    (band_param.b_limitPosB1 << 8) |
		    (band_param.b_gainB1), CORING_B1 | LIMIT_NEG_B1 | LIMIT_POS_B1 | GAIN_B1);
	vRegWt4BMsk(VDO_SHARP_CTRL_02,
		    /*(1<<23)| */
		    (band_param.bShrinkSelB1 << 20)
		    | (band_param.bClipSelB1 << 18)
		    |
		    (band_param.bClipEnB1
		     << 16) | (band_param.bClipNegB1 << 8) | (band_param.bClipPosB1),
		    /*SHARP_EN_B1| */
		    SHRINK_SEL_B1 | CLIP_SEL_B1 | CLIP_EN_B1 | CLIP_NEG_B1 | CLIP_POS_B1);
	vRegWt4BMsk(VDO_SHARP_CTRL_03, (band_param.bFilterSelB1 << 20) |
		    (band_param.bShift << 18) | (band_param.b_limitNeg << 8) |
		    (band_param.b_limitPos), FILTER_SEL_B1 | SHIFT | LIMIT_NEG | LIMIT_POS);
}

struct POST_SHN_PRE_BAND1_PARA pp_hal_get_pre_sharp_param(void)
{
	/*add for presharpness */

	uint32_t dwTmp;
	struct POST_SHN_PRE_BAND1_PARA ret_param;

	ret_param.bCoringB1 = 0;
	ret_param.b_limitNegB1 = 0;
	ret_param.b_limitPosB1 = 0;
	ret_param.b_gainB1 = 0;
	ret_param.bShrinkSelB1 = 0;
	ret_param.bClipSelB1 = 0;
	ret_param.bClipEnB1 = 0;
	ret_param.bClipNegB1 = 0;
	ret_param.bClipPosB1 = 0;
	ret_param.bFilterSelB1 = 0;
	ret_param.bShift = 0;
	ret_param.b_limitNeg = 0;
	ret_param.b_limitPos = 0;
	/*0x421B0 */
	dwTmp = u4RegRd4B(VDO_SHARP_CTRL_01);
	ret_param.bCoringB1 = (dwTmp & 0xFF000000) >> 24;	/*8bit */
	ret_param.b_limitNegB1 = (dwTmp & 0x00FF0000) >> 16;	/*8bit */
	ret_param.b_limitPosB1 = (dwTmp & 0x0000FF00) >> 8;	/*8bit */
	ret_param.b_gainB1 = (dwTmp & 0x000000FF) >> 0;	/*8bit */
	/*0x421B4 */
	dwTmp = u4RegRd4B(VDO_SHARP_CTRL_02);
	ret_param.bShrinkSelB1 = (dwTmp & 0x00700000) >> 20;	/*3bit */
	ret_param.bClipSelB1 = ((dwTmp & 0x00040000) >> 18) > 0 ? 1 : 0;	/*1bit */
	ret_param.bClipEnB1 = (dwTmp & 0x00030000) >> 16;	/*2bit */
	ret_param.bClipNegB1 = (dwTmp & 0x0000FF00) >> 8;	/*8bit */
	ret_param.bClipPosB1 = (dwTmp & 0x000000FF) >> 0;	/*8bit */
	/*0x421B8 */
	dwTmp = u4RegRd4B(VDO_SHARP_CTRL_03);
	ret_param.bFilterSelB1 = (dwTmp & 0x00100000) >> 20;	/*1bit */
	ret_param.bShift = (dwTmp & 0x000C0000) >> 18;	/*2bit */
	ret_param.b_limitNeg = (dwTmp & 0x0000FF00) >> 8;	/*8bit */
	ret_param.b_limitPos = (dwTmp & 0x000000FF) >> 0;	/*8bit */
	return ret_param;
}

/******************************************************************************
*Function: pp_hal_set_sharp_param(struct POST_SHN_BAND_PARA band_param)
*Description: Set Sharpness band parameters
*Parameter: struct POST_SHN_BAND_PARA
*Return: None
******************************************************************************/
void pp_hal_set_sharp_param(struct
			    POST_SHN_BAND_PARA band_param)
{

	switch (band_param.eShnBand) {
	case SHN_BAND_H1:

		vRegWt4BMsk(TDPROC_00,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN1 | TDSHARP_LIMIT_POS1 | TDSHARP_LIMIT_NEG1 |
			    TDSHARP_CORING1);
		vRegWt4BMsk(TDPROC_01,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS1 | TDSHARP_CLIP_THNEG1 | TDSHARP_ARP_CLIP_EN1);
		break;
	case SHN_BAND_H2:


		vRegWt4BMsk(TDPROC_04,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN2 | TDSHARP_LIMIT_POS2 | TDSHARP_LIMIT_NEG2 |
			    TDSHARP_CORING2);
		vRegWt4BMsk(TDPROC_05,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS2 | TDSHARP_CLIP_THNEG2 | TDSHARP_ARP_CLIP_EN2);
		break;
	case SHN_BAND_V1:


		vRegWt4BMsk(TDPROC_08,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN3 | TDSHARP_LIMIT_POS3 | TDSHARP_LIMIT_NEG3 |
			    TDSHARP_CORING3);
		vRegWt4BMsk(TDPROC_09,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS3 | TDSHARP_CLIP_THNEG3 | TDSHARP_ARP_CLIP_EN3);
		break;
	case SHN_BAND_V2:


		vRegWt4BMsk(TDPROC_0C,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN4 | TDSHARP_LIMIT_POS4 | TDSHARP_LIMIT_NEG4 |
			    TDSHARP_CORING4);
		vRegWt4BMsk(TDPROC_0D,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS4 | TDSHARP_CLIP_THNEG4 | TDSHARP_ARP_CLIP_EN4);
		break;
	case SHN_BAND_X1:

		vRegWt4BMsk(TDPROC_10,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN5 | TDSHARP_LIMIT_POS5 | TDSHARP_LIMIT_NEG5 |
			    TDSHARP_CORING5);
		vRegWt4BMsk(TDPROC_11,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS5 | TDSHARP_CLIP_THNEG5 | TDSHARP_ARP_CLIP_EN5);
		break;
	case SHN_BAND_X2:

		vRegWt4BMsk(TDPROC_14,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_GAIN6 | TDSHARP_LIMIT_POS6 | TDSHARP_LIMIT_NEG6 |
			    TDSHARP_CORING6);
		vRegWt4BMsk(TDPROC_15,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_CLIP_THPOS6 | TDSHARP_CLIP_THNEG6 | TDSHARP_ARP_CLIP_EN6);
		break;
	case SHN_BAND_LTI1:

		vRegWt4BMsk(LTI_00,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    LTI_GAIN1 | LTI_LIMIT_POS1 | LTI_LIMIT_NEG1 | LTI_CORING1);
		vRegWt4BMsk(LTI_01,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    LTI_CLIP_THPOS1 | LTI_CLIP_THNEG1 | LTI_ARP_CLIP_EN1);
		break;
	case SHN_BAND_LTI2:
		vRegWt4BMsk(LTI_04,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    LTI_GAIN2 | LTI_LIMIT_POS2 | LTI_LIMIT_NEG2 | LTI_CORING2);
		vRegWt4BMsk(LTI_05,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    LTI_CLIP_THPOS2 | LTI_CLIP_THNEG2 | LTI_ARP_CLIP_EN2);
		break;
	case SHN_BAND_H1_1:

		vRegWt4BMsk(TDPROC_20,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_H1_1_GAIN | TDSHARP_H1_1_LIMIT_POS | TDSHARP_H1_1_LIMIT_NEG |
			    TDSHARP_H1_1_CORING);
		vRegWt4BMsk(TDPROC_21,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_H1_1_CLIP_THPOS | TDSHARP_H1_1_CLIP_THNEG |
			    TDSHARP_H1_1_CLIP_EN);
		break;
	case SHN_BAND_H2_1:

		vRegWt4BMsk(TDPROC_34,
			    (band_param.b_gain << 24) | (band_param.b_limitPos << 16) |
			    (band_param.b_limitNeg << 8) | (band_param.bCoring),
			    TDSHARP_H2_1_GAIN | TDSHARP_H2_1_LIMIT_POS | TDSHARP_H2_1_LIMIT_NEG |
			    TDSHARP_H2_1_CORING);
		vRegWt4BMsk(TDPROC_35,
			    (band_param.bClipThPos << 24) | (band_param.bClipThNeg << 16) |
			    (band_param.bClipEn << 7),
			    TDSHARP_H2_1_CLIP_THPOS | TDSHARP_H2_1_CLIP_THNEG |
			    TDSHARP_H2_1_CLIP_EN);
		break;
	default:
		break;
	}
}

/******************************************************************************
*Function: pp_hal_sharp_enable(uint8_t b_on_off)
*Description: Set Sharpness level
*Parameter: b_level
*Return: None
******************************************************************************/
void pp_hal_sharp_enable(uint8_t b_on_off)
{

	if (b_on_off > 0)
		_pp_status |= TDSHARP_ON_STATUS;
	else
		_pp_status &= ~TDSHARP_ON_STATUS;

	vRegWt4BMsk(TDPROC_24, (b_on_off > 0) ? TDSHARP_EN : 0, TDSHARP_EN);
}

/******************************************************************************
*Function: pp_hal_update_sharp()
*Description: Set registers for sharpness register
*Parameter: b_level
*Return: None
******************************************************************************/
void pp_hal_update_sharp(void)
{
	/*_gain Setup*/
	vRegWt4BMsk(VDO_SHARP_CTRL_01,
		    (read_qty_table(QUALITY_PRE_SHARP_CORING_B1) << 24) |
		    (read_qty_table(QUALITY_PRE_SHARP_LIMIT_NEG_B1) << 16) |
		    (read_qty_table(QUALITY_PRE_SHARP_LIMIT_POS_B1) << 8) |
		    (read_qty_table(QUALITY_PRE_SHARP_GAIN_B1)),
		    CORING_B1 | LIMIT_NEG_B1 | LIMIT_POS_B1 | GAIN_B1);
	vRegWt4BMsk(VDO_SHARP_CTRL_02,
		    (read_qty_table(QUALITY_PRE_SHARP_SHRINK_SEL_B1) << 20) |
		    (read_qty_table(QUALITY_PRE_SHARP_CLIP_SEL_B1) << 18) |
		    (read_qty_table(QUALITY_PRE_SHARP_CLIP_EN_B1) << 16) |
		    (read_qty_table(QUALITY_PRE_SHARP_CLIP_NEG_B1) << 8) |
		    (read_qty_table(QUALITY_PRE_SHARP_CLIP_POS_B1)) | (1 << 23),
		    SHARP_EN_B1 | SHRINK_SEL_B1 | CLIP_SEL_B1 | CLIP_EN_B1 | CLIP_NEG_B1 |
		    CLIP_POS_B1);
	vRegWt4BMsk(VDO_SHARP_CTRL_03,
		    (read_qty_table(QUALITY_PRE_SHARP_FILTER_SEL_B1) << 20) |
		    (read_qty_table(QUALITY_PRE_SHARP_SHIFT_B1) << 18) |
		    (read_qty_table(QUALITY_PRE_SHARP_PREC_B1) << 16) |
		    (read_qty_table(QUALITY_PRE_SHARP_LIMIT_NEG) << 8) |
		    (read_qty_table(QUALITY_PRE_SHARP_LIMIT_POS) << 0),
		    FILTER_SEL_B1 | SHIFT | PREC_B1 | LIMIT_NEG | LIMIT_POS);
	/*band1 */
	vRegWt4BMsk(TDPROC_00,
		    (read_qty_table(QUALITY_TDSHARP_GAIN1) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING1) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG1) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS1) << 16),
		    TDSHARP_GAIN1 | TDSHARP_CORING1 | TDSHARP_LIMIT_NEG1 | TDSHARP_LIMIT_POS1);
	vRegWt4BMsk(TDPROC_01,
		    (read_qty_table(QUALITY_TDSHARP_CLIP_EN1) << 7) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THPOS1) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THNEG1) << 16) |
		    (TDS_H1_CLIP_BAND_SEL << 0) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_H1_ATT_SEL : TDS_H1_ATT_SEL) <<
		     8) | (TDS_H1_LPF_SEL << 4) | (TDS_H1_CLIP_LC_SEL << 12),
		    TDSHARP_ARP_CLIP_EN1 | TDSHARP_CLIP_THPOS1 | TDSHARP_CLIP_THNEG1 |
		    TDSHARP_CLIP_BAND_SEL1 | TDSHARP_ATTENUATE_SEL1 | TDSHARP_ARP_CLIP_LC_SEL1 |
		    TDSHARP_LPF_SEL1);
	vRegWt4BMsk(TDPROC_02,
		    (read_qty_table(QUALITY_H1_SOFT_COR_GAIN) << 12) | (TDS_H1_PREC << 8) |
		    (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN1) << 0),
		    TDSHARP_H1_SOFT_COR_GAIN | TDSHARP_PREC1 | TDSHARP_SOFT_CLIP_GAIN1);
	vRegWt4BMsk(TDPROC_03, (read_qty_table(QUALITY_TDSHARP_GAIN_NEG1) << 24),
		    TDSHARP_GAIN_NEG1);
	/*band2 */
	vRegWt4BMsk(TDPROC_04,
		    (read_qty_table(QUALITY_TDSHARP_GAIN2) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING2) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG2) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS2) << 16),
		    TDSHARP_GAIN2 | TDSHARP_CORING2 | TDSHARP_LIMIT_NEG2 | TDSHARP_LIMIT_POS2);
	vRegWt4BMsk(TDPROC_05,
		    (read_qty_table(QUALITY_TDSHARP_CLIP_EN2) << 7) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THPOS2) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THNEG2) << 16) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_H2_CLIP_BAND_SEL :
		      TDS_H2_CLIP_BAND_SEL) << 0) |
		    /*(0x08<<0)|//[-1020-1] */
		    (TDS_H2_ATT_SEL << 8) |	/*[-1020-1]T */
		    (TDS_H2_LPF_SEL << 4),
		    TDSHARP_ARP_CLIP_EN2 | TDSHARP_CLIP_THPOS2 | TDSHARP_CLIP_THNEG2 |
		    TDSHARP_CLIP_BAND_SEL2 | TDSHARP_ATTENUATE_SEL2 | TDSHARP_ARP_CLIP_LC_SEL2 |
		    TDSHARP_LPF_SEL2);
	vRegWt4BMsk(TDPROC_06,
		    (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN2) << 0) | (TDS_H2_PREC << 8) |
		    (read_qty_table(QUALITY_H2_SOFT_COR_GAIN) << 12),
		    TDSHARP_H2_SOFT_COR_GAIN | TDSHARP_PREC2 | TDSHARP_SOFT_CLIP_GAIN2);
	vRegWt4BMsk(TDPROC_07, (read_qty_table(QUALITY_TDSHARP_GAIN_NEG2) << 24),
		    TDSHARP_GAIN_NEG2);
	/*band3 */
	vRegWt4BMsk(TDPROC_08,
		    (read_qty_table(QUALITY_TDSHARP_GAIN3) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING3) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG3) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS3) << 16),
		    TDSHARP_GAIN3 | TDSHARP_CORING3 | TDSHARP_LIMIT_NEG3 | TDSHARP_LIMIT_POS3);
	vRegWt4BMsk(TDPROC_09,
		    (read_qty_table(QUALITY_TDSHARP_CLIP_EN3) << 7) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THPOS3) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THNEG3) << 16) |
		    (TDS_V1_CLIP_LC_SEL << 12) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P1080_TDS_V1_CLIP_BAND_SEL :
		      TDS_V1_CLIP_BAND_SEL) << 0) |
		    /*(0x0A<<0)|//[-10-1;  040;  -10-1] */
		    (TDS_V1_ATT_SEL << 8),	/*[-10-1;  040;  -10-1] */
		    TDSHARP_ARP_CLIP_EN3 |
		    TDSHARP_CLIP_THPOS3 |
		    TDSHARP_CLIP_THNEG3 |
		    TDSHARP_CLIP_BAND_SEL3 | TDSHARP_ATTENUATE_SEL3 | TDSHARP_ARP_CLIP_LC_SEL3);
	/*(0x0A<<0)|//[-10-1;  040;  -10-1] */
	/*(0x01<<8), //[-10-1;  040;  -10-1] */
	/*TDSHARP_ARP_CLIP_EN3|TDSHARP_CLIP_THPOS3|TDSHARP_CLIP_THNEG3|
	 *  TDSHARP_CLIP_BAND_SEL3|TDSHARP_ATTENUATE_SEL3);
	 */
	/*vRegWt4BMsk(TDPROC_0A, read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN3),
	 *  TDSHARP_PREC3|TDSHARP_SOFT_CLIP_GAIN3);
	 */
	vRegWt4BMsk(TDPROC_0A, (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN3) << 0) |
		    (read_qty_table(QUALITY_V1_SOFT_COR_GAIN) << 12) |
		    (TDS_V1_PREC << 8),
		    TDSHARP_V1_SOFT_COR_GAIN | TDSHARP_PREC3 | TDSHARP_SOFT_CLIP_GAIN3);
	vRegWt4BMsk(TDPROC_0B, read_qty_table(QUALITY_TDSHARP_GAIN_NEG3) << 24, TDSHARP_GAIN_NEG3);
	/*band4 */
	vRegWt4BMsk(TDPROC_0C,
		    (read_qty_table(QUALITY_TDSHARP_GAIN4) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING4) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG4) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS4) << 16),
		    TDSHARP_GAIN4 | TDSHARP_CORING4 | TDSHARP_LIMIT_NEG4 | TDSHARP_LIMIT_POS4);
	vRegWt4BMsk(TDPROC_0D,
			(read_qty_table(QUALITY_TDSHARP_CLIP_EN4) << 7) |
			(read_qty_table(QUALITY_TDSHARP_CLIP_THPOS4) << 24) |
			(read_qty_table(QUALITY_TDSHARP_CLIP_THNEG4) << 16) |
			(TDS_V2_CLIP_LC_SEL << 12) | (TDS_V2_CLIP_BAND_SEL << 0) |	/*[-10-1;  040;  -10-1] */
		    (TDS_V2_ATT_SEL << 8),	/*[-10-1;  040;  -10-1] */
		    TDSHARP_ARP_CLIP_EN4 | TDSHARP_CLIP_THPOS4 | TDSHARP_CLIP_THNEG4 |
		    TDSHARP_CLIP_BAND_SEL4 | TDSHARP_ATTENUATE_SEL4 | TDSHARP_ARP_CLIP_LC_SEL4);
	vRegWt4BMsk(TDPROC_0E,
		    (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN4) << 0) |
		    (TDS_V2_PREC << 8) |
		    (read_qty_table(QUALITY_V2_SOFT_COR_GAIN) << 12),
		    TDSHARP_V2_SOFT_COR_GAIN | TDSHARP_PREC4 | TDSHARP_SOFT_CLIP_GAIN4);
	vRegWt4BMsk(TDPROC_0F, read_qty_table(QUALITY_TDSHARP_GAIN_NEG4) << 24, TDSHARP_GAIN_NEG4);
	/*band5 */
	vRegWt4BMsk(TDPROC_10,
		    (read_qty_table(QUALITY_TDSHARP_GAIN5) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING5) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG5) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS5) << 16),
		    TDSHARP_GAIN5 | TDSHARP_CORING5 | TDSHARP_LIMIT_NEG5 | TDSHARP_LIMIT_POS5);
	vRegWt4BMsk(TDPROC_11,
		    (read_qty_table(QUALITY_TDSHARP_CLIP_EN5) << 7) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THPOS5) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THNEG5) << 16) |
		    (TDS_X1_CLIP_LC_SEL << 12) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_X1_CLIP_BAND_SEL :
		      TDS_X1_CLIP_BAND_SEL) << 0) |
		    /*(0x07<<0)| */
		    (TDS_X1_ATT_SEL << 8),
		    TDSHARP_ARP_CLIP_EN5 | TDSHARP_CLIP_THPOS5 | TDSHARP_CLIP_THNEG5 |
		    TDSHARP_CLIP_BAND_SEL5 | TDSHARP_ATTENUATE_SEL5 | TDSHARP_ARP_CLIP_LC_SEL5);
	/*(0x07<<0)| */
	/*(0x01<<8), */
	/*TDSHARP_ARP_CLIP_EN5|TDSHARP_CLIP_THPOS5|TDSHARP_CLIP_THNEG5|
	 *TDSHARP_CLIP_BAND_SEL5|TDSHARP_ATTENUATE_SEL5);
	 */
	/*vRegWt4BMsk(TDPROC_12, read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN5),
	 *TDSHARP_PREC5|TDSHARP_SOFT_CLIP_GAIN5);
	 */
	vRegWt4BMsk(TDPROC_12,
		    (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN5) << 0) |
		    (TDS_X1_PREC << 8) |
		    (read_qty_table(QUALITY_X1_SOFT_COR_GAIN) << 12),
		    TDSHARP_X1_SOFT_COR_GAIN | TDSHARP_PREC5 | TDSHARP_SOFT_CLIP_GAIN5);
	vRegWt4BMsk(TDPROC_13, read_qty_table(QUALITY_TDSHARP_GAIN_NEG5) << 24, TDSHARP_GAIN_NEG5);
	/*band6 */
	vRegWt4BMsk(TDPROC_14,
		    (read_qty_table(QUALITY_TDSHARP_GAIN6) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CORING6) << 0) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_NEG6) << 8) |
		    (read_qty_table(QUALITY_TDSHARP_LIMIT_POS6) << 16),
		    TDSHARP_GAIN6 | TDSHARP_CORING6 | TDSHARP_LIMIT_NEG6 | TDSHARP_LIMIT_POS6);
	vRegWt4BMsk(TDPROC_15,
		    (read_qty_table(QUALITY_TDSHARP_CLIP_EN6) << 7) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THPOS6) << 24) |
		    (read_qty_table(QUALITY_TDSHARP_CLIP_THNEG6) << 16) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_X2_CLIP_BAND_SEL :
		      TDS_X2_CLIP_BAND_SEL) << 0) |
		    /*(0x09<<0)| */
		    (TDS_X2_ATT_SEL << 8) | (TDS_X2_CLIP_LC_SEL << 12),
		    TDSHARP_ARP_CLIP_EN6 | TDSHARP_CLIP_THPOS6 | TDSHARP_CLIP_THNEG6 |
		    TDSHARP_CLIP_BAND_SEL6 | TDSHARP_ATTENUATE_SEL6 | TDSHARP_ARP_CLIP_LC_SEL6);
	/*(0x09<<0)| */
	/*(0x01<<8), */
	/*TDSHARP_ARP_CLIP_EN6|TDSHARP_CLIP_THPOS6|TDSHARP_CLIP_THNEG6|
	 *TDSHARP_CLIP_BAND_SEL6|TDSHARP_ATTENUATE_SEL6);
	 */
	/*vRegWt4BMsk(TDPROC_16, */
	/*read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN6), TDSHARP_PREC6|TDSHARP_SOFT_CLIP_GAIN6);  */
	vRegWt4BMsk(TDPROC_16,
		    (read_qty_table(QUALITY_TDSHARP_SOFT_CLIP_GAIN6) << 0) |
		    (TDS_X2_PREC << 8) |
		    (read_qty_table(QUALITY_X1_SOFT_COR_GAIN) << 12),
		    TDSHARP_X2_SOFT_COR_GAIN | TDSHARP_PREC6 | TDSHARP_SOFT_CLIP_GAIN6);
	vRegWt4BMsk(TDPROC_17, read_qty_table(QUALITY_TDSHARP_GAIN_NEG6) << 24, TDSHARP_GAIN_NEG6);
	vRegWt4BMsk(TDPROC_20,
		    (read_qty_table(QUALITY_H1_1_GAIN) << 24) |
		    (read_qty_table(QUALITY_H1_1_CORING) << 0) |
		    (read_qty_table(QUALITY_H1_1_LIMIT_NEG) << 8) |
		    (read_qty_table(QUALITY_H1_1_LIMIT_POS) << 16),
		    TDSHARP_H1_1_GAIN | TDSHARP_H1_1_LIMIT_POS |
		    TDSHARP_H1_1_LIMIT_NEG | TDSHARP_H1_1_CORING);
	vRegWt4BMsk(TDPROC_21,
		    (read_qty_table(QUALITY_H1_1_CLIP_EN) << 7) |
		    (read_qty_table(QUALITY_H1_1_CLIP_THPOS) << 24) |
		    (read_qty_table(QUALITY_H1_1_CLIP_THNEG) << 16) |
		    (TDS_H1_1_LPF_SEL << 4),
		    TDSHARP_H1_1_CLIP_EN | TDSHARP_H1_1_CLIP_THPOS |
		    TDSHARP_H1_1_CLIP_THNEG | TDSHARP_H1_1_LPF_SEL);
	vRegWt4BMsk(TDPROC_22, (read_qty_table(QUALITY_H1_1_SOFT_COR_GAIN) << 12) |
		    (read_qty_table(QUALITY_H1_1_SOFT_CLIP_GAIN) << 0),
		    TDSHARP_H1_1_SOFT_COR_GAIN | TDSHARP_H1_1_SOFT_CLIP_GAIN);
	vRegWt4BMsk(TDPROC_23, read_qty_table(QUALITY_H1_1_GAIN_NEG) << 24, TDSHARP_H1_1_GAIN_NEG);
/*------------------------------------------------------------------------------------------*/
	vRegWt4BMsk(TDPROC_34,
		    (read_qty_table(QUALITY_H2_1_GAIN) << 24) |
		    (read_qty_table(QUALITY_H2_1_CORING) << 0) |
		    (read_qty_table(QUALITY_H2_1_LIMIT_NEG) << 8) |
		    (read_qty_table(QUALITY_H2_1_LIMIT_POS) << 16),
		    TDSHARP_H2_1_GAIN | TDSHARP_H2_1_LIMIT_POS |
		    TDSHARP_H2_1_LIMIT_NEG | TDSHARP_H2_1_CORING);
	vRegWt4BMsk(TDPROC_35,
		    (read_qty_table(QUALITY_H2_1_CLIP_EN) << 7) |
		    (read_qty_table(QUALITY_H2_1_CLIP_THPOS) << 24) |
		    (read_qty_table(QUALITY_H2_1_CLIP_THNEG) << 16) |
		    (TDS_H2_1_FLT_SEL << 8) | (TDS_H2_1_LPF_SEL << 4),
		    TDSHARP_H2_1_CLIP_EN | TDSHARP_H2_1_CLIP_THPOS |
		    TDSHARP_H2_1_CLIP_THNEG | TDSHARP_H2_1_LPF_SEL | TDSHARP_H2_1_FLT_SEL);
	vRegWt4BMsk(TDPROC_36, (read_qty_table(QUALITY_H2_1_SOFT_COR_GAIN) << 12) |
		    (read_qty_table(QUALITY_H2_1_SOFT_CLIP_GAIN) << 0),
		    TDSHARP_H2_1_SOFT_COR_GAIN | TDSHARP_H2_1_SOFT_CLIP_GAIN);
	vRegWt4BMsk(TDPROC_37, read_qty_table(QUALITY_H2_1_GAIN_NEG) << 24, TDSHARP_H2_1_GAIN_NEG);
	vRegWt4BMsk(TDPROC_24,
		    (TDS_TDPROC_BYPASS_ALL << 30) | (TDS_NEG_GAIN_EN << 29) |
		    (TDS_INK_EN << 27) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_CLIP_EN : TDS_CLIP_EN) << 26) |
		    (TDS_CLIP_SEL << 25) | (TDS_CRC2_EN << 24) |
		    (TDS_LTI1_INDEPENDENT << 23) | (TDS_LIMIT_POS_ALL << 12) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_LIMIT_NEG_ALL :
		      TDS_LIMIT_NEG_ALL) << 0),
		    TDPPROC_BYPASS_ALL | TDSHARP_NEG_GAIN_EN | TDSHARP_LTI_EN | TDSHARP_INK_EN |
		    TDSHARP_CLIP_EN | TDSHARP_CLIP_SEL | CRC2_EN | BAND8_INDEPENDENT_CLIP |
		    TDSHARP_LIMIT_POS_ALL | TDSHARP_LIMIT_NEG_ALL);
	vRegWt4BMsk(TDPROC_26,
		    (TDS_MAX_MIN_ATT << 30) | (TDS_MAX_MIN_SEL << 29) | (TDS_X1_FLT_SEL << 28) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_H2_FLT_SEL : TDS_H2_FLT_SEL) <<
		     26) | (TDS_V2_FLT_SEL << 25) | (TDS_LTI2_FLT_SEL << 24) | (TDS_H1_FLT_SEL <<
										23) |
		    (TDS_V1_FLT_SEL << 22) |
		    ((pp_is_video_hd((uint8_t) src_res) ? P720_TDS_SFT : TDS_SFT) << 20) |
		    (TDS_AC_LPF_EN << 18) | (TDS_MAX_MIN_LMT << 0),
		    TDSHARP_MAX_MIN_ATT | TDSHARP_MAX_MIN_SEL | TDSHARP_X1_FLT_SEL |
		    TDSHARP_H2_FLT_SEL | TDSHARP_BAND4_SEL | TDSHARP_BAND9_SEL | TDSHARP_H1_FLT_SEL
		    | TDSHARP_V1_FLT_SEL | TDSHARP_SFT | AC_LPF_EN | TDSHARP_MAX_MIN_LMT);
}

/******************************************************************************
*Function: pp_hal_set_path(uint8_t path)
*Description: Set input source to postprocessor
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_path(uint8_t path)
{

	switch (path) {
	case SV_MAIN:
		/*vRegWtFldMulti(FMTCTRL, MAIN_PATH);  */
		vRegWt4BMsk(FMTCTRL, MAIN_PATH, PATH_MSK);	/*0x3574=0x48 */
		break;
	case SV_PIP:
		/*vRegWtFldMulti(FMTCTRL, PIP_PATH);  */
		vRegWt4BMsk(FMTCTRL, PIP_PATH, PATH_MSK);	/*0x3574=0x86 */
		break;
	case SV_MIXER:
		/*vRegWtFldMulti(FMTCTRL, MIX_PATH);  */
		vRegWt4BMsk(FMTCTRL, MIX_PATH, PATH_MSK);	/*0x3574=0xe */
		break;
	case SV_BYPASS:
		/*vRegWtFldMulti(FMTCTRL, BYPASS_ALL);  */
		vRegWt4BMsk(FMTCTRL, BYPASS_ALL, PATH_MSK);	/*0x3574=0x0 */
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------------------*/
/******************************************************************************
*Function: void pp_hal_color_proc_bypass(uint8_t mode)
*Description: Set  input source to postprocessor
*Parameter: mode
*Return: None
******************************************************************************/
void pp_hal_color_proc_bypass(int16_t mode)
{
	switch (mode) {
	case 0:
		vRegWt4BMsk(CFG_MAIN, (0 << 7), ALLBP);
		_pp_status |= CP_ON_STATUS;
		break;
	case 1:
		vRegWt4BMsk(CFG_MAIN, (1 << 7), ALLBP);
		_pp_status &= ~CP_OFF_STATUS;
		break;
	default:
		break;
	}
}

void pp_hal_set_sharp_lti_band_param(int16_t mode, int16_t _gain)
{

	switch (mode) {
	case SHN_BAND_H1:

		vRegWt4BMsk(TDPROC_00, (_gain << 24), TDSHARP_GAIN1
			    /*|TDSHARP_LIMIT_POS1|TDSHARP_LIMIT_NEG1|TDSHARP_CORING1 */
		    );
		/*vRegWt4BMsk(TDPROC_01, (_gain<<24),
		 *TDSHARP_CLIP_THPOS1|TDSHARP_CLIP_THNEG1|TDSHARP_ARP_CLIP_EN1);
		 */
		break;
	case SHN_BAND_H2:


		vRegWt4BMsk(TDPROC_04, (_gain << 24), TDSHARP_GAIN2
			    /*|TDSHARP_LIMIT_POS2|TDSHARP_LIMIT_NEG2|TDSHARP_CORING2 */
		    );
		/*vRegWt4BMsk(TDPROC_05, (_gain<<24),
		 *TDSHARP_CLIP_THPOS2|TDSHARP_CLIP_THNEG2|TDSHARP_ARP_CLIP_EN2);
		 */
		break;
	case SHN_BAND_V1:


		vRegWt4BMsk(TDPROC_08, (_gain << 24), TDSHARP_GAIN3
			    /*|TDSHARP_LIMIT_POS3|TDSHARP_LIMIT_NEG3|TDSHARP_CORING3 */
		    );
		/*vRegWt4BMsk(TDPROC_09, (_gain<<24),
		 *TDSHARP_CLIP_THPOS3|TDSHARP_CLIP_THNEG3|TDSHARP_ARP_CLIP_EN3);
		 */
		break;
	case SHN_BAND_V2:


		vRegWt4BMsk(TDPROC_0C, (_gain << 24), TDSHARP_GAIN4
			    /*|TDSHARP_LIMIT_POS4|TDSHARP_LIMIT_NEG4|TDSHARP_CORING4 */
		    );
		/*vRegWt4BMsk(TDPROC_0D, (_gain<<24),
		 *TDSHARP_CLIP_THPOS4|TDSHARP_CLIP_THNEG4|TDSHARP_ARP_CLIP_EN4);
		 */
		break;
	case SHN_BAND_X1:

		vRegWt4BMsk(TDPROC_10, (_gain << 24), TDSHARP_GAIN5
			    /*|TDSHARP_LIMIT_POS5|TDSHARP_LIMIT_NEG5|TDSHARP_CORING5 */
		    );
		/*vRegWt4BMsk(TDPROC_11, (_gain<<24),
		 *TDSHARP_CLIP_THPOS5|TDSHARP_CLIP_THNEG5|TDSHARP_ARP_CLIP_EN5);
		 */
		break;
	case SHN_BAND_X2:

		vRegWt4BMsk(TDPROC_14, (_gain << 24), TDSHARP_GAIN6
			    /*|TDSHARP_LIMIT_POS6|TDSHARP_LIMIT_NEG6|TDSHARP_CORING6 */
		    );
		/*vRegWt4BMsk(TDPROC_15, (_gain<<24),
		 *TDSHARP_CLIP_THPOS6|TDSHARP_CLIP_THNEG6|TDSHARP_ARP_CLIP_EN6);
		 */
		break;
	case SHN_BAND_LTI1:

		vRegWt4BMsk(LTI_00, (_gain << 24), LTI_GAIN1
			    /*|LTI_LIMIT_POS1|LTI_LIMIT_NEG1|LTI_CORING1 */
		    );
		/*vRegWt4BMsk(LTI_01,
		 *  (band_param.bClipThPos<<24)|
		 *  (band_param.bClipThNeg<<16)|
		 *  (band_param.bClipEn<<7),
		 *  LTI_CLIP_THPOS1|LTI_CLIP_THNEG1|LTI_ARP_CLIP_EN1);
		 */
		break;
	case SHN_BAND_LTI2:
		vRegWt4BMsk(LTI_04, (_gain << 24) | (0x00 << 16) | (0x00 << 8) | (0x00),
			    LTI_GAIN2 | LTI_LIMIT_POS2 | LTI_LIMIT_NEG2 | LTI_CORING2);
		vRegWt4BMsk(LTI_05, (0x00 << 24) | (0x00 << 16) | (0x00 << 7),
			    LTI_CLIP_THPOS2 | LTI_CLIP_THNEG2 | LTI_ARP_CLIP_EN2);
		break;
	case SHN_BAND_H1_1:

		vRegWt4BMsk(TDPROC_20, (_gain << 24), TDSHARP_H1_1_GAIN
			    /*|LTI_LIMIT_POS1|LTI_LIMIT_NEG1|LTI_CORING1 */
		    );
		/*vRegWt4BMsk(LTI_01,
		 *  (band_param.bClipThPos<<24)|
		 *  (band_param.bClipThNeg<<16)|
		 *  (band_param.bClipEn<<7),
		 *  LTI_CLIP_THPOS1|LTI_CLIP_THNEG1|LTI_ARP_CLIP_EN1);
		 */
		break;
	case SHN_BAND_H2_1:
		vRegWt4BMsk(TDPROC_34, (_gain << 24)
			    /*|(0x00<<16)|(0x00<<8)|(0x00) */
			    , TDSHARP_H2_1_GAIN
			    /*|LTI_LIMIT_POS2|LTI_LIMIT_NEG2|LTI_CORING2 */
		    );
		break;
	default:
		break;
	}
	if ((mode > 0) && (mode < 9)) {
		vRegWt4BMsk(TDPROC_24, TDSHARP_EN, TDSHARP_EN);
		vRegWt4BMsk(TDPROC_24, TDSHARP_LTI_EN, TDSHARP_LTI_EN);
	} else {
		vRegWt4BMsk(TDPROC_24, ~TDSHARP_EN, TDSHARP_EN);
		vRegWt4BMsk(TDPROC_24, ~TDSHARP_LTI_EN, TDSHARP_LTI_EN);
	}
	PP_INFO("Band=%d, _gain=%d\n", mode, _gain);
}

void pp_hal_set_sharp_band1_param(int8_t _gain, bool enable)
{
	vRegWt4BMsk(VDO_SHARP_CTRL_01,
		/*(read_qty_table(QUALITY_PRE_SHARP_CORING_B1)<<24)|
		 *(read_qty_table(QUALITY_PRE_SHARP_LIMIT_NEG_B1)<<16)|
		 *(read_qty_table(QUALITY_PRE_SHARP_LIMIT_POS_B1)<<8)|
		 */
		(_gain << 0),
		    /*CORING_B1|LIMIT_NEG_B1|LIMIT_POS_B1| */
			GAIN_B1);
	vRegWt4BMsk(VDO_SHARP_CTRL_02, (enable << 23), SHARP_EN_B1);
}


/*--------------------------------------------------------------------------------------*/
/******************************************************************************
*Function: pp_hal_update_main_con(void)
*Description: Set global contrast
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_update_main_con(uint8_t value)
{

	/*vRegWtFldAlign(G_PIC_ADJ_MAIN, value, CONT);  */
	vRegWt4BMsk(G_PIC_ADJ_MAIN, value, CONT);
}

/******************************************************************************
*Function: pp_hal_update_main_bri(void)
*Description: Set global brightness
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_update_main_bri(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(G_PIC_ADJ_MAIN, u1Tmp, BRI);  */
	vRegWt4BMsk(G_PIC_ADJ_MAIN, u1Tmp << 8, BRI);
}

/******************************************************************************
*Function: pp_hal_update_main_sat(void)
*Description: Set global saturation
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_update_main_sat(uint8_t value)
{

	/*vRegWtFldAlign(G_PIC_ADJ_MAIN, value, SAT);  */
	vRegWt4BMsk(G_PIC_ADJ_MAIN, value << 16, SAT);
}

/******************************************************************************
*Function: pp_hal_update_main_hue(void)
*Description: Set global hue
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_update_main_hue(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(G_PIC_ADJ_MAIN, (((value&0x80)>>7)?0:1), HUE_SIGN);  */
	/*vRegWtFldAlign(G_PIC_ADJ_MAIN, u1Tmp, HUE);  */
	vRegWt4BMsk(G_PIC_ADJ_MAIN, u1Tmp << 24, HUE);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_y2(uint8_t value)
*Description: Set Magenta luma2
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_y2(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_3_2_MAIN, u1Tmp, Y_SLOPE_3_2_MAIN_VALUE_2);  */
	vRegWt4BMsk(Y_SLOPE_3_2_MAIN, u1Tmp, Y_SLOPE_3_2_MAIN_VALUE_2);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_y3(uint8_t value)
*Description: Set Magenta luma3
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_y3(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_3_2_MAIN, u1Tmp, Y_SLOPE_3_2_MAIN_VALUE_3);  */
	vRegWt4BMsk(Y_SLOPE_3_2_MAIN, u1Tmp << 16, Y_SLOPE_3_2_MAIN_VALUE_3);
}

/******************************************************************************
*Function: pp_hal_set_scered_y4(uint8_t value)
*Description: Set Red luma4
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scered_y4(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_5_4_MAIN, u1Tmp, Y_SLOPE_5_4_MAIN_VALUE_4);  */
	vRegWt4BMsk(Y_SLOPE_5_4_MAIN, u1Tmp, Y_SLOPE_5_4_MAIN_VALUE_4);
}

/******************************************************************************
*Function: pp_hal_set_scered_y5(uint8_t value)
*Description: Set Red luma5
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scered_y5(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_5_4_MAIN, u1Tmp, Y_SLOPE_5_4_MAIN_VALUE_5);  */
	vRegWt4BMsk(Y_SLOPE_5_4_MAIN, u1Tmp << 16, Y_SLOPE_5_4_MAIN_VALUE_5);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_y7(uint8_t value)
*Description: Set Yellow luma7
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_y7(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_7_6_MAIN, u1Tmp, Y_SLOPE_7_6_MAIN_VALUE_7);  */
	vRegWt4BMsk(Y_SLOPE_7_6_MAIN, u1Tmp << 16, Y_SLOPE_7_6_MAIN_VALUE_7);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_y8(uint8_t value)
*Description: Set Yellow luma8
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_y8(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_9_8_MAIN, u1Tmp, Y_SLOPE_9_8_MAIN_VALUE_8);  */
	vRegWt4BMsk(Y_SLOPE_9_8_MAIN, u1Tmp, Y_SLOPE_9_8_MAIN_VALUE_8);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_y10(uint8_t value)
*Description: Set Green luma10
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_y10(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_11_10_MAIN, u1Tmp, Y_SLOPE_11_10_MAIN_VALUE_10);  */
	vRegWt4BMsk(Y_SLOPE_11_10_MAIN, u1Tmp, Y_SLOPE_11_10_MAIN_VALUE_10);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_y11(uint8_t value)
*Description: Set Green luma11
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_y11(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_11_10_MAIN, u1Tmp, Y_SLOPE_11_10_MAIN_VALUE_11);  */
	vRegWt4BMsk(Y_SLOPE_11_10_MAIN, u1Tmp << 16, Y_SLOPE_11_10_MAIN_VALUE_11);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_y12(uint8_t value)
*Description: Set Cyan luma12
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_y12(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_13_12_MAIN, u1Tmp, Y_SLOPE_13_12_MAIN_VALUE_12);  */
	vRegWt4BMsk(Y_SLOPE_13_12_MAIN, u1Tmp, Y_SLOPE_13_12_MAIN_VALUE_12);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_y13(uint8_t value)
*Description: Set Cyan luma13
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_y13(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_13_12_MAIN, u1Tmp, Y_SLOPE_13_12_MAIN_VALUE_13);  */
	vRegWt4BMsk(Y_SLOPE_13_12_MAIN, u1Tmp << 16, Y_SLOPE_13_12_MAIN_VALUE_13);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_y14(uint8_t value)
*Description: Set Blue luma14
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_y14(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_15_14_MAIN, u1Tmp, Y_SLOPE_15_14_MAIN_VALUE_14);  */
	vRegWt4BMsk(Y_SLOPE_15_14_MAIN, u1Tmp, Y_SLOPE_15_14_MAIN_VALUE_14);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_y15(uint8_t value)
*Description: Set Blue luma15
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_y15(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x80)
		u1Tmp = (value & 0x7F);
	else
		u1Tmp = (0x7F - (value & 0x7F)) | 0x80;

	/*vRegWtFldAlign(Y_SLOPE_15_14_MAIN, u1Tmp, Y_SLOPE_15_14_MAIN_VALUE_15);  */
	vRegWt4BMsk(Y_SLOPE_15_14_MAIN, u1Tmp << 16, Y_SLOPE_15_14_MAIN_VALUE_15);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_s2(void)
*Description: Set Magenta Sat2
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_s2(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_3_2_MAIN, value, S_G2_3_2_MAIN_VALUE_2);  */
	vRegWt4BMsk(S_G2_3_2_MAIN, value, S_G2_3_2_MAIN_VALUE_2);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_s3(void)
*Description: Set Magenta Sat3
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_s3(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_3_2_MAIN, value, S_G2_3_2_MAIN_VALUE_3);  */
	vRegWt4BMsk(S_G2_3_2_MAIN, value << 16, S_G2_3_2_MAIN_VALUE_3);
}

/******************************************************************************
*Function: pp_hal_set_scered_s4(void)
*Description: Set Red Sat4
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scered_s4(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_5_4_MAIN, value, S_G2_5_4_MAIN_VALUE_4);  */
	vRegWt4BMsk(S_G2_5_4_MAIN, value, S_G2_5_4_MAIN_VALUE_4);
}

/******************************************************************************
*Function: pp_hal_set_scered_s5(void)
*Description: Set Red Sat5
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scered_s5(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_5_4_MAIN, value, S_G2_5_4_MAIN_VALUE_5);  */
	vRegWt4BMsk(S_G2_5_4_MAIN, value << 16, S_G2_5_4_MAIN_VALUE_5);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_s7(void)
*Description: Set Yellow Sat7
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_s7(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_7_6_MAIN, value, S_G2_7_6_MAIN_VALUE_7);  */
	vRegWt4BMsk(S_G2_7_6_MAIN, value << 16, S_G2_7_6_MAIN_VALUE_7);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_s8(void)
*Description: Set Yellow Sat8
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_s8(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_9_8_MAIN, value, S_G2_9_8_MAIN_VALUE_8);  */
	vRegWt4BMsk(S_G2_9_8_MAIN, value, S_G2_9_8_MAIN_VALUE_8);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_s10(void)
*Description: Set Green Sat10
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_s10(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_11_10_MAIN, value, S_G2_11_10_MAIN_VALUE_10);  */
	vRegWt4BMsk(S_G2_11_10_MAIN, value, S_G2_11_10_MAIN_VALUE_10);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_s11(void)
*Description: Set Green Sat11
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_s11(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_11_10_MAIN, value, S_G2_11_10_MAIN_VALUE_11);  */
	vRegWt4BMsk(S_G2_11_10_MAIN, value << 16, S_G2_11_10_MAIN_VALUE_11);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_s12(void)
*Description: Set Cyan Sat12
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_s12(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_13_12_MAIN, value, S_G2_13_12_MAIN_VALUE_12);  */
	vRegWt4BMsk(S_G2_13_12_MAIN, value, S_G2_13_12_MAIN_VALUE_12);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_s13(void)
*Description: Set Cyan Sat13
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_s13(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_13_12_MAIN, value, S_G2_13_12_MAIN_VALUE_13);  */
	vRegWt4BMsk(S_G2_13_12_MAIN, value << 16, S_G2_13_12_MAIN_VALUE_13);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_s14(void)
*Description: Set Blue Sat14
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_s14(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_15_14_MAIN, value, S_G2_15_14_MAIN_VALUE_14);  */
	vRegWt4BMsk(S_G2_15_14_MAIN, value, S_G2_15_14_MAIN_VALUE_14);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_s15(void)
*Description: Set Blue Sat15
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_s15(uint8_t value)
{

	/*vRegWtFldAlign(S_G2_15_14_MAIN, value, S_G2_15_14_MAIN_VALUE_15);  */
	vRegWt4BMsk(S_G2_15_14_MAIN, value << 16, S_G2_15_14_MAIN_VALUE_15);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_h2(uint8_t value)
*Description: Set Magenta Hue2
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_h2(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_3_2_MAIN, u1Tmp, H_FTN_3_2_MAIN_VALUE_2);  */
	vRegWt4BMsk(H_FTN_3_2_MAIN, u1Tmp, H_FTN_3_2_MAIN_VALUE_2);
}

/******************************************************************************
*Function: pp_hal_set_scemagenta_h3(uint8_t value)
*Description: Set Magenta Hue3
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scemagenta_h3(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_3_2_MAIN, u1Tmp, H_FTN_3_2_MAIN_VALUE_3);  */
	vRegWt4BMsk(H_FTN_3_2_MAIN, u1Tmp << 16, H_FTN_3_2_MAIN_VALUE_3);
}

/******************************************************************************
*Function: pp_hal_set_scered_h4(uint8_t value)
*Description: Set Red Hue4
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scered_h4(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_5_4_MAIN, u1Tmp, H_FTN_5_4_MAIN_VALUE_4);  */
	vRegWt4BMsk(H_FTN_5_4_MAIN, u1Tmp, H_FTN_5_4_MAIN_VALUE_4);
}

/******************************************************************************
*Function: pp_hal_set_scered_h5(uint8_t value)
*Description: Set Red Hue5
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scered_h5(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_5_4_MAIN, u1Tmp, H_FTN_5_4_MAIN_VALUE_5);  */
	vRegWt4BMsk(H_FTN_5_4_MAIN, u1Tmp << 16, H_FTN_5_4_MAIN_VALUE_5);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_h7(uint8_t value)
*Description: Set Yellow Hue7
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_h7(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_7_6_MAIN, u1Tmp, H_FTN_7_6_MAIN_VALUE_7);  */
	vRegWt4BMsk(H_FTN_7_6_MAIN, u1Tmp << 16, H_FTN_7_6_MAIN_VALUE_7);
}

/******************************************************************************
*Function: pp_hal_set_sceyellow_h8(uint8_t value)
*Description: Set Yellow Hue8
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceyellow_h8(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_9_8_MAIN, u1Tmp, H_FTN_9_8_MAIN_VALUE_8);  */
	vRegWt4BMsk(H_FTN_9_8_MAIN, u1Tmp, H_FTN_9_8_MAIN_VALUE_8);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_h10(uint8_t value)
*Description: Set Green Hue10
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_h10(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_11_10_MAIN, u1Tmp, H_FTN_11_10_MAIN_VALUE_10);  */
	vRegWt4BMsk(H_FTN_11_10_MAIN, u1Tmp, H_FTN_11_10_MAIN_VALUE_10);
}

/******************************************************************************
*Function: pp_hal_set_scegreen_h11(uint8_t value)
*Description: Set Green Hue11
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scegreen_h11(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_11_10_MAIN, u1Tmp, H_FTN_11_10_MAIN_VALUE_11);  */
	vRegWt4BMsk(H_FTN_11_10_MAIN, u1Tmp << 16, H_FTN_11_10_MAIN_VALUE_11);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_h12(uint8_t value)
*Description: Set Cyan Hue12
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_h12(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_13_12_MAIN, u1Tmp, H_FTN_13_12_MAIN_VALUE_12);  */
	vRegWt4BMsk(H_FTN_13_12_MAIN, u1Tmp, H_FTN_13_12_MAIN_VALUE_12);
}

/******************************************************************************
*Function: pp_hal_set_scecyan_h13(uint8_t value)
*Description: Set Cyan Hue13
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_scecyan_h13(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_13_12_MAIN, u1Tmp, H_FTN_13_12_MAIN_VALUE_13);  */
	vRegWt4BMsk(H_FTN_13_12_MAIN, u1Tmp << 16, H_FTN_13_12_MAIN_VALUE_13);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_h14(uint8_t value)
*Description: Set Blue Hue14
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_h14(uint8_t value)
{

	uint8_t u1Tmp;

	if (value & 0x40)
		u1Tmp = (value & 0x3F);
	else
		u1Tmp = (0x3F - (value & 0x3F)) | 0x40;

/*vRegWtFldAlign(H_FTN_15_14_MAIN, u1Tmp, H_FTN_15_14_MAIN_VALUE_14);  */
	vRegWt4BMsk(H_FTN_15_14_MAIN, u1Tmp, H_FTN_15_14_MAIN_VALUE_14);
}

/******************************************************************************
*Function: pp_hal_set_sceblue_H15(uint8_t value)
*Description: Set Blue Hue15
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_set_sceblue_H15(uint8_t value)
{

	uint8_t tmp;

	if (value & 0x40)
		tmp = (value & 0x3F);
	else
		tmp = (0x3F - (value & 0x3F)) | 0x40;

	/*vRegWtFldAlign(H_FTN_15_14_MAIN, u1Tmp, H_FTN_15_14_MAIN_VALUE_15);  */
	vRegWt4BMsk(H_FTN_15_14_MAIN, tmp << 16, H_FTN_15_14_MAIN_VALUE_15);
}

/******************************************************************************
*Function: pp_hal_main_sec_on_off(void)
*Description: enable/Disable SCE
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_main_sec_on_off(uint8_t b_on_off)
{

	if (b_on_off > 0)
		_pp_status |= SCE_ON_STATUS;
	else
		_pp_status |= SCE_OFF_STATUS;

	/*vRegWtFldAlign(CFG_MAIN, ((b_on_off>0)?0:7), YSHBP);  */
	vRegWt4BMsk(CFG_MAIN, ((b_on_off > 0) ? 0 : 7) << 2, YSHBP);
}


/******************************************************************************
*Function: pp_hal_main_video_mode(uint8_t mode)
*Description: Set video mode
*Parameter: mode=0->STDmode
*Return: None
******************************************************************************/
void pp_hal_main_video_mode(uint8_t mode)
{

	switch (mode) {
	case 0:
	default:
		pp_hal_update_main_bri(read_qty_table(QUALITY_BRIGHTNESS_STD));
		pp_hal_update_main_con(read_qty_table(QUALITY_CONTRAST_STD));
		pp_hal_update_main_sat(read_qty_table(QUALITY_SATURATION_STD));
		pp_hal_update_main_hue(read_qty_table(QUALITY_HUE_STD));
		break;
	case 1:
		pp_hal_update_main_bri(read_qty_table(QUALITY_BRIGHTNESS_VIVID));
		pp_hal_update_main_con(read_qty_table(QUALITY_CONTRAST_VIVID));
		pp_hal_update_main_sat(read_qty_table(QUALITY_SATURATION_VIVID));
		pp_hal_update_main_hue(read_qty_table(QUALITY_HUE_VIVID));
		break;
	case 2:
		pp_hal_update_main_bri(read_qty_table(QUALITY_BRIGHTNESS_CINEMA));
		pp_hal_update_main_con(read_qty_table(QUALITY_CONTRAST_CINEMA));
		pp_hal_update_main_sat(read_qty_table(QUALITY_SATURATION_CINEMA));
		pp_hal_update_main_hue(read_qty_table(QUALITY_HUE_CINEMA));
		break;
	case 3:		/*CUSTOMER do notthing */
		break;
	}
}

/******************************************************************************
*Function: pp_hal_set_sce_table(enum POST_VIDEO_MODE_ENUM emode)
*Description: load SCE table
*Parameter: emode
*Return: None
******************************************************************************/
void pp_hal_set_sce_table(enum
			  POST_VIDEO_MODE_ENUM emode)
{

	uint8_t bi;
	uint16_t w_offset = 0;

	switch (emode) {
	case POST_VIDEO_MODE_STD:
	case POST_VIDEO_MODE_CUSTOMER:
	default:
		for (bi = 0; bi < 16; bi++) {
			vRegWt1B(Y_SLOPE_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_G1_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_G2_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_G3_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_P1_1_0_MAIN + w_offset, 0x20);
			vRegWt1B(S_P2_1_0_MAIN + w_offset, 0x40);
			vRegWt1B(S_Y0_G_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_Y64_G_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_Y128_G_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_Y192_G_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(S_Y256_G_1_0_MAIN + w_offset, 0x80);
			vRegWt1B(H_FTN_1_0_MAIN + w_offset, 0x00);
			w_offset += 2;
		}
		break;
	case POST_VIDEO_MODE_VIVID:
		if (_vivid_mode_tbl == NULL)
			return;

		w_offset = 0;
		for (bi = 0; bi < 16; bi++) {
			vRegWt1B(Y_SLOPE_1_0_MAIN + w_offset, _vivid_mode_tbl[Y_SLOPE_MAIN][bi]);	/*Y_Slope */
			vRegWt1B(S_G1_1_0_MAIN + w_offset, _vivid_mode_tbl[S_G1_MAIN][bi]);	/*S__gain1 */
			vRegWt1B(S_G2_1_0_MAIN + w_offset, _vivid_mode_tbl[S_G2_MAIN][bi]);	/*S__gain2 */
			vRegWt1B(S_G3_1_0_MAIN + w_offset, _vivid_mode_tbl[S_G3_MAIN][bi]);	/*S__gain3 */
			vRegWt1B(S_P1_1_0_MAIN + w_offset, _vivid_mode_tbl[S_P1_MAIN][bi]);	/*S_P1 */
			vRegWt1B(S_P2_1_0_MAIN + w_offset, _vivid_mode_tbl[S_P2_MAIN][bi]);	/*S_P2 */
			vRegWt1B(S_Y0_G_1_0_MAIN + w_offset, _vivid_mode_tbl[S_Y0_G_MAIN][bi]);	/*S_Y0 */
			vRegWt1B(S_Y64_G_1_0_MAIN + w_offset, _vivid_mode_tbl[S_Y64_G_MAIN][bi]);	/*S_Y64 */
			vRegWt1B(S_Y128_G_1_0_MAIN + w_offset, _vivid_mode_tbl[S_Y128_G_MAIN][bi]);	/*S_Y128 */
			vRegWt1B(S_Y192_G_1_0_MAIN + w_offset, _vivid_mode_tbl[S_Y192_G_MAIN][bi]);	/*S_Y192 */
			vRegWt1B(S_Y256_G_1_0_MAIN + w_offset, _vivid_mode_tbl[S_Y256_G_MAIN][bi]);	/*S_Y256 */
			vRegWt1B(H_FTN_1_0_MAIN + w_offset, _vivid_mode_tbl[H_FTN_MAIN][bi]);	/*H_FTN */
			w_offset += 2;
		}

		break;
	case POST_VIDEO_MODE_CINEMA:
		if (_cinema_mode_tbl == NULL)
			return;

		w_offset = 0;
		for (bi = 0; bi < 16; bi++) {
			vRegWt1B(Y_SLOPE_1_0_MAIN + w_offset, _cinema_mode_tbl[Y_SLOPE_MAIN][bi]);	/*Y_Slope */
			vRegWt1B(S_G1_1_0_MAIN + w_offset, _cinema_mode_tbl[S_G1_MAIN][bi]);	/*S__gain1 */
			vRegWt1B(S_G2_1_0_MAIN + w_offset, _cinema_mode_tbl[S_G2_MAIN][bi]);	/*S__gain2 */
			vRegWt1B(S_G3_1_0_MAIN + w_offset, _cinema_mode_tbl[S_G3_MAIN][bi]);	/*S__gain3 */
			vRegWt1B(S_P1_1_0_MAIN + w_offset, _cinema_mode_tbl[S_P1_MAIN][bi]);	/*S_P1 */
			vRegWt1B(S_P2_1_0_MAIN + w_offset, _cinema_mode_tbl[S_P2_MAIN][bi]);	/*S_P2 */
			vRegWt1B(S_Y0_G_1_0_MAIN + w_offset, _cinema_mode_tbl[S_Y0_G_MAIN][bi]);	/*S_Y0 */
			vRegWt1B(S_Y64_G_1_0_MAIN + w_offset, _cinema_mode_tbl[S_Y64_G_MAIN][bi]);	/*S_Y64 */
			vRegWt1B(S_Y128_G_1_0_MAIN + w_offset, _cinema_mode_tbl[S_Y128_G_MAIN][bi]);	/*S_Y128 */
			vRegWt1B(S_Y192_G_1_0_MAIN + w_offset, _cinema_mode_tbl[S_Y192_G_MAIN][bi]);	/*S_Y192 */
			vRegWt1B(S_Y256_G_1_0_MAIN + w_offset, _cinema_mode_tbl[S_Y256_G_MAIN][bi]);	/*S_Y256 */
			vRegWt1B(H_FTN_1_0_MAIN + w_offset, _cinema_mode_tbl[H_FTN_MAIN][bi]);	/*H_FTN */
			w_offset += 2;
		}
		break;
	}
}

/******************************************************************************
*Function: Adaptive Luma Part
******************************************************************************/

/******************************************************************************
*Function: pp_get_adaptive_info(uint8_t bindex)
*Description: get adaptive info for debug
*Parameter: None
*Return: None
******************************************************************************/

void pp_get_adaptive_info(void)
{

	PP_INFO
	    ("\nbBS_on_off%dbBS_level%dbBS_gain%dbBS_offset%dbBS_ratio%dbBS_limit%d\n",
	     _bs_on_off, _bs_level, _bs_gain, _bs_offset, _bs_ratio, _bs_limit);
	PP_INFO
	    ("bWS_on_off%dbWS_level%dbWS_gain%dbWS_offset%dbWS_ratio%dbWS_limit%d\n",
	     _ws_on_off, _ws_level, _ws_gain, _ws_offset, _ws_ratio, _ws_limit);
	PP_INFO
	    ("bAdapLuma_gain%dbAdapLuma_offset%dbAdapLuma_limit%d\n",
	     _adap_luma_gain, _adap_luma_offset, _adap_luma_limit);
	PP_INFO("\n");
}


/******************************************************************************
*Function: pp_build_con_curve_dft(uint16_t *curve)
*Description: create default curve
*Parameter: pointer of curve
*Return: None
******************************************************************************/
void pp_build_con_curve_dft(uint16_t *curve)
{

	uint8_t bi;

	if (curve == NULL)
		return;

#ifdef NEW_LUMA_ALGORITHM
	if ((_dynamic_state >= LUMA_RESET_AL2)
	    && (_dynamic_state < LUMA_RESET_AL1)) {
		for (bi = 0; bi < LUMA_HIST_NUM; bi++)
			curve[bi] = (uint16_t) bi << 5;	/*0326496...960992 */
		curve[LUMA_HIST_NUM] = 1023;
	} else {
		curve[0] = 0;
		curve[1] = 25;
		curve[2] = 50;
		for (bi = 3; bi < (LUMA_HIST_NUM - 2); bi++)
			curve[bi] = curve[bi - 1] + 33;
		curve[30] = 974;
		curve[31] = 999;
		curve[32] = 1023;
	}
#else
	for (bi = 0; bi < LUMA_HIST_NUM; bi++)
		curve[bi] = (uint16_t) bi << 5;	/*0326496...960992 */

	curve[LUMA_HIST_NUM] = 1023;
#endif
}

/******************************************************************************
*Function: pp_build_con_curve_dyn(const uint32_t *p_hist_cur, uint16_t *p_dync_curve)
*Description: Build dynamic curve base on the adaptive gain
*Parameter: current histogram, pointer of dynamic curve
*Return: None
******************************************************************************/
void pp_build_con_curve_dyn(const uint32_t *p_hist_cur, uint16_t *p_dync_curve)
{

#ifdef NEW_LUMA_ALGORITHM
	uint8_t bi, b_gainH, b_gainM, b_gainL;
	uint8_t bStart;

	if ((p_dync_curve == NULL)
	    || (p_hist_cur == NULL)) {
		return;
	}

	pp_build_con_curve_dft(p_dync_curve);
	if ((_dynamic_state >= LUMA_RESET_AL2)
	    && (_dynamic_state < LUMA_RESET_AL1)) {
		b_gainL = 32 - _cur_gain_l;
		b_gainM = 32;
		b_gainH = 32 + _cur_gain_h;
		bStart = 1;
	} else {
		b_gainL = 33 - _cur_gain_l;
		b_gainM = 33;
		b_gainH = 33 + _cur_gain_h;
		bStart = 3;
	}

	for (bi = bStart; bi < LUMA_HIST_NUM; bi++) {
		if (p_hist_cur[bi - 1] < _value_l)
			p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainL;
		else if (p_hist_cur[bi - 1] > _value_h)
			p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainH;
		else
			p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainM;

		p_dync_curve[bi] = (p_dync_curve[bi] > 0x3FF) ? 0x3FF : p_dync_curve[bi];
	}

#else
	uint8_t bi, b_gainH, b_gainM, b_gainL;
	uint8_t bExtra;
	uint16_t wHistSun;

	if ((p_dync_curve == NULL)
	    || (p_hist_cur == NULL)) {
		return;
	}

	pp_build_con_curve_dft(p_dync_curve);
	wHistSun = 0;
	b_gainL = 32 - _cur_gain_l;
	b_gainM = 32;		/*+_cur_gain_m;  */
	b_gainH = 32 + _cur_gain_h;
	for (bi = 1; bi < LUMA_HIST_NUM; bi++) {
		wHistSun += p_hist_cur[bi - 1];
		if (p_hist_cur[bi - 1] < _value_l) {
			if (bi > 2)
				p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainL;

		} else if (p_hist_cur[bi - 1] > _value_h) {
			if (p_hist_cur[bi - 1] > 0x1E0)
				bExtra = p_hist_cur[bi - 1] >> 5;
			else if (p_hist_cur[bi - 1] > 0x100)
				bExtra = p_hist_cur[bi - 1] >> 6;
			else
				bExtra = 0;

			if ((bi < _bs_limit) && (bExtra > 0)) {	/*black add extra */
				p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainH + bExtra;
			} else {	/*not add extra */

				if (p_dync_curve[bi - 1] <
				    (((512 - wHistSun) / (LUMA_HIST_NUM - bi)) >> 1)) {
					p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainM;
				} else {
					p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainH;
				}
			}
		} else {
			if (p_dync_curve[bi - 1] < ((512 - wHistSun) / (LUMA_HIST_NUM - bi))) {
				if (p_dync_curve[bi - 1] > (0x20 * (bi - 1)))
					p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainL;
				else
					p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainH;
			} else {
				p_dync_curve[bi] = p_dync_curve[bi - 1] + b_gainM;
			}
		}

		p_dync_curve[bi] = (p_dync_curve[bi] > 0x3FF) ? 0x3FF : p_dync_curve[bi];
	}
#endif
}

/******************************************************************************
*Function: pp_build_con_curve_target(uint16_t *p_target_curve)
*Description: Build luma curve with black and white stretch
*Parameter: pointer of target curve
*Return: None
******************************************************************************/
void pp_build_con_curve_target(uint16_t *p_target_curve)
{

#ifdef NEW_LUMA_ALGORITHM
#else
	uint8_t bi;
#endif
	if (p_target_curve == NULL)
		return;

	pp_build_con_curve_dft(p_target_curve);
#ifdef NEW_LUMA_ALGORITHM
#else
	if (_bws_on_off == SV_ON) {
		uint16_t wDiff;
		uint8_t bStep;
		uint8_t bBs_gainMap, bWs_gainMap, bBSDetailEnhace, bWSDetailEnhace;
		uint8_t bWSAdjust_gain, bBSAdjust_gain;
		uint8_t bWSAdjust_offset, bBSAdjust_offset;

		bBs_gainMap = pp_get_bs_gain();	/*APLMapping */

		bWs_gainMap = pp_get_ws_gain();	/*APLMapping */
		wDiff = AbsDiff(_bs_gain, 0x80 + _bs_limit);
		bBSAdjust_gain = (uint8_t) (_bs_gain + ((wDiff * (0xFF - bBs_gainMap)) >> 8));
		wDiff = AbsDiff(_ws_gain, 0x80 + _ws_limit);
		bWSAdjust_gain = (uint8_t) (_ws_gain + ((wDiff * bWs_gainMap) >> 8));
		wDiff = _bs_offset + _bs_limit;
		bBSAdjust_offset = (uint8_t) ((wDiff * bBs_gainMap) >> 8);
		wDiff = _ws_offset + _ws_limit;
		bWSAdjust_offset = (uint8_t) ((wDiff * (0xFF - bWs_gainMap)) >> 8);
		if (bBSAdjust_offset > _bs_limit) {
			bBSAdjust_offset = bBSAdjust_offset - _bs_limit;
			bBSDetailEnhace = SV_OFF;
		} else {
			bBSAdjust_offset = _bs_limit - bBSAdjust_offset;
			bBSDetailEnhace = SV_ON;
		}

		if (bWSAdjust_offset > _ws_limit) {
			bWSAdjust_offset = bWSAdjust_offset - _ws_limit;
			bWSDetailEnhace = SV_OFF;
		} else {
			bWSAdjust_offset = _ws_limit - bWSAdjust_offset;
			bWSDetailEnhace = SV_ON;
		}

		PP_DEBUG("bAPL%dbBs_gainMap%dbWs_gainMap%d\n", _apl, bBs_gainMap, bWs_gainMap);
		PP_DEBUG("bBSAdjust_gain%dbBSAdjust_offset%d\n", bBSAdjust_gain, bBSAdjust_offset);
		PP_DEBUG("bWSAdjust_gain%dbWSAdjust_offset%d\n", bWSAdjust_gain, bWSAdjust_offset);
		if (_bs_on_off == SV_ON) {
			for (bi = 0; bi < _bs_level; bi++) {
				if (bBSDetailEnhace == SV_OFF) {
					bStep = (bBSAdjust_gain >> 2);
					if (bStep > bBSAdjust_offset) {
						bStep -= bBSAdjust_offset;
						bBSAdjust_offset = 0;
					} else {
						bBSAdjust_offset -= bStep;
						bStep = 0;
					}
				} else {
					bBSDetailEnhace = SV_OFF;
					bStep = (bBSAdjust_gain >> 2) + bBSAdjust_offset;
					bBSAdjust_offset = 0;
				}

				p_target_curve[bi + 2] = p_target_curve[bi + 1] + bStep;
			}
		}

		if (_ws_on_off == SV_ON) {
			for (bi = 0; bi < _ws_level; bi++) {
				if (bWSDetailEnhace == SV_OFF) {
					bStep = (bWSAdjust_gain >> 2);
					if (bStep > bWSAdjust_offset) {
						bStep -= bWSAdjust_offset;
						bWSAdjust_offset = 0;
					} else {
						bWSAdjust_offset -= bStep;
						bStep = 0;
					}
				} else {
					bWSDetailEnhace = SV_OFF;
					bStep = (bWSAdjust_gain >> 2) + bWSAdjust_offset;
					bWSAdjust_offset = 0;
				}

				p_target_curve[30 - bi] = p_target_curve[31 - bi] - bStep;
			}
		}
/*(26-6)/(33-3-10)*/
		wDiff =
		    ((p_target_curve[(31 - _ws_level)] -
		      p_target_curve[(1 + _bs_level)])) / (30 - (_ws_level + _bs_level));
		for (bi = (_bs_level + 2); bi < (31 - _ws_level); bi++)	/*7~26 */
			p_target_curve[bi] = p_target_curve[bi - 1] + wDiff;
	}
#endif
}

/******************************************************************************
*Function: pp_build_con_curve(const uint16_t *p_dync_curve, const uint16_t *p_target_curve)
*Description: The final output of adaptive luma is the combination of
			the dynamic luma curve and the target curve
*Parameter: pointer of dynamic and target curves
*Return: None
******************************************************************************/
void pp_build_con_curve(const uint16_t *p_dync_curve, const uint16_t *p_target_curve)
{

#ifdef NEW_LUMA_ALGORITHM
	uint8_t bi;

	if ((p_dync_curve == NULL)
	    || (p_target_curve == NULL)) {
		return;
	}

	for (bi = 1; bi < 32; bi++) {
		if (p_dync_curve[bi] > _wa_luma_array[bi]) {
			_change_cnt[bi]++;
			if (_change_cnt[bi] >= LUMA_CHANGE_CNT) {
				_wa_luma_array[bi] = _wa_luma_array[bi] + 1;
				_change_cnt[bi] = 0;
			}
		} else if (p_dync_curve[bi] < _wa_luma_array[bi]) {
			_change_cnt[bi]++;
			if (_change_cnt[bi] >= LUMA_CHANGE_CNT) {
				_wa_luma_array[bi] = _wa_luma_array[bi] - 1;
				_change_cnt[bi] = 0;
			}
		}
	}

#else
	uint8_t bi, bIndex;
	uint16_t waFinalcurve[LUMA_HIST_NUM + 1];

	if ((p_dync_curve == NULL)
	    || (p_target_curve == NULL)) {
		return;
	}

	for (bi = 0; bi < 33; bi++) {
		bIndex = p_target_curve[bi] >> 5;
		waFinalcurve[bi] =
		    ((p_dync_curve[bIndex + 1] *
		      (p_target_curve[bi] - (bIndex << 5))) +
		     (p_dync_curve[bIndex] * (((bIndex + 1) << 5) - p_target_curve[bi])) + 16) >> 5;
	}

	if (_fg_scene_change == SV_ON) {
		for (bi = 0; bi < 33; bi++) {
			_wa_luma_array[bi] =
			    ((_wa_luma_array[bi] * ((1 << _scene_change) - 1)) +
			     waFinalcurve[bi]) >> _scene_change;
		}
	} else {
		for (bi = 0; bi < 33; bi++) {
			_wa_luma_array[bi] =
			    ((_wa_luma_array[bi] * ((1 << _scene_normal) - 1)) +
			     waFinalcurve[bi]) >> _scene_normal;
		}
	}
#endif
	PP_DEBUG("\np_dync_curve p_target_curve baLumaArray\n");
	for (bi = 0; bi < 33; bi++) {
		PP_DEBUG("%d\t\t%d\t\t%d\n", p_dync_curve[bi], p_target_curve[bi],
			 _wa_luma_array[bi]);
	}
}

/******************************************************************************
*Function: pp_update_luma_curve(void)
*Description: HW update luma curve
*Parameter: None
*Return: None
******************************************************************************/
void pp_update_luma_curve(void)
{

	if (_compute_done && !_curve_freeze) {
		pp_set_luma_sw_reg(_wa_luma_array);
		_compute_done = SV_OFF;
	}
}

/******************************************************************************
*Function: pp_get_normalized_hist(uint32_t *p_hist_cur)
*Description: Get current histogram data and normalized to 512
*Parameter: pointer of histogram
*Return: SV_ON/SV_OFF
******************************************************************************/
uint8_t pp_get_normalized_hist(uint32_t *p_hist_cur)
{

	uint8_t bi;
	uint32_t p_histSum = 0;

	if (p_hist_cur == NULL)
		return SV_OFF;

	for (bi = 0; bi < LUMA_HIST_NUM; bi++) {
		p_hist_cur[bi] = pp_get_hist_cur(bi);
		p_histSum += p_hist_cur[bi];
	}

	if (p_histSum != 0) {
		for (bi = 0; bi < LUMA_HIST_NUM; bi++) {
			p_hist_cur[bi] =
			    (uint16_t) ((((uint32_t) p_hist_cur[bi] << 9) +
					 (p_histSum >> 1)) / p_histSum);
		}

		for (bi = 0; bi < LUMA_HIST_NUM; bi++)
			PP_DEBUG("Hist%d%d\n", bi, p_hist_cur[bi]);

		return SV_ON;
	} else {
		return SV_OFF;
	}
}


/******************************************************************************
*Function: pp_detect_scene_changed(const uint32_t *p_hist_cur)
*Description: Compare with the old histogram data to determine does the scene changed.
		Record the current histogram data.
*Parameter: pointer of histogram
*Return: None
******************************************************************************/
void pp_detect_scene_changed(const uint32_t *p_hist_cur)
{

	uint8_t bi;
	uint16_t wMaxDiff = 0, wTotalDiff = 0, wDiff = 0;

	if (p_hist_cur == NULL)
		return;

	for (bi = 0; bi < LUMA_HIST_NUM; bi++) {
		wDiff = AbsDiff(p_hist_cur[bi], (pp_get_hist_rec(bi)));
		wMaxDiff = (wMaxDiff < wDiff) ? wDiff : wMaxDiff;
		wTotalDiff += wDiff;
		pp_set_hist_rec(bi, p_hist_cur[bi]);
	}

	if (_fg_scene_change == SV_ON)
		return;


	if ((wMaxDiff > _max_diff_threshold)
	    && ((wTotalDiff >> 2) > _total_diff_threshold)) {
		_fg_scene_change = SV_ON;
		PP_DEBUG("\nScene Changed bMaxDiff=%d, wTotalDiff=%d\n", wMaxDiff, wTotalDiff);
	} else {
		_fg_scene_change = SV_OFF;
	}
}

/******************************************************************************
*Function: pp_get_adaptive_gain(const uint32_t *p_hist_cur)
*Description: Analyze the histogram data to get the proper adaptive gain
*Parameter: pointer of histogram
*Return: None
******************************************************************************/
void pp_get_adaptive_gain(const uint32_t *p_hist_cur)
{

	uint8_t bi;
	uint8_t bCountL = 0, bCountM = 0, bCountH = 0;
	uint8_t bDiffCur;
	uint8_t bDyn_gain = ((uint16_t) _adap_luma_gain * (255 - _apl)) >> 8;
	uint8_t b_gainL = ((16 + _adap_luma_offset) * (uint16_t) bDyn_gain) >> 7;
	uint8_t b_gainM = ((8 + _adap_luma_offset) * (uint16_t) bDyn_gain) >> 7;
	uint8_t b_gainH = b_gainL;
#ifdef NEW_LUMA_ALGORITHM
#else
	uint32_t dwBSSum, dwWSSum;
#endif
	b_gainL = (b_gainL > 16) ? 16 : b_gainL;
	if (p_hist_cur == NULL)
		return;

#ifdef NEW_LUMA_ALGORITHM

	_bws_on_off = SV_OFF;
#else
	dwBSSum = 0;
	dwWSSum = 0;
	for (bi = 0; bi < _bs_limit; bi++)
		dwBSSum += p_hist_cur[bi];

	for (bi = 31 - _bs_limit; bi < LUMA_HIST_NUM; bi++)
		dwWSSum += p_hist_cur[bi];

	/*disable if histogram sum>1/5 */
	/*_bws_on_off=((dwBSSum+dwWSSum)>BW_STRETCH_TH)?SV_OFF:SV_ON;  */
	/*disable if histogram sum>1/8 */
	_bs_on_off = (dwBSSum > B_STRETCH_TH) ? SV_OFF : SV_ON;
	_ws_on_off = (dwWSSum > W_STRETCH_TH) ? SV_OFF : SV_ON;
#endif
	for (_value_l = AL_LOW_START; _value_l < AL_LOW_END; _value_l += 4) {
		bCountL = 0;
		for (bi = 0; bi < LUMA_HIST_NUM; bi++) {
			if (p_hist_cur[bi] < _value_l)
				bCountL++;
		}

		if (bCountL >= AL_LOW_BIN)
			break;
	}

	for (_value_h = AL_HIGH_START; _value_h > AL_HIGH_END; _value_h -= 4) {
		bCountH = 0;
		for (bi = 0; bi < LUMA_HIST_NUM; bi++) {
			if (p_hist_cur[bi] >= _value_h)
				bCountH++;
		}

		if (bCountH >= AL_HIGH_BIN)
			break;
	}

	bCountM = LUMA_HIST_NUM - (bCountH + bCountL);
	bCountL -= 4;		/*16~240 */
	PP_DEBUG("\nbDyn_gain%d valueL%d valueH%d\n", bDyn_gain, _value_l, _value_h);
	PP_DEBUG("bCountL%d bCountM%d bCountH%d\n", bCountL, bCountM, bCountH);
	bDiffCur = AL_LOW_END - _value_l;
	/*_cur_gain_l=b_gainL;  */
	_cur_gain_m = ((uint16_t) b_gainM * bDiffCur) / (AL_LOW_END - AL_LOW_START);
	_cur_gain_h = ((uint16_t) b_gainH * bDiffCur) / (AL_LOW_END - AL_LOW_START);
	PP_DEBUG("Cur_gainL%d Cur_gainM%d bCur_gainH%d\n", _cur_gain_l, _cur_gain_m, _cur_gain_h);
	if ((bCountL != 0) && (bCountH != 0)) {
		_cur_gain_l =
		    (((uint16_t) _cur_gain_h * bCountH) +
		     ((uint16_t) _cur_gain_m * bCountM) + (bCountL >> 1)) / bCountL;
		_cur_gain_l = (_cur_gain_l > b_gainL) ? b_gainL : _cur_gain_l;
	} else {
		_cur_gain_l = 0;
		_cur_gain_m = 0;
		_cur_gain_h = 0;
	}

	PP_DEBUG("Cur_gainL%d Cur_gainM%d bCur_gainH%d\n", _cur_gain_l, _cur_gain_m, _cur_gain_h);
}

/******************************************************************************
*Function: pp_get_bs_gain(void)
*Description: get black stratch gain
*Parameter: None
*Return: BS_gain
******************************************************************************/
uint8_t pp_get_bs_gain(void)
{

	uint8_t bRet;

	if (_apl < 64)
		bRet = (uint8_t) (((uint16_t) _bs_ratio * _apl) >> 6);
	else
		bRet = (uint8_t) ((((uint16_t) (255 - _bs_ratio) * (_apl - 64)) / 192) + _bs_ratio);

	return bRet;
}

/******************************************************************************
*Function: pp_get_ws_gain(void)
*Description: get white stratch gain
*Parameter: None
*Return: WS_gain
******************************************************************************/
uint8_t pp_get_ws_gain(void)
{

	uint8_t bRet;

	if (_apl < 192)
		bRet = (uint8_t) (((uint16_t) (255 - _ws_ratio) * _apl) / 192);
	else
		bRet = (uint8_t) ((((uint16_t) _ws_ratio * (_apl - 192)) >> 6) + (255 - _ws_ratio));

	return bRet;
}

/******************************************************************************
*Function: pp_contrast_proc(void)
*Description: adaptive luma process
*Parameter: None
*Return: None
******************************************************************************/
void pp_contrast_proc(void)
{

	uint32_t p_hist_cur[LUMA_HIST_NUM];
	uint16_t p_target_curve[LUMA_HIST_NUM + 1];
	uint16_t p_dync_curve[LUMA_HIST_NUM + 1];

	if (pp_get_normalized_hist(p_hist_cur) == SV_ON) {
		if (_max_sat == u2RegRd2B(SAT_HIST_1_0_MAIN)) {
			if (_dynamic_state < LUMA_ADADTIVE_AL2)
				_dynamic_state++;
		} else {
			if (_dynamic_state >= LUMA_RESET_AL1)
				_dynamic_state = LUMA_ADADTIVE_AL1;
			else if (_dynamic_state >= LUMA_ADADTIVE_AL2)
				_dynamic_state++;
			else if (_dynamic_state > LUMA_ADADTIVE_AL1)
				_dynamic_state--;
		}
		pp_get_adaptive_gain(p_hist_cur);
		pp_build_con_curve_dyn(p_hist_cur, p_dync_curve);
		pp_build_con_curve_target(p_target_curve);
		pp_detect_scene_changed(p_hist_cur);
		pp_build_con_curve(p_dync_curve, p_target_curve);
	}

	_fg_scene_change = SV_OFF;
}

/******************************************************************************
*Function: wReadHist32Cur(uint8_t addr)
*Description: read current histogram value
*Parameter: address
*Return: histogram value
******************************************************************************/
uint32_t pp_get_hist_cur(uint8_t addr)
{

	return _hist_cur[addr];
}

/******************************************************************************
*Function: pp_set_hist_cur(uint8_t addr, uint16_t data)
*Description: write current histogram value
*Parameter: address, value
*Return: None
******************************************************************************/
void pp_set_hist_cur(uint8_t addr, uint32_t ddata)
{

	_hist_cur[addr] = ddata & 0x1FFFFF;
}

/******************************************************************************
*Function: wReadHist32Rec(uint8_t addr)
*Description: read recode histogram value
*Parameter: address
*Return: histogram value
******************************************************************************/
uint32_t pp_get_hist_rec(uint8_t addr)
{

	return _hist_rec[addr];
}

/******************************************************************************
*Function: pp_set_hist_rec(uint8_t addr, uint16_t data)
*Description: write current histogram value
*Parameter: address, value
*Return: None
******************************************************************************/
void pp_set_hist_rec(uint8_t addr, uint32_t ddata)
{

	_hist_rec[addr] = ddata;
}

/******************************************************************************
*Function: pp_hal_is_hist_ready(void)
*Description: read HW METRIC_RDY
*Parameter: None
*Return: METRIC_RDY
******************************************************************************/
uint8_t pp_hal_is_hist_ready(void)
{
	uint8_t ret;

	ret = (RegRdFldAlign(M_VDO_LUMA_STATUS_1C, METRIC_RDY) ? SV_ON : SV_OFF);
	return ret;
}

/******************************************************************************
*Function: pp_get_hist_info(void)
*Description: get histogram information
*Parameter: None
*Return: None
******************************************************************************/
void pp_get_hist_info(void)
{

/*uint8_t i;
*STATISTICS_T *pVDQStatT, *pVDQStatB;
*uint8_t bTriState;

*pVDQStatT=VPQ_DeintAccessNextFieldT(VDP_1);
*pVDQStatB=VPQ_DeintAccessNextFieldB(VDP_1);

*if(pVDQStatT->fgTrickmode)

*_normal_cnt=0;
*if(_trick_cnt<0x80)_trick_cnt++;
*}
*else
*
*_trick_cnt=0;
*if(_normal_cnt<0x80)_normal_cnt++;
*}
*
*deglitch function
*bTriState=(_trick_cnt>FIELD_GLITCH)?1:((_normal_cnt>FIELD_GLITCH)?0:2);

*if(bTriState!=2)

*_luma_max=(pVDQStatT->luma_max>pVDQStatB->luma_max)?pVDQStatT->luma_max:pVDQStatB->luma_max;
*_luma_min=(pVDQStatT->luma_min<pVDQStatB->luma_min)?pVDQStatT->luma_min:pVDQStatB->luma_min;

*if(pVDQStatT->luma_tot_pixel!=0&&pVDQStatB->luma_tot_pixel!=0)

*_apl=(pVDQStatT->luma_sum+pVDQStatB->luma_sum)/(pVDQStatT->luma_tot_pixel+pVDQStatB->luma_tot_pixel);
*}
*else

*_apl=0;
*}

*for(i=0;  i<LUMA_HIST_NUM;  i++)

*pp_set_hist_cur(i, (pVDQStatT->luma_histo[i]+pVDQStatB->luma_histo[i])*4);
*}
*}
*else

*LOG(15, "autocontrastde-glitch");
*}

*for(i=0;  i<LUMA_HIST_NUM;  i+=4)

*LOG(11, "Hist[%d]=%d, Hist[%d]=%d, Hist[%d]=%d, Hist[%d]=%d\n",
*i, pp_get_hist_cur(i),
*i+1, pp_get_hist_cur(i+1),
*i+2, pp_get_hist_cur(i+2),
*i+3, pp_get_hist_cur(i+3));
*}
*/
}

/******************************************************************************
*Function: pp_set_luma_curve(void)
*Description: Set Luma curve
*Parameter: None
*Return: None
******************************************************************************/
void pp_set_luma_curve(void)
{

	uint8_t u1Idx;

	for (u1Idx = 0; u1Idx < HAL_POST_LUMA_MAIN_REG_NUM; u1Idx++)
		_luma_main_hw_reg->au4Reg[u1Idx] = _luma_main_sw_reg.au4Reg[u1Idx];
}

/******************************************************************************
*Function: pp_hal_set_con_dft(void)
*Description: set Y transfer function to default value
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_set_con_dft(void)
{

	uint8_t bi;
	/*Note: Due to Fetch in VSync, can't use vRegWrite2B */
	for (bi = 0; bi < LUMA_HIST_LEVEL; bi++)
		vRegWt4B(Y_FTN_1_0 + 4 * bi, 0x400040 * bi + 0x200000);

	vRegWt2B(Y_FTN_32_, 0x3FF);
	_luma_main_sw_reg.rField.u2Y_FTN0 = 0x00;
	_luma_main_sw_reg.rField.u2Y_FTN1 = 0x20;
	_luma_main_sw_reg.rField.u2Y_FTN2 = 0x40;
	_luma_main_sw_reg.rField.u2Y_FTN3 = 0x60;
	_luma_main_sw_reg.rField.u2Y_FTN4 = 0x80;
	_luma_main_sw_reg.rField.u2Y_FTN5 = 0xA0;
	_luma_main_sw_reg.rField.u2Y_FTN6 = 0xC0;
	_luma_main_sw_reg.rField.u2Y_FTN7 = 0xE0;
	_luma_main_sw_reg.rField.u2Y_FTN8 = 0x100;
	_luma_main_sw_reg.rField.u2Y_FTN9 = 0x120;
	_luma_main_sw_reg.rField.u2Y_FTN10 = 0x140;
	_luma_main_sw_reg.rField.u2Y_FTN11 = 0x160;
	_luma_main_sw_reg.rField.u2Y_FTN12 = 0x180;
	_luma_main_sw_reg.rField.u2Y_FTN13 = 0x1A0;
	_luma_main_sw_reg.rField.u2Y_FTN14 = 0x1C0;
	_luma_main_sw_reg.rField.u2Y_FTN15 = 0x1E0;
	_luma_main_sw_reg.rField.u2Y_FTN16 = 0x200;
	_luma_main_sw_reg.rField.u2Y_FTN17 = 0x220;
	_luma_main_sw_reg.rField.u2Y_FTN18 = 0x240;
	_luma_main_sw_reg.rField.u2Y_FTN19 = 0x260;
	_luma_main_sw_reg.rField.u2Y_FTN20 = 0x280;
	_luma_main_sw_reg.rField.u2Y_FTN21 = 0x2A0;
	_luma_main_sw_reg.rField.u2Y_FTN22 = 0x2C0;
	_luma_main_sw_reg.rField.u2Y_FTN23 = 0x2E0;
	_luma_main_sw_reg.rField.u2Y_FTN24 = 0x300;
	_luma_main_sw_reg.rField.u2Y_FTN25 = 0x320;
	_luma_main_sw_reg.rField.u2Y_FTN26 = 0x340;
	_luma_main_sw_reg.rField.u2Y_FTN27 = 0x360;
	_luma_main_sw_reg.rField.u2Y_FTN28 = 0x380;
	_luma_main_sw_reg.rField.u2Y_FTN29 = 0x3A0;
	_luma_main_sw_reg.rField.u2Y_FTN30 = 0x3C0;
	_luma_main_sw_reg.rField.u2Y_FTN31 = 0x3E0;
	_luma_main_sw_reg.rField.u2Y_FTN32 = 0x3FF;
}

/******************************************************************************
*Function: pp_set_luma_sw_reg(uint16_t *luma_array)
*Description: set Ytransfer function to luma curve
*Parameter: lumacurvepointer
*Return: None
******************************************************************************/
void pp_set_luma_sw_reg(uint16_t *luma_array)
{

	_luma_main_sw_reg.rField.u2Y_FTN0 = luma_array[0];
	_luma_main_sw_reg.rField.u2Y_FTN1 = luma_array[1];
	_luma_main_sw_reg.rField.u2Y_FTN2 = luma_array[2];
	_luma_main_sw_reg.rField.u2Y_FTN3 = luma_array[3];
	_luma_main_sw_reg.rField.u2Y_FTN4 = luma_array[4];
	_luma_main_sw_reg.rField.u2Y_FTN5 = luma_array[5];
	_luma_main_sw_reg.rField.u2Y_FTN6 = luma_array[6];
	_luma_main_sw_reg.rField.u2Y_FTN7 = luma_array[7];
	_luma_main_sw_reg.rField.u2Y_FTN8 = luma_array[8];
	_luma_main_sw_reg.rField.u2Y_FTN9 = luma_array[9];
	_luma_main_sw_reg.rField.u2Y_FTN10 = luma_array[10];
	_luma_main_sw_reg.rField.u2Y_FTN11 = luma_array[11];
	_luma_main_sw_reg.rField.u2Y_FTN12 = luma_array[12];
	_luma_main_sw_reg.rField.u2Y_FTN13 = luma_array[13];
	_luma_main_sw_reg.rField.u2Y_FTN14 = luma_array[14];
	_luma_main_sw_reg.rField.u2Y_FTN15 = luma_array[15];
	_luma_main_sw_reg.rField.u2Y_FTN16 = luma_array[16];
	_luma_main_sw_reg.rField.u2Y_FTN17 = luma_array[17];
	_luma_main_sw_reg.rField.u2Y_FTN18 = luma_array[18];
	_luma_main_sw_reg.rField.u2Y_FTN19 = luma_array[19];
	_luma_main_sw_reg.rField.u2Y_FTN20 = luma_array[20];
	_luma_main_sw_reg.rField.u2Y_FTN21 = luma_array[21];
	_luma_main_sw_reg.rField.u2Y_FTN22 = luma_array[22];
	_luma_main_sw_reg.rField.u2Y_FTN23 = luma_array[23];
	_luma_main_sw_reg.rField.u2Y_FTN24 = luma_array[24];
	_luma_main_sw_reg.rField.u2Y_FTN25 = luma_array[25];
	_luma_main_sw_reg.rField.u2Y_FTN26 = luma_array[26];
	_luma_main_sw_reg.rField.u2Y_FTN27 = luma_array[27];
	_luma_main_sw_reg.rField.u2Y_FTN28 = luma_array[28];
	_luma_main_sw_reg.rField.u2Y_FTN29 = luma_array[29];
	_luma_main_sw_reg.rField.u2Y_FTN30 = luma_array[30];
	_luma_main_sw_reg.rField.u2Y_FTN31 = luma_array[31];
	_luma_main_sw_reg.rField.u2Y_FTN32 = luma_array[32];
/*
*uint8_t bi;

*for(bi=0;  bi<LUMA_HIST_LEVEL;  bi++)

*vRegWt4B(Y_FTN_1_0+4*bi,
*(uint32_t)luma_array[2*bi]+(luma_array[2*bi+1]<<16));
*}

*vRegWt4B(Y_FTN_32_, luma_array[LUMA_HIST_NUM]);
*/
}

/******************************************************************************
*Function: pp_get_apl_value(void)
*Description: get APL value
*Parameter: None
*Return: APL
******************************************************************************/
uint8_t pp_get_apl_value(void)
{

	return _apl;
}

/******************************************************************************
*Function: pp_get_luma_max(void)
*Description: get luma max.value
*Parameter: None
*Return: lumamax.value
******************************************************************************/
uint8_t pp_get_luma_max(void)
{

	return _luma_max;
}

/******************************************************************************
*Function: pp_get_luma_min(void)
*Description: get luma min.value
*Parameter: None
*Return: luma min.value
******************************************************************************/
uint8_t pp_get_luma_min(void)
{

	return _luma_min;
}

/******************************************************************************
*Function: pp_get_hist(uint32_t *p_hist)
*Description: get luma histogram
*Parameter: histogram pointer
*Return: true/false
******************************************************************************/
uint8_t pp_get_hist(uint32_t *p_hist)
{

	uint8_t i;

	if (p_hist == NULL)
		return SV_OFF;

	for (i = 0; i < LUMA_HIST_NUM; i++)
		p_hist[i] = pp_get_hist_cur(i);

	return SV_ON;
}

/******************************************************************************
*Function: pp_get_norm_hist(uint32_t *p_hist)
*Description: normalized histogram for CLI use
*Parameter: histogram pointer
*Return: true/false
******************************************************************************/
uint8_t pp_get_norm_hist(uint32_t *p_hist)
{

	uint8_t i;
	uint32_t dwTotal = 0;

	if (p_hist == NULL)
		return SV_OFF;

	for (i = 0; i < LUMA_HIST_NUM; i++)
		dwTotal += pp_get_hist_cur(i);

	for (i = 0; i < LUMA_HIST_NUM; i++) {
		/*User UINT64 to avoid data overflow when full HD resolution. */
		if (dwTotal == 0)
			p_hist[i] = 0;
		else
			p_hist[i] = pp_get_hist_cur(i);
	}

	return SV_ON;
}

/******************************************************************************
*Function: pp_al_init(void)
*Description: initial average lum avaribles
*Parameter: None
*Return: None
******************************************************************************/
void pp_al_init(void)
{

	_adap_luma_gain = AL_GAIN;
	_adap_luma_offset = AL_OFFSET;
	_adap_luma_limit = AL_LIMIT;
	_max_diff_threshold = MAX_DIFF_THRESHOLD;	/*scene change max.TH */
	_total_diff_threshold = TOTAL_DIFF_THRESHOLD;	/*scene change total TH */
}

/******************************************************************************
*Function: pp_bws_init(void)
*Description: initial black/white stretch varibles
*Parameter: None
*Return: None
******************************************************************************/
void pp_bws_init(void)
{

	_bws_on_off = SV_ON;
	_bs_on_off = BLACK_STRETCH_ENABLE;
	_ws_on_off = WHITE_STRETCH_ENABLE;
	_bs_level = BLACK_STRETCH_LEVEL;
	_bs_gain = BLACK_STRETCH_GAIN;
	_bs_offset = BLACK_STRETCH_OFFSET;
	_bs_ratio = BLACK_STRETCH_RATIO;
	_bs_limit = BLACK_STRETCH_LIMIT;
	_ws_level = WHITE_STRETCH_LEVEL;
	_ws_gain = WHITE_STRETCH_GAIN;
	_ws_offset = WHITE_STRETCH_OFFSET;
	_ws_ratio = WHITE_STRETCH_RATIO;
	_ws_limit = WHITE_STRETCH_LIMIT;
}

/******************************************************************************
*Function: pp_limit_con_param(void)
*Description: Constrain varibles
*Parameter: None
*Return: None
******************************************************************************/
void pp_limit_con_param(void)
{

	_adap_luma_gain = (_adap_luma_gain > 0x80) ? 0x80 : _adap_luma_gain;
	_adap_luma_offset = (_adap_luma_offset > 16) ? 16 : _adap_luma_offset;
	_bs_level = (_bs_level > 8) ? 8 : _bs_level;
	_bs_gain = (_bs_gain > 0x80) ? 0x80 : _bs_gain;
	_ws_level = (_ws_level > 8) ? 8 : _ws_level;
	_ws_gain = (_ws_gain > 0x80) ? 0x80 : _ws_gain;
}

/******************************************************************************
*Function: pp_auto_con_init(void)
*Description: initial adaptive luma varibles
*Parameter: None
*Return: None
******************************************************************************/
void pp_auto_con_init(void)
{

	uint8_t bCnt;

	_adaptive_on = SV_OFF;
	_curve_freeze = SV_ON;
	_fg_scene_change = SV_OFF;
	_compute_done = SV_OFF;
	pp_al_init();
	pp_build_con_curve_dft(_wa_luma_array);
	pp_bws_init();
	pp_limit_con_param();
	pp_set_luma_curve();
	_dynamic_state = LUMA_INITIAL;
	for (bCnt = 0; bCnt < 33; bCnt++)
		_change_cnt[bCnt] = 0;
}

/******************************************************************************
*Function: pp_hal_boost_chroma(uint8_t b_on_off)
*Description: Boost chroma by (Yout-Yin)*_gain, Increase saturation at luma change part
*Parameter: b_on_off
*Return: None
******************************************************************************/
void pp_hal_boost_chroma(uint8_t b_on_off)
{

	uint8_t benable;

	benable = (b_on_off > 0) ? 1 : 0;
	vRegWt4BMsk(C_BOOST_MAIN, (benable ? ENABLE : 0) | (0x2 << 8)
		    | (0xE0 << 0), ENABLE | RANGE_SEL | BOOST_GAIN);
}

/******************************************************************************
*Function: pp_freeze_auto_con(uint8_t freeze)
*Description: set varibles
*Parameter: freeze
*Return: None
******************************************************************************/
void pp_freeze_auto_con(uint8_t freeze)
{

	_curve_freeze = freeze;
	_adaptive_on = freeze;
}

/******************************************************************************
*Function: pp_auto_contrastOn(void)
*Description: init alvaribles
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_auto_con_enable(uint8_t b_on_off)
{

#ifdef NEW_LUMA_ALGORITHM
	uint8_t bCnt, b_offset;
#endif
	if (b_on_off) {
		_adaptive_on = SV_ON;
		_fg_scene_change = SV_ON;
		_curve_freeze = SV_OFF;
		_compute_done = SV_OFF;
		_pp_status |= ADAPTIVE_LUMA_ON_STATUS;
		_trick_cnt = 0;
		_normal_cnt = 0;
#ifdef NEW_LUMA_ALGORITHM
		_sat_gain = 0;
		b_offset = 0;
		for (bCnt = 0; bCnt < 16; bCnt++) {
			vRegWt1B(Y_SLOPE_1_0_MAIN + b_offset, 0x80);	/*Y_Slope */
			b_offset += 2;
		}
#endif
	} else {
		pp_hal_set_con_dft();
		pp_auto_con_init();
		_pp_status |= ADAPTIVE_LUMA_OFF_STATUS;
#ifdef NEW_LUMA_ALGORITHM
		b_offset = 0;
		switch (_video_mode) {
		case POST_VIDEO_MODE_VIVID:
			for (bCnt = 0; bCnt < 16; bCnt++) {
				vRegWt1B(Y_SLOPE_1_0_MAIN + b_offset,
					_vivid_mode_tbl[Y_SLOPE_MAIN][bCnt]);	/*Y_Slope */
				vRegWt1B(S_G2_1_0_MAIN + b_offset,
					_vivid_mode_tbl[S_G2_MAIN][bCnt]);	/*S__gain2 */
				b_offset += 2;
			}
			break;
		case POST_VIDEO_MODE_CINEMA:
			for (bCnt = 0; bCnt < 16; bCnt++) {
				vRegWt1B(Y_SLOPE_1_0_MAIN + b_offset,
					_cinema_mode_tbl[Y_SLOPE_MAIN][bCnt]);	/*Y_Slope */
				vRegWt1B(S_G2_1_0_MAIN + b_offset,
					_cinema_mode_tbl[S_G2_MAIN][bCnt]);	/*S__gain2 */
				b_offset += 2;
			}
			break;
		default:
			for (bCnt = 0; bCnt < 16; bCnt++) {
				vRegWt1B(Y_SLOPE_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_G1_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_G2_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_G3_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_P1_1_0_MAIN + b_offset, 0x20);
				vRegWt1B(S_P2_1_0_MAIN + b_offset, 0x40);
				vRegWt1B(S_Y0_G_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_Y64_G_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_Y128_G_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_Y192_G_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(S_Y256_G_1_0_MAIN + b_offset, 0x80);
				vRegWt1B(H_FTN_1_0_MAIN + b_offset, 0x00);
				b_offset += 2;
			}
			break;
		}
#endif
	}
	pp_hal_boost_chroma(b_on_off);
}

#ifdef NEW_LUMA_ALGORITHM
/******************************************************************************
*Function: pp_norm_hist(void)
*Description: Get current histogram data and normalized to 512
*Parameter: None
*Return: SV_ON/SV_OFF
******************************************************************************/
void pp_norm_hist(void)
{

	uint8_t bi, b_offset;
	uint32_t p_histSum = 0;

	b_offset = 0;
	for (bi = 0; bi < 8; bi++) {
		p_histSum += u2RegRd2B(SAT_HIST_1_0_MAIN + b_offset) * (0x18 * bi);
		b_offset += 2;
	}

	_aps = p_histSum / _max_sat;
}

/******************************************************************************
*Function: pp_hal_auto_sat(void)
*Description: AutoSat
*Parameter: None
*Return: None
******************************************************************************/
void pp_hal_auto_sat(void)
{

/*Todo:somealgorithm*/
	uint8_t bCnt, b_offset;
	uint8_t b_gain;
	uint16_t wTmp;

	pp_norm_hist();
	if (_aps < 0x30) {
		wTmp = ((uint16_t) (0xFF - _apl) * (0x30 - _aps)) >> 11;
		_sat_gain = (_sat_gain < wTmp) ? _sat_gain + 1 : wTmp;	/*hysteresis. */
	} else {
		_sat_gain = (_sat_gain > 0) ? _sat_gain - 1 : 0;	/*hysteresis. */
	}
	b_gain = _sat_gain;
	b_offset = 0;
	for (bCnt = 0; bCnt < 16; bCnt++) {
		switch (_video_mode) {
		case POST_VIDEO_MODE_VIVID:
			wTmp = _vivid_mode_tbl[S_G2_MAIN][bCnt] + b_gain;
			break;
		case POST_VIDEO_MODE_CINEMA:
			wTmp = _cinema_mode_tbl[S_G2_MAIN][bCnt] + b_gain;
			break;
		default:
			wTmp = 0x80;
			break;
		}

		wTmp = (wTmp > 0xC0) ? 0xC0 : wTmp;
		vRegWt1B(S_G2_1_0_MAIN + b_offset, wTmp);	/*S__gain2 */
		b_offset += 2;
	}
}
#endif
/******************************************************************************
*Function: pp_auto_contrast(void)
*Description: Auto Contrast
*Parameter: None
*Return: None
******************************************************************************/
void pp_auto_contrast(void)
{

	if (_adaptive_on) {
		pp_set_luma_curve();	/*updatelastcalculation */
		pp_get_hist_info();
		pp_contrast_proc();
#ifdef NEW_LUMA_ALGORITHM
		pp_hal_auto_sat();
#endif
		_compute_done = SV_ON;
		if (_curve_freeze == SV_OFF) {	/*SSWuconsiderpausesituation */
			pp_update_luma_curve();
		}
	}
}




/*for  tuning*/
void pp_auto_con_set_dft(void)
{

	_adap_luma_gain = read_qty_table(QUALITY_AL_GAIN);
	_adap_luma_offset = read_qty_table(QUALITY_AL_OFFSET);
	_adap_luma_limit = read_qty_table(QUALITY_AL_LIMIT);
	_max_diff_threshold = read_qty_table(QUALITY_AL_SCENE_CHANGE_MAX_THD);
	_total_diff_threshold = read_qty_table(QUALITY_AL_SCENE_CHANGE_TOTAL_THD);
	/*bMaxNoiseThreshold=read_qty_table(QUALITY_AL_NOISE_THRESHOLD);  */
	_bws_on_off = read_qty_table(QUALITY_BWS_ON_1_OFF_0);
	_bs_on_off = read_qty_table(QUALITY_BWS_BLACK_ON_1_OFF_0);
	_ws_on_off = read_qty_table(QUALITY_BWS_WHITE_ON_1_OFF_0);
	_bs_level = read_qty_table(QUALITY_BWS_BLACK_LEVEL);
	_bs_gain = read_qty_table(QUALITY_BWS_BLACK_GAIN);
	_bs_offset = read_qty_table(QUALITY_BWS_BLACK_OFFSET);
	_bs_ratio = read_qty_table(QUALITY_BWS_BLACK_RATIO);
	_bs_limit = read_qty_table(QUALITY_BWS_BLACK_LIMIT);
	_ws_level = read_qty_table(QUALITY_BWS_WHITE_LEVEL);
	_ws_gain = read_qty_table(QUALITY_BWS_WHITE_GAIN);
	_ws_offset = read_qty_table(QUALITY_BWS_WHITE_OFFSET);
	_ws_ratio = read_qty_table(QUALITY_BWS_WHITE_RATIO);
	_ws_limit = read_qty_table(QUALITY_BWS_WHITE_LIMIT);
	pp_limit_con_param();
	_fg_scene_change = SV_ON;
}

/*for tuning*/
void pp_hal_set_bs_param(uint8_t item, uint8_t value)
{

	switch (item) {
	case 0:
		write_qty_table(QUALITY_BWS_BLACK_LEVEL, value);
		PP_INFO("QUALITY_BWS_BLACK_LEVEL=%d\n", value);
		break;
	case 1:
		write_qty_table(QUALITY_BWS_BLACK_GAIN, value);
		PP_INFO("QUALITY_BWS_BLACK_GAIN=%d\n", value);
		break;
	case 2:
		write_qty_table(QUALITY_BWS_BLACK_OFFSET, value);
		PP_INFO("QUALITY_BWS_BLACK_OFFSET=%d\n", value);
		break;
	case 3:
		write_qty_table(QUALITY_BWS_BLACK_RATIO, value);
		PP_INFO("QUALITY_BWS_BLACK_RATIO=%d\n", value);
		break;
	case 4:
		write_qty_table(QUALITY_BWS_BLACK_LIMIT, value);
		PP_INFO("QUALITY_BWS_BLACK_LIMIT=%d\n", value);
		break;
	case 5:
		write_qty_table(QUALITY_BWS_BLACK_ON_1_OFF_0, value);
		PP_INFO("QUALITY_BWS_BLACK_ON_OFF=%d\n", value);
		break;
	default:
		PP_INFO("QUALITY_BWS_BLACK_LEVEL=%d\n", _bs_level);
		PP_INFO("QUALITY_BWS_BLACK_GAIN=%d\n", _bs_gain);
		PP_INFO("QUALITY_BWS_BLACK_OFFSET=%d\n", _bs_offset);
		PP_INFO("QUALITY_BWS_BLACK_RATIO=%d\n", _bs_ratio);
		PP_INFO("QUALITY_BWS_BLACK_LIMIT=%d\n", _bs_limit);
		PP_INFO("QUALITY_BWS_BLACK_ON_OFF=%d\n", _bs_on_off);
		break;
	}

	pp_auto_con_set_dft();
}

void pp_hal_set_ws_param(uint8_t item, uint8_t value)
{

	switch (item) {
	case 0:
		write_qty_table(QUALITY_BWS_WHITE_LEVEL, value);
		PP_INFO("QUALITY_BWS_WHITE_LEVEL=%d\n", value);
		break;
	case 1:
		write_qty_table(QUALITY_BWS_WHITE_GAIN, value);
		PP_INFO("QUALITY_BWS_WHITE_GAIN=%d\n", value);
		break;
	case 2:
		write_qty_table(QUALITY_BWS_WHITE_OFFSET, value);
		PP_INFO("QUALITY_BWS_WHITE_OFFSET=%d\n", value);
		break;
	case 3:
		write_qty_table(QUALITY_BWS_WHITE_RATIO, value);
		PP_INFO("QUALITY_BWS_WHITE_RATIO=%d\n", value);
		break;
	case 4:
		write_qty_table(QUALITY_BWS_WHITE_LIMIT, value);
		PP_INFO("QUALITY_BWS_WHITE_LIMIT=%d\n", value);
		break;
	case 5:
		write_qty_table(QUALITY_BWS_WHITE_ON_1_OFF_0, value);
		PP_INFO("QUALITY_BWS_WHITE_ON_OFF=%d\n", value);
		break;
	default:
		PP_INFO("QUALITY_BWS_WHITE_LEVEL=%d\n", _ws_level);
		PP_INFO("QUALITY_BWS_WHITE_GAIN=%d\n", _ws_gain);
		PP_INFO("QUALITY_BWS_WHITE_OFFSET=%d\n", _ws_offset);
		PP_INFO("QUALITY_BWS_WHITE_RATIO=%d\n", _ws_ratio);
		PP_INFO("QUALITY_BWS_WHITE_LIMIT=%d\n", _ws_limit);
		PP_INFO("QUALITY_BWS_WHITE_ON_OFF=%d\n", _ws_on_off);
		break;
	}

	pp_auto_con_set_dft();
}

void pp_hal_set_al_param(uint8_t item, uint8_t value)
{

	switch (item) {
	case 0:
		write_qty_table(QUALITY_AL_GAIN, value);
		PP_INFO("QUALITY_AL_GAIN=%d\n", value);
		break;
	case 1:
		write_qty_table(QUALITY_AL_OFFSET, value);
		PP_INFO("QUALITY_AL_OFFSET=%d\n", value);
		break;
	case 2:
		write_qty_table(QUALITY_AL_LIMIT, value);
		PP_INFO("QUALITY_AL_LIMIT=%d\n", value);
		break;
	case 3:
		write_qty_table(QUALITY_AL_METHOD, value);
		PP_INFO("QUALITY_AL_METHOD=%d\n", value);
		break;
	case 4:
		write_qty_table(QUALITY_AL_SCENE_CHANGE_MAX_THD, value);
		PP_INFO("QUALITY_AL_SCENE_CHANGE_MAX_THD=%d\n", value);
		break;
	case 5:
		write_qty_table(QUALITY_AL_SCENE_CHANGE_TOTAL_THD, value);
		PP_INFO("QUALITY_AL_SCENE_CHANGE_TOTAL_THD=%d\n", value);
		break;
	default:
		PP_INFO("QUALITY_AL_GAIN=%d\n", _adap_luma_gain);
		PP_INFO("QUALITY_AL_OFFSET=%d\n", _adap_luma_offset);
		PP_INFO("QUALITY_AL_LIMIT=%d\n", _adap_luma_limit);
		PP_INFO("QUALITY_AL_SCENE_CHANGE_MAX_THD=%d\n", _max_diff_threshold);
		PP_INFO("QUALITY_AL_SCENE_CHANGE_TOTAL_THD=%d\n", _total_diff_threshold);
		break;
	}
	pp_auto_con_set_dft();
}

/******************************************************************************
*Function: pp_hal_set_gama(uint8_t value)
*Description: set gamma
*Parameter: value
*Return: None
******************************************************************************/
void pp_hal_set_gama(uint8_t value)
{

	uint8_t bi;

	if (value < 9) {	/*1.4, 1.3, 1.2, 1.1, 1.0, 0.9, 0.8, 0.7, 0.6 */

		pp_hal_sce_init();
		PP_INFO("pp_hal_set_gamabvalue=%d\n", value);
		for (bi = 0; bi < LUMA_HIST_LEVEL; bi++) {
			vRegWt4B(Y_FTN_1_0 + (4 * bi),
				 (_gamma_tbl[value][bi * 2]) | (_gamma_tbl[value][bi * 2 + 1] <<
								16));
		}
		/*vRegWt2B(Y_FTN_32_, _gamma_tbl[valu [32]);  */

		pp_hal_boost_chroma(0);
		vRegWt4BMsk(CFG_MAIN, (0x00 << 0), COLORBP);
		pp_hal_boost_chroma(1);
	} else {
		PP_INFO("Set Gamma Fail\n");
	}
}
