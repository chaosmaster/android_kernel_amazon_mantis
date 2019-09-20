/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MTK_DI_REG_H
#define MTK_DI_REG_H


#include <linux/string.h>
#include "mtk_vq_info.h"
#include "di_util.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>

#define DI_VDO_SUPPORT_METRIC        0
#define DI_VDO_SUPPORT_FUSION        0
#define DI_VDO_SUPPORT_MDDI_PE       0

#ifndef UINT32
#define UINT32 unsigned int
#endif

/* reg */

#define BITS(high, low)                       ((1 << (high - low + 1)) - 1)
#define REG_R(dAddr)                          (*(volatile UINT32 *)(uintptr_t)(dAddr))
#define REG_W(reg, val)                       ((*(volatile UINT32 *)(uintptr_t)(reg)) = (val))
#define REG_R_BITS(reg, bits, shift)          (((REG_R(reg))>>(shift))&(bits))
#define REG_W_BITS(reg, bits, shift, val) \
(REG_W((reg), (((REG_R((reg)))&(~((UINT32)(bits)<<(shift)))) | (((UINT32)(val)&(UINT32)(bits))<<(shift)))))
#define REG_W_MASK(reg, val, mask) \
(REG_W(reg, (((REG_R(reg)) & (~((unsigned int)(mask)))) | ((unsigned int)(val)))))
/* dispsys_cfg */
#define DISPSYSCFG_DISP_TOP_DI_REG               (di_init.disp_top_di_reg_base)
#define DISPSYS_DISP_TOP_DI_PD_CTRL              (DISPSYSCFG_DISP_TOP_DI_REG + 0x058)
#define DISPSYSCFG_W_DISP_DI_PD_CTRL(val) \
REG_W(DISPSYS_DISP_TOP_DI_PD_CTRL, val)
#define DISPSYS_DISP_DI_PD_CTRL1    (DISPSYSCFG_DISP_TOP_DI_REG + 0x800)
#define DISPSYSCFG_W_DISP_DI_PD_CTRL1(val) \
REG_W(DISPSYS_DISP_DI_PD_CTRL1, val)

#define DISPSYS_DISP_READ_ULTRA(val) \
REG_W_BITS(DISPSYSCFG_DISP_TOP_DI_REG+0x380, BITS(19, 19), 19, val)

#define DISPSYS_DISP_WRITE_ULTRA(val) \
REG_W_BITS(DISPSYSCFG_DISP_TOP_DI_REG+0x380, BITS(3, 3), 3, val)

#define DISPSYS_DISP_MAX_OUTSTANDING(val) \
REG_W_BITS(DISPSYSCFG_DISP_TOP_DI_REG+0x37C, BITS(15, 8), 8, val)




#define DISPSYSCFG_DISP_PCLK_REG               (di_init.disp_top_pclk_reg_base)
#define DISPSYS_DISP_TOP_PCLK_CTRL             (DISPSYSCFG_DISP_PCLK_REG + 0x078)
#define DISPSYSCFG_W_DISP_TOP_PCLK_CTRL(val) \
REG_W(DISPSYS_DISP_TOP_PCLK_CTRL, val)
#define DISPSYSCFG_REG               (di_init.disp_top_reg_base)
#define DISPSYS_INT_CLR             (DISPSYSCFG_REG + 0x000)

#define DISPSYSCFG_R_INT_CLR_STATUS \
REG_R(DISPSYS_INT_CLR)

#define DISPSYSCFG_W_INT_CLR(val) \
REG_W_BITS(DISPSYS_INT_CLR, BITS(20, 17), 17, val)
#define DISPSYS_INT_STATUS          (DISPSYSCFG_REG + 0x004)

#define DISPSYSCFG_R_INT_STATUS \
REG_R(DISPSYS_INT_STATUS)

#define DISPSYS_DISP_TOP_RESET_EN    (DISPSYSCFG_REG + 0x008)

#define DISPSYSCFG_W_DISP_TOP_RESET_EN(val) \
REG_W(DISPSYS_DISP_TOP_RESET_EN, val)
#define DISPSYS_DISP_TOP_CLK_CTRL    (DISPSYSCFG_REG + 0x00C)
#define DISPSYSCFG_W_DISP_TOP_CLK_CTRL(val) \
REG_W(DISPSYS_DISP_TOP_CLK_CTRL, val)

#define DISPSYSCFG_W_DISP_TOP_DI_FULL_CLK_EN(val) \
REG_W_BITS(DISPSYS_DISP_TOP_CLK_CTRL, BITS(27, 27), 27, val)


