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

#ifndef _VDP_SEC_H
#define _VDP_SEC_H

#include "vdp_hal.h"
#include "disp_hw_mgr.h"

enum SERVICE_CALL_CMD {
	TZCMD_VDP_INIT = 0,
	TZCMD_VDP_DEINIT = 1,
	TZCMD_VDP_HAL_CONFIG = 2,
	TZCMD_VDP_SET_LOG_LEVEL = 3,
	TZCMD_VDP_DEBUG_HDMI10_PLUS = 4,
	TZCMD_VDP_READ_REG = 5,
	TZCMD_VDP_WRITE_REG = 6,
	TZCMD_VDP_STOP = 7,
	TZCMD_VDP_DISABLE_ACTIVE = 8,

	TZCMD_MAX,
};

enum SERVICE_CALL_DIRECTION {
	SERVICE_CALL_DIRECTION_INPUT = 0,
	SERVICE_CALL_DIRECTION_OUTPUT = 1,
	SERVICE_CALL_DIRECTION_INOUT = 2,
};


struct vdp_sec_init_info {
	int layer_id;
	struct vdp_hal_data_info vdp_data;
	struct vdp_hal_disp_info vdp_disp;
	struct vdo_sw_shadow vdp_shadow;

	/* normal mva need to map secure */
	unsigned int normal_mva;
	int mva_size;
};

struct tz_disp_hw_common_info {
	struct disp_hw_tv_capbility tv;
	struct disp_hw_resolution resolution;
	uint32_t ctl_info;
};


struct dispfmt_setting {
	uint64_t dispfmt_register_setting; /* char * buffer ptr */
	int dispfmt_register_setting_size;
	uint64_t dispfmt_register_setting_mask;
};


int disp_vdp_sec_init(unsigned int layer_id, uint32_t normal_mva, int normal_mva_size);
int disp_vdp_sec_deinit(unsigned int layer_id);
int disp_vdp_sec_config(struct vdp_hal_config_info *config_info,
	struct dispfmt_setting *dispfmt_info, uint32_t osd_enable);

int disp_vdp_sec_stop_hw(int layer_id);
int disp_vdp_sec_disable_active_zone(int layer_id);

int disp_vdp_sec_set_log_level(int level);

int vdp_sec_service_call(enum SERVICE_CALL_CMD cmd, enum SERVICE_CALL_DIRECTION direct,
	void *buffer, uint32_t size);

#endif	/*  */
