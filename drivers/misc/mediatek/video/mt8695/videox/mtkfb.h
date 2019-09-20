/*
 * Copyright (C) 2015 MediaTek Inc.
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

#ifndef __MTKFB_H
#define __MTKFB_H

#include <linux/types.h>
#include <linux/compat.h>
#include "disp_info.h"

#define MTKFB_DRIVER "mtkfb"
/* #define MTK_NO_DISP_IN_LK */

enum mtkfb_state {
	MTKFB_DISABLED = 0,
	MTKFB_SUSPENDED = 99,
	MTKFB_ACTIVE = 100
};

struct mtkfb_device {
	int state;
	void *fb_va_base;	/* MPU virtual address */
	dma_addr_t fb_pa_base;	/* Bus physical address */
	unsigned long fb_size_in_byte;

	enum DISP_HW_COLOR_FORMAT *layer_format;

	int xscale, yscale, mirror;	/* transformations.rotate is stored in fb_info->var */
	u32 pseudo_palette[17];

	struct fb_info *fb_info;	/* Linux fbdev framework data */
	struct device *dev;
};


#endif
