/*
 * Copyright (c) 2017 MediaTek Inc.
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

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>

#include "nr_hal.h"
#include "vq_ion.h"
#include "vq_def.h"

unsigned int _bNRQtyTbl[QUALITY_NR_MAX] = {
	0x05,			/* QUALITY_SNR_MessSft_SM_Co1Mo */
	0x05,			/* QUALITY_SNR_MessThl_SM_Co1Mo */
	0x05,			/* QUALITY_SNR_MessSft_Mess_Co1Mo */
	0x05,			/* QUALITY_SNR_MessThl_Mess_Co1Mo */
	0x05,			/* QUALITY_SNR_MessSft_Edge_Co1Mo */
	0x05,			/* QUALITY_SNR_MessThl_Edge_Co1Mo */
	0x05,			/* QUALITY_SNR_MessSft_Mos_Co1Mo */
	0x05,			/* QUALITY_SNR_MessThl_Mos_Co1Mo */
	0x03,			/* QUALITY_SNR_MessSft_SM_Co1St */
	0x03,			/* QUALITY_SNR_MessThl_SM_Co1St */
	0x03,			/* QUALITY_SNR_MessSft_Mess_Co1St */
	0x03,			/* QUALITY_SNR_MessThl_Mess_Co1St */
	0x03,			/* QUALITY_SNR_MessSft_Edge_Co1St */
	0x03,			/* QUALITY_SNR_MessThl_Edge_Co1St */
	0x03,			/* QUALITY_SNR_MessSft_Mos_Co1St */
	0x03,			/* QUALITY_SNR_MessThl_Mos_Co1St */
	0x06,			/* QUALITY_SNR_BldLv_BK_Co1 */
	0x02,			/* QUALITY_SNR_BldLv_SM_Co1 */
	0x01,			/* QUALITY_SNR_BldLv_Mess_Co1 */
	0x01,			/* QUALITY_SNR_BldLv_Edge_Co1 */
	0x06,			/* QUALITY_SNR_BldLv_Mos_Co1 */
	0x04,			/* QUALITY_SNR_MessSft_SM_Co2Mo */
	0x04,			/* QUALITY_SNR_MessThl_SM_Co2Mo */
	0x04,			/* QUALITY_SNR_MessSft_Mess_Co2Mo */
	0x04,			/* QUALITY_SNR_MessThl_Mess_Co2Mo */
	0x04,			/* QUALITY_SNR_MessSft_Edge_Co2Mo */
	0x04,			/* QUALITY_SNR_MessThl_Edge_Co2Mo */
	0x04,			/* QUALITY_SNR_MessSft_Mos_Co2Mo */
	0x04,			/* QUALITY_SNR_MessThl_Mos_Co2Mo */
	0x04,			/* QUALITY_SNR_MessSft_SM_Co2St */
	0x04,			/* QUALITY_SNR_MessThl_SM_Co2St */
	0x04,			/* QUALITY_SNR_MessSft_Mess_Co2St */
	0x04,			/* QUALITY_SNR_MessThl_Mess_Co2St */
	0x04,			/* QUALITY_SNR_MessSft_Edge_Co2St */
	0x04,			/* QUALITY_SNR_MessThl_Edge_Co2St */
	0x04,			/* QUALITY_SNR_MessSft_Mos_Co2St */
	0x04,			/* QUALITY_SNR_MessThl_Mos_Co2St */
	0x06,			/* QUALITY_SNR_BldLv_BK_Co2 */
	0x02,			/* QUALITY_SNR_BldLv_SM_Co2 */
	0x01,			/* QUALITY_SNR_BldLv_Mess_Co2 */
	0x01,			/* QUALITY_SNR_BldLv_Edge_Co2 */
	0x06,			/* QUALITY_SNR_BldLv_Mos_Co2 */
	0x05,			/* QUALITY_SNR_MessSft_SM_Co3Mo */
	0x05,			/* QUALITY_SNR_MessThl_SM_Co3Mo */
	0x05,			/* QUALITY_SNR_MessSft_Mess_Co3Mo */
	0x05,			/* QUALITY_SNR_MessThl_Mess_Co3Mo */
	0x05,			/* QUALITY_SNR_MessSft_Edge_Co3Mo */
	0x05,			/* QUALITY_SNR_MessThl_Edge_Co3Mo */
	0x05,			/* QUALITY_SNR_MessSft_Mos_Co3Mo */
	0x05,			/* QUALITY_SNR_MessThl_Mos_Co3Mo */
	0x05,			/* QUALITY_SNR_MessSft_SM_Co3St */
	0x05,			/* QUALITY_SNR_MessThl_SM_Co3St */
	0x05,			/* QUALITY_SNR_MessSft_Mess_Co3St */
	0x05,			/* QUALITY_SNR_MessThl_Mess_Co3St */
	0x05,			/* QUALITY_SNR_MessSft_Edge_Co3St */
	0x05,			/* QUALITY_SNR_MessThl_Edge_Co3St */
	0x05,			/* QUALITY_SNR_MessSft_Mos_Co3St */
	0x05,			/* QUALITY_SNR_MessThl_Mos_Co3St */
	0x06,			/* QUALITY_SNR_BldLv_BK_Co3 */
	0x03,			/* QUALITY_SNR_BldLv_SM_Co3 */
	0x01,			/* QUALITY_SNR_BldLv_Mess_Co3 */
	0x01,			/* QUALITY_SNR_BldLv_Edge_Co3 */
	0x06,			/* QUALITY_SNR_BldLv_Mos_Co3 */
	0x05,			/* QUALITY_SNR_MessSft_SM_FrSt */
	0x05,			/* QUALITY_SNR_MessThl_SM_FrSt */
	0x05,			/* QUALITY_SNR_MessSft_Mess_FrSt */
	0x05,			/* QUALITY_SNR_MessThl_Mess_FrSt */
	0x05,			/* QUALITY_SNR_MessSft_Edge_FrSt */
	0x05,			/* QUALITY_SNR_MessThl_Edge_FrSt */
	0x05,			/* QUALITY_SNR_MessSft_Mos_FrSt */
	0x05,			/* QUALITY_SNR_MessThl_Mos_FrSt */
	0x06,			/* QUALITY_SNR_BldLv_BK_FrSt */
	0x01,			/* QUALITY_SNR_BldLv_SM_FrSt */
	0x01,			/* QUALITY_SNR_BldLv_Mess_FrSt */
	0x01,			/* QUALITY_SNR_BldLv_Edge_FrSt */
	0x06,			/* QUALITY_SNR_BldLv_Mos_FrSt */
	0x06,			/* QUALITY_SNR_MessSft_SM_Mo */
	0x06,			/* QUALITY_SNR_MessThl_SM_Mo */
	0x06,			/* QUALITY_SNR_MessSft_Mess_Mo */
	0x06,			/* QUALITY_SNR_MessThl_Mess_Mo */
	0x06,			/* QUALITY_SNR_MessSft_Edge_Mo */
	0x06,			/* QUALITY_SNR_MessThl_Edge_Mo */
	0x06,			/* QUALITY_SNR_MessSft_Mos_Mo */
	0x06,			/* QUALITY_SNR_MessThl_Mos_Mo */
	0x06,			/* QUALITY_SNR_BldLv_BK_Mo */
	0x03,			/* QUALITY_SNR_BldLv_SM_Mo */
	0x01,			/* QUALITY_SNR_BldLv_Mess_Mo */
	0x01,			/* QUALITY_SNR_BldLv_Edge_Mo */
	0x06,			/* QUALITY_SNR_BldLv_Mos_Mo */
	0x05,			/* QUALITY_SNR_MessSft_SM_St */
	0x05,			/* QUALITY_SNR_MessThl_SM_St */
	0x05,			/* QUALITY_SNR_MessSft_Mess_St */
	0x05,			/* QUALITY_SNR_MessThl_Mess_St */
	0x05,			/* QUALITY_SNR_MessSft_Edge_St */
	0x05,			/* QUALITY_SNR_MessThl_Edge_St */
	0x05,			/* QUALITY_SNR_MessSft_Mos_St */
	0x05,			/* QUALITY_SNR_MessThl_Mos_St */
	0x06,			/* QUALITY_SNR_BldLv_BK_St */
	0x03,			/* QUALITY_SNR_BldLv_SM_St */
	0x01,			/* QUALITY_SNR_BldLv_Mess_St */
	0x01,			/* QUALITY_SNR_BldLv_Edge_St */
	0x06,			/* QUALITY_SNR_BldLv_Mos_St */
	0x05,			/* QUALITY_SNR_MessSft_SM_BK */
	0x05,			/* QUALITY_SNR_MessThl_SM_BK */
	0x05,			/* QUALITY_SNR_MessSft_Mess_BK */
	0x05,			/* QUALITY_SNR_MessThl_Mess_BK */
	0x05,			/* QUALITY_SNR_MessSft_Edge_BK */
	0x05,			/* QUALITY_SNR_MessThl_Edge_BK */
	0x05,			/* QUALITY_SNR_MessSft_Mos_BK */
	0x05,			/* QUALITY_SNR_MessThl_Mos_BK */
	0x07,			/* QUALITY_SNR_BldLv_BK_BK */
	0x03,			/* QUALITY_SNR_BldLv_SM_BK */
	0x02,			/* QUALITY_SNR_BldLv_Mess_BK */
	0x02,			/* QUALITY_SNR_BldLv_Edge_BK */
	0x06,			/* QUALITY_SNR_BldLv_Mos_BK */
	0x05,			/* QUALITY_SNR_MessSft_SM_Def */
	0x05,			/* QUALITY_SNR_MessThl_SM_Def */
	0x05,			/* QUALITY_SNR_MessSft_Mess_Def */
	0x05,			/* QUALITY_SNR_MessThl_Mess_Def */
	0x05,			/* QUALITY_SNR_MessSft_Edge_Def */
	0x05,			/* QUALITY_SNR_MessThl_Edge_Def */
	0x05,			/* QUALITY_SNR_MessSft_Mos_Def */
	0x05,			/* QUALITY_SNR_MessThl_Mos_Def */
	0x02,			/* QUALITY_SNR_BldLv_SM_Def */
	0x01,			/* QUALITY_SNR_BldLv_Mess_Def */
	0x01,			/* QUALITY_SNR_BldLv_Edge_Def */
	0x06,			/* QUALITY_SNR_BldLv_Mos_Def */
	0x02,			/* QUALITY_SNR_cur_sm_num */
	0x0C,			/* QUALITY_SNR_cur_sm_thr */
	0x04,			/* QUALITY_SNR_nearedge_selwidth */
	0x19,			/* QUALITY_SNR_nearedge_edge_thr */
	0x0A,			/* QUALITY_SNR_global_blend */
};

