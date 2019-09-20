/*
 * Copyright (C) 2016 MediaTek Inc.
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

#define LOG_TAG "DOVI_MAIN"

#include<linux/fs.h>
#include<linux/uaccess.h>

#include "dovi_log.h"
#include "dovi_type.h"
#include "disp_hw_mgr.h"

#include "videoin_hal.h"
#include "disp_dovi_io.h"
#include "linux/dma-mapping.h"

#include "vdp_hal.h"
#include "fmt_def.h"
#include "fmt_hal.h"
#include "disp_path.h"
#include "disp_sys_hal.h"

#define MAX_FILE_NAME 100
#define MAX_VDO_ID 2

#define FMT_4K_DELAY 0x62

bool dump_rpu_enable;


static uint32_t *y_addr_va[MAX_VDO_ID];
static dma_addr_t y_addr_pa[MAX_VDO_ID];

static uint32_t *c_addr_va[MAX_VDO_ID];
static dma_addr_t c_addr_pa[MAX_VDO_ID];

static uint32_t *y_len_addr_va[MAX_VDO_ID];
static dma_addr_t y_len_addr_pa[MAX_VDO_ID];

static uint32_t *c_len_addr_va[MAX_VDO_ID];
static dma_addr_t c_len_addr_pa[MAX_VDO_ID];

static uint32_t *graphic_addr_va[MAX_VDO_ID];
static dma_addr_t graphic_addr_pa[MAX_VDO_ID];

static uint32_t *graphic_header_va[MAX_VDO_ID];
static dma_addr_t graphic_header_pa[MAX_VDO_ID];

static bool allocated[MAX_VDO_ID];

struct pat_res {
	uint32_t width;
	uint32_t height;
};

struct pat_res pattern_res[] = {
	{360, 240}, /* 0 */
	{640, 360}, /* 1 */
	{720, 480}, /* 2 */
	{720, 576}, /* 3 */
	{960, 540}, /* 4 */
	{1280, 720}, /* 5 */
	{1920, 1080}, /* 6 */
	{3840, 2160}, /* 7 */
	{4096, 2160} /* 8 */
};

uint32_t pattern_idx[] = {
	240,
	360,
	480,
	576,
	540,
	720,
	1080,
	2160,
	2161
};

uint32_t graphic_header[] = {
	0xE0000000, /* color mode */
	0xFFC00000, /* 23:0 -> 27:4*/
	0xE40001E0, /* 1080p */
	0x00000000,
	0x00001000,
	0x00001000,
	0x00000001, /* 3:2 -> 31:30  */
	0x51800000, /* 24:23 -> 29:28  */
	0x00000000,
	0x04380780,
	0x00000438,
	0x00000780,
};

#define DOVI_ALIGN_TO(x, n) ((x + n - 1)/n*n)


bool dump_file_opened;
mm_segment_t idk_dump_fs;
struct file *idk_dump_fp;


static uint32_t *y_idk_dump_addr_va;
static dma_addr_t y_idk_dump_addr_pa;

static uint32_t *cb_idk_dump_addr_va;
static dma_addr_t cb_idk_dump_addr_pa;

static uint32_t *cr_idk_dump_addr_va;
static dma_addr_t cr_idk_dump_addr_pa;

uint32_t *graphic_idk_load_addr_va;
dma_addr_t graphic_idk_dump_load_pa;

uint32_t *graphic_header_idk_load_addr_va;
dma_addr_t graphic_header_idk_dump_load_pa;

enum VIDEO_BIT_MODE idk_dump_bpp;
uint32_t idk_dump_len;

bool graphic_buf_allocated;

void disp_dovi_idk_dump_frame_end(void)
{
	if (dump_file_opened) {
		uint32_t idk_dump_size = 4096*2160*2;
		uint32_t graphic_size = 4096*2160*4;
		uint32_t graphic_header_size = 48;

		filp_close(idk_dump_fp, NULL);
		set_fs(idk_dump_fs);
		dump_file_opened = false;

		dma_free_coherent(dovi_dev, idk_dump_size, y_idk_dump_addr_va,
			y_idk_dump_addr_pa);
		dma_free_coherent(dovi_dev, idk_dump_size, cb_idk_dump_addr_va,
			cb_idk_dump_addr_pa);
		dma_free_coherent(dovi_dev, idk_dump_size, cr_idk_dump_addr_va,
			cr_idk_dump_addr_pa);
		dma_free_coherent(dovi_dev, graphic_size, graphic_idk_load_addr_va,
			graphic_idk_dump_load_pa);
		dma_free_coherent(dovi_dev, graphic_header_size, graphic_header_idk_load_addr_va,
			graphic_header_idk_dump_load_pa);
	}
}

