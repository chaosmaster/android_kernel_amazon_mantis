/*
 * Copyright (c) 2015-2016 MediaTek Inc.
 * Author: Yong Wu <yong.wu@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/err.h>
#include <linux/io.h>
#include <linux/iopoll.h>

#include "imgresz.h"
#include "imgresz_priv.h"
#include "imgresz_hal.h"

#define RW_IMG_RESZ_START			0x000
#define IMG_RESZ_ACTIVATE			(0x1 << 0)
#define IMG_RESZ_ENABLE				(0x1 << 1)
#define IMG_RESZ_SW_RESET_ON			(0x3 << 2)
#define IMG_RESZ_MMU_RESET_ON			(0x3 << 20)
#define IMG_RESZ_CHK_SUM_CLR			(0x1 << 4)
#define IMG_RESZ_DMA_SW_RST			(0x1 << 5)
#define IMG_RESZ_WAIT_WR_DONE			(0x1 << 6)
#define IMG_RESZ_MMU_ENABLE			(0x1 << 22)
#define IMG_RESZ_INT_ON				(0x1 << 7)
#define IMG_RESZ_REGISTER_RESET_ON		(0x3 << 8)
#define IMG_RESZ_RD_BURST_ON			(0x1 << 16)
#define IMG_RESZ_DMA_SAFE_MODE			(0x1 << 18)
#define IMG_RESZ_IRQ_CLEAR				(0x1 << 23)
#define IMG_RESZ_WR_BURST_ON			(0x1 << 24)
#define IMG_RESZ_WR_BST_NCROSS			(0x1 << 25)


#define RW_IMG_RESZ_TYPE			0x004
#define IMG_RESZ_VIDEO_MODE				(0x0 << 0)
#define IMG_RESZ_JPEG_MODE				(0x1 << 0)
#define IMG_RESZ_SEL_VID_MODE			(0x0 << 1)
#define IMG_RESZ_SEL_OSD_MODE			(0x1 << 1)
#define IMG_RESZ_INTERLACE_BOTTOM_FIELD		(0x0 << 2)
#define IMG_RESZ_INTERLACE_TOP_FIELD		(0x1 << 2)
#define IMG_RESZ_FRAME					(0x0 << 3)
#define IMG_RESZ_FIELD					(0x1 << 3)
#define IMG_RESZ_BLOCK_BASED_IN			(0x0 << 4)
#define IMG_RESZ_RASTER_SCAN_IN			(0x1 << 4)
#define IMG_RESZ_OSD_NORMAL_MODE		(0x0 << 5)
#define IMG_RESZ_OSD_PARTIAL_MODE		(0x1 << 5)
#define IMG_RESZ_BILINEAR_FILTER		(0x0 << 8)
#define IMG_RESZ_V_4_TAP_FILTER			(0x1 << 8)
/* Turbe Mode, alpha compostion will enable this */
#define IMG_RESZ_FIX4_ENABLE			(0x1 << 12)
#define IMG_RESZ_BLOCK_BASED_OUT		(0x0 << 16)
#define IMG_RESZ_RASTER_SCAN_OUT		(0x1 << 16)
#define IMG_RESZ_420_OUT			(0x0 << 17)
#define IMG_RESZ_422_OUT			(0x1 << 17)
#define IMG_RESZ_444_OUT			(0x2 << 17)
#define IMG_RESZ_CBCR_NO_PADDING	(0x0 << 19)
#define IMG_RESZ_CR_PADDING			(0x1 << 19)
#define IMG_RESZ_CB_PADDING			(0x2 << 19)
#define IMG_RESZ_CBCR_PADDING		(0x3 << 19)
#define IMG_RESZ_CBCRSWAP			(0x1 << 21)
#define IMG_RESZ_V2V				(0x0 << 24)
#define IMG_RESZ_V2OSD				(0x1 << 24)
#define IMG_RESZ_ARGB_ONE_PHASE		(0x1 << 25)
#define IMG_RESZ_ONE_PHASE_CSC		(0x1 << 26)
#define IMG_RESZ_10BIT_OUT_CALC		(0x1 << 29)
#define IMG_RESZ_10BIT_OUT			(0x1 << 30)
#define IMG_RESZ_10BIT_ON			(0x1 << 31)

#define RW_IMG_RESZ_JPG_MODE			0x008
#define IMG_RESZ_LINES_ASSIGNED_DIRECTLY	(0x1 << 31)
#define IMG_RESZ_PRELOAD_DRAM_DATA		(0x1 << 18)
#define IMG_RESZ_RECORD_CR			(0x1 << 14)
#define IMG_RESZ_RECORD_CB			(0x2 << 14)
#define IMG_RESZ_RECORD_Y			(0x4 << 14)
#define IMG_RESZ_FIRST_BLOCK_LINE		(0x1 << 13)
#define IMG_RESZ_LAST_BLOCK_LINE		(0x1 << 12)

#define RW_IMG_RESZ_MEM_IF_MODE			0x00C
#define IMG_RESZ_DRAM_BURST_LIMIT_1		(0x1 << 8)
#define IMG_RESZ_DRAM_BURST_LIMIT_2		(0x2 << 8)
#define IMG_RESZ_DRAM_BURST_LIMIT_3		(0x3 << 8)
#define IMG_RESZ_DRAM_BURST_LIMIT_4		(0x4 << 8)
#define IMG_RESZ_DRAM_BURST_LIMIT_8		(0x8 << 8)
#define IMG_RESZ_DRAM_BURST_LIMIT_16		(0x10 << 8)
#define IMG_RESZ_RESET_ADDR_NORMAL		(0x0 << 8)
#define IMG_RESZ_RESET_ADDR_NEW_BLOCK_LINE	(0x1 << 8)
#define IMG_RESZ_REST_ADDRESS			(0x1 << 20)
#define IMG_RESZ_KEEP_DRAM_READ_REQUEST		(0x1 << 16)
#define IMG_RESZ_MMU_TEMPBUF_AGENTID_MASK	(0x1 << 31)

#define RW_IMG_RESZ_SRC_BUF_LEN			0x010

#define RW_IMG_RESZ_INTERFACE_SWITCH		0x014
#define IMG_RESZ_TRACKING_WITH_JPG_HW		(0x1 << 0)

#define RW_IMG_RESZ_TGT_BUF_LEN			0x018
#define IMG_RESZ_BOUND_EXTEND_16_OFF		(0x0 << 31)
#define IMG_RESZ_BOUND_EXTEND_16_ON		(0x1 << 31)
#define IMG_RESZ_LINE_BUFFER_LEN_SHIFT		24
#define IMG_RESZ_TGT_BUFFER_LEN_SHIFT		0

#define RW_IMG_RESZ_SRC_Y_ADDR_BASE1		0x01C
#define RW_IMG_RESZ_SRC_Y_ADDR_BASE2		0x020

#define RW_IMG_RESZ_SRC_CB_ADDR_BASE1		0x024
#define RW_IMG_RESZ_SRC_CB_ADDR_BASE2		0x028
#define RW_IMG_RESZ_SRC_CR_ADDR_BASE1		0x02C
#define RW_IMG_RESZ_SRC_CR_ADDR_BASE2		0x030

#define RW_IMG_RESZ_TGT_Y_ADDR_BASE		0x034
#define RW_IMG_RESZ_TGT_C_ADDR_BASE		0x038

#define RW_IMG_RESZ_SRC_SIZE_Y			0x040
#define RW_IMG_RESZ_SRC_SIZE_CB			0x044
#define RW_IMG_RESZ_SRC_SIZE_CR			0x048

#define RW_IMG_RESZ_TGT_SIZE			0x04C
#define RW_IMG_RESZ_SRC_OFFSET_Y		0x054
#define RW_IMG_RESZ_SRC_OFFSET_CB		0x058
#define RW_IMG_RESZ_SRC_OFFSET_CR		0x05C

#define RW_IMG_RESZ_TGT_OFFSET			0x060
#define RW_IMG_RESZ_H8TAPS_FAC_Y		0x064
#define RW_IMG_RESZ_H8TAPS_FAC_CB		0x068
#define RW_IMG_RESZ_H8TAPS_FAC_CR		0x06C

#define RW_IMG_RESZ_HSA_SCL_Y			0x070
#define RW_IMG_RESZ_HSA_SCL_CB			0x074

#define RW_IMG_RESZ_HSA_SCL_CR			0x078

#define RW_IMG_RESZ_V_SCL_Y			0x07C
#define IMG_RESZ_Y_VERTICAL_DOWN_SCALING	(0x0 << 0)
#define IMG_RESZ_Y_VERTICAL_UP_SCALING		(0x1 << 0)

#define RW_IMG_RESZ_V_SCL_CB			0x080
#define IMG_RESZ_CB_VERTICAL_DOWN_SCALING	(0x0 << 0)
#define IMG_RESZ_CB_VERTICAL_UP_SCALING		(0x1 << 0)

#define RW_IMG_RESZ_V_SCL_CR			0x084
#define IMG_RESZ_CR_VERTICAL_DOWN_SCALING	(0x0 << 0)
#define IMG_RESZ_CR_VERTICAL_UP_SCALING		(0x1 << 0)

#define RW_IMG_RESZ_V4TAPS_SCL_Y		0x088
#define RW_IMG_RESZ_V4TAPS_SCL_CB		0x08C
#define RW_IMG_RESZ_V4TAPS_SCL_CR		0x090

#define RW_IMG_RESZ_TMP_ADDR_BASE		0x094
#define RW_IMG_RESZ_Y_PRELOAD_OW_ADDR_BASE	0x098
#define RW_IMG_RESZ_C_PRELOAD_OW_ADDR_BASE	0x09C

#define RW_IMG_RESZ_H8TAP_OFSET_Y		0x0C8
#define RW_IMG_RESZ_H8TAP_OFSET_CB		0x0CC
#define RW_IMG_RESZ_H8TAP_OFSET_CR		0x0D0
#define RW_IMG_RESZ_V4TAP_OFSET_Y		0x0D4
#define RW_IMG_RESZ_V4TAP_OFSET_C		0x0D8

#define RO_IMG_RESZ_CHECK_SUM_REG		0x0E4

#define RO_IMG_RESZ_INTERFACE_MONITOR_REG	0x0EC

#define RO_IMG_RESZ_STATUS_MONITOR_REG		0x0F4
	#define IMG_RESZ_RD_REQ				(0x1 << 0)

#define RO_IMG_RESZ_DATA_MONITOR_REG		0x0F8

#define RO_IMG_RESZ_DONE			0x0FC

