/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _DRV_OSD_HW_H
#define _DRV_OSD_HW_H

#include <asm/memory.h>
#include "disp_osd_if.h"

#define OSD_RESET_PLANE_MASK       ((uint32_t) 0xFFFFFFFF)

#define OSD_SC_REG_NUM                 9
#define OSD_CORE_REG_NUM               46
#define OSD_CORE_COMMON_REG_NUM        2
#define OSD_BASE_SKIP                  2
#define OSD_BASE_REG_NUM               63
#define OSD_BASE_COMMON_REG_NUM        43
#define OSD_PREMIX_REG_NUM			   15

#define OSD_HAL_WRITE32(_reg32_, _val_) (*(unsigned int *)(_reg32_) = (_val_))
#define OSD_HAL_READ32(reg32) (*(unsigned int *)(reg32))

#define IO_OSD_WRITE32(base, offset, value)		OSD_HAL_WRITE32((base) + (offset), (value))
#define	IO_OSD_REG32(base, offset)				OSD_HAL_READ32((base) + (offset))

static inline uint32_t PHYSICAL(uint32_t addr)
{
	return (uint32_t) __pa(addr);
}

static inline uint32_t VIRTUAL(uint32_t addr)
{
	return (uintptr_t) __va(addr);
}

typedef struct _OSD_BASE_FIELD_T {
	/* DWORD - 000 OSD_FMT_00 */
	UINT32 fgUpdate:1;
	UINT32 fgAlwaysUpdate:1;
	UINT32:1;
	UINT32 fg_source_sync_select:1;
	UINT32 fg_frame_pol_select:1;
	UINT32 fg_field_pol_select:1;
    UINT32:2;

	UINT32 fgPlaneUpdate:6;
	UINT32:2;
	UINT32 fgSclUpdate:5;
	UINT32 fgPremixUpdate:1;
	UINT32:2;
	UINT32 fgPlaneReflip:5;
	UINT32 fgScalerReCfg:1;
	UINT32:2;

	/* DWORD - 004 OSD_FMT_04 */
	 UINT32:1;
	 UINT32:1;
	 UINT32:1;
	 UINT32:1;
	UINT32 fgRstOsd1:2;
	UINT32 fgRstOsd2:2;
	UINT32 fgRstOsd3:2;
	UINT32 fgRstOsd4:2;
	UINT32 fgRstOsd5:2;
	UINT32 fgRstCsr:2;

	 UINT32:2;
	UINT32 fgRstOsdDec:2;
	 UINT32:2;
	UINT32 fgRstDispFmt:2;
	UINT32 fgRstDgiFmt:2;
	UINT32 fgRstMegFmt:2;
	UINT32 fgRstAuxFmt:2;
	UINT32 fgRstMainFmt:2;

	/* DWORD - 008 OSD_FMT_08 */
	UINT32 fgHsEdge:1;
	UINT32 fgVsEdge:1;
	UINT32 fgFldPol:1;
	UINT32 fgOsd1Prgs:1;
	UINT32 fgOsd2Prgs:1;
	UINT32 fgOsd3Prgs:1;
	UINT32 fgOsd4Prgs:1;
	UINT32 fgOsd5Prgs:1;
	UINT32 fgCsrPrgs:1;
	 UINT32:1;
	UINT32 fgAutoSwEn:1;
	 UINT32:3;

	UINT32 fgHsEdgeDgi:1;
	UINT32 fgVsEdgeDgi:1;
	UINT32 fgFldPolDgi:1;

	UINT32 fgHsEdgeMeg:1;
	UINT32 fgVsEdgeMeg:1;
	UINT32 fgFldPolMeg:1;
	UINT32 u4Osd1Dotctl:2;
	UINT32 u4Osd2Dotctl:2;
	UINT32 u4Osd3Dotctl:2;
	UINT32 u4Osd4Dotctl:2;
	UINT32 u4Osd5Dotctl:2;
	UINT32 u4CsrDotctl:2;

	/* DWORD - 00C OSD_FMT_0C */

	UINT32 u4OvtMain:12;
	UINT32 u4VsWidthMain:9;
	 UINT32:1;
	UINT32 u4HsWidthMain:9;
	 UINT32:1;

	/* DWORD - 010 OSD_FMT_10 */
	UINT32 u4ScrnHStartOsd2:10;
	 UINT32:6;
	UINT32 u4ScrnHStartOsd1:10;
	 UINT32:6;


	/* DWORD - 014 OSD_FMT_14 */
	UINT32 u4ScrnHStartCsr:10;
	 UINT32:6;
	UINT32 u4ScrnHStartOsd3:10;
	 UINT32:6;
	/* DWORD - 018 OSD_FMT_18 */
	UINT32 u4ScrnVStartBotMain:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopMain:9;
	 UINT32:7;

	/* DWORD - 01C OSD_FMT_1C */
	UINT32 u4ScrnVSizeMain:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeMain:13;
	 UINT32:3;

	/* DWORD - 020 OSD_FMT_20 */
	UINT32 u4Osd1VStart:12;
	 UINT32:4;
	UINT32 u4Osd1HStart:12;
	 UINT32:4;

	/* DWORD - 024 OSD_FMT_24 */
	UINT32 u4Osd2VStart:12;
	 UINT32:4;
	UINT32 u4Osd2HStart:12;
	 UINT32:4;

	/* DWORD - 028 OSD_FMT_28 */
	UINT32 u4Osd3VStart:12;
	 UINT32:4;
	UINT32 u4Osd3HStart:12;
	 UINT32:4;

	/* DWORD - 02C OSD_FMT_2C */
	UINT32 u4OvtDgi:12;
	UINT32 u4VsWidthDgi:9;
	 UINT32:1;
	UINT32 u4HsWidthDgi:9;
	 UINT32:1;
/* DWORD - 030 OSD_FMT_30 */
	 UINT32:32;

/* DWORD - 034 OSD_FMT_34 */
	 UINT32:32;

	/* DWORD - 038 OSD_FMT_38 */
	UINT32 u4ScrnVStartBotDgi:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopDgi:9;
	 UINT32:7;

	/* DWORD - 03C OSD_FMT_3C */
	UINT32 u4ScrnVSizeDgi:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeDgi:12;
	 UINT32:4;

	/* DWORD - 040 OSD_FMT_40 */
	UINT32 u4ScrnHStartOsd4:10;
	 UINT32:6;
	UINT32 u4ScrnHStartOsd5:10;
	 UINT32:6;

	/* DWORD - 044 OSD_FMT_44 */
	UINT32 u4Osd4VStart:11;
	 UINT32:5;
	UINT32 u4Osd4HStart:11;
	 UINT32:5;

	/* DWORD - 048 OSD_FMT_48 */
	UINT32 u4Osd5VStart:12;
	 UINT32:4;
	UINT32 u4Osd5HStart:12;
	 UINT32:4;

	/* DWORD - 04C OSD_FMT_4C */
	UINT32 u4OvtAux:12;
	UINT32 u4VsWidthAux:9;
	 UINT32:1;
	UINT32 u4HsWidthAux:9;
	 UINT32:1;

	/* DWORD - 050 OSD_FMT_50 */
	UINT32 u4ScrnVStartBotAux:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopAux:9;
	 UINT32:7;

	/* DWORD - 054 OSD_FMT_54 */
	UINT32 u4ScrnVSizeAux:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeAux:12;
	 UINT32:4;

	/* DWORD - 058 OSD_FMT_58 */
	UINT32 u4OvtMeg:12;
	UINT32 u4VsWidthMeg:9;
	 UINT32:1;
	UINT32 u4HsWidthMeg:9;
	 UINT32:1;

	/* DWORD - 05C OSD_FMT_5C */
	UINT32 u4ScrnVStartBotMeg:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopMeg:9;
	 UINT32:7;

	/* DWORD - 060 OSD_FMT_60 */
	UINT32 u4ScrnVSizeMeg:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeMeg:13;
	 UINT32:3;

	/* DWORD - 064 OSD_FMT_64 */
	UINT32 u4OhtMain:13;
	 UINT32:19;

	/* DWORD - 068 OSD_FMT_68 */
	UINT32 u4OhtAux:13;
	 UINT32:3;
	UINT32 u4OhtDgi:13;
	 UINT32:3;

	/* DWORD - 06C OSD_FMT_6C */
	UINT32 u4OhtMeg:13;
	 UINT32:3;
	UINT32 u4OhtDisp:13;
	 UINT32:3;

	/* DWORD - 070 OSD_FMT_70 */
	 UINT32:5;
	UINT32 fgIntTGen:1;
	UINT32 fgCheckSumEn:1;
	 UINT32:8;
	UINT32 u4Sc1CheckSumSel:2;
	UINT32 u4Sc2CheckSumSel:2;
	UINT32 u4Sc3CheckSumSel:2;
	UINT32 u4Sc4CheckSumSel:2;
	 UINT32:9;

	/* DWORD - 074  */
	UINT32 u4OhtFmt2:13;
	 UINT32:3;
	UINT32 u4OhtPost:13;
	 UINT32:3;

	/* DWORD - 078 OSD_FMT_78 */
	UINT32 u4Osd1CheckSum:32;

	/* DWORD - 07C OSD_FMT_7C */
	UINT32 u4Osd1ScCheckSum:32;

	/* DWORD - 080 OSD_FMT_80 */
	UINT32 u4Osd2CheckSum:32;

	/* DWORD - 084 OSD_FMT_84 */
	UINT32 u4Osd2ScCheckSum:32;

	/* DWORD - 088 OSD_FMT_88 */
	UINT32 u4Osd3CheckSum:32;

	/* DWORD - 08C OSD_FMT_8C */
	UINT32 u4Osd3ScCheckSum:32;

	/* DWORD - 090 OSD_FMT_90 */
	UINT32 u4Osd4CheckSum:32;

	/* DWORD - 094 OSD_FMT_94 */
	UINT32 u4Osd4ScCheckSum:32;

	/* DWORD - 098 OSD_FMT_98 */
	UINT32 u4Osd5CheckSum:32;

	/* DWORD - 09C OSD_FMT_9C */
	UINT32 u4CSRCheckSum:32;

	/* DWORD - 0A0 OSD_FMT_A0 */
	UINT32 u4OSD5IntState:1;
	UINT32 u4OSD4IntStatus:1;
	UINT32 u4OSD3IntStatus:1;
	UINT32 u4OSD2IntStatus:1;
	UINT32 u4OSD1IntStatus:1;
	 UINT32:27;

	/* DWORD - 0A4 OSD_FMT_A4 */
	UINT32 fgOsd12Ex:1;
	UINT32 fgOsd34Ex:1;
	 UINT32:3;
	UINT32 fgOsdShareSram:1;
	 UINT32:8;

	UINT32 u4SRamType:6;
	UINT32 u4AlphaSel:5;
	 UINT32:1;
	UINT32 u4IOMonSel:6;


	/* DWORD - 0A8 OSD_FMT_A8 */
	UINT32 fgOsd14ScShare:1;
	UINT32 fgOsd14ScShareSrcVr:1;
	 UINT32:2;
	UINT32 fgFldPolDisp:1;
	UINT32 fgVsEdgeDisp:1;
	UINT32 fgHsEdgeDisp:1;
	UINT32 fgFldPolPost:1;
	UINT32 fgVsEdgePost:1;
	UINT32 fgHsEdgePost:1;
	UINT32 fgFldPolFmt2:1;
	UINT32 fgVsEdgeFmt2:1;
	UINT32 fgHsEdgeFmt2:1;
	 UINT32:3;
	UINT32 fgOsd1AuxHI:1;
	UINT32 fgOsd2AuxHI:1;
	UINT32 fgOsd3AuxHI:1;
	UINT32 fgOsd4AuxHI:1;
	UINT32 fgOsd5AuxHI:1;
	UINT32 fgCsrAuxHI:1;
	 UINT32:10;

	/* DWORD - 0AC OSD_FMT_AC */
	UINT32 fgOsd1Aux:4;
	UINT32 fgOsd2Aux:4;
	UINT32 fgOsd3Aux:4;
	UINT32 fgOsd4Aux:4;
	UINT32 fgOsd5Aux:4;
	UINT32 fgCsrAux:4;
	 UINT32:8;

	/*DWORD - 0B0 OSD_FMT_B0 */
	UINT32 u4OvtDisp:12;
	UINT32 u4VsWidthDisp:9;
	 UINT32:1;
	UINT32 u4HsWidthDisp:9;
	 UINT32:1;


	/*DWORD - 0B4 OSD_FMT_B4 */
	UINT32 u4ScrnVStartBotDisp:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopDisp:9;
	 UINT32:7;

	/*DWORD - 0B8 OSD_FMT_B8 */
	UINT32 u4ScrnVSizeDisp:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeDisp:13;
	 UINT32:3;

	/*DWORD - 0BC OSD_FMT_BC */
	UINT32 fgWtDecOnOsd1:1;
	UINT32 fgWtDecOnOsd2:1;
	UINT32 fgWtDecOnOsd3:1;
	UINT32 fgWtDecOnOsd4:1;
	 UINT32:28;

/*DWORD - 0C0 OSD_FMT_C0 */
	 UINT32:32;
/*DWORD - 0C4 OSD_FMT_C4*/
	 UINT32:32;
/*DWORD - 0C8 OSD_FMT_C8 */
	 UINT32 osd_start_update:1;
	 UINT32 osd_plane_update:1;
	 UINT32 osd_sync_threshold_update:1;
	 UINT32 osd_window_update:1;
	 UINT32:28;
/*DWORD - 0CC OSD_FMT_CC */
	 UINT32:32;

/*DWORD - 0D0 OSD_FMT_D0 */
	 UINT32:27;
	UINT32 fgShutterShiftEn:1;
	UINT32 fgTgenLRInv:1;
	UINT32 fgTgenLRDly:1;
	UINT32 fgTgenLLRR:1;
	UINT32 fgTgenLREn:1;
	/*DWORD - 0D4 OSD_FMT_D4 */
	UINT32 u4OSD1LineShift:8;
	UINT32 u4OSD2LineShift:8;
	UINT32 u4OSD3LineShift:8;
	UINT32 u4CSRLineShift:8;
/*DWORD - 0D8 OSD_FMT_D8 */
	 UINT32:32;
/*DWORD - 0DC OSD_FMT_DC */
	 UINT32:32;

	/*DWORD - 0E0 OSD_FMT_E0 */
	UINT32 u4OvtPost:12;
	UINT32 u4VsWidthPost:9;
	 UINT32:1;
	UINT32 u4HsWidthPost:9;
	 UINT32:1;
	/*DWORD - 0E4 OSD_FMT_E4 */
	UINT32 u4ScrnVStartBotPost:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopPost:9;
	 UINT32:7;
	/*DWORD - 0E8 OSD_FMT_E8 */
	UINT32 u4ScrnVSizePost:12;
	 UINT32:4;
	UINT32 u4ScrnHSizePost:13;
	 UINT32:3;
	/*DWORD - 0EC OSD_FMT_EC */
	 UINT32:32;

	/*DWORD - 0F0 OSD_FMT_F0 */
	UINT32 u4OvtFmt2:12;
	UINT32 u4VsWidthFmt2:9;
	 UINT32:1;
	UINT32 u4HsWidthFmt2:9;
	 UINT32:1;
	/*DWORD - 0F4 OSD_FMT_F4 */
	UINT32 u4ScrnVStartBotFmt2:12;
	 UINT32:4;
	UINT32 u4ScrnVStartTopFmt2:9;
	 UINT32:7;
	/*DWORD - 0F8 OSD_FMT_F8 */
	UINT32 u4ScrnVSizeFmt2:12;
	 UINT32:4;
	UINT32 u4ScrnHSizeFmt2:13;
	 UINT32:3;
} OSD_BASE_FIELD_T;

