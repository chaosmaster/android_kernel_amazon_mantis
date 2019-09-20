/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
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

#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>


#include "di_table.h"

#include "ion_drv.h"
#include "di_util.h"
#include "di_reg.h"
#include "di_hal.h"
#include "nr_hal.h"
#include "vq_def.h"

#if (VQ_SUPPORT_ION)
/* #include <linux/ion.h> */
#endif

/*********************************************** define ***************************************************************/

#define VQ_ADDR_SWITCH(ion, mva)        ((-1 == (ion))?(mva):(ion))

#define TIMING_SIZE_720_480_H           720
#define TIMING_SIZE_720_480_V           480
#define TIMING_SIZE_720_576_H           720
#define TIMING_SIZE_720_576_V           576
#define TIMING_SIZE_1280_720_H          1280
#define TIMING_SIZE_1280_720_V          720
#define TIMING_SIZE_1920_1080_H         1920
#define TIMING_SIZE_1920_1080_V         1088

#define FHD_HIGH 1080

#define DI_OTHER_AREA_SIZE (360 * 240)
#define DI_NTSC_AREA_SIZE (TIMING_SIZE_720_480_H * TIMING_SIZE_720_480_V)
#define DI_PAL_AREA_SIZE (TIMING_SIZE_720_576_H * TIMING_SIZE_720_576_V)
#define DI_HD_AREA_SIZE (TIMING_SIZE_1280_720_H * TIMING_SIZE_1280_720_V)
#define DI_FHD_AREA_SIZE (TIMING_SIZE_1920_1080_H * FHD_HIGH)

#define IS_FULL_FHD_PICTURE(v)     ((v>=DI_FHD_AREA_SIZE) ? 1 : 0)
#define IS_HD_PICTURE(v)     ((v>=DI_HD_AREA_SIZE) ? 1 : 0)
#define IS_PAL_PICTURE(v)     ((v>=DI_PAL_AREA_SIZE) ? 1 : 0)
#define IS_NTSC_PICTURE(v)     ((v>=DI_NTSC_AREA_SIZE) ? 1 : 0)
#define IS_OTHER_PICTURE(v)     ((v>=DI_OTHER_AREA_SIZE) ? 1 : 0)

/* HVTOTAL */
/* 480P: 950X530 */
/* 576P: 950X650 */
/* 720P: 1650X750 */
/* 1080P:2200X1125 */
#define FMT_TOTAL_PIXEL_720_480_H       950
#define FMT_TOTAL_PIXEL_720_480_V       530
#define FMT_TOTAL_PIXEL_720_576_H       950
#define FMT_TOTAL_PIXEL_720_576_V       650
#define FMT_TOTAL_PIXEL_1280_720_H      1650
#define FMT_TOTAL_PIXEL_1280_720_V      750
#define FMT_TOTAL_PIXEL_1920_1080_H     2200
#define FMT_TOTAL_PIXEL_1920_1080_V     1125

/* HVSIZE */
#define FMT_SIZE_720_480_H              720
#define FMT_SIZE_720_480_V              480
#define FMT_SIZE_720_576_H              720
#define FMT_SIZE_720_576_V              576
#define FMT_SIZE_1280_720_H             1280
#define FMT_SIZE_1280_720_V             720
#define FMT_SIZE_1920_1080_H            1920
#define FMT_SIZE_1920_1080_V            1080

/* active */
/* 480P:  H 165 ~ 884     V  45 ~ (45 + 479) */
/* 576P:  H 165 ~ 884     V  45 ~ (45 + 575) */
/* 720P:  H 185 ~ 1646    V  26 ~ (26 + 719) */
/* 1080P: H 193 ~ 2112    V  32 ~ (32 + 1079) */
#define FMT_ACTIVE_720_480_H_START      165	/* resolved */
#define FMT_ACTIVE_720_480_H_END        (FMT_ACTIVE_720_480_H_START + (FMT_SIZE_720_480_H - 1))
#define FMT_ACTIVE_720_480_V_START      45	/* resolved */
#define FMT_ACTIVE_720_480_V_END        (FMT_ACTIVE_720_480_V_START + (FMT_SIZE_720_480_V - 1))
#define FMT_ACTIVE_720_576_H_START      165	/* resolved */
#define FMT_ACTIVE_720_576_H_END        (FMT_ACTIVE_720_576_H_START + (FMT_SIZE_720_576_H - 1))
#define FMT_ACTIVE_720_576_V_START      45	/* resolved */
#define FMT_ACTIVE_720_576_V_END        (FMT_ACTIVE_720_576_V_START + (FMT_SIZE_720_576_V - 1))
#define FMT_ACTIVE_1280_720_H_START     185
#define FMT_ACTIVE_1280_720_H_END       (FMT_ACTIVE_1280_720_H_START + (FMT_SIZE_1280_720_H - 1))
#define FMT_ACTIVE_1280_720_V_START     24
#define FMT_ACTIVE_1280_720_V_END       (FMT_ACTIVE_1280_720_V_START + (FMT_SIZE_1280_720_V - 1))
#define FMT_ACTIVE_1920_1080_H_START    193
#define FMT_ACTIVE_1920_1080_H_END      (FMT_ACTIVE_1920_1080_H_START + (FMT_SIZE_1920_1080_H - 1))
#define FMT_ACTIVE_1920_1080_V_START    32
#define FMT_ACTIVE_1920_1080_V_END      (FMT_ACTIVE_1920_1080_V_START + (FMT_SIZE_1920_1080_V - 1))

#define MA8F_BUFFER_ALIGN 2048
/* additional 128x6 bits */
#define MA8F_PER_BUF_SIZE (((768 * 576 / 2 / 8 / 2 + 6 * 128 / 8) + MA8F_BUFFER_ALIGN - 1) & ~(MA8F_BUFFER_ALIGN - 1))
#define MA8F_BUFFER_TOTAL_SIZE (MA8F_PER_BUF_SIZE * 2 * (4 + 1) + (MA8F_BUFFER_ALIGN - 1))

#define VQ_NR_LEVEL_MAX                         16
#define VQ_NR_LEVEL_RANGE_CNT                   3
#define NR_MESS_FILTER_TYPE_CNT                 3
#define NR_MESS_FILTER_THL_CNT                  64
#define NR_MESS_FILTER_TYPE_VALUE(level)        (NR_MESS_FILTER_TYPE_CNT * level / VQ_NR_LEVEL_MAX)
#define NR_MESS_FILTER_THL_VALUE(level)         ((NR_MESS_FILTER_THL_CNT / VQ_NR_LEVEL_MAX) * level)
#define NR_BLENDING_LEVEL_CNT                   15
#define NR_BLENDING_LEVEL_VALUE(level)          (NR_BLENDING_LEVEL_CNT * level / VQ_NR_LEVEL_MAX)

/************************************************* type ***************************************************************/

struct VQ_ADDR_SWITCH_PARAM_T {
	unsigned char y_en;
	unsigned int calc_addr;
	unsigned int y_addr;
	unsigned int y_size;
	unsigned int mva;
	unsigned int ion_fd;
	unsigned int width;
	unsigned int height;
	enum VQ_COLOR_FMT color_fmt;
};

struct FMT_TIMING_INFO_T {
	unsigned int size_h;
	unsigned int size_v;
	unsigned int start_h;
	unsigned int end_h;
	unsigned int start_v;
	unsigned int end_v;
	unsigned int total_pixel_h;
	unsigned int total_pixel_v;
};

struct DISPFMT_PATTERN_T {
	unsigned char enable;
	unsigned char en_422;
	unsigned short type;
	unsigned short width;
};

struct DI_VDO_PARAM_T {
	bool cur_top_enable;
	bool top_field_first_en;
	bool h265_enable;

	unsigned int width;
	unsigned int height;

	unsigned int align_width;
	unsigned int align_height;

	unsigned int area_size;

	unsigned int addr_y_prev;
	unsigned int addr_y_curr;
	unsigned int addr_y_next;

	unsigned int addr_cbcr_prev;
	unsigned int addr_cbcr_curr;
	unsigned int addr_cbcr_next;
	enum VDO_DI_MODE di_mode;
	enum VQ_COLOR_FMT src_fmt;
	enum VQ_TIMING_TYPE timing_type;
};


struct DISPFMT_PARAM_T {

	bool nr_enable;
	bool h265_enable;
	unsigned int width;
	unsigned int height;
	enum VQ_TIMING_TYPE timing_type;
	enum VQ_COLOR_FMT src_fmt;	/* 420,422 */
	struct DISPFMT_PATTERN_T pattern;
};



struct DI_WC_PARAM_T {
	unsigned int width;
	unsigned int height;
	unsigned int output_addr_y;
	unsigned int output_addr_cbcr;

	enum VQ_TIMING_TYPE timing_type;
	enum VQ_COLOR_FMT dst_fmt;	/* 420,422 */
};



enum VDO_YUV420_START_LINE_MODE {
	VDO_YUV420_START_LINE_MODE1,
	VDO_YUV420_START_LINE_MODE2,
	VDO_YUV420_START_LINE_MODE3,
	VDO_YUV420_START_LINE_MODE4,
	VDO_YUV420_START_LINE_MODE_NUM
};

enum VDO_YUV422_START_LINE_MODE {
	VDO_YUV422_START_LINE_MODE1,
	VDO_YUV422_START_LINE_MODE2,
	VDO_YUV422_START_LINE_MODE3,
	VDO_YUV422_START_LINE_MODE_NUM
};

enum VDO_VERTICAL_FILTER_MODE {
	VDO_VERTICAL_FILTER_LINEAR,
	VDO_VERTICAL_FILTER_4TAP,
	VDO_VERTICAL_FILTER_8TAP,
	VDO_VERTICAL_FILTER_NUM
};

struct VDO_HW_SRAM_UTILIZATION_T {
	unsigned int hd_mem_1920;
	unsigned int hd_mem;
	unsigned int hd_en;
};

struct VDO_START_LINE_T {
	unsigned int startline_y;
	unsigned int startline_c;
	unsigned int sub_startline_y;
	unsigned int sub_startline_c;
	unsigned int alter_startline_y;
	unsigned int alter_startline_c;
};

struct VQ_NR_PATTERN_T {
	unsigned char pattern_enable;
	unsigned int color_y;
	unsigned int color_c;
};

struct VQ_NR_CRC_T {
	unsigned char crc_anable;
	unsigned char crc_type;
};

struct VQ_NR_DEMO_T {
	unsigned char demo_enable;
	unsigned char demo_type;
};

struct VQ_NR_PARAM_T {
	unsigned int mnr_level;
	unsigned int bnr_level;

	struct VQ_NR_PATTERN_T pattern;
	struct VQ_NR_CRC_T crc;
	struct VQ_NR_DEMO_T demo;
};



/*********************************************** variables ************************************************************/




struct di_info di_init;


unsigned int di_dbg_level;


#if VQ_MVA_MAP_VA
unsigned int _vq_set_count = 1;
#endif

#if (VQ_SUPPORT_ION)
static struct ion_client *_vq_ion_client;
#endif


static unsigned short vdo_contrast_threshold = 0x20;	/* 0x438[15:8] */
static unsigned short vdo_moving_threshold = 0x10;	/* 0x438[23:16] */
static unsigned short vdo_pulldown_comb_threshold = 0x03;	/* 0x4c4[22:16] */
static struct VDO_HW_SRAM_UTILIZATION_T rVdoHwSramUtil[] = {
	{0, 0, 0},
	{0, 0, 1},
	{0, 1, 0},
	{1, 1, 0}
};

#if DI_VDO_SUPPORT_FUSION
/*vdo1 startline*/
static struct VDO_START_LINE_T vdo1_420_startline[] = {
	{0x07fb07fa, 0x07fd07fc, 0x80800000, 0x2060a0e0, 0x07fa07f9, 0x07fe07fd},
	{0x07fa07f9, 0x07fd07fc, 0x00000000, 0x20602060, 0x07fa07f9, 0x07fd07fc},
	{0x07fd07fc, 0x07ff07fe, 0x80800000, 0xa0e02060, 0x07fc07fb, 0x07fe07fd},
	{0x07fb07fa, 0x07ff07fe, 0x80800000, 0xa0e02060, 0x07fa07f9, 0x07fe07fd}
};

static struct VDO_START_LINE_T vdo1_422_startline[] = {
	{0x07fb07fa, 0x07fb07fa, 0x80800000, 0x80800000, 0x07fa07f9, 0x07fa07f9},
	{0x07fa07f9, 0x07fa07f9, 0x00000000, 0x00000000, 0x07fa07f9, 0x07fa07f9},
	{0x07fd07fc, 0x07fb07fa, 0x80800000, 0x00008080, 0x07fc07fb, 0x07fc07fb}
};

#else
/*vdo2 startline*/
static struct VDO_START_LINE_T vdo2_420_startline[] = {
	{0x07fb07fa, 0x07fd07fc, 0x80800000, 0x2060a0e0, 0x07fa07f9, 0x07fe07fd},
	{0x07fa07f9, 0x07fd07fc, 0x00000000, 0x20602060, 0x07fa07f9, 0x07fd07fc},
};

