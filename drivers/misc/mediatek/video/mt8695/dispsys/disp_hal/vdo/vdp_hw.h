/*
 * Copyright (C) 2016 MediaTek Inc.
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




#ifndef _VDP_HW_H
#define _VDP_HW_H

#include <linux/types.h>


#define HAL_VDO1_OFST			0x400
#define HAL_VDO2_OFST			0x400
#define HAL_VDO3_UFO_OFST		0x200
#define HAL_VDO3_OFST			0x400
#define HAL_VDO4_UFO_OFST		0x200
#define HAL_VDO4_OFST			0x400
#define HAL_VDO3_SCL_OFST		0x100
#define HAL_VDO4_SCL_OFST		0x100



#define HAL_VDO_REG_NUM			(0x100/4)

#define HAL_VDO_UFOD_REG_NUM		(0x100/4)

#define HAL_VDO_ADV_REG_NUM		(0x100/4)

#define HAL_VDO_SCL_REG_NUM		(0xBC/4)

#define HAL_VDO_APQ_REG_NUM		(0x100/4)

#define HAL_VDO_FUSION_REG_NUM		(0x100/4)

#define HAL_VDO_CS_REG_NUM		(0x100/4)

#define HAL_MVDO_RESET_ALL		0xFF
#define HAL_MVDO_RESET_CLEAR		0x0

#define VDO_LAYER_COUNT			2	/*hw video layer base count */



#if 1
/* 0x42100, 0x43100, 0x45100, 0x45900 */
struct vdo_hal_scl_field {
	/* DWORD - 000 */
	uint32_t YCoef00:32;

	/* DWORD - 004 */
	uint32_t YCoef01:32;

	/* DWORD - 008 */
	uint32_t YCoef02:32;

	/* DWORD - 00C */
	uint32_t YCoef03:32;

	/* DWORD - 010 */
	uint32_t YCoef04:32;

	/* DWORD - 014 */
	uint32_t YCoef05:32;

	/* DWORD - 018 */
	uint32_t YCoef06:32;

	/* DWORD - 01C */
	uint32_t YCoef07:32;

	/* DWORD - 020 */
	uint32_t YCoef08:32;

	/* DWORD - 024 */
	uint32_t YCoef09:32;

	/* DWORD - 028 */
	uint32_t YCoef0A:32;

	/* DWORD - 02C */
	uint32_t YCoef0B:32;

	/* DWORD - 030 */
	uint32_t YCoef0C:32;

	/* DWORD - 034 */
	uint32_t YCoef0D:32;

	/* DWORD - 038 */
	uint32_t YCoef0E:32;

	/* DWORD - 03C */
	uint32_t YCoef0F:32;

	/* DWORD - 040 */
	uint32_t YCoef10:32;

	/* DWORD - 044 */
	 uint32_t:32;

	/* DWORD - 048 */
	 uint32_t:32;

	/* DWORD - 04C */
	 uint32_t:13;
	uint32_t V8TAP_16PHASE:1;
	uint32_t V8TAP_EVEN_FIR:1;
	uint32_t V8TAP_C110:1;
	uint32_t V8TAP_DERECT_0:1;
	 uint32_t:1;
	uint32_t INIT_8TAP_CRC:1;
	uint32_t CLR_8TAP_CRC:1;
	uint32_t use_4tap_only:1;
	uint32_t old_next_YC:1;
	 uint32_t:2;
	uint32_t V8TAP_C_ON:1;
	uint32_t V8TAP_Y_ON:1;
	uint32_t V8TAP_Y_LINEAR:1;
	uint32_t V8TAP_C_LINEAR:1;
	uint32_t MSB_Padding:1;
	 uint32_t:3;

	/* DWORD - 050 */
	uint32_t CCoef00:32;

	/* DWORD - 054 */
	uint32_t CCoef01:32;

	/* DWORD - 058 */
	uint32_t CCoef02:32;

	/* DWORD - 05C */
	uint32_t CCoef03:32;

	/* DWORD - 060 */
	uint32_t CCoef04:32;

	/* DWORD - 064 */
	uint32_t CCoef05:32;

	/* DWORD - 068 */
	uint32_t CCoef06:32;

	/* DWORD - 06C */
	uint32_t CCoef07:32;

	/* DWORD - 070 */
	uint32_t CCoef08:32;

	/* DWORD - 074 */
	uint32_t CCoef09:32;

	/* DWORD - 078 */
	uint32_t CCoef0A:32;

	/* DWORD - 07C */
	uint32_t CCoef0B:32;

	/* DWORD - 080 */
	uint32_t CCoef0C:32;

	/* DWORD - 084 */
	uint32_t CCoef0D:32;

	/* DWORD - 088 */
	uint32_t CCoef0E:32;

	/* DWORD - 08C */
	uint32_t CCoef0F:32;

	/* DWORD - 090 */
	uint32_t CCoef10:32;

	/* DWORD - 094 */
	uint32_t V8TAP_VALID_START:11;
	 uint32_t:5;
	uint32_t V8TAP_VALID_END:11;
	 uint32_t:5;

	/* DWORD - 098 */
	uint32_t crc_result_8tap:24;
	 uint32_t:8;

