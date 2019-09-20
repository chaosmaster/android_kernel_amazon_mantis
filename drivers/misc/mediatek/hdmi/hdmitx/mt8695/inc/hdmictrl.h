/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef __hdmictrl_h__
#define __hdmictrl_h__
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <linux/printk.h>

#include "hdmitable.h"
#include "internal_hdmi_drv.h"
#include "hdmiedid.h"

/* ////////////////////////////////////// */

/*for color space*/
#define RGB_444  0
#define RGBFULL_444  1
#define YCBCR_444 2
#define YCBCR_422 3

#define HDR_PACKET_DISABLE    0x00
#define HDR_PACKET_ACTIVE        0x01
#define HDR_PACKET_ZERO        0x02

#define HDR_TYPE            0x87
#define HDR_VERS            0x01
#define HDR_LEN             0x1A

/* Phi Hdr Info Frame */
#define PHI_HDR_TYPE            0x81
#define PHI_HDR_VERS            0x01
#define PHI_HDR_LEN             0x04

/* DolbyVision VS Info Frame */
#define DOLBYVSIF_TYPE            0x81
#define DOLBYVSIF_VERS            0x01
#define DOLBYVSIF_LEN             0x1B

/* Dynamic HDR EMP */
#define DYNAMIC_HDR_EMP_TYPE            0x7F
#define DYNAMIC_HDR_EMP_DS_TYPE         0x01
#define DYNAMIC_HDR_EMP_AFR             0x00
#define DYNAMIC_HDR_EMP_VFR             0x01
#define DYNAMIC_HDR_EMP_SYNC            0x01
#define DYNAMIC_HDR_EMP_ORGID           0x02
#define EMP_SIZE_FIRST      21
#define EMP_SIZE_MAX        28

/* Dynamic HDR10 PLUS VSIF */
#define DYNAMIC_HDR10P_VSIF_TYPE		0x01
#define DYNAMIC_HDR10P_VSIF_VERSION		0x01
#define DYNAMIC_HDR10P_VSIF_MAXLEN		27
#define DYNAMIC_HDR10P_VSIF_PKTHW		GEN_PKT_HW7

typedef struct {
	char ui1_EOTF;
	char ui1_Static_Metadata_ID;
	unsigned short ui2_DisplayPrimariesX0;
	unsigned short ui2_DisplayPrimariesY0;
	unsigned short ui2_DisplayPrimariesX1;
	unsigned short ui2_DisplayPrimariesY1;
	unsigned short ui2_DisplayPrimariesX2;
	unsigned short ui2_DisplayPrimariesY2;
	unsigned short ui2_WhitePointX;
	unsigned short ui2_WhitePointY;
	unsigned short ui2_MaxDisplayMasteringLuminance;
	unsigned short ui2_MinDisplayMasteringLuminance;
	unsigned short ui2_MaxCLL;
	unsigned short ui2_MaxFALL;
} HDMI_STATIC_METADATA_INFO_T;

struct mtk_hdmi {
	struct cdev *cdev;
	struct device *dev;
	struct mtk_cec *cec;
};
typedef enum {
	REG_HDMI_RGB2HDMI,
	REG_HDMI_CEC1,
	REG_HDMI_CEC2,
	REG_HDMI_DIG,
	REG_HDMI_ANA,
	HDMI_REG_NUM,
} HDMI_REG_ENUM;

typedef enum {
	AP_CCIF0,		/*  PLL/TVD */
	TOPCK_GEN,
	INFRA_SYS,
	MMSYS_CONFIG,
	GPIO_REG,
	DPI0_REG,
	EFUSE_REG,
	GIC_REG,
	PERISYS_REG,
	P2I_REG,
	VDOUT10_REG,
	VDOUT15_REG,
	VDOUT1C_REG,
	HDMI_REF_REG_NUM,
} HDMI_REF_MODULE_ENUM;

typedef enum {
	MMSYS_HDMI_HDCLK,
	MMSYS_HDMI_SDCLK,
	MMSYS_HDMI_HDCP,
	MMSYS_HDMI_HDCP24,
	MMSYS_HDMI_RGB2HDMIEN,
	MMSYS_HDMI_HDMIEN,
	MMSYS_HDMI_SDCKSEL,
	MMSYS_HDMI_HDTVD,
	MMSYS_HDMI_SDOSD,

	TOP_HDMI_SEL,
	TOP_HDMISEL_DIG_CTS,
	TOP_HDMISEL_D2,
	TOP_HDMISEL_D3,
	TOP_HDCP_SEL,
	TOP_HDCPSEL_SYS4D2,
	TOP_HDCPSEL_SYS3D4,
	TOP_HDCPSEL_UNIV2D2,
	TOP_HDCP24_SEL,
	TOP_HDCP24SEL_UNIVPD26,
	TOP_HDCP24SEL_UNIVPD52,
	TOP_HDCP24SEL_UNIVP2D8,
	HDMI_SEL_CLOCK_NUM,
} HDMI_REF_CLOCK_ENUM;

#define TX_DEF_LOG(fmt, arg...) pr_info("[HDMI]"fmt, ##arg)

extern unsigned long hdmi_reg[HDMI_REG_NUM];
extern unsigned long hdmi_ref_reg[HDMI_REF_REG_NUM];
extern struct clk *hdmi_ref_clock[HDMI_SEL_CLOCK_NUM];

extern unsigned char efuseValue0;
extern unsigned char efuseValue1;
extern unsigned char efuseValue2;
extern unsigned char efuseValue3;
extern unsigned char efuseValue4;

#define HDMIDRV_BASE   (hdmi_reg[REG_HDMI_DIG])
#define HDMICEC_BASE   (hdmi_reg[REG_HDMI_CEC2])

#define HDMISYS_BASE   (hdmi_ref_reg[MMSYS_CONFIG])
#define HDMIPLL_BASE   (hdmi_ref_reg[AP_CCIF0])	/* pll */
#define HDMICKGEN_BASE (hdmi_ref_reg[TOPCK_GEN])
#define HDMIPAD_BASE   (hdmi_ref_reg[GPIO_REG])
#define HDMI_INFRA_SYS (hdmi_ref_reg[INFRA_SYS])
#define HDMI_EFUSE_BASE (hdmi_ref_reg[EFUSE_REG])

#define TVD_DIV1 0x0
#define TVD_DIV2 0x1
#define TVD_DIV4 0x2
#define TVD_DIV8 0x3
#define TVD_OUTPUT_216M 0x40000000
#define TVD_OUTPUT_594M 0x58000000
#define TVD_OUTPUT_593M 0x57e93e94


/* ////////////////////////////////////// */
#define PORD_MODE     (1<<0)
#define HOTPLUG_MODE  (1<<1)
/* ////////////////////////////////////// */
#define AV_INFO_HD_ITU709           0x80
#define AV_INFO_SD_ITU601           0x40
#define AV_INFO_4_3_OUTPUT          0x10
#define AV_INFO_16_9_OUTPUT         0x20

/* AVI Info Frame */
#define AVI_TYPE            0x82
#define AVI_VERS            0x02
#define AVI_LEN             0x0d

/* Audio Info Frame */
#define AUDIO_TYPE             0x84
#define AUDIO_VERS             0x01
#define AUDIO_LEN              0x0A

#define VS_TYPE            0x81
#define VS_VERS            0x01
#define VS_LEN             0x05
#define VS_PB_LEN          0x0b	/* VS_LEN+1 include checksum */
#define VS_DOLBYVISION_LEN  0x18

/* GAMUT Data */
#define GAMUT_TYPE             0x0A
#define GAMUT_PROFILE          0x81
#define GAMUT_SEQ              0x31

/* ACP Info Frame */
#define ACP_TYPE            0x04
/* #define ACP_VERS            0x02 */
#define ACP_LEN             0x00

/* ISRC1 Info Frame */
#define ISRC1_TYPE            0x05
/* #define ACP_VERS            0x02 */
#define ISRC1_LEN             0x00

/* SPD Info Frame */
#define SPD_TYPE            0x83
#define SPD_VERS            0x01
#define SPD_LEN             0x19


#define SV_ON        (unsigned short)(1)
#define SV_OFF       (unsigned short)(0)

#define TMDS_CLK_X1 1
#define TMDS_CLK_X1_25  2
#define TMDS_CLK_X1_5  3
#define TMDS_CLK_X2  4

#define RJT_24BIT 0
#define RJT_16BIT 1
#define LJT_24BIT 2
#define LJT_16BIT 3
#define I2S_24BIT 4
#define I2S_16BIT 5

#define STATUS_HTPLG		(0x1 << 0)
#define STATUS_PORD			(0x1 << 1)

/********************HDMI tx analog register start***************************/
#define HDMI20_CFG_0	0x00

#define	RG_HDMITX20_SER_EN (0xf << 28)
#define	RG_HDMITX20_PRD_EN (0xf << 24)
#define	RG_HDMITX20_DRV_EN (0xf << 20)
#define	RG_HDMITX20_DRV_IMP_EN (0xf << 16)
#define	RG_HDMITX20_DRV_IMP_EN_SHIFT (16)
#define	RG_HDMITX20_MHLCK_FORCE (0x1 << 15)
#define	RG_HDMITX20_MHLCK_PPIX_EN (0x1 << 14)
#define	RG_HDMITX20_MHLCK_EN (0x1 << 13)
#define	RG_HDMITX20_SER_5T1_BIST_EN (0x1 << 12)
#define	RG_HDMITX20_SER_DIN_SEL (0xf << 8)
#define	RG_HDMITX20_SER_BIST_TOG (0x1 << 7)
#define	RG_HDMITX20_SER_CLKDIG_INV (0x1 << 6)

#define HDMI20_CFG_1	0x04

#define RG_HDMITX20_PRD_IBIAS_CLK (0x1f << 24)
#define RG_HDMITX20_PRD_IBIAS_D2 (0x1f << 16)
#define RG_HDMITX20_PRD_IBIAS_D1 (0x1f << 8)
#define RG_HDMITX20_PRD_IBIAS_D0 (0x1f << 0)
#define RG_HDMITX20_PRD_IBIAS_CLK_SHIFT (24)
#define RG_HDMITX20_PRD_IBIAS_D2_SHIFT (16)
#define RG_HDMITX20_PRD_IBIAS_D1_SHIFT (8)
#define RG_HDMITX20_PRD_IBIAS_D0_SHIFT (0)

#define HDM20_CFG_2	0x08

#define RG_HDMITX20_DRV_IBIAS_CLK (0x3f << 24)
#define RG_HDMITX20_DRV_IBIAS_D2 (0x3f << 16)
#define RG_HDMITX20_DRV_IBIAS_D1 (0x3f << 8)
#define RG_HDMITX20_DRV_IBIAS_D0 (0x3f << 0)
#define RG_HDMITX20_DRV_IBIAS_CLK_SHIFT (24)
#define RG_HDMITX20_DRV_IBIAS_D2_SHIFT (16)
#define RG_HDMITX20_DRV_IBIAS_D1_SHIFT (8)
#define RG_HDMITX20_DRV_IBIAS_D0_SHIFT (0)

#define HDMI20_CFG_3	0x0c

#define RG_HDMITX20_DRV_IMP_CLK_EN1 (0x3f << 24)
#define RG_HDMITX20_DRV_IMP_D2_EN1 (0x3f << 16)
#define RG_HDMITX20_DRV_IMP_D1_EN1 (0x3f << 8)
#define RG_HDMITX20_DRV_IMP_D0_EN1  (0x3f << 0)
#define RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT (24)
#define RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT (16)
#define RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT (8)
#define RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT  (0)


#define HDMI20_CFG_4	0x10

#define RG_HDMITX20_SER_DIN (0x3ff << 16)
#define RG_HDMITX20_SER_PASS_SEL (0xf << 12)
#define RG_HDMITX20_SER_TEST_SEL (0x3 << 10)
#define RG_HDMITX20_SER_TEST_CKEN (0x1 << 9)
#define RG_HDMITX20_SER_TEST_CKDIV2 (0x1 << 8)
#define RG_HDMITX20_SLDO_EN (0xf << 4)
#define RG_HDMITX20_SLDOLPF_EN (0x1 << 3)
#define RG_HDMITX20_SLDO_LVROD (0x3 << 1)
#define RG_HDMITX20_SLDO_MONEN (0x1 << 0)


#define HDMI20_CFG_5	0x14

#define RG_HDMITX20_CKLDO_EN (0x1 << 31)
#define RG_HDMITX20_CKLDOLPF_EN (0x1 << 30)
#define RG_HDMITX20_CKLDO_LVROD (0x3 << 28)
#define RG_HDMITX20_CKLDO_MONEN (0x1 << 27)
#define RG_HDMITX20_ABIST_EN (0x1 << 26)
#define RG_HDMITX20_CKTST_DIV (0x3 << 24)
#define RG_HDMITX20_CKTST_EN (0x1 << 23)
#define RG_HDMITX20_CKTST_SEL (0x7 << 20)
#define RG_HDMITX20_VDCTSTBUF_EN (0x1 << 17)
#define RG_HDMITX20_VDCTST_BYPASS (0x1 << 16)
#define RG_HDMITX20_VDCTST_SEL (0x1f << 8)
#define RG_HDMITX20_PRED_IMP_CLK (0x3 << 6)
#define RG_HDMITX20_PRED_IMP_D0 (0x3 << 4)
#define RG_HDMITX20_PRED_IMP_D1 (0x3 << 2)
#define RG_HDMITX20_PRED_IMP_D2 (0x3 << 0)

#define RG_HDMITX20_PRED_IMP_CLK_SHIFT (6)
#define RG_HDMITX20_PRED_IMP_D0_SHIFT (4)
#define RG_HDMITX20_PRED_IMP_D1_SHIFT (2)
#define RG_HDMITX20_PRED_IMP_D2_SHIFT (0)

#define HDMI20_CFG_6	0x18

#define RG_HDMITX20_DRV_IMP_CLK_EN2 (0x3f << 24)
#define RG_HDMITX20_DRV_IMP_D0_EN2 (0x3f << 16)
#define RG_HDMITX20_DRV_IMP_D1_EN2 (0x3f << 8)
#define RG_HDMITX20_DRV_IMP_D2_EN2 (0x3f << 0)
#define RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT (24)
#define RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT (16)
#define RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT (8)
#define RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT (0)


#define HDMI20_CFG_7	0x1c

#define RG_HDMITX20_RESERVE (0x7 << 0)


#define HDMI20_CFG_8	0x20

#define RG_HDMITX20_SER_ABEDG_EN (0x1 << 29)
#define RG_HDMITX20_BIAS_LPF_EN (0x1 << 28)
#define RG_HDMITX20_BIAS_EN (0x1 << 27)
#define RG_HDMITX20_TX_POSDIV_EN (0x1 << 26)
#define RG_HDMITX20_TX_POSDIV (0x3 << 24)
#define RG_HDMITX20_TX_POSDIV_SHIFT (24)
#define RG_HDMITX20_INTR_EN (0x1 << 21)
#define RG_HDMITX20_INTR_CAL (0x1f << 16)
#define RG_HDMITX20_INTR_CAL_SHIFT (16)

#define HDMI20_CFG_READ	0x28

#define RGS_HDMITX20_PLUG_TST (0x1 << 31)
#define RGS_HDMITX20_5T1_EDG (0xf << 27)
#define RGS_HDMITX20_5T1_LEV (0xf << 23)
#define RGS_HDMITX20_2T1_EDG (0xf << 19)
#define RGS_HDMITX20_2T1_LEV (0xf << 15)


