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

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/of_fdt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/libfdt.h>
#include <linux/dma-buf.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <asm/cacheflush.h>
#include <linux/io.h>
#include <linux/compat.h>
#include <linux/dma-mapping.h>
#include <linux/pm_runtime.h>

#include "mtkfb.h"
#include "hdmitx.h"
#include "disp_hw_mgr.h"
#include "disp_clk.h"
#include "smi.h"
#include "disp_assert_layer.h"

#include <linux/memblock.h>
#include <linux/bootmem.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/memory_hotplug.h>
#include <linux/pfn.h>
#include <linux/page_ext.h>
#include <linux/kernel.h>

#undef CONFIG_PM

struct fb_info *mtkfb_fbi;

static u32 MTK_FB_XRES = 1920;
static u32 MTK_FB_YRES = 1080;
static u32 MTK_FB_BPP = 32;
static u32 MTK_FB_PAGES;

struct tag_videolfb {
	u16		lfb_width;
	u16		lfb_height;
	u16		lfb_depth;
	u16		lfb_linelength;
	u32		lfb_base;
	u32		lfb_size;
	u32		lfb_res;
	u32		lfb_colorspace;
	u32		lfb_colordepth;
	u32 		lfb_force_dolby;
	u32		lfb_hdr_type;
	u32 		lfb_out_format;
	u8		red_size;
	u8		red_pos;
	u8		green_size;
	u8		green_pos;
	u8		blue_size;
	u8		blue_pos;
	u8		rsvd_size;
	u8		rsvd_pos;
};

unsigned int g_hdmi_res = 0xff;
unsigned int g_hdmi_colordepth = 0xff;
unsigned int g_hdmi_colorspace = 0xff;
unsigned int g_force_dolby;
unsigned int g_hdr_type;
unsigned int g_out_format;

static phys_addr_t framebuffer_base;
static unsigned int framebuffer_size;

unsigned long fb_bus_addr;
void *fb_va_base;

static int is_videofb_parse_done;

#define MTK_FB_ALIGNMENT 32

#define ALIGN_TO(x, n)  \
	(((x) + ((n) - 1)) & ~((n) - 1))

#define MTK_FB_XRESV (ALIGN_TO(MTK_FB_XRES, MTK_FB_ALIGNMENT))
#define MTK_FB_YRESV (ALIGN_TO(MTK_FB_YRES, MTK_FB_ALIGNMENT) * MTK_FB_PAGES)	/* For page flipping */
#define MTK_FB_BYPP  ((MTK_FB_BPP + 7) >> 3)
#define MTK_FB_LINE  (ALIGN_TO(MTK_FB_XRES, MTK_FB_ALIGNMENT) * MTK_FB_BYPP)
#define MTK_FB_SIZE  (MTK_FB_LINE * ALIGN_TO(MTK_FB_YRES, MTK_FB_ALIGNMENT))

#define MTK_FB_SIZEV (MTK_FB_LINE * ALIGN_TO(MTK_FB_YRES, MTK_FB_ALIGNMENT) * MTK_FB_PAGES)

static size_t mtkfb_log_on;

#define MTKFB_ERR(fmt, arg...) pr_err("DISP/MTKFB " fmt, ##arg)
#define MTKFB_INFO(fmt, arg...) pr_info("DISP/MTKFB " fmt, ##arg)

#ifndef ASSERT
#define ASSERT(expr)					\
	do {						\
		if (expr)				\
			break;				\
		pr_err("DDP ASSERT FAILED %s, %d\n",	\
			__FILE__, __LINE__); WARN_ON(1);	\
	} while (0)
#endif