	/* DWORD - 09C */
	uint32_t V3TAP_THRESHOLD:6;
	 uint32_t:2;
	uint32_t V3TAP_EN:1;
	uint32_t V3TAP_DEMO_EN:1;
	 uint32_t:22;

	/* DWORD - 0A0 *//* 0x421A0 */
	uint32_t LINE_BUF_LEFT_RIGHT_START:7;
	uint32_t:8;
	uint32_t:1;
	uint32_t:15;
	uint32_t BUFFER_SP_USE_REG_OFFSET:1;

	/* DWORD - 0A4 */
	uint32_t OW_RIGHT_START:9;
	uint32_t:23;

	/* DWORD - 0A8 */
	 uint32_t:32;


	/* DWORD - 0AC */
	 uint32_t:32;

	/* DWORD - 0B0 *//* vdo sharp ctrl 01 */
	uint32_t GAIN_Band1:8;
	uint32_t LIMIT_POS_Band1:8;
	uint32_t Limit_NEG_Band1:8;
	uint32_t Coring_Band1:8;

	/* DWORD - 0B4 *//* vdo sharp ctrl 02 */
	uint32_t CLIP_POS_Band1:8;
	uint32_t CLIP_NEG_Band1:8;
	uint32_t CLIP_EN_Band1:2;
	uint32_t CLIP_SEL_Band1:1;
	 uint32_t:1;
	uint32_t SHRINGK_SEL_Band1:3;
	uint32_t SHARP_EN_Band1:1;
	 uint32_t:8;

	/* DWORD - 0B8 */
	uint32_t LIMIT_POS:8;
	uint32_t LIMIT_NEG:8;
	uint32_t PREC_Band1:2;
	uint32_t SHIFT:2;
	uint32_t FILTER_SEL_Band1:1;
	 uint32_t:3;
	uint32_t BYPASS_ShARP:1;
	 uint32_t:7;
};

union vdo_hal_scl_union {
	uint32_t reg[HAL_VDO_SCL_REG_NUM];
	struct vdo_hal_scl_field field;
};

/* 0x42400, 0x43400, 0x45400, 0x45C00 */
struct vdo_hal_field {
	/* DWORD - 000 */
	uint32_t VDOY1:29;	/* Y */
	 uint32_t:3;

	/* DWORD - 004 */
	uint32_t VDOC1:29;
	 uint32_t:3;

	/* DWORD - 008 */
	uint32_t VDOY2:29;	/* X */
	 uint32_t:3;

	/* DWORD - 00C */
	uint32_t VDOC2:29;
	 uint32_t:3;

	/* DWORD - 010  0x42410 */
	uint32_t HBLOCK:8;	/* frame buffer widht / 8 */
	uint32_t DW_NEED:8;	/* pixel width / 4 */
	uint32_t PIC_HEIGHT:12;	/*for MT8580 4kx2k */
	uint32_t HBLOCK_EXT:2;	/*for MT8580 4kx2k high 2 bit */
	uint32_t DW_NEED_BIT10:1;
	uint32_t swap_off:1;

	/* DWORD - 014 */
	uint32_t VSCALE:16;
	uint32_t P_SKIP:12;	/*for MT8580 4kx2k */
	 uint32_t:2;
	uint32_t SHADOW_MODE:1;
	uint32_t ENABLE_TO_LAYER2:1;

	/* DWORD - 018 */
	uint32_t Y_STAMBR:8;
	uint32_t C_STAMBR:8;
	uint32_t MAX_MBR:8;
	uint32_t TOTAL_MBR:8;

	/* DWORD - 01C  0x4241c */
	uint32_t VDOEN:1;
	uint32_t RACE:1;
	uint32_t YLAVG:1;
	uint32_t CLAVG:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t YLR:1;
	uint32_t CLR:1;
	 uint32_t:1;
	 uint32_t:1;
	/*[10] DRAM address swapping enable */
	uint32_t ADRSW:1;
	/*[11] Horizental sample reduce to half (for progressive mode) */
	uint32_t XHALF:1;
	 uint32_t:1;		/*[12] */
	uint32_t APF:1;		/*[13] */
	uint32_t Y2FF:1;	/*[14] */
	uint32_t C2FF:1;	/*[15] */
	uint32_t TFLD:1;	/*[16] */
	uint32_t MI2P:1;	/*[17] */
	/*[18] force all picture regions as still regions */
	uint32_t STILL:1;
	uint32_t MOVE:1;	/*[19] */
	uint32_t PD32_F:1;	/*[20] */
	uint32_t DISP3:1;	/*[21] */
	uint32_t PD32:1;	/*[22] */
	uint32_t DEL_BK:1;	/*[23] */
	uint32_t Tst0:1;	/*[24] */
	uint32_t Tst1:1;	/*[25] */
	uint32_t Tst2:1;	/*[26] */
	uint32_t Tst3:1;	/*[27] */
	uint32_t AutoRst:1;	/*[28] */
	uint32_t Tst5:1;	/*[29] */
	uint32_t F_CMB:1;	/*[30] */
	uint32_t Tst7:1;	/*[31] */

	/* DWORD - 020 */
	uint32_t YSLTT:12;
	 uint32_t:4;
	uint32_t YSLBB:11;
	 uint32_t:5;