#define RW_IMG_RESZ_OSD_MODE_SETTING		0x100
#define IMG_RESZ_OSD_INDEX_MODE			(0x0 << 0)
#define IMG_RESZ_OSD_DIRECT_MODE		(0x1 << 0)
#define IMG_RESZ_OSD_INDEX_2BPP			(0x1 << 1)
#define IMG_RESZ_OSD_INDEX_4BPP			(0x2 << 1)
#define IMG_RESZ_OSD_INDEX_8BPP			(0x3 << 1)
#define IMG_RESZ_OSD_DIRECT_RGB565		(0x0 << 1)
#define IMG_RESZ_OSD_DIRECT_ARGB1555		(0x1 << 1)
#define IMG_RESZ_OSD_DIRECT_ARGB4444		(0x2 << 1)
#define IMG_RESZ_OSD_DIRECT_ARGB8888		(0x3 << 1)
#define IMG_RESZ_OSD_REPEATING			(0x1 << 4)
#define IMG_RESZ_OSD_ALPHA_SCALE_NORMAL		(0x0 << 5)
#define IMG_RESZ_OSD_ALPHA_SCALE_REF_LEFT	(0x1 << 5)
#define IMG_RESZ_OSD_ALPHA_SCALE_REF_NEAREST	(0x2 << 5)
#define IMG_RESZ_OSD_ALPHA_BILINEAR_BOUNDARY	(0x1 << 8)
#define IMG_RESZ_OSD_ONLY_DISTINGUISH_ALPHA	(0x1 << 7)
#define IMG_RESZ_OSD_OUTPUT_RGB565		(0x0 << 9)
#define IMG_RESZ_OSD_OUTPUT_ARGB1555		(0x1 << 9)
#define IMG_RESZ_OSD_OUTPUT_ARGB4444		(0x2 << 9)
#define IMG_RESZ_OSD_OUTPUT_ARGB8888		(0x3 << 9)
#define IMG_RESZ_OSD_SWITCH_CBCR		(0x1 << 12)
#define IMG_RESZ_OSD_ARGB_RGBA			(0x1 << 14)
#define IMG_RESZ_A_BLEND_SHIFT			24

#define RW_IMG_RESZ_OSD_MD_CTRL			0x104
#define IMG_RESZ_OSD_WR_CPT			(0x0 << 0)
#define IMG_RESZ_OSD_ED_CPT			(0x1 << 0)
#define IMG_RESZ_OSD_CPT_DISABLE		(0x0 << 1)
#define IMG_RESZ_OSD_CPT_ENABLE			(0x1 << 1)
#define IMG_RESZ_OSD_ALU_ENABLE			(0x1 << 28)

#define RW_IMG_RESZ_OSD_ALPHA_TBL		0x108

#define RW_IMG_RESZ_OSD_COLOR_TRANSLATION0	0x10C
#define RW_IMG_RESZ_OSD_COLOR_TRANSLATION1	0x110
#define RW_IMG_RESZ_OSD_COLOR_TRANSLATION2	0x114
#define RW_IMG_RESZ_OSD_COLOR_TRANSLATION3	0x118

#define RW_IMG_RESZ_H_COEF0			0x124
#define RW_IMG_RESZ_H_COEF1			0x128
#define RW_IMG_RESZ_H_COEF2			0x12C
#define RW_IMG_RESZ_H_COEF3			0x130
#define RW_IMG_RESZ_H_COEF4			0x134
#define RW_IMG_RESZ_H_COEF5			0x138
#define RW_IMG_RESZ_H_COEF6			0x13C
#define RW_IMG_RESZ_H_COEF7			0x140
#define RW_IMG_RESZ_H_COEF8			0x144
#define RW_IMG_RESZ_H_COEF9			0x148
#define RW_IMG_RESZ_H_COEF10			0x14C
#define RW_IMG_RESZ_H_COEF11			0x150
#define RW_IMG_RESZ_H_COEF12			0x154
#define RW_IMG_RESZ_H_COEF13			0x158
#define RW_IMG_RESZ_H_COEF14			0x15C
#define RW_IMG_RESZ_H_COEF15			0x160
#define RW_IMG_RESZ_H_COEF16			0x164
#define RW_IMG_RESZ_H_COEF17			0x168

#define RW_IMG_RESZ_V_COEF0			0x16C
#define RW_IMG_RESZ_V_COEF1			0x170
#define RW_IMG_RESZ_V_COEF2			0x174
#define RW_IMG_RESZ_V_COEF3			0x178
#define RW_IMG_RESZ_V_COEF4			0x17C
#define RW_IMG_RESZ_V_COEF5			0x180
#define RW_IMG_RESZ_V_COEF6			0x184
#define RW_IMG_RESZ_V_COEF7			0x188
#define RW_IMG_RESZ_V_COEF8			0x18C

#define RW_IMG_RESZ_OSD_DITHER_SETTING		0x190
#define RW_IMG_RESZ_OSD_CSC_SETTING		0x194
#define IMG_RESZ_OSD_CSC_ENABLE			(0x1 << 0)
#define IMG_RESZ_OSD_CSC_YIN_D16		(0x1 << 1)
#define IMG_RESZ_OSD_CSC_CIN_D128		(0x1 << 2)
#define IMG_RESZ_OSD_CSC_YOUT_A16		(0x1 << 3)
#define IMG_RESZ_OSD_CSC_COUT_A128		(0x1 << 4)

#define RW_IMG_RESZ_OSD_CSC_COEF11		0x198
#define RW_IMG_RESZ_OSD_CSC_COEF12		0x19C
#define RW_IMG_RESZ_OSD_CSC_COEF13		0x1A0
#define RW_IMG_RESZ_OSD_CSC_COEF21		0x1A4
#define RW_IMG_RESZ_OSD_CSC_COEF22		0x1A8
#define RW_IMG_RESZ_OSD_CSC_COEF23		0x1AC
#define RW_IMG_RESZ_OSD_CSC_COEF31		0x1B0
#define RW_IMG_RESZ_OSD_CSC_COEF32		0x1B4
#define RW_IMG_RESZ_OSD_CSC_COEF33		0x1B8

#define RW_IMG_RESZ_RPR				0x1D0
  #define IMG_RESZ_RPR_FLAG_ON			(0x1 << 0)
  #define IMG_RESZ_URCRPR_ON			(0x1 << 1)
  #define IMG_RESZ_TRC_VDEC_EN			(0x1 << 8)
  #define IMG_RESZ_TRC_VDEC_INT			(0x1 << 9)

#define RW_IMG_RESZ_FLIP			0x1D8
  #define IMG_RESZ_OUT_FLIP_ON			(0x1 << 0)

#define RW_IMG_RESZ_UFO_LINEB_ECO	0x1DC
  #define IMG_RESZ_UFO_LINEB_ECO		(0x1 << 5)

#define RW_IMG_RESZ_LINE_NUM_IN_Y_COLOR_BUF	0x1C4
#define RW_IMG_RESZ_LINE_NUM_IN_CB_COLOR_BUF	0x1C8
#define RW_IMG_RESZ_LINE_NUM_IN_CR_COLOR_BUF	0x1CC

#define RW_IMG_RESZ_VENC_SKIP			0x1EC
#define RM_IMG_RESZ_VENC_SKIP_ON		BIT(0)
#define IMG_RESZ_RD_BURST_SHIFT			(4)
#define IMG_RESZ_WR_BURST_SHIFT			(8)
#define IMG_RESZ_RD_BURST_LIMIT_CLEAR	0xFFFFFF0F
#define IMG_RESZ_WR_BURST_LIMIT_CLEAR	0xFFFFF0FF
#define IMG_RESZ_ENABLE_CHROMA			(0x1 << 16)


#define RW_IMG_RESZ_READ_CHECKSUM		0x1E4
#define RW_IMG_RESZ_ALPHA_COMPOSITION		0x200

#define RW_IMG_RESZ_UFO_TRIG			0x204
#define IMG_RESZ_UFO_TRIG			(0x1 << 0)
#define RW_IMG_RESZ_UFO_POWER			0x208
#define IMG_RESZ_UFO_ON				(0x1 << 0)
#define IMG_RESZ_BITS_POWER			(0x1 << 1)
#define IMG_RESZ_LEN_POWER			(0x1 << 2)
#define IMG_RESZ_DRAM_CLK			(0x1 << 3)
#define IMG_RESZ_UFO_CLK			(0x1 << 4)
/*1: ufo_interrupt function on  0: off*/
#define IMG_RESZ_UFO_INT_ON			(0x1 << 5)
/*1: make ufo line buffer ready after imgrz done  0: off*/
#define IMG_RESZ_UFO_LINEB_READY	(0x1 << 6)
#define IMG_RESZ_UFO_OUTSTANDING	(0x1 << 7)

#define RW_IMG_RESZ_UFO_CFG			0x20C
#define IMG_RESZ_COMPRESS_EN		(0x1 << 0)
#define IMG_RESZ_CHROMA				(0x1 << 3)
/*no need to wait for imgrz req up, then trig ufo_dec.*/
/*if this bit's up, after trig imgrz, ufo will be auto trig*/
#define IMG_RESZ_UFO_AUTO_TRIG		(0x1 << 5)

/* UFO PIC is the buffer wid(pitch) and buffer height */
#define	RW_IMG_RESZ_UFO_PIC_SZ			0x210
#define RW_IMG_RESZ_UFO_PAGE_SZ			0x214
#define RW_IMG_RESZ_UFO_START_POINT		0x218

#define RW_IMG_RESZ_SEC_UFO_PAGE_SZ		0x300

#define RW_IMG_RESZ_UFO_Y_ADDR			0x21C
#define RW_IMG_RESZ_UFO_Y_LEN_ADDR		0x220
#define RW_IMG_RESZ_UFO_C_ADDR			0x224
#define	RW_IMG_RESZ_UFO_C_LEN_ADDR		0x228

#define RW_IMG_RESZ_UFO_JUMPMODE		0x238 /* Only for 10bit */
	#define IMG_RESZ_UFO_JUMPMODE_EN	0x1

#define loghal(string, args...)  pr_debug("[ImgResz]"string, ##args)

static void imgresz_ufo_trigger(void __iomem *base);

enum IMGRZ_SCALE_FACTOR {
	FACTOR_0 = 0,
	FACTOR_0_0625,
	FACTOR_0_125,
	FACTOR_0_25,
	FACTOR_0_5,
	FACTOR_1,
	FACTOR_RM,
	FACTOR_MAX
};

static unsigned int h_filtercoeff[FACTOR_MAX][18] = {
	/* FACTOR_0 */
	{0x40202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	 0x20202020, 0x00000000, 0x00000000},
	/* FACTOR_0_0625 */
	{0x3f21201f, 0x2121201f, 0x2121201f, 0x2121201f, 0x2121201f,
	 0x2121201f, 0x2121201f, 0x2121201e, 0x2121201e, 0x2121201e,
	 0x2021201e, 0x2021201e, 0x21211f1e, 0x22201f1e, 0x22201f1e,
	 0x22201f1e, 0x00000000, 0x00000000},
	/* FACTOR_0_125 */
	{0x3c24211d, 0x2524211c, 0x2524211c, 0x2524201b, 0x2524201b,
	 0x2523201b, 0x25231f1a, 0x25231f1a, 0x24231f1a, 0x24231f19,
	 0x26221e19, 0x25221e18, 0x24221e18, 0x25221d18, 0x24221d17,
	 0x25211d17, 0x00000000, 0x00000000},
	/* FACTOR_0_25 */
	{0x3833220f, 0x3831210e, 0x3830200d, 0x382f1f0c, 0x382f1d0b,
	 0x372e1c0a, 0x372d1b09, 0x372c1a08, 0x362b1807, 0x342a1706,
	 0x34291605, 0x34281504, 0x33271403, 0x34251302, 0x34241102,
	 0x34231001, 0x00000000, 0x00000000},
	/* FACTOR_0_5 */
	{0x845000ee, 0x844bfdef, 0x8345faf0, 0x8340f7f1, 0x813af5f2,
	 0x7f35f3f3, 0x7d2ff1f4, 0x7a29eff6, 0x7724eef7, 0x741fedf8,
	 0x6e1aedfa, 0x6915edfb, 0x6610ecfc, 0x5f0cedfd, 0x5c07edfe,
	 0x5404eeff, 0x33333331, 0x33333333},
	/* FACTOR_1 */
	{0x00000000, 0xfef40300, 0xf9ea0500, 0xf0e30700, 0xe5de0800,
	 0xd6db0800, 0xc5da0800, 0xb1db0700, 0x9ddd0600, 0x87e10500,
	 0x70e50400, 0x5aea0300, 0x44ef0200, 0x31f40100, 0x20f80000,
	 0x0ffc0000, 0x44444448, 0x44444444},
	/* FACTOR_RM */
	{0x00000000, 0xf0000000, 0xe0000000, 0xd0000000, 0xc0000000,
	 0xb0000000, 0xa0000000, 0x90000000, 0x80000000, 0x70000000,
	 0x60000000, 0x50000000, 0x40000000, 0x30000000, 0x20000000,
	 0x10000000, 0x00000008, 0x00000000}
};