#define wReadNRQualityTable(wAddr)	 _bNRQtyTbl[wAddr]

struct nr_info nrInfo;

static const char *const _ap_mtk_nr_clk_name[MTK_NR_CLK_CNT] = {
	"nr", "nr_sel", "univpll_d2"
};

static irqreturn_t nr_irq_handler(int value, void *dev_id)
{
	struct nr_info *nr = (struct nr_info *)dev_id;

	vNRWriteRegMsk(nr->nr_reg_base + RW_NR_INT_CLR, 1 << 4, 1 << 4);

	atomic_set(&nr->wait_nr_irq_flag, 1);
	wake_up_interruptible(&nr->wait_nr_irq_wq);

	return IRQ_HANDLED;
}

static void nr_hal_set_dram_burst_read(void __iomem *reg_base, bool fgEnable)
{
	unsigned int u4Value;

	/* check other setting */
	if ((vNrReadReg(reg_base + RW_NR_HD_LINE_OFST) & 0x3) != 0)
		fgEnable = false;	/* VERIFY(0); */

	if (fgEnable) {
		u4Value = (vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL) | BURST_READ);
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);
	} else {
		u4Value = (vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL) & (~BURST_READ));
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);
	}
}

static void nr_hal_set_swap_mode(void __iomem *reg_base, unsigned char u1SwapMode)
{
	unsigned int u4Value;

	switch (u1SwapMode) {
	case MT8520_SWAP_MODE_0:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x01400000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);
		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000040;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT8520_SWAP_MODE_1:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x01500000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);

		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000050;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT8520_SWAP_MODE_2:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x01600000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);

		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000060;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT5351_SWAP_MODE_0:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x00000000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);

		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000000;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT5351_SWAP_MODE_1:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x00100000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);
		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000010;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT5351_SWAP_MODE_2:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x00200000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);

		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000020;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	case MT5351_SWAP_MODE_3:
		u4Value = vNrReadReg(reg_base + RW_NR_HD_MODE_CTRL);
		u4Value = u4Value & 0xFC8FFFFF;
		u4Value = u4Value | 0x01300000;
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, u4Value);

		u4Value = vNrReadReg(reg_base + RW_NR_DRAM_CTRL_00);

		u4Value = u4Value & 0xFFFFFF8F;
		u4Value = u4Value | 0x00000030;

		vNrWriteReg(reg_base + RW_NR_DRAM_CTRL_00, u4Value);

		break;
	}
}

