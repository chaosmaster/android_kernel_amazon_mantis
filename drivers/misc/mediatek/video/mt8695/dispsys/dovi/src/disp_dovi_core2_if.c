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

/* ----------------------------------------------------------------------------- */
/* Include files */
/* ----------------------------------------------------------------------------- */

#include "dovi_type.h"
#include "dovi_core2_hw.h"
#include "disp_dovi_core2_if.h"
#include "disp_hw_mgr.h"

#include "dovi_core2_hal.h"

#include "dovi_log.h"
#include "dovi_common_hal.h"
#include <linux/slab.h>

#include "linux/dma-mapping.h"

/******************************************************************************
* local macro
******************************************************************************/

uint32_t *core2_lut_addr;
uint32_t core2_lut_addr_pa;

uint32_t *core2_lut_addr1;
uint32_t core2_lut_addr_pa1;

int core2_init;

int dovi_core2_init(char *reg_base)
{
	INT32 i4Ret = 0;
#if DOVI_IOMMU_SUPPORT
	dma_addr_t mva_address = 0;
#endif

	if (core2_init) {
		dovi_info("core2 already inited\n");
		return 0;
	}

	if (!dovi_dev) {
		dovi_info("dovi_dev is null\n");
		return 0;
	}

#if DOVI_IOMMU_SUPPORT
	core2_lut_addr =
	dma_alloc_coherent(dovi_dev, DOVI_LUT_SIZE, &mva_address, GFP_KERNEL);
	core2_lut_addr_pa = mva_address;

	core2_lut_addr1 = kmalloc(DOVI_LUT_SIZE, GFP_KERNEL);
	core2_lut_addr_pa1 = __pa(core2_lut_addr1);
#else
	core2_lut_addr = kmalloc(DOVI_LUT_SIZE, GFP_KERNEL);
	core2_lut_addr_pa = __pa(core2_lut_addr);
#endif

	dovi_core2_hal_init(reg_base);

	core2_init = 1;
	return i4Ret;
}

int dovi_core2_config_lut(uint32_t *p_core2_lut)
{
	memcpy(core2_lut_addr, p_core2_lut, DOVI_LUT_SIZE);

	/* todo flush cache */

	dovi_core2_hal_config_lut(core2_lut_addr_pa);

	return 1;
}

int dovi_core2_uninit(void)
{
	INT32 i4Ret = 0;

	return i4Ret;
}

int dovi_core2_lut_status(void)
{
	uint32_t idx = 0;
	dovi_default("core2 lut m4u addr 0x%p, pa 0x%x\n",
		    core2_lut_addr, core2_lut_addr_pa);

	dovi_default("core2 lut addr 0x%p, pa 0x%x\n",
		    core2_lut_addr1, core2_lut_addr_pa1);
	if (core2_lut_addr != NULL) {
		for (idx = 0; idx < DOVI_LUT_SIZE/4; idx += 4)
			dovi_default("core2 lut1 %.8X%.8X%.8X%.8X\n", core2_lut_addr[idx],
			core2_lut_addr[idx+1], core2_lut_addr[idx+2], core2_lut_addr[idx+3]);
	}
	return 1;
}

int dovi_core2_status(void)
{
	dovi_core2_lut_status();
	dovi_core2_hal_status();

	return 1;
}
