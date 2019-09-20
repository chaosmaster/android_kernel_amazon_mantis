/*
 * Copyright (c) 2016-2017 MediaTek Inc.
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
#ifndef MTK_IMGRESZ_EX_H
#define MTK_IMGRESZ_EX_H

#if (defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) || \
	defined(CONFIG_TRUSTY))
#include <kree/mem.h>
#include <kree/system.h>
#include <tz_cross/ta_mem.h>
#define IMGRZ_TEE_ENABLE 1
#define TZ_TA_IMGRZ_NAME "IMGRESZ_TA"
#define TZ_TA_IMGRZ_UUID "imgresz_ta_uuid_11575"

enum IMGRESZ_TZ_CMD {
	IMGRESZ_TZ_CMD_MIN = 0,
	IMGRESZ_TZ_CMD_TRIGGER = 1,
	IMGRESZ_TZ_CMD_MAX,
};

#else
#define IMGRZ_TEE_ENABLE 0
#endif

#define IMGRESZ_TEST_EP 0
#define IMGRESZ_TEST_EP_PA 0
#if IMGRESZ_TEST_EP
#if IMGRZ_TEE_ENABLE
extern KREE_SECUREMEM_HANDLE imgrz_test_sec_src_mem;
extern KREE_SECUREMEM_HANDLE imgrz_test_sec_dst_mem;
#endif
extern unsigned int imgrz_test_buffer_pa;
extern unsigned int imgrz_test_buffer_size;
extern unsigned long imgrz_test_buffer_va;
extern void imgresz_debug_init(void);
#endif
extern enum IMGRESZ_CHIP_VER imgresz_cur_chip_ver;

typedef int IMGRESZ_TICKET;

enum IMGRESZ_LOG_LEVEL {
	IMGRESZ_LOG_DEFAULT      = BIT(0),
	IMGRESZ_LOG_CMD          = BIT(1),
	IMGRESZ_LOG_WARNING      = BIT(2),
	IMGRESZ_LOG_SHOWREG      = BIT(3),
	IMGRESZ_LOG_UFO          = BIT(4),
	IMGRESZ_LOG_CHECKSUM     = BIT(5),
	IMGRESZ_LOG_DONEREG      = BIT(6),
	IMGRESZ_LOG_PERFORMANCE  = BIT(7),
	IMGRESZ_LOG_REG			 = BIT(8),
};

/*
 * Each imgresz HW may has different functions.
 * Here survey what the consumer want before dispatching the corresponding HW.
 * like hw0/hw2/hw3 only support UFO while hw1 don't support in mt8581.
 */
enum imgresz_ticket_fun_type {
	IMGRESZ_FUN_MIN,
	IMGRESZ_FUN_NORMAL,			/* Normal case that is not belong to below. */
	IMGRESZ_FUN_JPGPIC,			/* jpg picture mode, jpg -> ayuv */
	IMGRESZ_FUN_ALPHA_COM,		/* alpha composition */
	IMGRESZ_FUN_UFO,			/* ufo src */
	IMGRESZ_FUN_UFO_2HW,		/* 2hw work for a ufo src */
	IMGRESZ_FUN_ONEPHASE_2HW,	/* 2hw work for a y/c src */
	IMGRESZ_FUN_RM,
	IMGRESZ_FUN_WEBP,
	IMGRESZ_FUN_MAX,
};

enum IMGRESZ_SRC_COLOR_MODE {
	IMGRESZ_SRC_COL_MD_NONE = 0,
	IMGRESZ_SRC_COL_MD_JPG_DEF,
	IMGRESZ_SRC_COL_MD_420_BLK,
	IMGRESZ_SRC_COL_MD_422_BLK,
	IMGRESZ_SRC_COL_MD_420_RS,
	IMGRESZ_SRC_COL_MD_422_RS,
	IMGRESZ_SRC_COL_MD_AYUV,
	IMGRESZ_SRC_COL_MD_ARGB_1555,
	IMGRESZ_SRC_COL_MD_RGB_565,
	IMGRESZ_SRC_COL_MD_ARGB_4444,
	IMGRESZ_SRC_COL_MD_ARGB_8888,
};

enum IMGRESZ_DST_COLOR_MODE {
	IMGRESZ_DST_COL_MD_NONE = 0,
	IMGRESZ_DST_COL_MD_420_BLK,
	IMGRESZ_DST_COL_MD_422_BLK,
	IMGRESZ_DST_COL_MD_420_RS,
	IMGRESZ_DST_COL_MD_422_RS,
	IMGRESZ_DST_COL_MD_AYUV,
	IMGRESZ_DST_COL_MD_ARGB_1555,
	IMGRESZ_DST_COL_MD_RGB_565,
	IMGRESZ_DST_COL_MD_ARGB_4444,
	IMGRESZ_DST_COL_MD_ARGB_8888,
};

