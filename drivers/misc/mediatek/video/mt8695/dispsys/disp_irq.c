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

#define LOG_TAG "DISP_IRQ"

#include "disp_hw_log.h"
#include "disp_irq.h"

static const char *_get_irq_name(enum DISP_IRQ_E irq)
{
	switch (irq) {
	case DISP_IRQ_OSD2_UNDERFLOW:
		return "osd2_underflow";
	case DISP_IRQ_OSD3_UNDERFLOW:
		return "osd3_underflow";
	case DISP_IRQ_OSD5_UNDERFLOW:
		return "osd5_underflow";
	case DISP_IRQ_OSD2_VSYNC:
		return "osd2_vsync";
	case DISP_IRQ_OSD3_VSYNC:
		return "osd3_vsync";
	case DISP_IRQ_OSD5_VSYNC:
		return "osd5_vsync";
	case DISP_IRQ_FMT_ACTIVE_START:
		return "vsync_start";
	case DISP_IRQ_FMT_ACTIVE_END:
		return "vsync_end";
	case DISP_IRQ_FMT_VSYNC:
		return "vsync";
	case DISP_IRQ_VDOIN:
		return "videoin";
	case DISP_IRQ_VM:
		return "videomark";
	case DISP_IRQ_SCLER:
		return "scler";
	case DISP_IRQ_DOLBY1:
		return "dolby1";
	case DISP_IRQ_DOLBY2:
		return "dolby2";
	case DISP_IRQ_DOLBY3:
		return "dolby3";
	case DISP_IRQ_DISP3_END:
		return "disp3_end";
	case DISP_IRQ_VDO3_UNDERRUN:
		return "vdo3_underrun";
	case DISP_IRQ_DISP3_VSYNC:
		return "disp3_vsync";
	case DISP_IRQ_DISP4_END:
		return "disp4_end";
	case DISP_IRQ_VDO4_UNDERRUN:
		return "vdo4_underrun";
	case DISP_IRQ_DISP4_VSYNC:
		return "disp4_vsync";
	default:
		DISP_LOG_E("invalid irq = 0x%x\n", irq);
		return "unknown";
	}
}

static void _clear_irq(enum DISP_IRQ_E irq)
{
	DISP_LOG_IRQ("clear irq: %s\n", _get_irq_name(irq));

	if (DISP_IRQ_IS_VDOUT_SYS(irq))
		vdout_sys_clear_irq(DISP_IRQ_SUBSYS_BIT(irq));
	else
		disp_sys_clear_irq(DISP_IRQ_SUBSYS_BIT(irq));
}

int disp_irq_manager(enum DISP_IRQ_E irq)
{
	DISP_LOG_IRQ("irq bit: %s\n", _get_irq_name(irq));

	_clear_irq(irq);

	if (irq == DISP_IRQ_OSD2_UNDERFLOW ||
		irq == DISP_IRQ_OSD3_UNDERFLOW ||
		irq == DISP_IRQ_OSD5_UNDERFLOW ||
		irq == DISP_IRQ_VDO3_UNDERRUN ||
		irq == DISP_IRQ_VDO4_UNDERRUN)
		DISP_LOG_W("%s\n", _get_irq_name(irq));
	/*need to dump register when underflow happened*/
	return 0;
}