#define HDMI20_PLL_CFG_0 0x40

#define RG_HDMITX20PLL_BG_EN (0x1 << 31)
#define RG_HDMITX20PLL_EN (0x1 << 30)
#define RG_HDMITX20PLL_FBKSEL (0x1 << 29)
#define RG_HDMITX20PLL_FBKSEL_SHIFT (29)
#define RG_HDMITX20PLL_PREDIV (0x3 << 27)
#define RG_HDMITX20PLL_PREDIV_SHIFT (27)
#define RG_HDMITX20PLL_POSDIV (0x1 << 25)
#define RG_HDMITX20PLL_POSDIV_SHIFT (25)
#define RG_HDMITX20PLL_FBKDIV (0x7ff << 13)
#define RG_HDMITX20PLL_FBKDIV_SHIFT (13)
#define RG_HDMITX20PLL_BAND (0x7f << 1)
#define RG_HDMITX20PLL_BAND_SHIFT (1)

#define HDMI20_PLL_CFG_1	0x44

#define RG_HDMITX20PLL_BC (0x1f << 27)
#define RG_HDMITX20PLL_BC_SHIFT (27)
#define RG_HDMITX20PLL_BP2 (0x3 << 25)
#define RG_HDMITX20PLL_BP2_SHIFT (25)
#define RG_HDMITX20PLL_BP (0x1f << 19)
#define RG_HDMITX20PLL_BP_SHIFT (19)
#define RG_HDMITX20PLL_BR (0xf << 12)
#define RG_HDMITX20PLL_BR_SHIFT (12)
#define RG_HDMITX20PLL_IC (0xf << 8)
#define RG_HDMITX20PLL_IC_SHIFT (8)
#define RG_HDMITX20PLL_IR (0xf << 4)
#define RG_HDMITX20PLL_IR_SHIFT (4)
#define RG_HDMITX20PLL_DL_CODE (0x3 << 2)
#define RG_HDMITX20PLL_FC_DLY (0x3 << 0)


#define HDMI20_PLL_CFG_2	0x48

#define RG_HDMITX20PLL_HIKVCO_SEL (0x1 << 31)
#define RG_HDMITX20PLL_IPATH_DEBUG_SEL (0x7 << 28)
#define RG_HDMITX20PLL_IPATH_GAIN_SEL (0x7 << 25)
#define RG_HDMITX20PLL_IPATH_IDAC (0x1fff << 11)
#define RG_HDMITX20PLL_IPATH_SDM_DI_EN (0x1 << 7)
#define RG_HDMITX20PLL_IPATH_SDM_DI_LS (0x3 << 5)
#define RG_HDMITX20PLL_IPATH_SDM_IFM (0x1 << 4)
#define RG_HDMITX20PLL_IPATH_SDM_ORD (0x3 << 2)
#define RG_HDMITX20PLL_IPATH_SDM_OUT (0x1 << 1)


#define HDMI20_PLL_CFG_3	0x4c

#define RG_HDMITX20PLL_KBAND_PREDIV (0x3 << 30)
#define RG_HDMITX20PLL_LOAD_EN (0x1 << 29)
#define RG_HDMITX20PLL_LVR_REFSEL (0x3 << 27)
#define RG_HDMITX20PLL_MONCK_EN (0x1 << 26)
#define RG_HDMITX20PLL_MONREF_EN (0x1 << 25)
#define RG_HDMITX20PLL_MONVC_EN (0x1 << 24)
#define RG_HDMITX20PLL_RST_DLY (0x3 << 22)
#define RG_HDMITX20PLL_SLEEP_EN (0x1 << 21)
#define RG_HDMITX20PLL_VREF_RANGE_SEL (0x3 << 19)
#define RG_HDMITX20PLL_VOD_EN (0x1 << 18)
#define RG_HDMITX20PLL_REFCK_SEL (0x1 << 17)
#define RG_HDMITX20PLL_CLKTX_EN (0x1 << 16)
#define RG_HDMITX20PLL_RESERVE (0x1ffff << 0)


#define HDMI20_PLL_CFG_READ	0x58

#define RGS_HDMITX20PLL_IPLL_DIG_MON (0x3f << 26)
#define RGS_HDMITX20PLL_KBAND_CPLT (0x1 << 25)


#define HDMI20_CLK_CFG	0x70

#define REG_TXC_DATA_0 (0x3ff << 0)
#define REG_TXC_DATA_1 (0x3ff << 10)
#define REG_TXC_DATA_2 (0x3ff << 20)
#define REG_TXC_DIV (0x3 << 30)
#define REG_TXC_DIV_SHIFT (30)


#define HDMI_ANA_CTL 0x7c

#define REG_ANA_HDMI20_DATA_SYNC (0x1 << 20)
#define REG_ANA_HDMI20_PACKET_MODE (0x1 << 19)
#define REG_ANA_HDMI20_MHL_MODE (0x1 << 18)
#define REG_ANA_HDMI20_RD_INV  (0x1 << 17)
#define REG_ANA_HDMI20_FIFO_EN (0x1 << 16)

/********************HDMI tx analog register end***************************/

/********************HDMI tx digital register start***************************/
#define TOP_CFG00 0x000

#define ABIST_ENABLE (1 << 31)
#define ABIST_DISABLE (0 << 31)
#define ABIST_SPEED_MODE_SET (1 << 27)
#define ABIST_SPEED_MODE_CLR (0 << 27)
#define ABIST_DATA_FORMAT_STAIR (0 << 24)
#define ABIST_DATA_FORMAT_0xAA (1 << 24)
#define ABIST_DATA_FORMAT_0x55 (2 << 24)
#define ABIST_DATA_FORMAT_0x11 (3 << 24)
#define ABIST_VSYNC_POL_LOW (0 << 23)
#define ABIST_VSYNC_POL_HIGH (1 << 23)
#define ABIST_HSYNC_POL_LOW (0 << 22)
#define ABIST_HSYNC_POL_HIGH (1 << 22)
#define ABIST_VIDEO_FORMAT_720x480P (0x2 << 16)
#define ABIST_VIDEO_FORMAT_720P60 (0x3 << 16)
#define ABIST_VIDEO_FORMAT_1080I60 (0x4 << 16)
#define ABIST_VIDEO_FORMAT_480I (0x5 << 16)
#define ABIST_VIDEO_FORMAT_1440x480P (0x9 << 16)
#define ABIST_VIDEO_FORMAT_1080P60 (0xA << 16)
#define ABIST_VIDEO_FORMAT_576P (0xB << 16)
#define ABIST_VIDEO_FORMAT_720P50 (0xC << 16)
#define ABIST_VIDEO_FORMAT_1080I50 (0xD << 16)
#define ABIST_VIDEO_FORMAT_576I (0xE << 16)
#define ABIST_VIDEO_FORMAT_1440x576P (0x12 << 16)
#define ABIST_VIDEO_FORMAT_1080P50 (0x13 << 16)
#define ABIST_VIDEO_FORMAT_3840x2160P25 (0x18 << 16)
#define ABIST_VIDEO_FORMAT_3840x2160P30 (0x19 << 16)
#define ABIST_VIDEO_FORMAT_4096X2160P30 (0x1A << 16)
#define HDMI2_BYP_ON (1 << 13)
#define HDMI2_BYP_OFF (0 << 13)
#define DEEPCOLOR_PAT_EN (1 << 12)
#define DEEPCOLOR_PAT_DIS (0 << 12)
#define DEEPCOLOR_MODE_8BIT (0 << 8)
#define DEEPCOLOR_MODE_10BIT (1 << 8)
#define DEEPCOLOR_MODE_12BIT (2 << 8)
#define DEEPCOLOR_MODE_16BIT (3 << 8)
#define DEEPCOLOR_MODE_MASKBIT (3 << 8)

#define HDMIMODE_VAL_HDMI (1 << 7)
#define HDMIMODE_VAL_DVI (0 << 7)
#define HDMIMODE_OVR_ON (1 << 6)
#define HDMIMODE_OVR_OFF (0 << 6)
#define SCR_MODE_CTS (1 << 5)
#define SCR_MODE_NORMAL (0 << 5)
#define SCR_ON (1 << 4)
#define SCR_OFF (0 << 4)
#define HDMI_MODE_HDMI (1 << 3)
#define HDMI_MODE_DVI (0 << 3)
#define HDMI2_ON (1 << 2)
#define HDMI2_OFF (0 << 2)


/***********************************************/
#define TOP_CFG01 0x004

#define VIDEO_MUTE_DATA (0xFFFFFF << 4)
#define NULL_PKT_VSYNC_HIGH_EN (1 << 3)
#define NULL_PKT_VSYNC_HIGH_DIS (0 << 3)
#define NULL_PKT_EN (1 << 2)
#define NULL_PKT_DIS (0 << 2)
#define CP_CLR_MUTE_EN (1 << 1)
#define CP_CLR_MUTE_DIS (0 << 1)
#define CP_SET_MUTE_EN (1 << 0)
#define CP_SET_MUTE_DIS (0 << 0)


/***********************************************/
#define TOP_CFG02 0x008

#define MHL_IEEE_NO (0xFFFFFF << 8)
#define MHL_VFMT_EXTD (0x3 << 5)
#define VSI_OVERRIDE_DIS (0x1 << 4)
#define AVI_OVERRIDE_DIS (0x1 << 3)
#define MINIVSYNC_ON (0x1 << 2)
#define MHL_3DCONV_EN (0x1 << 1)
#define INTR_ENCRYPTION_EN (0x1 << 0)
#define INTR_ENCRYPTION_DIS (0x0 << 0)

/***********************************************/
#define TOP_AUD_MAP 0x00C

#define SD7_MAP (0x7 << 28)
#define SD6_MAP (0x7 << 24)
#define SD5_MAP (0x7 << 20)
#define SD4_MAP (0x7 << 16)
#define SD3_MAP (0x7 << 12)
#define SD2_MAP (0x7 << 8)
#define SD1_MAP (0x7 << 4)
#define SD0_MAP (0x7 << 0)

#define C_SD7 (0x7 << 28)
#define C_SD6 (0x6 << 24)
#define C_SD5 (0x5 << 20)
#define C_SD4 (0x4 << 16)
#define C_SD3 (0x3 << 12)
#define C_SD2 (0x2 << 8)
#define C_SD1 (0x1 << 4)
#define C_SD0 (0x0 << 0)


/***********************************************/
#define TOP_OUT00 0x010

#define QC_SEL (0x3 << 10)
#define Q2_SEL (0x3 << 8)
#define Q1_SEL (0x3 << 6)
#define Q0_SEL (0x3 << 4)
#define USE_CH_MUX4 (0x1 << 2)
#define USE_CH_MUX0 (0x0 << 2)
#define TX_BIT_INV_EN (0x1 << 1)
#define TX_BIT_INV_DIS (0x0 << 1)
#define Q_9T0_BIT_SWAP (0x1 << 0)
#define Q_9T0_NO_SWAP (0x0 << 0)

#define SELECT_CK_CHANNEL 0x3
#define SELECT_Q2_CHANNEL 0x2
#define SELECT_Q1_CHANNEL 0x1
#define SELECT_Q0_CHANNEL 0x0

/***********************************************/
#define TOP_OUT01 0x014

#define TXC_DIV8 (0x3 << 0x30)
#define TXC_DIV4 (0x2 << 0x30)
#define TXC_DIV2 (0x1 << 0x30)
#define TXC_DIV1 (0x0 << 0x30)
#define TXC_DATA2 (0x3FF << 20)
#define TXC_DATA1 (0x3FF << 10)
#define TXC_DATA0 (0x3FF << 0)

/***********************************************/
#define TOP_OUT02 0x018

#define FIX_OUT_EN (0x1 << 31)
#define FIX_OUT_DIS (0x0 << 31)
#define QOUT_DATA (0x3FFFFFFF << 0)

/***********************************************/
#define TOP_INFO_EN 0x01C

#define GEN8_EN_WR (0x1 << 30)
#define GEN8_DIS_WR (0x0 << 30)
#define GEN7_EN_WR (0x1 << 29)
#define GEN7_DIS_WR (0x0 << 29)
#define GEN6_EN_WR (0x1 << 28)
#define GEN6_DIS_WR (0x0 << 28)
#define VSIF_EN_WR (0x1 << 27)
#define VSIF_DIS_WR (0x0 << 27)
#define GAMUT_EN_WR (0x1 << 26)
#define GAMUT_DIS_WR (0x0 << 26)
#define GEN5_EN_WR (0x1 << 25)
#define GEN5_DIS_WR (0x0 << 25)
#define GEN4_EN_WR (0x1 << 24)
#define GEN4_DIS_WR (0x1 << 24)
#define GEN3_EN_WR (0x1 << 23)
#define GEN3_DIS_WR (0x0 << 23)
#define GEN2_EN_WR (0x1 << 22)
#define GEN2_DIS_WR (0x0 << 22)
#define CP_EN_WR (0x1 << 21)
#define CP_DIS_WR (0x0 << 21)
#define GEN_EN_WR (0x1 << 20)
#define GEN_DIS_WR (0x0 << 20)
#define MPEG_EN_WR (0x1 << 19)
#define MPEG_DIS_WR (0x0 << 19)
#define AUD_EN_WR (0x1 << 18)
#define AUD_DIS_WR (0x0 << 18)
#define SPD_EN_WR (0x1 << 17)
#define SPD_DIS_WR (0x0 << 17)
#define AVI_EN_WR (0x1 << 16)
#define AVI_DIS_WR (0x1 << 16)
#define GEN8_EN (0x1 << 14)
#define GEN8_DIS (0x0 << 14)
#define GEN7_EN (0x1 << 13)
#define GEN7_DIS (0x0 << 13)
#define GEN6_EN (0x1 << 12)
#define GEN6_DIS (0x0 << 12)
#define VSIF_EN (0x1 << 11)
#define VSIF_DIS (0x0 << 11)
#define HDMI2_GAMUT_EN (0x1 << 10)
#define HDMI2_GAMUT_DIS (0x0 << 10)
#define GEN5_EN (0x1 << 9)
#define GEN5_DIS (0x0 << 9)
#define GEN4_EN (0x1 << 8)
#define GEN4_DIS (0x1 << 8)
#define GEN3_EN (0x1 << 7)
#define GEN3_DIS (0x0 << 7)
#define GEN2_EN (0x1 << 6)
#define GEN2_DIS (0x0 << 6)
#define CP_EN (0x1 << 5)
#define CP_DIS (0x0 << 5)
#define GEN_EN (0x1 << 4)
#define GEN_DIS (0x0 << 4)
#define MPEG_EN (0x1 << 3)
#define MPEG_DIS (0x0 << 3)
#define AUD_EN (0x1 << 2)
#define AUD_DIS (0x0 << 2)
#define SPD_EN (0x1 << 1)
#define SPD_DIS (0x0 << 1)
#define AVI_EN (0x1 << 0)
#define AVI_DIS (0x0 << 0)

/***********************************************/
#define TOP_INFO_RPT 0x020