static void nr_hal_set_level(void __iomem *reg_base, unsigned int u4Strength,
	unsigned int u4MNRStrength, unsigned int u4BNRStrength)
{
    /* 2D NR setting */
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO1MO) << 24,
		       MESSSFT_SMOOTH_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO1MO) << 16,
		       MESSTHL_SMOOTH_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO1MO) << 24,
		       MESSSFT_MESS_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO1MO) << 16,
		       MESSTHL_MESS_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO1MO) << 8,
		       MESSSFT_EDGE_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO1MO) << 0,
		       MESSTHL_EDGE_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO1ST) << 24,
		       MESSSFT_SMOOTH_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO1ST) << 16,
		       MESSTHL_SMOOTH_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO1ST) << 24,
		       MESSSFT_MESS_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO1ST) << 16,
		       MESSTHL_MESS_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO1ST) << 8,
		       MESSSFT_EDGE_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO1ST) << 0,
		       MESSTHL_EDGE_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_CO1) << 8,
		       BLDLV_SM_CO1);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_CO1) << 4,
		       BLDLV_MESS_CO1);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_CO1) << 0,
		       BLDLV_EDGE_CO1);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO2MO) << 24,
		       MESSSFT_SMOOTH_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO2MO) << 16,
		       MESSTHL_SMOOTH_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO2MO) << 24,
		       MESSSFT_MESS_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO2MO) << 16,
		       MESSTHL_MESS_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO2MO) << 8,
		       MESSSFT_EDGE_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO2MO) << 0,
		       MESSTHL_EDGE_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO2ST) << 24,
		       MESSSFT_SMOOTH_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO2ST) << 16,
		       MESSTHL_SMOOTH_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO2ST) << 24,
		       MESSSFT_MESS_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO2ST) << 16,
		       MESSTHL_MESS_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO2ST) << 8,
		       MESSSFT_EDGE_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO2ST) << 0,
		       MESSTHL_EDGE_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_CO2) << 24,
		       BLDLV_SM_CO2);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_CO2) << 20,
		       BLDLV_MESS_CO2);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_CO2) << 16,
		       BLDLV_EDGE_CO2);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO3MO) << 24,
		       MESSSFT_SMOOTH_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO3MO) << 16,
		       MESSTHL_SMOOTH_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO3MO) << 24,
		       MESSSFT_MESS_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO3MO) << 16,
		       MESSTHL_MESS_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO3MO) << 8,
		       MESSSFT_EDGE_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO3MO) << 0,
		       MESSTHL_EDGE_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_CO3ST) << 24,
		       MESSSFT_SMOOTH_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_CO3ST) << 16,
		       MESSTHL_SMOOTH_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42,
		       wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_CO3ST) << 24,
		       MESSSFT_MESS_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42,
		       wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_CO3ST) << 16,
		       MESSTHL_MESS_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_CO3ST) << 8,
		       MESSSFT_EDGE_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_CO3ST) << 0,
		       MESSTHL_EDGE_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_CO3) << 8,
		       BLDLV_SM_CO3);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_CO3) << 4,
		       BLDLV_MESS_CO3);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_CO3) << 0,
		       BLDLV_EDGE_CO3);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_FRST) << 24,
		       MESSSFT_SMOOTH_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_FRST) << 16,
		       MESSTHL_SMOOTH_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_FRST) << 24,
		       MESSSFT_MESS_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_FRST) << 16,
		       MESSTHL_MESS_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_FRST) << 8,
		       MESSSFT_EDGE_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_FRST) << 0,
		       MESSTHL_EDGE_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_FRST) << 24,
		       BLDLV_SM_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_FRST) << 20,
		       BLDLV_MESS_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_FRST) << 16,
		       BLDLV_EDGE_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_MO) << 24,
		       MESSSFT_SMOOTH_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_MO) << 16,
		       MESSTHL_SMOOTH_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_MO) << 24,
		       MESSSFT_MESS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_MO) << 16,
		       MESSTHL_MESS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_MO) << 8,
		       MESSSFT_EDGE_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_MO) << 0,
		       MESSTHL_EDGE_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_MO) << 8,
		       BLDLV_SM_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_MO) << 4,
		       BLDLV_MESS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_MO) << 0,
		       BLDLV_EDGE_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_ST) << 24,
		       MESSSFT_SMOOTH_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_ST) << 16,
		       MESSTHL_SMOOTH_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_ST) << 24,
		       MESSSFT_MESS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_ST) << 16,
		       MESSTHL_MESS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_ST) << 8,
		       MESSSFT_EDGE_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_ST) << 0,
		       MESSTHL_EDGE_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_ST) << 24,
		       BLDLV_SM_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_ST) << 20,
		       BLDLV_MESS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_ST) << 16,
		       BLDLV_EDGE_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_BK) << 24,
		       MESSSFT_SMOOTH_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_BK) << 16,
		       MESSTHL_SMOOTH_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_BK) << 24,
		       MESSSFT_MESS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_BK) << 16,
		       MESSTHL_MESS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_BK) << 8,
		       MESSSFT_EDGE_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_BK) << 0,
		       MESSTHL_EDGE_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_BK) << 24,
		       BLDLV_SM_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_BK) << 20,
		       BLDLV_MESS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_BK) << 16,
		       BLDLV_EDGE_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, wReadNRQualityTable(QUALITY_SNR_MESSSFT_SM_DEF) << 24,
		       MESSSFT_SMOOTH_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, wReadNRQualityTable(QUALITY_SNR_MESSTHL_SM_DEF) << 16,
		       MESSTHL_SMOOTH_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MESS_DEF) << 24,
		       MESSSFT_MESS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MESS_DEF) << 16,
		       MESSTHL_MESS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, wReadNRQualityTable(QUALITY_SNR_MESSSFT_EDGE_DEF) << 8,
		       MESSSFT_EDGE_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, wReadNRQualityTable(QUALITY_SNR_MESSTHL_EDGE_DEF) << 0,
		       MESSTHL_EDGE_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_SM_DEF) << 8,
		       BLDLV_SM_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_MESS_DEF) << 4,
		       BLDLV_MESS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_EDGE_DEF) << 0,
		       BLDLV_EDGE_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_6E, wReadNRQualityTable(QUALITY_SNR_GLOBAL_BLEND) << 28,
		       Y_GLOBAL_BLEND);

	/* BNR setting */
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO1MO) << 8,
		       MESSSFT_MOS_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO1MO) << 0,
		       MESSTHL_MOS_CO1MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO1ST) << 8,
		       MESSSFT_MOS_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO1ST) << 0,
		       MESSTHL_MOS_CO1ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_CO1) << 12,
		       BLDLV_BK_CO1);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_CO1) << 4,
		       BLDLV_MOS_CO1);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO2MO) << 8,
		       MESSSFT_MOS_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO2MO) << 0,
		       MESSTHL_MOS_CO2MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO2ST) << 8,
		       MESSSFT_MOS_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO2ST) << 0,
		       MESSTHL_MOS_CO2ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_CO2) << 28,
		       BLDLV_BK_CO2);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_CO2) << 8,
		       BLDLV_MOS_CO2);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO3MO) << 8,
		       MESSSFT_MOS_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO3MO) << 0,
		       MESSTHL_MOS_CO3MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_CO3ST) << 8,
		       MESSSFT_MOS_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_CO3ST) << 0,
		       MESSTHL_MOS_CO3ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_CO3) << 12,
		       BLDLV_BK_CO3);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_CO3) << 12,
		       BLDLV_MOS_CO3);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_FRST) << 8,
		       MESSSFT_MOS_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_FRST) << 0,
		       MESSTHL_MOS_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_FRST) << 28,
		       BLDLV_BK_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_FRST) << 0,
		       BLDLV_NEAR_FRST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_MO) << 8,
		       MESSSFT_MOS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_MO) << 0,
		       MESSTHL_MOS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_MO) << 12,
		       BLDLV_BK_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_MO) << 24,
		       BLDLV_MOS_MO);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_ST) << 8,
		       MESSSFT_MOS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_ST) << 0,
		       MESSTHL_MOS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_ST) << 28,
		       BLDLV_BK_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_ST) << 20,
		       BLDLV_MOS_ST);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_BK) << 8,
		       MESSSFT_MOS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_BK) << 0,
		       MESSTHL_MOS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, wReadNRQualityTable(QUALITY_SNR_BLDLV_BK_BK) << 28,
		       BLDLV_BK_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_BK) << 28,
		       BLDLV_MOS_BK);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, wReadNRQualityTable(QUALITY_SNR_MESSSFT_MOS_DEF) << 8,
		       MESSSFT_MOS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, wReadNRQualityTable(QUALITY_SNR_MESSTHL_MOS_DEF) << 0,
		       MESSTHL_MOS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, wReadNRQualityTable(QUALITY_SNR_BLDLV_MOS_DEF) << 16,
		       BLDLV_MOS_DEF);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_65, wReadNRQualityTable(QUALITY_SNR_CUR_SM_NUM) << 8,
		       SM_NUM_THR);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_65, wReadNRQualityTable(QUALITY_SNR_CUR_SM_THR) << 24,
		       MNR_SM_THR);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_65, wReadNRQualityTable(QUALITY_SNR_NEAREDGE_SELWIDTH) << 0,
		       NEAREDGE_SEL_WIDTH);
	vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_65, wReadNRQualityTable(QUALITY_SNR_NEAREDGE_EDGE_THR) << 16,
		       MNR_EDGE_THR);

	if (u4Strength != NR_STRENGTH_OFF) {
		switch (u4Strength) {
		case NR_STRENGTH_LOW:
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 12), BLDLV_BK_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 28), BLDLV_BK_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 12), BLDLV_BK_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 28), BLDLV_BK_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 12), BLDLV_BK_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 28), BLDLV_BK_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 12), BLDLV_BK_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 28), BLDLV_BK_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 4), BLDLV_MOS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 8), BLDLV_MOS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 12), BLDLV_MOS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 20), BLDLV_MOS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 24), BLDLV_MOS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 0), BLDLV_NEAR_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 16), BLDLV_MOS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 28), BLDLV_MOS_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 8), BLDLV_SM_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 4), BLDLV_MESS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 0), BLDLV_EDGE_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 24), BLDLV_SM_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 20), BLDLV_MESS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 16), BLDLV_EDGE_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 8), BLDLV_SM_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 4), BLDLV_MESS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 0), BLDLV_EDGE_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 24), BLDLV_SM_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 20), BLDLV_MESS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 16), BLDLV_EDGE_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 8), BLDLV_SM_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 4), BLDLV_MESS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 0), BLDLV_EDGE_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 24), BLDLV_SM_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 20), BLDLV_MESS_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 16), BLDLV_EDGE_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 8), BLDLV_SM_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 4), BLDLV_MESS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 0), BLDLV_EDGE_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x48084808, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x20000000, 0x28000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
			break;
		case NR_STRENGTH_MED:
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 12), BLDLV_BK_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 28), BLDLV_BK_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 12), BLDLV_BK_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 28), BLDLV_BK_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 12), BLDLV_BK_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 28), BLDLV_BK_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 12), BLDLV_BK_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 28), BLDLV_BK_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 4), BLDLV_MOS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 8), BLDLV_MOS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 12), BLDLV_MOS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 20), BLDLV_MOS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 24), BLDLV_MOS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 0), BLDLV_NEAR_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 16), BLDLV_MOS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 28), BLDLV_MOS_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 8), BLDLV_SM_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 4), BLDLV_MESS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 0), BLDLV_EDGE_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 24), BLDLV_SM_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 20), BLDLV_MESS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 16), BLDLV_EDGE_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 8), BLDLV_SM_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 4), BLDLV_MESS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 0), BLDLV_EDGE_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 24), BLDLV_SM_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 20), BLDLV_MESS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 16), BLDLV_EDGE_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 8), BLDLV_SM_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 4), BLDLV_MESS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 0), BLDLV_EDGE_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 24), BLDLV_SM_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 20), BLDLV_MESS_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 16), BLDLV_EDGE_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 8), BLDLV_SM_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 4), BLDLV_MESS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 0), BLDLV_EDGE_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x4B0B4B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
			break;
		case NR_STRENGTH_HIGH:
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 12), BLDLV_BK_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 28), BLDLV_BK_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 12), BLDLV_BK_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 28), BLDLV_BK_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 12), BLDLV_BK_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 28), BLDLV_BK_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 12), BLDLV_BK_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 28), BLDLV_BK_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 4), BLDLV_MOS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 8), BLDLV_MOS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 12), BLDLV_MOS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 20), BLDLV_MOS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 24), BLDLV_MOS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 0), BLDLV_NEAR_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 16), BLDLV_MOS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 28), BLDLV_MOS_BK);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 8), BLDLV_SM_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 4), BLDLV_MESS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 0), BLDLV_EDGE_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 24), BLDLV_SM_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 20), BLDLV_MESS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 16), BLDLV_EDGE_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 8), BLDLV_SM_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 4), BLDLV_MESS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 0), BLDLV_EDGE_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 24), BLDLV_SM_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 20), BLDLV_MESS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 16), BLDLV_EDGE_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 8), BLDLV_SM_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 4), BLDLV_MESS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 0), BLDLV_EDGE_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 24), BLDLV_SM_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 20), BLDLV_MESS_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 16), BLDLV_EDGE_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 8), BLDLV_SM_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 4), BLDLV_MESS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 0), BLDLV_EDGE_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x8F0F8B0B, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x8B0B8F0F, 0xFF3FFF3F);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x04000000, 0x14000000);
			break;
		default:
			break;
		}
	} else {
		if (u4MNRStrength == NR_STRENGTH_OFF) {
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 4), BLDLV_MOS_CO1);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 8), BLDLV_MOS_CO2);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 12), BLDLV_MOS_CO3);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 20), BLDLV_MOS_ST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 24), BLDLV_MOS_MO);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 0), BLDLV_NEAR_FRST);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 16), BLDLV_MOS_DEF);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 28), BLDLV_MOS_BK);
		} else {
			switch (u4MNRStrength) {
			case NR_STRENGTH_LOW:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x20000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
				break;
			case NR_STRENGTH_MED:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
				break;
			case NR_STRENGTH_HIGH:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x04000000, 0x14000000);
				break;
			default:
				break;
			}
		}

		if (u4BNRStrength == NR_STRENGTH_OFF) {
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000000, BLOCK_PROC_ENABLE);
		} else {
			switch (u4BNRStrength) {
			case NR_STRENGTH_LOW:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x1 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x1 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x1 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x1 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x48084808, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x20000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
				break;
			case NR_STRENGTH_MED:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x2 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x2 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x2 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x2 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x4B0B4B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x10000000, 0x14000000);
				break;
			case NR_STRENGTH_HIGH:
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 12), BLDLV_BK_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 28), BLDLV_BK_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 12), BLDLV_BK_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 28), BLDLV_BK_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 12), BLDLV_BK_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 28), BLDLV_BK_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 12), BLDLV_BK_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 28), BLDLV_BK_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 28), BLDLV_MOS_BK);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 8), BLDLV_SM_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 4), BLDLV_MESS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 0), BLDLV_EDGE_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 24), BLDLV_SM_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 20), BLDLV_MESS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 16), BLDLV_EDGE_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 8), BLDLV_SM_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 4), BLDLV_MESS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x3 << 0), BLDLV_EDGE_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 24), BLDLV_SM_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 20), BLDLV_MESS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 16), BLDLV_EDGE_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 8), BLDLV_SM_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 4), BLDLV_MESS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x3 << 0), BLDLV_EDGE_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 24), BLDLV_SM_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 20), BLDLV_MESS_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x3 << 16), BLDLV_EDGE_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 8), BLDLV_SM_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 4), BLDLV_MESS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x3 << 0), BLDLV_EDGE_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_37, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_38, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_39, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3A, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3B, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3C, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3D, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3E, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_3F, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_40, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_41, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_42, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_4F, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_50, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_51, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_52, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_53, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_54, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_55, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_56, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_57, 0x8F0F8B0B, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_58, 0x8B0B8F0F, 0xFF3FFF3F);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x08000000, 0x28000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_8E, 0x00000000, 0x02000000);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_09, 0x04000000, 0x14000000);
				break;
			default:
				break;
			}
		}
	}
}

