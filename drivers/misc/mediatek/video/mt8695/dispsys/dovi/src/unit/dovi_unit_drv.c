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



#define LOG_TAG "DOVI_VFY_DRV"

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "disp_info.h"
#include "disp_hw_mgr.h"
#include "disp_clk.h"
#include "dovi_type.h"
#include "dovi_log.h"

#include "disp_dovi_main.h"

#include "dovi_core1_hal.h"
#include "dovi_core2_hal.h"
#include "dovi_core3_hal.h"
#include "dovi_common_hal.h"

#include "disp_dovi_cmd.h"
#include "disp_dovi_core1_if.h"
#include "disp_dovi_core2_if.h"
#include "disp_dovi_core3_if.h"

#include "disp_dovi_io.h"

#include "dovi_table.h"

#include "fmt_hal.h"
#include "disp_dovi_common.h"
#include "disp_dovi_common_if.h"

#include "dovi_unit_drv.h"

int dovi_unit_get_idk230_hw_setting(
uint32_t **p_core1_reg,
uint32_t **p_core2_reg,
uint32_t **p_core3_reg,
uint32_t **p_core1_lut,
uint32_t **p_core2_lut,
struct dovi_unit_info_t *p_dovi_unit_info)
{
	enum dovi_unit_id unit_id = p_dovi_unit_info->unit_id;

	if ((unit_id == DOVI_DOUBLE_LAYER_BYPASS_CSC_CVM)
	|| (unit_id == DOVI_SIGNLE_LAYER_BYPASS_CSC_CVM)
	|| (unit_id == DOVI_SD_DOUBLE_LAYER)
	|| (unit_id == DOVI_SHADOW_TEST)
	|| (unit_id == DOVI_EFUSE_TEST)
	|| (unit_id == DOVI_SWITCH_ON_TEST)
	|| (unit_id == DOVI_SWITCH_OFF_TEST)
	|| (unit_id == DOVI_SD_DITHER_BYPASS)
	|| (unit_id == DOVI_SD_SDR10_MODE)) {
		/* idk 2.3 5005, core1 yuv input, core3 sdr output, 480p */
		dovi_printf("dovi unit_id %d, SD resolution, SDR out\n", unit_id);
		*p_core1_reg = dovi_v23_480p_yuv_yuv_core1_reg;
		*p_core2_reg = dovi_v23_480p_yuv_yuv_core2_reg;
		*p_core3_reg = dovi_v23_480p_yuv_yuv_core3_reg;
		*p_core1_lut = dovi_v23_480p_yuv_yuv_core1_lut;
		*p_core2_lut = dovi_v23_480p_yuv_yuv_core2_lut;
	} else if ((unit_id == DOVI_SD_IPT_BYPASS_MODE)
	|| (unit_id == DOVI_SD_IPT_TUNNELED_MODE)
	|| (unit_id == DOVI_SD_IPT_TUNNELED_MODE_MUTE_TEST)) {
		/* idk 2.3 5005, core1 yuv input, core3 dovi output, 480p */
		dovi_printf("dovi unit_id %d, SD resolution, DOVI out\n", unit_id);
		*p_core1_reg = dovi_v23_480p_yuv_ipt_core1_reg;
		*p_core2_reg = dovi_v23_480p_yuv_ipt_core2_reg;
		*p_core3_reg = dovi_v23_480p_yuv_ipt_core3_reg;
		*p_core1_lut = dovi_v23_480p_yuv_ipt_core1_lut;
		*p_core2_lut = dovi_v23_480p_yuv_ipt_core2_lut;
	} else if (unit_id == DOVI_SD_HDR10_MODE) {
		/* idk 2.3 5005, core1 yuv input, core3 hdr10 output, 480p */
		dovi_printf("dovi unit_id %d, SD resolution, HDR10 out\n", unit_id);
		*p_core1_reg = dovi_v23_480p_yuv_hdr_core1_reg;
		*p_core2_reg = dovi_v23_480p_yuv_hdr_core2_reg;
		*p_core3_reg = dovi_v23_480p_yuv_hdr_core3_reg;
		*p_core1_lut = dovi_v23_480p_yuv_hdr_core1_lut;
		*p_core2_lut = dovi_v23_480p_yuv_hdr_core2_lut;
	} else if (unit_id == DOVI_HD_DOUBLE_LAYER) {
		/* idk 2.3 5004, core1 yuv input, core3 sdr output, 720p */
		dovi_printf("dovi unit_id %d, HD resolution, SDR out\n", unit_id);
		*p_core1_reg = dovi_v23_720p_yuv_yuv_core1_reg;
		*p_core2_reg = dovi_v23_720p_yuv_yuv_core2_reg;
		*p_core3_reg = dovi_v23_720p_yuv_yuv_core3_reg;
		*p_core1_lut = dovi_v23_720p_yuv_yuv_core1_lut;
		*p_core2_lut = dovi_v23_720p_yuv_yuv_core2_lut;
	} else if ((unit_id == DOVI_FHD_DOUBLE_LAYER)
	|| (unit_id == DOVI_FHD_24HZ_DOUBLE_LAYER)) {
		/* idk 2.3 5000, core1 yuv input, core3 sdr output, 1080p */
		dovi_printf("dovi unit_id %d, FHD resolution, SDR out\n", unit_id);
		*p_core1_reg = dovi_v23_1080p_yuv_yuv_core1_reg;
		*p_core2_reg = dovi_v23_480p_yuv_yuv_core2_reg;
		*p_core3_reg = dovi_v23_1080p_yuv_yuv_core3_reg;
		*p_core1_lut = dovi_v23_1080p_yuv_yuv_core1_lut;
		*p_core2_lut = dovi_v23_480p_yuv_yuv_core2_lut;
	} else if ((unit_id == DOVI_UHD_DOUBLE_LAYER)
	|| (unit_id == DOVI_UHD_SIGNLE_LAYER)) {
		/* idk 2.3 5003, core1 yuv input, core3 sdr output, 2160 */
		dovi_printf("dovi unit_id %d, UHD resolution, SDR out\n", unit_id);
		*p_core1_reg = dovi_v23_2160p_yuv_yuv_core1_reg;
		*p_core2_reg = dovi_v23_480p_yuv_yuv_core2_reg;
		*p_core3_reg = dovi_v23_2160p_yuv_yuv_core3_reg;
		*p_core1_lut = dovi_v23_2160p_yuv_yuv_core1_lut;
		*p_core2_lut = dovi_v23_480p_yuv_yuv_core2_lut;
	} else {
		/* idk 2.3 5003, core1 yuv input, core3 dovi output, 2160 */
		dovi_printf("dovi unit_id %d, UHD resolution, SDR out\n", unit_id);
		*p_core1_reg = dovi_v23_2160p_yuv_ipt_core1_reg;
		*p_core2_reg = dovi_v23_2160p_yuv_ipt_core2_reg;
		*p_core3_reg = dovi_v23_2160p_yuv_ipt_core3_reg;
		*p_core1_lut = dovi_v23_2160p_yuv_ipt_core1_lut;
		*p_core2_lut = dovi_v23_2160p_yuv_ipt_core2_lut;
	}