#define DISPSYS_DRAM_BRIDGE_CONFIG1 (DISPSYSCFG_REG + 0x010)
#define DISPSYSCFG_W_awmmu(val) \
REG_W_BITS(DISPSYS_DRAM_BRIDGE_CONFIG1, BITS(7, 7), 7, val)
#define DISPSYSCFG_W_armmu(val) \
REG_W_BITS(DISPSYS_DRAM_BRIDGE_CONFIG1, BITS(23, 23), 23, val)
#define DISPSYS_DISP_COMM_CONFIG    (DISPSYSCFG_REG + 0x01C)
#define DISPSYSCFG_W_DISP_DISP_SRC_SEL(val) \
REG_W_BITS(DISPSYS_DISP_COMM_CONFIG, BITS(2, 2), 2, val)
#define DISPSYS_CG_CON0             (DISPSYSCFG_REG + 0x100)
#define DISPSYSCFG_W_DISPSYS_CG_CON0(val) \
REG_W(DISPSYS_CG_CON0, val)
#define DISPSYS_CG_CLR0             (DISPSYSCFG_REG + 0x108)
#define DISPSYSCFG_W_DISPSYS_CG_CLR0(val) \
REG_W(DISPSYS_CG_CLR0, val)
#define DISPSYS_CG_CLR1             (DISPSYSCFG_REG + 0x118)
#define DISPSYSCFG_W_DISPSYS_CG_CLR1(val) \
REG_W(DISPSYS_CG_CLR1, val)
#define DISPSYS_HW_DCM_DIS0         (DISPSYSCFG_REG + 0x120)
#define DISPSYSCFG_W_DISPSYS_HW_DCM_DIS0(val) \
REG_W(DISPSYS_HW_DCM_DIS0, val)
#define DISPSYS_SW_RST_B            (DISPSYSCFG_REG + 0x138)
/* dispfmt */
#define DISPFMT_REG                  (di_init.dispfmt_reg_base)
#define FMT_VALID_READY                 (DISPFMT_REG + 0x000)
#define DISPFMT_W_HOR_VALID_END(val) \
REG_W_BITS(FMT_VALID_READY, BITS(12, 0), 0, val)
#define DISPFMT_W_HOR_VALID_STAR(val) \
REG_W_BITS(FMT_VALID_READY, BITS(28, 16), 16, val)
#define FMT_NR_VSYNC_CTRL_IN            (DISPFMT_REG + 0x004)
#define DISPFMT_W_NR_IN_VSYNC_END(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(10, 0), 0, val)
#define DISPFMT_W_NR_IN_VSYNC_START(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(22, 12), 12, val)
#define DISPFMT_W_NR_IN_VSYNC_POLAR(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(24, 24), 24, val)
#define DISPFMT_W_NR_IN_HSYNC_POLAR(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(25, 25), 25, val)
#define DISPFMT_W_NR_IN_FIELD_POLAR(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(26, 26), 26, val)
#define DISPFMT_W_NR_IN_DE_SELF(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(27, 27), 27, val)
#define DISPFMT_W_NR_IN_ODD_V_START_OPT(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(28, 28), 28, val)
#define DISPFMT_W_NR_IN_ODD_V_END_OPT(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(29, 29), 29, val)
#define DISPFMT_W_NR_IN_EVEN_V_START_OPT(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(30, 30), 30, val)
#define DISPFMT_W_NR_IN_EVEN_V_END_OPT(val) \
REG_W_BITS(FMT_NR_VSYNC_CTRL_IN, BITS(31, 31), 31, val)
#define FMT_NR_VALID_READY              (DISPFMT_REG + 0x008)
#define DISPFMT_W_HOR_NR_VALID_END(val) \
REG_W_BITS(FMT_NR_VALID_READY, BITS(12, 0), 0, val)
#define DISPFMT_W_HOR_NR_VALID_STAR(val) \
REG_W_BITS(FMT_NR_VALID_READY, BITS(28, 16), 16, val)
#define FMT_NR_IN_HOR                   (DISPFMT_REG + 0x00C)
#define DISPFMT_W_NR_IN_HOR_END(val) \
REG_W_BITS(FMT_NR_IN_HOR, BITS(12, 0), 0, val)
#define DISPFMT_W_NR_IN_HOR_STAT(val) \
REG_W_BITS(FMT_NR_IN_HOR, BITS(28, 16), 16, val)
#define FMT_DI_PATH_CONFIG              (DISPFMT_REG + 0x010)
#define DISPFMT_W_BYPASS_NR(val) \
REG_W_BITS(FMT_DI_PATH_CONFIG, BITS(0, 0), 0, val)
#define COLOR_BAR_CTRL                  (DISPFMT_REG + 0x014)
#define DISPFMT_W_COLOR_BAR_ON(val) \
REG_W_BITS(COLOR_BAR_CTRL, BITS(0, 0), 0, val)
#define DISPFMT_W_COLOR_BAR_TYPE(val) \
REG_W_BITS(COLOR_BAR_CTRL, BITS(1, 1), 1, val)
#define DISPFMT_W_COLOR_BAR_WIDHT(val) \
REG_W_BITS(COLOR_BAR_CTRL, BITS(23, 16), 16, val)
#define DISPFMT_W_ENABLE_422_444(val) \
REG_W_BITS(COLOR_BAR_CTRL, BITS(31, 31), 31, val)
#define DI_AGENT_CTRL                   (DISPFMT_REG + 0x018)
#define DISPFMT_W_DI_AGENT_TRIG(val) \
REG_W_BITS(DI_AGENT_CTRL, BITS(0, 0), 0, val)
#define WAIT_VDO_WDMA_START             (DISPFMT_REG + 0x01C)
#define DISPFMT_W_FMT_H_WAIT_VDO_WDMA_START(val) \
REG_W_BITS(WAIT_VDO_WDMA_START, BITS(12, 0), 0, val)
#define DISPFMT_W_NR_H_WAIT_VDO_WDMA_START(val) \
REG_W_BITS(WAIT_VDO_WDMA_START, BITS(28, 16), 16, val)
#define CRC                             (DISPFMT_REG + 0x050)
#define DISPFMT_W_CRC_INIT(val) \
REG_W_BITS(CRC, BITS(0, 0), 0, val)
#define DISPFMT_W_CRC_CLEAR(val) \
REG_W_BITS(CRC, BITS(1, 1), 1, val)
#define DISPFMT_W_CRC_RESULT(val) \
REG_W_BITS(CRC, BITS(31, 8), 8, val)
#define DERING                          (DISPFMT_REG + 0x054)
#define DISPFMT_W_DERING_THRESHOLD_Y(val) \
REG_W_BITS(DERING, BITS(11, 0), 0, val)
#define DISPFMT_W_DERING_THRESHOLD_C(val) \
REG_W_BITS(DERING, BITS(23, 12), 12, val)
#define DISPFMT_W_DERING_TRANS(val) \
REG_W_BITS(DERING, BITS(27, 24), 24, val)
#define DISPFMT_W_DERING_EN_Y(val) \
REG_W_BITS(DERING, BITS(28, 28), 28, val)
#define DISPFMT_W_DERING_EN_C(val) \
REG_W_BITS(DERING, BITS(29, 29), 29, val)
#define BORDER_CONTROL                  (DISPFMT_REG + 0x058)
#define DISPFMT_W_BORDER_X_WIDTH(val) \
REG_W_BITS(BORDER_CONTROL, BITS(21, 16), 16, val)
#define DISPFMT_W_BORDER_Y_WIDTH(val) \
REG_W_BITS(BORDER_CONTROL, BITS(29, 24), 24, val)
#define DISPFMT_W_BD_ON(val) \
REG_W_BITS(BORDER_CONTROL, BITS(31, 31), 31, val)
#define Border_color                    (DISPFMT_REG + 0x05C)
#define DISPFMT_W_BDY(val) \
REG_W_BITS(Border_color, BITS(7, 2), 2, val)
#define DISPFMT_W_BDCB(val) \
REG_W_BITS(Border_color, BITS(15, 10), 10, val)
#define DISPFMT_W_BDCR(val) \
REG_W_BITS(Border_color, BITS(23, 18), 18, val)
#define DOWN_CONTROL                    (DISPFMT_REG + 0x06C)
#define Down_scaler_range_0             (DISPFMT_REG + 0x070)
#define Down_scaler_range_1             (DISPFMT_REG + 0x074)
#define Down_scaler_range_2             (DISPFMT_REG + 0x078)
#define Down_scaler_output_range        (DISPFMT_REG + 0x07C)
#define NR_CONTROL                      (DISPFMT_REG + 0x080)
#define DISPFMT_W_ADJ_SYNC_EN(val) \
REG_W_BITS(NR_CONTROL, BITS(0, 0), 0, val)
#define DISPFMT_W_NR_ADJ_FORWARD(val) \
REG_W_BITS(NR_CONTROL, BITS(1, 1), 1, val)
#define DISPFMT_W_TOGGLE_OPTION(val) \
REG_W_BITS(NR_CONTROL, BITS(2, 2), 2, val)
#define DISPFMT_W_HSYNC_DELAY(val) \
REG_W_BITS(NR_CONTROL, BITS(19, 8), 8, val)
#define DISPFMT_W_VSYNC_DELAY(val) \
REG_W_BITS(NR_CONTROL, BITS(31, 20), 20, val)
#define NR_VSYNC_CONTROL                (DISPFMT_REG + 0x084)
#define DISPFMT_W_VSYNC_END(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(10, 0), 0, val)
#define DISPFMT_W_VSYNC_START(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(22, 12), 12, val)
#define DISPFMT_W_VSYNC_POLAR(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(24, 24), 24, val)
#define DISPFMT_W_HSYNC_POLAR(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(25, 25), 25, val)
#define DISPFMT_W_FIELD_POLAR(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(26, 26), 26, val)
#define DISPFMT_W_DE_SELF(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(27, 27), 27, val)
#define DISPFMT_W_ODD_V_START_OPT(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(28, 28), 28, val)
#define DISPFMT_W_ODD_V_END_OPT(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(29, 29), 29, val)
#define DISPFMT_W_EVEN_V_START_OPT(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(30, 30), 30, val)
#define DISPFMT_W_EVEN_V_END_OPT(val) \
REG_W_BITS(NR_VSYNC_CONTROL, BITS(31, 31), 31, val)
#define NR_HORIZONTAL_CONTROL           (DISPFMT_REG + 0x088)
#define DISPFMT_W_HOR_END(val) \
REG_W_BITS(NR_HORIZONTAL_CONTROL, BITS(11, 0), 0, val)
#define DISPFMT_W_HOR_START(val) \
REG_W_BITS(NR_HORIZONTAL_CONTROL, BITS(27, 16), 16, val)
#define NR_ODD_VERTICAL_CONTRL          (DISPFMT_REG + 0x08C)
#define DISPFMT_W_VO_END(val) \
REG_W_BITS(NR_ODD_VERTICAL_CONTRL, BITS(10, 0), 0, val)
#define DISPFMT_W_VO_START(val) \
REG_W_BITS(NR_ODD_VERTICAL_CONTRL, BITS(26, 16), 16, val)
#define NR_EVEN_VERTICAL_CONTRL         (DISPFMT_REG + 0x090)
#define DISPFMT_W_VE_END(val) \
REG_W_BITS(NR_EVEN_VERTICAL_CONTRL, BITS(10, 0), 0, val)
#define DISPFMT_W_VE_START(val) \
REG_W_BITS(NR_EVEN_VERTICAL_CONTRL, BITS(26, 16), 16, val)
#define Mode_Control                    (DISPFMT_REG + 0x094)
#define DISPFMT_W_HSYNWIDTH(val) \
REG_W_BITS(Mode_Control, BITS(7, 0), 0, val)
#define DISPFMT_W_VSYNWIDTH(val) \
REG_W_BITS(Mode_Control, BITS(12, 8), 8, val)
#define DISPFMT_W_HD_TP(val) \
REG_W_BITS(Mode_Control, BITS(13, 13), 13, val)
#define DISPFMT_W_HD_ON(val) \
REG_W_BITS(Mode_Control, BITS(14, 14), 14, val)
#define DISPFMT_W_PRGS(val) \
REG_W_BITS(Mode_Control, BITS(15, 15), 15, val)
#define DISPFMT_W_PRGS_AUTOFLD(val) \
REG_W_BITS(Mode_Control, BITS(17, 17), 17, val)
#define DISPFMT_W_PRGS_INVFLD(val) \
REG_W_BITS(Mode_Control, BITS(18, 18), 18, val)
#define DISPFMT_W_YUV_RST_OPT(val) \
REG_W_BITS(Mode_Control, BITS(20, 20), 20, val)
#define DISPFMT_W_PRGS_FLD(val) \
REG_W_BITS(Mode_Control, BITS(24, 24), 24, val)
#define DISPFMT_W_NEW_SD_144MHz(val) \
REG_W_BITS(Mode_Control, BITS(27, 27), 27, val)
#define DISPFMT_W_NEW_SD_MODE(val) \
REG_W_BITS(Mode_Control, BITS(28, 28), 28, val)
#define DISPFMT_W_NEW_SD_USE_EVEN(val) \
REG_W_BITS(Mode_Control, BITS(29, 29), 29, val)
#define DISPFMT_W_TVMODE(val) \
REG_W_BITS(Mode_Control, BITS(31, 30), 30, val)
#define VSYN_Offset                     (DISPFMT_REG + 0x098)
#define DISPFMT_W_PF_ADV(val) \
REG_W_BITS(VSYN_Offset, BITS(19, 16), 16, val)
#define Pixel_Length                    (DISPFMT_REG + 0x09C)
#define DISPFMT_W_PXLLEN(val) \
REG_W_BITS(Pixel_Length, BITS(12, 0), 0, val)
#define DISPFMT_W_RIGHT_SKIP(val) \
REG_W_BITS(Pixel_Length, BITS(19, 16), 16, val)
#define Horizontal_Active_Zone          (DISPFMT_REG + 0x0A0)
#define DISPFMT_W_HACTEND(val) \
REG_W_BITS(Horizontal_Active_Zone, BITS(12, 0), 0, val)
#define DISPFMT_W_HACTBGN(val) \
REG_W_BITS(Horizontal_Active_Zone, BITS(28, 16), 16, val)
#define Vertical_Odd_Active_Zone        (DISPFMT_REG + 0x0A4)
#define DISPFMT_W_VOACTEND(val) \
REG_W_BITS(Vertical_Odd_Active_Zone, BITS(11, 0), 0, val)
#define DISPFMT_W_VOACTBGN(val) \
REG_W_BITS(Vertical_Odd_Active_Zone, BITS(27, 16), 16, val)
#define DISPFMT_W_HIDE_OST(val) \
REG_W_BITS(Vertical_Odd_Active_Zone, BITS(31, 28), 28, val)
#define Vertical_Even_Active_Zone       (DISPFMT_REG + 0x0A8)
#define DISPFMT_W_VEACTEND(val) \
REG_W_BITS(Vertical_Even_Active_Zone, BITS(11, 0), 0, val)
#define DISPFMT_W_VEACTBGN(val) \
REG_W_BITS(Vertical_Even_Active_Zone, BITS(27, 16), 16, val)
#define DISPFMT_W_HIDE_EST(val) \
REG_W_BITS(Vertical_Even_Active_Zone, BITS(31, 28), 28, val)
#define Video_Formatter_Control         (DISPFMT_REG + 0x0AC)
#define DISPFMT_W_VDO1_EN(val) \
REG_W_BITS(Video_Formatter_Control, BITS(0, 0), 0, val)
#define DISPFMT_W_FMTM(val) \
REG_W_BITS(Video_Formatter_Control, BITS(1, 1), 1, val)
#define DISPFMT_W_HPOR(val) \
REG_W_BITS(Video_Formatter_Control, BITS(3, 3), 3, val)
#define DISPFMT_W_VPOR(val) \
REG_W_BITS(Video_Formatter_Control, BITS(4, 4), 4, val)
#define DISPFMT_W_C_RST_SEL(val) \
REG_W_BITS(Video_Formatter_Control, BITS(7, 7), 7, val)
#define DISPFMT_W_PXLSEL(val) \
REG_W_BITS(Video_Formatter_Control, BITS(9, 8), 8, val)
#define DISPFMT_W_FTRST(val) \
REG_W_BITS(Video_Formatter_Control, BITS(10, 10), 10, val)
#define DISPFMT_W_SHVSYN(val) \
REG_W_BITS(Video_Formatter_Control, BITS(11, 11), 11, val)
#define DISPFMT_W_SYN_DEL(val) \
REG_W_BITS(Video_Formatter_Control, BITS(15, 14), 14, val)
#define DISPFMT_W_UVSW(val) \
REG_W_BITS(Video_Formatter_Control, BITS(19, 19), 19, val)
#define DISPFMT_W_BLACK(val) \
REG_W_BITS(Video_Formatter_Control, BITS(25, 25), 25, val)
#define DISPFMT_W_PFOFF(val) \
REG_W_BITS(Video_Formatter_Control, BITS(27, 27), 27, val)
#define DISPFMT_W_HW_OPT(val) \
REG_W_BITS(Video_Formatter_Control, BITS(31, 28), 28, val)
#define Horizontal_Scaling              (DISPFMT_REG + 0x0B0)
#define DISPFMT_W_Horizontal_Scaling(val) \
REG_W(Horizontal_Scaling, val)
#define Build_In_Color                  (DISPFMT_REG + 0x0B4)
#define DISPFMT_W_BIY(val) \
REG_W_BITS(Build_In_Color, BITS(7, 4), 4, val)
#define DISPFMT_W_BICB(val) \
REG_W_BITS(Build_In_Color, BITS(15, 12), 12, val)
#define DISPFMT_W_BICR(val) \
REG_W_BITS(Build_In_Color, BITS(23, 20), 20, val)
#define DISPFMT_W_PF2OFF(val) \
REG_W_BITS(Build_In_Color, BITS(24, 24), 24, val)
#define DISPFMT_W_HIDE_L(val) \
REG_W_BITS(Build_In_Color, BITS(25, 25), 25, val)
#define Background_Color                (DISPFMT_REG + 0x0B8)
#define DISPFMT_W_BGY(val) \
REG_W_BITS(Background_Color, BITS(7, 4), 4, val)
#define DISPFMT_W_BGCB(val) \
REG_W_BITS(Background_Color, BITS(15, 12), 12, val)
#define DISPFMT_W_BGCR(val) \
REG_W_BITS(Background_Color, BITS(23, 20), 20, val)
#define Y_val_limitation              (DISPFMT_REG + 0x0C4)
#define DISPFMT_W_OLD_C_ACC(val) \
REG_W_BITS(Y_val_limitation, BITS(16, 16), 16, val)
#define DISPFMT_W_Out_Sel2(val) \
REG_W_BITS(Y_val_limitation, BITS(18, 18), 18, val)
#define DISPFMT_W_TVE_ND(val) \
REG_W_BITS(Y_val_limitation, BITS(20, 20), 20, val)
#define DISPFMT_W_FIRST_PXL_LEAD(val) \
REG_W_BITS(Y_val_limitation, BITS(31, 24), 24, val)
#define NEW_SCL_MODE_CTRL               (DISPFMT_REG + 0x0C8)
#define DISPFMT_W_NEW_SCL_MODE_CTRL(val) \
REG_W(NEW_SCL_MODE_CTRL, val)
#define Dispfmt_Configure               (DISPFMT_REG + 0x0CC)
#define DISPFMT_W_Dispfmt_Configure(val) \
REG_W(Dispfmt_Configure, val)
#define DISPFMT_0D0                     (DISPFMT_REG + 0x0D0)
#define DISPFMT_W_0D0_11_0(val) \
REG_W_BITS(DISPFMT_0D0, BITS(11, 0), 0, val)
#define DISPFMT_W_0D0_28_16(val) \
REG_W_BITS(DISPFMT_0D0, BITS(28, 16), 16, val)
#define DISPFMT_W_0D0_31_31(val) \
REG_W_BITS(DISPFMT_0D0, BITS(31, 31), 31, val)
#define Horizontal_Vertical_Total_pixel (DISPFMT_REG + 0x0D4)
#define DISPFMT_W_V_TOTAL(val) \
REG_W_BITS(Horizontal_Vertical_Total_pixel, BITS(11, 0), 0, val)
#define DISPFMT_W_H_TOTAL(val) \
REG_W_BITS(Horizontal_Vertical_Total_pixel, BITS(28, 16), 16, val)
#define DISPFMT_W_ADJ_T(val) \
REG_W_BITS(Horizontal_Vertical_Total_pixel, BITS(31, 31), 31, val)
#define MULTI_INC                       (DISPFMT_REG + 0x0D8)
#define MULTI_DEC                       (DISPFMT_REG + 0x0DC)
#define NEW_SCL_MODE_CTRL2              (DISPFMT_REG + 0x0E0)
#define Multi_Ratio                     (DISPFMT_REG + 0x0E4)
#define DISPFMT_W_Multi_Ratio(val) \
REG_W(Multi_Ratio, val)
#define MVDO_adjustment                 (DISPFMT_REG + 0x0E8)
#define DISPFMT_W_HSYN_DELAY(val) \
REG_W_BITS(MVDO_adjustment, BITS(11, 0), 0, val)
#define DISPFMT_W_VSYN_DELAY(val) \
REG_W_BITS(MVDO_adjustment, BITS(27, 16), 16, val)
#define DISPFMT_W_ADJ_FORWARD(val) \
REG_W_BITS(MVDO_adjustment, BITS(31, 31), 31, val)
#define MIXER_CTRL                      (DISPFMT_REG + 0x0F8)
#define DISPFMT_W_C110(val) \
REG_W_BITS(MIXER_CTRL, BITS(4, 4), 4, val)
#define DISPFMT_W_OLD_CHROMA(val) \
REG_W_BITS(MIXER_CTRL, BITS(5, 5), 5, val)
#define DISPFMT_W_EN_235_255(val) \
REG_W_BITS(MIXER_CTRL, BITS(6, 6), 6, val)
#define DISPFMT_W_FROM_235_TO_255(val) \
REG_W_BITS(MIXER_CTRL, BITS(7, 7), 7, val)
#define MIXER_CTRL2                     (DISPFMT_REG + 0x0FC)
#define DISPFMT_W_LUMA_KEY(val) \
REG_W_BITS(MIXER_CTRL2, BITS(11, 0), 0, val)
#define DISPFMT_W_WINDOW_LINEAR_SIZE_SEL(val) \
REG_W_BITS(MIXER_CTRL2, BITS(17, 16), 16, val)
#define DISPFMT_W_WINDOW_ACC_SIZE_SEL(val) \
REG_W_BITS(MIXER_CTRL2, BITS(19, 18), 18, val)
#define DISPFMT_W_PRE_DATA_NEXT_LUMA_Y_OPTION(val) \
REG_W_BITS(MIXER_CTRL2, BITS(20, 20), 20, val)
#define DISPFMT_W_NEXT_DATA_PRE_LUMA_Y_OPTION(val) \
REG_W_BITS(MIXER_CTRL2, BITS(21, 21), 21, val)
#define DISPFMT_W_PRE_DATA_NEXT_LUMA_C_OPTION(val) \
REG_W_BITS(MIXER_CTRL2, BITS(22, 22), 22, val)
#define DISPFMT_W_NEXT_DATA_PRE_LUMA_C_OPTION(val) \
REG_W_BITS(MIXER_CTRL2, BITS(23, 23), 23, val)
#define DISPFMT_W_post_div2_sel(val) \
REG_W_BITS(MIXER_CTRL2, BITS(31, 31), 31, val)
/* vdo */
#define DI_VDO_BASE         (di_init.vdo_reg_base)
#define VDO_SCL_CTRL                                (DI_VDO_BASE + 0x14c)
#define DI_VDO_W_VDO_SCL_CTRL(val) \
REG_W(VDO_SCL_CTRL, val)
#define VDO_8TAP_VALID                              (DI_VDO_BASE + 0x194)
#define DI_VDO_W_VDO_8TAP_VALID(val) \
REG_W(VDO_8TAP_VALID, val)
#define VDO_8TAP_CTRL_04                            (DI_VDO_BASE + 0x1a0)
#define DI_VDO_W_VDO_8TAP_CTRL_04(val) \
REG_W(VDO_8TAP_CTRL_04, val)
#define DI_VDO_H265_CTRL                        (DI_VDO_BASE + 0x1a0)
#define DI_VDO_W_H265_EN(val) \
REG_W_BITS(DI_VDO_H265_CTRL, BITS(31, 31), 31, val)
#define VDO_SHARP_CTRL_01                           (DI_VDO_BASE + 0x1b0)
#define DI_VDO_W_VDO_SHARP_CTRL_01(val) \
REG_W(VDO_SHARP_CTRL_01, val)
#define VDO_SHARP_CTRL_02                         (DI_VDO_BASE + 0x1b4)
#define DI_VDO_W_VDO_SHARP_CTRL_02(val) \
REG_W(VDO_SHARP_CTRL_02, val)
#define VDO_SHARP_CTRL_03                         (DI_VDO_BASE + 0x1b8)
#define DI_VDO_W_VDO_SHARP_CTRL_03(val) \
REG_W(VDO_SHARP_CTRL_03, val)
#define DI_VDO_W_BYPASS_SHARP(val) \
REG_W_BITS(VDO_SHARP_CTRL_03, BITS(24, 24), 24, val)
#define VDOY1                         (DI_VDO_BASE + 0x400)
#define DI_VDO_W_VDOY1(val) \
REG_W(VDOY1, val)
#define VDOC1                         (DI_VDO_BASE + 0x404)
#define DI_VDO_W_VDOC1(val) \
REG_W(VDOC1, val)
#define VDOY2                         (DI_VDO_BASE + 0x408)
#define DI_VDO_W_VDOY2(val) \
REG_W(VDOY2, val)
#define VDOC2                         (DI_VDO_BASE + 0x40c)
#define DI_VDO_W_VDOC2(val) \
REG_W(VDOC2, val)
#define HBLOCK                         (DI_VDO_BASE + 0x410)
#define DI_VDO_W_swap_off(val) \
REG_W_BITS(HBLOCK, BITS(31, 31), 31, val)
#define DI_VDO_W_HBLOCK_9_8(val) \
REG_W_BITS(HBLOCK, BITS(29, 28), 28, val)
#define DI_VDO_W_PICHEIGHT(val) \
REG_W_BITS(HBLOCK, BITS(26, 16), 16, val)
#define DI_VDO_W_DW_NEED(val) \
REG_W_BITS(HBLOCK, BITS(15, 8), 8, val)
#define DI_VDO_W_HBLOCK_7_0(val) \
REG_W_BITS(HBLOCK, BITS(7, 0), 0, val)
#define VDO_VSCALE                         (DI_VDO_BASE + 0x414)
#define DI_VDO_W_VSCALE(val) \
REG_W(VDO_VSCALE, val)
#define STAMBR                         (DI_VDO_BASE + 0x418)
#define DI_VDO_W_STAMBR(val) \
REG_W(STAMBR, val)
#define VMODE                         (DI_VDO_BASE + 0x41c)
#define DI_VDO_W_VMODE(val) \
REG_W(VMODE, val)
#define YSL                         (DI_VDO_BASE + 0x420)
#define DI_VDO_W_YSL(val) \
REG_W(YSL, val)
#define CSL                         (DI_VDO_BASE + 0x424)
#define DI_VDO_W_CSL(val) \
REG_W(CSL, val)
#define YSSL                         (DI_VDO_BASE + 0x428)
#define DI_VDO_W_YSSL(val) \
REG_W(YSSL, val)
#define CSSL                         (DI_VDO_BASE + 0x42c)
#define DI_VDO_W_CSSL(val) \
REG_W(CSSL, val)
#define VDOCTRL                         (DI_VDO_BASE + 0x430)
#define DI_VDO_W_VDOCTRL(val) \
REG_W(VDOCTRL, val)
#define DI_VDO_W_YUV422(val) \
REG_W_BITS(VDOCTRL, BITS(21, 21), 21, val)
#define DI_VDO_W_AFLD(val) \
REG_W_BITS(VDOCTRL, BITS(5, 5), 5, val)
#define DI_VDO_W_PFLD(val) \
REG_W_BITS(VDOCTRL, BITS(2, 2), 2, val)
#define DI_VDO_W_FRMC(val) \
REG_W_BITS(VDOCTRL, BITS(1, 1), 1, val)
#define DI_VDO_W_FRMY(val) \
REG_W_BITS(VDOCTRL, BITS(0, 0), 0, val)
#define VSLACK                         (DI_VDO_BASE + 0x434)
#define DI_VDO_W_VSLACK(val) \
REG_W(VSLACK, val)
#define MP                         (DI_VDO_BASE + 0x438)
#define DI_VDO_W_MP(val) \
REG_W(MP, val)
#define DI_VDO_W_MTHRD(val) \
REG_W_BITS(MP, BITS(23, 16), 16, val)
#define DI_VDO_W_CT_THRD(val) \
REG_W_BITS(MP, BITS(15, 8), 8, val)
#define VDORST                         (DI_VDO_BASE + 0x43c)
#define DI_VDO_W_VDORST(val) \
REG_W(VDORST, val)
#define COMB_8F                         (DI_VDO_BASE + 0x440)
#define DI_VDO_W_COMB_8F(val) \
REG_W(COMB_8F, val)
#define YSL2                         (DI_VDO_BASE + 0x450)
#define DI_VDO_W_YSL2(val) \
REG_W(YSL2, val)
#define CSL2                         (DI_VDO_BASE + 0x454)
#define DI_VDO_W_CSL2(val) \
REG_W(CSL2, val)
#define MBAVG1                         (DI_VDO_BASE + 0x458)
#define DI_VDO_W_MBAVG1(val) \
REG_W(MBAVG1, val)
#define MBAVG2                         (DI_VDO_BASE + 0x45c)
#define DI_VDO_W_MBAVG2(val) \
REG_W(MBAVG2, val)
#define CMB_CNT                          (DI_VDO_BASE + 0x460)
#define DI_VDO_W_CMB_CNT(val) \
REG_W(CMB_CNT, val)
#define PS_MODE                         (DI_VDO_BASE + 0x464)
#define DI_VDO_W_PS_MODE(val) \
REG_W(PS_MODE, val)
#define STA_POS                         (DI_VDO_BASE + 0x46c)
#define DI_VDO_W_STA_POS(val) \
REG_W(STA_POS, val)
#define PS_FLT                         (DI_VDO_BASE + 0x470)
#define DI_VDO_W_PS_FLT(val) \
REG_W(PS_FLT, val)
#define F_CTRL                         (DI_VDO_BASE + 0x478)
#define DI_VDO_W_F_CTRL(val) \
REG_W(F_CTRL, val)
#define DI_VDO_W_INTRA_EDGEP_MODE(val) \
REG_W_BITS(F_CTRL, BITS(11, 11), 11, val)
#define VIDEO_OPTION                         (DI_VDO_BASE + 0x47c)
#define DI_VDO_W_VIDEO_OPTION(val) \
REG_W(VIDEO_OPTION, val)
#define PTR_WF_Y                         (DI_VDO_BASE + 0x480)
#define DI_VDO_W_PTR_WF_Y(val) \
REG_W(PTR_WF_Y, val)
#define PTR_ZF_Y                         (DI_VDO_BASE + 0x484)
#define DI_VDO_W_PTR_ZF_Y(val) \
REG_W(PTR_ZF_Y, val)

