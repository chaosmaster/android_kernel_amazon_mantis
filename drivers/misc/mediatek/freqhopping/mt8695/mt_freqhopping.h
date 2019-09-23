/*
 * Copyright (C) 2011 MediaTek, Inc.
 *
 * Author: Holmes Chiou <holmes.chiou@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MT_FREQHOPPING_H__
#define __MT_FREQHOPPING_H__

#define FHTAG "[FH] "

#define FH_MSG_ERROR(fmt, args...)	pr_err(FHTAG fmt, ##args)
#define FH_MSG_NOTICE(fmt, args...)	pr_notice(FHTAG fmt, ##args)
#define FH_MSG_INFO(fmt, args...)	/* pr_info(FHTAG fmt, ##args) */

#define FH_ARM_PLLID		0u
#define FH_MM_PLLID		1u
#define FH_MAIN_PLLID		2u
#define FH_VDEC_PLLID		3u
#define FH_MSDC_PLLID		4u
#define FH_TVD_PLLID		5u
#define FH_MEM_PLLID		6u
#define FH_MAX_PLLID           6u
#define FH_PLL_NUM		7u

struct freqhopping_ssc {
	unsigned int freq;
	unsigned int dt;
	unsigned int df;
	unsigned int upbnd;
	unsigned int lowbnd;
	unsigned int dds;
};

extern int mtk_fhctl_enable_ssc_by_id(unsigned int fh_pll_id);
extern int mtk_fhctl_disable_ssc_by_id(unsigned int fh_pll_id);
extern int mtk_fhctl_hopping_by_id(unsigned int fh_pll_id, unsigned int target_vco_frequency);

#endif				/* !__MT_FREQHOPPING_H__ */
