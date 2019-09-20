/*
 * Copyright (c) 2017 MediaTek Inc.
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

#include <linux/debugfs.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/module.h>
#include "nr_hal.h"
#include "di_hal.h"
#include "mtk_vq_mgr.h"
#include "ion_drv.h"
#include "mtk_ion.h"
#include "vq_def.h"
/* ------------------------------------------- */
/* Debug Options */
/* -------------------------------------------- */
static char STR_HELP[] =
	"USAGE\n"
		"echo [ACTION]... > mtkvq\n"
	"ACTION\n";

/* ---------------------------------------------- */
/* Command Processor */
/* ---------------------------------------------- */
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
bool allocSecureBuffer(KREE_SESSION_HANDLE session, KREE_SECUREMEM_HANDLE *handle, int size)
{
	TZ_RESULT ret = TZ_RESULT_SUCCESS;

	ret = KREE_AllocSecurechunkmemWithTag(session, handle, 512, size, "vq");
	if (ret != TZ_RESULT_SUCCESS)
		pr_info("[VQ] alloc secure bufer fail\n");
	else
		pr_info("[VQ] alloc success with handle %d, size 0x%x\n", *handle, size);

	return ret;
}

static KREE_SESSION_HANDLE mem_session;
#endif

int nr_test_get_fd(struct ion_client *client, bool en)
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
			memset(debug_va, (i+1) * 40, 720 * 120);
			debug_va += 720 * 120;
		}
	}

	return fd;
}

void nrStartTest(void)
{
	struct mtk_vq_config config;
	int src_fd = -1, dst_fd = -1;
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	TZ_RESULT ret = TZ_RESULT_SUCCESS;
#endif
	struct vq_data *data = mtk_vq_get_data();

	memset((void *)&config, 0, sizeof(config));

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	if (mtk_vq_get_secure_debug_enable()) {
		if (!mem_session) {
			ret = KREE_CreateSession(TZ_TA_MEM_UUID, &mem_session);
			if (ret != TZ_RESULT_SUCCESS)
				pr_info("create memory session fail:%d\n", ret);
		}
		allocSecureBuffer(mem_session, &src_fd, 1920 * 1088 * 2);
		allocSecureBuffer(mem_session, &dst_fd, 1920 * 1088 * 2);
		config.secruity_en = 1;
	} else
#endif
	{
		src_fd = nr_test_get_fd(data->client, true);
		dst_fd = nr_test_get_fd(data->client, false);
	}

	pr_info("[VQ]src fd %d dst fd %d\n", src_fd, dst_fd);

	config.dst_fmt = VQ_COLOR_FMT_420BLK;
	config.src_fmt = VQ_COLOR_FMT_420BLK;

	config.dst_fd = dst_fd;
	config.src_fd[0] = src_fd;

	config.bnr_level = 3;
	config.mnr_level = 3;
	config.vq_mode = VQ_NR_STANDALONE;

	config.src_width = 720;
	config.src_height = 480;
	config.src_align_width = 720;
	config.src_align_height = 480;

	config.src_ofset_y_len[0] = 0;
	config.src_ofset_c_len[0] = 1920 * 1088;

	config.dst_ofset_y_len = 0;
	config.dst_ofset_c_len = 1920 * 1088;

	config.vq_mode = VQ_NR_STANDALONE;

	mtk_vq_mgr_set_input_buffer(data, &config);

	pr_info("[NR] nrStartTest end\n");
}


