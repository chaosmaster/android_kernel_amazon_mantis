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

#ifndef _IRT_HW_H_
#define _IRT_HW_H_

#define DISP_TOP_INT_CLR                (0x00)
#define INT_CLR_IRT_DMA                     (0x1 << 16)
#define DISP_TOP_INT_STATUS             (0x04)
#define INT_STATUS_IRT_DMA                  (0x1 << 16)
#define DISP_TOP_SYS_CFG_08             (0x08)
#define SYS_CFG_08_IRT_DMA                  (0x1 << 11)
#define DISP_TOP_SYS_CFG_0C             (0x0C)
#define SYS_CFG_0C_SMI_LARB6                (0x1 << 5)
#define SYS_CFG_0C_IRT_DMA                  (0x1 << 11)

/*irt reg*/
#define IRT_DMA_REG_TIRG                (0x00)
#define IRT_DMA_MSK_RG_5351_MODE_EN         (0x1 << 24)
#define IRT_DMA_MSK_RG_5351_MODE_SEL        (0x3 << 20)
#define IRT_DMA_MSK_RG_SCAN_LINE            (0x1 << 16)
#define IRT_DMA_MSK_RG_DONE_SEL             (0x1 << 12)
#define IRT_DMA_MSK_RG_DMA_MODE             (0xF << 4)
#define IRT_DMA_MSK_RG_BLOCK_BURST_EN       (0x1 << 1)
#define IRT_DMA_MSK_RG_START                (0x1 << 0)

#define IRT_DMA_REG_CTRL                (0x04)
#define IRT_DMA_MSK_RG_DRAM_CLK_EN          (0x1 << 31)
#define IRT_DMA_MSK_RG_MON_SEL              (0xF << 4)
#define IRT_DMA_MSK_RG_RESET_B              (0x1 << 0)

#define IRT_DMA_REG_HVSIZE              (0x08)
#define IRT_DMA_MSK_RG_VSIZE                (0x1FFF << 16)
#define IRT_DMA_MSK_RG_HSIZE                (0x1FFF << 0)

#define IRT_DMA_REG_DRAM_Y_RD_STA       (0x0C)
#define IRT_DMA_REG_DRAM_Y_WR_STA       (0x10)
#define IRT_DMA_REG_DRAM_C_RD_STA       (0x14)
#define IRT_DMA_REG_DRAM_C_WR_STA       (0x18)
#define IRT_DMA_REG_MON                 (0x20)
#define IRT_DMA_10BIT_CTL               (0x60)
#define IRT_DMA_MSK_DITHER_MODE_ENABLE      (0x1 << 7)
#define IRT_DMA_MSK_DITHER_MODE_SEL         (0x7 << 4)
#define IRT_DMA_MSK_RG_WR_GARBAGE_CANCEL    (0x1 << 3)
#define IRT_DMA_MSK_RG_10BIT_MODE_SEL       (0x1 << 2)
#define IRT_DMA_MSK_RG_10BIT_ROTATE_EN      (0x1 << 1)
#define IRT_DMA_MSK_RG_SOURCE_10BIT_EN      (0x1 << 0)

/*[IRT_DMA_TIRG] - 0x00*/
#define IRT_DMA_FLD_RG_5351_MODE_EN(x)      (((x) << 24)&(IRT_DMA_MSK_RG_5351_MODE_EN))
#define IRT_DMA_FLD_RG_5351_MODE_SEL(x)     (((x) << 20)&(IRT_DMA_MSK_RG_5351_MODE_SEL))
#define IRT_DMA_FLD_RG_SCAN_LINE(x)         (((x) << 16)&(IRT_DMA_MSK_RG_SCAN_LINE))
#define IRT_DMA_FLD_RG_DONE_SEL(x)          (((x) << 12)&(IRT_DMA_MSK_RG_DONE_SEL))
#define IRT_DMA_FLD_RG_DMA_MODE(x)          (((x) << 4)&(IRT_DMA_MSK_RG_DMA_MODE))
#define IRT_DMA_FLD_RG_BLOCK_BURST_EN(x)    (((x) << 1)&(IRT_DMA_MSK_RG_BLOCK_BURST_EN))
#define IRT_DMA_FLD_RG_START(x)             (((x) << 0)&(IRT_DMA_MSK_RG_START))

/*[IRT_DMA_CTRL] - 0x04*/
#define IRT_DMA_FLD_RG_DRAM_CLK_EN(x)       (((x) << 31)&(IRT_DMA_MSK_RG_DRAM_CLK_EN))
#define IRT_DMA_FLD_RG_MON_SEL(x)           (((x) << 4)&(IRT_DMA_MSK_RG_MON_SEL))
#define IRT_DMA_FLD_RG_RESET_B(x)           (((x) << 0)&(IRT_DMA_MSK_RG_RESET_B))

/*[IRT_DMA_HVSIZE] - 0x08*/
#define IRT_DMA_FLD_RG_VSIZE(x)             (((x) << 16)&(IRT_DMA_MSK_RG_VSIZE))
#define IRT_DMA_FLD_RG_HSIZE(x)             (((x) << 0)&(IRT_DMA_MSK_RG_HSIZE))

/*[IRT_DMA_10BIT_CTL] - 0x60*/
#define IRT_DMA_FLD_DITHER_MODE_ENABLE(x)   (((x) << 7)&(IRT_DMA_MSK_DITHER_MODE_ENABLE))
#define IRT_DMA_FLD_DITHER_MODE_SEL(x)      (((x) << 4)&(IRT_DMA_MSK_DITHER_MODE_SEL))
#define IRT_DMA_FLD_RG_WR_GARBAGE_CANCEL(x) (((x) << 3)&(IRT_DMA_MSK_RG_WR_GARBAGE_CANCEL))
#define IRT_DMA_FLD_RG_10BIT_MODE_SEL(x)    (((x) << 2)&(IRT_DMA_MSK_RG_10BIT_MODE_SEL))
#define IRT_DMA_FLD_RG_10BIT_ROTATE_EN(x)   (((x) << 1)&(IRT_DMA_MSK_RG_10BIT_ROTATE_EN))
#define IRT_DMA_FLD_RG_SOURCE_10BIT_EN(x)   (((x) << 0)&(IRT_DMA_MSK_RG_SOURCE_10BIT_EN))

#define IRT_DMA_READ32(addr)                   (*(volatile unsigned int *)(addr))
#define IRT_DMA_WRITE32(addr, value)           (*(volatile unsigned int *)(addr) = (value))
#define IRT_DMA_MSK_WRITE32(addr, value, mask) \
IRT_DMA_WRITE32((addr), (IRT_DMA_READ32(addr) & (~(mask))) | ((value) & (mask)))

#endif /*_IRT_HW_H_*/
