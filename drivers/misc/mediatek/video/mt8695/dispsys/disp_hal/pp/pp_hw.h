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


#ifndef _PP_HW_H_
#define _PP_HW_H_

#include "disp_reg.h"


/****************************************************************************
 * HW define
 ****************************************************************************/
extern uintptr_t pp_reg_base;
extern uintptr_t fmt_reg_base;
extern uintptr_t vdo_reg_base;
extern uintptr_t io_reg_base;

#define POST_PROC_BASE		(pp_reg_base)


/* field macros */
#define Fld(wid, shft, ac)	(((uint32_t)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)	    (uint8_t)((fld)>>16)
#define Fld_shft(fld)	    (uint8_t)((fld)>>8)
#define Fld_ac(fld)		    (uint8_t)(fld)

/* access method*/
#define	AC_FULLB0		1
#define	AC_FULLB1		2
#define	AC_FULLB2		3
#define	AC_FULLB3		4
#define	AC_FULLW10		5
#define	AC_FULLW21		6
#define	AC_FULLW32		7
#define	AC_FULLDW		8
#define	AC_MSKB0		11
#define	AC_MSKB1		12
#define	AC_MSKB2		13
#define	AC_MSKB3		14
#define	AC_MSKW10		15
#define	AC_MSKW21		16
#define	AC_MSKW32		17
#define	AC_MSKDW		18


/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
/* mask32 -> mask8 */
#define MSKB0(msk)	(uint8_t)(msk)
#define MSKB1(msk)	(uint8_t)((msk)>>8)
#define MSKB2(msk)	(uint8_t)((msk)>>16)
#define MSKB3(msk)	(uint8_t)((msk)>>24)
/* mask32 -> mask16 */
#define MSKW0(msk)	(uint16_t)(msk)
#define MSKW1(msk)	(uint16_t)((msk)>>8)
#define MSKW2(msk)	(uint16_t)((msk)>>16)
/* mask32 -> maskalign */
#define MSKAlignB(msk)	(((msk)&0xff)?(msk):(\
	((msk)&0xff00)?((msk)>>8):(((msk)&0xff0000)?((msk)>>16):((msk)>>24))))

/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
#define Fld2Msk32(fld)	(((uint32_t)0xffffffff>>(32-Fld_wid(fld)))<<Fld_shft(fld))
#define Fld2MskB0(fld)	MSKB0(Fld2Msk32(fld))
#define Fld2MskB1(fld)	MSKB1(Fld2Msk32(fld))
#define Fld2MskB2(fld)	MSKB2(Fld2Msk32(fld))
#define Fld2MskB3(fld)	MSKB3(Fld2Msk32(fld))
#define Fld2MskBX(fld, byte)	((uint8_t)(Fld2Msk32(fld)>>((byte&3)*8)))

#define Fld2MskW0(fld)	MSKW0(Fld2Msk32(fld))
#define Fld2MskW1(fld)	MSKW1(Fld2Msk32(fld))
#define Fld2MskW2(fld)	MSKW2(Fld2Msk32(fld))
#define Fld2MskWX(fld, byte)	((uint16_t)(Fld2Msk32(fld)>>((byte&3)*8)))


#define Fld2MskAlignB(fld)  MSKAlignB(Fld2Msk32(fld))
#define FldshftAlign(fld)	((Fld_shft(fld) < 8)?Fld_shft(fld):(\
			(Fld_shft(fld) < 16)?(Fld_shft(fld)-8):(\
			(Fld_shft(fld) < 24)?(Fld_shft(fld)-16):(Fld_shft(fld)-24)\
		)\
	))
#define ValAlign2Fld(val, fld)	((val)<<FldshftAlign(fld))


/* Reg READ/WRITE function, for 82xx & 53xx
 *    provide for VDOON(16bit,2002xxxx)register
 *    82xx only has 16 bit address space.
 */


/* #define u1RegRd1B(reg)              ((*(volatile uint32_t *)(IO_BASE+(reg&(~3))))>>(8*(reg&3)))&0xFF */
#define u4RegRd4B(reg)		ReadREG32((unsigned long)reg)
#define vRegWt4B(reg, val32)	WriteREG32((unsigned long)reg, val32)

uint8_t u1RegRd1B(uintptr_t reg);
uint16_t u2RegRd2B(uintptr_t reg);
void vRegWt1B(uintptr_t reg, uint8_t val8);
void vRegWt1BMsk(uintptr_t reg, uint8_t val8, uint8_t msk8);
void vRegWt2B(uintptr_t reg, uint16_t val16);
void vRegWt2BMsk(uintptr_t reg, uint16_t val16, uint16_t msk16);
void vRegWt4BMsk(uintptr_t reg, uint32_t val32, uint32_t msk32);

uint8_t u1IO32Rd1B(uintptr_t reg);
uint16_t u2IO32Rd2B(uintptr_t reg);
void vIO32Wt1B(uintptr_t reg, uint8_t val8);
void vIO32Wt1BMsk(uintptr_t reg, uint8_t val8, uint8_t msk8);
void vIO32Wt2B(uintptr_t reg, uint16_t val16);
void vIO32Wt2BMsk(uintptr_t reg, uint16_t val16, uint16_t msk16);
void vIO32Wt4BMsk(uintptr_t reg, uint32_t val32, uint32_t msk32);

/* some issue lint -e666 need to add outside */
#define	RegRdFld(reg, fld)	/*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld) >= AC_FULLB0) && (Fld_ac(fld) <= AC_FULLB3)) ? \
	u1RegRd1B((reg)+(Fld_ac(fld)-AC_FULLB0)) : \
	(((Fld_ac(fld) >= AC_FULLW10) && (Fld_ac(fld) <= AC_FULLW32)) ? \
	u2RegRd2B((reg)+(Fld_ac(fld)-AC_FULLW10)) : \
	((Fld_ac(fld) == AC_FULLDW) ? \
	u4RegRd4B(reg) : \
	(((Fld_ac(fld) >= AC_MSKB0) && (Fld_ac(fld) <= AC_MSKB3)) ? \
	(u1RegRd1B((reg)+(Fld_ac(fld)-AC_MSKB0)) & Fld2MskBX(fld, (Fld_ac(fld)-AC_MSKB0))) : \
	(((Fld_ac(fld) >= AC_MSKW10) && (Fld_ac(fld) <= AC_MSKW32)) ? \
	(u2RegRd2B((reg)+(Fld_ac(fld)-AC_MSKW10))&Fld2MskWX(fld, (Fld_ac(fld)-AC_MSKW10))) : \
	((Fld_ac(fld) == AC_MSKDW)?(u4RegRd4B(reg)&Fld2Msk32(fld)):0))))))	/*lint -restore */


#define	RegRdFldAlign(reg, fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld) >= AC_FULLB0) && (Fld_ac(fld) <= AC_FULLB3)) ? \
	u1RegRd1B((reg)+(Fld_ac(fld)-AC_FULLB0)) : \
	(((Fld_ac(fld) >= AC_FULLW10) && (Fld_ac(fld) <= AC_FULLW32)) ? \
	u2RegRd2B((reg)+(Fld_ac(fld)-AC_FULLW10)) : \
	((Fld_ac(fld) == AC_FULLDW) ? \
	u4RegRd4B(reg) : \
	(((Fld_ac(fld) >= AC_MSKB0) && (Fld_ac(fld) <= AC_MSKB3)) ? \
	((u1RegRd1B((reg)+(Fld_ac(fld)-AC_MSKB0)) & \
		Fld2MskBX(fld, (Fld_ac(fld)-AC_MSKB0)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKB0))&7)) : \
	(((Fld_ac(fld) >= AC_MSKW10) && (Fld_ac(fld) <= AC_MSKW32)) ? \
	((u2RegRd2B((reg)+(Fld_ac(fld)-AC_MSKW10)) & \
		Fld2MskWX(fld, (Fld_ac(fld)-AC_MSKW10)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKW10))&15)) : \
	((Fld_ac(fld) == AC_MSKDW) ? \
	((u4RegRd4B(reg)&Fld2Msk32(fld))>>Fld_shft(fld)):0))))))		/*lint -restore */

#define	vRegWtFld(reg, val, fld)  /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld) >= AC_FULLB0) && (Fld_ac(fld) <= AC_FULLB3)) ? \
	vRegWt1B((reg)+(Fld_ac(fld)-AC_FULLB0), (val)), 0 : \
	(((Fld_ac(fld) >= AC_FULLW10) && (Fld_ac(fld) <= AC_FULLW32)) ? \
	vRegWt2B((reg)+(Fld_ac(fld)-AC_FULLW10), (val)), 0 : \
	((Fld_ac(fld) == AC_FULLDW) ? \
	vRegWt4B((reg), (val)), 0 : \
	(((Fld_ac(fld) >= AC_MSKB0) && (Fld_ac(fld) <= AC_MSKB3)) ? \
	vRegWt1BMsk((reg)+(Fld_ac(fld)-AC_MSKB0), (val), Fld2MskBX(fld, (Fld_ac(fld)-AC_MSKB0))), 0 : \
	(((Fld_ac(fld) >= AC_MSKW10) && (Fld_ac(fld) <= AC_MSKW32)) ? \
	vRegWt2BMsk((reg)+(Fld_ac(fld)-AC_MSKW10), (val), Fld2MskWX(fld, (Fld_ac(fld)-AC_MSKW10))), 0 : \
	((Fld_ac(fld) == AC_MSKDW)?vRegWt4BMsk((reg), (val), Fld2Msk32(fld)), 0:0))))))	/*lint -restore */

#define	vRegWtFldAlign(reg, val, fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
	(((Fld_ac(fld) >= AC_FULLB0) && (Fld_ac(fld) <= AC_FULLB3)) ? \
	vRegWt1B((reg)+(Fld_ac(fld)-AC_FULLB0), (val)), 0 : \
	(((Fld_ac(fld) >= AC_FULLW10) && (Fld_ac(fld) <= AC_FULLW32)) ? \
	vRegWt2B((reg)+(Fld_ac(fld)-AC_FULLW10), (val)), 0 : \
	((Fld_ac(fld) == AC_FULLDW) ? \
	vRegWt4B((reg), (val)), 0 : \
	(((Fld_ac(fld) >= AC_MSKB0) && (Fld_ac(fld) <= AC_MSKB3)) ? \
	vRegWt1BMsk((reg)+(Fld_ac(fld)-AC_MSKB0), ValAlign2Fld((val), fld), \
		Fld2MskBX(fld, (Fld_ac(fld)-AC_MSKB0))), 0 : \
	(((Fld_ac(fld) >= AC_MSKW10) && (Fld_ac(fld) <= AC_MSKW32)) ? \
	vRegWt2BMsk((reg)+(Fld_ac(fld)-AC_MSKW10), ValAlign2Fld((val), fld), \
		Fld2MskWX(fld, (Fld_ac(fld)-AC_MSKW10))), 0 : \
	((Fld_ac(fld) == AC_MSKDW) ? \
	vRegWt4BMsk((reg), ((uint32_t)(val)<<Fld_shft(fld)), Fld2Msk32(fld)), 0:0))))))		/*lint -restore */


#define P_Fld(val, fld) ((sizeof(upk) > 1)?Fld2Msk32(fld) : \
						(((uint32_t)(val)&((1<<Fld_wid(fld))-1))<<Fld_shft(fld)))

