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

#ifndef _PP_QTY_TBL_H_
#define _PP_QTY_TBL_H_

/**********************************SD CONFIG***************************************/
/**************TDSharp Control**************/

/* 0x5B000       TDPROC_00       Q */
#define TDS_H1_GAIN					0x03	/* (0xff << 24) */
#define TDS_H1_LIMIT_POS			0xff	/* (0xff << 16) */
#define TDS_H1_LIMIT_NEG			0xff	/* (0xff << 8) */
#define TDS_H1_CORING				0x50	/* (0xff << 0) */

/* 0x5B004       TDPROC_01       Q */
#define TDS_H1_CLIP_THPOS			0xff	/* (0xff << 24) */
#define TDS_H1_CLIP_THNEG			0xff	/* (0xff << 16) */
#define TDS_H1_CLIP_LC_SEL			0x01	/* (0x01 << 12) */
#define TDS_H1_ATT_SEL				0x00	/* (0x07 << 8) */
#define TDS_H1_CLIP_EN				0x01	/* (0x01 << 7) */
#define TDS_H1_LPF_SEL				0x02	/*(0x03 << 4) */
#define TDS_H1_CLIP_BAND_SEL		0x0f	/* (0x0f << 0) */

/* 0x5B008       TDPROC_02       Q */
#define TDS_H1_SOFT_COR_GAIN		0x02	/* (0x0f << 12) */
#define TDS_H1_PREC					0x02	/* (0x03 << 8) */
#define TDS_H1_SOFT_CLIP_GAIN		0x00	/* (0xff << 0) */

/* 0x5B00C       TDPROC_03       Q */
#define TDS_H1_GAIN_NEG				0x00	/* (0xff << 24) */

/* 0x5B010       TDPROC_04       Q */
#define TDS_H2_GAIN					0x03	/* (0xff << 24) */
#define TDS_H2_LIMIT_POS			0xff	/* (0xff << 16) */
#define TDS_H2_LIMIT_NEG			0xff	/* (0xff << 8) */
#define TDS_H2_CORING				0x62	/* (0xff << 0) */

/* 0x5B014       TDPROC_05       Q */
#define TDS_H2_CLIP_THPOS			0x04	/* (0xff << 24) */
#define TDS_H2_CLIP_THNEG			0x00	/* (0xff << 16) */
#define TDS_H2_CLIP_LC_SEL			0x00	/* (0x01 << 12) */
#define TDS_H2_ATT_SEL				0x00	/* (0x07 << 8) */
#define TDS_H2_CLIP_EN				0x00	/* (0x01 << 7) */
#define TDS_H2_LPF_SEL				0x02	/* 0x06  //(0x03 << 4) */
#define TDS_H2_CLIP_BAND_SEL		0x04	/* (0x0f << 0) */

/* 0x5B018       TDPROC_06       Q */
#define TDS_H2_SOFT_COR_GAIN		0x08	/* (0x0f << 12) */
#define TDS_H2_PREC					0x02	/* (0x03 << 8) */
#define TDS_H2_SOFT_CLIP_GAIN		0xff	/* (0xff << 0) */

/* 0x5B01C       TDPROC_07       Q */
#define TDS_H2_GAIN_NEG				0x00	/* (0xff << 24) */

/* 0x5B020       TDPROC_08       Q */
#define TDS_V1_GAIN					0xff	/* (0xff << 24) */
#define TDS_V1_LIMIT_POS			0xff	/* (0xff << 16) */
#define TDS_V1_LIMIT_NEG			0xff	/* (0xff << 8) */
#define TDS_V1_CORING				0x58	/* (0xff << 0) */

/* 0x5B024       TDPROC_09       Q */
#define TDS_V1_CLIP_THPOS			0x48	/* (0xff << 24) */
#define TDS_V1_CLIP_THNEG			0x48	/* (0xff << 16) */
#define TDS_V1_CLIP_LC_SEL			0x01	/* (0x01 << 12) */
#define TDS_V1_ATT_SEL				0x00	/* (0x07 << 8) */
#define TDS_V1_CLIP_EN				0x01	/* (0x01 << 7) */
#define TDS_V1_CLIP_BAND_SEL		0x04	/* (0x0f << 0) */

/* 0x5B028       TDPROC_0A       Q */
#define TDS_V1_SOFT_COR_GAIN		0x08	/* (0x0f << 12) */
#define TDS_V1_PREC					0x00	/* (0x03 << 8) */
#define TDS_V1_SOFT_CLIP_GAIN		0x04	/* (0xff << 0) */

/* 0x5B02C       TDPROC_0B       Q */
#define TDS_V1_GAIN_NEG				0xff	/* (0xff << 24) */

/* 0x5B030       TDPROC_0C       Q */
#define TDS_V2_GAIN					0xff	/* (0xff << 24) */
#define TDS_V2_LIMIT_POS			0x06	/* (0xff << 16) */
#define TDS_V2_LIMIT_NEG			0x06	/* (0xff << 8) */
#define TDS_V2_CORING				0x58	/* (0xff << 0) */

/* 0x5B034       TDPROC_0D       Q */
#define TDS_V2_CLIP_THPOS			0xff	/* (0xff << 24) */
#define TDS_V2_CLIP_THNEG			0xff	/* (0xff << 16) */
#define TDS_V2_CLIP_LC_SEL			0x01	/* (0x01 << 12) */
#define TDS_V2_ATT_SEL				0x04	/* (0x07 << 8) */
#define TDS_V2_CLIP_EN				0x01	/* (0x01 << 7) */
#define TDS_V2_CLIP_BAND_SEL		0x0f	/* modify 0xff   //(0x0f << 0) */

/* 0x5B038       TDPROC_0E       Q */
#define TDS_V2_SOFT_COR_GAIN		0x03	/* (0x0f << 12) */
#define TDS_V2_PREC					0x02	/*(0x03 << 8) */
#define TDS_V2_SOFT_CLIP_GAIN		0xff	/* (0xff << 0) */

/* 0x5B03C       TDPROC_0F       Q */
#define TDS_V2_GAIN_NEG				0xff	/* (0xff << 24) */

/* 0x5B040       TDPROC_10       Q */
#define TDS_X1_GAIN					0xff	/* (0xff << 24) */
#define TDS_X1_LIMIT_POS			0x06	/* (0xff << 16) */
#define TDS_X1_LIMIT_NEG			0x30	/* (0xff << 8) */
#define TDS_X1_CORING				0x00	/* (0xff << 0) */

/* 0x5B044       TDPROC_11       Q */
#define TDS_X1_CLIP_THPOS			0x00	/* (0xff << 24) */
#define TDS_X1_CLIP_THNEG			0x01	/* (0xff << 16) */
#define TDS_X1_CLIP_LC_SEL			0x01	/* (0x01 << 12) */
#define TDS_X1_ATT_SEL				0x00	/* (0x07 << 8) */
#define TDS_X1_CLIP_EN				0x01	/* (0x01 << 7) */
#define TDS_X1_CLIP_BAND_SEL		0x03	/* (0x0f << 0) */

/* 0x5B048       TDPROC_12       Q */
#define TDS_X1_SOFT_COR_GAIN		0x03	/* (0x0f << 12) */
#define TDS_X1_PREC					0x02	/* */
#define TDS_X1_SOFT_CLIP_GAIN		0x00	/* (0xff << 0) */

/* 0x5B04C       TDPROC_13       Q */
#define TDS_X1_GAIN_NEG				0xff	/* (0xff << 24) */

/* 0x5B050       TDPROC_14       Q */
#define TDS_X2_GAIN					0xff	/* (0xff << 24) */
#define TDS_X2_LIMIT_POS			0x40	/* (0xff << 16) */
#define TDS_X2_LIMIT_NEG			0x06	/* (0xff << 8) */
#define TDS_X2_CORING				0x00	/* (0xff << 0) */

/* 0x5B054       TDPROC_15       Q */
#define TDS_X2_CLIP_THPOS			0x40	/* (0xff << 24) */
#define TDS_X2_CLIP_THNEG			0x40	/* (0xff << 16) */
#define TDS_X2_CLIP_LC_SEL			0x01	/* (0x01 << 12) */
#define TDS_X2_ATT_SEL				0x00	/* (0x07 << 8) */
#define TDS_X2_CLIP_EN				0x01	/* (0x01 << 7) */
#define TDS_X2_CLIP_BAND_SEL		0x0a	/* modify 0x7a   //(0x0f << 0) */

/* 0x5B058       TDPROC_16       Q */
#define TDS_X2_SOFT_COR_GAIN		0x03	/* (0x0f << 12) */
#define TDS_X2_PREC					0x00	/* (0x03 << 8) */
#define TDS_X2_SOFT_CLIP_GAIN		0x7a	/* (0xff << 0) */

/* 0x5B05C       TDPROC_17       Q */
#define TDS_X2_GAIN_NEG				0xff	/* (0xff << 24) */

/* 0x5B080       TDPROC_20       TDSHARP_BAND H1_1       Q */
#define TDS_H1_1_GAIN				0xff	/* (0xff << 24) */
#define TDS_H1_1_LIMIT_POS			0x06	/* (0xff << 16) */
#define TDS_H1_1_LIMIT_NEG			0x06	/* (0xff << 8) */
#define TDS_H1_1_CORING				0x00	/* (0xff << 0) */

/* 0x5B084       TDPROC_21       TDSHARP_BAND H1_1       Q */
#define TDS_H1_1_CLIP_THPOS			0x00	/* (0xff << 24) */
#define TDS_H1_1_CLIP_THNEG			0x00	/* (0xff << 16) */
#define TDS_H1_1_CLIP_EN			0x00	/* (0x01 << 7) */
#define TDS_H1_1_LPF_SEL			0x00	/* (0x03 << 4) */

