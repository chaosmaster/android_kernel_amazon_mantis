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

#ifndef _NR_HAL_H_
#define _NR_HAL_H_

#include "nr_hw.h"
#include "mtk_vq_info.h"

#define MTK_NR_CLK_CNT 3

enum NR_HW_CLK {
	CLK_DISP_NR,
	CLK_TOP_NR_SEL,
	CLK_TOP_UNIVPLL2_D2,
};

#define NR_DEMO_0               1
#define NR_DEMO_1               2

#define MODE_FRAME              0
#define MODE_FIELD              1
#define MODE_TOP_FIELD          2
#define MODE_BOT_FIELD          3

#define MT8520_SWAP_MODE_0      0
#define MT8520_SWAP_MODE_1      1
#define MT8520_SWAP_MODE_2      2
#define MT5351_SWAP_MODE_0      3
#define MT5351_SWAP_MODE_1      4
#define MT5351_SWAP_MODE_2      5
#define MT5351_SWAP_MODE_3      6

#define NR_STRENGTH_OFF         0
#define NR_STRENGTH_LOW         1
#define NR_STRENGTH_MED         2
#define NR_STRENGTH_HIGH        3
#define NR_STRENGTH_LV4         4
#define NR_STRENGTH_LV5         5
#define NR_STRENGTH_LV6         6
#define NR_STRENGTH_LV7         7
#define NR_STRENGTH_LV8         8

enum NR_MODE {
	NR_STD_MODE = 0,
	NR_NSTD_2D_MODE
};

struct nr_config_param {
	bool fgBypassEn;
	bool fgBurstRdEn;
	bool fgRangeRemapYEn;
	bool fgRangeRemapUVEn;
	bool fgUseBlockMeter;

	unsigned char u1AddrSwapMode;
	unsigned char u1FrameMode;
	unsigned char u1DemoMode;

	unsigned short u2PicWidth;
	unsigned short u2PicHeight;
	unsigned short u2FrameWidth;
	unsigned short u2FrameHeight;

	enum NR_MODE u4NRMode;

	unsigned int u4MNRStrength;
	unsigned int u4BNRStrength;
	unsigned int u4RangeMapY;
	unsigned int u4RangeMapUV;
	unsigned int u4CurrRdYAddr;
	unsigned int u4CurrRdCAddr;

	unsigned int u4Strength;
	unsigned int u4FNRStrength;
	unsigned int u4CurrWrYAddr;
	unsigned int u4CurrWrCAddr;
};

