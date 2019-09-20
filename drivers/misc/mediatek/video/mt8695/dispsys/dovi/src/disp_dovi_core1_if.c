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
#include "dovi_type.h"
#include "dovi_log.h"
#include "dovi_common_hal.h"
#include <linux/slab.h>
#include "disp_hw_mgr.h"
#include "dovi_core1_hal.h"
#include "disp_dovi_core1_if.h"

#include "linux/dma-mapping.h"

/******************************************************************************
* Function prototype
******************************************************************************/

/******************************************************************************
* local variable
******************************************************************************/
uint32_t core1_lut_addr_pa;

/******************************************************************************
* local variable, default
******************************************************************************/


/*****************************************************************************
* Internal Function
*****************************************************************************/


/******************************************************************************
* Global Function
******************************************************************************/
uint32_t *core1_lut_addr;
uint32_t core1_init;

uint32_t *core1_lut_addr1;
uint32_t core1_lut_addr_pa1;

int dovi_core1_init(char *reg_base)
{
#if DOVI_IOMMU_SUPPORT
	dma_addr_t mva_address = 0;
#endif

	if (core1_init) {
		dovi_info("core1 already inited\n");
		return 0;
	}

	if (!dovi_dev) {
		dovi_info("dovi_dev is null\n");
		return 0;
	}

#if DOVI_IOMMU_SUPPORT
	core1_lut_addr =
	dma_alloc_coherent(dovi_dev, DOVI_LUT_SIZE, &mva_address, GFP_KERNEL);
	core1_lut_addr_pa = mva_address;

	core1_lut_addr1 = kmalloc(DOVI_LUT_SIZE, GFP_KERNEL);
	core1_lut_addr_pa1 = __pa(core1_lut_addr1);
#else
	core1_lut_addr = kmalloc(DOVI_LUT_SIZE, GFP_KERNEL);
	core1_lut_addr_pa = __pa(core1_lut_addr);
#endif

	/* dovi_core1_lut_status(); */

	dovi_core1_hal_init(reg_base);

	core1_init = 1;
	return 1;
}

int dovi_core1_config_lut(uint32_t *p_core1_lut)
{
	uint32_t idx = 0;
	/* memcpy(core1_lut_addr, p_core1_lut, DOVI_LUT_SIZE); */
	for (idx = 0; idx < DOVI_LUT_SIZE/4; idx++)
		core1_lut_addr[idx] = p_core1_lut[idx];

#if !DOVI_IOMMU_SUPPORT
	/* todo flush cache */
	dma_map_single(dovi_dev, core1_lut_addr, DOVI_LUT_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dovi_dev, core1_lut_addr_pa, DOVI_LUT_SIZE, DMA_TO_DEVICE);
#endif

	dovi_core1_hal_config_lut(core1_lut_addr_pa);

	return 1;
}

int dovi_core1_uninit(void)
{
	return 1;
}

int dovi_core1_lut_status(void)
{
	uint32_t idx = 0;
	dovi_default("core1 lut m4u addr 0x%p, pa 0x%x\n",
		    core1_lut_addr, core1_lut_addr_pa);

	dovi_default("core1 lut addr 0x%p, pa 0x%x\n",
		    core1_lut_addr1, core1_lut_addr_pa1);
	if (core1_lut_addr != NULL) {
		for (idx = 0; idx < DOVI_LUT_SIZE/4; idx += 4)
			dovi_default("core1 lut1 %.8X%.8X%.8X%.8X\n", core1_lut_addr[idx],
			core1_lut_addr[idx+1], core1_lut_addr[idx+2], core1_lut_addr[idx+3]);
	}
	return 1;
}

int dovi_core1_status(void)
{
	dovi_core1_lut_status();
	dovi_core1_hal_status();

	return 1;
}