typedef struct _OSD_PLA_CORE_FIELD_T {
	/* DWORD - 000 */
	UINT32 fgOsdEn:1;	/* OSD enable */
	 UINT32:1;		/* skip */
	UINT32 fgFakeHdr:1;
	UINT32 fgPrngEn:1;
	 UINT32:1;		/* skip  */
	UINT32 fgOsdDbg:1;	/* skip  */
	 UINT32:1;		/* skip  */
	UINT32 fgAlphaZeroBlack:1;
	UINT32 fgOutRngColorMode:1;
	 UINT32:4;
	UINT32 fgHw3DFrameModeEn:1;
	 UINT32:18;		/* skip */

	/* DWORD - 004 */
	UINT32 u4HeaderAddr:28;
	 UINT32:4;		/* skip */

	/* DWORD - 008 */
	UINT32 u4GobalBlending:8;	/*  global blending ratio */
	UINT32 u4FadingRatio:8;	/*  fading blending ratio */
	UINT32 fgHFilter:1;	/*  h interpolation en */
	UINT32 fgColorExpSel:1;	/*  exp mode for color conv */
	UINT32 fgAlphaRatioEn:1;	/*  exp mode for alpha */
	 UINT32:4;		/* skip */
	UINT32 fgHMirrorEn:1;
	UINT32 fgVFlipEn:1;
	UINT32 fgRgb2YcbrbEn:1;
	UINT32 fgXVYCCEn:1;	/* Full Range YCC Color conv */
	UINT32 fgYCbCr709En:1;
	UINT32 u4PreMultiMode:2;
	 /**/ UINT32 fgNonAlphaShift:1;	/* skip */
	 UINT32:1;		/* skip */

	/* DWORD - 00C */
	UINT32 u4ContReqLmt:4;
	 UINT32:2;
	UINT32 u4FifoSize:4;	/* OSD FIFO logical size */
	 UINT32:2;
	UINT32 u4PauseCnt:4;
	UINT32 u4ContReqLmt0:4;
	 UINT32:2;
	UINT32 fgBurstDis:1;	/* DRAM read burst control */
	UINT32 fgRgbMode:1;	/*  WB is RGB / YCbCr */
	UINT32 u4VacancyThr:4;	/*  FIFO vacancy thred */
	 UINT32:4;
	 /**/
	    /* DWORD - 110 */
	 UINT32 fgOsd1ArbRgnEn:1;	/*  arbitrary region en */
	 UINT32:31;

	/* DWORD - 014 */
	UINT32 u4ReplaceCr:8;	/* replace Cr value */
	UINT32 u4ReplaceCb:8;
	UINT32 u4ReplaceY:8;
	UINT32 fgReplaceEn:1;	/* repl pixel whose alpha= 0 */
	 UINT32:7;
/* DWORD - 018  OSD_PLA_018*/
	 UINT32 fgOsdEnEffect:1;	 /* OSD enable effect*/
	 UINT32:31;		/* Reserved */
/* DWORD - 01C  OSD_PLA_01C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 020  OSD_PLA_020*/
	 UINT32:32;		/* Reserved */
/* DWORD - 024  OSD_PLA_024*/
	 UINT32:32;		/* Reserved */
/* DWORD - 028  OSD_PLA_028*/
	 UINT32:32;		/* Reserved */
/* DWORD - 02C  OSD_PLA_02C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 030  OSD_PLA_030*/
	 UINT32:32;		/* Reserved */
/* DWORD - 034  OSD_PLA_034*/
	 UINT32:32;		/* Reserved */
/* DWORD - 038  OSD_PLA_038*/
	 UINT32:32;		/* Reserved */
/* DWORD - 03C  OSD_PLA_03C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 040  OSD_PLA_040*/
	 UINT32:32;		/* Reserved */
/* DWORD - 044  OSD_PLA_044*/
	 UINT32:32;		/* Reserved */
/* DWORD - 048  OSD_PLA_048*/
	 UINT32:32;		/* Reserved */
/* DWORD - 04C  OSD_PLA_04C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 050  OSD_PLA_050*/
	 UINT32:32;		/* Reserved */
/* DWORD - 054  OSD_PLA_054*/
	 UINT32:32;		/* Reserved */
/* DWORD - 058  OSD_PLA_058*/
	 UINT32:32;		/* Reserved */
/* DWORD - 05C  OSD_PLA_05C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 060  OSD_PLA_060*/
	UINT32:16;
	UINT32 u4line_cnt:11;
	UINT32:5;
/* DWORD - 064  OSD_PLA_064*/
	UINT32 u4AlphaCoordinate:32;
/* DWORD - 068  OSD_PLA_068*/
	 UINT32:32;		/* Reserved */
/* DWORD - 06C  OSD_PLA_06C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 070  OSD_PLA_070*/
	UINT32 u4OsdPtrR:26;	/* Reserved */
	 UINT32:6;
/* DWORD - 074  OSD_PLA_074*/
	 UINT32:32;		/* Reserved */
/* DWORD - 078  OSD_PLA_078*/
	 UINT32:32;		/* Reserved */
/* DWORD - 07C  OSD_PLA_07C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 080  OSD_PLA_080*/
	 UINT32:32;		/* Reserved */
/* DWORD - 084  OSD_PLA_084*/
	 UINT32:32;		/* Reserved */
/* DWORD - 088  OSD_PLA_088*/
	 UINT32:32;		/* Reserved */
/* DWORD - 08C  OSD_PLA_08C*/
	 UINT32:32;		/* Reserved */

/* DWORD - 090  OSD_PLA_090*/
	 UINT32:32;		/* Reserved */
/* DWORD - 094  OSD_PLA_094*/
	 UINT32:32;		/* Reserved */
/* DWORD - 098  OSD_PLA_098*/
	 UINT32:32;		/* Reserved */
/* DWORD - 09C  OSD_PLA_09C*/
	 UINT32:32;		/* Reserved */

	/* DWORD - 0A0  OSD_PLA_0A0 */
	UINT32 u4YMulR:10;	/* R coef within Y equation */
	UINT32 u4YMulG:10;	/* G coef within Y equation */
	UINT32 u4YMulB:10;	/* B coef within Y equation */
	UINT32 u4EableRGB2YCC:1;	/* en config RGB2YCC coef */
	 UINT32:1;		/*skip */
	/* DWORD - 0A4  OSD_PLA_0A4 */
	UINT32 u4CbMulR:10;	/* R coef within Y  */
	UINT32 u4CbMulG:10;	/* G coef within Y  */
	UINT32 u4CbMulB:10;	/* B coef within Y  */
	 UINT32:2;		/*skip */
	/* DWORD - 0A8  OSD_PLA_0A8 */
	UINT32 u4CrMulR:10;	/* R coef within Y  */
	UINT32 u4CrMulG:10;	/* G coef within Y  */
	UINT32 u4CrMulB:10;	/* B coef within Y  */
	 UINT32:2;		/*skip */

/* DWORD - 0AC  OSD_PLA_0AC*/
	 UINT32 zero_cl_rp_cr:8;
	 UINT32 zero_cl_rp_cb:8;
	 UINT32 zero_cl_rp_y:8;
	 UINT32 zero_cl_rp_en:1;
	 UINT32:3;
	 UINT32 burst_8_enable:1;
	 UINT32 mptr_lmt_select:1;
	 UINT32 no_dle_select:1;
	 UINT32 rg_gcalst_sel:1;

/* DWORD - 0B0  OSD_PLA_0B0*/
	 UINT32 alpha_threshold:8;
	 UINT32 region_width:8;
	 UINT32 alpha_det_mon_sel:3;
	 UINT32 rg_alpha_det_clear:1;
	 UINT32 rg_alpha_det_en:1;
	 UINT32:11;
/* DWORD - 0B4  OSD_PLA_0B4*/
	 UINT32 alpha_region_flag:32;
/* DWORD - 0B8  OSD_PLA_0B8*/
	 UINT32:32;
/* DWORD - 0BC  OSD_PLA_0BC*/
	 UINT32	auto_ultra:32;

/* DWORD - 0C0  OSD_PLA_0C0*/
	 UINT32:32;
/* DWORD - 0C4  OSD_PLA_0C4*/
	 UINT32:32;
/* DWORD - 0C8  OSD_PLA_0C8*/
	 UINT32:32;
/* DWORD - 0CC  OSD_PLA_0CC*/
	 UINT32:32;

/* DWORD - 0D0  OSD_PLA_0D0*/
	 UINT32:32;
/* DWORD - 0D4  OSD_PLA_0D4*/
	 UINT32:32;
/* DWORD - 0D8  OSD_PLA_0d8*/
	 UINT32:32;
/* DWORD - 0DC  OSD_PLA_0DC*/
	 UINT32:32;
/* DWORD - 0E0  OSD_PLA_0E0*/
	 UINT32:32;
/* DWORD - 0E4  OSD_PLA_0E4*/
	UINT32 u4SyncThres:12;
	 UINT32:20;
} OSD_PLA_CORE_FIELD_T;

