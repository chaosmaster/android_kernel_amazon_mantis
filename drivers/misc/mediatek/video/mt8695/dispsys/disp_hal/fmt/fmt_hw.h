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


#ifndef _FMT_HW_H_
#define _FMT_HW_H_

#include <linux/types.h>

#include "disp_type.h"

#define HAL_DISP_FMT_MAIN_REG_NUM     (0x100/4)
#define HAL_DISP_FMT_SUB_REG_NUM      (0x100/4)
#define HAL_VDOUT_FMT_REG_NUM     (0x100/4)

/**********************************************************************
** define register
***********************************************************************/
/*for vdout fmt*/
#define VDOUT_FMT_REG_MIX_REORDER				0x1C
#define VDOUT_FMT_REG_PREMIX					0x24
#define VDOUT_FMT_REG_FMT_MODE					0x94
#define VDOUT_FMT_REG_ACTIVE_H					0xA0
#define VDOUT_FMT_REG_ODD_ACTIVE_V				0xA4
#define VDOUT_FMT_REG_EVEN_ACTIVE_V				0xA8
#define VDOUT_FMT_REG_FMT_CTRL					0xAC
#define VDOUT_FMT_REG_BG_COLOR					0xB8
#define VDOUT_FMT_REG_FMT_HV_TOTAL				0xD4
#define VDOUT_FMT_REG_MULTI						0xE4
#define VDOUT_FMT_REG_MIXER_MODE				0xF4
#define VDOUT_FMT_REG_PLANE						0xF8
#define VDOUT_FMT_REG_RATIO						0xFC

/*for disp fmt*/
#define DISP_FMT_REG_Y_H_COFF0					0x00
#define DISP_FMT_REG_Y_H_COFF19					0x4C
#define DISP_FMT_REG_DERING						0x54
#define DISP_FMT_REG_DOWN_CTRL					0x6C
#define DISP_FMT_REG_DOWN_RNG_H					0x70
#define DISP_FMT_REG_DOWN_RNG_VO				0x74
#define DISP_FMT_REG_DOWN_RNG_VE				0x78
#define DISP_FMT_REG_DOWN_RNG_OUTPUT			0x7C
#define DISP_FMT_REG_MODE_CTRL					0x94
#define DISP_FMT_REG_PIXEL_LENGTH				0x9C
#define DISP_FMT_REG_ACTIVE_H					0xA0
#define DISP_FMT_REG_ODD_ACTIVE_V				0xA4
#define DISP_FMT_REG_EVEN_ACTIVE_V				0xA8
#define DISP_FMT_REG_VDO_FMT_CTRL				0xAC
#define DISP_FMT_REG_HORIZONTAL_SCALING			0xB0
#define DISP_FMT_REG_BG_COLOR					0xB8
#define DISP_FMT_REG_DOWN_CONFIG				0xC0
#define DISP_FMT_REG_Y_LIMITATION				0xC4
#define DISP_FMT_REG_CONFIG						0xCC
#define DISP_FMT_REG_HV_TOTAL_MIX				0xD0
#define DISP_FMT_REG_HV_TOTAL					0xD4
#define DISP_FMT_REG_RATIO						0xE4
#define DISP_FMT_REG_DELAY						0xE8
#define DISP_FMT_REG_DSD_DELAY					0xEC
#define DISP_FMT_REG_FORMAT_422_CTRL			0xF0

#define BYTE_PER_32BIT	4

 /* 0x42000 */
struct disp_fmt_field_t {
	/* DWORD - 000 */
	uint32_t u4YHCOEF0:32;

	/* DWORD - 004 */
	uint32_t u4YHCOEF1:32;

	/* DWORD - 008 */
	uint32_t u4YHCOEF2:32;

	/* DWORD - 00C */
	uint32_t u4YHCOEF3:32;

	/* DWORD - 010 */
	uint32_t u4YHCOEF4:32;

	/* DWORD - 014 */
	uint32_t u4YHCOEF5:32;

	/* DWORD - 018 */
	uint32_t u4YHCOEF6:32;

	/* DWORD - 01C */
	uint32_t u4YHCOEF7:32;

	/* DWORD - 020 */
	uint32_t u4YHCOEF8:32;

	/* DWORD - 024 */
	uint32_t u4YHCOEF9:32;

	/* DWORD - 028 */
	uint32_t u4YHCOEF10:32;

	/* DWORD - 02C */
	uint32_t u4YHCOEF11:32;

	/* DWORD - 030 */
	uint32_t u4YHCOEF12:32;

	/* DWORD - 034 */
	uint32_t u4YHCOEF13:32;

	/* DWORD - 038 */
	uint32_t u4YHCOEF14:32;

	/* DWORD - 03C */
	uint32_t u4YHCOEF15:32;

	/* DWORD - 040 */
	/* ASYNC MVDO adjustment */
	uint32_t u4ASYNC_HSYN_DELAY:12;	/* horizontal shift of output hsync relative to input vsync */
	 uint32_t:4;
	uint32_t u4ASYNC_VSYN_DELAY:12;	/* vertical shift of output vsync relative to input vsync */
	uint32_t fgASYNC_ADJ_FORWARD:1;
	 uint32_t:3;

	/* DWORD - 044 */
	/* Horizontal Active Zone */
	uint32_t u4ASYNC_VDO_HACTEND:13;	/* Pixel position of the end of horizontal active area */
	 uint32_t:3;
	uint32_t u4ASYNC_VDO_HACTBGN:13;	/* Pixel position of the beginning of horizontal active area */
	 uint32_t:2;
	uint32_t fgASYNC_MODE_EN:1;	/* HD/SD scale up to 4K ASYNC mode enable */