	return 1;
}

int dovi_unit_get_idk241_hw_setting(
uint32_t **p_core1_reg,
uint32_t **p_core2_reg,
uint32_t **p_core3_reg,
uint32_t **p_core1_lut,
uint32_t **p_core2_lut,
struct dovi_unit_info_t *p_dovi_unit_info)
{
	dovi_error("not support idk 2.4.1 yet\n");

	*p_core1_reg = dovi_v23_480p_yuv_yuv_core1_reg;
	*p_core2_reg = dovi_v23_480p_yuv_yuv_core2_reg;
	*p_core3_reg = dovi_v23_480p_yuv_yuv_core3_reg;
	*p_core1_lut = dovi_v23_480p_yuv_yuv_core1_lut;
	*p_core2_lut = dovi_v23_480p_yuv_yuv_core2_lut;
	return 1;
}

int dovi_unit_get_idk242_hw_setting(
uint32_t **p_core1_reg,
uint32_t **p_core2_reg,
uint32_t **p_core3_reg,
uint32_t **p_core1_lut,
uint32_t **p_core2_lut,
struct dovi_unit_info_t *p_dovi_unit_info)
{
	*p_core1_reg = dovi_v23_480p_yuv_yuv_core1_reg;
	*p_core2_reg = dovi_v23_480p_yuv_yuv_core2_reg;
	*p_core3_reg = dovi_v23_480p_yuv_yuv_core3_reg;
	*p_core1_lut = dovi_v23_480p_yuv_yuv_core1_lut;
	*p_core2_lut = dovi_v23_480p_yuv_yuv_core2_lut;
	dovi_error("not support idk 2.4.2 yet\n");
	return 1;
}