static void nr_hal_adaptive_bnr_para(void __iomem *reg_base, struct nr_config_param *ptNrPrm)
{
	unsigned char u1HineCnt, u1VLineCnt, u1HBKLv, u1VBKLv;
	unsigned int u4RegValue;

	u4RegValue = vNrReadReg(reg_base + RW_NR_2DNR_CTRL_8F);

	u1HineCnt = ((u4RegValue >> 8) & 0x7FF);
	u1VLineCnt = ((u4RegValue >> 20) & 0x3FF);

	u1HBKLv = ((u4RegValue >> 0) & 0x7);
	u1VBKLv = ((u4RegValue >> 4) & 0x7);

	if ((u1VLineCnt >= (ptNrPrm->u2PicWidth / (16 * 2)))
	    || (u1HineCnt >= (ptNrPrm->u2PicHeight / (16 * 2)))) {
		ptNrPrm->fgUseBlockMeter = false;
	} else {
		ptNrPrm->fgUseBlockMeter = true;
	}

	if (u1VBKLv == 7) {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_08, 0x0005A040, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0C, 0x0006405A, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_10, 0x0005A040, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_14, 0x0005A040, 0x007FF7FF);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_08, 0x0002D020, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0C, 0x0003602D, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_10, 0x0002D820, 0x007FF7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_14, 0x0002D020, 0x007FF7FF);
	}
	if (u1HBKLv == 7) {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0A, 0x20180000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0E, 0x20180000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_12, 0x20180000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_16, 0x20180000, 0xFFFF0000);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0A, 0x0E0A0000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_0E, 0x0E0A0000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_12, 0x0E0A0000, 0xFFFF0000);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_16, 0x0E0A0000, 0xFFFF0000);
	}
}

