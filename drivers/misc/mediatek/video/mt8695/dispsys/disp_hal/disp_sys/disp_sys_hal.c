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


#include "hdmitx.h"
#include "disp_sys_hal.h"
#include "disp_sys_hw.h"

struct disp_sys_context_t disp_sys;

void disp_sys_hal_set_disp_out(enum DISP_TYPE type, enum DISP_OUT_SEL out_sel)
{
	int reg = 0;

	if (type == DISP_MAIN)
		reg = DISP_MAIN_CONFIG;
	else
		reg = DISP_SUB_CONFIG;

	WriteREG32Msk((DISPSYS_BASE + reg), out_sel, DISP_OUT_SEL_MASK);
}


void disp_sys_hal_set_main_sub_swap(uint32_t swap)
{
	/* write Disp Top 0x15000014[19] = 1 */
	uint32_t value = (!!swap) << 19;
	uint32_t mask = 1 << 19;

	WriteREG32Msk((DISPSYS_BASE + DISP_MAIN_CONFIG), value, mask);
}

void disp_sys_hal_shadow_en(bool en)
{
	DISP_SYS_LOG_I("shadow_en: %d\n", en);

	WriteREG32Msk((DISPSYS_BASE + DISP_SYS_CONFIG_A0), en, DISP_OUT_SHADOW_EN);
}

void disp_sys_hal_shadow_update(void)
{
	DISP_SYS_LOG_D("shadow_update be call.\n");

	WriteREG32Msk((DISPSYS_BASE + DISP_SYS_CONFIG_A0),
		DISP_OUT_SHADOW_UPDATE, DISP_OUT_SHADOW_UPDATE);
	WriteREG32Msk((DISPSYS_BASE + DISP_SYS_CONFIG_A0), 0, DISP_OUT_SHADOW_UPDATE);
}

void disp_sys_clear_irq(enum DISP_SYS_IRQ_BIT irq)
{
	/*set 1 and 0 to clear interrupt*/
	WriteREG32Msk(DISPSYS_BASE + DISP_INT_CLR, irq, irq);
	WriteREG32Msk(DISPSYS_BASE + DISP_INT_CLR, 0, irq);
}

void disp_sys_clear_irq_all(void)
{
	/*set 1 and 0 to clear interrupt*/
	WriteREG32(DISPSYS_BASE + DISP_INT_CLR, 0xffffffff);
	WriteREG32(DISPSYS_BASE + DISP_INT_CLR, 0);
}

void disp_sys_sram_pd(enum DISP_SYS_SRAM_PD pd, bool en)
{
	/*to-do*/
	uintptr_t reg = 0;
	int mask_val = 0;

	switch (pd) {
	case DISP_SYS_SRAM_DRAM2AXI:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_DRAM2AXI;
		break;
	case DISP_SYS_SRAM_SDR2HDR_LUTA:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_SDR_LUTA;
		break;
	case DISP_SYS_SRAM_SDR2HDR_LUTB:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_SDR_LUTB;
		break;
	case DISP_SYS_SRAM_VM:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_VM;
		break;
	case DISP_SYS_SRAM_OSD_FHD:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_OSD_FHD;
		break;
	case DISP_SYS_SRAM_OSD_UHD:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_OSD_UHD;
		break;
	case DISP_SYS_SRAM_OSD_TVE:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW0;
		mask_val = MMSYS_MEM_PD_OSD_TVE;
		break;
	case DISP_SYS_SRAM_VIDEOIN:
		if (en) {
			WriteREG32Msk(MMSYS_BASE + MMSYS_MEM_PD_SW0,
				MMSYS_MEM_PD_VIDEOIN_0, MMSYS_MEM_PD_VIDEOIN_0);
			WriteREG32Msk(MMSYS_BASE + MMSYS_MEM_PD_SW1,
				MMSYS_MEM_PD_VIDEOIN_1, MMSYS_MEM_PD_VIDEOIN_1);
		} else {
			WriteREG32Msk(MMSYS_BASE + MMSYS_MEM_PD_SW0, 0, MMSYS_MEM_PD_VIDEOIN_0);
			WriteREG32Msk(MMSYS_BASE + MMSYS_MEM_PD_SW1, 0, MMSYS_MEM_PD_VIDEOIN_1);
		}
		break;
	case DISP_SYS_SRAM_HDMI:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW1;
		mask_val = MMSYS_MEM_PD_HDMI;
		break;
	case DISP_SYS_SRAM_P2I:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW1;
		mask_val = MMSYS_MEM_PD_P2I;
		break;
	case DISP_SYS_SRAM_DOLBY:
		reg = MMSYS_BASE + MMSYS_MEM_PD_SW1;
		mask_val = MMSYS_MEM_PD_DOLBY0;
		if (en) {
			WriteREG32(MMSYS_BASE + MMSYS_MEM_PD_SW2, 0xffffffff);
			WriteREG32(MMSYS_BASE + MMSYS_MEM_PD_SW3, 0xffffffff);
		} else {
			WriteREG32(MMSYS_BASE + MMSYS_MEM_PD_SW2, 0x0);
			WriteREG32(MMSYS_BASE + MMSYS_MEM_PD_SW3, 0x0);
		}
		break;
	case DISP_SYS_SRAM_MVDO_HDR2SDR_LUTA:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_30;
		mask_val = DISP_SYS_PD_MVDO_HDR_LUTA;
		break;
	case DISP_SYS_SRAM_MVDO_HDR2SDR_LUTB:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_30;
		mask_val = DISP_SYS_PD_MVDO_HDR_LUTB;
		break;
	case DISP_SYS_SRAM_DISPFMT3:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_30;
		mask_val = DISP_SYS_PD_DISPFMT3;
		break;
	case DISP_SYS_SRAM_MVDO_0:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_30;
		mask_val = DISP_SYS_PD_MVDO_0;
		break;
	case DISP_SYS_SRAM_MVDO_1:
		if (en)
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_34, 0xffffffff);
		else
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_34, 0x0);
		break;
	case DISP_SYS_SRAM_MVDO_2:
		if (en)
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_38, 0xffffffff);
		else
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_38, 0x0);
		break;
	case DISP_SYS_SRAM_MVDO_3:
		if (en)
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_3c, 0xffffffff);
		else
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_3c, 0x0);
		break;
	case DISP_SYS_SRAM_SVDO_0:
		if (en)
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_40, 0xffffffff);
		else
			WriteREG32(DISPSYS_BASE + DISP_SYS_CONFIG_40, 0x0);
		break;
	case DISP_SYS_SRAM_SVDO_1:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_44;
		mask_val = DISP_SYS_PD_SVDO_1;
		break;
	case DISP_SYS_SRAM_DISPFMT4:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_44;
		mask_val = DISP_SYS_PD_DISPFMT4;
		break;
	case DISP_SYS_SRAM_SVDO_HDR2SDR_LUTA:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_44;
		mask_val = DISP_SYS_PD_SVDO_HDR_LUTA;
		break;
	case DISP_SYS_SRAM_SVDO_HDR2SDR_LUTB:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_44;
		mask_val = DISP_SYS_PD_SVDO_HDR_LUTB;
		break;
	case DISP_SYS_SRAM_IMGRZ:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_48;
		mask_val = DISP_SYS_PD_IMGRZ;
		break;
	case DISP_SYS_SRAM_WC:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_48;
		mask_val = DISP_SYS_PD_WC;
		break;
	case DISP_SYS_SRAM_NR_DL:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_48;
		mask_val = DISP_SYS_PD_NR_DL;
		break;
	case DISP_SYS_SRAM_NR_COMMON:
		reg = DISPSYS_BASE + DISP_SYS_CONFIG_48;
		mask_val = DISP_SYS_PD_NR_COMMON;
		break;
	case DISP_SYS_SRAM_VDO2:
		if (en) {
			WriteREG32Msk(DISPSYS_BASE + DISP_SYS_CONFIG_48, DISP_SYS_PD_VDO2_0, DISP_SYS_PD_VDO2_0);
			WriteREG32Msk(DISPSYS_BASE + DISP_SYS_CONFIG_4c, DISP_SYS_PD_VDO2_1, DISP_SYS_PD_VDO2_1);
		} else {
			WriteREG32Msk(DISPSYS_BASE + DISP_SYS_CONFIG_48, 0, DISP_SYS_PD_VDO2_0);
			WriteREG32Msk(DISPSYS_BASE + DISP_SYS_CONFIG_4c, 0, DISP_SYS_PD_VDO2_1);
		}
		break;
	default:
		break;
	}

	if (reg && mask_val) {
		if (en)
			WriteREG32Msk(reg, mask_val, mask_val);
		else
			WriteREG32Msk(reg, 0, mask_val);
	}
}

