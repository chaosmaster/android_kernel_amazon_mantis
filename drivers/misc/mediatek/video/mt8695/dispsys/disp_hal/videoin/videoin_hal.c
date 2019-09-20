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

#include "videoin_hw.h"
#include "videoin_hal.h"
#include "disp_clk.h"

struct videoin_context_t videoin;

int videoin_hal_enable(bool en)
{
	if (en) {
		videoin_hal_clock(true);
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0x401D, 0xffff);
	} else {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0x401C, 0xffff);
		videoin_hal_clock(false);
	}
	/* vWriteVIDEOINMsk(VIDEO_IN_CONFIG, en, CONFIG_ON); */
	return 0;
}

int videoin_hal_set_h(int h_active, int h_total, bool is_444, int is_16_packet, enum VIDEO_BIT_MODE bit_mode)
{
	int pixel_cnt = 0;
	int unit_size = 0;  /* the unit size of pixel cnt in width */

	if (bit_mode == 0)
		return -1;

	if (is_16_packet)
		pixel_cnt = 128 / 16;
	else
		pixel_cnt = 128 / bit_mode;

	if (pixel_cnt == 0)
		return -1;

	unit_size = h_active / pixel_cnt;

	vWriteVIDEOIN(VIDEO_IN_H_CNT, h_active << 16 | (h_total - 1));
	vWriteVIDEOIN(VIDEO_IN_H_CNTEND, (unit_size - 1) << 16 | (unit_size - 1));
	/* vWriteVIDEOINMsk(VIDEO_IN_H_BACKUP, unit_size << 16, 0xfff << 16); */
	vWriteVIDEOIN(VIDEO_IN_H_BACKUP, unit_size << 16 | 0x54E3);  /*for 8695 test*/
	if (is_444)
		vWriteVIDEOINMsk(VIDEO_IN_C2_ACTIVE, (unit_size - 1), 0xffff);

	return 0;
}

int videoin_hal_set_v(int v_active, enum VIDEOIN_YCbCr_FORMAT format)
{
	vWriteVIDEOINMsk(VIDEO_IN_LINE, (v_active - 1), H_ACTIVE_LINE);
	if (format == VIDEOIN_FORMAT_420)
		vWriteVIDEOIN(VIDEO_IN_ACTIVE_LINE, (v_active / 2 - 1) << 16 | (v_active - 1));
	else
		vWriteVIDEOIN(VIDEO_IN_ACTIVE_LINE, (v_active - 1) << 16 | (v_active - 1));

	if (format == VIDEOIN_FORMAT_444)
		vWriteVIDEOINMsk(VIDEO_IN_C2_ACTIVE, (v_active - 1) << 16, 0xfff << 16);

	return 0;
}

int videoin_hal_set_color_format(enum VIDEOIN_YCbCr_FORMAT format)
{
	if (format == VIDEOIN_FORMAT_420) {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_420_MODE, CONFIG_420_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_INPUT_CTRL, VIDEO_IN_444_MODE, VIDEO_IN_444_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_LINE, 0, C2_ENABLE);
	} else if (format == VIDEOIN_FORMAT_422) {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0, CONFIG_420_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_INPUT_CTRL, VIDEO_IN_444_MODE, VIDEO_IN_444_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_LINE, 0, C2_ENABLE);
	} else {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0, CONFIG_420_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_INPUT_CTRL, 0, VIDEO_IN_444_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_LINE, C2_ENABLE, C2_ENABLE);
	}

	return 0;
}

int videoin_hal_set_channel_select(enum VIDEO_CHN_SEL y_sel, enum VIDEO_CHN_SEL cb_sel, enum VIDEO_CHN_SEL cr_sel)
{
	int select = y_sel | (cb_sel << 2) | (cr_sel << 4);

	VIDEOIN_LOG_D("y_sel=%d, cb_sel=%d, cr_sel=%d\n", y_sel, cb_sel, cr_sel);
	if (y_sel == cb_sel || y_sel == cr_sel || cb_sel == cr_sel) {
		VIDEOIN_LOG_E("y/cb/cr channel select setting fail.\n");
		return -1;
	}
	vWriteVIDEOINMsk(VIDEO_IN_INPUT_CTRL, select, 0x3f);
	return 0;
}