enum NR_QUALITY_ITEM {
	QUALITY_SNR_MESSSFT_SM_CO1MO,
	QUALITY_SNR_MESSTHL_SM_CO1MO,
	QUALITY_SNR_MESSSFT_MESS_CO1MO,
	QUALITY_SNR_MESSTHL_MESS_CO1MO,
	QUALITY_SNR_MESSSFT_EDGE_CO1MO,
	QUALITY_SNR_MESSTHL_EDGE_CO1MO,
	QUALITY_SNR_MESSSFT_MOS_CO1MO,
	QUALITY_SNR_MESSTHL_MOS_CO1MO,
	QUALITY_SNR_MESSSFT_SM_CO1ST,
	QUALITY_SNR_MESSTHL_SM_CO1ST,
	QUALITY_SNR_MESSSFT_MESS_CO1ST,
	QUALITY_SNR_MESSTHL_MESS_CO1ST,
	QUALITY_SNR_MESSSFT_EDGE_CO1ST,
	QUALITY_SNR_MESSTHL_EDGE_CO1ST,
	QUALITY_SNR_MESSSFT_MOS_CO1ST,
	QUALITY_SNR_MESSTHL_MOS_CO1ST,
	QUALITY_SNR_BLDLV_BK_CO1,
	QUALITY_SNR_BLDLV_SM_CO1,
	QUALITY_SNR_BLDLV_MESS_CO1,
	QUALITY_SNR_BLDLV_EDGE_CO1,
	QUALITY_SNR_BLDLV_MOS_CO1,
	QUALITY_SNR_MESSSFT_SM_CO2MO,
	QUALITY_SNR_MESSTHL_SM_CO2MO,
	QUALITY_SNR_MESSSFT_MESS_CO2MO,
	QUALITY_SNR_MESSTHL_MESS_CO2MO,
	QUALITY_SNR_MESSSFT_EDGE_CO2MO,
	QUALITY_SNR_MESSTHL_EDGE_CO2MO,
	QUALITY_SNR_MESSSFT_MOS_CO2MO,
	QUALITY_SNR_MESSTHL_MOS_CO2MO,
	QUALITY_SNR_MESSSFT_SM_CO2ST,
	QUALITY_SNR_MESSTHL_SM_CO2ST,
	QUALITY_SNR_MESSSFT_MESS_CO2ST,
	QUALITY_SNR_MESSTHL_MESS_CO2ST,
	QUALITY_SNR_MESSSFT_EDGE_CO2ST,
	QUALITY_SNR_MESSTHL_EDGE_CO2ST,
	QUALITY_SNR_MESSSFT_MOS_CO2ST,
	QUALITY_SNR_MESSTHL_MOS_CO2ST,
	QUALITY_SNR_BLDLV_BK_CO2,
	QUALITY_SNR_BLDLV_SM_CO2,
	QUALITY_SNR_BLDLV_MESS_CO2,
	QUALITY_SNR_BLDLV_EDGE_CO2,
	QUALITY_SNR_BLDLV_MOS_CO2,
	QUALITY_SNR_MESSSFT_SM_CO3MO,
	QUALITY_SNR_MESSTHL_SM_CO3MO,
	QUALITY_SNR_MESSSFT_MESS_CO3MO,
	QUALITY_SNR_MESSTHL_MESS_CO3MO,
	QUALITY_SNR_MESSSFT_EDGE_CO3MO,
	QUALITY_SNR_MESSTHL_EDGE_CO3MO,
	QUALITY_SNR_MESSSFT_MOS_CO3MO,
	QUALITY_SNR_MESSTHL_MOS_CO3MO,
	QUALITY_SNR_MESSSFT_SM_CO3ST,
	QUALITY_SNR_MESSTHL_SM_CO3ST,
	QUALITY_SNR_MESSSFT_MESS_CO3ST,
	QUALITY_SNR_MESSTHL_MESS_CO3ST,
	QUALITY_SNR_MESSSFT_EDGE_CO3ST,
	QUALITY_SNR_MESSTHL_EDGE_CO3ST,
	QUALITY_SNR_MESSSFT_MOS_CO3ST,
	QUALITY_SNR_MESSTHL_MOS_CO3ST,
	QUALITY_SNR_BLDLV_BK_CO3,
	QUALITY_SNR_BLDLV_SM_CO3,
	QUALITY_SNR_BLDLV_MESS_CO3,
	QUALITY_SNR_BLDLV_EDGE_CO3,
	QUALITY_SNR_BLDLV_MOS_CO3,
	QUALITY_SNR_MESSSFT_SM_FRST,
	QUALITY_SNR_MESSTHL_SM_FRST,
	QUALITY_SNR_MESSSFT_MESS_FRST,
	QUALITY_SNR_MESSTHL_MESS_FRST,
	QUALITY_SNR_MESSSFT_EDGE_FRST,
	QUALITY_SNR_MESSTHL_EDGE_FRST,
	QUALITY_SNR_MESSSFT_MOS_FRST,
	QUALITY_SNR_MESSTHL_MOS_FRST,
	QUALITY_SNR_BLDLV_BK_FRST,
	QUALITY_SNR_BLDLV_SM_FRST,
	QUALITY_SNR_BLDLV_MESS_FRST,
	QUALITY_SNR_BLDLV_EDGE_FRST,
	QUALITY_SNR_BLDLV_MOS_FRST,
	QUALITY_SNR_MESSSFT_SM_MO,
	QUALITY_SNR_MESSTHL_SM_MO,
	QUALITY_SNR_MESSSFT_MESS_MO,
	QUALITY_SNR_MESSTHL_MESS_MO,
	QUALITY_SNR_MESSSFT_EDGE_MO,
	QUALITY_SNR_MESSTHL_EDGE_MO,
	QUALITY_SNR_MESSSFT_MOS_MO,
	QUALITY_SNR_MESSTHL_MOS_MO,
	QUALITY_SNR_BLDLV_BK_MO,
	QUALITY_SNR_BLDLV_SM_MO,
	QUALITY_SNR_BLDLV_MESS_MO,
	QUALITY_SNR_BLDLV_EDGE_MO,
	QUALITY_SNR_BLDLV_MOS_MO,
	QUALITY_SNR_MESSSFT_SM_ST,
	QUALITY_SNR_MESSTHL_SM_ST,
	QUALITY_SNR_MESSSFT_MESS_ST,
	QUALITY_SNR_MESSTHL_MESS_ST,
	QUALITY_SNR_MESSSFT_EDGE_ST,
	QUALITY_SNR_MESSTHL_EDGE_ST,
	QUALITY_SNR_MESSSFT_MOS_ST,
	QUALITY_SNR_MESSTHL_MOS_ST,
	QUALITY_SNR_BLDLV_BK_ST,
	QUALITY_SNR_BLDLV_SM_ST,
	QUALITY_SNR_BLDLV_MESS_ST,
	QUALITY_SNR_BLDLV_EDGE_ST,
	QUALITY_SNR_BLDLV_MOS_ST,
	QUALITY_SNR_MESSSFT_SM_BK,
	QUALITY_SNR_MESSTHL_SM_BK,
	QUALITY_SNR_MESSSFT_MESS_BK,
	QUALITY_SNR_MESSTHL_MESS_BK,
	QUALITY_SNR_MESSSFT_EDGE_BK,
	QUALITY_SNR_MESSTHL_EDGE_BK,
	QUALITY_SNR_MESSSFT_MOS_BK,
	QUALITY_SNR_MESSTHL_MOS_BK,
	QUALITY_SNR_BLDLV_BK_BK,
	QUALITY_SNR_BLDLV_SM_BK,
	QUALITY_SNR_BLDLV_MESS_BK,
	QUALITY_SNR_BLDLV_EDGE_BK,
	QUALITY_SNR_BLDLV_MOS_BK,
	QUALITY_SNR_MESSSFT_SM_DEF,
	QUALITY_SNR_MESSTHL_SM_DEF,
	QUALITY_SNR_MESSSFT_MESS_DEF,
	QUALITY_SNR_MESSTHL_MESS_DEF,
	QUALITY_SNR_MESSSFT_EDGE_DEF,
	QUALITY_SNR_MESSTHL_EDGE_DEF,
	QUALITY_SNR_MESSSFT_MOS_DEF,
	QUALITY_SNR_MESSTHL_MOS_DEF,
	QUALITY_SNR_BLDLV_SM_DEF,
	QUALITY_SNR_BLDLV_MESS_DEF,
	QUALITY_SNR_BLDLV_EDGE_DEF,
	QUALITY_SNR_BLDLV_MOS_DEF,
	QUALITY_SNR_CUR_SM_NUM,
	QUALITY_SNR_CUR_SM_THR,
	QUALITY_SNR_NEAREDGE_SELWIDTH,
	QUALITY_SNR_NEAREDGE_EDGE_THR,
	QUALITY_SNR_GLOBAL_BLEND,
	QUALITY_NR_MAX
};

