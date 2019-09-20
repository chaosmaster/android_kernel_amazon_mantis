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


#ifndef _VDOUT_HW_H_
#define _VDOUT_HW_H_

#include <linux/types.h>

/********************************
*vdout sys 1
********************************/

#define RW_VDOUT_SYS	0x04
	#define CVBS_SEL			(0x3 << 0)
		#define CVBS_SEL_MAIN		(0x0 << 0)
		#define CVBS_SEL_SCALE		(0x1 << 0)
		#define CVBS_SEL_P2I		(0x2 << 0)
		#define CVBS_SEL_MEG		(0x3 << 0)
	#define CAV_SEL				(0x3 << 2)
		#define CAV_SEL_MAIN		(0x0 << 2)
		#define CAV_SEL_SCALE		(0x1 << 2)
		#define CAV_SEL_P2I			(0x2 << 2)
	#define HDMI_SEL			(0x3 << 4)
		#define HDMI_SEL_MAIN		(0x0 << 4)
		#define HDMI_SEL_SCALE		(0x1 << 4)
		#define HDMI_SEL_P2I		(0x2 << 4)
	#define TVE_ROUND_EN		(0x1 << 6)
	#define RGB_ROUND_EN		(0x1 << 7)
	#define DGO_SEL				(0x1 << 8)
	#define HDMI_MAST_EN		(0x1 << 10)
	#define P2I_CST_SEL			(0x1 << 11)
		#define P2I_FROM_MAIN		(0x0 << 11)
		#define P2I_FROM_SCL		(0x1 << 11)
	#define C_SCL_UI1_SEL		(0x1 << 12)
	#define C_SCL_UI2_SEL		(0x1 << 15)
	#define CVBS_OSD5_SEL		(0x1 << 18)
	#define CAV_OSD5_SEL		(0x1 << 19)
	#define HDMI_OSD5_SEL		(0x1 << 20)
	#define DGO_OSD5_SEL		(0x1 << 21)
	#define OSD5_SEL			(0x1 << 22)
		#define OSD5_SEL_CVBS		(0x0 << 22)
		#define OSD5_SEL_CAV		(0x1 << 22)
		#define OSD5_SEL_HDMI		(0x2 << 22)
		#define OSD5_SEL_DGO		(0x3 << 22)
		#define OSD5_SEL_MASK		(0x3 << 22)
	#define DGO_SEL2			(0x1 << 24)
	#define C_HDMI_CLK54		(0x1 << 25)
	#define HIS_DIV_32			(0x1 << 26)
	#define C_VIU_SEL			(0x1 << 27)
	#define TWO_DEC				(0x1 << 28)
	#define DEC_SEL				(0x1 << 29)
	#define VDO_SEL				(0x1 << 30)


#define RW_VDOUT_CLK	0x08
	#define MAIN_CLK_OFF		(0x1 << 0)
	#define CVBS_CLK_OFF		(0x1 << 2)
	#define CAV_CLK_OFF			(0x1 << 3)
	#define HDMI_CLK_OFF		(0x1 << 4)
	#define CAV27_CLK_OFF		(0x1 << 6)
	#define SCL_CLK_OFF			(0x1 << 7)
	#define CVBS_AUX			(0x1 << 8)
		#define CVBS_AUX_MAIN		(0x0 << 8)
		#define CVBS_AUX_SCALE		(0x1 << 8)
	#define CAV_AUX				(0x1 << 9)
		#define CAV_AUX_MAIN		(0x0 << 9)
		#define CAV_AUX_SCALE		(0x1 << 9)
	#define HDMI_AUX			(0x1 << 10)
		#define HDMI_AUX_MAIN		(0x0 << 10)
		#define HDMI_AUX_SCALE		(0x1 << 10)
		#define HDMI_PRGS27M		(0x1 << 11)
	#define VDAC_148			(0x1 << 12)
	#define VDAC_108			(0x1 << 13)
	#define DIV_SEL				(0x1 << 14)
	#define SELF_OPT_HDMI		(0x1 << 15)
	#define VDOUT_CLK_SELF_OPTION  (1<<16)
	#define VDOUT_CAV_HD		(0x1 << 17)
	#define VDOUT_CAV_PRGS		(0x1 << 18)
	#define VDOUT_CAV_1080P		(0x1 << 19)
	#define VDOUT_CLK_HDMI_HD	(0x1 << 20)
	#define VDOUT_CLK_HDMI_PRGS	(0x1 << 21)
	#define VDOUT_CLK_HDMI_1080P	(0x1 << 22)
	#define HD_2FS_148			(0x1 << 23)
	#define VDOUT_CLK_SCL_HD	(0x1 << 24)
	#define VDOUT_CLK_SCL_PRGS	(0x1 << 25)
	#define VDOUT_CLK_SCL_1080P	(0x1 << 26)
	#define SCL_PRGS27M			(0x1 << 27)
	#define SCL_HCKSEL			(0x1 << 28)
	#define SCL_VCKSEL			(0x1 << 29)
	#define HD_HALF				(0x1 << 30)
	#define SCL_HALF			(unsigned int)((unsigned int)0x1 << 31)

