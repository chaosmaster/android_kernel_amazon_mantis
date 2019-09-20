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


#ifndef _P2I_HW_H_
#define _P2I_HW_H_

#include "disp_reg.h"

/* ********************************************************************* */
/* VDOUT System & Clock Macros */
/* ********************************************************************* */

#define P2I_CTRL	0x40
#define P2I_CST_ON			(1 << 0)
#define OP_MODE				(3 << 1)
#define TV_FLD				(1 << 3)
#define TVP_FLD_INV			(1 << 4)
#define ADJ_TOTAL_EN		(1 << 5)
#define P2I_INV_LINE_EVEN	(1 << 8)
#define LINE_SHIFT			(1 << 9)
#define AUTO_R_NOEQ_W_EN	(1 << 11)
#define CI_SEL				(1 << 16)
#define CI_ROUND_EN			(1 << 17)
#define CI_REPEAT_EN		(1 << 18)
#define CI_VRF_OFF			(1 << 19)
#define CI_REPEAT_SEL		(1 << 20)
#define HRF_MODE			(1 << 22)
#define VRF_MODE			(1 << 23)
#define CI_Y_OFFSET			(1 << 24)
#define CI_C_OFFSET			(1 << 26)
#define CI_VRF_OPT			(1 << 28)
#define CI_HRF_OPT			(1 << 29)


#define P2I_H_TIME0 0x44
#define P2I_H_TIME1 0x48
#define P2I_H_TIME2 0x4c
#define P2I_V_TIME0 0x50
#define P2I_V_TIME1 0x54
#define P2I_V_TIME2 0x58
#define CI_H_TIME0	0x5c
#define CI_H_TIME1	0x60
#define CI_H_TIME2	0x64
#define CI_V_TIME0	0x68
#define CI_V_TIME1	0x6c
#define VH_TOTAL	0x70
#define P2I_CTRL_2  0x74
#define LINE_SHIFT_MSK	0x03


#endif