#define GEN15_RPT_EN (0x1 << 21)
#define GEN15_RPT_DIS (0x1 << 21)
#define GEN14_RPT_EN (0x1 << 20)
#define GEN14_RPT_DIS (0x1 << 20)
#define GEN13_RPT_EN (0x1 << 19)
#define GEN13_RPT_DIS (0x1 << 19)
#define GEN12_RPT_EN (0x1 << 18)
#define GEN12_RPT_DIS (0x1 << 18)
#define GEN11_RPT_EN (0x1 << 17)
#define GEN11_RPT_DIS (0x1 << 17)
#define GEN10_RPT_EN (0x1 << 16)
#define GEN10_RPT_DIS (0x1 << 16)
#define GEN9_RPT_EN (0x1 << 15)
#define GEN9_RPT_DIS (0x1 << 15)
#define GEN8_RPT_EN (0x1 << 14)
#define GEN8_RPT_DIS (0x1 << 14)
#define GEN7_RPT_EN (0x1 << 13)
#define GEN7_RPT_DIS (0x1 << 13)
#define GEN6_RPT_EN (0x1 << 12)
#define GEN6_RPT_DIS (0x1 << 12)
#define VSIF_RPT_EN (0x1 << 11)
#define VSIF_RPT_DIS (0x0 << 11)
#define GAMUT_RPT_EN (0x1 << 10)
#define GAMUT_RPT_DIS (0x0 << 10)
#define GEN5_RPT_EN (0x1 << 9)
#define GEN5_RPT_DIS (0x0 << 9)
#define GEN4_RPT_EN (0x1 << 8)
#define GEN4_RPT_DIS (0x1 << 8)
#define GEN3_RPT_EN (0x1 << 7)
#define GEN3_RPT_DIS (0x0 << 7)
#define GEN2_RPT_EN (0x1 << 6)
#define GEN2_RPT_DIS (0x0 << 6)
#define CP_RPT_EN (0x1 << 5)
#define CP_RPT_DIS (0x0 << 5)
#define GEN_RPT_EN (0x1 << 4)
#define GEN_RPT_DIS (0x0 << 4)
#define MPEG_RPT_EN (0x1 << 3)
#define MPEG_RPT_DIS (0x0 << 3)
#define AUD_RPT_EN (0x1 << 2)
#define AUD_RPT_DIS (0x0 << 2)
#define SPD_RPT_EN (0x1 << 1)
#define SPD_RPT_DIS (0x0 << 1)
#define AVI_RPT_EN (0x1 << 0)
#define AVI_RPT_DIS (0x1 << 0)

/***********************************************/
#define TOP_AVI_HEADER 0x024

#define AVI_HEADER (0xFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT00 0x028

#define AVI_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT01 0x02C

#define AVI_PKT01 (0xFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT02 0x030

#define AVI_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT03 0x034

#define AVI_PKT03 (0xFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT04 0x038

#define AVI_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_AVI_PKT05 0x03C

#define AVI_PKT05 (0xFFFFFF << 0)

/***********************************************/
#define TOP_AIF_HEADER 0x040

#define AIF_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_AIF_PKT00 0x044

#define AIF_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_AIF_PKT01 0x048

#define AIF_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_AIF_PKT02 0x04C

#define AIF_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_AIF_PKT03 0x050

#define AIF_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_HEADER 0x054

#define SPDIF_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT00 0x058

#define SPDIF_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT01 0x05C

#define SPDIF_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT02 0x060

#define SPDIF_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT03 0x064

#define SPDIF_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT04 0x068

#define SPDIF_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT05 0x06C

#define SPDIF_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT06 0x070

#define SPDIF_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_SPDIF_PKT07 0x074

#define SPDIF_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_MPEG_HEADER 0x078

#define MPEG_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT00 0x07C

#define MPEG_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT01 0x080

#define MPEG_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT02 0x084

#define MPEG_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT03 0x088

#define MPEG_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT04 0x08C

#define MPEG_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT05 0x090

#define MPEG_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT06 0x094

#define MPEG_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_MPEG_PKT07 0x098

#define MPEG_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN_HEADER 0x09C

#define GEN_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT00 0x0A0

#define GEN_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT01 0x0A4

#define GEN_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT02 0x0A8

#define GEN_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT03 0x0AC

#define GEN_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT04 0x0B0

#define GEN_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT05 0x0B4

#define GEN_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT06 0x0B8

#define GEN_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN_PKT07 0x0BC

#define GEN_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN2_HEADER 0x0C0

#define GEN2_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT00 0x0C4

#define GEN2_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT01 0x0C8

#define GEN2_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT02 0x0CC

#define GEN2_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT03 0x0D0

#define GEN2_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT04 0x0D4

#define GEN2_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT05 0x0D8

#define GEN2_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT06 0x0DC

#define GEN2_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN2_PKT07 0x0E0

#define GEN2_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN3_HEADER 0x0E4

#define GEN3_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT00 0x0E8

#define GEN3_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT01 0x0EC

#define GEN3_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT02 0x0F0

#define GEN3_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT03 0x0F4

#define GEN3_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT04 0x0F8

#define GEN3_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT05 0x0FC

#define GEN3_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT06 0x100

#define GEN3_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN3_PKT07 0x104

#define GEN3_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN4_HEADER 0x108

#define GEN4_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT00 0x10C

#define GEN4_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT01 0x110

#define GEN4_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT02 0x114

#define GEN4_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT03 0x118

#define GEN4_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT04 0x11C

#define GEN4_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT05 0x120

#define GEN4_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT06 0x124

#define GEN4_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN4_PKT07 0x128

#define GEN4_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN5_HEADER 0x12C

#define GEN5_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT00 0x130

#define GEN5_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT01 0x134

#define GEN5_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT02 0x138

#define GEN5_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT03 0x13C

#define GEN5_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT04 0x140

#define GEN5_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT05 0x144

#define GEN5_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT06 0x148

#define GEN5_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GEN5_PKT07 0x14C

#define GEN5_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_HEADER 0x150

#define GAMUT_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT00 0x154

#define GAMUT_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT01 0x158

#define GAMUT_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT02 0x15C

#define GAMUT_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT03 0x160

#define GAMUT_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT04 0x164

#define GAMUT_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT05 0x168

#define GAMUT_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT06 0x16C

#define GAMUT_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_GAMUT_PKT07 0x170

#define GAMUT_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_VSIF_HEADER 0x174

#define VSIF_HEADER (0x00FFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT00 0x178

#define VSIF_PKT00 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT01 0x17C

#define VSIF_PKT01 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT02 0x180

#define VSIF_PKT02 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT03 0x184

#define VSIF_PKT03 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT04 0x188

#define VSIF_PKT04 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT05 0x18C

#define VSIF_PKT05 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT06 0x190

#define VSIF_PKT06 (0xFFFFFFFF << 0)

/***********************************************/
#define TOP_VSIF_PKT07 0x194

#define VSIF_PKT07 (0x00FFFFFF << 0)

/***********************************************/
#define TOP_DROP_CFG00 0x198

#define LR_SWAP (0x01<<0)
#define LFE_CC_SWAP (0x01<<1)
#define LSRS_SWAP (0x01<<2)
#define RLS_RRS_SWAP (0x01<<3)
#define LR_STATUS_SWAP (0x01<<4)


/***********************************************/
#define TOP_DROP_CFG01 0x19C

/***********************************************/


/***********************************************/
#define TOP_MISC_CTLR 0x1A4

#define INFO_HEADER (0xFF << 16)
#define DISABLE_IDLE_DDC_RESET (0x1 << 7)
#define DEEP_COLOR_ADD (0x1 << 4)
#define HSYNC_POL_POS (0x1 << 1)
#define HSYNC_POL_NEG (0x0 << 1)
#define VSYNC_POL_POS (0x1 << 0)
#define VSYNC_POL_NEG (0x0 << 0)

/***********************************************/
#define TOP_INT_STA00 0x1A8

#define DDC_FIFO_HALF_FULL_INT_STA (1 << 31)
#define DDC_FIFO_FULL_INT_STA (1 << 30)
#define DDC_I2C_IN_PROG_INT_STA (1 << 29)
#define HDCP_RI_128_INT_STA (1 << 28)
#define HDCP_SHA_START_INT_STA (1 << 27)
#define HDCP2X_RX_RPT_READY_DDCM_INT_STA (1 << 26)
#define HDCP2X_RX_REAUTH_REQ_DDCM_INT_STA (1 << 25)
#define HDCP2X_RPT_SMNG_XFER_DONE_INT_STA (1 << 24)
#define HDCP2X_RPT_RCVID_CHANGED_INT_STA (1 << 23)
#define HDCP2X_CERT_SEND_RCVD_INT_STA (1 << 22)
#define HDCP2X_SKE_SENT_RCVD_INT_STA (1 << 21)
#define HDCP2X_AKE_SENT_RCVD_INT_STA (1 << 20)
#define HDCP2X_HASH_FAIL_INT_STA (1 << 19)
#define HDCP2X_CCHK_DONE_INT_STA (1 << 18)
#define HDCP2X_AUTH_FAIL_INT_STA (1 << 17)
#define HDCP2X_AUTH_DONE_INT_STA (1 << 16)
#define HDCP2X_MSG_7_INT_STA (1 << 15)
#define HDCP2X_MSG_6_INT_STA (1 << 14)
#define HDCP2X_MSG_5_INT_STA (1 << 13)
#define HDCP2X_MSG_4_INT_STA (1 << 12)
#define HDCP2X_MSG_3_INT_STA (1 << 11)
#define HDCP2X_MSG_2_INT_STA (1 << 10)
#define HDCP2X_MSG_1_INT_STA (1 << 9)
#define HDCP2X_MSG_0_INT_STA (1 << 8)
#define INFO_DONE_INT_STA (1 << 7)
#define PB_FULL_INT_STA (1 << 6)
#define AUDIO_INT_STA (1 << 5)
#define VSYNC_INT_STA (1 << 4)
#define PORD_F_INT_STA (1 << 3)
#define PORD_R_INT_STA (1 << 2)
#define HTPLG_F_INT_STA (1 << 1)
#define HTPLG_R_INT_STA (1 << 0)

/***********************************************/
#define TOP_INT_STA01 0x1AC

#define HDCP2X_STATE_CHANGE_P_INT_STA (1 << 10)
#define SCDC_UP_FLAG_DONE_INT_STA (1 << 9)
#define SCDC_SLAVE_RD_REQ_P_INT_STA (1 << 8)
#define SCDC_DDC_CONFICT_INT_STA (1 << 7)
#define SCDC_DDC_DONE_INT_STA (1 << 6)
#define RI_ERR3_INT_STA (1 << 5)
#define RI_ERR2_INT_STA (1 << 4)
#define RI_ERR1_INT_STA (1 << 3)
#define RI_ERR0_INT_STA (1 << 2)
#define RI_BCAP_ON_INT_STA (1 << 1)
#define DDC_FIFO_EMPTY_INT_STA (1 << 0)

/***********************************************/
#define TOP_INT_MASK00 0x1B0

#define DDC_FIFO_HALF_FULL_INT_MASK (0 << 31)
#define DDC_FIFO_HALF_FULL_INT_UNMASK (1 << 31)
#define DDC_FIFO_FULL_INT_MASK (0 << 30)
#define DDC_FIFO_FULL_INT_UNMASK (1 << 30)
#define DDC_I2C_IN_PROG_INT_MASK (0 << 29)
#define DDC_I2C_IN_PROG_INT_UNMASK (1 << 29)
#define HDCP_RI_128_INT_MASK (0 << 28)
#define HDCP_RI_128_INT_UNMASK (1 << 28)
#define HDCP_SHA_START_INT_MASK (0 << 27)
#define HDCP_SHA_START_INT_UNMASK (1 << 27)
#define HDCP2X_RX_RPT_READY_DDCM_INT_MASK (0 << 26)
#define HDCP2X_RX_RPT_READY_DDCM_INT_UNMASK (1 << 26)
#define HDCP2X_RX_REAUTH_REQ_DDCM_INT_MASK (0 << 25)
#define HDCP2X_RX_REAUTH_REQ_DDCM_INT_UNMASK (1 << 25)
#define HDCP2X_RPT_SMNG_XFER_DONE_INT_MASK (0 << 24)
#define HDCP2X_RPT_SMNG_XFER_DONE_INT_UNMASK (1 << 24)
#define HDCP2X_RPT_RCVID_CHANGED_INT_MASK (0 << 23)
#define HDCP2X_RPT_RCVID_CHANGED_INT_UNMASK (1 << 23)
#define HDCP2X_CERT_SEND_RCVD_INT_MASK (0 << 22)
#define HDCP2X_CERT_SEND_RCVD_INT_UNMASK (1 << 22)
#define HDCP2X_SKE_SENT_RCVD_INT_MASK (0 << 21)
#define HDCP2X_SKE_SENT_RCVD_INT_UNMASK (1 << 21)
#define HDCP2X_AKE_SENT_RCVD_INT_MASK (0 << 20)
#define HDCP2X_AKE_SENT_RCVD_INT_UNMASK (1 << 20)
#define HDCP2X_HASH_FAIL_INT_MASK (0 << 19)
#define HDCP2X_HASH_FAIL_INT_UNMASK (1 << 19)
#define HDCP2X_CCHK_DONE_INT_MASK (0 << 18)
#define HDCP2X_CCHK_DONE_INT_UNMASK (1 << 18)
#define HDCP2X_AUTH_FAIL_INT_MASK (0 << 17)
#define HDCP2X_AUTH_FAIL_INT_UNMASK (1 << 17)
#define HDCP2X_AUTH_DONE_INT_MASK (0 << 16)
#define HDCP2X_AUTH_DONE_INT_UNMASK (1 << 16)
#define HDCP2X_MSG_7_INT_MASK (0 << 15)
#define HDCP2X_MSG_7_INT_UNMASK (1 << 15)
#define HDCP2X_MSG_6_INT_MASK (0 << 14)
#define HDCP2X_MSG_6_INT_UNMASK (1 << 14)
#define HDCP2X_MSG_5_INT_MASK (0 << 13)
#define HDCP2X_MSG_5_INT_UNMASK (1 << 13)
#define HDCP2X_MSG_4_INT_MASK (0 << 12)
#define HDCP2X_MSG_4_INT_UNMASK (1 << 12)
#define HDCP2X_MSG_3_INT_MASK (0 << 11)
#define HDCP2X_MSG_3_INT_UNMASK (1 << 11)
#define HDCP2X_MSG_2_INT_MASK (0 << 10)
#define HDCP2X_MSG_2_INT_UNMASK (1 << 10)
#define HDCP2X_MSG_1_INT_MASK (0 << 9)
#define HDCP2X_MSG_1_INT_UNMASK (1 << 9)
#define HDCP2X_MSG_0_INT_MASK (0 << 8)
#define HDCP2X_MSG_0_INT_UNMASK (1 << 8)
#define INFO_DONE_INT_MASK (0 << 7)
#define INFO_DONE_INT_UNMASK (1 << 7)
#define PB_FULL_INT_MASK (0 << 6)
#define PB_FULL_INT_UNMASK (1 << 6)
#define AUDIO_INT_MASK (0 << 5)
#define AUDIO_INT_UNMASK (1 << 5)
#define VSYNC_INT_MASK (0 << 4)
#define VSYNC_INT_UNMASK (1 << 4)
#define PORD_F_INT_MASK (0 << 3)
#define PORD_F_INT_UNMASK (1 << 3)
#define PORD_R_INT_MASK (0 << 2)
#define PORD_R_INT_UNMASK (1 << 2)
#define HTPLG_F_INT_MASK (0 << 1)
#define HTPLG_F_INT_UNMASK (1 << 1)
#define HTPLG_R_INT_MASK (0 << 0)
#define HTPLG_R_INT_UNMASK (1 << 0)

/***********************************************/
#define TOP_INT_MASK01 0x1B4