typedef struct _OSD_PLA_CORE_AUTO_FLP_FIELD_T {
	UINT32 u4NonRegionV:8;
	UINT32 u4NonRegionU:8;
	UINT32 u4NonRegionY:8;
	UINT32 u4NonRegionA:8;
	 UINT32:32;
	 UINT32:32;
	 UINT32:32;
	UINT32 u4DbgRdyModL:4;
	UINT32 u4DbgAckModL:4;
	UINT32 u4DbgRdyModR:4;
	UINT32 u4DbgAckModR:4;
	UINT32 fgDbgRdyRlbL:1;
	UINT32 fgDbgAckRlbL:1;
	UINT32 fgDbgRdyRlbR:1;
	UINT32 fgDbgAckRlbR:1;
	UINT32 fgFrameForceInit:1;
	 UINT32:6;
	UINT32 fg3DRacAuto:1;
	UINT32 fgFrameInitBufL:1;
	UINT32 fgFrameInitBufR:1;
	UINT32 fgFrameInitRLb:1;
	UINT32 fgFlipEn:1;
	 UINT32:32;
	UINT32 u4TmpAddrL:32;
	UINT32 u4NormAddrR:32;
	UINT32 u4TmpAddrR:32;
	 UINT32:32;
	UINT32 u4DbgRdyVsL:13;
	 UINT32:3;
	UINT32 u4DbgRdyVeL:13;
	 UINT32:3;
	UINT32 u4DbgRdyVsR:13;
	 UINT32:3;
	UINT32 u4DbgRdyVeR:13;
	 UINT32:3;
	UINT32 u4DbgAckVsL:13;
	 UINT32:3;
	UINT32 u4DbgAckVsR:13;
	 UINT32:3;
} OSD_PLA_CORE_AUTO_FLP_FIELD_T;


