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
#define LOG_TAG "VDP_SEC"

#include <kree/mem.h>
#include <kree/system.h>
#include <tz_cross/ta_mem.h>
#include <tz_cross/trustzone.h>
#include <linux/slab.h>

#include "disp_vdp_if.h"
#include "disp_vdp_vsync.h"
#include "disp_hw_log.h"
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "fmt_hal.h"
#include "disp_vdp_sec.h"

#if VIDEO_DISPLAY_SECURE_ENABLE
static KREE_SESSION_HANDLE vdp_session;

/* static struct hdr_share_memory_struct gShareMemory; */
static int _vdp_sec_create_session(void)
{
	TZ_RESULT ret;

	do {
		if (vdp_session == 0) {
			ret = KREE_CreateSession("vdp_uuid_40021", &vdp_session);
			if (ret != TZ_RESULT_SUCCESS) {
				DISP_LOG_E("create vdp_session fail:%d\n", ret);
				return VDP_CREATE_SESSION_FAIL;
			}
		}

	} while (0);

	return VDP_OK;
}

#if 0
static int _vdp_sec_destroy_session(void)
{
	TZ_RESULT ret;

	if (vdp_session != 0) {
		ret = KREE_CloseSession(vdp_session);
		if (ret != TZ_RESULT_SUCCESS) {
			DISP_LOG_E("vdp session invalid,id=%d\n", vdp_session);
			return VDP_DESTROY_SESSION_FAIL;
		}
		vdp_session = 0;
	}
	return VDP_OK;
}
#endif

/* we need to map normal buffer to secure,
* because this nomal buffer may still display after convert m4u port to secure.
*/
int disp_vdp_sec_init(unsigned int layer_id, uint32_t normal_mva, int normal_mva_size)
{
	int ret = 0;
	struct vdp_sec_init_info init_info;

	ret = _vdp_sec_create_session();

	do {
		fmt_hal_set_secure(layer_id, true);
		/* fill vdp_sec_init_info structure. */
		memset(&init_info, 0, sizeof(init_info));
		init_info.layer_id = layer_id;
		init_info.normal_mva = normal_mva;
		init_info.mva_size = normal_mva_size;

		vdp_hal_get_info(layer_id, &init_info.vdp_data, &init_info.vdp_disp, &init_info.vdp_shadow);
		/* service call */
		ret = vdp_sec_service_call(TZCMD_VDP_INIT, SERVICE_CALL_DIRECTION_INPUT, &init_info, sizeof(init_info));
		if (ret != VDP_OK) {
			DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n", TZCMD_VDP_INIT, ret);
			return VDP_SERVICE_CALL_FAIL;
		}
	} while (0);

	return ret;
}

int disp_vdp_sec_set_log_level(int level)
{
	MTEEC_PARAM vdp_param[4];
	TZ_RESULT tz_ret;
	int status = 0;

	status = _vdp_sec_create_session();
	if (status != VDP_OK)
		return status;

	vdp_param[0].value.a = level;
	vdp_param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(vdp_session, TZCMD_VDP_SET_LOG_LEVEL,
		TZ_ParamTypes1(TZPT_VALUE_INPUT), vdp_param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n",
			TZCMD_VDP_SET_LOG_LEVEL, tz_ret);
		return VDP_SERVICE_CALL_FAIL;
	}
	return VDP_OK;
}

int disp_vdp_sec_stop_hw(int layer_id)
{
	MTEEC_PARAM vdp_param[4];
	TZ_RESULT tz_ret;

	vdp_param[0].value.a = (uint32_t) layer_id;
	vdp_param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(vdp_session, TZCMD_VDP_STOP,
		TZ_ParamTypes1(TZPT_VALUE_INPUT), vdp_param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n",
			TZCMD_VDP_STOP, tz_ret);
		return VDP_SERVICE_CALL_FAIL;
	}

	return tz_ret;
}


int disp_vdp_sec_disable_active_zone(int layer_id)
{
	MTEEC_PARAM vdp_param[4];
	TZ_RESULT tz_ret;

	vdp_param[0].value.a = (uint32_t) layer_id;
	vdp_param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(vdp_session, TZCMD_VDP_DISABLE_ACTIVE,
		TZ_ParamTypes1(TZPT_VALUE_INPUT), vdp_param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n",
			TZCMD_VDP_DISABLE_ACTIVE, tz_ret);
		return VDP_SERVICE_CALL_FAIL;
	}

	return tz_ret;
}


int disp_vdp_sec_deinit(unsigned int layer_id)
{
	int ret = VDP_OK;
	MTEEC_PARAM vdp_param[4];
	TZ_RESULT tz_ret;
	struct vdp_hal_data_info vdp_data;
	struct vdp_hal_disp_info vdp_disp;
	struct vdo_sw_shadow vdp_shadow;


	vdp_param[0].mem.buffer = (void *)&vdp_data;
	vdp_param[0].mem.size = sizeof(struct vdp_hal_data_info);

	vdp_param[1].mem.buffer = (void *)&vdp_disp;
	vdp_param[1].mem.size = sizeof(struct vdp_hal_disp_info);

	vdp_param[2].mem.buffer = (void *)&vdp_shadow;
	vdp_param[2].mem.size = sizeof(struct vdo_sw_shadow);

	vdp_param[3].value.a = (uint32_t) layer_id;
	vdp_param[3].value.b = 0;

	tz_ret = KREE_TeeServiceCall(vdp_session, TZCMD_VDP_DEINIT,
		TZ_ParamTypes4(TZPT_MEM_INOUT, TZPT_MEM_INOUT,
		TZPT_MEM_INOUT, TZPT_VALUE_INPUT), vdp_param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n",
			TZCMD_VDP_DEINIT, tz_ret);
		return VDP_SERVICE_CALL_FAIL;
	}

	fmt_hal_set_secure(layer_id, false);

	return ret;
}