	/* DWORD - 024 */
	uint32_t CSLTT:12;
	 uint32_t:4;
	uint32_t CSLBB:11;
	 uint32_t:5;

	/* DWORD - 028 */
	uint32_t YSSLTT:8;
	uint32_t YSSLBB:8;
	uint32_t YSSLBT:8;
	uint32_t YSSLTB:8;

	/* DWORD - 02C */
	uint32_t CSSLTT:8;
	uint32_t CSSLBB:8;
	uint32_t CSSLBT:8;
	uint32_t CSSLTB:8;

	/* DWORD - 030  0x42430 */
	uint32_t FRMY:1;	/*[0] */
	uint32_t FRMC:1;	/*[1] */
	uint32_t PFLD:1;	/*[2] */
	 uint32_t:1;		/*[3] */
	 uint32_t:1;		/*[4] */
	uint32_t AFLD:1;	/*[5] */
	uint32_t UAFLD:1;	/*[6] */
	 uint32_t:1;		/*[7] */
	 uint32_t:1;		/*[8] */
	uint32_t CR80:1;	/*[9] */
	 uint32_t:8;		/* 17:10 */
	 uint32_t:1;		/*[18] */
	 uint32_t:1;		/*[19] */
	uint32_t SRAM:1;	/*[20] */
	uint32_t YUV422:1;	/*[21] */
	 uint32_t:1;		/*[22] */
	 uint32_t:1;
	uint32_t YSL0:1;
	uint32_t CSL0:1;
	uint32_t YS3L:1;
	uint32_t YS3_NA:1;
	uint32_t AYS3L_DIS:1;
	uint32_t WY_PF:1;
	uint32_t NF5S_1ST:1;
	 uint32_t:1;

	/* DWORD - 034  0x42434 */
	uint32_t:16;	/*[15:0] */
	uint32_t DITHER_MODE:3; /* [18:16]*/
	uint32_t DITHER_EN:1;	/*[19]*/
	uint32_t:12;	/*31:20*/

	/* DWORD - 038 */
	uint32_t RBTH_WA:3;
	 uint32_t:5;
	uint32_t CT_THRD:8;
	uint32_t MTHRD:8;
	uint32_t CTHRD:7;
	 uint32_t:1;

	/* DWORD - 03C */
	uint32_t MBR_Cnt:7;
	uint32_t VDOV:1;
	uint32_t CS_MBR:7;
	uint32_t TV_fld:1;
	uint32_t YL_MBR:7;
	uint32_t swap:1;
	uint32_t Y_LINE:8;

	/* DWORD - 040 */
	uint32_t FLD_DYN_8F_LOW:8;
	uint32_t FLD_DYN_8F_HIGH:8;
	uint32_t FLD_DYN_8F_CNT:8;
	uint32_t M8F_XZ:1;
	uint32_t M8F_XZ_SEL:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t DYN_INV:1;
	/* DYN_8F enable dynamic 8-field motion detection */
	uint32_t DYN_8F:1;
	uint32_t SCN_OFF:1;
	uint32_t COMB_8F_MODE7:1;

	/* DWORD - 044 */
	uint32_t SAW_DYN_8F_LOW:8;
	uint32_t SAW_DYN_8F_HIGH:8;
	uint32_t SAW_DYN_8F_CNT:8;
	uint32_t SAW_8F_MODE:8;

	/* DWORD - 048 */
	uint32_t SAW_DYN_2_LOW:8;
	uint32_t SAW_DYN_2_HIGH:8;
	uint32_t SAW_DYN_3_LOW:8;
	uint32_t SAW_DYN_3_HIGH:8;

	/* DWORD - 04C */
	uint32_t COMB_DYN_2_LOW:8;
	uint32_t COMB_DYN_2_HIGH:8;
	uint32_t COMB_DYN_3_LOW:8;
	uint32_t COMB_DYN_3_HIGH:8;

	/* DWORD - 050 */
	uint32_t YSLBT:12;
	 uint32_t:4;
	uint32_t YSLTB:11;
	 uint32_t:5;

	/* DWORD - 054 */
	uint32_t CSLBT:12;
	 uint32_t:4;
	uint32_t CSLTB:11;
	 uint32_t:5;

	/* DWORD - 058 */
	uint32_t MBAVG1:29;
	 uint32_t:3;

	/* DWORD - 05C */
	uint32_t MBAVG2:29;
	 uint32_t:3;

	/* DWORD - 060 */
	uint32_t comb_rst:1;
	uint32_t fld_YX_comb_read_reg_sel:19;
	uint32_t region_level_0_5_reg:12;

	/* DWORD - 064 */
	uint32_t GET3F:1;
	uint32_t PHA1:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t GMLINE:1;
	uint32_t UMLINE:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t NLS:1;
	uint32_t NLS2:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t YI2P:1;
	uint32_t CI2P:1;
	 uint32_t:1;
	uint32_t SP_LD:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t M_AVG:1;
	uint32_t F_STL:1;
	uint32_t NSCL:1;
	 uint32_t:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t S_CMB:1;
	 uint32_t:1;
	 uint32_t:1;
	 uint32_t:1;
	 uint32_t:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t NSC:1;