	/* DWORD - 048 : H_FIR_MANUAL  0x42048 */
	uint32_t fgV1_Y:1;	/* 0 */
	uint32_t fgV1_C:1;	/* 1 */
	uint32_t fgV1_NO_SYM:1;	/* 2 */
	uint32_t fgV1_SYM_OPT:1;	/* 3 */
	uint32_t u1V1_Y_MSB:2;	/* 5:4 */
	uint32_t u1V1_C_MSB:2;	/* 7:6 */
	 uint32_t:2;		/* 9:8 */
	uint32_t fgC_16_PHAEE:1;	/* 10 */
	uint32_t fgC_EVEN_FILTER:1;	/* 11 */
	 uint32_t:20;		/* 31:12 */

	/* DWORD - 04C */
	uint32_t u4YHCOEF19:32;

	/* DWORD - 050 */
	uint32_t fgCRC_INIT:1;
	uint32_t fgCRC_CLR:1;
	 uint32_t:6;
	uint32_t u4CRC_RLT:24;

	/* DWORD - 054 : DERING */
	uint32_t u2DERING_THRESHOLD_Y:12;
	uint32_t u2DERING_THRESHOLD_C:12;
	uint32_t u1DERING_TRANS:4;
	uint32_t fgDERING_EN_Y:1;
	uint32_t fgDERING_EN_C:1;
	uint32_t fgDERING_EN_EX:1;
	 uint32_t:1;

	/* DWORD - 058 : BORDER_CONTROL */
	 uint32_t:16;
	uint32_t u1BD_X_WDH:6;
	 uint32_t:2;
	uint32_t u1BD_Y_WDH:6;
	 uint32_t:1;
	uint32_t fgBD_ON:1;

	/* DWORD - 05C : BORDER_COLOR */
	 uint32_t:2;
	uint32_t u1BD_Y:6;
	 uint32_t:2;
	uint32_t u1BD_CB:6;
	 uint32_t:2;
	uint32_t u1BD_CR:6;
	 uint32_t:8;

	/* DWORD - 060 */
	 uint32_t:32;

	/* DWORD - 064 */
	 uint32_t:32;

	/* DWORD - 068 */
	 uint32_t:32;

	/* DWORD - 06C */
	uint32_t fgHDWN_EN:1;
	 uint32_t:1;
	uint32_t u1HDWN_TT_MD:2;
	 uint32_t:1;
	uint32_t fgFULL_FIFO:1;
	 uint32_t:2;
	uint32_t fgHDWN_422:1;
	uint32_t fgSIDE_BLK:1;
	uint32_t fgSIDE_LNR:1;
	uint32_t fgHDWN_444:1;
	uint32_t u1HDWN_OPT:4;
	uint32_t u2HDWN_FAC:16;

	/* DWORD - 070 */
	uint32_t u2HDWN_HEND:13;
	 uint32_t:3;
	uint32_t u2HDWN_HBGN:12;
	 uint32_t:1;
	uint32_t u2HDWN_2F:1;
	 uint32_t:1;
	uint32_t fgHDWN_ALL:1;

	/* DWORD - 074 : DOWN_SCALER_RANGE_1 */
	uint32_t u2HDWN_VOEND:12;
	uint32_t:1;
	uint32_t u2HDWN_HEND_14_13:2;
	uint32_t:1;
	uint32_t u2HDWN_VOBGN:12;
	uint32_t:1;
	uint32_t u2HDWN_HBEG_14_13:2;
	uint32_t:1;

	/* DWORD - 078 : DOWN_SCALER_RANGE_2 */
	uint32_t u2HDWN_VEEND:12;
	 uint32_t:4;
	uint32_t u2HDWN_VEBGN:12;
	 uint32_t:4;

	/* DWORD - 07C */
	uint32_t u2HDWN_HO_END:13;
	uint32_t u2HDWN_CYCLE_2_0:3;
	uint32_t u2HDWN_HO_BGN:13;
	uint32_t u1HDWN_VO_POS:3;

	/* DWORD - 080 */
	uint32_t fgNR_ADJ_SYNC_EN:1;
	uint32_t fgNR_ADJ_FWD:1;
	uint32_t fgNR_TOGGLE_OPT:1;
	 uint32_t:5;
	uint32_t u2NR_HSYNC_DELAY:12;
	uint32_t u2NR_VSYNC_DELAY:12;

	/* DWORD - 084 */
	uint32_t u2NR_VSYNC_END:11;
	 uint32_t:1;
	uint32_t u2NR_VSYNC_SRT:11;
	 uint32_t:1;
	uint32_t fgNR_VSYNC_POLAR:1;
	uint32_t fgNR_HSYNC_POLAR:1;
	uint32_t fgNR_FIELD_POLAR:1;
	uint32_t fgNR_DE_SELF:1;
	uint32_t fgNR_ODD_V_SRT_OPT:1;
	uint32_t fgNR_ODD_V_END_OPT:1;
	uint32_t fgNR_EVEN_V_SRT_OPT:1;
	uint32_t fgNR_EVEN_V_END_OPT:1;

	/* DWORD - 088 */
	uint32_t u2NR_HOR_END:12;
	 uint32_t:4;
	uint32_t u2NR_HOR_SRT:12;
	 uint32_t:4;


	/* DWORD - 08C */
	uint32_t u2NR_VO_END:11;
	 uint32_t:5;
	uint32_t u2NR_VO_SRT:11;
	 uint32_t:5;

	/* DWORD - 090 */
	uint32_t u2NR_VE_END:11;
	 uint32_t:5;
	uint32_t u2NR_VE_SRT:11;
	 uint32_t:5;

