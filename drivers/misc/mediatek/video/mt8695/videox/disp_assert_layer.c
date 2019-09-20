/*
* Copyright (C) 2016 MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See http://www.gnu.org/licenses/gpl-2.0.html for more details.
*/

#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>

#include "disp_assert_layer.h"

/* /common part */
#define DAL_BPP             (2)
#define DAL_WIDTH           (1920)
#define DAL_HEIGHT          (1080)

/* #ifdef CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER */

#include <linux/string.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <linux/module.h>

#include "mtkfb_console.h"
#include "disp_hw_mgr.h"
#include "mtkfb.h"
#include "disp_info.h"
#include "ion_drv.h"
#include "mtk_ion.h"

/* --------------------------------------------------------------------------- */
#define DAL_FORMAT          (DISP_HW_COLOR_FORMAT_RGB565)
#define DAL_BG_COLOR        (dal_bg_color)
#define DAL_FG_COLOR        (dal_fg_color)

#define RGB888_To_RGB565(x) ((((x) & 0xF80000) >> 8) |                      \
			     (((x) & 0x00FC00) >> 5) |                      \
			     (((x) & 0x0000F8) >> 3))

#define MAKE_TWO_RGB565_COLOR(high, low)  (((low) << 16) | (high))

#define DAL_LOG(fmt, arg...)	pr_debug("DISP/DAL " fmt, ##arg)
/* --------------------------------------------------------------------------- */

static MFC_HANDLE mfc_handle;

static void *dal_fb_addr;
unsigned int isAEEEnabled;
static bool isInit;
bool dal_shown;
static unsigned int dal_fg_color = RGB888_To_RGB565(DAL_COLOR_WHITE);
static unsigned int dal_bg_color = RGB888_To_RGB565(DAL_COLOR_RED);

DEFINE_SEMAPHORE(dal_sem);

static char dal_print_buffer[1024];
/* --------------------------------------------------------------------------- */

uint32_t DAL_GetLayerSize(void)
{
	/* xuecheng, avoid lcdc read buffersize+1 issue */
	return DAL_WIDTH * DAL_HEIGHT * DAL_BPP + 4096;
}

enum DAL_STATUS DAL_SetScreenColor(enum DAL_COLOR color)
{
	uint32_t i;
	uint32_t size;
	uint32_t BG_COLOR;
	struct MFC_CONTEXT *ctxt = NULL;
	uint32_t offset;
	unsigned int *addr;

	color = RGB888_To_RGB565(color);
	BG_COLOR = MAKE_TWO_RGB565_COLOR(color, color);

	ctxt = (struct MFC_CONTEXT *) mfc_handle;
	if (!ctxt)
		return DAL_STATUS_FATAL_ERROR;
	if (ctxt->screen_color == color)
		return DAL_STATUS_OK;
	offset = MFC_Get_Cursor_Offset(mfc_handle);
	addr = (unsigned int *)(ctxt->fb_addr + offset);
	size = DAL_GetLayerSize() - offset;

	for (i = 0; i < size / sizeof(uint32_t); ++i)
		*addr++ = BG_COLOR;

	ctxt->screen_color = color;

	return DAL_STATUS_OK;
}
EXPORT_SYMBOL(DAL_SetScreenColor);

static void *aee_va;
static int aee_mva;
static int _get_buffer(struct device *dev)
{
	int size = (1920 * 1080 * 2 + 4096);

	if (aee_mva == 0)
		aee_va = dma_alloc_coherent
			(dev, size, (dma_addr_t *)&aee_mva, GFP_KERNEL);

	return 0;
}


enum DAL_STATUS DAL_Init(struct device *dev)
{
	enum MFC_STATUS ret;

	if (isInit)
		return DAL_STATUS_OK;

	_get_buffer(dev);
	if (aee_va == 0x0)
		return 0;

	dal_fb_addr = aee_va;

	pr_debug("%s, layerVA=%p, layerPA=0x%x\n", __func__, aee_va, aee_mva);

