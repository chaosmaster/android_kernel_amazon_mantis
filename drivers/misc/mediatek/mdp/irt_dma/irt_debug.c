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


#include <linux/debugfs.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "irt_if.h"
#include "ion_drv.h"
#include "mtk_ion.h"

/* ------------------------------------------- */
/* Debug Options */
/* -------------------------------------------- */
static const char STR_HELP[] =
	"USAGE\n"
		"echo [ACTION]... > /d/mtkirt\n"
	"ACTION\n";

/* ---------------------------------------------- */
/* Command Processor */
/* ---------------------------------------------- */

int irt_hw_done_callback(enum irt_cb_state hw_state, void *privdata)
{
	pr_info("irt_hw_done_callback %d\n", hw_state);

	if (hw_state == IRT_CB_TIMEOUT)
		pr_info("[IRT] error framedone timeout\n");

	return 0;
}

int irt_test_get_fd(struct ion_client *client, bool en)
{
	int fd = -1;
	int i;

	struct ion_handle *handle = NULL;
	unsigned int debug_size = 1920 * 1088 * 2;
	void *debug_va = NULL;

	handle = ion_alloc(client, debug_size, 0, ION_HEAP_MULTIMEDIA_MASK, 0);
	if (IS_ERR(handle)) {
		pr_info("[IRT]allocate ion handle fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	debug_va = ion_map_kernel(client, handle);
	if (debug_va == NULL) {
		pr_info("[IRT]map va fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	fd = ion_share_dma_buf_fd(client, handle);

	pr_info("[IRT] alloc va addr is 0x%lx fd %d\n", (unsigned long)debug_va, fd);

	if (en) {
		for (i = 0; i < 4; i++) {
			memset(debug_va, (i+1) * 60, 720 * 120);
			debug_va += 720 * 120;
		}
	}

	return fd;
}

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
bool irtAllocSecureBuffer(KREE_SESSION_HANDLE session, KREE_SECUREMEM_HANDLE *handle, int size)
{
	TZ_RESULT ret = TZ_RESULT_SUCCESS;

	ret = KREE_AllocSecurechunkmemWithTag(session, handle, 512, size, "irt");
	if (ret != TZ_RESULT_SUCCESS)
		pr_info("[VQ] alloc secure bufer fail\n");

	return ret;
}

static KREE_SESSION_HANDLE mem_session;

#endif
void irtStartTest(void)
{
	struct irt_dma_info irt_info;

	static struct ion_client *client;
	int src_fd = -1, dst_fd = -1;
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	TZ_RESULT ret = TZ_RESULT_SUCCESS;
#endif
	if (!client)
		client = ion_client_create(g_ion_device, "irt_debug");

	memset((void *)&irt_info, 0x0, sizeof(irt_info));

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	if (irt_get_secure_debug_enable()) {
		if (!mem_session) {
			ret = KREE_CreateSession(TZ_TA_MEM_UUID, &mem_session);
			if (ret != TZ_RESULT_SUCCESS)
				pr_info("create memory session fail:%d\n", ret);
		}
		irtAllocSecureBuffer(mem_session, &src_fd, 1920 * 1088 * 2);
		irtAllocSecureBuffer(mem_session, &dst_fd, 1920 * 1088 * 2);

		irt_info.secruity_en = true;
	} else
#endif
	{
		src_fd = irt_test_get_fd(client, true);
		dst_fd = irt_test_get_fd(client, false);
	}

	pr_info("[IRT]src fd %d dst fd %d\n", src_fd, dst_fd);

	irt_info.rotate_mode = IRT_DMA_MODE_ROTATE_90;
	irt_info.dither_mode = 0;
	irt_info.src_color_fmt = IRT_DMA_SRC_COL_MD_YC420_8BIT_SCL;
	irt_info.dst_color_fmt = IRT_DMA_DST_COL_MD_YC420_8BIT_SCL;

	irt_info.src_width_align = 720;
	irt_info.src_height_align = 480;

	irt_info.src_fd = src_fd;
	irt_info.dst_fd = dst_fd;

	irt_info.src_offset_y_len = 0;
	irt_info.src_offset_c_len = 1920 * 1088;
	irt_info.dst_offset_y_len = 0;
	irt_info.dst_offset_c_len = 1920 * 1088;

	irt_ticket_get();

	irt_dma_trigger_sync(&irt_info);

	irt_ticket_put();

	pr_info("[IRT] irtStartTest end\n");
}

static void process_dbg_opt(const char *opt)
{
	pr_info("[IRT] process_dbg_opt\n");

	if (strncmp(opt, "logen:", 6) == 0) {
		char *p = (char *)opt + 6;
		int logen = 0;

		if (kstrtoul(p, 16, (unsigned long int *)&logen))
			goto error;

		irt_set_log_enable(logen);

		pr_info("[IRT] log level is %d\n", logen);
	} else if (strncmp(opt, "irttest", 7) == 0) {
		irtStartTest();
	}
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	else if (strncmp(opt, "secen:", 6) == 0) {
		char *p = (char *)opt + 6;
		int sec_en = 0;

		if (kstrtoul(p, 16, (unsigned long int *)&sec_en))
			goto error;

		irt_set_secure_debug_enable(sec_en);

		pr_info("[IRT] debug enable is %d\n", sec_en);
	}
#endif
	else
		goto error;

	return;

error:
	pr_info("[IRT]Parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
	char *tok;

	pr_info("[mtkirt_dbg] %s\n", cmd);
	while ((tok = strsep(&cmd, " ")) != NULL)
		process_dbg_opt(tok);
}

static int irt_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t irt_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));
}

static ssize_t irt_debug_write(struct file *file, const char __user *ubuf,
			       size_t count, loff_t *ppos)
{
	static char irt_cmd_buf[512];
	const int debug_bufmax = sizeof(irt_cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&irt_cmd_buf, ubuf, count))
		return -EFAULT;

	irt_cmd_buf[count] = 0;

	process_dbg_cmd(irt_cmd_buf);

	return ret;
}

struct dentry *mtkirt_dbg;
static const struct file_operations debug_fops = {
	.read = irt_debug_read,
	.write = irt_debug_write,
	.open = irt_debug_open,
};

static int __init mtk_irt_debug_init(void)
{
	/*pr_info("[IRT] %s\n", __func__);*/
	mtkirt_dbg = debugfs_create_file("mtkirt", S_IFREG | S_IRUGO |
					 S_IWUSR | S_IWGRP, NULL, (void *)0, &debug_fops);

	return 0;
}

static void __exit mtk_irt_debug_deinit(void)
{
	debugfs_remove(mtkirt_dbg);
}
module_init(mtk_irt_debug_init);
module_exit(mtk_irt_debug_deinit);