	/* DWORD - 068 */
	uint32_t M3F_TH:8;
	uint32_t M3F_LO:8;
	 uint32_t:7;
	 uint32_t:5;
	uint32_t YS_CR_D:2;
	 uint32_t:1;
	uint32_t MBL_S2:1;

	/* DWORD - 06C */
	uint32_t Y_STA_T:5;
	 uint32_t:3;
	uint32_t Y_STA_B:5;
	 uint32_t:3;
	uint32_t C_STA_T:3;
	uint32_t DL_MD:1;
	uint32_t DL_2ND:1;
	 uint32_t:3;
	uint32_t C_STA_B:3;
	 uint32_t:5;

	/* DWORD - 070 */
	uint32_t FLT_TW:1;
	uint32_t CF_TW:1;
	uint32_t M_EXP:1;
	uint32_t S_EXP:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t OPT0:1;
	 uint32_t:1;
	uint32_t OPT2:1;
	uint32_t OPT3:1;
	uint32_t U_MA4F:1;
	uint32_t OPT4:1;
	uint32_t NL_CS:1;
	uint32_t SEL_F3:1;
	uint32_t PF_S2:1;
	uint32_t CPF_S2:1;
	 uint32_t:2;
	 uint32_t:1;
	uint32_t OPT7:1;
	uint32_t OPT8:1;
	uint32_t OPT9:1;
	uint32_t CSCON:1;
	uint32_t CSCEN:1;
	 uint32_t:1;
	uint32_t ST_EX_6F:1;
	 uint32_t:1;
	 uint32_t:1;
	uint32_t EHF_DIS:1;
	uint32_t OPT12:1;
	 uint32_t:1;
	uint32_t OPT14:1;

	/* DWORD - 074 */
	uint32_t fld_XZ_motion_red_reg_sel:20;
	uint32_t region_level_11_6_reg:12;

	/* DWORD - 078   0x42478 */
	uint32_t YFIR_ON:1;	/* 0 */
	uint32_t YFIR_LNR:1;	/* 1 */
	uint32_t GAU62_15_0575:1;	/* 2 */
	uint32_t GAU62_15_0675:1;	/* 3 */
	uint32_t YFIR_CF_PRG:1;	/* 4 */
	uint32_t PH16:1;	/* 5 */
	uint32_t EVN_FIR:1;	/* 6 */
	uint32_t MIRROR_EN:1;	/* 7 */
	 uint32_t:3;		/*8-10 */
	/* [11] turn intra mode with edge preserving */
	uint32_t INTRA_EDGEP_MODE:1;
	uint32_t HD_SCALING_DOWN_FORCE:1;
	uint32_t PE_4LINE_FORCE:1;	/*[13]co-used for MT8560 */
	uint32_t PE_4LINE_DIS:1;
	uint32_t DRAM_TST_MODE:1;
	uint32_t DOWN_SCL_4TAP_FORCE:1;	/* 16 */
	uint32_t CSP_DIS:1;	/* 17 */
	uint32_t YS5L:1;	/* 18 */
	uint32_t AYS5L_DIS:1;	/* 19 */
	uint32_t YS4L:1;	/* 20 */
	uint32_t AYS4L_DIS:1;	/* 21 */
	uint32_t YS2L:1;	/* 22 */
	uint32_t AYS2L_DIS:1;	/* 23 */
	uint32_t YSL:1;		/* 24 */
	uint32_t AYSL_DIS:1;	/* 25 */
	uint32_t SP_CNT:1;	/* 26 */
	uint32_t YC_PF:1;	/* 27 */
	uint32_t SPF_LIM:4;	/*[28:31] */

	/* DWORD - 07C  0x4247c */
	/*[1:0]  5351 swap mode selection */
	uint32_t swap_mode_selection:2;
	 uint32_t:1;
	uint32_t enable_10bit_read:1;
	uint32_t Video_Opt4:1;
	uint32_t Video_Opt5:1;
	uint32_t Video_Opt6:1;
	uint32_t Video_Opt7:1;
	uint32_t Video_Opt8:1;
	uint32_t swap_mode_enable:1;	/* [9]  swap mode enable */
	uint32_t Video_Opt10:1;
	uint32_t Video_Opt11:1;
	uint32_t PROT_WR_STA:1;
	uint32_t PROT_WR_END:1;
	uint32_t burst_length_enable:1;	/* 14 */
	uint32_t Video_Opt15:1;
	uint32_t AddrSwapMode:1;
	uint32_t Video_Opt17:1;
	uint32_t cross_MBL_cycle_sel:1;
	uint32_t p_track_32_Line:1;
	uint32_t Video_Opt20:1;
	uint32_t Video_Opt21:1;
	uint32_t Video_Opt22:1;
	uint32_t Video_Opt23:1;
	uint32_t Video_Opt24:1;
	uint32_t luma_histogram_input_selection:1;
	uint32_t CRC_Region:1;
	uint32_t Video_Opt27:1;
	uint32_t Video_Opt28:1;
	uint32_t Video_Opt29:1;
	uint32_t Video_Opt30:1;
	uint32_t Video_Opt31:1;

	/* DWORD - 080 */
	uint32_t PTR_WF_Y:30;
	 uint32_t:2;

