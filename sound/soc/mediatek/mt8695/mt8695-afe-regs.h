/*
 * mt8695_afe_regs.h  --  Mediatek audio register definitions
 *
 * Copyright (c) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

 #ifndef MT8695_AFE_REGS_H_
 #define MT8695_AFE_REGS_H_

#include <linux/bitops.h>

#define AUDIO_TOP_CON0  (0x0000)
	#define AUD_TCON0_PDN_TML		BIT(27)
	#define AUD_TCON0_PDN_APLL		BIT(23)
	#define AUD_TCON0_PDN_SPDF_CK		BIT(21)
	#define AUD_TCON0_PDN_HDMI_CK		BIT(20)
	#define AUD_IEC_8CHI2S_CK_SEL_MASK	(1 << 15)
	#define AUD_IEC_8CHI2S_CK_SEL_APLL	(0)
	#define AUD_IEC_8CHI2S_CK_SEL_APL2	(1 << 15)
	#define AUD_TCON0_PDN_SPDIFIN_TUNER_APLL_CK	BIT(10)
	#define AUD_TCON0_PDN_SPDIFIN_TUNER_DBG_CK	BIT(11)
	#define AUD_TCON0_PDN_LRCK_CNT	BIT(4)
	#define AUD_TCON0_PDN_AFE		BIT(2)
#define AUDIO_TOP_CON1  (0x0004)
#define AUDIO_TOP_CON2  (0x0008)
#define HDMI_BCK_DIV_POS 8
#define HDMI_BCK_DIV_MASK (0x3f << HDMI_BCK_DIV_POS)
#define AUDIO_TOP_CON3  (0x000C)
#define AUDIO_TOP_CON4  (0x0010)
	#define AUD_TCON4_PDN_ASRCI4	(0x1<<27)
	#define AUD_TCON4_PDN_ASRCI3	(0x1<<26)
	#define AUD_TCON4_PDN_PCMIF		(0x1<<24)
	#define AUD_TCON4_PDN_AFE_CONN	(0x1<<23)
	#define AUD_TCON4_PDN_A2SYS		(0x1<<22)
	#define AUD_TCON4_PDN_A1SYS		(0x1<<21)
	#define AUD_TCON4_PDN_INTDIR	(0x1<<20)
	#define AUD_TCON4_PDN_MULTI_IN	(0x1<<19)
	#define AUD_TCON4_PDN_ASRCI2	(0x1<<13)
	#define AUD_TCON4_PDN_ASRCI1	(0x1<<12)
	#define AUD_TCON4_PDN_I2SO6		(0x1<<11)
	#define AUD_TCON4_PDN_I2SO5		(0x1<<10)
	#define AUD_TCON4_PDN_I2SO4		(0x1<<9)
	#define AUD_TCON4_PDN_I2SO3		(0x1<<8)
	#define AUD_TCON4_PDN_I2SO2		(0x1<<7)
	#define AUD_TCON4_PDN_I2SO1		(0x1<<6)
	#define AUD_TCON4_PDN_I2SIN4	(0x1<<3)
	#define AUD_TCON4_PDN_I2SIN3	(0x1<<2)
	#define AUD_TCON4_PDN_I2SIN2	(0x1<<1)
	#define AUD_TCON4_PDN_I2SIN1	(0x1<<0)

#define AUDIO_TOP_CON5  (0x0014)
	#define AUD_TCON5_PDN_MEMIF_DLMCH	(0x1<<12)
	#define AUD_TCON5_PDN_MEMIF_DL6		(0x1<<11)
	#define AUD_TCON5_PDN_MEMIF_DL2		(0x1<<7)
	#define AUD_TCON5_PDN_MEMIF_DL1		(0x1<<6)
	#define AUD_TCON5_PDN_MEMIF_UL5		(0x1<<4)
	#define AUD_TCON5_PDN_MEMIF_UL4		(0x1<<3)
	#define AUD_TCON5_PDN_MEMIF_UL3		(0x1<<2)
	#define AUD_TCON5_PDN_MEMIF_UL2		(0x1<<1)
	#define AUD_TCON5_PDN_MEMIF_UL1		(0x1<<0)

#define AFE_DAC_CON0    (0x1200)
#define TDM_IN_ON_MASK      (0x1<<25)
#define TDM_IN_OFF          (0x0<<25)
#define TDM_IN_ON           (0x1<<25)
#define DMA_ON_MASK      (0x1<<24)
#define DMA_OFF          (0x0<<24)
#define DMA_ON           (0x1<<24)
#define ULMCH_ON_MASK    (0x1<<16)
#define ULMCH_OFF        (0x0<<16)
#define ULMCH_ON         (0x1<<16)
#define DL6_ON_MASK      (0x1<<6)
#define DL6_OFF          (0x0<<6)
#define DL6_ON           (0x1<<6)
#define DL5_ON_MASK      (0x1<<5)
#define DL5_OFF          (0x0<<5)
#define DL5_ON           (0x1<<5)
#define DL4_ON_MASK      (0x1<<4)
#define DL4_OFF          (0x0<<4)
#define DL4_ON           (0x1<<4)
#define DL3_ON_MASK      (0x1<<3)
#define DL3_OFF          (0x0<<3)
#define DL3_ON           (0x1<<3)
#define DL2_ON_MASK      (0x1<<2)
#define DL2_OFF          (0x0<<2)
#define DL2_ON           (0x1<<2)
#define DL1_ON_MASK      (0x1<<1)
#define DL1_OFF          (0x0<<1)
#define DL1_ON           (0x1<<1)
#define AFE_ON_MASK      (0x1<<0)
#define AFE_OFF          (0x0<<0)
#define AFE_ON           (0x1<<0)
#define AFE_DAC_CON1    (0x1204)
#define DL6_MODE_MASK     (0x1F<<25)  /* bit[29:25] */
#define DL6_MODE_8000HZ     (0x0<<25)
#define DL6_MODE_12000HZ    (0x1<<25)
#define DL6_MODE_16000HZ    (0x2<<25)
#define DL6_MODE_24000HZ    (0x3<<25)
#define DL6_MODE_32000HZ    (0x4<<25)
#define DL6_MODE_48000HZ    (0x5<<25)
#define DL6_MODE_96000HZ    (0x6<<25)
#define DL6_MODE_192000HZ   (0x7<<25)
#define DL6_MODE_384000HZ   (0x8<<25)
#define DL6_MODE_7350HZ     (0x10<<25)
#define DL6_MODE_11025HZ    (0x11<<25)
#define DL6_MODE_14700HZ    (0x12<<25)
#define DL6_MODE_22050HZ    (0x13<<25)
#define DL6_MODE_28000HZ    (0x14<<25)
#define DL6_MODE_44100HZ    (0x15<<25)
#define DL6_MODE_88200HZ    (0x16<<25)
#define DL6_MODE_176400HZ   (0x17<<25)
#define DL6_MODE_352800HZ   (0x18<<25)
#define DL5_MODE_MASK     (0x1F<<20)  /* bit[24:20] */
#define DL5_MODE_8000HZ     (0x0<<20)
#define DL5_MODE_12000HZ    (0x1<<20)
#define DL5_MODE_16000HZ    (0x2<<20)
#define DL5_MODE_24000HZ    (0x3<<20)
#define DL5_MODE_32000HZ    (0x4<<20)
#define DL5_MODE_48000HZ    (0x5<<20)
#define DL5_MODE_96000HZ    (0x6<<20)
#define DL5_MODE_192000HZ   (0x7<<20)
#define DL5_MODE_384000HZ   (0x8<<20)
#define DL5_MODE_7350HZ     (0x10<<20)
#define DL5_MODE_11025HZ    (0x11<<20)
#define DL5_MODE_14700HZ    (0x12<<20)
#define DL5_MODE_22050HZ    (0x13<<20)
#define DL5_MODE_28000HZ    (0x14<<20)
#define DL5_MODE_44100HZ    (0x15<<20)
#define DL5_MODE_88200HZ    (0x16<<20)
#define DL5_MODE_176400HZ   (0x17<<20)
#define DL5_MODE_352800HZ   (0x18<<20)
#define DL4_MODE_MASK     (0x1F<<15)  /* bit[19:15] */
#define DL4_MODE_8000HZ     (0x0<<15)
#define DL4_MODE_12000HZ    (0x1<<15)
#define DL4_MODE_16000HZ    (0x2<<15)
#define DL4_MODE_24000HZ    (0x3<<15)
#define DL4_MODE_32000HZ    (0x4<<15)
#define DL4_MODE_48000HZ    (0x5<<15)
#define DL4_MODE_96000HZ    (0x6<<15)
#define DL4_MODE_192000HZ   (0x7<<15)
#define DL4_MODE_384000HZ   (0x8<<15)
#define DL4_MODE_7350HZ     (0x10<<15)
#define DL4_MODE_11025HZ    (0x11<<15)
#define DL4_MODE_14700HZ    (0x12<<15)
#define DL4_MODE_22050HZ    (0x13<<15)
#define DL4_MODE_28000HZ    (0x14<<15)
#define DL4_MODE_44100HZ    (0x15<<15)
#define DL4_MODE_88200HZ    (0x16<<15)
#define DL4_MODE_176400HZ   (0x17<<15)
#define DL4_MODE_352800HZ   (0x18<<15)
#define DL3_MODE_MASK     (0x1F<<10)  /* bit[14:10] */
#define DL3_MODE_8000HZ     (0x0<<10)
#define DL3_MODE_12000HZ    (0x1<<10)
#define DL3_MODE_16000HZ    (0x2<<10)
#define DL3_MODE_24000HZ    (0x3<<10)
#define DL3_MODE_32000HZ    (0x4<<10)
#define DL3_MODE_48000HZ    (0x5<<10)
#define DL3_MODE_96000HZ    (0x6<<10)
#define DL3_MODE_192000HZ   (0x7<<10)
#define DL3_MODE_384000HZ   (0x8<<10)
#define DL3_MODE_7350HZ     (0x10<<10)
#define DL3_MODE_11025HZ    (0x11<<10)
#define DL3_MODE_14700HZ    (0x12<<10)
#define DL3_MODE_22050HZ    (0x13<<10)
#define DL3_MODE_28000HZ    (0x14<<10)
#define DL3_MODE_44100HZ    (0x15<<10)
#define DL3_MODE_88200HZ    (0x16<<10)
#define DL3_MODE_176400HZ   (0x17<<10)
#define DL3_MODE_352800HZ   (0x18<<10)
#define DL2_MODE_MASK     (0x1F<<5)  /* bit[9:5] */
#define DL2_MODE_8000HZ     (0x0<<5)
#define DL2_MODE_12000HZ    (0x1<<5)
#define DL2_MODE_16000HZ    (0x2<<5)
#define DL2_MODE_24000HZ    (0x3<<5)
#define DL2_MODE_32000HZ    (0x4<<5)
#define DL2_MODE_48000HZ    (0x5<<5)
#define DL2_MODE_96000HZ    (0x6<<5)
#define DL2_MODE_192000HZ   (0x7<<5)
#define DL2_MODE_384000HZ   (0x8<<5)
#define DL2_MODE_7350HZ     (0x10<<5)
#define DL2_MODE_11025HZ    (0x11<<5)
#define DL2_MODE_14700HZ    (0x12<<5)
#define DL2_MODE_22050HZ    (0x13<<5)
#define DL2_MODE_28000HZ    (0x14<<5)
#define DL2_MODE_44100HZ    (0x15<<5)
#define DL2_MODE_88200HZ    (0x16<<5)
#define DL2_MODE_176400HZ   (0x17<<5)
#define DL2_MODE_352800HZ   (0x18<<5)
#define DL1_MODE_MASK     (0x1F<<0)  /* bit[4:0] */
#define DL1_MODE_8000HZ     (0x0<<0)
#define DL1_MODE_12000HZ    (0x1<<0)
#define DL1_MODE_16000HZ    (0x2<<0)
#define DL1_MODE_24000HZ    (0x3<<0)
#define DL1_MODE_32000HZ    (0x4<<0)
#define DL1_MODE_48000HZ    (0x5<<0)
#define DL1_MODE_96000HZ    (0x6<<0)
#define DL1_MODE_192000HZ   (0x7<<0)
#define DL1_MODE_384000HZ   (0x8<<0)
#define DL1_MODE_7350HZ     (0x10<<0)
#define DL1_MODE_11025HZ    (0x11<<0)
#define DL1_MODE_14700HZ    (0x12<<0)
#define DL1_MODE_22050HZ    (0x13<<0)
#define DL1_MODE_28000HZ    (0x14<<0)
#define DL1_MODE_44100HZ    (0x15<<0)
#define DL1_MODE_88200HZ    (0x16<<0)
#define DL1_MODE_176400HZ   (0x17<<0)
#define DL1_MODE_352800HZ   (0x18<<0)

#define AFE_DAC_CON2    (0x1208)
#define VUL_MODE_MASK    (0x1F<<0)  /* bit[4:0] */
#define UL2_MODE_MASK    (0x1F<<5)  /* bit[9:5] */
#define UL3_MODE_MASK    (0x1F<<10)  /* bit[14:10] */
#define UL4_MODE_MASK    (0x1F<<15)  /* bit[19:15] */
#define UL5_MODE_MASK    (0x1F<<20)  /* bit[24:20] */
#define UL6_MODE_MASK    (0x1F<<25)  /* bit[29:25] */
#define DAI_MODE_MASK    (0x1<<30)
#define MOD_PCM_MODE_MASK    (0x1<<31)

#define AFE_DAC_CON3    (0x120C)
#define DSDW_MSB_OR_LSB_FIRST_MASK (0x1<<31)
#define DSDR_MSB_OR_LSB_FIRST_MASK (0x1<<30)
#define DL6_MSB_OR_LSB_FIRST_MASK (0x1<<29)
#define DL5_MSB_OR_LSB_FIRST_MASK (0x1<<28)
#define DL4_MSB_OR_LSB_FIRST_MASK (0x1<<27)
#define DL3_MSB_OR_LSB_FIRST_MASK (0x1<<26)
#define DL2_MSB_OR_LSB_FIRST_MASK (0x1<<25)
#define DL1_MSB_OR_LSB_FIRST_MASK (0x1<<24)
#define DSDR_DATA_MASK  (0x1<<23)
#define ARB1_DATA_MASK  (0x1<<22)
#define DL6_DATA_MASK   (0x1<<21)
#define DL6_DATA_STEREO (0x0<<21)
#define DL6_DATA_MONO   (0x1<<21)
#define DL5_DATA_MASK   (0x1<<20)
#define DL5_DATA_STEREO (0x0<<20)
#define DL5_DATA_MONO   (0x1<<20)
#define DL4_DATA_MASK   (0x1<<19)
#define DL4_DATA_STEREO (0x0<<19)
#define DL4_DATA_MONO   (0x1<<19)
#define DL3_DATA_MASK   (0x1<<18)
#define DL3_DATA_STEREO (0x0<<18)
#define DL3_DATA_MONO   (0x1<<18)
#define DL2_DATA_MASK   (0x1<<17)
#define DL2_DATA_STEREO (0x0<<17)
#define DL2_DATA_MONO   (0x1<<17)
#define DL1_DATA_MASK   (0x1<<16)
#define DL1_DATA_STEREO (0x0<<16)
#define DL1_DATA_MONO   (0x1<<16)
#define ARB1_MODE_MASK  (0x1F<<10) /* bit[14:10] */
#define AWB2_MODE_MASK  (0x1F<<5)  /* bit[9:5] */
#define AWB_MODE_MASK   (0x1F<<0)  /* bit[4:0] */

#define AFE_DAC_CON4    (0x1210)
#define VUL_DATA_MASK     (0x1<<0)
#define VUL_R_MONO_MASK   (0x1<<1)
#define UL2_DATA_MASK     (0x1<<2)
#define UL2_R_MONO_MASK   (0x1<<3)
#define UL3_DATA_MASK     (0x1<<4)
#define UL3_R_MONO_MASK   (0x1<<5)
#define UL4_DATA_MASK     (0x1<<6)
#define UL4_R_MONO_MASK   (0x1<<7)
#define UL5_DATA_MASK     (0x1<<8)
#define UL5_R_MONO_MASK   (0x1<<9)
#define UL6_DATA_MASK     (0x1<<10)
#define UL6_R_MONO_MASK   (0x1<<11)
#define ULMCH_DATA_MASK   (0x1<<12)
#define ULMCH_R_MONO_MASK (0x1<<13)
#define DAI_DUP_WR_MASK       (0x1<<14)
#define DAI_DUP_WR_NORMAL     (0x0<<14)
#define DAI_DUP_WR_DUPLICATED (0x1<<14)
#define MOD_PCM_DUP_WR_MASK       (0x1<<15)
#define MOD_PCM_DUP_WR_NORMAL     (0x0<<15)
#define MOD_PCM_DUP_WR_DUPLICATED (0x1<<15)
#define AWB_DATA_MASK     (0x1<<16)
#define AWB_R_MONO_MASK   (0x1<<17)
#define AWB2_DATA_MASK    (0x1<<18)
#define AWB2_R_MONO_MASK  (0x1<<19)
#define DSDW_DATA_MASK    (0x1<<20)
#define DSDW_R_MONO_MASK  (0x1<<21)
#define DSDW_DSD_WIDTH_MASK  (0x3<<22)
#define VUL_MSB_OR_LSB_FIRST_MASK (0x1<<24)
#define UL2_MSB_OR_LSB_FIRST_MASK (0x1<<25)
#define UL3_MSB_OR_LSB_FIRST_MASK (0x1<<26)
#define UL4_MSB_OR_LSB_FIRST_MASK (0x1<<27)
#define UL5_MSB_OR_LSB_FIRST_MASK (0x1<<28)
#define UL6_MSB_OR_LSB_FIRST_MASK (0x1<<29)
#define AWB_MSB_OR_LSB_FIRST_MASK (0x1<<30)
#define AWB2_MSB_OR_LSB_FIRST_MASK (0x1<<31)


#define AFE_DAC_CON5    (0x1214)
#define DL1_DSD_WIDTH_MASK  (0x3<<0)
#define DL2_DSD_WIDTH_MASK  (0x3<<2)
#define DL3_DSD_WIDTH_MASK  (0x3<<4)
#define DL4_DSD_WIDTH_MASK  (0x3<<6)
#define DL5_DSD_WIDTH_MASK  (0x3<<8)
#define DL6_DSD_WIDTH_MASK  (0x3<<10)
#define ARB1_DSD_WIDTH_MASK (0x3<<12)
#define DSDR_DSD_WIDTH_MASK (0x3<<14)
#define VUL_DSD_WIDTH_MASK  (0x3<<16)
#define UL2_DSD_WIDTH_MASK  (0x3<<18)
#define UL3_DSD_WIDTH_MASK  (0x3<<20)
#define UL4_DSD_WIDTH_MASK  (0x3<<22)
#define UL5_DSD_WIDTH_MASK  (0x3<<24)
#define UL6_DSD_WIDTH_MASK  (0x3<<26)
#define AWB_DSD_WIDTH_MASK  (0x3<<28)
#define AWB2_DSD_WIDTH_MASK (0x3<<30)

#define AFE_DAIBT_CON0  (0x001c)
#define USE_MRGIF_INPUT_POS         (12)
#define USE_MRGIF_INPUT_MASK        (1<<USE_MRGIF_INPUT_POS)
#define DAIBT_MODE_POS              (9)
#define DAIBT_MODE_MASK             (1<<DAIBT_MODE_POS)
#define DAI_SEL_POS                 (8)
#define DAI_SEL_MASK                (1<<DAI_SEL_POS)
#define BT_LEN_POS                  (4)
#define BT_LEN_MASK                 (7<<BT_LEN_POS)
#define DATA_RDY_POS                (3)
#define DATA_RDY_MASK               (1<<DATA_RDY_POS)
#define BTSYNC_POS                  (2)
#define BTSYNC_MASK                 (1<<BTSYNC_POS)
#define C_POS                       (1)
#define C_MASK                      (1<<C_POS)
#define DAIBT_ON_POS                (0)
#define DAIBT_ON_MASK               (1<<DAIBT_ON_POS)

#define AFE_MRGIF_CON    (0x003C)
#define MRGIF_I2S_MODE_POS          (20)
#define MRGIF_I2S_MODE_MASK         (0xF<<MRGIF_I2S_MODE_POS)
#define MRGIF_LOOPBACK_POS          (19)
#define MRGIF_LOOPBACK_MASK         (1<<MRGIF_LOOPBACK_POS)
#define MRGIF_I2S_EN_POS            (16)
#define MRGIF_I2S_EN_MASK           (1<<MRGIF_I2S_EN_POS)
#define MRG_CLK_NO_INV_POS          (14)
#define MRG_CLK_NO_INV_MASK         (1<<MRG_CLK_NO_INV_POS)
#define MRG_I2S_TX_DIS_POS          (13)
#define MRG_I2S_TX_DIS_MASK         (1<<MRG_I2S_TX_DIS_POS)
#define MRG_CNT_CLR_POS             (12)
#define MRG_CNT_CLR_MASK            (1<<MRG_CNT_CLR_POS)
#define MRG_SYNC_DLY_POS            (8)
#define MRG_SYNC_DLY_MASK           (0xF<<MRG_SYNC_DLY_POS)
#define MRG_CLK_EDGE_DLY_POS        (6)
#define MRG_CLK_EDGE_DLY_MASK       (3<<MRG_CLK_EDGE_DLY_POS)
#define MRG_CLK_DLY_POS             (4)
#define MRG_CLK_DLY_MASK            (3<<MRG_CLK_DLY_POS)
#define MRGIF_EN_POS                (0)
#define MRGIF_EN_MASK               (1<<MRGIF_EN_POS)

#define AFE_BT_SECURITY0  (0x0320)
#define AFE_BT_SECURITY1  (0x0324)

#define AFE_IRQ1_MCU_CNT_MON  (0x03C0)
#define AFE_IRQ2_MCU_CNT_MON  (0x03C4)
#define AFE_IRQ5_MCU_CNT_MON  (0x03CC)
#define AFE_IRQ_MCU_CNT_MON_MASK   (0x3ffff)

#define AFE_CONN0       (0x06C0)
#define AFE_CONN1       (0x06C4)
#define AFE_CONN2       (0x06C8)
#define AFE_CONN3       (0x06CC)
#define AFE_CONN4       (0x06D0)
#define AFE_CONN8       (0x06E0)
#define AFE_CONN9       (0x06E4)
#define AFE_CONN10       (0x06E8)
#define AFE_CONN11       (0x06EC)
#define AFE_CONN15		(0x06FC)
#define AFE_CONN16		(0x0700)
#define AFE_CONN33      (0x0744)
#define AFE_CONN34      (0x0748)
#define AFE_CONN35      (0x074C)
#define AFE_CONN36      (0x0750)
#define I26_PCM_RX_SEL_MASK   (0x1 << 31)
#define I26_PCM_RX_SEL_DAIBT  (0x0 << 31)
#define I26_PCM_RX_SEL_PCMRX  (0x1 << 31)
#define O31_PCM_TX_SEL_MASK   (0x1 << 30)
#define O31_PCM_TX_SEL_DAIBT  (0x0 << 30)
#define O31_PCM_TX_SEL_PCMTX  (0x1 << 30)
#define AFE_CONN40      (0x0760)
#define AFE_CONN41      (0x0764)

#define ASYS_TOP_CON	(0x0600)
#define AMUTE_2CH_RESET_MASK	(0x1 << 23)
#define AMUTE_2CH_RESET	(0)
#define AMUTE_2CH_UNRESET	(0x1 << 23)
#define AMUTE_12CH_RESET_MASK	(0x1 << 22)
#define AMUTE_12CH_RESET	(0)
#define AMUTE_12CH_UNRESET	(0x1 << 22)
#define AMUTE_2CH_INV_MASK	(0x1 << 21)
#define AMUTE_2CH_INV	(0x1 << 21)
#define AMUTE_2CH_NOT_INV	(0)
#define AMUTE_12CH_INV_MASK	(0x1 << 20)
#define AMUTE_12CH_INV	(0x1 << 20)
#define AMUTE_12CH_NOT_INV	(0)
#define I2S2_DSD_USE_MASK       (0x1<<4)
#define I2S2_DSD_USE_NORMAL     (0x0<<4)
#define I2S2_DSD_USE_SONY_IP    (0x1<<4)
#define I2S1_DSD_USE_MASK       (0x1<<3)
#define I2S1_DSD_USE_NORMAL     (0x0<<3)
#define I2S1_DSD_USE_SONY_IP    (0x1<<3)
#define A1SYS_TIMING_ON_MASK	(0x3<<0)
#define A1SYS_TIMING_OFF        (0x0<<0)
#define A1SYS_TIMING_ON         (0x3<<0)

