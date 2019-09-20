/*
* disp_hw_log.c - display MMP/MET/log
*
*  Copyright (C) 2015-2016 MediaTek Inc.
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
#define LOG_TAG "LOG"
#include "disp_hw_log.h"
#ifdef CONFIG_MTK_IOMMU
#include "linux/iommu.h"
#endif

int down_sample_x = 10;
int down_sample_y = 10;
int dump_enable;

struct mtk_disp_log disp_log = {
#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
	.mmp_event = {
		{.level = MMP_LEVEL_0, .name = "Display"},

		{.level = MMP_LEVEL_1, .name = "HWC"},
		{.level = MMP_LEVEL_2, .name = "Config"},
		{.level = MMP_LEVEL_2, .name = "WaitVsync"},

		{.level = MMP_LEVEL_1, .name = "HWMgr"},
		{.level = MMP_LEVEL_2, .name = "HW_Config"},
		{.level = MMP_LEVEL_2, .name = "Config_done"},
		{.level = MMP_LEVEL_2, .name = "Change_res"},
		{.level = MMP_LEVEL_2, .name = "Event"},

		{.level = MMP_LEVEL_1, .name = "Bitmap"},
		{.level = MMP_LEVEL_2, .name = "Bitmap_Mvdo"},
		{.level = MMP_LEVEL_2, .name = "Bitmap_Svdo"},
		{.level = MMP_LEVEL_2, .name = "Bitmap_Osd1"},
		{.level = MMP_LEVEL_2, .name = "Bitmap_Osd2"},

		{.level = MMP_LEVEL_1, .name = "IRQ"},
		{.level = MMP_LEVEL_2, .name = "FMT"},
		{.level = MMP_LEVEL_2, .name = "VDP"},

		{.level = MMP_LEVEL_1, .name = "MDP"},
		{.level = MMP_LEVEL_2, .name = "Config"},
		{.level = MMP_LEVEL_2, .name = "fill"},
		{.level = MMP_LEVEL_2, .name = "fence_create"},
		{.level = MMP_LEVEL_2, .name = "get ticket"},
		{.level = MMP_LEVEL_2, .name = "async mode"},
		{.level = MMP_LEVEL_2, .name = "async"},
		{.level = MMP_LEVEL_2, .name = "set_hw"},
		{.level = MMP_LEVEL_2, .name = "got_irq"},
		{.level = MMP_LEVEL_2, .name = "HW done"},
		{.level = MMP_LEVEL_3, .name = "fence_release"},
		{.level = MMP_LEVEL_2, .name = "Config_done"},
	},
#endif
};

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
void DISP_MMP(enum MMP_DISP_EVENT event, enum MMP_DISP_EVENT_TYPE type,
			uint64_t data1, uint64_t data2)
{
	if (disp_log.mmp_event[event].value == 0) {
		DISP_LOG_E("mmp event %d is not register\n", event);
		return;
	}

	DISP_LOG_MMP("%s %d %d\n", disp_log.mmp_event[event].name,
			(uint32_t)data1, (uint32_t)data2);

	switch (type) {
	case MMP_START:
		mmprofile_log_ex(disp_log.mmp_event[event].value,
				MMPROFILE_FLAG_START, data1, data2);
		break;
	case MMP_END:
		mmprofile_log_ex(disp_log.mmp_event[event].value,
				MMPROFILE_FLAG_END, data1, data2);
		break;
	case MMP_PULSE:
		mmprofile_log_ex(disp_log.mmp_event[event].value,
				MMPROFILE_FLAG_PULSE, data1, data2);
		break;
	default:
		break;
	}
}


#define MVA_LINE_MAP
#ifdef MVA_LINE_MAP
struct page **pages;
void *disp_mmp_map_mva_to_va(unsigned long mva, unsigned int size)
{
	void *va = NULL;
	phys_addr_t pa;
#ifdef CONFIG_MTK_IOMMU
	struct device *dev = disp_hw_mgr_get_dev();
	struct iommu_domain *domain;
	unsigned int addr;
	unsigned int page_count;
	unsigned int i = 0;

	if (dev) {
		domain = iommu_get_domain_for_dev(dev);
		pa = iommu_iova_to_phys(domain, (dma_addr_t) mva);

		/* line map */
		page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
		pages = kmalloc((sizeof(struct page *) * page_count), GFP_KERNEL);
		DISP_LOG_D("page count: %d\n", page_count);
		for (i = 0; i < page_count; i++) {
			addr = iommu_iova_to_phys(domain, (dma_addr_t) (mva + i * PAGE_SIZE));
			pages[i] = pfn_to_page(addr >> PAGE_SHIFT);
		}

		va = vmap(pages, page_count, VM_MAP, PAGE_KERNEL);
		DISP_LOG_D("ddp_map_mva_to_va: mva=0x%lx, pa=0x%lx, va=0x%p, size=%d\n",
		       mva, (unsigned long)pa, va, size);
	}