#define HDCP2X_STATE_CHANGE_P_INT_MASK (1 << 10)
#define SCDC_UP_FLAG_DONE_INT_MASK (1 << 9)
#define SCDC_SLAVE_RD_REQ_P_INT_MASK (1 << 8)
#define SCDC_DDC_CONFICT_INT_MASK (1 << 7)
#define SCDC_DDC_DONE_INT_MASK (1 << 6)
#define RI_ERR3_INT_MASK (1 << 5)
#define RI_ERR2_INT_MASK (1 << 4)
#define RI_ERR1_INT_MASK (1 << 3)
#define RI_ERR0_INT_MASK (1 << 2)
#define RI_BCAP_ON_INT_MASK (1 << 1)
#define DDC_FIFO_EMPTY_INT_MASK (1 << 0)

#define HDCP2X_STATE_CHANGE_P_INT_UNMASK (0 << 10)
#define SCDC_UP_FLAG_DONE_INT_UNMASK (0 << 9)
#define SCDC_SLAVE_RD_REQ_P_INT_UNMASK (0 << 8)
#define SCDC_DDC_CONFICT_INT_UNMASK (0 << 7)
#define SCDC_DDC_DONE_INT_UNMASK (0 << 6)
#define RI_ERR3_INT_UNMASK (0 << 5)
#define RI_ERR2_INT_UNMASK (0 << 4)
#define RI_ERR1_INT_UNMASK (0 << 3)
#define RI_ERR0_INT_UNMASK (0 << 2)
#define RI_BCAP_ON_INT_UNMASK (0 << 1)
#define DDC_FIFO_EMPTY_INT_UNMASK (0 << 0)
/***********************************************/
#define TOP_INT_CLR00 0x1B8

#define DDC_FIFO_HALF_FULL_INT_CLR (1 << 31)
#define DDC_FIFO_FULL_INT_CLR (1 << 30)
#define DDC_I2C_IN_PROG_INT_CLR (1 << 29)
#define HDCP_RI_128_INT_CLR (1 << 28)
#define HDCP_SHA_START_INT_CLR (1 << 27)
#define HDCP2X_RX_RPT_READY_DDCM_INT_CLR (1 << 26)
#define HDCP2X_RX_REAUTH_REQ_DDCM_INT_CLR (1 << 25)
#define HDCP2X_RPT_SMNG_XFER_DONE_INT_CLR (1 << 24)
#define HDCP2X_RPT_RCVID_CHANGED_INT_CLR (1 << 23)
#define HDCP2X_CERT_SEND_RCVD_INT_CLR (1 << 22)
#define HDCP2X_SKE_SENT_RCVD_INT_CLR (1 << 21)
#define HDCP2X_AKE_SENT_RCVD_INT_CLR (1 << 20)
#define HDCP2X_HASH_FAIL_INT_CLR (1 << 19)
#define HDCP2X_CCHK_DONE_INT_CLR (1 << 18)
#define HDCP2X_AUTH_FAIL_INT_CLR (1 << 17)
#define HDCP2X_AUTH_DONE_INT_CLR (1 << 16)
#define HDCP2X_MSG_7_INT_CLR (1 << 15)
#define HDCP2X_MSG_6_INT_CLR (1 << 14)
#define HDCP2X_MSG_5_INT_CLR (1 << 13)
#define HDCP2X_MSG_4_INT_CLR (1 << 12)
#define HDCP2X_MSG_3_INT_CLR (1 << 11)
#define HDCP2X_MSG_2_INT_CLR (1 << 10)
#define HDCP2X_MSG_1_INT_CLR (1 << 9)
#define HDCP2X_MSG_0_INT_CLR (1 << 8)
#define INFO_DONE_INT_CLR (1 << 7)
#define PB_FULL_INT_CLR (1 << 6)
#define AUDIO_INT_CLR (1 << 5)
#define VSYNC_INT_CLR (1 << 4)
#define PORD_F_INT_CLR (1 << 3)
#define PORD_R_INT_CLR (1 << 2)
#define HTPLG_F_INT_CLR (1 << 1)
#define HTPLG_R_INT_CLR (1 << 0)

#define DDC_FIFO_HALF_FULL_INT_UNCLR (0 << 31)
#define DDC_FIFO_FULL_INT_UNCLR (0 << 30)
#define DDC_I2C_IN_PROG_INT_UNCLR (0 << 29)
#define HDCP_RI_128_INT_UNCLR (0 << 28)
#define HDCP_SHA_START_INT_UNCLR (0 << 27)
#define HDCP2X_RX_RPT_READY_DDCM_INT_UNCLR (0 << 26)
#define HDCP2X_RX_REAUTH_REQ_DDCM_INT_UNCLR (0 << 25)
#define HDCP2X_RPT_SMNG_XFER_DONE_INT_UNCLR (0 << 24)
#define HDCP2X_RPT_RCVID_CHANGED_INT_UNCLR (0 << 23)
#define HDCP2X_CERT_SEND_RCVD_INT_UNCLR (0 << 22)
#define HDCP2X_SKE_SENT_RCVD_INT_UNCLR (0 << 21)
#define HDCP2X_AKE_SENT_RCVD_INT_UNCLR (0 << 20)
#define HDCP2X_HASH_FAIL_INT_UNCLR (0 << 19)
#define HDCP2X_CCHK_DONE_INT_UNCLR (0 << 18)
#define HDCP2X_AUTH_FAIL_INT_UNCLR (0 << 17)
#define HDCP2X_AUTH_DONE_INT_UNCLR (0 << 16)
#define HDCP2X_MSG_7_INT_UNCLR (0 << 15)
#define HDCP2X_MSG_6_INT_UNCLR (0 << 14)
#define HDCP2X_MSG_5_INT_UNCLR (0 << 13)
#define HDCP2X_MSG_4_INT_UNCLR (0 << 12)
#define HDCP2X_MSG_3_INT_UNCLR (0 << 11)
#define HDCP2X_MSG_2_INT_UNCLR (0 << 10)
#define HDCP2X_MSG_1_INT_UNCLR (0 << 9)
#define HDCP2X_MSG_0_INT_UNCLR (0 << 8)
#define INFO_DONE_INT_UNCLR (0 << 7)
#define PB_FULL_INT_UNCLR (0 << 6)
#define AUDIO_INT_UNCLR (0 << 5)
#define VSYNC_INT_UNCLR (0 << 4)
#define PORD_F_INT_UNCLR (0 << 3)
#define PORD_R_INT_UNCLR (0 << 2)
#define HTPLG_F_INT_UNCLR (0 << 1)
#define HTPLG_R_INT_UNCLR (0 << 0)

/***********************************************/
#define TOP_INT_CLR01 0x1BC

#define HDCP2X_STATE_CHANGE_P_INT_CLR (1 << 10)
#define SCDC_UP_FLAG_DONE_INT_CLR (1 << 9)
#define SCDC_SLAVE_RD_REQ_P_INT_CLR (1 << 8)
#define SCDC_DDC_CONFICT_INT_CLR (1 << 7)
#define SCDC_DDC_DONE_INT_CLR (1 << 6)
#define RI_ERR3_INT_CLR (1 << 5)
#define RI_ERR2_INT_CLR (1 << 4)
#define RI_ERR1_INT_CLR (1 << 3)
#define RI_ERR0_INT_CLR (1 << 2)
#define RI_BCAP_ON_INT_CLR (1 << 1)
#define DDC_FIFO_EMPTY_INT_CLR (1 << 0)

#define HDCP2X_STATE_CHANGE_P_INT_UNCLR (0 << 10)
#define SCDC_UP_FLAG_DONE_INT_UNCLR (0 << 9)
#define SCDC_SLAVE_RD_REQ_P_INT_UNCLR (0 << 8)
#define SCDC_DDC_CONFICT_INT_UNCLR (0 << 7)
#define SCDC_DDC_DONE_INT_UNCLR (0 << 6)
#define RI_ERR3_INT_UNCLR (0 << 5)
#define RI_ERR2_INT_UNCLR (0 << 4)
#define RI_ERR1_INT_UNCLR (0 << 3)
#define RI_ERR0_INT_UNCLR (0 << 2)
#define RI_BCAP_ON_INT_UNCLR (0 << 1)
#define DDC_FIFO_EMPTY_INT_UNCLR (0 << 0)

/***********************************************/
#define TOP_STA 0x1C0

#define STARTUP_FLAG (1 << 13)
#define P_STABLE (1 << 12)
#define CEA_AVI_EN (1 << 11)
#define CEA_SPD_EN (1 << 10)
#define CEA_AUD_EN (1 << 9)
#define CEA_MPEG_EN (1 << 8)
#define CEA_GEN_EN (1 << 7)
#define CEA_CP_EN (1 << 6)
#define CEA_GEN2_EN (1 << 5)
#define CEA_GEN3_EN (1 << 4)
#define CEA_GEN4_EN (1 << 3)
#define CEA_GEN5_EN (1 << 2)
#define CEA_GAMUT_EN (1 << 1)
#define CEA_VSIF_EN (1 << 0)

#define TOP_VMUTE_CFG1 0x1C8
#define REG_VMUTE_EN (1 << 16)

#define TOP_VMUTE_CFG2 0x1CC

#define HDMI_PLLMETER_TRIGGER 0x1D0
#define HDMI_PLL_FREQ_TRGGIER (1 << 0)

#define HDMI_PLLMETER 0x1D4
#define HDMI_PLLMETER_ENABLE (1 << 10)
#define HDMI_PLLMETER_RESULT (0xFFFF << 16)

#define TOP_GEN6_HEADER 0x200
#define TOP_GEN6_PKT00 0x204

#define TOP_GEN7_HEADER 0x224
#define TOP_GEN7_PKT00 0x228

#define TOP_GEN8_HEADER 0x248
#define TOP_GEN8_PKT00 0x24c

#define TOP_GEN9_HEADER 0x26c
#define TOP_GEN9_PKT00 0x270

#define TOP_GEN10_HEADER 0x290
#define TOP_GEN10_PKT00 0x294

#define TOP_GEN11_HEADER 0x2b4
#define TOP_GEN11_PKT00 0x2b8

#define TOP_GEN12_HEADER 0x2d8
#define TOP_GEN12_PKT00 0x2dc

#define TOP_GEN13_HEADER 0x2fc
#define TOP_GEN13_PKT00 0x300

#define TOP_GEN14_HEADER 0x320
#define TOP_GEN14_PKT00 0x324

#define TOP_GEN15_HEADER 0x344
#define TOP_GEN15_PKT00 0x348

#define TOP_INFO_EN_EXPAND 0x368

#define GEN15_EN_WR (0x1 << 22)
#define GEN15_DIS_WR (0x0 << 22)
#define GEN14_EN_WR (0x1 << 21)
#define GEN14_DIS_WR (0x0 << 21)
#define GEN13_EN_WR (0x1 << 20)
#define GEN13_DIS_WR (0x0 << 20)
#define GEN12_EN_WR (0x1 << 19)
#define GEN12_DIS_WR (0x0 << 19)
#define GEN11_EN_WR (0x1 << 18)
#define GEN11_DIS_WR (0x0 << 18)
#define GEN10_EN_WR (0x1 << 17)
#define GEN10_DIS_WR (0x0 << 17)
#define GEN9_EN_WR (0x1 << 16)
#define GEN9_DIS_WR (0x1 << 16)
#define GEN15_EN (0x1 << 6)
#define GEN15_DIS (0x0 << 6)
#define GEN14_EN (0x1 << 5)
#define GEN14_DIS (0x0 << 5)
#define GEN13_EN (0x1 << 4)
#define GEN13_DIS (0x0 << 4)
#define GEN12_EN (0x1 << 3)
#define GEN12_DIS (0x0 << 3)
#define GEN11_EN (0x1 << 2)
#define GEN11_DIS (0x0 << 2)
#define GEN10_EN (0x1 << 1)
#define GEN10_DIS (0x0 << 1)
#define GEN9_EN (0x1 << 0)
#define GEN9_DIS (0x0 << 0)

#define TOP_VSYNC_SEND 0x370
#define MAX_HSYNC_NUM (0xFF << 16)
#define MIN_HSYNC_NUM (0xFF)

typedef enum {
	GEN_PKT_HW1,
	GEN_PKT_HW2,
	GEN_PKT_HW3,
	GEN_PKT_HW4,
	GEN_PKT_HW5,
	GEN_PKT_HW6,
	GEN_PKT_HW7,
	GEN_PKT_HW8,
	GEN_PKT_HW9,
	GEN_PKT_HW10,
	GEN_PKT_HW11,
	GEN_PKT_HW12,
	GEN_PKT_HW13,
	GEN_PKT_HW14,
	GEN_PKT_HW15,
	GEN_PKT_HW_NUM,
} HDMI_GEN_PACKET_HW_ENUM;

/****************HDMI TX digital register end*******************************/


/*****************HDMI audio register start*************************/
#define AIP_CTRL 0x400

#define SPDIF_INTERNAL_MODULE (1 << 24)
#define CTS_CAL_N4 (1 << 23)
#define CTS_CAL_N2 (1 << 22)
#define RESET_NEW_SPDIF (1 << 21)
#define HBR_FROM_SPDIF (1 << 20)
#define I2S_EN (0xF << 16)
#define I2S_EN_SHIFT 16
#define DSD_EN (1 << 15)
#define HBRA_ON (1 << 14)
#define HBRA_OFF (0 << 14)
#define SPDIF_EN (1 << 13)
#define SPDIF_EN_SHIFT 13
#define SPDIF_DIS (0 << 13)
#define AUD_PKT_EN (1 << 12)
#define AUD_PAR_EN (1 << 10)
#define AUD_SEL_OWRT (1 << 9)
#define AUD_IN_EN (1 << 8)
#define AUD_IN_EN_SHIFT 8
#define FM_IN_VAL_SW (0x7 << 4)
#define MCLK_128FS 0x0
#define MCLK_256FS 0x1
#define MCLK_384FS 0x2
#define MCLK_512FS 0x3
#define MCLK_768FS 0x4
#define MCLK_1024FS 0x5
#define MCLK_1152FS 0x6
#define MCLK_192FS 0x7
#define NO_MCLK_CTSGEN_SEL (1 << 3)
#define MCLK_CTSGEN_SEL (0 << 3)
#define MCLK_EN (1 << 2)
#define CTS_REQ_EN (1 << 1)
#define CTS_REQ_DIS (0 << 1)
#define CTS_SW_SEL (1 << 0)
#define CTS_HW_SEL (0 << 0)

/***********************************************/
#define AIP_N_VAL 0x404

#define N_VAL_SW (0xFFFFF << 0)

/***********************************************/
#define AIP_CTS_SVAL 0x408

#define CTS_VAL_SW (0xFFFFF << 0)

/***********************************************/
#define AIP_SPDIF_CTRL 0x40C

#define I2S2DSD_EN (1 << 30)
#define AUD_ERR_THRESH (0x3F << 24)
#define AUD_ERR_THRESH_SHIFT 24

#define MAX_2UI_WRITE (0xFF << 16)
#define MAX_2UI_WRITE_SHIFT 16

#define MAX_1UI_WRITE (0xFF << 8)
#define MAX_1UI_WRITE_SHIFT 8

#define WR_2UI_LOCK (1 << 2)
#define WR_2UI_UNLOCK (0 << 2)
#define FS_OVERRIDE_WRITE (1 << 1)
#define FS_UNOVERRIDE (0 << 1)
#define WR_1UI_LOCK (1 << 0)
#define WR_1UI_UNLOCK (0 << 0)

/***********************************************/
#define AIP_I2S_CTRL 0x410

