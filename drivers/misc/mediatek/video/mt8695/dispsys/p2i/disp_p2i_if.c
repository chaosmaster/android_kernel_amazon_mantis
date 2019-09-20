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

#include "disp_type.h"
#include "p2i_hal.h"
#include "disp_p2i_if.h"
#include "disp_hw_mgr.h"

static HDMI_VIDEO_RESOLUTION saved_res_mode = HDMI_VIDEO_RESOLUTION_NUM;
static int disp_p2i_change_resolution(const struct disp_hw_resolution *info)
{
	P2I_DEBUG("change res mode=%d\n", info->res_mode);

	if ((info->res_mode == HDMI_VIDEO_720x480i_60Hz)
	    || (info->res_mode == HDMI_VIDEO_720x576i_50Hz)
	    || (info->res_mode == HDMI_VIDEO_1920x1080i_60Hz)
	    || (info->res_mode == HDMI_VIDEO_1920x1080i_50Hz)) {
		p2i_hal_enable_clk(true);
		p2i_hal_enable_cst(true);
		p2i_hal_set_cstmode(P2I_AND_CST);
		p2i_hal_set_rwnotsametime(true);
		p2i_hal_set_time(info->res_mode);
	} else {
		p2i_hal_enable_cst(false);
		p2i_hal_enable_clk(false);
	}
	saved_res_mode = info->res_mode;

	return P2I_OK;
}


/* Turn off Constrained image */
void disp_p2i_turnoff_cst(void)
{
	p2i_hal_turnoff_cst(TRUE);
}


/* Turn on Constrained image */
void disp_p2i_turnon_cst(void)
{
	p2i_hal_turnoff_cst(FALSE);
}

/* ============================================ */
uint32_t disp_p2i_get_tvfield(void)
{
	return p2i_hal_get_tv_field();
}

void disp_p2i_reset(void)
{
	pi2_hal_reset();
}

static bool p2i_initiated;
static int disp_p2i_init(struct disp_hw_common_info *info)
{
	int ret = P2I_OK;

	P2I_FUNC();

	if (!p2i_initiated) {
		ret = p2i_hal_init();
		if (ret != P2I_OK) {
			P2I_ERR("init p2i fail.\n");
			return ret;
		}
		p2i_hal_enable_clk(true);
		disp_p2i_change_resolution(info->resolution);
		p2i_initiated = true;
	}

	return ret;

}

static int disp_p2i_deinit(void)
{
	p2i_hal_enable_clk(false);
	return P2I_OK;
}

static int disp_p2i_suspend(void)
{
	p2i_hal_enable_cst(false);
	p2i_hal_enable_clk(false);
	return P2I_OK;
}

static int disp_p2i_resume(void)
{
	struct disp_hw_resolution info;

	info.res_mode = saved_res_mode;
	disp_p2i_change_resolution(&info);
	return P2I_OK;
}


/***************** driver************/
struct disp_hw disp_p2i_driver = {
	.name = P2I_DRV_NAME,
	.init = disp_p2i_init,
	.deinit = disp_p2i_deinit,
	.start = NULL,
	.stop = NULL,
	.suspend = disp_p2i_suspend,
	.resume = disp_p2i_resume,
	.get_info = NULL,
	.change_resolution = disp_p2i_change_resolution,
	.config = NULL,
	.irq_handler = NULL,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = NULL,
};

struct disp_hw *disp_p2i_get_drv(void)
{
	return &disp_p2i_driver;
}