#define ASYS_I2SIN1_CON   (0x0604)
#define ASYS_I2SO1_CON   (0x061C)
#define I2S_ENABLE_MASK		(0x1)
#define I2S_ENABLE			(0x1)
#define I2S_DISABLE			(0)
#define I2S_WLEN_MASK		(0x2)
#define I2S_WLEN_16BIT		(0)
#define I2S_WLEN_32BIT		(0x2)
#define I2S_SLAVE_MODE_MASK	(0x4)
#define I2S_SLAVE_MODE		(0x4)
#define I2S_MASTER_MODE		(0)
#define I2S_FMT_MASK		(0x1 << 3)
#define I2S_FMT_EIAJ			(0)
#define I2S_FMT_I2S			(0x1 << 3)
#define I2S_LRCK_INV_MASK	(0x1 << 5)
#define I2S_LRCK_INV		(0x1 << 5)
#define I2S_LRCK_NO_INV		(0)
#define I2S_FS_MODE_MASK	(0x1F << 8)
#define I2S_FS_MODE_POS		(8)
#define I2S_VALID_BITS_MASK	(0x1 << 13)
#define I2S_VALID_24BIT		(0x1 << 13)
#define I2S_VALID_16BIT		(0)
#define I2S_FMT_ADJUST_MASK	(0x1 << 14)
#define I2S_FMT_ADJUST_LJ	(0)
#define I2S_FMT_ADJUST_RJ	(0x1 << 14)
#define I2S_OUT_ONE_HEART_MASK	(0x1 << 16)
#define I2S_OUT_2CH_MODE		(0)
#define I2S_OUT_MCH_MODE		(0x1 << 16)
#define I2S_IN_OUT_COUPLE_MASK	(0x1 << 17)
#define I2S_IN_OUT_SELF_CLOCK	(0)
#define I2S_IN_OUT_SHARE_CLOCK	(0x1 << 17)
#define I2S_DSD_MODE_MASK		(0x1 << 18)
#define I2S_NORMAL_MODE			(0)
#define I2S_DSD_MODE			(0x1 << 18)
#define I2S_SW_RESET_MASK		(0x1 << 30)
#define I2S_SW_RESET_EN			(0x1 << 30)
#define I2S_SW_RESET_DIS		(0)
#define I2S_LR_SWAP_MASK		(0x1 << 31)
#define I2S_LR_SWAP_EN			(0x1 << 31)
#define I2S_LR_SWAP_DIS			(0)
#define ASYS_I2SO2_CON   (0x0620)
#define LAT_DATA_EN_POS  (26)  /* for I2SO1 and I2SO2 */

#define AFE_MEMIF_HD_CON1   (0x121C)
#define ULMCH_HD_AUDIO_ON    (0x0 << 12)
#define ULMCH_HD_AUDIO_ALIGN (0x1 << 12)
/* Memory interface */
#define AFE_MEMIF_HD_CON0  (0x123C)
#define DLMCH_DL6_SEL      (0x1<<23)
#define DLMCH_DL5_SEL      (0x1<<22)
#define DLMCH_DL4_SEL      (0x1<<21)
#define DLMCH_DL3_SEL      (0x1<<20)
#define DLMCH_DL2_SEL      (0x1<<19)
#define DLMCH_DL1_SEL      (0x1<<18)
#define DL1_HD_AUDIO_MASK  (0x1<<0)
#define DL1_HD_AUDIO_ON    (0x1<<0)
#define DL1_HD_AUDIO_OFF   (0x0<<0)
#define DL2_HD_AUDIO_MASK  (0x1<<2)
#define DL2_HD_AUDIO_ON    (0x1<<2)
#define DL2_HD_AUDIO_OFF   (0x0<<2)
#define DL3_HD_AUDIO_MASK  (0x1<<4)
#define DL3_HD_AUDIO_ON    (0x1<<4)
#define DL3_HD_AUDIO_OFF   (0x0<<4)
#define DL4_HD_AUDIO_MASK  (0x1<<6)
#define DL4_HD_AUDIO_ON    (0x1<<6)
#define DL4_HD_AUDIO_OFF   (0x0<<6)
#define DL5_HD_AUDIO_MASK  (0x1<<8)
#define DL5_HD_AUDIO_ON    (0x1<<8)
#define DL5_HD_AUDIO_OFF   (0x0<<8)
#define DL6_HD_AUDIO_MASK  (0x1<<10)
#define DL6_HD_AUDIO_ON    (0x1<<10)
#define DL6_HD_AUDIO_OFF   (0x0<<10)
#define ARB1_HD_AUDIO_MASK  (0x1<<14)
#define ARB1_HD_AUDIO_ON    (0x1<<14)
#define ARB1_HD_AUDIO_OFF   (0x0<<14)
#define DSDR_HD_AUDIO_MASK  (0x1<<16)
#define DSDR_HD_AUDIO_ON    (0x1<<16)
#define DSDR_HD_AUDIO_OFF   (0x0<<16)
#define AFE_MEMIF_HD_CON1  (0x121C)
#define VUL_HD_AUDIO_MASK (0x1<<0)
#define UL2_HD_AUDIO_MASK (0x1<<2)
#define UL3_HD_AUDIO_MASK (0x1<<4)
#define UL4_HD_AUDIO_MASK (0x1<<6)
#define UL5_HD_AUDIO_MASK (0x1<<8)
#define UL6_HD_AUDIO_MASK (0x1<<10)
#define ULMCH_HD_AUDIO_MASK (0x1<<12)
#define AWB_HD_AUDIO_MASK (0x1<<14)
#define AWB2_HD_AUDIO_MASK (0x1<<16)
#define TDMIN_HD_AUDIO_MASK  (0x1<<18)

#define MASM_TRAC_CON1  (0x108C)
#define POS_MASRC1_CALC_LRCK_SEL    0

#define AFE_DL1_BASE    (0x1240)
#define AFE_DL1_CUR     (0x1244)
#define AFE_DL1_END     (0x1248)
#define AFE_I2S_CON3    (0x004C)
#define AFE_DL2_BASE    (0x1250)
#define AFE_DL2_CUR     (0x1254)
#define AFE_DL2_END     (0x1258)
#define AFE_DL3_BASE    (0x1260)
#define AFE_DL3_CUR     (0x1264)
#define AFE_DL3_END     (0x1268)
#define AFE_DL4_BASE    (0x1270)
#define AFE_DL4_CUR     (0x1274)
#define AFE_DL4_END     (0x1278)
#define AFE_DL5_BASE    (0x1280)
#define AFE_DL5_CUR     (0x1284)
#define AFE_DL5_END     (0x1288)
#define AFE_DL6_BASE    (0x1290)
#define AFE_DL6_CUR     (0x1294)
#define AFE_DL6_END     (0x1298)
#define AFE_ARB1_BASE    (0x12B0)
#define AFE_ARB1_CUR     (0x12B4)
#define AFE_ARB1_END     (0x12B8)
#define AFE_DLMCH_BASE    (0x12A0)
#define AFE_DLMCH_CUR     (0x12A4)
#define AFE_DLMCH_END     (0x12A8)
#define AFE_DSDR_BASE    (0x12C0)
#define AFE_DSDR_CUR     (0x12C4)
#define AFE_DSDR_END     (0x12C8)
#define AFE_AWB_BASE    (0x12D0)
#define AFE_AWB_END     (0x12D8)
#define AFE_AWB_CUR     (0x12DC)
#define AFE_AWB2_BASE   (0x12E0)
#define AFE_AWB2_END    (0x12E8)
#define AFE_AWB2_CUR    (0x12EC)
#define AFE_DSDW_BASE   (0x12F0)
#define AFE_DSDW_END    (0x12F8)
#define AFE_DSDW_CUR    (0x12FC)
#define AFE_VUL_BASE    (0x1300)
#define AFE_VUL_END     (0x1308)
#define AFE_VUL_CUR     (0x130C)
#define AFE_UL2_BASE    (0x1310)
#define AFE_UL2_END     (0x1318)
#define AFE_UL2_CUR     (0x131C)
#define AFE_UL3_BASE    (0x1320)
#define AFE_UL3_END     (0x1328)
#define AFE_UL3_CUR     (0x132C)
#define AFE_UL4_BASE    (0x1330)
#define AFE_UL4_END     (0x1338)
#define AFE_UL4_CUR     (0x133C)
#define AFE_UL5_BASE    (0x1340)
#define AFE_UL5_END     (0x1348)
#define AFE_UL5_CUR     (0x134C)
#define AFE_UL6_BASE    (0x1350)
#define AFE_UL6_END     (0x1358)
#define AFE_UL6_CUR     (0x135C)

#define AFE_DAI_BASE    (0x1370)
#define AFE_DAI_END     (0x1378)
#define AFE_DAI_CUR     (0x137C)

#define AFE_DMA_BASE    (0x13A0)
#define POS_AFE_DMA_BASE_ADDR   0
#define MASK_AFE_DMA_BASE_ADDR  (0xFFFFFFF0)
/* keep the [3:0] = 0000b for the convenience of HW implementation */
#define AFE_DMA_CUR     (0x13A4)
/* DMA current read pointer */
#define AFE_DMA_END     (0x13A8)
#define AFE_DMA_WR_CUR  (0x13AC)
/* DMA current write pointer */
#define AFE_DMA_CON0    (0x13B0)
#define POS_AFE_DMA_NEXT_OFF        (20)
#define MASK_AFE_DMA_NEXT_OFF       (1<<POS_AFE_DMA_NEXT_OFF)
#define POS_AFE_DMA_RD_COMP_EN      (18)
#define MASK_AFE_DMA_RD_COMP_EN     (1<<POS_AFE_DMA_RD_COMP_EN)
#define POS_AFE_DL1_ALL_COMP        (17)
#define MASK_AFE_DL1_ALL_COMP       (1<<POS_AFE_DL1_ALL_COMP)
#define POS_AFE_DL1_AUTO_OFF        (16)
#define MASK_AFE_DL1_AUTO_OFF       (1<<POS_AFE_DL1_AUTO_OFF)
#define POS_AFE_DMA_ALL_COMP        (15)
#define MASK_AFE_DMA_ALL_COMP       (1<<POS_AFE_DMA_ALL_COMP)
#define POS_AFE_DMA_RD_COMP         (14)
#define MASK_AFE_DMA_RD_COMP        (1<<POS_AFE_DMA_RD_COMP)
#define POS_AFE_DMA_1ST_COMP        (13)
#define MASK_AFE_DMA_1ST_COMP       (1<<POS_AFE_DMA_1ST_COMP)
#define POS_AFE_DL1_AUTO_ON         (12)
#define MASK_AFE_DL1_AUTO_ON        (1<<POS_AFE_DL1_AUTO_ON)
#define POS_AFE_DMA_INTR            (11)
#define MASK_AFE_DMA_INTR           (1<<POS_AFE_DMA_INTR)
#define POS_AFE_DMA_INTR_CLR        (10)
#define MASK_AFE_DMA_INTR_CLR       (1<<POS_AFE_DMA_INTR_CLR)
#define POS_AFE_DMA_PBUF_SIZE       (8)
#define MASK_AFE_DMA_PBUF_SIZE      (3<<POS_AFE_DMA_PBUF_SIZE)
#define POS_AFE_DMA_MAXLEN          (4)
#define MASK_AFE_DMA_MAXLEN         (0xF<<POS_AFE_DMA_MAXLEN)
#define POS_AFE_DMA_MINLEN          (0)
#define MASK_AFE_DMA_MAINLEN         (0xF<<POS_AFE_DMA_MINLEN)
#define AFE_DMA_THRESHOLD     (0x13B4)
#define AFE_DMA_INTR_SIZE     (0x13B8)
#define AFE_DMA_NEXT_INTR     (0x13BC)
#define AFE_DMA_NEXT_BASE     (0x13C0)
#define AFE_DMA_NEXT_END      (0x13C8)

/* Downlink counter */
#define AFE_DL_COUNTER_CON (0x1544)
#define AFE_DL1_RD_COUNTER (0x1568)
#define AFE_DL2_RD_COUNTER (0x156C)
#define AFE_DL3_RD_COUNTER (0x1570)
#define AFE_DL4_RD_COUNTER (0x1574)

/* Memory interface monitor */
#define AFE_MEMIF_MON0 (0x15D0)
#define AFE_MEMIF_MON1 (0x15D4)
#define AFE_MEMIF_MON2 (0x15D8)
#define AFE_MEMIF_MON3 (0x15DC)
#define AFE_MEMIF_MON4 (0x15E0)


/* 6582 Add */
#define AFE_ADDA_DL_SRC2_CON0	(0x00108)
#define AFE_ADDA_DL_SRC2_CON1	(0x0010C)
#define AFE_ADDA_UL_SRC_CON0	(0x00114)
#define AFE_ADDA_UL_SRC_CON1	(0x00118)
#define AFE_ADDA_TOP_CON0		(0x00120)
#define AFE_ADDA_UL_DL_CON0		(0x00124)
#define AFE_ADDA_SRC_DEBUG		(0x0012C)
#define AFE_ADDA_SRC_DEBUG_MON0	(0x00130)
#define AFE_ADDA_SRC_DEBUG_MON1	(0x00134)
#define AFE_ADDA_NEWIF_CFG0		(0x00138)
#define AFE_ADDA_NEWIF_CFG1		(0x0013C)

#define ASMI_TIMING_CON0   (0x00fc)
#define ASMI0_MODE_POS   (0)
#define ASMI0_MODE_MASK  (0x1F<<ASMI0_MODE_POS)

#define ASMI_TIMING_CON1   (0x0100)
#define ASMI1_MODE_MASK  (0x1F<<0)
#define ASMI2_MODE_MASK  (0x1F<<5)


#define ASMI1_MODE_POS   (0)
#define ASMI1_MODE_MASK  (0x1F<<0)

#define ASMI4_MODE_MASK  (0x1F<<15)
#define ASMI5_MODE_MASK  (0x1F<<20)
#define ASMI6_MODE_MASK  (0x1F<<25)
#define ASMO_TIMING_CON1   (0x0104)
#define ASMO1_MODE_MASK   (0x1f<<0)
#define ASMO2_MODE_MASK   (0x1f<<5)
#define ASMO3_MODE_MASK   (0x1f<<10)
#define ASMO4_MODE_MASK   (0x1f<<15)
#define ASMO5_MODE_MASK   (0x1f<<20)
#define ASMO6_MODE_MASK   (0x1f<<25)

#define AFE_DL_SDM_CON0 (0x0110)
#define AFE_UL_SRC_0    (0x0114)
#define AFE_UL_SRC_1    (0x0118)

#define AFE_VAGC_CON1  (0x0120)
#define AFE_VAGC_CON2  (0x0124)
#define AFE_VAGC_CON3  (0x0128)
#define AFE_VAGC_CON4  (0x012C)
#define AFE_VAGC_CON5  (0x0130)
#define AFE_VAGC_CON6  (0x0134)
#define AFE_VAGC_CON7  (0x0138)
#define AFE_VAGC_CON8  (0x013C)
#define AFE_VAGC_CON9  (0x0140)
#define AFE_VAGC_CON10 (0x0144)
#define AFE_VAGC_CON11 (0x0148)
#define AFE_VAGC_CON12 (0x014C)
#define AFE_VAGC_CON13 (0x0150)
#define AFE_VAGC_CON14 (0x0154)
#define AFE_VAGC_CON15 (0x0158)
#define AFE_VAGC_CON16 (0x015C)
#define AFE_VAGC_CON17 (0x0160)
#define AFE_VAGC_CON18 (0x0164)
#define AFE_VAGC_CON19 (0x0168)

#define AFE_FOC_CON  (0x0170)
#define AFE_FOC_CON1 (0x0174)
#define AFE_FOC_CON2 (0x0178)
#define AFE_FOC_CON3 (0x017C)
#define AFE_FOC_CON4 (0x0180)
#define AFE_FOC_CON5 (0x0184)

#define AFE_MON_STEP        (0x0188)
#define AFE_SIDETONE_DEBUG  (0x01D0)
#define AFE_SIDETONE_MON    (0x01D4)
#define AFE_SIDETONE_CON0   (0x01E0)
#define AFE_SIDETONE_COEFF  (0x01E4)
#define AFE_SIDETONE_CON1   (0x01E8)
#define AFE_SIDETONE_GAIN   (0x01EC)

#define AFE_SGEN_CON0   (0x01F0)
#define INNER_LOOP_BACK_MODE_MASK    (0x1F<<27)
#define INNER_LOOP_BACK_MODE_I00_I01 (0x0<<27)
#define INNER_LOOP_BACK_MODE_I02_I03 (0x1<<27)
#define INNER_LOOP_BACK_MODE_I04_I05 (0x2<<27)
#define INNER_LOOP_BACK_MODE_I06_I07 (0x3<<27)
#define INNER_LOOP_BACK_MODE_I08_I09 (0x4<<27)
#define INNER_LOOP_BACK_MODE_I10_I11 (0x5<<27)
#define INNER_LOOP_BACK_MODE_I12_I13 (0x6<<27)
#define INNER_LOOP_BACK_MODE_I14_I15 (0x7<<27)
#define INNER_LOOP_BACK_MODE_I16_I17 (0x8<<27)
#define INNER_LOOP_BACK_MODE_I18_I19 (0x9<<27)
#define INNER_LOOP_BACK_MODE_I20_I21 (0xA<<27)
#define INNER_LOOP_BACK_MODE_I22_I23 (0xB<<27)
#define INNER_LOOP_BACK_MODE_I24_I25 (0xC<<27)
#define INNER_LOOP_BACK_MODE_I26     (0xD<<27)
#define INNER_LOOP_BACK_MODE_I31_I32 (0xE<<27)
#define INNER_LOOP_BACK_MODE_I33_I34 (0xF<<27)
#define INNER_LOOP_BACK_MODE_I35_I36 (0x10<<27)
#define INNER_LOOP_BACK_MODE_O00_O01 (0x11<<27)
#define INNER_LOOP_BACK_MODE_O15_O16 (0x19<<27)
#define INNER_LOOP_BACK_MODE_O17_O18 (0x1A<<27)
#define INNER_LOOP_BACK_MODE_O19_O20 (0x1B<<27)
#define INNER_LOOP_BACK_MODE_O21_O22 (0x1C<<27)
#define INNER_LOOP_BACK_MODE_O23_O24 (0x1D<<27)
#define INNER_LOOP_BACK_MODE_O25_O26 (0x1E<<27)
#define SGEN_EN_MASK                 (0x1<<26)
#define SGEN_ENABLE                 (0x1<<26)
#define SGEN_DISABLE                (0x0<<26)
#define SINE_MODE_CH2_MASK	    (0xF<<20)
#define SINE_MODE_CH2_8K	    (0x0<<20)
#define SINE_MODE_CH2_11p025K	    (0x1<<20)
#define SINE_MODE_CH2_12K	    (0x2<<20)
#define SINE_MODE_CH2_16K	    (0x3<<20)
#define SINE_MODE_CH2_22p05K	    (0x4<<20)
#define SINE_MODE_CH2_24K	    (0x5<<20)
#define SINE_MODE_CH2_32K	    (0x6<<20)
#define SINE_MODE_CH2_44p1K	    (0x7<<20)
#define SINE_MODE_CH2_48K	    (0x8<<20)
#define SINE_MODE_CH2_88p2K	    (0x9<<20)
#define SINE_MODE_CH2_96K	    (0xA<<20)
#define SINE_MODE_CH2_176p4K	    (0xB<<20)
#define SINE_MODE_CH2_192K	    (0xC<<20)
#define SINE_MODE_CH2_352p8K	    (0xD<<20)
#define SINE_MODE_CH2_384K	    (0xE<<20)
#define AMP_DIV_CH2_MASK            (0x7<<17)
#define AMP_DIV_CH2_1               (0x7<<17)	/* amp/1 */
#define AMP_DIV_CH2_2               (0x6<<17)	/* amp/2 */
#define AMP_DIV_CH2_4               (0x5<<17)	/* amp/4 */
#define AMP_DIV_CH2_8               (0x4<<17)	/* amp/8 */
#define AMP_DIV_CH2_16              (0x3<<17)	/* amp/16 */
#define AMP_DIV_CH2_32              (0x2<<17)	/* amp/32 */
#define AMP_DIV_CH2_64              (0x1<<17)	/* amp/64 */
#define AMP_DIV_CH2_128             (0x0<<17)	/* amp/128 */
#define FREQ_DIV_CH2_MASK           (0x1F<<12)
#define FREQ_DIV_CH2_1              (0x1<<12)	/* 64/1 samples/period */
#define FREQ_DIV_CH2_2              (0x2<<12)	/* 64/2 samples/period */
#define SINE_MODE_CH1_MASK	    (0xF<<8)
#define SINE_MODE_CH1_8K	    (0x0<<8)
#define SINE_MODE_CH1_11p025K	    (0x1<<8)
#define SINE_MODE_CH1_12K	    (0x2<<8)
#define SINE_MODE_CH1_16K	    (0x3<<8)
#define SINE_MODE_CH1_22p05K	    (0x4<<8)
#define SINE_MODE_CH1_24K	    (0x5<<8)
#define SINE_MODE_CH1_32K	    (0x6<<8)
#define SINE_MODE_CH1_44p1K	    (0x7<<8)
#define SINE_MODE_CH1_48K	    (0x8<<8)
#define SINE_MODE_CH1_88p2K	    (0x9<<8)
#define SINE_MODE_CH1_96K	    (0xA<<8)
#define SINE_MODE_CH1_176p4K	    (0xB<<8)
#define SINE_MODE_CH1_192K	    (0xC<<8)
#define SINE_MODE_CH1_352p8K	    (0xD<<8)
#define SINE_MODE_CH1_384K	    (0xE<<8)
#define AMP_DIV_CH1_MASK            (0x7<<5)
#define AMP_DIV_CH1_1               (0x7<<5)	/* amp/1 */
#define AMP_DIV_CH1_2               (0x6<<5)	/* amp/2 */
#define AMP_DIV_CH1_4               (0x5<<5)	/* amp/4 */
#define AMP_DIV_CH1_8               (0x4<<5)	/* amp/8 */
#define FREQ_DIV_CH1_MASK           (0x1F<<0)
#define FREQ_DIV_CH1_1              (0x1<<0)	/* 64/1 samples/period */
#define FREQ_DIV_CH1_2              (0x2<<0)	/* 64/2 samples/period */


#define AFE_TOP_CON0    (0x0200)
#define AFE_VAGC_CON0   (0x020C)

/* 6582 Add */
#define AFE_ADDA_PREDIS_CON0	(0x00260)
#define AFE_ADDA_PREDIS_CON1	(0x00264)


#define AFE_AGC_MON0 (0x0290)
#define AFE_AGC_MON1 (0x0294)
#define AFE_AGC_MON2 (0x0298)
#define AFE_AGC_MON3 (0x029C)
#define AFE_AGC_MON4 (0x02A0)
#define AFE_AGC_MON5 (0x0318)

#define AFE_VAD_MON0  (0x02A4)
#define AFE_VAD_MON1  (0x02A8)
#define AFE_VAD_MON2  (0x02AC)
#define AFE_VAD_MON3  (0x02B0)
#define AFE_VAD_MON4  (0x02B4)
#define AFE_VAD_MON5  (0x02B8)
#define AFE_VAD_MON6  (0x02BC)
#define AFE_VAD_MON7  (0x02C0)
#define AFE_VAD_MON8  (0x02C4)
#define AFE_VAD_MON9  (0x02C8)
#define AFE_VAD_MON10 (0x02CC)
#define AFE_VAD_MON11 (0x02D0)

#define AFE_MOD_PCM_BASE (0x1380)
#define AFE_MOD_PCM_END  (0x1388)
#define AFE_MOD_PCM_CUR  (0x138C)

#define AFE_SPDIF2_OUT_CON0 (0x360)
#define AFE_SPDIF2_BASE     (0x364)
#define AFE_SPDIF2_CUR      (0x368)
#define AFE_SPDIF2_END      (0x36C)


/* TDM Memory interface */
#define AFE_TDM_OUT_CON0   (0x0370)
#define TDM_OUT_ON         (0x1)
#define TDM_OUT_BIT_WIDTH  (0x1 << 1)
#define TDM_OUT_CH_NUM     (0x1F << 4)