void disp_dovi_idk_dump_frame_start(uint32_t file_id)
{
	if (!dump_file_opened) {
		char fileName[MAX_FILE_NAME];
		uint32_t idk_dump_size = 4096*2160*2;
		uint32_t graphic_size = 4096*2160*4;
		uint32_t graphic_header_size = 48;

		memset(fileName, 0, MAX_FILE_NAME);
		if (ll_rgb_desired)
			sprintf(fileName, "/data/dovi/dump_%d_frame.rgb", file_id);
		else
			sprintf(fileName, "/data/dovi/dump_%d_frame.yuv", file_id);

		idk_dump_fs = get_fs();
		set_fs(KERNEL_DS);
		idk_dump_fp = filp_open(fileName, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);

		if (IS_ERR(idk_dump_fp)) {
			dovi_error("open file %s fail\n", fileName);
		}

		dovi_printf("dump %s\n", fileName);
		dump_file_opened = true;

		y_idk_dump_addr_va =
			dma_alloc_coherent(dovi_dev, idk_dump_size, &y_idk_dump_addr_pa, GFP_KERNEL);
		cb_idk_dump_addr_va =
			dma_alloc_coherent(dovi_dev, idk_dump_size, &cb_idk_dump_addr_pa, GFP_KERNEL);
		cr_idk_dump_addr_va =
			dma_alloc_coherent(dovi_dev, idk_dump_size, &cr_idk_dump_addr_pa, GFP_KERNEL);
		graphic_idk_load_addr_va =
			dma_alloc_coherent(dovi_dev, graphic_size, &graphic_idk_dump_load_pa, GFP_KERNEL);
		graphic_header_idk_load_addr_va =
			dma_alloc_coherent(dovi_dev, graphic_header_size, &graphic_header_idk_dump_load_pa, GFP_KERNEL);

		dovi_default("y_idk_dump_addr_va %p 0x%x\n", y_idk_dump_addr_va,
			(uint32_t)y_idk_dump_addr_pa);
		dovi_default("cb_idk_dump_addr_va %p 0x%x\n", cb_idk_dump_addr_va,
			(uint32_t)cb_idk_dump_addr_pa);
		dovi_default("cr_idk_dump_addr_va %p 0x%x\n", cr_idk_dump_addr_va,
			(uint32_t)cr_idk_dump_addr_pa);
		dovi_default("graphic_addr_va %p 0x%x\n", graphic_idk_load_addr_va,
			(uint32_t)graphic_idk_dump_load_pa);
		dovi_default("graphic_header_va %p 0x%x\n",
			graphic_header_idk_load_addr_va,
			(uint32_t)graphic_header_idk_dump_load_pa);
	}
}

void disp_dovi_alloc_graphic_buffer(void)
{
	if (!graphic_buf_allocated) {
		uint32_t graphic_size = 4096*2160*4;
		uint32_t graphic_header_size = 48;

		graphic_idk_load_addr_va =
			dma_alloc_coherent(dovi_dev, graphic_size, &graphic_idk_dump_load_pa, GFP_KERNEL);
		graphic_header_idk_load_addr_va =
			dma_alloc_coherent(dovi_dev, graphic_header_size, &graphic_header_idk_dump_load_pa, GFP_KERNEL);

		dovi_default("graphic_addr_va %p 0x%x\n", graphic_idk_load_addr_va,
			(uint32_t)graphic_idk_dump_load_pa);
		dovi_default("graphic_header_va %p 0x%x\n",
			graphic_header_idk_load_addr_va,
			(uint32_t)graphic_header_idk_dump_load_pa);

		graphic_buf_allocated = true;
	}
}

void disp_dovi_free_graphic_buffer(void)
{
	if (graphic_buf_allocated) {
		uint32_t graphic_size = 4096*2160*4;
		uint32_t graphic_header_size = 48;

		graphic_buf_allocated = false;

		dma_free_coherent(dovi_dev, graphic_size, graphic_idk_load_addr_va,
			graphic_idk_dump_load_pa);
		dma_free_coherent(dovi_dev, graphic_header_size, graphic_header_idk_load_addr_va,
			graphic_header_idk_dump_load_pa);
	}
}