	/* DWORD - 094 0x42094 */
	uint32_t u1HSYNWIDTH:8;
	uint32_t u1VSYNWIDTH:5;
	uint32_t fgHD_TP:1;	/* [13] 1:HD 720 format,0:HD 1080 format */
	uint32_t fgHD_ON:1;	/* [14] 1:HD mode(1080i/720p),0:SD mode(480/576) */
	uint32_t fgPRGS:1;	/* [15] Progressive output enable */
	 uint32_t:1;
	uint32_t fgPRGS_AUTOFLD:1;
	uint32_t fgPRGS_INVFLD:1;
	 uint32_t:1;
	uint32_t fgYUV_RST_OPT:1;
	 uint32_t:3;
	uint32_t fgPRGS_FLD:1;
	uint32_t fgNEW_SD_4SEL1_EN:1;	/* 4 line sel 1 line mode enable */
	uint32_t fgNEW_SD_USE_LOW:1;	/* 4 line sel low 2 line */
	uint32_t fgDSDCLK_EANBLE:1;	/* NEW_SD_144MHz */
	uint32_t fgNEW_SD_MODE:1;
	uint32_t fgNEW_SD_USE_EVEN:1;
	uint32_t u1TVMODE:2;	/* [31:30] TV mode selection 00 NTSC 01 PAL_N 10 PAL_M 11 PAL_BDGHI */

	/* DWORD - 098 */
	 uint32_t:16;
	uint32_t u1PF_ADV:4;
	uint32_t u1LUMACONTROL:12;

	/* DWORD - 09C */
	uint32_t u2PXLLEN:13;
	 uint32_t:2;
	uint32_t FMT_VP_RESET_SELECT:1;
	uint32_t u2RIGHT_SKIP:4;
	 uint32_t:11;
	uint32_t fgNEW_4K_DOWN_MODE:1;	/* new 4k scaling down mode enable */

	/* DWORD - 0A0  0x420A0 */
	uint32_t u2HACTEND:13;
	 uint32_t:3;
	uint32_t u2HACTBGN:12;
	 uint32_t:1;
	uint32_t fgLayer3_En:1;
	uint32_t fgPmx_From_NR:1;
	uint32_t fgLayer3_Trigger:1;

	/* DWORD - 0A4 */
	uint32_t u2VOACTEND:12;
	 uint32_t:4;
	uint32_t u2VOACTBGN:12;
	/* uint32_t                              : 1; */
	uint32_t u1HIDE_OST:4;

	/* DWORD - 0A8 */
	uint32_t u2VEACTEND:12;
	 uint32_t:4;
	uint32_t u2VEACTBGN:12;
	/* uint32_t                              : 1; */
	uint32_t u1HIDE_EST:4;

	/* DWORD - 0AC : VIDEO_FORMATTER_CONTROL */
	uint32_t fgVDO_EN:1;
	uint32_t fgFMTM:1;	/* set video formatter in master mode */
	 uint32_t:1;
	uint32_t fgHPOR:1;
	uint32_t fgVPOR:1;
	 uint32_t:2;
	 uint32_t:1;
	uint32_t u4PXLSEL:2;
	uint32_t fgFTRST:1;
	uint32_t fgSHVSYN:1;
	 uint32_t:2;
	uint32_t u1SYN_DEL:2;
	 uint32_t:3;
	uint32_t fgUVSW:1;
	 uint32_t:5;
	uint32_t fgBLACK:1;	/* Set screen to all black */
	 uint32_t:1;
	uint32_t fgPFOFF:1;
	uint32_t u1HW_OPT:4;

	/* DWORD - 0B0 : HORIZONTAL_SCALING 0x420B0 */
	uint32_t fgHSON:1;
	uint32_t fgHSLR:1;
	uint32_t fgLPF_ON:1;
	uint32_t fgLPF_SL:1;
	uint32_t u1ED_YUV_ST:2;
	uint32_t fgO_DIS:1;
	uint32_t fgHDLPF:1;
	uint32_t u1YACC_ST:4;
	uint32_t u1CACC_ST:4;
	uint32_t u2HSFACTOR:14;
	uint32_t fgEVEN_FIR:1;
	uint32_t fgPHASE_16:1;

	/* DWORD - 0B4 : BUILD_IN_COLOR */
	 uint32_t:4;
	uint32_t u1BIY:4;
	 uint32_t:4;
	uint32_t u1BICB:4;
	 uint32_t:4;
	uint32_t u1BICR:4;
	uint32_t fgPF2OFF:1;
	uint32_t fgHIDE_L:1;
	 uint32_t:6;

	/* DWORD - 0B8 : BACKGROUND_COLOR */
	 uint32_t:4;
	uint32_t u1BGY:4;
	 uint32_t:4;
	uint32_t u1BGCB:4;
	 uint32_t:4;
	uint32_t u1BGCR:4;
	 uint32_t:8;

	/* DWORD - 0BC */
	uint32_t u1EDGE_RATIO:5;
	uint32_t fgKNEE:1;
	uint32_t fgZCORE:1;
	uint32_t fgDNR:1;
	uint32_t u1CORE:4;
	uint32_t u1DOUT_CTL:3;
	 uint32_t:17;

	/* DWORD - 0C0 */
	uint32_t u2DowndScaleEnd:13;
	uint32_t u2Cycle_4_3:2;
	uint32_t u24K_TO_480OUTDSD:1;
	uint32_t u2DowndScaleStart:13;
	uint32_t u2On_H_target_sta_out:3;

	/* DWORD - 0C4 */
	uint32_t u2C4Default:16;
	uint32_t fgOLD_C_ACC:1;
	 uint32_t:3;		/* bit18: OUT_SEL2 */
	uint32_t fgTVE_ND:1;
	 uint32_t:3;
	uint32_t u1FIRST_PXL_LEAD:8;

	/* DWORD - 0C8 */
	uint32_t NEW_SCL_MODE_EN:1;
	uint32_t POST_SCL_USE_AC:1;
	 uint32_t:14;
	uint32_t POST_SCL_WINDOW_LINEAR_SIZE_SEL:2;
	uint32_t POST_SCL_WINDOW_ACC_SIZE_SEL:2;
	uint32_t POST_SCL_PRE_DATA_NEXT_LUMA_Y_OPTION:1;	/* 2'b00:2, 2'b01:4, 2'b10:6, 2'b11:8 */
	uint32_t POST_SCL_NEXT_DATA_PRE_LUMA_Y_OPTION:1;
	uint32_t POST_SCL_PRE_DATA_NEXT_LUMA_C_OPTION:1;
	uint32_t POST_SCL_NEXT_DATA_PRE_LUMA_C_OPTION:1;
	 uint32_t:7;
	uint32_t POST_DIV2_SEL:1;