void disp_sys_sram_pd_all(bool en, int except_flags)
{
	int i = 0;
	int pd_bum = 8 * sizeof(unsigned int);

	for (i = 0; i < pd_bum; i++) {
		if (!((0x1 << i) & except_flags))
			disp_sys_sram_pd(0x1 << i, en);
	}
}

int disp_sys_hal_init(struct disp_sys_init_param *param)
{
	if (disp_sys.inited)
		return 0;

	disp_sys.init_param.dispsys_reg_base = param->dispsys_reg_base;
	disp_sys.init_param.mmsys_reg_base = param->mmsys_reg_base;
	DISP_SYS_LOG_D("dispsys reg base: 0x%lx\n", disp_sys.init_param.dispsys_reg_base);
	DISP_SYS_LOG_D("mmsys reg base: 0x%lx\n", disp_sys.init_param.mmsys_reg_base);

	if (!disp_sys.init_param.dispsys_reg_base || !disp_sys.init_param.mmsys_reg_base) {
		DISP_SYS_LOG_I("can not get correct register base.\n");
		return -1;
	}

	disp_sys.inited = 1;

	return 0;
}

int disp_sys_hal_video_preultra_en(enum DISP_TYPE type, bool en)
{
	int reg = 0;

	if (type == DISP_MAIN)
		reg = DISP_MAIN_CONFIG;
	else
		reg = DISP_SUB_CONFIG;

	if (en) {
		WriteREG32Msk((DISPSYS_BASE + reg), DISP_TOP_Y_PRE_ULTRA,
			DISP_TOP_Y_PRE_ULTRA);
		WriteREG32Msk((DISPSYS_BASE + reg), DISP_TOP_C_PRE_ULTRA,
			DISP_TOP_C_PRE_ULTRA);
	}

	return 0;
}

int disp_sys_hal_video_ultra_en(enum DISP_TYPE type, bool en)
{
	int reg = 0;

	if (type == DISP_MAIN)
		reg = DISP_MAIN_CONFIG;
	else
		reg = DISP_SUB_CONFIG;

	if (en) {
		WriteREG32Msk((DISPSYS_BASE + reg), DISP_TOP_Y_ULTRA,
			DISP_TOP_Y_ULTRA);
		WriteREG32Msk((DISPSYS_BASE + reg), DISP_TOP_C_ULTRA,
			DISP_TOP_C_ULTRA);
	}

	return 0;
}

void disp_sys_hal_hdr2sdr_dynamic_clock_ctrl(uint32_t value, bool en)
{
	int reg = DISP_HDR2SDR_DCM_CTRL;

	if (en)
		WriteREG32((DISPSYS_BASE + reg), value);
}