int nr_hal_request_irq(struct nr_info *nr)
{
	return devm_request_irq(nr->dev, nr->irq, nr_irq_handler, 0, "nr", (void *)nr);
}

void nr_hal_free_irq(struct nr_info *nr)
{
	devm_free_irq(nr->dev, nr->irq, (void *)nr);
}

int nr_hal_hw_init(struct platform_device *pdev, struct nr_info *nr)
{
	int ret;
	int i;
	struct device *dev = &pdev->dev;
	struct resource *res;

	for (i = 0; i < MTK_NR_CLK_CNT; i++) {
		nr->clks[i] = devm_clk_get(dev, _ap_mtk_nr_clk_name[i]);
		if (IS_ERR(nr->clks[i])) {
			dev_notice(dev, "[NR]fail to get clk[%d] %s\n", i, _ap_mtk_nr_clk_name[i]);
			return -EPROBE_DEFER;
		}

		VQ_INFO("[NR] get clk[%d] %s 0x%lx\n", i, _ap_mtk_nr_clk_name[i],
			(unsigned long)nr->clks[i]);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_NR);
	if (!res) {
		dev_notice(dev, "[NR]failed to get MEM resource 0\n");
		return -ENXIO;
	}

	nr->dev = dev;

	nr->nr_reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(nr->nr_reg_base)) {
		dev_notice(dev, "[NR]get nr reg base err\n");
		return PTR_ERR(nr->nr_reg_base);
	}
	VQ_INFO("[NR] nr reg base is 0x%lx\n", (unsigned long)nr->nr_reg_base);

	nr->irq = platform_get_irq(pdev, MTK_VQ_IRQ_NR);
	if (nr->irq < 0) {
		dev_notice(dev, "[NR] get irq err\n");
		return nr->irq;
	}
	VQ_INFO("[NR] nr irq is %d\n", nr->irq);

	ret = nr_hal_request_irq(nr);
	if (ret) {
		dev_notice(dev, "[NR]failed to install irq (%d)\n", ret);
		return -ENXIO;
	}

	init_waitqueue_head(&nr->wait_nr_irq_wq);

	return ret;
}