#define DSD_INTERLEAVE (0xFF << 20)
#define I2S_IN_LENGTH (0xF << 16)
#define I2S_IN_LENGTH_SHIFT 16
#define I2S_LENGTH_24BITS 0xB
#define I2S_LENGTH_23BITS 0x9
#define I2S_LENGTH_22BITS 0x5
#define I2S_LENGTH_21BITS 0xD
#define I2S_LENGTH_20BITS 0xA
#define I2S_LENGTH_19BITS 0x8
#define I2S_LENGTH_18BITS 0x4
#define I2S_LENGTH_17BITS 0xC
#define I2S_LENGTH_16BITS 0x2
#define SCK_EDGE_RISE (0x1 << 14)
#define SCK_EDGE_FALL (0x0 << 14)
#define CBIT_ORDER_SAME (0x1 << 13)
#define CBIT_ORDER_CON (0x0 << 13)
#define VBIT_PCM (0x0 << 12)
#define VBIT_COM (0x1 << 12)
#define WS_LOW (0x0 << 11)
#define WS_HIGH (0x1 << 11)
#define JUSTIFY_RIGHT (0x1 << 10)
#define JUSTIFY_LEFT (0x0 << 10)
#define DATA_DIR_MSB (0x0 << 9)
#define DATA_DIR_LSB (0x1 << 9)
#define I2S_1ST_BIT_SHIFT (0x0 << 8)
#define I2S_1ST_BIT_NOSHIFT (0x1 << 8)
#define FIFO3_MAP (0x3 << 6)
#define FIFO2_MAP (0x3 << 4)
#define FIFO1_MAP (0x3 << 2)
#define FIFO0_MAP (0x3 << 0)
#define MAP_SD0 0x0
#define MAP_SD1 0x1
#define MAP_SD2 0x2
#define MAP_SD3 0x3

/***********************************************/
#define AIP_I2S_CHST0 0x414

#define CBIT3B (0xF << 28)
#define CBIT3A (0xF << 24)
#define CBIT2B (0xF << 20)
#define CBIT2A (0xF << 16)
#define FS_2205KHZ 0x4
#define FS_441KHZ 0x0
#define FS_882KHZ 0x8
#define FS_1764KHZ 0xC
#define FS_24KHZ 0x6
#define FS_48KHZ 0x2
#define FS_96KHZ 0xA
#define FS_192KHZ 0xE
#define FS_32KHZ 0x3
#define FS_768KHZ 0x9
#define CBIT1 (0xFF << 8)
#define CBIT0 (0xFF << 0)

/***********************************************/
#define AIP_I2S_CHST1 0x418

#define CBIT_MSB (0xFFFF << 8)
#define CBIT4B (0xF << 4)
#define CBIT4A (0xF << 0)
#define NOT_INDICATE 0x0
#define BITS20_16 0x1
#define BITS22_18 0x2
#define BITS23_19 0x4
#define BITS24_20 0x5
#define BITS22_17 0x6

/***********************************************/
#define AIP_DOWNSAMPLE_CTRL 0x41C

/***********************************************/
#define AIP_PAR_CTRL 0x420

/***********************************************/
#define AIP_TXCTRL 0x424

#define DSD_MUTE_DATA (0x1 << 7)
#define AUD_PACKET_DROP (0x1 << 6)
#define AUD_MUTE_FIFO_EN (0x1 << 5)
#define AUD_MUTE_DIS (0x0 << 5)
#define LAYOUT0 (0x0 << 4)
#define LAYOUT1 (0x1 << 4)
#define RST4AUDIO_ACR (0x1 << 2)
#define RST4AUDIO_FIFO (0x1 << 1)
#define RST4AUDIO (0x1 << 0)

/***********************************************/
#define AIP_TPI_CTRL 0x428

#define TPI_AUD_SF_OVRD_VALUE (0x1 << 15)
#define TPI_AUD_SF_OVRD_STREAM (0x0 << 15)
#define TPI_AUD_SF (0x3F << 8)
#define SF2205 0x4
#define SF441 0x0
#define SF882 0x8
#define SF1764 0xC
#define SF3528 0xD
#define SF7056 0x2D
#define SF14112 0x1D
#define SF24 0x6
#define SF48 0x2
#define SF96 0xA
#define SF192 0xE
#define SF384 0x5
#define SF768 0x9
#define SF1536 0x15
#define SF32 0x3
#define SF64 0xB
#define SF128 0x2B
#define SF256 0x1B
#define SF512 0x3B
#define SF1024 0x35
#define TPI_SPDIF_SAMPLE_SIZE (0x3 << 6)
#define REFER_HEADER 0x0
#define SAMPLE_SIZE_BIT16 0x1
#define SAMPLE_SIZE_BIT20 0x2
#define SAMPLE_SIZE_BIT24 0x3
#define TPI_AUD_MUTE (0x1 << 4)
#define TPI_AUD_UNMUTE (0x0 << 4)
#define TPI_AUDIO_LOOKUP_EN (0x1 << 2)
#define TPI_AUDIO_LOOKUP_DIS (0x0 << 2)
#define TPI_AUD_HNDL (0x3 << 0)
#define BLOCK_AUDIO 0x0
#define DOWNSAMPLE 0x2
#define PASSAUDIO 0x3

/***********************************************/
#define AIP_INT_CTRL 0x42C
#define BURST_PREAMBLE_RST (0x01 << 15)

#define DSD_VALID_INT_MSK (0x1 << 24)
#define P_ERR_INT_MSK (0x1 << 23)
#define PREAMBLE_ERR_INT_MSK (0x1 << 22)
#define HW_CTS_CHANGED_INT_MSK (0x1 << 21)
#define PKT_OVRWRT_INT_MSK (0x1 << 20)
#define DROP_SMPL_ERR_INT_MSK (0x1 << 19)
#define BIPHASE_ERR_INT_MSK (0x1 << 18)
#define UNDERRUN_INT_MSK (0x1 << 17)
#define OVERRUN_INT_MSK (0x1 << 16)
#define DSD_VALID_INT_CLR (0x1 << 8)
#define P_ERR_INT_CLR (0x1 << 7)
#define PREAMBLE_ERR_INT_CLR (0x1 << 6)
#define HW_CTS_CHANGED_INT_CLR (0x1 << 5)
#define PKT_OVRWRT_INT_CLR (0x1 << 4)
#define DROP_SMPL_ERR_INT_CLR (0x1 << 3)
#define BIPHASE_ERR_INT_CLR (0x1 << 2)
#define UNDERRUN_INT_CLR (0x1 << 1)
#define OVERRUN_INT_CLR (0x1 << 0)
#define DSD_VALID_INT_UNMSK (0x0 << 24)
#define P_ERR_INT_UNMSK (0x0 << 23)
#define PREAMBLE_ERR_INT_UNMSK (0x0 << 22)
#define HW_CTS_CHANGED_INT_UNMSK (0x0 << 21)
#define PKT_OVRWRT_INT_UNMSK (0x0 << 20)
#define DROP_SMPL_ERR_INT_UNMSK (0x0 << 19)
#define BIPHASE_ERR_INT_UNMSK (0x0 << 18)
#define UNDERRUN_INT_UNMSK (0x0 << 17)
#define OVERRUN_INT_UNMSK (0x0 << 16)
#define DSD_VALID_INT_UNCLR (0x0 << 8)
#define P_ERR_INT_UNCLR (0x0 << 7)
#define PREAMBLE_ERR_INT_UNCLR (0x0 << 6)
#define HW_CTS_CHANGED_INT_UNCLR (0x0 << 5)
#define PKT_OVRWRT_INT_UNCLR (0x0 << 4)
#define DROP_SMPL_ERR_INT_UNCLR (0x0 << 3)
#define BIPHASE_ERR_INT_UNCLR (0x0 << 2)
#define UNDERRUN_INT_UNCLR (0x0 << 1)
#define OVERRUN_INT_UNCLR (0x0 << 0)

/***********************************************/
#define AIP_STA00 0x430

#define AUD_NO_AUDIO (0x1 << 29)
#define RO_2UI_LOCK (0x1 << 28)
#define FS_OVERRIDE_READ (0x1 << 27)
#define RO_1UI_LOCK (0x1 << 26)
#define AUD_ID (0x3F << 20)
#define N_VAL_SW (0xFFFFF << 0)

/***********************************************/
#define AIP_STA01 0x434

#define AUDIO_SPDIF_FS (0x3F << 24)
#define AUDIO_LENGTH (0xF << 20)
#define CTS_VAL_HW (0xFFFFF << 0)

/***********************************************/
#define AIP_STA02 0x438

#define FIFO_DIFF (0x3F << 24)
#define CBIT_L (0xFF << 16)
#define MAX_2UI_READ (0xFF << 8)
#define MAX_1UI_READ (0xFF << 0)

/***********************************************/
#define AIP_STA03 0x43C

#define SRC_EN (0x1 << 20)
#define SRC_CTRL (0x1 << 19)
#define LAYOUT (0x1 << 18)
#define AUD_MUTE_EN (0x1 << 17)
#define HDMI_MUTE (0x1 << 16)
#define DSD_VALID_INT_STA (0x1 << 8)
#define P_ERR_INT_STA (0x1 << 7)
#define PREAMBLE_ERR_INT_STA (0x1 << 6)
#define HW_CTS_CHANGED_INT_STA (0x1 << 5)
#define PKT_OVRWRT_INT_STA (0x1 << 4)
#define DROP_SMPL_ERR_INT_STA (0x1 << 3)
#define BIPHASE_ERR_INT_STA (0x1 << 2)
#define UNDERRUN_INT_STA (0x1 << 1)
#define OVERRUN_INT_STA (0x1 << 0)
/*********************HDMI audio register end**************************/

/*********************HDMI video register start**************************/

#define VID_CSC_COEF_0 0x8D0

#define MULTI_CSC_MULTCOEFFR1C1 (0xffff << 16)
#define MULTI_CSC_CONFIG_DITHER_ENABLE (0x1 << 10)
#define MULTI_CSC_CONFIG_IN_RGB (0x1 << 9)
#define MULTI_CSC_CONFIG_IN_PC (0x1 << 8)
#define MULTI_CSC_CONFIG_IN_SD (0x3 << 6)
#define BT_709_IN (0x0 << 6)
#define BT_601_IN (0x1 << 6)
#define BT_NON2020_IN (0x2 << 6)
#define BT_2020_IN (0x3 << 6)
#define MULTI_CSC_CONFIG_OUT_RGB (0x1 << 5)
#define MULTI_CSC_CONFIG_OUT_PC (0x1 << 4)
#define MULTI_CSC_CONFIG_OUT_SD (0x3 << 2)
#define BT_709_OUT (0x0 << 2)
#define BT_601_OUT (0x1 << 2)
#define BT_NON2020_OUT (0x2 << 2)
#define BT_2020_OUT (0x3 << 2)
#define MULTI_CSC_CONFIG_ENABLE (0x3 << 0)
#define CONVERSION_DISABLED (0x0 << 0)
#define CONVERSION_ENABLED_BY_IN_OUT (0x1 << 0)
#define CONVERSION_ENABLED_BY_PROGRAM (0x2 << 0)
#define CONVERSION_RESERVED (0x3 << 0)

/***********************************************/
#define VID_DOWNSAMPLE_CONFIG 0x8F0

#define RANGE_CLIP_CONFIG (0x1 << 16)
#define DITHER_CONFIG_SPATIAL_DUAL (0x1 << 12)
#define DITHER_CONFIG_SPATIAL_ENABLE (0x1 << 11)
#define DITHER_CONFIG_RND_ENABLE (0x1 << 10)
#define DITHER_CONFIG_MODE (0x3 << 8)
#define D_12_TO_10 (0x0 << 8)
#define D_12_TO_8 (0x1 << 8)
#define D_10_TO_8 (0x2 << 8)
#define D_NO_CHANGE (0x3 << 8)
#define C422_C420_CONFIG_OUT_CB_CR_POLARITY (0x1 << 7)
#define C422_C420_CONFIG_OUT_CB_OR_CR (0x1 << 6)
#define C422_C420_CONFIG_BYPASS (0x1 << 5)
#define C422_C420_CONFIG_ENABLE (0x1 << 4)
#define C444_C422_CONFIG_DISABLE_FILTER (0x1 << 2)
#define C444_C422_CONFIG_ENABLE (0x1 << 0)
/***********************************************/
#define VID_OUT_FORMAT 0x8FC

#define OUTPUT_FORMAT_DEMUX_CB_OR_CR (0x1 << 11)
#define OUTPUT_FORMAT_DEMUX_420_ENABLE (0x1 << 10)
#define OUTPUT_FORMAT_PIXEL_RATE (0x3 << 8)
#define OUTPUT_FORMAT_CBCR_ORDER (0x1 << 7)
#define OUTPUT_FORMAT_YC_MUX_POLARITY (0x1 << 6)
#define OUTPUT_FORMAT_YC_MUX_ENABLE (0x1 << 5)
#define OUTPUT_FORMAT_DDR_MODE (0x7 << 2)
#define OUTPUT_FORMAT_DDR_POLARITY (0x1 << 1)
#define OUTPUT_FORMAT_DDR_ENABLE (0x1 << 0)

/*********************HDMI video register end**************************/

/*********************HDMI HDCP register start**************************/

#define HDCP_TOP_CTRL 0xC00

#define OTP2XAOVR_EN (0x1 << 13)
#define OTP2XVOVR_EN (0x1 << 12)
#define OTPAMUTEOVR_SET (0x1 << 10)
#define OTPADROPOVR_SET (0x1 << 9)
#define OTPVMUTEOVR_SET (0x1 << 8)
#define OTP14AOVR_EN (0x1 << 5)
#define OTP14VOVR_EN (0x1 << 4)
#define HDCP_DISABLE (0x1 << 0)
#define OTP2XAOVR_DIS (0x0 << 13)
#define OTP2XVOVR_DIS (0x0 << 12)
#define OTPAMUTEOVR_UNSET (0x0 << 10)
#define OTPADROPOVR_UNSET (0x0 << 9)
#define OTPVMUTEOVR_UNSET (0x0 << 8)
#define OTP14AOVR_DIS (0x0 << 5)
#define OTP14VOVR_DIS (0x0 << 4)
#define HDCP_ENABLE (0x0 << 0)

/***********************************************/
#define HPD_PORD_CTRL 0xC04

#define PORD_T2 (0xFF << 24)
#define PORD_T1 (0xFF << 16)
#define HPD_T2 (0xFF << 8)
#define HPD_T1 (0xFF << 0)

/***********************************************/
#define HPD_DDC_CTRL 0xC08

#define DDC_DELAY_CNT (0xFFFF << 16)
#define DDC_DELAY_CNT_SHIFT (16)

#define HW_DDC_MASTER (0x1 << 14)
#define DDC_DEBUG (0x1 << 13)
#define MAN_DDC_SYNC (0x1 << 12)
#define DSDA_SYNC (0x1 << 11)
#define DSCL_SYNC (0x1 << 10)
#define TPI_DDC_REQ_LEVEL (0x3 << 8)
#define DDC_GPU_REQUEST (0x1 << 7)
#define TPI_DDC_BURST_MODE (0x1 << 6)
#define DDC_SHORT_RI_RD (0x1 << 5)
#define DDC_FLT_EN_SYNC (0x1 << 4)
#define PORD_DEBOUNCE_EN (0x1 << 3)
#define HPD_DEBOUNCE_EN (0x1 << 2)
#define HPDIN_STA (0x1 << 1)
#define HPDIN_OVER_EN (0x1 << 0)