/* 0x5B088       TDPROC_22       TDSHARP_BAND H1_1       Q */
#define TDS_H1_1_SOFT_COR_GAIN		0x03	/* (0x0f << 12) */
#define TDS_H1_1_SOFT_CLIP_GAIN		0x00	/* (0xff << 0) */

/* 0x5B08C       TDPROC_23 TDSHARP_BAND H1_1 Q */
#define TDS_H1_1_GAIN_NEG			0xff	/* (0xff << 24) */

/* 0x5B090       TDPROC_24       Q */
#define TDS_EN						0x01	/* (0x01 << 31) */
#define TDS_TDPROC_BYPASS_ALL		0x00	/*(0x01 << 30) */
#define TDS_NEG_GAIN_EN				0x00	/* (0x01 << 29) */
#define TDS_LTI_EN					0x00	/*(0x01 << 28) */
#define TDS_INK_EN					0x00	/*(0x01 << 27) */
#define TDS_CLIP_EN					0x01	/* (0x01 << 26) */
#define TDS_CLIP_SEL				0x01	/* (0x01 << 25) */
#define TDS_CRC2_EN					0x01	/* (0x01 << 24) */
#define TDS_LTI1_INDEPENDENT		0x01	/* (0x01 << 23) */
#define TDS_LIMIT_POS_ALL			0x300	/* (0x3ff << 12) */
#define TDS_LIMIT_NEG_ALL			0x330	/* (0x3ff << 0) */

/* 0x5B098       TDPROC_26       Q */
#define TDS_MAX_MIN_ATT				0x00	/* modify 0x40   //(0x03 << 30) */
#define TDS_MAX_MIN_SEL				0x01	/* (0x01 << 29) */
#define TDS_X1_FLT_SEL				0x01	/* (0x01 << 28) */
#define TDS_H2_FLT_SEL				0x02	/* modify 0x4a   //(0x03 << 26) */
#define TDS_V2_FLT_SEL				0x01	/* (0x01 << 25) */
#define TDS_LTI2_FLT_SEL			0x01	/* (0x01 << 24) */
#define TDS_H1_FLT_SEL				0x01	/* (0x01 << 23) */
#define TDS_V1_FLT_SEL				0x01	/* (0x01 << 22) */
#define TDS_SFT						0x00	/* (0x03 << 20) */
#define TDS_AC_LPF_EN				0x00	/* (0x01 << 18) */
#define TDS_MAX_MIN_LMT				0x40	/* (0xff << 0) */

/* 0x5B0D0       TDPROC_34       TDSHARP_BAND H2_1       Q */
#define TDS_H2_1_GAIN				0xff	/* (0xff << 24) */
#define TDS_H2_1_LIMIT_POS			0x00	/* (0xff << 16) */
#define TDS_H2_1_LIMIT_NEG			0x00	/* (0xff << 8) */
#define TDS_H2_1_CORING				0x00	/* (0xff << 0) */

/* 0x5B0D4       TDPROC_35       TDSHARP_BAND H2_1       Q */
#define TDS_H2_1_CLIP_THPOS			0x40	/* (0xff << 24) */
#define TDS_H2_1_CLIP_THNEG			0x80	/* (0xff << 16) */
#define TDS_H2_1_FLT_SEL			0x03	/* (0x03 << 8) */
#define TDS_H2_1_CLIP_EN			0x01	/* (0x01 << 7) */
#define TDS_H2_1_LPF_SEL			0x02	/* (0x03 << 4) */

/* 0x5B0D8       TDPROC_36       TDSHARP_BAND H2_1       Q */
#define TDS_H2_1_SOFT_COR_GAIN		0x03	/* (0x0f << 12) */
#define TDS_H2_1_SOFT_CLIP_GAIN		0x40	/* (0xff << 0) */

/* 0x5B0DC       TDPROC_37 TDSHARP_BAND H2_1 */
#define TDS_H2_1_GAIN_NEG			0xff	/* (0xff << 24) */

/**************TDSharp End**************/

/**************HVBand Control**************/

/* 0x5B1AC       TDPROC_6B       Q */
#define TDS_HVBAND_COR				0x08	/* (0xff << 0) */
#define TDS_HVBAND_EN				0x01	/* (0x01 << 8) */
#define TDS_BGTESTMODE				0x00ff	/* (0xffff << 16) */

/* 0x5B1B0       TDPROC_6C       Q */
#define TDS_HVBAND0LV				0x60	/* (0xff << 0) */
#define TDS_HVBAND1LV				0x0f	/* (0xff << 8) */
#define TDS_HVBAND2LV				0x30	/* (0xff << 16) */
#define TDS_HVBAND3LV				0x0f	/* (0xff << 24) */

/* 0x5B1B4       TDPROC_6D       Q */
#define TDS_HVBAND4LV				0x01	/* (0xff << 0) */
#define TDS_HVBAND5LV				0x01	/* (0xff << 8) */
#define TDS_HVBAND6LV				0x00	/* (0xff << 16) */
#define TDS_HVBAND7LV				0x03	/* (0xff << 24) */

/* 0x5B1B8       TDPROC_6E       Q */
#define TDS_LTIHVBAND0LV			0x01	/* (0xff << 0) */
#define TDS_LTIHVBAND1LV			0x01	/* (0xff << 8) */
#define TDS_LTIHVBAND2LV			0x01	/* (0xff << 16) */
#define TDS_LTIHVBAND3LV			0x01	/* (0xff << 24) */

/* 0x5B1BC       TDPROC_6F       Q */
#define TDS_LTIHVBAND4LV			0x0c	/* (0xff << 0) */
#define TDS_LTIHVBAND5LV			0x0c	/* (0xff << 8) */
#define TDS_LTIHVBAND6LV			0x0a	/* (0xff << 16) */
#define TDS_LTIHVBAND7LV			0x0a	/* (0xff << 24) */

/**************HVBand Control End**************/

/**************TDSharp ADAP**************/

/* 0x5B2C0       TDPROC_B0       Q */
#define TDS_ADAP_SHP_EN				0x00	/*(0x01 << 30) */
#define TDS_ADAP_SHP_INK_EN			0x00	/* (0x01 << 29) */
#define TDS_ADAP_SHP_EDGE_SEL		0x00	/* (0x07 << 24) */
#define TDS_ADAP_SHP_P3				0x1a	/* (0xff << 16) */
#define TDS_ADAP_SHP_P2				0x7F	/* (0xff << 8) */
#define TDS_ADAP_SHP_P1				0x08	/* (0xff << 0) */

/* 0x5B2C4       TDPROC_B1       Q */
#define TDS_ADAP_SHP_L_BOUND		0x30	/* (0xff << 16) */
#define TDS_ADAP_SHP_EDGE_SCALE		0x30	/* (0x3ff << 0) */

/* 0x5B2C8       TDPROC_B2       Q */
#define TDS_ADAP_SHP_G1				0x201	/* (0x3ff << 16) */
#define TDS_ADAP_SHP_U_BOUND		0x01	/* (0xff << 8) */
#define TDS_ADAP_SHP_OFFSET			0x02	/* (0xff << 0) */

/* 0x5B2CC       TDPROC_B3       Q */
#define TDS_ADAP_SHP_G3				0x204	/* (0x3ff << 16) */
#define TDS_ADAP_SHP_G2				0x320	/* (0x3ff << 0) */

/* 0x5B300       TDPROC_C0       Q */
#define TDS_LC_EN					0x01	/* (0x01 << 31) */
#define TDS_LC_INDEX_SEL			0x01	/* (0x01 << 26) */
#define TDS_LC_MODE					0x00	/* (0x03 << 24) */
#define TDS_LC_IDX_LPF_EN			0x01	/* (0x01 << 19) */
#define TDS_AC_LPF_COE				0x00	/*(0x0f << 0) */

/* 0x5B308       TDPROC_C2       Q */
#define TDS_LC_IDX_LUT1_G1			0x50	/* (0x3ff << 16) */
#define TDS_LC_IDX_LUT1_P1			0x50	/* (0xff << 8) */
#define TDS_CHROMA_THD				0x80	/* (0xff << 0) */

/* 0x5B30C       TDPROC_C3       Q */
#define TDS_POS_CLIP				0x300	/* (0x3ff << 20) */
#define TDS_NEG_CLIP				0x302	/* (0x3ff << 8) */
#define TDS_CLIP_GAIN				0x00	/* (0xff << 0) */

/* 0x5B09C       TDPROC_27       Q */
#define TDS_LC_LOAD_ENA_H			0x01	/* (0x01 << 3) */
#define TDS_LC_LOAD_BURST			0x01	/* (0x01 << 2) */
#define TDS_LC_LOAD_ENA				0x01	/* (0x01 << 1) */
#define TDS_LC_READ_ENA				0x01	/* (0x01 << 0) */

/* 0x5B0A0       TDPROC_28       Q */
#define TDS_LC_LOAD_H				0x04	/* (0xff << 8) */
#define TDS_LC_LOAD_IND				0x60	/* (0x7f << 0) */

/* 0x5B0A4       TDPROC_29       Q */
#define TDS_YLEV_TBL				0xff	/* (0xff << 16) */
#define TDS_LC_TBL_H				0xff	/* (0xff << 0) */

