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

#ifndef __DISP_DOVI_MAIN_H__
#define __DISP_DOVI_MAIN_H__

#define DOVI_DTS_MAX_ND_SIZE 50
#define DOVI_DRV_NAME  "disp_drv_dovi"

enum dovi_core_id {
	DOVI_CORE1 = 0,
	DOVI_CORE2 = 1,
	DOVI_CORE3 = 2,
	DOVI_CORE_MAX = 3,
};

#define CHK_CORE_ID(core_id) \
(((core_id) >= DOVI_CORE1) || ((core_id) <= DOVI_CORE3))

int disp_dovi_resolution_change(const struct disp_hw_resolution *info);

extern int disp_dovi_common_init(void);
extern enum dovi_status dovi_sec_init(void);
extern int32_t vdout_hal_set_path(int32_t dolby_path);
extern int disp_dovi_process_core2(uint32_t enable);
extern int disp_dovi_process(uint32_t enable,
	struct mtk_disp_hdr_md_info_t *hdr_metadata);

extern uint32_t dovi_set_out_res(uint32_t out_res,
	uint16_t width, uint16_t height);

extern int core3_hal_enable;
extern int core3_hal_pre_enable;
extern bool dump_rpu_enable;

extern uint32_t dovi_idk_test;
extern enum VIDEO_BIT_MODE idk_dump_bpp;
extern uint32_t disp_dovi_set_idk_info(void);
extern uint32_t disp_dovi_set_sdk_info(void);
int disp_dovi_sdk_handle(uint32_t enable);
void disp_dovi_set_osd_clk_enable(bool enable);
uint32_t disp_dovi_set_osd_showdoblylogo(void);
void disp_dovi_set_graphic_header(uint32_t *va, dma_addr_t graphic_pa);
extern uint32_t *graphic_idk_load_addr_va;
extern dma_addr_t graphic_idk_dump_load_pa;
extern uint32_t *graphic_header_idk_load_addr_va;
extern dma_addr_t graphic_header_idk_dump_load_pa;

extern uint32_t *p_core1_reg;
extern uint32_t *p_core1_lut;

extern uint32_t *p_core2_reg;
extern uint32_t *p_core2_lut;

extern uint32_t *p_core3_reg;

extern void vDolbyHdrEnable(bool fgEnable);
extern void vHdrEnable(bool fgEnable);
extern void vSetStaticHdrType(char bType);

int disp_dovi_show_res_status(HDMI_VIDEO_RESOLUTION res);
int disp_dovi_show_tv_cap(struct disp_hw_tv_capbility *cap);
void disp_dovi_set_osd_all_black(void);
int disp_dovi_set_mute_unmute(uint32_t enable);
void disp_dovi_set_osd_active_zone(void);
int disp_dovi_set_clk_disable(uint32_t enable);
int disp_dovi_vsync_init(void);
void disp_dovi_set_event(int event);
void disp_dovi_isr(void);

extern unsigned int g_hdmi_res;
extern unsigned int g_force_dolby;
extern unsigned int g_hdr_type;
extern unsigned int g_out_format;

#endif