/***********************************************/
#define HDCP_RI_CTRL 0xC0C

#define RI_BCAP_EN (0x1 << 29)
#define RI_EN (0x1 << 28)
#define RI_128_COMP (0x7F << 20)
#define KSV_FORWARD (0x1 << 19)
#define INTERM_RI_CHECK_EN (0x1 << 18)
#define R0_ABSOLUTE  (0x1 << 17)
#define DOUBLE_RI_CHECK (0x1 << 16)
#define TPI_R0_CALC_TIME (0xF << 12)
#define RI_CHECK_SHIP (0x1 << 11)
#define LEGACY_TPI_RI_CHECK (0x7 << 8)
#define RI_LN_NUM (0xFF << 0)

/***********************************************/
#define DDC_CTRL 0xC10

#define DDC_CMD (0xF << 28)
#define DDC_CMD_SHIFT (28)

#define ABORT_TRANSACTION 0xF
#define CLEAR_FIFO 0x9
#define CLOCK_SCL 0xA
#define CURR_READ_NO_ACK 0x0
#define CURR_READ_ACK 0x1
#define SEQ_READ_NO_ACK 0x2
#define SEQ_READ_ACK 0x3
#define ENH_READ_NO_ACK 0x4
#define ENH_READ_ACK 0x5
#define SEQ_WRITE_IGN_ACK 0x6
#define SEQ_WRITE_REQ_ACK 0x7
#define DDC_DIN_CNT (0x3FF << 16)
#define DDC_DIN_CNT_SHIFT (16)

#define DDC_OFFSET (0xFF << 8)
#define DDC_OFFSET_SHIFT (8)

#define DDC_ADDR (0x7F << 1)

/***********************************************/
#define HDCP_TPI_CTRL 0xC14

#define EDID_MODE_EN (0x1 << 8)
#define CANCEL_PROT_EN (0x1 << 7)
#define TPI_AUTH_RETRY_CNT (0x7 << 4)
#define COPP_PROTLEVEL (0x1 << 3)
#define TPI_REAUTH_CTL (0x1 << 2)
#define TPI_HDCP_PREP_EN (0x1 << 1)
#define SW_TPI_EN (0x1 << 0)

/***********************************************/
#define SCDC_CTRL 0xC18

#define DDC_SEGMENT (0xFF << 8)
#define DDC_SEGMENT_SHIFT (8)

#define SCDC_AUTO_REPLY_STOP (0x1 << 3)
#define SCDC_AUTO_POLL (0x1 << 2)
#define SCDC_AUTO_REPLY (0x1 << 1)
#define SCDC_ACCESS (0x1 << 0)

/***********************************************/
#define TXDS_BSTATUS 0xC1C

#define DS_BSTATUS (0x7 << 13)
#define DS_HDMI_MODE (0x1 << 12)
#define DS_CASC_EXCEED (0x1 << 11)
#define DS_DEPTH (0x7 << 8)
#define DS_DEV_EXCEED (0x1 << 7)
#define DS_DEV_CNT (0x7F << 0)

/***********************************************/
#define HDCP2X_CTRL_0 0xC20

#define HDCP2X_CPVER (0xF << 20)
#define HDCP2X_CUPD_START (0x1 << 16)
#define HDCP2X_REAUTH_MSK (0xF << 12)
#define HDCP2X_REAUTH_MSK_SHIFT (12)

#define HDCP2X_HPD_SW (0x1 << 11)
#define HDCP2X_HPD_OVR (0x1 << 10)
#define HDCP2X_CTL3MSK (0x1 << 9)
#define HDCP2X_REAUTH_SW (0x1 << 8)
#define HDCP2X_ENCRYPT_EN (0x1 << 7)
#define HDCP2X_POLINT_SEL (0x1 << 6)
#define HDCP2X_POLINT_OVR (0x1 << 5)
#define HDCP2X_PRECOMPUTE (0x1 << 4)
#define HDCP2X_HDMIMODE (0x1 << 3)
#define HDCP2X_REPEATER (0x1 << 2)
#define HDCP2X_HDCPTX (0x1 << 1)
#define HDCP2X_EN (0x1 << 0)

/***********************************************/
#define HDCP2X_CTRL_1 0xC24

#define HDCP2X_CUPD_SIZE (0xFFFF << 0)

/***********************************************/
#define HDCP2X_CTRL_2 0xC28

#define HDCP2X_RINGOSC_BIST_START (0x1 << 28)
#define HDCP2X_MSG_SZ_CLR_OPTION (0x1 << 26)
#define HDCP2X_RPT_READY_CLR_OPTION (0x1 << 25)
#define HDCP2X_REAUTH_REQ_CLR_OPTION (0x1 << 24)
#define HDCP2X_RPT_SMNG_IN (0xFF << 16)
#define HDCP2X_RPT_SMNG_K (0xFF << 8)
#define HDCP2X_RPT_SMNG_XFER_START (0x1 << 4)
#define HDCP2X_RPT_SMNG_WR_START (0x1 << 3)
#define HDCP2X_RPT_SMNG_WR (0x1 << 2)
#define HDCP2X_RPT_RCVID_RD_START (0x1 << 1)
#define HDCP2X_RPT_RCVID_RD (0x1 << 0)

#define HDCP2X_RPT_SMNG_K_SHIFT (8)
#define HDCP2X_RPT_SMNG_IN_SHIFT (16)

/***********************************************/
#define HDCP2X_CTRL_STM 0xC2C

#define HDCP2X_STM_CTR (0xFFFFFFFF << 0)

/***********************************************/
#define HDCP2X_TEST_TP0 0xC30

#define HDCP2X_TP3 (0xFF << 24)
#define HDCP2X_TP2 (0xFF << 16)
#define HDCP2X_TP1 (0xFF << 8)
#define HDCP2X_TP0 (0xFF << 0)
#define HDCP2X_TP3_SHIFT (24)
#define HDCP2X_TP2_SHIFT (16)
#define HDCP2X_TP1_SHIFT (8)
#define HDCP2X_TP0_SHIFT (0)
/***********************************************/
#define HDCP2X_TEST_TP1 0xC34

#define HDCP2X_TP7 (0xFF << 24)
#define HDCP2X_TP6 (0xFF << 16)
#define HDCP2X_TP5 (0xFF << 8)
#define HDCP2X_TP4 (0xFF << 0)

/***********************************************/
#define HDCP2X_TEST_TP2 0xC38

#define HDCP2X_TP11 (0xFF << 24)
#define HDCP2X_TP10 (0xFF << 16)
#define HDCP2X_TP9 (0xFF << 8)
#define HDCP2X_TP8 (0xFF << 0)

/***********************************************/
#define HDCP2X_TEST_TP3 0xC3C

#define HDCP2X_TP15 (0xFF << 24)
#define HDCP2X_TP14 (0xFF << 16)
#define HDCP2X_TP13 (0xFF << 8)
#define HDCP2X_TP12 (0xFF << 0)

/***********************************************/
#define HDCP2X_GP_IN 0xC40

#define HDCP2X_GP_IN3 (0xFF << 24)
#define HDCP2X_GP_IN2 (0xFF << 16)
#define HDCP2X_GP_IN1 (0xFF << 8)
#define HDCP2X_GP_IN0 (0xFF << 0)
#define HDCP2X_GP_IN3_SHIFT (24)
#define HDCP2X_GP_IN2_SHIFT (16)
#define HDCP2X_GP_IN1_SHIFT (8)
#define HDCP2X_GP_IN0_SHIFT (0)
/***********************************************/
#define HDCP2X_DEBUG_CTRL 0xC44

#define HDCP2X_DB_CTRL3 (0xFF << 24)
#define HDCP2X_DB_CTRL2 (0xFF << 16)
#define HDCP2X_DB_CTRL1 (0xFF << 8)
#define HDCP2X_DB_CTRL0 (0xFF << 0)

/***********************************************/
#define HDCP2X_SPI_SI_ADDR 0xC48

#define HDCP2X_SPI_SI_S_ADDR (0xFFFF << 16)
#define HDCP2X_SPI_SI_E_ADDR (0xFFFF << 0)

/***********************************************/
#define HDCP2X_SPI_ADDR_CTRL 0xC4C

#define HDCP2X_SPI_SI_S_ADDR (0xFFFF << 16)
#define HDCP2X_SPI_SI_E_ADDR (0xFFFF << 0)

/***********************************************/
#define HDCP2X_RPT_SEQ_NUM 0xC50

#define MHL3_P0_STM_ID (0x7 << 24)
#define HDCP2X_RPT_SEQ_NUM_M (0xFFFFFF << 0)

/***********************************************/
#define HDCP2X_POL_CTRL 0xC54

#define HDCP2X_DIS_POLL_EN (0x1 << 16)
#define HDCP2X_POL_VAL1 (0xFF << 8)
#define HDCP2X_POL_VAL0 (0xFF << 0)

/***********************************************/
#define WR_PULSE 0xC58

#define HDCP2X_SPI_ADDR_RESET (0x1 << 3)
#define DDC_BUS_LOW (0x1 << 2)
#define DDC_NO_ACK (0x1 << 1)
#define DDCM_ABORT (0x1 << 0)

/***********************************************/
#define HPD_DDC_STATUS 0xC60

#define DDC_DATA_OUT (0xFF << 16)
#define DDC_DATA_OUT_SHIFT (16)

#define DDC_FIFO_FULL (0x1 << 15)
#define DDC_FIFO_EMPTY (0x1 << 14)
#define DDC_I2C_IN_PROG (0x1 << 13)
#define DDC_DATA_OUT_CNT (0x1F << 8)
#define RX_HDCP2_CAP_EN (0x1 << 7)
#define HDCP2_AUTH_RXCAP_FAIL (0x1 << 6)
#define PORD_PIN_STA (0x1 << 5)
#define HPD_PIN_STA (0x1 << 4)
#define PORD_STATE (0x3 << 2)
#define PORD_STATE_SHIFT (2)
#define PORD_STATE_CONNECTED (2)
#define PORD_STATE_DISCONNECTED (0)
#define HPD_STATE (0x3 << 0)
#define HPD_STATE_SHIFT (0)
#define HPD_STATE_CONNECTED (2)
#define HPD_STATE_DISCONNECTED (0)

/***********************************************/
#define SCDC_STATUS 0xC64

#define SCDC_STATE (0xF << 28)
#define SCDC_RREQ_STATE (0xF << 24)
#define SCDC_UP_FLAG1_STATUS (0xFF << 16)
#define SCDC_UP_FLAG0_STATUS (0xFF << 8)
#define SCDC_UP_FLAG_DONE (0x1 << 6)
#define SCDC_SLAVE_RD_REQ_P (0x1 << 5)
#define SCDC_DDC_CONFLICT (0x1 << 4)
#define SCDC_DDC_DONE (0x1 << 3)
#define SCDC_IN_PROG (0x1 << 2)
#define SCDC_RREQ_IN_PROG (0x1 << 1)
#define SCDC_ACTIVE (0x1 << 0)

/***********************************************/
#define HDCP2X_DDCM_STATUS 0xC68

#define RI_ON (0x1 << 31)
#define HDCP_I_CNT (0x7F << 24)
#define HDCP_RI_RDY (0x1 << 23)
#define KSV_FIFO_FIRST (0x1 << 22)
#define KSV_FIFO_LAST (0x1 << 21)
#define KSV_FIFO_BYTES (0x1F << 16)
#define HDCP2X_DDCM_CTL_CS (0xF << 12)
#define DDC_I2C_BUS_LOW	(0x1 << 11)
#define DDC_I2C_NO_ACK (0x1 << 10)
#define DDC_FIFO_WRITE_IN_USE (0x1 << 9)
#define DDC_FIFO_READ_IN_USE (0x1 << 8)
#define HDCP1_DDC_TPI_GRANT (0x1 << 6)
#define HDCP2X_DDCM_AUTH_POLL_ERR (0x1 << 5)
#define HDCP2X_DDCM_RCV_FAIL (0x1 << 4)
#define HDCP2X_DDCM_SND_FAIL (0x1 << 3)
#define HDCP2X_DDCM_AUTH_ERR (0x1 << 2)
#define HDCP2_DDC_TPI_GRANT (0x1 << 1)
#define HDCP2X_DIS_POLL_GNT (0x1 << 0)

/***********************************************/
#define TPI_STATUS_0 0xC6C

#define TPI_AUTH_STATE (0x3 << 30)
#define TPI_COPP_LINK_STATUS (0x3 << 28)
#define TPI_COPP_GPROT (0x1 << 27)
#define TPI_COPP_LPROT (0x1 << 26)
#define TPI_COPP_HDCP_REP (0x1 << 25)
#define TPI_COPP_PROTYPE (0x1 << 24)
#define TPI_RI_PRIME1 (0xFF << 16)
#define TPI_RI_PRIME0 (0xFF << 8)
#define DS_BKSV (0xFF << 0)

/***********************************************/
#define TPI_STATUS_1 0xC70

#define TPI_READ_V_PRIME_ERR (0x1 << 26)
#define TPI_READ_RI_PRIME_ERR (0x1 << 25)
#define TPI_READ_RI_2ND_ERR (0x1 << 24)
#define TPI_READ_R0_PRIME_ERR (0x1 << 23)
#define TPI_READ_KSV_FIFO_RDY_ERR (0x1 << 22)
#define TPI_READ_BSTATUS_ERR (0x1 << 21)
#define TPI_READ_KSV_LIST_ERR (0x1 << 20)
#define TPI_WRITE_AKSV_ERR (0x1 << 19)
#define TPI_WRITE_AN_ERR (0x1 << 18)
#define TPI_READ_RX_REPEATER_ERR (0x1 << 17)
#define TPI_READ_BKSV_ERR (0x1 << 16)
#define TPI_READ_V_PRIME_DONE (0x1 << 10)
#define TPI_READ_RI_PRIME_DONE (0x1 << 9)
#define TPI_READ_RI_2ND_DONE (0x1 << 8)
#define TPI_READ_R0_PRIME_DONE (0x1 << 7)
#define TPI_READ_KSV_FIFO_RDY_DONE (0x1 << 6)
#define TPI_READ_BSTATUS_DONE (0x1 << 5)
#define TPI_READ_KSV_LIST_DONE (0x1 << 4)
#define TPI_WRITE_AKSV_DONE (0x1 << 3)
#define TPI_WRITE_AN_DONE (0x1 << 2)
#define TPI_READ_RX_REPEATER_DONE (0x1 << 1)
#define TPI_READ_BKSV_DONE (0x1 << 0)

/***********************************************/
#define TPI_STATUS_FSM 0xC74

#define DDC_HDCP_ACC_NMB (0x3FF << 20)
#define TPI_DDCM_CTL_CS (0xF << 16)
#define TPI_DS_AUTH_CS (0xF << 12)
#define TPI_LINK_ENC_CS (0x7 << 9)
#define TPI_RX_AUTH_CS (0x1F << 4)
#define TPI_HW_CS (0xF << 0)

/***********************************************/
#define KSV_RI_STATUS 0xC78

#define RI_RX (0xFFFF << 16)
#define RI_TX (0xFFFF << 0)

/***********************************************/
#define HDCP2X_DEBUG_STATUS_0 0xC7C

#define HDCP2X_DEBUG_STAT3 (0xFF << 24)
#define HDCP2X_DEBUG_STAT2 (0xFF << 16)
#define HDCP2X_DEBUG_STAT1 (0xFF << 8)
#define HDCP2X_DEBUG_STAT0 (0xFF << 0)