int disp_vdp_sec_config(struct vdp_hal_config_info *config_info,
	struct dispfmt_setting *dispfmt_info, uint32_t hdr10_plus_ctl_info)
{
	MTEEC_PARAM vdp_param[4];
	TZ_RESULT ret;
	struct tz_disp_hw_common_info tz_disp_common_info = { {0} };

	_vdp_sec_create_session();
	if (vdp_session == 0) {
		DISP_LOG_E("disp_vdp_sec_config error: vdp_session not created\n");
		return VDP_SESSION_NOT_CREATE;
	}

	memcpy(&tz_disp_common_info.resolution, disp_common_info.resolution, sizeof(tz_disp_common_info.resolution));
	memcpy(&tz_disp_common_info.tv, &disp_common_info.tv, sizeof(tz_disp_common_info.tv));
	tz_disp_common_info.ctl_info= hdr10_plus_ctl_info;

	vdp_param[0].mem.buffer = config_info;
	vdp_param[0].mem.size = sizeof(struct vdp_hal_config_info);
	vdp_param[1].mem.buffer = &tz_disp_common_info;
	vdp_param[1].mem.size = sizeof(struct tz_disp_hw_common_info);
	vdp_param[2].mem.buffer = dispfmt_info;
	vdp_param[2].mem.size = sizeof(struct dispfmt_setting);
	ret = KREE_TeeServiceCall(vdp_session, TZCMD_VDP_HAL_CONFIG,
		TZ_ParamTypes3(TZPT_MEM_INPUT, TZPT_MEM_INPUT, TZPT_MEM_INPUT), vdp_param);

	if (ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n",
			TZCMD_VDP_HAL_CONFIG, ret);
		DISP_LOG_E("HDMI_VIDEO_RESOLUTION:%zu uint16_t:%zu bool:%zu long long:%zu\n",
			sizeof(HDMI_VIDEO_RESOLUTION), sizeof(uint16_t), sizeof(bool), sizeof(long long));
		DISP_LOG_E("(char *):%zu (uint64_t)(%zu) (int)(%zu)\n",
			sizeof(char *), sizeof(uint64_t), sizeof(int));
		return VDP_SERVICE_CALL_FAIL;
	}

	return VDP_OK;
}

static int _vdp_sec_map_direction(enum SERVICE_CALL_DIRECTION direct, TZ_PARAM_TYPES *pType)
{
	switch (direct) {
	case SERVICE_CALL_DIRECTION_INPUT:
		*pType = TZPT_MEM_INPUT;
		break;
	case SERVICE_CALL_DIRECTION_OUTPUT:
		*pType = TZPT_MEM_OUTPUT;
		break;
	case SERVICE_CALL_DIRECTION_INOUT:
		*pType = TZPT_MEM_INOUT;
		break;
	default:
		DISP_LOG_E("invalid service call direction:%d\n", direct);
		return VDP_FAIL;
	}
	return VDP_OK;
}

int vdp_sec_service_call(enum SERVICE_CALL_CMD cmd, enum SERVICE_CALL_DIRECTION direct,
	void *buffer, uint32_t size)
{
	unsigned int paramTypes = 0;
	MTEEC_PARAM vdp_param[4];
	TZ_PARAM_TYPES type;
	TZ_RESULT ret;
	int status = 0;

	status = _vdp_sec_create_session();
	if (status != VDP_OK)
		return status;

	status = _vdp_sec_map_direction(direct, &type);
	if (status != VDP_OK)
		return status;

	paramTypes = TZ_ParamTypes1(type);

	vdp_param[0].mem.buffer = buffer;
	vdp_param[0].mem.size = size;

	ret = KREE_TeeServiceCall(vdp_session, cmd, paramTypes, vdp_param);
	if (ret != TZ_RESULT_SUCCESS) {
		DISP_LOG_E("service call fail: cmd[%d] ret[%d]\n", cmd, ret);
		return VDP_SERVICE_CALL_FAIL;
	}

	return VDP_OK;
}


#else	/*  */

int vdp_sec_service_call(enum SERVICE_CALL_CMD cmd, enum SERVICE_CALL_DIRECTION direct,
	void *buffer, uint32_t size)
{
	return VDP_OK;
}

int disp_vdp_sec_init(unsigned int layer_id, uint32_t normal_mva, int normal_mva_size)
{
	return VDP_OK;
}

int disp_vdp_sec_stop_hw(int layer_id)
{
	return VDP_OK;
}

int disp_vdp_sec_deinit(unsigned int layer_id)
{
	return VDP_OK;
}


int disp_vdp_sec_config(struct vdp_hal_config_info *config_info,
	struct dispfmt_setting *dispfmt_info, uint32_t hdr10_plus_ctl_info)
{
	return VDP_OK;
}

#endif	/*  */

