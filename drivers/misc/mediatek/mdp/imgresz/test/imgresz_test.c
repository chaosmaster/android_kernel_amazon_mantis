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

#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/io.h>
#include "../imgresz.h"
#include "../imgresz_hal.h"


#if IMGRZ_TEE_ENABLE
KREE_SECUREMEM_HANDLE imgrz_test_sec_src_mem;
KREE_SECUREMEM_HANDLE imgrz_test_sec_dst_mem;
#endif
unsigned int imgrz_test_buffer_pa;
unsigned int imgrz_test_buffer_size;
unsigned long imgrz_test_buffer_va;

static char dbg_buf[2048];
static char STR_HELP[] =
	"USAGE:\n"
	"       echo [ACTION]>/d/imgresz\n"
	"ACTION:\n";
static int debug_init;
static struct dentry *debugfs;

int mdp_get_resz_callback(IMGRESZ_TICKET ti, enum imgresz_cb_state hw_state,
			  void *privdata)
{
	pr_info("hw done %u, priv 0x%p", hw_state, privdata);
	/* Send successful event. */
	/* Don't call imgresz_ticket_put() here.*/
	return 0;
}


void imgresz_source_init_ep(unsigned char *mem, size_t wid, size_t hei)
{
	unsigned int h = 0;
	unsigned char *cur_line = NULL;
	unsigned char byte = 0xfe;

	for (h = 0; h < hei; h++) {
		cur_line = mem + h * 1920;
		memset(cur_line, byte--, 1920);
		if (byte == 0)
			byte = 0xfe;
	}
}

static void imgresz_prepare_buf(struct imgresz_src_buf_info *src, struct imgresz_dst_buf_info *dst)
{
	unsigned int y;
	unsigned char *src_y_buf, *src_cb_buf;
	unsigned char *dst_y_buf, *dst_cb_buf;
	unsigned char *src_addr, *dst_addr;
	unsigned long page_offset = imgrz_test_buffer_va - imgrz_test_buffer_pa;

	switch (src->src_mode) {
	case IMGRESZ_SRC_COL_MD_420_RS:
	case IMGRESZ_SRC_COL_MD_420_BLK:
		src_y_buf = (unsigned char *)src->y_buf_addr + page_offset;
		src_cb_buf = (unsigned char *)src->cb_buf_addr + page_offset;
		dst_y_buf = (unsigned char *)dst->y_buf_addr + page_offset;
		dst_cb_buf = (unsigned char *)dst->c_buf_addr + page_offset;
		pr_info("src(%p,%p) dst(%p,%p)\n", src_y_buf, src_cb_buf, dst_y_buf, dst_cb_buf);
		memset(src_y_buf, 0, src->buf_width*src->buf_height*3/2);
		memset(dst_y_buf, 1, dst->buf_width*dst->pic_height*3/2);

		for (y = 0; y < src->pic_height; y++) {
			src_addr = (unsigned char *)imgrz_test_buffer_va + y * src->pic_width;
			dst_addr = src_y_buf + y*src->buf_width;
			memcpy(dst_addr, src_addr, src->pic_width);
		}

		for (y = 0; y < src->pic_height; y += 2) {
			src_addr = (unsigned char *)imgrz_test_buffer_va + src->pic_width*src->pic_height
					+ y*src->pic_width/2;
			dst_addr = src_cb_buf + y*src->buf_width/2;
			memcpy(dst_addr, src_addr, src->pic_width);
		}
		break;
	default:
		break;
	}
}

static void imgresz_prepare_test_case(unsigned int test_case, unsigned int addition,
		struct imgresz_src_buf_info *src, struct imgresz_dst_buf_info *dst)
{
	unsigned int y_len, cb_len, cr_len;