int nr_hal_wait_complete_timeout(struct nr_info *nr, unsigned int time)
{
	int ret = 0;

	if (wait_event_interruptible_timeout(nr->wait_nr_irq_wq,
						  atomic_read(&nr->wait_nr_irq_flag), time) == 0) {
		pr_info("[NR] error wait frame done timeout %dms\n", time);
		ret = -ENXIO;
	}

	atomic_set(&nr->wait_nr_irq_flag, 0);

	return ret;

}

int nr_hal_hw_reset(void __iomem *reg_base)
{
	vNrWriteReg(reg_base + RW_NR_MAIN_CTRL_01, 0xfe000000);

	vNrWriteReg(reg_base + RW_NR_MAIN_CTRL_01, 0x00000000);

	return 0;
}

static void nr_hal_config(void __iomem *reg_base, struct nr_config_param *ptNrPrm)
{
	unsigned short u2HActive = 0;
	unsigned short u2VActive = 0;
	unsigned int u4MBCnt = 0;
	unsigned int u4Value = 0;

	u2HActive = ptNrPrm->u2FrameWidth;
	u2VActive = ptNrPrm->u2PicHeight;	/* ptNrPrm->u2FrameHeight; */
	u4MBCnt = ((ptNrPrm->u2FrameWidth + 15) >> 4);

	if (ptNrPrm->u4NRMode == NR_STD_MODE) {
		vNRWriteRegMsk(reg_base + RW_NR_3D_00, 0x38000000, 0x38000000);
		vNrWriteReg(reg_base + RW_NR_YCBCR2YC_MAIN_00, 0x1);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_3D_00, 0x08000000, 0x08000000);
		vNrWriteReg(reg_base + RW_NR_YCBCR2YC_MAIN_00, 0x40);
	}

	if (ptNrPrm->u4NRMode == NR_STD_MODE) {
		vNrWriteReg(reg_base + RW_NR_CURR_Y_RD_ADDR, ptNrPrm->u4CurrRdYAddr);
		vNrWriteReg(reg_base + RW_NR_CURR_C_RD_ADDR, ptNrPrm->u4CurrRdCAddr);

		vNrWriteReg(reg_base + RW_NR_Y_WR_SADDR, ptNrPrm->u4CurrWrYAddr);
		vNrWriteReg(reg_base + RW_NR_Y_WR_EADDR, 0xFFFFFFFF);
		vNrWriteReg(reg_base + RW_NR_C_WR_SADDR, ptNrPrm->u4CurrWrCAddr);
		vNrWriteReg(reg_base + RW_NR_C_WR_EADDR, 0xFFFFFFFF);
	}
	vNrWriteReg(reg_base + RW_NR_HD_HDE_RATIO, 0x00000000);

	u2HActive = u2HActive / 2;
	if (ptNrPrm->u1FrameMode == MODE_FRAME) {
		u4Value = u2VActive;
		u4Value = u4Value << 20;
		u4Value = u4Value | u2HActive;
		vNrWriteReg(reg_base + RW_NR_HD_ACTIVE, u4Value);
	} else {
		u4Value = u2VActive / 2;
		u4Value = u4Value << 20;
		u4Value = u4Value | u2HActive;
		vNrWriteReg(reg_base + RW_NR_HD_ACTIVE, u4Value);
	}
	vNRWriteRegMsk(reg_base + RW_NR_HD_LINE_OFST, u4MBCnt, 0x0000007F);

	if (ptNrPrm->u1FrameMode == MODE_FRAME)
		vNRWriteRegMsk(reg_base + RW_NR_DRAM_CTRL_00, 0x00001000, 0x0000FF01);
	else
		vNRWriteRegMsk(reg_base + RW_NR_DRAM_CTRL_00, 0x00001001, 0x0000FF01);

	/* reset NR module */
	vNrWriteReg(reg_base + RW_NR_MISC_CTRL, 0x000000ff);

	/* select CRC source */
	vNrWriteReg(reg_base + RW_NR_MAIN_CTRL_00, 0x6010007c);	/* current output as CRC source */

	vNrWriteReg(reg_base + RW_NR_HD_PATH_ENABLE, 0x00000000);

	vNrWriteReg(reg_base + RW_NR_MISC_CTRL, 0x00000000);

	/* Fram / Field / Top Field / Bot Field */
	switch (ptNrPrm->u1FrameMode) {
	case MODE_FRAME:
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, 0x01400103);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000004);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000000);
		/* enable CRC */
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000101);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		break;
	case MODE_FIELD:
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, 0x01405143);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000004);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000000);
		/* enable CRC */
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000109);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000108);
		break;
	case MODE_TOP_FIELD:
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, 0x01405103);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000005);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000000);
		/* enable CRC */
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000101);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		break;
	case MODE_BOT_FIELD:
		vNrWriteReg(reg_base + RW_NR_HD_MODE_CTRL, 0x01405183);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000005);
		vNrWriteReg(reg_base + RW_NR_HD_SYNC_TRIGGER, 0x00000000);
		/* enable CRC */
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000101);
		vNrWriteReg(reg_base + RW_NR_CRC_SETTING, 0x00000100);
		break;
	default:
		break;
	}

	if (ptNrPrm->u4NRMode == NR_STD_MODE) {
		nr_hal_set_swap_mode(reg_base, ptNrPrm->u1AddrSwapMode);

		nr_hal_set_dram_burst_read(reg_base, ptNrPrm->fgBurstRdEn);
	}

	/* Range Remap Setting */
	{
		u4Value = (vNrReadReg(reg_base + RW_NR_HD_RANGE_MAP) & 0xFFFFE0E0);
		if (ptNrPrm->fgRangeRemapYEn)
			u4Value |= ((ptNrPrm->u4RangeMapY + 9) & 0x1F);
		else
			u4Value |= 0x8;

		if (ptNrPrm->fgRangeRemapUVEn)
			u4Value |= (((ptNrPrm->u4RangeMapUV + 9) & 0x1F) << 8);
		else
			u4Value |= 0x800;

		vNrWriteReg(reg_base + RW_NR_HD_RANGE_MAP, u4Value);
	}

	u4MBCnt = ((ptNrPrm->u2FrameWidth + 15) >> 4);
	u2HActive = ptNrPrm->u2FrameWidth;	/* ptNrPrm->u2PicWidth; */
	u2VActive = ptNrPrm->u2PicHeight;	/* ptNrPrm->u2FrameHeight; */

	/* output buffer width */
	vNRWriteRegMsk(reg_base + RW_NR_DRAM_CTRL_00, (u4MBCnt << 24), 0xFF000000);

	/* NR target frame width and height */
	u2HActive = (u2HActive / 2) << 1;
	if (ptNrPrm->u1FrameMode == MODE_FRAME) {
		u4Value = u2VActive;
		u4Value = u4Value << 16;
		u4Value = u4Value | u2HActive;
		vNRWriteRegMsk(reg_base + RW_NR_DRAM_CTRL_01, u4Value, 0x07FF0FFF);
	} else {
		u4Value = (u2VActive / 2);
		u4Value = u4Value << 16;
		u4Value = u4Value | u2HActive;
		vNRWriteRegMsk(reg_base + RW_NR_DRAM_CTRL_01, u4Value, 0x07FF0FFF);
		vNRWriteRegMsk(reg_base + RW_NR_LAST_SIZE_CTRL, u4Value, 0x07FF0FFF);
	}

	if (ptNrPrm->u1DemoMode == NR_DEMO_0) {
		vNRWriteRegMsk(reg_base + RW_NR_3D_0D, (((ptNrPrm->u2PicWidth / 2) - 1) << 20),
			       C_LINE_MANUAL_LEN);
		vNRWriteRegMsk(reg_base + RW_NR_3D_00, (0x1 << 12) | ((ptNrPrm->u2PicWidth / 4) - 1),
			       C_LINE_HALF_WIDTH);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_3D_0D, (0x3FF << 20), C_LINE_MANUAL_LEN);	/* for 4096 width */
		vNRWriteRegMsk(reg_base + RW_NR_3D_00, 0xEF, C_LINE_HALF_WIDTH);
	}

	if (ptNrPrm->u1DemoMode == NR_DEMO_0) {
		vNRWriteRegMsk(reg_base + RW_NR_3D_0D, (0x2 << 30), (0x3 << 30));
		vNRWriteRegMsk(reg_base + RW_NR_3D_00, (0x1 << 12), (0x1 << 12));
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, (0x3 << 12),
			       SLICE_DEMO_CTRL | SLICE_DEMO_ENABLE);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_02, ((ptNrPrm->u2PicWidth / 2) << 16),
			       SLICE_X_POSITION);
	} else if (ptNrPrm->u1DemoMode == NR_DEMO_1) {
		vNRWriteRegMsk(reg_base + RW_NR_3D_0D, (0x3 << 30), (0x3 << 30));
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, (0x1 << 12),
			       SLICE_DEMO_CTRL | SLICE_DEMO_ENABLE);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_02, ((ptNrPrm->u2PicWidth / 2) << 16),
			       SLICE_X_POSITION);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_3D_0D, (0x3 << 30), (0x3 << 30));
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, (0x0 << 12),
			       SLICE_DEMO_CTRL | SLICE_DEMO_ENABLE);
	}


	/* Block meter setting */
	if (ptNrPrm->u1FrameMode == MODE_FRAME) {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_01, ptNrPrm->u2PicHeight, 0x7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_01, (ptNrPrm->u2PicWidth << 16), 0x7FF0000);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_01, (ptNrPrm->u2PicHeight / 2), 0x7FF);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_01, (ptNrPrm->u2PicWidth << 16), 0x7FF0000);
	}

	if (ptNrPrm->u1FrameMode == MODE_FRAME) {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_5B, ptNrPrm->u2PicHeight, BK_METER_HEIGHT);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_5B, ((ptNrPrm->u2PicWidth / 2) << 16),
			       BK_METER_WIDTH);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_95,
			       ((((ptNrPrm->u2PicHeight / 8) / 2) / 6) * 2) << 10, 0x000FFC00);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_97,
			       (((((ptNrPrm->u2PicWidth / 8) / 2) / 6) * 2) << 11), 0x003FF800);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_96,
			       (((ptNrPrm->u2PicHeight + 31) / (8 * 8)) * 2) << 11, 0x0003FF800);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_94, ((ptNrPrm->u2PicWidth / (64 * 6)) * 3) << 15,
			       0x3FFF8000);
	} else {
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_5B, ptNrPrm->u2PicHeight / 2, BK_METER_HEIGHT);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_5B, ((ptNrPrm->u2PicWidth / 2) << 16),
			       BK_METER_WIDTH);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_95,
			       ((((ptNrPrm->u2PicHeight / 8) / 4) / 6) * 2) << 10, 0x000FFC00);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_97,
			       (((((ptNrPrm->u2PicWidth / 8) / 2) / 6) * 2) << 11), 0x003FF800);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_96,
			       (((ptNrPrm->u2PicHeight + 63) / (8 * 8 * 2)) * 2) << 11,
			       0x0003FF800);
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_94, ((ptNrPrm->u2PicWidth / (64 * 6)) * 3) << 15,
			       0x3FFF8000);
	}

	if (ptNrPrm->fgBypassEn)
		vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000000, 0x00000001);	/* turn off NR kernel process */
	else {
#if 1
		nr_hal_adaptive_bnr_para(reg_base, ptNrPrm);
		nr_hal_set_level(reg_base, ptNrPrm->u4Strength, ptNrPrm->u4MNRStrength, ptNrPrm->u4BNRStrength);

#else
		if (ptNrPrm->u4Strength == NR_STRENGTH_OFF) {
			if (ptNrPrm->u4MNRStrength == NR_STRENGTH_OFF) {
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 4), BLDLV_MOS_CO1);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 8), BLDLV_MOS_CO2);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 12), BLDLV_MOS_CO3);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 20), BLDLV_MOS_ST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 24), BLDLV_MOS_MO);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x0 << 0), BLDLV_NEAR_FRST);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 16), BLDLV_MOS_DEF);
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x0 << 28), BLDLV_MOS_BK);
			} else {
				switch (ptNrPrm->u4MNRStrength) {
				case NR_STRENGTH_LOW:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 4),
						       BLDLV_MOS_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 8),
						       BLDLV_MOS_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 12),
						       BLDLV_MOS_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 20),
						       BLDLV_MOS_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 24),
						       BLDLV_MOS_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x4 << 0),
						       BLDLV_NEAR_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 16),
						       BLDLV_MOS_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x4 << 28),
						       BLDLV_MOS_BK);
					break;
				case NR_STRENGTH_MED:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 4),
						       BLDLV_MOS_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 8),
						       BLDLV_MOS_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 12),
						       BLDLV_MOS_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 20),
						       BLDLV_MOS_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 24),
						       BLDLV_MOS_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x6 << 0),
						       BLDLV_NEAR_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 16),
						       BLDLV_MOS_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x6 << 28),
						       BLDLV_MOS_BK);
					break;
				case NR_STRENGTH_HIGH:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 4),
						       BLDLV_MOS_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 8),
						       BLDLV_MOS_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 12),
						       BLDLV_MOS_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 20),
						       BLDLV_MOS_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 24),
						       BLDLV_MOS_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_84, (0x8 << 0),
						       BLDLV_NEAR_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 16),
						       BLDLV_MOS_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_83, (0x8 << 28),
						       BLDLV_MOS_BK);
					break;
				default:
					break;
				}
			}
			if (ptNrPrm->u4BNRStrength == NR_STRENGTH_OFF) {
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000000, BLOCK_PROC_ENABLE);
			} else {
				vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000080, BLOCK_PROC_ENABLE);
				switch (ptNrPrm->u4BNRStrength) {
				case NR_STRENGTH_LOW:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 12),
						       BLDLV_BK_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 28),
						       BLDLV_BK_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x4 << 12),
						       BLDLV_BK_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 28),
						       BLDLV_BK_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 12),
						       BLDLV_BK_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x4 << 28),
						       BLDLV_BK_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x4 << 12),
						       BLDLV_BK_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x4 << 28),
						       BLDLV_BK_BK);
					break;
				case NR_STRENGTH_MED:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 12),
						       BLDLV_BK_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 28),
						       BLDLV_BK_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x6 << 12),
						       BLDLV_BK_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 28),
						       BLDLV_BK_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 12),
						       BLDLV_BK_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x6 << 28),
						       BLDLV_BK_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x6 << 12),
						       BLDLV_BK_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x6 << 28),
						       BLDLV_BK_BK);
					break;
				case NR_STRENGTH_HIGH:
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 12),
						       BLDLV_BK_CO1);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 28),
						       BLDLV_BK_CO2);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_81, (0x8 << 12),
						       BLDLV_BK_CO3);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 28),
						       BLDLV_BK_ST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 12),
						       BLDLV_BK_MO);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_80, (0x8 << 28),
						       BLDLV_BK_FRST);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7E, (0x8 << 12),
						       BLDLV_BK_DEF);
					vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_7F, (0x8 << 28),
						       BLDLV_BK_BK);
					break;
				default:
					break;
				}
			}
		} else {
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000080, BLOCK_PROC_ENABLE);
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x00000001, 0x00000001);
		}
