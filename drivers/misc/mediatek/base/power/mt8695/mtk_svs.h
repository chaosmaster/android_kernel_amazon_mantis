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

#ifndef _MT_SVS_
#define _MT_SVS_

#include <linux/kernel.h>
#include <sync_write.h>

#define EN_SVS_OD (1)

extern void __iomem *svs_base;
#define SVS_BASEADDR svs_base

#ifdef CONFIG_OF
struct devinfo_svs_tag {
	u32 size;
	u32 tag;
	u32 volt0;
	u32 volt1;
	u32 volt2;
	u32 have_550;
};
#endif

/* SVS Register Definition */
#define SVS_DESCHAR         (SVS_BASEADDR + 0x0C00)
#define SVS_TEMPCHAR        (SVS_BASEADDR + 0x0C04)
#define SVS_DETCHAR         (SVS_BASEADDR + 0x0C08)
#define SVS_AGECHAR         (SVS_BASEADDR + 0x0C0C)
#define SVS_DCCONFIG        (SVS_BASEADDR + 0x0C10)
#define SVS_AGECONFIG       (SVS_BASEADDR + 0x0C14)
#define SVS_FREQPCT30       (SVS_BASEADDR + 0x0C18)
#define SVS_FREQPCT74       (SVS_BASEADDR + 0x0C1C)
#define SVS_LIMITVALS       (SVS_BASEADDR + 0x0C20)
#define SVS_VBOOT           (SVS_BASEADDR + 0x0C24)
#define SVS_DETWINDOW       (SVS_BASEADDR + 0x0C28)
#define SVS_SVSCONFIG       (SVS_BASEADDR + 0x0C2C)
#define SVS_TSCALCS         (SVS_BASEADDR + 0x0C30)
#define SVS_RUNCONFIG       (SVS_BASEADDR + 0x0C34)
#define SVS_SVSEN           (SVS_BASEADDR + 0x0C38)
#define SVS_INIT2VALS       (SVS_BASEADDR + 0x0C3C)
#define SVS_DCVALUES        (SVS_BASEADDR + 0x0C40)
#define SVS_AGEVALUES       (SVS_BASEADDR + 0x0C44)
#define SVS_VOP30           (SVS_BASEADDR + 0x0C48)
#define SVS_VOP74           (SVS_BASEADDR + 0x0C4C)
#define SVS_TEMP            (SVS_BASEADDR + 0x0C50)
#define SVS_SVSINTSTS       (SVS_BASEADDR + 0x0C54)
#define SVS_SVSINTSTSRAW    (SVS_BASEADDR + 0x0C58)
#define SVS_SVSINTEN        (SVS_BASEADDR + 0x0C5C)
#define SVS_SVSCHKINT       (SVS_BASEADDR + 0x0C60)
#define SVS_SVSCHKSHIFT     (SVS_BASEADDR + 0x0C64)
#define SVS_SVSSTATUS       (SVS_BASEADDR + 0x0C68)
#define SVS_VDESIGN30       (SVS_BASEADDR + 0x0C6C)
#define SVS_VDESIGN74       (SVS_BASEADDR + 0x0C70)
#define SVS_DVT30           (SVS_BASEADDR + 0x0C74)
#define SVS_DVT74           (SVS_BASEADDR + 0x0C78)
#define SVS_AGECOUNT        (SVS_BASEADDR + 0x0C7C)
#define SVS_SMSTATE0        (SVS_BASEADDR + 0x0C80)
#define SVS_SMSTATE1        (SVS_BASEADDR + 0x0C84)
#define SVS_SVSCTL0         (SVS_BASEADDR + 0x0C88)
#define SVS_SVSCTRLSPARE0   (SVS_BASEADDR + 0x0CF0)
#define SVS_SVSCTRLSPARE1   (SVS_BASEADDR + 0x0CF4)
#define SVS_SVSCTRLSPARE2   (SVS_BASEADDR + 0x0CF8)
#define SVS_SVSCTRLSPARE3   (SVS_BASEADDR + 0x0CFC)
#define SVS_SVSCORESEL      (SVS_BASEADDR + 0x0F00)
#define SVS_THERMINTST      (SVS_BASEADDR + 0x0F04)
#define SVS_SVSINTST        (SVS_BASEADDR + 0x0F08)
#define SVS_THSTAGE0ST      (SVS_BASEADDR + 0x0F0C)
#define SVS_THSTAGE1ST      (SVS_BASEADDR + 0x0F10)
#define SVS_THSTAGE2ST      (SVS_BASEADDR + 0x0F14)
#define SVS_THAHBST0        (SVS_BASEADDR + 0x0F18)
#define SVS_THAHBST1        (SVS_BASEADDR + 0x0F1C)
#define SVS_SVSSPARE0       (SVS_BASEADDR + 0x0F20)
#define SVS_SVSSPARE1       (SVS_BASEADDR + 0x0F24)
#define SVS_SVSSPARE2       (SVS_BASEADDR + 0x0F28)
#define SVS_SVSSPARE3       (SVS_BASEADDR + 0x0F2C)
#define SVS_THSLPEVEB       (SVS_BASEADDR + 0x0F30)