static unsigned int v_filtercoeff[FACTOR_MAX][9] = {
	/* FACTOR_0 */
	{0x40408040, 0x40404040, 0x40404040, 0x40404040, 0x40404040,
	 0x40404040, 0x40404040, 0x40404040, 0x00000000},
	/* FACTOR_0_0625 */
	{0x41408040, 0x41404140, 0x41404140, 0x40404140, 0x413f4040,
	 0x403f403f, 0x403f403f, 0x403f403f, 0x00000000},
	/* FACTOR_0_125 */
	{0x43417e41, 0x42404340, 0x423f4240, 0x423e423f, 0x423e423e,
	 0x423d423d, 0x423c413d, 0x403c413c, 0x00000000},
	/* FACTOR_0_25 */
	{0x4b427a43, 0x4a3f4b40, 0x493c4a3e, 0x483a493b, 0x47374838,
	 0x47344636, 0x45324533, 0x442f4431, 0x00000000},
	/* FACTOR_0_5 */
	{0x72417446, 0x6f36703b, 0x6a2c6c31, 0x65226827, 0x6019621e,
	 0x59115c15, 0x510a560d, 0x4a034f06, 0x00000000},
	/* FACTOR_1 */
	{0x01f40000, 0xf8e2ffea, 0xdfd9eddd, 0xbad9ced8, 0x8edfa5db,
	 0x5fe976e4, 0x33f348ee, 0x0ffc1ff8, 0x00000005},
	/* FACTOR_RM */
	{0xf0000000, 0xd000e000, 0xb000c000, 0x9000a000, 0x70008000,
	 0x50006000, 0x30004000, 0x10002000, 0x00000001}
};

static void imgresz_hal_hw_reset(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695)
		reg |= IMG_RESZ_SW_RESET_ON;
	else
		reg |= IMG_RESZ_SW_RESET_ON | IMG_RESZ_MMU_RESET_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_REGISTER_RESET_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);

	reg &= ~IMG_RESZ_REGISTER_RESET_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
	reg &= ~(IMG_RESZ_SW_RESET_ON | IMG_RESZ_MMU_RESET_ON);
	reg &= ~IMG_RESZ_DMA_SW_RST;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

static void imgresz_hal_hw_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_ENABLE + IMG_RESZ_CHK_SUM_CLR;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

static void imgresz_hal_hw_disable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg &= ~IMG_RESZ_ENABLE;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

void imgresz_hal_mmuclk_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_MMU_ENABLE;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

static void imgresz_hal_interrupt_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_INT_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

static void imgresz_hal_waitdone_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_WAIT_WR_DONE;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

static void imgresz_hal_set_dram_burstlimit(void __iomem *base, int limit)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_MEM_IF_MODE);

	reg &= ~(IMG_RESZ_DRAM_BURST_LIMIT_1 | IMG_RESZ_DRAM_BURST_LIMIT_2 |
			IMG_RESZ_DRAM_BURST_LIMIT_4 | IMG_RESZ_DRAM_BURST_LIMIT_8 |
			IMG_RESZ_DRAM_BURST_LIMIT_16);

	reg |= ((limit & 0x1f) << 8);

	writel_relaxed(reg, base + RW_IMG_RESZ_MEM_IF_MODE);
}

void imgresz_hal_set_wr_gmc_burstlimit(void __iomem *base, int limit)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_VENC_SKIP);
	reg &= IMG_RESZ_WR_BURST_LIMIT_CLEAR;
	reg |= (limit << IMG_RESZ_WR_BURST_SHIFT);
	writel_relaxed(reg, base + RW_IMG_RESZ_VENC_SKIP);
}

void imgresz_hal_set_rd_gmc_burstlimit(void __iomem *base, int limit)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_VENC_SKIP);
	reg &= IMG_RESZ_RD_BURST_LIMIT_CLEAR;
	reg |= (limit << IMG_RESZ_RD_BURST_SHIFT);
	writel_relaxed(reg, base + RW_IMG_RESZ_VENC_SKIP);
}

void imgresz_hal_HW_init(void __iomem *base)
{
	imgresz_hal_hw_enable(base);
	imgresz_hal_hw_reset(base);
	imgresz_hal_set_dram_burstlimit(base, 16);
	imgresz_hal_interrupt_enable(base);
	imgresz_hal_waitdone_enable(base);
}

void imgresz_hal_HW_uninit(void __iomem *base)
{
	imgresz_hal_hw_reset(base);
	imgresz_hal_hw_disable(base);
}

/* Help debug, Print the register. */
void imgresz_hal_print_reg(void __iomem *base)
{
	int temp = 0;
	uint32_t dump_sz = 144;

	/* UFO need more reg. */
	if (readl_relaxed(base + RW_IMG_RESZ_UFO_POWER) & IMG_RESZ_UFO_ON)
		dump_sz += 13 * 4;

	for (temp = 0; temp < dump_sz; temp += 4) {
		pr_info("[%p]0x%8x   0x%8x   0x%8x   0x%8x\n",
			base + temp * 4,
			readl_relaxed(base + temp * 4),
			readl_relaxed(base + temp * 4 + 0x4),
			readl_relaxed(base + temp * 4 + 0x8),
			readl_relaxed(base + temp * 4 + 0xc));
	}
}

void imgresz_hal_trigger_hw(void __iomem *base, bool ufo)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_ACTIVATE;
	writel(reg, base + RW_IMG_RESZ_START);

	/* Trigger UFO if current is UFO */
	if (ufo)
		imgresz_ufo_trigger(base);
}

void imgresz_hal_clr_irq(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_IRQ_CLEAR;
	writel(reg, base + RW_IMG_RESZ_START);
}

int
imgresz_hal_set_resz_mode(void __iomem *base, enum imgresz_scale_mode reszmode,
			  enum IMGRESZ_BUF_MAIN_FORMAT_T mainformat)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);

	switch (mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		reg &= ~IMG_RESZ_JPEG_MODE;
		reg &= ~IMG_RESZ_OSD_PARTIAL_MODE;
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		reg |= IMG_RESZ_JPEG_MODE;
		reg &= ~IMG_RESZ_OSD_PARTIAL_MODE;
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
		reg &= ~IMG_RESZ_JPEG_MODE;
		if (reszmode == IMGRESZ_PARTIAL_SCALE)
			reg |= IMG_RESZ_OSD_PARTIAL_MODE;
		break;
	default:
		return -EINVAL;
	}

	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);

	return 0;
}

void
imgresz_hal_set_resample_method(void __iomem *base,
				enum IMGRESZ_RESAMPLE_METHOD_T h_method,
				enum IMGRESZ_RESAMPLE_METHOD_T v_method)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);
	if (v_method == IMGRESZ_RESAMPLE_METHOD_4_TAP)
		reg |= IMG_RESZ_V_4_TAP_FILTER;
	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);
}

void imgresz_hal_burst_enable(void __iomem *base, bool wr_enable, bool rd_enable)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg &= ~IMG_RESZ_RD_BURST_ON;
	reg &= ~IMG_RESZ_WR_BURST_ON;
	reg |= IMG_RESZ_WR_BST_NCROSS;
	if (rd_enable)
		reg |= IMG_RESZ_RD_BURST_ON;
	if (wr_enable)
		reg |= IMG_RESZ_WR_BURST_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);

	imgresz_hal_set_wr_gmc_burstlimit(base, wr_enable?8:1);
	imgresz_hal_set_rd_gmc_burstlimit(base, rd_enable?8:1);
}

static void imgresz_hal_dma_reset(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_START);
	reg |= IMG_RESZ_DMA_SW_RST;
	writel_relaxed(reg, base + RW_IMG_RESZ_START);
}

void
imgresz_hal_set_src_buf_addr(void __iomem *base, uint32_t y_buf_addr,
			     uint32_t cb_buf_addr, uint32_t cr_buf_addr)
{
	writel_relaxed(y_buf_addr >> 4, base + RW_IMG_RESZ_SRC_Y_ADDR_BASE1);
	writel_relaxed(cb_buf_addr >> 4, base + RW_IMG_RESZ_SRC_CB_ADDR_BASE1);
	writel_relaxed(cr_buf_addr >> 4, base + RW_IMG_RESZ_SRC_CR_ADDR_BASE1);
}