/* 0x5B0B0       TDPROC_2C       Q */
#define TDS_YLEV_ENA				0x00	/* (0x01 << 28) */
#define TDS_YLEV_ALPHA				0x08	/* (0xff << 20) */
#define TDS_YLEV_LOAD				0x40	/* (0xff << 12) */
#define TDS_YLEV_IND				0x7f	/*(0x7f << 4) */
#define C_YLEV_TBL_LD				0x01	/* (0x01 << 3) */
#define TDS_YLEV_LOAD_BURST			0x01	/* (0x01 << 2) */
#define TDS_YLEV_LOAD_ENA			0x01	/* (0x01 << 1) */
#define TDS_YLEV_READ_ENA			0x01	/* (0x01 << 0) */

/**************TDSharp ADAP End**************/

/************** LTI **************/

/* 0x5B0E0       LTI_00  BAND1   Q */
#define TV_LTI_GAIN1				0x00	/* (0xff << 24) */
#define TV_LTI_LIMIT_POS1			0x08	/* (0xff << 16) */
#define TV_LTI_LIMIT_NEG1			0x0c	/* (0xff << 8) */
#define TV_LTI_CORING1				0x08	/* (0xff << 0) */

/* 0x5B0E4       LTI_01  BAND1   Q */
#define TV_LTI_CLIP_THPOS1			0x05	/* (0xff << 24) */
#define TV_LTI_CLIP_THNEG1			0x05	/* (0xff << 16) */
#define TV_LTI_CLIP_LC_SEL1			0x00	/* (0x01 << 12) */
#define TV_LTI_ATTENUATE_SEL1		0x00	/* (0x07 << 8) */
#define TV_LTI_CLIP_EN1				0x00	/* (0x01 << 7) */
#define TV_LTI_LPF_SEL1				0x01	/* (0x03 << 4) */
#define TV_LTI_CLIP_BAND_SEL1		0x0c	/* (0x0f << 0) */

/* 0x5B0E8       LTI_02  BAND1   Q */
#define TV_LTI_SOFT_COR_GAIN1		0x0c	/* (0x0f << 12) */
#define TV_LTI_PREC1				0x00	/* (0x03 << 8) */
#define TV_LTI_SOFT_CLIP_GAIN1		0x0a	/* (0xff << 0) */

/* 0x5B0EC       LTI_03  BAND1   Q */
#define TV_LTI_GAIN_NEG1			0x00	/* (0xff << 24) */

/* 0x5B0F0       LTI_04  BAND2   Q */
#define TV_LTI_GAIN2				0x0a	/* (0xff << 24) */
#define TV_LTI_LIMIT_POS2			0x1a	/* (0xff << 16) */
#define TV_LTI_LIMIT_NEG2			0x1a	/* (0xff << 8) */
#define TV_LTI_CORING2				0x03	/* (0xff << 0) */

/* 0x5B0F4       LTI_05  BAND2   Q */
#define TV_LTI_CLIP_THPOS2			0x00	/* (0xff << 24) */
#define TV_LTI_CLIP_THNEG2			0x01	/* (0xff << 16) */
#define TV_LTI_CLIP_LC_SEL2			0x00	/* (0x01 << 12) */
#define TV_LTI_ATTENUATE_SEL2		0x00	/* (0x07 << 8) */
#define TV_LTI_CLIP_EN2				0x01	/* (0x01 << 7) */
#define TV_LTI_LPF_SEL2				0x00	/* (0x03 << 4) */
#define TV_LTI_CLIP_BAND_SEL2		0x00	/* (0xf << 0) */

/* 0x5B0F8       LTI_06  BAND2   Q */
#define TV_LTI_SOFT_COR_GAIN2		0x00	/* (0x0f << 12) */
#define TV_LTI_PREC2				0x00	/* (0x03 << 8) */
#define TV_LTI_SOFT_CLIP_GAIN2		0x07	/* (0xff << 0) */

/* 0x5B0FC       LTI_07  BAND2   Q */
#define TV_LTI_GAIN_NEG2			0x0b	/* (0xff << 24) */

/**************LTI End **************/

/**************HLTI****************/

/* 0x5B184       HLTI_01 Q */
#define TV_HLTI_HDEG_GAIN			0x04	/* (0xff << 8) */
#define TV_HLTI_HDIFF_OFFSET		0x20	/* (0xff << 0) */
#define TV_HLTI_VDEG_GAIN			0x02	/* (0xff << 24) */
#define TV_HLTI_VDIFF_OFFSET		0x00	/* (0xff << 16) */

/* 0x5B188       HLTI_00 Q */
#define TV_HLTI_EN					0x01	/* (0x1 << 0) */
#define TV_HLTI_PEAKING				0x00	/* (0x1 << 1) */

/* 0x5B18C       HLTI_02 Q */
#define TV_HLTI_END_X				0x301	/* (0x3ff << 16) */
#define TV_HLTI_START_X				0x004	/* (0x3ff << 0) */

/* 0x5B190       HLTI_03 Q */
#define TV_HLTI_SLOPE_X				0x00	/* (0x7fff << 0) */

/* 0x5B194       HLTI_04 */
#define TV_HLTI_END_HV				0x102	/* (0x3ff << 20) */
#define TV_HLTI_MIDDLE_HV			0x004	/* (0x3ff << 10) */
#define TV_HLTI_START_HV			0x005	/* (0x3ff << 0) */

/* 0x5B198       HLTI_05 */
#define TV_HLTI_SLOPEUP_HV			0x05	/* (0x7fff << 16) */
#define TV_HLTI_SLOPEDOWN_HV		0x05	/* (0x7fff << 0) */
/**************HLTIEnd**************/


/**********************************720HD CONFIG***************************************/
/**************TDSharpControl**************/

/* 0x5B000TDPROC_00Q*/
#define P720_TDS_H1_GAIN			0x08	/*( 0xff<<24) */
#define P720_TDS_H1_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P720_TDS_H1_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P720_TDS_H1_CORING			0x60	/*( 0xff<<0) */

/* 0x5B004TDPROC_01Q*/
#define P720_TDS_H1_CLIP_THPOS		0xff	/*( 0xff<<24) */
#define P720_TDS_H1_CLIP_THNEG		0xff	/*( 0xff<<16) */
#define P720_TDS_H1_CLIP_LC_SEL		0x01	/*( 0x01<<12) */
#define P720_TDS_H1_ATT_SEL			0x00	/*( 0x07<<8) */
#define P720_TDS_H1_CLIP_EN			0x01	/*( 0x01<<7) */
#define P720_TDS_H1_LPF_SEL			0x02	/* 0x06//( 0x03<<4) */
#define P720_TDS_H1_CLIP_BAND_SEL	0x0f	/*( 0x0f<<0) */

/* 0x5B008TDPROC_02Q*/
#define P720_TDS_H1_SOFT_COR_GAIN	0x09	/*( 0x0f<<12) */
#define P720_TDS_H1_PREC			0x02	/*( 0x03<<8) */
#define P720_TDS_H1_SOFT_CLIP_GAIN	0x00	/*( 0xff<<0) */

/* 0x5B00CTDPROC_03Q*/
#define P720_TDS_H1_GAIN_NEG		0x00	/*( 0xff<<24) */

/* 0x5B010TDPROC_04Q*/
#define P720_TDS_H2_GAIN			0x0f	/*( 0xff<<24) */
#define P720_TDS_H2_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P720_TDS_H2_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P720_TDS_H2_CORING			0x49	/*( 0xff<<0) */

#define P720_TDS_H2_CLIP_THPOS		0x0a	/*( 0xff<<24) */
#define P720_TDS_H2_CLIP_THNEG		0x00	/*( 0xff<<16) */
#define P720_TDS_H2_CLIP_LC_SEL		0x00	/*( 0x01<<12) */
#define P720_TDS_H2_ATT_SEL			0x00	/*( 0x07<<8) */
#define P720_TDS_H2_CLIP_EN			0x00	/*( 0x01<<7) */
#define P720_TDS_H2_LPF_SEL			0x02	/*( 0x03<<4) */
#define P720_TDS_H2_CLIP_BAND_SEL	0x0a	/*( 0x0f<<0) */

/* 0x5B018TDPROC_06Q*/
#define P720_TDS_H2_SOFT_COR_GAIN	0x00	/*( 0x0f<<12) */
#define P720_TDS_H2_PREC			0x02	/*( 0x03<<8) */
#define P720_TDS_H2_SOFT_CLIP_GAIN	0xff	/*( 0xff<<0) */

/* 0x5B01CTDPROC_07Q*/
#define P720_TDS_H2_GAIN_NEG		0x00	/*( 0xff<<24) */

/* 0x5B020TDPROC_08Q*/
#define P720_TDS_V1_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_V1_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P720_TDS_V1_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P720_TDS_V1_CORING			0x60	/*( 0xff<<0) */

/* 0x5B024TDPROC_09Q*/
#define P720_TDS_V1_CLIP_THPOS		0x4d	/*( 0xff<<24) */
#define P720_TDS_V1_CLIP_THNEG		0x4d	/*( 0xff<<16) */
#define P720_TDS_V1_CLIP_LC_SEL		0x01	/*( 0x01<<12) */
#define P720_TDS_V1_ATT_SEL			0x00	/*( 0x07<<8) */
#define P720_TDS_V1_CLIP_EN			0x01	/*( 0x01<<7) */
#define P720_TDS_V1_CLIP_BAND_SEL	0x00	/*( 0x0f<<0) */

/* 0x5B028TDPROC_0AQ*/
#define P720_TDS_V1_SOFT_COR_GAIN	0x09	/*( 0x0f<<12) */
#define P720_TDS_V1_PREC			0x00	/*( 0x03<<8) */
#define P720_TDS_V1_SOFT_CLIP_GAIN	0x00	/*( 0xff<<0) */