	/* 0x420CC */
	uint32_t u4ScalerFactor:16;
	uint32_t fgDemoModeEnable:1;	/* [16] */
	uint32_t fgDemoModeLeft:1;	/* [17] */
	 uint32_t:2;
	uint32_t u2DemoModeWidth:12;

	/* 0x420D0 */
	/* DWORD - 0D0 */
	uint32_t u2V_TOTAL_MIX:12;
	 uint32_t:4;
	uint32_t u2H_TOTAL_MIX:13;
	 uint32_t:2;
	uint32_t fgADJ_T_MIX:1;

	/* DWORD - 0D4  0x420D4 */
	uint32_t u2V_TOTAL:12;
	 uint32_t:4;
	uint32_t u2H_TOTAL:13;
	 uint32_t:2;
	uint32_t fgADJ_T:1;

	/* DWORD - 0D8 */
	uint32_t INC_END:12;
	uint32_t INC_START:12;
	uint32_t INC_STEP:8;

	/* DWORD - 0DC */
	uint32_t DEC_END:12;
	uint32_t DEC_START:12;
	uint32_t INIT_FACTOR:8;

	/* DWORD - 0E0 */
	uint32_t FORCE_NEW_SCL_MODE_PATH_CLOCK_EN:1;
	uint32_t FORCE_OLD_SCL_MODE_PATH_CLOCK_EN:1;
	 uint32_t:30;

	/* DWORD - 0E4 */
	uint32_t fgMULTI_RATIO:1;
	uint32_t fgDIRECT:1;
	uint32_t fgC_FMTRST_M2:1;
	uint32_t fgC_CHK_A_SCA:1;	/* 0x423E4[3] luma key option */
	uint32_t fgY_ALL_8TAP_OUT:1;
	uint32_t fgFACT_PREC:1;
	uint32_t fgNOT_PST_D2:1;
	uint32_t fgC_ALL_8TAP_OUT:1;
	uint32_t fgHD_C_FIR_EN:1;
	uint32_t u1LPF_SEL:3;
	uint32_t fgUsePhase32:1;
	uint32_t fgHD_C_POS:1;
	uint32_t fgRST_PHASE:1;
	uint32_t fgDemoModeHoriEnable:1;
	/* MULTI_EXTEND[7:0] */
	uint32_t INIT_FACTOR_0:1;	/* bit 16 */
	uint32_t INIT_FACTOR_9:1;	/* bit 17 */
	uint32_t INC_STEP_0:1;	/* bit 18 */
	 uint32_t:1;		/* bit 19 */
	uint32_t DEC_END_12:1;	/* bit 20 */
	uint32_t DEC_START_12:1;	/* bit 21 */
	uint32_t INC_END_12:1;	/* bit 22 */
	uint32_t INC_START_12:1;	/* bit 23 */
	uint32_t fgMVDO_4TAP_ONLY:1;
	uint32_t fgMATCH_2T:1;
	uint32_t fgOLD_NEXT_YC:1;
	uint32_t fgNO_SUB_1:1;
	uint32_t fgEN_FIRST_PXL_LEAD:1;
	uint32_t fgDownScaleMode:1;
	uint32_t fg8TapCSControl:1;
	 uint32_t:1;

	/* 0x420E8 */

	/* DWORD - 0E8 */
	uint32_t u2HSYN_DELAY:13;
	 uint32_t:3;
	uint32_t u2VSYN_DELAY:13;
	 uint32_t:2;
	uint32_t fgADJ_FWD:1;

	/* DWORD - 0EC */
	uint32_t u2DSDHSYN_DELAY:13;
	 uint32_t:3;
	uint32_t u2DSDVSYN_DELAY:13;
	 uint32_t:2;
	uint32_t fgDSDADJ_FWD:1;

	/* DWORD - 0F0 */
	uint32_t LPF_ON:1;  /* [0] */
	uint32_t LPF_ROUND:1;   /* [1] */
	uint32_t LPF_TYPE:1;    /* [2] */
	uint32_t LPF11_SEL:1; /* [3] */
	uint32_t:8;    /* [11:4] */
	uint32_t UV_STA:2; /* [13:12] */
	uint32_t:4;    /*[17:14]*/
	uint32_t  Y_DEL:2; /* [19:18] */
	uint32_t  CB_DEL:2; /* [21:20] */
	uint32_t  CR_DEL:2; /* [23:22] */
	uint32_t  OUTPUT_444:1; /* [24] */
	uint32_t:7;    /* [31:25] */

	/* DWORD - 0F4 */
	 uint32_t:32;

	/* DWORD - 0F8 */
	uint32_t fgCCONV_VP1:1;
	uint32_t fgCC7TO6_VP1:1;
	 uint32_t:2;
	uint32_t fgAverage422to444:1;	/* C110 */
	uint32_t fgOLD_CHROMA:1;
	uint32_t fg235_TO_255_EN:1;
	uint32_t fgDATA_235_255:1;
	 uint32_t:20;
	uint32_t fgAdap_Chroma:1;	/* [28] */
	 uint32_t:3;

	/* DWORD - 0FC */
	uint32_t u1LUMA_KEY:12;
	 uint32_t:4;
	uint32_t WINDOW_LINEAR_SIZE_SEL:2;
	uint32_t WINDOW_ACC_SIZE_SEL:2;
	uint32_t PRE_DATA_NEXT_LUMA_Y_OPTION:1;
	uint32_t NEXT_DATA_PRE_LUMA_Y_OPTION:1;
	uint32_t PRE_DATA_NEXT_LUMA_C_OPTION:1;
	uint32_t NEXT_DATA_PRE_LUMA_C_OPTION:1;
	 uint32_t:7;
	uint32_t POST_DIV2_SEL_2:1;
};