int dovi_unit_set_path(struct dovi_unit_info_t *p_dovi_unit_info)
{
	enum dovi_idk_version idk_version = p_dovi_unit_info->idk_version;

	uint32_t *p_core1_reg;
	uint32_t *p_core2_reg;
	uint32_t *p_core3_reg;

	uint32_t *p_core1_lut;
	uint32_t *p_core2_lut;

	dovi_core1_hal_set_enable(1);
	dovi_core2_hal_set_enable(1);
	dovi_core3_hal_set_enable(1);

	dovi_core1_hal_control_config(&dovi_res);
	dovi_core2_hal_control_config(&dovi_res);
	dovi_core3_hal_control_config(&dovi_res);

	if (idk_version == DOVI_IDK_230)
		dovi_unit_get_idk230_hw_setting(&p_core1_reg, &p_core2_reg, &p_core3_reg,
		&p_core1_lut, &p_core2_lut, p_dovi_unit_info);
	else
		dovi_unit_get_idk241_hw_setting(&p_core1_reg, &p_core2_reg, &p_core3_reg,
		&p_core1_lut, &p_core2_lut, p_dovi_unit_info);

	dovi_printf("p_core_reg 0x%p 0x%p 0x%p lut 0x%p 0x%p\n",
		    p_core1_reg, p_core2_reg,
		    p_core3_reg, p_core1_lut,
		    p_core2_lut);

	dovi_core1_hal_config_reg(p_core1_reg);
	dovi_core2_hal_config_reg(p_core2_reg);
	dovi_core3_hal_config_reg(p_core3_reg);

	dovi_core1_config_lut(p_core1_lut);
	dovi_core2_config_lut(p_core2_lut);

	disp_dovi_isr();

	return DOVI_STATUS_OK;
}

int dovi_unit_test(enum dovi_unit_id unit_id)
{
	uint32_t *p_core1_reg;
	uint32_t *p_core2_reg;
	uint32_t *p_core3_reg;

	uint32_t *p_core1_lut;
	uint32_t *p_core2_lut;

	if (unit_id == DOVI_REGISTER_TEST) {
		dovi_printf("register r/w test\n");
		dovi_unit_register_test();
	} else if (unit_id == DOVI_SHADOW_TEST) {
		dovi_printf("shadow register test\n");
		dovi_unit_register_test();
	} else {
		enum dovi_idk_version idk_version = DOVI_IDK_230;
		struct dovi_unit_info_t dovi_unit_info;

		dovi_unit_info.unit_id = unit_id;

		dovi_core1_hal_set_enable(1);
		dovi_core2_hal_set_enable(1);
		dovi_core3_hal_set_enable(1);

		dovi_core1_hal_control_config(&dovi_res);
		dovi_core2_hal_control_config(&dovi_res);
		dovi_core3_hal_control_config(&dovi_res);

		if (idk_version == DOVI_IDK_230)
			dovi_unit_get_idk230_hw_setting(&p_core1_reg,
			&p_core2_reg, &p_core3_reg,
			&p_core1_lut, &p_core2_lut,
			&dovi_unit_info);
		else
			dovi_unit_get_idk241_hw_setting(&p_core1_reg,
			&p_core2_reg, &p_core3_reg,
			&p_core1_lut, &p_core2_lut,
			&dovi_unit_info);

		dovi_printf("p_core_reg 0x%p 0x%p 0x%p lut 0x%p 0x%p\n",
			    p_core1_reg, p_core2_reg,
			    p_core3_reg, p_core1_lut,
			    p_core2_lut);

		dovi_core1_hal_config_reg(p_core1_reg);
		dovi_core2_hal_config_reg(p_core2_reg);
		dovi_core3_hal_config_reg(p_core3_reg);

		dovi_core1_config_lut(p_core1_lut);
		dovi_core2_config_lut(p_core2_lut);

		if ((unit_id == DOVI_DOUBLE_LAYER_BYPASS_CSC_CVM)
		|| (unit_id == DOVI_SIGNLE_LAYER_BYPASS_CSC_CVM)) {
			dovi_printf("dovi unit_id %d, bypass csc/cvm\n", unit_id);
			dovi_core1_hal_bypass_csc(1);
			dovi_core1_hal_bypass_cvm(1);

			if (unit_id == DOVI_SIGNLE_LAYER_BYPASS_CSC_CVM) {
				dovi_printf("dovi unit_id %d, not compose el\n", unit_id);
				dovi_core1_hal_set_composer_mode(0);
			}
		} else if (unit_id == DOVI_SD_DITHER_BYPASS) {
			dovi_printf("dovi unit_id %d, dither bypass\n", unit_id);
			dovi_core3_hal_dither_bypass(1);
		} else if ((unit_id == DOVI_SD_SDR10_MODE)
		|| (unit_id == DOVI_SD_IPT_BYPASS_MODE)) {
			enum dovi_core3_out_mode mode = DOVI_CORE3_IPT444_BYPASS;

			if (unit_id == DOVI_SD_SDR10_MODE)
				mode = DOVI_CORE3_SDR10;

			dovi_printf("dovi unit_id %d, core3 out mode %d\n", unit_id, mode);

			dovi_core3_hal_set_out_mode(mode);
		} else if (unit_id == DOVI_UHD_SIGNLE_LAYER) {
			dovi_printf("dovi unit_id %d, not compose el\n", unit_id);
			dovi_core1_hal_set_composer_mode(0);
		}


		disp_dovi_isr();


		if (unit_id == DOVI_SWITCH_OFF_TEST) {
			dovi_printf("dovi unit_id %d, not compose el\n", unit_id);
			/* sleep several vsync, then switch off normal path*/
		}
	}

	return DOVI_STATUS_OK;
}