#define MTKFB_LOG(fmt, arg...)					\
		do {							\
			if (mtkfb_log_on)				\
				pr_debug("DISP/MTKFB " fmt, ##arg); \
		} while (0)

/* always show this debug info while the global debug log is off */
#define MTKFB_LOG_DBG(fmt, arg...)				\
		do {							\
			if (!mtkfb_log_on)				\
				pr_debug("DISP/MTKFB " fmt, ##arg); \
		} while (0)

static int mtkfb_open(struct fb_info *info, int user)
{
	MTKFB_LOG("mtkfb_open\n");
	return 0;
}

static int mtkfb_release(struct fb_info *info, int user)
{
	MTKFB_LOG("mtkfb_release\n");
	return 0;
}

static int mtkfb_setcolreg(u_int regno, u_int red, u_int green,
			   u_int blue, u_int transp, struct fb_info *info)
{
	int r = 0;
	unsigned bpp, m;

	bpp = info->var.bits_per_pixel;
	m = 1 << bpp;
	if (regno >= m) {
		r = -EINVAL;
		goto exit;
	}

	switch (bpp) {
	case 16:
		/* RGB 565 */
		((uint32_t *) (info->pseudo_palette))[regno] =
		    ((red & 0xF800) | ((green & 0xFC00) >> 5) | ((blue & 0xF800) >> 11));
		break;
	case 32:
		/* ARGB8888 */
		((uint32_t *) (info->pseudo_palette))[regno] =
		    (0xff000000) |
		    ((red & 0xFF00) << 8) | ((green & 0xFF00)) | ((blue & 0xFF00) >> 8);
		break;

		/* TODO: RGB888, BGR888, ABGR8888 */

	default:
		ASSERT(0);
	}

exit:
	return r;
}

static int mtkfb_pan_display_impl(struct fb_var_screeninfo *var, struct fb_info *info)
{
	static int first = 1;
	int offset = 0;
	unsigned long paStart = 0;
	char *vaStart = NULL, *vaEnd = NULL;
	int ret = 0;
	struct mtk_disp_config config = {0};
	struct disp_hw_common_info hw_info;

	MTKFB_LOG("pan_display: offset(%u,%u), res(%u,%u), resv(%u,%u).\n",
		var->xoffset, var->yoffset, info->var.xres, info->var.yres, info->var.xres_virtual,
		info->var.yres_virtual);

	if (first) {
		first = 0;
		MTKFB_LOG("skip first picture.\n");
		return 0;
	}
	ret = disp_hw_mgr_get_info(&hw_info);
	if (ret != 0) {
		MTKFB_INFO("mtkfb_pan_display_impl get hw mgr info fail\n");
		return -1;
	}

	info->var.yoffset = var->yoffset;
	offset = var->yoffset * info->fix.line_length;
	paStart = fb_bus_addr + offset;
	vaStart = info->screen_base + offset;
	vaEnd = vaStart + info->var.yres * info->fix.line_length;

	config.buffer_num = 1;
	config.buffer_info[0].layer_id = 0;
	config.buffer_info[0].layer_enable = 1;
	config.buffer_info[0].src_base_addr = vaStart;
	config.buffer_info[0].src_phy_addr = (void *)paStart;
	config.buffer_info[0].src_fmt =
		(var->blue.offset == 0) ? DISP_HW_COLOR_FORMAT_BGRA8888 : DISP_HW_COLOR_FORMAT_RGBA8888;
	config.buffer_info[0].type = DISP_LAYER_OSD;
	config.buffer_info[0].src.x = 0;
	config.buffer_info[0].src.y = 0;
	config.buffer_info[0].src.width = var->xres;
	config.buffer_info[0].src.height = var->yres;
	config.buffer_info[0].tgt.x = 0;
	config.buffer_info[0].tgt.y = 0;
	config.buffer_info[0].tgt.width = hw_info.resolution->width;
	config.buffer_info[0].tgt.height = hw_info.resolution->height;
	config.buffer_info[0].alpha_en = 0;
	config.buffer_info[0].ion_fd = -1;
	config.buffer_info[0].acquire_fence_fd = -1;
	config.buffer_info[0].res_mode = hw_info.resolution->res_mode;

	ret = disp_hw_mgr_config(&config);

	return ret;
}

static int mtkfb_pan_display_proxy(struct fb_var_screeninfo *var, struct fb_info *info)
{
	return mtkfb_pan_display_impl(var, info);
}

static int mtkfb_soft_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	return 0;
}

/* Set fb_info.fix fields and also updates fbdev.
 * When calling this fb_info.var must be set up already.
 */
static void set_fb_fix(struct mtkfb_device *fbdev)
{
	struct fb_info *fbi = fbdev->fb_info;
	struct fb_fix_screeninfo *fix = &fbi->fix;
	struct fb_var_screeninfo *var = &fbi->var;
	struct fb_ops *fbops = fbi->fbops;

	strncpy(fix->id, MTKFB_DRIVER, sizeof(fix->id));
	fix->type = FB_TYPE_PACKED_PIXELS;

	switch (var->bits_per_pixel) {
	case 16:
	case 24:
	case 32:
		fix->visual = FB_VISUAL_TRUECOLOR;
		break;
	case 1:
	case 2:
	case 4:
	case 8:
		fix->visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	default:
		ASSERT(0);
	}

	fix->accel = FB_ACCEL_NONE;
	fix->line_length = ALIGN_TO(var->xres_virtual, MTK_FB_ALIGNMENT) * var->bits_per_pixel / 8;
	fix->smem_len = fbdev->fb_size_in_byte;
	fix->smem_start = fbdev->fb_pa_base;

	fix->xpanstep = 0;
	fix->ypanstep = 1;

	fbops->fb_fillrect = cfb_fillrect;
	fbops->fb_copyarea = cfb_copyarea;
	fbops->fb_imageblit = cfb_imageblit;
}

static int mtkfb_check_var(struct fb_var_screeninfo *var, struct fb_info *fbi)
{
	unsigned int bpp;
	unsigned long max_frame_size;
	unsigned long line_size;

	struct mtkfb_device *fbdev = (struct mtkfb_device *)fbi->par;

	MTKFB_LOG("mtkfb_check_var, xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u,\n",
		var->xres, var->yres, var->xres_virtual, var->yres_virtual);
	MTKFB_LOG("xoffset=%u, yoffset=%u, bits_per_pixel=%u\n", var->xoffset, var->yoffset, var->bits_per_pixel);

	bpp = var->bits_per_pixel;

	if (bpp != 16 && bpp != 24 && bpp != 32) {
		MTKFB_ERR("[%s]unsupported bpp: %d", __func__, bpp);
		return -1;
	}

	switch (var->rotate) {
	case 0:
	case 180:
		var->xres = MTK_FB_XRES;
		var->yres = MTK_FB_YRES;
		break;
	case 90:
	case 270:
		var->xres = MTK_FB_YRES;
		var->yres = MTK_FB_XRES;
		break;
	default:
		return -1;
	}

	if (var->xres_virtual < var->xres)
		var->xres_virtual = var->xres;
	if (var->yres_virtual < var->yres)
		var->yres_virtual = var->yres;

	max_frame_size = fbdev->fb_size_in_byte;
	MTKFB_LOG("fbdev->fb_size_in_byte=0x%08lx\n", fbdev->fb_size_in_byte);
	line_size = var->xres_virtual * bpp / 8;

	if (line_size * var->yres_virtual > max_frame_size) {
		/* Try to keep yres_virtual first */
		line_size = max_frame_size / var->yres_virtual;
		var->xres_virtual = line_size * 8 / bpp;
		if (var->xres_virtual < var->xres) {
			/* Still doesn't fit. Shrink yres_virtual too */
			var->xres_virtual = var->xres;
			line_size = var->xres * bpp / 8;
			var->yres_virtual = max_frame_size / line_size;
		}
	}
	MTKFB_LOG("mtkfb_check_var, xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u,\n",
		var->xres, var->yres, var->xres_virtual, var->yres_virtual);
	MTKFB_LOG("xoffset=%u, yoffset=%u, bits_per_pixel=%u\n", var->xoffset, var->yoffset, var->bits_per_pixel);

	if (var->xres + var->xoffset > var->xres_virtual)
		var->xoffset = var->xres_virtual - var->xres;
	if (var->yres + var->yoffset > var->yres_virtual)
		var->yoffset = var->yres_virtual - var->yres;


	MTKFB_LOG("mtkfb_check_var, xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u,\n",
		var->xres, var->yres, var->xres_virtual, var->yres_virtual);
	MTKFB_LOG("xoffset=%u, yoffset=%u, bits_per_pixel=%u\n", var->xoffset, var->yoffset, var->bits_per_pixel);

	if (bpp == 16) {
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->transp.length = 0;
	} else if (bpp == 24) {
		var->red.length = var->green.length = var->blue.length = 8;
		var->transp.length = 0;

		/* Check if format is RGB565 or BGR565 */

		ASSERT(var->green.offset == 8);
		ASSERT((var->red.offset == 16 + var->blue.offset) || (var->blue.offset == 16 + var->red.offset));
		ASSERT(var->red.offset == 16 || var->red.offset == 0);
	} else if (bpp == 32) {
		var->red.length = var->green.length = var->blue.length = var->transp.length = 8;

		/* Check if format is ARGB565 or ABGR565 */

		ASSERT(var->green.offset == 8 && var->transp.offset == 24);
		ASSERT((var->red.offset == 16 + var->blue.offset) || (var->blue.offset == 16 + var->red.offset));
		ASSERT(var->red.offset == 16 || var->red.offset == 0);
	}

	var->red.msb_right = var->green.msb_right = var->blue.msb_right = var->transp.msb_right = 0;

	var->activate = FB_ACTIVATE_NOW;

	var->height = UINT_MAX;
	var->width = UINT_MAX;
	var->grayscale = 0;
	var->nonstd = 0;

	var->pixclock = UINT_MAX;
	var->left_margin = UINT_MAX;
	var->right_margin = UINT_MAX;
	var->upper_margin = UINT_MAX;
	var->lower_margin = UINT_MAX;
	var->hsync_len = UINT_MAX;
	var->vsync_len = UINT_MAX;

	var->vmode = FB_VMODE_NONINTERLACED;
	var->sync = 0;

	return 0;

}

static int mtkfb_set_par(struct fb_info *fbi)
{
	struct mtkfb_device *fbdev = (struct mtkfb_device *)fbi->par;

	set_fb_fix(fbdev);

	return 0;
}

static int mtkfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

#ifdef CONFIG_COMPAT
static int mtkfb_compat_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	return mtkfb_ioctl(info, cmd, arg);
}
#endif