void disp_dovi_idk_dump_frame(void)
{
	if (dump_file_opened) {
		/* write date */
		dovi_default("write one frame to file!\n");
		if (ll_rgb_desired) {
			/* GBR TO RGB */
			vfs_write(idk_dump_fp, (const char *)cr_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
			vfs_write(idk_dump_fp, (const char *)y_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
			vfs_write(idk_dump_fp, (const char *)cb_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
		} else {
			vfs_write(idk_dump_fp, (const char *)y_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
			vfs_write(idk_dump_fp, (const char *)cb_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
			vfs_write(idk_dump_fp, (const char *)cr_idk_dump_addr_va, idk_dump_len, &idk_dump_fp->f_pos);
		}
	} else {
		dovi_error("file is not open!\n");
	}
}

void disp_dovi_idk_dump_vin(bool enable, enum VIDEOIN_SRC_SEL src,
	enum VIDEOIN_YCbCr_FORMAT fmt)
{
	int h_start = 0;
	int v_odd_start = 0;
	int v_even_start = 0;
	enum VIDEO_BIT_MODE bit_mode;
	bool is_16_packet;
	bool is_uv_swap;
	uint16_t htotal = dovi_res.htotal;
	/* uint16_t vtotal = dovi_res.vtotal; */
	uint16_t width = dovi_res.width;
	uint16_t height = dovi_res.height;
	bool is_progressive = dovi_res.is_progressive;

	enum VIDEO_CHN_SEL cb_sel;
	enum VIDEO_CHN_SEL cr_sel;


	bool is_444;


	if (idk_dump_bpp == VIDEOIN_BITMODE_8) {
		bit_mode = VIDEOIN_BITMODE_8;
		is_16_packet = false;
	} else if (idk_dump_bpp == VIDEOIN_BITMODE_10) {
		bit_mode = VIDEOIN_BITMODE_10;
		is_16_packet = true;
	} else if (idk_dump_bpp == VIDEOIN_BITMODE_12) {
		bit_mode = VIDEOIN_BITMODE_12;
		is_16_packet = true;
	} else {
		bit_mode = VIDEOIN_BITMODE_8;
		is_16_packet = false;
		dovi_error("bpp error %d\n", idk_dump_bpp);
		return;
	}

	if (!is_16_packet)
		idk_dump_len = width * height;
	else if (is_16_packet)
		idk_dump_len = width * height * 2;

	if (fmt == VIDEOIN_FORMAT_420)
		is_444 = false;
	else if (fmt == VIDEOIN_FORMAT_422)
		is_444 = false;
	else if (fmt == VIDEOIN_FORMAT_444)
		is_444 = true;
	else if (fmt > VIDEOIN_FORMAT_444) {
		dovi_error("color fmt error %d\n", fmt);
		return;
	}

	if (src > VIDEOIN_SRC_SEL_DOLBY3) {
		dovi_error("src select error %d\n", src);
		return;
	}


	is_uv_swap = false;

	if (height == 480) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xC4;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x80;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;
		if (is_progressive == true) {
			v_odd_start = 0x2A;
			v_even_start = 0x2A;
		} else {
			v_odd_start = 0x16;
			v_even_start = 0x16;
		}
	} else if (height == 576) {
		h_start = 0xEC;
		v_odd_start = 0x2C;
		v_even_start = 0x2C;
	} else if (height == 720) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xC4;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x10C;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;
		v_odd_start = 0x19;
		v_even_start = 0x19;
	} else if (height == 1080) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xC4;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0xC8;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;

		if (is_progressive == true) {
			v_odd_start = 0x29;
			v_even_start = 0x29;
		} else {
			v_odd_start = 0x15;
			v_even_start = 0x15;
		}
	} else if (height == 2160) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0x189;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x126;
		else
			h_start = 0x17F; /* RGB2HDMI */

		v_odd_start = 0x52;
		v_even_start = 0x52;
	}

	if (h_start%2)
		is_uv_swap = true;

	if (is_uv_swap) {
		cb_sel = SEL_CR_CHN;
		cr_sel = SEL_CB_CHN;
	} else {
		cb_sel = SEL_CB_CHN;
		cr_sel = SEL_CR_CHN;
	}

	videoin_hal_enable(enable);
	if (enable) {
		videoin_hal_update_addr(y_idk_dump_addr_pa,
			cb_idk_dump_addr_pa, cr_idk_dump_addr_pa, is_444);
		vdout_sys_hal_videoin_source_sel(src);
		videoin_hal_set_h(width, htotal, is_444, is_16_packet, bit_mode);
		videoin_hal_set_v(height, fmt);
		videoin_hal_set_color_format(fmt);
		videoin_hal_set_channel_select(SEL_Y_CHN, cb_sel, cr_sel);
		videoin_hal_set_bitmode(bit_mode, is_16_packet);
		videoin_hal_demode_enable(false);
		videoin_hal_set_active_zone(h_start, v_odd_start, v_even_start);
	}
}

void disp_dovi_dump_buffer(char *file_name, unsigned char *buff, uint32_t len)
{
	char fileName[MAX_FILE_NAME];
	mm_segment_t fs;
	struct file *fp = NULL;

	memset(fileName, 0, MAX_FILE_NAME);
	if (file_name != NULL && *file_name != '\0')
		sprintf(fileName, "/sdcard/dovi/%s", file_name);

	fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(fileName, O_RDWR | O_CREAT | O_TRUNC, 0644);

	if (IS_ERR(fp)) {
		dovi_error("open file %s fail\n", fileName);
		return;
	}

	dovi_printf("dump %s len %d\n", fileName, len);

	/* write date */
	vfs_write(fp, buff, len, &fp->f_pos);

	filp_close(fp, NULL);
	set_fs(fs);
}

void disp_dovi_load_buffer(char *file_name, unsigned char *buff, uint32_t len)
{
	mm_segment_t fs;
	struct file *fp = NULL;

	fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(file_name, O_RDONLY, 0x0);

	dovi_printf("load %s len %d\n", file_name, len);

	if (IS_ERR(fp)) {
		dovi_error("open file %s fail\n", file_name);
		return;
	}


	/* read date */
	vfs_read(fp, buff, len, &fp->f_pos);

	filp_close(fp, NULL);
	set_fs(fs);
}

void disp_dovi_dump_comp(unsigned int frame_num,
unsigned char *buff, uint32_t len)
{
	char fileName[MAX_FILE_NAME];

	memset(fileName, 0, MAX_FILE_NAME);
	sprintf(fileName, "tmp_comp.%04u.bin", frame_num);

	disp_dovi_dump_buffer(fileName, buff, len);
}

void disp_dovi_dump_orig_md(unsigned int frame_num,
unsigned char *buff, uint32_t len)
{
	char fileName[MAX_FILE_NAME];

	memset(fileName, 0, MAX_FILE_NAME);
	sprintf(fileName, "tmp_dm.%04u.bin", frame_num);

	disp_dovi_dump_buffer(fileName, buff, len);
}

void disp_dovi_dump_hdmi_md(unsigned int frame_num,
unsigned char *buff, uint32_t len)
{
	char fileName[MAX_FILE_NAME];

	memset(fileName, 0, MAX_FILE_NAME);
	sprintf(fileName, "tmp_hdmi_dm.%04u.bin", frame_num);

	disp_dovi_dump_buffer(fileName, buff, len);
}

void disp_dovi_dump_core_reg(unsigned int core_id,
unsigned int frame_num,
unsigned char *buff,
uint32_t len)
{
	char fileName[MAX_FILE_NAME];

	memset(fileName, 0, MAX_FILE_NAME);
	sprintf(fileName, "tmp_dm_core%u_reg.%04u.bin", core_id, frame_num);

	disp_dovi_dump_buffer(fileName, buff, len);
}

void disp_dovi_dump_core_lut(unsigned int core_id,
unsigned int frame_num,
unsigned char *buff,
uint32_t len)
{
	char fileName[MAX_FILE_NAME];

	memset(fileName, 0, MAX_FILE_NAME);
	sprintf(fileName, "tmp_dm_core%u_lut.%04u.bin", core_id, frame_num);

	disp_dovi_dump_buffer(fileName, buff, len);
}

void disp_dovi_dump_rpu(bool dump_enable, unsigned char *buff, uint32_t rpu_len)
{
	static char file_name[MAX_FILE_NAME] = "/sdcard/rpu.bin";
	static mm_segment_t fs;
	static struct file *fp;
	static bool rpu_file_opened;
	static uint32_t rpu_frame;
	static uint32_t rpu_total_len;

	if (dump_enable && dump_rpu_enable) {
		if (!rpu_file_opened) {
			fs = get_fs();
			set_fs(KERNEL_DS);

			fp = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);
			if (IS_ERR(fp)) {
				dovi_error("open file %s fail\n", file_name);
				return;
			}

			rpu_file_opened = 1;
			rpu_frame = 0;
			rpu_total_len = 0;
			dovi_printf("open rpu file %s ok\n", file_name);
		}

		/* write date */
		if (rpu_len != 0) {
			vfs_write(fp, buff, rpu_len, &fp->f_pos);
			rpu_total_len += (rpu_len - 2);
			dovi_printf("dump frame %3d len %d total %d\n",
				    rpu_frame++, rpu_len, rpu_total_len);
		}
	} else if (rpu_file_opened && !IS_ERR(fp)) {
		rpu_file_opened = 0;
		filp_close(fp, NULL);
		set_fs(fs);

		dovi_printf("close rpu file %s ok\n", file_name);
	}
}