int
imgresz_hal_set_src_buf_format(void __iomem *base,
			       const struct imgresz_buf_format *src_buf_format)
{
	u32 resz_type, osd_setting, reg;

	resz_type = readl_relaxed(base + RW_IMG_RESZ_TYPE);
	osd_setting = readl_relaxed(base + RW_IMG_RESZ_OSD_MODE_SETTING);

	/* If not clear, OSD mode will incorrect.*/
	resz_type &= ~IMG_RESZ_FIELD;

	switch (src_buf_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		resz_type &= ~IMG_RESZ_SEL_OSD_MODE;
		if (src_buf_format->progressive) {
			resz_type &= ~IMG_RESZ_FIELD;
		} else {/* Interlace */
			resz_type |= IMG_RESZ_FIELD;
			if (src_buf_format->top_field)
				resz_type |= IMG_RESZ_INTERLACE_TOP_FIELD;
			else
				resz_type &= ~IMG_RESZ_INTERLACE_TOP_FIELD;
		}
		if (src_buf_format->block)
			resz_type &= ~IMG_RESZ_RASTER_SCAN_IN;
		else
			resz_type |= IMG_RESZ_RASTER_SCAN_IN;
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		resz_type &= ~IMG_RESZ_SEL_OSD_MODE;
		/* Set Sample Factor */
		reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);
		reg &= 0xFFFFF000;
		reg |= (((src_buf_format->h_sample[0] - 1) & 3) << 10);
		reg |= (((src_buf_format->v_sample[0] - 1) & 3) << 8);
		reg |= (((src_buf_format->h_sample[1] - 1) & 3) << 6);
		reg |= (((src_buf_format->v_sample[1] - 1) & 3) << 4);
		reg |= (((src_buf_format->h_sample[2] - 1) & 3) << 2);
		reg |= (((src_buf_format->v_sample[2] - 1) & 3) << 0);
		writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
		break;

	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
		resz_type |= IMG_RESZ_SEL_OSD_MODE;
		osd_setting |= IMG_RESZ_OSD_DIRECT_MODE;

		osd_setting &= ~(IMG_RESZ_OSD_DIRECT_RGB565 |
					IMG_RESZ_OSD_DIRECT_ARGB1555 |
					IMG_RESZ_OSD_DIRECT_ARGB4444 |
					IMG_RESZ_OSD_DIRECT_ARGB8888);
		switch (src_buf_format->argb_format) {
		case IMGRESZ_ARGB_FORMAT_0565:
			osd_setting |= IMG_RESZ_OSD_DIRECT_RGB565;
			break;
		case IMGRESZ_ARGB_FORMAT_1555:
			osd_setting |= IMG_RESZ_OSD_DIRECT_ARGB1555;
			break;
		case IMGRESZ_ARGB_FORMAT_4444:
			osd_setting |= IMG_RESZ_OSD_DIRECT_ARGB4444;
			break;
		case IMGRESZ_ARGB_FORMAT_8888:
			osd_setting |= IMG_RESZ_OSD_DIRECT_ARGB8888;
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	writel_relaxed(resz_type, base + RW_IMG_RESZ_TYPE);
	writel_relaxed(osd_setting, base + RW_IMG_RESZ_OSD_MODE_SETTING);

	return 0;
}

void
imgresz_hal_set_src_buf_pitch(void __iomem *base, uint32_t buf_width,
			      const struct imgresz_buf_format *src_buf_format)
{
	uint32_t buf_width_y, buf_width_c;

	if (src_buf_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR) {
		buf_width_y = buf_width;
		/* For 411, 420 or 444 jpg */
		if ((src_buf_format->h_sample[1] < src_buf_format->h_sample[0]) &&
		    (src_buf_format->h_sample[2] < src_buf_format->h_sample[0])) {
			buf_width_c = IMGALIGN(buf_width /
				(src_buf_format->h_sample[0] / src_buf_format->h_sample[1]), 16);
		} else {
			buf_width_c = buf_width;
		}
		writel_relaxed((((buf_width_c >> 4) & 0x3FFF) << 12) | (buf_width_y >> 4),
			       base + RW_IMG_RESZ_SRC_BUF_LEN);
	} else {
		writel_relaxed((((buf_width >> 4) & 0x3FFF) << 12) | (buf_width >> 4),
			       base + RW_IMG_RESZ_SRC_BUF_LEN);
	}
}

void
imgresz_hal_set_src_rowbuf_height(void __iomem *base, uint32_t row_buf_hei,
				  const struct imgresz_buf_format *src_buf_format,
				  bool rm_rpr_racing_mode)
{
	u32 reg;
	u32 row_hei_y = 0, row_hei_cb = 0, row_hei_cr = 0;

	if (row_buf_hei == 0) {
		reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);
		reg &= ~IMG_RESZ_LINES_ASSIGNED_DIRECTLY;
		writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
	} else {
		reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);
		reg |= IMG_RESZ_LINES_ASSIGNED_DIRECTLY;
		writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);

		row_hei_y = row_buf_hei;

		if (rm_rpr_racing_mode)
			row_hei_cb = row_buf_hei / 2;

		if (src_buf_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR) {
			row_hei_cb = row_buf_hei *
					src_buf_format->v_sample[1] /
					src_buf_format->v_sample[0];
			row_hei_cr = row_buf_hei *
					src_buf_format->v_sample[2] /
					src_buf_format->v_sample[0];
		}
		writel_relaxed(row_hei_y, base + RW_IMG_RESZ_LINE_NUM_IN_Y_COLOR_BUF);
		writel_relaxed(row_hei_cb, base + RW_IMG_RESZ_LINE_NUM_IN_CB_COLOR_BUF);
		writel_relaxed(row_hei_cr, base + RW_IMG_RESZ_LINE_NUM_IN_CR_COLOR_BUF);
	}
}

int imgresz_hal_set_src_pic_wid_hei(void __iomem *base, uint32_t width, uint32_t height,
				    const struct imgresz_buf_format *src_format)
{
	unsigned int src_width, src_height;

	switch (src_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		writel_relaxed(((width & 0xFFFF) << 16) | (height & 0xFFFF),
			       base + RW_IMG_RESZ_SRC_SIZE_Y);
		switch (src_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
			writel_relaxed((((width / 2) & 0xFFFF) << 16) | ((height / 2) & 0xFFFF),
				       base + RW_IMG_RESZ_SRC_SIZE_CB);
			break;
		case IMGRESZ_YUV_FORMAT_422:
			writel_relaxed((((width / 2) & 0xFFFF) << 16) | (height & 0xFFFF),
				       base + RW_IMG_RESZ_SRC_SIZE_CB);
			break;
		default:
			break;
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		writel_relaxed(((width & 0xFFFF) << 16) | (height & 0xFFFF),
			       base + RW_IMG_RESZ_SRC_SIZE_Y);

		if ((src_format->h_sample[1] != 0) && (src_format->v_sample[1] != 0)) {
			src_width = width * src_format->h_sample[1] / src_format->h_sample[0];
			src_height = height * src_format->v_sample[1] / src_format->v_sample[0];
			/* For jpeg picture mode, prevent source height 401 come
			 * two interrupt (Y interrupt and C interrupt)
			 */
			if ((src_height * src_format->v_sample[0]) != (height * src_format->v_sample[1]))
				src_height++;
			writel_relaxed(((src_width & 0xFFFF) << 16) | (src_height & 0xFFFF),
				       base + RW_IMG_RESZ_SRC_SIZE_CB);
		}

		if ((src_format->h_sample[2] != 0) && (src_format->v_sample[2] != 0)) {
			src_width = width * src_format->h_sample[2] / src_format->h_sample[0];
			src_height = height * src_format->v_sample[2] / src_format->v_sample[0];
			/* For jpeg picture mode, prevent source height 401 come
			 * two interrupt (Y interrupt and C interrupt)
			 */
			if ((src_height * src_format->v_sample[0]) != (height * src_format->v_sample[2]))
				src_height++;
			writel_relaxed(((src_width & 0xFFFF) << 16) | (src_height & 0xFFFF),
				       base + RW_IMG_RESZ_SRC_SIZE_CR);
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		writel_relaxed(((width & 0xFFFF) << 16) | (height & 0xFFFF),
			       base + RW_IMG_RESZ_SRC_SIZE_Y);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int imgresz_hal_set_src_pic_offset(void __iomem *base,
				   unsigned int x_offset, unsigned int y_offset,
				   const struct imgresz_buf_format *src_format)
{
	unsigned int x_off1 = 0, y_off1 = 0;
	unsigned int x_off2 = 0, y_off2 = 0;
	unsigned int x_off3 = 0, y_off3 = 0;
	unsigned int h_sample, v_sample;

	x_off1 = x_offset;
	y_off1 = y_offset;

	switch (src_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		switch (src_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
			x_off2 = x_offset >> 1;
			y_off2 = y_offset >> 1;
			break;
		case IMGRESZ_YUV_FORMAT_422:
			x_off2 = x_offset >> 1;
			y_off2 = y_offset;
			break;
		default:
			break;
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		if ((src_format->h_sample[1] != 0) && (src_format->v_sample[1] != 0)) {
			h_sample = src_format->h_sample[0] / src_format->h_sample[1];
			v_sample = src_format->v_sample[0] / src_format->v_sample[1];
			x_off2 = x_offset / h_sample;
			y_off2 = y_offset / v_sample;
		} else {
			x_off2 = 0;
			y_off2 = 0;
		}

		if ((src_format->h_sample[2] != 0) && (src_format->v_sample[2] != 0)) {
			h_sample = src_format->h_sample[0] / src_format->h_sample[2];
			v_sample = src_format->v_sample[0] / src_format->v_sample[2];
			x_off3 = x_offset / h_sample;
			y_off3 = y_offset / v_sample;
		} else {
			x_off3 = 0;
			y_off3 = 0;
		}
		break;
	default:
		break;
	}
	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695) {
		writel_relaxed(((x_off1 & 0xFFFF) << 16) | (y_off1 & 0xFFFF), base + RW_IMG_RESZ_SRC_OFFSET_Y);
		writel_relaxed(((x_off2 & 0xFFFF) << 16) | (y_off2 & 0xFFFF), base + RW_IMG_RESZ_SRC_OFFSET_CB);
		writel_relaxed(((x_off3 & 0xFFFF) << 16) | (y_off3 & 0xFFFF), base + RW_IMG_RESZ_SRC_OFFSET_CR);
	} else {
		writel_relaxed(((x_off1 & 0xFFF) << 12) | (y_off1 & 0xFFF), base + RW_IMG_RESZ_SRC_OFFSET_Y);
		writel_relaxed(((x_off2 & 0xFFF) << 12) | (y_off2 & 0xFFF), base + RW_IMG_RESZ_SRC_OFFSET_CB);
		writel_relaxed(((x_off3 & 0xFFF) << 12) | (y_off3 & 0xFFF), base + RW_IMG_RESZ_SRC_OFFSET_CR);
	}
	return 0;
}

/*just for vdo partition*/
void imgresz_hal_set_src_offset_vdo_partition(void __iomem *base,
	unsigned int y_h_offset, unsigned int y_v_offset, unsigned int c_h_offset)
{
	writel_relaxed(((y_h_offset & 0xFFFF) << 16) | (y_v_offset & 0xFFFF), base + RW_IMG_RESZ_SRC_OFFSET_Y);
	writel_relaxed(((c_h_offset & 0xFFFF) << 16) | ((y_v_offset >> 1) & 0xFFFF), base + RW_IMG_RESZ_SRC_OFFSET_CB);
}


void imgresz_hal_set_src_rm_rpr(void __iomem *base, bool rpr_mode, bool rpr_racing)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_RPR);
	if (rpr_mode)
		reg |= IMG_RESZ_RPR_FLAG_ON;
	else
		reg &= ~IMG_RESZ_RPR_FLAG_ON;

	if (rpr_racing) {
		reg &= ~(0x1 << 10);
		reg |= IMG_RESZ_TRC_VDEC_EN;
		reg |= IMG_RESZ_TRC_VDEC_INT;
	} else {
		reg &= ~IMG_RESZ_TRC_VDEC_EN;
		reg &= ~IMG_RESZ_TRC_VDEC_INT;
	}
	writel_relaxed(reg, base + RW_IMG_RESZ_RPR);
}

void imgresz_hal_set_src_first_row(void __iomem *base, bool firstrow)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);
	if (firstrow)
		reg |= IMG_RESZ_FIRST_BLOCK_LINE;
	else
		reg &= ~IMG_RESZ_FIRST_BLOCK_LINE;
	writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
}

void imgresz_hal_set_src_last_row(void __iomem *base, bool lastrow)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);
	if (lastrow)
		reg |= IMG_RESZ_LAST_BLOCK_LINE;
	else
		reg &= ~IMG_RESZ_LAST_BLOCK_LINE;
	writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
}

void imgresz_hal_set_vdo_cbcr_swap(void __iomem *base, bool cbcr_swap)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);
	if (cbcr_swap)
		reg |= IMG_RESZ_CBCRSWAP;
	else
		reg &= ~IMG_RESZ_CBCRSWAP;
	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);
}