#define FDIFF_TH3                         (DI_VDO_BASE + 0x488)
#define DI_VDO_W_FDIFF_TH3(val) \
REG_W(FDIFF_TH3, val)
#define DI_VDO_W_FDIFF_CTRL(val) \
REG_W_BITS(FDIFF_TH3, BITS(27, 25), 25, val)
#define DI_VDO_W_MA4F(val) \
REG_W_BITS(FDIFF_TH3, BITS(24, 24), 24, val)
#define DI_VDO_W_FDIFF_TH_3(val) \
REG_W_BITS(FDIFF_TH3, BITS(23, 0), 0, val)

#define FDIFF_TH2                         (DI_VDO_BASE + 0x48c)
#define DI_VDO_W_FDIFF_TH2(val) \
REG_W(FDIFF_TH2, val)
#define DI_VDO_W_FDIFF_TH_2(val) \
REG_W_BITS(FDIFF_TH2, BITS(23, 0), 0, val)

#define FDIFF_TH1                         (DI_VDO_BASE + 0x490)
#define DI_VDO_W_FDIFF_TH1(val) \
REG_W(FDIFF_TH1, val)
#define DI_VDO_W_FDIFF_TH_1(val) \
REG_W_BITS(FDIFF_TH1, BITS(23, 0), 0, val)

