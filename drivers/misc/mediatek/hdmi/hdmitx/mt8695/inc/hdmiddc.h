/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef __hdmiddc_h__
#define __hdmiddc_h__
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT


#define EDID_BLOCK_LEN      128
#define EDID_SIZE 512

#define SIF1_CLOK 288		/* 26M/(v) = 100Khz */
#define DDC2_CLOK 1100		/* BIM=400M/(v*4) = 90Khz */
#define DDC2_CLOK_EDID 1600	/* BIM=400M/(v*4) = 62.5Khz *//* for HF1-55 */

#define EDID_ID     0x50	/* 0xA0 */
#define EDID_ID1    0x51	/* 0xA2 */

#define EDID_ADDR_HEADER                      0x00
#define EDID_ADDR_VERSION                     0x12
#define EDID_ADDR_REVISION                    0x13
#define EDID_IMAGE_HORIZONTAL_SIZE            0x15
#define EDID_IMAGE_VERTICAL_SIZE              0x16
#define EDID_ADDR_FEATURE_SUPPORT             0x18
#define EDID_ADDR_TIMING_DSPR_1               0x36
#define EDID_ADDR_TIMING_DSPR_2               0x48
#define EDID_ADDR_MONITOR_DSPR_1              0x5A
#define EDID_ADDR_MONITOR_DSPR_2              0x6C
#define EDID_ADDR_EXT_BLOCK_FLAG              0x7E
#define EDID_ADDR_EXTEND_BYTE3                0x03	/* EDID address: 0x83 */
						   /* for ID receiver if RGB, YCbCr 4:2:2 or 4:4:4 */
/* Extension Block 1: */
#define EXTEDID_ADDR_TAG                      0x00
#define EXTEDID_ADDR_REVISION                 0x01
#define EXTEDID_ADDR_OFST_TIME_DSPR           0x02


typedef enum {
	SIF_8_BIT,		/* /< [8 bits data address.] */
	SIF_16_BIT,		/* /< [16 bits data address.] */
} SIF_BIT_T;

typedef enum {
	SIF_NORMAL,		/* /< [Normal, always select this.] */
	SIF_OTHER,		/* /< [Other.] */
} SIF_TYPE_T;
  /* / [Sif control mode select.] */
typedef struct _SIF_MODE_T {
	SIF_BIT_T eBit;		/* /< [The data address type. ] */
	SIF_TYPE_T eType;	/* /< [The control mode.] */
} SIF_MODE_T;

extern unsigned char fgDDCDataRead(unsigned char bDevice, unsigned char bData_Addr,
				   unsigned char bDataCount, unsigned char *prData);
extern unsigned char fgDDCDataWrite(unsigned char bDevice, unsigned char bData_Addr,
				    unsigned char bDataCount, unsigned char *prData);
#endif
#endif