#define   vRegWtFldMulti(reg, list) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
{ \
	uint16_t upk;\
	const uint32_t msk = (list); /*enum {msk=(INT32)(list)};*/\
	{uint8_t upk;\
	((uint32_t)msk == 0xff) ? vRegWt1B(reg, (list)), 0 : (\
	((uint32_t)msk == 0xff00) ? vRegWt1B(reg+1, (list)>>8), 0 : (\
	((uint32_t)msk == 0xff0000) ? vRegWt1B(reg+2, (list)>>16), 0 : (\
	((uint32_t)msk == 0xff000000) ? vRegWt1B(reg+3, (list)>>24), 0 : (\
	((uint32_t)msk == 0xffff) ? vRegWt2B(reg, (list)), 0 : (\
	((uint32_t)msk == 0xffff00) ? vRegWt2B(reg+1, (list)>>8), 0 : (\
	((uint32_t)msk == 0xffff0000) ? vRegWt2B(reg+2, (list)>>16), 0 : (\
	((uint32_t)msk == 0xffffffff) ? vRegWt4B(reg, (list)), 0 : (\
	(((uint32_t)msk&0xff) && (!((uint32_t)msk&0xffffff00))) ? \
	vRegWt1BMsk(reg, (list), (uint8_t)(uint32_t)msk), 0 : (\
	(((uint32_t)msk&0xff00) && (!((uint32_t)msk&0xffff00ff))) ? \
	vRegWt1BMsk(reg+1, (list)>>8, (uint8_t)((uint32_t)msk>>8)), 0 : (\
	(((uint32_t)msk&0xff0000) && (!((uint32_t)msk&0xff00ffff))) ? \
	vRegWt1BMsk(reg+2, (list)>>16, (uint8_t)((uint32_t)msk>>16)), 0 : (\
	(((uint32_t)msk&0xff000000) && (!((uint32_t)msk&0x00ffffff))) ? \
	vRegWt1BMsk(reg+3, (list)>>24, (uint8_t)((uint32_t)msk>>24)), 0 : (\
	(((uint32_t)msk&0xffff) && (!((uint32_t)msk&0xffff0000))) ? \
	vRegWt2BMsk(reg, (list), (uint16_t)(uint32_t)msk), 0 : (\
	(((uint32_t)msk&0xffff00) && (!((uint32_t)msk&0xff0000ff))) ? \
	vRegWt2BMsk(reg+1, (list)>>8, (uint16_t)((uint32_t)msk>>8)), 0 : (\
	(((uint32_t)msk&0xffff0000) && (!((uint32_t)msk&0x0000ffff))) ? \
	vRegWt2BMsk(reg+2, (list)>>16, (uint16_t)((uint32_t)msk>>16)), 0 : (\
	(uint32_t)(msk) ? vRegWt4BMsk(reg, (list), (uint32_t)msk), 0 : 0\
	)))))))))))))));\
	} \
}				/*lint -restore */

/*****************************************************************************
 *  FMT CTRL
 *****************************************************************************/
#define POSTHACT                        (fmt_reg_base+0x0)
#define POST_HACTBGN                    (0xFFF << 16)       /* Fld(12, 16, AC_MSKW32) //16:27 */
#define POST_HACTEND                    (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //0:11 */

#define POSTVOACT                       (fmt_reg_base+0x4)
#define POST_VOACTBGN                   (0xFFF << 16)       /* Fld(12, 16, AC_MSKW32) //16:27 */
#define POST_VOACTEND                   (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //0:11 */

#define POSTVEACT                       (fmt_reg_base+0x8)
#define POST_VEACTBGN                   (0xFFF << 16)       /* Fld(12, 16, AC_MSKW32) //16:27 */
#define POST_VEACTEND                   (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //0:11 */

#define POSTINCTRL                      (fmt_reg_base+0xC)
#define POST_H_DELAY_OPT2               (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) //30 */
#define POST_H_DELAY_OPT1               (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) //29 */
#define POST_HSYNC_DELAY_EN             (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) //28 */
#define POST_HSYNC_DELAY                (0xFFF << 16)       /* Fld(12, 16, AC_MSKW32) //16:27 */
#define POST_MIX_PLN2_BORDER_EN         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) // 7 */
#define POST_MIX_PLN1_BORDER_EN         (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) // 6 */
#define POST_MIX_PLN2_MASK_SEL          (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) // 3 */
#define POST_MIX_PLN1_MASK_SEL          (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) // 2 */

#define POSTMIXER                       (fmt_reg_base+0x18)
#define POST_VSYNC_DELAY_EN             (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) //28 */
#define POST_VSYNC_DELAY                (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //16:26 */
#define CCONV_VP2_MODE                  (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) // 6 */
#define CCONV_VP2_POST                  (0x01 << 5)         /* Fld(1, 5, AC_MSKB0) // 5 */
#define CCONV_VP1_MODE                  (0x01 << 4)         /* Fld(1, 4, AC_MSKB0) // 4 */
#define CCONV_VP1_POST                  (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) // 3 */
#define SWAP_P1_P2_POST                 (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) // 2 */
#define MIXER_POST_OFF                  (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) // 1 */

#define FMTCTRL                         (fmt_reg_base+0x574)
#define VDO2_TO_FMT_SEL                 (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) // 7 */
#define VDO1_TO_FMT_SEL                 (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) // 6 */
#define MIXER_TO_POST_RND               (0x01 << 5)         /* Fld(1, 5, AC_MSKB0) // 5 */
#define MIXER_TO_POST_SEL               (0x03 << 2)         /* Fld(2, 2, AC_MSKB0) // 3:2 */
#define DISP2_TO_FMT_SEL                (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) // 1 */
#define DISP1_TO_FMT_SEL                (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) // 0 */

/*****************************************************************************
 *MT8580 MVDO SHARP-B1 Control
 *****************************************************************************/
#define VDO_SHARP_CTRL_01               (vdo_reg_base+0x1B0)
#define CORING_B1                       (0xFF << 24)
#define LIMIT_NEG_B1                    (0xFF << 16)
#define LIMIT_POS_B1                    (0xFF << 8)
#define GAIN_B1                         (0xFF << 0)

#define VDO_SHARP_CTRL_02               (vdo_reg_base+0x1B4)
#define SHARP_EN_B1                     (0x01 << 23)
#define SHRINK_SEL_B1                   (0x07 << 20)
#define CLIP_SEL_B1                     (0x01 << 18)
#define CLIP_EN_B1                      (0x03 << 16)
#define CLIP_NEG_B1                     (0xFF << 8)
#define CLIP_POS_B1                     (0xFF << 0)

#define	VDO_SHARP_CTRL_03               (vdo_reg_base+0x1B8)
#define BYPASS_SHARP                    (0x01 << 24)
#define FILTER_SEL_B1                   (0x01 << 20)
#define SHIFT                           (0x03 << 18)
#define PREC_B1                         (0x03 << 16)
#define LIMIT_NEG                       (0xFF << 8)
#define LIMIT_POS                       (0xFF << 0)

/******************************************************************************
 *  MIAN/SUB VDO METRIC
 *******************************************************************************/
#define M_METRIC_00                     (vdo_reg_base+0x700)
#define S_METRIC_00                     0x43700
#define METRIC_SHIFT_RATIO              (0x0F << 28)        /* Fld(4, 28, AC_MSKB3) //31:28 */
#define METRIC_ENA                      (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) //27 */
#define METRIC_SEE_NEXT16               (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) //26 */
#define METRIC_32BIN                    (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) //25 */
#define METRIC_FRAME_ENA                (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) //24 */
#define METRIC_MASK_FALL                (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) //23:12 */
#define METIRC_MASK_RISE                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10)  //11:0 */

#define M_METRIC_01                     (vdo_reg_base+0x704)
#define S_METRIC_01                     0x43704
#define CNTRES                          (0x03 << 24)        /* Fld(2, 24, AC_MSKB3) //25:24 */
#define METRIC_HEND                     (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) //23:12 */
#define METRIC_HSTART                   (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //11:0 */

#define M_METRIC_02                     (vdo_reg_base+0x708)
#define S_METRIC_02                     0x43708
#define METRIC_Y1                       (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) //23:12 */
#define METRIC_X1                       (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //11:0 */

#define M_METRIC_03                     (vdo_reg_base+0x70C)
#define S_METRIC_03                     0x4370C
#define METRIC_Y2                       (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) //23:12 */
#define METRIC_X2                       (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //11:0 */

#define M_METRIC_04                     (vdo_reg_base+0x710)
#define S_METRIC_04                     0x43710
#define C_BYPASS                        (0x01 << 31)        /* Fld(1, 31, AC_MSKB3) //31 */
#define C_COESEL                        (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) //30 */
#define C_RGB_HISTOGRAM                 (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) //29 */
#define Y2R_MATRIX00                    (0x1FFF << 16)      /* Fld(13, 16, AC_MSKW21) //22:12 */
#define Y2R_MATRIX01                    (0x1FFF << 0)       /* Fld(13, 0, AC_MSKW10) //10:0 */

#define M_METRIC_05                     (vdo_reg_base+0x714)
#define S_METRIC_05                     0x43714
#define Y2R_MATRIX02                    (0x1FFF << 16)      /* Fld(13, 16, AC_MSKW21) //31:24 */
#define Y2R_MATRIX10                    (0x1FFF << 0)       /* Fld(13, 0, AC_MSKW10) //22:12 */

#define M_METRIC_06                     (vdo_reg_base+0x718)
#define S_METRIC_06                     0x43718
#define Y2R_MATRIX11                    (0x1FFF << 16)      /* Fld(13, 16, AC_MSKW21) //22:12 */
#define Y2R_MATRIX12                    (0x1FFF << 0)       /* Fld(13, 0, AC_MSKW10) //10:0 */

#define M_METRIC_07                     (vdo_reg_base+0x71C)
#define S_METRIC_07                     0x4371C
#define Y2R_MATRIX20                    (0x1FFF << 16)      /* Fld(13, 16, AC_MSKW21) //9:0 */
#define Y2R_MATRIX21                    (0x1FFF << 0)       /* Fld(13, 0, AC_MSKW10) //9:0 */

#define M_METRIC_08                     (vdo_reg_base+0x720)
#define S_METRIC_08                     0x43720
#define Y2R_MATRIX22                    (0x1FFF << 0)       /* Fld(13, 0, AC_MSKW10) //12:0 */