void disp_dovi_dump_vin(bool enable, enum VIDEOIN_SRC_SEL src,
enum VIDEOIN_YCbCr_FORMAT fmt, enum VIDEO_BIT_MODE bpp)
{
	int h_start = 0;
	int v_odd_start = 0;
	int v_even_start = 0;
	enum VIDEO_BIT_MODE bit_mode;
	bool is_16_packet;
	bool is_uv_swap;
	uint16_t htotal = dovi_res.htotal;
	/* uint16_t vtotal = dovi_res.vtotal; */
	uint16_t width = dovi_res.width;
	uint16_t height = dovi_res.height;
	bool is_progressive = dovi_res.is_progressive;

	enum VIDEO_CHN_SEL cb_sel;
	enum VIDEO_CHN_SEL cr_sel;

	static uint32_t *y_addr_va;
	static dma_addr_t y_addr_pa;

	static uint32_t *cb_addr_va;
	static dma_addr_t cb_addr_pa;

	static uint32_t *cr_addr_va;
	static dma_addr_t cr_addr_pa;

	bool is_444;

	uint32_t dump_size = 4096*2160*2;

	static bool allocated;

	if (bpp == VIDEOIN_BITMODE_8) {
		bit_mode = VIDEOIN_BITMODE_8;
		is_16_packet = false;
	} else if (bpp == VIDEOIN_BITMODE_10) {
		bit_mode = VIDEOIN_BITMODE_10;
		is_16_packet = true;
	} else if (bpp == VIDEOIN_BITMODE_12) {
		bit_mode = VIDEOIN_BITMODE_12;
		is_16_packet = true;
	} else {
		bit_mode = VIDEOIN_BITMODE_8;
		is_16_packet = false;
		dovi_error("bpp error %d\n", bpp);
		return;
	}