struct NR_QUALITY_INST_T {
	unsigned char u1BNRLevel;
	unsigned char u1MNRLevel;
	unsigned char u1SNRLevel[7];
	unsigned char u1Tone1YMax;
	unsigned char u1Tone1YMin;
	unsigned char u1Tone1UMax;
	unsigned char u1Tone1UMin;
	unsigned char u1Tone1VMax;
	unsigned char u1Tone1VMin;
	unsigned char u1Tone2YMax;
	unsigned char u1Tone2YMin;
	unsigned char u1Tone2UMax;
	unsigned char u1Tone2UMin;
	unsigned char u1Tone2VMax;
	unsigned char u1Tone2VMin;
	unsigned char u1Tone3YMax;
	unsigned char u1Tone3YMin;
	unsigned char u1Tone3UMax;
	unsigned char u1Tone3UMin;
	unsigned char u1Tone3VMax;
	unsigned char u1Tone3VMin;
	unsigned char u1MNRThd;
	unsigned short u2SNRSMThd[7];
	unsigned short u2SNREdgeThd[7];
	unsigned short u2FilterType[6];
	unsigned char u1BNRHBlockThd;
	unsigned short u2BNRHEdgeThd;
	unsigned char u1BNRVBlockThd;
	unsigned short u2BNRVEdgeThd;
	unsigned char u1SNRStillThd;
	unsigned char u1SNRMotionThd;
	unsigned short u2SNRFrmStillThd;
	unsigned char u1FNRStillThd;
	unsigned char u1FNRMotionThd;
	unsigned short u2FNRFrmStillThd;
	unsigned char u1ChainThd[8];
	unsigned int u4ChainTableGain[8];
};

struct nr_info {
	void __iomem *nr_reg_base;
	int irq;
	struct clk *clks[MTK_NR_CLK_CNT];
	struct device *dev;

	wait_queue_head_t wait_nr_irq_wq;
	atomic_t wait_nr_irq_flag;
};

#define vNrWriteReg(dAddr, dVal)  (*(volatile unsigned int *)(dAddr) = (dVal))
#define vNrReadReg(dAddr)         (*(volatile unsigned int *)(dAddr))
#define vNRWriteRegMsk(dAddr, dVal, dMsk) vNrWriteReg((dAddr), (vNrReadReg(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))

int nr_hal_hw_init(struct platform_device *pdev, struct nr_info *nr);
int nr_hal_wait_complete_timeout(struct nr_info *nr, unsigned int time);
int nr_hal_hw_reset(void __iomem *reg_base);
int nr_hal_set_info(void __iomem *reg_base, struct mtk_vq_config_info *config_info);
int nr_hal_power_on(struct nr_info *nr);
int nr_hal_power_off(struct nr_info *nr);
int nr_hal_request_irq(struct nr_info *nr);
void nr_hal_free_irq(struct nr_info *nr);
void nr_hal_switch_mode(void __iomem *reg_base, enum VQ_PATH_MODE vq_mode);

#endif				/* _NR_HAL_H_ */