static struct VDO_START_LINE_T vdo2_422_startline[] = {
	{0x07fb07fa, 0x07fb07fa, 0x80800000, 0x80800000, 0x07fa07f9, 0x07fa07f9},
	{0x07fa07f9, 0x07fa07f9, 0x00000000, 0x00000000, 0x07fa07f9, 0x07fa07f9},
};
#endif

static struct FMT_TIMING_INFO_T _arFmtTimingInfo[VQ_TIMING_TYPE_MAX] = {
	{
	 FMT_SIZE_720_480_H, FMT_SIZE_720_480_V,
	 FMT_ACTIVE_720_480_H_START, FMT_ACTIVE_720_480_H_END,
	 FMT_ACTIVE_720_480_V_START, FMT_ACTIVE_720_480_V_END,
	 FMT_TOTAL_PIXEL_720_480_H, FMT_TOTAL_PIXEL_720_480_V},
	{
	 FMT_SIZE_720_576_H, FMT_SIZE_720_576_V,
	 FMT_ACTIVE_720_576_H_START, FMT_ACTIVE_720_576_H_END,
	 FMT_ACTIVE_720_576_V_START, FMT_ACTIVE_720_576_V_END,
	 FMT_TOTAL_PIXEL_720_576_H, FMT_TOTAL_PIXEL_720_576_V},
	{
	 FMT_SIZE_1280_720_H, FMT_SIZE_1280_720_V,
	 FMT_ACTIVE_1280_720_H_START, FMT_ACTIVE_1280_720_H_END,
	 FMT_ACTIVE_1280_720_V_START, FMT_ACTIVE_1280_720_V_END,
	 FMT_TOTAL_PIXEL_1280_720_H, FMT_TOTAL_PIXEL_1280_720_V},
	{
	 FMT_SIZE_1920_1080_H, FMT_SIZE_1920_1080_V,
	 FMT_ACTIVE_1920_1080_H_START, FMT_ACTIVE_1920_1080_H_END,
	 FMT_ACTIVE_1920_1080_V_START, FMT_ACTIVE_1920_1080_V_END,
	 FMT_TOTAL_PIXEL_1920_1080_H, FMT_TOTAL_PIXEL_1920_1080_V}
};




#if VQ_TIME_CHECK
unsigned int _au4VqTimeRec[VQ_TIME_CHECK_COUNT];
#endif







/********************************************** internal function *****************************************************/

static int vq_di_EventReg(unsigned char enable)
{
	if ((di_init.di_reg_irq_st == 0) && (enable != 0)) {
		di_init.di_reg_irq_st = 1;
		init_waitqueue_head(&di_init.di_event_irq_handle);

		DI_Printf(DI_LOG_FLOW, "[D] init EventWqIrqHandle");

	} else if ((di_init.di_reg_irq_st == 1) && (enable == 0)) {

		di_init.di_reg_irq_st = 0;
		DI_Printf(DI_LOG_FLOW, "[D] free EventWqIrqHandle");
	}

	return DI_RET_OK;
}


#if (VQ_SUPPORT_ION)
static int di_ion_init(void)
{
	if (!_vq_ion_client && g_ion_device) {

		_vq_ion_client = ion_client_create(g_ion_device, "vq");

		if (!_vq_ion_client) {

			DI_Printf(DI_LOG_ERROR, "[E] create ion client failed");
			return DI_RET_ERR_EXCEPTION;
		}

		DI_Printf(DI_LOG_FLOW, "[D] create ion client to 0x%x",
			  (unsigned int)_vq_ion_client);
	}

	return DI_RET_OK;
}

static int MTK_DI_IonToAddr(unsigned int ion_fd, unsigned int *pu4Addr, unsigned int *pu4Size)
{
	/*struct ion_handle *handle = NULL; */

	if (_vq_ion_client == NULL) {

		DI_Printf(DI_LOG_ERROR, "[E] ion client is null");
		return DI_RET_ERR_EXCEPTION;
	}

	if (ion_fd == VQ_INVALID_DW) {

		DI_Printf(DI_LOG_ERROR, "[E] ion fd 0x%x is invalid", ion_fd);
		return DI_RET_ERR_EXCEPTION;
	}

	ion_phys(_vq_ion_client, ion_fd, pu4Addr, pu4Size);

	DI_Printf(DI_LOG_FLOW, "[ADDR] ion fd[0x%x] handle[0x%x] address[0x%x] size[%d]",
		  ion_fd, ion_fd, pu4Addr, pu4Size);

	return DI_RET_OK;
}
#endif

static int di_dispfmt_config_dispsyscfg(struct DISPFMT_PARAM_T *prParam)
{

	/* DI_Printf(DI_LOG_FLOW, "disp top reg base=0x%lx\n", di_init.disp_top_reg_base); */

	/* enable m4u */
	/* DISPSYSCFG_W_awmmu(1); */
	/* DISPSYSCFG_W_armmu(1); */

	/* DISPSYSCFG_W_DISP_DI_PD_CTRL(0x0); */
	DISPSYSCFG_W_DISP_DI_PD_CTRL1(0x55aa8000);
	/* DISPSYSCFG_W_DISP_TOP_PCLK_CTRL(0x00000008); */


	/* DISPSYSCFG_W_DISP_TOP_RESET_EN(0xffffffff); */
	/* DISPSYSCFG_W_DISP_TOP_CLK_CTRL(0x0802C040); */
	DISPSYSCFG_W_DISP_TOP_DI_FULL_CLK_EN(1);
	DISPSYS_DISP_READ_ULTRA(1);
	DISPSYS_DISP_WRITE_ULTRA(0);
	DISPSYS_DISP_MAX_OUTSTANDING(0xFF);

	if (prParam->nr_enable)
		DISPSYSCFG_W_DISP_DISP_SRC_SEL(1);
	else
		DISPSYSCFG_W_DISP_DISP_SRC_SEL(0);
	return DI_RET_OK;
}


irqreturn_t _di_wrch_end_irq_handler(int irq, void *dev_id)
{
	DI_Printf(DI_LOG_IRQ, "[IRQ]before irq clr 0x%x,0x%x", DISPSYSCFG_R_INT_CLR_STATUS,
		  DISPSYSCFG_R_INT_STATUS);
	DISPSYSCFG_W_INT_CLR(0xD);
	DI_Printf(DI_LOG_IRQ, "[IRQ]set 1 to irq clr 0x%x,0x%x", DISPSYSCFG_R_INT_CLR_STATUS,
		  DISPSYSCFG_R_INT_STATUS);
	DISPSYSCFG_W_INT_CLR(0x0);
	DI_Printf(DI_LOG_IRQ, "[IRQ]set 0 to irq clr 0x%x,0x%x", DISPSYSCFG_R_INT_CLR_STATUS,
		  DISPSYSCFG_R_INT_STATUS);

#if VQ_TIME_CHECK
	VQ_TIME_REC(7);
#endif

	atomic_set(&di_init.di_event_Irq_flag, 1);

	wake_up_interruptible(&di_init.di_event_irq_handle);

	return IRQ_HANDLED;
}

irqreturn_t _di_vsync_irq_handle(int irq, void *dev_id)
{
	/* DISPSYSCFG_W_INT_CLR(0xD); */
	/* DISPSYSCFG_W_INT_CLR(0x0); */
	return IRQ_HANDLED;
}

irqreturn_t _di_disp_end_irq_handler(int irq, void *dev_id)
{
	/* DISPSYSCFG_W_INT_CLR(0xD); */
	/* DISPSYSCFG_W_INT_CLR(0x0); */

	pr_info("_di_disp_end_irq_handler");
	return IRQ_HANDLED;
}

irqreturn_t _di_under_run_irq_handle(int irq, void *dev_id)
{
	/* DISPSYSCFG_W_INT_CLR(0xD); */
	/* DISPSYSCFG_W_INT_CLR(0x0); */

	pr_info("_di_under_run_irq_handle");
	return IRQ_HANDLED;
}

#if 0
int di_hal_request_irq(struct device_node *np, struct di_info *di)
{

	di_init.wrch_frame_end_irq = irq_of_parse_and_map(np, 0);
	DI_Printf(DI_LOG_FLOW, " wrch_frame_end_irq value is %d", di_init.wrch_frame_end_irq);
	if (di_init.wrch_frame_end_irq == 0)
		DI_Printf(DI_LOG_FLOW, " get wrch_frame_end_irq from dts fail\n");

	di_init.di_vsync_irq = irq_of_parse_and_map(np, 1);
	DI_Printf(DI_LOG_FLOW, " di_vsync_irq value is %d", di_init.di_vsync_irq);
	if (di_init.di_vsync_irq == 0)
		DI_Printf(DI_LOG_FLOW, " get di_vsync_irq from dts fail\n");

	di_init.di_disp_end_irq = irq_of_parse_and_map(np, 2);
	DI_Printf(DI_LOG_FLOW, " di_disp_end_irq value is %d", di_init.di_disp_end_irq);
	if (di_init.di_disp_end_irq == 0)
		DI_Printf(DI_LOG_FLOW, " get di_disp_end_irq from dts fail\n");

	di_init.di_under_run_irq = irq_of_parse_and_map(np, 3);
	DI_Printf(DI_LOG_FLOW, " di_vsync_irq value is %d", di_init.di_under_run_irq);
	if (di_init.di_under_run_irq == 0)
		DI_Printf(DI_LOG_FLOW, " get di_under_run_irq from dts fail\n");

	if (request_irq
	    (di->wrch_frame_end_irq, (irq_handler_t) _di_wrch_end_irq_handler, IRQF_TRIGGER_NONE,
	     "di", (void *)di)
	    < 0) {
		DI_Printf(DI_LOG_FLOW, " request irq error\n");
	}

	return DI_RET_OK;
}
#else

int di_hal_request_irq(struct platform_device *pdev, struct di_info *di)
{
	struct device *dev = &pdev->dev;

	di->wrch_frame_end_irq = platform_get_irq(pdev, MTK_VQ_IRQ_WC);
	if (di->wrch_frame_end_irq < 0) {
		dev_notice(dev, "[DI] get wrch_frame_end_irq irq err\n");
		return di->wrch_frame_end_irq;
	}
	DI_Printf(DI_LOG_FLOW, " wrch_frame_end_irq value is %d", di->wrch_frame_end_irq);

	di->di_vsync_irq = platform_get_irq(pdev, MTK_VQ_IRQ_DI_VSYNC);
	if (di->di_vsync_irq < 0) {
		dev_notice(dev, "[DI] get di_vsync_irq irq err\n");
		return di->di_vsync_irq;
	}
	DI_Printf(DI_LOG_FLOW, " di_vsync_irq value is %d", di->di_vsync_irq);


	di->di_disp_end_irq = platform_get_irq(pdev, MTK_VQ_IRQ_DISP_END);
	if (di->di_disp_end_irq < 0) {
		dev_notice(dev, "[DI] get di_disp_end_irq irq err\n");
		return di->di_disp_end_irq;
	}
	DI_Printf(DI_LOG_FLOW, " di_disp_end_irq value is %d", di->di_disp_end_irq);


	di->di_under_run_irq = platform_get_irq(pdev, MTK_VQ_IRQ_DI_UNDERRUN);
	if (di->di_under_run_irq < 0) {
		dev_notice(dev, "[DI] get di_under_run_irq irq err\n");
		return di->di_under_run_irq;
	}
	DI_Printf(DI_LOG_FLOW, " di_under_run_irq value is %d", di->di_under_run_irq);

	if (request_irq
	    (di->wrch_frame_end_irq, (irq_handler_t) _di_wrch_end_irq_handler, IRQF_TRIGGER_NONE,
	     "di", (void *)di)
	    < 0) {
		DI_Printf(DI_LOG_FLOW, " request irq error\n");
	}

	return DI_RET_OK;
}
#endif
static const char *const _ap_mtk_di_clk_name[MTK_DI_CLK_CNT] = {
	"vdo_di", "dispfmt_di", "wr_channel", "dramc_larb8", "di_sel", "tvdpll_d2", "osdpll_d3"
	    /* "vdo_di","dispfmt_di","wr_channel","di_sel","tvdpll_d2","osdpll_d3" */
};