#define TH_XZ_MIN                         (DI_VDO_BASE + 0x494)
#define DI_VDO_W_TH_XZ_MIN(val) \
REG_W(TH_XZ_MIN, val)
#define DI_VDO_W_TH_MED_XZ(val) \
REG_W_BITS(TH_XZ_MIN, BITS(21, 12), 12, val)
#define DI_VDO_W_TH_MIN_XZ(val) \
REG_W_BITS(TH_XZ_MIN, BITS(10, 0), 0, val)


#define TH_XZ_NM                          (DI_VDO_BASE + 0x498)
#define DI_VDO_W_TH_XZ_NM(val) \
REG_W(TH_XZ_NM, val)
#define DI_VDO_W_H_ED_TH(val) \
REG_W_BITS(TH_XZ_NM, BITS(31, 24), 24, val)
#define DI_VDO_W_TH_EDGE_XZ(val) \
REG_W_BITS(TH_XZ_NM, BITS(21, 12), 12, val)
#define DI_VDO_W_TH_NORM_XZ(val) \
REG_W_BITS(TH_XZ_NM, BITS(9, 0), 0, val)

#define TH_YW_MIN                         (DI_VDO_BASE + 0x49c)
#define DI_VDO_W_TH_YW_MIN(val) \
REG_W(TH_YW_MIN, val)
#define DI_VDO_W_SAW_TH(val) \
REG_W_BITS(TH_YW_MIN, BITS(31, 24), 24, val)
#define DI_VDO_W_TH_MED_YW(val) \
REG_W_BITS(TH_YW_MIN, BITS(20, 12), 12, val)
#define DI_VDO_W_TH_MIN_YW(val) \
REG_W_BITS(TH_YW_MIN, BITS(8, 0), 0, val)