	/* DWORD - 084 */
	uint32_t PTR_ZF_Y:30;
	 uint32_t:2;

	/* DWORD - 088 */
	uint32_t FDIFF_TH3:24;
	uint32_t MA4F:1;
	uint32_t REGION_BLENDING_EN:1;
	uint32_t REGION_BLENDING_MIN:1;
	uint32_t MAX_PIX_FDIFF:1;
	uint32_t BP_YC:1;
	uint32_t ASAW:2;
	uint32_t MA6F:1;

	/* DWORD - 08C */
	uint32_t FDIFF_TH2:24;
	uint32_t MA_Video_Mode0:1;
	uint32_t MA_Video_Mode1:1;
	uint32_t MA_Video_Mode2:1;
	uint32_t MA_Video_Mode3:1;
	uint32_t MA_Video_Mode4:1;
	uint32_t MA_Video_Mode5:1;
	uint32_t MA_Video_Mode6:1;
	uint32_t MA_Video_Mode7:1;

	/* DWORD - 090 */
	uint32_t FDIFF_TH1:24;
	uint32_t MA_HW_Option0:1;
	uint32_t MA_HW_Option1:1;
	uint32_t MA_HW_Option2:1;
	uint32_t MA_HW_Option3:1;
	uint32_t MA_HW_Option4:1;
	uint32_t MA_HW_Option5:1;
	uint32_t MA_HW_Option6:1;
	uint32_t MA_HW_Option7:1;

	/* DWORD - 094 */
	uint32_t TH_MIN_XZ:10;
	 uint32_t:2;
	uint32_t TH_MED_XZ:10;
	 uint32_t:2;
	uint32_t MA_TST_MODE:8;

	/* DWORD - 098 */
	uint32_t TH_NORM_XZ:10;
	 uint32_t:2;
	uint32_t TH_EDGE_XZ:10;
	 uint32_t:2;
	uint32_t H_ED_TH:8;

	/* DWORD - 09C */
	uint32_t TH_MIN_YW:9;
	 uint32_t:3;
	uint32_t TH_MED_YW:9;
	 uint32_t:3;
	uint32_t SAW_TH:8;

	/* DWORD - 0A0 */
	uint32_t TH_NM_YW:9;
	 uint32_t:3;
	uint32_t TH_ED_YW:9;
	 uint32_t:3;
	uint32_t WH_TX_TH:8;

	/* DWORD - 0A4 */
	uint32_t FCH_MIN_XZ:10;
	 uint32_t:2;
	uint32_t FCH_NM_XZ:10;
	 uint32_t:2;
	uint32_t VMV_FCH:8;

	/* DWORD - 0A8 */
	uint32_t FCH_MIN_YW:9;
	 uint32_t:3;
	uint32_t FCH_NM_YW:9;
	 uint32_t:1;
	uint32_t CRM_LVL:2;
	uint32_t FR_LVL:2;
	uint32_t X_POS_ST:6;

	/* DWORD - 0AC */
	uint32_t CRM_SAW:8;
	uint32_t TV_LINE_ST:8;
	uint32_t FD_CNT:8;
	uint32_t MA_QUALITY_MODE:3;
	uint32_t APPLY_TV_LINE_SET:1;
	uint32_t OTHERS:4;

	/* DWORD - 0B0 */
	uint32_t EDGE_P_TH:8;
	uint32_t EDGE_VERT_TH:8;
	uint32_t EDGE_CROSS_TH:8;
	uint32_t MA_EDGE_MODE0:1;
	uint32_t MA_EDGE_MODE1:1;
	uint32_t MA_EDGE_MODE2:1;
	uint32_t MA_EDGE_MODE3:1;
	uint32_t MA_EDGE_MODE4:1;
	uint32_t MA_EDGE_MODE5:1;
	uint32_t MA_EDGE_MODE6:1;
	uint32_t MA_EDGE_MODE7:1;

	/* DWORD - 0B4 */
	uint32_t EDGE_63D_TH:8;
	uint32_t EDGE_ABS_GRAD_TH:8;
	uint32_t EDGE_UD_RESTRICT_TH:8;
	uint32_t MA_EDGE_ADV_CTRL:8;

	/* DWORD - 0B8   424b8 ege misc ellaneous thresholds */
	uint32_t EDGE_MULTI_EDGE_TH:8;
	uint32_t EDGE_MEDGE_CNT_TH:8;
	uint32_t EDGE_3LINE_GRAD_TH:8;
	uint32_t MA_EDGE_MISC:8;

	/* DWORD - 0BC */
	uint32_t EDGE_HDIFF_TH:8;
	uint32_t EDGE_EXP_TH:8;
	uint32_t EDGE_V3_CTRL:8;
	uint32_t EDGE_V3_QUAL_0:1;
	uint32_t EDGE_V3_QUAL_1:7;

