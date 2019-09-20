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
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/pm_runtime.h>
#include "smi.h"

#include "imgresz.h"
#include "imgresz_priv.h"
#include "imgresz_hal.h"

#define LOG_TAG "IMGRESZ"
#include "disp_hw_log.h"


#define TICKET_FROM_HWID(id)         ((0x1 << 28) | (id))
#define TICKET_TO_HWID(ticket)       ((ticket) & 0x3)

#define IMGRESZ_MAX_NR			2
#define IMGRZ_HW_TIMEOUT		600

enum IMGRESZ_STATE {
	IMGRESZ_STATE_IDLE	= 0,
	IMGRESZ_STATE_BUSY	= 1,
	IMGRESZ_STATE_DONE	= 2,
	IMGRESZ_STATE_TIMEOUT	= 3,
	IMGRESZ_STATE_STOP	= 4,
};

#if IMGRZ_TEE_ENABLE
static KREE_SESSION_HANDLE imgresz_ta_session;
#if IMGRESZ_TEST_EP
static KREE_SESSION_HANDLE imgresz_mem_session;
#endif
#endif

struct mtk_imgresz_compatible {
	enum IMGRESZ_CHIP_VER imgresz_curr_chip_ver;
};

static const struct mtk_imgresz_compatible mt8697_compat = {
	.imgresz_curr_chip_ver = IMGRESZ_CURR_CHIP_VER_8697,
};

static const struct mtk_imgresz_compatible mt8695_compat = {
	.imgresz_curr_chip_ver = IMGRESZ_CURR_CHIP_VER_8695,
};

struct imgresz_scale_data {
	enum imgresz_ticket_fun_type	funtype;
	enum imgresz_scale_mode		scale_mode;

	enum IMGRESZ_RESAMPLE_METHOD_T	h_method;
	enum IMGRESZ_RESAMPLE_METHOD_T	v_method;
	struct imgresz_src_buf_info	src_buf;
	struct imgresz_dst_buf_info	dst_buf;
	struct imgresz_buf_format	src_format;
	struct imgresz_buf_format	dst_format;
	struct imgresz_rm_info		rm_info;
	struct imgresz_jpg_info		jpg_info;
	struct imgresz_partial_buf_info partial_info;
	struct imgresz_hal_info     hal_info;
	union imgresz_partition_info  partition_info;

	bool				ufo_v_partition;
	bool				ufo_h_partition;
	bool				ufo_page0;
	bool				one_phase;
	bool				outstanding;
};

struct imgresz_data {
	struct platform_device	*pdev;
	unsigned int	irq;
	void __iomem	*base;
	struct clk		*reszclk;
	struct device	*smi_larb_dev;

	spinlock_t			state_lock; /* lock for hw state */
	enum IMGRESZ_STATE		state;
	unsigned int			hw_id;
	struct timer_list		timer;      /* For imgresz timeout */
	unsigned long			hw_timeout;
	unsigned long long		time_start, time_end;
	struct imgresz_scale_data	scale_data; /* The scale info in this time. */
	struct imgresz_cb_data	cb_info;
	wait_queue_head_t	wait_hw_done_queue;
};

struct imgresz_tz_data {
	uint32_t	base;
	unsigned int	irq;
	unsigned int	hw_id;
	int log_level;
	int	hw_timeout;
	enum IMGRESZ_STATE		state;
	struct imgresz_scale_data	scale_data;
};

static unsigned int imgresz_hw_num;
static struct imgresz_data *imgresz_inst_data[IMGRESZ_MAX_NR];
static struct imgresz_tz_data *imgresz_tz_inst_data[IMGRESZ_MAX_NR];

/* Debug help. */
int imgresz_log_level;
enum IMGRESZ_CHIP_VER imgresz_cur_chip_ver;

static void imgresz_hal_set_chip_ver(enum IMGRESZ_CHIP_VER chip)
{
	imgresz_cur_chip_ver = chip;
	if (chip == IMGRESZ_CURR_CHIP_VER_8695)
		imgresz_hw_num = 2;
	else if (chip == IMGRESZ_CURR_CHIP_VER_8697)
		imgresz_hw_num = 4;
}

static bool imgresz_valid_ticket(const IMGRESZ_TICKET ti)
{
	return (!!(ti & (0x1 << 28))) && (TICKET_TO_HWID(ti) < IMGRESZ_MAX_NR);
}

static struct imgresz_data *imgresz_get_data(const IMGRESZ_TICKET ti)
{
	if (!imgresz_valid_ticket(ti)) {
		WARN_ON(1);
		return NULL;
	} else {
		return imgresz_inst_data[TICKET_TO_HWID(ti)];
	}
}

#if IMGRZ_TEE_ENABLE
static struct imgresz_tz_data *imgresz_g_tz_data(const IMGRESZ_TICKET ti)
{
	if (!imgresz_valid_ticket(ti)) {
		WARN_ON(1);
		return NULL;
	} else {
		return imgresz_tz_inst_data[TICKET_TO_HWID(ti)];
	}
}

static void imgresz_tz_data_init(struct imgresz_data *data, struct imgresz_data *data_2nd)
{
	struct imgresz_tz_data *data_tz = NULL, *data_tz_2nd = NULL;

	data_tz = imgresz_g_tz_data(TICKET_FROM_HWID(data->hw_id));
	data_tz->hw_id = data->hw_id;
	data_tz->state = data->state;
	data_tz->scale_data = data->scale_data;

	if (data_2nd != NULL) {
		data_tz_2nd = imgresz_g_tz_data(TICKET_FROM_HWID(data_2nd->hw_id));
		data_tz_2nd->hw_id = data_2nd->hw_id;
		data_tz_2nd->state = data_2nd->state;
		data_tz_2nd->scale_data = data_2nd->scale_data;
	}
}
#endif

static void imgresz_scale_data_init(struct imgresz_scale_data *data)
{
	memset(data, 0, sizeof(*data));
	data->jpg_info.y_exist = true;
	data->jpg_info.cb_exist = true;
	data->jpg_info.cr_exist = true;
	data->outstanding = true;
}

/* CLK info always saved in HW0-data.*/
int imgresz_clk_enable(struct imgresz_data *data)
{
	int ret = 0;

	ret = clk_prepare(data->reszclk);
	if (ret != 0)
		return ret;

	return clk_enable(data->reszclk);
}

/* CLK info always saved in HW0-data.*/
void imgresz_clk_disable(struct imgresz_data *data)
{
	clk_disable(data->reszclk);
	clk_unprepare(data->reszclk);
}

IMGRESZ_TICKET imgresz_ticket_get_8695(enum imgresz_ticket_fun_type type)
{
	struct imgresz_data *data, *data_2nd;
	unsigned long flags;
	int tryhwid, i, ret;
	spinlock_t *state_lock;

	/* only use hw0's state lock*/
	state_lock = &(imgresz_get_data(TICKET_FROM_HWID(0))->state_lock);
	if (type == IMGRESZ_FUN_UFO_2HW)
		tryhwid = BIT(0); /* Fix use hw0 and hw1.*/
	else
		tryhwid = 0xf;
	spin_lock_irqsave(state_lock, flags);
	for (i = 0; i < imgresz_hw_num; i++) {
		if (BIT(i) & tryhwid) {
			data = imgresz_get_data(TICKET_FROM_HWID(i));
			WARN_ON(!data);
			if (data->state == IMGRESZ_STATE_IDLE) {
				/* someone get the resource, so it is busy now */
				data->state = IMGRESZ_STATE_BUSY;
				imgresz_scale_data_init(&data->scale_data);
				data->scale_data.funtype = type;
				data->time_start = 0;
				data->time_end = 0;
				/* 2ufoHW case. Fix HW1 */
				if (type == IMGRESZ_FUN_UFO_2HW || type == IMGRESZ_FUN_ONEPHASE_2HW) {
					data_2nd = imgresz_get_data(TICKET_FROM_HWID(1));
					if (data_2nd->state == IMGRESZ_STATE_IDLE) {
						data_2nd->state = IMGRESZ_STATE_BUSY;
						imgresz_scale_data_init(&data_2nd->scale_data);
						data_2nd->scale_data.funtype = type;
						data_2nd->time_start = 0;
						data_2nd->time_end = 0;
						break;
					}
					/* hw1 busy, revert hw0*/
					data->state = IMGRESZ_STATE_IDLE;
					i = 10; /* Invalid */
				}
				break;
			}
		}
	}
	spin_unlock_irqrestore(state_lock, flags);

	if (i <= imgresz_hw_num - 1) {
		if (type == IMGRESZ_FUN_UFO_2HW) {
			data->scale_data.ufo_h_partition = true;
			data->scale_data.ufo_page0 = true;
		} else if (type == IMGRESZ_FUN_ONEPHASE_2HW) {
			data->scale_data.one_phase = true;
		}
		pm_runtime_get_sync(&data->pdev->dev);
		ret = imgresz_clk_enable(data);
		if (ret) {
			logwarn("clk enable fail %d\n", ret);
			return ret;
		}
		mtk_smi_larb_get(data->smi_larb_dev);
		return TICKET_FROM_HWID(i);
	}
	if (imgresz_log_level & IMGRESZ_LOG_CMD) {
		unsigned int hwstatus[IMGRESZ_MAX_NR], fun[IMGRESZ_MAX_NR];

		spin_lock_irqsave(state_lock, flags);
		for (i = 0; i < imgresz_hw_num; i++) {
			data = imgresz_get_data(TICKET_FROM_HWID(i));
			hwstatus[i] = data->state;
			fun[i] = data->scale_data.funtype;
		}
		spin_unlock_irqrestore(state_lock, flags);
		pr_notice_ratelimited("[Imgresz]HW busy(Req%d), HW0:%u-%u; HW1:%u-%u\n",
				    type, hwstatus[0], fun[0], hwstatus[1], fun[1]);
	}
	return -EBUSY;
}