#define TH_YW_NM                         (DI_VDO_BASE + 0x4a0)
#define DI_VDO_W_TH_YW_NM(val) \
REG_W(TH_YW_NM, val)
#define DI_VDO_W_WH_TX_TH(val) \
REG_W_BITS(TH_YW_NM, BITS(31, 24), 24, val)
#define DI_VDO_W_TH_ED_YW(val) \
REG_W_BITS(TH_YW_NM, BITS(20, 12), 12, val)
#define DI_VDO_W_TH_NM_YW(val) \
REG_W_BITS(TH_YW_NM, BITS(8, 0), 0, val)

#define FCH_XZ                         (DI_VDO_BASE + 0x4a4)
#define DI_VDO_W_FCH_XZ(val) \
REG_W(FCH_XZ, val)
#define DI_VDO_W_VMV_FCH(val) \
REG_W_BITS(FCH_XZ, BITS(31, 24), 24, val)
#define DI_VDO_W_FCH_NM_XZ(val) \
REG_W_BITS(FCH_XZ, BITS(21, 12), 12, val)
#define DI_VDO_W_FCH_MIN_XZ(val) \
REG_W_BITS(FCH_XZ, BITS(9, 0), 0, val)

#define FCH_YW                         (DI_VDO_BASE + 0x4a8)
#define DI_VDO_W_FCH_YW(val) \
REG_W(FCH_YW, val)
#define DI_VDO_W_X_POS_ST(val) \
REG_W_BITS(FCH_YW, BITS(31, 26), 26, val)
#define DI_VDO_W_FR_LVL(val) \
REG_W_BITS(FCH_YW, BITS(25, 24), 24, val)
#define DI_VDO_W_CRM_LVL(val) \
REG_W_BITS(FCH_YW, BITS(23, 22), 22, val)
#define DI_VDO_W_FCH_NM_YW(val) \
REG_W_BITS(FCH_YW, BITS(20, 12), 12, val)
#define DI_VDO_W_FCH_MIN_YW(val) \
REG_W_BITS(FCH_YW, BITS(8, 0), 0, val)