	/* DWORD - 0C0   0x424c0 motion detection advance */
	uint32_t EXP_MOTION:1;
	uint32_t EXP_STILL:1;
	uint32_t EXP_2PT:1;
	uint32_t BLEND_EXP_OFF:1;
	uint32_t GET5F:1;
	uint32_t WA_NA24:1;
	uint32_t SAW_5LA:1;	/*[6] */
	uint32_t SAW_5L:1;
	uint32_t ADPT_ICP_TH:8;
	uint32_t VAC_6F:4;
	uint32_t MD_EXP:1;
	uint32_t VT_BL:1;
	uint32_t OLD_SAW:1;
	uint32_t MA5F:1;
	/*[24] 6/8-field motion detection dram write disable */
	uint32_t NO_M_W:1;
	uint32_t MA8F_OR:1;	/*[25] 8-field motion detection */
	uint32_t FIX_ICP:1;	/*[26]  fix ICP enable */
	uint32_t ADPT_FIX_ICP:1;
	/*[28] 6/8-field motion also apply edge-preserving interpolation */
	uint32_t ECTL_6F:1;
	uint32_t CUE_6F:1;
	uint32_t BLEND_EXP_OFF_6F:1;
	uint32_t SPTH_6F:1;

	/* DWORD - 0C4   0x424c4 pull down field like */
	uint32_t PD_LINE_UNLIKE_TH:8;
	uint32_t PD_LINE_UNLIKE_INTV:5;
	 uint32_t:3;
	uint32_t PD_COMB_TH:7;	/* comb counter threshold */
	 uint32_t:1;
	uint32_t VDO_32_PD_EN:1;
	 uint32_t:1;
	uint32_t VDO_32_PD_MODE:1;
	 uint32_t:1;
	uint32_t PD_CTRL_MODE_HD_EN:1;
	uint32_t PD_CTRL_MODE_rest:3;

	/* DWORD - 0C8  0x424c8 pull down band pass filter */
	uint32_t LUMA_KEY_TH:8;	/* [7:0] luma key threshold */
	uint32_t BD_BPF_THRD:8;
	uint32_t FIFO_UNDERRUN_CNT:7;
	uint32_t FIFO_UNDERRUN_SEL:1;
	uint32_t PD_UL_SEL:1;
	uint32_t LM_KEY:1;	/* [25] luma key enable */
	uint32_t LUMAKEY_CHROMA_SEL:1;
	 uint32_t:5;

	/* DWORD - 0CC */
	uint32_t PD_DST_START:8;
	uint32_t PD_DST_END:8;
	uint32_t SUBTITLE_THRD:8;
	uint32_t SUBTITLE_ERASE_EN:1;
	uint32_t SUBTITLE_REG_EN:1;
	uint32_t SUBTITLE_REG_VDO:1;
	 uint32_t:5;

	/* DWORD - 0D0  0x424d0 choma motion detection */
	/* motion 3-field motion detection */
	uint32_t CRM_3FMD:1;
	uint32_t CRM_FDIFF:1;
	uint32_t CRM_EXP_OFF:1;
	uint32_t C_INTER_X:1;
	uint32_t C_VT_121:1;
	uint32_t C_VT_BLEND:1;
	uint32_t CRM_3LSAW:1;
	uint32_t CHROMA_SAW_CNT_4LINE_SEL:1;
	uint32_t CRM_DIFF:8;
	uint32_t FDIFF_LMT:1;
	uint32_t FDIFF_SAW:1;
	uint32_t FDIFF_ADJ:2;
	 uint32_t:4;
	uint32_t CRM_MOTION_CNT_TH:8;

	/* DWORD - 0D4 */
	uint32_t PD_FLD_LIKE_TH:16;
	uint32_t PD_SCN_CHG_TH:8;
	uint32_t DYN_8F_THRD:8;

	/* DWORD - 0D8 */
	uint32_t MBAvg3:28;
	 uint32_t:4;

	/* DWORD - 0DC */
	uint32_t PROT_END_ADDR:28;
	 uint32_t:4;

	/* DWORD - 0E0  0x424E0 */
	uint32_t DW_NEED_HD:9;	/*[8:0] */
	uint32_t MA_X_R_SCL:3;	/*[11:9] */
	uint32_t MA_Y_R_SCL:2;	/*[13:12]    */
	uint32_t HD_C:6;	/*[19:14] */
	uint32_t HD_LINE_MODE:1;	/*[20] */
	uint32_t HD_MEM_1920:1;	/*[21] */
	uint32_t HD_MEM:1;	/* 22 */
	/*[23] scan-line based dram address enable */
	uint32_t SLE:1;
	uint32_t HD_EN:1;	/*24 */
	uint32_t DN_FLT:1;
	uint32_t ND_MR_END:1;
	uint32_t DW_NEED_BIT9:1;	/*for MT8580 4kx2k */
	uint32_t:1;
	uint32_t F_L_SEL:1;
	uint32_t I_IN_P:1;
	uint32_t F_PRGS:1;

	/* DWORD - 0E4 */
	uint32_t RM_YF_Y:3;
	uint32_t RM_YY_EN:1;
	uint32_t RM_YF_C:3;
	uint32_t RM_YC_EN:1;
	uint32_t RM_XF_Y:3;
	uint32_t RM_XY_EN:1;
	uint32_t RM_XF_C:3;
	uint32_t RM_XC_EN:1;
	uint32_t RM_ZF_Y:3;
	uint32_t RM_ZY_EN:1;
	uint32_t RM_ZF_C:3;
	uint32_t RM_ZC_EN:1;
	uint32_t RM_WF_Y:3;
	uint32_t RM_WY_EN:1;
	uint32_t RM_AF_Y:3;
	uint32_t RM_AY_EN:1;