#define AFE_TDM_OUT_BASE (0x0374)
#define AFE_TDM_OUT_CUR  (0x0378)
#define AFE_TDM_OUT_END  (0x037C)

#define AFE_TDM_IN_BASE   (0x13D0)
#define AFE_TDM_IN_CUR    (0x13D4)
#define AFE_TDM_IN_END    (0x13D8)

#define AFE_SPDIF_OUT_CON0 (0x0380)
#define AFE_SPDIF_BASE     (0x0384)
#define AFE_SPDIF_CUR      (0x0388)
#define AFE_SPDIF_END      (0x038C)

#define AFE_HDMI_CONN0      (0x0390)
  #define HDMI_INTER_O20_POS	(0)
  #define HDMI_INTER_O21_POS	(3)
  #define HDMI_INTER_O22_POS	(6)
  #define HDMI_INTER_O23_POS	(9)
  #define HDMI_INTER_O24_POS	(12)
  #define HDMI_INTER_O25_POS	(15)
  #define HDMI_INTER_O26_POS	(18)
  #define HDMI_INTER_O27_POS	(21)
#define AFE_8CH_I2S_OUT_CON (0x0394)
#define AFE_HDMI_CONN1      (0x0398)

#define AFE_IRQ_CON             (0x03A0)
#define AFE_IRQ_STATUS          (0x03A4)
#define AFE_IRQ_CLR             (0x03A8)
#define AFE_IRQ_STATUS_BITS     (0x33)
#define AFE_IRQ_CNT1            (0x03AC)
#define IRQ_CNT1_MASK       (0x3FFFF)
#define AFE_IRQ_CNT2            (0x03B0)
#define IRQ_CNT2_MASK       (0x3FFFF)
#define AFE_IRQ_MON2            (0x03B8)
#define AFE_IRQ_CNT5            (0x03BC)
#define AFE_IRQ1_CNT_MON        (0x03C0)
#define AFE_IRQ2_CNT_MON        (0x03C4)
#define AFE_IRQ1_EN_CNT_MON     (0x03C8)
#define AFE_IRQ_MCU_EN			(0x03D0)
#define AFE_IRQ_CNT11            (0x171C)
#define IRQ_CNT11_MASK           (0x3FFFF)

#define AFE_IRQ_CNT12            (0x1720)
#define IRQ_CNT12_MASK           (0x3FFFF)

#define AFE_IRQ_ACC1_CNT         (0x1700)
#define IRQ_ACC1_CNT_MASK        (0xFFFFFFFF)

#define AFE_IRQ_ACC2_CNT         (0x1704)
#define IRQ_ACC2_CNT_MASK        (0xFFFFFFFF)

#define AFE_IRQ_ACC1_CNT_MON1    (0x1708)
#define AFE_IRQ_ACC1_CNT_MON2    (0x170C)

#define AFE_IRQ_ACC2_CNT_MON     (0x1718)

#define AFE_IRQ11_CNT_MON        (0x1724)
#define AFE_IRQ12_CNT_MON        (0x1728)

/************  ASYS_IRQ  ************/
#define ASYS_IRQ_CONFIG         (0x077C)
#define ASYS_IRQ_MON_SEL_MASK   (0xF << 0)
#define ASYS_IRQ_MON_SEL_IRQ1   (0x0 << 0)

#define ASYS_IRQ1_CON           (0x0780)
#define ASYS_IRQ2_CON           (0x0784)
#define ASYS_IRQ3_CON           (0x0788)
#define ASYS_IRQ4_CON           (0x078C)
#define ASYS_IRQ5_CON           (0x0790)
#define ASYS_IRQ6_CON           (0x0794)
#define ASYS_IRQ7_CON           (0x0798)
#define ASYS_IRQ8_CON           (0x079C)
#define ASYS_IRQ9_CON           (0x07A0)
#define ASYS_IRQ10_CON           (0x07A4)
#define ASYS_IRQ11_CON           (0x07A8)
#define ASYS_IRQ12_CON           (0x07AC)
#define ASYS_IRQ13_CON           (0x07B0)
#define ASYS_IRQ14_CON           (0x07B4)
#define ASYS_IRQ15_CON           (0x07B8)
#define ASYS_IRQ16_CON           (0x07BC)
#define ASYS_IRQ_ON_MASK        (0x1<<31)
#define ASYS_IRQ_ON             (0x1<<31)
#define ASYS_IRQ_OFF            (0x0<<31)
#define ASYS_IRQ_BLK_MODE_MASK        (0x1<<30)
#define ASYS_IRQ_BLK_MODE_PINGPANG    (0x0<<30)
#define ASYS_IRQ_BLK_MODE_NORMAL      (0x1<<30)
#define ASYS_IRQ_MODE_MASK          (0x1F<<24)
#define ASYS_IRQ_CNT_INIT_MASK      (0xffffff<<0)

#define ASYS_IRQ_CLR            (0x07C0)
#define ASYS_IRQ_CLR_ALL  (0xffff<<0)

#define ASYS_IRQ_STATUS         (0x07C4)
#define ASYS_IRQ_STATUS_BITS    (0xF)

#define ASYS_IRQ_MON2         (0x07CC)
#define IRQ_CNT_STATUS_MASK   (0x00ffffff)

#define PWR2_ASM_CON2           (0x1074)
#define MEM_ASRC_1_RESET_POS  20
#define MEM_ASRC_2_RESET_POS  21
#define MEM_ASRC_3_RESET_POS  22
#define MEM_ASRC_4_RESET_POS  23
#define MEM_ASRC_5_RESET_POS  24

/************************************************/

#define AFE_MEMIF_MINLEN0        (0x1220)
#define DL1_MINLEN_MASK    (0xF<<0)
#define DL1_MINLEN_3       (0x3<<0)
#define DL2_MINLEN_MASK    (0xF<<4)
#define DL2_MINLEN_3       (0x3<<4)
#define DL3_MINLEN_MASK    (0xF<<8)
#define DL3_MINLEN_3       (0x3<<8)
#define DL4_MINLEN_MASK    (0xF<<12)
#define DL4_MINLEN_3       (0x3<<12)
#define DL5_MINLEN_MASK    (0xF<<16)
#define DL5_MINLEN_3       (0x3<<16)
#define DL6_MINLEN_MASK    (0xF<<20)
#define DL6_MINLEN_3       (0x3<<20)
#define AFE_MEMIF_MAXLEN0        (0x122C)
#define DL1_MAXLEN_MASK        (0xF<<0)
#define DL1_MAXLEN_NOTSUPPORT  (0x0<<0)
#define DL1_MAXLEN_16BYTEBURST (0x1<<0)
#define DL2_MAXLEN_MASK        (0xF<<4)
#define DL2_MAXLEN_NOTSUPPORT  (0x0<<4)
#define DL2_MAXLEN_16BYTEBURST (0x1<<4)
#define DL3_MAXLEN_MASK        (0xF<<8)
#define DL3_MAXLEN_NOTSUPPORT  (0x0<<8)
#define DL3_MAXLEN_16BYTEBURST (0x1<<8)
#define DL4_MAXLEN_MASK        (0xF<<12)
#define DL4_MAXLEN_NOTSUPPORT  (0x0<<12)
#define DL4_MAXLEN_16BYTEBURST (0x1<<12)
#define DL5_MAXLEN_MASK        (0xF<<16)
#define DL5_MAXLEN_NOTSUPPORT  (0x0<<16)
#define DL5_MAXLEN_16BYTEBURST (0x1<<16)
#define DL6_MAXLEN_MASK        (0xF<<20)
#define DL6_MAXLEN_NOTSUPPORT  (0x0<<20)
#define DL6_MAXLEN_16BYTEBURST (0x1<<20)
#define DLMCH_MAXLEN_MASK        (0xF<<24)
#define DLMCH_MAXLEN_NOTSUPPORT  (0x0<<24)
#define DLMCH_MAXLEN_16BYTEBURST (0x1<<24)
#define ARB1_MAXLEN_MASK        (0xF<<28)
#define ARB1_MAXLEN_NOTSUPPORT  (0x0<<28)
#define ARB1_MAXLEN_16BYTEBURST (0x1<<28)
#define AFE_MEMIF_MAXLEN1        (0x1230)
#define DSDR_MAXLEN_MASK   (0xF<<0)
#define DSDR_MAXLEN_NOTSUPPORT  (0x0<<0)
#define DSDR_MAXLEN_16BYTEBURST (0x1<<0)

#define AFE_MEMIF_MAXLEN2        (0x1234)
#define AFE_IEC_PREFETCH_SIZE   (0x03D8)

/* AFE GAIN CONTROL REGISTER */
#define AFE_GAIN1_CON0         (0x0410)
#define POS_GAIN_SAMPLE_PER_STEP    8
#define MASK_GAIN_SAMPLE_PER_STEP   (0xFF<<POS_GAIN_SAMPLE_PER_STEP)
#define POS_GAIN_MODE               3
#define MASK_GAIN_MODE              (0x1F<<POS_GAIN_MODE)
#define POS_GAIN_ON                 0
#define MASK_GAIN_ON                (1<<POS_GAIN_ON)
#define AFE_GAIN1_CON1         (0x0414)
#define POS_GAIN_TARGET     0
#define MASK_GAIN_TARGET    (0xFFFFF<<POS_GAIN_TARGET)
#define AFE_GAIN1_CON2         (0x0418)
#define POS_DOWN_STEP       0
#define MASK_DOWN_STEP      (0xFFFFF<<POS_DOWN_STEP)
#define AFE_GAIN1_CON3         (0x041C)
#define POS_UP_STEP         0
#define MASK_UP_STEP        (0xFFFFF<<POS_UP_STEP)
#define AFE_GAIN1_CONN         (0x0420)
#define AFE_GAIN1_CUR          (0x0424)
#define POS_GAIN_CUR        0
#define MASK_GAIN_CUR       (0xFFFFF<<POS_GAIN_CUR)
#define AFE_GAIN2_CON0         (0x0428)
#define AFE_GAIN2_CON1         (0x042C)
#define AFE_GAIN2_CON2         (0x0430)
#define AFE_GAIN2_CON3         (0x0434)
#define AFE_GAIN2_CONN         (0x0438)
#define AFE_GAIN2_CUR          (0x043C)


#define AFE_IEC_CFG         (0x0480)
#define AFE_IEC_NSNUM       (0x0484)
#define AFE_IEC_BURST_INFO  (0x0488)
#define AFE_IEC_BURST_LEN   (0x048C)
#define AFE_IEC_NSADR       (0x0490)
#define AFE_IEC_CHL_STAT0   (0x04A0)
#define AFE_IEC_CHL_STAT1   (0x04A4)
#define AFE_IEC_CHR_STAT0   (0x04A8)
#define AFE_IEC_CHR_STAT1   (0x04AC)

#define AFE_IEC2_CFG        (0x4B0)
#define AFE_IEC2_NSNUM      (0x4B4)
#define AFE_IEC2_BURST_INFO (0x4B8)
#define AFE_IEC2_BURST_LEN  (0x4BC)
#define AFE_IEC2_NSADR      (0x4C0)
#define AFE_IEC2_CHL_STAT0  (0x4D0)
#define AFE_IEC2_CHL_STAT1  (0x4D4)
#define AFE_IEC2_CHR_STAT0  (0x4D8)
#define AFE_IEC2_CHR_STAT1  (0x4DC)

#define FPGA_CFG2           (0x4E8)
#define FPGA_CFG3                 (0x4EC)
#define FPGA_TSF_DELAY_VALUE      (0xFFFFF << 0)
#define FPGA_RG_TSF_MODEL_EN      (0x1 << 20)
#define FPGA_RG_TSF_CLK_EN        (0x1 << 21)
#define FPGA_TSF_PULSE_WIDTH_DIV  (0xFF << 24)

#define FPFA_I2SOUT1_M_CK_POS (9)
#define FPFA_I2SOUT1_M_DAT_POS (3)
#define FPFA_I2SIN2_M_CK_POS (8)
#define FPFA_I2SIN1_M_CK_POS (7)
#define FPFA_I2S_CK_DAT_SEL_MASK (0x3FF << 0)

#define FPGA_CFG0           (0x4C0)
#define FPGA_CFG1           (0x4C4)
#define FPGA_VERSION        (0x4C8)
#define FPGA_AUDIO_CLOCK    (0x4CC)

#define AFE_ASRC_CON0   (0x500)
#define AFE_ASRC_CON1   (0x504)
#define AFE_ASRC_CON2   (0x508)
#define AFE_ASRC_CON3   (0x50C)
#define AFE_ASRC_CON4   (0x510)
#define AFE_ASRC_CON5   (0x514)
#define AFE_ASRC_CON6   (0x518)
#define AFE_ASRC_CON7   (0x51C)
#define AFE_ASRC_CON8   (0x520)
#define AFE_ASRC_CON9   (0x524)
#define AFE_ASRC_CON10  (0x528)
#define AFE_ASRC_CON11  (0x52C)

#define PCM_INTF_CON    (0x530)

#define AFE_SPDIFIN_CFG0 (0x500)
	#define SPDIFIN_CHSTS_SEL_MASK	(0x1 << 28)
	#define SPDIFIN_CHSTS_SEL_L	(0)
	#define SPDIFIN_CHSTS_SEL_R	(0x1 << 28)
#define AFE_SPDIFIN_CFG1 (0x504)
#define AFE_SPDIFIN_LOOPBACK_IEC2 (0x1<<15)
#define AFE_SPDIFIN_LOOPBACK_IEC2_MASK (0x1<<15)
#define AFE_SPDIFIN_LOOPBACK_IEC1 (0x1<<14)
#define AFE_SPDIFIN_LOOPBACK_IEC1_MASK (0x1<<14)
#define AFE_SPDIFIN_REAL_OPTICAL (0x0<<14)
#define AFE_SPDIFIN_SWITCH_REAL_OPTICAL (0x0<<15)
#define AFE_SPDIFIN_SEL_SPDIFIN_EN (0x1<<0)
#define AFE_SPDIFIN_SEL_SPDIFIN_DIS (0x0<<0)
#define AFE_SPDIFIN_SEL_SPDIFIN_CLK_EN (0x1<<1)
#define AFE_SPDIFIN_SEL_SPDIFIN_CLK_DIS (0x0<<1)
#define AFE_SPDIFIN_FIFOSTARTPOINT_5 (0x1<<4)

#define AFE_SPDIFIN_SEL_SPDIFIN_CLK_DIS (0x0<<1)
#define SPDIFRX_EN            (0x01<<0) /*SPDIFRX enabled (soft reset)*/
#define SPDIFRX_DIS           (0x00<<0)
#define SPDIFRX_FLIP          (0x01<<1) /*SPDIFRX enabled (soft reset)*/
#define SPDIFRX_NOFLIP        (0x00<<1)
#define SPDIFRX_INT_EN        (0x01<<6) /*SPDIFRX interrupt enabled*/
#define SPDIFRX_INT_DIS       (0x00<<6)
#define SPDIFRX_MAX_LEN_MASK  (0xFF << 16)
#define SPDIFRX_MAX_LEN_32KHz (0xBE << 16)
#define SPDIFRX_MAX_LEN_48KHz (0x8C << 16)
#define SPDIFRX_MAX_LEN_432M  (0xA4 << 16)
#define SPDIFRX_MAX_LEN_594M  (0xE0 << 16)
#define SPDIFRX_BITCELL_NUM256CYC (0x03 << 24)
#define SPDIFRX_INV_LRCK  (0x01 << 31)

#define AFE_SPDIFIN_CHSTS1 (0x508)
#define CHSTS_PCM         (0x00<<1)
#define CHSTS_NONPCM      (0x01<<1)
#define CHSTS_COPYRIGHT   (0x01<<2)
#define CHSTS_PRE_EMP     (0x01<<3)
#define CHSTS_NO_PRE_EMP  (0x00<<3)
#define AFE_SPDIFIN_CHSTS2 (0x50c)
#define AFE_SPDIFIN_CHSTS3 (0x510)
#define AFE_SPDIFIN_CHSTS4 (0x514)
#define AFE_SPDIFIN_CHSTS5 (0x518)
#define AFE_SPDIFIN_CHSTS6 (0x51c)

#define AFE_SPDIFIN_DEBUG1   (0x520)
#define AFE_SPDIFIN_DEBUG2   (0x524)
#define AFE_SPDIFIN_DEBUG3  (0x528)
#define AFE_SPDIFIN_EC       (0x530)
#define AFE_SPDIFIN_CLK_CFG (0x534)
/*Define the nominator value of the CALI_CLOCK/REFERENCE clock.*/
#define  SPDIFIN_REF_CALI_RATIO_NOM      (0xFF << 4)
/*16*6 for 432M ,BD  use,90 not use*/
#define  SPDIFIN_REF_CALI_RATIO_NOM_432M      (0x60 << 4)
/*22*6 for 594M // BD  use,90 not use*/
#define  SPDIFIN_REF_CALI_RATIO_NOM_594M      (0x84 << 4)
/* 4(denom)*24(624(fcali)/26(fref))*2(Posdiv),90 just use this*/
#define  SPDIFIN_REF_CALI_RATIO_NOM_624M      (0xC0 << 4)

/*Define the denominator value of the CALI_CLOCK/REFERENCE clock.// 4*/
#define  SPDIFIN_REF_CALI_RATIO_DENOM  (0x1F << 12)
/*Define the denominator value of the CALI_CLOCK/REFERENCE clock.// 4*/
#define  SPDIFIN_REF_CALI_RATIO_DENOM_624M  (0x4 << 12)

#define AFE_SPDIFIN_BUF_CFG  (0x538)

#define AFE_SPDIFIN_BR  (0x53C)
#define AFE_SPDIFIN_ARC_REAL  (0x0<<24)
#define AFE_SPDIFIN_ARC_FROM_OPTICAL  (0x1<<24)

#define AFE_SPDIFIN_BR_DBG1  (0x540)

#define AFE_SPDIFIN_INT_EXT  (0x548)
#define EDGE_FLAG_OPT  (0x1 << 28)
#define EDGE_FLAG_COA  (0x1 << 29)
#define EDGE_FLAG_ARC  (0x1 << 30)

#define AFE_SPDIFIN_INT_EXT2 (0x54C)

#define AFE_SPDIFIN_FREQ_INF   (0x550)

#define AFE_SPDIFIN_FREQ_INF_2	(0x554)

#define AFE_SPDIFIN_FREQ_INF_3	(0x558)

#define AFE_SPDIFIN_USERCODE_1	(0x560)
#define AFE_SPDIFIN_USERCODE_2	(0x564)
#define AFE_SPDIFIN_USERCODE_3	(0x568)
#define AFE_SPDIFIN_USERCODE_4	(0x56c)
#define AFE_SPDIFIN_USERCODE_5	(0x570)
#define AFE_SPDIFIN_USERCODE_6	(0x574)
#define AFE_SPDIFIN_USERCODE_7	(0x578)
#define AFE_SPDIFIN_USERCODE_8	(0x580)
#define AFE_SPDIFIN_USERCODE_9	(0x584)
#define AFE_SPDIFIN_USERCODE_10	(0x588)
#define AFE_SPDIFIN_USERCODE_11	(0x58c)
#define AFE_SPDIFIN_USERCODE_12	(0x590)

/* 6582 Add */
#define AFE_ASRC_CON13	(0x00550)
#define AFE_ASRC_CON14	(0x00554)
#define AFE_ASRC_CON15	(0x00558)
#define AFE_ASRC_CON16	(0x0055C)
#define AFE_ASRC_CON17	(0x00560)
#define AFE_ASRC_CON18	(0x00564)
#define AFE_ASRC_CON19	(0x00568)
#define AFE_ASRC_CON20	(0x0056C)

#define PWR2_TOP_CON    (0x00634)
#define LAT_DATA_EN_I2SO3_POS  (17)
#define LAT_DATA_EN_I2SO4_POS  (18)
#define LAT_DATA_EN_I2SO5_POS  (19)
#define LAT_DATA_EN_I2SO6_POS  (20)
#define PDN_ASRC_BGR_POS        16
#define PDN_DSD_ENC_POS     15
#define PDN_DSD_ENC_MASK    (1<<PDN_DSD_ENC_POS)
#define PDN_MEM_ASRC5_POS   14
#define PDN_MEM_ASRC5_MASK  (1<<PDN_MEM_ASRC5_POS)
#define PDN_MEM_ASRC4_POS   13
#define PDN_MEM_ASRC4_MASK  (1<<PDN_MEM_ASRC4_POS)
#define PDN_MEM_ASRC3_POS   12
#define PDN_MEM_ASRC3_MASK  (1<<PDN_MEM_ASRC3_POS)
#define PDN_MEM_ASRC2_POS   11
#define PDN_MEM_ASRC2_MASK  (1<<PDN_MEM_ASRC2_POS)
#define PDN_MEM_ASRC1_POS   10
#define PDN_MEM_ASRC1_MASK  (1<<PDN_MEM_ASRC1_POS)
#define PDN_ASRCO6_POS      9
#define PDN_ASRCO6_MASK     (1<<PDN_ASRCO6_POS)
#define PDN_ASRCO5_POS      8
#define PDN_ASRCO5_MASK     (1<<PDN_ASRCO5_POS)
#define PDN_ASRCO4_POS      7
#define PDN_ASRCO4_MASK     (1<<PDN_ASRCO4_POS)
#define PDN_ASRCO3_POS      6
#define PDN_ASRCO3_MASK     (1<<PDN_ASRCO3_POS)
#define PDN_ASRCI6_POS      5
#define PDN_ASRCI6_MASK     (1<<PDN_ASRCI6_POS)
#define PDN_ASRCI5_POS      4
#define PDN_ASRCI5_MASK     (1<<PDN_ASRCI5_POS)
#define PDN_ASRCI4_POS      3
#define PDN_ASRCI4_MASK     (1<<PDN_ASRCI4_POS)
#define PDN_ASRCI3_POS      2
#define PDN_ASRCI3_MASK     (1<<PDN_ASRCI3_POS)
#define PDN_DMIC2_POS       1
#define PDN_DMIC2_MASK      (1<<PDN_DMIC2_POS)
#define PDN_DMIC1_POS       0
#define PDN_DMIC1_MASK      (1<<PDN_DMIC1_POS)

#define DSD1_FADER_CON0  (0x648)
#define DSD2_FADER_CON0  (0x64C)


#define AFE_MPHONE_MULTI_CON0    (0x6a4)
#define HW_EN_MASK                     ((unsigned)0x1 << 0)
#define HW_EN                                    ((unsigned)0x1 << 0)
#define HW_DISABLE                       ((unsigned)0x0 << 0)
#define DATA_BIT_MASK                              ((unsigned)0x1 << 1)  /* bit 1 : 0 : 16-bit, 1 : 24-bit */
#define DATA_16BIT                                      ((unsigned)0x0 << 1)  /* 16-bit */
#define DATA_24BIT                                      ((unsigned)0x1 << 1)  /* 24-bit */
#define DATA_16BIT_SWAP_MASK                           ((unsigned)0x1 << 3) /* 1<<3 */
#define DATA_16BIT_NON_SWAP               ((unsigned)0x0 << 3) /* 0<<3 */
#define DATA_16BIT_SWAP                           ((unsigned)0x1 << 3)/* 1<<3 */
#define INTR_PERIOD_MASK             ((unsigned)0x3 << 4) /* Interrupt period */
#define INTR_PERIOD_DISABLE  ((unsigned)0x0 << 4)  /* 0<<4  : SPDIF In , disable */
#define INTR_PERIOD_32                ((unsigned)0x0 << 4)  /*0<<4  : Multiple Line In , 32 double words */
#define INTR_PERIOD_64                ((unsigned)0x1 << 4) /*1<<4 */
#define INTR_PERIOD_128              ((unsigned)0x2 << 4) /* 2<<4 */
#define INTR_PERIOD_256              ((unsigned)0x3 << 4) /* 3<<4 */
#define SPDIF_PRE_DETECT          ((unsigned)0x1 << 6)  /* 1 <<6    : Multiple Line In */
#define CLK_SEL_MASK             (0x1 << 12)
#define CLK_SEL_8CH_I2S          (0x1 << 12)
#define CLK_SEL_HDMI_RX          (0x0 << 12)
#define SDATA_SEL_MASK           (0x1 << 13)
#define SDATA_SEL_8CH_I2S        (0x1 << 13)
#define SDATA_SEL_HDMI_RX        (0x0 << 13)

