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


#include "p2i_hw.h"
#include "p2i_hal.h"
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

/* Local Variable */
/* For P2I reg */
uint32_t p2i_1080i60_time[] = {
	0x00C10858,		/* 0x3744 */
	0x00C10858,		/* 0x3748 */
	0x017C017C,		/* 0x374c */
	0x00280462,		/* 0x3750 */
	0x00140231,		/* 0x3754 */
	0x02470464		/* 0x3758 */
};

uint32_t p2i_1080i50_time[] = {
	0x00C10859,		/* 0x3744 */
	0x00C10859,		/* 0x3748 */
	0x017C017C,		/* 0x374c */
	0x00280462,		/* 0x3750 */
	0x00140231,		/* 0x3754 */
	0x02470464		/* 0x3758 */
};

uint32_t p2i_480i_time[] = {
	0x00180359,		/* 0x17 -> 0x17+ 858(H total)  //jitao 0x17  -> 857(H total -1) */
	0x00180359,
	0x002B002B,		/* 0x00200020, */
	0x0010021D,		/* 0x10 -> 0x10 + 525(V total) */
	0x00150107,		/* by spec line 22 to line 261 */
	0x011C020E,		/* by spec line 285 to line 524 */
};

uint32_t p2i_576i_time[] = {
	0x00300360,		/* 0x00640834, */
	0x00300360,
	0x00010001,
	0x002A0461,
	0x00160137,
	0x014F0270
};

uintptr_t p2i_reg_base;

int p2i_hal_init(void)
{
	struct device_node *np;
	unsigned int reg_value;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-p2i");
	if (np == NULL) {
		P2I_ERR("dts error, no p2i device node.\n");
		return -1;
	}
	of_property_read_u32_index(np, "reg", 1, &reg_value);
	p2i_reg_base = (uintptr_t) of_iomap(np, 0);

	return 0;
}

void vWriteP2I(uintptr_t addr, uint32_t value)
{
	WriteREG32(p2i_reg_base + addr, value);
}

uint32_t dReadP2I(uintptr_t addr)
{
	return ReadREG32(p2i_reg_base + addr);
}

void vWriteP2IMsk(uintptr_t addr, uint32_t value, uint32_t mask)
{
	vWriteP2I((addr), (dReadP2I(addr) & (~(mask))) | ((value) & (mask)));
}

void p2i_hal_set_cstmode(unsigned char mode)
{
	vWriteP2IMsk(P2I_CTRL, (mode << 1), OP_MODE);
}

/* read/write doesn't at the same time. */
void p2i_hal_set_rwnotsametime(bool is_on)
{
	if (is_on)
		vWriteP2IMsk(P2I_CTRL, AUTO_R_NOEQ_W_EN, AUTO_R_NOEQ_W_EN);
	else
		vWriteP2IMsk(P2I_CTRL, 0, AUTO_R_NOEQ_W_EN);

}

void p2i_hal_enable_cst(bool is_on)
{
	if (is_on)
		vWriteP2IMsk(P2I_CTRL, P2I_CST_ON, P2I_CST_ON);
	else
		vWriteP2IMsk(P2I_CTRL, 0, P2I_CST_ON);
}

void p2i_hal_turnoff_cst(bool turn_off)
{
	if (turn_off == TRUE) {
		vWriteP2IMsk(P2I_CTRL, CI_VRF_OFF | CI_REPEAT_EN | CI_HRF_OPT,
			     CI_VRF_OFF | CI_REPEAT_EN | CI_HRF_OPT | CI_SEL);
	} else {
		vWriteP2IMsk(P2I_CTRL, 0, CI_VRF_OFF);
	}
}

void p2i_hal_enable_clk(bool on)
{
	static bool is_enable;

	if (on && (!is_enable)) {
		disp_clock_enable(DISP_CLK_P2I, true);
		is_enable = true;
	} else if ((!on) && is_enable) {
		disp_clock_enable(DISP_CLK_P2I, false);
		is_enable = false;
	} else
		P2I_LOG("on=%d,is_enable=%d\n", on, is_enable);
}

void p2i_hal_set_time(HDMI_VIDEO_RESOLUTION res)
{
	uint32_t index;

	switch (res) {
	case HDMI_VIDEO_1920x1080i_60Hz:
		for (index = 0; index < (sizeof(p2i_1080i60_time) / 4); index += 1)
			vWriteP2I(P2I_H_TIME0 + index * 4, p2i_1080i60_time[index]);
		vWriteP2IMsk(P2I_CTRL_2, 0x02, LINE_SHIFT_MSK);
		break;

	case HDMI_VIDEO_1920x1080i_50Hz:
		for (index = 0; index < (sizeof(p2i_1080i50_time) / 4); index += 1)
			vWriteP2I(P2I_H_TIME0 + index * 4, p2i_1080i50_time[index]);
		vWriteP2IMsk(P2I_CTRL_2, 0x02, LINE_SHIFT_MSK);
		break;


	case HDMI_VIDEO_720x480i_60Hz:
		for (index = 0; index < (sizeof(p2i_480i_time) / 4); index += 1)
			vWriteP2I(P2I_H_TIME0 + index * 4, p2i_480i_time[index]);
		vWriteP2IMsk(P2I_CTRL_2, 0, LINE_SHIFT_MSK);
		break;

	case HDMI_VIDEO_720x576i_50Hz:
		for (index = 0; index < (sizeof(p2i_576i_time) / 4); index += 1)
			vWriteP2I(P2I_H_TIME0 + index * 4, p2i_576i_time[index]);
		vWriteP2IMsk(P2I_CTRL_2, 0, LINE_SHIFT_MSK);
		break;

	default:
		P2I_ERR("unknown timing.\n");
		break;
	}
}

/* ============================================ */
uint32_t p2i_hal_get_tv_field(void)
{
	uint32_t field;

	field = (dReadP2I(P2I_CTRL) & TV_FLD) >> 3;
	return field;
}

void pi2_hal_reset(void)
{
	vWriteP2I(P2I_CTRL, (dReadP2I(P2I_CTRL) | 0xC0000000));
	vWriteP2I(P2I_CTRL, (dReadP2I(P2I_CTRL) & 0x3FFFFFFF));
}