/* 0x5B02CTDPROC_0BQ*/
#define P720_TDS_V1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B030TDPROC_0CQ*/
#define P720_TDS_V2_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_V2_LIMIT_POS		0x06	/*( 0xff<<16) */
#define P720_TDS_V2_LIMIT_NEG		0x06	/*( 0xff<<8) */
#define P720_TDS_V2_CORING			0x49	/*( 0xff<<0) */

/* 0x5B034TDPROC_0DQ*/
#define P720_TDS_V2_CLIP_THPOS		0xff	/*( 0xff<<24) */
#define P720_TDS_V2_CLIP_THNEG		0xff	/*( 0xff<<16) */
#define P720_TDS_V2_CLIP_LC_SEL		0x01	/*( 0x01<<12) */
#define P720_TDS_V2_ATT_SEL			0x04	/*( 0x07<<8) */
#define P720_TDS_V2_CLIP_EN			0x00	/*( 0x01<<7) */
#define P720_TDS_V2_CLIP_BAND_SEL	0x0f	/*( 0x0f<<0) */

/* 0x5B038TDPROC_0EQ*/
#define P720_TDS_V2_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P720_TDS_V2_PREC			0x02	/*( 0x03<<8) */
#define P720_TDS_V2_SOFT_CLIP_GAIN	0xff	/*( 0xff<<0) */

/* 0x5B03CTDPROC_0FQ*/
#define P720_TDS_V2_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B040TDPROC_10Q*/
#define P720_TDS_X1_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_X1_LIMIT_POS		0x06	/*( 0xff<<16) */
#define P720_TDS_X1_LIMIT_NEG		0x40	/*( 0xff<<8) */
#define P720_TDS_X1_CORING			0x00	/*( 0xff<<0) */

/* 0x5B044TDPROC_11Q*/

#define P720_TDS_X1_CLIP_THPOS		0x00	/*( 0xff<<24) */
#define P720_TDS_X1_CLIP_THNEG		0x01	/*( 0xff<<16) */
#define P720_TDS_X1_CLIP_LC_SEL		0x01	/*( 0x01<<12) */
#define P720_TDS_X1_ATT_SEL			0x00	/*( 0x07<<8) */
#define P720_TDS_X1_CLIP_EN			0x01	/*( 0x01<<7) */
#define P720_TDS_X1_CLIP_BAND_SEL	0x04	/*( 0x0f<<0) */

/* 0x5B048TDPROC_12Q*/
#define P720_TDS_X1_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P720_TDS_X1_PREC			0x02	/*( 0x03<<8) */
#define P720_TDS_X1_SOFT_CLIP_GAIN	0x00	/*( 0xff<<0) */

/* 0x5B04CTDPROC_13Q*/
#define P720_TDS_X1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B050TDPROC_14Q*/
#define P720_TDS_X2_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_X2_LIMIT_POS		0x40	/*( 0xff<<16) */
#define P720_TDS_X2_LIMIT_NEG		0x06	/*( 0xff<<8) */
#define P720_TDS_X2_CORING			0x00	/*( 0xff<<0) */

/* 0x5B054TDPROC_15Q*/
#define P720_TDS_X2_CLIP_THPOS		0x40	/*( 0xff<<24) */
#define P720_TDS_X2_CLIP_THNEG		0x40	/*( 0xff<<16) */
#define P720_TDS_X2_CLIP_LC_SEL		0x01	/*( 0x01<<12) */
#define P720_TDS_X2_ATT_SEL			0x00	/*( 0x07<<8) */
#define P720_TDS_X2_CLIP_EN			0x01	/*( 0x01<<7) */
#define P720_TDS_X2_CLIP_BAND_SEL	0x00	/*( 0x0f<<0) */

/* 0x5B058TDPROC_16Q*/
#define P720_TDS_X2_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P720_TDS_X2_PREC			0x00	/*( 0x03<<8) */
#define P720_TDS_X2_SOFT_CLIP_GAIN	0x50	/*( 0xff<<0) */

/* 0x5B05CTDPROC_17Q*/
#define P720_TDS_X2_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B080TDPROC_20TDSHARP_BANDH1_1Q*/
#define P720_TDS_H1_1_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_H1_1_LIMIT_POS		0x06	/*( 0xff<<16) */
#define P720_TDS_H1_1_LIMIT_NEG		0x06	/*( 0xff<<8) */
#define P720_TDS_H1_1_CORING		0x00	/*( 0xff<<0) */

/* 0x5B084TDPROC_21TDSHARP_BANDH1_1Q*/
#define P720_TDS_H1_1_CLIP_THPOS	0x00	/*( 0xff<<24) */
#define P720_TDS_H1_1_CLIP_THNEG	0x00	/*( 0xff<<16) */
#define P720_TDS_H1_1_CLIP_EN		0x00	/*( 0x01<<7) */
#define P720_TDS_H1_1_LPF_SEL		0x00	/*( 0x03<<4) */

/* 0x5B088TDPROC_22TDSHARP_BANDH1_1Q*/
#define P720_TDS_H1_1_SOFT_COR_GAIN 0x08	/*( 0x0f<<12) */
#define P720_TDS_H1_1_SOFT_CLIP_GAIN 0x00	/*( 0xff<<0) */

/* 0x5B08CTDPROC_23TDSHARP_BANDH1_1Q*/
#define P720_TDS_H1_1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B090TDPROC_24Q*/

#define P720_TDS_EN					0x01	/*( 0x01<<31) */
#define P720_TDS_TDPROC_BYPASS_ALL	0x01	/*( 0x01<<30) */
#define P720_TDS_NEG_GAIN_EN		0x01	/*( 0x01<<29) */
#define P720_TDS_LTI_EN				0x00	/*( 0x01<<28) */
#define P720_TDS_INK_EN				0x00	/*( 0x01<<27) */
#define P720_TDS_CLIP_EN			0x00	/*( 0x01<<26) */
#define P720_TDS_CLIP_SEL			0x01	/*( 0x01<<25) */
#define P720_TDS_CRC2_EN			0x01	/*( 0x01<<24) */
#define P720_TDS_LTI1_INDEPENDENT	0x01	/*( 0x01<<23) */
#define P720_TDS_LIMIT_POS_ALL		0x300	/*( 0x3ff<<12) */
#define P720_TDS_LIMIT_NEG_ALL		0x340	/*( 0x3ff<<0) */

/* 0x5B098TDPROC_26Q*/
#define P720_TDS_MAX_MIN_ATT		0x00	/*( 0x03<<30) */
#define P720_TDS_MAX_MIN_SEL		0x01	/*( 0x01<<29) */
#define P720_TDS_X1_FLT_SEL			0x01	/*( 0x01<<28) */
#define P720_TDS_H2_FLT_SEL			0x00	/*( 0x03<<26) */
#define P720_TDS_V2_FLT_SEL			0x01	/*( 0x01<<25) */
#define P720_TDS_LTI2_FLT_SEL		0x01	/*( 0x01<<24) */
#define P720_TDS_H1_FLT_SEL			0x01	/*( 0x01<<23) */
#define P720_TDS_V1_FLT_SEL			0x01	/*( 0x01<<22) */
#define P720_TDS_SFT				0x00	/*( 0x03<<20) */
#define P720_TDS_AC_LPF_EN			0x01	/*( 0x01<<18) */
#define P720_TDS_MAX_MIN_LMT		0x40	/*( 0xff<<0) */

/* 0x5B0D0TDPROC_34TDSHARP_BANDH2_1Q*/
#define P720_TDS_H2_1_GAIN			0xff	/*( 0xff<<24) */
#define P720_TDS_H2_1_LIMIT_POS		0x00	/*( 0xff<<16) */
#define P720_TDS_H2_1_LIMIT_NEG		0x00	/*( 0xff<<8) */
#define P720_TDS_H2_1_CORING		0x00	/*( 0xff<<0) */

/* 0x5B0D4TDPROC_35TDSHARP_BANDH2_1Q*/
#define P720_TDS_H2_1_CLIP_THPOS	0x40	/*( 0xff<<24) */
#define P720_TDS_H2_1_CLIP_THNEG	0x00	/*( 0xff<<16) */
#define P720_TDS_H2_1_FLT_SEL		0x03	/*( 0x03<<8) */
#define P720_TDS_H2_1_CLIP_EN		0x01	/*( 0x01<<7) */
#define P720_TDS_H2_1_LPF_SEL		0x02	/*( 0x03<<4) */

/* 0x5B0D8TDPROC_36TDSHARP_BANDH2_1Q*/
#define P720_TDS_H2_1_SOFT_COR_GAIN 0x08	/*( 0x0f<<12) */
#define P720_TDS_H2_1_SOFT_CLIP_GAIN 0x40	/*( 0xff<<0) */

/* 0x5B0DCTDPROC_37TDSHARP_BANDH2_1*/
#define P720_TDS_H2_1_GAIN_NEG		0xff	/*( 0xff<<24) */

/**************TDSharpEnd**************/

/**************HVBandControl**************/

/* 0x5B1ACTDPROC_6BQ*/
#define P720_TDS_HVBAND_COR			0x08	/*( 0xff<<0) */
#define P720_TDS_HVBAND_EN			0x00	/*( 0x01<<8) */
#define P720_TDS_BGTESTMODE			0x00ff	/*( 0xffff<<16) */

/* 0x5B1B0TDPROC_6CQ*/
#define P720_TDS_HVBAND0LV			0x60	/*( 0xff<<0) */
#define P720_TDS_HVBAND1LV			0x08	/*( 0xff<<8) */
#define P720_TDS_HVBAND2LV			0x30	/*( 0xff<<16) */
#define P720_TDS_HVBAND3LV			0x08	/*( 0xff<<24) */