#define AFE_MPHONE_MULTI_CON1    (0x6a8)
#define MULTI_RESET_MASK         ((unsigned)0x1 << 27)
#define MULTI_RESET_ENABLE       (0)
#define MULTI_RESET_DISABLE      ((unsigned)0x1 << 27)
#define AINACK_MULTI_SYNC_MASK   ((unsigned)0x1 << 24)   /* to avoid L/R Rervse when turn on.*/
#define MULTI_SYNC_DISABLE   ((unsigned)0x0 << 24)
#define MULTI_SYNC_ENABLE   ((unsigned)0x1 << 24)
#define MULTI_INV_BCK       (0x1<<23)
#define MULTI_24BIT_SWAP_MASK           ((unsigned)0x1<<22)
#define MULTI_24BIT_NO_SWAP         ((unsigned)0x1<<22)
#define MULTI_24BIT_SWAP            ((unsigned)0x0<<22)
#define MULTI_DSD_MODE_MASK             ((unsigned)0x1<<21)
#define MULTI_DSD_8BIT_MODE         ((unsigned)0x1<<21)
#define MULTI_DSD_24BIT_MODE        ((unsigned)0x0<<21)
#define AINACK_CFG_IN_MODE               ((unsigned)0x1 << 20)   /* Input Mode Selection */
#define AINACK_CFG_PCM_MODE    ((unsigned)0x0 << 20)    /* PCM Mode */
#define AINACK_CFG_DSD_MODE    ((unsigned)0x1 << 20)    /* DSD Mode */
#define MULTI_NONE_COMPACT_POS	(19)
#define MULTI_NONE_COMPACT_MASK  (0x1<<19)
#define MULTI_NONE_COMPACT_POS (19)
#define AINACK_HBRMOD_MASK         ((unsigned)0x1 << 18)    /* HBR Mode Mask */
#define AINACK_NONHBR_MODE      ((unsigned)0x0 << 18)    /* non-HBR Mode : L0L1L2L3R0R1R2R3 */
#define AINACK_HBR_MODE                ((unsigned)0x1 << 18)    /* HBR Mode : L0R0L1R1L2R2L3R3 */
#define AINACK_CFG_LRCK_MASK      ((unsigned)0x3 << 16)
#define AINACK_CFG_LRCK_SEL_16    ((unsigned)0x0 << 16)    /* LRCK 16 cycle */
#define AINACK_CFG_LRCK_SEL_24    ((unsigned)0x1 << 16)    /* LRCK 24 cycle */
#define AINACK_CFG_LRCK_SEL_32    ((unsigned)0x2 << 16)    /* LRCK 32 cycle */
#define AINACK_CFG_LRCK_SEL_0    ((unsigned)0x3 << 16)      /* LRCK 0 cycle */
#define AINACK_CFG_INV_LRCK           ((unsigned)0x1 << 15)    /* Inverse LRCK */
#define AINACK_CFG_CLK_MASK                ((unsigned)0x3 << 13)
#define AINACK_CFG_CLK_RJ                ((unsigned)0x0 << 13)    /* Clock mode : right just */
#define AINACK_CFG_CLK_LJ                ((unsigned)0x1 << 13)     /* Clock mode : left just */
#define AINACK_CFG_CLK_I2S                ((unsigned)0x3 << 13)    /* Clock mode : I2S */
#define AINACK_CFG_BITNUM_MASK    ((unsigned)0x1F << 8)     /* DAC bit number */
#define AINACK_CFG_BITNUM_16     ((unsigned)0x0F << 8)    /* DAC bit number 24 */
#define AINACK_CFG_BITNUM_24     ((unsigned)0x17 << 8)    /* DAC bit number 24 */
#define AINACK_CFG_INT_MASK  ((unsigned)0x1 << 6)   /* 0 : no effect (SPDIF) , 1 : interrupt select multi*/
#define AINACK_CFG_INT_SPD                 ((unsigned)0x0 << 6)    /* SPDIF INT */
#define AINACK_CFG_INT_MULTI           ((unsigned)0x1 << 6)    /* Multiline INT */
#define AINACK_CFG_ADDR_UPDATE_MASK    ((unsigned)0x1 << 5)
#define AINACK_CFG_DRAM_ALE     ((unsigned)0x0 << 5)    /* 0 : with DRAM ALE */
#define AINACK_CFG_SMP_CNT         ((unsigned)0x1 << 5)   /* 1 : with input sample count */
#define AINACK_CFG_FS_DOWN_MASK    ((unsigned)0x3 << 2)
#define AINACK_CFG_FS_DOWN_0     ((unsigned)0x0 << 2)    /* no down sample */
#define AINACK_CFG_FS_DOWN_2     ((unsigned)0x1 << 2)    /* 1/2 down sample */
#define AINACK_CFG_FS_DOWN_4     ((unsigned)0x2 << 2)    /* 1/4 down sample */
#define AINACK_CFG_FS_DOWN_8    ((unsigned)0x3 << 2)     /* 1/8 down sample */
#define AINACK_CFG_CH_NUM_MASK    ((unsigned)0x3 << 0)
#define AINACK_CFG_CH_NUM_2     ((unsigned)0x0 << 0)    /* audio 2 channels for line in module.*/
#define AINACK_CFG_CH_NUM_4     ((unsigned)0x1 << 0)    /* audio 4 channels for line in module.*/
#define AINACK_CFG_CH_NUM_6     ((unsigned)0x2 << 0)    /* audio 6 channels for line in module.*/
#define AINACK_CFG_CH_NUM_8    ((unsigned)0x3 << 0)     /* audio 8 channels for line in module.*/

#define AFE_MPHONE_MULTI_MON	(0x6b0)

#define PWR1_ASM_CON1  (0x0108)
#define I2S_IN1_ASRC_CALI_CK_SEL_MASK   (0x1 << 2)
#define I2S_IN1_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 2)
#define I2S_IN1_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 2)
#define I2S_IN2_ASRC_CALI_CK_SEL_MASK   (0x1 << 5)
#define I2S_IN2_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 5)
#define I2S_IN2_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 5)
#define I2S_OUT1_ASRC_CALI_CK_SEL_MASK   (0x1 << 8)
#define I2S_OUT1_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 8)
#define I2S_OUT1_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 8)
#define I2S_OUT2_ASRC_CALI_CK_SEL_MASK   (0x1 << 11)
#define I2S_OUT2_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 11)
#define I2S_OUT2_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 11)

#define PWR2_ASM_CON1  (0x1070)
#define I2S_IN3_ASRC_CALI_CK_SEL_MASK   (0x1 << 2)
#define I2S_IN3_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 2)
#define I2S_IN3_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 2)
#define I2S_IN4_ASRC_CALI_CK_SEL_MASK   (0x1 << 5)
#define I2S_IN4_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 5)
#define I2S_IN4_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 5)
#define I2S_IN5_ASRC_CALI_CK_SEL_MASK   (0x1 << 8)
#define I2S_IN5_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 8)
#define I2S_IN5_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 8)
#define I2S_IN6_ASRC_CALI_CK_SEL_MASK   (0x1 << 11)
#define I2S_IN6_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 11)
#define I2S_IN6_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 11)
#define I2S_OUT3_ASRC_CALI_CK_SEL_MASK   (0x1 << 14)
#define I2S_OUT3_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 14)
#define I2S_OUT3_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 14)
#define I2S_OUT4_ASRC_CALI_CK_SEL_MASK   (0x1 << 17)
#define I2S_OUT4_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 17)
#define I2S_OUT4_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 17)
#define I2S_OUT5_ASRC_CALI_CK_SEL_MASK   (0x1 << 20)
#define I2S_OUT5_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 20)
#define I2S_OUT5_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 20)
#define I2S_OUT6_ASRC_CALI_CK_SEL_MASK   (0x1 << 23)
#define I2S_OUT6_ASRC_CALI_CK_SEL_A1SYS  (0x0 << 23)
#define I2S_OUT6_ASRC_CALI_CK_SEL_A2SYS  (0x1 << 23)

#define AFE_ASRC_NEW_CON0   (0x0800)
#define ASRC_ONE_HEART_MODE_MASK (0x1 << 31)
#define ASRC_ONE_HEART_MODE_OFF  (0x0 << 31)
#define ASRC_ONE_HEART_MODE_ON   (0x1 << 31)
#define O16BIT_MASK     (0x1<<19)
#define CLR_IIR_HISTORY_MASK (0x1<<17)
#define CLR_IIR_HISTORY      (0x1<<17)
#define IS_MONO_MASK    (0x1<<16)
#define IIR_EN_MASK     (0x1<<11)
#define IIR_EN          (0x1<<11)
#define IIR_DIS         (0x0<<11)
#define IIR_STAGE_MASK  (0x7<<8)
#define IIR_STAGE_8     (0x7<<8)
#define IIR_STAGE_7     (0x6<<8)
#define ASM_ON_MASK     (0x1<<0)
#define AFE_ASRC_NEW_CON1   (0x0804)
#define AFE_ASRC_NEW_CON2   (0x0808)
#define AFE_ASRC_NEW_CON3   (0x080C)
#define AFE_ASRC_NEW_CON4   (0x0810)
#define AFE_ASRC_NEW_CON5   (0x0814)
#define AFE_ASRC_NEW_CON6   (0x0818)
#define AFE_ASRC_NEW_CON7   (0x081C)
#define AFE_ASRC_NEW_CON10  (0x0828)
#define AFE_ASRC_NEW_CON11  (0x082C)
#define AFE_ASRC_NEW_CON13  (0x0834)
#define AFE_ASRC_NEW_CON14  (0x0838)

#define AFE_ASRCI2_NEW_CON0   (0x0840)
#define AFE_ASRCI2_NEW_CON1   (0x0844)
#define AFE_ASRCI2_NEW_CON2   (0x0848)
#define AFE_ASRCI2_NEW_CON3   (0x084C)
#define AFE_ASRCI2_NEW_CON4   (0x0850)
#define AFE_ASRCI2_NEW_CON5   (0x0854)
#define AFE_ASRCI2_NEW_CON6   (0x0858)
#define AFE_ASRCI2_NEW_CON7   (0x085C)
#define AFE_ASRCI2_NEW_CON10  (0x0868)
#define AFE_ASRCI2_NEW_CON11  (0x086C)
#define AFE_ASRCI2_NEW_CON13  (0x0874)
#define AFE_ASRCI2_NEW_CON14  (0x0878)

#define AFE_ASRCI3_NEW_CON0   (0x0880)
#define AFE_ASRCI3_NEW_CON1   (0x0884)
#define AFE_ASRCI3_NEW_CON2   (0x0888)
#define AFE_ASRCI3_NEW_CON3   (0x088C)
#define AFE_ASRCI3_NEW_CON4   (0x0890)
#define AFE_ASRCI3_NEW_CON5   (0x0894)
#define AFE_ASRCI3_NEW_CON6   (0x0898)
#define AFE_ASRCI3_NEW_CON7   (0x089C)
#define AFE_ASRCI3_NEW_CON10  (0x08A8)
#define AFE_ASRCI3_NEW_CON11  (0x08AC)
#define AFE_ASRCI3_NEW_CON13  (0x08B4)
#define AFE_ASRCI3_NEW_CON14  (0x08B8)

#define AFE_ASRCI4_NEW_CON0   (0x08C0)
#define AFE_ASRCI4_NEW_CON1   (0x08C4)
#define AFE_ASRCI4_NEW_CON2   (0x08C8)
#define AFE_ASRCI4_NEW_CON3   (0x08CC)
#define AFE_ASRCI4_NEW_CON4   (0x08D0)
#define AFE_ASRCI4_NEW_CON5   (0x08D4)
#define AFE_ASRCI4_NEW_CON6   (0x08D8)
#define AFE_ASRCI4_NEW_CON7   (0x08DC)
#define AFE_ASRCI4_NEW_CON10  (0x08E8)
#define AFE_ASRCI4_NEW_CON11  (0x08EC)
#define AFE_ASRCI4_NEW_CON13  (0x08F4)
#define AFE_ASRCI4_NEW_CON14  (0x08F8)

#define AFE_ASRCI5_NEW_CON0   (0x0900)
#define AFE_ASRCI5_NEW_CON1   (0x0904)
#define AFE_ASRCI5_NEW_CON2   (0x0908)
#define AFE_ASRCI5_NEW_CON3   (0x090C)
#define AFE_ASRCI5_NEW_CON4   (0x0910)
#define AFE_ASRCI5_NEW_CON5   (0x0914)
#define AFE_ASRCI5_NEW_CON6   (0x0918)
#define AFE_ASRCI5_NEW_CON7   (0x091C)
#define AFE_ASRCI5_NEW_CON10  (0x0928)
#define AFE_ASRCI5_NEW_CON11  (0x092C)
#define AFE_ASRCI5_NEW_CON13  (0x0934)
#define AFE_ASRCI5_NEW_CON14  (0x0938)

#define AFE_ASRCI6_NEW_CON0 (0x1100)
#define AFE_ASRCI6_NEW_CON1 (0x1104)
#define AFE_ASRCI6_NEW_CON2 (0x1108)
#define AFE_ASRCI6_NEW_CON3 (0x110C)
#define AFE_ASRCI6_NEW_CON6 (0x1118)
#define AFE_ASRCI6_NEW_CON7 (0x111C)
#define AFE_ASRCI6_NEW_CON10  (0x1128)
#define AFE_ASRCI6_NEW_CON11  (0x112C)
#define AFE_ASRCI6_NEW_CON13  (0x1134)
#define AFE_ASRCI6_NEW_CON14  (0x1138)

#define AFE_ASRCPCMI_NEW_CON0 (0xA80)
#define AFE_ASRCPCMI_NEW_CON1 (0xA84)
#define AFE_ASRCPCMI_NEW_CON2 (0xA88)
#define AFE_ASRCPCMI_NEW_CON3 (0xA8C)
#define AFE_ASRCPCMI_NEW_CON4 (0xA90)
#define AFE_ASRCPCMI_NEW_CON5 (0xA94)
#define AFE_ASRCPCMI_NEW_CON6 (0xA98)
#define AFE_ASRCPCMI_NEW_CON7 (0xA9C)
#define AFE_ASRCPCMI_NEW_CON10  (0xAA8)
#define AFE_ASRCPCMI_NEW_CON11  (0xAAC)
#define AFE_ASRCPCMI_NEW_CON13  (0xAB4)
#define AFE_ASRCPCMI_NEW_CON14  (0xAB8)

#define AFE_ASRCO1_NEW_CON0   (0x0940)
#define AFE_ASRCO1_NEW_CON1   (0x0944)
#define AFE_ASRCO1_NEW_CON2   (0x0948)
#define AFE_ASRCO1_NEW_CON3   (0x094C)
#define AFE_ASRCO1_NEW_CON4   (0x0950)
#define AFE_ASRCO1_NEW_CON5   (0x0954)
#define AFE_ASRCO1_NEW_CON6   (0x0958)
#define AFE_ASRCO1_NEW_CON7   (0x095C)
#define AFE_ASRCO1_NEW_CON10  (0x0968)
#define AFE_ASRCO1_NEW_CON11  (0x096C)
#define AFE_ASRCO1_NEW_CON13  (0x0974)
#define AFE_ASRCO1_NEW_CON14  (0x0978)

#define AFE_ASRCO2_NEW_CON0   (0x0980)
#define AFE_ASRCO2_NEW_CON1   (0x0984)
#define AFE_ASRCO2_NEW_CON2   (0x0988)
#define AFE_ASRCO2_NEW_CON3   (0x098C)
#define AFE_ASRCO2_NEW_CON4   (0x0990)
#define AFE_ASRCO2_NEW_CON5   (0x0994)
#define AFE_ASRCO2_NEW_CON6   (0x0998)
#define AFE_ASRCO2_NEW_CON7   (0x099C)
#define AFE_ASRCO2_NEW_CON10  (0x09A8)
#define AFE_ASRCO2_NEW_CON11  (0x09AC)
#define AFE_ASRCO2_NEW_CON13  (0x09B4)
#define AFE_ASRCO2_NEW_CON14  (0x09B8)

#define AFE_ASRCO3_NEW_CON0   (0x09C0)
#define AFE_ASRCO3_NEW_CON1   (0x09C4)
#define AFE_ASRCO3_NEW_CON2   (0x09C8)
#define AFE_ASRCO3_NEW_CON3   (0x09CC)
#define AFE_ASRCO3_NEW_CON4   (0x09D0)
#define AFE_ASRCO3_NEW_CON5   (0x09D4)
#define AFE_ASRCO3_NEW_CON6   (0x09D8)
#define AFE_ASRCO3_NEW_CON7   (0x09DC)
#define AFE_ASRCO3_NEW_CON10  (0x09E8)
#define AFE_ASRCO3_NEW_CON11  (0x09EC)
#define AFE_ASRCO3_NEW_CON13  (0x09F4)
#define AFE_ASRCO3_NEW_CON14  (0x09F8)

#define AFE_ASRCO4_NEW_CON0   (0x0A00)
#define AFE_ASRCO4_NEW_CON1   (0x0A04)
#define AFE_ASRCO4_NEW_CON2   (0x0A08)
#define AFE_ASRCO4_NEW_CON3   (0x0A0C)
#define AFE_ASRCO4_NEW_CON4   (0x0A10)
#define AFE_ASRCO4_NEW_CON5   (0x0A14)
#define AFE_ASRCO4_NEW_CON6   (0x0A18)
#define AFE_ASRCO4_NEW_CON7   (0x0A1C)
#define AFE_ASRCO4_NEW_CON10  (0x0A28)
#define AFE_ASRCO4_NEW_CON11  (0x0A2C)
#define AFE_ASRCO4_NEW_CON13  (0x0A34)
#define AFE_ASRCO4_NEW_CON14  (0x0A38)

#define AFE_ASRCO5_NEW_CON0   (0x0A40)
#define AFE_ASRCO5_NEW_CON1   (0x0A44)
#define AFE_ASRCO5_NEW_CON2   (0x0A48)
#define AFE_ASRCO5_NEW_CON3   (0x0A4C)
#define AFE_ASRCO5_NEW_CON4   (0x0A50)
#define AFE_ASRCO5_NEW_CON5   (0x0A54)
#define AFE_ASRCO5_NEW_CON6   (0x0A58)
#define AFE_ASRCO5_NEW_CON7   (0x0A5C)
#define AFE_ASRCO5_NEW_CON10  (0x0A68)
#define AFE_ASRCO5_NEW_CON11  (0x0A6C)
#define AFE_ASRCO5_NEW_CON13  (0x0A74)
#define AFE_ASRCO5_NEW_CON14  (0x0A78)

#define AFE_ASRCO6_NEW_CON0   (0x1140)
#define AFE_ASRCO6_NEW_CON1   (0x1144)
#define AFE_ASRCO6_NEW_CON2   (0x1148)
#define AFE_ASRCO6_NEW_CON3   (0x114C)
#define AFE_ASRCO6_NEW_CON4   (0x1150)
#define AFE_ASRCO6_NEW_CON5   (0x1154)
#define AFE_ASRCO6_NEW_CON6   (0x1158)
#define AFE_ASRCO6_NEW_CON7   (0x115C)
#define AFE_ASRCO6_NEW_CON10  (0x1168)
#define AFE_ASRCO6_NEW_CON11  (0x116C)
#define AFE_ASRCO6_NEW_CON13  (0x1174)
#define AFE_ASRCO6_NEW_CON14  (0x1178)

#define DSD_ENC_CON0          (0x11C0)
#define DSD_ENC_CON1          (0x11C4)
#define DSD_ENC_CON2          (0x11C8)

#define AFE_ASRCPCMO_NEW_CON0   (0xAC0)
#define AFE_ASRCPCMO_NEW_CON1   (0xAC4)
#define AFE_ASRCPCMO_NEW_CON2   (0xAC8)
#define AFE_ASRCPCMO_NEW_CON3   (0xACC)
#define AFE_ASRCPCMO_NEW_CON4   (0xAD0)
#define AFE_ASRCPCMO_NEW_CON5   (0xAD4)
#define AFE_ASRCPCMO_NEW_CON6   (0xAD8)
#define AFE_ASRCPCMO_NEW_CON7   (0xADC)
#define AFE_ASRCPCMO_NEW_CON10  (0xAE8)
#define AFE_ASRCPCMO_NEW_CON11  (0xAEC)
#define AFE_ASRCPCMO_NEW_CON13  (0xAF4)
#define AFE_ASRCPCMO_NEW_CON14  (0xAF8)

#define AFE_DMIC_TOP_CON        (0x1180)
#define AFE_DMIC2_TOP_CON       (0x1030)
/* AFE_DMIC_TOP_CON and AFE_DMIC2_TOP_CON */
	#define DMIC_COEF_CFG_MASK	(0x1 << 31)
		#define DMIC_COEF_CFG_SW	(0x1 << 31)
		#define DMIC_COEF_CFG_FIX	(0)
	#define DMIC_4CH_MODE_MASK      (0x1 << 29)
		#define DMIC_4CH_MODE_ON    (0x1 << 29)
		#define DMIC_4CH_MODE_OFF   (0)
	#define DMIC_RAMP_GEN_RON_MASK	(0x1 << 28)
		#define DMIC_RAMP_GEN_RON	(0x1 << 28)
		#define DMIC_RAMP_GEN_ROFF	(0)
	#define DMIC_RAMP_GEN_LON_MASK	(0x1 << 27)
		#define DMIC_RAMP_GEN_LON	(0x1 << 27)
		#define DMIC_RAMP_GEN_LOFF	(0)
	#define DMIC_WIRE_MODE_MASK	(0x1 << 26)
		#define DMIC_ONE_WIRE_MODE	(0)
		#define DMIC_TWO_WIRE_MODE	(0x1 << 26)
	#define DMIC_CH2_CK_PHASE_MASK	(0x7 << 23)
	#define DMIC_CH2_CK_PHASE_POS	(23)
	#define DMIC_CH1_CK_PHASE_MASK	(0x7 << 20)
	#define DMIC_CH1_CK_PHASE_POS	(20)
	#define DMIC_TIMING_ON_MASK	(0x1 << 19)
		#define DMIC_TIMING_ON	(0x1 << 19)
		#define DMIC_TIMING_OFF	(0)
	#define DMIC_SINE_GEN_RON_MASK	(0x1 << 18)
		#define DMIC_SINE_GEN_RON	(0x1 << 18)
		#define DMIC_SINE_GEN_ROFF	(0x0)
	#define DMIC_SINE_GEN_LON_MASK	(0x1 << 17)
		#define DMIC_SINE_GEN_LON	(0x1 << 17)
		#define DMIC_SINE_GEN_LOFF	(0)
	#define DMIC_SINE_MUTE_MASK	(0x1 << 16)
		#define DMIC_SINE_MUTE	(0x1 << 16)
		#define DMIC_SINE_UNMUTE	(0)
	#define DMIC_SINE_GEN_AMP_DIV_MASK	(0x7 << 13)
	#define DMIC_SINE_GEN_AMP_DIV_POS	(13)
	#define DMIC_SINE_GEN_FREQ_DIV_MASK	(0x1F << 8)
	#define DMIC_SINE_GEN_FREQ_DIV_POS	(8)
	#define DMIC_IIR_ON_MASK	(0x1 << 7)
		#define DMIC_IIR_ON	(0x1 << 7)
		#define DMIC_IIR_OFF	(0)
	#define DMIC_IIR_MODE_MASK	(0x7 << 4)
	#define DMIC_IIR_MODE_POS	(0x4)
	#define DMIC_VOICE_MODE_MASK	(0x3 << 2)
		#define DMIC_VOID_MODE_8K	(0)
		#define DMIC_VOID_MODE_16K	(0x1 << 2)
		#define DMIC_VOID_MODE_32K	(0x2 << 2)
		#define DMIC_VOID_MODE_48_44K	(0x3 << 2)
	#define DMIC_DOMAIN_MASK	(0x1 << 1)
		#define DMIC_DOMAIN_48K	(0x0)
		#define DMIC_DOMAIN_44K	(0x1 << 1)
	#define DMIC_CIC_ON_MASK	(0x1)
		#define DMIC_CIC_ON	(0x1)
		#define DMIC_CIC_OFF	(0)

#define AFE_MEMIF_PBUF_SIZE (0x1238)
#define DLMCH_OUT_SEL_MASK   (0x1<<29)
#define DLMCH_OUT_SEL_PAIR_INTERLEAVE  (0x0<<29)
#define DLMCH_OUT_SEL_FULL_INTERLEAVE  (0x1<<29)
#define DLMCH_BIT_WIDTH_MASK   (0x1<<28)
#define DLMCH_CH_NUM_MASK           (0xF<<24)
#define DLMCH_PBUF_SIZE_MASK    (0x3<<12)
#define DLMCH_PBUF_SIZE_32BYTES (0x1<<12)
#define AFE_ULMCH_BASE      (0x1360)
#define AFE_ULMCH_END       (0x1368)
#define AFE_ULMCH_CUR       (0x136C)