/* Thermal Register Definition */
#define THERMAL_BASE            svs_base

#define SVS_TEMPMONCTL0         (THERMAL_BASE + 0x0000)
#define SVS_TEMPMONCTL1         (THERMAL_BASE + 0x0004)
#define SVS_TEMPMONCTL2         (THERMAL_BASE + 0x0008)
#define SVS_TEMPMONINT          (THERMAL_BASE + 0x000C)
#define SVS_TEMPMONINTSTS       (THERMAL_BASE + 0x0010)
#define SVS_TEMPMONIDET0        (THERMAL_BASE + 0x0014)
#define SVS_TEMPMONIDET1        (THERMAL_BASE + 0x0018)
#define SVS_TEMPMONIDET2        (THERMAL_BASE + 0x001c)
#define SVS_TEMPH2NTHRE         (THERMAL_BASE + 0x0024)
#define SVS_TEMPHTHRE           (THERMAL_BASE + 0x0028)
#define SVS_TEMPCTHRE           (THERMAL_BASE + 0x002c)
#define SVS_TEMPOFFSETH         (THERMAL_BASE + 0x0030)
#define SVS_TEMPOFFSETL         (THERMAL_BASE + 0x0034)
#define SVS_TEMPMSRCTL0         (THERMAL_BASE + 0x0038)
#define SVS_TEMPMSRCTL1         (THERMAL_BASE + 0x003c)
#define SVS_TEMPAHBPOLL         (THERMAL_BASE + 0x0040)
#define SVS_TEMPAHBTO           (THERMAL_BASE + 0x0044)
#define SVS_TEMPADCPNP0         (THERMAL_BASE + 0x0048)
#define SVS_TEMPADCPNP1         (THERMAL_BASE + 0x004c)
#define SVS_TEMPADCPNP2         (THERMAL_BASE + 0x0050)
#define SVS_TEMPADCMUX          (THERMAL_BASE + 0x0054)
#define SVS_TEMPADCEXT          (THERMAL_BASE + 0x0058)
#define SVS_TEMPADCEXT1         (THERMAL_BASE + 0x005c)
#define SVS_TEMPADCEN           (THERMAL_BASE + 0x0060)
#define SVS_TEMPPNPMUXADDR      (THERMAL_BASE + 0x0064)
#define SVS_TEMPADCMUXADDR      (THERMAL_BASE + 0x0068)
#define SVS_TEMPADCEXTADDR      (THERMAL_BASE + 0x006c)
#define SVS_TEMPADCEXT1ADDR     (THERMAL_BASE + 0x0070)
#define SVS_TEMPADCENADDR       (THERMAL_BASE + 0x0074)
#define SVS_TEMPADCVALIDADDR    (THERMAL_BASE + 0x0078)
#define SVS_TEMPADCVOLTADDR     (THERMAL_BASE + 0x007c)
#define SVS_TEMPRDCTRL          (THERMAL_BASE + 0x0080)
#define SVS_TEMPADCVALIDMASK    (THERMAL_BASE + 0x0084)
#define SVS_TEMPADCVOLTAGESHIFT (THERMAL_BASE + 0x0088)
#define SVS_TEMPADCWRITECTRL    (THERMAL_BASE + 0x008c)
#define SVS_TEMPMSR0            (THERMAL_BASE + 0x0090)
#define SVS_TEMPMSR1            (THERMAL_BASE + 0x0094)
#define SVS_TEMPMSR2            (THERMAL_BASE + 0x0098)
#define SVS_TEMPADCHADDR        (THERMAL_BASE + 0x009C)
#define SVS_TEMPIMMD0           (THERMAL_BASE + 0x00A0)
#define SVS_TEMPIMMD1           (THERMAL_BASE + 0x00A4)
#define SVS_TEMPIMMD2           (THERMAL_BASE + 0x00A8)
#define SVS_TEMPMONIDET3        (THERMAL_BASE + 0x00B0)
#define SVS_TEMPADCPNP3         (THERMAL_BASE + 0x00B4)
#define SVS_TEMPMSR3            (THERMAL_BASE + 0x00B8)
#define SVS_TEMPIMMD3           (THERMAL_BASE + 0x00BC)
#define SVS_TEMPPROTCTL         (THERMAL_BASE + 0x00C0)
#define SVS_TEMPPROTTA          (THERMAL_BASE + 0x00C4)
#define SVS_TEMPPROTTB          (THERMAL_BASE + 0x00C8)
#define SVS_TEMPPROTTC          (THERMAL_BASE + 0x00CC)
#define SVS_TEMPSPARE0          (THERMAL_BASE + 0x00F0)
#define SVS_TEMPSPARE1          (THERMAL_BASE + 0x00F4)
#define SVS_TEMPSPARE2          (THERMAL_BASE + 0x00F8)
#define SVS_TEMPSPARE3          (THERMAL_BASE + 0x00FC)