typedef struct _OSD_PLA_CORE_PLT_FIELD_T {
	UINT32 u4PLT01:32;
	UINT32 u4PLT02:32;
	UINT32 u4PLT03:32;
	UINT32 u4PLT04:32;
	UINT32 u4PLT05:32;
	UINT32 u4PLT06:32;
	UINT32 u4PLT07:32;
	UINT32 u4PLT08:32;
	UINT32 u4PLT09:32;
	UINT32 u4PLT10:32;
	UINT32 u4PLT11:32;
	UINT32 u4PLT12:32;
	UINT32 u4PLT13:32;
	UINT32 u4PLT14:32;
	UINT32 u4PLT15:32;
	UINT32 u4PLT16:32;
} OSD_PLA_CORE_PLT_FIELD_T;

typedef struct _OSD_SC_FIELD_T {
	/* DWORD - 000  OSD_SC_00 */
	UINT32:2;
	UINT32 fgVuscEn:1;	/* vertical up scaler en switch */
	UINT32 fgVdscEn:1;	/* vertical down scaler en switch */
	UINT32 fgHuscEn:1;	/* h up scaler en switch */
	UINT32 fgHdscEn:1;	/* h down scaler en switch */
	UINT32 fgScLpfEn:1;	/* anti-aliasing filter en switch */
	UINT32 fgScEn:1;	/* scaler enable switch */

	 UINT32:4;
	UINT32 fgVuscAlphaEdgeRcvr:1;
	UINT32 fgVuscAlphaEdgeElt:1;
	UINT32 fgVdscAlphaEdgeRcvr:1;
	UINT32 fgVdscAlphaEdgeElt:1;
	UINT32 fgHuscAlphaEdgeRcvr:1;
	UINT32 fgHuscAlphaEdgeElt:1;
	UINT32 fgHdscColorOnly:1;
	UINT32 fgHdscAlphaEdgeElt:1;
	UINT32 fgHdscAlphaEdgeRcvr:1;
	 UINT32:11;

	/* DWORD - 004  OSD_SC_04 */
	UINT32 u4SrcVSize:13;
	 UINT32:3;
	UINT32 u4SrcHSize:13;
	 UINT32:3;


	/* DWORD - 008 OSD_SC_08 */
	UINT32 u4DstVSize:13;
	 UINT32:3;
	UINT32 u4DstHSize:13;
	 UINT32:3;

	/* DWORD - 00C OSD_SC_0C */
	UINT32 u4VscHSize:13;
	 UINT32:19;

	/* DWORD - 010 OSD_SC_10 */
	UINT32 u4HdscStep:14;
	 UINT32:2;
	UINT32 u4HdscOfst:14;
	 UINT32:2;

	/* DWORD - 014 */
	UINT32 u4HuscStep:14;
	 UINT32:2;
	UINT32 u4HuscOfst:14;
	 UINT32:2;

	/* DWORD - 018 */
	UINT32 u4VscOfstBot:14;
	 UINT32:2;
	UINT32 u4VscOfstTop:14;
	 UINT32:2;

	/* DWORD - 01C */
	UINT32 u4VscStep:14;
	 UINT32:18;

	/* DWORD - 020 Anti-Aliasing Filter configuration Register OSD_SC_20 */
	UINT32 u4ScLpfC5:7;
	 UINT32:1;
	UINT32 u4ScLpfC4:7;
	 UINT32:1;
	UINT32 u4ScLpfC3:6;
	 UINT32:10;
} OSD_SC_FIELD_T;