#define AFE_LRCK_CNT     (0x1088)
#define LRCK_COUNTER_NUMBER   (0x1fff<<19)
#define LRCK_COUNTING_STATUS  (0x1<<18)
#define SLAVE_LRCK_SEL_MASK        (0x7<<1)
#define LRCK_COUNT_MASK       (0x1<<0)
#define LRCK_COUNT_ENABLE     (0x1<<0)
#define LRCK_COUNT_DISABLE    (0x0<<0)

/**********************************
 *  Detailed Definitions
 **********************************/
/* AUDIO_TOP_CON0 */
#define PDN_AFE               (1 << 2)
/* The below 3 bits for bus setting must be set 100 */
#define APB3_SEL              (1 << 14)
#define APB_R2T               (1 << 13)
#define APB_W2T               (1 << 12)
#define PDN_APLL_TUNER_CK     (1 << 19)
#define PDN_HDMI_CK           (1 << 20)
#define PDN_SPDIF_CK          (1 << 21)
#define PDN_SPDIF2_CK         (1 << 22)
#define PDN_APLL_CK           (1 << 23)
/* AUDIO_TOP_CON2 */
#define SPDIF_CK_DIV_MASK     (0xFF)
#define SPDIF2_CK_DIV_MASK    (0xFF << 16)
#define HDMI_CK_DIV_MASK      (0x3F << 8)

/* AUDIO_TOP_CON3 */
#define HDMI_IEC_FROM_SEL_MASK    (1 << 16)
#define HDMI_IEC_FROM_SEL_SPDIF  0
#define HDMI_IEC_FROM_SEL_SPDIF2 (1 << 16)
#define PAD_IEC_FROM_SEL_MASK     (1 << 17)
#define PAD_IEC_FROM_SEL_SPDIF2   0
#define PAD_IEC_FROM_SEL_SPDIF    (1 << 17)
#define SPEAKER_OUT_HDMI_SEL_MASK (0x3 << 6)

#define SPEAKER_CLOCK_AND_DATA_FROM_MASK      (0x1 << 5)
#define SPEAKER_CLOCK_AND_DATA_FROM_ORI   0
#define SPEAKER_CLOCK_AND_DATA_FROM_HDMI  (0x1 << 5)

/* AFE_DAC_CON0 */
/* #define AFE_ON      0 */
/* #define DL1_ON      1 */
/* #define DL2_ON      2 */
#define VUL_ON      3
#define DAI_ON      4
#define I2S_ON      5
#define AWB_ON      6
#define MOD_PCM_ON  7
#define AFE_ON_RETM  12
#define AFE_DL1_RETM 13
#define AFE_DL2_RETM 14
#define AFE_AWB_RETM 16

/* AFE_DAC_CON1 */
#define DL1_MODE_LEN    4
#define DL1_MODE_POS    0

#define DL2_MODE_LEN    4
#define DL2_MODE_POS    4

#define I2S_MODE_LEN    4
#define I2S_MODE_POS    8

#define AWB_MODE_LEN    4
#define AWB_MODE_POS    12

#define VUL_MODE_LEN    4
#define VUL_MODE_POS    16

#define DAI_MODE_LEN    1
#define DAI_MODE_POS    20

#define DL1_DATA_LEN    1
#define DL1_DATA_POS    21

#define DL2_DATA_LEN    1
#define DL2_DATA_POS    22

#define I2S_DATA_LEN    1
#define I2S_DATA_POS    23

#define AWB_DATA_LEN    1
#define AWB_DATA_POS    24

#define AWB_R_MONO_LEN  1
#define AWB_R_MONO_POS  25

#define VUL_DATA_LEN    1
#define VUL_DATA_POS    27

#define VUL_R_MONO_LEN  1
#define VUL_R_MONO_POS  28

#define DAI_DUP_WR_LEN  1
#define DAI_DUP_WR_POS  29

#define MOD_PCM_MODE_LEN    1
#define MOD_PCM_MODE_POS    30

#define MOD_PCM_DUP_WR_LEN  1
#define MOD_PCM_DUP_WR_POS  31

/* AFE_I2S_CON1 and AFE_I2S_CON2  [AFE_I2S_CON3 (6582)] */
#define AI2S_EN_POS             0
#define AI2S_EN_LEN             1
#define AI2S_WLEN_POS           1
#define AI2S_WLEN_LEN           1
#define AI2S_FMT_POS            3
#define AI2S_FMT_LEN            1
#define AI2S_OUT_MODE_POS       8
#define AI2S_OUT_MODE_LEN       4
#define AI2S_UPDATE_WORD_POS    24
#define AI2S_UPDATE_WORD_LEN    5
#define AI2S_LR_SWAP_POS        31
#define AI2S_LR_SWAP_LEN        1


#define I2S_EN_POS          0
#define I2S_EN_LEN          1
#define I2S_WLEN_POS        1
#define I2S_WLEN_LEN        1
#define I2S_SRC_POS         2
#define I2S_SRC_LEN         1
#define I2S_FMT_POS         3
#define I2S_FMT_LEN         1
#define I2S_DIR_POS         4
#define I2S_DIR_LEN         1
#define I2S_OUT_MODE_POS    8
#define I2S_OUT_MODE_LEN    4
#define I2S_PHASE_SHIFT_FIX_POS    31
#define I2S_PHASE_SHIFT_FIX_LEN    1


#define FOC_EN_POS  0
#define FOC_EN_LEN  1


/* Morning add for 2701 PCM intf test */
#define PCM_INTF_CON1       (0x063C)
#define FIX_VALUE_SEL_POS        (31)
#define FIX_VALUE_SEL_MASK       (1<<FIX_VALUE_SEL_POS)
#define BUFFER_LOOPBACK_POS      (30)
#define BUFFER_LOOPBACK_MASK     (1<<BUFFER_LOOPBACK_POS)
#define PARALLEL_LOOPBACK_POS    (29)
#define PARALLEL_LOOPBACK_MASK   (1<<PARALLEL_LOOPBACK_POS)
#define SERIAL_LOOPBACK_POS      (28)
#define SERIAL_LOOPBACK_MASK     (1<<SERIAL_LOOPBACK_POS)
#define DAI_PCM_LOOPBACK_POS     (27)
#define DAI_PCM_LOOPBACK_MASK    (1<<DAI_PCM_LOOPBACK_POS)
#define I2S_PCM_LOOPBACK_POS     (26)
#define I2S_PCM_LOOPBACK_ON      (1<<I2S_PCM_LOOPBACK_POS)
#define I2S_PCM_LOOPBACK_OFF     (0<<I2S_PCM_LOOPBACK_POS)
#define I2S_PCM_LOOPBACK_MASK    (1<<I2S_PCM_LOOPBACK_POS)
#define SYNC_DELSEL_POS          (25)
#define SYNC_DELSEL_MASK         (1<<SYNC_DELSEL_POS)
#define TX_LR_SWAP_POS           (24)
#define TX_LR_SWAP_MASK          (1<<TX_LR_SWAP_POS)
#define SYNC_OUT_INV_POS         (23)
#define SYNC_OUT_INV_MASK        (1<<SYNC_OUT_INV_POS)
#define BCLK_OUT_INV_POS         (22)
#define BCLK_OUT_INV_MASK        (1<<BCLK_OUT_INV_POS)
#define SYNC_IN_INV_POS          (21)
#define SYNC_IN_INV_MASK         (1<<SYNC_IN_INV_POS)
#define BCLK_IN_INV_POS          (20)
#define BCLK_IN_INV_MASK         (1<<BCLK_IN_INV_POS)
#define TX_LCH_RPT_POS           (19)
#define TX_LCH_RPT_MASK          (1<<TX_LCH_RPT_POS)
#define VBT_16K_MODE_POS         (18)
#define VBT_16K_MODE_MASK        (1<<VBT_16K_MODE_POS)
#define EXT_MODEM_POS            (17)
#define EXT_MODEM_MASK           (1<<EXT_MODEM_POS)
#define PCM_INT_MD               (0x0 << EXT_MODEM_POS)
#define PCM_EXT_MD               (0x1 << EXT_MODEM_POS)
#define DATA_TX2RX_POS           (14)
#define DATA_TX2RX_MASK          (1 << DATA_TX2RX_POS)
#define SYNC_LENGTH_POS          (9)
#define SYNC_LENGTH_MASK         (0x1F << SYNC_LENGTH_POS)
#define SYNC_LENGTH_16_BCK       (0x11 << SYNC_LENGTH_POS)
#define SYNC_TYPE_POS            (8)
#define SYNC_TYPE_MASK           (1 << SYNC_TYPE_POS)
#define BT_MODE_POS              (7)
#define BT_MODE_MASK             (1 << BT_MODE_POS)
#define BT_MODE_DUAL             (0 << BT_MODE_POS)
#define BT_MODE_SINGLE             (1 << BT_MODE_POS)
#define BYP_ASRC_POS             (6)
#define BYP_ASRC_MASK            (1 << BYP_ASRC_POS)
#define PCM_GO_ASRC              (0x0 << BYP_ASRC_POS)
#define PCM_GO_ASYNC_FIFO        (0x1 << BYP_ASRC_POS)
#define PCM_SLAVE_POS            (5)
#define PCM_SLAVE_MASK           (1 << PCM_SLAVE_POS)
#define PCM_MASTER_MODE           (0x0 << PCM_SLAVE_POS)
#define PCM_SLAVE_MODE          (0x1 << PCM_SLAVE_POS)
#define PCM_WLEN_POS             (4)
#define PCM_WLEN_MASK            (1 << PCM_WLEN_POS)
#define PCM_MODE_POS             (3)
#define PCM_MODE_MASK            (0x3 << PCM_MODE_POS)
#define PCM_8K					 (0x0 << PCM_MODE_POS)
#define PCM_16K					 (0x1 << PCM_MODE_POS)
#define PCM_32K					 (0x2 << PCM_MODE_POS)
#define PCM_48K					 (0x3 << PCM_MODE_POS)
#define PCM_FMT_POS              (1)
#define PCM_FMT_MASK             (0x3 << PCM_FMT_POS)
#define PCM_FMT_I2S				 (0x0 << PCM_FMT_POS)
#define PCM_FMT_EIAJ			 (0x1 << PCM_FMT_POS)
#define PCM_FMT_MODEA			 (0x2 << PCM_FMT_POS)
#define PCM_FMT_MODEB			 (0x3 << PCM_FMT_POS)
#define PCM_EN_POS               (0)
#define PCM_EN_MASK              (1 << PCM_EN_POS)

#define PCM_BCK_POS              (14)
#define PCM_BCK_MASK             (0x3 << PCM_BCK_POS)
#define PCM_BCK_32               (0x0 << PCM_BCK_POS)
#define PCM_BCK_64               (0x1 << PCM_BCK_POS)
#define PCM_24BIT_POS            (16)
#define PCM_24BIT_MASK           (1 << PCM_24BIT_POS)
#define PCM_WLEN_16BIT           (0x0 << PCM_24BIT_POS)
#define PCM_WLEN_32BIT           (0x1 << PCM_24BIT_POS)

#define AFE_MUX_SEL_CFG          (0x1740)
#define AMUTE_2CH_SEL_MASK	(0x1)
#define AMUTE_2CH_SEL_2CH	(0x0)
#define AMUTE_2CH_SEL_12CH	(0x1)
#define AMUTE_12CH_SEL_MASK	(0x1 << 1)
#define AMUTE_12CH_SEL_12CH	(0)
#define AMUTE_12CH_SEL_2CH	(0x1 << 1)
#define DMIC1_IN_L_SEL_MASK	(0x3 << 2)
#define DMIC1_IN_L_SEL_DMIC0_DAT	(0)
#define DMIC1_IN_L_SEL_DMIC0_DAT_R	(1 << 2)
#define	DMIC1_IN_L_SEL_DMIC1_DAT	(2 << 2)
#define DMIC1_IN_R_SEL_MASK	(0x3 << 4)
#define DMIC1_IN_R_SEL_DMIC0_DAT	(0)
#define DMIC1_IN_R_SEL_DMIC0_DAT_R	(1 << 4)
#define	DMIC1_IN_R_SEL_DMIC1_DAT	(2 << 4)

#define DMIC2_IN_L_SEL_MASK	(0x3 << 6)
#define DMIC2_IN_L_SEL_DMIC0_DAT	(0)
#define DMIC2_IN_L_SEL_DMIC0_DAT_R	(1 << 6)
#define	DMIC2_IN_L_SEL_DMIC1_DAT	(2 << 6)
#define DMIC2_IN_R_SEL_MASK	(0x3 << 8)
#define DMIC2_IN_R_SEL_DMIC0_DAT	(0)
#define DMIC2_IN_R_SEL_DMIC0_DAT_R	(1 << 8)
#define	DMIC2_IN_R_SEL_DMIC1_DAT	(2 << 8)

#define I2SOUT6_MUTE_SEL_MASK	(0x1 << 10)
#define I2SOUT6_MUTE_SEL_AMUTE_2CH	(0)
#define I2SOUT6_MUTE_SEL_AMUTE_12CH	(0x1 << 10)
#define PCM_TX2RX_POS            (11)
#define PCM_TX2RX_MASK           (0x1 << PCM_TX2RX_POS)
#define I2SIN3_DATA_FROM_SINE_GEN_SEL_POS	(20)
#define I2SIN3_DATA_FROM_SINE_GEN_SEL_MASK	(1 << 20)
#define I2SIN4_DATA_FROM_SINE_GEN_SEL_POS	(21)
#define I2SIN4_DATA_FROM_SINE_GEN_SEL_MASK	(1 << 21)
#define DMIC1_DATA_FROM_SINE_GEN_SEL_POS	(22)
#define DMIC1_DATA_FROM_SINE_GEN_SEL_MASK	(1 << 22)
#define DMIC2_DATA_FROM_SINE_GEN_SEL_POS	(23)
#define DMIC2_DATA_FROM_SINE_GEN_SEL_MASK	(1 << 23)
#define I2SOUT1_SEL_DMIC_SINE_GEN_POS	(24)
#define I2SOUT1_SEL_DMIC_SINE_GEN_MASK	(1 << 24)
#define I2SOUT4_SEL_DMIC_SINE_GEN_POS	(25)
#define I2SOUT4_SEL_DMIC_SINE_GEN_MASK	(1 << 25)
#define I2SOUT5_SEL_DMIC_SINE_GEN_POS	(26)
#define I2SOUT5_SEL_DMIC_SINE_GEN_MASK	(1 << 26)


#define AMUTE_12CH_CNT_CFG	(0x1744)
#define AMUTE_2CH_CNT_CFG	(0x1748)
#define AFE_LOOPBACK_CFG0	(0x1730)
#define I2SIN3_DAT_SEL_POS	(24)
#define I2SIN3_DAT_SEL_MASK	(0xF << 24)
#define I2SIN3_BCK_WS_SEL_POS	(20)
#define I2SIN3_BCK_WS_SEL_MASK	(0xF << 20)

#define AFE_LOOPBACK_CFG1        (0x1734)
#define I2SIN1_DAT_SEL_POS       (4)
#define I2SIN1_DAT_SEL_MASK      (0xF << I2SIN1_DAT_SEL_POS)
#define I2SIN1_BCK_WS_SEL_POS       (0)
#define I2SIN1_BCK_WS_SEL_MASK      (0xF << I2SIN1_BCK_WS_SEL_POS)
#define I2SIN2_DAT_SEL_POS       (12)
#define I2SIN2_DAT_SEL_MASK      (0xF << I2SIN2_DAT_SEL_POS)
#define I2SIN2_BCK_WS_SEL_POS       (8)
#define I2SIN2_BCK_WS_SEL_MASK      (0xF << I2SIN2_BCK_WS_SEL_POS)
#define I2SIN4_DAT_SEL_POS       (28)
#define I2SIN4_DAT_SEL_MASK      (0xF << I2SIN4_DAT_SEL_POS)
#define I2SIN4_BCK_WS_SEL_POS       (24)
#define I2SIN4_BCK_WS_SEL_MASK      (0xF << I2SIN4_BCK_WS_SEL_POS)


#define UL2_ASRC_WS_SEL_POS      (20)
#define UL2_ASRC_WS_SEL_MASK     (0xf << UL2_ASRC_WS_SEL_POS)
#define UL1_ASRC_WS_SEL_POS      (16)
#define UL1_ASRC_WS_SEL_MASK     (0xf << UL1_ASRC_WS_SEL_POS)

#define AFE_I2SOUT_CH_CFG   (0x1738)
#define S_I2S1_OUT_BCK_WS_SEL_POS   (0)
#define S_I2S1_OUT_BCK_WS_SEL_MASK   (0x7 << S_I2S1_OUT_BCK_WS_SEL_POS)
#define S_I2S2_OUT_BCK_WS_SEL_POS   (4)
#define S_I2S2_OUT_BCK_WS_SEL_MASK   (0x7 << S_I2S2_OUT_BCK_WS_SEL_POS)
#define S_I2S1_OUT_DAT_SEL_POS   (8)
#define S_I2S1_OUT_DAT_SEL_MASK   (0xf << S_I2S1_OUT_DAT_SEL_POS)
#define S_I2S2_OUT_DAT_SEL_POS   (12)
#define S_I2S2_OUT_DAT_SEL_MASK   (0xf << S_I2S2_OUT_DAT_SEL_POS)
#define S_I2S3_OUT_DAT_SEL_POS   (16)
#define S_I2S3_OUT_DAT_SEL_MASK   (0xf << S_I2S3_OUT_DAT_SEL_POS)
#define S_I2S4_OUT_DAT_SEL_POS   (20)
#define S_I2S4_OUT_DAT_SEL_MASK   (0xf << S_I2S4_OUT_DAT_SEL_POS)
#define S_I2S5_OUT_DAT_SEL_POS   (24)
#define S_I2S5_OUT_DAT_SEL_MASK   (0xf << S_I2S5_OUT_DAT_SEL_POS)
#define S_I2S6_OUT_DAT_SEL_POS   (28)
#define S_I2S6_OUT_DAT_SEL_MASK   (0xf << S_I2S6_OUT_DAT_SEL_POS)

#define PCM_INTF_CON2       (0x0640)
#define PCMIN_USE_I2SIN1_ASRC_POS (24)
#define PCMIN_USE_I2SIN1_ASRC_MASK (1<<PCMIN_USE_I2SIN1_ASRC_POS)
#define PCMOUT_USE_I2SIN2_ASRC_POS (23)
#define PCMOUT_USE_I2SIN2_ASRC_MASK (1<<PCMOUT_USE_I2SIN2_ASRC_POS)
#define PCM1_TX_FIFO_OV_POS      (31)
#define PCM1_TX_FIFO_OV_MASK     (1<<PCM1_TX_FIFO_OV_POS)
#define PCM1_RX_FIFO_OV_POS      (30)
#define PCM1_RX_FIFO_OV_MASK     (1<<PCM1_RX_FIFO_OV_POS)
#define PCM2_TX_FIFO_OV_POS      (29)
#define PCM2_TX_FIFO_OV_MASK     (1<<PCM2_TX_FIFO_OV_POS)
#define PCM2_RX_FIFO_OV_POS      (28)
#define PCM2_RX_FIFO_OV_MASK     (1<<PCM2_RX_FIFO_OV_POS)
#define PCM1_SYNC_GLITCH_POS     (27)
#define PCM1_SYNC_GLITCH_MASK    (1<<PCM1_SYNC_GLITCH_POS)
#define PCM2_SYNC_GLITCH_POS     (26)
#define PCM2_SYNC_GLITCH_MASK    (1<<PCM2_SYNC_GLITCH_POS)
#define TX_FIX_VALUE_POS         (0)
#define TX_FIX_VALUE_MASK        (0xFF<<TX_FIX_VALUE_POS)

/* Modem PCM 1 */
/* #define PCM_EN_POS          0 */
#define PCM_EN_LEN          1

/* #define PCM_FMT_POS         1 */
#define PCM_FMT_LEN         2

/* #define PCM_MODE_POS        3 */
#define PCM_MODE_LEN        1

/* #define PCM_WLEN_POS        4 */
#define PCM_WLEN_LEN        1

/* #define PCM_SLAVE_POS       5 */
#define PCM_SLAVE_LEN       1

#define PCM_BYP_ASRC_POS    6
#define PCM_BYP_ASRC_LEN    1

#define PCM_BTMODE_POS      7
#define PCM_BTMODE_LEN      1

#define PCM_SYNC_TYPE_POS   8
#define PCM_SYNC_TYPE_LEN   1

#define PCM_SYNC_LEN_POS    9
#define PCM_SYNC_LEN_LEN    5

#define PCM_EXT_MODEM_POS   17
#define PCM_EXT_MODEM_LEN   1

#define PCM_VBT16K_MODE_POS 18
#define PCM_VBT16K_MODE_LEN 1


/* #define PCM_BCKINV_POS      6 */
/* #define PCM_BCKINV_LEN      1 */
/* #define PCM_SYNCINV_POS     7 */
/* #define PCM_SYNCINV_LEN     1 */

#define PCM_SERLOOPBK_POS   28
#define PCM_SERLOOPBK_LEN   1

#define PCM_PARLOOPBK_POS   29
#define PCM_PARLOOPBK_LEN   1

#define PCM_BUFLOOPBK_POS   30
#define PCM_BUFLOOPBK_LEN   1

#define PCM_FIX_VAL_SEL_POS 31
#define PCM_FIX_VAL_SEL_LEN 1

/* BT PCM */
#define DAIBT_EN_POS   0
#define DAIBT_EN_LEN   1
#define BTPCM_EN_POS   1
#define BTPCM_EN_LEN   1
#define BTPCM_SYNC_POS   2
#define BTPCM_SYNC_LEN   1
#define DAIBT_DATARDY_POS   3
#define DAIBT_DATARDY_LEN   1
#define BTPCM_LENGTH_POS   4
#define BTPCM_LENGTH_LEN   3
/* #define DAIBT_MODE_POS   9 */
#define DAIBT_MODE_LEN   1



/* AFE_IRQ_CON */
#define IRQ1_ON         (1 << 0)
#define IRQ2_ON         (1 << 1)
#define IRQ_MULTI_ON    (1 << 2)
#define IRQ_MULTI_OFF   (0 << 2)
#define IRQ4_ON         (1 << 3)

/* this bit controls the enable for IRQ5_MCU,which is speciallized for HDMI 8ch I2S */
#define IRQ5_ON         (1 << 12)

/* this bit controls the enable for IRQ5_MCU,which is speciallized for SPDIF */
#define IRQ6_ON         (1 << 13)

#define IRQ1_FS_MASK    (0xF << 4)
#define IRQ1_FS_8K    (0x0)
#define IRQ1_FS_12K   (0x1 << 4)
#define IRQ1_FS_16K   (0x2 << 4)
#define IRQ1_FS_24K   (0x3 << 4)
#define IRQ1_FS_32K   (0x4 << 4)
#define IRQ1_FS_48K   (0x5 << 4)
#define IRQ1_FS_96K   (0x6 << 4)
#define IRQ1_FS_192K  (0x7 << 4)
#define IRQ1_FS_11K   (0x9 << 4)
#define IRQ1_FS_22K   (0xb << 4)
#define IRQ1_FS_44K   (0xd << 4)
#define IRQ1_FS_88K   (0xe << 4)

#define IRQ2_FS_MASK    (0xF << 8)
#define IRQ2_FS_8K    (0x0)
#define IRQ2_FS_12K   (0x1 << 8)
#define IRQ2_FS_16K   (0x2 << 8)
#define IRQ2_FS_24K   (0x3 << 8)
#define IRQ2_FS_32K   (0x4 << 8)
#define IRQ2_FS_48K   (0x5 << 8)
#define IRQ2_FS_96K   (0x6 << 8)
#define IRQ2_FS_192K  (0x7 << 8)
#define IRQ2_FS_11K   (0x9 << 8)
#define IRQ2_FS_22K   (0xb << 8)
#define IRQ2_FS_44K   (0xd << 8)
#define IRQ2_FS_88K   (0xe << 8)

#define IRQ11_FS_MASK       (0xF << 28)
#define IRQ11_ON            (1 << 18)

#define IRQ12_ON            (1 << 19)

#define IRQ_ACC1_FS_MASK    (0xF << 20)
#define IRQ_ACC1_ON         (1 << 16)