enum IMGRESZ_UFO_TYPE {
	IMGRESZ_UFO_MIN,
	IMGRESZ_UFO_8BIT,
	IMGRESZ_UFO_10BIT_COMPACT,
	IMGRESZ_UFO_10BIT_COMPACT_UNCOMPRESS,
	IMGRESZ_UFO_MAX,
};

/* Only for JPG mode, Y/Cb/Cr component sample */
/* Example: jpg420: the component sample is 22 11 11.
 *          jpg422: the component sample is 22 11 11.
 *          jpg444: the component sample is 11 11 11.
 */
struct imgresz_jpg_comp_sample {
	unsigned char      y_comp_sample_h;
	unsigned char      y_comp_sample_v;
	unsigned char      cb_comp_sample_h;
	unsigned char      cb_comp_sample_v;
	unsigned char      cr_comp_sample_h;
	unsigned char      cr_comp_sample_v;
};

typedef enum IMGRZ_MEM_ENUM {
	IMGRZ_MEM_VA,
	IMGRZ_MEM_ION,
	IMGRZ_MEM_PHY,
	IMGRZ_MEM_MVA,
	IMGRZ_MEM_SECURE,
} IMGRZ_MEM_ENUM;

struct imgrz_buffer_info {
	IMGRZ_MEM_ENUM mem_type;
	uint32_t fd;
	uint32_t y_offset;
	uint32_t cb_offset;
	uint32_t cr_offset;
	uint32_t ylen_offset;
	uint32_t clen_offset;
};

/*
 *  picture layout
 *
 *  --------A----------
 *  |                 |
 *  |   y             |
 *  |                 |
 *  | x ----a------   |
 *  |   |         |   |
 *  |   |         |   |
 *  B   b         |   |
 *  |   |         |   |
 *  |   |         |   |
 *  |   |         |   |
 *  |   |         |   |
 *  |   -----------   |
 *  |                 |
 *  |                 |
 *  -------------------
 *
 *  A: buf_width (buffer width/pitch)
 *  B: buf_height (buffer height)
 *  x: pic_x_offset(x offset)
 *  y: pic_y_offset(y offset)
 *  a: pic_width(picture width)
 *  b: pic_height(picture height)
 */
struct imgresz_src_buf_info {
	enum IMGRESZ_SRC_COLOR_MODE  src_mode;
	/*
	 * All the address are the dma addr that hw require.
	 * There are 2 types memory:
	 * a. physical address. while iommu don't support. it's for mt8697.
	 * b. iova address. it's from dma_xx mapped from m4u. it's for mt8695.
	 * (It is to say that setting physical address in mt8697.)
	 *
	 * Note that,
	 * 1) please don't set virtual address here as current is 64bit kernel.
	 *    the HW don't support long-descriptor va.
	 *    If you'd like to use 32bit va, please call imgresz_legacy_iommu_enable.
	 * 2) all the dma_addr_t below are the same.
	 */
	uint32_t                   y_buf_addr;
	uint32_t                   cb_buf_addr;
	uint32_t                   cr_buf_addr; /* Only needed by jpg. */
	struct imgrz_buffer_info     buf_info;
	uint32_t                   buf_width;   /* Known as pitch. */
	uint32_t                   buf_height;
	uint32_t                   pic_width;
	uint32_t                   pic_height;
	unsigned int                 pic_x_offset;
	unsigned int                 pic_y_offset;
	bool                         interlaced;
	bool                         topfield;
	bool                         bottomfield;
	struct imgresz_jpg_comp_sample jpg_comp; /* Only for jpg compoent factor ratio */

	enum IMGRESZ_UFO_TYPE        ufo_type;
	uint32_t                   ufo_ylen_buf;
	uint32_t                   ufo_clen_buf;
	uint32_t                       ufo_ylen_buf_len;/* C always half of Y*/
	uint32_t                       ufo_ybuf_len;    /* C always half of Y*/
	bool                         ufo_jump;
};

struct imgresz_dst_buf_info {
	enum IMGRESZ_DST_COLOR_MODE  dst_mode;
	uint32_t                   y_buf_addr;
	uint32_t                   c_buf_addr;
	struct imgrz_buffer_info     buf_info;
	uint32_t                       buf_width;
	uint32_t                       pic_width;
	uint32_t                       pic_width_cb;
	uint32_t                       pic_width_cr;
	uint32_t                       pic_height;
	unsigned int                 pic_x_offset;
	unsigned int                 pic_y_offset;
	bool                         cbcr_swap; /* cb/cr swap in NV21. */
	bool                         bit10;
};

struct imgresz_partial_buf_info {
	uint32_t         y_buf_addr;
	uint32_t         cb_buf_addr;
	uint32_t         cr_buf_addr;
	unsigned int       y_buf_line;
	unsigned int       cb_buf_line;
	unsigned int       cr_buf_line;
	bool               first_row;
	bool               last_row;
	uint32_t           temp_buf_addr; /* temp buffer : buf_len * 6.
					   * always physical address. not support iommu.
					   */
};