IMGRESZ_TICKET imgresz_ticket_get_8697(enum imgresz_ticket_fun_type type)
{
	struct imgresz_data *data, *data_2nd;
	unsigned long flags;
	int tryhwid, i;
	bool hw2ufo;
	unsigned int hwstatus[4], fun[4];

	if (type == IMGRESZ_FUN_JPGPIC)
		tryhwid = BIT(0);
	else if (type == IMGRESZ_FUN_UFO)
		tryhwid = BIT(0) | BIT(2) | BIT(3);
	else if (type == IMGRESZ_FUN_UFO_2HW)
		tryhwid = BIT(2); /* Fix use hw2 and hw3.*/
	else
		tryhwid = 0xf;

	hw2ufo = (type == IMGRESZ_FUN_UFO_2HW) ? true : false;

	for (i = 0; i < IMGRESZ_MAX_NR; i++) {
		if (BIT(i) & tryhwid) {
			data = imgresz_get_data(TICKET_FROM_HWID(i));
			WARN_ON(!data);
			if (hw2ufo)/* 2ufoHW case. Fix HW3 */
				data_2nd = imgresz_get_data(TICKET_FROM_HWID(3));

			spin_lock_irqsave(&data->state_lock, flags);
			if (data->state == IMGRESZ_STATE_IDLE) {
				/* someone get the resource, so it is busy now */
				data->state = IMGRESZ_STATE_BUSY;
				imgresz_scale_data_init(&data->scale_data);
				data->scale_data.funtype = type;
				data->time_start = 0;
				data->time_end = 0;

				/* we don't get data_2nd lock */
				if (hw2ufo) {
					if (data_2nd->state == IMGRESZ_STATE_IDLE) {
						data_2nd->state = IMGRESZ_STATE_BUSY;
						imgresz_scale_data_init(&data_2nd->scale_data);
						data_2nd->scale_data.funtype = type;
						data->scale_data.ufo_v_partition = true;
						data->scale_data.ufo_page0 = true;
						data_2nd->time_start = 0;
						data_2nd->time_end = 0;
					} else {
						data->state = IMGRESZ_STATE_IDLE;
						i = 10; /* Invalid */
					}
				}
				spin_unlock_irqrestore(&data->state_lock, flags);
				break;
			}
			spin_unlock_irqrestore(&data->state_lock, flags);
		}
	}

	/* Need add sema to wait the resource. */

	if (i <= IMGRESZ_MAX_NR - 1)
		return TICKET_FROM_HWID(i);

	if (imgresz_log_level & IMGRESZ_LOG_CMD) {
		for (i = 0; i < IMGRESZ_MAX_NR; i++) {
			data = imgresz_get_data(TICKET_FROM_HWID(i));
			spin_lock_irqsave(&data->state_lock, flags);
			hwstatus[i] = data->state;
			fun[i] = data->scale_data.funtype;
			spin_unlock_irqrestore(&data->state_lock, flags);
		}
		pr_notice_ratelimited("[Imgresz]HW busy(Req%d), HW0:%d-%d; HW1:%d-%d,HW2:%d-%d,HW3:%d-%d\n",
				    type, hwstatus[0], fun[0], hwstatus[1], fun[1],
				    hwstatus[2], fun[2], hwstatus[3], fun[3]);
	}
	return -EBUSY;
}


int imgresz_ticket_put_8695(IMGRESZ_TICKET ti)
{
	struct imgresz_data *data, *data2;
	unsigned long flags;
	spinlock_t *state_lock;

	/* only use hw0's state lock*/
	state_lock = &(imgresz_get_data(TICKET_FROM_HWID(0))->state_lock);

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;

	spin_lock_irqsave(state_lock, flags);
	data->state = IMGRESZ_STATE_IDLE;

	/* Release the 3th HW in ufo2hw case.
	 * change the hw3 status in HW2 spinlock as it only check hw2 lock in ticket_get.
	 */
	if (data->scale_data.ufo_h_partition) {
		data2 = imgresz_get_data(TICKET_FROM_HWID(1));
		data2->state = IMGRESZ_STATE_IDLE;
	}
	spin_unlock_irqrestore(state_lock, flags);

	mtk_smi_larb_put(data->smi_larb_dev);
	imgresz_clk_disable(data);
	pm_runtime_put_sync(&data->pdev->dev);

	return 0;
}

int imgresz_ticket_put_8697(IMGRESZ_TICKET ti)
{
	struct imgresz_data *data, *data2;
	unsigned long flags, flags_2;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;

	spin_lock_irqsave(&data->state_lock, flags);
	data->state = IMGRESZ_STATE_IDLE;

	/* Release the 3th HW in ufo2hw case.
	 * change the hw3 status in HW2 spinlock as it only check hw2 lock in ticket_get.
	 */
	if (data->scale_data.ufo_v_partition) {
		data2 = imgresz_get_data(TICKET_FROM_HWID(3));
		spin_lock_irqsave(&data2->state_lock, flags_2);
		data2->state = IMGRESZ_STATE_IDLE;
		spin_unlock_irqrestore(&data2->state_lock, flags_2);
	}
	spin_unlock_irqrestore(&data->state_lock, flags);

	imgresz_clk_disable(data);
	return 0;
}

IMGRESZ_TICKET imgresz_ticket_get(enum imgresz_ticket_fun_type type)
{
	switch (imgresz_cur_chip_ver) {
	case IMGRESZ_CURR_CHIP_VER_8697:
		return imgresz_ticket_get_8697(type);
	case IMGRESZ_CURR_CHIP_VER_8695:
		return imgresz_ticket_get_8695(type);
	default:
		return 0;
	}
}

int imgresz_ticket_put(IMGRESZ_TICKET ti)
{
	switch (imgresz_cur_chip_ver) {
	case IMGRESZ_CURR_CHIP_VER_8697:
		return imgresz_ticket_put_8697(ti);
	case IMGRESZ_CURR_CHIP_VER_8695:
		return imgresz_ticket_put_8695(ti);
	default:
		return 0;
	}
}

int imgresz_set_scale_mode(IMGRESZ_TICKET ti, enum imgresz_scale_mode scalemd)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;
	data->scale_data.scale_mode = scalemd;
	return 0;
}

static int imgresz_src_buf_format_init(struct imgresz_src_buf_info *srcbufinfo,
				       struct imgresz_buf_format *src_format)
{
	switch (srcbufinfo->src_mode) {
	case IMGRESZ_SRC_COL_MD_JPG_DEF:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR;
		src_format->h_sample[0] = srcbufinfo->jpg_comp.y_comp_sample_h;
		src_format->h_sample[1] = srcbufinfo->jpg_comp.cb_comp_sample_h;
		src_format->h_sample[2] = srcbufinfo->jpg_comp.cr_comp_sample_h;
		src_format->v_sample[0] = srcbufinfo->jpg_comp.y_comp_sample_v;
		src_format->v_sample[1] = srcbufinfo->jpg_comp.cb_comp_sample_v;
		src_format->v_sample[2] = srcbufinfo->jpg_comp.cr_comp_sample_v;
		break;
	case IMGRESZ_SRC_COL_MD_420_BLK:
		src_format->block = true;
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		src_format->yuv_format = IMGRESZ_YUV_FORMAT_420;
		break;
	case IMGRESZ_SRC_COL_MD_422_BLK:
		src_format->block = true;
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		src_format->yuv_format = IMGRESZ_YUV_FORMAT_422;
		break;
	case IMGRESZ_SRC_COL_MD_420_RS:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		src_format->yuv_format = IMGRESZ_YUV_FORMAT_420;
		break;
	case IMGRESZ_SRC_COL_MD_422_RS:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		src_format->yuv_format = IMGRESZ_YUV_FORMAT_422;
		break;
	case IMGRESZ_SRC_COL_MD_AYUV:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_AYUV;
		break;
	case IMGRESZ_SRC_COL_MD_ARGB_8888:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		src_format->argb_format = IMGRESZ_ARGB_FORMAT_8888;
		break;
	case IMGRESZ_SRC_COL_MD_ARGB_1555:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		src_format->argb_format = IMGRESZ_ARGB_FORMAT_1555;
		break;
	case IMGRESZ_SRC_COL_MD_RGB_565:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		src_format->argb_format = IMGRESZ_ARGB_FORMAT_0565;
		break;
	case IMGRESZ_SRC_COL_MD_ARGB_4444:
		src_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		src_format->argb_format = IMGRESZ_ARGB_FORMAT_4444;
		break;
	default:
		return -EINVAL;
	}

	src_format->progressive = !srcbufinfo->interlaced;
	src_format->top_field = srcbufinfo->topfield;
	src_format->jump_10bit = srcbufinfo->ufo_jump;
	return 0;
}

static int imgresz_dst_buf_format_init(struct imgresz_dst_buf_info *dstbufinfo,
				       struct imgresz_buf_format *dst_format)
{
	switch (dstbufinfo->dst_mode) {
	case IMGRESZ_DST_COL_MD_420_BLK:
		dst_format->block = true;
	case IMGRESZ_DST_COL_MD_420_RS:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		dst_format->yuv_format = IMGRESZ_YUV_FORMAT_420;
		break;
	case IMGRESZ_DST_COL_MD_422_BLK:
		dst_format->block = true;
	case IMGRESZ_DST_COL_MD_422_RS:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_Y_C;
		dst_format->yuv_format = IMGRESZ_YUV_FORMAT_422;
		break;
	case IMGRESZ_DST_COL_MD_AYUV:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_AYUV;
		break;
	case IMGRESZ_DST_COL_MD_ARGB_8888:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		dst_format->argb_format = IMGRESZ_ARGB_FORMAT_8888;
		break;
	case IMGRESZ_DST_COL_MD_ARGB_1555:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		dst_format->argb_format = IMGRESZ_ARGB_FORMAT_1555;
		break;
	case IMGRESZ_DST_COL_MD_RGB_565:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		dst_format->argb_format = IMGRESZ_ARGB_FORMAT_0565;
		break;
	case IMGRESZ_DST_COL_MD_ARGB_4444:
		dst_format->mainformat = IMGRESZ_BUF_MAIN_FORMAT_ARGB;
		dst_format->argb_format = IMGRESZ_ARGB_FORMAT_4444;
		break;
	default:
		return -EINVAL;
	}
	dst_format->bit10 = dstbufinfo->bit10;
	return 0;
}