#if 0
static int _di_parse_dev_node(struct platform_device *pdev)
{
	struct device_node *np;
	unsigned int reg_value;
	unsigned int irq_value;
	int i;
	struct device *dev = &pdev->dev;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-vq");
	if (np == NULL) {
		DI_Printf(DI_LOG_ERROR, "dts error, no vq device node.\n");
		return DI_RET_ERR_PARAM;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);
	of_property_read_u32_index(np, "interrupts", 1, &irq_value);

	di_init.dispfmt_reg_base = (uintptr_t) of_iomap(np, 0);	/*dispfmt, 0x1500e000 */
	if (IS_ERR((void *)di_init.dispfmt_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map va addr error 0x%lx\n", di_init.dispfmt_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " dispfmt reg base=0x%lx\n", di_init.dispfmt_reg_base);

	di_init.vdo_reg_base = di_init.dispfmt_reg_base;

	DI_Printf(DI_LOG_FLOW, " vdo reg base=0x%lx\n", di_init.vdo_reg_base);

	di_init.wch_reg_base = (uintptr_t) of_iomap(np, 1);	/*wch, 0x1500f000 */
	if (IS_ERR((void *)di_init.wch_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " wch map va addr error 0x%lx\n", di_init.wch_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " wch reg base=0x%lx\n", di_init.wch_reg_base);


	di_init.disp_top_reg_base = (uintptr_t) of_iomap(np, 2);	/*disp top, 15000000 */
	if (IS_ERR((void *)di_init.disp_top_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top reg base=0x%lx", di_init.disp_top_reg_base);



	di_init.disp_top_di_reg_base = (uintptr_t) of_iomap(np, 3);	/*disp top, 0x14000000 */
	if (IS_ERR((void *)di_init.disp_top_di_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_di_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top di reg base=0x%lx\n", di_init.disp_top_di_reg_base);


	di_init.disp_top_pclk_reg_base = (uintptr_t) of_iomap(np, 4);	/*disp top, 0x14001500 */
	if (IS_ERR((void *)di_init.disp_top_pclk_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_pclk_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top pclk reg base=0x%lx", di_init.disp_top_pclk_reg_base);

	for (i = 0; i < MTK_DI_CLK_CNT; i++) {
		di_init.clks[i] = devm_clk_get(dev, _ap_mtk_di_clk_name[i]);
		if (IS_ERR(di_init.clks[i]))
			dev_notice(dev, "fail to get clk[%d] %s\n", i, _ap_mtk_di_clk_name[i]);
		DI_Printf(DI_LOG_FLOW, " get clk[%d] %s 0x%lx\n", i, _ap_mtk_di_clk_name[i],
			  (unsigned long)di_init.clks[i]);
	}
	di_hal_request_irq(np, &di_init);

	atomic_set(&di_init.di_event_Irq_flag, 0);
	di_init.pm_dev = dev;

	return DI_RET_OK;

}
#endif


static int _di_parse_reg_dev_node(struct platform_device *pdev)
{
	struct device_node *np;
	unsigned int reg_value;
	struct device *dev = &pdev->dev;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-vq");
	if (np == NULL) {
		DI_Printf(DI_LOG_ERROR, "dts error, no vq device node.\n");
		return DI_RET_ERR_PARAM;
	}
	of_property_read_u32_index(np, "reg", 1, &reg_value);
	di_init.disp_top_reg_base = (uintptr_t) of_iomap(np, MTK_VQ_REG_DISP_TOP);	/*disp top, 15000000 */
	if (IS_ERR((void *)di_init.disp_top_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top reg base=0x%lx", di_init.disp_top_reg_base);

	di_init.dispfmt_reg_base = (uintptr_t) of_iomap(np, MTK_VQ_REG_DISPFMT_VDO);	/*dispfmt, 0x1500e000 */
	if (IS_ERR((void *)di_init.dispfmt_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map va addr error 0x%lx\n", di_init.dispfmt_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " dispfmt reg base=0x%lx\n", di_init.dispfmt_reg_base);

	di_init.vdo_reg_base = di_init.dispfmt_reg_base;

	DI_Printf(DI_LOG_FLOW, " vdo reg base=0x%lx\n", di_init.vdo_reg_base);

	di_init.wch_reg_base = (uintptr_t) of_iomap(np, MTK_VQ_REG_WC);	/*wch, 0x1500f000 */
	if (IS_ERR((void *)di_init.wch_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " wch map va addr error 0x%lx\n", di_init.wch_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " wch reg base=0x%lx\n", di_init.wch_reg_base);

	di_init.disp_top_di_reg_base = (uintptr_t) of_iomap(np, MTK_VQ_REG_TOP_DI);	/*disp top, 0x14000000 */
	if (IS_ERR((void *)di_init.disp_top_di_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_di_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top di reg base=0x%lx\n", di_init.disp_top_di_reg_base);

	di_init.disp_top_pclk_reg_base = (uintptr_t) of_iomap(np, MTK_VQ_REG_TOP_PCLK);	/*disp top, 0x14001500 */
	if (IS_ERR((void *)di_init.disp_top_pclk_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_pclk_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top pclk reg base=0x%lx", di_init.disp_top_pclk_reg_base);

	di_init.pm_dev = dev;

	return DI_RET_OK;

}


int di_hal_hw_init(struct platform_device *pdev, struct di_info *di)
{
	int ret = 0;
	int i;
	struct device *dev = &pdev->dev;
	/* struct resource *res; */
	DI_Printf(DI_LOG_FLOW, "hw init start dev=%p\n", pdev);
#if 1
	for (i = 0; i < MTK_DI_CLK_CNT; i++) {
		di->clks[i] = devm_clk_get(dev, _ap_mtk_di_clk_name[i]);
		if (IS_ERR(di->clks[i])) {
			dev_notice(dev, "fail to get clk[%d] %s", i, _ap_mtk_di_clk_name[i]);
			return -EPROBE_DEFER;
		}
		DI_Printf(DI_LOG_FLOW, "get clk[%d] %s 0x%lx", i, _ap_mtk_di_clk_name[i],
			  (unsigned long)di->clks[i]);
	}
	di->pm_dev = dev;
#if 0
	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_DISP_TOP);
	if (!res) {
		dev_notice(dev, "[DI]failed to get MEM resource MTK_VQ_REG_DISP_TOP\n");
		return -ENXIO;
	}
	di->disp_top_reg_base = (uintptr_t) devm_ioremap_resource(dev, res);
	di_init.disp_top_reg_base = di->disp_top_reg_base;
	if (IS_ERR((void *)di->disp_top_reg_base)) {
		dev_notice(dev, "[DI]get disp_top_reg_base err\n");
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, "disp top reg base=0x%lx", di_init.disp_top_reg_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_DISPFMT_VDO);
	if (!res) {
		dev_notice(dev, "[DI]failed to get MEM resource MTK_VQ_REG_DISPFMT_VDO\n");
		return -ENXIO;
	}
	di->dispfmt_reg_base = (uintptr_t) devm_ioremap_resource(dev, res);
	di_init.dispfmt_reg_base = di->dispfmt_reg_base;

	if (IS_ERR((void *)di->dispfmt_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map va addr error 0x%lx\n", di->dispfmt_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " dispfmt reg base=0x%lx", di_init.dispfmt_reg_base);
	di->vdo_reg_base = di->dispfmt_reg_base;

	di_init.vdo_reg_base = di->vdo_reg_base;
	DI_Printf(DI_LOG_FLOW, " vdo reg base=0x%lx", di_init.vdo_reg_base);


	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_WC);
	if (!res) {
		dev_notice(dev, "[DI]failed to get MEM resource MTK_VQ_REG_DISPFMT_VDO\n");
		return -ENXIO;
	}
	di->wch_reg_base = (uintptr_t) devm_ioremap_resource(dev, res);

	di_init.wch_reg_base = di->wch_reg_base;
	if (IS_ERR((void *)di->wch_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " wch map va addr error 0x%lx\n", di->wch_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " wch reg base=0x%lx", di_init.wch_reg_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_TOP_DI);
	if (!res) {
		dev_notice(dev, "[DI]failed to get MEM resource MTK_VQ_REG_TOP_DI\n");
		return -ENXIO;
	}
	di->disp_top_di_reg_base = (uintptr_t) devm_ioremap_resource(dev, res);
	if (IS_ERR((void *)di->disp_top_di_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di->disp_top_di_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top di reg base=0x%lx\n", di->disp_top_di_reg_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_TOP_PCLK);
	if (!res) {
		dev_notice(dev, "[DI]failed to get MEM resource MTK_VQ_REG_TOP_PCLK\n");
		return -ENXIO;
	}
	di->disp_top_pclk_reg_base = (uintptr_t) devm_ioremap_resource(dev, res);
	if (IS_ERR((void *)di->disp_top_pclk_reg_base)) {
		DI_Printf(DI_LOG_ERROR, " map disp top va addr error 0x%lx\n",
			  di_init.disp_top_pclk_reg_base);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, " disp top pclk reg base=0x%lx", di->disp_top_pclk_reg_base);
#else
	_di_parse_reg_dev_node(pdev);
#endif
	ret = di_hal_request_irq(pdev, di);
	if (ret) {
		dev_notice(dev, "[DI]failed to install irq (%d)\n", ret);
		return -ENXIO;
	}
	di_init.count = 0;
	atomic_set(&di_init.di_event_Irq_flag, 0);
#else
	ret = _di_parse_dev_node(pdev);
#endif
	return DI_RET_OK;
}




static int di_dispfmt_config_nr(struct DISPFMT_PARAM_T *prParam)
{

	DI_Printf(DI_LOG_FLOW, "di_dispfmt_config_nr enable=%d", prParam->nr_enable);
	if (prParam->nr_enable) {

		/* 0x010 */
		DISPFMT_W_BYPASS_NR(0);

		/* 0x080 */
		DISPFMT_W_ADJ_SYNC_EN(1);
		DISPFMT_W_NR_ADJ_FORWARD(0);
		DISPFMT_W_TOGGLE_OPTION(0);
		DISPFMT_W_HSYNC_DELAY(0x38);
		DISPFMT_W_VSYNC_DELAY(0x5);

		/* 0x084 */
#if VQ_WH_TIMING
		DISPFMT_W_VSYNC_END(_arFmtTimingInfo[prParam->timing_type].start_v +
				    prParam->height - 1);
#else
		DISPFMT_W_VSYNC_END(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
		DISPFMT_W_VSYNC_START(_arFmtTimingInfo[prParam->timing_type].start_v - 1);
		DISPFMT_W_VSYNC_POLAR(1);
		DISPFMT_W_HSYNC_POLAR(0);
		DISPFMT_W_FIELD_POLAR(0);
		DISPFMT_W_DE_SELF(0);
		DISPFMT_W_ODD_V_START_OPT(0);
		DISPFMT_W_ODD_V_END_OPT(0);
		DISPFMT_W_EVEN_V_START_OPT(1);
		DISPFMT_W_EVEN_V_END_OPT(1);

		/* 0x088 */
#if VQ_WH_TIMING
		DISPFMT_W_HOR_END(_arFmtTimingInfo[prParam->timing_type].start_h +
				  prParam->width - 1 + 7);
#else
		DISPFMT_W_HOR_END(_arFmtTimingInfo[prParam->timing_type].end_h + 7);
#endif
		DISPFMT_W_HOR_START(_arFmtTimingInfo[prParam->timing_type].start_h + 7);

		/* 0x08C */
#if VQ_WH_TIMING
		DISPFMT_W_VO_END(_arFmtTimingInfo[prParam->timing_type].start_v +
				 prParam->height - 1);
#else
		DISPFMT_W_VO_END(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
		DISPFMT_W_VO_START(_arFmtTimingInfo[prParam->timing_type].start_v);

		/* 0x090 */
#if VQ_WH_TIMING
		DISPFMT_W_VE_END(_arFmtTimingInfo[prParam->timing_type].start_v +
				 prParam->height - 1);
#else
		DISPFMT_W_VE_END(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
		DISPFMT_W_VE_START(_arFmtTimingInfo[prParam->timing_type].start_v);

	} else {

		/* 0x010 */
		DISPFMT_W_BYPASS_NR(1);

		DI_Printf(DI_LOG_FLOW, "di_dispfmt_config_nr nr 0x010 disable");

		/* 0x080 */
		DISPFMT_W_ADJ_SYNC_EN(0);
		DISPFMT_W_NR_ADJ_FORWARD(0);
		DISPFMT_W_TOGGLE_OPTION(0);
		DISPFMT_W_HSYNC_DELAY(0);
		DISPFMT_W_VSYNC_DELAY(0);

		/* 0x084 */
		DISPFMT_W_VSYNC_END(0);
		DISPFMT_W_VSYNC_START(0);
		DISPFMT_W_VSYNC_POLAR(0);
		DISPFMT_W_HSYNC_POLAR(0);
		DISPFMT_W_FIELD_POLAR(0);
		DISPFMT_W_DE_SELF(0);
		DISPFMT_W_ODD_V_START_OPT(0);
		DISPFMT_W_ODD_V_END_OPT(0);
		DISPFMT_W_EVEN_V_START_OPT(0);
		DISPFMT_W_EVEN_V_END_OPT(0);

		/* 0x088 */
		DISPFMT_W_HOR_END(0);
		DISPFMT_W_HOR_START(0);

		/* 0x08C */
		DISPFMT_W_VO_END(0);
		DISPFMT_W_VO_START(0);

		/* 0x090 */
		DISPFMT_W_VE_END(0);
		DISPFMT_W_VE_START(0);
	}
	return DI_RET_OK;
}

static int di_dispfmt_config_timingtype(struct DISPFMT_PARAM_T *prParam)
{
	/* 0x000 */
	DISPFMT_W_HOR_VALID_STAR(_arFmtTimingInfo[prParam->timing_type].start_h + 7);
#if VQ_WH_TIMING
	DISPFMT_W_HOR_VALID_END(_arFmtTimingInfo[prParam->timing_type].start_h + prParam->width -
				1 + 7);
#else
	DISPFMT_W_HOR_VALID_END(_arFmtTimingInfo[prParam->timing_type].end_h + 7);
#endif

	/* 0x004 */
#if VQ_WH_TIMING
	DISPFMT_W_NR_IN_VSYNC_END(_arFmtTimingInfo[prParam->timing_type].start_v +
				  prParam->height - 1);
#else
	DISPFMT_W_NR_IN_VSYNC_END(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
	DISPFMT_W_NR_IN_VSYNC_START(_arFmtTimingInfo[prParam->timing_type].start_v - 1);
	DISPFMT_W_NR_IN_VSYNC_POLAR(1);
	DISPFMT_W_NR_IN_HSYNC_POLAR(1);
	DISPFMT_W_NR_IN_FIELD_POLAR(0);
	DISPFMT_W_NR_IN_DE_SELF(0);
	DISPFMT_W_NR_IN_ODD_V_START_OPT(0);
	DISPFMT_W_NR_IN_ODD_V_END_OPT(0);
	DISPFMT_W_NR_IN_EVEN_V_START_OPT(1);
	DISPFMT_W_NR_IN_EVEN_V_END_OPT(1);

	/* 0x008 */
	DISPFMT_W_HOR_NR_VALID_STAR(_arFmtTimingInfo[prParam->timing_type].start_h + 7);
#if VQ_WH_TIMING
	DISPFMT_W_HOR_NR_VALID_END(_arFmtTimingInfo[prParam->timing_type].start_h +
				   prParam->width - 1 + 7);
#else
	DISPFMT_W_HOR_NR_VALID_END(_arFmtTimingInfo[prParam->timing_type].end_h + 7);
#endif

	/* 0x00C */
	DISPFMT_W_NR_IN_HOR_STAT(_arFmtTimingInfo[prParam->timing_type].start_h + 7);
#if VQ_WH_TIMING
	DISPFMT_W_NR_IN_HOR_END(_arFmtTimingInfo[prParam->timing_type].start_h + prParam->width -
				1 + 7);
#else
	DISPFMT_W_NR_IN_HOR_END(_arFmtTimingInfo[prParam->timing_type].end_h + 7);
#endif

	/* 0x014 */

	DI_Printf(DI_LOG_FLOW, "prParam->pattern.enable:%d, type:%d,type:%d",
		  prParam->pattern.enable, prParam->pattern.type, prParam->pattern.width);

	DISPFMT_W_COLOR_BAR_ON(prParam->pattern.enable);
	if (prParam->pattern.enable) {

		DISPFMT_W_COLOR_BAR_TYPE(prParam->pattern.type);
		DISPFMT_W_COLOR_BAR_WIDHT(prParam->pattern.width);
		DISPFMT_W_ENABLE_422_444(1);

	} else {

		DISPFMT_W_ENABLE_422_444(0);
	}

	/* 0x094 */
	DISPFMT_W_HSYNWIDTH(0x20);
	DISPFMT_W_VSYNWIDTH(0xd);
	DISPFMT_W_HD_TP((prParam->timing_type == VQ_TIMING_TYPE_720P) ? 1 : 0);
	DISPFMT_W_HD_ON((prParam->timing_type >= VQ_TIMING_TYPE_720P) ? 1 : 0);
	DISPFMT_W_PRGS(1);
	DISPFMT_W_PRGS_AUTOFLD(0);
	DISPFMT_W_PRGS_INVFLD(0);
	DISPFMT_W_YUV_RST_OPT(0);
	DISPFMT_W_PRGS_FLD(0);
	DISPFMT_W_NEW_SD_144MHz(0);
	DISPFMT_W_NEW_SD_MODE(0);
	DISPFMT_W_NEW_SD_USE_EVEN(0);
	DISPFMT_W_TVMODE(0);

	/* 0x09C */
#if VQ_WH_TIMING
	DISPFMT_W_PXLLEN(prParam->width);
#else
	DISPFMT_W_PXLLEN(_arFmtTimingInfo[prParam->timing_type].size_h);
#endif

	DISPFMT_W_RIGHT_SKIP(0);

	/* 0x0A0 */
#if VQ_WH_TIMING
	DISPFMT_W_HACTEND(_arFmtTimingInfo[prParam->timing_type].start_h + prParam->width - 1);
#else
	DISPFMT_W_HACTEND(_arFmtTimingInfo[prParam->timing_type].end_h);
#endif
	DISPFMT_W_HACTBGN(_arFmtTimingInfo[prParam->timing_type].start_h);

	/* 0x0A4 */
#if VQ_WH_TIMING
	DISPFMT_W_VOACTEND(_arFmtTimingInfo[prParam->timing_type].start_v + prParam->height - 1);
#else
	DISPFMT_W_VOACTEND(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
	DISPFMT_W_VOACTBGN(_arFmtTimingInfo[prParam->timing_type].start_v);
	DISPFMT_W_HIDE_OST(0);

	/* 0x0A8 */
#if VQ_WH_TIMING
	DISPFMT_W_VEACTEND(_arFmtTimingInfo[prParam->timing_type].start_v + prParam->height - 1);
#else
	DISPFMT_W_VEACTEND(_arFmtTimingInfo[prParam->timing_type].end_v);
#endif
	DISPFMT_W_VEACTBGN(_arFmtTimingInfo[prParam->timing_type].start_v);
	DISPFMT_W_HIDE_EST(0);

	/* 0x0AC */
	DISPFMT_W_VDO1_EN(1);
	DISPFMT_W_FMTM(1);
	DISPFMT_W_HPOR(0);
	DISPFMT_W_VPOR(0);
	DISPFMT_W_C_RST_SEL(0);
	DISPFMT_W_PXLSEL(0);
	DISPFMT_W_FTRST(0);
	DISPFMT_W_FTRST(1);
	DISPFMT_W_SHVSYN(0);
	DISPFMT_W_SYN_DEL(0);
	DISPFMT_W_UVSW(0);
	DISPFMT_W_BLACK(0);
	DISPFMT_W_PFOFF(1);
	DISPFMT_W_HW_OPT(0);

	/* 0x0B0 */
	DISPFMT_W_Horizontal_Scaling(0x01000001);

	/* 0x0B4 */
	DISPFMT_W_BIY(8);
	DISPFMT_W_BICB(8);
	DISPFMT_W_BICR(8);
	DISPFMT_W_PF2OFF(0);
	DISPFMT_W_HIDE_L(0);

	/* 0x0B8 */
	DISPFMT_W_BGY(1);
	DISPFMT_W_BGCB(2);
	DISPFMT_W_BGCR(3);

	/* vFmt_WriteReg(0x1c0010bc, 0x00000000); */

	/* 0x0C8 */
	DISPFMT_W_NEW_SCL_MODE_CTRL(0x00000000);

	/* 0x0CC */
	DISPFMT_W_Dispfmt_Configure(0x00100000);

	/* 0x0D0 */
	DISPFMT_W_0D0_11_0(_arFmtTimingInfo[prParam->timing_type].total_pixel_v);
	DISPFMT_W_0D0_28_16(_arFmtTimingInfo[prParam->timing_type].total_pixel_h);
	DISPFMT_W_0D0_31_31(1);

	/* 0x0D4 */
	DISPFMT_W_V_TOTAL(_arFmtTimingInfo[prParam->timing_type].total_pixel_v);
	DISPFMT_W_H_TOTAL(_arFmtTimingInfo[prParam->timing_type].total_pixel_h);
	DISPFMT_W_ADJ_T(1);

	DISPFMT_W_Multi_Ratio(0x00000000);

	/* vFmt_WriteReg(0x1c0010f4, 0x00000000); */

	DISPFMT_W_C110(0);
	DISPFMT_W_OLD_CHROMA(0);
	DISPFMT_W_EN_235_255(0);
	DISPFMT_W_FROM_235_TO_255(0);

	DI_Printf(DI_LOG_FLOW, "dispfmt h265, h265_enable[0x%x]", prParam->h265_enable);

	/* vFmt_WriteReg(0x1c0013b0, 0x00000001);  //check */
	return DI_RET_OK;
}



int di_dispfmt_setparam(struct DISPFMT_PARAM_T *prParam)
{
	di_dispfmt_config_dispsyscfg(prParam);
	di_dispfmt_config_nr(prParam);
	di_dispfmt_config_timingtype(prParam);

	return DI_RET_OK;
}

static int di_dispfmt_trigger(struct DISPFMT_PARAM_T *prParam)
{
	unsigned long start_timer, end_timer;

	DI_Printf(DI_LOG_FLOW, "[F] will triggle, irq_reg[0x%x]", DISPSYSCFG_R_INT_STATUS);
#if VQ_TIME_CHECK
	VQ_TIME_REC(6);
#endif
	start_timer = sched_clock();

	/* 0x018 */
	DISPFMT_W_DI_AGENT_TRIG(1);
	DISPFMT_W_DI_AGENT_TRIG(0);

#if VQ_WAIT_IRQ

	while (1) {

		if (wait_event_interruptible_timeout
		    (di_init.di_event_irq_handle, atomic_read(&di_init.di_event_Irq_flag),
		     HZ / 10) == 0) {

			di_init.di_event_irq_timeout_count++;

			DI_Printf(DI_LOG_ERROR,
				  "[IRQ] Irq timeout, St[%d], Flag[%d], count[%d], irq_reg[0x%x]",
				  di_init.di_reg_irq_st,
				  di_init.di_event_Irq_flag.counter,
				  di_init.di_event_irq_timeout_count, DISPSYSCFG_R_INT_STATUS);

		} else {

#if VQ_TIME_CHECK
			VQ_TIME_REC(8);
#endif

			end_timer = sched_clock();
			DI_Printf(DI_LOG_IRQ, "hw time is %ld wait IRQ",
				  (end_timer - start_timer) / 1000000);

			DI_Printf(DI_LOG_IRQ,
				  "[IRQ] Irq OK, St[%d], Flag[%d], count[%d], irq_reg[0x%x]",
				  di_init.di_reg_irq_st,
				  di_init.di_event_Irq_flag.counter,
				  di_init.di_event_irq_timeout_count, DISPSYSCFG_R_INT_STATUS);

			di_init.di_event_irq_timeout_count = 0;

			break;
		}
	}

	atomic_set(&di_init.di_event_Irq_flag, 0);

#endif
	DISPFMT_W_DI_AGENT_TRIG(1);

	return DI_RET_OK;
}

static int di_vdo_config_base(struct DI_VDO_PARAM_T *prParam)
{
	/* DI_Printf(DI_LOG_FLOW, "%s DI_VDO_BASE=0x%lx", __func__, DI_VDO_BASE); */

	unsigned int align_width = prParam->align_width;
	unsigned int height = prParam->height;

	if (prParam->di_mode != VDO_FRAME_MODE) {

		if (prParam->timing_type > VQ_TIMING_TYPE_720P) {	/* 1080p no 8 tap */

			DI_VDO_W_VDO_SCL_CTRL(0x00212f00);	/* 0x14c[25:24] y c 8 tap,full hd no 8tap */

		} else {

			DI_VDO_W_VDO_SCL_CTRL(0x03212f00);
		}

	} else {

		DI_VDO_W_VDO_SCL_CTRL(0x00212f00);
	}

	DI_VDO_W_VDO_8TAP_VALID(0x01e00002);	/* 0x194 */
	DI_VDO_W_VDO_8TAP_CTRL_04(0x00000000);	/* 0x1a0 */
	DI_VDO_W_VDO_SHARP_CTRL_01(0x40ffff80);	/* 0x1b0 */
	DI_VDO_W_VDO_SHARP_CTRL_02(0x00822020);	/* 0x1b4 */
	/* DI_VDO_W_VDO_SHARP_CTRL_03(0x01108080);    //0x1b8 */

	DI_VDO_W_VDOY1(0x26200000);	/* 0x400      //Y - luma */
	DI_VDO_W_VDOC1(0x26300000);	/* 0x404      //Y - chroma */
	DI_VDO_W_VDOY2(0x26200000);	/* 0x408      //X - luma */
	DI_VDO_W_VDOC2(0x26300000);	/* 0x40c      //X - chroma */

	/* 0x410 */
	DI_VDO_W_swap_off(0);	/* check */
	DI_VDO_W_HBLOCK_9_8(0);	/* check */
	DI_VDO_W_PICHEIGHT(height);	/* check */
	DI_VDO_W_DW_NEED(align_width / 4);	/* check */
	DI_VDO_W_HBLOCK_7_0(align_width / 8);	/* check */

	DI_VDO_W_VSCALE(0x80000800);	/* 0x414 */
	DI_VDO_W_STAMBR(0x1e1e0000);	/* 0x418 */
	DI_VDO_W_VMODE(0x00600003);	/* 0x41c */
	DI_VDO_W_YSL(0x0ffd0ffc);	/* 0x420 */
	DI_VDO_W_CSL(0x0fff0ffe);	/* 0x424 */
	DI_VDO_W_YSSL(0x80800000);	/* 0x428 */
	DI_VDO_W_CSSL(0xa0e02060);	/* 0x42c */
	DI_VDO_W_VDOCTRL(0x00000204);	/* 0x430 */
	DI_VDO_W_VSLACK(0x14c81414);	/* 0x434 */
	DI_VDO_W_MP(0x06000e07);	/* 0x438 */
	DI_VDO_W_VDORST(0x000000ff);	/* 0x43c */
	DI_VDO_W_VDORST(0x00000000);	/* 0x43c */
	DI_VDO_W_COMB_8F(0x2002ff00);	/* 0x440 */
	DI_VDO_W_YSL2(0x0ffc0ffb);	/* 0x450 */
	DI_VDO_W_CSL2(0x0ffe0ffd);	/* 0x454 */
	DI_VDO_W_MBAVG1(0x00800000);	/* 0x458 */
	DI_VDO_W_MBAVG2(0x00840000);	/* 0x45c */
	DI_VDO_W_CMB_CNT(0x00000001);	/* 0x460 */
	DI_VDO_W_CMB_CNT(0x00000000);	/* 0x460 */
	DI_VDO_W_PS_MODE(0x00008000);	/* 0x464 */
	DI_VDO_W_STA_POS(0x05110000);	/* 0x46c */
	DI_VDO_W_PS_FLT(0x10001000);	/* 0x470 */

	if (prParam->di_mode != VDO_FRAME_MODE) {

		if (prParam->timing_type > VQ_TIMING_TYPE_720P)
			DI_VDO_W_F_CTRL(0x00000000);	/* 0x478[7]  4tap for full hd 1080 */
		else
			DI_VDO_W_F_CTRL(0x00000000);	/* 0x478 */


	} else {

		/*for keep brace */
		DI_VDO_W_F_CTRL(0x00000000);	/* 0x478 */

		DI_VDO_W_VMODE(0x00000003);	/* 0x41c */
	}

	DI_VDO_W_VIDEO_OPTION(0x04000000);	/* 0x47c //check */
	DI_VDO_W_PTR_WF_Y(0x0006ba80);	/* 0x480      //W - luma */
	DI_VDO_W_PTR_ZF_Y(0x00091a00);	/* 0x484      //Z - luma */
	DI_VDO_W_FDIFF_TH3(0x4a0186a0);	/* 0x488 */
	DI_VDO_W_FDIFF_TH2(0x1b011940);	/* 0x48c */
	DI_VDO_W_FDIFF_TH1(0x0000c350);	/* 0x490 */
	DI_VDO_W_TH_XZ_MIN(0x00012003);	/* 0x494 */
	DI_VDO_W_TH_XZ_NM(0x3202A02A);	/* 0x498 */
	DI_VDO_W_TH_YW_MIN(0x0600c002);	/* 0x49c */

#if 1
	if (prParam->area_size <= DI_PAL_AREA_SIZE)
		DI_VDO_W_TH_YW_NM(0x6401C004);	/* 0x4a0 */
	else
		DI_VDO_W_TH_YW_NM(0x6401C01C);	/* 0x4a0 */
#endif
	DI_VDO_W_FCH_XZ(0x32032004);	/* 0x4a4 */
	DI_VDO_W_FCH_YW(0x01036004);	/* 0x4a8 */
	DI_VDO_W_CRM_SAW(0x02000020);	/* 0x4ac */
	DI_VDO_W_EDGE_CTL(0x0332323c);	/* 0x4b0 */
	DI_VDO_W_MD_ADV(0x02332093);	/* 0x4c0 */
	DI_VDO_W_PD_FLD_LIKE(0x01030510);	/* 0x4c4 */
	DI_VDO_W_PD_REGION(0x00007601);	/* 0x4cc */
	DI_VDO_W_CHROMA_MD(0x10001433);	/* 0x4d0 */
	DI_VDO_W_MBAVG3(0x00800000);	/* 0x4d8 */

	DI_VDO_W_HD_MODE(0xa20000b4);	/* 0x4e0 */

#if 1				/* VQ_WH_TIMING */
	DI_VDO_W_HD_MEM(((prParam->timing_type > VQ_TIMING_TYPE_576P) ? 1 : 0));
	DI_VDO_W_HD_MEM_1920(((prParam->timing_type > VQ_TIMING_TYPE_720P) ? 1 : 0));
	DI_VDO_W_DW_NEED_HD((align_width / 4));	/* check */
#else
	DI_VDO_W_HD_MEM(((_arFmtTimingInfo[prParam->timing_type].size_h > 720) ? 1 : 0));
	DI_VDO_W_HD_MEM_1920(((_arFmtTimingInfo[prParam->timing_type].size_h > 1280) ? 1 : 0));
	DI_VDO_W_DW_NEED_HD((_arFmtTimingInfo[prParam->timing_type].size_h / 4));
#endif

	DI_VDO_W_PTR_AF_Y(0x26100000);	/* 0x4ec      //W - chroma */
	DI_VDO_W_WMV_DISABLE(0xffffffff);	/* 0x4f8 */
	DI_VDO_W_PTR_ZF_YC(0x26500000);	/* 0x4fc      //Z - chroma */

#if DI_VDO_SUPPORT_METRIC
	DI_VDO_W_METRIC_00(0x780f1001);	/* 0x700 */
#endif

#if DI_VDO_SUPPORT_FUSION
	DI_VDO_W_MDDI_FUSION_00(0x00000000);	/* 0x800 */
	DI_VDO_W_MDDI_FUSION_08(0x14140041);	/* 0x820 */
	DI_VDO_W_MDDI_FUSION_1A(0x00000000);	/* 0x868 */
	DI_VDO_W_MDDI_FUSION_1C(0x00388000);	/* 0x870 */
	DI_VDO_W_MDDI_FUSION_20(0x00040000);	/* 0x880 */
	DI_VDO_W_MDDI_FUSION_22(0xc0000400);	/* 0x888 */
	DI_VDO_W_MDDI_FUSION_23(0x0020fffe);	/* 0x88c */
#endif

#if DI_VDO_SUPPORT_MDDI_PE
	DI_VDO_W_MDDI_PE_00(0x00000005);	/* 0x8b0 */
	DI_VDO_W_MDDI_PE_03(0x00054444);	/* 0x8bc */
#endif

	DI_VDO_W_VDO_CRC_00(0x10000000);	/* 0xf00 */
	REG_W(DI_VDO_BASE + 0xf20, 0x00000000);	/* 0xf20 //check */
	REG_W(DI_VDO_BASE + 0xf24, 0x00c800c8);	/* 0xf24 //check */
	REG_W(DI_VDO_BASE + 0xf28, 0x03070206);	/* 0xf28 //check */
	REG_W(DI_VDO_BASE + 0xf2c, 0x05070406);	/* 0xf2c //check */
	DI_VDO_W_VDO_PQ_OPTION(0x00e0802b);	/* 0xf40 */
	DI_VDO_W_VDO_PQ_OPTION2(0x1e007800);	/* 0xf44 */
	REG_W(DI_VDO_BASE + 0xf4c, 0x00f29a0a);	/* 0xf4c //check */
	DI_VDO_W_MDDI_FILM_02(0x00033535);	/* 0xf60 */
	DI_VDO_W_CHROMA_SAW_CNT(0x00ff00ff);	/* 0xfe0 */
	DI_VDO_W_VDO_PQ_OPTION5(0x8050001e);	/* 0xfe4 */
	DI_VDO_W_DEMO_MODE(0x00000000);	/* 0xfec */
	DI_VDO_W_VDO_PQ_OPTION9(0x18060032);	/* 0xff4 */

	DI_VDO_W_VDO_CRC_00(0x20000000);	/* 0xf00 */
	DI_VDO_W_VDO_CRC_00(0x00000000);	/* 0xf00 */
	DI_VDO_W_VDO_CRC_00(0x10000000);	/* 0xf00 */
	DI_VDO_W_VDO_CRC_00(0x00000000);	/* 0xf00 */
	return DI_RET_OK;
}

static int di_vdo_config_di_mode(enum VDO_DI_MODE di_mode)
{
	if (di_mode == VDO_FRAME_MODE) {

		DI_VDO_W_FRMC(0x1);	/* 0x430[1:1] */
		DI_VDO_W_FRMY(0x1);	/* 0x430[0:0] */
		DI_VDO_W_MA4F(0x0);	/* 0x488[24] */
		DI_VDO_W_MA8F_OR(0x0);	/* 0x4c0[25] */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x0);	/* 0x800[0] */
#endif

	} else if (di_mode == VDO_FIELD_MODE) {

		DI_VDO_W_FRMC(0x0);	/* 0x430[1:1] */
		DI_VDO_W_FRMY(0x0);	/* 0x430[0:0] */
		DI_VDO_W_INTRA_EDGEP_MODE(0x0);	/* 0x478[11] */
		DI_VDO_W_MA4F(0x0);	/* 0x488[24] */
		DI_VDO_W_MA8F_OR(0x0);	/* 0x4c0[25] */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x0);	/* 0x800[0] */
#endif

	} else if (di_mode == VDO_INTRA_MODE_WITH_EDGE_PRESERVING) {

		DI_VDO_W_FRMC(0x0);	/* 0x430[1:1] */
		DI_VDO_W_FRMY(0x0);	/* 0x430[0:0] */
		DI_VDO_W_INTRA_EDGEP_MODE(0x1);	/* 0x478[11] */
		DI_VDO_W_MA4F(0x0);	/* 0x488[24] */
		DI_VDO_W_MA8F_OR(0x0);	/* 0x4c0[25] */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x0);	/* 0x800[0] */
#endif

	} else if (di_mode == VDO_4FIELD_MA_MODE) {

		DI_VDO_W_MA4F(0x1);	/* 0x488[24] */
		DI_VDO_W_MA8F_OR(0x0);	/* 0x4c0[25] */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x0);	/* 0x800[0] */
#endif

	} else if (di_mode == VDO_8FIELD_MA_MODE) {
		DI_VDO_W_MA4F(0x1);	/* 0x488[24]  // 4fld */
		DI_VDO_W_MA8F_OR(0x1);	/* 0x4c0[25]  // 8fld */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x0);	/* 0x800[0] */
#endif

	} else if (di_mode == VDO_FUSION_MODE) {
		DI_VDO_W_MA4F(0x1);	/* 0x488[24] */
		DI_VDO_W_MA8F_OR(0x0);	/* 0x4c0[25] */
#if DI_VDO_SUPPORT_FUSION
		DI_VDO_W_IFUSION_EN(0x1);	/* 0x800[0] */
#endif
	}

	return DI_RET_OK;
}

static int di_vdo_config_srcsize(unsigned int width, unsigned int height, enum VQ_COLOR_FMT src_fmt)
{
	DI_VDO_W_HBLOCK_7_0(width / 8);	/* 410[7:0] */

	DI_VDO_W_DW_NEED(width / 4);	/* 0x410[15:8] */

	DI_VDO_W_PICHEIGHT(height);	/* 0x410[26:16] */

	DI_VDO_W_DW_NEED_HD(width / 4);	/* 0x4e0[8:0] */

	DI_VDO_W_HD_LINE_MODE(((height >= 720) ? 1 : 0));	/* 0x4e0[20] //check */

	switch (src_fmt) {
	case VQ_COLOR_FMT_420BLK:
		DI_VDO_W_YUV422(0);
		DI_VDO_W_SLE(0);
		break;
	case VQ_COLOR_FMT_422BLK:
		DI_VDO_W_YUV422(1);
		DI_VDO_W_SLE(0);
		break;
	case VQ_COLOR_FMT_420SCL:
		DI_VDO_W_YUV422(0);
		DI_VDO_W_SLE(1);
		break;
	case VQ_COLOR_FMT_422SCL:
		DI_VDO_W_YUV422(1);
		DI_VDO_W_SLE(1);
		break;
	default:
		break;
	}

	return DI_RET_OK;
}

static int di_vdo_config_hwsram(enum VDO_DI_MODE di_mode, unsigned int width)
{
	unsigned int u4Hw_Sram_Util = 0;
	unsigned int hd_mem_1920;	/* 0x4e0[21] */
	unsigned int hd_mem;	/* 0x4e0[22] */
	unsigned int hd_en;	/* 0x4e0[24] */

	if (width <= 720) {

		/*for keep brace */
		u4Hw_Sram_Util = 0;

	} else if ((width > 720) && (width <= 1920)
		   && (di_mode == VDO_FRAME_MODE || di_mode == VDO_FIELD_MODE)) {

		/* check */
		/*for keep brace */
		u4Hw_Sram_Util = 1;

	} else if ((width > 720) && (width <= 1280) && (di_mode >= VDO_4FIELD_MA_MODE)) {

		/*for keep brace */
		u4Hw_Sram_Util = 2;

	} else if ((width > 1280) && (width <= 1920) && (di_mode >= VDO_4FIELD_MA_MODE)) {

		/*for keep brace */
		u4Hw_Sram_Util = 3;

	} else {

		/*for keep brace */
		DI_Printf(DI_LOG_ERROR, "[E] vdo sram config fail, w = %d, di = %d", width,
			  di_mode);
	}

	DI_Printf(DI_LOG_FLOW, "[F] vdo sram config to %d, w = %d, di = %d", u4Hw_Sram_Util, width,
		  di_mode);

	hd_mem_1920 = rVdoHwSramUtil[u4Hw_Sram_Util].hd_mem_1920;
	hd_mem = rVdoHwSramUtil[u4Hw_Sram_Util].hd_mem;
	hd_en = rVdoHwSramUtil[u4Hw_Sram_Util].hd_en;

	DI_VDO_W_HD_MEM_1920(hd_mem_1920);	/* 0x4e0[21] */
	DI_VDO_W_HD_MEM(hd_mem);	/* 0x4e0[22] */
	DI_VDO_W_HD_EN(hd_en);	/* 0x4e0[24] */

	return DI_RET_OK;
}

#if DI_VDO_SUPPORT_FUSION
static int di_vdo1_config_startline(enum VDO_DI_MODE di_mode,
				    unsigned int width,
				    enum VDO_VERTICAL_FILTER_MODE eVFilterMode,
				    enum VQ_COLOR_FMT src_fmt)
{
	unsigned int startline_y = 0;
	unsigned int startline_c = 0;
	unsigned int sub_startline_y = 0;
	unsigned int sub_startline_c = 0;
	unsigned int alter_startline_y = 0;
	unsigned int alter_startline_c = 0;
	enum VDO_YUV420_START_LINE_MODE vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE1;
	enum VDO_YUV422_START_LINE_MODE vdo1_422_startline_mode = VDO_YUV422_START_LINE_MODE1;

	if (width >= 720)
		eVFilterMode = VDO_VERTICAL_FILTER_LINEAR;

	if (src_fmt == VQ_COLOR_FMT_420BLK || src_fmt == VQ_COLOR_FMT_420SCL) {

		if (
			   /* SD, field/4field, no 4/8tap */
			   ((width <= 720) && (di_mode >= VDO_FIELD_MODE)
			    && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))
			   /* HD, field/4field, no 4tap */
			   || ((width > 720) && (di_mode >= VDO_FIELD_MODE)
			       && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))
			   /* SD, frame/4field, 4tap */
			   || ((width <= 720) && (di_mode == VDO_FRAME_MODE
						  || di_mode >= VDO_4FIELD_MA_MODE)
			       && (eVFilterMode == VDO_VERTICAL_FILTER_4TAP))) {

			vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE1;

		} else if (
				  /* SD, frame, no 4/8tap */
				  /* HD, frame, no 4tap */
				  ((di_mode == VDO_FRAME_MODE)
				   && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))) {

			vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE2;

		} else if (
				  /* SD, frame/4field, 4/8tap */
				  ((width <= 720) &&
				   (di_mode == VDO_FRAME_MODE || di_mode >= VDO_4FIELD_MA_MODE) &&
				   (eVFilterMode >= VDO_VERTICAL_FILTER_4TAP))) {

			vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE3;

		} else if (
				  /* 720p, frame/4field, 8tap */
				  ((width > 720 && width <= 1280) &&
				   (di_mode == VDO_FRAME_MODE || di_mode >= VDO_4FIELD_MA_MODE) &&
				   (eVFilterMode == VDO_VERTICAL_FILTER_8TAP))) {

			vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE4;

		} else {

			vdo1_420_startline_mode = VDO_YUV420_START_LINE_MODE1;
			DI_Printf(DI_LOG_ERROR,
				  "[E] vdo startline fail, srcfmt = %d, w = %d, di = %d, vfl = %d",
				  src_fmt, width, di_mode, eVFilterMode);
		}
		startline_y = vdo1_420_startline[vdo1_420_startline_mode].startline_y;
		startline_c = vdo1_420_startline[vdo1_420_startline_mode].startline_c;
		sub_startline_y = vdo1_420_startline[vdo1_420_startline_mode].sub_startline_y;
		sub_startline_c = vdo1_420_startline[vdo1_420_startline_mode].sub_startline_c;
		alter_startline_y = vdo1_420_startline[vdo1_420_startline_mode].alter_startline_y;
		alter_startline_c = vdo1_420_startline[vdo1_420_startline_mode].alter_startline_c;
	} else if (src_fmt == VQ_COLOR_FMT_422BLK || src_fmt == VQ_COLOR_FMT_422SCL) {

		if (
			   /* SD, field/4field, no 4/8tap */
			   ((width <= 720) && (di_mode >= VDO_FIELD_MODE)
			    && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))
			   /* HD, field/4field, no 4tap */
			   || ((width > 720) && (di_mode >= VDO_FIELD_MODE)
			       && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))) {

			vdo1_422_startline_mode = VDO_YUV422_START_LINE_MODE1;

		} else if (
				  /* SD, frame, no 4/8tap */
				  /* HD, frame, no 4tap */
				  ((di_mode == VDO_FRAME_MODE)
				   && (eVFilterMode == VDO_VERTICAL_FILTER_LINEAR))) {

			vdo1_422_startline_mode = VDO_YUV422_START_LINE_MODE2;

		} else if (
				  /* SD, frame/4field, 4/8tap */
				  ((width <= 720) &&
				   (di_mode == VDO_FRAME_MODE || di_mode >= VDO_4FIELD_MA_MODE) &&
				   (eVFilterMode >= VDO_VERTICAL_FILTER_4TAP))) {

			vdo1_422_startline_mode = VDO_YUV422_START_LINE_MODE3;

		} else {

			vdo1_422_startline_mode = VDO_YUV422_START_LINE_MODE1;
			DI_Printf(DI_LOG_ERROR,
				  "[E] vdo startline fail, srcfmt = %d, w = %d, di = %d, vfl = %d",
				  src_fmt, width, di_mode, eVFilterMode);
		}

		startline_y = vdo1_422_startline[vdo1_422_startline_mode].startline_y;
		startline_c = vdo1_422_startline[vdo1_422_startline_mode].startline_c;
		sub_startline_y = vdo1_422_startline[vdo1_422_startline_mode].sub_startline_y;
		sub_startline_c = vdo1_422_startline[vdo1_422_startline_mode].sub_startline_c;
		alter_startline_y = vdo1_422_startline[vdo1_422_startline_mode].alter_startline_y;
		alter_startline_c = vdo1_422_startline[vdo1_422_startline_mode].alter_startline_c;
	}

	DI_VDO_W_YSL(startline_y);	/* 0x420 */
	DI_VDO_W_CSL(startline_c);	/* 0x424 */
	DI_VDO_W_YSSL(sub_startline_y);	/* 0x428 */
	DI_VDO_W_CSSL(sub_startline_c);	/* 0x42C */
	DI_VDO_W_YSL2(alter_startline_y);	/* 0x450 */
	DI_VDO_W_CSL2(alter_startline_c);	/* 0x454 */

	DI_Printf(DI_LOG_FLOW,
		  "vdo startline success, srcfmt = %d, w = %d, di = %d, vfl = %d, 420 = %d, 422 = %d",
		  src_fmt, width, di_mode, eVFilterMode, vdo1_420_startline_mode,
		  vdo1_422_startline_mode);

	return DI_RET_OK;
}

#else
static void di_vdo2_config_startline(struct DI_VDO_PARAM_T *prParam)
{
	unsigned int startline_y = 0;
	unsigned int startline_c = 0;
	unsigned int sub_startline_y = 0;
	unsigned int sub_startline_c = 0;
	unsigned int alter_startline_y = 0;
	unsigned int alter_startline_c = 0;
	enum VDO_YUV420_START_LINE_MODE vdo_420_startline_mode = VDO_YUV420_START_LINE_MODE1;
	enum VDO_YUV422_START_LINE_MODE vdo_422_startline_mode = VDO_YUV422_START_LINE_MODE1;
	struct VDO_START_LINE_T vdo2_startline;

	if (prParam->src_fmt == VQ_COLOR_FMT_422BLK || prParam->src_fmt == VQ_COLOR_FMT_422SCL) {
		if (prParam->di_mode == VDO_FRAME_MODE)
			vdo_422_startline_mode = VDO_YUV422_START_LINE_MODE2;
		else
			vdo_422_startline_mode = VDO_YUV422_START_LINE_MODE1;

		vdo2_startline = vdo2_422_startline[vdo_422_startline_mode];
	} else {
		if (prParam->di_mode == VDO_FRAME_MODE)
			vdo_420_startline_mode = VDO_YUV420_START_LINE_MODE2;
		else
			vdo_420_startline_mode = VDO_YUV420_START_LINE_MODE1;

		vdo2_startline = vdo2_420_startline[vdo_420_startline_mode];
	}
	startline_y = vdo2_startline.startline_y;
	startline_c = vdo2_startline.startline_c;
	sub_startline_y = vdo2_startline.sub_startline_y;
	sub_startline_c = vdo2_startline.sub_startline_c;
	alter_startline_y = vdo2_startline.alter_startline_y;
	alter_startline_c = vdo2_startline.alter_startline_c;

	DI_VDO_W_YSL(startline_y);	/* 0x420 */
	DI_VDO_W_CSL(startline_c);	/* 0x424 */
	DI_VDO_W_YSSL(sub_startline_y);	/* 0x428 */
	DI_VDO_W_CSSL(sub_startline_c);	/* 0x42C */
	DI_VDO_W_YSL2(alter_startline_y);	/* 0x450 */
	DI_VDO_W_CSL2(alter_startline_c);	/* 0x454 */
}
#endif
static int di_vdo_config_ma8f(unsigned char bIsOn, unsigned int AddrW, unsigned int Addr1,
			      unsigned int Addr2)
{
#if 0
	if (bIsOn) {

		vVdo_WriteReg(0x1c002458, PHYSICAL(Addr1));
		vVdo_WriteReg(0x1c00245c, PHYSICAL(Addr2));
		vVdo_WriteReg(0x1c0024d8, PHYSICAL(AddrW));

		vVdo_WriteReg(0x1c002f54, PHYSICAL(AddrW));
		vVdo_WriteReg(0x1c0024dc, PHYSICAL(Addr2));
	}

	vVdo_WriteRegMask(0x1c002440, bIsOn << 29, 0x1 << 29);	/* DYN_8F enable dynamic 8-field motion detection */
	vVdo_WriteRegMask(0x1c002488, bIsOn << 31, 0x1 << 31);	/* 6fld */
	vVdo_WriteRegMask(0x1c0024c0, bIsOn << 25, 0x1 << 25);	/* 8fld */
	vVdo_WriteRegMask(0x1c0024c0, 0x3 << 16, 0xf << 16);	/* u4VAC_6F,SD */

	vVdo_WriteRegMask(0x1c00247c, 0 << 12, 0x1 << 12);	/* fgPROT_WR_STA */
	vVdo_WriteRegMask(0x1c00247c, 0 << 13, 0x1 << 13);	/* fgPROT_WR_END */

	if (bIsOn) {

		/*for keep brace */
		vVdo_WriteRegMask(0x1c002494, 0x48 << 24, 0xff << 24);	/* test mode */

	} else {

		/*for keep brace */
		vVdo_WriteRegMask(0x1c002494, 0x00 << 24, 0xff << 24);	/* test mode */
	}
#endif

	return DI_RET_OK;
}



static int di_vdo_config_deint_wxyza(unsigned int *pu4AddrY, unsigned int *pu4AddrC)
{
	if ((pu4AddrY) && (pu4AddrC)) {

		DI_VDO_W_PTR_WF_Y((pu4AddrY[0] >> 2));	/* 0x480   // W */
		DI_VDO_W_VDOY2((pu4AddrY[1] >> 2));	/* 0x408   // X */
		DI_VDO_W_VDOY1((pu4AddrY[2] >> 2));	/* 0x400   // Y */
		DI_VDO_W_PTR_ZF_Y((pu4AddrY[3] >> 2));	/* 0x484   // Z */
		DI_VDO_W_PTR_AF_Y((pu4AddrY[4] >> 2));	/* 0x4ec   // A */


		DI_VDO_W_VDOC2((pu4AddrC[1] >> 2));	/* 0x40c   // x */
		DI_VDO_W_VDOC1((pu4AddrC[2] >> 2));	/* 0x404   // y */
		DI_VDO_W_PTR_ZF_YC((pu4AddrC[3] >> 2));	/* 0x4fc   // z */
	}

	return DI_RET_OK;
}

static void di_vdo_config_wxyza_buffer(struct DI_VDO_PARAM_T *prParam)
{
	unsigned int addry[5] = {0};	/*wxyza addr */
	unsigned int addrc[5] = {0};
	int idx = 0;

	if (prParam->top_field_first_en) {
		if (prParam->cur_top_enable) {
			DI_Printf(DI_LOG_FLOW, "top_first,cur is top field");
			addry[0] = prParam->addr_y_next;
			addry[1] = prParam->addr_y_curr;
			addry[2] = prParam->addr_y_curr;
			addry[3] = prParam->addr_y_prev;
			addry[4] = prParam->addr_y_prev;

			addrc[0] = prParam->addr_cbcr_next;
			addrc[1] = prParam->addr_cbcr_curr;
			addrc[2] = prParam->addr_cbcr_curr;
			addrc[3] = prParam->addr_cbcr_prev;
			addrc[4] = prParam->addr_cbcr_prev;
		} else {
			DI_Printf(DI_LOG_FLOW, "top_first,cur is bottom field");
			addry[0] = prParam->addr_y_next;
			addry[1] = prParam->addr_y_next;
			addry[2] = prParam->addr_y_curr;
			addry[3] = prParam->addr_y_curr;
			addry[4] = prParam->addr_y_prev;

			addrc[0] = prParam->addr_cbcr_next;
			addrc[1] = prParam->addr_cbcr_next;
			addrc[2] = prParam->addr_cbcr_curr;
			addrc[3] = prParam->addr_cbcr_curr;
			addrc[4] = prParam->addr_cbcr_prev;
		}
	} else {
		if (!prParam->cur_top_enable) {
			DI_Printf(DI_LOG_FLOW, "bottom_first,cur is top field");
			addry[0] = prParam->addr_y_next;
			addry[1] = prParam->addr_y_curr;
			addry[2] = prParam->addr_y_curr;
			addry[3] = prParam->addr_y_prev;
			addry[4] = prParam->addr_y_prev;

			addrc[0] = prParam->addr_cbcr_next;
			addrc[1] = prParam->addr_cbcr_curr;
			addrc[2] = prParam->addr_cbcr_curr;
			addrc[3] = prParam->addr_cbcr_prev;
			addrc[4] = prParam->addr_cbcr_prev;
		} else {
			DI_Printf(DI_LOG_FLOW, "bottom_first,cur is bottom field");
			addry[0] = prParam->addr_y_next;
			addry[1] = prParam->addr_y_next;
			addry[2] = prParam->addr_y_curr;
			addry[3] = prParam->addr_y_curr;
			addry[4] = prParam->addr_y_prev;

			addrc[0] = prParam->addr_cbcr_next;
			addrc[1] = prParam->addr_cbcr_next;
			addrc[2] = prParam->addr_cbcr_curr;
			addrc[3] = prParam->addr_cbcr_curr;
			addrc[4] = prParam->addr_cbcr_prev;
		}
	}

	for (idx = 0; idx < 5; ++idx)
		DI_Printf(DI_LOG_FLOW, "wxyza[%d]addry:0x%x,addrc:0x%x", idx, addry[idx],
			  addrc[idx]);

	di_vdo_config_deint_wxyza(addry, addrc);

}




static int di_vdo_config_acs_top(unsigned char u1AcsTop)
{
	DI_VDO_W_PFLD((!u1AcsTop));	/* 0x430[2] */
	DI_VDO_W_AFLD((!u1AcsTop));	/* 0x430[5] */

	return DI_RET_OK;
}



static int di_vdo_config_threshold(unsigned short u2CtThrehold, unsigned short u2MovingThrehold,
				   unsigned short u2Pd_Threhold)
{
	DI_VDO_W_CT_THRD(u2CtThrehold);	/* 0x438[15:8] */
	DI_VDO_W_MTHRD(u2MovingThrehold);	/* 0x438[23:16] */
	DI_VDO_W_PD_COMB_TH(u2Pd_Threhold);	/* 0x4C4[22:16] */

	return DI_RET_OK;
}

static int di_vdo_config_certical_chroma_detect(unsigned char bIsOn, unsigned char bRange,
						unsigned int u4Threhold)
{
	DI_VDO_W_chroma_multi_burst_en(bIsOn);	/* 0xFF4[18] */
	DI_VDO_W_chroma_multi_burst_pixel_sel(bRange);	/* 0xFF4[19] */
	DI_VDO_W_chroma_multi_burst_threshold(u4Threhold);	/* 0xFF4[31:24] */

	return DI_RET_OK;
}

static int di_vdo_config_sharp_prepara(unsigned char bypass)
{
	DI_VDO_W_BYPASS_SHARP(bypass);	/* 0x1B8[24] */

	return DI_RET_OK;
}


#if DI_VDO_SUPPORT_FUSION

static int di_vdo_config_fusion_buffer(unsigned char enableBuffer)
{
	if ((enableBuffer == 1) && (di_init.fusionbuf_va == 0) && (di_init.fusionbuf_mva == 0)) {

		di_init.fusionbuf_va = dma_alloc_coherent(di_init.pm_dev,
							  MA8F_BUFFER_TOTAL_SIZE,
							  &di_init.fusionbuf_mva, GFP_KERNEL);

		DI_Printf(DI_LOG_FLOW,
			  "[D] FusionBuf alloc success,pm_dev=%p, Va = 0x%p, Mva = 0x%x, size = 0x%x",
			  di_init.pm_dev, di_init.fusionbuf_va, (uint32_t) di_init.fusionbuf_mva,
			  MA8F_BUFFER_TOTAL_SIZE);
	} else if ((enableBuffer == 0) && (di_init.fusionbuf_va != 0)
		   && (di_init.fusionbuf_mva != 0)) {

		dma_free_coherent(di_init.pm_dev,
				  MA8F_BUFFER_TOTAL_SIZE,
				  di_init.fusionbuf_va, di_init.fusionbuf_mva);

		DI_Printf(DI_LOG_FLOW, "[D] FusionBuf free success, Va = 0x%p, Mva= 0x%x",
			  di_init.fusionbuf_va, (uint32_t) di_init.fusionbuf_mva);

		di_init.fusionbuf_va = 0;
		di_init.fusionbuf_mva = 0;
	} else {

		DI_Printf(DI_LOG_FLOW, "[W] FusionBuf status, Enable = %d, Va = 0x%p, Mva= 0x%x",
			  enableBuffer, di_init.fusionbuf_va, (uint32_t) di_init.fusionbuf_mva);
	}
	return DI_RET_OK;
}

static int di_vdo_config_fusion_drammode(unsigned char fusion_enable, unsigned int AddrW)
{
	DI_VDO_W_IFUSION_EN(fusion_enable);	/* 0x800[0] */

	/* AddrW = (AddrW) ? PHYSICAL(AddrW) : 0;    //check */

	if (fusion_enable) {

		DI_VDO_W_fusion_flag_addr_base(AddrW / 16);	/* 0x870[27:0] */
		DI_VDO_W_da_flag_waddr_hi_limit(((AddrW + MA8F_BUFFER_TOTAL_SIZE)) / 16);	/* 0x874[27:0] */
		DI_VDO_W_da_flag_waddr_lo_limit(AddrW / 16);	/* 0x878[27:0] */
	}

	DI_VDO_W_en_lmw(fusion_enable);	/* 0x870[28] */
	DI_VDO_W_en_lmr(fusion_enable);	/* 0x870[29] */

	return DI_RET_OK;
}


#endif

int di_vdo_config_set_param(struct DI_VDO_PARAM_T *prParam)
{

	di_vdo_config_base(prParam);


	di_vdo_config_di_mode(prParam->di_mode);

	di_vdo_config_srcsize(prParam->align_width, prParam->height, prParam->src_fmt);

	di_vdo_config_hwsram(prParam->di_mode, prParam->align_width);

#if DI_VDO_SUPPORT_FUSION
	di_vdo1_config_startline(prParam->di_mode, prParam->align_width, VDO_VERTICAL_FILTER_4TAP,
				 prParam->src_fmt);
#else
	di_vdo2_config_startline(prParam);
#endif
	di_vdo_config_wxyza_buffer(prParam);

#if 0
	if (!prParam->top_field_first_en
	//&& prParam->area_size <= DI_SD_AREA_SIZE
	)
		di_vdo_config_acs_top(!prParam->cur_top_enable);
	else
		di_vdo_config_acs_top(prParam->cur_top_enable);
#else
	di_vdo_config_acs_top(prParam->cur_top_enable);
#endif

#if DI_VDO_SUPPORT_FUSION
	if (prParam->di_mode == VDO_FUSION_MODE) {

		di_vdo_config_fusion_buffer(1);

		di_vdo_config_fusion_drammode(1, di_init.fusionbuf_mva);

	} else {

		di_vdo_config_fusion_buffer(0);

		di_vdo_config_fusion_drammode(0, di_init.fusionbuf_mva);

	}
#endif

	if (prParam->h265_enable) {
		DI_VDO_W_H265_EN(1);
		DI_VDO_W_SLE(1);
	} else {
		DI_VDO_W_H265_EN(0);
	}

	di_vdo_config_threshold(vdo_contrast_threshold, vdo_moving_threshold,
				vdo_pulldown_comb_threshold);

	di_vdo_config_certical_chroma_detect(1, 0, 0x18);

	di_vdo_config_sharp_prepara(0);

	di_vdo_config_ma8f(0, 0, 0, 0);

	return DI_RET_OK;
}

static int di_vdo_config_enable(unsigned char enable)
{

	return DI_RET_OK;
}

static void di_wch_config_set_block_param(struct DI_WC_PARAM_T *prParam)
{
	int act_hcnt = 0;

	if (prParam->width % 16 == 0)
		act_hcnt = prParam->width;
	else
		act_hcnt = ((prParam->width / 16) * 16) + 16;


	DI_Printf(DI_LOG_ERROR, "width=%d, height= %d timing_type=%d act_hcnt=%d",
		  prParam->width, prParam->height, prParam->timing_type, act_hcnt);
	DI_WC_W_BLOCK_ACT_HCNT(act_hcnt);

}


static int di_wch_config_set_param(struct DI_WC_PARAM_T *prParam)
{
	DI_WC_W_VDOIN_EN(0x00000000);	/* 0x000 */
	DI_WC_W_VDOIN_EN(0x8200400d);	/* 0x000 */

	/* 0x000[25] 16*32 block */
	if (prParam->dst_fmt == VQ_COLOR_FMT_420BLK) {
		DI_WC_W_mode_422(0);
		DI_WC_W_MACRO_64_32_BLOCK(0);
		di_wch_config_set_block_param(prParam);
		DI_WC_W_linear_enable(0);

	} else if (prParam->dst_fmt == VQ_COLOR_FMT_420SCL) {
		DI_WC_W_mode_422(0);
		DI_WC_W_linear_enable(1);

	} else if (prParam->dst_fmt == VQ_COLOR_FMT_422BLK) {
		DI_WC_W_mode_422(1);
		DI_WC_W_MACRO_64_32_BLOCK(0);
		di_wch_config_set_block_param(prParam);
		DI_WC_W_linear_enable(0);

	} else if (prParam->dst_fmt == VQ_COLOR_FMT_422SCL) {
		DI_WC_W_mode_422(1);
		DI_WC_W_linear_enable(1);

	} else {

		DI_Printf(DI_LOG_ERROR, "[E] not support dst color %d", prParam->dst_fmt);
		return DI_RET_ERR_PARAM;
	}
	DI_Printf(DI_LOG_FLOW, "[WCH] y[0x%x,0x%x], c[0x%x,0x%x]",
		  prParam->output_addr_y, prParam->output_addr_y / 4,
		  prParam->output_addr_cbcr, prParam->output_addr_cbcr / 4);

	DI_WC_W_VDOIN_MODE(0x38000040);	/* 0x004 */

	DI_WC_W_YBUF0_ADDR((prParam->output_addr_y >> 2));	/* 0x008 */

	DI_WC_W_ACT_LINE(0xf20811df);	/* 0x00C */

#if 1				/* VQ_WH_TIMING */
	DI_WC_W_ACTLINE((prParam->height - 1));	/* 0x00C[11:0] */
#else
	DI_WC_W_ACTLINE((_arFmtTimingInfo[prParam->timing_type].size_v - 1));	/* 0x00C[11:0] */
#endif

	DI_WC_W_CBUF0_ADDR((prParam->output_addr_cbcr >> 2));	/* 0x010 */

#if 1				/* VQ_WH_TIMING */
	if ((prParam->dst_fmt == VQ_COLOR_FMT_420BLK) || (prParam->dst_fmt == VQ_COLOR_FMT_420SCL)) {

		/*for keep brace */
		DI_WC_W_DW_NEED_C_LINE(((prParam->height / 2) - 1));	/* 0x014[27:16] */

	} else if ((prParam->dst_fmt == VQ_COLOR_FMT_422BLK)
		   || (prParam->dst_fmt == VQ_COLOR_FMT_422SCL)) {

		/*for keep brace */
		DI_WC_W_DW_NEED_C_LINE((prParam->height - 1));	/* 0x014[27:16] */
	}

	DI_WC_W_DW_NEED_Y_LINE((prParam->height - 1));	/* 0x014[11:0] */
#else
	DI_WC_W_DW_NEED_C_LINE((_arFmtTimingInfo[prParam->timing_type].size_v - 1));	/* 0x014[27:16] */
	DI_WC_W_DW_NEED_Y_LINE((_arFmtTimingInfo[prParam->timing_type].size_v - 1));	/* 0x014[11:0] */
#endif

	DI_WC_W_HPIXEL(0x0001003b);	/* 0x018 */

	DI_WC_W_INPUT_CTRL(0x0400c024);	/* 0x020 */

	DI_WC_W_HCNT_SETTING(0x02d00359);	/* 0x034 */

#if 1				/* VQ_WH_TIMING */
	DI_WC_W_HACTCNT(prParam->width);	/* 0x034[28:16] */
#else
	DI_WC_W_HACTCNT(_arFmtTimingInfo[prParam->timing_type].size_h);	/* 0x034[28:16] */
#endif

#if 1				/* VQ_WH_TIMING */
	DI_WC_W_CHCNT((prParam->width / 16 - 1));	/* 0x038[25:16] */
	DI_WC_W_YHCNT((prParam->width / 16 - 1));	/* 0x038[9:0] */
#else
	DI_WC_W_CHCNT((_arFmtTimingInfo[prParam->timing_type].size_h / 16 - 1));	/* 0x038[25:16] */
	DI_WC_W_YHCNT((_arFmtTimingInfo[prParam->timing_type].size_h / 16 - 1));	/* 0x038[9:0] */
#endif

	DI_WC_W_VSCALE(0x002d40e3);	/* 0x03C */
#if 1				/* VQ_WH_TIMING */
	DI_WC_W_bghsize_dw((prParam->width / 16));	/* 0x03C[25:16] */
#else
	DI_WC_W_bghsize_dw((_arFmtTimingInfo[prParam->timing_type].size_h / 16));	/* 0x03C[25:16] */
#endif

	DI_WC_W_swrst(0x3);	/* 0x030 */
	DI_WC_W_swrst(0x0);	/* 0x030 */

	return DI_RET_OK;
}


int di_config_power_switch(int iPowerOn)
{
	if (iPowerOn == 0) {

	} else {
#if (VQ_SUPPORT_ION)
		di_ion_init();
#endif
	}
	di_vdo_config_enable(iPowerOn);

	return DI_RET_OK;
}

int pq_tuning = 0;

int di_hal_config(struct mtk_vq_config_info *config_info)
{
	int iRet = DI_RET_OK;
	struct DI_VDO_PARAM_T rVdoParam;
	struct DISPFMT_PARAM_T rDispfmtParam;
	struct DI_WC_PARAM_T rWcParam;

	enum VQ_TIMING_TYPE timing_type;
	struct mtk_vq_config *config = config_info->vq_config;
	unsigned long start_timer, end_timer;

	start_timer = sched_clock();

	if ((config->src_align_width <= TIMING_SIZE_720_480_H) &&
	    (config->src_align_height <= TIMING_SIZE_720_480_V)) {
		timing_type = VQ_TIMING_TYPE_480P;
	} else if ((config->src_align_width <= TIMING_SIZE_720_576_H) &&
		   (config->src_align_height <= TIMING_SIZE_720_576_V)) {
		timing_type = VQ_TIMING_TYPE_576P;
	} else if ((config->src_align_width <= TIMING_SIZE_1280_720_H) &&
		   (config->src_align_height <= TIMING_SIZE_1280_720_V)) {
		timing_type = VQ_TIMING_TYPE_720P;
	} else if ((config->src_align_width <= TIMING_SIZE_1920_1080_H) &&
		   (config->src_align_height <= TIMING_SIZE_1920_1080_V)) {
		timing_type = VQ_TIMING_TYPE_1080P;
	} else {
		DI_Printf(DI_LOG_ERROR, "[E] not support width = %d, height = %d",
			  config->src_width, config->src_height);
		return DI_RET_ERR_PARAM;
	}


	if (config->src_fmt > VQ_COLOR_FMT_422SCL) {
		DI_Printf(DI_LOG_ERROR, "[E] not support src color format %d", config->src_fmt);
		return DI_RET_ERR_PARAM;
	}

	if (config->dst_fmt > VQ_COLOR_FMT_422SCL) {
		DI_Printf(DI_LOG_ERROR, "[E] not support dst color format %d", config->dst_fmt);
		return DI_RET_ERR_PARAM;
	}

	DI_Printf(DI_LOG_FLOW,
		  "[PARAM]Timing[%d], Pic[%d, %d], Frm[%d, %d], DI[%d, %d, %d,%d], Col[%d, %d], Nr[%d, %d]",
		  timing_type,
		  config->src_width,
		  config->src_height,
		  config->src_align_width,
		  config->src_align_height,
		  config->di_mode,
		  config->cur_field,
		  config->topfield_first_enable,
		  config->h265_enable,
		  config->src_fmt, config->dst_fmt, config->mnr_level, config->bnr_level);

#if VQ_TIME_CHECK
	VQ_TIME_REC(2);
#endif

	/* config dispfmt */
	rDispfmtParam.timing_type = timing_type;
	rDispfmtParam.width = config->src_align_width;
	rDispfmtParam.height = config->src_align_height;
	if (config->vq_mode == VQ_DI_STANDALONE)
		rDispfmtParam.nr_enable = 0;
	else
		rDispfmtParam.nr_enable =
		    (((config->mnr_level == 0) && (config->bnr_level == 0)) ? 0 : 1);
	rDispfmtParam.h265_enable = config->h265_enable;
	rDispfmtParam.pattern.enable = 0;
	di_dispfmt_setparam(&rDispfmtParam);

#if VQ_TIME_CHECK
	VQ_TIME_REC(3);
#endif
	/* config vdo */
	switch (config->di_mode) {
	case VQ_DI_MODE_FRAME:
		rVdoParam.di_mode = VDO_FRAME_MODE;
		break;
	case VQ_DI_MODE_4_FIELD:
#if DI_VDO_SUPPORT_FUSION
		rVdoParam.di_mode = VDO_FUSION_MODE;
#else
		rVdoParam.di_mode = VDO_4FIELD_MA_MODE;
#endif
		break;
	case VQ_DI_MODE_FIELD:
		rVdoParam.di_mode = VDO_FIELD_MODE;
		break;
	default:
		DI_Printf(DI_LOG_ERROR, "[E] map dimode fail, input %d", config->di_mode);
		return DI_RET_ERR_PARAM;
	}

	rVdoParam.src_fmt = config->src_fmt;
	rVdoParam.cur_top_enable = (config->cur_field == VQ_FIELD_TYPE_TOP) ? 1 : 0;
	rVdoParam.top_field_first_en = config->topfield_first_enable;
	rVdoParam.width = config->src_width;
	rVdoParam.height = config->src_height;
	rVdoParam.align_width = config->src_align_width;
	rVdoParam.align_height = config->src_align_height;
	if (config_info->src_mva[0]) {
		rVdoParam.addr_y_prev = config_info->src_mva[0] + config->src_ofset_y_len[0];
		rVdoParam.addr_cbcr_prev = config_info->src_mva[0] + config->src_ofset_c_len[0];

		rVdoParam.addr_y_curr = config_info->src_mva[1] + config->src_ofset_y_len[1];
		rVdoParam.addr_cbcr_curr = config_info->src_mva[1] + config->src_ofset_c_len[1];

		rVdoParam.addr_y_next = config_info->src_mva[2] + config->src_ofset_y_len[2];
		rVdoParam.addr_cbcr_next = config_info->src_mva[2] + config->src_ofset_c_len[2];
	}
	rVdoParam.h265_enable = config->h265_enable;
	rVdoParam.area_size = rVdoParam.width * rVdoParam.height;
#if 0
	if (!config->topfield_first_enable
	//&& rVdoParam.area_size <= DI_SD_AREA_SIZE
	)
		rVdoParam.di_mode = VDO_FIELD_MODE;
#endif

	DI_Printf(DI_LOG_FLOW, "map DiMode %d to vdo mode %d src_mva[0x%x]", config->di_mode,
		  rVdoParam.di_mode, config_info->src_mva[0]);

	di_vdo_config_set_param(&rVdoParam);

	if (IS_OTHER_PICTURE(rVdoParam.area_size) || pq_tuning) {
		MA_THRESHOLD_T *p_ma_th = &MA_FHD_TH;

		if (IS_FULL_FHD_PICTURE(rVdoParam.area_size))
			p_ma_th = &MA_FHD_TH;
		else if (IS_HD_PICTURE(rVdoParam.area_size))
			p_ma_th = &MA_HD_TH;
		else if (IS_PAL_PICTURE(rVdoParam.area_size))
			p_ma_th = &MA_PAL_TH;
		else if (IS_NTSC_PICTURE(rVdoParam.area_size))
			p_ma_th = &MA_NTSC_TH;
		else
			p_ma_th = &MA_OTHER_TH;

		VDP_HalSetDeintParam(p_ma_th);
	}

#if VQ_TIME_CHECK
	VQ_TIME_REC(4);
#endif
	/* config write channel */
	rWcParam.timing_type = timing_type;
	rWcParam.dst_fmt = config->dst_fmt;
	rWcParam.width = config->src_align_width;
	rWcParam.height = config->src_align_height;
	if (config_info->dst_mva) {
		rWcParam.output_addr_y = config_info->dst_mva;
		rWcParam.output_addr_cbcr = config_info->dst_mva + config->dst_ofset_c_len;
		di_init.wch_va_y = config_info->dst_va;
		di_init.wch_va_c = config_info->dst_va + config->dst_ofset_c_len;
	}
	di_init.count++;

	di_wch_config_set_param(&rWcParam);
	/* init event */
	vq_di_EventReg(1);
	DI_Printf(DI_LOG_FLOW, "hw time is %ld before trigger",
		  (end_timer - start_timer) / 1000000);

	/* triggle hw */
	di_dispfmt_trigger(&rDispfmtParam);
	end_timer = sched_clock();

	DI_Printf(DI_LOG_FLOW, "[F] config end");
	return iRet;
}


int di_hal_power_on(struct di_info *di)
{
	int ret = 0;
	int i;

	DI_Printf(DI_LOG_FLOW, "di_hal_power_on");
	if (di == NULL)
		di = &di_init;
	for (i = 0; i < MTK_DI_CLK_CNT - 1; i++) {
		ret = clk_prepare_enable(di->clks[i]);
		if (ret) {
			for (i -= 1; i >= 0; i--)
				clk_disable_unprepare(di->clks[i]);
			break;
		}
	}

	DI_Printf(DI_LOG_FLOW, "clk_set_parent CLK_TOP_DI_SEL[%d]->CLK_TOP_TVDPLL_D2[%d]",
		  CLK_TOP_DI_SEL, CLK_TOP_TVDPLL_D2);
	clk_set_parent(di->clks[CLK_TOP_DI_SEL], di->clks[CLK_TOP_TVDPLL_D2]);

	return ret;
}

int di_hal_power_off(struct di_info *di)
{
	int i;

	if (di == NULL)
		di = &di_init;
	DI_Printf(DI_LOG_FLOW, "di_hal_power_off ");

	for (i = 0; i < MTK_DI_CLK_CNT - 1; i++)
		clk_disable_unprepare(di->clks[i]);


	return 0;
}

void di_hal_free_irq(struct di_info *di)
{
	/* devm_free_irq(nr->dev, nr->irq, (void *)nr); */
}

void VDP_HalSetDeintParam (MA_THRESHOLD_T * pThr)
{
#if 0
	DI_VDO_W_FDIFF_TH3(pThr->DIFF_THR3); // 0x488
	DI_VDO_W_FDIFF_TH2(pThr->DIFF_THR2); // 0x48C
	DI_VDO_W_FDIFF_TH1(pThr->DIFF_THR1); // 0x490

	DI_VDO_W_TH_MIN_XZ(pThr->TH_MIN_XZ); // 0x494
	DI_VDO_W_TH_MED_XZ(pThr->TH_MED_XZ);

	DI_VDO_W_TH_EDGE_XZ(pThr->TH_ED_XZ); // 0x498
	DI_VDO_W_TH_NORM_XZ(pThr->TH_NM_XZ); // 0x498

	DI_VDO_W_SAW_TH(pThr->SAW_TH);       // 0x49c
	DI_VDO_W_TH_MED_YW(pThr->TH_MED_YW);
	DI_VDO_W_TH_MIN_YW(pThr->TH_MIN_YW);

	DI_VDO_W_VMV_FCH(pThr->VMV_FCH); // 0X4A4
	DI_VDO_W_FCH_NM_XZ(pThr->FCH_NM_XZ);
	DI_VDO_W_FCH_MIN_XZ(pThr->FCH_MIN_XZ);


	DI_VDO_W_FCH_NM_YW(pThr->FCH_NM_YW); // 0X4A8
	DI_VDO_W_FCH_MIN_YW(pThr->FCH_MIN_YW);

	DI_VDO_W_EDGE_3LINE_GRAD_TH(pThr->EDGE_3LINE_GRAD_TH);  // 0x4B8

	DI_VDO_W_WH_TX_TH(pThr->WH_TX_TH);   // 0x4a0
	DI_VDO_W_TH_ED_YW(pThr->TH_ED_YW);  // 0x4a0
#endif

	DI_VDO_W_H_ED_TH(pThr->H_ED_TH);     // 0x498

	DI_VDO_W_TH_NM_YW(pThr->TH_NM_YW);  // 0x4a0

	DI_VDO_W_MA_EDGE_MODE6(pThr->MA_EDGE_MODE6); // 0x4A0
}