#define M_VDO_LUMA_STATUS_00            (vdo_reg_base+0x728)
#define S_VDO_LUMA_STATUS_00            0x43728
#define WIN0_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT0                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_01            (vdo_reg_base+0x72C)
#define S_VDO_LUMA_STATUS_01            0x4372C
#define WIN1_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT1                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_02            (vdo_reg_base+0x730)
#define S_VDO_LUMA_STATUS_02            0x43730
#define WIN2_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT2                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_03            (vdo_reg_base+0x734)
#define S_VDO_LUMA_STATUS_03            0x43734
#define WIN3_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT3                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_04            (vdo_reg_base+0x738)
#define S_VDO_LUMA_STATUS_04            0x43738
#define WIN4_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT4                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_05            (vdo_reg_base+0x73C)
#define S_VDO_LUMA_STATUS_05            0x4373C
#define WIN5_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT5                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_06            (vdo_reg_base+0x740)
#define S_VDO_LUMA_STATUS_06            0x43740
#define WIN6_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT6                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_07            (vdo_reg_base+0x744)
#define S_VDO_LUMA_STATUS_07            0x43744
#define WIN7_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT7                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_08            (vdo_reg_base+0x748)
#define S_VDO_LUMA_STATUS_08            0x43748
#define WIN8_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT8                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_09            (vdo_reg_base+0x74C)
#define S_VDO_LUMA_STATUS_09            0x4374C
#define WIN9_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNT9                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0A            (vdo_reg_base+0x750)
#define S_VDO_LUMA_STATUS_0A            0x43750
#define WINA_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTA                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0B            (vdo_reg_base+0x754)
#define S_VDO_LUMA_STATUS_0B            0x43754
#define WINB_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTB                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0C            (vdo_reg_base+0x758)
#define S_VDO_LUMA_STATUS_0C            0x43758
#define WINC_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTC                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0D            (vdo_reg_base+0x75C)
#define S_VDO_LUMA_STATUS_0D            0x4375C
#define WIND_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTD                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0E            (vdo_reg_base+0x760)
#define S_VDO_LUMA_STATUS_0E            0x43760
#define WINE_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTE                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_0F            (vdo_reg_base+0x764)
#define S_VDO_LUMA_STATUS_0F            0x43764
#define WINF_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define METRIC_PCNTF                    (0xFFFFF << 0)      /* Fld(20, 0, AC_MSKDW) //20:0 */

#define M_VDO_LUMA_STATUS_10            (vdo_reg_base+0x768)
#define S_VDO_LUMA_STATUS_10            0x43768
#define METRIC_PCNTALL                  (0x3FFFFF << 0)     /* Fld(22, 0, AC_FULLDW) //22:0 */

#define M_VDO_LUMA_STATUS_11            (vdo_reg_base+0x76C)
#define S_VDO_LUMA_STATUS_11            0x4376C
#define WIN8_MAX                        (0xFF << 24)        /* Fld(8, 24, AC_MSKB3) //31:24 */
#define WIN8_MIN                        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //23:16 */
#define METRIC_YMAX                     (0xFF << 8)         /* Fld(8, 8, AC_MSKB1) //15:8 */
#define METRIC_YMIN                     (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //8:0 */

#define M_VDO_LUMA_STATUS_12            (vdo_reg_base+0x770)
#define S_VDO_LUMA_STATUS_12            0x43770
#define METRIC_YSUM                     (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_13            (vdo_reg_base+0x774)
#define S_VDO_LUMA_STATUS_13            0x43774
#define WIN0_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_14            (vdo_reg_base+0x778)
#define S_VDO_LUMA_STATUS_14            0x43778
#define WIN1_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_15            (vdo_reg_base+0x77C)
#define S_VDO_LUMA_STATUS_15            0x4377C
#define WIN2_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_16            (vdo_reg_base+0x780)
#define S_VDO_LUMA_STATUS_16            0x43780
#define WIN3_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_17            (vdo_reg_base+0x784)
#define S_VDO_LUMA_STATUS_17            0x43784
#define WIN4_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_18            (vdo_reg_base+0x788)
#define S_VDO_LUMA_STATUS_18            0x43788
#define WIN5_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_19            (vdo_reg_base+0x78C)
#define S_VDO_LUMA_STATUS_19            0x4378C
#define WIN6_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_1A            (vdo_reg_base+0x790)
#define S_VDO_LUMA_STATUS_1A            0x43790
#define WIN7_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_1B            (vdo_reg_base+0x794)
#define S_VDO_LUMA_STATUS_1B            0x43794
#define WIN8_SUM                        (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */

#define M_VDO_LUMA_STATUS_1C            (vdo_reg_base+0x798)
#define S_VDO_LUMA_STATUS_1C            0x43798
#define METRIC_RDY                      (0x01 << 16)        /* Fld(1, 16, AC_MSKB2) //16:16 */
#define METRIC_BRIT_POINT               (0x1F << 8)         /* Fld(5, 8, AC_MSKB1) //12:8 */
#define METRIC_DARK_POINT               (0x1F << 0)         /* Fld(5, 0, AC_MSKB0) //4:0 */

/******************************************************************************
 *  2D SHARPNESS
 *****************************************************************************/

#define TDPROC_00                       (pp_reg_base+0x0)
#define TDSHARP_GAIN1                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS1              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG1              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING1                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_01                       (pp_reg_base+0x4)
#define TDSHARP_CLIP_THPOS1             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG1             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL1        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL1          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN1            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL1                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL1          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_02                       (pp_reg_base+0x8)
#define TDSHARP_H1_SOFT_COR_GAIN        (0x0F << 12)        /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_PREC1                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN1         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_03                       (pp_reg_base+0xC)
#define TDSHARP_GAIN_NEG1               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_04                       (pp_reg_base+0x10)
#define TDSHARP_GAIN2                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS2              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG2              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING2                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_05                       (pp_reg_base+0x14)
#define TDSHARP_CLIP_THPOS2             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG2             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL2        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL2          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN2            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL2                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL2          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_06                       (pp_reg_base+0x18)
#define TDSHARP_H2_SOFT_COR_GAIN        (0x0f << 12)
#define TDSHARP_PREC2                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN2         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_07                       (pp_reg_base+0x1C)
#define TDSHARP_GAIN_NEG2               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_08                       (pp_reg_base+0x20)
#define TDSHARP_GAIN3                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS3              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG3              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING3                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_09                       (pp_reg_base+0x24)
#define TDSHARP_CLIP_THPOS3             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG3             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL3        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL3          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN3            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL3                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL3          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_0A                       (pp_reg_base+0x28)
#define TDSHARP_V1_SOFT_COR_GAIN        (0x0f << 12)
#define TDSHARP_PREC3                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN3         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_0B                       (pp_reg_base+0x2C)
#define TDSHARP_GAIN_NEG3               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_0C                       (pp_reg_base+0x30)
#define TDSHARP_GAIN4                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS4              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG4              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING4                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_0D                       (pp_reg_base+0x34)
#define TDSHARP_CLIP_THPOS4             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG4             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL4        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL4          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN4            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL4                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL4          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_0E                       (pp_reg_base+0x38)
#define TDSHARP_V2_SOFT_COR_GAIN        (0x0f << 12)
#define TDSHARP_PREC4                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN4         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_0F                       (pp_reg_base+0x3C)
#define TDSHARP_GAIN_NEG4               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_10                       (pp_reg_base+0x40)
#define TDSHARP_GAIN5                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS5              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG5              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING5                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_11                       (pp_reg_base+0x44)
#define TDSHARP_CLIP_THPOS5             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG5             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL5        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL5          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN5            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL5                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL5          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_12                       (pp_reg_base+0x48)
#define TDSHARP_X1_SOFT_COR_GAIN        (0x0f << 12)
#define TDSHARP_PREC5                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN5         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_13                       (pp_reg_base+0x4C)
#define TDSHARP_GAIN_NEG5               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_14                       (pp_reg_base+0x50)
#define TDSHARP_GAIN6                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_LIMIT_POS6              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_LIMIT_NEG6              (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define TDSHARP_CORING6                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define TDPROC_15                       (pp_reg_base+0x54)
#define TDSHARP_CLIP_THPOS6             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define TDSHARP_CLIP_THNEG6             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define TDSHARP_ARP_CLIP_LC_SEL6        (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define TDSHARP_ATTENUATE_SEL6          (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define TDSHARP_ARP_CLIP_EN6            (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define TDSHARP_LPF_SEL6                (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define TDSHARP_CLIP_BAND_SEL6          (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define TDPROC_16                       (pp_reg_base+0x58)
#define TDSHARP_X2_SOFT_COR_GAIN        (0x0f << 12)
#define TDSHARP_PREC6                   (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define TDSHARP_SOFT_CLIP_GAIN6         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_17                       (pp_reg_base+0x5C)
#define TDSHARP_GAIN_NEG6               (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define TDPROC_20                       (pp_reg_base+0x80)	/* TDSHARP_BAND H1_1     Q */
#define TDSHARP_H1_1_GAIN               (0xff << 24)
#define TDSHARP_H1_1_LIMIT_POS          (0xff << 16)
#define TDSHARP_H1_1_LIMIT_NEG          (0xff << 8)
#define TDSHARP_H1_1_CORING             (0xff << 0)

#define	TDPROC_21                       (pp_reg_base+0x84)	/* TDSHARP_BAND H1_1 Q */
#define TDSHARP_H1_1_CLIP_THPOS         (0xff << 24)
#define TDSHARP_H1_1_CLIP_THNEG         (0xff << 16)
#define TDSHARP_H1_1_CLIP_EN            (0x01 << 7)
#define TDSHARP_H1_1_LPF_SEL            (0x03 << 4)

#define TDPROC_22                       (pp_reg_base+0x88)	/* TDSHARP_BAND H1_1 Q */
#define TDSHARP_H1_1_SOFT_COR_GAIN      (0x0f << 12)
#define TDSHARP_H1_1_SOFT_CLIP_GAIN     (0xff << 0)

#define TDPROC_23                       (pp_reg_base+0x8C)	/* TDSHARP_BAND H1_1 Q */
#define TDSHARP_H1_1_GAIN_NEG           (0xff << 24)

#define TDPROC_24                       (pp_reg_base+0x90)
#define TDSHARP_EN                      (0x01 << 31)        /* Fld(1, 31, AC_MSKB3) */
#define TDPPROC_BYPASS_ALL              (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) */
#define TDSHARP_NEG_GAIN_EN             (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) */
#define TDSHARP_LTI_EN                  (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) */
#define TDSHARP_INK_EN                  (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) */
#define TDSHARP_CLIP_EN                 (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) */
#define TDSHARP_CLIP_SEL                (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) */
#define CRC2_EN                         (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) */
#define BAND8_INDEPENDENT_CLIP          (0x01 << 23)        /* Fld(1, 23, AC_MSKB3) */
#define TDSHARP_LIMIT_POS_ALL           (0x3FF << 12)       /* Fld(10,12, AC_MSKW21) */
#define TDSHARP_LIMIT_NEG_ALL           (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) */

#define TDPROC_25                       (pp_reg_base+0x94)
#define TDSHARP_HEND                    (0xFFF << 12)       /* Fld(12,12, AC_MSKW21) */
#define TDSHARP_LSTART                  (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define TDPROC_26                       (pp_reg_base+0x98)
#define TDSHARP_MAX_MIN_ATT             (0x03 << 30)        /* Fld(2, 30, AC_MSKB3) */
#define TDSHARP_MAX_MIN_SEL             (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) */
#define TDSHARP_X1_FLT_SEL              (0x01 << 28)
#define TDSHARP_H2_FLT_SEL              (0x03 << 26)

#define TDSHARP_BAND4_SEL               (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) */
#define TDSHARP_BAND9_SEL               (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) */
#define TDSHARP_H1_FLT_SEL              (0x01 << 23)
#define TDSHARP_V1_FLT_SEL              (0x01 << 22)
#define TDSHARP_SFT                     (0x03 << 20)        /* Fld(2, 20, AC_MSKB2) */
#define AC_LPF_EN                       (0x01 << 18)        /* Fld(1, 18, AC_MSKB2) */
#define TDSHARP_MAX_MIN_LMT             (0xFF << 0)         /* Fld(8,  0, AC_MSKB0) */