/* 0x5B1B4TDPROC_6DQ*/
#define P720_TDS_HVBAND4LV			0x01	/*( 0xff<<0) */
#define P720_TDS_HVBAND5LV			0x01	/*( 0xff<<8) */
#define P720_TDS_HVBAND6LV			0x00	/*( 0xff<<16) */
#define P720_TDS_HVBAND7LV			0x00	/*( 0xff<<24) */

/* 0x5B1B8TDPROC_6EQ*/
#define P720_TDS_LTIHVBAND0LV		0x00	/*( 0xff<<0) */
#define P720_TDS_LTIHVBAND1LV		0x00	/*( 0xff<<8) */
#define P720_TDS_LTIHVBAND2LV		0x00	/*( 0xff<<16) */
#define P720_TDS_LTIHVBAND3LV		0x00	/*( 0xff<<24) */

/* 0x5B1BCTDPROC_6FQ*/
#define P720_TDS_LTIHVBAND4LV		0x00	/*( 0xff<<0) */
#define P720_TDS_LTIHVBAND5LV		0x00	/*( 0xff<<8) */
#define P720_TDS_LTIHVBAND6LV		0x00	/*( 0xff<<16) */
#define P720_TDS_LTIHVBAND7LV		0x00	/*( 0xff<<24) */

/**************HVBandControlEnd**************/

/**************TDSharpADAP**************/

/* 0x5B2C0TDPROC_B0Q*/
#define P720_TDS_ADAP_SHP_EN		0x00	/*( 0x01<<30) */
#define P720_TDS_ADAP_SHP_INK_EN	0x00	/*( 0x01<<29) */
#define P720_TDS_ADAP_SHP_EDGE_SEL	0x00	/*( 0x07<<24) */
#define P720_TDS_ADAP_SHP_P3		0x00	/*( 0xff<<16) */
#define P720_TDS_ADAP_SHP_P2		0x00	/*( 0xff<<8) */
#define P720_TDS_ADAP_SHP_P1		0x00	/*( 0xff<<0) */

/* 0x5B2C4TDPROC_B1Q*/
#define P720_TDS_ADAP_SHP_L_BOUND	0x00	/* 0x30//( 0xff<<16) */
#define P720_TDS_ADAP_SHP_EDGE_SCALE 0x20	/*( 0x3ff<<0) */

/* 0x5B2C8TDPROC_B2Q*/
#define P720_TDS_ADAP_SHP_G1		0x201	/*( 0x3ff<<16) */
#define P720_TDS_ADAP_SHP_U_BOUND	0xff	/* 0x01//( 0xff<<8) */
#define P720_TDS_ADAP_SHP_OFFSET	0x02	/*( 0xff<<0) */

/* 0x5B2CCTDPROC_B3Q*/
#define P720_TDS_ADAP_SHP_G3		0x204	/*( 0x3ff<<16) */
#define P720_TDS_ADAP_SHP_G2		0x320	/*( 0x3ff<<0) */

/* 0x5B300TDPROC_C0Q*/
#define P720_TDS_LC_EN				0x01	/*( 0x01<<31) */
#define P720_TDS_LC_INDEX_SEL		0x01	/*( 0x01<<26) */
#define P720_TDS_LC_MODE			0x00	/*( 0x03<<24) */
#define P720_TDS_LC_IDX_LPF_EN		0x01	/*( 0x01<<19) */
#define P720_TDS_AC_LPF_COE			0x00	/*( 0x0f<<0) */

/* 0x5B308TDPROC_C2Q*/
#define P720_TDS_LC_IDX_LUT1_G1		0x50	/*( 0x3ff<<16) */
#define P720_TDS_LC_IDX_LUT1_P1		0x50	/*( 0xff<<8) */
#define P720_TDS_CHROMA_THD			0x80	/*( 0xff<<0) */

/* 0x5B30CTDPROC_C3Q*/
#define P720_TDS_POS_CLIP			0x300	/*( 0x3ff<<20) */
#define P720_TDS_NEG_CLIP			0x300	/*( 0x3ff<<8) */
#define P720_TDS_CLIP_GAIN			0x00	/*( 0xff<<0) */

/* 0x5B09CTDPROC_27Q*/
#define P720_TDS_LC_LOAD_ENA_H		0x00	/*( 0x01<<3) */
#define P720_TDS_LC_LOAD_BURST		0x00	/*( 0x01<<2) */
#define P720_TDS_LC_LOAD_ENA		0x00	/*( 0x01<<1) */
#define P720_TDS_LC_READ_ENA		0x01	/*( 0x01<<0) */

/* 0x5B0A0TDPROC_28Q*/
#define P720_TDS_LC_LOAD_H			0x00	/*( 0xff<<8) */
#define P720_TDS_LC_LOAD_IND		0x00	/*( 0x7f<<0) */

/* 0x5B0A4TDPROC_29Q*/
#define P720_TDS_YLEV_TBL			0xff	/*( 0xff<<16) */
#define P720_TDS_LC_TBL_H			0xff	/*( 0xff<<0) */

/* 0x5B0B0TDPROC_2CQ*/
#define P720_TDS_YLEV_ENA			0x00	/*( 0x01<<28) */
#define P720_TDS_YLEV_ALPHA			0x08	/*( 0xff<<20) */
#define P720_TDS_YLEV_LOAD			0x40	/*( 0xff<<12) */
#define P720_TDS_YLEV_IND			0x7f	/*( 0x7f<<4) */
#define P720_C_YLEV_TBL_LD			0x01	/*( 0x01<<3) */
#define P720_TDS_YLEV_LOAD_BURST	0x01	/*( 0x01<<2) */
#define P720_TDS_YLEV_LOAD_ENA		0x01	/*( 0x01<<1) */
#define P720_TDS_YLEV_READ_ENA		0x01	/*( 0x01<<0) */

/**************TDSharpADAPEnd**************/

/**************HLTI**************/

/* 0x5B0E0LTI_00BAND1Q*/
#define P720_TV_LTI_GAIN1			0x00	/*( 0xff<<24) */
#define P720_TV_LTI_LIMIT_POS1		0x00	/*( 0xff<<16) */
#define P720_TV_LTI_LIMIT_NEG1		0x30	/*( 0xff<<8) */
#define P720_TV_LTI_CORING1			0x00	/*( 0xff<<0) */

/* 0x5B0E4LTI_01BAND1Q*/
#define P720_TV_LTI_CLIP_THPOS1		0x06	/*( 0xff<<24) */
#define P720_TV_LTI_CLIP_THNEG1		0x06	/*( 0xff<<16) */
#define P720_TV_LTI_CLIP_LC_SEL1	0x01	/*( 0x01<<12) */
#define P720_TV_LTI_ATTENUATE_SEL1	0x00	/*( 0x07<<8) */
#define P720_TV_LTI_CLIP_EN1		0x01	/*( 0x01<<7) */
#define P720_TV_LTI_LPF_SEL1		0x01	/*( 0x03<<4) */
#define P720_TV_LTI_CLIP_BAND_SEL1	0x08	/*( 0x0f<<0) */

/* 0x5B0E8LTI_02BAND1Q*/
#define P720_TV_LTI_SOFT_COR_GAIN1	0x08	/*( 0x0f<<12) */
#define P720_TV_LTI_PREC1			0x01	/*( 0x03<<8) */
#define P720_TV_LTI_SOFT_CLIP_GAIN1 0x10	/*( 0xff<<0) */

/* 0x5B0ECLTI_03BAND1Q*/
#define P720_TV_LTI_GAIN_NEG1		0x00	/*( 0xff<<24) */

/* 0x5B0F0LTI_04BAND2Q*/
#define P720_TV_LTI_GAIN2			0x10	/*( 0xff<<24) */
#define P720_TV_LTI_LIMIT_POS2		0x10	/*( 0xff<<16) */
#define P720_TV_LTI_LIMIT_NEG2		0x10	/*( 0xff<<8) */
#define P720_TV_LTI_CORING2			0x03	/*( 0xff<<0) */

/* 0x5B0F4LTI_05BAND2Q*/
#define P720_TV_LTI_CLIP_THPOS2		0x00	/*( 0xff<<24) */
#define P720_TV_LTI_CLIP_THNEG2		0x01	/*( 0xff<<16) */
#define P720_TV_LTI_CLIP_LC_SEL2	0x00	/*( 0x01<<12) */
#define P720_TV_LTI_ATTENUATE_SEL2	0x00	/*( 0x07<<8) */
#define P720_TV_LTI_CLIP_EN2		0x01	/*( 0x01<<7) */
#define P720_TV_LTI_LPF_SEL2		0x00	/*( 0x03<<4) */
#define P720_TV_LTI_CLIP_BAND_SEL2	0x00	/*( 0xf<<0) */

/* 0x5B0F8LTI_06BAND2Q*/
#define P720_TV_LTI_SOFT_COR_GAIN2	0x00	/*( 0x0f<<12) */
#define P720_TV_LTI_PREC2			0x00	/*( 0x03<<8) */
#define P720_TV_LTI_SOFT_CLIP_GAIN2 0x0a	/*( 0xff<<0) */

/* 0x5B0FCLTI_07BAND2Q*/
#define P720_TV_LTI_GAIN_NEG2		0x0b	/*( 0xff<<24) */

/**************LTIEnd**************/

/**************HLTI****************/

