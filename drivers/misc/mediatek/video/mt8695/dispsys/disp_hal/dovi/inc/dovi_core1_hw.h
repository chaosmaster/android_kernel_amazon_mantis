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

#ifndef _DOVI_CORE1_HW_H_
#define _DOVI_CORE1_HW_H_


#define CORE1_REG_BASE	            (0x000)
#define CORE1_CFG_REG_BASE			(CORE1_REG_BASE + 0x000)
#define CORE1_DPM_REG_BASE	        (CORE1_REG_BASE + 0x018)
#define CORE1_RAP_REG_BASE	        (CORE1_REG_BASE + 0x400)


/* 0x00 ~ 0x14 (0x18 - 0x00 = 0x18 )*/
#define CORE1_CFG_REG_NUM		(0x18/4)

/* 0xC00 ~ 0xC3C (0xCA8 - 0x0 = 0x40 ) */
#define CORE1_RAP_REG_NUM	    (0x100/4)

/* 0x18 ~ 0x398 (0x39C - 0x18 = 0x384) */
#define CORE1_DPM_REG_NUM	    (0x384/4)

#define CORE1_RST_REG	CORE1_RAP_REG_BASE

#define CORE1_RESET_ALL       0xF
#define CORE1_RESET_CLEAR     0x0

#define CORE1_PROG_START      (CORE1_REG_BASE+0x08)
#define CORE1_PROG_FINISH     (CORE1_REG_BASE+0x0C)

#define CORE1_CRC_TRIGGER     (CORE1_REG_BASE+0x3A8)

#define CORE1_EFUSE_REG      (CORE1_REG_BASE+0x004)

#define CORE1_BL_CRC     (CORE1_REG_BASE+0x3AC)
#define CORE1_EL_CRC     (CORE1_REG_BASE+0x3B0)
#define CORE1_CMP_CRC    (CORE1_REG_BASE+0x3B4)
#define CORE1_2TO4_CRC   (CORE1_REG_BASE+0x3B8)
#define CORE1_CSC_CRC    (CORE1_REG_BASE+0x3BC)
#define CORE1_CVM_CRC    (CORE1_REG_BASE+0x3C0)
#define CORE1_LUT_CRC    (CORE1_REG_BASE+0x3C4)

#define CORE1_CFG_REG_004 (0x004/4)
#define CORE1_BYPASS_COMPOSER	0x0
#define CORE1_BYPASS_CSC		0x1
#define CORE1_BYPASS_CVM		0x2
#define CORE1_OPERATING_MODE	0x3
#define CORE1_PIXEL_RATE		0x4
#define CORE1_EFUSE_DISABLE		0x8

/* 0x41800, 0x41900, 0x41A00 , 0x41B00 ~ 0x41BFC */
struct core1_cfg_reg_t {
uint32_t core1_cfg_reg[CORE1_CFG_REG_NUM];
};

/* 0x41C00 ~ 0x41C3C*/
struct core1_rap_reg_t {
/* ================ Control Start =================================*/
	/* 0xC00 CTRL_REG_00 */
	/* bit 3 : dma dram reset */
	/* bit 2 : core reset */
	/* bit 1 : dma reset */
	/* bit 0 : dolby AHB reset */
	UINT32 sw_rst:4;
	 UINT32:28;

	/* 0x04 CTRL_REG_04 */
	UINT32 dma_trigger:1;	/* write 0 then write 1 to trigger dma  */
	UINT32 dma_trigger_select:1;	/* 0 use bit 0 to trigger ; 1 use vysnc to trigger */
	 UINT32:30;

	/* 0x08 CTRL_REG_08 */
	UINT32 dma_vsync_select:1;	/* 0 select BL vsync; 1 select EL vsync */
	UINT32 dma_vsync_polarity_config:1;	/* 0 default vsync polarity; 1 invert polarity */
	UINT32 dma_en:1;	/* read clock domain */
	UINT32 dma_dram_en:1;	/* dram clock domain */
	UINT32 hsync_select:1;	/* 0 self gen hsync; 1 select external hsync */
	UINT32 vsync_select:1;	/* 0 self gen vsync; 1 select external vsync */
	UINT32 de_select:1;
	 UINT32:25;

	/* 0x0C  */
	 UINT32:32;

	/* 0x10 CTRL_REG_10 */
	 UINT32:4;
	UINT32 dma_start_addr:28;

	/* 0x14 CTRL_REG_14 */
	 UINT32:4;
	UINT32 dma_data_length:11;
	 UINT32:17;