#define TDPROC_27                       (pp_reg_base+0x9C)
#define TDSHARP_LC_LOAD_ENA_H           (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) */
#define TDSHARP_LC_LOAD_BURST           (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) */
#define TDSHARP_LC_LOAD_ENA             (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) */
#define TDSHARP_LC_READ_ENA             (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) */

#define TDPROC_28                       (pp_reg_base+0xA0)
#define TDSHARP_LC_LOAD_H               (0xFF << 8)         /* Fld(8, 16, AC_FULLB2) */
#define TDSHARP_LC_LOAD_IND             (0x7F << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_29                       (pp_reg_base+0xA4)
#define TDSHARP_YLEV_TBL                (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define TDSHARP_LC_TBL_H                (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_2C                       (pp_reg_base+0xB0)
#define TDSHARP_YLEV_ENA                (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) */
#define TDSHARP_YLEV_ALPHA              (0xFF << 20)        /* Fld(8, 20, AC_MSKW32) */
#define TDSHARP_YLEV_LOAD_VAL           (0xFF << 12)        /* Fld(8, 12, AC_MSKW21) */
#define TDSHARP_YLEV_IND                (0x7F << 4)         /* Fld(10, 4, AC_MSKW10) */
#define TDSHARP_YLEV_LOAD_TRIG          (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) */
#define TDSHARP_YLEV_LOAD_BURST         (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) */
#define TDSHARP_YLEV_LOAD               (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) */
#define TDSHARP_YLEV_READ_ENA           (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) */

#define TDPROC_2F                       (pp_reg_base+0xBC)
#define TDPEAK_HSP_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define TDSHARP_CDEG_SMP1               (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define TDPROC_30                       (pp_reg_base+0xC0)
#define TDPEAK_VMASK                    (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) */
#define TDPEAK_HMASK                    (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) */
#define TDPEAK_VSP_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define TDPEAK_VSP_START                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define TDPROC_31                       (pp_reg_base+0xC4)
#define TDPEAK_HDM_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define TDPEAK_HDM_START                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define TDPROC_32                       (pp_reg_base+0xC8)
#define TDPEAK_DEMO_SX                  (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) */
#define TDPEAK_DEMO                     (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) */
#define TDPEAK_VDM_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define TDPEAK_VDM_START                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define TDPROC_34                       (pp_reg_base+0xD0)	/* TDSHARP_BAND H2_1     Q */
#define TDSHARP_H2_1_GAIN               (0xff << 24)
#define TDSHARP_H2_1_LIMIT_POS          (0xff << 16)
#define TDSHARP_H2_1_LIMIT_NEG          (0xff << 8)
#define TDSHARP_H2_1_CORING             (0xff << 0)

#define	TDPROC_35                       (pp_reg_base+0xD4)	/* TDSHARP_BAND H2_1 Q */
#define TDSHARP_H2_1_CLIP_THPOS         (0xff << 24)
#define TDSHARP_H2_1_CLIP_THNEG         (0xff << 16)
#define TDSHARP_H2_1_FLT_SEL            (0x03 << 8)
#define TDSHARP_H2_1_CLIP_EN            (0x01 << 7)
#define TDSHARP_H2_1_LPF_SEL            (0x03 << 4)

#define	TDPROC_36                       (pp_reg_base+0xD8)	/* TDSHARP_BAND H2_1 Q */
#define TDSHARP_H2_1_SOFT_COR_GAIN      (0x0f << 12)
#define TDSHARP_H2_1_SOFT_CLIP_GAIN     (0xff << 0)

#define TDPROC_37                       (pp_reg_base+0xDC)	/* TDSHARP_BAND H2_1 */
#define TDSHARP_H2_1_GAIN_NEG           (0xff << 24)

/**************HVBand Control**************/
#define	TDPROC_6B                       (pp_reg_base+0x1AC)
#define TDSHARP_HVBAND_COR              (0xff << 0)
#define TDSHARP_HVBAND_EN               (0x01 << 8)
#define TDSHARP_BGTESTMODE              (0xffff << 16)

#define	TDPROC_6C                       (pp_reg_base+0x1B0)
#define TDSHARP_HVBAND0LV               (0xff << 0)
#define TDSHARP_HVBAND1LV               (0xff << 8)
#define TDSHARP_HVBAND2LV               (0xff << 16)
#define TDSHARP_HVBAND3LV               (0xff << 24)

#define	TDPROC_6D                       (pp_reg_base+0x1B4)
#define TDSHARP_HVBAND4LV               (0xff << 0)
#define TDSHARP_HVBAND5LV               (0xff << 8)
#define TDSHARP_HVBAND6LV               (0xff << 16)
#define TDSHARP_HVBAND7LV               (0xff << 24)

#define	TDPROC_6E                       (pp_reg_base+0x1B8)
#define TDSHARP_LTIHVBAND0LV            (0xff << 0)
#define TDSHARP_LTIHVBAND1LV            (0xff << 8)
#define TDSHARP_LTIHVBAND2LV            (0xff << 16)
#define TDSHARP_LTIHVBAND3LV            (0xff << 24)

#define	TDPROC_6F                       (pp_reg_base+0x1BC)
#define TDSHARP_LTIHVBAND4LV            (0xff << 0)
#define TDSHARP_LTIHVBAND5LV            (0xff << 8)
#define TDSHARP_LTIHVBAND6LV            (0xff << 16)
#define TDSHARP_LTIHVBAND7LV            (0xff << 24)
/**************HVBand Control End**************/

#define LTI_00                          (pp_reg_base+0xE0)
#define LTI_GAIN1                       (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define LTI_LIMIT_POS1                  (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define LTI_LIMIT_NEG1                  (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define LTI_CORING1                     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define LTI_01                          (pp_reg_base+0xE4)
#define LTI_CLIP_THPOS1                 (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define LTI_CLIP_THNEG1                 (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define LTI_ARP_CLIP_LC_SEL1            (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define LTI_ATTENUATE_SEL1              (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define LTI_ARP_CLIP_EN1                (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define LTI_LPF_SEL1                    (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define LTI_CLIP_BAND_SEL1              (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define LTI_02                          (pp_reg_base+0xE8)
#define LTI_SOFT_COR_GAIN1              (0x0f << 12)
#define LTI_PREC1                       (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define LTI_SOFT_CLIP_GAIN1             (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define LTI_03                          (pp_reg_base+0xEC)
#define LTI_GAIN_NEG1                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */

#define LTI_04                          (pp_reg_base+0xF0)
#define LTI_GAIN2                       (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define LTI_LIMIT_POS2                  (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define LTI_LIMIT_NEG2                  (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define LTI_CORING2                     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define LTI_05                          (pp_reg_base+0xF4)
#define LTI_CLIP_THPOS2                 (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define LTI_CLIP_THNEG2                 (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define LTI_ARP_CLIP_LC_SEL2            (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) //10:8 */
#define LTI_ATTENUATE_SEL2              (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define LTI_ARP_CLIP_EN2                (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7:6 */
#define LTI_LPF_SEL2                    (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //7:6 */
#define LTI_CLIP_BAND_SEL2              (0x0F << 0)         /* Fld(3, 0, AC_MSKB0) // 2:0 */

#define LTI_06                          (pp_reg_base+0xF8)
#define LTI_SOFT_COR_GAIN2              (0x0f << 12)
#define LTI_PREC2                       (0x03 << 8)         /* Fld(2, 8, AC_MSKB1) */
#define LTI_SOFT_CLIP_GAIN2             (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define LTI_07                          (pp_reg_base+0xFC)
#define LTI_GAIN_NEG2                   (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */



#define TDPROC_B0                       (pp_reg_base+0x2C0)
#define ADAP_SHP_EN                     (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) */
#define ADAP_SHP_INK_EN                 (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) */
#define ADAP_SHP_EDGE_SEL               (0x07 << 24)        /* Fld(3, 24, AC_MSKB3) */
#define ADAP_SHP_P3                     (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define ADAP_SHP_P2                     (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ADAP_SHP_P1                     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_B1                       (pp_reg_base+0x2C4)
#define ADAP_SHP_L_BOUND                (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define ADAP_SHP_EDGE_SCALE             (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) */


#define TDPROC_B2                       (pp_reg_base+0x2C8)
#define ADAP_SHP_G1                     (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_U_BOUND                (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ADAP_SHP_OFFSET                 (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_B3                       (pp_reg_base+0x2CC)
#define ADAP_SHP_G3                     (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_G2                     (0x3FF << 0)        /* Fld(10, 0, AC_MSKW21) */

#define TDPROC_B4                       (pp_reg_base+0x2D0)
#define ADAP_SHP_G1_ARB1                (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_BOUND_ARB1             (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ADAP_SHP_OFFSET_ARB1            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_B5                       (pp_reg_base+0x2D4)
#define ADAP_SHP_G3_ARB1                (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_G2_ARB1                (0x3FF << 0)        /* Fld(10, 0, AC_MSKW21) */

#define TDPROC_B6                       (pp_reg_base+0x2D8)
#define ADAP_SHP_G1_ARB2                (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_BOUND_ARB2             (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ADAP_SHP_OFFSET_ARB2            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_B7                       (pp_reg_base+0x2DC)
#define ADAP_SHP_G3_ARB2                (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define ADAP_SHP_G2_ARB2                (0x3FF << 0)        /* Fld(10, 0, AC_MSKW21) */

#define TDPROC_C0                       (pp_reg_base+0x300)
#define LC_EN                           (0x01 << 31)        /* Fld(1, 31, AC_MSKB3) */
#define LC_INDEX_SEL                    (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) */
#define LC_MODE                         (0x03 << 24)        /* Fld(2, 24, AC_MSKB3) */
#define LC_IDX_LPF_EN                   (0x01 << 19)        /* Fld(1, 19, AC_MSKB2) */
#define LC_LPF_COE                      (0x0F << 0)         /* Fld(4, 0, AC_MSKB0) */

#define TDPROC_C2                       (pp_reg_base+0x308)
#define LC_IDX_LUT1_G1                  (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) */
#define LC_IDX_LUT1_P1                  (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define CHROMA_THD                      (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */


#define TDPROC_C3                       (pp_reg_base+0x30C)
#define POS_CLIP                        (0x3FF << 20)       /* Fld(10, 20, AC_MSKW32) */
#define NEG_CLIP                        (0x3FF << 8)        /* Fld(10, 8, AC_MSKW21) */
#define CLIP_MAIN                       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define TDPROC_C4                       (pp_reg_base+0x310)
#define TDSHARP_LCE_GAIN                (0xFF << 16)        /* Fld(8, 16, AC_FULLB3) */
#define TDSHARP_LCE_CONTRAST            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */


/******************************************************************************
 *  ECTI
 ******************************************************************************/