static bool imgresz_src_is_ufo(const enum IMGRESZ_UFO_TYPE ufotype)
{
	if (ufotype >= IMGRESZ_UFO_8BIT &&
	    ufotype <= IMGRESZ_UFO_10BIT_COMPACT_UNCOMPRESS)
		return true;
	else
		return false;
}

int
imgresz_set_src_bufinfo(IMGRESZ_TICKET ti, struct imgresz_src_buf_info *srcbuf)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;
	memcpy(&data->scale_data.src_buf, srcbuf, sizeof(*srcbuf));
	if (srcbuf->buf_info.mem_type == IMGRZ_MEM_PHY || srcbuf->buf_info.mem_type == IMGRZ_MEM_MVA) {
		/* All the address must align 16bytes. */
		WARN_ON_ONCE(!!(srcbuf->buf_info.y_offset & 0xf));
		WARN_ON_ONCE(!!(srcbuf->buf_info.cb_offset & 0xf));
		WARN_ON_ONCE(!!(srcbuf->buf_info.cr_offset & 0xf));
		data->scale_data.src_buf.y_buf_addr = srcbuf->buf_info.y_offset;
		data->scale_data.src_buf.cb_buf_addr = srcbuf->buf_info.cb_offset;
		data->scale_data.src_buf.cr_buf_addr = srcbuf->buf_info.cr_offset;
		data->scale_data.src_buf.ufo_ylen_buf = srcbuf->buf_info.ylen_offset;
		data->scale_data.src_buf.ufo_clen_buf = srcbuf->buf_info.clen_offset;
	}

	/* UFO2HW must be enabled while src is ufo and src must be >= FHD. */
	#if 0
	if (data->scale_data.funtype == IMGRESZ_FUN_UFO_2HW &&
	    (!imgresz_src_is_ufo(srcbuf->ufo_type) || srcbuf->pic_width * srcbuf->pic_height < 2073600)) {
		logwarn("UFO2HW only support >= FHD\n");
		return -EINVAL;
	}
	#endif
	return imgresz_src_buf_format_init(srcbuf, &data->scale_data.src_format);
}

int
imgresz_set_dst_bufinfo(IMGRESZ_TICKET ti, struct imgresz_dst_buf_info *dstbuf)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;


	memcpy(&data->scale_data.dst_buf, dstbuf, sizeof(*dstbuf));
	if (dstbuf->buf_info.mem_type == IMGRZ_MEM_PHY || dstbuf->buf_info.mem_type == IMGRZ_MEM_MVA) {
		WARN_ON_ONCE(!!(dstbuf->buf_info.y_offset & 0xf));
		WARN_ON_ONCE(!!(dstbuf->buf_info.cb_offset & 0xf));
		data->scale_data.dst_buf.y_buf_addr = dstbuf->buf_info.y_offset;
		data->scale_data.dst_buf.c_buf_addr = dstbuf->buf_info.cb_offset;
	}
	return imgresz_dst_buf_format_init(dstbuf, &data->scale_data.dst_format);
}

int imgresz_set_jpeg_info(IMGRESZ_TICKET ti, struct imgresz_jpg_info *jpeginfo)
{
	return 0;
}

int imgresz_set_rm_info(IMGRESZ_TICKET ti, struct imgresz_rm_info *rm_info)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;
	memcpy(&data->scale_data.rm_info, rm_info, sizeof(*rm_info));
	return 0;
}

int imgresz_set_partial_info(IMGRESZ_TICKET ti,
			     struct imgresz_partial_buf_info *partialbuf)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;
	memcpy(&data->scale_data.partial_info, partialbuf, sizeof(*partialbuf));
	return 0;
}

int imgresz_legacy_iommu_enable(IMGRESZ_TICKET ti, phys_addr_t pgtablebase,
				bool rd_iommu_en, bool wr_iommu_en)
{
	return 0;
}

int imgresz_set_callback_fun(IMGRESZ_TICKET ti, imgresz_callback_fun fun,
			     void *privdata)
{
	struct imgresz_data *data;

	data = imgresz_get_data(ti);
	data->cb_info.fun_cb = fun;
	data->cb_info.cb_data = privdata;
	return 0;
}

#define fgsrc_hei_8_is_even(hei) (!((hei) & 1) && (!!((hei) & 0x8)) == (!!((hei) & 0x7)))
#if 0
/* vertical partition.
 *---------------
 *|             |
 *|             |
 *|             |
 *| ----------- | a
 *| ----------- | b
 *|             |
 *|             |
 *---------------
 *a: part1_src_begin(must be 128align).
 *b: part0_src_end.
 *Currently there are only A-part0 and B-part1 in the src.
 *Dst is the same.
 */
