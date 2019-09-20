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

#include <linux/debugfs.h>
#include "irt_hal.h"
#include "irt_hw.h"

void irt_dma_hal_clear_irq(void __iomem *reg_base)
{
	IRT_DMA_MSK_WRITE32(reg_base + DISP_TOP_INT_CLR, 1 << 16, INT_CLR_IRT_DMA);
	IRT_DMA_MSK_WRITE32(reg_base + DISP_TOP_INT_CLR, 0 << 16, INT_CLR_IRT_DMA);
}

void irt_dma_hal_hw_reset(void __iomem *reg_base)
{
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_CTRL, IRT_DMA_FLD_RG_DRAM_CLK_EN(1),
			    IRT_DMA_MSK_RG_DRAM_CLK_EN);

	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_CTRL, IRT_DMA_FLD_RG_RESET_B(0),
			    IRT_DMA_MSK_RG_RESET_B);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_CTRL, IRT_DMA_FLD_RG_RESET_B(1),
			    IRT_DMA_MSK_RG_RESET_B);

	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_CTRL, IRT_DMA_FLD_RG_DRAM_CLK_EN(0),
			    IRT_DMA_MSK_RG_DRAM_CLK_EN);
}

int irt_dma_hal_config(struct irt_dma_config_param *config, void __iomem *reg_base)
{
	/*source mode set */
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG, 1 << 28, 1 << 28);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG,
			    IRT_DMA_FLD_RG_5351_MODE_EN(config->rg_5351_mode_en),
			    IRT_DMA_MSK_RG_5351_MODE_EN);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG,
			    IRT_DMA_FLD_RG_5351_MODE_SEL(config->rg_5351_mode_sel),
			    IRT_DMA_MSK_RG_5351_MODE_SEL);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG,
			    IRT_DMA_FLD_RG_SCAN_LINE(config->rg_scan_line),
			    IRT_DMA_MSK_RG_SCAN_LINE);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG,
			    IRT_DMA_FLD_RG_BLOCK_BURST_EN(config->rg_blk_burst_en),
			    IRT_DMA_MSK_RG_BLOCK_BURST_EN);

	/*rotate */
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG,
			    IRT_DMA_FLD_RG_DMA_MODE(config->rg_rotate_mode),
			    IRT_DMA_MSK_RG_DMA_MODE);

	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_CTRL, IRT_DMA_FLD_RG_MON_SEL(0x8),
			    IRT_DMA_MSK_RG_MON_SEL);

	/*size */
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_HVSIZE,
			    IRT_DMA_FLD_RG_VSIZE(config->rg_align_vsize), IRT_DMA_MSK_RG_VSIZE);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_HVSIZE,
			    IRT_DMA_FLD_RG_HSIZE(config->rg_align_hsize), IRT_DMA_MSK_RG_HSIZE);

	/*source addr */
	IRT_DMA_WRITE32(reg_base + IRT_DMA_REG_DRAM_Y_RD_STA, config->rg_y_dram_rd_addr >> 4);
	IRT_DMA_WRITE32(reg_base + IRT_DMA_REG_DRAM_C_RD_STA, config->rg_c_dram_rd_addr >> 4);

	/*dst addr */
	IRT_DMA_WRITE32(reg_base + IRT_DMA_REG_DRAM_Y_WR_STA, config->rg_y_dram_wr_addr >> 4);
	IRT_DMA_WRITE32(reg_base + IRT_DMA_REG_DRAM_C_WR_STA, config->rg_c_dram_wr_addr >> 4);

	/*10bit control */
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_RG_SOURCE_10BIT_EN(config->rg_src_10bit_en),
			    IRT_DMA_MSK_RG_SOURCE_10BIT_EN);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_RG_10BIT_ROTATE_EN(config->rg_10bit_rotate_en),
			    IRT_DMA_MSK_RG_10BIT_ROTATE_EN);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_RG_10BIT_MODE_SEL(config->rg_10bit_mode_sel),
			    IRT_DMA_MSK_RG_10BIT_MODE_SEL);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_RG_WR_GARBAGE_CANCEL(config->rg_wr_garbage_cancel),
			    IRT_DMA_MSK_RG_WR_GARBAGE_CANCEL);

	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_DITHER_MODE_ENABLE(config->rg_dither_en),
			    IRT_DMA_MSK_DITHER_MODE_ENABLE);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_10BIT_CTL,
			    IRT_DMA_FLD_DITHER_MODE_SEL(config->rg_dither_mode),
			    IRT_DMA_MSK_DITHER_MODE_SEL);
	/*start */
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG, IRT_DMA_FLD_RG_START(1),
			    IRT_DMA_MSK_RG_START);
	IRT_DMA_MSK_WRITE32(reg_base + IRT_DMA_REG_TIRG, IRT_DMA_FLD_RG_START(0),
			    IRT_DMA_MSK_RG_START);

	return 0;
}