/***********************************************/
#define HDCP2X_DEBUG_STATUS_1 0xC80

#define HDCP2X_DEBUG_STAT7 (0xFF << 24)
#define HDCP2X_DEBUG_STAT6 (0xFF << 16)
#define HDCP2X_DEBUG_STAT5 (0xFF << 8)
#define HDCP2X_DEBUG_STAT4 (0xFF << 0)

/***********************************************/
#define HDCP2X_DEBUG_STATUS_2 0xC84

#define HDCP2X_DEBUG_STAT11 (0xFF << 24)
#define HDCP2X_DEBUG_STAT10 (0xFF << 16)
#define HDCP2X_DEBUG_STAT9 (0xFF << 8)
#define HDCP2X_DEBUG_STAT8 (0xFF << 0)

/***********************************************/
#define HDCP2X_DEBUG_STATUS_3 0xC88

#define HDCP2X_DEBUG_STAT15 (0xFF << 24)
#define HDCP2X_DEBUG_STAT14 (0xFF << 16)
#define HDCP2X_DEBUG_STAT13 (0xFF << 8)
#define HDCP2X_DEBUG_STAT12 (0xFF << 0)

/***********************************************/
#define HDCP2X_STATUS_0 0xC8C

#define HDCP2X_AUTH_STAT (0xFF << 24)
#define HDCP2X_STATE (0xFF << 16)
#define HDCP2X_RPT_REPEATER (0x1 << 8)
#define HDCP2X_PRG_SEL (0x1 << 7)
#define HDCP2X_CUPD_DONE (0x1 << 6)
#define HDCP2X_RPT_MX_DEVS_EXC (0x1 << 5)
#define HDCP2X_RPT_MAX_CASC_EXC (0x1 << 4)
#define HDCP2X_RPT_HDCP20RPT_DSTRM (0x1 << 3)
#define HDCP2X_RPT_HDCP1DEV_DSTRM (0x1 << 2)
#define HDCP2X_RPT_RCVID_CHANGED (0x1 << 1)
#define HDCP2X_RPT_SMNG_XFER_DONE (0x1 << 0)

/***********************************************/
#define HDCP2X_STATUS_1 0xC90

#define HDCP2X_RINGOSS_BIST_FAIL (0x1 << 25)
#define HDCP2X_RINGOSC_BIST_DONE (0x1 << 24)
#define HDCP2X_RPT_RCVID_OUT (0xFF << 16)
#define HDCP2X_RPT_DEVCNT (0xFF << 8)
#define HDCP2X_RPT_DEPTH (0xFF << 0)

#define HDCP2X_RPT_DEVCNT_SHIFT (8)
#define HDCP2X_RPT_RCVID_OUT_SHIFT (16)

/***********************************************/
#define HDCP2X_RCVR_ID 0xC94

#define HDCP2X_RCVR_ID_L (0xFFFFFFFF << 0)

/***********************************************/
#define HDCP2X_RPT_SEQ 0xC98

#define HDCP2X_RCVR_ID_H (0xFF << 24)
#define HDCP2X_RPT_SEQ_NUM_V (0xFFFFFF << 0)

/***********************************************/
#define HDCP2X_GP_OUT 0xC9C

#define HDCP2X_GP_OUT3 (0xFF << 24)
#define HDCP2X_GP_OUT2 (0xFF << 16)
#define HDCP2X_GP_OUT1 (0xFF << 8)
#define HDCP2X_GP_OUT0 (0xFF << 0)

/***********************************************/
#define PROM_CTRL 0xCA0

#define PROM_ADDR (0xFFFF << 16)
#define PROM_ADDR_SHIFT (16)

#define PROM_WDATA (0xFF << 8)
#define PROM_WDATA_SHIFT (8)

#define PROM_CS (0x1 << 1)
#define PROM_WR (0x1 << 0)

/***********************************************/
#define PRAM_CTRL 0xCA4

#define PRAM_ADDR (0xFFFF << 16)
#define PRAM_ADDR_SHIFT (16)

#define PRAM_WDATA (0xFF << 8)
#define PRAM_WDATA_SHIFT (8)

#define PRAM_CTRL_SEL (0x1 << 2)
#define PRAM_CS (0x1 << 1)
#define PRAM_WR (0x1 << 0)

/***********************************************/
#define PROM_DATA 0xCA8

#define PROM_RDATA (0xFF << 8)
#define PRAM_RDATA (0xFF << 0)

/***********************************************/
#define SI2C_CTRL 0xCAC

#define SI2C_ADDR (0xFFFF << 16)
#define SI2C_ADDR_SHIFT (16)
#define SI2C_ADDR_READ (0xF4)

#define SI2C_WDATA (0xFF << 8)
#define SI2C_WDATA_SHIFT (8)

#define RX_CAP_RD_TRIG (0x1 << 5)
#define KSV_RD (0x1 << 4)
#define SI2C_STOP (0x1 << 3)
#define SI2C_CONFIRM_READ (0x1 << 2)
#define SI2C_RD (0x1 << 1)
#define SI2C_WR (0x1 << 0)


#define HDCP1X_CTRL 0xCD0

#define HDCP1X_ENC_EN (0x01 << 6)
#define HDCP1X_ENC_EN_SHIFT (6)
#define ANA_TOP (0x01 << 4)
#define ANA_TOP_SHIFT (4)

#define HDCP1x_STATUS 0xCF4

#define HDCP_ENCRYPTING_ON (0x1 << 26)

#define HDCP2X_STATUS_0 0xC8C

#define HDCP2X_STATE (0xFF << 16)
#define HDCP2X_ENCRYPTING_ON (0x1 << 10)

/* hdcp cfg */
#define SLOW_CLK_DIV_CNT (0xFF << 16)
#define RISC_CLK_DDC_RST (0x1 << 15)
#define HDCP_1P4_TCLK_EN (0x1 << 9)
#define HDCP_1P4_TCLK_DIS (0x0 << 9)
#define HDCP_TCLK_EN (0x1 << 8)
#define HDCP_TCLK_DIS (0x0 << 8)
#define SOFT_VIDEO_RST (0x1 << 5)
#define SOFT_VIDEO_NOR (0x0 << 5)
#define SOFT_HDCP_1P4_RST (0x1 << 2)
#define SOFT_HDCP_1P4_NOR (0x0 << 2)
#define SOFT_HDCP_CORE_RST (0x1 << 1)
#define SOFT_HDCP_CORE_NOR (0x0 << 1)
#define SOFT_HDCP_RST (0x1 << 0)
#define SOFT_HDCP_NOR (0x0 << 0)

/*********************HDMI HDCP register end**************************/

/************************HDMI RGB2HDMI REG start*******************************/
#define HDMI_RGB_CTRL       0x1800
#define RGB2HDMI_ON          (1<<0)
#define Y_OFF                (1<<1)
#define RGB_MOD              (1<<2)
#define YCBCR422_CBCR_INV         (1<<3)
#define LPF_RGB_DOMAIN       (1<<4)
#define IN_DELAY_2T          (1<<5)
#define ADJ_SYNC_EN          (1<<6)
#define YCBCR422_REPEAT_4PXL      (1<<7)
#define RGB_FULL_RANGE       (1<<8)
#define HDMI_UV_SWAP         (1<<9)
#define YVU_FULL             (1<<10)
#define UV_OFFSET            (1<<11)

#define YUV_422              (1<<16)
#define Y_LPF_EN             (1<<17)
#define C_LPF_EN             (1<<18)
#define HALF_LPF_EN          (1<<19)
#define REPEAT_EN            (1<<20)
#define DOUBLE_EN            (1<<21)
#define PRGS_CLK54_EN        (1<<23)
#define PRGS_CLK108_EN  (1<<24)
#define LMT_EN  (1<<25)
#define REP_4PXL  (1<<26)
#define OUT_MODE_12BIT   (0<<28)
#define OUT_MODE_8BIT    (1<<28)
#define OUT_MODE_10BIT   (2<<28)
#define OUT_MODE_16BIT   (3<<28)

#define HDMI_RGB_GAMMA0     0x1804
#define HDMI_RGB_GAMMA1     0x1808
#define HDMI_RGB_GAMMA2     0x180c
#define HDMI_RGB_GAMMA3     0x1810
#define HDMI_RGB_GAMMA4	 0x1838
#define HDMI_RGB_CBGAIN     0x1814
#define HDMI_RGB_CRGAIN     0x1818
#define HDMI_RGB_TIME0      0x181c
#define V_POLAR           (0x1 << 24)
#define H_POLAR	    (0x1 << 25)
#define DE_POLAR	    (0x1 << 26)
#define DE_SEL	    (0x1 << 27)
#define HDMI_RGB_TIME1      0x1820
#define HDMI_RGB_TIME2      0x1824
#define HDMI_RGB_TIME3      0x1828
#define HDMI_RGB_TIME4      0x182c
#define HDMI_RGB_YLPF1		0x1834
#define HDMI_RGB2HDMI_4O    0x1840
#define R_DELAY           7
#define G_DELAY		    (7<<4)
#define B_DELAY		    (7<<8)
#define Y_DELAY		    (7<<12)
#define CB_DELAY		    (7<<16)
#define CR_DELAY		    (7<<20)
#define HDMI_RGB2HDMI_44    0x1844
#define DEBUG_MONITOR_SELECT (0xf<<0)
#define HDMI_RGB2HDMI_48    0x1848
#define ODD_V_START_OPT   (0xF)
#define ODD_V_END_OPT	    (0xF << 4)
#define EVEN_V_START_OPT  (0xF << 8)
#define EVEN_V_END_OPT	(0xF << 12)
#define HDMI_RGB2HDMI_4C    0x184C
#define USE_UV_LMT        (1 << 26)
#define Y_GAMMA_OFF	    (1 << 27)
#define CBCR_IN_SIGN	    (1 << 30)
#define HDMI_RGB2HDMI_50    0x1850
#define BOT_LIMIT_MASK (0xffff)
#define TOP_LIMIT_MASK (0xffff << 16)
#define HDMI_RGB2HDMI_54    0x1854
#define BOT_UV_LIMIT_MASK (0xffff)
#define TOP_UV_LIMIT_MASK (0xffff << 16)
#define HDMI_RGB2HDMI_60    0x1860
#define YCBCR2RGB_3X3_ON   (1<<31)
#define HDMI_RGB2HDMI_64    0x1864
#define YCBCR2RGB_601_ORG        (0<<28)	/* 16-235  16-235 */
#define YCBCR2RGB_601_NEW1       (1<<28)	/* 0-255    0-255 */
#define YCBCR2RGB_601_NEW2       (2<<28)	/* 16-235  0-255 */
#define YCBCR2RGB_709_ORG        (4<<28)	/* 16-235  16-235 */
#define YCBCR2RGB_709_NEW1       (5<<28)	/* 0-255    0-255 */
#define YCBCR2RGB_709_NEW2       (6<<28)	/* 16-235  0-255 */
#define YCBCR2RGB_3X3_BY_COEFFICIENTS  (7<<28)
#define HDMI_RGB2HDMI_68    0x1868
#define HDMI_RGB2HDMI_6C    0x186C
#define HDMI_RGB2HDMI_70    0x1870
#define HDMI_RGB2HDMI_74    0x1874
#define HDMI_RGB2HDMI_78    0x1878
#define HDMI_RGB2HDMI_7C    0x187C
#define HDMI_RGB2HDMI_80    0x1880
#define HDMI_RGB2HDMI_84    0x1884
#define HDMI_RGB2HDMI_88    0x1888
#define HDMI_RGB2HDMI_8C    0x188C
#define HDMI_RGB2HDMI_90      0x1890
#define HDMI_RGB2HDMI_B0      0x18B0
#define RGB2HDMI_ADJUST_TOTAL    (1<<31)
#define RGB2HDMI_HSYN_TOTAL_BIT12	(1 << 30)
#define RGB2HDMI_PRGS                    (1<<29)
#define RGB2HDMI_ADJUST_PRGS      (1<<28)
#define  RGB2HDMI_HSYN_TOTAL       (0xfff<<16)
#define  RGB2HDMI_VSYN_TOTAL       (0xfff)
#define HDMI_RGB2HDMI_B4      0x18B4
#define ADJ_HSYNC_DELAY_MASK	0xFFF
#define ADJ_VSYNC_DELAY_MASK  (0xFFF << 16)
#define ADJ_FORWARD           (1 << 31)
#define HDMI_RGB2HDMI_C0      0x18C0
#define HSYNC_DELAY		 (1)
#define DOUBLE422_DELAY	 (1<<4)
#define HDMI_RGB2HDMI_F0	0x18F0
#define RGB2HDMI_ALTERNATIVE_OSTART2  (0xFFF << 16)
#define RGB2HDMI_ALTERNATIVE_OEND2  (0xFFF << 0)
#define HDMI_RGB2HDMI_F4	0x18F4
#define RGB2HDMI_ALTERNATIVE_ESTART2  (0xFFF << 16)
#define RGB2HDMI_ALTERNATIVE_ENABLE	(1 << 15)
#define RGB2HDMI_ALTERNATIVE_EEND2  (0xFFF << 0)

#define HDMI_RGB2HDMI_F8      0x18F8

#define HDMI_RGB2HDMI_FC      0x18FC
#define FIELD_STATUS            (3<<2)

/************************HDMI RGB2HDMI REG end*******************************/

#define MMSYS_VDOUT_SW_RST		0x888

#define MMSYS_VDOUT_HDMI_RST		(0x1 << 8)
#define MMSYS_VDOUT_RGB2HDMI_RST		(0x1 << 9)

#define HDR_DEBUG_DISABLE_METADATA        (1<<0)
#define HDR_DEBUG_DISABLE_HDR             (1<<1)
#define HDR_DEBUG_DISABLE_BT2020          (1<<2)
#define HDR_DEBUG_DISABLE_DOLBY_HDR       (1<<3)
#define HDR_DEBUG_DISABLE_PHI_HDR         (1<<4)



/***********************************************/
extern HDMI_AV_INFO_T _stAvdAVInfo;
#define HAL_Delay_us(delay) udelay(delay)
#define hal_delay_2us(delay) udelay(delay<<1)
extern unsigned char _bHdcpOff;
extern HDMI_SINK_AV_CAP_T _HdmiSinkAvCap;
extern unsigned char hdcp2_version_flag;
extern unsigned char hdmi2_force_output;
extern unsigned char hdmi_powerenable;

extern unsigned char debug_force_hotplug;

extern unsigned int hdmi_rgb2hdmi_read(unsigned int u2Reg);
extern void hdmi_rgb2hdmi_write(unsigned int u2Reg, unsigned int u4Data);
extern unsigned int hdmi_p2i_read(unsigned int u2Reg);
extern void hdmi_p2i_write(unsigned int u2Reg, unsigned int u4Data);
extern unsigned int hdmi_vdout10_read(unsigned int u2Reg);
extern void hdmi_vdout10_write(unsigned int u2Reg, unsigned int u4Data);
extern unsigned int hdmi_vdout15_read(unsigned int u2Reg);
extern void hdmi_vdout15_write(unsigned int u2Reg, unsigned int u4Data);
extern unsigned int hdmi_vdout1c_read(unsigned int u2Reg);
extern void hdmi_vdout1c_write(unsigned int u2Reg, unsigned int u4Data);

extern unsigned int hdmi_drv_read(unsigned short u2Reg);
extern void hdmi_drv_write(unsigned short u2Reg, unsigned int u4Data);

extern unsigned int hdmi_sys_read(unsigned short u2Reg);
extern void hdmi_sys_write(unsigned short u2Reg, unsigned int u4Data);