/* 0x5B184HLTI_01Q*/
#define P720_TV_HLTI_HDEG_GAIN		0x05	/*( 0xff<<8) */
#define P720_TV_HLTI_HDIFF_OFFSET	0x10	/*( 0xff<<0) */
#define P720_TV_HLTI_VDEG_GAIN		0x05	/*( 0xff<<24) */
#define P720_TV_HLTI_VDIFF_OFFSET	0x00	/*( 0xff<<16) */

/* 0x5B188HLTI_00Q*/
#define P720_TV_HLTI_EN				0x01	/*( 0x1<<0) */
#define P720_TV_HLTI_PEAKING		0x01	/*( 0x1<<1) */

/* 0x5B18CHLTI_02Q*/
#define P720_TV_HLTI_END_X			0x301	/*( 0x3ff<<16) */
#define P720_TV_HLTI_START_X		0x004	/*( 0x3ff<<0) */

/* 0x5B190HLTI_03Q*/
#define P720_TV_HLTI_SLOPE_X		0x00	/*( 0x7fff<<0) */

/* 0x5B194HLTI_04*/
#define P720_TV_HLTI_END_HV			0x102	/*( 0x3ff<<20) */
#define P720_TV_HLTI_MIDDLE_HV		0x004	/*( 0x3ff<<10) */
#define P720_TV_HLTI_START_HV		0x005	/*( 0x3ff<<0) */

/* 0x5B198HLTI_05*/
#define P720_TV_HLTI_SLOPEUP_HV		0x05	/*( 0x7fff<<16) */
#define P720_TV_HLTI_SLOPEDOWN_HV	0x05	/*( 0x7fff<<0) */
/**************HLTIEnd**************/



/**********************************1080HD CONFIG***************************************/
/**************TD Sharp Control**************/

/* 0x5B000 TDPROC_00 Q*/
#define P1080_TDS_H1_GAIN			0x08	/*( 0xff<<24) */
#define P1080_TDS_H1_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P1080_TDS_H1_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P1080_TDS_H1_CORING			0x60	/*( 0xff<<0) */

/* 0x5B004 TDPROC_01 Q*/
#define P1080_TDS_H1_CLIP_THPOS		0xff	/*( 0xff<<24) */
#define P1080_TDS_H1_CLIP_THNEG		0xff	/*( 0xff<<16) */
#define P1080_TDS_H1_CLIP_LC_SEL	0x01	/*( 0x01<<12) */
#define P1080_TDS_H1_ATT_SEL		0x02	/*modify 0x82//( 0x07<<8) */
#define P1080_TDS_H1_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_H1_LPF_SEL		0x02	/* 0x06//( 0x03<<4) */
#define P1080_TDS_H1_CLIP_BAND_SEL	0x0f	/*modify 0xff//( 0x0f<<0) */

/* 0x5B008 TDPROC_02 Q*/
#define P1080_TDS_H1_SOFT_COR_GAIN	0x49	/*( 0x0f<<12) */
#define P1080_TDS_H1_PREC			0x02	/*modify 0x06//( 0x03<<8) */
#define P1080_TDS_H1_SOFT_CLIP_GAIN 0x00	/*( 0xff<<0) */

/* 0x5B00C TDPROC_03 Q*/
#define P1080_TDS_H1_GAIN_NEG		0x00	/*( 0xff<<24) */

/* 0x5B010 TDPROC_04 Q*/
#define P1080_TDS_H2_GAIN			0x0f	/*( 0xff<<24) */
#define P1080_TDS_H2_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P1080_TDS_H2_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P1080_TDS_H2_CORING			0x49	/*( 0xff<<0) */

/* 0x5B014 TDPROC_05 Q*/
#define P1080_TDS_H2_CLIP_THPOS		0x0a	/*( 0xff<<24) */
#define P1080_TDS_H2_CLIP_THNEG		0x00	/*( 0xff<<16) */
#define P1080_TDS_H2_CLIP_LC_SEL	0x00	/*( 0x01<<12) */
#define P1080_TDS_H2_ATT_SEL		0x00	/*( 0x07<<8) */
#define P1080_TDS_H2_CLIP_EN		0x00	/*( 0x01<<7) */
#define P1080_TDS_H2_LPF_SEL		0x02	/* 0x06//( 0x03<<4) */
#define P1080_TDS_H2_CLIP_BAND_SEL	0x0a	/*( 0x0f<<0) */

/* 0x5B018 TDPROC_06 Q*/
#define P1080_TDS_H2_SOFT_COR_GAIN	0x60	/*( 0x0f<<12) */
#define P1080_TDS_H2_PREC			0x02	/*modify 0x06//( 0x03<<8) */
#define P1080_TDS_H2_SOFT_CLIP_GAIN 0xff	/*( 0xff<<0) */

/* 0x5B01C TDPROC_07 Q*/
#define P1080_TDS_H2_GAIN_NEG		0x00	/*( 0xff<<24) */

/* 0x5B020 TDPROC_08 Q*/
#define P1080_TDS_V1_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_V1_LIMIT_POS		0xff	/*( 0xff<<16) */
#define P1080_TDS_V1_LIMIT_NEG		0xff	/*( 0xff<<8) */
#define P1080_TDS_V1_CORING			0x60	/*( 0xff<<0) */

/* 0x5B024 TDPROC_09 Q*/
#define P1080_TDS_V1_CLIP_THPOS		0x4d	/*( 0xff<<24) */
#define P1080_TDS_V1_CLIP_THNEG		0x4d	/*( 0xff<<16) */
#define P1080_TDS_V1_CLIP_LC_SEL	0x01	/*( 0x01<<12) */
#define P1080_TDS_V1_ATT_SEL		0x00	/*( 0x07<<8) */
#define P1080_TDS_V1_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_V1_CLIP_BAND_SEL	0x0a	/*( 0x0f<<0) */

/* 0x5B028 TDPROC_0A Q*/
#define P1080_TDS_V1_SOFT_COR_GAIN	0x09	/*modify 0x49//( 0x0f<<12) */
#define P1080_TDS_V1_PREC			0x00	/*modify 0x40//( 0x03<<8) */
#define P1080_TDS_V1_SOFT_CLIP_GAIN 0x0a	/*( 0xff<<0) */

/* 0x5B02C TDPROC_0B Q*/
#define P1080_TDS_V1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B030 TDPROC_0C Q*/
#define P1080_TDS_V2_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_V2_LIMIT_POS		0x06	/*( 0xff<<16) */
#define P1080_TDS_V2_LIMIT_NEG		0x06	/*( 0xff<<8) */
#define P1080_TDS_V2_CORING			0x49	/*( 0xff<<0) */

/* 0x5B034 TDPROC_0D Q*/
#define P1080_TDS_V2_CLIP_THPOS		0xff	/*( 0xff<<24) */
#define P1080_TDS_V2_CLIP_THNEG		0xff	/*( 0xff<<16) */
#define P1080_TDS_V2_CLIP_LC_SEL	0x01	/*( 0x01<<12) */
#define P1080_TDS_V2_ATT_SEL		0x04	/*modify 0x9C//( 0x07<<8) */
#define P1080_TDS_V2_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_V2_CLIP_BAND_SEL	0x0f	/*modify 0xff//( 0x0f<<0) */

/* 0x5B038 TDPROC_0E Q*/
#define P1080_TDS_V2_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P1080_TDS_V2_PREC			0x02	/*modify 0x06//( 0x03<<8) */
#define P1080_TDS_V2_SOFT_CLIP_GAIN 0xff	/*( 0xff<<0) */

/* 0x5B03C TDPROC_0F Q*/
#define P1080_TDS_V2_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B040 TDPROC_10 Q*/
#define P1080_TDS_X1_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_X1_LIMIT_POS		0x06	/*( 0xff<<16) */
#define P1080_TDS_X1_LIMIT_NEG		0x40	/*( 0xff<<8) */
#define P1080_TDS_X1_CORING			0x00	/*( 0xff<<0) */

/* 0x5B044 TDPROC_11 Q*/
#define P1080_TDS_X1_CLIP_THPOS		0x00	/*( 0xff<<24) */
#define P1080_TDS_X1_CLIP_THNEG		0x01	/*( 0xff<<16) */
#define P1080_TDS_X1_CLIP_LC_SEL	0x01	/*( 0x01<<12) */
#define P1080_TDS_X1_ATT_SEL		0x00	/*( 0x07<<8) */
#define P1080_TDS_X1_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_X1_CLIP_BAND_SEL	0x04	/*( 0x0f<<0) */

/* 0x5B048 TDPROC_12 Q*/
#define P1080_TDS_X1_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P1080_TDS_X1_PREC			0x02	/*modify 0x06//( 0x03<<8) */
#define P1080_TDS_X1_SOFT_CLIP_GAIN 0x00	/*( 0xff<<0) */

/* 0x5B04C TDPROC_13 Q*/
#define P1080_TDS_X1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B050 TDPROC_14 Q*/
#define P1080_TDS_X2_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_X2_LIMIT_POS		0x40	/*( 0xff<<16) */
#define P1080_TDS_X2_LIMIT_NEG		0x06	/*( 0xff<<8) */
#define P1080_TDS_X2_CORING			0x00	/*( 0xff<<0) */

/* 0x5B054 TDPROC_15 Q*/
#define P1080_TDS_X2_CLIP_THPOS		0x40	/*( 0xff<<24) */
#define P1080_TDS_X2_CLIP_THNEG		0x40	/*( 0xff<<16) */
#define P1080_TDS_X2_CLIP_LC_SEL	0x01	/*( 0x01<<12) */
#define P1080_TDS_X2_ATT_SEL		0x00	/*modify 0x80//( 0x07<<8) */
#define P1080_TDS_X2_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_X2_CLIP_BAND_SEL	0x00	/*modify 0x50//( 0x0f<<0) */

