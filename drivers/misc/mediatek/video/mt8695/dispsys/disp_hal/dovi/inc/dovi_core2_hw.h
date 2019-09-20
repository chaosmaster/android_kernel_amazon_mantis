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


#ifndef _DOVI_CORE2_HW_H
#define _DOVI_CORE2_HW_H

/* ----------------------------------------------------------------------------- */
/* Include files */
/* ----------------------------------------------------------------------------- */

#include "dovi_common_hw.h"

#define CORE2_REG_BASE                 (0x00)

#define CORE2_IG_RAP_REG_BASE (CORE2_REG_BASE + 0x400)
#define CORE2_IG_CFG_REG_BASE (CORE2_REG_BASE + 0x000)
#define CORE2_IG_DPM_REG_BASE (CORE2_REG_BASE + 0x018)


#define CORE2_CFG_REG_BASE			CORE2_IG_CFG_REG_BASE
#define CORE2_DPM_REG_BASE	        CORE2_IG_DPM_REG_BASE
#define CORE2_RAP_REG_BASE	        CORE2_IG_RAP_REG_BASE

#define CORE2_PROG_START      (CORE2_CFG_REG_BASE+0x08)
#define CORE2_PROG_FINISH     (CORE2_CFG_REG_BASE+0x0C)

/* 0x00 ~ 0x14 (0x18 - 0x00 = 0x18)*/
#define CORE2_CFG_REG_NUM		(0x18/4)

/* 0x18 ~ 0x78 (0x78 - 0x18 = 0x60) */
#define CORE2_DPM_REG_NUM	    (0x60/4)

/* 0xC00 ~ 0xC3C (0xC40 - 0x0 = 0x40) */
#define CORE2_RAP_REG_NUM	    (0x100/4)

#define CORE2_EFUSE_REG (CORE2_REG_BASE+0x004)

#define CORE2_RESET_ALL       0xA
#define CORE2_RESET_CLEAR     0x0

#define CORE2_DISP_WITH_INTERNAL_TIMING 1

struct core2_cfg_reg_t {

	/* DWORD - 000 */
	UINT32 u4Min_Ver_Num:8;
	UINT32 u4Mar_Ver_Num:8;
	UINT32:16;

	/* DWORD - 004 */
	UINT32 fgBypassCVM:1;
	UINT32 fgYUV2RGB:1;
	UINT32 fg422to444:1;
	UINT32 fgEfuse:1;
	UINT32:28;

	/* DWORD - 008 */
	UINT32 fgMetaStart:1;
	UINT32:31;

	/* DWORD - 00C */
	UINT32 fgMetaFinish:1;
	UINT32:31;

	/* DWORD - 010 */
	 UINT32:1;
	UINT32 fgFrameErr:1;
	UINT32 fgMetaCPFinish:1;
	UINT32 fgToneCuverErr:1;
	UINT32:28;

	/* DWORD - 014 */
	UINT32 fgMetaErrEnable:1;
	UINT32 fgFrameErrEnable:1;
	UINT32 fgMetaCPFinishEnanle:1;
	UINT32 fgToneCuverErrEnable:1;
	UINT32:28;
};

/* Base address 1f700 */
struct core2_rap_reg_t {

	/* DWORD - 000 */
	UINT32 u4SW_Rst:4;
	UINT32:28;

	/* DWORD - 004 */
	UINT32 fgDmaTrigger:1;
	UINT32 fgDmaVsTrigger:1;
	UINT32:30;

	/* DWORD - 008 */
	UINT32:1;
	UINT32 fgDmaVsPol:1;
	UINT32 fgEnDma:1;
	UINT32 fgEnDmaDRAM:1;
	UINT32 fgHsync_Sel_Ext:1;
	UINT32 fgVsync_Set_Ext:1;
	UINT32 fgDe_Sel_Ext:1;
	UINT32:25;

	/* DWORD - 00C */
	UINT32:32;

	/* DWORD - 010 */
	UINT32:4;
	UINT32 u4DmaAddr:28;

	/* DWORD - 014 */
	UINT32:4;
	UINT32 u4DmaAddrNum:11;
	UINT32:17;

	/* DWORD - 018 */
	UINT32 u4DmaReadThrd:7;
	UINT32:1;
	UINT32 u4DmaWriteThrd:7;
	UINT32:17;

	/* DWORD - 01C */
	UINT32 fgHsync_De_Swap:1;
	UINT32 fgVsync_Sel_Vde:1;
	UINT32 fgPost_Pol_N:1;
	UINT32:1;
	UINT32 fgHsync_Pol_N:1;
	UINT32 fgVsync_Pol_N:1;
	UINT32 fgDe_Pol_N:1;
	UINT32:25;

