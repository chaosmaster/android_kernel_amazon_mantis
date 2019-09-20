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



#ifndef _DISP_SYS_HW_H_
#define _DISP_SYS_HW_H_

#include <linux/types.h>

#define MMSYS_MEM_PD_SW0	0x58
	#define MMSYS_MEM_PD_DRAM2AXI	(0x3 << 0)
	#define MMSYS_MEM_PD_SDR_LUTA	(0x1 << 2)
	#define MMSYS_MEM_PD_SDR_LUTB	(0x1 << 3)
	#define MMSYS_MEM_PD_VM			(0x3 << 4)
	#define MMSYS_MEM_PD_OSD_FHD	(0x1ff << 6)
	#define MMSYS_MEM_PD_OSD_UHD	(0x7fff << 15)
	#define MMSYS_MEM_PD_OSD_TVE	(0x1 << 30)
	#define MMSYS_MEM_PD_VIDEOIN_0	(0x1 << 31)
#define MMSYS_MEM_PD_SW1	0x5c
	#define MMSYS_MEM_PD_VIDEOIN_1	(0x3 << 0)
	#define MMSYS_MEM_PD_HDMI		(0x1ffff << 2)
	#define MMSYS_MEM_PD_P2I		(0x7 << 19)
	#define MMSYS_MEM_PD_DOLBY0		(0x3ff << 22)
#define MMSYS_MEM_PD_SW2	0x60
#define MMSYS_MEM_PD_SW3	0x64
#define MMSYS_CLOCK_SET0	0x104
#define MMSYS_CLOCK_VDOUT	0x884

#define DISP_INT_CLR 0x0000
#define DISP_INT_STATUS 0x0004

#define DISP_MAIN_CONFIG	0x0014
	#define DISP_OUT_SEL_MASK	(0x3 << 16)
	#define DISP_TOP_OUT_SWAP	(0x1 << 19)
	#define DISP_TOP_Y_PRE_ULTRA	(0x1 << 28)
	#define DISP_TOP_C_PRE_ULTRA	(0x1 << 29)
	#define DISP_TOP_Y_ULTRA	(0x1 << 30)
	#define DISP_TOP_C_ULTRA	(0x1 << 31)


#define DISP_SUB_CONFIG		0x0018

#define DISP_HDR2SDR_DCM_CTRL 0x0024

#define DISP_SYS_CONFIG_30	0x30
	#define DISP_SYS_PD_MVDO_HDR_LUTA	(0x1 << 0)
	#define DISP_SYS_PD_MVDO_HDR_LUTB	(0x1 << 1)
	#define DISP_SYS_PD_DISPFMT3	(0x7f << 2)
	#define DISP_SYS_PD_MVDO_0		(0x7fffff << 9)
#define DISP_SYS_CONFIG_34	0x34
#define DISP_SYS_CONFIG_38	0x38
#define DISP_SYS_CONFIG_3c	0x3c
#define DISP_SYS_CONFIG_40	0x40
#define DISP_SYS_CONFIG_44	0x44
	#define DISP_SYS_PD_SVDO_1		(0x3ffffff << 0)
	#define DISP_SYS_PD_DISPFMT4	(0xf << 26)
	#define DISP_SYS_PD_SVDO_HDR_LUTA	(0x1 << 30)
	#define DISP_SYS_PD_SVDO_HDR_LUTB	(0x1 << 31)
#define DISP_SYS_CONFIG_48	0x48
	#define DISP_SYS_PD_IMGRZ		(0x1 << 21)
	#define DISP_SYS_PD_WC			(0x3 << 22)
	#define DISP_SYS_PD_NR_DL		(0x3 << 24)
	#define DISP_SYS_PD_NR_COMMON	(0x1f << 26)
	#define DISP_SYS_PD_VDO2_0		(0x1 << 31)
#define DISP_SYS_CONFIG_4c	0x4c
	#define DISP_SYS_PD_VDO2_1		(0xff << 0)

#define DISP_SYS_CONFIG_A0		0x00a0
	#define DISP_OUT_SHADOW_EN		(0x1 << 0)
	#define DISP_OUT_SHADOW_UPDATE	(0x1 << 1)

#endif