	if (fmt == VIDEOIN_FORMAT_420)
		is_444 = false;
	else if (fmt == VIDEOIN_FORMAT_422)
		is_444 = false;
	else if (fmt == VIDEOIN_FORMAT_444)
		is_444 = true;
	else if (fmt > VIDEOIN_FORMAT_444) {
		dovi_error("color fmt error %d\n", fmt);
		return;
	}

	if (src > VIDEOIN_SRC_SEL_DOLBY3) {
		dovi_error("src select error %d\n", src);
		return;
	}

	if (!allocated) {
		y_addr_va =
		dma_alloc_coherent(dovi_dev, dump_size, &y_addr_pa, GFP_KERNEL);

		cb_addr_va =
		dma_alloc_coherent(dovi_dev, dump_size, &cb_addr_pa, GFP_KERNEL);

		cr_addr_va =
		dma_alloc_coherent(dovi_dev, dump_size, &cr_addr_pa, GFP_KERNEL);

		allocated = true;
	}

	dovi_default("y_addr_va %p 0x%x\n", y_addr_va, (uint32_t)y_addr_pa);
	dovi_default("cb_addr_va %p 0x%x\n", cb_addr_va, (uint32_t)cb_addr_pa);
	dovi_default("cr_addr_va %p 0x%x\n", cr_addr_va, (uint32_t)cr_addr_pa);

	is_uv_swap = false;