#define CRM_SAW                         (DI_VDO_BASE + 0x4ac)
#define DI_VDO_W_CRM_SAW(val) \
REG_W(CRM_SAW, val)

#define EDGE_CTL                         (DI_VDO_BASE + 0x4b0)
#define DI_VDO_W_EDGE_CTL(val) \
REG_W(EDGE_CTL, val)
#define DI_VDO_W_MA_EDGE_MODE6(val) \
REG_W_BITS(EDGE_CTL, BITS(30, 30), 30, val)

#define EDGE_MISC_TH                      (DI_VDO_BASE + 0x4b8)
#define DI_VDO_W_EDGE_3LINE_GRAD_TH(val) \
REG_W_BITS(EDGE_MISC_TH, BITS(23, 16), 16, val)

#define MD_ADV                         (DI_VDO_BASE + 0x4c0)
#define DI_VDO_W_MD_ADV(val) \
REG_W(MD_ADV, val)
#define DI_VDO_W_MA8F_OR(val) \
REG_W_BITS(MD_ADV, BITS(25, 25), 25, val)

#define PD_FLD_LIKE                         (DI_VDO_BASE + 0x4c4)
#define DI_VDO_W_PD_FLD_LIKE(val) \
REG_W(PD_FLD_LIKE, val)
#define DI_VDO_W_PD_COMB_TH(val) \
REG_W_BITS(PD_FLD_LIKE, BITS(22, 16), 16, val)
#define PD_REGION                         (DI_VDO_BASE + 0x4cc)
#define DI_VDO_W_PD_REGION(val) \
REG_W(PD_REGION, val)
#define CHROMA_MD                         (DI_VDO_BASE + 0x4d0)
#define DI_VDO_W_CHROMA_MD(val) \
REG_W(CHROMA_MD, val)
#define MBAVG3                         (DI_VDO_BASE + 0x4d8)
#define DI_VDO_W_MBAVG3(val) \
REG_W(MBAVG3, val)
#define HD_MODE                         (DI_VDO_BASE + 0x4e0)
#define DI_VDO_W_HD_MODE(val) \
REG_W(HD_MODE, val)
#define DI_VDO_W_HD_EN(val) \
REG_W_BITS(HD_MODE, BITS(24, 24), 24, val)
#define DI_VDO_W_SLE(val) \
REG_W_BITS(HD_MODE, BITS(23, 23), 23, val)
#define DI_VDO_W_HD_MEM(val) \
REG_W_BITS(HD_MODE, BITS(22, 22), 22, val)
#define DI_VDO_W_HD_MEM_1920(val) \
REG_W_BITS(HD_MODE, BITS(21, 21), 21, val)
#define DI_VDO_W_HD_LINE_MODE(val) \
REG_W_BITS(HD_MODE, BITS(20, 20), 20, val)
#define DI_VDO_W_DW_NEED_HD(val) \
REG_W_BITS(HD_MODE, BITS(8, 0), 0, val)
#define PTR_AF_Y                         (DI_VDO_BASE + 0x4ec)
#define DI_VDO_W_PTR_AF_Y(val) \
REG_W(PTR_AF_Y, val)
#define WMV_DISABLE                         (DI_VDO_BASE + 0x4f8)
#define DI_VDO_W_WMV_DISABLE(val) \
REG_W(WMV_DISABLE, val)
#define PTR_ZF_YC                         (DI_VDO_BASE + 0x4fc)
#define DI_VDO_W_PTR_ZF_YC(val) \
REG_W(PTR_ZF_YC, val)
#if DI_VDO_SUPPORT_METRIC
#define METRIC_00                         (DI_VDO_BASE + 0x700)
#define DI_VDO_W_METRIC_00(val) \
REG_W(METRIC_00, val)
#endif
#if DI_VDO_SUPPORT_FUSION
#define MDDI_FUSION_00                         (DI_VDO_BASE + 0x800)
#define DI_VDO_W_MDDI_FUSION_00(val) \
REG_W(MDDI_FUSION_00, val)
#define DI_VDO_W_IFUSION_EN(val) \
REG_W_BITS(MDDI_FUSION_00, BITS(0, 0), 0, val)
#define MDDI_FUSION_08                         (DI_VDO_BASE + 0x820)
#define DI_VDO_W_MDDI_FUSION_08(val) \
REG_W(MDDI_FUSION_08, val)
#define MDDI_FUSION_1A                         (DI_VDO_BASE + 0x868)
#define DI_VDO_W_MDDI_FUSION_1A(val) \
REG_W(MDDI_FUSION_1A, val)
#define MDDI_FUSION_1C                         (DI_VDO_BASE + 0x870)
#define DI_VDO_W_MDDI_FUSION_1C(val) \
REG_W(MDDI_FUSION_1C, val)
#define DI_VDO_W_en_lmr(val) \
REG_W_BITS(MDDI_FUSION_1C, BITS(29, 29), 29, val)
#define DI_VDO_W_en_lmw(val) \
REG_W_BITS(MDDI_FUSION_1C, BITS(28, 28), 28, val)
#define DI_VDO_W_fusion_flag_addr_base(val) \
REG_W_BITS(MDDI_FUSION_1C, BITS(27, 0), 0, val)
#define MDDI_FUSION_1D                         (DI_VDO_BASE + 0x874)
#define DI_VDO_W_MDDI_FUSION_1D(val) \
REG_W(MDDI_FUSION_1D, val)
#define DI_VDO_W_da_flag_waddr_hi_limit(val) \
REG_W_BITS(MDDI_FUSION_1D, BITS(27, 0), 0, val)
#define MDDI_FUSION_1E                         (DI_VDO_BASE + 0x878)
#define DI_VDO_W_MDDI_FUSION_1E(val) \
REG_W(MDDI_FUSION_1E, val)
#define DI_VDO_W_da_flag_waddr_lo_limit(val) \
REG_W_BITS(MDDI_FUSION_1E, BITS(27, 0), 0, val)
#define MDDI_FUSION_20                         (DI_VDO_BASE + 0x880)
#define DI_VDO_W_MDDI_FUSION_20(val) \
REG_W(MDDI_FUSION_20, val)
#define MDDI_FUSION_22                         (DI_VDO_BASE + 0x888)
#define DI_VDO_W_MDDI_FUSION_22(val) \
REG_W(MDDI_FUSION_22, val)
#define MDDI_FUSION_23                         (DI_VDO_BASE + 0x88c)
#define DI_VDO_W_MDDI_FUSION_23(val) \
REG_W(MDDI_FUSION_23, val)
#endif
#if DI_VDO_SUPPORT_MDDI_PE
#define MDDI_PE_00                         (DI_VDO_BASE + 0x8b0)
#define DI_VDO_W_MDDI_PE_00(val) \
REG_W(MDDI_PE_00, val)
#define MDDI_PE_03                         (DI_VDO_BASE + 0x8bc)
#define DI_VDO_W_MDDI_PE_03(val) \
REG_W(MDDI_PE_03, val)
#endif
#define VDO_CRC_00                         (DI_VDO_BASE + 0xf00)
#define DI_VDO_W_VDO_CRC_00(val) \
REG_W(VDO_CRC_00, val)
#define VDO_PQ_OPTION                         (DI_VDO_BASE + 0xf40)
#define DI_VDO_W_VDO_PQ_OPTION(val) \
REG_W(VDO_PQ_OPTION, val)
#define VDO_PQ_OPTION2                         (DI_VDO_BASE + 0xf44)
#define DI_VDO_W_VDO_PQ_OPTION2(val) \
REG_W(VDO_PQ_OPTION2, val)
#define MDDI_FILM_02                         (DI_VDO_BASE + 0xf60)
#define DI_VDO_W_MDDI_FILM_02(val) \
REG_W(MDDI_FILM_02, val)
#define CHROMA_SAW_CNT                         (DI_VDO_BASE + 0xfe0)
#define DI_VDO_W_CHROMA_SAW_CNT(val) \
REG_W(CHROMA_SAW_CNT, val)
#define VDO_PQ_OPTION5                         (DI_VDO_BASE + 0xfe4)
#define DI_VDO_W_VDO_PQ_OPTION5(val) \
REG_W(VDO_PQ_OPTION5, val)
#define DEMO_MODE                               (DI_VDO_BASE + 0xfec)
#define DI_VDO_W_DEMO_MODE(val) \
REG_W(DEMO_MODE, val)
#define VDO_PQ_OPTION9                         (DI_VDO_BASE + 0xff4)
#define DI_VDO_W_VDO_PQ_OPTION9(val) \
REG_W(VDO_PQ_OPTION9, val)
#define DI_VDO_W_chroma_multi_burst_threshold(val) \
REG_W_BITS(VDO_PQ_OPTION9, BITS(31, 24), 24, val)
#define DI_VDO_W_chroma_multi_burst_pixel_sel(val) \
REG_W_BITS(VDO_PQ_OPTION9, BITS(19, 19), 19, val)
#define DI_VDO_W_chroma_multi_burst_en(val) \
REG_W_BITS(VDO_PQ_OPTION9, BITS(18, 18), 18, val)
/* write channel 1 */
#define DI_WC_BASE                          (di_init.wch_reg_base)
#define VDOIN_EN                            (DI_WC_BASE + 0x000)
#define DI_WC_W_VDOIN_EN(val) \
REG_W(VDOIN_EN, val)
#define DI_WC_W_mode_422(val) \
REG_W_BITS(VDOIN_EN, BITS(25, 25), 25, val)
#define DI_WC_W_linear_enable(val) \
REG_W_BITS(VDOIN_EN, BITS(14, 14), 14, val)