extern unsigned int hdmi_ana_read(unsigned short u2Reg);
extern void hdmi_ana_write(unsigned short u2Reg, unsigned int u4Data);

extern unsigned int hdmi_pad_read(unsigned short u2Reg);
extern void hdmi_pad_write(unsigned short u2Reg, unsigned int u4Data);

extern unsigned int hdmi_hdmitopck_read(unsigned short u2Reg);
extern void hdmi_hdmitopck_write(unsigned short u2Reg, unsigned int u4Data);

extern void hdmi_infrasys_write(unsigned short u2Reg, unsigned int u4Data);
extern unsigned int hdmi_infrasys_read(unsigned short u2Reg);
extern void hdmi_perisys_write(unsigned short u2Reg, unsigned int u4Data);
extern unsigned int hdmi_perisys_read(unsigned short u2Reg);

extern void internal_hdmi_read(unsigned long u4Reg, unsigned int *p4Data);

extern void internal_hdmi_write(unsigned long u4Reg, unsigned int u4data);
extern void vInitHdr(void);
extern int hdmi_video_config(HDMI_VIDEO_RESOLUTION vformat);
extern void hdmi_power_on(void);
extern void hdmi_power_off(void);
extern bool fgCaHDMICreate(void);
extern bool fgCaHDMIClose(void);
extern void vCaHDMIWriteReg(unsigned int u4addr, unsigned int u4data);
extern void vCaHDMIWriteProReg(unsigned int u4addr, unsigned int u4data);
extern bool fgCaHDMIInstallHdcpKey(unsigned char *pdata, unsigned int u4Len);
extern bool fgCaHDMIGetAKsv(unsigned char *pdata);
extern bool fgCaHDMILoadHDCPKey(void);
extern bool fgCaHDMIHDCPReset(bool fgen);
extern bool fgCaHDMIHDCPEncEn(bool fgen);
extern bool fgCaHDMIVideoUnMute(bool fgen);
extern bool fgCaHDMIAudioUnMute(bool fgen);
extern void vCaDPI1WriteReg(unsigned int u4addr, unsigned int u4data);
extern bool fgCaHDMILoadROM(void);
extern bool fgCaHDMITestHDCPVersion(void);

extern unsigned char bResolution_SD(unsigned char ui1resindex);
/* for hdmi digital register read and write */
#define vWriteByteHdmiGRL(dAddr, dVal)  (hdmi_drv_write(dAddr, dVal))
#define bReadByteHdmiGRL(bAddr)         (hdmi_drv_read(bAddr))
#define vWriteHdmiGRLMsk(dAddr, dVal, dMsk) \
	(vWriteByteHdmiGRL((dAddr), (bReadByteHdmiGRL(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
/* for sys register read and write */
#define vWriteHdmiSYS(dAddr, dVal)  (hdmi_sys_write(dAddr, dVal))
#define dReadHdmiSYS(dAddr)         (hdmi_sys_read(dAddr))
#define vWriteHdmiSYSMsk(dAddr, dVal, dMsk) \
	(vWriteHdmiSYS((dAddr), (dReadHdmiSYS(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
#define vWriteHdmiTOPCK(dAddr, dVal)  (hdmi_hdmitopck_write(dAddr, dVal))
#define dReadHdmiTOPCK(dAddr)         (hdmi_hdmitopck_read(dAddr))
#define vWriteHdmiTOPCKMsk(dAddr, dVal, dMsk) \
	(vWriteHdmiTOPCK((dAddr), (dReadHdmiTOPCK(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
#define vWriteHdmiTOPCKUnMsk(dAddr, dMsk)(vWriteHdmiTOPCK((dAddr), (dReadHdmiTOPCK(dAddr) & (~(dMsk)))))
#define vWriteIoHdmiAna(dAddr, dVal)  (hdmi_ana_write(dAddr, dVal))
#define dReadIoHdmiAna(dAddr)         (hdmi_ana_read(dAddr))
#define vWriteIoHdmiAnaMsk(dAddr, dVal, dMsk) \
	vWriteIoHdmiAna((dAddr), (dReadIoHdmiAna(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#define vWriteIoPad(dAddr, dVal)  (hdmi_pad_write(dAddr, dVal))
#define dReadIoPad(dAddr)         (hdmi_pad_read(dAddr))
#define vWriteIoPadMsk(dAddr, dVal, dMsk) vWriteIoPad((dAddr), (dReadIoPad(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#define vWriteINFRASYS(dAddr, dVal)  (hdmi_infrasys_write(dAddr, dVal))
#define dReadINFRASYS(dAddr)         (hdmi_infrasys_read(dAddr))
#define vWriteINFRASYSMsk(dAddr, dVal, dMsk) \
	(vWriteINFRASYS((dAddr), (dReadINFRASYS(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
#define vWriteINFRASYSUnMsk(dAddr, dMsk)(vWriteINFRASYS((dAddr), (dReadINFRASYS(dAddr) & (~(dMsk)))))
#define vWriteHdmiCEC(dAddr, dVal)  (CEC_REG_WRITE(dAddr, dVal))
#define dReadHdmiCEC(dAddr)         (CEC_REG_READ(dAddr))
#define vWriteHdmiCECMsk(dAddr, dVal, dMsk) \
	(vWriteHdmiCEC((dAddr), (dReadHdmiCEC(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
#define vWriteHdmiCECUnMsk(dAddr, dMsk)(vWriteHdmiCEC((dAddr), (dReadHdmiCEC(dAddr) & (~(dMsk)))))
#define vWritePeriSYS(dAddr, dVal)  (hdmi_perisys_write(dAddr, dVal))
#define dReadPeriSYS(dAddr)         (hdmi_perisys_read(dAddr))
#define vWritePeriSYSMsk(dAddr, dVal, dMsk) \
	(vWriteINFRASYS((dAddr), (dReadINFRASYS(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))
#define vWriteRGB2HDMI(dAddr, dVal)  (hdmi_rgb2hdmi_write(dAddr, dVal))
#define dReadRGB2HDMI(dAddr)         (hdmi_rgb2hdmi_read(dAddr))
#define vWriteRGB2HDMIMsk(dAddr, dVal, dMsk) \
	(vWriteRGB2HDMI((dAddr), (dReadRGB2HDMI(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteP2I(dAddr, dVal)  (hdmi_p2i_write(dAddr, dVal))
#define dReadP2I(dAddr)         (hdmi_p2i_read(dAddr))
#define vWriteP2IMsk(dAddr, dVal, dMsk) \
	(vWriteP2I((dAddr), (dReadP2I(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteVdout10(dAddr, dVal)  (hdmi_vdout10_write(dAddr, dVal))
#define dReadVdout10(dAddr)         (hdmi_vdout10_read(dAddr))
#define vWriteVdout10Msk(dAddr, dVal, dMsk) \
(vWriteVdout10((dAddr), (dReadVdout10(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteVdout15(dAddr, dVal)  (hdmi_vdout15_write(dAddr, dVal))
#define dReadVdout15(dAddr)         (hdmi_vdout15_read(dAddr))
#define vWriteVdout15Msk(dAddr, dVal, dMsk) \
	(vWriteVdout15((dAddr), (dReadVdout15(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

#define vWriteVdout1c(dAddr, dVal)  (hdmi_vdout1c_write(dAddr, dVal))
#define dReadVdout1c(dAddr)         (hdmi_vdout1c_read(dAddr))
#define vWriteVdout1cMsk(dAddr, dVal, dMsk) \
	(vWriteVdout1c((dAddr), (dReadVdout1c(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))))

extern unsigned int hdmi_boot_res;
extern unsigned int hdmi_boot_colordepth;
extern unsigned int hdmi_boot_colorspace;
extern unsigned int hdmi_audio_event;
extern unsigned int hdmi_boot_forcehdr;
extern char _bHdrType;
extern bool hdmi_force_sdr;

extern unsigned int pll_read_state;

extern bool dovi_off_delay_needed;

extern char _bStaticHdrStatus;
extern char *_bHdrMetadataBuff;
extern int hdmitx_debug_init(void);
extern void hdmitx_debug_uninit(void);
extern void vSend_AVUNMUTE(void);
extern void vSend_AVMUTE(void);
extern void vSendTMDSConfiguration(unsigned char enscramble);
extern void force_plugin(unsigned int colorspace);
extern int force_video_config(HDMI_VIDEO_RESOLUTION vformat);
extern int force_audio_config(void);
extern int pmx_parameter(HDMI_VIDEO_RESOLUTION vformat, int temp);
extern void vEnableHdmiMode(char bOn);
extern void vChgHDMIVideoResolution(void);
extern void vChangeVpll(unsigned int bRes, unsigned int bdeepmode);
extern void vEnableDeepColor(bool fgEnable, unsigned int ui1Mode);
extern void v4k6G_set_clock(void);
extern void vDolbyHdrEnable(bool fgEnable);
extern void vdout_sys_422_to_420(bool en);

extern void vHDMIAVUnMute(void);
extern void vHDMIAVMute(void);
extern void vTmdsOnOffAndResetHdcp(unsigned char fgHdmiTmdsEnable);
extern void vChgHDMIAudioOutput(unsigned char ui1hdmifs, unsigned char ui1resindex,
				unsigned char bdeepmode);
extern void vChgtoSoftNCTS(unsigned char ui1resindex, unsigned char ui1audiosoft,
			   unsigned char ui1hdmifs, unsigned char bdeepmode);
extern void vSendAVIInfoFrame(unsigned char ui1resindex, unsigned char ui1colorspace);
extern void hdmi_hdmistatus(void);
extern unsigned char bCheckPordHotPlug(unsigned char bMode);
extern void vBlackHDMIOnly(void);
extern void MuteHDMI2Audio(void);
extern void vUnBlackHDMIOnly(void);
extern void UnMuteHDMIAudio(void);
extern void MuteHDMIAudio(void);
extern unsigned char hdmi_get_port_hpd_value(void);
extern void vTxSignalOnOff(unsigned char bOn);
extern void vTmdsPresetOn(void);
extern bool fgTVisHDMI(void);

extern void vHdmiSetInit(void);
extern void vSetHdmiI2SChNum(unsigned char bChNum, unsigned char bChMapping);
extern void vHwNCTSOnOff(unsigned char bHwNctsOn);
extern void vHDMI_I2S_C_Status(void);
extern void vSendAudioInfoFrame(void);
extern void vHalSendVendorSpecificInfoFrame(unsigned char fg3DRes, unsigned char bVIC,
					    unsigned char b3dstruct,
					    unsigned char fgDolbyHdrEnable);
extern unsigned int IS_HDMI_HTPLG(void);
extern unsigned int IS_HDMI_PORD(void);
extern unsigned char bCheckStatus(unsigned char bMode);
extern void vHDMIAudioSRC(unsigned char ui1hdmifs, unsigned char ui1resindex,
			  unsigned char bdeepmode);

extern unsigned char bResolution_4K2K(unsigned char bResIndex);
extern unsigned char bResolution_4K2K_50_60Hz(unsigned char bResIndex);
extern unsigned int hdmi_check_status(void);
extern void hdmi_hdcp_information(HDCP_INFO *hdcp_information);
extern void hdmi_show_hdcp_information(void);
extern unsigned char bCheckPordHotPlug2(unsigned char bMode);
extern unsigned int IS_HDMI2_HTPLG(void);
extern unsigned int IS_HDMI2_PORD(void);
extern void vHalSendAudioInfoFrame(unsigned char bData1, unsigned char bData2,
				   unsigned char bData4, unsigned char bData5);
extern void vSendVendorSpecificInfoFrame(unsigned char ui1resindex);
extern void vSetHDMITxPLL(unsigned int bResIndex, unsigned int bClockRate);
extern void vResetHDMI(char bRst);
extern void vHalSendAVIInfoFrame(unsigned char *pr_bData);
extern void hdmi_clock_debug(bool bEnable, unsigned char bItem);

void vSetHdmiSpdifConfig(void);
void vAipCtrlInit(void);
void vEnableAudio(unsigned int bOn);
void vAudioNCTSOn(char bOn);
unsigned int bColorspace420(void);
void vEnableInputAudioType(unsigned int bspdifi2s);
void vVdpSetHdrMetadata(bool enable, VID_PLA_HDR_METADATA_INFO_T hdr_metadata);
void vHdrEnable(bool fgEnable);
extern void vShowIrqRegister(void);
extern unsigned int i4GetHotPlugStatus(void);
extern void vHdcpDdcHwPoll(unsigned char _bhw);
extern unsigned char fgIsHDCPCtrlTimeOut(void);
extern void vSetHDCPTimeOut(unsigned int i4_count);
extern unsigned char fgIsHDCPCtrlTimeOut(void);
extern unsigned int u4GetHDMIHdcpDelayTime(void);
extern void vShowHdcpTmr(void);
bool fgIsRepeater(void);
bool fgUseDolbyVSIF(void);
void vTrySendDolbyVSIF(HDMI_VIDEO_RESOLUTION res);
void hdmi_clock_probe(struct platform_device *pdev);
void hdmi_clock_enable(bool bEnable);
void hdmi_sdck_select(bool bEnable);
void hdmi_poll_isr(unsigned long n);
void cec_poll_isr(unsigned long n);
extern VID_PLA_DR_TYPE_T hdr_status(char *str);
void vResetAudioHDMI(unsigned char bRst);
void vResetVideoHDMI(unsigned char bRst);
void vAudioPacketOff(unsigned char bOn);
void vLowLatencyDolbyVisionEnable(bool fgEnable);
void vHdr10PlusEnable(bool fgEnable);
void vSetStaticHdrType(char bType);
void vHdr10PlusVSIFEnable(bool fgEnable, unsigned int forcedHdrType, int dolbyOpFmt);
void vBlackHDMIOnly(void);
void vUnBlackHDMIOnly(void);
void UnMuteHDMIAudio(void);
void MuteHDMIAudio(void);

void vHalSendStaticHdrInfoFrame(char bEnable, char *pr_bData);
void vHdrEnable(bool fgEnable);
void vBT2020Enable(bool fgEnable);
void vDolbyHdrEnable(bool fgEnable);
void vSetHdrDebugType(unsigned int u4Data);

extern VID_PLA_DR_TYPE_T hdr_status(char *str);
extern void vcommon_status(char *str);
void vshow_hdr_status(void);
void vResetAudioHDMI(unsigned char bRst);
void vResetVideoHDMI(unsigned char bRst);
void vAudioPacketOff(unsigned char bOn);
void vLowLatencyDolbyVisionEnable(bool fgEnable);
void vHdr10PlusEnable(bool fgEnable);
void vSetStaticHdrType(char bType);
void vBlackHDMIOnly(void);
void vUnBlackHDMIOnly(void);
void UnMuteHDMIAudio(void);
void MuteHDMIAudio(void);
bool hdmi_ddc_request(unsigned char req);
void hdmi_ddc_free(unsigned char req);
void Hdr10DelayOffHandler(void);
void Hdr10pVsifOffImmediately(void);
void Hdr10pVsifDelayOffHandler(void);
extern unsigned char vIsDviMode(void);
void vSendStaticHdrInfoFrame(void);
void vDisable_AVMUTE_Packet(void);
void vDisable_HDCP_Encrypt(void);
unsigned int hdmi_clock_read(unsigned char sel);
void hdmi_timing_monitor_stop(void);
void hdmi_timing_monitor_start(void);
void hdmi_timing_monitor_sel(unsigned int sel);
unsigned char bOver340M(void);

#endif
#endif