#endif
	return va;
}

void disp_mmp_umap_va(void *va)
{
	if (va) {
		vunmap(va);
		kfree(pages);
	}
}
#endif

void DISP_MMP_DUMP(enum MMP_DISP_EVENT event, unsigned long addr, uint32_t size,
			enum DISP_HW_COLOR_FORMAT fmt, uint32_t w, uint32_t h)
{
	mmp_metadata_bitmap_t Bitmap;
	mmp_metadata_t meta;
	mmp_pixel_format format;
	uint32_t bpp;
	int raw = 0;

	if (disp_log.mmp_event[event].value == 0) {
		DISP_LOG_E("mmp data event %d is not register\n", event);
		return;
	}

	if (addr == 0 || w == 0 || h == 0) {
		DISP_LOG_E("mmp invalid parameter: addr_0x%lx, w_%d, h_%d\n", (unsigned long int)addr, w, h);
		return;
	}

	if (!dump_enable)
		return;

	switch (fmt) {
	case DISP_HW_COLOR_FORMAT_RGB565:
		format = MMPROFILE_BITMAP_RGB565;
		bpp = 16;
		break;
	case DISP_HW_COLOR_FORMAT_RGB888:
		format = MMPROFILE_BITMAP_RGB888;
		bpp = 24;
		break;
	case DISP_HW_COLOR_FORMAT_BGR888:
		format = MMPROFILE_BITMAP_BGR888;
		bpp = 24;
		break;
	case DISP_HW_COLOR_FORMAT_ARGB8888:
	case DISP_HW_COLOR_FORMAT_RGBA8888:
		format = MMPROFILE_BITMAP_RGBA8888;
		bpp = 32;
		break;
	case DISP_HW_COLOR_FORMAT_BGRA8888:
	case DISP_HW_COLOR_FORMAT_ABGR8888:
		format = MMPROFILE_BITMAP_BGRA8888;
		bpp = 32;
		break;
	default:
		DISP_LOG_E("unknown format=0x%x, dump raw\n", fmt);
		raw = 1;
	}

	if (!raw) {
		Bitmap.data1 = size;
		Bitmap.data2 = fmt;
		Bitmap.format = format;
		Bitmap.bpp = bpp;
		Bitmap.start_pos = 0;
		Bitmap.width = w;
		Bitmap.height = h;
		Bitmap.pitch = w*bpp/8;
		Bitmap.data_size = size;
		Bitmap.down_sample_x = down_sample_x;
		Bitmap.down_sample_y = down_sample_y;
		Bitmap.p_data = (void *)addr;/*disp_mmp_map_mva_to_va(addr, size);*/
		if (Bitmap.p_data) {
			mmprofile_log_meta_bitmap(disp_log.mmp_event[event].value,
						MMPROFILE_FLAG_PULSE, &Bitmap);
			/*disp_mmp_umap_va(Bitmap.p_data);*/
		}
	} else {
		meta.data1 = size;
		meta.data2 = fmt;
		meta.data_type = MMPROFILE_META_RAW;
		meta.p_data = (void *)addr;/*disp_mmp_map_mva_to_va(addr, size);*/
		meta.size = size;
		if (meta.p_data) {
			mmprofile_log_meta(disp_log.mmp_event[event].value,
						MMPROFILE_FLAG_PULSE, &meta);
			/*disp_mmp_umap_va(Bitmap.p_data);*/
		}
	}
}


static int disp_mmp_init(void)
{
	int i;
	struct mmp_disp_event *event = disp_log.mmp_event;
	MMP_Event *parent = disp_log.parent;

	mmprofile_enable(1);

	for (i = 0; i < MMP_EVENTS_MAX; i++) {
		event = &(disp_log.mmp_event[i]);
		if (event->level == MMP_LEVEL_0) {
			event->value = mmprofile_register_event(MMP_ROOT_EVENT, event->name);
			parent[event->level] = event->value;
			DISP_LOG_D("disp_mmp_init value=%d, name=%s\n", event->value, event->name);
		} else {
			event->value = mmprofile_register_event(parent[event->level-1], event->name);
			parent[event->level] = event->value;
			DISP_LOG_D("disp_mmp_init value=%d, name=%s\n", event->value, event->name);
		}
	}

	mmprofile_enable_event_recursive(disp_log.mmp_event[MMP_DISP_ROOT].value, 1);

	mmprofile_start(1);

	return 0;
}
#endif

void disp_log_set_level(uint32_t level)
{
	disp_log.mgr_level = level;
}

void disp_log_set_sub_level(enum DISP_MODULE_ENUM module, uint32_t level)
{
	disp_log.sub_level[module] = level;
}

void disp_log_init(struct device *dev)
{
	disp_log.dev = dev;
	disp_log_set_level(DISP_LOG_LEVEL_DEBUG2);

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
	disp_mmp_init();
#endif
}