int imgresz_hal_set_dst_buf_format(void __iomem *base,
			       const struct imgresz_buf_format *src_format,
			       const struct imgresz_buf_format *dst_format)
{
	u32 reg, osd_setting, addr_swap_setting, csc_setting;

	reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);
	osd_setting = readl_relaxed(base + RW_IMG_RESZ_OSD_MODE_SETTING);
	addr_swap_setting = readl_relaxed(base + RW_IMG_RESZ_MEM_IF_MODE);
	csc_setting = readl_relaxed(base + RW_IMG_RESZ_OSD_CSC_SETTING);

	reg &= ~IMG_RESZ_V2OSD; /* If not clear, OSD mode will incorrect. */
	csc_setting &= ~IMG_RESZ_OSD_CSC_ENABLE;

	switch (dst_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		/* Set destination buffer YUV format */
		reg &= ~(IMG_RESZ_420_OUT | IMG_RESZ_422_OUT | IMG_RESZ_444_OUT);
		switch (dst_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
			reg |= IMG_RESZ_420_OUT;
			break;
		case IMGRESZ_YUV_FORMAT_422:
			reg |= IMG_RESZ_422_OUT;
			break;
		case IMGRESZ_YUV_FORMAT_444:
			reg |= IMG_RESZ_444_OUT;
			break;
		default:
			reg |= IMG_RESZ_420_OUT;
			break;
		}
		/* Set destination buffer Raster Scan/Block Mode format */
		if (dst_format->block)
			reg &= ~IMG_RESZ_RASTER_SCAN_OUT;
		else
			reg |= IMG_RESZ_RASTER_SCAN_OUT;
	break;
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
		osd_setting &= ~(IMG_RESZ_OSD_OUTPUT_RGB565 | IMG_RESZ_OSD_OUTPUT_ARGB1555 |
		  IMG_RESZ_OSD_OUTPUT_ARGB4444 | IMG_RESZ_OSD_OUTPUT_ARGB8888);
		switch (dst_format->argb_format) {
		case IMGRESZ_ARGB_FORMAT_0565:
			osd_setting |= IMG_RESZ_OSD_OUTPUT_RGB565;
			break;
		case IMGRESZ_ARGB_FORMAT_1555:
			osd_setting |= IMG_RESZ_OSD_OUTPUT_ARGB1555;
			break;
		case IMGRESZ_ARGB_FORMAT_4444:
			osd_setting |= IMG_RESZ_OSD_OUTPUT_ARGB4444;
			break;
		case IMGRESZ_ARGB_FORMAT_8888:
			osd_setting |= IMG_RESZ_OSD_OUTPUT_ARGB8888;
			break;
		default:
			osd_setting |= IMG_RESZ_OSD_OUTPUT_ARGB8888;
			break;
		}

		if (dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) {
			switch (src_format->mainformat) {
			case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
			case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
				reg |= IMG_RESZ_V2OSD;
				reg &= ~(IMG_RESZ_420_OUT |
						IMG_RESZ_422_OUT |
						IMG_RESZ_444_OUT);
				reg |= IMG_RESZ_444_OUT;
				break;
			case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
				csc_setting |= IMG_RESZ_OSD_CSC_ENABLE;
				/* Do not down-scale Y */
				csc_setting &= ~IMG_RESZ_OSD_CSC_YIN_D16;
				csc_setting &= ~IMG_RESZ_OSD_CSC_CIN_D128;
				csc_setting &= ~IMG_RESZ_OSD_CSC_YOUT_A16;
				csc_setting |= IMG_RESZ_OSD_CSC_COUT_A128;
				writel_relaxed(0x132, base + RW_IMG_RESZ_OSD_CSC_COEF11);
				writel_relaxed(0x259, base + RW_IMG_RESZ_OSD_CSC_COEF12);
				writel_relaxed(0x75, base + RW_IMG_RESZ_OSD_CSC_COEF13);
				writel_relaxed(0x1F50, base + RW_IMG_RESZ_OSD_CSC_COEF21);
				writel_relaxed(0x1EA5, base + RW_IMG_RESZ_OSD_CSC_COEF22);
				writel_relaxed(0x20B, base + RW_IMG_RESZ_OSD_CSC_COEF23);
				writel_relaxed(0x20B, base + RW_IMG_RESZ_OSD_CSC_COEF31);
				writel_relaxed(0x1E4A, base + RW_IMG_RESZ_OSD_CSC_COEF32);
				writel_relaxed(0x1FAB, base + RW_IMG_RESZ_OSD_CSC_COEF33);
				break;
			default:
				break;
			}
		} else if (dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) {
			switch (src_format->mainformat) {
			case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
				csc_setting |= IMG_RESZ_OSD_CSC_ENABLE;
				/* Do not down-scale Y */
				csc_setting &= ~IMG_RESZ_OSD_CSC_YIN_D16;
				csc_setting |= IMG_RESZ_OSD_CSC_CIN_D128;
				csc_setting &= ~IMG_RESZ_OSD_CSC_YOUT_A16;
				csc_setting &= ~IMG_RESZ_OSD_CSC_COUT_A128;
				writel_relaxed(0x400, base + RW_IMG_RESZ_OSD_CSC_COEF11);
				writel_relaxed(0x0, base + RW_IMG_RESZ_OSD_CSC_COEF12);
				writel_relaxed(0x57C, base + RW_IMG_RESZ_OSD_CSC_COEF13);
				writel_relaxed(0x400, base + RW_IMG_RESZ_OSD_CSC_COEF21);
				writel_relaxed(0x1EA8, base + RW_IMG_RESZ_OSD_CSC_COEF22);
				writel_relaxed(0x1D35, base + RW_IMG_RESZ_OSD_CSC_COEF23);
				writel_relaxed(0x400, base + RW_IMG_RESZ_OSD_CSC_COEF31);
				writel_relaxed(0x6EE, base + RW_IMG_RESZ_OSD_CSC_COEF32);
				writel_relaxed(0x0, base + RW_IMG_RESZ_OSD_CSC_COEF33);
				break;
			default:
				break;
			}
		}
		break;
	default:
		return -EINVAL;
	}

	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);
	writel_relaxed(osd_setting, base + RW_IMG_RESZ_OSD_MODE_SETTING);
	writel_relaxed(addr_swap_setting, base + RW_IMG_RESZ_MEM_IF_MODE);
	writel_relaxed(csc_setting, base + RW_IMG_RESZ_OSD_CSC_SETTING);

	return 0;
}

void imgresz_hal_set_dst_buf_addr(void __iomem *base, uint32_t addr_y, uint32_t addr_c)
{
	writel_relaxed(addr_y >> 4, base + RW_IMG_RESZ_TGT_Y_ADDR_BASE);
	writel_relaxed(addr_c >> 4, base + RW_IMG_RESZ_TGT_C_ADDR_BASE);
}

void imgresz_hal_set_dst_buf_pitch(void __iomem *base, uint32_t buf_width)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_TGT_BUF_LEN);
	reg = (reg & 0xFF000000) |
		(((buf_width >> 4) & 0x3FFF) << 12) |
		(buf_width >> 4);
	writel_relaxed(reg, base + RW_IMG_RESZ_TGT_BUF_LEN);
}

void imgresz_hal_set_dst_pic_wid_hei(void __iomem *base, uint32_t width, uint32_t height)
{
	writel_relaxed(((width & 0xFFFF) << 16) | (height & 0xFFFF),
		       base + RW_IMG_RESZ_TGT_SIZE);
}

void imgresz_hal_set_dst_pic_offset(void __iomem *base,
				    unsigned int x_offset, unsigned int y_offset)
{
	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695) {
		writel_relaxed(((x_offset & 0xFFFF) << 16) | (y_offset & 0xFFFF),
				base + RW_IMG_RESZ_TGT_OFFSET);
	} else {
		writel_relaxed(((x_offset & 0xFFF) << 12) | (y_offset & 0xFFF),
				base + RW_IMG_RESZ_TGT_OFFSET);
	}
}

static void imgresz_hal_set_linebuflen_ext16(void __iomem *base, bool enable)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_TGT_BUF_LEN);

	if (enable)
		reg |= IMG_RESZ_BOUND_EXTEND_16_ON;
	else
		reg &= ~IMG_RESZ_BOUND_EXTEND_16_ON;

	writel_relaxed(reg, base + RW_IMG_RESZ_TGT_BUF_LEN);
}

static void imgresz_hal_set_sram_linebuflen(void __iomem *base, uint32_t linebuf)
{
	u32 mask = ~(0x1F << IMG_RESZ_LINE_BUFFER_LEN_SHIFT);
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_TGT_BUF_LEN);

	reg &= mask;
	reg += (linebuf << IMG_RESZ_LINE_BUFFER_LEN_SHIFT);
	writel_relaxed(reg, base + RW_IMG_RESZ_TGT_BUF_LEN);
}

static void imgresz_ufo_linebuf_eco(void __iomem *base)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_UFO_LINEB_ECO);

	writel_relaxed(reg | IMG_RESZ_UFO_LINEB_ECO, base + RW_IMG_RESZ_UFO_LINEB_ECO);
}

/*
 * Only for UFO or vdo_argb_onephase.
 * Params: src_height only is for HW limitation., it is only for !onephase.
 * Return: linebuf in the normal case.
 *         ch1 line-buf-len while onephase case.
 */
static int imgresz_hal_calc_linebuflen(bool ufo, bool onephase, bool osdmode, bool dst_blk,
				       uint32_t src_width, uint32_t dst_width, unsigned int linebuflen)
{
	unsigned int cnt;

	if (ufo) {
		if ((src_width <= dst_width || src_width <= 992) && (!onephase)) { /* scale up */
			/* lineBuf v7 formula: ufo2osd one phase case, not care about whether width <= 1024*/
			/* 992 is for clip.*/
			linebuflen = 16;
		} else { /* scale down while srcwid > 1024. */
			/* osd one phase just use this, no matter scale up or down*/
			linebuflen = (1024 - 32) * dst_width / (src_width * 32);
		}
		if ((onephase && osdmode) || dst_blk)/* osd one phase or Dst blk need even.*/
			linebuflen &= 0xfffffffe;
		if (linebuflen > 16)
			linebuflen = 16;
	}

	if ((!ufo) && osdmode && onephase) {/* normal vdo2argb, lineB v8*/
		/* phoenix es2 verify 420vdo 520x480 onephase to argb8888 777x740 shows garbage in the right */
		if (src_width < dst_width) {
			while ((dst_width % (linebuflen * 32) <= 16) && (linebuflen > 1))
				linebuflen--;
		}
	}

	if (linebuflen < 1)
		linebuflen = 1;

	cnt = dst_width / (linebuflen * 32);
	if ((dst_width - cnt * linebuflen * 32 < 16) && (dst_width - cnt * linebuflen * 32 > 0)	&& linebuflen > 1) {
		loginfo(IMGRESZ_LOG_UFO,
			"[ImgResz] The last part wid too small:%u. SrcW:%u, DstW:%u, Count:%u, curline:%u\n",
			(dst_width - cnt * linebuflen*32), src_width, dst_width, cnt, linebuflen);
	}

	if (onephase && osdmode && ufo)/* normal vdo2argb not div 2,phoenix es2. normal vdo2argb, lineB v8*/
		linebuflen /= 2;

	loginfo(IMGRESZ_LOG_UFO,
		"[ImgResz] srcW:%u dstW:%u lineB:%u Hw partial NUM:%d last part:%u, ufo:%d one phase %d(%d)\n",
		src_width, dst_width, linebuflen, cnt, (dst_width - cnt * linebuflen * 32), ufo, onephase, osdmode);

	return linebuflen;
}