#ifdef CONFIG_PM
static void mtkfb_blank_suspend(void)
{
	int ret = 0;

	MTKFB_LOG("enter early_suspend\n");

	ret = disp_hw_mgr_suspend();
	if (ret < 0) {
		MTKFB_ERR("suspend failed\n");
		return;
	}

	MTKFB_LOG("leave early_suspend\n");

}

static void mtkfb_blank_resume(void)
{
	int ret = 0;

	MTKFB_LOG("enter late_resume\n");

	ret = disp_hw_mgr_resume();
	if (ret) {
		MTKFB_ERR("primary display resume failed\n");
		return;
	}

	MTKFB_LOG("leave late_resume\n");
}
#endif

static int mtkfb_blank(int blank_mode, struct fb_info *info)
{
	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
	case FB_BLANK_NORMAL:
		pr_info("mtkfb enter late_resume\n");
#ifdef CONFIG_PM
		mtkfb_blank_resume();
		msleep(30);
#endif
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
		break;
	case FB_BLANK_POWERDOWN:
		pr_info("mtkfb enter early_suspend\n");
#ifdef CONFIG_PM
		mtkfb_blank_suspend();
#endif
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


static struct fb_ops mtkfb_ops = {
	.owner = THIS_MODULE,
	.fb_open = mtkfb_open,
	.fb_release = mtkfb_release,
	.fb_setcolreg = mtkfb_setcolreg,
	.fb_pan_display = mtkfb_pan_display_proxy,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_cursor = mtkfb_soft_cursor,
	.fb_check_var = mtkfb_check_var,
	.fb_set_par = mtkfb_set_par,
	.fb_ioctl = mtkfb_ioctl,
#ifdef CONFIG_COMPAT
	.fb_compat_ioctl = mtkfb_compat_ioctl,
#endif
	.fb_blank = mtkfb_blank,
};

static int mtkfb_fbinfo_init(struct fb_info *info)
{
	struct mtkfb_device *fbdev = (struct mtkfb_device *)info->par;
	struct fb_var_screeninfo var;
	int r = 0;

	WARN_ON(!fbdev->fb_va_base);
	info->fbops = &mtkfb_ops;
	info->flags = FBINFO_FLAG_DEFAULT;
	info->screen_base = (char *)fbdev->fb_va_base;
	info->screen_size = fbdev->fb_size_in_byte;
	info->pseudo_palette = fbdev->pseudo_palette;

	r = fb_alloc_cmap(&info->cmap, 32, 0);
	if (r != 0)
		MTKFB_ERR("unable to allocate color map memory\n");

	/* setup the initial video mode (RGB565) */

	memset(&var, 0, sizeof(var));

	var.xres = MTK_FB_XRES;
	var.yres = MTK_FB_YRES;
	var.xres_virtual = MTK_FB_XRESV;
	var.yres_virtual = MTK_FB_YRESV;
	MTKFB_LOG
		("mtkfb_fbinfo_init var.xres=%d,var.yres=%d,var.xres_virtual=%d,var.yres_virtual=%d\n",
		 var.xres, var.yres, var.xres_virtual, var.yres_virtual);
	/* use 32 bit framebuffer as default */
	var.bits_per_pixel = 32;

	var.transp.offset = 24;
	var.red.length = 8;
#if 0
	var.red.offset = 16;
	var.red.length = 8;
	var.green.offset = 8;
	var.green.length = 8;
	var.blue.offset = 0;
	var.blue.length = 8;
#else
	var.red.offset = 0;
	var.red.length = 8;
	var.green.offset = 8;
	var.green.length = 8;
	var.blue.offset = 16;
	var.blue.length = 8;
#endif

	var.width = MTK_FB_XRES;
	var.height = MTK_FB_YRES;

	var.activate = FB_ACTIVATE_NOW;

	r = mtkfb_check_var(&var, info);
	if (r != 0)
		MTKFB_ERR("failed to mtkfb_check_var\n");

	info->var = var;

	r = mtkfb_set_par(info);
	if (r != 0)
		MTKFB_ERR("failed to mtkfb_set_par\n");

	return r;

}

static void mtkfb_fbinfo_cleanup(struct mtkfb_device *fbdev)
{
	fb_dealloc_cmap(&fbdev->fb_info->cmap);
}

static void mtkfb_free_resources(struct mtkfb_device *fbdev, int state)
{
	int r = 0;

	switch (state) {
	case MTKFB_ACTIVE:
		r = unregister_framebuffer(fbdev->fb_info);
		ASSERT(r == 0);
		/* lint -fallthrough */
	case 2:
		mtkfb_fbinfo_cleanup(fbdev);
		/* lint -fallthrough */
	case 1:
		dev_set_drvdata(fbdev->dev, NULL);
		framebuffer_release(fbdev->fb_info);
		/* lint -fallthrough */
	case 0:
		/* nothing to free */
		break;
	default:
		WARN_ON(1);
	}
}

static int mtkfb_allocate_framebuffer(phys_addr_t pa_start, phys_addr_t pa_end, unsigned long *va,
				  unsigned long *mva, struct device *dev)
{
	int ret = 0;
	unsigned long bus_addr = 0;

	/**va = (unsigned long)ioremap_nocache(pa_start, pa_end - pa_start + 1);*/
	*va = (unsigned long)__va(pa_start);
	MTKFB_INFO("disphal_allocate_fb, pa=%pa, va=0x%lx\n", &pa_start, *va);

	bus_addr = (unsigned long)phys_to_dma(dev, pa_start);

	MTKFB_INFO("disphal_allocate_fb, mva=0x%lx\n", bus_addr);

	*mva = bus_addr;

	MTKFB_INFO("disphal_allocate_fb, bus=0x%lx\n", *mva);

	return ret;
}

void mtkfb_free_reserved_fb(void)
{
	MTKFB_INFO(" free fb buffer pa 0x%x, size 0x%x\n",
		(unsigned int)framebuffer_base, framebuffer_size);

	if (framebuffer_base) {
		free_reserved_area((void *)__va(framebuffer_base),
			(void *)__va(framebuffer_base + framebuffer_size - 1), -1, "fb");
		framebuffer_base = 0;
	}

}

static void _parse_tag_videolfb(struct device *dev)
{
#ifdef MTK_NO_DISP_IN_LK
	struct device_node *np;
	struct resource res;
#else
	int offset;
	const char *p;
	int l;
	const void *fdt = initial_boot_params;
	struct tag_videolfb *videolfb_tag = NULL;
#endif

	if (is_videofb_parse_done)
		return;

#ifdef MTK_NO_DISP_IN_LK
	if (dev == NULL)
		return;
	/*need to get fb base, vramsize, after LK part is ready */
	np = of_parse_phandle(dev->of_node, "memory-region", 1);
	if (np == NULL) {
		MTKFB_ERR("No memory-region specified\n");
		return;
	}

	if (of_address_to_resource(np, 0, &res))
		pr_err("mtkfb of_address_to_resource failed\n");

	framebuffer_base = (unsigned int)res.start;
	framebuffer_size = (unsigned int)resource_size(&res);
	g_hdmi_res = HDMI_VIDEO_1920x1080p_60Hz;
#else
	offset = fdt_path_offset(fdt, "/chosen");
	if (offset < 0)
		offset = fdt_path_offset(fdt, "/chosen@0");
	if (offset < 0) {
		MTKFB_ERR("can not find chosen.\n");
		return;
	}

	p = fdt_getprop(fdt, offset, "atag,videolfb", &l);
	if (!p || !l) {
		MTKFB_ERR("can not find videolfb.\n");
		return;
	}

	videolfb_tag = (struct tag_videolfb *)(p + 8);
	if (videolfb_tag) {
		framebuffer_base = videolfb_tag->lfb_base;
		framebuffer_size = videolfb_tag->lfb_size;
		g_hdmi_res = videolfb_tag->lfb_res;
		g_hdmi_colorspace = videolfb_tag->lfb_colorspace;
		g_hdmi_colordepth = videolfb_tag->lfb_colordepth;
		g_force_dolby = videolfb_tag->lfb_force_dolby;
		g_hdr_type = videolfb_tag->lfb_hdr_type;
		g_out_format = videolfb_tag->lfb_out_format;
	}
#endif
	MTKFB_INFO("[DT][dts]fb_base    = 0x%x\n", (unsigned int)(framebuffer_base));
	MTKFB_INFO("[DT][dts]vram       = 0x%x\n", framebuffer_size);
	MTKFB_INFO("[DT][videolfb]hdmi_res   = %d\n", g_hdmi_res);
	MTKFB_INFO("[DT][videolfb]colorspace   = %d\n", g_hdmi_colorspace);
	MTKFB_INFO("[DT][videolfb]colordepth   = %d\n", g_hdmi_colordepth);
	MTKFB_INFO("[DT][videolfb]force dolby   = %d\n", g_force_dolby);
	MTKFB_INFO("[DT][videolfb]hdr_type   = %d\n", g_hdr_type);
	MTKFB_INFO("[DT][videolfb]out_format  = %d\n",g_out_format);

	is_videofb_parse_done = 1;
}

static int mtkfb_probe(struct platform_device *pdev)
{
	struct mtkfb_device *fbdev = NULL;
	struct fb_info *fbi;
	int ret = 0;
	int state = 0;
	unsigned int vramsize = 0;
	phys_addr_t fb_base = 0;
#ifdef CONFIG_MTK_IOMMU
	int i = 0;
	int max_larb_num = 0;
	struct device_node *larb_node;
	struct platform_device *larb_pdev;
#endif

	if (!disp_clock_register_success()) {
		MTKFB_LOG("display clk is not ready, return defer.\n");
		return -EPROBE_DEFER;
	}

	fbi = framebuffer_alloc(sizeof(struct mtkfb_device), &(pdev->dev));
	if (!fbi) {
		MTKFB_ERR("unable to allocate memory for device info\n");
		ret = -ENOMEM;
		goto cleanup;
	}

	fbdev = (struct mtkfb_device *)fbi->par;
	fbdev->fb_info = fbi;
	fbdev->dev = &(pdev->dev);
	dev_set_drvdata(&(pdev->dev), fbdev);

#ifdef CONFIG_MTK_IOMMU
	max_larb_num = of_count_phandle_with_args(pdev->dev.of_node, "mediatek,larb", NULL);
	for (i = 0; i < max_larb_num; i++) {
		larb_node = of_parse_phandle(pdev->dev.of_node, "mediatek,larb", i);
		if (!larb_node)
			return -EINVAL;

		larb_pdev = of_find_device_by_node(larb_node);
		of_node_put(larb_node);
		if ((!larb_pdev) || (!larb_pdev->dev.driver)) {
			MTKFB_LOG("disp_probe is earlier than SMI\n");
			return -EPROBE_DEFER;
		}
		disp_clock_set_larb_dev(&larb_pdev->dev, i);
	}
#endif
	DAL_Init(&(pdev->dev));

	_parse_tag_videolfb(&(pdev->dev));
	fb_base = framebuffer_base;
	vramsize = framebuffer_size;

	mtkfb_allocate_framebuffer(fb_base,
			(fb_base + (unsigned long)vramsize - 1),
			(unsigned long *)&fbdev->fb_va_base, &fb_bus_addr, &(pdev->dev));
	fbdev->fb_pa_base = fb_base;
	fb_va_base = fbdev->fb_va_base;
	state++;
	/* temporary resolution , need to get it from lk */

	disp_hw_mgr_init(&(pdev->dev), g_hdmi_res);

	MTK_FB_BPP = 32;
	MTK_FB_PAGES = 3;
	MTKFB_LOG
		("MTK_FB_XRES=%d, MTKFB_YRES=%d, MTKFB_BPP=%d, MTK_FB_PAGES=%d, MTKFB_LINE=%d, MTKFB_SIZEV=%d\n",
		 MTK_FB_XRES, MTK_FB_YRES, MTK_FB_BPP, MTK_FB_PAGES, MTK_FB_LINE, MTK_FB_SIZEV);
	fbdev->fb_size_in_byte = MTK_FB_SIZEV;

	/* TODO : get frame buffer address */
	ret = mtkfb_fbinfo_init(fbi);
	if (ret) {
		MTKFB_ERR("mtkfb_fbinfo_init fail, r = %d\n", ret);
		goto cleanup;
	}

	mtkfb_fbi = fbi;
	state++;
	ret = register_framebuffer(fbi);
	if (ret != 0) {
		MTKFB_ERR("register_framebuffer failed\n");
		goto cleanup;
	}

	fbdev->state = MTKFB_ACTIVE;

	return 0;
cleanup:
	mtkfb_free_resources(fbdev, state);
	MTKFB_INFO("mtkfb_probe end\n");
	return ret;

}

phys_addr_t mtkfb_get_fb_base(void)
{
	phys_addr_t base = 0;

	_parse_tag_videolfb(NULL);

	if (framebuffer_base)
		base = framebuffer_base;
	else
		base = 0x50000000;

	MTKFB_LOG("mtkfb_get_fb_base=0x%x\n", (unsigned int)base);
	return base;

}
EXPORT_SYMBOL(mtkfb_get_fb_base);

size_t mtkfb_get_fb_size(void)
{
	unsigned int vramsize = 0;

	_parse_tag_videolfb(NULL);

	if (framebuffer_size)
		vramsize = framebuffer_size;
	else
		vramsize = MTK_FB_SIZE * 3;

	MTKFB_LOG("mtkfb_get_fb_size=0x%x\n", vramsize);
	return vramsize;

}
EXPORT_SYMBOL(mtkfb_get_fb_size);

static int mtkfb_remove(struct device *dev)
{
	struct mtkfb_device *fbdev = dev_get_drvdata(dev);
	enum mtkfb_state saved_state = fbdev->state;

	fbdev->state = MTKFB_DISABLED;
	mtkfb_free_resources(fbdev, saved_state);
	return 0;
}

static int mtkfb_suspend(struct device *pdev, pm_message_t mesg)
{
	MTKFB_LOG("[FB Driver] mtkfb_suspend(): 0x%x\n", mesg.event);

	return 0;
}

static int mtkfb_resume(struct device *pdev)
{
	MTKFB_LOG("[FB Driver] mtkfb_resume()\n");

	return 0;
}

static void mtkfb_shutdown(struct platform_device *pdev)
{
	MTKFB_LOG("[FB Driver] mtkfb_shutdown()\n");

	msleep(30);

	if (disp_hw_mgr_is_slept()) {
		MTKFB_LOG("mtkfb has been power off\n");
		return;
	}

	disp_hw_mgr_suspend();
	MTKFB_LOG("[FB Driver] leave mtkfb_shutdown\n");
}

static const struct of_device_id mtkfb_of_ids[] = {
	{.compatible = "mediatek,mtkfb",},
	{}
};

static struct platform_driver mtkfb_driver = {
	.probe = mtkfb_probe,
	.shutdown = mtkfb_shutdown,
	.driver = {
		   .name = MTKFB_DRIVER,
		   .bus = &platform_bus_type,
		   .remove = mtkfb_remove,
		   .suspend = mtkfb_suspend,
		   .resume = mtkfb_resume,
		   .of_match_table = mtkfb_of_ids,
		   },
};

/* Register both the driver and the device */
int __init mtkfb_init(void)
{
	int r = 0;

	if (platform_driver_register(&mtkfb_driver)) {
		MTKFB_ERR("failed to register mtkfb driver\n");
		r = -ENODEV;
		goto exit;
	}
exit:
	return r;
}

static void __exit mtkfb_cleanup(void)
{
	platform_driver_unregister(&mtkfb_driver);
}

module_init(mtkfb_init);
module_exit(mtkfb_cleanup);
MODULE_DESCRIPTION("MEDIATEK framebuffer driver");
MODULE_AUTHOR("Chuanfei Wang <Chuanfei.Wang@mediatek.com>");
MODULE_LICENSE("GPL");