union disp_fmt_union_t {
	uint32_t au4Reg[HAL_DISP_FMT_MAIN_REG_NUM];
	struct disp_fmt_field_t rField;
};


/* 0x3000 */
struct vdout_fmt_field_t {
	/* DWORD - 000 */
	uint32_t u2PST_HACTEND:12;
	 uint32_t:4;
	uint32_t u2PST_HACTBGN:12;
	 uint32_t:4;

	/* DWORD - 004 */
	uint32_t u2PST_VOACTEND:12;
	 uint32_t:4;
	uint32_t u2PST_VOACTBGN:12;
	 uint32_t:4;

	/* DWORD - 008 */
	uint32_t u2PST_VEACTEND:12;
	 uint32_t:4;
	uint32_t u2PST_VEACTBGN:12;
	 uint32_t:4;

	/* DWORD - 00C */
	uint32_t fgMIX_PLN1_MSK_SEL:1;
	uint32_t fgMIX_PLN2_MSK_SEL:1;
	uint32_t fgPST_MIX_PL1_MSK_SEL:1;
	uint32_t fgPST_MIX_PL2_MSK_SEL:1;
	uint32_t fgMIX_PLN1_BD_EN:1;
	uint32_t fgMIX_PLN2_BD_EN:1;
	uint32_t fgPST_MIX_PLN1_BD_EN:1;
	uint32_t fgPST_MIX_PLN2_BD_EN:1;
	uint32_t fgDGI_INBUF_TST:1;
	uint32_t fgDGI_FIFO_IN_TST:1;
	uint32_t fgDGI_FIFO_OUT_TST:1;
	uint32_t fgDGI_LAST_STG_TST:1;
	 uint32_t:4;
	uint32_t u2PST_HSYNC_DELAY:12;
	uint32_t fgPST_HSYNC_DELAY_EN:1;
	uint32_t fgPST_H_DE_OPT1:1;
	uint32_t fgPST_H_DE_OPT2:1;
	uint32_t fgFRM_L_O_EDGE_SEL:1;

	/* DWORD - 010 */
	uint32_t fgDGI_ON:1;
	uint32_t fgDGI_FMT:1;
	uint32_t fgMTK_DATA_EN:1;
	uint32_t fg10BIT_EN:1;
	uint32_t fg12BIT_EN:1;
	uint32_t fgSWAP_YC:1;
	uint32_t fgINV_BIT:1;
	uint32_t fgFIFO_RST_SEL:1;
	uint32_t fgFIFO_SW_RST:1;
	uint32_t fgFIFO_VSYNC_POL:1;
	uint32_t u1YUV_SEL:2;
	uint32_t fgFIFO_HSYNC_POL:1;
	 uint32_t:3;
	uint32_t u1FIFO_OUT_CR:2;
	uint32_t u1FIFO_OUT_CB:2;
	uint32_t u1FIFO_OUT_Y:2;
	uint32_t fgMIXING_MD:1;
	uint32_t fgSWAP_UV:1;
	uint32_t fgMSB_EN:1;
	uint32_t fgFIFO_RST_ON:1;
	uint32_t fgTST_MD_ON:1;
	 uint32_t:1;
	uint32_t u1MD_CODE:4;

	/* DWORD - 014 */
	uint32_t u1AVState:4;
	 uint32_t:22;
	uint32_t fgIN_HSYN_OUT:1;
	uint32_t fgIN_VSYN_OUT:1;
	uint32_t fgTST_HSYN_IN:1;
	uint32_t fgTST_VSYN_IN:1;
	uint32_t fgHSYN_IN_PAD:1;
	uint32_t fgVSYN_IN_PAD:1;

	/* DWORD - 018 */
	uint32_t fgSWAP_P3_P4_1:1;
	uint32_t fgMIX_PST_OFF:1;
	uint32_t fgSWAP_P1_P2_PST:1;
	uint32_t fgCCONV_VP1_PST:1;
	uint32_t fgCC7TO6_VP1_PST:1;
	uint32_t fgCCONV_VP2_PST:1;
	uint32_t fgCC7TO6_VP2_PST:1;
	 uint32_t:3;
	uint32_t fgVDOUT_TIMING_TTD:2;
	 uint32_t:4;
	uint32_t u1CBAR_WIDTH:8;
	uint32_t fgDGI_MIX_OSD_EN:1;
	uint32_t fgDGI_4FS_EN:1;
	uint32_t fgDGI_2FS_EN:1;
	 uint32_t:2;
	uint32_t fgH_MAX_2FS:1;
	uint32_t fgDGI_VSYNC_INT_SEL:1;
	uint32_t fgVSYNC_120HZ:1;

	/* DWORD - 01C */
	uint32_t u1Pln0Sel:3;
	 uint32_t:1;
	uint32_t u1Pln1Sel:3;
	 uint32_t:1;
	uint32_t u1Pln2Sel:3;
	 uint32_t:1;
	uint32_t u1Pln3Sel:3;
	 uint32_t:1;
	uint32_t u1Pln4Sel:3;
	 uint32_t:1;
	uint32_t u1Pln5Sel:3;
	 uint32_t:1;
	uint32_t u1Pln6Sel:3;
	 uint32_t:1;
	uint32_t u1Pln7Sel:3;
	 uint32_t:1;

	/* DWORD - 020 */
	uint32_t u2POST_HSYNC_DLY:12;
	 uint32_t:4;
	uint32_t u2POST_VSYNC_DLY:12;
	 uint32_t:3;
	uint32_t fgPOST_ADJ_FORWARD:1;