#define IRQ_ACC2_FS_MASK    (0xF << 24)
#define IRQ_ACC2_ON         (1 << 17)

#define IRQ5_ON         (1 << 12)
#define IRQ6_ON         (1 << 13)
#define IRQ_SETTING_BIT (0x3007)

/* AFE_IRQ_CLR */
#define IRQ1_IRQ_CLR     (1 << 0)
#define IRQ2_IRQ_CLR     (1 << 1)
#define MULTIIN_IRQ_CLR  (1 << 2)
#define SPDIF2_IRQ_CLR   (1 << 3)
#define HDMI_IRQ_CLR     (1 << 4)
#define SPDIF_IRQ_CLR    (1 << 5)
#define SPDIFIN_IRQ_CLR  (1 << 6)
#define IRQ11_IRQ_CLR     (1 << 10)
#define IRQ_CLR_ALL (IRQ1_IRQ_CLR|IRQ2_IRQ_CLR|MULTIIN_IRQ_CLR|SPDIF2_IRQ_CLR|\
	HDMI_IRQ_CLR|SPDIF_IRQ_CLR|SPDIFIN_IRQ_CLR|IRQ11_IRQ_CLR)


#define IRQ1_MISS_CLR (1<<8)
#define IRQ2_MISS_CLR (1<<9)
#define IRQ3_MISS_CLR (1<<10)
#define IRQ4_MISS_CLR (1<<11)
#define IRQ5_MISS_CLR (1<<12)
#define IRQ6_MISS_CLR (1<<13)

/* AFE_IRQ_MON2 */
#define IRQ1_MISS_BIT       (1<<8)
#define IRQ2_MISS_BIT       (1<<9)
#define IRQ3_MISS_BIT       (1<<10)
#define IRQ4_MISS_BIT       (1<<11)
#define IRQ5_MISS_BIT       (1<<12)
#define IRQ6_MISS_BIT       (1<<13)
#define IRQ_MISS_STATUS_BIT (0x3F00)

/* AUDIO_TOP_CON3 */
#define HDMI_OUT_SPEAKER_BIT    4
#define SPEAKER_OUT_HDMI        5
#define HDMI_2CH_SEL_POS        6
#define HDMI_2CH_SEL_LEN        2


#define AFE_HDMI_OUT_CON0 (0x370)
#define AFE_HDMI_OUT_BASE (0x374)
#define AFE_HDMI_OUT_CUR  (0x378)
#define AFE_HDMI_OUT_END  (0x37C)

/* AFE_HDMI_OUT_CON0 */
#define HDMI_OUT_ON_MASK        1
#define HDMI_OUT_ON         1
#define HDMI_OUT_OFF        0
#define HDMI_OUT_BIT_WIDTH_MASK      (1 << 1)
#define HDMI_OUT_BIT_WIDTH_16     0
#define HDMI_OUT_BIT_WIDTH_32     (1 << 1)
#define HDMI_OUT_DSD_WDITH_MASK      (0x3 << 2)
#define HDMI_OUT_DSD_32BIT       (0x0 << 2)
#define HDMI_OUT_DSD_16BIT       (0x1 << 2)
#define HDMI_OUT_DSD_8BIT        (0x2 << 2)
#define HDMI_OUT_CH_NUM_MASK     (0xF << 4)
#define HDMI_OUT_CH_NUM_POS      4

/* AFE_HDMI_CONN0 */
#define SPDIF_LRCK_SRC_SEL_MASK  (1 << 30)
#define SPDIF_LRCK_SRC_SEL_SPDIF 0
#define SPDIF_LRCK_SRC_SEL_HDMI  (1 << 30)
#define SPDIF2_LRCK_SRC_SEL_MASK  (1 << 31)
#define SPDIF2_LRCK_SRC_SEL_SPDIF 0
#define SPDIF2_LRCK_SRC_SEL_HDMI  (1 << 31)

/* AFE_8CH_I2S_OUT_CON */
#define HDMI_8CH_I2S_CON_EN_MASK 1
#define HDMI_8CH_I2S_CON_EN  1
#define HDMI_8CH_I2S_CON_DIS 0

#define HDMI_8CH_I2S_CON_BCK_INV_MASK   (1 << 1)
#define HDMI_8CH_I2S_CON_BCK_INV              (1 << 1)
#define HDMI_8CH_I2S_CON_BCK_NO_INV  0

#define HDMI_8CH_I2S_CON_LRCK_INV_MASK   (1 << 2)
#define HDMI_8CH_I2S_CON_LRCK_INV    (1 << 2)
#define HDMI_8CH_I2S_CON_LRCK_NO_INV 0

#define HDMI_8CH_I2S_CON_I2S_DELAY_MASK    (1 << 3)
#define HDMI_8CH_I2S_CON_I2S_DELAY     (1 << 3)
#define HDMI_8CH_I2S_CON_I2S_NO_DELAY  0

#define HDMI_8CH_I2S_CON_I2S_WLEN_MASK     (3 << 4)
#define HDMI_8CH_I2S_CON_I2S_8BIT      0
#define HDMI_8CH_I2S_CON_I2S_16BIT     (1 << 4)
#define HDMI_8CH_I2S_CON_I2S_24BIT     (2 << 4)
#define HDMI_8CH_I2S_CON_I2S_32BIT     (3 << 4)

#define HDMI_8CH_I2S_CON_DSD_MODE_MASK     (0x1 << 6)
#define HDMI_8CH_I2S_CON_DSD           (0x1 << 6)
#define HDMI_8CH_I2S_CON_NON_DSD       (0x0 << 6)
/* AFE_SIDETONE_DEBUG */
#define STF_SRC_SEL_POS     16
#define STF_SRC_SEL_LEN     2
#define STF_I5I6_SEL_POS    19
#define STF_I5I6_SEL_LEN    1

/* AFE_SIDETONE_CON0 */
#define STF_COEFF_VAL_POS       0
#define STF_COEFF_VAL_LEN       16
#define STF_COEFF_ADDRESS_POS   16
#define STF_COEFF_ADDRESS_LEN   5
#define STF_CH_SEL_POS          23
#define STF_CH_SEL_LEN          1
#define STF_COEFF_W_ENABLE_POS  24
#define STF_COEFF_W_ENABLE_LEN  1
#define STF_W_ENABLE_POS        25
#define STF_W_ENABLE_LEN        1
#define STF_COEFF_BIT       0x0000FFFF

/* AFE_SIDETONE_CON1 */
#define STF_TAP_NUM_POS 0
#define STF_TAP_NUM_LEN	5
#define STF_ON_POS      8
#define STF_ON_LEN      1
#define STF_BYPASS_POS  31
#define STF_BYPASS_LEN  1

/* AFE_SGEN_CON0 */
#define SINE_TONE_FREQ_DIV_CH1  0
#define SINE_TONE_AMP_DIV_CH1   5
#define SINE_TONE_MODE_CH1      8
#define SINE_TONE_FREQ_DIV_CH2  12
#define SINE_TONE_AMP_DIV_CH2   17
#define SINE_TONE_MODE_CH2      20
#define SINE_TONE_MUTE_CH1      24
#define SINE_TONE_MUTE_CH2      25
#define SINE_TONE_ENABLE        26
#define SINE_TONE_LOOPBACK_MOD  28


/* AFE_SPDIF_OUT_CON0 */
#define SPDIF_OUT_CLOCK_ON_OFF_MASK  1
#define SPDIF_OUT_CLOCK_ON       1
#define SPDIF_OUT_CLOCK_OFF      0
#define SPDIF_OUT_MEMIF_ON_OFF_MASK  (1 << 1)
#define SPDIF_OUT_MEMIF_ON       (1 << 1)
#define SPDIF_OUT_MEMIF_OFF      0


/* AFE_IEC_CFG */
#define IEC_RAW_SEL_MASK       (0x1)
#define IEC_RAW_SEL_COOK   (0x0)
#define IEC_RAW_SEL_RAW    (0x1)
#define IEC_PCM_SEL_MASK       (0x2)
#define IEC_PCM_SEL_PCM    (0x0)
#define IEC_PCM_SEL_ENC    (0x2)
#define IEC_MUTE_DATA_MASK     (0x1 << 3)
#define IEC_MUTE_DATA_NORMAL (0x0)
#define IEC_MUTE_DATA_MUTE   (0x1 << 3)
#define IEC_VALID_DATA_MASK    (0x1 << 4)
#define IEC_INVALID_DATA   (0x0)
#define IEC_VALID_DATA      (0x1 << 4)
#define IEC_RAW_24BIT_MODE_MASK          (0x1 << 5)
#define IEC_RAW_24BIT_MODE_ON       (0x1 << 5)
#define IEC_RAW_24BIT_MODE_OFF      (0x0 << 5)
#define IEC_RAW_24BIT_SWITCH_MODE_MASK     (0x1 << 6)
#define IEC_RAW_24BIT_SWITCH_MODE_ON   (0x1 << 6)
#define IEC_RAW_24BIT_SWITCH_MODE_OFF  (0x0 << 6)
#define IEC_MAT_LEN_MASK       (0x1 << 7)
#define IEC_MAT_CRC_EN_MASK    (0x1 << 8)
#define IEC_DST_BIT_REV_MASK   (0x1 << 9)
#define IEC_EN_MASK            (0x1 << 16)
#define IEC_ENABLE             (0x1 << 16)
#define IEC_DISABLE            (0x0 << 16)
#define IEC_RISC_SWAP_IEC_BYTE_MASK (0x1 << 17)
#define IEC_DOWN_SAMPLE_MASK   (0x3 << 18)
#define IEC_DOWN_SAMPLE_0  (0x0 << 18)
#define IEC_DOWN_SAMPLE_2  (0x1 << 18)
#define IEC_DOWN_SAMPLE_4  (0x3 << 18)
#define IEC_FORCE_UPDATE_MASK  (0x1 << 20)
#define IEC_FORCE_UPDATE       (0x1 << 20)
#define IEC_MUTE_MASK          (0x1 << 21)
#define IEC_MUTE				(0x1 << 21)
#define IEC_UNMUTE				(0)
#define IEC_REG_LOCK_EN_MASK   (0x1 << 22)
#define IEC_SW_RST_MASK        (0x1 << 23)
#define IEC_SW_NORST           (0x1 << 23)
#define IEC_SW_RST             (0x0 << 23)
#define IEC_FORCE_UPDATE_SIZE_MASK (0xFF << 24)


/* AFE_IEC_NSNUM */
#define IEC_NEXT_SAM_NUM_MASK    0x3FFF
#define IEC_INT_SAM_NUM_MASK     (0x3FFF << 16)

/* AFE_IEC_BURST_INFO */
#define IEC_BURST_INFO_MASK    0x7FFFF

#define IEC_BURST_NOT_READY_MASK (1 << 16)
#define IEC_BURST_NOT_READY      (1 << 16)
#define IEC_BURST_READY          (0 << 16)

/* AFE_IEC_BURST_LEN */
#define IEC_BURST_LEN_MASK       0x7FFFF

/* AFE_SPDIF2_OUT_CON0 */
#define SPDIF2_OUT_CLOCK_ON_OFF_MASK  1
#define SPDIF2_OUT_CLOCK_ON       1
#define SPDIF2_OUT_CLOCK_OFF      0
#define SPDIF2_OUT_MEMIF_ON_OFF_MASK  (1 << 1)
#define SPDIF2_OUT_MEMIF_ON       (1 << 1)
#define SPDIF2_OUT_MEMIF_OFF      0
/* AFE_IEC2_NSNUM */
#define IEC2_NEXT_SAM_NUM_MASK    0x3FFF
#define IEC2_INT_SAM_NUM_MASK     (0x3FFF << 16)

/* AFE_IEC2_BURST_INFO */
#define IEC2_BURST_INFO_MASK      0xFFFF

#define IEC2_BURST_NOT_READY_MASK (1 << 16)
#define IEC2_BURST_NOT_READY      (1 << 16)
#define IEC2_BURST_READY          (0 << 16)

/* AFE_IEC2_BURST_LEN */
#define IEC2_BURST_LEN_MASK    0x7FFFF

/* AFE_IEC2_CFG */
#define IEC2_RAW_SEL_MASK       (0x1)
#define IEC2_RAW_SEL_COOK   (0x0)
#define IEC2_RAW_SEL_RAW    (0x1)
#define IEC2_PCM_SEL_MASK       (0x2)
#define IEC2_PCM_SEL_PCM    (0x0)
#define IEC2_PCM_SEL_ENC    (0x2)
#define IEC2_MUTE_DATA_MASK     (0x1 << 3)
#define IEC2_MUTE_DATA_NORMAL (0x0)
#define IEC2_MUTE_DATA_MUTE   (0x1 << 3)
#define IEC2_VALID_DATA_MASK    (0x1 << 4)
#define IEC2_INVALID_DATA   (0x0 << 4)
#define IEC2_VALID_DATA     (0x1 << 4)
#define IEC2_RAW_24BIT_MODE_MASK            (0x1 << 5)
#define IEC2_RAW_24BIT_MODE_ON          (0x1 << 5)
#define IEC2_RAW_24BIT_MODE_OFF         (0x0 << 5)
#define IEC2_RAW_24BIT_SWITCH_MODE_MASK     (0x1 << 6)
#define IEC2_RAW_24BIT_SWITCH_MODE_ON   (0x1 << 6)
#define IEC2_RAW_24BIT_SWITCH_MODE_OFF  (0x0 << 6)
#define IEC2_MAT_LEN_MASK       (0x1 << 7)
#define IEC2_MAT_CRC_EN_MASK    (0x1 << 8)
#define IEC2_DST_BIT_REV_MASK   (0x1 << 9)
#define IEC2_EN_MASK            (0x1 << 16)
#define IEC2_ENABLE             (0x1 << 16)
#define IEC2_DISABLE            (0x0 << 16)
#define IEC2_RISC_SWAP_IEC_BYTE_MASK (0x1 << 17)
#define IEC2_DOWN_SAMPLE_MASK   (0x3 << 18)
#define IEC2_DOWN_SAMPLE_0  (0x0 << 18)
#define IEC2_DOWN_SAMPLE_2  (0x1 << 18)
#define IEC2_DOWN_SAMPLE_4  (0x3 << 18)
#define IEC2_FORCE_UPDATE_MASK  (0x1 << 20)
#define IEC2_FORCE_UPDATE       (0x1 << 20)
#define IEC2_MUTE_MASK          (0x1 << 21)
#define IEC2_REG_LOCK_EN_MASK   (0x1 << 22)
#define IEC2_SW_RST_MASK        (0x1 << 23)
#define IEC2_SW_NORST           (0x1 << 23)
#define IEC2_SW_RST             (0x0 << 23)
#define IEC2_FORCE_UPDATE_SIZE_MASK (0xFF << 24)

/* FPGA_CFG2 */
#define AFE_ON_AP_MODEL_POS	31
#define AFE_ON_AP_MODEL_LEN	1
#define DL_2_SRC_ON_AP_MODEL_POS 30
#define DL_2_SRC_ON_AP_MODEL_LEN 1
#define UL_SRC_ON_AP_MODEL_POS 29
#define UL_SRC_ON_AP_MODEL_LEN 1
#define UP8X_RXIF_CLKINV_ADC_POS	27
#define UP8X_RXIF_CLKINV_ADC_LEN	1
#define UP8X_RXIF_FIFO_RSP_ADC_POS	24
#define UP8X_RXIF_FIFO_RSP_ADC_LEN	3
#define UP8X_SYNC_WORD_POS	0
#define UP8X_SYNC_WORD_LEN	16

/* FPGA_CFG3 */
#define UP8X_RXIF_DL_2_INPUT_MODE_POS	28
#define UP8X_RXIF_DL_2_INPUT_MODE_LEN	4
#define UP8X_RXIF_INVALID_SYNC_CHECK_ROUND_POS	24
#define UP8X_RXIF_INVALID_SYNC_CHECK_ROUND_LEN	4
#define UP8X_RXIF_SYNC_CHECK_ROUND_POS	20
#define UP8X_RXIF_SYNC_CHECK_ROUND_LEN	4

#define UP8X_RXIF_SYNC_SEARCH_TABLE_POS	12
#define UP8X_RXIF_SYNC_SEARCH_TABLE_LEN	5

#define UP8X_RXIF_ADC_VOICE_MODE_POS	10
#define UP8X_RXIF_ADC_VOICE_MODE_LEN	2

#define UP8X_RXIF_SYNC_CNT_TABLE_POS	0
#define UP8X_RXIF_SYNC_CNT_TABLE_LEN	9


/* FPGA_CFG0 */
#define MCLK_MUX2_POS    26
#define MCLK_MUX2_LEN    1
#define MCLK_MUX1_POS    25
#define MCLK_MUX1_LEN    1
#define MCLK_MUX0_POS    24
#define MCLK_MUX0_LEN    1
#define SOFT_RST_POS   16
#define SOFT_RST_LEN   8
#define HOP26M_SEL_POS   12
#define HOP26M_SEL_LEN   2

/* FPGA_CFG1 */
#define CODEC_SEL_POS   0
#define DAC_SEL_POS     4
#define I2SIN_DIR_SEL     8

/* AFE_ASRC_CON13 */
#define CHSET2_OS_MONO_POS	16
#define CHSET2_OS_MONO_LEN	1

/* AFE_ASRC_CON14 */
#define ASM_FREQ_4_POS	0
#define ASM_FREQ_4_LEN 24

/* AFE_ASRC_CON15 */
#define ASM_FREQ_5_POS	0
#define ASM_FREQ_5_LEN 24

/* AFE_ASRC_CON16 */
#define CALI2_EN_POS	0
#define CALI2_EN_LEN	1
#define CALI2_USE_FREQ_OUT_POS	1
#define CALI2_USE_FREQ_OUT_LEN	1
#define FREQ_CALI2_AUTO_RESTART_POS	2
#define FREQ_CALI2_AUTO_RESTART_LEN	1
#define AUTO_TUNE_FREQ5_POS	3
#define AUTO_TUNE_FREQ5_LEN	1
#define FREQ_CALI2_MAX_GWIDTH_POS	4
#define FREQ_CALI2_MAX_GWIDTH_LEN	3
#define	FREQ_CALI2_BP_DG_POS	7
#define FREQ_CALI2_BP_DG_LEN	1
#define	FREQ_CALI2_SEL_POS	8
#define	FREQ_CALI2_SEL_LEN	2
#define	COMP_FREQ_RES_EN2_POS	11
#define	COMP_FREQ_RES_EN2_LEN	1
#define	AUTO_TUNE_FREQ4_POS		12
#define	AUTO_TUNE_FREQ4_LEN		1
#define	FREQ_CALC2_RUNNING_POS	13
#define	FREQ_CALC2_RUNNING_LEN	1
#define	FREQ_CALI2_CYCLE_POS	16
#define	FREQ_CALI2_CYCLE_LEN	16

/* AFE_ASRC_CON17 */
#define	FREQ_CALC2_DENOMINATOR_POS	0
#define	FREQ_CALC2_DENOMINATOR_LEN	24

/* AFE_ASRC_CON18 */
#define	PRD_CALI2_RESULT_RECORD_POS	0
#define	PRD_CALI2_RESULT_RECORD_LEN	24

/* AFE_ASRC_CON19 */
#define FREQ_CALI2_RESULT_POS	0
#define	FREQ_CALI2_RESULT_LEN	24


/* AFE_ADDA_DL_SRC2_CON0 */
#define	ADDA_dl_2_src_on_tmp_ctl_pre_POS	0
#define	ADDA_dl_2_src_on_tmp_ctl_pre_LEN	1

#define	ADDA_dl_2_gain_on_ctl_pre_POS	1
#define	ADDA_dl_2_gain_on_ctl_pre_LEN	1

#define	ADDA_dl_2_iir_on_ctl_pre_POS	2
#define	ADDA_dl_2_iir_on_ctl_pre_LEN	1

#define	ADDA_d2_2_mute_ch2_on_ctl_pre_POS	3
#define	ADDA_d2_2_mute_ch2_on_ctl_pre_LEN	1

#define	ADDA_d2_2_mute_ch1_on_ctl_pre_POS	4
#define	ADDA_d2_2_mute_ch1_on_ctl_pre_LEN	1

#define	ADDA_dl_2_voice_mode_ctl_pre_POS	5
#define	ADDA_dl_2_voice_mode_ctl_pre_LEN	1

#define	ADDA_dl_2_iirmode_ctl_pre_POS	6
#define	ADDA_dl_2_iirmode_ctl_pre_LEN	3

#define	ADDA_dl2_arampsp_ctl_pre_POS	9
#define	ADDA_dl2_arampsp_ctl_pre_LEN	2

#define	ADDA_dl_2_mute_ch2_off_ctl_pre_POS	11
#define	ADDA_dl_2_mute_ch2_off_ctl_pre_LEN	1

#define	ADDA_dl_2_mute_ch1_off_ctl_pre_POS	12
#define	ADDA_dl_2_mute_ch1_off_ctl_pre_LEN	1

#define	ADDA_dl_2_side_tone_on_ctl_pre_POS	13
#define	ADDA_dl_2_side_tone_on_ctl_pre_LEN	1

#define	ADDA_c_data_en_sel_ctl_pre_POS	14
#define	ADDA_c_data_en_sel_ctl_pre_LEN	1

#define	ADDA_dl_2_output_sel_ctl_POS	24
#define	ADDA_dl_2_output_sel_ctl_LEN	2

#define	ADDA_dl_2_ch2_saturation_en_ctl_POS	26
#define	ADDA_dl_2_ch2_saturation_en_ctl_LEN	1

#define	ADDA_dl_2_ch1_saturation_en_ctl_POS	27
#define	ADDA_dl_2_ch1_saturation_en_ctl_LEN	1

#define	ADDA_dl_2_input_mode_ctl_POS	28
#define	ADDA_dl_2_input_mode_ctl_LEN	4


/* AFE_ADDA_DL_SRC2_CON1 */
#define	ADDA_dl_2_gain_mode_ctl_POS	0
#define	ADDA_dl_2_gain_mode_ctl_LEN	1

#define	ADDA_dl_2_gain_ctl_pre_POS	16
#define	ADDA_dl_2_gain_ctl_pre_LEN	16

/* AFE_ADDA_UL_SRC_CON0 */
#define	ADDA_ul_src_on_tmp_ctl_pre_POS	0
#define	ADDA_ul_src_on_tmp_ctl_pre_LEN	1

#define	ADDA_ul_sdm_3_level_ctl_POS	1
#define	ADDA_ul_sdm_3_level_ctl_LEN	1

#define	ADDA_ul_loop_back_mode_ctl_POS	2
#define	ADDA_ul_loop_back_mode_ctl_LEN	1

#define	ADDA_agc_260k_sel_ch1_ctl_POS	3
#define	ADDA_agc_260k_sel_ch1_ctl_LEN	1

#define	ADDA_agc_260k_sel_ch2_ctl_POS	4
#define	ADDA_agc_260k_sel_ch2_ctl_LEN	1

#define	ADDA_digmic_3p25m_1p625m_sel_ctl_POS	5
#define	ADDA_digmic_3p25m_1p625m_sel_ctl_LEN	1

#define	ADDA_ul_iirmode_ctl_POS	7
#define	ADDA_ul_iirmode_ctl_LEN	3

#define	ADDA_ul_iir_on_tmp_ctl_POS	10
#define	ADDA_ul_iir_on_tmp_ctl_LEN	1

#define	ADDA_ul_voice_mode_ch1_ctl_POS	17
#define	ADDA_ul_voice_mode_ch1_ctl_LEN	2

#define	ADDA_ul_voice_mode_ch2_ctl_POS	19
#define	ADDA_ul_voice_mode_ch2_ctl_LEN	2

#define	ADDA_ul_mode_3p25m_ch1_ctl_POS	21
#define	ADDA_ul_mode_3p25m_ch1_ctl_LEN	1

#define	ADDA_ul_mode_3p25m_ch2_ctl_POS	22
#define	ADDA_ul_mode_3p25m_ch2_ctl_LEN	1

#define	ADDA_c_two_digital_mic_ctl_POS	23
#define	ADDA_c_two_digital_mic_ctl_LEN	1

#define	ADDA_c_digmic_phase_sel_ch2_ctl_POS	24
#define	ADDA_c_digmic_phase_sel_ch2_ctl_LEN	3

#define	ADDA_c_digmic_phase_sel_ch1_ctl_POS	27
#define	ADDA_c_digmic_phase_sel_ch1_ctl_LEN	2

