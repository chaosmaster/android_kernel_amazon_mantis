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

#include "dovi_type.h"
#include "dovi_log.h"
#include "disp_dovi_cmd.h"
#include "disp_dovi_io.h"
#include "dovi_table.h"
#include "disp_dovi_common_if.h"
#include "dovi_core1_hal.h"
#include "dovi_core2_hal.h"
#include "dovi_core3_hal.h"

#define LOG_TAG "DOVI_DEBUG"

static int dovi_dbg_init;
unsigned int dovi_dbg_level;
static struct dentry *dovi_debugfs;

static char dovi_dbg_buf[2048];
static char dovi_cmd_buf[512];

static const char DOVI_STR_HELP[] = "USAGE:\n"
"       echo [ACTION]>/d/dovi\n"
"ACTION:\n";


static void dovi_process_dbg_opt(const char *opt)
{
	int ret = 0;
	char *p;

	if (strncmp(opt, "log_level:", 10) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 10;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_dbg_level = value;
	} else if (strncmp(opt, "log_level_tz:", 13) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 13;
		ret = kstrtoul(p, 13, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_sec_debug_level_init(value);
	} else if (strncmp(opt, "status", 6) == 0) {
		disp_dovi_status(0);
	} else if (strncmp(opt, "dft:", 4) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 4;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dovi_printf("default path test %d.\n", value);
		/* disp_dovi_default_path(value); */
		disp_dovi_default_path_init(value);
	} else if (strncmp(opt, "dump_rpu:", 9) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 9, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dump_rpu_enable = value;
		/* only open the rpu file */
		disp_dovi_dump_rpu(true, NULL, 0);
		dovi_printf("set dump rpu enable %d.\n", value);
	} else if (strncmp(opt, "dump_md:", 8) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 8;
		ret = kstrtoul(p, 0, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dump_md_enable = value;
		dovi_printf("set dump md enable %d.\n", value);
	} else if (strncmp(opt, "core1_bypass_csc:", 17) == 0) {
		unsigned int bypass = 0;

		p = (char *)opt + 17;
		ret = kstrtoul(p, 0, (unsigned long int *)&bypass);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_core1_hal_bypass_csc(bypass);
	} else if (strncmp(opt, "core1_bypass_cvm:", 17) == 0) {
		unsigned int bypass = 0;

		p = (char *)opt + 17;
		ret = kstrtoul(p, 0, (unsigned long int *)&bypass);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_core1_hal_bypass_cvm(bypass);
	} else if (strncmp(opt, "core3_bypass_dither:", 20) == 0) {
		unsigned int bypass = 0;

		p = (char *)opt + 20;
		ret = kstrtoul(p, 0, (unsigned long int *)&bypass);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		dovi_core3_hal_dither_bypass(bypass);
	} else if (strncmp(opt, "vin_dump_bpp:", 13) == 0) {
		unsigned int dump_bpp = 0;

		p = (char *)opt + 13;
		ret = kstrtoul(p, 0, (unsigned long int *)&dump_bpp);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (dump_bpp == 10)
			idk_dump_bpp = VIDEOIN_BITMODE_10;
		else if (dump_bpp == 8)
			idk_dump_bpp = VIDEOIN_BITMODE_8;
		else if (dump_bpp == 12)
			idk_dump_bpp = VIDEOIN_BITMODE_12;
	} else if (strncmp(opt, "dovimute:", 9) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 0, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		dovi_mute = value;
		dovi_printf("set dovi_mute %d.\n", dovi_mute);
	} else if (strncmp(opt, "osdscale:", 9) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 0, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		fhd_scale_to_uhd = value;
		dovi_printf("set fhd_scale_to_uhd %d.\n", fhd_scale_to_uhd);
	} else if (strncmp(opt, "core3timing:", 12) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 12;
		ret = kstrtoul(p, 0, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		core3_select_external_timing = value;
		dovi_printf("set core3_select_external_timing %d.\n", core3_select_external_timing);
	} else if (strncmp(opt, "idk_dump:", 9) == 0) {
		unsigned int value1 = 0;
		unsigned int value2 = 0;

		p = (char *)opt + 9;
		STR_CVT(&p, &value1, uint, goto Error);
		STR_CVT(&p, &value2, uint, goto Error);


		dovi_idk_test = value1;
		dovi_idk_file_id = value2;
		dovi_printf("set dovi_idk_test %d, dovi_idk_test %d.\n",
			dovi_idk_test, dovi_idk_file_id);
		if (dovi_idk_test)
			disp_dovi_set_idk_info();
	} else if (strncmp(opt, "sdk_dump:", 9) == 0) {
		unsigned int value1 = 0;
		unsigned int value2 = 0;

		p = (char *)opt + 9;
		STR_CVT(&p, &value1, uint, goto Error);
		STR_CVT(&p, &value2, uint, goto Error);

		dovi_sdk_test = value1;

		if (value2 == 1) { /* UHD src -> UHD display */
			dovi_sdk_file_id = 1;
			fhd_scale_to_uhd = 0;
		} else if (value2 == 2) { /* HD src -> UHD display */
			dovi_sdk_file_id = 0;
			fhd_scale_to_uhd = 1;
		}
		dovi_printf("set test %d, file_id %d fhd_scale_to_uhd %d\n",
			value1, dovi_sdk_file_id, fhd_scale_to_uhd);
		disp_dovi_set_sdk_info();
	} else if (strncmp(opt, "gfx_max_lum:", 12) == 0) {
		unsigned int value1 = 0;
		unsigned int value2 = 0;

		p = (char *)opt + 12;
		STR_CVT(&p, &value1, uint, goto Error);
		STR_CVT(&p, &value2, uint, goto Error);

		set_graphic_max_lum_enable = value1;
		graphic_max_lum = value2;
		dovi_printf("set set_graphic_max_lum_enable %d, graphic_max_lum %d.\n",
			set_graphic_max_lum_enable, graphic_max_lum);
	} else if (strncmp(opt, "vdo_max_lum:", 12) == 0) {
		unsigned int value1 = 0;
		unsigned int value2 = 0;

		p = (char *)opt + 12;
		STR_CVT(&p, &value1, uint, goto Error);
		STR_CVT(&p, &value2, uint, goto Error);

		set_video_max_lum_enable = value1;
		video_max_lum = value2;
		dovi_printf("set set_video_max_lum_enable %d, video_max_lum %d.\n",
			set_video_max_lum_enable, video_max_lum);
	} else if (strncmp(opt, "hdmioutf:", 9) == 0) {
		unsigned int value1 = 0;
		unsigned int value2 = 0;

		p = (char *)opt + 9;
		STR_CVT(&p, &value1, uint, goto Error);
		STR_CVT(&p, &value2, uint, goto Error);
		dovi_printf("enable %d, format %d.\n",
			value1, value2);
	    disp_dovi_set_hdr_enable(value1, value2);
	} else if (strncmp(opt, "out_format:", 11) == 0) {
		unsigned int test_case_id = 0;
		unsigned int out_format = 0;
		uint32_t ret;

		p = (char *)opt + 11;
		STR_CVT(&p, &test_case_id, uint, goto Error);
		STR_CVT(&p, &out_format, uint, goto Error);

		get_dovi_out_format_status();
		ret = set_dovi_out_format(test_case_id, out_format);
		dovi_printf("set test_case_id %d out_format %d ret %d\n",
			test_case_id, out_format, ret);
	} else if (strncmp(opt, "dovi2hdr10:", 11) == 0) {
		unsigned int test_case_id = 0;
		unsigned int out_format = 0;
		uint32_t ret;

		p = (char *)opt + 11;
		STR_CVT(&p, &test_case_id, uint, goto Error);
		STR_CVT(&p, &out_format, uint, goto Error);

		get_dovi2hdr10_mapping_type_Status();
		ret = set_dovi2hdr10_mapping_type(test_case_id, out_format);
		dovi_printf("set test_case_id %d out_format %d ret %d\n",
			test_case_id, out_format, ret);
	} else if (strncmp(opt, "ll_mode:", 8) == 0) {
		unsigned int test_case_id = 0;
		unsigned int use_ll = 0;
		unsigned int ll_rgb_desired = 0;
		uint32_t ret;

		p = (char *)opt + 8;
		STR_CVT(&p, &test_case_id, uint, goto Error);
		STR_CVT(&p, &use_ll, uint, goto Error);
		STR_CVT(&p, &ll_rgb_desired, uint, goto Error);

		get_dovi_ll_mode_status();
		ret = set_dovi_ll_mode(test_case_id, use_ll, ll_rgb_desired);
		dovi_printf("set test_case_id %d use_ll %d ll_rgb_desired %d ret %d\n",
			test_case_id, use_ll, ll_rgb_desired, ret);
	} else if (strncmp(opt, "ll_format:", 10) == 0) {
		/* set low latency mode yuv(0) or rgb(1) */
		p = (char *)opt + 10;
		STR_CVT(&p, &ll_format, int, goto Error);
		dovi_printf("set ll_format %d\n", ll_format);
	} else if (strncmp(opt, "set_pri_mode:", 13) == 0) {
		unsigned int force_pri_mode = 0;
		unsigned int pri_mode = 0;

		p = (char *)opt + 13;
		STR_CVT(&p, &force_pri_mode, uint, goto Error);
		STR_CVT(&p, &pri_mode, uint, goto Error);

		set_dovi_priority_mode(force_pri_mode, pri_mode);
		dovi_set_priority_mode(pri_mode);
		dovi_printf("set force_pri_mode %d pri_mode %d\n",
			force_pri_mode, pri_mode);
	} else if (strncmp(opt, "get_pri_mode:", 13) == 0) {
		unsigned int pri_mode = 0;

		dovi_get_priority_mode(&pri_mode);
		dovi_printf("get pri_mode %d\n", pri_mode);
	} else if (strncmp(opt, "g_format:", 9) == 0) {
		unsigned int force_g_format = 0;
		unsigned int g_format = 0;

		p = (char *)opt + 9;
		STR_CVT(&p, &force_g_format, uint, goto Error);
		STR_CVT(&p, &g_format, uint, goto Error);

		set_dovi_g_format(force_g_format, g_format);
		dovi_printf("set force_g_format %d g_format %d\n",
			force_g_format, g_format);
	} else if (strncmp(opt, "reg_test:", 9) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		disp_dovi_common_test(value);

		dovi_printf("set reg_test option %d.\n", value);
	} else if (strncmp(opt, "dump_f:", 7) == 0) {
		bool enable;
		enum VIDEOIN_SRC_SEL src;
		enum VIDEOIN_YCbCr_FORMAT fmt;
		enum VIDEO_BIT_MODE bpp;

		p = (char *)opt + 7;

		STR_CVT(&p, &enable, uint, goto Error);
		STR_CVT(&p, &src, uint, goto Error);
		STR_CVT(&p, &fmt, uint, goto Error);
		STR_CVT(&p, &bpp, uint, goto Error);

		dovi_printf("set dump frame enable %d src %d fmt %d, bpp %d\n",
		enable, src, fmt, bpp);

		disp_dovi_dump_vin(enable, src, fmt, bpp);
	} else if (strncmp(opt, "debug:", 6) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 6;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			dovi_error("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (value == 1)
			dovi_error("test debug cmd pass.\n");
	} else if (strncmp(opt, "alloc:", 6) == 0) {
		uint32_t vdp_id;
		unsigned int enable = 0;

		p = (char *)opt + 6;
		STR_CVT(&p, &vdp_id, uint, goto Error);
		STR_CVT(&p, &enable, uint, goto Error);

		disp_dovi_alloc_input_buf(vdp_id, enable);
	} else if (strncmp(opt, "load:", 5) == 0) {
		unsigned int vdp_id;
		uint32_t pattern;
		enum dovi_clr_fmt color_fmt;

		p = (char *)opt + 5;
		STR_CVT(&p, &vdp_id, uint, goto Error);
		STR_CVT(&p, &pattern, uint, goto Error);
		STR_CVT(&p, &color_fmt, uint, goto Error);

		dovi_default("load vdp %d pattern %d fmt %d.\n",
		vdp_id, pattern, color_fmt);

		disp_dovi_load_video_pattern(vdp_id, pattern, color_fmt);
	} else if (strncmp(opt, "en_vdp:", 7) == 0) {
		unsigned int vdp_id;
		uint32_t enable;

		ret = sscanf(opt, "en_vdp:%d,%d\n", &vdp_id, &enable);
		if (ret != 1) {
			dovi_error("set vdp enable = %d\n", ret);
			goto Error;
		}

		dovi_default("vdp %d enable %d.\n",
		vdp_id, enable);
	} else if (strncmp(opt, "core2_rst", 9) == 0) {
		ret = sscanf(opt, "core2_rst 0x%x\n", &core2_reset_all);
		if (ret != 1) {
			dovi_error("set core2 reset all ret = %d\n", ret);
			goto Error;
		}

		dovi_default("set core2 reset all to 0x%x ret %d.\n",
		core2_reset_all, ret);
	} else
		dovi_unit_process_dbg_opt(opt);

	return;

Error:
	dovi_error("parse command error!%s\n", DOVI_STR_HELP);
}

static void dovi_process_dbg_cmd(char *cmd)
{

	dovi_printf("cmd: %s\n", cmd);
	memset(dovi_dbg_buf, 0, sizeof(dovi_dbg_buf));
	dovi_process_dbg_opt(cmd);
}

/* --------------------------------------------------------------------------- */
/* Debug FileSystem Routines */
/* --------------------------------------------------------------------------- */

static int dovi_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t dovi_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(dovi_dbg_buf))
		return simple_read_from_buffer(ubuf, count, ppos, dovi_dbg_buf,
					       strlen(dovi_dbg_buf));
	else
		return simple_read_from_buffer(ubuf, count, ppos, DOVI_STR_HELP,
					       strlen(DOVI_STR_HELP));

}

static ssize_t dovi_debug_write(struct file *file, const char __user *ubuf, size_t count,
				loff_t *ppos)
{
	const int debug_bufmax = sizeof(dovi_cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&dovi_cmd_buf, ubuf, count))
		return -EFAULT;

	dovi_cmd_buf[count] = 0;

	dovi_process_dbg_cmd(dovi_cmd_buf);

	return ret;
}

static const struct file_operations dovi_debug_fops = {
	.read = dovi_debug_read,
	.write = dovi_debug_write,
	.open = dovi_debug_open,
};

void dovi_debug_init(void)
{
	if (!dovi_dbg_init) {
		dovi_dbg_init = 1;
		dovi_debugfs = debugfs_create_file("dovi",
						   S_IFREG | S_IRUGO, NULL, (void *)0,
						   &dovi_debug_fops);

		/* dovi_printf("disp hw debug init, fs= %p\n", dovi_debugfs); */
	}
}

void dovi_debug_deinit(void)
{
	debugfs_remove(dovi_debugfs);
}
