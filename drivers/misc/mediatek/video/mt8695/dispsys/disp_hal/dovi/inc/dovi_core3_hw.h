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


#ifndef _DOVI_CORE3_HW_H_
#define _DOVI_CORE3_HW_H_

#include "dovi_common_hw.h"

#define CORE3_REG_BASE	   (0x00)

#define CORE3_RAP_REG_BASE (CORE3_REG_BASE + 0x400)	/* (DOVI_CORE3_CTRL_BASE) */
#define CORE3_CFG_REG_BASE (CORE3_REG_BASE + 0x000)	/* (DOVI_CORE3_BASE) */
#define CORE3_DPM_REG_BASE (CORE3_REG_BASE + 0x000)	/* (DOVI_CORE3_BASE) */

/* 0x00 ~ 0x2C */
#define CORE3_RAP_REG_NUM    (0x100/4)
/* 0x00 ~ 0x14 */
#define CORE3_CFG_REG_NUM    (0x18/4)
/* 0x00 ~ 0x3FC */
#define CORE3_DPM_REG_NUM    (0x400/4)

#define CORE3_PROG_START  (CORE3_CFG_REG_BASE + 0x08)
#define CORE3_PROG_FINISH (CORE3_CFG_REG_BASE + 0x0C)

#define CORE3_EFUSE_REG      (CORE3_REG_BASE+0x004)

#define CORE3_RESET_ALL       0xF
#define CORE3_RESET_CLEAR     0x0

/* Base address 0x1FC00 */
struct core3_cfg_reg_t {

	/* DWORD - 000 */
	UINT32 Min_Ver_Num:8;
	UINT32 Mar_Ver_Num:8;
	UINT32:16;

	/* DWORD - 004 */
	UINT32 Out_mode:3;
	UINT32 Efuse_disable:1;
	UINT32:28;

	/* DWORD - 008 */
	UINT32 Md_Prog_Str:1;
	UINT32:31;

	/* DWORD - 00C */
	UINT32 Md_Prog_Fin:1;
	UINT32:31;

	/* DWORD - 010 */
	UINT32 Md_Prog_Err:1;
	UINT32 Unmat_F_Dec_Err:1;
	UINT32 Md_Copy_Fin:1;
	UINT32:29;

	/* DWORD - 014 */
	UINT32 Md_Prog_Err_En:1;
	UINT32 Unmat_F_Dec_Err_En:1;
	UINT32 Md_Copy_Fin_En:1;
	UINT32:29;
};


/* Base address 0x1FB00 */
struct core3_rap_reg_t {

	/* DWORD - 000 */
	UINT32 SW_Rst:4;
	UINT32:28;

	/* DWORD - 004 */
	 UINT32:32;

	/* DWORD - 008 */
	 UINT32:4;
	UINT32 Hsync_Sel_Ext:1;
	UINT32 Vsync_Set_Ext:1;
	UINT32 De_Sel_Ext:1;
	UINT32:25;

	/* DWORD - 00C */
	 UINT32:32;

	/* DWORD - 010 */
	 UINT32:32;

	/* DWORD - 014 */
	 UINT32:32;

	/* DWORD - 018 */
	 UINT32:32;

	/* DWORD - 01C */
	UINT32 Hsync_De_Swap:1;
	UINT32 Vsync_Sel_Vde:1;
	UINT32 Post_Pol_N:1;
	UINT32:1;
	UINT32 Hsync_Pol_N:1;
	UINT32 Vsync_Pol_N:1;
	UINT32 De_Pol_N:1;
	UINT32:25;

	/* DWORD - 020 */
	UINT32 Hsync_Width:12;
	UINT32:4;
	UINT32 Vsync_Width:8;
	UINT32:8;

	/* DWORD - 024  */
	UINT32 X_Active_Start:13;
	UINT32:3;
	UINT32 X_Active_End:13;
	UINT32:3;

	/* DWORD - 028 */
	UINT32 Y_Active_Start:12;
	UINT32:4;
	UINT32 Y_Active_End:12;
	UINT32:4;

	/* DWORD - 02C  */
	UINT32 Hsync_Delay:13;
	UINT32:3;
	UINT32 Vsync_Delay:13;
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
	UINT32:32;

	/* DWORD - 06C  */
	UINT32:31;
	UINT32 fgOut_Fix_Pattern:1;

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

	/* DWORD - 0AC  */
	UINT32:32;
};

struct core3_dpm_reg_t {
	uint32_t core3_dpm_reg[CORE3_DPM_REG_NUM];
};

union core3_rap_reg_u {
	UINT32 reg[CORE3_RAP_REG_NUM];
	struct core3_rap_reg_t fld;
};

union core3_cfg_reg_u {
	UINT32 reg[CORE3_CFG_REG_NUM];
	struct core3_cfg_reg_t fld;
};

union core3_dpm_reg_u {
	UINT32 reg[CORE3_DPM_REG_NUM];
	struct core3_dpm_reg_t fld;
};

#endif				/*  */