#define	ADDA_c_baseband_sin_gen_ctl_POS	30
#define	ADDA_c_baseband_sin_gen_ctl_LEN	1

#define	ADDA_c_comb_out_sin_gen_ctl_POS	31
#define	ADDA_c_comb_out_sin_gen_ctl_LEN	1

/* AFE_ADDA_UL_SRC_CON1 */
#define	ADDA_c_sine_mode_ch1_ctl_POS	0
#define	ADDA_c_sine_mode_ch1_ctl_LEN	4

#define	ADDA_c_freq_div_ch1_ctl_POS	4
#define	ADDA_c_freq_div_ch1_ctl_LEN	4

#define	ADDA_c_amp_div_ch1_ctl_POS	9
#define	ADDA_c_amp_div_ch1_ctl_LEN	3

#define	ADDA_c_sine_mode_ch2_ctl_POS	12
#define	ADDA_c_sine_mode_ch2_ctl_LEN	4

#define	ADDA_c_freq_div_ch2_ctl_POS	16
#define	ADDA_c_freq_div_ch2_ctl_LEN	5

#define	ADDA_c_amp_div_ch2_ctl_POS	21
#define	ADDA_c_amp_div_ch2_ctl_LEN	3

#define	ADDA_asdm_src_sel_ctl_POS	25
#define	ADDA_asdm_src_sel_ctl_LEN	1

#define	ADDA_c_mute_sw_ctl_POS	26
#define	ADDA_c_mute_sw_ctl_LEN	1

#define	ADDA_c_dac_en_ctl_POS	27
#define	ADDA_c_dac_en_ctl_LEN	1

#define	ADDA_adithval_ctl_POS	28
#define	ADDA_adithval_ctl_LEN	2

#define	ADDA_adithon_ctl_POS	30
#define	ADDA_adithon_ctl_LEN	1

#define	ADDA_c_sdm_reset_ctl_POS	31
#define	ADDA_c_sdm_reset_ctl_LEN	1

/* AFE_ADDA_TOP_CON0 */
#define	ADDA_c_ext_adc_ctl_POS	0
#define	ADDA_c_ext_adc_ctl_LEN	1

#define	ADDA_c_loop_back_mode_ctl_POS	12
#define	ADDA_c_loop_back_mode_ctl_LEN	4


/* AFE_ADDA_UL_DL_CON0 */
#define	ADDA_adda_afe_on_POS	0
#define	ADDA_adda_afe_on_LEN	1

#define	ADDA_afe_ul_dl_con0_reserved_POS	1
#define	ADDA_afe_ul_dl_con0_reserved_LEN	14

/* AFE_ADDA_SRC_DEBUG */
/* TODO */
/* AFE_ADDA_SRC_DEBUG_MON0 */
/* TODO */
/* AFE_ADDA_SRC_DEBUG_MON1 */
/* TODO */
/* AFE_ADDA_NEWIF_CFG0 */
#define	ADDA_up8x_txif_sat_en_POS	0
#define	ADDA_up8x_txif_sat_en_LEN	1

#define	ADDA_up8x_txif_clk_inv_POS	1
#define	ADDA_up8x_txif_clk_inv_LEN	1

#define	ADDA_if_lookback1_POS	4
#define	ADDA_if_lookback1_LEN	1

#define	ADDA_up8x_rxif_fifo_rsp_adc_POS	24
#define	ADDA_up8x_rxif_fifo_rsp_adc_LEN	3

#define	ADDA_up8x_rxif_clkinv_adc_POS	27
#define	ADDA_up8x_rxif_clkinv_adc_LEN	1

#define	ADDA_dlsatflag_clr_POS	31
#define	ADDA_dlsatflag_clr_LEN	1

/* AFE_ADDA_NEWIF_CFG1 */
#define ADDA_up8x_rxif_adc_sync_cnt_table_POS 0
#define ADDA_up8x_rxif_adc_sync_cnt_table_LEN 9
#define ADDA_up8x_rxif_adc_voice_mode_POS	10
#define ADDA_up8x_rxif_adc_voice_mode_LEN	2
#define ADDA_up8x_rxif_adc_sync_search_table_POS	12
#define ADDA_up8x_rxif_adc_sync_search_table_LEN	5
#define ADDA_up8x_rxif_adc_sync_check_round_POS		20
#define ADDA_up8x_rxif_adc_sync_check_round_LEN		4
#define ADDA_up8x_rxif_adc_invalid_sync_check_round_POS	24
#define	ADDA_up8x_rxif_adc_invalid_sync_check_round_LEN	4

/* AFE_ADDA_PREDIS_CON0 */
#define ADDA_predis_a3_ch1_ctl_POS	0
#define ADDA_predis_a3_ch1_ctl_LEN	12
#define	ADDA_predis_a2_ch1_ctl_POS	16
#define ADDA_predis_a2_ch1_ctl_LEN	12
#define ADDA_predis_on_ch1_ctl_POS	31
#define ADDA_predis_on_ch1_ctl_LEN	1
/* AFE_ADDA_PREDIS_CON1 */
#define ADDA_predis_a3_ch2_ctl_POS	0
#define ADDA_predis_a3_ch2_ctl_LEN	12
#define	ADDA_predis_a2_ch2_ctl_POS	16
#define ADDA_predis_a2_ch2_ctl_LEN	12
#define ADDA_predis_on_ch2_ctl_POS	31
#define ADDA_predis_on_ch2_ctl_LEN	1

/* AFE_MEMIF_MAXLEN // EMI access enable, sysram access disable */
#define AFE_MEMIF_MAXLEN_DL1_CONFIG_POS	0
#define AFE_MEMIF_MAXLEN_DL1_CONFIG_LEN	1
#define AFE_MEMIF_MAXLEN_DL2_CONFIG_POS	4
#define AFE_MEMIF_MAXLEN_DL2_CONFIG_LEN	1


/* AFE_MEMIF_PBUF_SIZE */
#define AFE_MEMIF_HDMI_PBUF_SIZE_MASK  (3 << 18)
#define AFE_MEMIF_HDMI_PBUF_SIZE_16BYTES  (0 << 18)
#define AFE_MEMIF_HDMI_PBUF_SIZE_32BYTES  (1 << 18)
#define AFE_MEMIF_HDMI_PBUF_SIZE_64BYTES  (2 << 18)
#define AFE_MEMIF_HDMI_PBUF_SIZE_128BYTES (3 << 18)

#define AFE_MEMIF_IEC_PBUF_SIZE_MASK   (3 << 20)
#define AFE_MEMIF_IEC_PBUF_SIZE_16BYTES   (0 << 20)
#define AFE_MEMIF_IEC_PBUF_SIZE_32BYTES   (1 << 20)
#define AFE_MEMIF_IEC_PBUF_SIZE_64BYTES   (2 << 20)
#define AFE_MEMIF_IEC_PBUF_SIZE_128BYTES  (3 << 20)

/* AFE_IRQ_CNT5 */
#define AFE_IRQ_MCU_CNT5_MASK  0x3FFFF


/* AFE_HDMI_OUT_BASE */
#define AFE_HDMI_OUT_BASE_MASK    0xFFFFFFF8

/* AFE_HDMI_OUT_END */
#define AFE_HDMI_OUT_END_MASK     0xFFFFFFF8
/* AFE_SPDIFIN_EC */
#define SPDIFIN_INT_ERR_CLEAR_MASK 0xFFF
#define SPDIFIN_PRE_ERR_CLEAR                  (0x1 << 0)
#define SPDIFIN_PRE_ERR_B_CLEAR                (0x1 << 1)
#define SPDIFIN_PRE_ERR_M_CLEAR                (0x1 << 2)
#define SPDIFIN_PRE_ERR_W_CLEAR                (0x1 << 3)
#define SPDIFIN_PRE_ERR_BITCNT_CLEAR           (0x1 << 4)
#define SPDIFIN_PRE_ERR_PARITY_CLEAR           (0x1 << 5)
#define SPDIFIN_TIMEOUT_INT_CLEAR              (0x1 << 8)
#define SPDIFIN_CHSTS_PREAMPHASIS_CLEAR        (0x1 << 9)	/* channel status and emphasis */
#define SPDIFIN_CHSTS_COLLECTION_CLEAR         (0x1 << 11)
#define EDGE_FLAG_OPT_CLEAR                    (0x1 << 12)
#define EDGE_FLAG_COA_CLEAR                    (0x1 << 13)
#define EDGE_FLAG_ARC_CLEAR                    (0x1 << 14)
#define SPDIFIN_DATA_LATCH_CLEAR               (0x1 << 17)
#define SPDIFIN_INT_CLEAR_ALL                  (SPDIFIN_PRE_ERR_CLEAR|SPDIFIN_PRE_ERR_B_CLEAR|\
						SPDIFIN_PRE_ERR_M_CLEAR|SPDIFIN_PRE_ERR_W_CLEAR|\
						SPDIFIN_PRE_ERR_BITCNT_CLEAR|SPDIFIN_PRE_ERR_PARITY_CLEAR|\
						SPDIFIN_TIMEOUT_INT_CLEAR|SPDIFIN_CHSTS_PREAMPHASIS_CLEAR|\
						SPDIFIN_DATA_LATCH_CLEAR|SPDIFIN_CHSTS_COLLECTION_CLEAR)



/* AFE_SPDIFIN_BR */
#define AFE_SPDIFIN_BRE_MASK           (0x1 << 0)
#define AFE_SPDIFIN_BR_FS_MASK         (0x7 << 4)
#define AFE_SPDIFIN_BR_FS_256      (0x3 << 4)

#define AFE_SPDIFIN_BR_SUBFRAME_MASK   (0xF << 8)
#define AFE_SPDIFIN_BR_SUBFRAME_256 (0x8 << 8)
#define AFE_SPDIFIN_BR_LOWBOUND_MASK   (0x1F << 12)
#define AFE_SPDIFIN_BR_TUNE_MODE_MASK  (0x3 << 17)
#define AFE_SPDIFIN_BR_TUNE_MODE0  (0x0 << 17)
#define AFE_SPDIFIN_BR_TUNE_MODE1  (0x1 << 17)
#define AFE_SPDIFIN_BR_TUNE_MODE2  (0x2 << 17)
/* AFE_SPDIFIN_INT_EXT */

#define MULTI_INPUT_DETECT_SEL_MASK    (0xF << 8)
#define MULTI_INPUT_DETECT_SEL_NONE	(0)
#define MULTI_INPUT_DETECT_SEL_OPT (0x1 << 8)
#define MULTI_INPUT_DETECT_SEL_COA (0x2 << 8)
#define MULTI_INPUT_DETECT_SEL_ARC (0x4 << 8)

#define MULTI_INPUT_SEL_MASK           (0x3 << 14)
#define MULTI_INPUT_SEL_OPT        (0x0 << 14)
#define MULTI_INPUT_SEL_COA        (0x1 << 14)
#define MULTI_INPUT_SEL_ARC        (0x2 << 14)
#define MULTI_INPUT_SEL_LOW        (0x3 << 14)

#define SPDIFIN_DATALATCH_ERR_EN_MASK  (0x1 << 17)
#define SPDIFIN_DATALATCH_ERR_EN   (0x1 << 17)
#define SPDIFIN_DATALATCH_ERR_DIS  (0x0 << 17)

#define MULTI_INPUT_STATUS_MASK    (0xF << 28)
#define MULTI_INPUT_STATUS_OPT     (0x1 << 28)
#define MULTI_INPUT_STATUS_COA     (0x2 << 28)
#define MULTI_INPUT_STATUS_ARC     (0x4 << 28)

/* AFE_SPDIFIN_INT_EXT2 */
#define SPDIFIN_LRC_MASK               0x7FFF
#define SPDIFIN_LRC_COMPARE_432M       0x10E
#define SPDIFIN_LRC_COMPARE_594M       0x173

#define SPDIFIN_LRCK_CHG_INT_MASK  (0x1 << 15)
#define SPDIFIN_LRCK_CHG_INT_EN    (0x1 << 15)
#define SPDIFIN_LRCK_CHG_INT_DIS   (0x0 << 15)

#define SPDIFIN_432MODE_MASK       (0x1 << 16)
#define SPDIFIN_432MODE_EN         (0x1 << 16)
#define SPDIFIN_432MODE_DIS        (0x0 << 16)

#define SPDIFIN_MODE_CLK_MASK      (0x3 << 16)

#define SPDIFIN_594MODE_MASK       (0x1 << 17)
#define SPDIFIN_594MODE_EN         (0x1 << 17)
#define SPDIFIN_594MODE_DIS        (0x0 << 17)

#define SPDIFIN_LRCK_CHG_INT_STS   (0x1 << 27)

#define SPDIFIN_ROUGH_FS_MASK      (0xF << 28)
#define SPDIFIN_ROUGH_FS_32K       (0x1 << 28)
#define SPDIFIN_ROUGH_FS_44K       (0x2 << 28)
#define SPDIFIN_ROUGH_FS_48K       (0x3 << 28)
#define SPDIFIN_ROUGH_FS_64K       (0x4 << 28)
#define SPDIFIN_ROUGH_FS_88K       (0x5 << 28)
#define SPDIFIN_ROUGH_FS_96K       (0x6 << 28)
#define SPDIFIN_ROUGH_FS_128K      (0x7 << 28)
#define SPDIFIN_ROUGH_FS_144K      (0x8 << 28)
#define SPDIFIN_ROUGH_FS_176K      (0x9 << 28)
#define SPDIFIN_ROUGH_FS_192K      (0xA << 28)
#define SPDIFIN_ROUGH_FS_216K      (0xB << 28)
/* AFE_SPDIFIN_CFG0 */
#define SPDIFIN_EN_MASK          (0x1 << 0)
#define SPDIFIN_EN               (0x1 << 0)
#define SPDIFIN_DIS              (0x0 << 0)

#define SPDIFIN_FLIP_EN_MASK     (0x1 << 1)
#define SPDIFIN_FLIP_EN          (0x1 << 1)
#define SPDIFIN_FLIP_DIS         (0x0 << 1)

#define SPDIFIN_INT_EN_MASK      (0x1 << 6)
#define SPDIFIN_INT_EN           (0x1 << 6)
#define SPDIFIN_INT_DIS          (0x0 << 6)

#define SPDIFIN_DE_CNT_MASK      (0x1F << 8)

#define SPDIFIN_DE_SEL_MASK      (0x3 << 13)
#define SPDIFIN_DE_SEL_3SAMPLES  (0x0)
#define SPDIFIN_DE_SEL_14SAMPLES (0x1 << 13)
#define SPDIFIN_DE_SEL_30SAMPLES (0x2 << 13)
#define SPDIFIN_DE_SEL_DECNT     (0x3 << 13)

#define MAX_LEN_NUM_MASK         (0xFF << 16)
/* AFE_SPDIFIN_CFG1 */

#define SPDIFIN_DATA_FROM_DECODER0_EN_MASK   (0x1 << 0)
#define SPDIFIN_DATA_FROM_DECODER0_EN      (0x1 << 0)
#define SPDIFIN_DATA_FROM_DECODER0_DIS     (0x0 << 0)

#define SPDIFIN_CLK_FROM_DECODER0_EN_MASK  (0x1 << 1)
#define SPDIFIN_CLK_FROM_DECODER0_EN       (0x1 << 1)
#define SPDIFIN_CLK_FROM_DECODER0_DIS      (0x0 << 1)

#define SPDIFIN_DATA_FROM_DECODER1_EN_MASK (0x1 << 2)
#define SPDIFIN_DATA_FROM_DECODER1_EN      (0x1 << 2)
#define SPDIFIN_DATA_FROM_DECODER1_DIS     (0x0 << 2)

#define SPDIFIN_CLK_FROM_DECODER1_EN_MASK  (0x1 << 3)
#define SPDIFIN_CLK_FROM_DECODER1_EN       (0x1 << 3)
#define SPDIFIN_CLK_FROM_DECODER1_DIS      (0x0 << 3)

#define SPDIFIN_IECCLK2_FROM_SPDIF_EN_MASK (0x1 << 9)
#define SPDIFIN_IECCLK2_FROM_SPDIF_EN      (0x1 << 9)
#define SPDIFIN_IECCLK2_FROM_SPDIF_DIS     (0x0 << 9)

#define SPDIFIN_INT_ERR_EN_MASK             (0xFFF << 20)

#define SEL_BCK_SPDIFIN       (0x01<<16)
#define PINMUX_SPMCLK         (0x00<<17)
#define PINMUX_SPBCK          (0x01<<17)
#define PINMUX_MASK           (0x01<<17)

#define SPDIFIN_PRE_ERR_NON_EN             (0x1 << 20)
#define SPDIFIN_PRE_ERR_NON_DIS            (0x0 << 20)

#define SPDIFIN_PRE_ERR_B_EN               (0x1 << 21)
#define SPDIFIN_PRE_ERR_B_DIS              (0x0 << 21)

#define SPDIFIN_PRE_ERR_M_EN               (0x1 << 22)
#define SPDIFIN_PRE_ERR_M_DIS              (0x0 << 22)

#define SPDIFIN_PRE_ERR_W_EN               (0x1 << 23)
#define SPDIFIN_PRE_ERR_W_DIS              (0x0 << 23)

#define SPDIFIN_PRE_ERR_BITCNT_EN          (0x1 << 24)
#define SPDIFIN_PRE_ERR_BITCNT_DIS         (0x0 << 24)

#define SPDIFIN_PRE_ERR_PARITY_EN          (0x1 << 25)
#define SPDIFIN_PRE_ERR_PARITY_DIS         (0x0 << 25)

#define SPDIFIN_TIMEOUT_INT_EN             (0x1 << 28)
#define SPDIFIN_TIMEOUT_INT_DIS            (0x0 << 28)

#define SPDIFIN_CHSTS_AUDIOBIT_PREAMPHASIS_EN       (0x1 << 29) /* channel status and emphasis */
#define SPDIFIN_CHSTS_AUDIOBIT_PREAMPHASIS_DIS      (0x0 << 29)

#define SPDIFIN_CHSTS_COLLECTION_EN      (0x1 << 31)
#define SPDIFIN_CHSTS_COLLECTION_DIS     (0x0 << 31)

#define SPDIFIN_ALL_ERR_INT_EN         (SPDIFIN_PRE_ERR_NON_EN|SPDIFIN_PRE_ERR_B_EN|SPDIFIN_PRE_ERR_M_EN|\
					SPDIFIN_PRE_ERR_W_EN|SPDIFIN_PRE_ERR_BITCNT_EN|SPDIFIN_PRE_ERR_PARITY_EN|\
					SPDIFIN_CHSTS_AUDIOBIT_PREAMPHASIS_EN|SPDIFIN_CHSTS_COLLECTION_EN)

#define SPDIFIN_ALL_ERR_INT_DIS         (SPDIFIN_PRE_ERR_NON_DIS|SPDIFIN_PRE_ERR_B_DIS|SPDIFIN_PRE_ERR_M_DIS|\
					SPDIFIN_PRE_ERR_W_DIS|SPDIFIN_PRE_ERR_BITCNT_DIS|SPDIFIN_PRE_ERR_PARITY_DIS|\
					SPDIFIN_CHSTS_AUDIOBIT_PREAMPHASIS_DIS|SPDIFIN_CHSTS_COLLECTION_DIS)

/* AFE_SPDIFIN_CLK_CFG */

#define SPDIFIN_AUTOLOCK_EN_MASK   (0x1 << 0)
#define SPDIFIN_AUTOLOCK_EN    (0x1 << 0)
#define SPDIFIN_AUTOLOCK_DIS   (0x0 << 0)
#define SPDIFIN_APLL_393_MASK      (1 << 21)
#define SPDIFIN_LOCKED_FLAG        (1 << 23)
/* AFE_SPDIFIN_DEBUG1 */
#define SPDIFIN_DATA_LATCH_ERR     (1 << 10)

/* AFE_SPDIFIN_DEBUG3 */
#define SPDIFIN_PRE_ERR_NON_STS                  (0x1 << 0)
#define SPDIFIN_PRE_ERR_B_STS                    (0x1 << 1)
#define SPDIFIN_PRE_ERR_M_STS                    (0x1 << 2)
#define SPDIFIN_PRE_ERR_W_STS                    (0x1 << 3)
#define SPDIFIN_PRE_ERR_BITCNT_STS               (0x1 << 4)
#define SPDIFIN_PRE_ERR_PARITY_STS               (0x1 << 5)
#define SPDIFIN_TIMEOUT_ERR_STS                  (0x1 << 6)
#define SPDIFIN_CHSTS_AUDIOBIT_PREAMPHASIS_STS   (0x1 << 7)
#define SPDIFIN_ALL_ERR_ERR_STS                  (SPDIFIN_PRE_ERR_NON_STS|SPDIFIN_PRE_ERR_B_STS|\
						SPDIFIN_PRE_ERR_M_STS|SPDIFIN_PRE_ERR_W_STS|\
						SPDIFIN_PRE_ERR_BITCNT_STS|SPDIFIN_PRE_ERR_PARITY_STS|\
						SPDIFIN_TIMEOUT_ERR_STS)
/*AFE_SPDIFIN_DEBUG2*/
#define SPDIFIN_CHANNEL_STATUS_INT_FLAG          (0x1<<26)

/* AFE_MPHONE_MULTI_CON0 */
#define MULTI_HW_EN_MASK          0x1
#define MULTI_HW_EN           0x1
#define MULTI_HW_DIS          0x0
#define MULTI_STORE_TYPE_MASK     (0x1 << 1)
#define MULTI_STORE_24BIT     (0x1 << 1)
#define MULTI_STORE_16BIT     0x0
#define MULTI_INT_PERIOD_MASK (0x3 << 4)
#define MULTI_INT_PERIOD_64   (0x1 << 4)
#define MULTI_INT_PERIOD_128  (0x2 << 4)
#define MULTI_INT_PERIOD_256  (0x3 << 4)
/* AFE_SPDIFIN_BUF_CFG */
#define SPDIFIN_LOOPBACK_EN_MASK      (0x7 << 8)
#define SPDIFIN_LOOPBACK_OPT_EN_MASK  (0x1 << 8)
#define SPDIFIN_LOOPBACK_OPT_EN   (0x1 << 8)
#define SPDIFIN_LOOPBACK_OPT_DIS  (0x0 << 8)
#define SPDIFIN_LOOPBACK_COA_EN_MASK  (0x1 << 9)
#define SPDIFIN_LOOPBACK_COA_EN   (0x1 << 9)
#define SPDIFIN_LOOPBACK_COA_DIS  (0x0 << 9)
#define SPDIFIN_LOOPBACK_ARC_EN_MASK  (0x1 << 10)
#define SPDIFIN_LOOPBACK_ARC_EN   (0x1 << 10)
#define SPDIFIN_LOOPBACK_ARC_DIS  (0x0 << 10)

#define REG_ASRC_GEN_CONF                       (0x0B00)
#define POS_CH_CNTX_SWEN        20
#define MASK_CH_CNTX_SWEN       (1<<POS_CH_CNTX_SWEN)
#define POS_CH_CLEAR            16
#define MASK_CH_CLEAR           (1<<POS_CH_CLEAR)
#define POS_CH_EN               12
#define MASK_CH_EN              (1<<POS_CH_EN)
#define POS_DSP_CTRL_COEFF_SRAM 11
#define MASK_DSP_CTRL_COEFF_SRAM (1<<POS_DSP_CTRL_COEFF_SRAM)
#define POS_ASRC_BUSY           9
#define MASK_ASRC_BUSY          (1<<POS_ASRC_BUSY)
#define POS_ASRC_EN             8
#define MASK_ASRC_EN            (1<<POS_ASRC_EN)

#define REG_ASRC_IER                            (0x0B04)
#define REG_ASRC_IFR                            (0x0B08)

#define REG_ASRC_CH01_CNFG                      (0x0B10)
#define POS_CLR_IIR_BUF         23
#define MASK_CLR_IIR_BUF        (1<<POS_CLR_IIR_BUF)
#define POS_OBIT_WIDTH          22
#define MASK_OBIT_WIDTH         (1<<POS_OBIT_WIDTH)
#define POS_IBIT_WIDTH          21
#define MASK_IBIT_WIDTH         (1<<POS_IBIT_WIDTH)
#define POS_MONO                20
#define MASK_MONO               (1<<POS_MONO)
#define POS_OFS                 18
#define MASK_ASRC_OFS                (3<<POS_OFS)
#define POS_IFS                 16
#define MASK_ASRC_IFS                (3<<POS_IFS)
#define POS_CLAC_AMOUNT         8
#define MASK_CLAC_AMOUNT        (0xFF<<POS_CLAC_AMOUNT)
#define POS_IIR_EN              7
#define MASK_IIR_EN             (1<<POS_IIR_EN)
#define POS_IIR_STAGE           4
#define MASK_IIR_STAGE          (7<<POS_IIR_STAGE)

