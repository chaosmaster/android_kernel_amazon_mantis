/*
 * Copyright (C) 2015 MediaTek Inc.
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

#ifndef __MT_EMI_H
#define __MT_EMI_H

#ifdef CONFIG_MTK_EMI_BWL
#define EMI_MDCT        (EMI_BASE + 0x0078)
#define EMI_IOCL        (EMI_BASE + 0x00D0)
#define EMI_IOCL_2ND    (EMI_BASE + 0x00D4)
#define EMI_IOCM        (EMI_BASE + 0x00D8)
#define EMI_IOCM_2ND    (EMI_BASE + 0x00DC)
#define EMI_TESTB       (EMI_BASE + 0x00E8)
#define EMI_ARBC        (EMI_BASE + 0x0110)
#define EMI_ARBD        (EMI_BASE + 0x0118)
#define EMI_ARBF        (EMI_BASE + 0x0128)
#define EMI_ARBG        (EMI_BASE + 0x0130)
#define EMI_ARBH        (EMI_BASE + 0x0138)
#define EMI_SLCT        (EMI_BASE + 0x0158)
#define EMI_SHF0        (EMI_BASE + 0x0710)
#define EMI_SLVA        (EMI_BASE + 0x0800)

#define EMI_CHA_SLCT    (EMI_CHA_BASE + 0x0158)
#define EMI_CHB_SLCT    (EMI_CHB_BASE + 0x0158)
#endif

#ifdef CONFIG_MTK_EMI_MPU
#define EMI_MPU_SA0 ((EMI_MPU_BASE + 0x0100))
#define EMI_MPU_SA1 ((EMI_MPU_BASE + 0x0104))
#define EMI_MPU_SA2 ((EMI_MPU_BASE + 0x0108))
#define EMI_MPU_SA3 ((EMI_MPU_BASE + 0x010C))
#define EMI_MPU_SA4 ((EMI_MPU_BASE + 0x0110))
#define EMI_MPU_SA5 ((EMI_MPU_BASE + 0x0114))
#define EMI_MPU_SA6 ((EMI_MPU_BASE + 0x0118))
#define EMI_MPU_SA7 ((EMI_MPU_BASE + 0x011C))

#define EMI_MPU_EA0 ((EMI_MPU_BASE + 0x0200))
#define EMI_MPU_EA1 ((EMI_MPU_BASE + 0x0204))
#define EMI_MPU_EA2 ((EMI_MPU_BASE + 0x0208))
#define EMI_MPU_EA3 ((EMI_MPU_BASE + 0x020C))
#define EMI_MPU_EA4 ((EMI_MPU_BASE + 0x0210))
#define EMI_MPU_EA5 ((EMI_MPU_BASE + 0x0214))
#define EMI_MPU_EA6 ((EMI_MPU_BASE + 0x0218))
#define EMI_MPU_EA7 ((EMI_MPU_BASE + 0x021C))

#define EMI_MPU_APC0 ((EMI_MPU_BASE + 0x0300))
#define EMI_MPU_APC1 ((EMI_MPU_BASE + 0x0304))
#define EMI_MPU_APC2 ((EMI_MPU_BASE + 0x0308))
#define EMI_MPU_APC3 ((EMI_MPU_BASE + 0x030C))
#define EMI_MPU_APC4 ((EMI_MPU_BASE + 0x0310))
#define EMI_MPU_APC5 ((EMI_MPU_BASE + 0x0314))
#define EMI_MPU_APC6 ((EMI_MPU_BASE + 0x0318))
#define EMI_MPU_APC7 ((EMI_MPU_BASE + 0x031C))
#endif

#define NO_PROTECTION 0
#define SEC_RW 1
#define SEC_RW_NSEC_R 2
#define NSEC_RW 3
#define SEC_R_NSEC_R 4
#define FORBIDDEN 5
#define SEC_R_NSEC_RW 6
#define SEC_R 6

#define EN_MPU_STR "ON"
#define DIS_MPU_STR "OFF"

/* define concurrency scenario ID */
enum {
#ifdef CONFIG_MTK_EMI_8695
#define X_CON_SCE(con_sce, mdct, iocl, iocl_2nd, iocm, iocm_2nd, arbc, arbd, arbf, arbg, \
	arbh, slct, shfo, slva, testb, cha_slct, chb_slct) con_sce,
#include "con_sce_lpddr4_8695.h"
#else
#define X_CON_SCE(con_sce, arba, arbb, arbc, arbd, arbe) con_sce,
#include "con_sce_ddr3_16.h"
#endif
#undef X_CON_SCE
	NR_CON_SCE
};

#define MAX_EMI_MPU_STORE_CMD_LEN 128

/* define control operation */
enum {
	ENABLE_CON_SCE = 0,
	DISABLE_CON_SCE = 1
};

#define EN_CON_SCE_STR "ON"
#define DIS_CON_SCE_STR "OFF"

/*
 * Define data structures.
 */

/* define control table entry */
struct emi_bwl_ctrl {
	unsigned int ref_cnt;
};

/* define control operation */
enum DDRTYPE {
	TYPE_LPDDR3 = 1,
	TYPE_LPDDR4,
	TYPE_LPDDR4X,
	TYPE_LPDDR2
};


/*
 * Define function prototype.
 */
extern int mtk_mem_bw_ctrl(int sce, int op);

#define SET_ACCESS_PERMISSON(d3, d2, d1, d0)\
		(((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0))

extern int emi_mpu_set_region_protection(unsigned int start_addr,
									unsigned int end_addr,
									int region, unsigned int access_permission);

#endif  /* !__MT_EMI_MPU_H */
