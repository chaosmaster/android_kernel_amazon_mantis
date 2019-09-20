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

#ifndef _IRT_HAL_H_
#define _IRT_HAL_H_

struct irt_dma_config_param {
	unsigned int rg_5351_mode_en;
	unsigned int rg_5351_mode_sel;
	unsigned int rg_scan_line;
	unsigned int rg_rotate_mode;
	unsigned int rg_blk_burst_en;
	unsigned int rg_align_vsize;
	unsigned int rg_align_hsize;
	unsigned int rg_y_dram_rd_addr;
	unsigned int rg_c_dram_rd_addr;
	unsigned int rg_y_dram_wr_addr;
	unsigned int rg_c_dram_wr_addr;

	unsigned int rg_src_10bit_en;
	unsigned int rg_10bit_rotate_en;
	unsigned int rg_10bit_mode_sel;
	unsigned int rg_wr_garbage_cancel;
	unsigned int rg_dither_mode;
	unsigned int rg_dither_en;

	unsigned int src_offset_y_len;
	unsigned int src_offset_c_len;
	unsigned int dst_offset_y_len;
	unsigned int dst_offset_c_len;

	/*ion fd & secure handle*/
	int src_fd;
	int dst_fd;
};

void irt_dma_hal_clear_irq(void __iomem *reg_base);
void irt_dma_hal_hw_reset(void __iomem *reg_base);
int irt_dma_hal_config(struct irt_dma_config_param *config, void __iomem *reg_base);
#endif	/*_IRT_HAL_H_*/