	if (height == 480) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xC4;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x80;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;
		if (is_progressive == true) {
			v_odd_start = 0x2A;
			v_even_start = 0x2A;
		} else {
			v_odd_start = 0x16;
			v_even_start = 0x16;
		}
	} else if (height == 576) {
		h_start = 0xEC;
		v_odd_start = 0x2C;
		v_even_start = 0x2C;
	} else if (height == 720) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xC4;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x10C;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;
		v_odd_start = 0x19;
		v_even_start = 0x19;
	} else if (height == 1080) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0xC9;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0x45;
		else if (src == VIDEOIN_SRC_SEL_HDMI)
			h_start = 0xBF;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0xC8;
		else if (src == VIDEOIN_SRC_SEL_VDO_MAIN)
			h_start = 0xBE;
		else
			h_start = 0xBE;

		if (is_progressive == true) {
			v_odd_start = 0x29;
			v_even_start = 0x29;
		} else {
			v_odd_start = 0x15;
			v_even_start = 0x15;
		}
	} else if (height == 2160) {
		if (src == VIDEOIN_SRC_SEL_FMT)
			h_start = 0x189;
		else if (src == VIDEOIN_SRC_SEL_OSD)
			h_start = 0xE0;
		else if (src == VIDEOIN_SRC_SEL_DOLBY3)
			h_start = 0x126;
		else
			h_start = 0x17F; /* RGB2HDMI */

		v_odd_start = 0x52;
		v_even_start = 0x52;
	}

	if (h_start%2)
		is_uv_swap = true;

	if (is_uv_swap) {
		cb_sel = SEL_CR_CHN;
		cr_sel = SEL_CB_CHN;
	} else {
		cb_sel = SEL_CB_CHN;
		cr_sel = SEL_CR_CHN;
	}

	videoin_hal_enable(enable);

	if (enable) {
		videoin_hal_update_addr(y_addr_pa, cb_addr_pa, cr_addr_pa, is_444);
		vdout_sys_hal_videoin_source_sel(src);
		videoin_hal_set_h(width, htotal, is_444, is_16_packet, bit_mode);
		videoin_hal_set_v(height, fmt);
		videoin_hal_set_color_format(fmt);
		videoin_hal_set_channel_select(SEL_Y_CHN, cb_sel, cr_sel);
		videoin_hal_set_bitmode(bit_mode, is_16_packet);
		videoin_hal_demode_enable(false);
		videoin_hal_set_active_zone(h_start, v_odd_start, v_even_start);
	}
}

void disp_dovi_show_input_buf_status(uint32_t vdp_id)
{
	if (vdp_id >= MAX_VDO_ID) {
		dovi_error("vdp_id %d error\n", vdp_id);
		return;
	}
	dovi_default("allocated %d\n", allocated[vdp_id]);
	dovi_default("y_addr_va %p 0x%x\n", y_addr_va[vdp_id], (uint32_t)y_addr_pa[vdp_id]);
	dovi_default("cb_addr_va %p 0x%x\n", c_addr_va[vdp_id], (uint32_t)c_addr_pa[vdp_id]);
	dovi_default("y_len_addr_va %p 0x%x\n", y_len_addr_va[vdp_id],
	(uint32_t)y_len_addr_pa[vdp_id]);
	dovi_default("c_len_addr_va %p 0x%x\n", c_len_addr_va[vdp_id],
	(uint32_t)c_len_addr_pa[vdp_id]);
	dovi_default("graphic_addr_va %p 0x%x\n", graphic_addr_va[vdp_id],
	(uint32_t)graphic_addr_pa[vdp_id]);

	dovi_default("graphic_header_va %p 0x%x\n",
	graphic_header_va[vdp_id],
	(uint32_t)graphic_header_pa[vdp_id]);
}

void disp_dovi_gen_graphic_header(uint32_t *va, dma_addr_t graphic_pa)
{
	uint32_t graphic_header_size = 48;
	uint32_t idx = 0;

	dovi_default("gen header va %p pa 0x%x\n", va, (uint32_t)graphic_pa);

	memcpy(va, graphic_header, graphic_header_size);

	va[1] &= 0xFF000000;
	va[1] |= ((graphic_pa & 0x0FFFFFFF) >> 4);

	va[6] &= (~(0x3 << 2));
	va[6] |= (((graphic_pa & 0xC0000000) >> 30) << 2);

	va[7] &= (~(0x3 << 23));
	va[7] |= (((graphic_pa & 0x30000000) >> 28) << 23);

	for (idx = 0; idx < graphic_header_size/4; idx++)
		dovi_default("va[%d] 0x%x\n", idx, va[idx]);

}