typedef struct _OSD_RGN_FIELD_T {
	/* DWORD - 000 */
	UINT32 u4NextOsdAddr:28;
	UINT32 u4ColorMode:4;

	/* DWORD - 004 */
	UINT32 u4DataAddr:24;
	UINT32 u4MixWeight:8;

	/* DWORD - 008 */
	UINT32 u4LineSize:11;
	 UINT32:13;
	UINT32 u4VbSel:2;
	UINT32 u4UgSel:2;
	UINT32 u4YrSel:2;
	UINT32 u4AlphaSel:2;

	/* DWORD - 00C */
	UINT32 u4PaletteAddr:26;
	UINT32 u4PaletteLen:2;
	UINT32 fgNewPalette:1;
	UINT32 fgDeCompEn:1;
	UINT32 fgDeCompLineBased:1;
	UINT32 fgSelectByteEn:1;

	/* DWORD - 010 */
	UINT32 u4VStep:17;
	 UINT32:15;

	/* DWORD - 014 */
	UINT32 u4HStep:17;
	 UINT32:15;

	/* DWORD - 018 */
	UINT32 fgFifoEx:1;
	UINT32 fgNextOsdEn:1;
	UINT32 u4DataAddrHI:2;
	UINT32 u4PaletteAddrHI:2;
	 UINT32:17;
	UINT32 fgColorKeyEn:1;
	UINT32 u4ColorKey:8;


	/*DWORD-01C */
	 UINT32:11;
	UINT32 fgWTEn:1;
	 UINT32:11;
	UINT32 u4DataAddrMD:2;
	UINT32 u4DeCompMode:2;
	UINT32 fgAcsFrame:1;
	UINT32 fgAcsAuto:1;
	UINT32 fgAcsTop:1;
	UINT32 u4MixSel:2;


	/*DWORD-020 */
	UINT32 u4HClip:13;
	 UINT32:3;
	UINT32 u4VClip:12;
	 UINT32:4;

	/*DWORD-024 */
	UINT32 u4Ihw:13;
	 UINT32:3;
	UINT32 u4Ivw:12;
	 UINT32:4;


	/*DWORD-028 */
	UINT32 u4Ovw:12;
	 UINT32:4;
	UINT32 u4Ovs:12;
	 UINT32:4;

	/*DWORD-02C */
	UINT32 u4Ohw:13;
	 UINT32:3;
	UINT32 u4Ohs:13;
	 UINT32:3;
	 UINT32:32;
	 UINT32:32;
	 UINT32:32;
	 UINT32:32;

/*DWORD-030 */
	 UINT32:32;
/*DWORD-034 */
	 UINT32:32;
/*DWORD-038 */
	 UINT32:32;
/*DWORD-03C */
	 UINT32:32;
} OSD_RGN_FIELD_T;

typedef struct {
	/*OSD_PREMIX_CFG00*/
	UINT32 rg_htotal:13;
	UINT32 rg_b_y_0_2:3;
	UINT32 rg_vtotal:13;
	UINT32 rg_b_y_3_5:3;
	/*OSD_PREMIX_CFG01*/
	UINT32 rg_win_hst:13;
	UINT32 rg_b_cb_0_2:3;
	UINT32 rg_win_hend:13;
	UINT32 rg_b_cb_3_5:3;
	/*OSD_PREMIX_CFG02*/
	UINT32 rg_win_vst:13;
	UINT32 rg_b_cr_0_2:3;
	UINT32 rg_win_vend:13;
	UINT32 rg_b_cr_3_5:3;
	/*OSD_PREMIX_CFG03*/
	UINT32 rg_win1_hst:13;
	UINT32:3;
	UINT32 rg_win1_hend:13;
	UINT32:3;
	/*OSD_PREMIX_CFG04*/
	UINT32 rg_win1_vst:13;
	UINT32:3;
	UINT32 rg_win1_vend:13;
	UINT32:3;
	/*OSD_PREMIX_CFG05*/
	UINT32 rg_win2_hst:13;
	UINT32:3;
	UINT32 rg_win2_hend:13;
	UINT32:3;
	/*OSD_PREMIX_CFG06*/
	UINT32 rg_win2_vst:13;
	UINT32:3;
	UINT32 rg_win2_vend:13;
	UINT32:3;
	/*OSD_PREMIX_CFG07*/
	UINT32 rg_alpha_val:8;

	UINT32 rg_win0_en:1;
	UINT32 rg_win1_en:1;
	UINT32 rg_win2_en:1;
	UINT32 shadow_en:1;

	UINT32 shadow_update:1;
	UINT32 rg_vs_pol_sel:1;
	UINT32:2;

	UINT32 rg_pln0_max_a_sel:1;
	UINT32 rg_pln1_max_a_sel:1;
	UINT32 rg_pln2_max_a_sel:1;
	UINT32 rg_sync_mode:1;

	UINT32 rg_dbg_sel:4;
	UINT32 rg_pln0_de_inv:1;
	UINT32 rg_pln1_de_inv:1;
	UINT32 rg_b_cr_6_7:2;
	UINT32 rg_b_cb_6_7:2;
	UINT32 rg_b_y_6_7:2;

	/*OSD_PREMIX_CFG08*/
	UINT32 swap_0:2;
	UINT32 plane0_y_sel:2;
	UINT32 plane0_cb_sel:2;
	UINT32 plane0_cr_sel:2;
	UINT32 swap_1:2;
	UINT32 plane1_y_sel:2;
	UINT32 plane1_cb_sel:2;
	UINT32 plane1_cr_sel:2;
	UINT32 swap_2:2;
	UINT32 plane2_y_sel:2;
	UINT32 plane2_cb_sel:2;
	UINT32 plane2_cr_sel:2;
	UINT32 rg_premul:1;
	UINT32 rg_max_a_sel_div:1;
	UINT32 rg_max_a_sel:1;
	UINT32 rg_pln0_mask:1;
	UINT32 rg_premix_dbg_sel:4;
	/*OSD_PREMIX_CFG09*/
	UINT32 mix1_pre_mul_a:1;
	UINT32 mix1_top_mix_bg:1;
	UINT32 mix1_top_mask:1;
	UINT32 mix1_bot_mask:1;

	UINT32:1;
	UINT32 mix1_bypass:1;
	UINT32 mix1_sign_en:1;
	UINT32 mix1_c_top_inv:1;

	UINT32:9;
	UINT32 mix1_c_bot_inv:1;
	UINT32:1;
	UINT32 mix1_c_aft_mix_inv:1;
	UINT32:9;
	UINT32 mix1_premul_set_rgb:1;
	UINT32:1;
	UINT32 mix1_de_sel:1;
	/*OSD_PREMIX_CFG0A*/
	UINT32 mix2_pre_mul_a:1;
	UINT32 mix2_top_mix_bg:1;
	UINT32 mix2_top_mask:1;
	UINT32 mix2_bot_mask:1;

	UINT32:1;
	UINT32 mix2_bypass:1;
	UINT32 mix2_sign_en:1;
	UINT32 mix2_c_top_inv:1;

	UINT32:9;
	UINT32 mix2_c_bot_inv:1;
	UINT32:1;
	UINT32 mix2_c_aft_mix_inv:1;
	UINT32:9;
	UINT32 mix2_premul_set_rgb:1;
	UINT32:1;
	UINT32 mix2_de_sel:1;
	/*OSD_PREMIX_CFG0B*/
	UINT32:32;
	/*OSD_PREMIX_CFG0C*/
	UINT32:32;
	/*OSD_PREMIX_CFG0D*/
	UINT32:32;
	/*OSD_PREMIX_CFG0E*/
	UINT32:16;
	UINT32 vdo4_cfg_alpha:8;
	UINT32:8;
} OSD_PREMIX_FIELD_T;

typedef union _OSD_BASE_UNION_T {
	UINT32 au4Reg[OSD_BASE_REG_NUM];
	OSD_BASE_FIELD_T rField;
} OSD_BASE_UNION_T;

typedef union _OSD_PLA_CORE_UNION_T {
	UINT32 au4Reg[0x50];
	OSD_PLA_CORE_FIELD_T rField;
} OSD_PLA_CORE_UNION_T;