/* SVS Structure */
struct SVS_INIT_T {
	unsigned int ADC_CALI_EN;
	unsigned int SVSINITEN;
	unsigned int SVSMONEN;
	unsigned int MDES;
	unsigned int BDES;
	unsigned int DCCONFIG;
	unsigned int DCMDET;
	unsigned int DCBDET;
	unsigned int AGECONFIG;
	unsigned int AGEM;
	unsigned int AGEDELTA;
	unsigned int DVTFIXED;
	unsigned int VCO;
	unsigned int MTDES;
	unsigned int MTS;
	unsigned int BTS;
	unsigned char FREQPCT0;
	unsigned char FREQPCT1;
	unsigned char FREQPCT2;
	unsigned char FREQPCT3;
	unsigned char FREQPCT4;
	unsigned char FREQPCT5;
	unsigned char FREQPCT6;
	unsigned char FREQPCT7;
	unsigned int DETWINDOW;
	unsigned int VMAX;
	unsigned int VMIN;
	unsigned int DTHI;
	unsigned int DTLO;
	unsigned int VBOOT;
	unsigned int DETMAX;
	unsigned int DCVOFFSETIN;
	unsigned int AGEVOFFSETIN;
};

typedef enum {
	SVS_CTRL_CPU = 0,
	NR_SVS_CTRL,
} svs_ctrl_id;

typedef enum {
	SVS_DET_CPU = SVS_CTRL_CPU,
	NR_SVS_DET,
} svs_det_id;


/* SVS Extern Function */
extern u32 get_devinfo_with_index(u32 index);
extern void mt_svs_lock(unsigned long *flags);
extern void mt_svs_unlock(unsigned long *flags);
extern int mt_svs_idle_can_enter(void);
extern int mt_svs_status(svs_det_id id);
extern int get_svs_status(void);
extern unsigned int get_vcore_svs_volt(int uv);
extern unsigned int is_have_550(void);
extern int is_svs_initialized_done(void);

#endif
