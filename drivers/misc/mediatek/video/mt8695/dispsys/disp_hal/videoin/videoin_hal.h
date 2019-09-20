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

#ifndef _VDOIN_HAL_H_
#define _VDOIN_HAL_H_

#include <linux/types.h>
#include <linux/printk.h>

#include "disp_reg.h"

extern struct videoin_context_t videoin;

#define VDOUT_IN_BASE		videoin.init_param.videoin_reg_base


/**********************************************************************
*  VIDEO-IN System & Clock Macros
*********************************************************************/

#define vWriteVIDEOIN(dAddr, dVal)  WriteREG32((VDOUT_IN_BASE + dAddr), dVal)
#define dReadVIDEOIN(dAddr)         ReadREG32(VDOUT_IN_BASE + dAddr)
#define vWriteVIDEOINMsk(dAddr, dVal, dMsk) \
		vWriteVIDEOIN((dAddr), (dReadVIDEOIN(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))


struct videoin_init_param {
	uintptr_t videoin_reg_base;
	uintptr_t larb4_reg_base;
};

struct videoin_context_t {
	bool inited;
	struct videoin_init_param init_param;
};

enum VIDEOIN_YCbCr_FORMAT {
	VIDEOIN_FORMAT_420,
	VIDEOIN_FORMAT_422,
	VIDEOIN_FORMAT_444
};

enum VIDEO_CHN_SEL {
	SEL_Y_CHN,
	SEL_CB_CHN,
	SEL_CR_CHN
};

enum VIDEO_BIT_MODE {
	VIDEOIN_BITMODE_8 = 8,
	VIDEOIN_BITMODE_10 = 10,
	VIDEOIN_BITMODE_12 = 12
};

#define VIDEOIN_LOG_E(format...) pr_err("[VIDEO-IN]error: "format)
#define VIDEOIN_LOG_I(format...) pr_info("[VIDEO-IN] "format)
#define VIDEOIN_LOG_D(format...) pr_debug("[VIDEO-IN] "format)

int videoin_hal_enable(bool en);
int videoin_hal_set_h(int h_active, int h_total, bool is_444, int is_16_packet, enum VIDEO_BIT_MODE bit_mode);
int videoin_hal_set_v(int v_active, enum VIDEOIN_YCbCr_FORMAT format);
int videoin_hal_set_color_format(enum VIDEOIN_YCbCr_FORMAT format);
int videoin_hal_set_channel_select(enum VIDEO_CHN_SEL y_sel, enum VIDEO_CHN_SEL cb_sel, enum VIDEO_CHN_SEL cr_sel);
int videoin_hal_set_active_zone(int h_active_start, int v_odd_active_start, int v_event_active_start);
int videoin_hal_demode_enable(bool en);
int videoin_hal_update_addr(int y_addr, int cb_addr, int cr_addr, bool is_444);
int videoin_hal_set_bitmode(enum VIDEO_BIT_MODE mode, bool packet_16bit_en);
int videoin_hal_clock(bool on);
void videoin_hal_set_m4u_port(bool en);
int videoin_hal_init(uintptr_t videoin_reg_base, uintptr_t larb4_reg_base);

#endif