/*
 * Only for mt8581 limitation: "Make sure: dst-width/linebuffer*32 is even."
 * In some case(scale to very small), that condition still will fail even though
 * the linebuflen is 0.
 * Add this function for hal_if.c trying to adjust v-partition division if it meet
 * HW limitation.
 *
 * Return FAIL: HW limitation,xCalcLineBuflen below will return 0.
 */
bool imgresz_hal_survey_linebuflen_is_ok(bool ufo, bool onephase, bool dst_blk,
						uint32_t src_width, uint32_t dst_width, unsigned int linebuflen)
{
	return !!imgresz_hal_calc_linebuflen(ufo, onephase, false, dst_blk,
									src_width, dst_width, linebuflen);
}

int
imgresz_hal_set_linebuflen(void __iomem *base,
			   enum IMGRESZ_RESAMPLE_METHOD_T v_method,
			   uint32_t src_width, uint32_t src_height,
			   uint32_t dst_width, uint32_t dst_height,
			   bool ufo, bool rpr_mode, bool one_phase,
			   bool osd_mode, bool dst_blk, bool dst_10bit)
{
	unsigned int linebuflen = 0x10;
	bool use_extend16 = 0;

	switch (v_method) {
	case IMGRESZ_RESAMPLE_METHOD_M_TAP:
		if (src_height < dst_height) {
			linebuflen = 0x1F;
			use_extend16 = 0;
		} else {
			use_extend16 = 1;
			linebuflen = 0x10;
		}

		/* Adjust tmp linebuflen for HW */
		if (osd_mode) {
			while (((dst_width * 4) % (linebuflen << 5)) <= 8) {
				linebuflen--;
				if (linebuflen == 0)
					return -EINVAL;
			}
		} else {
			while ((dst_width % (linebuflen << 5)) <= 8) {
				linebuflen--;
				if (linebuflen == 0)
					return -EINVAL;
			}
		}
		break;
	case IMGRESZ_RESAMPLE_METHOD_4_TAP:
		use_extend16 = 0;
		break;
	default:
		return -EINVAL;
	}

	if (dst_10bit)	/* 10bit can not extend 16. */
		use_extend16 = false;
	#if DRV_VERIFY_SUPPORT
	else if (ufo || one_phase)
		use_extend16 = false;	/* c-model always disable ext16. */
	#endif
	imgresz_hal_set_linebuflen_ext16(base, use_extend16);

	if (!(one_phase && osd_mode))	/* osd & one phase cal line buffer in i4HwImgReszSetARGBOnePhase*/
		linebuflen = imgresz_hal_calc_linebuflen(ufo, false, osd_mode, dst_blk,
						 src_width, dst_width, linebuflen);

	imgresz_ufo_linebuf_eco(base);
	if (rpr_mode)
		linebuflen = 0x10;
	imgresz_hal_set_sram_linebuflen(base, linebuflen);

	return 0;
}

void imgresz_hal_set_tempbuf(void __iomem *base, uint32_t addr)
{
	u32 reg;
	/* Temp buffer always No-iommu. */
	writel_relaxed(addr >> 4, base + RW_IMG_RESZ_TMP_ADDR_BASE);

	/* For temp buf agent to path PA */
	reg = readl_relaxed(base + RW_IMG_RESZ_MEM_IF_MODE);
	reg |= IMG_RESZ_MMU_TEMPBUF_AGENTID_MASK;
	writel_relaxed(reg, base + RW_IMG_RESZ_MEM_IF_MODE);
}

void imgresz_hal_jpg_picmode_enable(void __iomem *base)
{
	writel_relaxed(IMG_RESZ_TRACKING_WITH_JPG_HW,
		       base + RW_IMG_RESZ_INTERFACE_SWITCH);
}

void imgresz_hal_jpg_preload_enable(void __iomem *base)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);

	reg |= IMG_RESZ_PRELOAD_DRAM_DATA;
	writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
}

void imgresz_hal_jpg_component(void __iomem *base, bool y_exist,
			       bool cb_exist, bool cr_exist)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);

	if (y_exist)
		reg |= IMG_RESZ_RECORD_Y;
	else
		reg &= ~IMG_RESZ_RECORD_Y;

	if (cb_exist)
		reg |= IMG_RESZ_RECORD_CB;
	else
		reg &= ~IMG_RESZ_RECORD_CB;

	if (cr_exist)
		reg |= IMG_RESZ_RECORD_CR;
	else
		reg &= ~IMG_RESZ_RECORD_CR;

	writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
}

void imgresz_hal_jpg_component_ex(void __iomem *base, bool y_exist,
				  bool cb_exist, bool cr_exist, bool rpr_racing)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_JPG_MODE);

	if ((!(y_exist && cb_exist && cr_exist)) || rpr_racing) {
		reg &= 0xFFFFF000;
		reg |= 0x0 << 10;
		reg |= 0x0 & 3 << 8;
		reg |= 0x0 & 3 << 6;
		reg |= 0x0 & 3 << 4;
		reg |= 0x0 & 3 << 2;
		reg |= 0x0 & 3 << 0;

		writel_relaxed(reg, base + RW_IMG_RESZ_JPG_MODE);
	}
}

void imgresz_hal_jpg_cbcr_pad(void __iomem *base, bool cb_exist, bool cr_exist)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);

	if (!cb_exist)
		reg |= IMG_RESZ_CB_PADDING;
	if (!cr_exist)
		reg |= IMG_RESZ_CR_PADDING;

	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);
}

void imgresz_hal_jpg_preload_buf(void __iomem *base, uint32_t addr_y, uint32_t addr_c)
{
	writel_relaxed(addr_y >> 4, base + RW_IMG_RESZ_Y_PRELOAD_OW_ADDR_BASE);
	writel_relaxed(addr_c >> 4, base + RW_IMG_RESZ_C_PRELOAD_OW_ADDR_BASE);
}

void imgresz_hal_set_scaling_type(void __iomem *base, unsigned int type)
{
	u32 reg = readl_relaxed(base + RW_IMG_RESZ_OSD_MODE_SETTING);

	switch (type) {
	case 0:
		reg |= IMG_RESZ_OSD_ALPHA_SCALE_NORMAL;
		break;
	case 1:
		reg |= IMG_RESZ_OSD_ALPHA_SCALE_REF_LEFT;
		break;
	case 2:
		reg |= IMG_RESZ_OSD_ALPHA_SCALE_REF_NEAREST;
		break;
	default:
		reg |= IMG_RESZ_OSD_ALPHA_SCALE_NORMAL;
		break;
	}
	writel_relaxed(reg, base + RW_IMG_RESZ_OSD_MODE_SETTING);
}

static void imgresz_hal_coeff_set_h8_y(void __iomem *base,
			   unsigned int offset, unsigned int factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 == (factor & 0xFC000000)) {
		writel_relaxed(reg, base + RW_IMG_RESZ_H8TAPS_FAC_Y);
		writel_relaxed(offset, base + RW_IMG_RESZ_H8TAP_OFSET_Y);
	} else {
		logwarn("coeff_set_h8_y: factor too large!\n");
	}
}

inline unsigned int imgresz_hal_coeff_get_h8_y(void __iomem *base)
{
	return readl_relaxed(base + RW_IMG_RESZ_H8TAPS_FAC_Y);
}

static void imgresz_hal_coeff_set_h8_cb(void __iomem *base,
			    unsigned int offset, unsigned int factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 == (factor & 0xFC000000)) {
		writel_relaxed(reg, base + RW_IMG_RESZ_H8TAPS_FAC_CB);
		writel_relaxed(offset, base + RW_IMG_RESZ_H8TAP_OFSET_CB);
	} else {
		logwarn("coeff_set_h8_cb: factor too large!\n");
	}
}

unsigned int imgresz_hal_coeff_get_h8_cb(void __iomem *base)
{
	return readl_relaxed(base + RW_IMG_RESZ_H8TAPS_FAC_CB);
}

static void imgresz_hal_coeff_set_h8_cr(void __iomem *base,
			    unsigned int offset, unsigned int factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 == (factor & 0xFC000000)) {
		writel_relaxed(reg, base + RW_IMG_RESZ_H8TAPS_FAC_CR);
		writel_relaxed(offset, base + RW_IMG_RESZ_H8TAP_OFSET_CR);
	} else {
		logwarn("coeff_set_h8_cr: factor too large!\n");
	}
}

static void
imgresz_hal_coeff_set_h8_coeffs(void __iomem *base,
				enum IMGRZ_SCALE_FACTOR factor)
{
	writel_relaxed(h_filtercoeff[factor][0], base + RW_IMG_RESZ_H_COEF0);
	writel_relaxed(h_filtercoeff[factor][1], base + RW_IMG_RESZ_H_COEF1);
	writel_relaxed(h_filtercoeff[factor][2], base + RW_IMG_RESZ_H_COEF2);
	writel_relaxed(h_filtercoeff[factor][3], base + RW_IMG_RESZ_H_COEF3);
	writel_relaxed(h_filtercoeff[factor][4], base + RW_IMG_RESZ_H_COEF4);
	writel_relaxed(h_filtercoeff[factor][5], base + RW_IMG_RESZ_H_COEF5);
	writel_relaxed(h_filtercoeff[factor][6], base + RW_IMG_RESZ_H_COEF6);
	writel_relaxed(h_filtercoeff[factor][7], base + RW_IMG_RESZ_H_COEF7);
	writel_relaxed(h_filtercoeff[factor][8], base + RW_IMG_RESZ_H_COEF8);
	writel_relaxed(h_filtercoeff[factor][9], base + RW_IMG_RESZ_H_COEF9);
	writel_relaxed(h_filtercoeff[factor][10], base + RW_IMG_RESZ_H_COEF10);
	writel_relaxed(h_filtercoeff[factor][11], base + RW_IMG_RESZ_H_COEF11);
	writel_relaxed(h_filtercoeff[factor][12], base + RW_IMG_RESZ_H_COEF12);
	writel_relaxed(h_filtercoeff[factor][13], base + RW_IMG_RESZ_H_COEF13);
	writel_relaxed(h_filtercoeff[factor][14], base + RW_IMG_RESZ_H_COEF14);
	writel_relaxed(h_filtercoeff[factor][15], base + RW_IMG_RESZ_H_COEF15);
	writel_relaxed(h_filtercoeff[factor][16], base + RW_IMG_RESZ_H_COEF16);
	writel_relaxed(h_filtercoeff[factor][17], base + RW_IMG_RESZ_H_COEF17);
}

static void
imgresz_hal_coeff_set_v4_coeffs(void __iomem *base, enum IMGRZ_SCALE_FACTOR factor)
{
	writel_relaxed(v_filtercoeff[factor][0], base + RW_IMG_RESZ_V_COEF0);
	writel_relaxed(v_filtercoeff[factor][1], base + RW_IMG_RESZ_V_COEF1);
	writel_relaxed(v_filtercoeff[factor][2], base + RW_IMG_RESZ_V_COEF2);
	writel_relaxed(v_filtercoeff[factor][3], base + RW_IMG_RESZ_V_COEF3);
	writel_relaxed(v_filtercoeff[factor][4], base + RW_IMG_RESZ_V_COEF4);
	writel_relaxed(v_filtercoeff[factor][5], base + RW_IMG_RESZ_V_COEF5);
	writel_relaxed(v_filtercoeff[factor][6], base + RW_IMG_RESZ_V_COEF6);
	writel_relaxed(v_filtercoeff[factor][7], base + RW_IMG_RESZ_V_COEF7);
	writel_relaxed(v_filtercoeff[factor][8], base + RW_IMG_RESZ_V_COEF8);
}