	/* DWORD - 024 */
	uint32_t fgPLN2_PRE_MODE:1;
	uint32_t fgPLN3_PRE_MODE:1;
	uint32_t fgPLN4_PRE_MODE:1;
	uint32_t fgPLN5_PRE_MODE:1;
	uint32_t fgPLN6_PRE_MODE:1;
	uint32_t fgPLN7_PRE_MODE:1;
	uint32_t fgPLN8_PRE_MODE:1;
	uint32_t fgDGI_OSD_PRE_MODE:1;
	uint32_t u1DGI_OSD_PRE_SEL:3;
	uint32_t fgDGO_12BIT:1;
	uint32_t fgDGO_8bit:1;
	uint32_t fgDGI_OSD1_PRE_MODE:1;
	uint32_t fgDGI_MIX1_ON:1;
	 uint32_t:1;
	uint32_t u1COO_SEL:3;
	uint32_t fgCOO_PIN2_ACT_OPT:1;
	 uint32_t:2;
	uint32_t fgTEST_3D_ENABLE:1;
	 uint32_t:1;
	uint32_t u1TTD_SRC_SEL:3;
	 uint32_t:5;

	/* DWORD - 028 */
	 uint32_t:32;

	/* DWORD - 02C */
	 uint32_t:32;

	/* DWORD - 030 */
	uint32_t u4POST_VDO1_HACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO1_HACTBGN:12;
	 uint32_t:4;

	/* DWORD - 034 */
	uint32_t u4POST_VDO1_VOACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO1_VOACTBGN:12;
	 uint32_t:4;

	/* DWORD - 038 */
	uint32_t u4POST_VDO1_VEACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO1_VEACTBGN:12;
	 uint32_t:4;

	/* DWORD - 03C */
	uint32_t u4DGI_H_TOTAL_DETECT:12;
	 uint32_t:4;
	uint32_t u4DGI_V_TOTAL_DETECT:12;
	 uint32_t:4;

	/* DWORD - 040 */
	uint32_t u4POST_VDO2_HACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO2_HACTBGN:12;
	 uint32_t:4;

	/* DWORD - 044 */
	uint32_t u4POST_VDO2_VOACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO2_VOACTBGN:12;
	 uint32_t:4;

	/* DWORD - 048 */
	uint32_t u4POST_VDO2_VEACTEND:12;
	 uint32_t:4;
	uint32_t u4POST_VDO2_VEACTBGN:12;
	 uint32_t:4;

	/* DWORD - 04C */
	uint32_t u4DGI_H_WIDTH:12;
	 uint32_t:4;
	uint32_t u4DGI_V_WIDTH:12;
	uint32_t fgDGI_HSYNC_POL:1;
	uint32_t fgDGI_VSYNC_POL:1;
	uint32_t fgDGI_FIELD_POL:1;
	uint32_t fgDGI_OSD_TIMING_EN:1;

	/* DWORD - 050 */
	uint32_t fg656_EN:1;
	uint32_t fgPRGS_AUTOFLD:1;
	uint32_t fgPRGS_INVFLD:1;
	uint32_t fgP2I_FLD:1;
	uint32_t fgODD_FLD_SEL:1;
	uint32_t fgYUV_OUT_DIS:1;
	uint32_t fgYUV_OUT_SEL:1;
	uint32_t fgSWAP_DGOYC:1;
	uint32_t fgSMPTE_AUTOFLD:1;
	uint32_t fgCCIR_MOD:1;
	uint32_t fgDGO_NO_MIX:1;
	uint32_t fgESAVI:1;
	uint32_t u1DGO_MOD_CODE:4;
	uint32_t fgUSE_FLD:1;
	uint32_t fgFLD_INV:1;
	uint32_t fgADJ_SYN_FWD:1;
	uint32_t fgSYN_LENGTH_SEL:1;
	uint32_t fgADJ_SYN_EN:1;
	 uint32_t:2;
	uint32_t fgC_ESAV_D3:1;
	uint32_t u1DGO_SYN_DEL:2;
	uint32_t fgC_DEL:1;
	uint32_t fgHSYNC_POL:1;
	uint32_t fgVSYNC_POL:1;
	uint32_t fgDE_POL:1;
	uint32_t fgCLK_EDGE0:1;
	uint32_t fgCLK_EDGE1:1;

	/* DWORD - 054 */
	uint32_t fgDGO_LPF_EN:1;
	uint32_t fgLPF_ROUND:1;
	uint32_t fgDGO_LPF_TYPE:1;
	uint32_t fgDGO_LPF_11_SEL:1;
	uint32_t fgC_DGO_LMT_TOP:1;
	uint32_t fgC_DGO_LMT_BOT:1;
	uint32_t fgC_DGO_LMT_656C:1;
	uint32_t fgC_DGO_LMT_CBCR:1;
	uint32_t fgC_10B_DGO:1;
	uint32_t fgC_8_10_RND:1;
	uint32_t fgC_DGO_Y_INV:1;
	uint32_t fgC_DGO_C_INV:1;
	uint32_t fgDGO_YUV_SEL_ADJ:2;
	uint32_t fgC_UV_SYN_EN:1;
	uint32_t fgC_INV_TVEFLD:1;
	uint32_t fgC_ITLC_TOPLONG:1;
	uint32_t fgC_DGO_LMT_FF00:1;
	uint32_t u1DGO_Y_DEL:2;
	uint32_t u1DGO_CB_DEL:2;
	uint32_t u1DGO_CR_DEL:2;
	uint32_t fgDIS_ESAV_CODE:1;
	uint32_t fgSD_YC_16BIT:1;
	uint32_t u1SMPTE_C_YUV_SEL:2;
	uint32_t fgDGO_444:1;
	uint32_t fgRESET_V_SYN_CNT:1;
	uint32_t fgFRM_LO_SEL:1;
	uint32_t fgFRM_LO_USE_DE:1;

	/* DWORD - 058 */
	 uint32_t:6;
	uint32_t fgHDDVD_LKEY_ON:1;
	uint32_t fgHDDVD_LKEY_TP:1;
	uint32_t u1HDDVD_LKEY_END:8;
	uint32_t u1BD_X_WDH:6;
	 uint32_t:2;
	uint32_t u1BD_Y_WDH:6;
	 uint32_t:2;