#endif
	}

	/* enable NR */
	if (ptNrPrm->u4NRMode == NR_STD_MODE) {
		if (ptNrPrm->fgUseBlockMeter)
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x100, BLOCK_METER_ENABLE);
		else
			vNRWriteRegMsk(reg_base + RW_NR_2DNR_CTRL_1F, 0x000, BLOCK_METER_ENABLE);

		vNrWriteReg(reg_base + RW_NR_HD_PATH_ENABLE, 0x00000001);
	}
}

void nr_hal_switch_mode(void __iomem *reg_base, enum VQ_PATH_MODE vq_mode)
{
	if (vq_mode == VQ_NR_STANDALONE) {
		vNRWriteRegMsk(reg_base + 0x0C, 0 << 28, 1 << 28);
		vNRWriteRegMsk(reg_base + 0x1C, 0x0, 0x4);
	} else {
		vNRWriteRegMsk(reg_base + 0x0C, 1 << 28, 1 << 28);
		vNRWriteRegMsk(reg_base + 0x1C, 0x4, 0x4);
	}
}

int nr_hal_set_info(void __iomem *reg_base, struct mtk_vq_config_info *config_info)
{
	int ret = 0;

	struct nr_config_param nr_param;
	struct mtk_vq_config *config = config_info->vq_config;

	memset(&nr_param, 0, sizeof(nr_param));

	if (config->vq_mode == VQ_DI_NR_DIRECTLINK_NR_BYPASS)
		nr_param.fgBypassEn = true;

	if (config->vq_mode == VQ_NR_STANDALONE)
		nr_param.u4NRMode = NR_STD_MODE;
	else
		nr_param.u4NRMode = NR_NSTD_2D_MODE;

	nr_param.u2PicWidth = config->src_width;
	nr_param.u2PicHeight = config->src_height;
	nr_param.u2FrameWidth = config->src_align_width;
	nr_param.u2FrameHeight = config->src_align_height;

	nr_param.u4BNRStrength = config->bnr_level;
	nr_param.u4MNRStrength = config->mnr_level;

	if (config->vq_mode == VQ_NR_STANDALONE) {
		nr_param.u4CurrRdYAddr = config_info->src_mva[0] + config->src_ofset_y_len[0];
		nr_param.u4CurrRdCAddr = config_info->src_mva[0] + config->src_ofset_c_len[0];

		nr_param.u4CurrWrYAddr = config_info->dst_mva + config->dst_ofset_y_len;
		nr_param.u4CurrWrCAddr = config_info->dst_mva + config->dst_ofset_c_len;
	}

	nr_hal_config(reg_base, &nr_param);

	return ret;
}

int nr_hal_power_on(struct nr_info *nr)
{
	int ret = 0;
	int i;

	for (i = 0; i < MTK_NR_CLK_CNT - 1; i++) {
		ret = clk_prepare_enable(nr->clks[i]);
		if (ret) {
			pr_info("[NR] fail to enable clk %s\n", _ap_mtk_nr_clk_name[i]);
			for (i -= 1; i >= 0; i--)
				clk_disable_unprepare(nr->clks[i]);
			break;
		}
	}
	clk_set_parent(nr->clks[CLK_TOP_NR_SEL], nr->clks[CLK_TOP_UNIVPLL2_D2]);

	return ret;
}

int nr_hal_power_off(struct nr_info *nr)
{
	int i;

	for (i = 0; i < MTK_NR_CLK_CNT - 1; i++)
		clk_disable_unprepare(nr->clks[i]);

	return 0;
}
