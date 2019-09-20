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


#include <linux/kthread.h>
#include <linux/mutex.h>
#include "pp_drv.h"
#include "pp_drv.h"
#include "pp_hw.h"
#include "pp_hal.h"

uint32_t pp_debug_level;


/******************************************************************************
*Local variable
******************************************************************************/
static uint8_t _pp_vsync_initiated;
static uint8_t _pp_vsync_destroy;

static struct task_struct *h_pp_kthread;
struct mutex _pp_vsync_lock;

void pp_enable(bool enable)
{
	if (enable) {
		vRegWt4BMsk(io_reg_base + 0x300, (0x07 << 24), (0x07 << 24));
		vRegWt4BMsk(io_reg_base + 0x310, (0x07 << 24), (0x07 << 24));
	}

	else {			/*sequence should be reverse. */
		vRegWt4BMsk(io_reg_base + 0x310, (0x00 << 24), (0x07 << 24));
		vRegWt4BMsk(io_reg_base + 0x300, (0x00 << 24), (0x07 << 24));
	}
}

int pp_suspend(void *param)
{
	PP_INFO("PostP Suspend\n");
	pp_enable(false);
	/*pp_hal_sharp_enable(SV_OFF);*/
	return 0;
}

int pp_resume(void *param)
{
	PP_INFO("PostP Resume\n");
	pp_enable(true);
	/*pp_hal_sharp_enable(SV_ON);*/
	return 0;
}


/******************************************************************************
*Function: pp_vsync_tick
*Description: main routine for PostTask
*Parameter: None
*Return: None
******************************************************************************/
void pp_vsync_tick(void)
{
	if (_pp_vsync_initiated == SV_ON) {
		/*pp_set_luma_curve();  //update last calculation*/
		/*mutex unlock*/
		mutex_unlock(&_pp_vsync_lock);
	}
}


/******************************************************************************
*Function: vPostTaskMain
*Description: main routine for Posttask
*Parameter: None
*Return: None
******************************************************************************/
static int pp_mainloop(void *pvArg)
{
	pp_auto_con_init();
	while (_pp_vsync_destroy == 0) {

		/*mutex lock*/
		mutex_lock(&_pp_vsync_lock);
		pp_auto_contrast();

		/*pp_hal_auto_sat();  */
		if (kthread_should_stop())
			break;
	}
	return 0;
}


/******************************************************************************
*Function: pp_drv_init
*Description: initial
*Parameter: None
*Return: PP_OK PP_FAIL
******************************************************************************/
int32_t pp_drv_init(void)
{
	if (_pp_vsync_initiated == SV_OFF) {

		/*initial some post settings */
		pp_hal_yc_proc_init();
		h_pp_kthread = kthread_create(pp_mainloop, NULL, "postproc_kthread");
		/* wake_up_process(h_pp_kthread); */

		mutex_init(&_pp_vsync_lock);
		_pp_vsync_initiated = SV_ON;
	}
	return PP_OK;
}


/******************************************************************************
*Function: pp_drv_uninit
*Description: Un-initial
*Parameter: None
*Return: None
******************************************************************************/
int32_t pp_drv_uninit(void)
{
	if (_pp_vsync_initiated == SV_ON) {

		/*destroy thread*/
		_pp_vsync_destroy = 1;
		pp_vsync_tick();
		_pp_vsync_initiated = SV_OFF;
	}
	return PP_OK;
}