	/* 0x18 CTRL_REG_18 */
	UINT32 dma_read_empty_threshold:7;
	UINT32:1;
	UINT32 dma_write_full_threshold:7;
	UINT32:1;
	UINT32:16;

	/* 0x1C CTRL_REG_1C */
	UINT32 bl_hsync_de_swap:1;
	UINT32 bl_vsync_sel_vde:1;
	UINT32 bl_post_adj_forward:1;
	 UINT32:1;
	UINT32 bl_hsync_pol_ctrl:1;
	UINT32 bl_vsync_pol_ctrl:1;
	UINT32 bl_de_pol_ctrl:1;
	 UINT32:1;
	UINT32 el_hsync_de_swap:1;
	UINT32 el_vsync_sel_vde:1;
	UINT32 el_post_adj_forward:1;
	 UINT32:1;
	UINT32 el_hsync_pol_ctrl:1;
	UINT32 el_vsync_pol_ctrl:1;
	UINT32 el_de_pol_ctrl:1;
	 UINT32:17;

	/* 0x20 CTRL_REG_20 */
	UINT32 bl_hsync_width:12;
	 UINT32:4;
	UINT32 bl_vsync_width:12;
	 UINT32:4;

	/* 0x24 CTRL_REG_24 */
	UINT32 bl_x_active_start:13;
	 UINT32:3;
	UINT32 bl_x_active_end:13;
	 UINT32:3;

	/* 0x18 CTRL_REG_28 */
	UINT32 bl_y_active_start:12;
	 UINT32:4;
	UINT32 bl_y_active_end:12;
	 UINT32:4;

	/* 0x2C CTRL_REG_2C */
	UINT32 bl_hsync_delay:13;
	 UINT32:3;
	UINT32 bl_vsync_delay:13;
	 UINT32:3;

	/* 0x30 CTRL_REG_30 */
	UINT32 el_hsync_width:12;
	 UINT32:4;
	UINT32 el_vsync_width:12;
	 UINT32:4;

	/* 0x34 CTRL_REG_34 */
	UINT32 el_x_active_start:13;
	 UINT32:3;
	UINT32 el_x_active_end:13;
	 UINT32:3;

	/* 0x38 CTRL_REG_38 */
	UINT32 el_y_active_start:12;
	 UINT32:4;
	UINT32 el_y_active_end:12;
	 UINT32:4;

	/* 0x3C CTRL_REG_3C */
	UINT32 el_hsync_delay:13;
	 UINT32:3;
	UINT32 el_vsync_delay:13;
	 UINT32:3;

	 /* 0x40 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x44 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x48 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x4C CTRL_REG_40 */
	 UINT32:32;

	 /* 0x50 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x54 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x58 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x5C CTRL_REG_40 */
	 UINT32:32;

	 /* 0x60 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x64 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x68 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x6C CTRL_REG_40 */
	 UINT32 pattern_I:10;
	 UINT32 pattern_Cp:10;
	 UINT32 pattern_Ct:10;
	 UINT32:1;
	 UINT32 out_patten_en:1;

	 /* 0x70 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x74 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x78 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x7C CTRL_REG_40 */
	 UINT32:32;

	 /* 0x80 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x84 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x88 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x8C CTRL_REG_40 */
	 UINT32:32;

	 /* 0x90 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x94 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x98 CTRL_REG_40 */
	 UINT32:32;

	 /* 0x9C CTRL_REG_40 */
	 UINT32:32;

	 /* 0xA0 CTRL_REG_40 */
	 UINT32 shadow_en:1;
	 UINT32 shadow_update_trigger:1;
	 UINT32:30;

	 /* 0xA4 CTRL_REG_40 */
	 UINT32:32;

	 /* 0xA8 CTRL_REG_40 */
	 UINT32:32;

/* ================ Control End =================================*/
};

struct core1_dpm_reg_t {
	uint32_t core1_dpm_reg[CORE1_DPM_REG_NUM];
};

union core1_cfg_reg_u {
	UINT32 reg[CORE1_CFG_REG_NUM];
	struct core1_cfg_reg_t fld;
};

union core1_rap_reg_u {
	UINT32 reg[CORE1_RAP_REG_NUM];
	struct core1_rap_reg_t fld;
};

union core1_dpm_reg_u {
	UINT32 reg[CORE1_DPM_REG_NUM];
	struct core1_dpm_reg_t fld;
};

#endif				/*  */