int videoin_hal_set_active_zone(int h_active_start, int v_odd_active_start, int v_event_active_start)
{
	vWriteVIDEOIN(VIDEO_IN_H_PKCNT, h_active_start);
	vWriteVIDEOIN(VIDEO_IN_V_PKCNT, (v_event_active_start << 16) | v_odd_active_start);
	return 0;
}

int videoin_hal_demode_enable(bool en)
{
	if (en) {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_DE_MODE, CONFIG_DE_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, VIDEO_IN_IDX_MODE, VIDEO_IN_IDX_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, VIDEO_IN_ACT_SEL, VIDEO_IN_ACT_SEL);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, VIDEO_IN_END_SEL, VIDEO_IN_END_SEL);
	} else {
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0, CONFIG_DE_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, 0, VIDEO_IN_IDX_MODE);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, 0, VIDEO_IN_ACT_SEL);
		vWriteVIDEOINMsk(VIDEO_IN_REQ_OUT, 0, VIDEO_IN_END_SEL);
	}

	return 0;
}

int videoin_hal_update_addr(int y_addr, int cb_addr, int cr_addr, bool is_444)
{
	vWriteVIDEOIN(VIDEO_IN_Y_ADDR, (y_addr >> 4));
	vWriteVIDEOIN(VIDEO_IN_C_ADDR, (cb_addr >> 4));
	if (is_444)
		vWriteVIDEOIN(VIDEO_IN_CR_ADDR, (cr_addr >> 4));

	return 0;
}

int videoin_hal_set_bitmode(enum VIDEO_BIT_MODE mode, bool packet_16bit_en)
{
	if (packet_16bit_en)
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_16_PACKET, CONFIG_16_PACKET);
	else
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, 0, CONFIG_16_PACKET);

	if (mode == VIDEOIN_BITMODE_10)
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_10_BIT, CONFIG_PACKET_MODE);
	else if (mode == VIDEOIN_BITMODE_12)
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_12_BIT, CONFIG_PACKET_MODE);
	else
		vWriteVIDEOINMsk(VIDEO_IN_CONFIG, CONFIG_8_BIT, CONFIG_PACKET_MODE);

	return 0;
}

int videoin_hal_clock(bool on)
{
	disp_clock_enable(DISP_CLK_VDOIN, on);
	return 0;
}

void videoin_hal_set_m4u_port(bool en)
{
	/*just for fpga test*/
	if (en) {
		WriteREG32(videoin.init_param.larb4_reg_base + 0x90, 1);
		WriteREG32(videoin.init_param.larb4_reg_base + 0x94, 1);
		WriteREG32(videoin.init_param.larb4_reg_base + 0x98, 1);
	} else {
		WriteREG32(videoin.init_param.larb4_reg_base + 0x90, 0);
		WriteREG32(videoin.init_param.larb4_reg_base + 0x94, 0);
		WriteREG32(videoin.init_param.larb4_reg_base + 0x98, 0);
	}
}

int videoin_hal_init(uintptr_t videoin_reg_base, uintptr_t larb4_reg_base)
{
	videoin.init_param.videoin_reg_base = videoin_reg_base;
	videoin.init_param.larb4_reg_base = larb4_reg_base;

	VIDEOIN_LOG_D("video-in reg_base: 0x%lx, larb4: 0x%lx\n",
			  videoin_reg_base, larb4_reg_base);

	vWriteVIDEOINMsk(VIDEO_IN_REQ_CTRL, 0x04, 0xff);

	return 0;
}