	/* DWORD - 05C */
	 uint32_t:4;
	uint32_t u1BD_Y:4;
	 uint32_t:4;
	uint32_t u1BD_CB:4;
	 uint32_t:4;
	uint32_t u1BD_CR:4;
	 uint32_t:8;

	/* DWORD - 060 */
	uint32_t u2DE_SCL_HEND:12;
	 uint32_t:4;
	uint32_t u2DE_SCL_HBGN:12;
	 uint32_t:4;

	/* DWORD - 064 */
	uint32_t u2DE_SCL_VOEND:12;
	 uint32_t:4;
	uint32_t u2DE_SCL_VOBGN:12;
	 uint32_t:4;

	/* DWORD - 068 */
	uint32_t u2DE_SCL_VEEND:12;
	 uint32_t:4;
	uint32_t u2DE_SCL_VEBGN:12;
	 uint32_t:4;

	/* DWORD - 06C */
	 uint32_t:32;

	/* DWORD - 070 */
	 uint32_t:32;

	/* DWORD - 074 */
	 uint32_t:32;

	/* DWORD - 078 */
	 uint32_t:32;

	/* DWORD - 07C */
	 uint32_t:32;

	/* DWORD - 080 */
	 uint32_t:16;
	uint32_t u2SMPTE_SHIFT100:11;
	uint32_t fgSMPTE_F_POL:1;
	uint32_t fgSMPTE_V_POL:1;
	uint32_t fgSMPTE_ON:1;
	uint32_t fgSMPTE_SHIFT11:1;
	 uint32_t:1;

	/* DWORD - 084 */
	uint32_t fgMST_EN:1;
	uint32_t fgMST_HD:1;
	uint32_t fgMST_HD_TP:1;
	uint32_t fgMST_PRGS:1;
	 uint32_t:11;
	uint32_t u1MST_V_TOTAL:4;
	 uint32_t:1;
	uint32_t u2MST_H_TOTAL:12;

	/* DWORD - 088 */
	 uint32_t:32;

	/* DWORD - 08C */
	 uint32_t:32;

	/* DWORD - 090 */
	 uint32_t:32;

	/* DWORD - 094 */
	uint32_t u1HSYNWIDTH:8;
	uint32_t u1VSYNWIDTH:5;
	uint32_t fgHD_TP:1;
	uint32_t fgHD_ON:1;
	uint32_t fgPRGS:1;
	 uint32_t:14;
	uint32_t u1TVMODE:2;

	/* DWORD - 098 */
	uint32_t u2REG_1098:11;
	 uint32_t:5;
	uint32_t u1PF_ADV:4;
	 uint32_t:4;
	uint32_t u1SMPTE_ACT_BGN_LINE:8;

	/* DWORD - 09C */
	 uint32_t:32;

	/* DWORD - 0A0 */
	uint32_t u2HACTEND:13;
	 uint32_t:3;
	uint32_t u2HACTBGN:12;
	 uint32_t:4;

	/* DWORD - 0A4 */
	uint32_t u2VOACTEND:12;
	 uint32_t:4;
	uint32_t u2VOACTBGN:12;
	/* uint32_t                              : 1; */
	uint32_t u1HIDE_OST:4;

	/* DWORD - 0A8 */
	uint32_t u2VEACTEND:12;
	 uint32_t:4;
	uint32_t u2VEACTBGN:12;
	/* uint32_t                              : 1; */
	uint32_t u1HIDE_EST:4;

	/* DWORD - 0AC */
	uint32_t fgFMT_ON:1;
	uint32_t fgFMTM:1;
	 uint32_t:1;
	uint32_t fgHPOR:1;
	uint32_t fgVPOR:1;
	 uint32_t:3;
	uint32_t u1PXLSEL:2;
	uint32_t fgFTRST:1;
	uint32_t fgSYNC_SEL:1;
	 uint32_t:1;
	uint32_t fgDGOCLK_POL:1;
	uint32_t u1SYN_DEL:2;
	 uint32_t:12;
	uint32_t fgLayer3_En:1;
	uint32_t u1HW_OPT:2;
	uint32_t fgLayer3_Trigger:1;

	/* DWORD - 0B0 */
	 uint32_t:4;
	uint32_t u1ED_YUV_ST:2;
	 uint32_t:26;

	/* DWORD - 0B4 */
	 uint32_t:4;
	uint32_t u1BIY:4;
	 uint32_t:4;
	uint32_t u1BICB:4;
	 uint32_t:4;
	uint32_t u1BICR:4;
	uint32_t fgPF2OFF:1;
	uint32_t fgHIDE_L:1;
	 uint32_t:6;

	/* DWORD - 0B8 */
	 uint32_t:4;
	uint32_t u1BGY:4;
	 uint32_t:4;
	uint32_t u1BGCB:4;
	 uint32_t:4;
	uint32_t u1BGCR:4;
	 uint32_t:8;

	/* DWORD - 0BC */
	 uint32_t:12;
	uint32_t u1DOUT_CTL:3;
	 uint32_t:1;
	uint32_t u1DGO_Y_CHN_SEL:2;
	uint32_t u1DGO_C_CHN_SEL:2;
	uint32_t u1DGO_C2_CHN_SEL:2;
	 uint32_t:10;

	/* DWORD - 0C0 */
	uint32_t u1VAR_WDH_7_0:8;
	uint32_t u1SPEED:6;
	 uint32_t:2;
	uint32_t u1VAR_WDH_10_8:3;
	 uint32_t:13;

	/* DWORD - 0C4 */
	uint32_t u1Y_LMT_TOP:8;
	uint32_t u1Y_LMT_BOT:8;
	 uint32_t:4;
	uint32_t fgTVF_ND:1;
	 uint32_t:1;
	uint32_t fgSPL_S:1;
	uint32_t fgFRMLST_S:1;
	uint32_t u1FIRST_PXL_LEAD:8;