/* 0x5B058 TDPROC_16 Q*/
#define P1080_TDS_X2_SOFT_COR_GAIN	0x08	/*( 0x0f<<12) */
#define P1080_TDS_X2_PREC			0x00	/*modify 0x40//( 0x03<<8) */
#define P1080_TDS_X2_SOFT_CLIP_GAIN 0x50	/*( 0xff<<0) */

/* 0x5B05C TDPROC_17 Q*/
#define P1080_TDS_X2_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B080 TDPROC_20 TDSHARP_BANDH1_1 Q*/
#define P1080_TDS_H1_1_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_H1_1_LIMIT_POS	0x06	/*( 0xff<<16) */
#define P1080_TDS_H1_1_LIMIT_NEG	0x06	/*( 0xff<<8) */
#define P1080_TDS_H1_1_CORING		0x00	/*( 0xff<<0) */

/* 0x5B084 TDPROC_21 TDSHARP_BANDH1_1 Q*/
#define P1080_TDS_H1_1_CLIP_THPOS	0x00	/*( 0xff<<24) */
#define P1080_TDS_H1_1_CLIP_THNEG	0x00	/*( 0xff<<16) */
#define P1080_TDS_H1_1_CLIP_EN		0x00	/*( 0x01<<7) */
#define P1080_TDS_H1_1_LPF_SEL		0x00	/*modify 0x40//( 0x03<<4) */

/* 0x5B088 TDPROC_22 TDSHARP_BANDH1_1 Q*/
#define P1080_TDS_H1_1_SOFT_COR_GAIN 0x08	/*( 0x0f<<12) */
#define P1080_TDS_H1_1_SOFT_CLIP_GAIN 0x00	/*( 0xff<<0) */

/* 0x5B08C TDPROC_23 TDSHARP_BANDH1_1 Q*/
#define P1080_TDS_H1_1_GAIN_NEG		0xff	/*( 0xff<<24) */

/* 0x5B090 TDPROC_24 Q*/
#define P1080_TDS_EN				0x01	/*( 0x01<<31) */
#define P1080_TDS_TDPROC_BYPASS_ALL 0x01	/*( 0x01<<30) */
#define P1080_TDS_NEG_GAIN_EN		0x01	/*( 0x01<<29) */
#define P1080_TDS_LTI_EN			0x00	/*( 0x01<<28) */
#define P1080_TDS_INK_EN			0x00	/*modifyat06/30 0x01//( 0x01<<27) */
#define P1080_TDS_CLIP_EN			0x00	/*( 0x01<<26) */
#define P1080_TDS_CLIP_SEL			0x01	/*( 0x01<<25) */
#define P1080_TDS_CRC2_EN			0x01	/*( 0x01<<24) */
#define P1080_TDS_LTI1_INDEPENDENT	0x01	/*( 0x01<<23) */
#define P1080_TDS_LIMIT_POS_ALL		0x300	/*( 0x3ff<<12) */
#define P1080_TDS_LIMIT_NEG_ALL		0x340	/*( 0x3ff<<0) */

/* 0x5B098 TDPROC_26 Q*/
#define P1080_TDS_MAX_MIN_ATT		0x00	/*modify 0x40//( 0x03<<30) */
#define P1080_TDS_MAX_MIN_SEL		0x01	/*( 0x01<<29) */
#define P1080_TDS_X1_FLT_SEL		0x01	/*( 0x01<<28) */
#define P1080_TDS_H2_FLT_SEL		0x00	/*modify 0x40//( 0x03<<26) */
#define P1080_TDS_V2_FLT_SEL		0x01	/*( 0x01<<25) */
#define P1080_TDS_LTI2_FLT_SEL		0x01	/*( 0x01<<24) */
#define P1080_TDS_H1_FLT_SEL		0x01	/*( 0x01<<23) */
#define P1080_TDS_V1_FLT_SEL		0x01	/*( 0x01<<22) */
#define P1080_TDS_SFT				0x02	/*( 0x03<<20) */
#define P1080_TDS_AC_LPF_EN			0x01	/*( 0x01<<18) */
#define P1080_TDS_MAX_MIN_LMT		0x40	/*( 0xff<<0) */

/* 0x5B0D0 TDPROC_34 TDSHARP_BANDH2_1 Q*/
#define P1080_TDS_H2_1_GAIN			0xff	/*( 0xff<<24) */
#define P1080_TDS_H2_1_LIMIT_POS	0x00	/*( 0xff<<16) */
#define P1080_TDS_H2_1_LIMIT_NEG	0x00	/*( 0xff<<8) */
#define P1080_TDS_H2_1_CORING		0x00	/*( 0xff<<0) */

/* 0x5B0D4 TDPROC_35 TDSHARP_BANDH2_1 Q*/
#define P1080_TDS_H2_1_CLIP_THPOS	0x40	/*( 0xff<<24) */
#define P1080_TDS_H2_1_CLIP_THNEG	0x00	/*( 0xff<<16) */
#define P1080_TDS_H2_1_FLT_SEL		0x03	/*modify 0xff//( 0x03<<8) */
#define P1080_TDS_H2_1_CLIP_EN		0x01	/*( 0x01<<7) */
#define P1080_TDS_H2_1_LPF_SEL		0x02	/*modify 0x06//( 0x03<<4) */

/* 0x5B0D8 TDPROC_36 TDSHARP_BANDH2_1 Q*/
#define P1080_TDS_H2_1_SOFT_COR_GAIN 0x08	/*( 0x0f<<12) */
#define P1080_TDS_H2_1_SOFT_CLIP_GAIN 0x40	/*( 0xff<<0) */

/* 0x5B0DC TDPROC_37 TDSHARP_BANDH2_1*/
#define P1080_TDS_H2_1_GAIN_NEG		0xff	/*( 0xff<<24) */

/**************TDSharp End**************/

/**************HV Band Control**************/

/* 0x5B1AC TDPROC_6B Q*/
#define P1080_TDS_HVBAND_COR		0x08	/*( 0xff<<0) */
#define P1080_TDS_HVBAND_EN			0x00	/*( 0x01<<8) */
#define P1080_TDS_BGTESTMODE		0x00ff	/*( 0xffff<<16) */

/* 0x5B1B0 TDPROC_6C Q*/
#define P1080_TDS_HVBAND0LV			0x60	/*( 0xff<<0) */
#define P1080_TDS_HVBAND1LV			0x08	/*( 0xff<<8) */
#define P1080_TDS_HVBAND2LV			0x30	/*( 0xff<<16) */
#define P1080_TDS_HVBAND3LV			0x08	/*( 0xff<<24) */

/* 0x5B1B4 TDPROC_6D Q*/
#define P1080_TDS_HVBAND4LV			0x01	/*( 0xff<<0) */
#define P1080_TDS_HVBAND5LV			0x01	/*( 0xff<<8) */
#define P1080_TDS_HVBAND6LV			0x00	/*( 0xff<<16) */
#define P1080_TDS_HVBAND7LV			0x00	/*( 0xff<<24) */

/* 0x5B1B8 TDPROC_6E Q*/
#define P1080_TDS_LTIHVBAND0LV		0x00	/*( 0xff<<0) */
#define P1080_TDS_LTIHVBAND1LV		0x00	/*( 0xff<<8) */
#define P1080_TDS_LTIHVBAND2LV		0x00	/*( 0xff<<16) */
#define P1080_TDS_LTIHVBAND3LV		0x00	/*( 0xff<<24) */

/* 0x5B1BC TDPROC_6F Q*/
#define P1080_TDS_LTIHVBAND4LV		0x00	/*( 0xff<<0) */
#define P1080_TDS_LTIHVBAND5LV		0x00	/*( 0xff<<8) */
#define P1080_TDS_LTIHVBAND6LV		0x00	/*( 0xff<<16) */
#define P1080_TDS_LTIHVBAND7LV		0x00	/*( 0xff<<24) */

/**************HV Band Control End**************/

/**************TDSharp ADAP**************/

/* 0x5B2C0 TDPROC_B0 Q*/
#define P1080_TDS_ADAP_SHP_EN		0x00	/*shmmodifyat07/29 0x01 0x01//( 0x01<<30) */
#define P1080_TDS_ADAP_SHP_INK_EN	0x00	/*modify 0x00byshmat06/27//( 0x01<<29) */
#define P1080_TDS_ADAP_SHP_EDGE_SEL 0x00	/*modify 0x80//( 0x07<<24) */
#define P1080_TDS_ADAP_SHP_P3		0x00	/*( 0xff<<16) */
#define P1080_TDS_ADAP_SHP_P2		0x00	/*( 0xff<<8) */
#define P1080_TDS_ADAP_SHP_P1		0x00	/*( 0xff<<0) */

/* 0x5B2C4 TDPROC_B1 Q*/
#define P1080_TDS_ADAP_SHP_L_BOUND	0x30	/*( 0xff<<16) */
#define P1080_TDS_ADAP_SHP_EDGE_SCALE 0x20	/*( 0x3ff<<0) */

/* 0x5B2C8 TDPROC_B2 Q*/
#define P1080_TDS_ADAP_SHP_G1		0x201	/*( 0x3ff<<16) */
#define P1080_TDS_ADAP_SHP_U_BOUND	0x01	/*( 0xff<<8) */
#define P1080_TDS_ADAP_SHP_OFFSET	0x02	/*( 0xff<<0) */