#define ECTI_00                         (pp_reg_base+0x100)
#define ECTI_CLP_SZ                     (0x07 << 29)        /* Fld(3, 29, AC_MSKB3) // 4 */
#define ECTI_HUE_TIE                    (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) // 4 */
#define ECTI_VWGT                       (0x0F << 24)        /* Fld(4, 24, AC_MSKB3) // 4 */
#define ECTI_UWGT                       (0x0F << 20)        /* Fld(4, 20, AC_MSKB2) // 4 */
#define ECTI_CLP_ENA                    (0x01 << 13)        /* Fld(1, 13, AC_MSKB1) // 4 */
#define ECTI_FLPF                       (0x01 << 12)        /* Fld(1, 12, AC_MSKB1) // 4 */
#define ECTI_FLPF_SEL                   (0x01 << 11)        /* Fld(1, 11, AC_MSKB1) // 4 */
#define ECTI_Dx_SGN                     (0x01 << 9)         /* Fld(1, 9, AC_MSKB1) // 4 */
#define ECTI_MASK_EN                    (0x01 << 8)         /* Fld(1, 8, AC_MSKB1) // 4 */
#define ECTI_VMASKSP                    (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) // 4 */
#define ECTI_PRT_ENA                    (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) // 4 */
#define ECTI_SGN_PRT                    (0x01 << 5)         /* Fld(1, 5, AC_MSKB0) // 4 */
#define ECTI_HD                         (0x01 << 4)         /* Fld(1, 4, AC_MSKB0) // 4 */
#define ECTI_INK                        (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) // 3 */
#define ECTI_ENA                        (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //0 */

#define ECTI_01                         (pp_reg_base+0x104)
#define ECTI_LPF3                       (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) */
#define ECTI_LPF3_SEL                   (0x03 << 24)        /* Fld(2, 24, AC_MSKB3) */
#define ECTI_LPF2                       (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) */
#define ECTI_LPF2_SEL                   (0x03 << 20)        /* Fld(2, 20, AC_MSKB2) */
#define ECTI_LPF1                       (0x01 << 18)        /* Fld(1, 18, AC_MSKB2) */
#define ECTI_LPF1_SEL                   (0x03 << 16)        /* Fld(2, 16, AC_MSKB2) */
#define ECTI_FIX_SZ                     (0x07 << 12)        /* Fld(3, 12, AC_MSKB1) */
#define ECTI_FIX                        (0x01 << 11)        /* Fld(1, 11, AC_MSKB1) */
#define ECTI_SEXT_LARGE                 (0x01 << 10)        /* Fld(1, 10, AC_MSKB1) */
#define ECTI_SEXT                       (0x01 << 9)         /* Fld(1, 9, AC_MSKB1) */
#define ECTI_SGN                        (0x01 << 8)         /* Fld(1, 8, AC_MSKB1) */
#define ECTI_SGN_TH                     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define ECTI_02                         (pp_reg_base+0x108)
#define ECTI_U_STB_OFST2                (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */
#define ECTI_U_STB_GAIN                 (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define ECTI_U_STB_OFST1                (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ECTI_U_STB_BYPASS               (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) */
#define ECTI_U_WND_OFST                 (0x07 << 4)         /* Fld(3, 4, AC_MSKB0) */
#define ECTI_U_WND_SZ                   (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) */

#define ECTI_03                         (pp_reg_base+0x10C)
#define ECTI_V_STB_OFST2                (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */
#define ECTI_V_STB_GAIN                 (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define ECTI_V_STB_OFST1                (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ECTI_V_STB_BYPASS               (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) */
#define ECTI_V_WND_OFST                 (0x07 << 4)         /* Fld(3, 4, AC_MSKB0) */
#define ECTI_V_WND_SZ                   (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) */

#define ECTI_04                         (pp_reg_base+0x118)
#define ECTI_FLAT_OFST2                 (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) */
#define ECTI_FLAT_GAIN                  (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) */
#define ECTI_FLAT_OFST1                 (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define ECTI_FLAT_MODE                  (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) */
#define ECTI_FLAT_TIE                   (0x01 << 5)         /* Fld(1, 5, AC_MSKB0) */
#define ECTI_FLAT_BYPASS                (0x01 << 4)         /* Fld(1, 4, AC_MSKB0) */
#define ECTI_FLAT_SZ                    (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) */

#define ECTI_05                         (pp_reg_base+0x124)
#define ECTI_COR                        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define ECTI_06                         (pp_reg_base+0x128)
#define ECTI_V_LMT_ENA                  (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) */
#define ECTI_V_LMT                      (0x3FF << 12)       /* Fld(10, 12, AC_MSKW21) */
#define ECTI_U_LMT_ENA                  (0x01 << 10)        /* Fld(1, 10, AC_MSKB1) */
#define ECTI_U_LMT                      (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define ECTI_07                         (pp_reg_base+0x12C)
#define ECTI_HMSK_END                   (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define ECTI_HMSK_START                 (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define ECTI_08                         (pp_reg_base+0x130)
#define ECTI_VMSK_END                   (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define ECTI_VMSK_START                 (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define ECTI_09                         (pp_reg_base+0x134)
#define ECTI_HDEMO_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define ECTI_HDEMO_START                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

#define ECTI_0A                         (pp_reg_base+0x138)
#define ECTI_DEMO_SX                    (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) */
#define ECTI_DEMO_ENA                   (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) */
#define ECTI_VDEMO_END                  (0xFFF << 12)       /* Fld(12, 12, AC_MSKW21) */
#define ECTI_VDEMO_START                (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) */

/**************HLTI****************/

#define	HLTI_01                         (pp_reg_base+0x184)
#define HLTI_HDEG_GAIN                  (0xff << 8)
#define HLTI_HDIFF_OFFSET               (0xff << 0)
#define HLTI_VDEG_GAIN                  (0xff << 24)
#define HLTI_VDIFF_OFFSET               (0xff << 16)

#define	HLTI_00                         (pp_reg_base+0x188)
#define HLTI_EN                         (0x1 << 0)
#define HLTI_PEAKING                    (0x1 << 1)

#define	HLTI_02                         (pp_reg_base+0x18C)
#define HLTI_END_X                      (0x3ff << 16)
#define HLTI_START_X                    (0x3ff << 0)

#define	HLTI_03                         (pp_reg_base+0x190)
#define HLTI_SLOPE_X                    (0x7fff << 0)

#define	HLTI_04                         (pp_reg_base+0x194)
#define HLTI_END_HV                     (0x3ff << 20)
#define HLTI_MIDDLE_HV                  (0x3ff << 10)
#define HLTI_START_HV                   (0x3ff << 0)

#define	HLTI_05                         (pp_reg_base+0x198)
#define HLTI_SLOPEUP_HV                 (0x7fff << 16)
#define HLTI_SLOPEDOWN_HV               (0x7fff << 0)

/**************HLTI End**************/

/******************************************************************************
 *  1D SHARPNESS
 ******************************************************************************/
#define YSHARP_00                       (pp_reg_base+0x200)
#define SHARP2_BYPASS                   (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) //30 */
#define SHARP2_CLIP_POS_EN              (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) //27 */
#define SHARP2_CLIP_NEG_EN              (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) //26 */
#define SHARP2_CLP_RNG                  (0x03 << 24)        /* Fld(2, 24, AC_MSKB3) //25:24 */
#define SHARP2_HIGH                     (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define SHARP2_MID                      (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_LOW                      (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_01                       (pp_reg_base+0x204)
#define SHARP2_CLIP_POS_EN_H            (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) //29 */
#define SHARP2_CLIP_NEG_EN_H            (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) //28 */
#define SHARP2_CLIP_POS_EN_M            (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) //27 */
#define SHARP2_CLIP_NEG_EN_M            (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) //26 */
#define SHARP2_CLIP_POS_EN_L            (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) //25 */
#define SHARP2_CLIP_NEG_EN_L            (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) //24 */
#define SHARP2_HIGH_CORING              (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define SHARP2_MID_CORING               (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_LOW_CORING               (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_02                       (pp_reg_base+0x208)
#define SHARP2_HIGH_LIMIT_POS           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define SHARP2_MID_LIMIT_POS            (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_LOW_LIMIT_POS            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_03                       (pp_reg_base+0x20C)
#define SHARP2_HIGH_LIMIT_NEG           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define SHARP2_MID_LIMIT_NEG            (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_LOW_LIMIT_NEG            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_04                       (pp_reg_base+0x210)
#define SHARP2_DEMO_SWAP                (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) //27 */
#define SHARP2_DEMO_CNT                 (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define SHARP2_CLIP_POS_THRE            (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_CLIP_NEG_THRE            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_05                       (pp_reg_base+0x214)
#define SHARP2_Y_BND_PRT_DIS            (0x01 << 29)        /* Fld(1, 29, AC_MSKB3) //29 */
#define SHARP2_BND_EXT_ENA              (0x01 << 28)        /* Fld(1, 28, AC_MSKB3) //28 */
#define SHARP2_NRM                      (0x01 << 27)        /* Fld(1, 27, AC_MSKB3) //27 */
#define SHARP2_LOW_BAND_SEL             (0x01 << 26)        /* Fld(1, 26, AC_MSKB3) //26 */
#define SHARP2_HIGH_BAND_SEL            (0x01 << 25)        /* Fld(1, 25, AC_MSKB3) //25 */
#define SHARP2_SEP_GAIN                 (0x01 << 24)        /* Fld(1, 24, AC_MSKB3) //24 */
#define SHARP2_HIGH_NEG                 (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define SHARP2_MID_NEG                  (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define SHARP2_LOW_NEG                  (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define YSHARP_06                       (pp_reg_base+0x218)
#define SHARP2_HIGH_PREC3               (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define SHARP2_MID_PREC3                (0x07 << 4)         /* Fld(3, 4, AC_MSKB0) //6:4 */
#define SHARP2_LOW_PREC3                (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) //2:0 */


#define ARB_SHP1_01                     (pp_reg_base+0x280)
#define ARB_SHP1_GAIN_SIGN              (0x01 << 31)        /* Fld(1, 31, AC_MSKB3) //10:8 */
#define ARB_SHP1_GAIN                   (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //6:4 */
#define ARB_SHP1_CORING                 (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //2:0 */
#define ARB_SHP1_MODE                   (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) //6:4 */
#define ARB_SHP1_ENABLE                 (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //2:0 */

#define ARB_SHP1_02                     (pp_reg_base+0x284)
#define ARB_SHP1_LOWER_BOUND            (0x1FF << 16)       /* Fld(9, 16, AC_FULLW32) //10:8 */
#define ARB_SHP1_UPPER_BOUND            (0x1FF << 0)        /* Fld(9, 0, AC_FULLW10) //6:4 */

#define ARB_SHP1_03                     (pp_reg_base+0x288)
#define ARB_SHP1_VALID_RANGE_INV        (0x3FF << 16)       /* Fld(10, 16, AC_FULLW32) //10:8 */
#define ARB_SHP1_VALID_RANGE            (0x3FF << 0)        /* Fld(10, 0, AC_FULLW10) //6:4 */

#define ARB_SHP1_04                     (pp_reg_base+0x28C)
#define ARB_SHP1_CR_CENTER              (0x3FF << 16)       /* Fld(10, 16, AC_FULLW32) //10:8 */
#define ARB_SHP1_CB_CENTER              (0x3FF << 0)        /* Fld(10, 0, AC_FULLW10) //6:4 */

#define ARB_SHP2_01                     (pp_reg_base+0x290)
#define ARB_SHP2_GAIN_SIGN              (0x01 << 31)        /* Fld(1, 31, AC_MSKB3) //10:8 */
#define ARB_SHP2_GAIN                   (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //6:4 */
#define ARB_SHP2_CORING                 (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //2:0 */
#define ARB_SHP2_MODE                   (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) //6:4 */
#define ARB_SHP2_ENABLE                 (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //2:0 */