static enum IMGRZ_SCALE_FACTOR
imgresz_hal_coeff_get_factor(uint32_t src_size, uint32_t dst_size)
{
	uint32_t scale_ratio = dst_size * 10000 / src_size;

	if (scale_ratio >= 10000)
		return FACTOR_1;
	else if (scale_ratio >= 5000)
		return FACTOR_0_5;
	else if (scale_ratio >= 2500)
		return FACTOR_0_25;
	else if (scale_ratio >= 1250)
		return FACTOR_0_125;
	else if (scale_ratio >= 625)
		return FACTOR_0_0625;
	else
		return FACTOR_0;
}

static void
imgresz_hal_coeff_set_hsa_y(void __iomem *base, unsigned int offset,
			    unsigned int factor)
{
	u32 reg = factor & 0x00000FFF;

	reg += ((offset & 0x000007FF) << 12);
	writel_relaxed(reg, base + RW_IMG_RESZ_HSA_SCL_Y);
}

static void
imgresz_hal_coeff_set_hsa_cb(void __iomem *base, unsigned int offset,
			     unsigned int factor)
{
	u32 reg = factor & 0x00000FFF;

	reg += ((offset & 0x000007FF) << 12);
	writel_relaxed(reg, base + RW_IMG_RESZ_HSA_SCL_CB);
}

static void
imgresz_hal_coeff_set_hsa_cr(void __iomem *base, unsigned int offset,
			     unsigned int factor)
{
	u32 reg = factor & 0x00000FFF;

	reg += ((offset & 0x000007FF) << 12);
	writel_relaxed(reg, base + RW_IMG_RESZ_HSA_SCL_CR);
}

static int
imgresz_hal_coeff_set_v4_y(void __iomem *base, unsigned int offset,
			   unsigned int factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 != (factor & 0xFC000000))
		return -EINVAL;

	writel_relaxed(reg, base + RW_IMG_RESZ_V4TAPS_SCL_Y);
	writel_relaxed(offset, base + RW_IMG_RESZ_V4TAP_OFSET_Y);

	return 0;
}

static int
imgresz_hal_coeff_set_v4_cb(void __iomem *base, unsigned int offset,
			    unsigned int  factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 != (factor & 0xF3000000))
		return -EINVAL;

	writel_relaxed(reg, base + RW_IMG_RESZ_V4TAPS_SCL_CB);
	writel_relaxed(offset, base + RW_IMG_RESZ_V4TAP_OFSET_C);

	return 0;
}

static int
imgresz_hal_coeff_set_v4_cr(void __iomem *base, unsigned int offset,
			    unsigned int factor)
{
	u32 reg = factor & 0x03FFFFFF;

	if (0 != (factor & 0xF3000000))
		return -EINVAL;

	writel_relaxed(reg, base + RW_IMG_RESZ_V4TAPS_SCL_CR);

	return 0;
}

static int
imgresz_hal_coeff_set_vscale_y(void __iomem *base, unsigned int offset,
			       unsigned int factor, bool scale_up)
{
	u32 reg = factor & 0x000007FF;

	if ((0 != (factor & 0xFFFFF800)) || (0 != (offset & 0xFFFFF000)))
		return -EINVAL;

	reg += ((offset & 0x000007FF) << 12) + ((scale_up & 0x1) << 11);
	writel_relaxed(reg, base + RW_IMG_RESZ_V_SCL_Y);

	return 0;
}

static int
imgresz_hal_coeff_set_vscale_cb(void __iomem *base, unsigned int offset,
				unsigned int factor, bool scale_up)
{
	u32 reg = factor & 0x000007FF;

	if ((0 != (factor & 0xFFFFF800)) || (0 != (offset & 0xFFFFF000)))
		return -EINVAL;

	reg += ((offset & 0x000007FF) << 12) + ((scale_up & 0x1) << 11);
	writel_relaxed(reg, base + RW_IMG_RESZ_V_SCL_CB);

	return 0;
}

static int
imgresz_hal_coeff_set_vscale_cr(void __iomem *base, unsigned int offset,
				unsigned int factor, bool scale_up)
{
	u32 reg = factor & 0x000007FF;

	if ((0 != (factor & 0xFFFFF800)) || (0 != (offset & 0xFFFFF000)))
		return -EINVAL;

	reg += ((offset & 0x000007FF) << 12) + ((scale_up & 0x1) << 11);
	writel_relaxed(reg, base + RW_IMG_RESZ_V_SCL_CR);

	return 0;
}

int imgresz_hal_coeff_set_h_factor(void __iomem *base,
			       const struct imgresz_src_buf_info *src_buf,
			       const struct imgresz_dst_buf_info *dst_buf,
			       const struct imgresz_buf_format *src_format,
			       enum IMGRESZ_RESAMPLE_METHOD_T resample_method,
			       const struct imgresz_hal_info *hal_info)
{
	/* Y */
	imgresz_hal_coeff_set_h8_y(base, hal_info->h8_offset_y, hal_info->h8_factor_y);
	imgresz_hal_coeff_set_hsa_y(base, hal_info->hsa_offset_y, hal_info->hsa_factor_y);
	if ((resample_method == IMGRESZ_RESAMPLE_METHOD_8_TAP) ||
	    (src_buf->pic_width <= dst_buf->pic_width)) {
		imgresz_hal_coeff_set_h8_coeffs(base,
			imgresz_hal_coeff_get_factor(src_buf->pic_width, dst_buf->pic_width));
	}

	if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
	    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
	    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
		return 0;

	/* Cb */
	imgresz_hal_coeff_set_h8_cb(base, hal_info->h8_offset_cb, hal_info->h8_factor_cb);
	imgresz_hal_coeff_set_hsa_cb(base, hal_info->hsa_offset_cb, hal_info->hsa_factor_cb);

	if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
		return 0;

	/* Cr */
	imgresz_hal_coeff_set_h8_cr(base, hal_info->h8_offset_cr, hal_info->h8_factor_cr);
	imgresz_hal_coeff_set_hsa_cr(base, hal_info->hsa_offset_cr, hal_info->hsa_factor_cr);

	return 0;
}

int imgresz_hal_coeff_set_v_factor(void __iomem *base,
			       const struct imgresz_src_buf_info *src_buf,
			       const struct imgresz_dst_buf_info *dst_buf,
			       const struct imgresz_buf_format *src_format,
			       enum IMGRESZ_RESAMPLE_METHOD_T resample_method,
			       const struct imgresz_hal_info *hal_info)
{

	switch (resample_method) {
	case IMGRESZ_RESAMPLE_METHOD_4_TAP:
		imgresz_hal_coeff_set_v4_y(base, hal_info->v4_offset_y, hal_info->v4_factor_y);
		if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
			break;
		imgresz_hal_coeff_set_v4_cb(base, hal_info->v4_offset_cb, hal_info->v4_factor_cb);
		if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
			break;
		imgresz_hal_coeff_set_v4_cr(base, hal_info->v4_offset_cr, hal_info->v4_factor_cr);
		imgresz_hal_coeff_set_v4_coeffs(base,
			imgresz_hal_coeff_get_factor(src_buf->pic_height, dst_buf->pic_height));
		break;

	case IMGRESZ_RESAMPLE_METHOD_M_TAP:
		imgresz_hal_coeff_set_vscale_y(base, hal_info->vm_offset_y,
				hal_info->vm_factor_y, hal_info->vm_scale_up_y);
		if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
			break;
		imgresz_hal_coeff_set_vscale_cb(base, hal_info->vm_offset_cb,
				hal_info->vm_factor_cb, hal_info->vm_scale_up_cb);
		if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
			break;
		imgresz_hal_coeff_set_vscale_cr(base, hal_info->vm_offset_cr,
				hal_info->vm_factor_cr, hal_info->vm_scale_up_cr);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int imgresz_hal_coeff_get_v4_factor(void __iomem *base)
{
	return readl_relaxed(base + RW_IMG_RESZ_V4TAPS_SCL_Y);
}

void imgresz_hal_coeff_h8tap_vdo_partition_offset(void __iomem *base,
	unsigned int tg_offset, unsigned int hfactor_y, unsigned int hfactor_c)
{
	if ((tg_offset == 0) && (hfactor_y == 0)) {
		/*just for src_w&h == tg_w&h*/
		writel_relaxed(0, base + RW_IMG_RESZ_H8TAP_OFSET_Y);
		writel_relaxed(0, base + RW_IMG_RESZ_H8TAP_OFSET_CB);
	} else {
		writel_relaxed(((tg_offset * hfactor_y) & 0x3ffff) + 0xc0000, base + RW_IMG_RESZ_H8TAP_OFSET_Y);
		writel_relaxed(((tg_offset / 2 * hfactor_c) & 0x3ffff) + 0xc0000, base + RW_IMG_RESZ_H8TAP_OFSET_CB);
	}
}


void
imgresz_hal_coeff_v4tap_vdo_partition_offset(void __iomem *base, unsigned int part1_offset)
{
	writel_relaxed(part1_offset, base + RW_IMG_RESZ_V4TAP_OFSET_Y);
	writel_relaxed(part1_offset / 2, base + RW_IMG_RESZ_V4TAP_OFSET_C);
}

void imgresz_hal_set_src_pic_wid_hei_vdo_partition(void __iomem *base,
	unsigned int y_width, unsigned int c_width, unsigned int height)
{
	writel_relaxed(((y_width & 0xFFFF) << 16) | (height & 0xFFFF), base + RW_IMG_RESZ_SRC_SIZE_Y);
	writel_relaxed(((c_width & 0xFFFF) << 16) | ((height / 2) & 0xFFFF), base + RW_IMG_RESZ_SRC_SIZE_CB);
}

void
imgresz_hal_coeff_set_rpr_H_factor(void __iomem *base, uint32_t src_width, uint32_t dst_width)
{
	unsigned int factor, offset;
	int m = 0;
	int temp;
	int h_prime;
	int D;
	int UXR;
	int ax_initial;
	int ax_increment;

	temp = src_width;
	while (temp > 0) {
		m = m + 1;
		temp = temp >> 1;
	}

	/* check for case when src_width is power of two */
	if (src_width == (1 << (m - 1)))
		m = m - 1;
	h_prime = 1 << m;
	D = (64 * h_prime) / 16;

	/* UXL and UXR are independent of row, so compute once only */
	/* numerator part */
	UXR = ((((src_width - dst_width) << 1)) << (4 + m));
	/* complete iUxR init by dividing by H with rounding to nearest integer, */
	/* half-integers away from 0 */

	if (UXR >= 0)
		UXR = (UXR + (dst_width >> 1)) / dst_width;
	else
		UXR = (UXR - (dst_width >> 1)) / dst_width;

	/* initial x displacement and the x increment are independent of row */
	/* so compute once only */
	ax_initial = UXR + (D >> 1);
	ax_increment = (h_prime << 6) + (UXR << 1);

	factor = ax_initial << (18 - (m + 6));
	offset = ax_increment << (18 - (m + 6));

	imgresz_hal_coeff_set_h8_y(base, factor, offset);
	imgresz_hal_coeff_set_h8_cb(base, factor, offset);
	imgresz_hal_coeff_set_h8_coeffs(base, FACTOR_RM);
}

void imgresz_hal_coeff_set_rpr_V_factor(void __iomem *base, uint32_t src_height,
					uint32_t dst_height)
{
	unsigned int factor, offset;
	int n = 0;
	int temp;
	int uylb;
	int uyl_inc;

	temp = src_height;
	while (temp > 0) {
		n = n + 1;
		temp = temp >> 1;
	}

	/* check for case when uInHeight is power of two */
	if (src_height == (1 << (n - 1)))
		n = n - 1;

	uylb = ((src_height - dst_height) << (n + 5)); /* numerator */
	/* complete iUyLB by dividing by V with rounding to nearest integer, */
	/* half-integers away from 0 */
	if (uylb >= 0)
		uylb = (uylb + (dst_height >> 1)) / dst_height;
	else
		uylb = (uylb - (dst_height >> 1)) / dst_height;
	uyl_inc = uylb << 1;

	factor = ((1 << (6 + n)) + uyl_inc) << (18 - (n + 6));
	offset = (uylb + (1 << (1 + n))) << (18 - (n + 6));

	imgresz_hal_coeff_set_v4_y(base, offset, factor);
	imgresz_hal_coeff_set_v4_cb(base, offset, factor);
	imgresz_hal_coeff_set_v4_coeffs(base, FACTOR_RM);
}

u32 imgresz_checksum_write(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_FLIP);
	reg |= (0x1 << 5);
	writel_relaxed(reg, base + RW_IMG_RESZ_FLIP);

	return readl_relaxed(base + RO_IMG_RESZ_CHECK_SUM_REG);
}

u32 imgresz_checksum_read(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_FLIP);
	reg &= ~(0x1 << 5);
	writel_relaxed(reg, base + RW_IMG_RESZ_FLIP);

	return readl_relaxed(base + RW_IMG_RESZ_READ_CHECKSUM);
}