	ret = MFC_Open(&mfc_handle, dal_fb_addr, DAL_WIDTH, DAL_HEIGHT, DAL_BPP, DAL_FG_COLOR, DAL_BG_COLOR);
	if (ret != MFC_STATUS_OK) {
		pr_debug("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, ret);
		return DAL_STATUS_FATAL_ERROR;
	}

	/* DAL_Clean(); */
	DAL_SetScreenColor(DAL_COLOR_RED);

	isInit = true;

	return DAL_STATUS_OK;
}


enum DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor)
{
	enum MFC_STATUS ret;

	if (mfc_handle == NULL)
		return DAL_STATUS_NOT_READY;

	if (down_interruptible(&dal_sem)) {
		pr_debug("DISP/DAL Can't get semaphore in %s()\n", __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	dal_fg_color = RGB888_To_RGB565(fgColor);
	dal_bg_color = RGB888_To_RGB565(bgColor);

	ret = MFC_SetColor(mfc_handle, dal_fg_color, dal_bg_color);
	if (ret != MFC_STATUS_OK) {
		pr_debug("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, ret);
		return DAL_STATUS_FATAL_ERROR;
	}

	up(&dal_sem);

	return DAL_STATUS_OK;
}
EXPORT_SYMBOL(DAL_SetColor);

enum DAL_STATUS DAL_Dynamic_Change_FB_Layer(unsigned int isAEEEnabled)
{
	return DAL_STATUS_OK;
}

struct mtk_disp_config config = {0};

enum DAL_STATUS DAL_Clean(void)
{
	enum DAL_STATUS ret = DAL_STATUS_OK;
	enum MFC_STATUS r;

	/* static int dal_clean_cnt; */
	struct MFC_CONTEXT *ctxt = (struct MFC_CONTEXT *) mfc_handle;

	if (mfc_handle == NULL)
		return DAL_STATUS_NOT_READY;

	if (down_interruptible(&dal_sem)) {
		pr_debug("DISP/DAL Can't get semaphore in %s()\n", __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	r = MFC_ResetCursor(mfc_handle);
	if (r != MFC_STATUS_OK) {
		pr_debug("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, r);
		up(&dal_sem);
		return DAL_STATUS_FATAL_ERROR;
	}

	ctxt->screen_color = 0;
	DAL_SetScreenColor(DAL_COLOR_RED);

	/* TODO: if dal_shown=false, and 3D enabled, mtkfb may disable UI layer, please modify 3D driver */
	if (isAEEEnabled == 1) {
		struct disp_hw_common_info hw_info;

		memset(&config, 0, sizeof(struct mtk_disp_config));

		ret = disp_hw_mgr_get_info(&hw_info);
		if (ret != 0) {
			pr_debug("DAL_Clean get hw mgr info fail\n");
			return -1;
		}

		config.user = DISP_USER_AEE;
		config.buffer_num = 1;
		config.buffer_info[0].layer_id = 0;
		config.buffer_info[0].layer_enable = 0;
		config.buffer_info[0].src_base_addr = NULL;
		config.buffer_info[0].src_phy_addr = NULL;
		config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_RGB565;
		config.buffer_info[0].type = DISP_LAYER_OSD;
		config.buffer_info[0].src.x = 0;
		config.buffer_info[0].src.y = 0;
		config.buffer_info[0].src.pitch = DAL_WIDTH;
		config.buffer_info[0].src.width = DAL_WIDTH;
		config.buffer_info[0].src.height = DAL_HEIGHT;
		config.buffer_info[0].tgt.x = 0;
		config.buffer_info[0].tgt.y = 0;
		config.buffer_info[0].alpha = 0x80;
		config.buffer_info[0].alpha_en = true;
		config.buffer_info[0].tgt.width = hw_info.resolution->width;
		config.buffer_info[0].tgt.height = hw_info.resolution->height;
		config.buffer_info[0].ion_fd = -1;
		config.buffer_info[0].acquire_fence_fd = -1;

		ret = disp_hw_mgr_config(&config);
		isAEEEnabled = 0;
	}

	up(&dal_sem);

	return ret;
}
EXPORT_SYMBOL(DAL_Clean);

int is_DAL_Enabled(void)
{
	int ret = 0;

	ret = isAEEEnabled;

	return ret;
}

unsigned long get_Assert_Layer_PA(void)
{
	return 0;
}

enum DAL_STATUS DAL_Printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	enum DAL_STATUS ret = DAL_STATUS_OK;
	enum MFC_STATUS r;
	struct disp_hw_common_info hw_info;

	if (mfc_handle == NULL)
		return DAL_STATUS_NOT_READY;

	if (fmt == NULL)
		return DAL_STATUS_INVALID_ARGUMENT;

	memset(&config, 0, sizeof(struct mtk_disp_config));

	if (down_interruptible(&dal_sem)) {
		pr_debug("DISP/DAL Can't get semaphore in %s()\n",  __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	if (isAEEEnabled == 0) {
		ret = disp_hw_mgr_get_info(&hw_info);
		if (ret != 0) {
			pr_debug("DAL_Printf get hw mgr info fail\n");
			return -1;
		}
		config.user = DISP_USER_AEE;
		config.buffer_num = 1;
		config.buffer_info[0].layer_id = 0;
		config.buffer_info[0].layer_enable = 1;
		config.buffer_info[0].src_base_addr = aee_va;
		config.buffer_info[0].src_phy_addr = (void *)((unsigned long)aee_mva);
		config.buffer_info[0].src_fmt = DISP_HW_COLOR_FORMAT_RGB565;
		config.buffer_info[0].type = DISP_LAYER_OSD;
		config.buffer_info[0].src.x = 0;
		config.buffer_info[0].src.y = 0;
		config.buffer_info[0].src.pitch = DAL_WIDTH;
		config.buffer_info[0].src.width = DAL_WIDTH;
		config.buffer_info[0].src.height = DAL_HEIGHT;
		config.buffer_info[0].tgt.x = 0;
		config.buffer_info[0].tgt.y = 0;
		config.buffer_info[0].alpha = 0x80;
		config.buffer_info[0].alpha_en = true;
		config.buffer_info[0].tgt.width = hw_info.resolution->width;
		config.buffer_info[0].tgt.height = hw_info.resolution->height;
		config.buffer_info[0].ion_fd = -1;
		config.buffer_info[0].acquire_fence_fd = -1;

		ret = disp_hw_mgr_config(&config);

		isAEEEnabled = 1;
	}

	va_start(args, fmt);
	i = vsprintf(dal_print_buffer, fmt, args);
	WARN_ON(i >= ARRAY_SIZE(dal_print_buffer));
	va_end(args);

	r = MFC_Print(mfc_handle, dal_print_buffer);
	if (r != MFC_STATUS_OK) {
		pr_debug("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, r);
		ret = DAL_STATUS_FATAL_ERROR;
	} else {
		/*flush_cache_all();*/
		if (!dal_shown)
			dal_shown = true;
	}

	up(&dal_sem);

	return ret;
}
EXPORT_SYMBOL(DAL_Printf);

enum DAL_STATUS DAL_Get_Layer_Info(struct mtk_disp_buffer *info)
{
	int layer = DISP_BUFFER_MAX - 1;
	struct disp_hw_common_info hw_info;

	disp_hw_mgr_get_info(&hw_info);

	info->layer_id = layer;
	info->layer_enable = 1;
	info->src_base_addr = aee_va;
	info->src_phy_addr = (void *)((unsigned long)aee_mva);
	info->src_fmt = DISP_HW_COLOR_FORMAT_RGB565;
	info->type = DISP_LAYER_AEE;
	info->src.x = 0;
	info->src.y = 0;
	info->src.pitch = DAL_WIDTH;
	info->src.width = DAL_WIDTH;
	info->src.height = DAL_HEIGHT;
	info->tgt.x = 0;
	info->tgt.y = 0;
	info->alpha = 0x80;
	info->alpha_en = true;
	info->tgt.width = hw_info.resolution->width;
	info->tgt.height = hw_info.resolution->height;
	info->ion_fd = -1;
	info->acquire_fence_fd = -1;

	return DAL_STATUS_OK;
}

enum DAL_STATUS DAL_OnDispPowerOn(void)
{
	return DAL_STATUS_OK;
}
