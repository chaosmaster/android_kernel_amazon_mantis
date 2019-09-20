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

#ifndef _MTK_VQ_MGR_H_
#define _MTK_VQ_MGR_H_

#include "vq_def.h"
#include "mtk_vq_info.h"

#define VQ_SESSION_DEVICE "mtk_vq_mgr"
#define VQ_DEVICE_NODE "/dev/mtk_vq_mgr"

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
#define TZ_TA_VQ_NAME "VQ_TA"
#define TZ_TA_VQ_UUID "vq_ta_uuid_07605"

enum VQ_SERVICE_CALL_CMD {
	VQ_SERVICE_CALL_CMD_CONFIG = 0,
	SERVICE_CALL_CMD_MAX,
};

struct VQ_COPY_BUFFER_STRUCT {
	unsigned int normal_handle;
	unsigned int secure_handle;
	int buffer_size;
};

void mtk_vq_set_secure_debug_enable(unsigned int en);
bool mtk_vq_get_secure_debug_enable(void);
#endif

struct vq_data *mtk_vq_get_data(void);
void mtk_vq_set_log_enable(unsigned int en);
void mtk_vq_set_timer_enable(unsigned int en);
int mtk_vq_mgr_set_input_buffer(struct vq_data *data, struct mtk_vq_config *config);

int mtk_vq_power_off(struct vq_data *data, enum VQ_PATH_MODE vq_mode);
int mtk_vq_power_on(struct vq_data *data, enum VQ_PATH_MODE vq_mode);


#endif				/* _MTK_VQ_MGR_H_ */