void disp_dovi_alloc_input_buf(uint32_t vdp_id, bool enable)
{
	uint32_t graphic_size = 4096*2160*4;
	uint32_t video_size = 4096*2176*5/4;
	uint32_t graphic_header_size = 48;

	if (vdp_id >= MAX_VDO_ID) {
		dovi_error("vdp_id %d error\n", vdp_id);
		return;
	}

	if (!allocated[vdp_id] && enable) {
		y_addr_va[vdp_id] =
		dma_alloc_coherent(dovi_dev, video_size, &y_addr_pa[vdp_id], GFP_KERNEL);

		c_addr_va[vdp_id] =
		dma_alloc_coherent(dovi_dev, video_size, &c_addr_pa[vdp_id], GFP_KERNEL);

		y_len_addr_va[vdp_id] = dma_alloc_coherent(dovi_dev,
		video_size/256, &y_len_addr_pa[vdp_id], GFP_KERNEL);

		c_len_addr_va[vdp_id] = dma_alloc_coherent(dovi_dev,
		video_size/256, &c_len_addr_pa[vdp_id], GFP_KERNEL);

		graphic_addr_va[vdp_id] = dma_alloc_coherent(dovi_dev,
		graphic_size, &graphic_addr_pa[vdp_id], GFP_KERNEL);

		graphic_header_va[vdp_id] = dma_alloc_coherent(dovi_dev,
		graphic_header_size, &graphic_header_pa[vdp_id], GFP_KERNEL);

		disp_dovi_gen_graphic_header(graphic_header_va[vdp_id],
		graphic_addr_pa[vdp_id]);

		allocated[vdp_id] = true;
	} else if (allocated[vdp_id] && !enable) {
		dma_free_coherent(dovi_dev, video_size, y_addr_va[vdp_id],
		y_addr_pa[vdp_id]);
		dma_free_coherent(dovi_dev, video_size, c_addr_va[vdp_id],
		c_addr_pa[vdp_id]);
		dma_free_coherent(dovi_dev,
		video_size/256, y_len_addr_va[vdp_id], y_len_addr_pa[vdp_id]);
		dma_free_coherent(dovi_dev,
		video_size/256, c_len_addr_va[vdp_id], c_len_addr_pa[vdp_id]);

		dma_free_coherent(dovi_dev,
		graphic_size,
		graphic_addr_va[vdp_id],
		graphic_addr_pa[vdp_id]);

		dma_free_coherent(dovi_dev,
		graphic_header_size,
		graphic_header_va[vdp_id],
		graphic_header_pa[vdp_id]);

		allocated[vdp_id] = false;
	}

	disp_dovi_show_input_buf_status(vdp_id);
}

void disp_dovi_load_video_pattern(uint32_t vdp_id, uint32_t pattern,
enum dovi_clr_fmt color_fmt)
{
	if (vdp_id >= MAX_VDO_ID) {
		dovi_error("vdp_id %d error\n", vdp_id);
		return;
	}

	if (!allocated[vdp_id]) {
		disp_dovi_alloc_input_buf(vdp_id, true);
	}

	if (allocated[vdp_id]) {
		char y_file_name[MAX_FILE_NAME];
		char c_file_name[MAX_FILE_NAME];
		uint32_t file_size = 0;
		uint32_t src_width = 0;
		uint32_t src_height = 0;

		struct vdp_hal_fb_addr fb_addr;
		struct vdp_hal_color_format clr_fmt;

		memset(y_file_name, 0, MAX_FILE_NAME);
		memset(c_file_name, 0, MAX_FILE_NAME);

		memset((void *)&clr_fmt, 0, sizeof(struct vdp_hal_color_format));

		src_width = pattern_res[pattern].width;
		src_height = pattern_res[pattern].height;

		if ((color_fmt >= BLK_8B_420_NON_UFO)
		|| (color_fmt >= BLK_8B_420_UFO)
		|| (color_fmt >= BLK_10B_420_NON_UFO)
		|| (color_fmt >= BLK_10B_420_UFO))
			src_height = DOVI_ALIGN_TO(src_height, 32);

		file_size = src_width * src_height;

		if (color_fmt >= BLK_10B_420_NON_UFO)
			file_size = file_size * 5 / 4;

		sprintf(y_file_name, "/sdcard/vdp/%d/%s", pattern_idx[pattern],
		"pic_0_Y.out");
		sprintf(c_file_name, "/sdcard/vdp/%d/%s", pattern_idx[pattern],
		"pic_0_CbCr.out");

		disp_dovi_load_buffer(y_file_name,
		(unsigned char *)y_addr_va[vdp_id],
		file_size);

		disp_dovi_load_buffer(c_file_name,
		(unsigned char *)c_addr_va[vdp_id],
		file_size/2);

		fb_addr.addr_y = y_addr_pa[vdp_id];
		fb_addr.addr_c = c_addr_pa[vdp_id];
		fb_addr.addr_y_len = y_len_addr_pa[vdp_id];
		fb_addr.addr_c_len = c_len_addr_pa[vdp_id];

		if (color_fmt >= BLK_10B_420_NON_UFO)
			clr_fmt.is_10bit = 1;

		if (color_fmt == RST_10B_420_NON_UFO
		|| color_fmt == RST_8B_420_NON_UFO)
			clr_fmt.is_scan_line = 1;

		if (color_fmt == BLK_8B_420_UFO
		|| color_fmt == BLK_10B_420_UFO)
			clr_fmt.is_ufo = 1;

		if (vdp_id == 1)
			disp_dovi_enable_vdp(vdp_id, pattern);

		vdp_hal_set_ufo_addr(vdp_id, &fb_addr);
		vdp_hal_set_src_color_format(vdp_id, &clr_fmt);
	} else
		dovi_error("buffer not allocated yet!!!!\n");