static int imgresz_ufo_v_partition(struct imgresz_data *data)
{
	struct imgresz_scale_data *scaledata = &data->scale_data;
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *src_format = &scaledata->src_format;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	void __iomem *base = data->base;
	unsigned int part0_src_end = 0, part1_src_bgn = 0;
	unsigned int part0_dst_end = 0, part1_dst_bgn = 0;
	unsigned int xfactor = 0;
	bool linebuflenok, c_lenoffsetok, dst_blk, osd_mode;

	osd_mode = (dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB ||
		dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ? true : false;
	dst_blk = scaledata->dst_format.block;
	part0_src_end = src_buf->pic_height / 2;
	part1_src_bgn = src_buf->pic_height / 2;

	xfactor = imgresz_hal_coeff_get_v4_factor(data->base);
	if (!part1_src_bgn || !xfactor)
		return -EINVAL;

	/* part0 src height/8 need to be even. */
	part0_dst_end = ((part0_src_end + 1) << 18) / xfactor;
	while (!fgsrc_hei_8_is_even(part0_src_end) || (part0_dst_end & 0x1)) {
		part0_src_end = (part0_src_end + 8) / 8 * 8;/* 1080->1088 */

		part0_dst_end = (xfactor == 0x40000) ? part0_src_end : (part0_src_end + 1);
		part0_dst_end = (part0_dst_end << 18) / xfactor;

		if (part0_src_end >= src_buf->pic_height)
			return -EPERM;
	}

	do {
		if (part1_src_bgn <= 32)
			break;
		part1_src_bgn -= 32;
		part1_src_bgn &= ~31; /* align to 32. */

		part1_dst_bgn = (xfactor == 0x40000) ? part1_src_bgn : (part1_src_bgn + 1);
		part1_dst_bgn = (part1_dst_bgn << 18) / xfactor;

		/* whether the linebuf is 0. this is not allowed. */
		linebuflenok = imgresz_hal_survey_linebuflen_is_ok(true, false, dst_blk,
							src_buf->pic_width, dst_buf->pic_width,	16);

		/* If linebuf is not OK, we have to adjust the part1_src_bgn. */
		if (!linebuflenok)
			linebuflenok = fgsrc_hei_8_is_even(src_buf->pic_height - part1_src_bgn);

		/* C len offset, should 16 bytes align. */
		c_lenoffsetok = !((src_buf->buf_width * part1_src_bgn / 4 / 64 / 2) & 0xf);

		/*
		 * loginfo("try part1bgn %d part1dst %d linebufok %d c_lenoffsetok %d\n",
		 *	   part1_src_bgn, part1_dst_bgn, linebuflenok, c_lenoffsetok);
		 */
	} while ((part1_dst_bgn & 0x1) || /* dst begin can not be odd. */
			     !linebuflenok || !c_lenoffsetok);

	if (part0_src_end >= src_buf->pic_height || part1_src_bgn >= src_buf->pic_height ||
	    part0_dst_end >= dst_buf->pic_height || part1_dst_bgn >= dst_buf->pic_height) {
		logwarn("2part fail-hei, src:0x%zx(0x%x-0x%x),dst:0x%zx(0x%x-0x%x)\n",
			src_buf->pic_height, part0_src_end, part1_src_bgn,
			dst_buf->pic_height, part0_dst_end, part1_dst_bgn);
		return -EPERM;
	}

	if (part0_src_end < part1_src_bgn || part0_dst_end < part1_dst_bgn) {
		logwarn("2part fail, src:(0x%x-0x%x),dst:(0x%x-0x%x)\n",
			part0_src_end, part1_src_bgn, part0_dst_end, part1_dst_bgn);
		return -EPERM;
	}

	if (scaledata->ufo_page0) { /* the top part. */
		loginfo(IMGRESZ_LOG_UFO,
			"Partition%d V-factor 0x%x:part0 src(0-%d)dst(0-%d); part1 src(%d-%zu)dst(%d(even)-%zu);result %d\n",
			!scaledata->ufo_page0, xfactor,
			part0_src_end, part0_dst_end,
			part1_src_bgn, src_buf->pic_height,
			part1_dst_bgn, dst_buf->pic_height,
			(((src_buf->pic_height + 1) << 18) / xfactor == dst_buf->pic_height));

		if (part1_src_bgn <= 32) {
			logwarn("part1srcbgn %d linebufok %d, C_lenoffset %d\n",
				part1_dst_bgn, linebuflenok, c_lenoffsetok);
		}

		imgresz_hal_set_src_pic_wid_hei(base, src_buf->pic_width, part0_src_end, src_format);
		imgresz_hal_set_dst_pic_wid_hei(base, dst_buf->pic_width, part0_dst_end);

		imgresz_ufo_pagesz(base, src_buf->pic_width, part0_src_end, dst_buf->pic_width);

		imgresz_hal_set_linebuflen(base, scaledata->v_method,
					   src_buf->pic_width, part0_src_end,
					   dst_buf->pic_width, part0_dst_end,
					   true, 0, scaledata->one_phase,
					   osd_mode, dst_blk, scaledata->dst_format.bit10);
	} else { /* the bottom part. */

		imgresz_hal_set_linebuflen(base, scaledata->v_method,
					   src_buf->pic_width, src_buf->pic_height - part1_src_bgn,
					   dst_buf->pic_width, dst_buf->pic_height - part1_dst_bgn,
					   true, 0, scaledata->one_phase,
					   osd_mode, dst_blk, scaledata->dst_format.bit10);
		imgresz_hal_coeff_v4tap_vdo_partition_offset(
				base, part1_dst_bgn * xfactor);
		imgresz_hal_set_src_pic_wid_hei(base, src_buf->pic_width,
						src_buf->pic_height - part1_src_bgn,
						src_format);
		imgresz_hal_set_dst_pic_wid_hei(base, dst_buf->pic_width,
					       dst_buf->pic_height - part1_dst_bgn);
		imgresz_hal_set_dst_pic_offset(base, 0, part1_dst_bgn);
		imgresz_ufo_vdo_v_partition_offset_update(base, src_buf->buf_width,
							  part1_src_bgn);
		imgresz_ufo_pagesz(base, src_buf->pic_width,
				   src_buf->pic_height - part1_src_bgn,
				   dst_buf->pic_width);
	}
	return 0;
}

/*
*just h clip to 2 parts
	_ _ _ _ _
	|   |   |
	|   |   |
	|   |   |
	|_ _|_ _|
	use target x to calculate src x
*/
static int imgresz_ufo_h_partition_2hw(struct imgresz_data *data)
{
	struct imgresz_scale_data *scaledata = &data->scale_data;
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *src_format = &scaledata->src_format;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	void __iomem *base = data->base;
	unsigned int hfactor_y, hfactor_c;
	unsigned int x1, x1_c;
	unsigned int x0_end, x0_c_end;
	unsigned int tg_x1, u4Count;
	bool fgLinebufok;
	bool dst_blk = dst_format->block;
	bool osd_mode = (dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB ||
		dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ? true : false;

	tg_x1 = IMGALIGN(dst_buf->pic_width/2, 32);
	hfactor_y = imgresz_hal_coeff_get_h8_y(data->base);
	hfactor_c = imgresz_hal_coeff_get_h8_cb(data->base);
	if (!tg_x1 || !hfactor_y || !hfactor_c)
		return -EINVAL;

	if ((src_buf->pic_height == dst_buf->pic_height) && (src_buf->pic_width == dst_buf->pic_width)) {
		/*same raio just use x/y 32 align to get point*/
		x1 = x0_end = tg_x1;
		x1_c = x0_c_end = x1/2;
	} else {/*page0*/
		u4Count = 0;
		do {
			tg_x1 = IMGALIGN(dst_buf->pic_width/2 - u4Count*32, 32);
			x0_end = IMG_VdoPart_GetSrcW(tg_x1, hfactor_y);
			x0_c_end = IMG_VdoPart_GetSrcW(tg_x1/2, hfactor_c);

			/* whether the linebuf is 0. this is not allowed. */
			fgLinebufok = imgresz_hal_survey_linebuflen_is_ok(true, false, dst_blk, x0_end, tg_x1, 16);
			if (!fgLinebufok) {
				loginfo(IMGRESZ_LOG_UFO, "fgLinebufok:%d,tg_x1:%d x0_end:%d,u4Count:%d\n",
					fgLinebufok, tg_x1, x0_end, u4Count);
				u4Count++;
			}
			if (tg_x1 < 32) {
				loginfo(IMGRESZ_LOG_UFO, "page0 < 32,break\n");
				return -EINVAL;
			}
		} while (!fgLinebufok && (tg_x1 > 4));

		/* page1 */
		x1 = IMG_VdoPart_GetSrcPoint(tg_x1, hfactor_y);
		x1_c = IMG_VdoPart_GetSrcPoint(tg_x1/2, hfactor_c);
	}

	if (scaledata->ufo_page0) {
		imgresz_hal_set_src_pic_wid_hei(base, x0_end, src_buf->pic_height, src_format);
		imgresz_hal_set_dst_pic_wid_hei(base, tg_x1, dst_buf->pic_height);
		imgresz_ufo_pagesz(base, x0_end, src_buf->pic_height, tg_x1);
		imgresz_hal_coeff_h8tap_vdo_partition_offset(base, 0, 0, 0);
		imgresz_hal_set_linebuflen(base, scaledata->v_method,
						   x0_end, src_buf->pic_height,
						   tg_x1, dst_buf->pic_height,
						   true, 0, scaledata->one_phase, osd_mode, dst_blk, dst_format->bit10);
	} else { /* part1 */
		unsigned int cur_src_wid;

		/* 2*c_wid may > y_wid in clip. */
		if ((src_buf->pic_width - x1) >= 2 * (src_buf->pic_width / 2 - x1_c))
			cur_src_wid = src_buf->pic_width - x1;
		else
			cur_src_wid = 2 * (src_buf->pic_width / 2 - x1_c);

		loginfo(IMGRESZ_LOG_UFO,
			"Partitionhw%u-%d Hfactor %u,%u: p0 src(0-%u) p1 src(%u(%u)-%zu)%u dst(%u-%zu)\n",
			data->hw_id, scaledata->ufo_page0, hfactor_y, hfactor_c,
			x0_end, x1, x1_c, src_buf->pic_width, cur_src_wid, tg_x1, dst_buf->pic_width);

		imgresz_hal_set_linebuflen(base, scaledata->v_method,
						   cur_src_wid, src_buf->pic_height,
						   dst_buf->pic_width - tg_x1, dst_buf->pic_height,
						   true, 0, scaledata->one_phase,
						   osd_mode, dst_blk, dst_format->bit10);
		imgresz_hal_coeff_h8tap_vdo_partition_offset(base, tg_x1, hfactor_y, hfactor_c);
		imgresz_hal_coeff_v4tap_vdo_partition_offset(base, 0);
		imgresz_hal_set_src_pic_wid_hei_vdo_partition(base, src_buf->pic_width - x1,
					src_buf->pic_width/2 - x1_c, src_buf->pic_height);
		imgresz_hal_set_dst_pic_wid_hei(base, dst_buf->pic_width - tg_x1, dst_buf->pic_height);
		imgresz_hal_set_dst_pic_offset(base, tg_x1, 0);
		imgresz_ufo_partition_set_start_point(base, x1, 0);
		imgresz_hal_set_src_offset_vdo_partition(base, x1, 0, x1_c);
		imgresz_ufo_pagesz(base, cur_src_wid, src_buf->pic_height, dst_buf->pic_width - tg_x1);
	}

	if ((src_buf->pic_height == dst_buf->pic_height) && (src_buf->pic_width == src_buf->pic_width))
		imgresz_hal_coeff_h8tap_vdo_partition_offset(base, 0, 0, 0);

	if (imgresz_log_level & IMGRESZ_LOG_SHOWREG)
		imgresz_debug_vdo_partition_setting(base);

	return 0;
}
#endif

static bool imgresz_ufo_is_support(unsigned int hw_id)
{
	/* HW1 don't support UFO. */
	if (hw_id >= IMGRESZ_MAX_NR ||
		(hw_id == 1 && imgresz_cur_chip_ver == IMGRESZ_CURR_CHIP_VER_8697))
		return false;
	else
		return true;
}

static int _imgresz_hal_if_do_scale(struct imgresz_data *data)
{
	struct imgresz_scale_data *scaledata = &data->scale_data;
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *src_format = &scaledata->src_format;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	struct imgresz_hal_info   *hal_info = &scaledata->hal_info;
	union imgresz_partition_info *partition_info = &scaledata->partition_info;
	struct imgresz_h_partition_info *h_partition_info = &partition_info->h_partition_info;
	struct imgresz_rm_info *rm_info = &scaledata->rm_info;
	struct imgresz_jpg_info *jpg = &scaledata->jpg_info;
	void __iomem *base = data->base;
	bool h_partition = scaledata->ufo_h_partition;
	bool jpg_picmode = (scaledata->scale_mode == IMGRESZ_JPEG_PIC_SCALE);
	int ret = 0;

	/* 0. Set RM mode to HW */
	if (rm_info->rpr_mode)
		imgresz_hal_set_src_rm_rpr(base, true, rm_info->racing_mode);
	else
		imgresz_hal_set_src_rm_rpr(base, false, false);

	/* 1. Set Resize Mode to HW */
	imgresz_hal_set_resz_mode(base, scaledata->scale_mode, src_format->mainformat);

	/* 2. Set Resample Method to HW */
	imgresz_hal_set_resample_method(base, scaledata->h_method, scaledata->v_method);

	/*
	 * Ufo info setting.
	 * UFO setting should be before normal source buffer setting as ufo_full
	 * has its specail src buffer address.
*/
	if (imgresz_src_is_ufo(src_buf->ufo_type)) {
		if (!imgresz_ufo_is_support(data->hw_id))
			return -EPERM;

		imgresz_ufo_poweron(base);
		imgresz_ufo_config(base, src_buf->ufo_type);
		imgresz_ufo_pagesz(base,
			h_partition ? h_partition_info->src_w : src_buf->pic_width,
			src_buf->pic_height,
			h_partition ? h_partition_info->dst_w : dst_buf->pic_width);
		imgresz_ufo_picsz(base, src_buf->buf_width, src_buf->buf_height);
		imgresz_ufo_src_buf(base, src_buf->y_buf_addr, src_buf->cb_buf_addr);

		if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695) {
			imgresz_ufo_idle_int_on(base);
			if (dst_format->bit10)
				imgresz_ufo_10bit_output_enable(base);
			if (src_format->jump_10bit)
				imgresz_ufo_10bit_jump_mode_enable(base);
			if (scaledata->outstanding)
				imgresz_ufo_outstanding_enable(base);
		}
		if (src_buf->ufo_type != IMGRESZ_UFO_10BIT_COMPACT_UNCOMPRESS)
			imgresz_ufo_srclen_buf(base, src_buf->ufo_ylen_buf,
					       src_buf->ufo_clen_buf);
		imgresz_ufo_partition_set_start_point(base, src_buf->pic_x_offset, src_buf->pic_y_offset);
	}


	/* 3. Set Source Information to HW */
	imgresz_hal_set_src_buf_format(base, src_format);
	if (scaledata->scale_mode == IMGRESZ_FRAME_SCALE) {
		/* UFO has its owner buffer address register. */
		if (!imgresz_src_is_ufo(src_buf->ufo_type))
			imgresz_hal_set_src_buf_addr(base, src_buf->y_buf_addr,
						     src_buf->cb_buf_addr,
						     src_buf->cr_buf_addr);

		if (rm_info->racing_mode)
			src_buf->buf_height = 0x20;
		imgresz_hal_set_src_rowbuf_height(
				base, src_buf->buf_height, src_format,
				rm_info->racing_mode);
	} else if (scaledata->scale_mode == IMGRESZ_PARTIAL_SCALE) {
		/* PARTIAL MODE */
	}

	imgresz_hal_set_src_buf_pitch(base, src_buf->buf_width, src_format);

	if (h_partition && !scaledata->ufo_page0) {
		imgresz_hal_set_src_pic_wid_hei_vdo_partition(base,
			src_buf->pic_width - h_partition_info->src_x_offset,
			src_buf->pic_width/2 - h_partition_info->src_x_offset_c,
			src_buf->pic_height);
		imgresz_hal_set_src_offset_vdo_partition(base,
			h_partition_info->src_x_offset, 0,
			h_partition_info->src_x_offset_c);
	} else {
		imgresz_hal_set_src_pic_wid_hei(base, h_partition ?
			h_partition_info->src_w : src_buf->pic_width,
			src_buf->pic_height, src_format);
		imgresz_hal_set_src_pic_offset(base, src_buf->pic_x_offset,
			src_buf->pic_y_offset, src_format);
	}

	if (scaledata->scale_mode == IMGRESZ_FRAME_SCALE) {
		imgresz_hal_set_src_first_row(base, true);

		if (rm_info->racing_mode)
			imgresz_hal_set_src_last_row(base, false);
		else
			imgresz_hal_set_src_last_row(base, true);
	} else if (scaledata->scale_mode == IMGRESZ_PARTIAL_SCALE) {
		/* PARTIAL MODE */
	}

	/* 4. Set Destination Information to HW */
	imgresz_hal_set_dst_buf_format(base, src_format, dst_format);
	imgresz_hal_set_dst_buf_addr(base, dst_buf->y_buf_addr,
				     dst_buf->c_buf_addr);
	imgresz_hal_set_dst_buf_pitch(base,
		dst_format->bit10 ? dst_buf->buf_width*4/5 : dst_buf->buf_width);
	imgresz_hal_set_dst_pic_wid_hei(base,
		h_partition ? h_partition_info->dst_w : dst_buf->pic_width,
		dst_buf->pic_height);
	imgresz_hal_set_dst_pic_offset(base, h_partition ?
		h_partition_info->dst_x_offset + dst_buf->pic_x_offset : dst_buf->pic_x_offset,
		dst_buf->pic_y_offset);
	imgresz_hal_set_vdo_cbcr_swap(base, dst_buf->cbcr_swap);

	/* 6. Set Scale Factor to HW */
	if (rm_info->rpr_mode) {
		imgresz_hal_coeff_set_rpr_H_factor(base, src_buf->pic_width,
						   dst_buf->pic_width);
		imgresz_hal_coeff_set_rpr_V_factor(base, src_buf->pic_height,
						   dst_buf->pic_height);
	} else {
		imgresz_hal_coeff_set_h_factor(base, src_buf, dst_buf, src_format,
				scaledata->h_method, hal_info);
		imgresz_hal_coeff_set_v_factor(base, src_buf, dst_buf, src_format,
				scaledata->v_method, hal_info);
	}

	imgresz_hal_set_linebuflen(
			base, scaledata->v_method,
			h_partition ? h_partition_info->src_w : src_buf->pic_width,
			src_buf->pic_height,
			h_partition ? h_partition_info->dst_w : dst_buf->pic_width,
			dst_buf->pic_height,
			imgresz_src_is_ufo(src_buf->ufo_type),
			rm_info->rpr_mode,
			scaledata->one_phase,
			(dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB ||
			 dst_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV),
			 dst_format->block,
			 dst_format->bit10);

	/* Partition adjust, should be behind src/dst basic info. */
	/*
	if (scaledata->ufo_v_partition && imgresz_src_is_ufo(src_buf->ufo_type))
		ret = imgresz_ufo_v_partition(data);
	else if (scaledata->ufo_h_partition && imgresz_src_is_ufo(src_buf->ufo_type))
		ret = imgresz_ufo_h_partition_2hw(data);
	*/
	if (h_partition && imgresz_src_is_ufo(src_buf->ufo_type)) {
		if (scaledata->ufo_page0)
			imgresz_hal_coeff_h8tap_vdo_partition_offset(base, 0, 0, 0);
		else {
			imgresz_hal_coeff_h8tap_vdo_partition_offset(base,
				h_partition_info->dst_x_offset,
				hal_info->h8_factor_y, hal_info->h8_factor_cb);
			/*imgresz_hal_coeff_v4tap_vdo_partition_offset(base, 0);*/
			imgresz_ufo_partition_set_start_point(base,	h_partition_info->src_x_offset, 0);
		}
		if ((src_buf->pic_height == dst_buf->pic_height) && (src_buf->pic_width == dst_buf->pic_width))
		imgresz_hal_coeff_h8tap_vdo_partition_offset(base, 0, 0, 0);
	}
	/* 7. Jpeg info setting */
	if (scaledata->scale_mode == IMGRESZ_PARTIAL_SCALE || rm_info->rpr_mode)
		imgresz_hal_set_tempbuf(base, scaledata->partial_info.temp_buf_addr);

	if (jpg_picmode)
		imgresz_hal_jpg_picmode_enable(base);
	if (jpg->preload)
		imgresz_hal_jpg_preload_enable(base);

	if (rm_info->racing_mode) {
		imgresz_hal_jpg_component(base, true, true, true);
		imgresz_hal_jpg_component_ex(base, true, true, true, true);
	} else {
		imgresz_hal_jpg_component(base, jpg->y_exist, jpg->cb_exist, jpg->cr_exist);

		if (jpg_picmode || src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR)
			imgresz_hal_jpg_component_ex(base, jpg->y_exist, jpg->cb_exist, jpg->cr_exist, false);
		imgresz_hal_jpg_cbcr_pad(base, jpg->cb_exist, jpg->cr_exist);
	}

	if (jpg->preload) {
		/* preload buffer also support v7 short(32bit) iommu.
		 * currently it's always disable.
		 */
		imgresz_hal_jpg_preload_buf(base, dst_buf->y_buf_addr,
					    dst_buf->c_buf_addr);
	}

	/* 8. Miscellaneous setting */
	imgresz_hal_burst_enable(base, !dst_format->block,
		(!src_format->block) && (!imgresz_src_is_ufo(src_buf->ufo_type)));

	imgresz_hal_set_scaling_type(base, 0);
	/* End FrameMode */

	if (imgresz_log_level & IMGRESZ_LOG_SHOWREG) {
		loginfo(IMGRESZ_LOG_SHOWREG, "Begin to trigger hw: %d\n", data->hw_id);
		imgresz_hal_print_reg(base);
	}

	/* 9. Trigger HW */
	imgresz_hal_trigger_hw(base, imgresz_src_is_ufo(src_buf->ufo_type));

	return ret;
}

static void imgresz_decide_resample_method(struct imgresz_scale_data *scaledata)
{
	if (scaledata->scale_mode == IMGRESZ_FRAME_SCALE) {
		scaledata->h_method = IMGRESZ_RESAMPLE_METHOD_8_TAP;
		scaledata->v_method = IMGRESZ_RESAMPLE_METHOD_4_TAP;
	} else {/* Partial mode should use M tap. */
		scaledata->h_method = IMGRESZ_RESAMPLE_METHOD_8_TAP;
		scaledata->v_method = IMGRESZ_RESAMPLE_METHOD_M_TAP;
	}
}

static int imgresz_cal_h_factor(struct imgresz_scale_data *scaledata)
{
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *src_format = &scaledata->src_format;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	struct imgresz_hal_info *hal_info = &scaledata->hal_info;
	enum IMGRESZ_RESAMPLE_METHOD_T resample_method = scaledata->h_method;
	unsigned int src_width_y = 0, src_width_cb = 0, src_width_cr = 0;
	unsigned int dst_width_y = 0, dst_width_cb = 0, dst_width_cr = 0;
	unsigned int src_width = src_buf->pic_width;
	unsigned int dst_width = dst_buf->pic_width;
	unsigned int factor;

	switch (src_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		switch (src_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
		case IMGRESZ_YUV_FORMAT_422:
			src_width_y = src_width;
			src_width_cb = src_width >> 1;
			src_width_cr = src_width >> 1;
			break;
		case IMGRESZ_YUV_FORMAT_444:
			src_width_y = src_width;
			src_width_cb = src_width;
			src_width_cr = src_width;
			break;
		default:
			src_width_y = src_width;
			src_width_cb = src_width >> 1;
			src_width_cr = src_width >> 1;
			break;
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		src_width_y = src_width;
		src_width_cb = src_width * src_format->h_sample[1] /
				src_format->h_sample[0];
		src_width_cr = src_width * src_format->h_sample[2] /
				src_format->h_sample[0];
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		src_width_y = src_width;
		break;
	default:
		break;
	}

	switch (dst_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		switch (dst_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
		case IMGRESZ_YUV_FORMAT_422:
			dst_width_y = dst_width;
			dst_width_cb = dst_width >> 1;
			dst_width_cr = dst_width >> 1;
			break;
		case IMGRESZ_YUV_FORMAT_444:
			dst_width_y = dst_width;
			dst_width_cb = dst_width;
			dst_width_cr = dst_width;
			break;
		default:
			break;
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		dst_width_y = dst_width;
		dst_width_cb = dst_width;
		dst_width_cr = dst_width;
		break;
	default:
		break;
	}

	/* set default value */
	hal_info->h8_factor_y = 0x40000;
	hal_info->h8_factor_cb = 0x40000;
	hal_info->h8_factor_cr = 0x40000;
	hal_info->h8_offset_y = 0;
	hal_info->h8_offset_cb = 0;
	hal_info->h8_offset_cr = 0;
	hal_info->hsa_factor_y = 0x800;
	hal_info->hsa_factor_cb = 0x800;
	hal_info->hsa_factor_cr = 0x800;
	hal_info->hsa_offset_y = 0;
	hal_info->hsa_offset_cb = 0;
	hal_info->hsa_offset_cr = 0;

	/* Y */
	if ((resample_method == IMGRESZ_RESAMPLE_METHOD_8_TAP) ||
	    (src_width_y <= dst_width_y)) {
		if (src_width_y == dst_width_y)
			factor = (0x40000 * src_width_y + (dst_width_y >> 1))
				   / dst_width_y;
		else
			factor = (0x40000 * (src_width_y - 1) +
				 ((dst_width_y - 1) >> 1)) / (dst_width_y - 1);
		hal_info->h8_factor_y = factor;
	} else {
		factor = (2048 * dst_width_y + (src_width_y >> 1)) / src_width_y;
		hal_info->hsa_factor_y = factor;
		hal_info->hsa_offset_y = 2048 - factor;
	}

	if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
	    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
	    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
		return 0;

	/* Cb */
	if ((resample_method == IMGRESZ_RESAMPLE_METHOD_8_TAP) ||
	    (src_width_cb <= dst_width_cb)) {
		if ((src_width_cb == dst_width_cb) || (dst_width_cb == 1))
			factor = (0x40000 * src_width_cb +
					(dst_width_cb >> 1)) / dst_width_cb;
		else
			factor = (0x40000 * (src_width_cb - 1) +
				((dst_width_cb - 1) >> 1)) / (dst_width_cb - 1);
		hal_info->h8_factor_cb = factor;
	} else {
		factor = (2048 * dst_width_cb + (src_width_cb >> 1)) / src_width_cb;
		hal_info->hsa_factor_cb = factor;
		hal_info->hsa_offset_cb = 2048 - factor;
	}

	if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
		return 0;

	/* Cr */
	if ((resample_method == IMGRESZ_RESAMPLE_METHOD_8_TAP) ||
	    (src_width_cr <= dst_width_cr)) {
		if ((src_width_cr == dst_width_cr) || (dst_width_cb == 1))
			factor = (0x40000 * src_width_cr +
					(dst_width_cr >> 1)) / dst_width_cr;
		else
			factor = (0x40000 * (src_width_cr - 1) +
				((dst_width_cr - 1) >> 1)) / (dst_width_cr - 1);
		hal_info->h8_factor_cr = factor;
	} else {
		factor = (2048 * dst_width_cr + (src_width_cr >> 1)) / src_width_cr;
		hal_info->hsa_factor_cr = factor;
		hal_info->hsa_offset_cr = 2048 - factor;
	}
	return 0;
}

static int imgresz_cal_v_factor(struct imgresz_scale_data *scaledata)
{
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *src_format = &scaledata->src_format;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	struct imgresz_hal_info *hal_info = &scaledata->hal_info;
	enum IMGRESZ_RESAMPLE_METHOD_T resample_method = scaledata->v_method;
	unsigned int src_height_y = 0, src_height_cb = 0, src_height_cr = 0;
	unsigned int dst_height_y = 0, dst_height_cb = 0, dst_height_cr = 0;
	unsigned int src_height = src_buf->pic_height;
	unsigned int dst_height = dst_buf->pic_height;
	unsigned int factor, offset;

	switch (src_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		switch (src_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
			src_height_y = src_height;
			src_height_cb = src_height >> 1;
			src_height_cr = src_height >> 1;
			break;
		case IMGRESZ_YUV_FORMAT_422:
		case IMGRESZ_YUV_FORMAT_444:
			src_height_y = src_height;
			src_height_cb = src_height;
			src_height_cr = src_height;
			break;
		default:
			break;
		}
	break;
	case IMGRESZ_BUF_MAIN_FORMAT_Y_CB_CR:
		src_height_y = src_height;
		src_height_cb = src_height * src_format->v_sample[1] /
				src_format->v_sample[0];
		src_height_cr = src_height * src_format->v_sample[2] /
				src_format->v_sample[0];
		/*
		 * For jpeg picture mode, prevent source height 401 come
		 * two interrupt (Y interrupt and C interrupt)
		 */
		if ((src_height_cb * src_format->v_sample[0]) !=
			(src_height * src_format->v_sample[1]))
			src_height_cb++;
		if ((src_height_cr * src_format->v_sample[0]) !=
			(src_height * src_format->v_sample[2]))
			src_height_cr++;
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		src_height_y = src_height;
		break;
	default:
		break;
	}

	switch (dst_format->mainformat) {
	case IMGRESZ_BUF_MAIN_FORMAT_Y_C:
		switch (dst_format->yuv_format) {
		case IMGRESZ_YUV_FORMAT_420:
			dst_height_y = dst_height;
			dst_height_cb = dst_height >> 1;
			dst_height_cr = dst_height >> 1;
			break;
		case IMGRESZ_YUV_FORMAT_422:
		case IMGRESZ_YUV_FORMAT_444:
			dst_height_y = dst_height;
			dst_height_cb = dst_height;
			dst_height_cr = dst_height;
			break;
		default:
			break;
		}
		break;
	case IMGRESZ_BUF_MAIN_FORMAT_ARGB:
	case IMGRESZ_BUF_MAIN_FORMAT_AYUV:
	case IMGRESZ_BUF_MAIN_FORMAT_INDEX:
		dst_height_y = dst_height;
		dst_height_cb = dst_height;
		dst_height_cr = dst_height;
		break;
	default:
		break;
	}

	switch (resample_method) {
	case IMGRESZ_RESAMPLE_METHOD_4_TAP:
		/* Y */
		hal_info->v4_factor_y = (0x40000 * src_height_y + (dst_height_y >> 1)) /
				dst_height_y;
		hal_info->v4_offset_y = 0;

		if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
			break;
		/* Cb */
		hal_info->v4_factor_cb = (0x40000 * src_height_cb + (dst_height_cb >> 1)) /
				dst_height_cb;
		hal_info->v4_offset_cb = 0;

		if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
			break;
		/* Cr */
		hal_info->v4_factor_cr = (0x40000 * src_height_cr + (dst_height_cr >> 1)) /
				dst_height_cr;
		hal_info->v4_offset_cr = 0;
		break;

	case IMGRESZ_RESAMPLE_METHOD_M_TAP:
		/* Y */
		if (dst_height_y < src_height_y)  {/* scale down */
			factor = (2048 * dst_height_y + (src_height_y >> 1)) /
					src_height_y;
			offset = 2048 - factor;
			hal_info->vm_scale_up_y = false;
		} else {/* scale up, bilinear */
			if (dst_height_y == src_height_y) {
				factor = 0;
				offset = 0;
			} else {
				factor = (2048 * (src_height_y - 1) + ((dst_height_y - 1) >> 1)) / (dst_height_y - 1);
				offset = 0;
			}
			hal_info->vm_scale_up_y = true;
		}
		hal_info->vm_factor_y = factor;
		hal_info->vm_offset_y = offset;

		if ((src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_ARGB) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_AYUV) ||
		    (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_INDEX))
			break;
		/* Cb */
		if (dst_height_cb < src_height_cb) {/* scale dowm */
			factor = (2048 * dst_height_cb +
					(src_height_cb >> 1)) / src_height_cb;
			offset = 2048 - factor;
			hal_info->vm_scale_up_cb = false;
		} else { /* scale up, bilinear */
			if (dst_height_cb == src_height_cb) {
				factor = 0;
				offset = 0;
			} else {
				factor = (2048 * (src_height_cb - 1) +
						((dst_height_cb - 1) >> 1)) /
						(dst_height_cb - 1);
				offset = 0;
			}
			hal_info->vm_scale_up_cb = true;
		}
		hal_info->vm_factor_cb = factor;
		hal_info->vm_offset_cb = offset;

		if (src_format->mainformat == IMGRESZ_BUF_MAIN_FORMAT_Y_C)
			break;
		/* Cr */
		if (dst_height_cr < src_height_cr) { /* scale dowm, source accumulator */
			factor = (2048 * dst_height_cr + (src_height_cr >> 1))
					/ src_height_cr;
			offset = 2048 - factor;
			hal_info->vm_scale_up_cr = false;
		} else {/* scale up, bilinear */
			if (dst_height_cr == src_height_cr) {
				factor = 0;
				offset = 0;
			} else {
				factor = (2048 * src_height_cr +
					  (dst_height_cr >> 1)) / dst_height_cr;
				offset = 0;
			}
			hal_info->vm_scale_up_cr = true;
		}
		hal_info->vm_factor_cr = factor;
		hal_info->vm_offset_cr = offset;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int imgresz_cal_ufo_h_partition_2hw(struct imgresz_scale_data *scaledata,
		struct imgresz_scale_data *scaledata_2nd)
{
	struct imgresz_src_buf_info *src_buf = &scaledata->src_buf;
	struct imgresz_dst_buf_info *dst_buf = &scaledata->dst_buf;
	struct imgresz_buf_format *dst_format = &scaledata->dst_format;
	struct imgresz_hal_info *hal_info = &scaledata->hal_info;
	struct imgresz_h_partition_info *partition0 = &scaledata->partition_info.h_partition_info;
	struct imgresz_h_partition_info *partition1 = &scaledata_2nd->partition_info.h_partition_info;
	unsigned int x1, x1_c;
	unsigned int x0_end, x0_c_end;
	unsigned int tg_x1, u4Count;
	bool fgLinebufok;
	bool dst_blk = dst_format->block;

	tg_x1 = IMGALIGN(dst_buf->pic_width/2, 32);
	if (!tg_x1 || !hal_info->h8_factor_y || !hal_info->h8_factor_cb)
		return -EINVAL;
	if ((src_buf->pic_height == dst_buf->pic_height) &&
		(src_buf->pic_width == dst_buf->pic_width)) {
		/* same raio just use x/y 32 align to get point */
		x1 = x0_end = tg_x1;
		x1_c = x0_c_end = x1/2;
	} else {
		/* page0 */
		u4Count = 0;
		do {
			tg_x1 = IMGALIGN(dst_buf->pic_width/2 - u4Count*32, 32);
			x0_end = IMG_VdoPart_GetSrcW(tg_x1, hal_info->h8_factor_y);
			x0_c_end = IMG_VdoPart_GetSrcW(tg_x1/2, hal_info->h8_factor_cb);

			/* whether the linebuf is 0. this is not allowed. */
			fgLinebufok = imgresz_hal_survey_linebuflen_is_ok(true, false, dst_blk, x0_end, tg_x1, 16);
			if (!fgLinebufok) {
				loginfo(IMGRESZ_LOG_UFO, "fgLinebufok:%d,tg_x1:%d x0_end:%d,u4Count:%d\n",
					fgLinebufok, tg_x1, x0_end, u4Count);
				u4Count++;
			}
			if (tg_x1 < 32) {
				loginfo(IMGRESZ_LOG_UFO, "page0 < 32,break\n");
				return -EINVAL;
			}
		} while (!fgLinebufok && (tg_x1 > 4));

		/*page1*/
		x1 = IMG_VdoPart_GetSrcPoint(tg_x1, hal_info->h8_factor_y);
		x1_c = IMG_VdoPart_GetSrcPoint(tg_x1/2, hal_info->h8_factor_cb);
	}
	partition0->src_x_offset = 0;
	partition0->src_x_offset_c = 0;
	partition0->src_w = x0_end;
	partition0->dst_x_offset = 0;
	partition0->dst_w = tg_x1;
	partition1->src_x_offset = x1;
	partition1->src_x_offset_c = x1_c;
	partition1->src_w = max(src_buf->pic_width - x1, 2 * (src_buf->pic_width / 2 - x1_c));
	partition1->dst_x_offset = tg_x1;
	partition1->dst_w = dst_buf->pic_width - tg_x1;

	loginfo(IMGRESZ_LOG_UFO, "part0: 0~%d -> 0~%d; part1: %d(c:%d)~%d -> %d~%d\n",
		partition0->src_w, partition0->dst_w, partition1->src_x_offset,
		partition1->src_x_offset_c, partition1->src_w,
		partition1->dst_x_offset, partition1->dst_w);

	return 0;
}

static void imgresz_prepare_param(struct imgresz_data *data, struct imgresz_data *data_2nd)
{
	struct imgresz_scale_data *scaledata = &data->scale_data;
	struct imgresz_scale_data *scaledata2nd = &data_2nd->scale_data;

	imgresz_decide_resample_method(scaledata);
	imgresz_cal_h_factor(scaledata);
	imgresz_cal_v_factor(scaledata);
	if (scaledata->ufo_h_partition && data_2nd) {
		memcpy(scaledata2nd, scaledata, sizeof(*scaledata));
		scaledata2nd->ufo_page0 = false;
		imgresz_cal_ufo_h_partition_2hw(scaledata, scaledata2nd);
	}
}

static void imgresz_timeout(unsigned long hwid)
{
	struct imgresz_data *data, *data_2nd;

	data = imgresz_get_data(TICKET_FROM_HWID(hwid));

	logwarn("HW %ld Timeout %llu-%llu.\n", hwid, data->time_start, sched_clock());
	imgresz_hal_print_reg(data->base);

	if (data->scale_data.ufo_h_partition | data->scale_data.ufo_v_partition) {
		data_2nd = imgresz_get_data(TICKET_FROM_HWID(1));
		imgresz_hal_print_reg(data_2nd->base);
	}

	if (data->cb_info.fun_cb)
		data->cb_info.fun_cb(TICKET_FROM_HWID(data->hw_id),
				  IMGRESZ_CB_TIMEOUT,
				  data->cb_info.cb_data);
}

static bool imgresz_ticket_is_done(struct imgresz_data *data)
{
	struct imgresz_data *data_2nd;
	spinlock_t *state_lock;
	unsigned long flags;
	bool done = false;

	state_lock = &imgresz_get_data(TICKET_FROM_HWID(0))->state_lock;
	spin_lock_irqsave(state_lock, flags);
	if (data->scale_data.ufo_v_partition || data->scale_data.ufo_h_partition) {
		data = imgresz_get_data(TICKET_FROM_HWID(0));
		data_2nd = imgresz_get_data(TICKET_FROM_HWID(1));
		done = (data->state == IMGRESZ_STATE_DONE && data_2nd->state == IMGRESZ_STATE_DONE);
	} else {
		done = (data->state == IMGRESZ_STATE_DONE);
	}
	spin_unlock_irqrestore(state_lock, flags);

	return done;
}

static irqreturn_t __attribute__((unused)) imgresz_irq_handle(int irq, void *dev_id)
{
	struct imgresz_data *data = (struct imgresz_data *)dev_id;
	struct imgresz_scale_data *scaledata = &data->scale_data;
	spinlock_t	*state_lock;
	unsigned long flags;

	loginfo(IMGRESZ_LOG_CHECKSUM, "HW %d Checksum Rd 0x%x Wr 0x%x\n",
		data->hw_id, imgresz_checksum_read(data->base),
		imgresz_checksum_write(data->base));

	if (imgresz_log_level & IMGRESZ_LOG_DONEREG)
		imgresz_hal_print_reg(data->base);

	imgresz_hal_clr_irq(data->base);

	state_lock = &(imgresz_get_data(TICKET_FROM_HWID(0))->state_lock);
	spin_lock_irqsave(state_lock, flags);
	data->state = IMGRESZ_STATE_DONE;
	spin_unlock_irqrestore(state_lock, flags);

	/*
	 * Callback to the consumer that we has already done.
	 * In UFO2HW case: callback when both HW are ok.
	 *
	 * Don't callback if hw don't done.
	 */
	if (!imgresz_ticket_is_done(data))
		return IRQ_HANDLED;


	/* In 2HW case, unint both. */
	if (scaledata->ufo_h_partition || scaledata->ufo_v_partition) {
		data = imgresz_get_data(TICKET_FROM_HWID(1));
		imgresz_hal_HW_uninit(data->base);
		data = imgresz_get_data(TICKET_FROM_HWID(0));
		imgresz_hal_HW_uninit(data->base);
	} else
		imgresz_hal_HW_uninit(data->base);

	/* del_timer(&data->timer); */
	data->time_end = sched_clock();

	/* unit is ns, 4k60:1000/60=16.6ms. */
	loginfo(IMGRESZ_LOG_PERFORMANCE,
		"Performance:%llu-ns, src:%ux%u dst:%ux%u\n",
		data->time_end - data->time_start,
		data->scale_data.src_buf.pic_width,
		data->scale_data.src_buf.pic_height,
		data->scale_data.dst_buf.pic_width,
		data->scale_data.dst_buf.pic_height);

	wake_up(&data->wait_hw_done_queue);

	if (data->cb_info.fun_cb)
		data->cb_info.fun_cb(TICKET_FROM_HWID(data->hw_id),
		IMGRESZ_CB_DONE,
		data->cb_info.cb_data);
	DISP_MMP(MMP_DISP_MDP_GOT_IRQ, MMP_PULSE, 0, 0);

	return IRQ_HANDLED;
}

int imgresz_trigger_scale_async(IMGRESZ_TICKET ti, bool async)
{
	#if IMGRZ_TEE_ENABLE
	MTEEC_PARAM param[4];
	uint32_t param_type;
	spinlock_t *state_lock;
	unsigned long flags;
	struct imgresz_tz_data *data_tz = NULL, *data_tz_2nd = NULL;
	#endif
	struct imgresz_data *data = NULL, *data_2nd = NULL;
	struct imgresz_scale_data *scaledata = NULL, *scaledata2nd = NULL;
	int ret = 0;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;

	scaledata = &data->scale_data;
	if (imgresz_cur_chip_ver >= IMGRESZ_CURR_CHIP_VER_8695) {
		if (scaledata->ufo_h_partition)
			data_2nd = imgresz_get_data(TICKET_FROM_HWID(1));
	} else {
		if (scaledata->ufo_v_partition)
			data_2nd = imgresz_get_data(TICKET_FROM_HWID(3));
	}
	scaledata2nd = &data_2nd->scale_data;

	loginfo(IMGRESZ_LOG_CMD,
		"HW %d Md:%d src Md%d Res(%u-%u-%u) a(%pad-%pad) ufo-%d(part-%d,%d);Dst M %d Res(%u-%u-%u) a(%pad-%pad)\n",
		data->hw_id, scaledata->scale_mode,
		scaledata->src_buf.src_mode,
		scaledata->src_buf.buf_width, scaledata->src_buf.pic_width, scaledata->src_buf.pic_height,
		&scaledata->src_buf.y_buf_addr, &scaledata->src_buf.cb_buf_addr,
		scaledata->src_buf.ufo_type, scaledata->ufo_h_partition, scaledata->ufo_v_partition,
		scaledata->dst_buf.dst_mode,
		scaledata->dst_buf.buf_width, scaledata->dst_buf.pic_width, scaledata->dst_buf.pic_height,
		&scaledata->dst_buf.y_buf_addr, &scaledata->dst_buf.c_buf_addr);

	imgresz_prepare_param(data, data_2nd);
	/* data is the current hw-data.
	 * In 2HW case, all the timer and timer is saved in HW2 data.
	 */

	/* mod_timer(&data->timer, jiffies + msecs_to_jiffies(IMGRZ_HW_TIMEOUT)); */
	data->time_start = sched_clock();


#if IMGRZ_TEE_ENABLE
	if (scaledata->src_buf.buf_info.mem_type == IMGRZ_MEM_SECURE) {
		memset(param, 0, sizeof(param));
		imgresz_tz_data_init(data, data_2nd);
		data_tz = imgresz_g_tz_data(TICKET_FROM_HWID(data->hw_id));
		data_tz->log_level = imgresz_log_level;
		data_tz->hw_timeout = data->hw_timeout;
		param[0].mem.buffer = data_tz;
		param[0].mem.size = sizeof(*data_tz);
		if (scaledata->ufo_v_partition || scaledata->ufo_h_partition) {
			data_tz_2nd = imgresz_g_tz_data(TICKET_FROM_HWID(data_2nd->hw_id));
			data_tz_2nd->log_level = imgresz_log_level;
			data_tz_2nd->hw_timeout = data_2nd->hw_timeout;
			param[1].mem.buffer = data_tz_2nd;
			param[1].mem.size = sizeof(*data_tz_2nd);
		}

		/* free normal world irq */
		devm_free_irq(&data->pdev->dev, data->irq, (void *)data);
		if (scaledata->ufo_v_partition || scaledata->ufo_h_partition)
			devm_free_irq(&data_2nd->pdev->dev, data_2nd->irq, (void *)data_2nd);
		param_type = TZ_ParamTypes3(TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_VALUE_OUTPUT);
		ret = KREE_TeeServiceCall(imgresz_ta_session, IMGRESZ_TZ_CMD_TRIGGER, param_type, param);

		/* request normal world irq */
		if (devm_request_irq(&data->pdev->dev, data->irq, imgresz_irq_handle, 0,
				dev_name(&data->pdev->dev), (void *)data)) {
			dev_notice(&data->pdev->dev, "Failed @ IRQ-%d Request\n", data->irq);
			return -ENODEV;
		}
		if (scaledata->ufo_v_partition || scaledata->ufo_h_partition)
			if (devm_request_irq(&data_2nd->pdev->dev, data_2nd->irq, imgresz_irq_handle, 0,
					dev_name(&data_2nd->pdev->dev), (void *)data_2nd)) {
				dev_notice(&data_2nd->pdev->dev, "Failed @ IRQ-%d Request\n", data_2nd->irq);
				return -ENODEV;
			}
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("%s TZ fail ret 0x%x\n", __func__, ret);
			return -ret;
		}
		/* only use hw0's state lock*/
		state_lock = &(imgresz_get_data(TICKET_FROM_HWID(0))->state_lock);
		spin_lock_irqsave(state_lock, flags);
		data->state = param[2].value.a;
		if (scaledata->ufo_v_partition || scaledata->ufo_h_partition)
			data_2nd->state = param[2].value.a;
		spin_unlock_irqrestore(state_lock, flags);
		if (data->state == IMGRESZ_STATE_DONE)
			goto DONE;
		goto TIMEOUT;
	}
#endif
	DISP_MMP(MMP_DISP_MDP_SET_HW, MMP_PULSE, 0, 0);
	/* HW initial */
	imgresz_hal_HW_init(data->base);
	ret = _imgresz_hal_if_do_scale(data);
	if ((scaledata->ufo_v_partition || scaledata->ufo_h_partition) && data_2nd) {
		imgresz_hal_HW_init(data_2nd->base);
		ret |= _imgresz_hal_if_do_scale(data_2nd);
	}

	if (ret < 0) {
		logwarn("%s fail %d\n", __func__, ret);
		return ret;
	}

	if (!async) {
		if (wait_event_timeout(data->wait_hw_done_queue, imgresz_ticket_is_done(data),	data->hw_timeout))
			goto DONE;
		goto TIMEOUT;
	}


DONE:
	loginfo(IMGRESZ_LOG_CMD, "Ticket:0x%x, Done\n", ti);
	return IMGRESZ_CB_DONE;

TIMEOUT:
	imgresz_timeout(data->hw_id);
	return IMGRESZ_CB_TIMEOUT;
}

int imgresz_stop_scale(IMGRESZ_TICKET ti)
{
	struct imgresz_data *data;
	unsigned long flags;
	int ret;

	data = imgresz_get_data(ti);
	if (!data)
		return -EINVAL;

	spin_lock_irqsave(&data->state_lock, flags);
	if (data->state == IMGRESZ_STATE_BUSY) {
		ret = imgresz_hal_grace_reset(data->base);
		data->state = IMGRESZ_STATE_STOP;
	}
	spin_unlock_irqrestore(&data->state_lock, flags);

	if (data->scale_data.ufo_v_partition) {
		data = imgresz_get_data(TICKET_FROM_HWID(3));

		spin_lock_irqsave(&data->state_lock, flags);
		ret |= imgresz_hal_grace_reset(data->base);
		data->state = IMGRESZ_STATE_STOP;
		spin_unlock_irqrestore(&data->state_lock, flags);
	}
	return ret;
}

int imgresz_agent_trustzone_enable(IMGRESZ_TICKET ti, bool svp_enable)
{
	return 0;
}

void imgresz_debug_dump(IMGRESZ_TICKET ti)
{
	/* Imgresz dump itself while timeout. */
}

static const struct of_device_id mtk_imgresz_of_match[] = {
	{.compatible = "mediatek,mt8697-imgresz", .data = &mt8697_compat},
	{.compatible = "mediatek,mt8695-imgresz", .data = &mt8695_compat},
	{} /* NULL */
};

static int imgresz_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id;
	struct device *dev = &pdev->dev;
	struct imgresz_data *data;
	struct resource *res;
	int ret = 0;
#ifdef CONFIG_MTK_IOMMU
	struct device_node *larb_node;
	struct platform_device *larb_pdev;
#endif

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	data->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(data->base))
		return PTR_ERR(data->base);

	data->irq = platform_get_irq(pdev, 0);
	if (data->irq < 0)
		return data->irq;

	data->reszclk = devm_clk_get(dev, "imgrz_cg");
	if (IS_ERR(data->reszclk))
		return PTR_ERR(data->reszclk);

	of_id = of_match_node(mtk_imgresz_of_match, pdev->dev.of_node);
	imgresz_hal_set_chip_ver(((struct mtk_imgresz_compatible *)(of_id->data))->imgresz_curr_chip_ver);

	ret = of_property_read_u32(dev->of_node, "mediatek,imgreszid", &data->hw_id);
	if (ret) {
		dev_notice(dev, "missing mediatek,imgreszid property\n");
		return ret;
	} else if (data->hw_id >= IMGRESZ_MAX_NR) {
		dev_notice(dev, "Invalid imgresz hw id %d\n", data->hw_id);
		return -EINVAL;
	}
	imgresz_inst_data[data->hw_id] = data;
	platform_set_drvdata(pdev, data);

	#if IMGRESZ_TEST_EP
	if (data->hw_id == 0) {
		#if IMGRESZ_TEST_EP_PA
		struct device_node *np;
		struct resource res1;
		np = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
		if (np == NULL)
			logwarn("No memory-region specified\n");
		if (of_address_to_resource(np, 0, &res1))
			logwarn("imgrz of_address_to_resource failed\n");
		imgrz_test_buffer_pa = (unsigned int)res1.start;
		imgrz_test_buffer_size = (unsigned int)resource_size(&res1);
		imgrz_test_buffer_va = (unsigned long)ioremap_wc
				((phys_addr_t)imgrz_test_buffer_pa,	imgrz_test_buffer_size);
		#else
		imgrz_test_buffer_size = 128*1024*1024;
		imgrz_test_buffer_va = (unsigned long)dma_alloc_coherent
		(&pdev->dev, imgrz_test_buffer_size, (dma_addr_t *)&imgrz_test_buffer_pa, GFP_KERNEL);
		#endif
		logwarn("test region va:0x%lx, pa:0x%x, size:%u\n",
			imgrz_test_buffer_va, imgrz_test_buffer_pa, imgrz_test_buffer_size);
		imgresz_debug_init();
	}
	#endif

	if (devm_request_irq(dev, data->irq, imgresz_irq_handle, 0,
			     dev_name(dev), (void *)data)) {
		dev_notice(dev, "Failed @ IRQ-%d Request\n", data->irq);
		return -ENODEV;
	}

#ifdef CONFIG_MTK_IOMMU
	larb_node = of_parse_phandle(pdev->dev.of_node, "mediatek,larb", 0);
	if (!larb_node)
		return -EINVAL;

	larb_pdev = of_find_device_by_node(larb_node);
	of_node_put(larb_node);
	if ((!larb_pdev) || (!larb_pdev->dev.driver)) {
		pr_err("imgresz_probe is earlier than SMI\n");
		return -EPROBE_DEFER;
	}
	data->smi_larb_dev = &larb_pdev->dev;
#endif

#if IMGRZ_TEE_ENABLE
	imgresz_tz_inst_data[data->hw_id] = devm_kzalloc(dev, sizeof(struct imgresz_tz_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	if (data->hw_id == 0) {
		ret = KREE_CreateSession(TZ_TA_IMGRZ_UUID, &imgresz_ta_session);
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("KREE_CreateSession hw%u error, ret = %x\n", data->hw_id, ret);
			return ret;
		}
		#if IMGRESZ_TEST_EP
		ret = KREE_CreateSession(TZ_TA_MEM_UUID, &imgresz_mem_session);
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("Create memory session error %d\n", ret);
			return ret;
		}
		ret = KREE_AllocSecurechunkmem(imgresz_mem_session, &imgrz_test_sec_src_mem, 1024, 3840*2176*4);
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("KREE_AllocSecurechunkmem src, ret = %x\n", ret);
			return ret;
		}
		ret = KREE_AllocSecurechunkmem(imgresz_mem_session, &imgrz_test_sec_dst_mem, 1024, 3840*2176*4);
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("KREE_AllocSecurechunkmem dst, ret = %x\n", ret);
			return ret;
		}
		logwarn("KREE_AllocSecurechunkmem src:0x%x, dst:0x%x\n",
				imgrz_test_sec_src_mem, imgrz_test_sec_dst_mem);
		#endif
	}
