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

#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "disp_videoin_if.h"
#include "videoin_hal.h"
#include "ion_drv.h"
#include "mtk_ion.h"

static char dbg_buf[2048];
static char STR_HELP[] =
	"USAGE:\n"
	"    echo [ACTION] > /d/videoin\n"
	"ACTION:\n"
	"    echo pa_addr 0x50000000 > /d/videoin    set phyiscial addr\n"
	"    echo enable:1 > /d/videoin		0:disable 1:enable\n"
	"    echo ion_addr > /d/videoin		allocate ion buffer\n"
	"    echo get_ion_buffer > /d/videoin	get ion buffer\n";

static int debug_init;
static struct dentry *debugfs;

static void *buffer_va;
static unsigned int buffer_mva;


static int videoin_debug_get_ion_buffer(
	unsigned int *y_buffer,
	unsigned int *cb_buffer,
	unsigned int *cr_buffer)
{
	static struct ion_client *client;
	struct ion_handle *handle = NULL;
	struct ion_mm_data mm_data;
	int y_buffer_size = 1920 * 1080;
	int buffer_size = 1920 * 1080 * 4;

	if (buffer_mva)
		goto END;

	if (!client)
		client = ion_client_create(g_ion_device, "videoin_debug");

	handle = ion_alloc(client, buffer_size, 0, ION_HEAP_MULTIMEDIA_MASK, 0);
	if (IS_ERR(handle)) {
		pr_info("allocate ion handle fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	buffer_va = ion_map_kernel(client, handle);
	if (buffer_va == NULL) {
		pr_info("map va fail.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	mm_data.config_buffer_param.kernel_handle = handle;
	mm_data.config_buffer_param.module_id = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.coherent = 0;
	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	if (ion_kernel_ioctl(client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data) < 0) {
		pr_info("ion_test_drv: Config buffer failed.\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

	ion_phys(client, handle, (ion_phys_addr_t *) &buffer_mva, (size_t *) &buffer_size);
	if (buffer_mva == 0) {
		pr_info("Fatal Error, get mva failed\n");
		ion_free(client, handle);
		ion_client_destroy(client);
		return -1;
	}

END:
	*y_buffer = buffer_mva;
	*cb_buffer = buffer_mva + y_buffer_size * 1;
	*cr_buffer = buffer_mva + y_buffer_size * 2;
	pr_info("y_buffer(0x%x), cb_buffer(0x%x), y_va(%p), cb_va(%p)\n",
		*y_buffer, *cb_buffer, buffer_va, (buffer_va + y_buffer_size));

	return 0;
}

static void videoin_process_dbg_opt(const char *opt)
{
	int ret = 0;
	char *p;

	if (strncmp(opt, "pa_addr", 7) == 0) {
		unsigned int y_addr = 0;
		unsigned int cb_addr = 0;
		unsigned int cr_addr = 0;
		bool yuv444 = false;

		ret = sscanf(opt, "pa_addr 0x%x 0x%x 0x%x\n", &y_addr, &cb_addr, &cr_addr);
		pr_info("ret=%d, y_addr=0x%x, cb_addr=0x%x, cr_addr=0x%x\n", ret, y_addr, cb_addr, cr_addr);

		if (ret != 2) {
			pr_info("error to parse cmd %s, ret=%d\n", opt, ret);
			goto Error;
		}

		if (ret == 3)
			yuv444 = true;

		videoin_hal_set_m4u_port(false);
		videoin_hal_update_addr(y_addr, cb_addr, cr_addr, yuv444);
	} else if (strncmp(opt, "enable", 6) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 7;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			pr_info("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		pr_info("enable video in=%d\n", value);
		videoin_hal_enable(value);
	} else if (strncmp(opt, "ion_addr", 8) == 0) {
		unsigned int y_addr = 0;
		unsigned int cb_addr = 0;
		unsigned int cr_addr = 0;

		ret = videoin_debug_get_ion_buffer(&y_addr, &cb_addr, &cr_addr);
		if (ret) {
			pr_info("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		videoin_hal_set_m4u_port(true);
		videoin_hal_update_addr(y_addr, cb_addr, cr_addr, false);
	} else if (strncmp(opt, "get_ion_buffer", 14) == 0) {
		int size = 1920 * 1080;

		if (!buffer_mva) {
			pr_info("ion buffer has not be allocated");
			return;
		}
		pr_info("y_buffer=0x%x, cb_buffer=0x%x, y_va=%p, cb_va=%p\n",
			buffer_mva, buffer_mva + size, buffer_va, buffer_va + size);
	} else {
		goto Error;
	}

	return;
Error:
	pr_err("%s\n", STR_HELP);
}


static void videoin_process_dbg_cmd(char *cmd)
{
	pr_err("cmd: %s\n", cmd);
	memset(dbg_buf, 0, sizeof(dbg_buf));
	videoin_process_dbg_opt(cmd);
}

/* --------------------------------------------------------------------------- */
/* Debug FileSystem Routines */
/* --------------------------------------------------------------------------- */

static int videoin_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static char cmd_buf[512];

static ssize_t videoin_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(dbg_buf))
		return simple_read_from_buffer(ubuf, count, ppos, dbg_buf, strlen(dbg_buf));
	else
		return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));

}

static ssize_t videoin_debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&cmd_buf, ubuf, count))
		return -EFAULT;

	cmd_buf[count] = 0;

	videoin_process_dbg_cmd(cmd_buf);

	return ret;
}

static const struct file_operations debug_fops = {
	.read = videoin_debug_read,
	.write = videoin_debug_write,
	.open = videoin_debug_open,
};

void videoin_debug_init(void)
{
	if (!debug_init) {
		debug_init = 1;
		debugfs = debugfs_create_file("videoin",
					      S_IFREG | S_IRUGO, NULL, (void *)0, &debug_fops);

		pr_debug("video in debug init, fs= %p\n", debugfs);
	}
}

void videoin_debug_deinit(void)
{
	debugfs_remove(debugfs);
}