	disp_dovi_show_input_buf_status(vdp_id);
}

void disp_dovi_enable_vdp(uint32_t vdp_id, uint32_t pattern)
{
	enum FMT_TV_TYPE tv_type;
	int h_start, v_start_odd, v_start_even;
	struct fmt_active_info active_info;
	uint32_t src_width;
	uint32_t src_height;
	uint32_t dst_width, dst_height;
	uint32_t h_factor;
	struct vdp_hal_region src_region;
	struct vdp_hal_region out_region;


	if (dovi_res.frequency == 25 || dovi_res.frequency == 50)
		tv_type = FMT_TV_TYPE_PAL;
	else
		tv_type = FMT_TV_TYPE_NTSC;

	vdp_hal_init(vdp_id);

	fmt_hal_clock_on_off(DISP_FMT_SUB, true);
	fmt_hal_set_mode(DISP_FMT_SUB, dovi_res.res_mode, true);
	disp_path_set_delay(DISP_PATH_SVDO, dovi_res.res_mode);
	disp_path_set_delay(DISP_PATH_SVDO_OUT, dovi_res.res_mode);
	fmt_hal_set_tv_type(DISP_FMT_SUB, tv_type);

	disp_path_get_active_zone(vdp_id, dovi_res.res_mode,
				&h_start, &v_start_odd, &v_start_even);

	src_width = pattern_res[pattern].width;
	src_height = pattern_res[pattern].height;
	dst_width = dovi_res.width;
	dst_height = dovi_res.height;
	h_factor =  src_width * DISPFMT_H_FACTOR / dst_width;

	src_region.x = src_region.y = 0;
	src_region.width = src_width;
	src_region.height = src_height;

	out_region.x = out_region.y = 0;
	out_region.width = dst_width;
	out_region.height = dst_height;

	active_info.h_begine = h_start;
	active_info.h_end = active_info.h_begine + dst_width - 1;
	active_info.v_odd_begine = v_start_odd;
	active_info.v_odd_end = v_start_odd + dst_height - 1;
	active_info.v_even_begine = v_start_even;
	active_info.v_even_end = v_start_even + dst_height - 1;

	fmt_hal_set_pixel_factor(DISP_FMT_SUB, src_width,
				h_factor, 1, false);
	fmt_hal_set_active_zone(DISP_FMT_SUB, &active_info);

	if (fgFMTis4k2k(dovi_res.res_mode)) {
		active_info.h_begine += FMT_4K_DELAY;
		active_info.h_end += FMT_4K_DELAY;
	}

	fmt_hal_set_active_zone(VDOUT_FMT_SUB, &active_info);

	fmt_hal_enable(DISP_FMT_SUB, true);

	fmt_hal_hw_shadow_enable(DISP_FMT_SUB);

	disp_clock_enable(DISP_CLK_VDO4, true);

	vdp_hal_set_enable(vdp_id, 1);
	vdp_hal_set_src_size(vdp_id, src_width, src_height);
	vdp_hal_set_disp_config(vdp_id, &src_region, &out_region);


	fmt_hal_mix_plane(FMT_HW_PLANE_2);
	disp_sys_hal_video_ultra_en(DISP_SUB, true);
	disp_clock_smi_larb_en(DISP_SMI_LARB6, true);

}