#define REG_ASRC_FREQUENCY_0                    (0x0B20)
/* USE [23:0] */
#define REG_ASRC_FREQUENCY_1                    (0x0B24)
/* USE [23:0] */
#define REG_ASRC_FREQUENCY_2                    (0x0B28)
/* USE [23:0] */
#define REG_ASRC_FREQUENCY_3                    (0x0B2C)
/* USE [23:0] */

#define REG_ASRC_IBUF_SADR                      (0x0B30)
#define POS_IBUF_SADR           0
#define MASK_IBUF_SADR          (0xFFFFFFFF<<POS_IBUF_SADR)
#define REG_ASRC_IBUF_SIZE                      (0x0B34)
#define POS_CH_IBUF_SIZE        0
#define MASK_CH_IBUF_SIZE       (0xFFFFF<<POS_CH_IBUF_SIZE)
#define REG_ASRC_OBUF_SADR                      (0x0B38)
#define POS_OBUF_SADR           0
#define MASK_OBUF_SADR          (0xFFFFFFFF<<POS_OBUF_SADR)
#define REG_ASRC_OBUF_SIZE                      (0x0B3C)
#define POS_CH_OBUF_SIZE        0
#define MASK_CH_OBUF_SIZE       (0xFFFFF<<POS_CH_OBUF_SIZE)

#define REG_ASRC_CH01_IBUF_RDPNT                (0x0B40)
#define POS_ASRC_CH01_IBUF_RDPNT    0
#define MASK_ASRC_CH01_IBUF_RDPNT   (0xFFFFFFFF<<POS_ASRC_CH01_IBUF_RDPNT)
#define REG_ASRC_CH01_IBUF_WRPNT                (0x0B50)
#define POS_ASRC_CH01_IBUF_WRPNT    0
#define MASK_ASRC_CH01_IBUF_WRPNT   (0xFFFFFFFF<<POS_ASRC_CH01_IBUF_WRPNT)
#define REG_ASRC_CH01_OBUF_WRPNT                (0x0B60)
#define POS_ASRC_CH01_OBUF_WRPNT    0
#define MASK_ASRC_CH01_OBUF_WRPNT   (0xFFFFFFFF<<POS_ASRC_CH01_OBUF_WRPNT)
#define REG_ASRC_CH01_OBUF_RDPNT                (0x0B70)
#define POS_ASRC_CH01_OBUF_RDPNT    0
#define MASK_ASRC_CH01_OBUF_RDPNT   (0xFFFFFFFF<<POS_ASRC_CH01_OBUF_RDPNT)

#define REG_ASRC_IBUF_INTR_CNT0                 (0x0B80)
#define POS_CH01_IBUF_INTR_CNT      8
#define MASK_CH01_IBUF_INTR_CNT     (0xFF<<POS_CH01_IBUF_INTR_CNT)
#define REG_ASRC_OBUF_INTR_CNT0                 (0x0B88)
#define POS_CH01_OBUF_INTR_CNT      8
#define MASK_CH01_OBUF_INTR_CNT     (0xFF<<POS_CH01_OBUF_INTR_CNT)

#define REG_ASRC_BAK_REG                        (0x0B90)
#define POS_RESULT_SEL              0
#define MASK_RESULT_SEL             (7<<POS_CH01_IBUF_INTR_CNT)

#define REG_ASRC_FREQ_CALI_CTRL                      (0x0B94)
#define POS_FREQ_CALC_BUSY      20
#define MASK_FREQ_CALC_BUSY     (1<<POS_FREQ_CALC_BUSY)
#define POS_COMP_FREQRES_EN     19
#define MASK_COMP_FREQRES_EN    (1<<POS_COMP_FREQRES_EN)
#define POS_SRC_SEL             16
#define MASK_SRC_SEL            (3<<POS_SRC_SEL)
#define POS_BYPASS_DEGLITCH     15
#define MASK_BYPASS_DEGLITCH    (1<<POS_BYPASS_DEGLITCH)
#define POS_MAX_GWIDTH          12
#define MASK_MAX_GWIDTH         (7<<POS_MAX_GWIDTH)
#define POS_AUTO_FS2_UPDATE     11
#define MASK_AUTO_FS2_UPDATE    (1<<POS_AUTO_FS2_UPDATE)
#define POS_AUTO_RESTART        10
#define MASK_AUTO_RESTART       (1<<POS_AUTO_RESTART)
#define POS_FREQ_UPDATE_FS2     9
#define MASK_FREQ_UPDATE_FS2    (1<<POS_FREQ_UPDATE_FS2)
#define POS_CALI_EN             8
#define MASK_CALI_EN            (1<<POS_CALI_EN)

#define REG_ASRC_FREQ_CALI_CYC                       (0x0B98)
#define POS_ASRC_FREQ_CALI_CYC  8
#define MASK_ASRC_FREQ_CALI_CYC (0xFFFF<<POS_ASRC_FREQ_CALI_CYC)

#define REG_ASRC_PRD_CALI_RESULT                     (0x0B9C)
#define POS_ASRC_PRD_CALI_RESULT    0
#define MASK_ASRC_PRD_CALI_RESULT   (0xFFFFFF<<POS_ASRC_PRD_CALI_RESULT)

#define REG_ASRC_FREQ_CALI_RESULT                    (0x0BA0)
#define POS_ASRC_FREQ_CALI_RESULT   0
#define MASK_ASRC_FREQ_CALI_RESULT  (0xFFFFFF<<POS_ASRC_FREQ_CALI_RESULT)

#define REG_ASRC_CALI_DENOMINATOR                    (0x0BD8)
#define POS_ASRC_CALI_DENOMINATOR   0
#define MASK_ASRC_CALI_DENOMINATOR  (0xFFFFFF<<POS_ASRC_CALI_DENOMINATOR)

#define REG_ASRC_MAX_OUT_PER_IN0                     (0x0BE0)
#define POS_CH01_MAX_OUT_PER_IN0    8
#define MASK_CH01_MAX_OUT_PER_IN0   (0xF<<POS_CH01_MAX_OUT_PER_IN0)

#define REG_ASRC_IIR_CRAM_ADDR                       (0x0BF0)
#define POS_ASRC_IIR_CRAM_ADDR      0
#define MASK_ASRC_IIR_CRAM_ADDR     (0xFF<<POS_ASRC_IIR_CRAM_ADDR)

#define REG_ASRC_IIR_CRAM_DATA                       (0x0BF4)
#define POS_ASRC_IIR_CRAM_DATA      0
#define MASK_ASRC_IIR_CRAM_DATA     (0xFFFFFFFF<<POS_ASRC_IIR_CRAM_DATA)

#define REG_ASRC_OUT_BUF_MON0                        (0x0BF8)
#define POS_WDLE_CNT                8
#define MASK_WDLE_CNT               (0xFF<<POS_WDLE_CNT)
#define POS_ASRC_WRITE_DONE         0
#define MASK_ASRC_WRITE_DONE        (1<<POS_ASRC_WRITE_DONE)

#define REG_ASRC_OUT_BUF_MON1                        (0x0BFC)
#define POS_ASRC_WR_ADR             4
#define MASK_ASRC_WR_ADR            (0x0FFFFFFF<<POS_ASRC_WR_ADR)

#define CLK_CFG_5    (TOPCKGEN_BASE+0x0090)
#define APLL_DIV_CLK_MASK	(0x7 << 16)
#define HDMI_POWER_UP_MASK	(0x1 << 20)
#define SPDIF1_POWER_UP_MASK	(0x1 << 21)
#define SPDIF2_POWER_UP_MASK	(0x1 << 22)
#define HDMISPDIF_POWER_UP_MASK	(0x1 << 23)

#define AFE_I2S_UL1_REORDER   (0x172C)
#define I2S_REORDER_UL_SEL_UL6_POS	(14)
#define I2S_REORDER_UL_SEL_UL6_MASK       (0x1 << 14)
#define I2S_REORDER_UL_SEL_UL6_DEFAULT    (0x0 << 14)
#define I2S_REORDER_UL_SEL_UL6_REORDER    (0x1 << 14)
#define I2S_REORDER_BSWAP_UL6_MASK    (0x1 << 13)
#define I2S_REORDER_BSWAP_UL6_NO      (0x0 << 13)
#define I2S_REORDER_BSWAP_UL6_YES     (0x1 << 13)
#define I2S_REORDER_EN_UL6_POS  (12)
#define I2S_REORDER_EN_UL6_MASK	(0x1 << 12)
#define I2S_REORDER_DIS_UL6     (0x0 << 12)
#define I2S_REORDER_EN_UL6      (0x1 << 12)
#define I2S_REORDER_CHNUM_UL6_MASK	(0x3 << 8)
#define I2S_REORDER_CHNUM_UL6_2CH	(0x0 << 8)
#define I2S_REORDER_CHNUM_UL6_4CH	(0x1 << 8)
#define I2S_REORDER_CHNUM_UL6_6CH	(0x2 << 8)
#define I2S_REORDER_CHNUM_UL6_8CH	(0x3 << 8)

#define I2S_REORDER_UL_SEL_MASK       (0x1 << 6)
#define I2S_REORDER_UL_SEL_DEFAULT    (0x0 << 6)
#define I2S_REORDER_UL_SEL_REORDER    (0x1 << 6)
#define I2S_REORDER_BSWAP_MASK    (0x1 << 5)
#define I2S_REORDER_BSWAP_NO      (0x0 << 5)
#define I2S_REORDER_BSWAP_YES     (0x1 << 5)
#define I2S_REORDER_EN_MASK (0x1 << 4)
#define I2S_REORDER_DIS     (0x0 << 4)
#define I2S_REORDER_EN      (0x1 << 4)
#define I2S_REORDER_CHNUM_MASK   (0x3 << 0)
#define I2S_REORDER_CHNUM_2CH   (0x0 << 0)
#define I2S_REORDER_CHNUM_4CH   (0x1 << 0)
#define I2S_REORDER_CHNUM_6CH   (0x2 << 0)
#define I2S_REORDER_CHNUM_8CH   (0x3 << 0)

#define AFE_TDM_CON1             (0x0054)
#define TDM_OUT_EN               (0x1 << 0)
#define TDM_OUT_BCK_INVERSE      (0x1 << 1)
#define TDM_OUT_LRCK_INVERSE     (0x1 << 2)
#define TDM_OUT_DELAY_DATA       (0x1 << 3)
#define TDM_OUT_LEFT_ALIGN       (0x1 << 4)
#define TDM_OUT_WLEN             (0x3 << 8)
#define TDM_OUT_CH_BCK_CYCLE     (0x3 << 10)
#define TDM_OUT_CHNUM            (0x7 << 12)
#define TDM_OUT_BIT_NUM          (0x1F << 16)
#define TDM_OUT_LRCK_WIDTH       (0x1FF << 23)

#define AFE_TDM_CON2             (0x0058)
#define ST_CH_PAIR_SOUT0         (0xF << 0)
#define ST_CH_PAIR_SOUT1         (0xF << 4)
#define ST_CH_PAIR_SOUT2         (0xF << 8)
#define ST_CH_PAIR_SOUT3         (0xF << 12)
#define TDM_OUT_FIX_VAL_EN       (0x1 << 16)
#define TDM_OUT_I2S_LOOPBACK     (0x1 << 20)
#define TDM_OUT_I2S_LOOPBACK_CH  (0x3 << 21)
#define TDM_OUT_FIX_VAL          (0xFF << 24)

#define AFE_TDMOUT_CLKDIV_CFG    (0x0078)
#define TDMOUT_PDN_M             (0x1 << 0)
#define TDMOUT_PDN_B             (0x1 << 1)
#define TDMOUT_MCK_SEL           (0x1 << 2)
#define TDMOUT_INV_M             (0x1 << 4)
#define TDMOUT_INV_B             (0x1 << 5)
#define TDMOUT_CLK_DIV_M         (0xFFF << 8)
#define TDMOUT_CLK_DIV_B         (0xFFF << 20)

#define AFE_TDM_IN_CON1          (0x0060)
#define AFE_TDM_IN_CON1_LRCK_WIDTH(x) (((x) - 1) << 23)
#define TDM_IN_EN                (0x1 << 0)
#define TDM_IN_DISABLE            (0x0 << 0)
#define TDM_IN_BCK_INVERSE       (0x1 << 1)
#define TDM_IN_LRCK_INVERSE      (0x1 << 2)
#define TDM_IN_FMT               (0x1 << 3)
#define TDM_IN_LR_SWAP           (0x1 << 4)
#define TDM_LRCK_DLY			 (0x1 << 5)
#define TDM_IN_WLEN              (0x3 << 8)
#define TDM_IN_CHNUM             (0x7 << 12)
#define TDM_IN_LRCK_CYCLE        (0x3 << 16)
#define TDM_IN_OUT_SYNC          (0x1 << 18)
#define TDM_IN_LOOPBACK          (0x1 << 20)
#define TDM_IN_LOOPBACK_CH       (0x3 << 21)
#define TDM_IN_LRCK_WIDTH        (0x1FF << 23)
#define AFE_TDM_IN_CON1_EN  BIT(0)
#define AFE_TDM_IN_CON1_LRCK_DELAY BIT(5)
#define AFE_TDM_IN_CON1_I2S      BIT(3)
#define AFE_TDM_IN_CON1_LRCK_INV BIT(2)
#define AFE_TDM_IN_CON1_BCK_INV  BIT(1)
#define AFE_TDM_IN_CON1_WLEN_32BIT      (0x3 << 8)
#define AFE_TDM_IN_CON1_WLEN_24BIT      (0x2 << 8)
#define AFE_TDM_IN_CON1_WLEN_16BIT      (0x1 << 8)
#define AFE_TDM_IN_CON1_8CH_PER_SDATA   (0x2 << 12)
#define AFE_TDM_IN_CON1_4CH_PER_SDATA   (0x1 << 12)
#define AFE_TDM_IN_CON1_2CH_PER_SDATA   (0x0 << 12)
#define AFE_TDM_IN_CON1_FAST_LRCK_CYCLE_32BCK   (0x2 << 16)
#define AFE_TDM_IN_CON1_FAST_LRCK_CYCLE_24BCK   (0x1 << 16)
#define AFE_TDM_IN_CON1_FAST_LRCK_CYCLE_16BCK   (0x0 << 16)


#define AFE_TDM_IN_CON2          (0x0064)
#define TDMIN_DISABLE_OUT        (0xFF << 0)
#define TDMIN_ODD_FLAG           (0xFF << 8)
#define TDMIN_CH_SWAP            (0xFF << 16)

#define AFE_TDM_IN_MON1          (0x0068)
#define AFE_TDM_IN_MON2          (0x006C)
#define AFE_TDM_IN_MON3          (0x0070)

#define AFE_TDMIN_CLKDIV_CFG     (0x007C)
#define TDMIN_PDN_M              (0x1 << 0)
#define TDMIN_PDN_B              (0x1 << 1)
#define TDMIN_MCK_SEL_MASK       (0x1 << 2)
#define TDMIN_MCK_SEL_APLL1      (0x1 << 2)
#define TDMIN_MCK_SEL_APLL2      (0)
#define TDMIN_INV_M              (0x1 << 4)
#define TDMIN_INV_B              (0x1 << 5)
#define TDMIN_CLK_DIV_M_MASK     (0xFFF << 8)
#define TDMIN_CLK_DIV_B_MASK     (0xFFF << 20)
#define TDMIN_CLK_DIV_M_POS      (8)
#define TDMIN_CLK_DIV_B_POS      (20)
#define AFE_SINEGEN_CON_TDM      (0x005C)
#define AFE_SINEGEN_CON_TDM_IN   (0x0074)

#define SINE_TONE_FREQ_DIV_TDM1      (0x1F << 0)
#define SINE_TONE_AMP_DIV_TDM1       (0x7 << 5)
#define SINE_TONE_MUTE_SW_TDM1       (0x1 << 8)
#define SINE_TONE_FREQ_DIV_TDM2      (0x1F << 12)
#define SINE_TONE_AMP_DIV_TDM2       (0x7 << 17)
#define SINE_TONE_MUTE_SW_TDM2       (0x1 << 20)
#define SINE_TONE_C_DAC_EN_TDM       (0x1 << 24)
#define SINE_TONE_TDM_INPUT_EN       (0x1 << 28)

#define AFE_TSF_CON                  (0x1710)
#define CONNSYS_TSF_INTR_SEL         (0x1 << 0)
#define DL_AGENT_HW_DIS              (0x1 << 4)
#define DL1_AUTO_EN                  (0x1 << 16)
#define DL2_AUTO_EN                  (0x1 << 17)
#define DL3_AUTO_EN                  (0x1 << 18)
#define DL4_AUTO_EN                  (0x1 << 19)
#define DL5_AUTO_EN                  (0x1 << 20)
#define DL6_AUTO_EN                  (0x1 << 21)
#define DLMCH_AUTO_EN                (0x1 << 22)
#define TDM_OUT_AGENT_HW_DIS         (0x1 << 24)
#define TDM_OUT_AUTO_EN              (0x1 << 25)

#define AFE_TSF_MON                  (0x1714)
#define AFE_REORDER_CH_MAP			(0x17B0)
#define REORDER_UL6_CH1_SEL_MASK	(0x7)
#define REORDER_UL6_CH2_SEL_MASK	(0x7 << 3)
#define REORDER_UL6_CH3_SEL_MASK	(0x7 << 6)
#define REORDER_UL6_CH4_SEL_MASK	(0x7 << 9)
#define REORDER_UL6_CH5_SEL_MASK	(0x7 << 12)
#define REORDER_UL6_CH6_SEL_MASK	(0x7 << 15)
#define REORDER_UL6_CH7_SEL_MASK	(0x7 << 18)
#define REORDER_UL6_CH8_SEL_MASK	(0x7 << 21)
#define REORDER_UL6_CH_SEL_I2SIN3_L	0
#define REORDER_UL6_CH_SEL_I2SIN3_R	1
#define REORDER_UL6_CH_SEL_I2SIN4_L	2
#define REORDER_UL6_CH_SEL_I2SIN4_R	3
#define REORDER_UL6_CH_SEL_DMIC1_L	4
#define REORDER_UL6_CH_SEL_DMIC1_R	5
#define REORDER_UL6_CH_SEL_DMIC2_L	6
#define REORDER_UL6_CH_SEL_DMIC2_R	7

#define AFE_COMMON_EN_CFG			(0x18A0)
#define TDM_IN_ENABLE_SEL_MASK		(1 << 9)
#define TDM_IN_ENABLE_SEL_COMMON	(1 << 9)
#define TDM_IN_ENABLE_SEL_SELF		(0)
#define TDM_AGENT_ENABLE_SEL_MASK	(1 << 8)
#define TDM_AGENT_ENABLE_SEL_COMMON	(1 << 8)
#define TDM_AGENT_ENABLE_SEL_SELF	(0)
#define DMIC2_ENABLE_SEL_MASK	(1 << 7)
#define DMIC2_ENABLE_SEL_COMMON	(1 << 7)
#define DMIC2_ENABLE_SEL_SELF	(0)
#define DMIC1_ENABLE_SEL_MASK	(1 << 6)
#define DMIC1_ENABLE_SEL_COMMON	(1 << 6)
#define DMIC1_ENABLE_SEL_SELF	(0)
#define UL4_AGENT_ENABLE_SEL_MASK	(1 << 5)
#define UL4_AGENT_ENABLE_SEL_COMMON	(1 << 5)
#define UL4_AGENT_ENABLE_SEL_SELF	(0)
#define UL3_AGENT_ENABLE_SEL_MASK	(1 << 4)
#define UL3_AGENT_ENABLE_SEL_COMMON	(1 << 4)
#define UL3_AGENT_ENABLE_SEL_SELF	(0)
#define I2SIN4_ENABLE_SEL_MASK	(1 << 3)
#define I2SIN4_ENABLE_SEL_COMMON	(1 << 3)
#define I2SIN4_ENABLE_SEL_SELF	(0)
#define I2SIN3_ENABLE_SEL_MASK	(1 << 2)
#define I2SIN3_ENABLE_SEL_COMMON	(1 << 2)
#define I2SIN3_ENABLE_SEL_SELF	(0)
#define UL6_AGENT_ENABLE_SEL_MASK	(1 << 1)
#define UL6_AGENT_ENABLE_SEL_COMMON	(1 << 1)
#define UL6_AGENT_ENABLE_SEL_SELF	(0)
#define AFE_COMMON_ENABLE_MASK	(1)
#define AFE_COMMON_ENABLE_ON	(1)
#define AFE_COMMON_ENABLE_OFF	(0)

#define AFE_SECURE_CON              (0x17A0)
#define AFE_SRAM_BOUND              (0x17A4)
#define AFE_SE_SECURE_CON           (0x17A8)
#define AFE_SECURE_MASK_LOOPBACK0   (0x17B4)
#define AFE_SECURE_MASK_LOOPBACK1   (0x17B8)
#define AFE_SECURE_MASK_LOOPBACK    (0x17BC)
#define AFE_SECURE_MASK_CONN0       (0x17C0)
#define AFE_SECURE_MASK_CONN1       (0x17C4)
#define AFE_SECURE_MASK_CONN2       (0x17C8)
#define AFE_SECURE_MASK_CONN3       (0x17CC)
#define AFE_SECURE_MASK_CONN4       (0x17D0)
#define AFE_SECURE_MASK_CONN5       (0x17D4)
#define AFE_SECURE_MASK_CONN6       (0x17D8)
#define AFE_SECURE_MASK_CONN7       (0x17DC)
#define AFE_SECURE_MASK_CONN8       (0x17E0)
#define AFE_SECURE_MASK_CONN9       (0x17E4)
#define AFE_SECURE_MASK_CONN10      (0x17E8)
#define AFE_SECURE_MASK_CONN11      (0x17EC)
#define AFE_SECURE_MASK_CONN12      (0x17F0)
#define AFE_SECURE_MASK_CONN13      (0x17F4)
#define AFE_SECURE_MASK_CONN14      (0x17F8)
#define AFE_SECURE_MASK_CONN15      (0x17FC)
#define AFE_SECURE_MASK_CONN16      (0x1800)
#define AFE_SECURE_MASK_CONN17      (0x1804)
#define AFE_SECURE_MASK_CONN18      (0x1808)
#define AFE_SECURE_MASK_CONN19      (0x180C)
#define AFE_SECURE_MASK_CONN20      (0x1810)
#define AFE_SECURE_MASK_CONN21      (0x1814)
#define AFE_SECURE_MASK_CONN22      (0x1818)
#define AFE_SECURE_MASK_CONN23      (0x181C)
#define AFE_SECURE_MASK_CONN24      (0x1820)
#define AFE_SECURE_MASK_CONN25      (0x1824)
#define AFE_SECURE_MASK_CONN26      (0x1828)
#define AFE_SECURE_MASK_CONN27      (0x182C)
#define AFE_SECURE_MASK_CONN28      (0x1830)
#define AFE_SECURE_MASK_CONN29      (0x1834)
#define AFE_SECURE_MASK_CONN30      (0x1838)
#define AFE_SECURE_MASK_CONN31      (0x183C)
#define AFE_SECURE_MASK_CONN32      (0x1840)
#define AFE_SECURE_MASK_CONN33      (0x1844)
#define AFE_SECURE_MASK_CONN34      (0x1848)
#define AFE_SECURE_MASK_CONN35      (0x184C)
#define AFE_SECURE_MASK_CONN36      (0x1850)
#define AFE_SECURE_MASK_CONN37      (0x1854)
#define AFE_SECURE_MASK_CONN38      (0x1858)
#define AFE_SECURE_MASK_CONN39      (0x185C)
#define AFE_SECURE_MASK_CONN40      (0x1860)
#define AFE_SECURE_MASK_CONN41      (0x1864)
#define AFE_SECURE_MASK_CONN_24_BIT (0X1868)
#define AFE_SECURE_MASK_CONN_16_BIT (0X186C)
#define AFE_SECURE_MASK_SIDEBAND0   (0x1870)
#define AFE_SECURE_MASK_SIDEBAND1   (0x1874)
#define AFE_SECURE_MASK_SIDEBAND2   (0x1878)
#define AFE_SECURE_MASK_BASE_ADR_MSB    (0x188C)
#define AFE_SECURE_MASK_END_ADR_MSB     (0x1890)

#endif