	/* DWORD - 0E8 */
	uint32_t RR_YY_EN:1;
	uint32_t RR_YY_SEL:1;
	uint32_t RR_YC_EN:1;
	uint32_t RR_YC_SEL:1;
	uint32_t RR_XY_EN:1;
	uint32_t RR_XY_SEL:1;
	uint32_t RR_XC_EN:1;
	uint32_t RR_XC_SEL:1;
	uint32_t RR_ZY_EN:1;
	uint32_t RR_ZY_SEL:1;
	uint32_t RR_ZC_EN:1;
	uint32_t RR_ZC_SEL:1;
	uint32_t RR_WY_EN:1;
	uint32_t RR_WY_SEL:1;
	uint32_t RR_AY_EN:1;
	uint32_t RR_AY_SEL:1;
	 uint32_t:16;

	/* DWORD - 0EC */
	uint32_t PTR_AF_Y:29;
	 uint32_t:3;

	/* DWORD - 0F0 */
	uint32_t FLD_WY_MOTION:20;
	uint32_t Motion_Level_24Region_1217:12;

	/* DWORD - 0F4 */
	uint32_t FLD_WX_COMB:20;
	uint32_t Motion_Level_24Region_1823:12;

	/* DWORD - 0F8 */
	uint32_t WMV_DISABLE:32;

	/* DWORD - 0FC */
	uint32_t PTR_ZF_C:29;
	 uint32_t:3;
};

union vdo_hal_union {
	uint32_t reg[HAL_VDO_REG_NUM];
	struct vdo_hal_field field;
};

/* 0x45200, 0x45A00 */
struct vdo_hal_ufo_field {
	/*DWORD - 000 */
	uint32_t PTR_TO_Y:32;

	/*DWORD - 004 */
	uint32_t PTR_TO_C:32;

	/*DWORD - 008 */
	uint32_t H_WIDTH:12;
	 uint32_t:8;
	uint32_t V_HEIGTH:12;

	/*DWORD - 00C */
	uint32_t LINE_PITCH:10;
	 uint32_t:22;

	/*DWORD - 010 */
	uint32_t PTR_TO_Y_LENGTH:32;

	/*DWORD - 014 */
	uint32_t PTR_TO_C_LENGTH:32;

	/*DWORD - 018 */
	uint32_t HalfLine:7;
	 uint32_t:16;
	uint32_t TILE_MODE:1;
	uint32_t DATA_PACK_MODE:3;
	uint32_t UHD4K_MODE_ENABLE:1;
	uint32_t flip_en:1;
	uint32_t YUV422_MODE:1;
	uint32_t ADD_MODE:2;

	/*DWORD - 01C */
	uint32_t HD_REG_TH_H:4;
	uint32_t FLIP_SKIP_LINE:5;
	uint32_t:22;
	uint32_t UFO_DRAM_NEW_MODE:1;


	/*DWORD - 020 */
	uint32_t ULTRA_TH:5;
	uint32_t ULTRA_EN:1;
	uint32_t PRE_ULTRA_TH:5;
	uint32_t PRE_ULTRA_EN:1;
	uint32_t:20;

	/*DWORD - 024 */
	 uint32_t:32;

	/*DWORD - 028 */
	 uint32_t:32;

	/*DWORD - 02C */
	 uint32_t:32;

	/*DWORD - 030 */
	 uint32_t:32;

	/*DWORD - 034 */
	 uint32_t:32;

	/*DWORD - 038 */
	 uint32_t:32;

	/*DWORD - 03C */
	 uint32_t:32;

	/*DWORD - 040 */
	uint32_t UFOD_ENABLE:1;
	uint32_t SIMULATION_MODE:1;
	uint32_t SHADOW_ENABLE:1;
	 uint32_t:31;

	/*DWORD - 044 */
	 uint32_t:32;

	/*DWORD - 048 */
	 uint32_t:32;

	/*DWORD - 04C */
	 uint32_t:32;

	/*DWORD - 050 */
	 uint32_t:32;

	/*DWORD - 054 */
	 uint32_t:32;

	/*DWORD - 058 */
	 uint32_t:32;

	/*DWORD - 05C */
	 uint32_t:32;

	/*DWORD - 060 */
	 uint32_t:32;

	/*DWORD - 064 */
	 uint32_t:32;

	/*DWORD - 068 */
	 uint32_t:32;

	/*DWORD - 06C */
	 uint32_t:32;

	/*DWORD - 070 */
	 uint32_t:32;

	/*DWORD - 074 */
	 uint32_t:32;

	/*DWORD - 078 */
	 uint32_t:32;

	/*DWORD - 07C */
	 uint32_t:32;

	/*DWORD - 080 */
	 uint32_t:32;

	/*DWORD - 084 */
	 uint32_t:32;

	/*DWORD - 088 */
	 uint32_t:32;

	/*DWORD - 08C */
	 uint32_t:32;

	/*DWORD - 090 */
	 uint32_t:32;

	/*DWORD - 094 */
	 uint32_t:32;