	switch (test_case) {
	case 1:
		src->src_mode = IMGRESZ_SRC_COL_MD_420_RS;
		src->buf_width = 3840;
		src->buf_height = 2160;
		src->pic_width = 500;
		src->pic_height = 580;
		src->y_buf_addr = IMGALIGN(imgrz_test_buffer_pa, 1024);
		src->cb_buf_addr = IMGALIGN(src->y_buf_addr + src->buf_width*src->buf_height, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_RS;
		dst->buf_width = 3840;
		dst->pic_width = 1840;
		dst->pic_height = 320;
		dst->y_buf_addr = IMGALIGN(src->cb_buf_addr + src->buf_width*src->buf_height/2, 1024);
		dst->c_buf_addr = IMGALIGN(dst->y_buf_addr + dst->buf_width*dst->pic_height, 1024);

		imgresz_prepare_buf(src, dst);
		break;
	case 2: /* 240*144 ufo jump 10bit out */
		src->src_mode = IMGRESZ_SRC_COL_MD_420_BLK;
		src->buf_width = 3840;
		src->buf_height = 2176;
		src->pic_width = 3840;
		src->pic_height = 2160;
		src->ufo_type = IMGRESZ_UFO_10BIT_COMPACT;
		src->ufo_jump = false;
		src->y_buf_addr = IMGALIGN(imgrz_test_buffer_pa, 1024);
		src->ufo_ybuf_len = IMGALIGN(src->buf_width*src->buf_height*5/4, 1024);
		src->cb_buf_addr = IMGALIGN(src->y_buf_addr + src->ufo_ybuf_len, 1024);
		src->ufo_ylen_buf = IMGALIGN(src->cb_buf_addr + src->ufo_ybuf_len/2, 1024);
		src->ufo_ylen_buf_len = src->buf_width*src->buf_height/16/4/4;
		src->ufo_clen_buf =	IMGALIGN(src->ufo_ylen_buf + src->ufo_ylen_buf_len, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_BLK;
		dst->pic_width = 1920;
		dst->pic_height = 1088;
		dst->buf_width = IMGALIGN(dst->pic_width, 64);
		dst->y_buf_addr = IMGALIGN(src->ufo_clen_buf + src->ufo_ylen_buf_len/2, 1024);
		dst->c_buf_addr = IMGALIGN(dst->y_buf_addr + dst->buf_width*dst->pic_height, 1024);
		break;
	case 3: /* ufo 8bit 2hw clip outstanding*/
		src->src_mode = IMGRESZ_SRC_COL_MD_420_BLK;
		src->buf_width = 3840;
		src->buf_height = 2176;
		src->pic_width = 3840;
		src->pic_height = 2160;
		src->ufo_type = IMGRESZ_UFO_8BIT;
		src->y_buf_addr = IMGALIGN(imgrz_test_buffer_pa, 1024);
		src->ufo_ybuf_len = IMGALIGN(src->buf_width*src->buf_height, 1024);
		src->cb_buf_addr = IMGALIGN(src->y_buf_addr + src->ufo_ybuf_len, 1024);
		src->ufo_ylen_buf = IMGALIGN(src->cb_buf_addr + src->ufo_ybuf_len/2, 1024);
		src->ufo_ylen_buf_len = src->buf_width*src->buf_height/16/4/4;
		src->ufo_clen_buf =	IMGALIGN(src->ufo_ylen_buf + src->ufo_ylen_buf_len, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_RS;
		dst->pic_width = 1920;
		dst->pic_height = 1088;
		dst->buf_width = IMGALIGN(dst->pic_width, 64);
		dst->y_buf_addr = IMGALIGN(src->ufo_clen_buf + src->ufo_ylen_buf_len/2, 1024);
		dst->c_buf_addr = IMGALIGN(dst->y_buf_addr + dst->buf_width*dst->pic_height, 1024);
		break;
	case 4:
		src->src_mode = IMGRESZ_SRC_COL_MD_JPG_DEF;
		src->buf_width = 384;
		src->buf_height = 560;
		src->pic_width = 370;
		src->pic_height = 560;
		src->jpg_comp.y_comp_sample_h = 1;
		src->jpg_comp.y_comp_sample_v = 1;
		src->jpg_comp.cb_comp_sample_h = 1;
		src->jpg_comp.cb_comp_sample_v = 1;
		src->jpg_comp.cr_comp_sample_h = 1;
		src->jpg_comp.cr_comp_sample_v = 1;
		src->y_buf_addr = IMGALIGN(imgrz_test_buffer_pa, 1024);
		y_len = IMGALIGN(src->buf_width*src->buf_height, 1024);
		src->cb_buf_addr = IMGALIGN(src->y_buf_addr + y_len, 1024);
		cb_len = IMGALIGN((src->buf_width*src->buf_height
			*src->jpg_comp.cb_comp_sample_h*src->jpg_comp.cb_comp_sample_v)
			/(src->jpg_comp.y_comp_sample_h*src->jpg_comp.y_comp_sample_v), 1024);
		src->cr_buf_addr = IMGALIGN(src->cb_buf_addr + cb_len, 1024);
		cr_len = IMGALIGN((src->buf_width*src->buf_height
			*src->jpg_comp.cr_comp_sample_h*src->jpg_comp.cr_comp_sample_v)
			/(src->jpg_comp.y_comp_sample_h*src->jpg_comp.y_comp_sample_v), 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_RS;
		dst->pic_width = 800;
		dst->pic_height = 528;
		dst->buf_width = IMGALIGN(dst->pic_width, 16);
		dst->y_buf_addr = IMGALIGN(src->cr_buf_addr + cr_len, 1024);
		dst->c_buf_addr = IMGALIGN(dst->y_buf_addr + dst->buf_width*dst->pic_height, 1024);
		break;
	case 5:
		src->src_mode = IMGRESZ_SRC_COL_MD_ARGB_8888;
		src->buf_width = 2880;
		src->buf_height = 576;
		src->pic_width = 720;
		src->pic_height = 576;
		src->y_buf_addr = IMGALIGN(imgrz_test_buffer_pa, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_ARGB_8888;
		dst->buf_width = 5120;
		dst->pic_width = 1280;
		dst->pic_height = 720;
		dst->y_buf_addr = IMGALIGN(src->y_buf_addr + src->buf_width*src->buf_height, 1024);
		break;
	case 81:
		src->src_mode = IMGRESZ_SRC_COL_MD_420_RS;
		src->buf_width = 3840;
		src->buf_height = 2160;
		src->pic_width = 500;
		src->pic_height = 580;
		src->buf_info.mem_type = IMGRZ_MEM_SECURE;
		src->buf_info.fd = imgrz_test_sec_src_mem;
		src->buf_info.y_offset = 0;
		src->buf_info.cb_offset = IMGALIGN(src->buf_width*src->buf_height, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_RS;
		dst->buf_width = 3840;
		dst->pic_width = 1840;
		dst->pic_height = 320;
		dst->buf_info.mem_type = IMGRZ_MEM_SECURE;
		dst->buf_info.fd = imgrz_test_sec_dst_mem;
		dst->buf_info.y_offset = 0;
		dst->buf_info.cb_offset = IMGALIGN(dst->buf_width*dst->pic_height, 1024);
		break;
	case 83: /* ufo 8bit 2hw clip outstanding*/
		src->src_mode = IMGRESZ_SRC_COL_MD_420_BLK;
		src->buf_width = 3840;
		src->buf_height = 2176;
		src->pic_width = 3840;
		src->pic_height = 2160;
		src->ufo_type = IMGRESZ_UFO_8BIT;
		src->buf_info.mem_type = IMGRZ_MEM_SECURE;
		src->buf_info.fd = imgrz_test_sec_src_mem;
		src->buf_info.y_offset = 0;
		src->buf_info.cb_offset = IMGALIGN(src->buf_width*src->buf_height, 1024);
		src->buf_info.ylen_offset = IMGALIGN(src->buf_info.cb_offset*3/2, 1024);
		src->buf_info.clen_offset = IMGALIGN(src->buf_info.ylen_offset +
				src->buf_width*src->buf_height/16/4/4, 1024);

		dst->dst_mode = IMGRESZ_DST_COL_MD_420_RS;
		dst->pic_width = 1920;
		dst->pic_height = 1088;
		dst->buf_width = IMGALIGN(dst->pic_width, 64);
		dst->buf_info.mem_type = IMGRZ_MEM_SECURE;
		dst->buf_info.fd = imgrz_test_sec_dst_mem;
		dst->buf_info.y_offset = 0;
		dst->buf_info.cb_offset = IMGALIGN(dst->buf_width*dst->pic_height, 1024);
		break;
	default:
		break;
	}
}

static void imgresz_test(unsigned int test_case, unsigned int addition)
{
	IMGRESZ_TICKET ti;
	struct imgresz_src_buf_info src;
	struct imgresz_dst_buf_info dst;
	int ret;

	memset(&src, 0, sizeof(src));
	memset(&dst, 0, sizeof(dst));
/*	memset((void *)imgrz_test_buffer_va, 1, imgrz_test_buffer_size); */
	imgresz_prepare_test_case(test_case, addition, &src, &dst);
	ti = imgresz_ticket_get(((test_case&0xf) == 3 || (test_case&0xf) == 2) ?
			IMGRESZ_FUN_UFO_2HW : IMGRESZ_FUN_NORMAL);
	if (ti < 0)
		return;
	ret = imgresz_set_scale_mode(ti, IMGRESZ_FRAME_SCALE);
	ret = imgresz_set_src_bufinfo(ti, &src);/* set src info */
	ret = imgresz_set_dst_bufinfo(ti, &dst);/* set src info */

	imgresz_trigger_scale_async(ti, false);

	msleep(1000);
	imgresz_ticket_put(ti);
}

static void imgresz_process_dbg_opt(const char *opt)
{
	int ret = 0;

	if (strncmp(opt, "test ", 4) == 0) {
		unsigned int test_case = 0;

		ret = sscanf(opt, "test %d", &test_case);
		if (ret != 1) {
			pr_info("error to parse cmd %s, ret=%d\n", opt, ret);
			goto Error;
		}
		pr_info("test_case=%d\n", test_case);

		imgresz_test(test_case, 0);
		return;
	}
Error:
	pr_info("%s\n", STR_HELP);
}


static void imgresz_process_dbg_cmd(char *cmd)
{
	pr_info("cmd: %s\n", cmd);
	memset(dbg_buf, 0, sizeof(dbg_buf));
	imgresz_process_dbg_opt(cmd);
}

/* --------------------------------------------------------------------------- */
/* Debug FileSystem Routines */
/* --------------------------------------------------------------------------- */

static int imgresz_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static char cmd_buf[512];

static ssize_t imgresz_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(dbg_buf))
		return simple_read_from_buffer(ubuf, count, ppos, dbg_buf, strlen(dbg_buf));
	else
		return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));

}

static ssize_t imgresz_debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&cmd_buf, ubuf, count))
		return -EFAULT;

	cmd_buf[count] = 0;

	imgresz_process_dbg_cmd(cmd_buf);

	return ret;
}

static const struct file_operations debug_fops = {
	.read = imgresz_debug_read,
	.write = imgresz_debug_write,
	.open = imgresz_debug_open,
};

void imgresz_debug_init(void)
{
	if (!debug_init) {
		debug_init = 1;
		debugfs = debugfs_create_file("imgresz",
					      S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);

		pr_info("imgresz debug init, fs= %p\n", debugfs);
	}
}

void imgresz_debug_deinit(void)
{
	debugfs_remove(debugfs);
}