bool imgresz_hal_is_done(void __iomem *base)
{
	return readl_relaxed(base + RO_IMG_RESZ_DONE) & 0x1;
}

int imgresz_hal_grace_reset(void __iomem *base)
{
	u32 tmp;

	imgresz_hal_dma_reset(base);

	if (readl_poll_timeout_atomic(base + RO_IMG_RESZ_DONE, tmp, tmp & 0x2, 10, 100000))
		return -EAGAIN;

	imgresz_hal_hw_reset(base);
	return 0;
}

void imgresz_ufo_partition_set_start_point(void __iomem *base, unsigned int x, unsigned int y)
{
	writel_relaxed(x|(y << 16), base + RW_IMG_RESZ_UFO_START_POINT);
}

void imgresz_ufo_pagesz(void __iomem *base, uint32_t srcwid, uint32_t srchei, uint32_t dstwid)
{
	unsigned int lenbuf = 512;
	u32 page_width, page_height;

	if (srcwid > dstwid)
		page_width = 1024;
	else
		page_width = lenbuf * srcwid / dstwid + 128;
	if (page_width == 1024)
		page_width -= 16;
	page_height = (srchei + 7) / 8 * 8;
	writel_relaxed((page_height << 16) | page_width,
		       base + RW_IMG_RESZ_UFO_PAGE_SZ);
	writel_relaxed((page_height << 16) | page_width,
		       base + RW_IMG_RESZ_SEC_UFO_PAGE_SZ);
}

void imgresz_ufo_poweron(void __iomem *base)
{
	u32 u4ufopower = IMG_RESZ_UFO_ON | IMG_RESZ_BITS_POWER |
			IMG_RESZ_LEN_POWER | IMG_RESZ_DRAM_CLK |
			IMG_RESZ_UFO_CLK;

	writel_relaxed(u4ufopower, base + RW_IMG_RESZ_UFO_POWER);
}

void imgresz_ufo_config(void __iomem *base, enum IMGRESZ_UFO_TYPE type)
{
	u32 reg = 0;

	if (type == IMGRESZ_UFO_10BIT_COMPACT || type == IMGRESZ_UFO_8BIT)
		reg |= IMG_RESZ_COMPRESS_EN;
	else
		reg &= ~IMG_RESZ_COMPRESS_EN;

	if (type == IMGRESZ_UFO_8BIT)
		reg &= ~(0x3 << 1);
	else /* 2:1  00 8bit  01 10bit */
		reg |= (0x1 << 1);

	reg &= ~(0x1 << 3);/* bit3 default to 0 */
	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695)
		reg |= IMG_RESZ_UFO_AUTO_TRIG;

	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_CFG);
}

void imgresz_ufo_picsz(void __iomem *base, uint32_t buf_width, uint32_t buf_height)
{
	buf_height = (buf_height + 7) / 8 * 8;

	writel_relaxed(buf_width | (buf_height << 16),
		       base + RW_IMG_RESZ_UFO_PIC_SZ);
}

void imgresz_ufo_src_buf(void __iomem *base, uint32_t y_addr, uint32_t c_addr)
{
	loginfo(IMGRESZ_LOG_REG, "[Reg] W 0x21c:%pad, 0x224:%pad\n", &y_addr, &c_addr);
	writel_relaxed(y_addr, base + RW_IMG_RESZ_UFO_Y_ADDR);
	writel_relaxed(c_addr, base + RW_IMG_RESZ_UFO_C_ADDR);
}

void
imgresz_ufo_srclen_buf(void __iomem *base, uint32_t y_len_addr,
		       uint32_t c_len_addr)
{
	writel_relaxed(y_len_addr, base + RW_IMG_RESZ_UFO_Y_LEN_ADDR);
	writel_relaxed(c_len_addr, base + RW_IMG_RESZ_UFO_C_LEN_ADDR);
}

void imgresz_ufo_trigger(void __iomem *base)
{
	/* wait for imgrz req up, then trig ufo_dec */
	while (readl_relaxed(base + RO_IMG_RESZ_STATUS_MONITOR_REG) & IMG_RESZ_RD_REQ)
		break;

	writel(IMG_RESZ_UFO_TRIG, base + RW_IMG_RESZ_UFO_TRIG);
}

void imgresz_ufo_10bit_output_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_TYPE);
	reg |= IMG_RESZ_10BIT_OUT_CALC | IMG_RESZ_10BIT_OUT | IMG_RESZ_10BIT_ON;
	writel_relaxed(reg, base + RW_IMG_RESZ_TYPE);
}

void imgresz_ufo_10bit_jump_mode_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_JUMPMODE);
	reg |= IMG_RESZ_UFO_JUMPMODE_EN;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_JUMPMODE);
}


/* husky es2:instead ufo idle polling to fix when imgrz done but ufo not done issue */
void imgresz_ufo_idle_int_on(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_POWER);
	reg |= IMG_RESZ_UFO_INT_ON | IMG_RESZ_UFO_LINEB_READY;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_POWER);
}

void imgresz_ufo_outstanding_enable(void __iomem *base)
{
	u32 reg;

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_POWER);
	reg |= IMG_RESZ_UFO_OUTSTANDING;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_POWER);
}

int imgresz_ufo_vdo_v_partition_offset_update(
	void __iomem *base, uint32_t src_buf_wid, uint32_t part1_src_bgn)
{
	u32 reg, c_offset;
	u32 tmp;
	bool fg8bit;

	/* hw request part1 v offset should be align 32.*/
	if (part1_src_bgn & 0x1f)
		return -EINVAL;

	tmp = src_buf_wid * part1_src_bgn / 4 / 64;
	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_Y_LEN_ADDR);
	reg += tmp;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_Y_LEN_ADDR);

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_C_LEN_ADDR);
	reg += tmp / 2;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_C_LEN_ADDR);

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_CFG);
	fg8bit = !((reg >> 1) & 0x3);
	c_offset = fg8bit ? 16 : 20;

	tmp = (src_buf_wid * part1_src_bgn);
	if (!fg8bit)
		tmp = tmp / 4 * 5;

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_Y_ADDR);
	reg += tmp;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_Y_ADDR);

	reg = readl_relaxed(base + RW_IMG_RESZ_UFO_C_ADDR);
	reg += tmp / 2;
	writel_relaxed(reg, base + RW_IMG_RESZ_UFO_C_ADDR);

	return 0;
}

void imgresz_debug_vdo_partition_setting(void __iomem *base)
{
	pr_info("src_width_y(0x40)  :0x%8x 0x%8x\n",
		(readl_relaxed(base + RW_IMG_RESZ_SRC_SIZE_Y) & 0xffff0000) >> 16,
		readl_relaxed(base + RW_IMG_RESZ_SRC_SIZE_Y) & 0xffff);
	pr_info("src_width_c(0x44)  :0x%8x 0x%8x\n",
		(readl_relaxed(base + RW_IMG_RESZ_SRC_SIZE_CB) & 0xffff0000) >> 16,
		readl_relaxed(base + RW_IMG_RESZ_SRC_SIZE_CB) & 0xffff);
	pr_info("tg_y (0x4c)        :0x%8x 0x%8x\n",
		(readl_relaxed(base + RW_IMG_RESZ_TGT_SIZE) & 0xffff0000) >> 16,
		readl_relaxed(base + RW_IMG_RESZ_TGT_SIZE) & 0xffff);
	pr_info("Page_start (0x218) :0x%8x 0x%8x\n",
		(readl_relaxed(base + RW_IMG_RESZ_UFO_START_POINT) & 0xffff0000) >> 16,
		readl_relaxed(base + RW_IMG_RESZ_UFO_START_POINT) & 0xffff);

	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695) {
		pr_info("rd_h_offset_y(0x54):0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_Y) & 0xffff0000) >> 16,
			readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_Y) & 0xffff);
		pr_info("rd_h_offset_c(0x58):0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_CB) & 0xffff0000) >> 16,
			readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_CB) & 0xffff);
		pr_info("tg_offset  (0x60)  :0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_TGT_OFFSET) & 0xffff0000) >> 16,
			readl_relaxed(base + RW_IMG_RESZ_TGT_OFFSET) & 0xffff);
	} else {
		pr_info("rd_h_offset_y(0x54):0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_Y) & 0xfff000) >> 12,
			readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_Y) & 0xfff);
		pr_info("rd_h_offset_c(0x58):0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_CB) & 0xfff000) >> 12,
			readl_relaxed(base + RW_IMG_RESZ_SRC_OFFSET_CB) & 0xfff);
	    pr_info("tg_offset  (0x60)  :0x%8x 0x%8x\n",
			(readl_relaxed(base + RW_IMG_RESZ_TGT_OFFSET) & 0xfff000) >> 12,
			readl_relaxed(base + RW_IMG_RESZ_TGT_OFFSET) & 0xfff);
	}
	pr_info("H_factor_y (0x64)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_H8TAPS_FAC_Y) & 0xffffff);
	pr_info("H_factor_c (0x68)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_H8TAPS_FAC_CB) & 0xffffff);
	pr_info("V_factor_y (0x88)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_V4TAPS_SCL_Y));
	pr_info("V_factor_c (0x8c)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_V4TAPS_SCL_CB));
	pr_info("H_offset_y (0xc8)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_H8TAP_OFSET_Y));
	pr_info("H_offset_c (0xcc)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_H8TAP_OFSET_CB));
	pr_info("V_offset_y (0xd4)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_V4TAP_OFSET_Y));
	pr_info("V_offset_c (0xd8)  :0x%8x\n", readl_relaxed(base + RW_IMG_RESZ_V4TAP_OFSET_C));
}