/* 0x5B2CC TDPROC_B3 Q*/
#define P1080_TDS_ADAP_SHP_G3		0x204	/*( 0x3ff<<16) */
#define P1080_TDS_ADAP_SHP_G2		0x320	/*( 0x3ff<<0) */

/* 0x5B300 TDPROC_C0 Q*/
#define P1080_TDS_LC_EN				0x01	/*( 0x01<<31) */
#define P1080_TDS_LC_INDEX_SEL		0x01	/*( 0x01<<26) */
#define P1080_TDS_LC_MODE			0x00	/*( 0x03<<24) */
#define P1080_TDS_LC_IDX_LPF_EN		0x01	/*( 0x01<<19) */
#define P1080_TDS_AC_LPF_COE		0x00	/*modify 0x40//( 0x0f<<0) */

/* 0x5B308 TDPROC_C2 Q*/
#define P1080_TDS_LC_IDX_LUT1_G1	0x50	/*( 0x3ff<<16) */
#define P1080_TDS_LC_IDX_LUT1_P1	0x50	/*( 0xff<<8) */
#define P1080_TDS_CHROMA_THD		0x80	/*( 0xff<<0) */

/* 0x5B30C TDPROC_C3 Q*/
#define P1080_TDS_POS_CLIP			0x300	/*( 0x3ff<<20) */
#define P1080_TDS_NEG_CLIP			0x300	/*( 0x3ff<<8) */
#define P1080_TDS_CLIP_GAIN			0x00	/*( 0xff<<0) */

/* 0x5B09C TDPROC_27 Q*/
#define P1080_TDS_LC_LOAD_ENA_H		0x00	/*( 0x01<<3) */
#define P1080_TDS_LC_LOAD_BURST		0x00	/*( 0x01<<2) */
#define P1080_TDS_LC_LOAD_ENA		0x00	/*( 0x01<<1) */
#define P1080_TDS_LC_READ_ENA		0x01	/*( 0x01<<0) */

/* 0x5B0A0 TDPROC_28 Q*/
#define P1080_TDS_LC_LOAD_H			0x00	/*( 0xff<<8) */
#define P1080_TDS_LC_LOAD_IND		0x00	/*( 0x7f<<0) */

/* 0x5B0A4 TDPROC_29 Q*/
#define P1080_TDS_YLEV_TBL			0xff	/*( 0xff<<16) */
#define P1080_TDS_LC_TBL_H			0xff	/*( 0xff<<0) */

/* 0x5B0B0TDPROC_2CQ*/
#define P1080_TDS_YLEV_ENA			0x00	/*( 0x01<<28) */
#define P1080_TDS_YLEV_ALPHA		0x08	/*( 0xff<<20) */
#define P1080_TDS_YLEV_LOAD			0x40	/*( 0xff<<12) */
#define P1080_TDS_YLEV_IND			0x7f	/*modify 0xff//( 0x7f<<4) */
#define P1080_C_YLEV_TBL_LD			0x01	/*( 0x01<<3) */
#define P1080_TDS_YLEV_LOAD_BURST	0x01	/*( 0x01<<2) */
#define P1080_TDS_YLEV_LOAD_ENA		0x01	/*( 0x01<<1) */
#define P1080_TDS_YLEV_READ_ENA		0x01	/*( 0x01<<0) */

/**************TDSharp ADAP End**************/

/**************HLTI**************/

/* 0x5B0E0 LTI_00 BAND1 Q*/
#define P1080_TV_LTI_GAIN1			0x00	/*( 0xff<<24) */
#define P1080_TV_LTI_LIMIT_POS1		0x00	/*( 0xff<<16) */
#define P1080_TV_LTI_LIMIT_NEG1		0x30	/*( 0xff<<8) */
#define P1080_TV_LTI_CORING1		0x00	/*( 0xff<<0) */

/* 0x5B0E4 LTI_01 BAND1 Q*/
#define P1080_TV_LTI_CLIP_THPOS1	0x06	/*( 0xff<<24) */
#define P1080_TV_LTI_CLIP_THNEG1	0x06	/*( 0xff<<16) */
#define P1080_TV_LTI_CLIP_LC_SEL1	0x01	/*( 0x01<<12) */
#define P1080_TV_LTI_ATTENUATE_SEL1 0x00	/*( 0x07<<8) */
#define P1080_TV_LTI_CLIP_EN1		0x01	/*( 0x01<<7) */
#define P1080_TV_LTI_LPF_SEL1		0x01	/*( 0x03<<4) */
#define P1080_TV_LTI_CLIP_BAND_SEL1 0x08	/*( 0x0f<<0) */

/* 0x5B0E8 LTI_02BAND1 Q*/
#define P1080_TV_LTI_SOFT_COR_GAIN1 0x08	/*( 0x0f<<12) */
#define P1080_TV_LTI_PREC1			0x01	/*( 0x03<<8) */
#define P1080_TV_LTI_SOFT_CLIP_GAIN1 0x10	/*( 0xff<<0) */

/* 0x5B0EC LTI_03BAND1 Q*/
#define P1080_TV_LTI_GAIN_NEG1		0x00	/*( 0xff<<24) */

/* 0x5B0F0 LTI_04BAND2 Q*/
#define P1080_TV_LTI_GAIN2			0x10	/*( 0xff<<24) */
#define P1080_TV_LTI_LIMIT_POS2		0x10	/*( 0xff<<16) */
#define P1080_TV_LTI_LIMIT_NEG2		0x10	/*( 0xff<<8) */
#define P1080_TV_LTI_CORING2		0x03	/*( 0xff<<0) */

/* 0x5B0F4 LTI_05BAND2 Q*/
#define P1080_TV_LTI_CLIP_THPOS2	0x00	/*( 0xff<<24) */
#define P1080_TV_LTI_CLIP_THNEG2	0x01	/*( 0xff<<16) */
#define P1080_TV_LTI_CLIP_LC_SEL2	0x01	/*( 0x01<<12) */
#define P1080_TV_LTI_ATTENUATE_SEL2 0x00	/*( 0x07<<8) */
#define P1080_TV_LTI_CLIP_EN2		0x01	/*( 0x01<<7) */
#define P1080_TV_LTI_LPF_SEL2		0x00	/*( 0x03<<4) */
#define P1080_TV_LTI_CLIP_BAND_SEL2 0x00	/*modify 0x40//( 0xf<<0) */

/* 0x5B0F8 LTI_06BAND2 Q*/
#define P1080_TV_LTI_SOFT_COR_GAIN2 0x00	/*modify 0x30//( 0x0f<<12) */
#define P1080_TV_LTI_PREC2			0x00	/*modify 0x60//( 0x03<<8) */
#define P1080_TV_LTI_SOFT_CLIP_GAIN2 0x0a	/*( 0xff<<0) */

/* 0x5B0FC LTI_07BAND2 Q*/
#define P1080_TV_LTI_GAIN_NEG2		0x0b	/*( 0xff<<24) */

/**************LTIEnd**************/

/**************HLTI****************/

/* 0x5B184 HLTI_01 Q*/
#define P1080_TV_HLTI_HDEG_GAIN		0x05	/*( 0xff<<8) */
#define P1080_TV_HLTI_HDIFF_OFFSET	0x10	/*( 0xff<<0) */
#define P1080_TV_HLTI_VDEG_GAIN		0x05	/*( 0xff<<24) */
#define P1080_TV_HLTI_VDIFF_OFFSET	0x00	/*( 0xff<<16) */

/* 0x5B188 HLTI_00 Q*/
#define P1080_TV_HLTI_EN			0x01	/*( 0x1<<0) */
#define P1080_TV_HLTI_PEAKING		0x01	/*( 0x1<<1) */

/* 0x5B18C HLTI_02 Q*/
#define P1080_TV_HLTI_END_X			0x301	/*( 0x3ff<<16) */
#define P1080_TV_HLTI_START_X		0x004	/*( 0x3ff<<0) */

/* 0x5B190 HLTI_03 Q*/
#define P1080_TV_HLTI_SLOPE_X		0x00	/*( 0x7fff<<0) */

/* 0x5B194 HLTI_04*/
#define P1080_TV_HLTI_END_HV		0x102	/*( 0x3ff<<20) */
#define P1080_TV_HLTI_MIDDLE_HV		0x004	/*( 0x3ff<<10) */
#define P1080_TV_HLTI_START_HV		0x005	/*( 0x3ff<<0) */

/* 0x5B198 HLTI_05*/
#define P1080_TV_HLTI_SLOPEUP_HV	0x05	/*( 0x7fff<<16) */
#define P1080_TV_HLTI_SLOPEDOWN_HV	0x05	/*( 0x7fff<<0) */

/**************HLTIEnd**************/

extern struct pp_dft_qty dft_qty_tbl[QUALITY_MAX];
extern uint8_t pp_qty_tbl[QUALITY_MAX];
extern uint8_t _cinema_mode_tbl[12][16];
extern uint8_t _vivid_mode_tbl[12][16];
extern uint32_t _wQtTbl_TDS_Gain_Coring_Limit_SD[82];
extern uint32_t _wQtTbl_TDS_SD[4];
extern uint32_t _wQtTbl_HVBand_SD[10];
extern uint32_t _wQtTbl_ADAP_SD[22];
extern uint32_t _wQtTbl_LTI_SD[28];
extern uint32_t _qty_tbl_tds_gain_coring_limit_720[82];
extern uint32_t _qty_tbl_hv_band_720_1080[10];
extern uint32_t _qty_tbl_adap_720_1080[8];
extern uint32_t _qty_tbl_lti_720_1080[28];
extern uint32_t _qty_tbl_tds_gain_coring_limit_1080[82];

#endif