	/* DWORD - 020 */
	UINT32 u4Hsync_Width:12;
	UINT32:4;
	UINT32 u4Vsync_Width:8;
	UINT32:8;

	/* DWORD - 024  */
	UINT32 u4X_Active_Start:13;
	UINT32:3;
	UINT32 u4X_Active_End:13;
	UINT32:3;

	/* DWORD - 028 */
	UINT32 u4Y_Active_Start:12;
	UINT32:4;
	UINT32 u4Y_Active_End:12;
	UINT32:4;

	/* DWORD - 02C  */
	UINT32 u4Hsync_Delay:13;
	UINT32:3;
	UINT32 u4Vsync_Delay:13;
	UINT32:3;

	/* DWORD - 030  */
	 UINT32:32;

	/* DWORD - 034  */
	UINT32:32;

	/* DWORD - 038  */
	UINT32:32;

	/* DWORD - 03C  */
	UINT32:32;

	/* DWORD - 040  */
	UINT32:32;

	/* DWORD - 044  */
	UINT32:32;

	/* DWORD - 048  */
	UINT32 u4InputRGB:6;
	UINT32:26;

	/* DWORD - 04C  */
	UINT32 u4Tg_New_H_Polar:1;
	UINT32 u4Tg_New_V_Polar:1;
	UINT32 u4Tg_New_De_Polar:1;
	UINT32 u4Tg_New_Hsync_De_Swap:1;
	UINT32 u4Tg_New_Use_Hde:1;
	UINT32:3;
	UINT32 u4Use_Tg_New:1;
	UINT32 u4Use_Tg_New_Pattern:1;
	UINT32 u4Use_Tg_Sw_Trig:1;
	UINT32:17;
	UINT32 u4Use_Tg_New_Trig_Sel:1;
	UINT32:1;
	UINT32 u4Use_Tg_New_Off:1;
	UINT32:1;

	/* DWORD - 050  */
	UINT32 u4Tg_New_H_Total:13;
	UINT32:3;
	UINT32 u4Tg_New_V_Total:13;
	UINT32:3;

	/* DWORD - 054  */
	UINT32 u4Tg_New_H_Active:13;
	UINT32:3;
	UINT32 u4Tg_New_V_Active:13;
	UINT32:3;

	/* DWORD - 058  */
	UINT32 u4Tg_New_H_Width:13;
	UINT32:3;
	UINT32 u4Tg_New_V_Width:13;
	UINT32:3;

	/* DWORD - 05C  */
	UINT32 u4Tg_New_H_Front:13;
	UINT32:3;
	UINT32 u4Tg_New_V_Front:13;
	UINT32:3;

	/* DWORD - 060  */
	UINT32:32;

	/* DWORD - 064  */
	UINT32:32;

	/* DWORD - 068  */
	UINT32 u4In_Fix_Y:8;
	UINT32 u4In_Fix_C:8;
	UINT32 u4In_Fix_V:8;
	UINT32 u4In_Fix_A:6;
	UINT32 fgIn_Fix_Pattern_A:1;
	UINT32 fgIn_Fix_Pattern:1;

	/* DWORD - 06C  */
	UINT32:32;

	/* DWORD - 070  */
	UINT32:32;

	/* DWORD - 074  */
	UINT32:32;

	/* DWORD - 078  */
	UINT32:32;

	/* DWORD - 07C  */
	UINT32:32;

	/* DWORD - 080  */
	UINT32:32;

	/* DWORD - 084  */
	UINT32:32;

	/* DWORD - 088  */
	UINT32:32;

	/* DWORD - 08C  */
	UINT32:32;

	/* DWORD - 090  */
	UINT32:32;

	/* DWORD - 094  */
	UINT32:32;

	/* DWORD - 098  */
	UINT32:32;

	/* DWORD - 09C  */
	UINT32:32;

	/* DWORD - 0A0  */
	UINT32:32;

	/* DWORD - 0A4  */
	UINT32:32;

	/* DWORD - 0A8  */
	UINT32:32;

	/* DWORD - 03C  */
	UINT32:32;
};

struct core2_dpm_reg_t {
	uint32_t core2_dpm_reg[CORE2_DPM_REG_NUM];
};

union core2_cfg_reg_u {
	UINT32 reg[CORE2_CFG_REG_NUM];
	struct core2_cfg_reg_t fld;
};

union core2_dpm_reg_u {
	UINT32 reg[CORE2_DPM_REG_NUM];
	struct core2_dpm_reg_t fld;
};

union core2_rap_reg_u {
	UINT32 reg[CORE2_RAP_REG_NUM];
	struct core2_rap_reg_t fld;
};


#endif				/* _DOVI_CORE2_HW_H */