struct imgresz_jpg_info {
	bool               y_exist;
	bool               cb_exist;
	bool               cr_exist;
	bool               preload;
};

struct imgresz_rm_info {
	bool               rpr_mode;
	bool               racing_mode;
};

struct imgresz_hal_info {
	unsigned int h8_factor_y;
	unsigned int h8_offset_y;
	unsigned int h8_factor_cb;
	unsigned int h8_offset_cb;
	unsigned int h8_factor_cr;
	unsigned int h8_offset_cr;
	unsigned int hsa_factor_y;
	unsigned int hsa_offset_y;
	unsigned int hsa_factor_cb;
	unsigned int hsa_offset_cb;
	unsigned int hsa_factor_cr;
	unsigned int hsa_offset_cr;
	unsigned int v4_factor_y;
	unsigned int v4_factor_cb;
	unsigned int v4_factor_cr;
	unsigned int v4_offset_y;
	unsigned int v4_offset_cb;
	unsigned int v4_offset_cr;
	unsigned int vm_factor_y;
	unsigned int vm_factor_cb;
	unsigned int vm_factor_cr;
	unsigned int vm_offset_y;
	unsigned int vm_offset_cb;
	unsigned int vm_offset_cr;
	bool vm_scale_up_y;
	bool vm_scale_up_cb;
	bool vm_scale_up_cr;
};

struct imgresz_h_partition_info {
	unsigned int src_x_offset;
	unsigned int src_x_offset_c;
	unsigned int src_w;
	unsigned int dst_x_offset;
	unsigned int dst_w;
};

struct imgresz_v_partition_info {

};

union imgresz_partition_info {
	struct imgresz_h_partition_info h_partition_info;
	struct imgresz_v_partition_info v_partition_info;
};

enum imgresz_scale_mode {
	IMGRESZ_NONE_SCALE,
	IMGRESZ_FRAME_SCALE,
	IMGRESZ_PARTIAL_SCALE,
	IMGRESZ_JPEG_PIC_SCALE,
};

enum imgresz_cb_state {
	IMGRESZ_CB_DONE,   /* Successfully. */
	IMGRESZ_CB_TIMEOUT,/* HW timeout. */
	IMGRESZ_CB_STOP,
};

typedef int (*imgresz_callback_fun)(IMGRESZ_TICKET ti, enum imgresz_cb_state hw_state,
				    void *privdata);

struct imgresz_cb_data {
	imgresz_callback_fun	fun_cb;
	void				*cb_data;
};

/* Return : negative on failure. Others: successful. */
IMGRESZ_TICKET imgresz_ticket_get(enum imgresz_ticket_fun_type type);
int imgresz_ticket_put(IMGRESZ_TICKET ti);

/* All the functions below:
 * Returns 0 if successful, negative on failure.
 */
int imgresz_set_scale_mode(IMGRESZ_TICKET ti, enum imgresz_scale_mode scalemd);
int imgresz_set_src_bufinfo(IMGRESZ_TICKET ti, struct imgresz_src_buf_info *srcbufinfo);
int imgresz_set_dst_bufinfo(IMGRESZ_TICKET ti, struct imgresz_dst_buf_info *dstbufinfo);
int imgresz_set_jpeg_info(IMGRESZ_TICKET ti, struct imgresz_jpg_info *jpeginfo);
int imgresz_set_rm_info(IMGRESZ_TICKET ti, struct imgresz_rm_info *rminfo);
int imgresz_set_partial_info(IMGRESZ_TICKET ti, struct imgresz_partial_buf_info *partialbuf);

/*
 * Imgresz HW could acsess both sec and non-sec buffer.
 * Configure the hw agent whether the svp enable.
 */
int imgresz_agent_trustzone_enable(IMGRESZ_TICKET ti, bool svp_enable);

/*
 * Resz will notify the consumer after hw done or timeout.
 * please don't add so many code in the callback function as it is in irq context.
 */
int imgresz_set_callback_fun(IMGRESZ_TICKET ti, imgresz_callback_fun fun, void *privdata);

/* Asynchronous. Please get the hw state in the callback. */
int imgresz_trigger_scale_async(IMGRESZ_TICKET ti, bool async);
int imgresz_stop_scale(IMGRESZ_TICKET ti);

/* Help dump register info to debug. */
void imgresz_debug_dump(IMGRESZ_TICKET ti);

/*
 * Legacy iommu support(the format is armv7 32bit short-descriptor.):
 *
 * pgtablebase: the base physical address of the pagetable.
 * rd_iommu_en: Read iommu enable.
 * wr_iommu_en: Write iommu enable.
 *
 * Note that: don't call this in arm64. if this is enable, use va in "dma_addr_t".
 */
int imgresz_legacy_iommu_enable(IMGRESZ_TICKET ti, phys_addr_t pgtablebase,
				bool rd_iommu_en, bool wr_iommu_en);
#endif