/*64x32 block mode */
#define DI_WC_W_MACRO_64_32_BLOCK(val) \
REG_W_BITS(VDOIN_EN, BITS(15, 15), 15, val)


#define VDOIN_BLOCK_MODE                    (DI_WC_BASE + 0x88)

#define DI_WC_W_BLOCK_ACT_HCNT(val) \
REG_W(VDOIN_BLOCK_MODE, val)

#define VDOIN_MODE                          (DI_WC_BASE + 0x004)
#define DI_WC_W_VDOIN_MODE(val) \
REG_W(VDOIN_MODE, val)
#define YBUF_ADDR                           (DI_WC_BASE + 0x008)
#define DI_WC_W_YBUF0_ADDR(val) \
REG_W_BITS(YBUF_ADDR, BITS(29, 0), 0, val)
#define ACT_LINE                            (DI_WC_BASE + 0x00C)
#define DI_WC_W_ACT_LINE(val) \
REG_W(ACT_LINE, val)
#define DI_WC_W_ACTLINE(val) \
REG_W_BITS(ACT_LINE, BITS(11, 0), 0, val)
#define CBUF_ADDR                           (DI_WC_BASE + 0x010)
#define DI_WC_W_CBUF0_ADDR(val) \
REG_W_BITS(CBUF_ADDR, BITS(29, 0), 0, val)
#define DW_NEED                             (DI_WC_BASE + 0x014)
#define DI_WC_W_DW_NEED_C_LINE(val) \
REG_W_BITS(DW_NEED, BITS(27, 16), 16, val)
#define DI_WC_W_DW_NEED_Y_LINE(val) \
REG_W_BITS(DW_NEED, BITS(11, 0), 0, val)
#define HPIXEL                              (DI_WC_BASE + 0x018)
#define DI_WC_W_HPIXEL(val) \
REG_W(HPIXEL, val)
#define INPUT_CTRL                          (DI_WC_BASE + 0x020)
#define DI_WC_W_INPUT_CTRL(val) \
REG_W(INPUT_CTRL, val)
#define WRAPPER_3D_SETTING                  (DI_WC_BASE + 0x030)
#define DI_WC_W_swrst(val) \
REG_W_BITS(WRAPPER_3D_SETTING, BITS(31, 30), 30, val)
#define HCNT_SETTING                        (DI_WC_BASE + 0x034)
#define DI_WC_W_HCNT_SETTING(val) \
REG_W(HCNT_SETTING, val)
#define DI_WC_W_HACTCNT(val) \
REG_W_BITS(HCNT_SETTING, BITS(28, 16), 16, val)
#define HCNT_SETTING_1                      (DI_WC_BASE + 0x038)
#define DI_WC_W_CHCNT(val) \
REG_W_BITS(HCNT_SETTING_1, BITS(25, 16), 16, val)
#define DI_WC_W_YHCNT(val) \
REG_W_BITS(HCNT_SETTING_1, BITS(9, 0), 0, val)
#define WC_VSCALE                              (DI_WC_BASE + 0x03C)
#define DI_WC_W_VSCALE(val) \
REG_W(WC_VSCALE, val)
#define DI_WC_W_bghsize_dw(val) \
REG_W_BITS(WC_VSCALE, BITS(25, 16), 16, val)
#endif				/* MTK_DI_REG_H */