typedef union _OSD_PLA_CORE_AUTO_FLP_UNION_T {
	UINT32 au4Reg[13];
	OSD_PLA_CORE_AUTO_FLP_FIELD_T rField;
} OSD_PLA_CORE_AUTO_FLP_UNION_T;

typedef union _OSD_PLA_CORE_PLT_UNION_T {
	UINT32 au4Reg[16];
	OSD_PLA_CORE_PLT_FIELD_T rField;
} OSD_PLA_CORE_PLT_UNION_T;

typedef union _OSD_SC_UNION_T {
	UINT32 au4Reg[OSD_SC_REG_NUM];
	OSD_SC_FIELD_T rField;
} OSD_SC_UNION_T;

typedef union _OSD_RGN_UNION_T {
	UINT32 au4Reg[12];
	OSD_RGN_FIELD_T rField;
} OSD_RGN_UNION_T;

typedef union _OSD_PREMIX_UNION_T {
	UINT32 au4Reg[OSD_PREMIX_REG_NUM];
	OSD_PREMIX_FIELD_T rField;
} OSD_PREMIX_UNION_T;

typedef enum {
	OSD_RGN_STATE_IDLE,
	OSD_RGN_STATE_ACTIVE,
	OSD_RGN_STATE_USED,
	OSD_RGN_STATE_READ_DONE
} OSD_RGN_STATE;

typedef struct _OSD_REGION_NODE_T {
	unsigned long pu4_vAddr;
	unsigned int pu4_pAddr;
	OSD_RGN_STATE eOsdRgnState;
} OSD_REGION_NODE_T;