	/* DWORD - 0C8 */
	uint32_t u1VAR1_WDH_7_0:8;
	uint32_t u1SPEED1:6;
	 uint32_t:2;
	uint32_t u1VAR1_WDH_10_8:3;
	 uint32_t:13;

	/* DWORD - 0CC */
	 uint32_t:1;
	uint32_t fgPFDAT:1;
	uint32_t fgPFDINV:1;
	 uint32_t:1;
	uint32_t fgNO_U:1;
	 uint32_t:4;
	uint32_t gDCK90P:1;
	 uint32_t:22;

	/* DWORD - 0D0 */
	uint32_t u2CCIR_HEND:12;
	 uint32_t:4;
	uint32_t u2CCIR_HBGN:12;
	 uint32_t:3;
	uint32_t fgCCIR:1;

	/* DWORD - 0D4 */
	uint32_t u2V_TOTAL:13;
	 uint32_t:3;
	uint32_t u2H_TOTAL:13;
	 uint32_t:2;
	uint32_t fgADJ_T:1;

	/* DWORD - 0D8 */
	 uint32_t:32;

	/* DWORD - 0DC */
	 uint32_t:32;

	/* DWORD - 0E0 */
	uint32_t u2CCIR_VOEND:11;
	 uint32_t:5;
	uint32_t u2CCIR_VOBGN:11;
	 uint32_t:4;
	uint32_t fgPFLD:1;

	/* DWORD - 0E4 */
	 uint32_t:1;
	uint32_t fgDIRECT:1;
	uint32_t fgMAS_CNT_SYNC:1;
	 uint32_t:2;
	uint32_t fgFACT_PREC:1;
	uint32_t fgNOT_PST_D2:1;
	uint32_t fgMAGIC:1;
	uint32_t fgHD_C_FIR_EN:1;
	uint32_t u1LPF_SEL:3;
	uint32_t fgEXT_YUV_SEL:1;
	uint32_t fgMIXER_OFF:1;
	uint32_t fgRST_PHASE:1;
	uint32_t u1Prgs_for_osd_3d:1;
	 uint32_t:16;

	/* DWORD - 0E8 */
	uint32_t u2HSYN_DELAY:12;
	uint32_t u2VSYN_DELAY:4;
	uint32_t u2ADJ_H_L:10;
	uint32_t u1ADJ_V_L:6;

	/* DWORD - 0EC */
	 uint32_t:32;

	/* DWORD - 0F0 */
	uint32_t u2CCIR_VEEND:11;
	 uint32_t:5;
	uint32_t u2CCIR_VEBGN:11;
	 uint32_t:5;

	/* DWORD - 0F4 */
	uint32_t fgCB_ON:1;
	uint32_t fgCB_TP:1;
	uint32_t fgMOPT2:1;
	uint32_t fgMOPT3:1;
	uint32_t u1Y_DELAY:2;
	 uint32_t:1;
	uint32_t fgCC7TO6_AP:1;
	uint32_t fgCC7TO6:1;
	uint32_t fgCCONV_AP_EN:1;
	uint32_t fgCCONV_EN:1;
	uint32_t fgMOPT11:1;
	uint32_t u1DGO_SEL:2;
	uint32_t fgDGO_LPF:1;
	uint32_t fgDGO_LP_RND:1;
	uint32_t fgDGO_LPF_TP:1;
	uint32_t fgDGO_LP11SEL:1;
	 uint32_t:1;
	uint32_t fgMOPT19:1;
	uint32_t u1SVDO_POS:2;
	uint32_t fgSPU_SIGN:1;
	 uint32_t:1;
	uint32_t u1DOWN_SRC:4;
	uint32_t fgDN_LPF:1;
	uint32_t fgDN_LP_RND:1;
	uint32_t fgDN_LPF_TP:1;
	uint32_t fgDN_LP11SEL:1;

	/* DWORD - 0F8 */
	uint32_t fgPLN3_PG:1;
	 uint32_t:1;
	uint32_t fgCCONV_VP1:1;
	uint32_t fgCC7TO6_VP1:1;
	uint32_t fgVDOUT_CCONV:1;
	uint32_t fgSCLER_CCONV:1;
	uint32_t fgCCONV_VP2:1;
	uint32_t fgCC7TO6_VP2:1;
	 uint32_t:1;
	uint32_t fgOSD_DLY:1;
	uint32_t fgPLN3_DLY:1;
	uint32_t fgPLN4_DLY:1;
	uint32_t fgPLN5_DLY:1;
	uint32_t fgPLN6_DLY:1;
	uint32_t fgPLN7_DLY:1;
	uint32_t fgPLN8_DLY:1;
	uint32_t fgPLN2_OFF:1;
	uint32_t fgPLN3_OFF:1;
	uint32_t:6;
	uint32_t fgSWAP1_2:1;
	uint32_t fgSWAP3_4:1;
	uint32_t fgSWAP6_7:1;
	uint32_t fgSWAP4_7:1;
	uint32_t fgSWAP1_3:1;
	uint32_t fgSWAP5_8:1;
	uint32_t fgSCL_NO_UI:1;
	uint32_t fgSCL_8_TURN:1;

	/* DWORD - 0FC */
	uint32_t u1VDO1_RATIO:8;	/* [7:0] */
	uint32_t u1VDO2_RATIO:8;	/* [15:8] */
	uint32_t fgVDO1_R_EN:1;	/* [16] */
	uint32_t fgVDO2_R_EN:1;	/* [17] */
	uint32_t fgIS_LUMA_KEY:1;	/* [18] */
	uint32_t fgCLR_RECT:1;	/* [19] */
	uint32_t u1LUMA_KEY:12;	/* [31:20] */
};

union vdout_fmt_union_t {
	uint32_t au4Reg[HAL_VDOUT_FMT_REG_NUM];
	struct vdout_fmt_field_t rField;
};

#endif				/* _PMX_HW_H_ */