	/*DWORD - 098 */
	 uint32_t:32;

	/*DWORD - 09C */
	 uint32_t:32;

	/*DWORD - 0A0 */
	 uint32_t:32;

	/*DWORD - 0A4 */
	 uint32_t:32;

	/*DWORD - 0A8 */
	 uint32_t:32;

	/*DWORD - 0AC */
	 uint32_t:32;

	/*DWORD - 0B0 */
	 uint32_t:32;

	/*DWORD - 0B4 */
	 uint32_t:32;

	/*DWORD - 0B8 */
	 uint32_t:32;

	/*DWORD - 0BC */
	 uint32_t:32;

	/*DWORD - 0C0 */
	 uint32_t:32;

	/*DWORD - 0C4 */
	 uint32_t:32;

	/*DWORD - 0C8 */
	 uint32_t:32;

	/*DWORD - 0CC */
	 uint32_t:32;

	/*DWORD - 0D0 */
	 uint32_t:32;

	/*DWORD - 0D4 */
	 uint32_t:32;

	/*DWORD - 0D8 */
	 uint32_t:32;

	/*DWORD - 0DC */
	 uint32_t:32;

	/*DWORD - 0E0 */
	 uint32_t:32;

	/*DWORD - 0E4 */
	 uint32_t:32;

	/*DWORD - 0E8 */
	 uint32_t:32;

	/*DWORD - 0EC */
	 uint32_t:32;

	/*DWORD - 0F0 */
	 uint32_t:32;

	/*DWORD - 0F4 */
	 uint32_t:32;

	/*DWORD - 0F8 */
	 uint32_t:32;

	/*DWORD - 0FC */
	 uint32_t:32;
};

union vdo_hal_ufo_union {
	uint32_t reg[HAL_VDO_UFOD_REG_NUM];
	struct vdo_hal_ufo_field field;
};

struct vdo_sw_shadow {
	union vdo_hal_union vdo_reg;
	union vdo_hal_scl_union vdo_scl_reg;
	union vdo_hal_ufo_union vdo_ufo_reg;
	uint64_t vdo_reg_mode;
	uint64_t vdo_scl_reg_mode;
	uint64_t vdo_ufo_reg_mode;
};

struct vdo_hw_register {
	union vdo_hal_union *vdo_reg;
	union vdo_hal_ufo_union *vdo_ufo_reg;
	union vdo_hal_scl_union *vdo_scl_reg;
};
#endif


/*************************************************************************/
/* VDP HAL API */
/************************************************************************/
/*#define VDP_1					0	*/
/*#define VDP_2					1	*/

#if 0
extern union vdo_hal_union vdp1_sw_reg;
extern union vdo_hal_union vdp2_sw_reg;

extern union vdo_hal_scl_union vdp1_scl_sw_reg;
extern union vdo_hal_scl_union vdp2_scl_sw_reg;

extern union vdo_hal_ufo_union vdo3_ufo_sw_reg;
extern union vdo_hal_ufo_union vdo4_ufo_sw_reg;
#endif

extern uint64_t vdp3_reg_mode;
extern uint64_t vdp4_reg_mode;

extern uint64_t vdp3_scl_reg_mode;
extern uint64_t vdp4_scl_reg_mode;

extern uint64_t vdo3_ufo_reg_mode;
extern uint64_t vdo4_ufo_reg_mode;
extern char *disp_vdo_reg_base[VDO_LAYER_COUNT];

#define REG_MASK(idx)           ((uint64_t)1LL << (idx))

#define IS_REG_SET(r, mask)     ((r) & (mask))
#define REG_SET(r, mask)        ((r) |= (mask))
#define REG_RESET(r, mask)      ((r) &= ~(mask))

#define GET_VDP_PTR(id, reg, mode) do {	\
	if (id == VDP_1) {				\
		reg = &vdo_sw_reg[0].vdo_reg;			\
		mode = &vdo_sw_reg[0].vdo_reg_mode;			\
	} else if (id == VDP_2) {			\
		reg = &vdo_sw_reg[1].vdo_reg;			\
		mode = &vdo_sw_reg[1].vdo_reg_mode;			\
	}						\
} while (0)

#define GET_UFO_PTR(id, reg, mode)			\
do {							\
	if (id == VDP_1) {				\
		reg = &vdo_sw_reg[0].vdo_ufo_reg;			\
		mode = &vdo_sw_reg[0].vdo_ufo_reg_mode;		\
	} else if (id == VDP_2) {			\
		reg = &vdo_sw_reg[1].vdo_ufo_reg;			\
		mode = &vdo_sw_reg[1].vdo_ufo_reg_mode;		\
	}						\
} while (0)

#define GET_VDP_SCL_PTR(id, reg, mode)			\
do {							\
	if (id == VDP_1) {				\
		reg = &vdo_sw_reg[0].vdo_scl_reg;			\
		mode = &vdo_sw_reg[0].vdo_scl_reg_mode;		\
	} else if (id == VDP_2) {			\
		reg = &vdo_sw_reg[1].vdo_scl_reg;			\
		mode = &vdo_sw_reg[1].vdo_scl_reg_mode;		\
	}						\
} while (0)

#endif