#define ARB_SHP2_02                     (pp_reg_base+0x294)
#define ARB_SHP2_LOWER_BOUND            (0x1FF << 16)       /* Fld(9, 16, AC_FULLW32) //10:8 */
#define ARB_SHP2_UPPER_BOUND            (0x1FF << 0)        /* Fld(9, 0, AC_FULLW10) //6:4 */

#define ARB_SHP2_03                     (pp_reg_base+0x298)
#define ARB_SHP2_VALID_RANGE_INV        (0x3FF << 16)       /* Fld(10, 16, AC_FULLW32) //10:8 */
#define ARB_SHP2_VALID_RANGE            (0x3FF << 0)        /* Fld(10, 0, AC_FULLW10) //6:4 */

#define ARB_SHP2_04                     (pp_reg_base+0x29C)
#define ARB_SHP2_CR_CENTER              (0x3FF << 16)       /* Fld(10, 16, AC_FULLW32) //10:8 */
#define ARB_SHP2_CB_CENTER              (0x3FF << 0)        /* Fld(10, 0, AC_FULLW10) //6:4 */

/******************************************************************************
 *  MISC
 *******************************************************************************/
#define TDPROC_MISC_00                  (pp_reg_base+0x3C0)
#define TDPROC_RND_SEL                  (0x01 << 16)        /* Fld(1, 16, AC_MSKB2) //16 */
#define TDPROC_RND_ENA                  (0x01 << 8)         /* Fld(1, 8, AC_MSKB1) //8 */
#define TDPROC_DELSEL                   (0x03 << 4)         /* Fld(2, 4, AC_MSKB0) //5:4 */
#define TDPROC_DGB                      (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) //2:0 */

#define TDPROC_MISC_01                  (pp_reg_base+0x3C4)
#define TDPROC_RND_LD                   (0x01 << 30)        /* Fld(1, 30, AC_MSKB3) //30 */
#define TRPRCO_RND_SEED                 (0x3FFFFFFF << 0)   /* Fld(30, 0, AC_MSKDW) //29:0 */


/******************************************************************************
 *  COLOR PROCESS MAIN
 ******************************************************************************/
#define CFG_MAIN                        (pp_reg_base+0x800)
#define H_S_SEL                         (0x01 << 21)        /* Fld(1, 21, AC_MSKB2) //21 */
#define H_Y_SEL                         (0x01 << 20)        /* Fld(1, 20, AC_MSKB2) //20 */
#define S_H_SEL                         (0x01 << 19)        /* Fld(1, 19, AC_MSKB2) //19 */
#define S_Y_SEL                         (0x01 << 18)        /* Fld(1, 18, AC_MSKB2) //18 */
#define Y_H_SEL                         (0x01 << 17)        /* Fld(1, 17, AC_MSKB2) //17 */
#define Y_S_SEL                         (0x01 << 16)        /* Fld(1, 16, AC_MSKB2) //16 */
#define ENG_SWAP                        (0x01 << 11)        /* Fld(1, 11, AC_MSKB1) //11 */
#define ENG_SEQ_SEL                     (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define COLORBP                         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) // 7:0 */
#define ALLBP                           (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) // 7 */
#define YSHBP                           (0x07 << 2)         /* Fld(3, 2, AC_MSKB0) // 4:2 */
#define HEBP                            (0x01 << 4)         /* Fld(1, 4, AC_MSKB0) // 4 */
#define SEBP                            (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) // 3 */
#define YEBP                            (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) // 2 */
#define P2CBP                           (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) // 1 */
#define C2PBP                           (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //0 */

#define PXL_CNT_MAIN                    (pp_reg_base+0x808)
#define V_CONT                          (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define H_CONT                          (0xFFF << 0)        /* Fld(12, 0, AC_MSKW10) //11:0 */

#define CH_DLY_MAIN                     (pp_reg_base+0x814)
#define CR_DEL_MAIN                     (0x07 << 6)         /* Fld(3, 6, AC_MSKW10) //8:6 */
#define CB_DEL_MAIN                     (0x07 << 3)         /* Fld(3, 3, AC_MSKB0) //5:3 */
#define Y_DEL_MAIN                      (0x07 << 0)         /* Fld(3, 0, AC_MSKB0) //2:0 */

#define G_PIC_ADJ_MAIN                  (pp_reg_base+0x818)
    /* #define HUE_SIGN              0x01 << 31  //Fld(1, 31, AC_MSKB3) //31 */
#define HUE                             (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define SAT                             (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
    /* #define BRI_SIGN              0x01 << 15  //Fld(1, 15, AC_MSKB1) //15 */
#define BRI                             (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define CONT                            (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define G_PIC_ADJ                       (pp_reg_base+0x81C)
#define BRI_LSB                         (0x1 << 24)         /* Fld(8, 24, AC_FULLB3) //31:24 */

#define DBG_CFG_MAIN                    (pp_reg_base+0x820)
#define SPLIT_X_START                   (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define EXPAND                          (0x01 << 5)         /* Fld(1, 5, AC_MSKB0) */
#define SPLIT_SWAP                      (0x01 << 4)         /* Fld(1, 4, AC_MSKB0) //4 */
#define SPLIT_EN                        (0x01 << 3)         /* Fld(1, 3, AC_MSKB0) //3 */
#define CAP_EN                          (0x01 << 2)         /* Fld(1, 2, AC_MSKB0) //2 */
#define INK_MODE                        (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) //1 */
#define INK_EN                          (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //0 */

#define EXP_COEFF                       (pp_reg_base+0x824)
#define C_EXP_COEFF                     (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) */
#define Y_EXP_COEFF                     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) */

#define C_BOOST_MAIN                    (pp_reg_base+0x828)
#define ENABLE                          (0x01 << 15)        /* Fld(1, 15, AC_MSKB1) //15 */
#define RANGE_SEL                       (0x07 << 8)         /* Fld(3, 8, AC_MSKB1) //10:8 */
#define BOOST_GAIN                      (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define Y_FTN_1_0_MAIN                  (pp_reg_base+0x840)
#define Y_FTN_1                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_0                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_3_2_MAIN                  (pp_reg_base+0x844)
#define Y_FTN_3                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_2                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_5_4_MAIN                  (pp_reg_base+0x848)
#define Y_FTN_5                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_4                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_7_6_MAIN                  (pp_reg_base+0x84C)
#define Y_FTN_7                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_6                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_9_8_MAIN                  (pp_reg_base+0x850)
#define Y_FTN_9                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_8                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_11_10_MAIN                (pp_reg_base+0x854)
#define Y_FTN_11                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_10                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_13_12_MAIN                (pp_reg_base+0x858)
#define Y_FTN_13                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_12                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_15_14_MAIN                (pp_reg_base+0x85C)
#define Y_FTN_15                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_14                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_17_16_MAIN                (pp_reg_base+0x860)
#define Y_FTN_17                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_16                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_19_18_MAIN                (pp_reg_base+0x864)
#define Y_FTN_19                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_18                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_21_20_MAIN                (pp_reg_base+0x868)
#define Y_FTN_21                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_20                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_23_22_MAIN                (pp_reg_base+0x86C)
#define Y_FTN_23                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_22                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_25_24_MAIN                (pp_reg_base+0x870)
#define Y_FTN_25                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_24                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_27_26_MAIN                (pp_reg_base+0x874)
#define Y_FTN_27                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_26                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_29_28_MAIN                (pp_reg_base+0x878)
#define Y_FTN_29                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_28                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_31_30_MAIN                (pp_reg_base+0x87C)
#define Y_FTN_31                        (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_30                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_FTN_32_MAIN                   (pp_reg_base+0x880)
#define Y_FTN_32                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define POS_MAIN_MAIN                   (pp_reg_base+0x884)
#define POS_Y                           (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define POS_X                           (0x7FF << 0)        /* Fld(11, 0, AC_MSKW10) //10:0 */

#define INK_DATA_MAIN_CB                (pp_reg_base+0x888)
#define INK_DATA_CB                     (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define INK_DATA_Y                      (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //10:0 */

#define INK_DATA_MAIN_CR                (pp_reg_base+0x88C)
#define INK_DATA_CR                     (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define CAP_IN_DATA_MAIN                (pp_reg_base+0x890)
#define CAP_IN_CB                       (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define CAP_IN_Y                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define CAP_IN_DATA_MAIN_CR             (pp_reg_base+0x894)
#define CAP_IN_CR                       (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define CAP_OUT_DATA_MAIN               (pp_reg_base+0x898)
#define CAP_OUT_CB                      (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define CAP_OUT_Y                       (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define CAP_OUT_DATA_MAIN_CR            (pp_reg_base+0x89C)
#define CAP_OUT_CR                      (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */

#define Y_SLOPE_1_0_MAIN                (pp_reg_base+0x8A0)
#define Y_SLOPE_1_0_MAIN_SIGN_1         (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_1_0_MAIN_VALUE_1        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_1_0_MAIN_SIGN_0         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_1_0_MAIN_VALUE_0        (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_3_2_MAIN                (pp_reg_base+0x8A4)
#define Y_SLOPE_3_2_MAIN_SIGN_3         (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_3_2_MAIN_VALUE_3        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_3_2_MAIN_SIGN_2         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_3_2_MAIN_VALUE_2        (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_5_4_MAIN                (pp_reg_base+0x8A8)
#define Y_SLOPE_5_4_MAIN_SIGN_5         (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_5_4_MAIN_VALUE_5        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_5_4_MAIN_SIGN_4         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_5_4_MAIN_VALUE_4        (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_7_6_MAIN                (pp_reg_base+0x8AC)
#define Y_SLOPE_7_6_MAIN_SIGN_7         (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_7_6_MAIN_VALUE_7        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_7_6_MAIN_SIGN_6         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_7_6_MAIN_VALUE_6        (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_9_8_MAIN                (pp_reg_base+0x8B0)
#define Y_SLOPE_9_8_MAIN_SIGN_9         (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_9_8_MAIN_VALUE_9        (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_9_8_MAIN_SIGN_8         (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_9_8_MAIN_VALUE_8        (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_11_10_MAIN              (pp_reg_base+0x8B4)
#define Y_SLOPE_11_10_MAIN_SIGN_11      (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_11_10_MAIN_VALUE_11     (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_11_10_MAIN_SIGN_10      (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_11_10_MAIN_VALUE_10     (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_13_12_MAIN              (pp_reg_base+0x8B8)
#define Y_SLOPE_13_12_MAIN_SIGN_13      (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_13_12_MAIN_VALUE_13     (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_13_12_MAIN_SIGN_12      (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_13_12_MAIN_VALUE_12     (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define Y_SLOPE_15_14_MAIN              (pp_reg_base+0x8BC)
#define Y_SLOPE_15_14_MAIN_SIGN_15      (0x01 << 23)        /* Fld(1, 23, AC_MSKB2) //23 */
#define Y_SLOPE_15_14_MAIN_VALUE_15     (0xFF << 16)        /* Fld(8, 16, AC_MSKB2) //22:16 */
#define Y_SLOPE_15_14_MAIN_SIGN_14      (0x01 << 7)         /* Fld(1, 7, AC_MSKB0) //7 */
#define Y_SLOPE_15_14_MAIN_VALUE_14     (0xFF << 0)         /* Fld(8, 0, AC_MSKB0) //6:0 */