void diTest(const char *opt)
{
	unsigned int vq_mode = 0;
	unsigned int di_mode = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int h265_enable = 0;
	int i = 0;
	unsigned int top_first = 0;
	unsigned int top_current = 0;
	int ret = 0;
	struct mtk_vq_config_info config_info;
	struct mtk_vq_config config;

	char *p = (char *)opt + 3;

	pr_info("[VQ] di test\n");
	ret =
	    sscanf(p, "%d %d %d %d %d %d %d", &vq_mode, &di_mode, &width, &height, &h265_enable,
		   &top_first, &top_current);
	if (ret != 7 && ret != 6) {
		pr_info
		    ("vq_mode=%d di_mode=%d,width=%d,height=%d,h265=%d,topfirst=%d curfield=%d\n",
		     vq_mode, di_mode, width, height, h265_enable, top_first, top_current);
		pr_info("error to parse cmd %s, ret=%d\n", opt, ret);
		goto error;
	}
	pr_info("vq_mode=%d di_mode=%d,width=%d,height=%d,h265=%d,topfirst=%d curfield=%d\n",
		vq_mode, di_mode, width, height, h265_enable, top_first, top_current);

	memset(&config, 0, sizeof(struct mtk_vq_config));

	config.vq_mode = vq_mode;
	config.di_mode = di_mode;
	config.h265_enable = h265_enable;
	config.src_width = width;
	config.src_height = height;
	config.topfield_first_enable = top_first;
	config.cur_field = top_current;
	config.src_align_width = width;
	config.src_align_height = height;

	for (i = 0; i < MTK_VQ_BUFFER_COUINT; ++i) {
		config.src_ofset_y_len[i] = 0;
		config.src_ofset_c_len[i] = 1920 * 1088;
	}

	config.bnr_level = 3;
	config.mnr_level = 3;
	config.dst_ofset_y_len = 0;
	config.dst_ofset_c_len = 1920 * 1088;

	config.src_fmt = VQ_COLOR_FMT_420BLK;
	config.dst_fmt = VQ_COLOR_FMT_422SCL;

	config_info.vq_config = &config;
	if (di_mode != VQ_DI_MODE_FRAME) {
		config_info.src_mva[0] = 0x50000000;
		config_info.src_mva[1] = 0x51000000;
		config_info.src_mva[2] = 0x52000000;
		config_info.src_mva[3] = 0x53000000;
		config_info.dst_mva = 0x60000000;

	} else {
		config_info.src_mva[0] = 0x50000000;
		config_info.src_mva[1] = 0x50000000;
		config_info.src_mva[2] = 0x50000000;
		config_info.src_mva[3] = 0x50000000;
		config_info.dst_mva = 0x60000000;
	}
	mtk_vq_power_on(mtk_vq_get_data(), VQ_DI_STANDALONE);
	di_hal_config(&config_info);
	mtk_vq_power_off(mtk_vq_get_data(), VQ_DI_STANDALONE);

error:
	pr_info("Parse command error!\n\n%s", STR_HELP);

}

static void process_dbg_opt(const char *opt)
{
	pr_info("[VQ] process_dbg_opt\n");

	if (strncmp(opt, "logen:", 6) == 0) {
		char *p = (char *)opt + 6;
		int logen = 0;

		if (kstrtoul(p, 16, (unsigned long int *)&logen))
			goto error;

		mtk_vq_set_log_enable(logen);

		pr_info("[VQ] log level is %d\n", logen);
	} else if (strncmp(opt, "time:", 5) == 0) {
		char *p = (char *)opt + 5;
		int time_en = 0;

		if (kstrtoul(p, 16, (unsigned long int *)&time_en))
			goto error;

		mtk_vq_set_timer_enable(time_en);

		pr_info("[VQ] timer enable is %d\n", time_en);
	} else if (strncmp(opt, "nrtest", 6) == 0) {
		nrStartTest();
	} else if (strncmp(opt, "di", 2) == 0) {
		diTest(opt);
	}
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	else if (strncmp(opt, "secen:", 6) == 0) {
		char *p = (char *)opt + 6;
		int secureEn = 0;

		if (kstrtoul(p, 16, (unsigned long int *)&secureEn))
			goto error;

		mtk_vq_set_secure_debug_enable(secureEn);

		pr_info("[VQ] secure enable is %d\n", secureEn);
	}
#endif
	else
		goto error;

	return;

error:
	pr_info("[VQ]Parse command error!\n\n%s", STR_HELP);
}

static void process_dbg_cmd(char *cmd)
{
	char *tok;

	pr_info("[VQ][mtkvq_dbg] %s\n", cmd);
	while ((tok = strsep(&cmd, "\n")) != NULL)
		process_dbg_opt(tok);
}

static int vq_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t vq_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));
}

static ssize_t vq_debug_write(struct file *file, const char __user *ubuf,
			      size_t count, loff_t *ppos)
{
	static char vq_cmd_buf[512];
	const int debug_bufmax = sizeof(vq_cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&vq_cmd_buf, ubuf, count))
		return -EFAULT;

	vq_cmd_buf[count] = 0;

	process_dbg_cmd(vq_cmd_buf);

	return ret;
}

struct dentry *mtkvq_dbg;
static const struct file_operations debug_fops = {
	.read = vq_debug_read,
	.write = vq_debug_write,
	.open = vq_debug_open,
};

static int __init mtk_vq_debug_init(void)
{
	VQ_INFO("[VQ] %s\n", __func__);
	mtkvq_dbg = debugfs_create_file("mtkvq", S_IFREG | S_IRUGO |
					S_IWUSR | S_IWGRP, NULL, (void *)0, &debug_fops);

	return 0;
}

static void __exit mtk_vq_debug_deinit(void)
{
	debugfs_remove(mtkvq_dbg);
}
module_init(mtk_vq_debug_init);
module_exit(mtk_vq_debug_deinit);