#endif
	pm_runtime_enable(dev);
	spin_lock_init(&data->state_lock);
	data->hw_timeout = msecs_to_jiffies(IMGRZ_HW_TIMEOUT);
	/*
	setup_timer(&data->timer, imgresz_timeout, data->hw_id);
	*/
	init_waitqueue_head(&data->wait_hw_done_queue);
	/*
	dev_info(dev, "imgresz-%d probe done base %p\n", data->hw_id, data->base);
	*/
	return ret;
}

static int imgresz_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
#if IMGRZ_TEE_ENABLE
	struct imgresz_data *data = platform_get_drvdata(pdev);

	if (data->hw_id == 0) {
		ret = KREE_CloseSession(imgresz_ta_session);
		if (ret != TZ_RESULT_SUCCESS) {
			logwarn("KREE_CloseSession hw%u error, ret = %x\n", data->hw_id, ret);
			return ret;
		}
	}
#endif
	pm_runtime_disable(dev);

	return ret;
}

static struct platform_driver mtk_imgresz_driver = {
	.probe	= imgresz_probe,
	.remove	= imgresz_remove,
	/* No need the suspend/resume as the default clk always is disable. */
	.driver	= {
		.name = "mtk-imgresz",
		.of_match_table = of_match_ptr(mtk_imgresz_of_match),
	}
};

static int __init mtk_imgresz_init(void)
{
	return platform_driver_register(&mtk_imgresz_driver);
}

module_init(mtk_imgresz_init);

module_param_named(imgresz_log_level, imgresz_log_level, int, S_IRUGO | S_IWUSR);