#define S_G1_1_0_MAIN                   (pp_reg_base+0x8C0)
#define S_G1_1_0_MAIN_VALUE_1           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_1_0_MAIN_VALUE_0           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_3_2_MAIN                   (pp_reg_base+0x8C4)
#define S_G1_3_2_MAIN_VALUE_3           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_3_2_MAIN_VALUE_2           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_5_4_MAIN                   (pp_reg_base+0x8C8)
#define S_G1_5_4_MAIN_VALUE_5           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_5_4_MAIN_VALUE_4           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_7_6_MAIN                   (pp_reg_base+0x8CC)
#define S_G1_7_6_MAIN_VALUE_7           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_7_6_MAIN_VALUE_6           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_9_8_MAIN                   (pp_reg_base+0x8D0)
#define S_G1_9_8_MAIN_VALUE_9           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_9_8_MAIN_VALUE_8           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_11_10_MAIN                 (pp_reg_base+0x8D4)
#define S_G1_11_10_MAIN_VALUE_11        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_11_10_MAIN_VALUE_10        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_13_12_MAIN                 (pp_reg_base+0x8D8)
#define S_G1_13_12_MAIN_VALUE_13        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_13_12_MAIN_VALUE_12        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G1_15_14_MAIN                 (pp_reg_base+0x8DC)
#define S_G1_15_14_MAIN_VALUE_15        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G1_15_14_MAIN_VALUE_14        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_1_0_MAIN                   (pp_reg_base+0x8E0)
#define S_G2_1_0_MAIN_VALUE_1           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_1_0_MAIN_VALUE_0           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_3_2_MAIN                   (pp_reg_base+0x8E4)
#define S_G2_3_2_MAIN_VALUE_3           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_3_2_MAIN_VALUE_2           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_5_4_MAIN                   (pp_reg_base+0x8E8)
#define S_G2_5_4_MAIN_VALUE_5           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_5_4_MAIN_VALUE_4           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_7_6_MAIN                   (pp_reg_base+0x8EC)
#define S_G2_7_6_MAIN_VALUE_7           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_7_6_MAIN_VALUE_6           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_9_8_MAIN                   (pp_reg_base+0x8F0)
#define S_G2_9_8_MAIN_VALUE_9           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_9_8_MAIN_VALUE_8           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_11_10_MAIN                 (pp_reg_base+0x8F4)
#define S_G2_11_10_MAIN_VALUE_11        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_11_10_MAIN_VALUE_10        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_13_12_MAIN                 (pp_reg_base+0x8F8)
#define S_G2_13_12_MAIN_VALUE_13        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_13_12_MAIN_VALUE_12        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G2_15_14_MAIN                 (pp_reg_base+0x8FC)
#define S_G2_15_14_MAIN_VALUE_15        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G2_15_14_MAIN_VALUE_14        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_1_0_MAIN                   (pp_reg_base+0x900)
#define S_G3_1_0_MAIN_VALUE_1           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_1_0_MAIN_VALUE_0           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_3_2_MAIN                   (pp_reg_base+0x904)
#define S_G3_3_2_MAIN_VALUE_3           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_3_2_MAIN_VALUE_2           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_5_4_MAIN                   (pp_reg_base+0x908)
#define S_G3_5_4_MAIN_VALUE_5           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_5_4_MAIN_VALUE_4           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_7_6_MAIN                   (pp_reg_base+0x90C)
#define S_G3_7_6_MAIN_VALUE_7           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_7_6_MAIN_VALUE_6           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_9_8_MAIN                   (pp_reg_base+0x910)
#define S_G3_9_8_MAIN_VALUE_9           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_9_8_MAIN_VALUE_8           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_11_10_MAIN                 (pp_reg_base+0x914)
#define S_G3_11_10_MAIN_VALUE_11        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_11_10_MAIN_VALUE_10        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_13_12_MAIN                 (pp_reg_base+0x918)
#define S_G3_13_12_MAIN_VALUE_13        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_13_12_MAIN_VALUE_12        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_G3_15_14_MAIN                 (pp_reg_base+0x91C)
#define S_G3_15_14_MAIN_VALUE_15        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_G3_15_14_MAIN_VALUE_14        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_1_0_MAIN                   (pp_reg_base+0x920)
#define S_P1_1_0_MAIN_VALUE_1           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_1_0_MAIN_VALUE_0           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_3_2_MAIN                   (pp_reg_base+0x924)
#define S_P1_3_2_MAIN_VALUE_3           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_3_2_MAIN_VALUE_2           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_5_4_MAIN                   (pp_reg_base+0x55E278)
#define S_P1_5_4_MAIN_VALUE_5           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_5_4_MAIN_VALUE_4           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_7_6_MAIN                   (pp_reg_base+0x92C)
#define S_P1_7_6_MAIN_VALUE_7           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_7_6_MAIN_VALUE_6           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_9_8_MAIN                   (pp_reg_base+0x930)
#define S_P1_9_8_MAIN_VALUE_9           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_9_8_MAIN_VALUE_8           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_11_10_MAIN                 (pp_reg_base+0x934)
#define S_P1_11_10_MAIN_VALUE_11        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_11_10_MAIN_VALUE_10        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_13_12_MAIN                 (pp_reg_base+0x938)
#define S_P1_13_12_MAIN_VALUE_13        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_13_12_MAIN_VALUE_12        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P1_15_14_MAIN                 (pp_reg_base+0x93C)
#define S_P1_15_14_MAIN_VALUE_15        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P1_15_14_MAIN_VALUE_14        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_1_0_MAIN                   (pp_reg_base+0x940)
#define S_P2_1_0_MAIN_VALUE_1           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_1_0_MAIN_VALUE_0           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_3_2_MAIN                   (pp_reg_base+0x944)
#define S_P2_3_2_MAIN_VALUE_3           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_3_2_MAIN_VALUE_2           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_5_4_MAIN                   (pp_reg_base+0x948)
#define S_P2_5_4_MAIN_VALUE_5           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_5_4_MAIN_VALUE_4           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_7_6_MAIN                   (pp_reg_base+0x94C)
#define S_P2_7_6_MAIN_VALUE_7           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_7_6_MAIN_VALUE_6           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_9_8_MAIN                   (pp_reg_base+0x950)
#define S_P2_9_8_MAIN_VALUE_9           (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_9_8_MAIN_VALUE_8           (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_11_10_MAIN                 (pp_reg_base+0x954)
#define S_P2_11_10_MAIN_VALUE_11        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_11_10_MAIN_VALUE_10        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_13_12_MAIN                 (pp_reg_base+0x958)
#define S_P2_13_12_MAIN_VALUE_13        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_13_12_MAIN_VALUE_12        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_P2_15_14_MAIN                 (pp_reg_base+0x95C)
#define S_P2_15_14_MAIN_VALUE_15        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_P2_15_14_MAIN_VALUE_14        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_1_0_MAIN                 (pp_reg_base+0x960)
#define S_Y0_G_1_0_MAIN_VALUE_1         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_1_0_MAIN_VALUE_0         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_3_2_MAIN                 (pp_reg_base+0x964)
#define S_Y0_G_3_2_MAIN_VALUE_3         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_3_2_MAIN_VALUE_2         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_5_4_MAIN                 (pp_reg_base+0x968)
#define S_Y0_G_5_4_MAIN_VALUE_5         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_5_4_MAIN_VALUE_4         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_7_6_MAIN                 (pp_reg_base+0x96C)
#define S_Y0_G_7_6_MAIN_VALUE_7         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_7_6_MAIN_VALUE_6         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_9_8_MAIN                 (pp_reg_base+0x970)
#define S_Y0_G_9_8_MAIN_VALUE_9         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_9_8_MAIN_VALUE_8         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_11_10_MAIN               (pp_reg_base+0x974)
#define S_Y0_G_11_10_MAIN_VALUE_11      (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_11_10_MAIN_VALUE_10      (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_13_12_MAIN               (pp_reg_base+0x978)
#define S_Y0_G_13_12_MAIN_VALUE_13      (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y0_G_13_12_MAIN_VALUE_12      (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y0_G_15_14_MAIN               (pp_reg_base+0x97C)
#define S_Y0_G_15_14_MAIN_VALUE_15      (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */

#define S_Y64_G_1_0_MAIN                (pp_reg_base+0x980)
#define S_Y64_G_1_0_MAIN_VALUE_1        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_1_0_MAIN_VALUE_0        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_3_2_MAIN                (pp_reg_base+0x984)
#define S_Y64_G_3_2_MAIN_VALUE_3        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_3_2_MAIN_VALUE_2        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_5_4_MAIN                (pp_reg_base+0x988)
#define S_Y64_G_5_4_MAIN_VALUE_5        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_5_4_MAIN_VALUE_4        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_7_6_MAIN                (pp_reg_base+0x98C)
#define S_Y64_G_7_6_MAIN_VALUE_7        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_7_6_MAIN_VALUE_6        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_9_8_MAIN                (pp_reg_base+0x990)
#define S_Y64_G_9_8_MAIN_VALUE_9        (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_9_8_MAIN_VALUE_8        (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_11_10_MAIN              (pp_reg_base+0x994)
#define S_Y64_G_11_10_MAIN_VALUE_11     (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_11_10_MAIN_VALUE_10     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_13_12_MAIN              (pp_reg_base+0x998)
#define S_Y64_G_13_12_MAIN_VALUE_13     (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_13_12_MAIN_VALUE_12     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y64_G_15_14_MAIN              (pp_reg_base+0x99C)
#define S_Y64_G_15_14_MAIN_VALUE_15     (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y64_G_15_14_MAIN_VALUE_14     (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_1_0_MAIN               (pp_reg_base+0x9A0)
#define S_Y128_G_1_0_MAIN_VALUE_1       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_1_0_MAIN_VALUE_0       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_3_2_MAIN               (pp_reg_base+0x9A4)
#define S_Y128_G_3_2_MAIN_VALUE_3       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_3_2_MAIN_VALUE_2       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_5_4_MAIN               (pp_reg_base+0x9A8)
#define S_Y128_G_5_4_MAIN_VALUE_5       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_5_4_MAIN_VALUE_4       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_7_6_MAIN               (pp_reg_base+0x9AC)
#define S_Y128_G_7_6_MAIN_VALUE_7       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_7_6_MAIN_VALUE_6       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_9_8_MAIN               (pp_reg_base+0x9B0)
#define S_Y128_G_9_8_MAIN_VALUE_9       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_9_8_MAIN_VALUE_8       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_11_10_MAIN             (pp_reg_base+0x9B4)
#define S_Y128_G_11_10_MAIN_VALUE_11    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_11_10_MAIN_VALUE_10    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_13_12_MAIN             (pp_reg_base+0x9B8)
#define S_Y128_G_13_12_MAIN_VALUE_13    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_13_12_MAIN_VALUE_12    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y128_G_15_14_MAIN             (pp_reg_base+0x9BC)
#define S_Y128_G_15_14_MAIN_VALUE_15    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y128_G_15_14_MAIN_VALUE_14    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_1_0_MAIN               (pp_reg_base+0x9C0)
#define S_Y192_G_1_0_MAIN_VALUE_1       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_1_0_MAIN_VALUE_0       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_3_2_MAIN               (pp_reg_base+0x9C4)
#define S_Y192_G_3_2_MAIN_VALUE_3       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_3_2_MAIN_VALUE_2       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_5_4_MAIN               (pp_reg_base+0x9C8)
#define S_Y192_G_5_4_MAIN_VALUE_5       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_5_4_MAIN_VALUE_4       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_7_6_MAIN               (pp_reg_base+0x9CC)
#define S_Y192_G_7_6_MAIN_VALUE_7       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_7_6_MAIN_VALUE_6       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_9_8_MAIN               (pp_reg_base+0x9D0)
#define S_Y192_G_9_8_MAIN_VALUE_9       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_9_8_MAIN_VALUE_8       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_11_10_MAIN             (pp_reg_base+0x9D4)
#define S_Y192_G_11_10_MAIN_VALUE_11    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_11_10_MAIN_VALUE_10    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_13_12_MAIN             (pp_reg_base+0x9D8)
#define S_Y192_G_13_12_MAIN_VALUE_13    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_13_12_MAIN_VALUE_12    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y192_G_15_14_MAIN             (pp_reg_base+0x9DC)
#define S_Y192_G_15_14_MAIN_VALUE_15    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y192_G_15_14_MAIN_VALUE_14    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_1_0_MAIN               (pp_reg_base+0x9E0)
#define S_Y256_G_1_0_MAIN_VALUE_1       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_1_0_MAIN_VALUE_0       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_3_2_MAIN               (pp_reg_base+0x9E4)
#define S_Y256_G_3_2_MAIN_VALUE_3       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_3_2_MAIN_VALUE_2       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_5_4_MAIN               (pp_reg_base+0x9E8)
#define S_Y256_G_5_4_MAIN_VALUE_5       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_5_4_MAIN_VALUE_4       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_7_6_MAIN               (pp_reg_base+0x9EC)
#define S_Y256_G_7_6_MAIN_VALUE_7       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_7_6_MAIN_VALUE_6       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_9_8_MAIN               (pp_reg_base+0x9F0)
#define S_Y256_G_9_8_MAIN_VALUE_9       (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_9_8_MAIN_VALUE_8       (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_11_10_MAIN             (pp_reg_base+0x9F4)
#define S_Y256_G_11_10_MAIN_VALUE_11    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_11_10_MAIN_VALUE_10    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_13_12_MAIN             (pp_reg_base+0x9F8)
#define S_Y256_G_13_12_MAIN_VALUE_13    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_13_12_MAIN_VALUE_12    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define S_Y256_G_15_14_MAIN             (pp_reg_base+0x9FC)
#define S_Y256_G_15_14_MAIN_VALUE_15    (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define S_Y256_G_15_14_MAIN_VALUE_14    (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define H_FTN_1_0_MAIN                  (pp_reg_base+0xA00)
#define H_FTN_1_0_MAIN_SIGN_1           (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_1_0_MAIN_VALUE_1          (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_1_0_MAIN_SIGN_0           (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_1_0_MAIN_VALUE_0          (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_3_2_MAIN                  (pp_reg_base+0xA04)
#define H_FTN_3_2_MAIN_SIGN_3           (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_3_2_MAIN_VALUE_3          (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_3_2_MAIN_SIGN_2           (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_3_2_MAIN_VALUE_2          (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_5_4_MAIN                  (pp_reg_base+0xA08)
#define H_FTN_5_4_MAIN_SIGN_5           (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_5_4_MAIN_VALUE_5          (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_5_4_MAIN_SIGN_4           (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_5_4_MAIN_VALUE_4          (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_7_6_MAIN                  (pp_reg_base+0xA0C)
#define H_FTN_7_6_MAIN_SIGN_7           (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_7_6_MAIN_VALUE_7          (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_7_6_MAIN_SIGN_6           (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_7_6_MAIN_VALUE_6          (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_9_8_MAIN                  (pp_reg_base+0xA10)
#define H_FTN_9_8_MAIN_SIGN_9           (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_9_8_MAIN_VALUE_9          (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_9_8_MAIN_SIGN_8           (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_9_8_MAIN_VALUE_8          (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_11_10_MAIN                (pp_reg_base+0xA14)
#define H_FTN_11_10_MAIN_SIGN_11        (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_11_10_MAIN_VALUE_11       (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_11_10_MAIN_SIGN_10        (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_11_10_MAIN_VALUE_10       (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_13_12_MAIN                (pp_reg_base+0xA18)
#define H_FTN_13_12_MAIN_SIGN_13        (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_13_12_MAIN_VALUE_13       (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_13_12_MAIN_SIGN_12        (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_13_12_MAIN_VALUE_12       (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define H_FTN_15_14_MAIN                (pp_reg_base+0xA1C)
#define H_FTN_15_14_MAIN_SIGN_15        (0x01 << 22)        /* Fld(1, 22, AC_MSKB2) //22 */
#define H_FTN_15_14_MAIN_VALUE_15       (0x7F << 16)        /* Fld(7, 16, AC_MSKB2) //21:16 */
#define H_FTN_15_14_MAIN_SIGN_14        (0x01 << 6)         /* Fld(1, 6, AC_MSKB0) //6 */
#define H_FTN_15_14_MAIN_VALUE_14       (0x7F << 0)         /* Fld(7, 0, AC_MSKB0) //5:0 */

#define COLOR_P_OPTION                  (pp_reg_base+0xA28)
#define C2P_SX2                         (0x01 << 8)         /* Fld(1, 8, AC_MSKB1) //22 */
#define FILL_LSB                        (0x01 << 1)         /* Fld(1, 1, AC_MSKB0) //22 */
#define ORIGINAL_CLIP                   (0x01 << 0)         /* Fld(1, 0, AC_MSKB0) //21:16 */

#define SAT_HIST_CFG_MAIN_3             (pp_reg_base+0xB60)
#define BOUND_4                         (0xFF << 24)        /* Fld(8, 24, AC_FULLB3) //31:24 */
#define BOUND_3                         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define BOUND_2                         (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define BOUND_1                         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */

#define SAT_HIST_CFG_MAIN               (pp_reg_base+0xB64)
#define BOUND_7                         (0xFF << 16)        /* Fld(8, 16, AC_FULLB2) //23:16 */
#define BOUND_6                         (0xFF << 8)         /* Fld(8, 8, AC_FULLB1) //15:8 */
#define BOUND_5                         (0xFF << 0)         /* Fld(8, 0, AC_FULLB0) //7:0 */


#define SAT_HIST_X_CFG_MAIN             (pp_reg_base+0xB68)
#define WINDOW_X_END                    (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define WINDOW_X_START                  (0x7FF << 0)        /* Fld(11, 0, AC_MSKW10) //10:0 */

#define SAT_HIST_Y_CFG_MAIN             (pp_reg_base+0xB6C)
#define WINDOW_Y_END                    (0x7FF << 16)       /* Fld(11, 16, AC_MSKW32) //26:16 */
#define WINDOW_Y_START                  (0x7FF << 0)        /* Fld(11, 0, AC_MSKW10) //10:0 */


#define SAT_HIST_1_0_MAIN               (pp_reg_base+0xB70)
#define SAT_HIST_1                      (0xFFFF << 16)      /* Fld(16, 16, AC_FULLW32) //31:16 */
#define SAT_HIST_0                      (0xFFFF << 0)       /* Fld(16, 0, AC_FULLW10) //15:0 */

#define SAT_HIST_3_2_MAIN               (pp_reg_base+0xB74)
#define SAT_HIST_3                      (0xFFFF << 16)      /* Fld(16, 16, AC_FULLW32) //31:16 */
#define SAT_HIST_2                      (0xFFFF << 0)       /* Fld(16, 0, AC_FULLW10) //15:0 */

#define SAT_HIST_5_4_MAIN               (pp_reg_base+0xB78)
#define SAT_HIST_5                      (0xFFFF << 16)      /* Fld(16, 16, AC_FULLW32) //31:16 */
#define SAT_HIST_4                      (0xFFFF << 0)       /* Fld(16, 0, AC_FULLW10) //15:0 */

#define SAT_HIST_7_6_MAIN               (pp_reg_base+0xB7C)
#define SAT_HIST_7                      (0xFFFF << 16)      /* Fld(16, 16, AC_FULLW32) //31:16 */
#define SAT_HIST_6                      (0xFFFF << 0)       /* Fld(16, 0, AC_FULLW10) //15:0 */

#define Y_FTN_1_0                       (pp_reg_base+0x840)
#define Y_FTN_0                         (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */
#define Y_FTN_1                         (0x3FF << 16)       /* Fld(10, 16, AC_MSKW32) //25:16 */
#define Y_FTN_32_                       (pp_reg_base+0x880)
#define Y_FTN_32                        (0x3FF << 0)        /* Fld(10, 0, AC_MSKW10) //9:0 */


/* ///////////////////////////////////////////////////////////////////////// */

#define HAL_POST_LUMA_MAIN_REG       (pp_reg_base + 0x840)
#define HAL_POST_LUMA_MAIN_REG_NUM     (0x44/4)

struct _HAL_POST_LUMA_MAIN_FIELD_T {
	/* DWORD - 000 */
	uint32_t u2Y_FTN0:16;
	uint32_t u2Y_FTN1:16;

	/* DWORD - 004 */
	uint32_t u2Y_FTN2:16;
	uint32_t u2Y_FTN3:16;

	/* DWORD - 008 */
	uint32_t u2Y_FTN4:16;
	uint32_t u2Y_FTN5:16;

	/* DWORD - 00C */
	uint32_t u2Y_FTN6:16;
	uint32_t u2Y_FTN7:16;

	/* DWORD - 010 */
	uint32_t u2Y_FTN8:16;
	uint32_t u2Y_FTN9:16;

	/* DWORD - 014 */
	uint32_t u2Y_FTN10:16;
	uint32_t u2Y_FTN11:16;

	/* DWORD - 018 */
	uint32_t u2Y_FTN12:16;
	uint32_t u2Y_FTN13:16;

	/* DWORD - 01C */
	uint32_t u2Y_FTN14:16;
	uint32_t u2Y_FTN15:16;

	/* DWORD - 020 */
	uint32_t u2Y_FTN16:16;
	uint32_t u2Y_FTN17:16;

	/* DWORD - 024 */
	uint32_t u2Y_FTN18:16;
	uint32_t u2Y_FTN19:16;

	/* DWORD - 028 */
	uint32_t u2Y_FTN20:16;
	uint32_t u2Y_FTN21:16;

	/* DWORD - 02C */
	uint32_t u2Y_FTN22:16;
	uint32_t u2Y_FTN23:16;

	/* DWORD - 030 */
	uint32_t u2Y_FTN24:16;
	uint32_t u2Y_FTN25:16;

	/* DWORD - 034 */
	uint32_t u2Y_FTN26:16;
	uint32_t u2Y_FTN27:16;

	/* DWORD - 038 */
	uint32_t u2Y_FTN28:16;
	uint32_t u2Y_FTN29:16;

	/* DWORD - 03C */
	uint32_t u2Y_FTN30:16;
	uint32_t u2Y_FTN31:16;

	/* DWORD - 040 */
	uint32_t u2Y_FTN32:16;
	 uint32_t:16;

};

union HAL_POST_LUMA_MAIN_UNION_T {

	uint32_t au4Reg[HAL_POST_LUMA_MAIN_REG_NUM];
	struct _HAL_POST_LUMA_MAIN_FIELD_T rField;
};

#endif				/* #ifdef _PP_HW_H_ */
