/*
*  Copyright (c) 2017 MediaTek Inc.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*/

#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "disp_info.h"
#include "disp_hw_mgr.h"
#include "disp_videoin_if.h"
#include "disp_videoin_debug.h"
#include "videoin_hal.h"
#include "vdout_sys_hal.h"

enum VIDEO_BIT_MODE bit_mode;
bool is_16_packet;
enum VIDEOIN_YCbCr_FORMAT color_format;


static int _videoin_default_setting(const struct disp_hw_resolution *info)
{
	int h_start = 0;
	int v_odd_start = 0;
	int v_even_start = 0;

	bit_mode = VIDEOIN_BITMODE_8;
	is_16_packet = false;
	color_format = VIDEOIN_FORMAT_420;

	if (info->height == 480) {
		h_start = 0x7C;

		if (info->is_progressive == true) {
			v_odd_start = 0x2A;
			v_even_start = 0x2A;
		} else {
			v_odd_start = 0x16;
			v_even_start = 0x16;
		}
	} else if (info->height == 576) {
		h_start = 0xEC;
		v_odd_start = 0x2C;
		v_even_start = 0x2C;
	} else if (info->height == 720) {
		h_start = 0xF3;
		v_odd_start = 0x19;
		v_even_start = 0x19;
	} else if (info->height == 1080) {
		h_start = 0xFE;
		if (info->is_progressive == true) {
			v_odd_start = 0x29;
			v_even_start = 0x29;
		} else {
			v_odd_start = 0x15;
			v_even_start = 0x15;
		}
	} else if (info->height == 2160) {
		h_start = 0xD4;
		v_odd_start = 0x2A;
		v_even_start = 0x2A;
	}
	if (info->height == 480)
		vdout_sys_hal_set_new_sd_sel(true);
	else
		vdout_sys_hal_set_new_sd_sel(false);
	vdout_sys_hal_videoin_source_sel(VIDEOIN_SRC_SEL_FMT);
	videoin_hal_set_h(info->width, info->htotal, false, is_16_packet, bit_mode);
	videoin_hal_set_v(info->height, color_format);
	videoin_hal_set_color_format(color_format);
	videoin_hal_set_channel_select(SEL_Y_CHN, SEL_CB_CHN, SEL_CR_CHN);
	videoin_hal_set_bitmode(bit_mode, is_16_packet);
	videoin_hal_demode_enable(false);
	videoin_hal_set_active_zone(h_start, v_odd_start, v_even_start);

	return 0;
}


static int disp_videoin_init(struct disp_hw_common_info *info)
{
	uintptr_t reg_base = 0;
	uintptr_t larb4_regbase = 0;
	struct device_node *np;
	unsigned int reg_value;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-videoin");
	if (np == NULL) {
		pr_err("dts error, no videoin device node.\n");
		return 0;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);

	reg_base = (uintptr_t)of_iomap(np, 0);
	larb4_regbase = (uintptr_t)of_iomap(np, 1);

	videoin_hal_init(reg_base, larb4_regbase);
	_videoin_default_setting(info->resolution);

	videoin_debug_init();
	return 0;
}

static int disp_videoin_deinit(void)
{
	videoin_hal_clock(false);
	return 0;
}

static int disp_videoin_suspend(void)
{
	/* videoin_hal_clock(false); */
	return 0;
}

static int disp_videoin_resume(void)
{
	/* videoin_hal_clock(true); */
	return 0;
}

static int disp_videoin_change_resolution(const struct disp_hw_resolution *info)
{
	_videoin_default_setting(info);
	return 0;
}

/***************** driver************/
struct disp_hw disp_videoin_driver = {
	.name = VIDEOIN_DRV_NAME,
	.init = disp_videoin_init,
	.deinit = disp_videoin_deinit,
	.start = NULL,
	.stop = NULL,
	.suspend = disp_videoin_suspend,
	.resume = disp_videoin_resume,
	.get_info = NULL,
	.change_resolution = disp_videoin_change_resolution,
	.config = NULL,
	.irq_handler = NULL,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = NULL,
};

struct disp_hw *disp_videoin_get_drv(void)
{
	return &disp_videoin_driver;
}