#define RW_VDOUT_CLK2	0x0C

	#define HDMI_HDAUD_CLK		(0x01 << 9)
	#define OSD5_P2I_CST		(0x01 << 11)
	#define P2I_AUX				(0x01 << 12)
		#define P2I_AUX_MAIN		(0x00 << 12)
		#define P2I_AUX_SCALE		(0x01 << 12)
	#define SELF_OPTION_P2I		(0x01 << 13)
	#define P2I_HD				(0x01 << 14)
	#define P2I_1080P			(0x01 << 15)
	#define OSD5_AUX_MAIN_VIDEO	(0x00 << 28)
		#define OSD5_AUX_SCALER		(0x01 << 28)
		#define OSD5_AUX_480P2I		(0x02 << 28)
		#define OSD5_AUX_CST		(0x03 << 28)
		#define OSD5_AUX_MASK		(0x03 << 28)
	#define SELF_OPTION_OSD		(0x01 << 30)

#define VSYNC_PULSE_VDO1_OUT	0x20
#define VSYNC_PULSE_VDO2_OUT	0x24

#define RW_VDOUT_CLK3	0x2C
	#define CVBS_CLK_SEL_INTERLACE	(1 << 19)

#define VSYNC_PULSE_DOLBY1		0x30
#define VSYNC_PULSE_DOLBY2		0x34
#define VSYNC_PULSE_DOLBY3		0x38
#define OSD5BG			0x44
	#define OSD5_RGB_BCKGND_EN	(0x01 << 24)
	#define OSD5_DOT_CLK_INV	(0x01 << 25)
	#define SDPPF_SRC_SEL		(0x03 << 30)
		#define SDPPF_SRC_SEL_MIX	(0x0 << 30)
		#define SDPPF_SRC_SEL_DOLBY3	(0x2 << 30)

#define VSYNC_PULSE_OSD_UHD		0x74
#define VDTCLK_CFG3 0x78 /* 0x1578 */
	#define VIDEOIN_NEW_SD_SEL	(0x01 << 31)/* set this bit to 1 when use vdoin write 480p output*/
	#define MAST_296M_EN		(0x01 << 25)/*is used to set video clock to 296MHz for mast clock*/
	#define DIG_296M_EN		(0x01 << 24)/*is used to set video clock to 296MHz for DGI*/
	#define DGO_296M_EN		(0x01 << 23)/*is used to set video clock to 296MHz for DGO*/
	#define P2I_296M_EN		(0x01 << 22)/*is used to set video clock to 296MHz for HP2I*/
	#define NR_296M_EN			(0x01 << 21)/*is used to set video clock to 296MHz for NR*/
	#define RGB2HDMI_296M_EN	(0x01 << 19)/*is used to set video clock to 296MHz for HDMI*/
	#define VDO4_296M_EN		(0x01 << 17)/*is used to set video clock to 296MHz for VDO4*/
	#define FMT_296M_EN		(0x01 << 16)/*is used to set video clock to 296MHz for FMT*/

#define VSYNC_PULSE_OSD_FHD		0x7C
#define VSYNC_PULSE_VDO1	0x80
#define VSYNC_PULSE_VDO2	0x84
#define VSYNC_PULSE_VM			0x88

#define HDMI_CONFIG0_SUB 0x98
	#define VIDEO_IN_SRC_SEL	(0xf << 0)
	#define VM_BYPASS			(0x1 << 4)

#define HDMI_CONFIG_SUB 0x9c
	#define VM_SEL					(0x3 << 11)
		#define VM_SEL_FMT		(0x0 << 11)
		#define VM_SEL_OSD5		(0x1 << 11)
		#define VM_SEL_P2I		(0x2 << 11)
	#define SELF_OPT_HDMI_SUB		(0x1 << 18)
	#define VDOUT_CLK_HDMI_HD_SUB	(0x1 << 19)
	#define VDOUT_CLK_HDMI_1080P_SUB	(0x1 << 21)
	#define VDOUT_CLK_HDMI_PRGS_SUB (0x1 << 22)
	#define RGB2HDMI_296M_EN_SUB	(0x1 << 23)
	#define HDMI_PRGS27M_SUB		(0x1 << 24)
	#define HDMI_HDAUD_CLK_SUB		(0x1 << 25)
	#define HDMI_422_TO_420			(0x1 << 27)

#define SYS_CFG_B0	0xB0
	#define VSYNC_PULSE_SHADOW_SYNC_FMT	(0x1 << 0)
	#define VYSNC_PULSE_SHADOW_EN		(0x1 << 1)
	#define MUX_SHADOW_SYNC_FMT			(0x1 << 2)
	#define MUX_SHADOW_EN				(0x1 << 3)

#define SYS_CFG_CC	0xCC
	#define VM_SEL_DOLBY3		(0x1 << 7)

/********************************
*vdout sys 2
********************************/
#define VDTCLK_CONFIG4 0x30
	#define RGB2HDMI_594M_EN		(0x01 << 7)
	#define VDO3_594M_EN			(0x01 << 10)
	#define VDO3_296M_EN			(0x01 << 11)
	#define VDO3_150HZ_EN			(0x01 << 12)
	#define DISPFMT3_OFF			(0x01 << 13)
	#define FMT_594M_EN				(0x01 << 16)
	#define VDO4_594M_EN			(0x01 << 17)

#define	SYS_2_CFG_34 0x34
	#define SDR2HDR_BYPASS			(0x1 << 0)
	#define P2I_SRC_SEL				(0x1 << 1)
	#define DOLBY_MIX_ON			(0x1 << 2)
	#define VIDEO_IN_DOLBY_CONFIG	(0x3 << 8)
		#define VIDEO_IN_DOLBY1		(0x0 << 8)
		#define VIDEO_IN_DOLBY2		(0x1 << 8)
		#define VIDEO_IN_DOLBY3		(0x2 << 8)

#define VDOUT_INT_CLR 0x50
#define VDOUT_INT_STATUS 0x54
#endif