extern void _OSD_BASE_GET_REG_BASE(uintptr_t *base_reg, uintptr_t premix_reg);
extern void _OSD_AlwaysUpdateReg(unsigned int FMT_ID, BOOL fgEnable);
extern void _OSD_UpdateReg(unsigned int FMT_ID, BOOL fgEnable);
extern void _OSD_BASE_Reset(UINT32 u4Plane);
extern INT32 _OSD_BASE_GetReg(unsigned int u4Plane, UINT32 *pOsdBaseReg);
extern INT32 _OSD_BASE_SetReg(unsigned int u4Plane, const UINT32 *pOsdBaseReg);
extern INT32 _OSD_BASE_UpdateHwReg(void);
extern INT32 _OSD_BASE_Update(uint32_t plane);
extern INT32 OSD_PREMIX_Update(uint32_t plane);
extern INT32 OSD_PREMIX_Update_vdo4(void);
extern INT32 _OSD_BASE_GetPlaneUpdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetSclUpdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_SetPremixUpdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_GetPremixUpdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetPlaneReflip(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetScalerReCfg(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetUpdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetAlwaysUpdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetForceUnupdate(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetResetMainPath(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetResetOsd1(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetResetOsd2(unsigned int u4Plane, UINT32 *pu4Value);
/* 08h OSD mode configuration register */
extern INT32 _OSD_BASE_GetHsEdge(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetVsEdge(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetFldPol(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd3Prgs(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd2Prgs(unsigned int u4Plane, UINT32 *pu4Value);

extern INT32 _OSD_BASE_GetOsd1Path(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd2Path(unsigned int u4Plane, UINT32 *pu4Value);

extern INT32 _OSD_BASE_GetOsd1Dotctl(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd2Dotctl(unsigned int u4Plane, UINT32 *pu4Value);

/* 0Ch Main FMT Vsync Timing Configuration Register */
extern INT32 _OSD_BASE_GetOvtMain(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetVsWidthMain(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetHsWidthMain(unsigned int u4Plane, UINT32 *pu4Value);
/* 10h FMT H-Timing Configuration Register#1 */
extern INT32 _OSD_BASE_GetScrnHStartOsd2(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetScrnHStartOsd1(unsigned int u4Plane, UINT32 *pu4Value);

/* 18h Main FMT V-Timing Configuration Register #1 */
extern INT32 _OSD_BASE_GetScrnVStartBotMain(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetScrnVStartTopMain(unsigned int u4Plane, UINT32 *pu4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
extern INT32 _OSD_BASE_GetScrnVSize(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetScrnHSize(unsigned int u4Plane, UINT32 *pu4Value);
/* 20h OSD1 Window Position Configuration Register */
extern INT32 _OSD_BASE_GetOsd3VStart(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd3HStart(unsigned int u4Plane, UINT32 *pu4Value);
/* 24h OSD2 Window Position Configuration Register  */
extern INT32 _OSD_BASE_GetOsd2VStart(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd2HStart(unsigned int u4Plane, UINT32 *pu4Value);



/*  64 Main FMT Timing Configuration Register #2*/
extern INT32 _OSD_BASE_GetOhtMain(unsigned int u4Plane, UINT32 *pu4Value);
/*  70h */
extern INT32 _OSD_BASE_GetIntTGen(unsigned int u4Plane, UINT32 *pu4Value);

/* A4h */
/*Osd12Ex & Osd34Ex declaerd at 2cH for compatible*/
extern INT32 _OSD_BASE_GetSRamType(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetAlphaSel(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetIOMonSel(unsigned int u4Plane, UINT32 *pu4Value);

extern INT32 _OSD_BASE_GetFmt(UINT32 u4Path, UINT32 *pu4DisplayMode);
extern INT32 _OSD_BASE_GetScrnVSizeMain(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetScrnHSizeMain(unsigned int u4Plane, UINT32 *pu4Value);

/*---------------- set function ---------------*/
extern INT32 _OSD_BASE_SetUpdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetPlaneUpdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetSclUpdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetPlaneReflip(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetScalerReCfg(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetAlwaysUpdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetForceUnupdate(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetResetMainPath(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetResetOsd1(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetResetOsd2(unsigned int u4Plane, UINT32 u4Value);
/* 08h OSD mode configuration register */
extern INT32 _OSD_BASE_SetHsEdge(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetVsEdge(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetFldPol(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd3Prgs(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd2Prgs(unsigned int u4Plane, UINT32 u4Value);

extern INT32 _OSD_BASE_SetOsd1Path(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd2Path(unsigned int u4Plane, UINT32 u4Value);

extern INT32 _OSD_BASE_SetOsd1Dotctl(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd2Dotctl(unsigned int u4Plane, UINT32 u4Value);

extern INT32 _OSD_BASE_SetAutoSwEn(unsigned int u4Plane, UINT32 u4Value);


/* 0Ch Main FMT Vsync Timing Configuration Register */
extern INT32 _OSD_BASE_SetOvtMain(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetVsWidthMain(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetHsWidthMain(unsigned int u4Plane, UINT32 u4Value);
/* 10h FMT H-Timing Configuration Register#1 */
extern INT32 _OSD_BASE_SetScrnHStartOsd2(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetScrnHStartOsd3(unsigned int u4Plane, UINT32 u4Value);

/* 18h Main FMT V-Timing Configuration Register #1 */
extern INT32 _OSD_BASE_SetScrnVStartBotMain(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetScrnVStartTopMain(unsigned int u4Plane, UINT32 u4Value);
/* 1Ch Main FMT Timing Configuration Register #2 */
extern INT32 _OSD_BASE_SetScrnVSizeMain(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetScrnHSizeMain(unsigned int u4Plane, UINT32 u4Value);
/* 20h OSD1 Window Position Configuration Register */
extern INT32 _OSD_BASE_SetOsd3VStart(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd3HStart(unsigned int u4Plane, UINT32 u4Value);
/* 24h OSD2 Window Position Configuration Register  */
extern INT32 _OSD_BASE_SetOsd2VStart(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsd2HStart(unsigned int u4Plane, UINT32 u4Value);

/*  64 Main FMT Timing Configuration Register #2*/
extern INT32 _OSD_BASE_SetOhtMain(unsigned int u4Plane, UINT32 u4Value);

/*  70h */
extern INT32 _OSD_BASE_SetIntTGen(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetIOMonSel(unsigned int u4Plane, UINT32 u4Value);

/* A4h */
extern INT32 _OSD_BASE_SetAlphaSel(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetSRamType(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_SetOsdShareSram(unsigned int u4Plane, UINT32 u4Value);

extern INT32 _OSD_BASE_SetOsdChkSumEn(unsigned int u4Plane, UINT32 u4Value);
extern INT32 _OSD_BASE_GetOsd3ChkSum(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsdSc3ChkSum(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsd2ChkSum(unsigned int u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_BASE_GetOsdSc2ChkSum(unsigned int u4Plane, UINT32 *pu4Value);
extern void osd_base_set_start_update(unsigned int u4Plane, UINT32 u4Value);
extern int osd_base_get_start_update(unsigned int u4Plane, UINT32 *pu4Value);
extern void osd_pmx_set_window_update(unsigned int u4Plane, UINT32 u4Value);
extern int osd_pmx_get_window_update(unsigned int u4Plane, UINT32 *pu4Value);
/* ----plane register relative functions-------------*/
extern void _OSD_PLA_GET_REG_BASE(uintptr_t *osd_pln_base_reg);
extern INT32 _OSD_PLA_GetReg(UINT32 u4Plane, UINT32 *pOsdPlaneReg);
extern INT32 _OSD_PLA_SetReg(UINT32 u4Plane, const UINT32 *pOsdPlaneReg);
extern INT32 _OSD_PLA_UpdateHwReg(UINT32 u4Plane);
extern INT32 _OSD_PLA_Update(UINT32 u4Plane);
extern INT32  osd_pla_non_shadow_update(UINT32 u4Plane);
extern INT32 osd_plane_update_extend(UINT32 u4Plane);
extern INT32 osd_plane_update_sync_threshold(UINT32 u4Plane);
extern INT32 _OSD_PLA_SetUpdateStatus(UINT32 u4Plane, UINT32 u4Update);
extern INT32 _OSD_PLA_GetUpdateStatus(UINT32 u4Plane);
extern INT32 _OSD_PLA_SetReflip(UINT32 u4Plane, BOOL u4Update);
extern INT32 _OSD_PLA_GetReflip(UINT32 u4Plane);
/* OSD Plane registers */
extern INT32 _OSD_PLA_SetEnable(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetFakeHdr(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetPrngEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetAlphaZeroBlack(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetOutRngColorMode(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetHeaderAddr(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetBlending(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetFading(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetHFilter(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetColorExpSel(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetAlphaRatioEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetContReqLmt(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetFifoSize(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetPauseCnt(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetContReqLmt0(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetBurstDis(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetRgbMode(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetVacancyThr(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetPreMultiMode(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetHMirrorEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetVFlipEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetRgb2YcbrbEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetXVYCCEn(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetYCbCr709En(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetNonAlphaShift(UINT32 u4Plane, UINT32 u4Value);
/* DWORD - 064  OSD_PLA_064*/
extern INT32 _OSD_PLA_SetAlphaCoordinateV(UINT32 u4Plane, UINT32 u4Value);
extern INT32 _OSD_PLA_SetAlphaCoordinateH(UINT32 u4Plane, UINT32 u4Value);
/* DWORD - 0A0  OSD_PLA_0A0*/
extern INT32 _OSD_PLA_SetEableRGB2YCC(UINT32 u4Plane, UINT32 u4Value);
/* DWORD - 0A0  OSD_PLA_0AC*/
extern INT32 _OSD_PLA_SetBurst8_Enable(UINT32 u4Plane, UINT32 u4Value);
extern int osd_pla_set_alpha_detect_en(UINT32 u4Plane, UINT32 u4Value);
extern int osd_pla_set_alpha_detect_clear(UINT32 u4Plane, UINT32 u4Value);
extern int osd_pla_set_alpha_detect_threshold(UINT32 u4Plane, UINT32 u4Value);
extern int osd_pla_set_region_width(UINT32 u4Plane, UINT32 u4Value);
extern int osd_pla_get_alpha_region(UINT32 u4Plane, UINT32 *pu4Value);
extern int osd_pla_get_line_cnt(uint32_t layer_id);
extern int osd_pla_get_osd_en_effect(uint32_t layer_id);


/* DWORD - 0E4  OSD_PLA_0E4*/
extern INT32 _OSD_PLA_SetSyncThres(UINT32 u4Plane, UINT32 u4Value);
/* plane register get function */
extern INT32 _OSD_PLA_GetEnable(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetFakeHdr(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetPrngEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetAlphaZeroBlack(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetOutRngColorMode(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetHeaderAddr(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetBlending(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetFading(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetHFilter(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetColorExpSel(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetAlphaRatioEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetContReqLmt(UINT32 u4Plane, UINT32 *u4Value);
extern INT32 _OSD_PLA_GetFifoSize(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetPauseCnt(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetContReqLmt0(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetBurstDis(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetRgbMode(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetVacancyThr(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetPreMultiMode(UINT32 u4Plane, UINT32 *pu4Value);

extern INT32 _OSD_PLA_GetHMirrorEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetVFlipEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetRgb2YcbrbEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetXVYCCEn(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetYCbCr709En(UINT32 u4Plane, UINT32 *pu4Value);
extern INT32 _OSD_PLA_GetNonAlphaShift(UINT32 u4Plane, UINT32 *pu4Value);

/* DWORD - 064  OSD_PLA_064*/
extern INT32 _OSD_PLA_GetAlphaCoordinate(UINT32 u4Plane, UINT32 *pu4Value);
/* DWORD - 0A0  OSD_PLA_0A0*/
extern INT32 _OSD_PLA_GetEableRGB2YCC(UINT32 u4Plane, UINT32 *pu4Value);
/* DWORD - 0E4  OSD_PLA_0E4*/
extern INT32 _OSD_PLA_GetSyncThres(UINT32 u4Plane, UINT32 *pu4Value);

extern void osd_plane_set_extend_update(unsigned int u4Plane, UINT32 u4Value);
extern int osd_plane_get_extend_update(unsigned int u4Plane, UINT32 *pu4Value);
extern int osd_pla_set_sync_threshold(UINT32 u4Plane, UINT32 u4Value);
extern void osd_plane_set_sync_threshold_update(unsigned int u4Plane, UINT32 u4Value);
extern int osd_plane_get_sync_threshold_update(unsigned int u4Plane, UINT32 *pu4Value);
extern int osd_pla_set_auto_ultra(UINT32 u4Plane, UINT32 u4Value);

/* scaler register relative functions */
extern void _OSD_SC_GET_REG_BASE(uintptr_t *osd_scl_reg_base);
extern INT32 _OSD_SC_GetReg(UINT32 u4Scaler, UINT32 *pOsdScalerReg);
extern INT32 _OSD_SC_SetReg(UINT32 u4Scaler, const UINT32 *pOsdScalerReg);
extern INT32 _OSD_SC_UpdateHwReg(UINT32 u4Scaler);
extern INT32 _OSD_SC_Update(UINT32 u4Scaler);
extern INT32 _OSD_SC_SetUpdateStatus(UINT32 u4Plane, UINT32 u4Update);
extern INT32 _OSD_SC_GetUpdateStatus(UINT32 u4Plane);


extern INT32 _OSD_SC_SetVuscAlphaEdgeRcvr(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVuscAlphaEdgeElt(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVdscAlphaEdgeRcvr(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVdscAlphaEdgeElt(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHuscAlphaEdgeRcvr(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHuscAlphaEdgeElt(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHdscAlphaEdgeRcvr(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHdscAlphaEdgeElt(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVuscEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVdscEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHuscEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHdscEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetScLpfEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetScEn(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetSrcVSize(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetSrcHSize(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetDstVSize(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetDstHSize(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVscHSize(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHdscStep(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHdscOfst(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHuscStep(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetHuscOfst(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVscOfstTop(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVscOfstBot(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetVscStep(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetScLpfC3(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetScLpfC4(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetScLpfC5(UINT32 u4Scaler, UINT32 u4Value);
extern INT32 _OSD_SC_SetSlackEn(UINT32 u4Scaler, UINT32 u4SlackEn);

extern INT32 _OSD_SC_GetFormat(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetSlackEn(UINT32 u4SlackEn, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVuscEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVdscEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHuscEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHdscEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScEn(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetSrcVSize(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetSrcHSize(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetDstVSize(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetDstHSize(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVscHSize(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHdscStep(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHdscOfst(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHuscStep(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetHuscOfst(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVscOfstTop(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVscOfstBot(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetVscStep(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfC1(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfC2(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfC3(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfC4(UINT32 u4Scaler, UINT32 *pu4Value);
extern INT32 _OSD_SC_GetScLpfC5(UINT32 u4Scaler, UINT32 *pu4Value);

extern int _osd_region_init(void);
extern void _osd_region_uninit(void);
extern INT32 _osd_region_reset(UINT32 u4Region);

extern INT32 _OSD_RGN_SetNextRegion(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetFifoEx(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetNextEnable(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetColorMode(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetDataAddr(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetAlpha(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetHClip(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetVClip(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetLineSize8(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetVbSel(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetUgSel(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetYrSel(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetASel(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetInputWidth(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetInputHeight(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetLineSize(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetHStep(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetVStep(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetOutputHeight(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetOutputPosY(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetOutputWidth(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetOutputPosX(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetDataAddrHI(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetColorKeyEnable(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetColorKey(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetFrameMode(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetAutoMode(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetTopField(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetBlendMode(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetSelectByteEn(UINT32 u4Region, uint32_t index, UINT32 u4Value);
extern INT32 _OSD_RGN_SetMixSel(UINT32 u4Region, uint32_t index, UINT32 u4Value);


extern INT32 _OSD_RGN_GetNextRegion(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetFifoEx(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetNextEnable(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetColorMode(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetDataAddr(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetAlpha(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetHClip(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetVClip(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetLineSize8(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetVbSel(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetUgSel(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetYrSel(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetASel(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetInputWidth(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetInputHeight(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetLineSize(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetHStep(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetVStep(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetOutputHeight(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetOutputPosY(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetOutputWidth(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetOutputPosX(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetColorKeyEnable(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetColorKey(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetFrameMode(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetAutoMode(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetTopField(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetBlendMode(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);
extern INT32 _OSD_RGN_GetSelectByteEn(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);

extern OSD_RGN_UNION_T *OSD_RGN_GetAdd(UINT32 u4Region);
extern INT32 _OSD_RGN_GetAddress(UINT32 u4Region, uint32_t index, UINT32 *pu4Addr);
extern INT32 _OSD_RGN_GetColorMode(UINT32 u4Region, uint32_t index, UINT32 *pu4Value);

extern void _Osd_PustReset_Plane(UINT32 u4Plane);
extern void _Osd_ReleaseReset_Plane(UINT32 u4Plane);

extern void osd_pmx_set_rg_htotal(uint32_t value);
extern void osd_pmx_set_rg_vtotal(uint32_t value);
extern void osd_pmx_set_rg_win_hend(uint32_t value);
extern void osd_pmx_set_rg_win_hst(uint32_t value);
extern void osd_pmx_set_rg_win_vend(uint32_t value);
extern void osd_pmx_set_rg_win_vst(uint32_t value);
extern void osd_pmx_set_rg_win1_hend(uint32_t value);
extern void osd_pmx_set_rg_win1_hst(uint32_t value);
extern void osd_pmx_set_rg_win1_vend(uint32_t value);
extern void osd_pmx_set_rg_win1_vst(uint32_t value);
extern void osd_pmx_set_rg_win2_hend(uint32_t value);
extern void osd_pmx_set_rg_win2_hst(uint32_t value);
extern void osd_pmx_set_rg_win2_vend(uint32_t value);
extern void osd_pmx_set_rg_win2_vst(uint32_t value);
extern void osd_pmx_set_rg_pln2_max_a_sel(uint32_t value);
extern void osd_pmx_set_rg_pln1_max_a_sel(uint32_t value);
extern void osd_pmx_set_rg_pln0_max_a_sel(uint32_t value);
extern void osd_pmx_set_rg_vs_pol_sel(uint32_t value);
extern void osd_pmx_set_shadow_update(uint32_t value);
extern void osd_pmx_set_shadow_en(uint32_t value);
extern void osd_pmx_set_rg_win2_en(uint32_t value);
extern void osd_pmx_set_rg_win1_en(uint32_t value);
extern void osd_pmx_set_rg_win0_en(uint32_t value);
extern void osd_pmx_set_rg_alpha_val(uint32_t value);
extern void osd_pmx_set_rg_dbg_sel(uint32_t value);
extern void osd_pmx_set_rg_max_a_sel(uint32_t value);
extern void osd_pmx_set_rg_max_a_sel_div(uint32_t value);
extern void osd_pmx_set_rg_premul(uint32_t value);
extern void osd_pmx_set_plane2_cr_sel(uint32_t value);
extern void osd_pmx_set_plane2_cb_sel(uint32_t value);
extern void osd_pmx_set_plane2_y_sel(uint32_t value);
extern void osd_pmx_set_swap_2(uint32_t value);
extern void osd_pmx_set_plane1_cr_sel(uint32_t value);
extern void osd_pmx_set_plane1_cb_sel(uint32_t value);
extern void osd_pmx_set_plane1_y_sel(uint32_t value);
extern void osd_pmx_set_swap_1(uint32_t value);
extern void osd_pmx_set_plane0_cr_sel(uint32_t value);
extern void osd_pmx_set_plane0_cb_sel(uint32_t value);
extern void osd_pmx_set_plane0_y_sel(uint32_t value);
extern void osd_pmx_set_swap_0(uint32_t value);
extern void osd_pmx_set_mix1_premul_set_rgb(uint32_t value);
extern void osd_pmx_set_mix1_sign_en(uint32_t value);
extern void osd_pmx_set_mix1_bypass(uint32_t value);
extern void osd_pmx_set_mix1_bot_mask(uint32_t value);
extern void osd_pmx_set_mix1_top_mask(uint32_t value);
extern void osd_pmx_set_mix1_top_mix_bg(uint32_t value);
extern void osd_pmx_set_mix1_pre_mul_a(uint32_t value);
extern void osd_pmx_set_mix2_premul_set_rgb(uint32_t value);
extern void osd_pmx_set_mix2_sign_en(uint32_t value);
extern void osd_pmx_set_mix2_bypass(uint32_t value);
extern void osd_pmx_set_mix2_bot_mask(uint32_t value);
extern void osd_pmx_set_mix2_mix1_top_mask(uint32_t value);
extern void osd_pmx_set_mix2_top_mix_bg(uint32_t value);
extern void osd_pmx_set_mix2_pre_mul_a(uint32_t value);
extern void osd_pmx_set_mix2_de_sel(uint32_t value);
extern void osd_pmx_set_mix1_de_sel(uint32_t value);
extern void osd_pmx_set_pln0_msk(uint32_t value);
extern void osd_pmx_set_vdo4_alpha(uint32_t value);
#endif
