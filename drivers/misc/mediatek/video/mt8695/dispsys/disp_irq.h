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

#ifndef __DISP_IRQ_H__
#define __DISP_IRQ_H__

#include <linux/device.h>
#include <linux/types.h>
#include "vdout_sys_hal.h"
#include "disp_sys_hal.h"

enum DISP_IRQ_E {
	/*vdout sys irq bit*/
	DISP_IRQ_OSD2_UNDERFLOW		= 0x0 << 31 | VDOUT_SYS_IRQ_OSD2_UNDERFLOW,
	DISP_IRQ_OSD3_UNDERFLOW		= 0x0 << 31 | VDOUT_SYS_IRQ_OSD3_UNDERFLOW,
	DISP_IRQ_OSD5_UNDERFLOW		= 0x0 << 31 | VDOUT_SYS_IRQ_OSD5_UNDERFLOW,
	DISP_IRQ_OSD2_VSYNC			= 0x0 << 31 | VDOUT_SYS_IRQ_OSD2_VSYNC,
	DISP_IRQ_OSD3_VSYNC			= 0x0 << 31 | VDOUT_SYS_IRQ_OSD3_VSYNC,
	DISP_IRQ_OSD5_VSYNC			= 0x0 << 31 | VDOUT_SYS_IRQ_OSD5_VSYNC,
	DISP_IRQ_FMT_ACTIVE_START	= 0x0 << 31 | VDOUT_SYS_IRQ_FMT_ACTIVE_START,
	DISP_IRQ_FMT_ACTIVE_END		= 0x0 << 31 | VDOUT_SYS_IRQ_FMT_ACTIVE_END,
	DISP_IRQ_FMT_VSYNC			= 0x0 << 31 | VDOUT_SYS_IRQ_FMT_VSYNC,
	DISP_IRQ_VDOIN				= 0x0 << 31 | VDOUT_SYS_IRQ_VDOIN,
	DISP_IRQ_VM					= 0x0 << 31 | VDOUT_SYS_IRQ_VM,
	DISP_IRQ_SCLER				= 0x0 << 31 | VDOUT_SYS_IRQ_SCLER,
	DISP_IRQ_DOLBY1				= 0x0 << 31 | VDOUT_SYS_IRQ_DOLBY1,
	DISP_IRQ_DOLBY2				= 0x0 << 31 | VDOUT_SYS_IRQ_DOLBY2,
	DISP_IRQ_DOLBY3				= 0x0 << 31 | VDOUT_SYS_IRQ_DOLBY3,
	/*disp sys irq bit*/
	DISP_IRQ_DISP3_END			= 0x1 << 31 | DISP_SYS_IRQ_DISP3_END,
	DISP_IRQ_VDO3_UNDERRUN		= 0x1 << 31 | DISP_SYS_IRQ_VDO3_UNDERRUN,
	DISP_IRQ_DISP3_VSYNC		= 0x1 << 31 | DISP_SYS_IRQ_DISP3_VSYNC,
	DISP_IRQ_DISP4_END			= 0x1 << 31 | DISP_SYS_IRQ_DISP4_END,
	DISP_IRQ_VDO4_UNDERRUN		= 0x1 << 31 | DISP_SYS_IRQ_VDO4_UNDERRUN,
	DISP_IRQ_DISP4_VSYNC		= 0x1 << 31 | DISP_SYS_IRQ_DISP4_VSYNC,
};

#define DISP_IRQ_IS_VDOUT_SYS(irq) ((irq & 0x80000000) == 0)
#define DISP_IRQ_SUBSYS_BIT(irq) (irq & 0x7fffffff)

int disp_irq_manager(enum DISP_IRQ_E irq);
#endif
