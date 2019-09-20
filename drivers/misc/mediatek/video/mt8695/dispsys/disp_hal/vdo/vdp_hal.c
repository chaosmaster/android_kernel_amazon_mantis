/*
 * Copyright (C) 2016 MediaTek Inc.
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


#define LOG_TAG "VDP_HAL"
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>

#include "vdp_hw.h"
#include "vdp_hal.h"
#include "disp_vdp_if.h"
/* #include "mtk_disp_mgr.h" */
/* #include "disp_hw_mgr.h" */
#include "disp_hw_log.h"
#include "disp_reg.h"
#include "disp_vdp_vsync.h"

uint8_t vdo3_path_pre = 1;
uint8_t vdo4_path_pre;

uint8_t vdo_disp_chg[VDP_MAX] = {0, 0};
char *disp_vdo_reg_base[VDO_LAYER_COUNT];

struct vdp_hal_data_info vdp_hal_data[VDP_MAX];
struct vdp_hal_disp_info vdp_disp_data[VDP_MAX];

/* record HDMI resolution info */
struct disp_hw_resolution vdp_disp_res;

/* record HDMI full screen active zone.  */
uint32_t h_start[VDP_MAX];
uint32_t h_end[VDP_MAX];
uint32_t v_odd_start[VDP_MAX];
uint32_t v_even_start[VDP_MAX];


/*sw shadow*/

struct vdo_sw_shadow vdo_sw_reg[VDP_MAX];
struct vdo_hw_register vdo_hw_reg[VDP_MAX];

#if 0

union vdo_hal_union vdp3_sw_reg;
union vdo_hal_union vdp4_sw_reg;

union vdo_hal_scl_union vdo3_scl_sw_reg;
union vdo_hal_scl_union vdo4_scl_sw_reg;

union vdo_hal_ufo_union vdo3_ufo_sw_reg;
union vdo_hal_ufo_union vdo4_ufo_sw_reg;

/*hw register*/
union vdo_hal_union *p_vdo3_hw_reg;
union vdo_hal_union *p_vdo4_hw_reg;

union vdo_hal_ufo_union *p_vdo3_ufo_hw_reg;
union vdo_hal_ufo_union *p_vdo4_ufo_hw_reg;

union vdo_hal_scl_union *p_vdo3_scl_hw_reg;
union vdo_hal_scl_union *p_vdo4_scl_hw_reg;


uint64_t vdp3_reg_mode;
uint64_t vdp4_reg_mode;

uint64_t vdo3_scl_reg_mode;
uint64_t vdo4_scl_reg_mode;

uint64_t vdo3_ufo_reg_mode;
uint64_t vdo4_ufo_reg_mode;
#endif

int vdp_hal_chk_hw_id(unsigned char vdp_id)
{
	if ((vdp_id) >= VDP_MAX) {
		DISP_LOG_E("[VDP%d] hw id error in func %s\n", vdp_id, __func__);
		return -1;
	} else
		return 0;
}


void vdp_hal_reset(unsigned char vdp_id)
{
	struct vdp_hal_data_info *p_data;
	struct vdp_hal_color_format *p_clr_fmt;

	p_data = &vdp_hal_data[vdp_id];
	p_clr_fmt = &p_data->color_fmt;
	p_clr_fmt->is_scan_line = 0xff;
	p_clr_fmt->is_yuv422 = 0xff;
	p_clr_fmt->is_10bit = 0xff;
	p_clr_fmt->is_ufo = 0xff;
	p_clr_fmt->is_10bit_tile_mode = 0xff;
}

void vdp1_hal_underrun_isr(uint32_t vector)
{
}

void vdp2_hal_underrun_isr(uint32_t vector)
{
}

void vdp_init_underrun(unsigned char vdp_id)
{
}

/* 1 power on , 0 power down*/
void vdp_hal_power_down_vdo(unsigned char hw_id, bool power_on)
{
}

void vdp_hal_init(unsigned char vdp_id)
{
	/*Init register base */
	/*VDO1~VDO4 */
	if (vdp_id >= VDP_MAX) {
		DISP_LOG_E("invalid vdp id %d\n", vdp_id);
		return;
	}

	if (vdp_id == 0) {
		vdo_hw_reg[0].vdo_reg =
			(union vdo_hal_union *)(disp_vdo_reg_base[0] +
				HAL_VDO3_OFST);
		vdo_hw_reg[0].vdo_ufo_reg =
			(union vdo_hal_ufo_union *)(disp_vdo_reg_base[0] +
				HAL_VDO3_UFO_OFST);
		vdo_hw_reg[0].vdo_scl_reg =
			(union vdo_hal_scl_union *)(disp_vdo_reg_base[0] +
				HAL_VDO3_SCL_OFST);
	} else if (vdp_id == 1) {
		vdo_hw_reg[1].vdo_reg =
			(union vdo_hal_union *)(disp_vdo_reg_base[1] +
				HAL_VDO4_OFST);
		vdo_hw_reg[1].vdo_ufo_reg =
			(union vdo_hal_ufo_union *)(disp_vdo_reg_base[1] +
				HAL_VDO4_UFO_OFST);
		vdo_hw_reg[1].vdo_scl_reg =
			(union vdo_hal_scl_union *)(disp_vdo_reg_base[1] +
				HAL_VDO4_SCL_OFST);
	}

	/*one-time configuration */
	vdp_hal_reset(vdp_id);

	vdp_init_underrun(vdp_id);
}



void vdp_hal_update_luma_info(unsigned char vdp_id)
{
}

void vdp_hal_get_vsync_status(unsigned char vdp_id)
{
}

void vdp_hal_reset_reg_mode(uint8_t *p_mode, uint32_t size)
{
	int idx = 0;

	for (idx = 0; idx < size; idx++)
		p_mode[idx] = 0;
}

void vdp_hal_trigger(unsigned char vdp_id)
{
#if VIDEO_USE_HW_SHADOW
	if (vdp_id == 0) {
		/*trigger vdo*/
		WriteREG32Msk(disp_vdo_reg_base[0] + HAL_VDO3_OFST + 0x14,
			(1 << 31), (1 << 31));
		/*trigger ufo*/
		WriteREG32Msk(disp_vdo_reg_base[0] + HAL_VDO3_UFO_OFST + 0x40,
			(1 << 2), (1 << 2));
	} else if (vdp_id == 1) {
		/*trigger vdo*/
		WriteREG32Msk(disp_vdo_reg_base[1] + HAL_VDO3_OFST + 0x14,
			(1 << 31), (1 << 31));
		/*trigger ufo*/
		WriteREG32Msk(disp_vdo_reg_base[1] + HAL_VDO3_UFO_OFST + 0x40,
			(1 << 2), (1 << 2));
	}
#endif
}

void vdp_hal_isr(bool update_main, bool update_sub)
{
	int idx = 0;

	if (update_main) {
	/* update vdo3 hw reg */
	for (idx = 0; idx < HAL_VDO_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[0].vdo_reg_mode, REG_MASK(idx))) {
			vdo_hw_reg[0].vdo_reg->reg[idx] =
				vdo_sw_reg[0].vdo_reg.reg[idx];
		}
	}
	vdo_sw_reg[0].vdo_reg_mode = 0;

	/* update vdo3 ufo hw reg */
	for (idx = 0; idx < HAL_VDO_UFOD_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[0].vdo_ufo_reg_mode, REG_MASK(idx)))
			vdo_hw_reg[0].vdo_ufo_reg->reg[idx] =
				vdo_sw_reg[0].vdo_ufo_reg.reg[idx];
	}
	vdo_sw_reg[0].vdo_ufo_reg_mode = 0;

	/* update vdo3 scl hw reg */
	for (idx = 0; idx < HAL_VDO_SCL_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[0].vdo_scl_reg_mode, REG_MASK(idx)))
			vdo_hw_reg[0].vdo_scl_reg->reg[idx] =
				vdo_sw_reg[0].vdo_scl_reg.reg[idx];
	}
	vdo_sw_reg[0].vdo_scl_reg_mode = 0;
	vdp_hal_trigger(0);
	}

	if (update_sub) {
	/* update vdo4 hw reg */
	for (idx = 0; idx < HAL_VDO_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[1].vdo_reg_mode, REG_MASK(idx)))
			vdo_hw_reg[1].vdo_reg->reg[idx] =
				vdo_sw_reg[1].vdo_reg.reg[idx];
	}
	vdo_sw_reg[1].vdo_reg_mode = 0;



	/* update vdo4 ufo hw reg */

	for (idx = 0; idx < HAL_VDO_UFOD_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[1].vdo_ufo_reg_mode, REG_MASK(idx)))
			vdo_hw_reg[1].vdo_ufo_reg->reg[idx] =
				vdo_sw_reg[1].vdo_ufo_reg.reg[idx];
	}
	vdo_sw_reg[1].vdo_ufo_reg_mode = 0;



	/* update vdo4 scl hw reg */
	for (idx = 0; idx < HAL_VDO_SCL_REG_NUM; idx++) {
		if (IS_REG_SET(vdo_sw_reg[1].vdo_scl_reg_mode, REG_MASK(idx)))
			vdo_hw_reg[1].vdo_scl_reg->reg[idx] =
				vdo_sw_reg[1].vdo_scl_reg.reg[idx];
	}
	vdo_sw_reg[1].vdo_scl_reg_mode = 0;
	vdp_hal_trigger(1);
	}

}

void vdp_hal_set_enable(uint8_t vdp_id, uint8_t enable)
{
	union vdo_hal_union *p_vdp_sw_reg;
	uint64_t *p_vdp_reg_mode;

	union vdo_hal_ufo_union *p_ufo_sw_reg;
	uint64_t *p_ufo_reg_mode;

	GET_VDP_PTR(vdp_id, p_vdp_sw_reg, p_vdp_reg_mode);
	p_vdp_sw_reg->field.VDOEN = enable;
	#if VIDEO_USE_HW_SHADOW
	p_vdp_sw_reg->field.SHADOW_MODE = 1;
	REG_SET(*p_vdp_reg_mode, REG_MASK(0x1c / 4) | REG_MASK(0x14 / 4));
	#else
	REG_SET(*p_vdp_reg_mode, REG_MASK(0x1c / 4));
	#endif



	GET_UFO_PTR(vdp_id, p_ufo_sw_reg, p_ufo_reg_mode);
	p_ufo_sw_reg->field.UFOD_ENABLE = enable;
	#if !VIDEO_USE_HW_SHADOW
	p_ufo_sw_reg->field.SIMULATION_MODE = 1;
	#endif
	p_ufo_sw_reg->field.HD_REG_TH_H = 5;
	if (vdp_id == 0) {
		/*main video*/
		p_ufo_sw_reg->field.ULTRA_TH = 0x1F;
		p_ufo_sw_reg->field.ULTRA_EN = 1;
	} else {
		/*sub video*/
		p_ufo_sw_reg->field.PRE_ULTRA_TH = 0x1F;
		p_ufo_sw_reg->field.PRE_ULTRA_EN = 1;
	}


	p_vdp_sw_reg->field.CLR = 1;

	REG_SET(*p_ufo_reg_mode, REG_MASK(0x40 / 4) |
		REG_MASK(0x1c / 4) | REG_MASK(0x20 / 4));


#if 0				/* CONFIG_DRV_SUPPORT_DOVI */
	/* set disp_fmt out 422 mode */
	if (dovi_core1_is_support())
		PMX_HalSetOutput444Enable(ucVdpId, !_bVideoDobly);

	/* set disp_fmt fgPFOFF */
#endif

	/* set disp_fmt fgVDO_EN = enable */
}

/* set display buffer width & height in UFOD & VDO. */
void vdp_hal_set_src_size(uint8_t vdp_id, uint32_t width, uint32_t height)
{
	uint32_t h_block;
	union vdo_hal_union *p_vdp_sw_reg;
	union vdo_hal_ufo_union *p_ufo_sw_reg;
	union vdo_hal_scl_union *p_vdp_scl_sw_reg;

	uint64_t *p_vdp_reg_mode;
	uint64_t *p_ufo_reg_mode;
	uint64_t *p_vdp_scl_reg_mode;

	GET_VDP_PTR(vdp_id, p_vdp_sw_reg, p_vdp_reg_mode);

	h_block = ((width + 7) / 8);

	p_vdp_sw_reg->field.HBLOCK = h_block & 0x000000ff;
	p_vdp_sw_reg->field.HBLOCK_EXT = (h_block & 0x000003ff) >> 8;

	p_vdp_sw_reg->field.PIC_HEIGHT = height;
	REG_SET(*p_vdp_reg_mode, REG_MASK(0x10 / 4));


	GET_UFO_PTR(vdp_id, p_ufo_sw_reg, p_ufo_reg_mode);
	GET_VDP_SCL_PTR(vdp_id, p_vdp_scl_sw_reg, p_vdp_scl_reg_mode);

	p_ufo_sw_reg->field.LINE_PITCH = (width + 15) / 16;	/* pitch */

	p_ufo_sw_reg->field.H_WIDTH = (((width >= 3840) ? 4096 : width) + 1) / 2;	/* picw */

	p_ufo_sw_reg->field.V_HEIGTH = ((height + 3) / 4) * 4;	/* picHigh 4 align */

	if (width > 1024)
		p_ufo_sw_reg->field.UHD4K_MODE_ENABLE = 1;
	else
		p_ufo_sw_reg->field.UHD4K_MODE_ENABLE = 0;

	p_vdp_scl_sw_reg->field.BUFFER_SP_USE_REG_OFFSET = 1;
	p_vdp_scl_sw_reg->field.LINE_BUF_LEFT_RIGHT_START = 0x40;

	REG_SET(*p_ufo_reg_mode, REG_MASK(0x08 / 4)
	| REG_MASK(0x0c / 4)
	| REG_MASK(0x18 / 4));

	REG_SET(*p_vdp_scl_reg_mode, REG_MASK(0xA0 / 4));
}

/* set UFOD color format */
void vdp_hal_set_src_color_format(uint8_t vdp_id, struct vdp_hal_color_format *p_clr_fmt)
{
	union vdo_hal_ufo_union *p_ufo_sw_reg;
	uint64_t *p_ufo_reg_mode;
	uint8_t ufo_add_mode = 0;
	uint8_t ufo_pack_mode = 0x11; /* 8bit unpack */
	uint8_t tile_mode = 0;

	GET_UFO_PTR(vdp_id, p_ufo_sw_reg, p_ufo_reg_mode);
	p_ufo_sw_reg->field.YUV422_MODE = p_clr_fmt->is_yuv422;
	if (p_clr_fmt->is_scan_line)
		ufo_add_mode = 2; /* 2 raster mode */

	p_ufo_sw_reg->field.ADD_MODE = ufo_add_mode;
	if (p_clr_fmt->is_10bit) {
		/* 10bit mode */
		if (p_clr_fmt->is_ufo) {
			if (p_clr_fmt->is_jumpmode)
				ufo_pack_mode = VDP_10BIT_UFO_JUMP;
			else
				ufo_pack_mode = VDP_10BIT_UFO;
		}
		else
			ufo_pack_mode = VDP_10BIT_UNPACK;
	} else {
			/* 8bit mode */
			if (p_clr_fmt->is_ufo)
				ufo_pack_mode = VDP_8BIT_UFO;
			else
				ufo_pack_mode = VDP_8BIT_UNPACK;
	}

	if (!p_clr_fmt->is_scan_line && p_clr_fmt->is_10bit)
		tile_mode = 1;

	p_ufo_sw_reg->field.TILE_MODE = tile_mode;
	p_ufo_sw_reg->field.DATA_PACK_MODE = ufo_pack_mode;
	if (p_clr_fmt->is_jumpmode)
		p_ufo_sw_reg->field.UFO_DRAM_NEW_MODE = 1;
	else
		p_ufo_sw_reg->field.UFO_DRAM_NEW_MODE = 0;
	REG_SET(*p_ufo_reg_mode, REG_MASK(0x18 / 4) |
		REG_MASK(0x1C / 4));
}


void vdp_hal_set_disp_config(uint8_t vdp_id,
	struct vdp_hal_region *p_src,
	struct vdp_hal_region *p_out)
{
	union vdo_hal_union *p_vdp_sw_reg;
	struct vdp_hal_disp_info *p_disp;
	uint64_t *p_vdp_reg_mode;
	uint32_t dw_need;
	uint32_t scl_factor;
	uint32_t h_scl_factor = VDP_H_FACTOR;
	uint8_t h_down_scl_on = 0;

	p_disp = &vdp_disp_data[vdp_id];
	GET_VDP_PTR(vdp_id, p_vdp_sw_reg, p_vdp_reg_mode);
	scl_factor = VDP_VERTICAL_SCALING_FACTOR(vdp_id);
	p_vdp_sw_reg->field.P_SKIP = (p_src->x / 2) * 2;
	p_vdp_sw_reg->field.VSCALE = ((p_src->height*scl_factor)/p_out->height);
	if ((vdp_id == 1) && video_layer[vdp_id].sdr2hdr)
		p_vdp_sw_reg->field.YFIR_ON = 0x0;
	else
		p_vdp_sw_reg->field.YFIR_ON = 0x1;
	dw_need = (p_src->width + p_src->x) >> 2;
	p_vdp_sw_reg->field.DW_NEED = (dw_need & 0xff);
	p_vdp_sw_reg->field.DW_NEED_HD = (dw_need & 0x1ff);
	p_vdp_sw_reg->field.DW_NEED_BIT10 = ((dw_need & 0x400) >> 10);
	p_vdp_sw_reg->field.DW_NEED_BIT9 = ((dw_need & 0x200) >> 9);
	p_vdp_sw_reg->field.YSLTT = p_src->y;


	/* for dolby idk test chroma repeat */
	if (p_src->y == 0) {
		p_vdp_sw_reg->field.CSLTT = 0x1; /* 0x24[11:0] */
		p_vdp_sw_reg->field.CSSLTT = 0x80; /* 0x2C[7:0] */
		p_vdp_sw_reg->field.Video_Opt8 = 0x1; /* 0x78[8] */
	} else {
		p_vdp_sw_reg->field.CSLTT = 0x0;
		p_vdp_sw_reg->field.CSSLTT = 0x0;
		p_vdp_sw_reg->field.Video_Opt8 = 0x0;
	}

	REG_SET(*p_vdp_reg_mode,
	REG_MASK(0x10 / 4)
	| REG_MASK(0x14 / 4)
	| REG_MASK(0x20 / 4)
	| REG_MASK(0xE0 / 4)
	| REG_MASK(0x24 / 4)
	| REG_MASK(0x2C / 4)
	| REG_MASK(0x78 / 4)
	| REG_MASK(0x7C / 4));

	p_disp->pxl_len = p_src->width;
	if (p_src->width > p_out->width)
		h_down_scl_on = 1;
	p_disp->h_start = h_start[vdp_id] + p_out->x; /* 0x420A0 */
	p_disp->h_end = p_disp->h_start + p_out->width - 1;   /* 0x420A0 */
	p_disp->v_odd_start = v_odd_start[vdp_id] + p_out->y; /* 0x420A4 */
	p_disp->v_odd_end = p_disp->v_odd_start + p_out->height - 1;   /* 0x420A4 */
	p_disp->v_even_start = v_even_start[vdp_id] + p_out->y; /* 0x420A8 */
	p_disp->v_even_end = p_disp->v_even_start + p_out->height - 1;   /* 0x420A8 */
	if (h_down_scl_on == 0)
		p_disp->h_factor =
		((p_src->width*h_scl_factor)/p_out->width); /* 0x420B0 */
	p_disp->h_scl_on = 1; /* 0x420B0 */
	p_disp->h_scl_linear = 1; /* 0x420B0 */

	if (h_down_scl_on) {
		p_disp->h_start = h_start[vdp_id];/* 0x420A0 */
		p_disp->h_end = p_disp->h_start + p_src->width;/* 0x420A0 */
	}
	vdo_disp_chg[vdp_id] = 1;
}

void vdp_hal_set_dsd_config(uint8_t vdp_id,
	uint16_t src_height,
	uint16_t dst_height,
	uint16_t res_height)
{
	uint32_t scl_factor;
	union vdo_hal_union *p_vdp_sw_reg;
	uint64_t *p_vdp_reg_mode;

	scl_factor = VDP_VERTICAL_SCALING_FACTOR(vdp_id);
	GET_VDP_PTR(vdp_id, p_vdp_sw_reg, p_vdp_reg_mode);
	if ((res_height == 480) && (src_height > 1080))	/*4k to 480p dsd*/
		p_vdp_sw_reg->field.VSCALE = ((src_height*scl_factor)/(dst_height * 4));
	else
		p_vdp_sw_reg->field.VSCALE = ((src_height*scl_factor)/(dst_height * 2));
	REG_SET(*p_vdp_reg_mode, REG_MASK(0x14 / 4));
}

void vdp_hal_set_fb_addr(uint8_t vdp_id, uint32_t *p_addr_y, uint32_t *p_addr_c)
{
}

void vdp_hal_set_ufo_addr(uint8_t vdp_id, struct vdp_hal_fb_addr *p_fb_addr)
{
	union vdo_hal_ufo_union *p_ufo_sw_reg;
	uint64_t *p_ufo_reg_mode;

	GET_UFO_PTR(vdp_id, p_ufo_sw_reg, p_ufo_reg_mode);
	p_ufo_sw_reg->field.PTR_TO_Y = p_fb_addr->addr_y;
	p_ufo_sw_reg->field.PTR_TO_C = p_fb_addr->addr_c;
	p_ufo_sw_reg->field.PTR_TO_Y_LENGTH = p_fb_addr->addr_y_len;
	p_ufo_sw_reg->field.PTR_TO_C_LENGTH = p_fb_addr->addr_c_len;
	REG_SET(*p_ufo_reg_mode, REG_MASK(0x00 / 4)
	| REG_MASK(0x04 / 4)
	| REG_MASK(0x10 / 4)
	| REG_MASK(0x14 / 4));
}
/* p_src == p_dst, return 1, else return 0 */
uint8_t vdp_hal_cmp_region(struct vdp_hal_region *p_src,
	struct vdp_hal_region *p_dst)
{
	uint8_t ret = 1;

	if (p_src->x != p_dst->x
	    || p_src->y != p_dst->y
	    || p_src->width != p_dst->width
	    || p_src->height != p_dst->height) {
		ret = 0;
	}
	return ret;
}
uint8_t vdp_hal_cmp_fb_addr(struct vdp_hal_fb_addr *p_src,
	struct vdp_hal_fb_addr *p_dst)
{
	uint8_t ret = 1;

	if (p_src->addr_y != p_dst->addr_y
		|| p_src->addr_c != p_dst->addr_c
		|| p_src->addr_y_len != p_dst->addr_y_len
		|| p_src->addr_c_len != p_dst->addr_c_len) {
		ret = 0;
		*p_src = *p_dst;
	}

	return ret;
}


/*vdp config buffer info :
 *for normal playback, only use buf1
 *for interlace playback, should use buf1, buf2, buf3
*/
void vdp_hal_config(unsigned char vdp_id, struct vdp_hal_config_info *p_config)
{
	struct vdp_hal_data_info *p_data;
	struct vdp_hal_color_format *p_clr_fmt;
	struct vdp_hal_fb_info *p_cur_fb;
	uint8_t vdp_idx = 1;
	int ret = 0;

	ret = vdp_hal_chk_hw_id(vdp_id);

	if (ret < 0)
		return;

	p_data = &vdp_hal_data[vdp_id];
	if (vdp_id == VDP_1 && p_config->is_el_exist) {
		p_data->is_el_exist = 1;
		vdp_idx = 2;
	}

	p_clr_fmt = &p_data->color_fmt;
	if (p_data->is_el_exist)
		p_cur_fb = &p_config->el_cur_fb_info;
	else
		p_cur_fb = &p_config->cur_fb_info;

	/* set hw id */
	if (p_data->vdp_id != vdp_id)
		p_data->vdp_id = vdp_id;

	/* set enable */
	if (p_data->enable != p_config->enable) {
		vdp_hal_set_enable(vdp_id, p_config->enable);

		p_data->enable = p_config->enable;
		vdo_disp_chg[vdp_id] = 1;
	}

	/* set color format */
	if (p_clr_fmt->is_scan_line != p_cur_fb->is_scan_line
	    || p_clr_fmt->is_yuv422 != p_cur_fb->is_yuv422
	    || p_clr_fmt->is_10bit != p_cur_fb->is_10bit
	    || p_clr_fmt->is_ufo != p_cur_fb->is_ufo
	    || p_clr_fmt->is_10bit_tile_mode != p_cur_fb->is_10bit_tile_mode
	    || p_clr_fmt->is_jumpmode != p_cur_fb->is_jumpmode) {
		p_clr_fmt->is_scan_line = p_cur_fb->is_scan_line;
		p_clr_fmt->is_yuv422 = p_cur_fb->is_yuv422;
		p_clr_fmt->is_10bit = p_cur_fb->is_10bit;
		p_clr_fmt->is_ufo = p_cur_fb->is_ufo;
		p_clr_fmt->is_10bit_tile_mode = p_cur_fb->is_10bit_tile_mode;
		p_clr_fmt->is_jumpmode = p_cur_fb->is_jumpmode;
		vdp_hal_set_src_color_format(vdp_id, p_clr_fmt);
	}

	/* set src size */
	if (p_data->fb_size.buff_w != p_cur_fb->fb_size.buff_w
	    || p_data->fb_size.pic_h != p_cur_fb->fb_size.pic_h) {
		p_data->fb_size.buff_w = p_cur_fb->fb_size.buff_w;
		p_data->fb_size.pic_h = p_cur_fb->fb_size.pic_h;
		vdp_hal_set_src_size(vdp_id, p_data->fb_size.buff_w,
			(p_data->fb_size.pic_h + p_cur_fb->src_region.y));
	}

	/* set out region */
	if (!vdp_hal_cmp_region(&p_data->src_region, &p_cur_fb->src_region)
		|| !vdp_hal_cmp_region(&p_data->out_region,
			&p_cur_fb->out_region)) {
		p_data->src_region = p_cur_fb->src_region;
		p_data->out_region = p_cur_fb->out_region;
		if (p_cur_fb->out_region.width != 0 && p_cur_fb->out_region.height != 0)
			vdp_hal_set_disp_config(vdp_id, &p_data->src_region,
				&p_data->out_region);
	}

	/* set fb addr */
	if (!vdp_hal_cmp_fb_addr(&p_data->fb_addr, &p_cur_fb->fb_addr)) {
		vdp_hal_set_ufo_addr(vdp_id, &p_data->fb_addr);
	}
}

void vdp_hal_config_timing(unsigned char vdp_id,
	const struct disp_hw_resolution *res_info,
	int h_sta, int v_sta_odd, int v_sta_even)
{
	int ret = 0;

	ret = vdp_hal_chk_hw_id(vdp_id);
	if (ret < 0)
		return;
	vdp_disp_res = *res_info;

	h_start[vdp_id] = h_sta;
	h_end[vdp_id] = h_start[vdp_id] + vdp_disp_res.width - 1;
	v_odd_start[vdp_id] = v_sta_odd;
	v_even_start[vdp_id] = v_sta_even;

}

void vdp_hal_get_info(unsigned char vdp_id,
	struct vdp_hal_data_info *vdp_hal,
	struct vdp_hal_disp_info *vdp_disp,
	struct vdo_sw_shadow *vdp_sw_shadow)
{
	int ret = 0;

	ret = vdp_hal_chk_hw_id(vdp_id);
	if (ret < 0)
		return;

	memcpy((void *)vdp_hal, (const void *)&vdp_hal_data[vdp_id],
		sizeof(struct vdp_hal_data_info));
	memcpy((void *)vdp_disp, (const void *)&vdp_disp_data[vdp_id],
		sizeof(struct vdp_hal_disp_info));
	memcpy((void *)vdp_sw_shadow, (const void *)&vdo_sw_reg[vdp_id],
		sizeof(struct vdo_sw_shadow));
}

void vdp_hal_set_info(unsigned char vdp_id,
	struct vdp_hal_data_info *vdp_hal,
	struct vdp_hal_disp_info *vdp_disp,
	struct vdo_sw_shadow *vdp_sw_shadow)
{
	int ret = 0;

	ret = vdp_hal_chk_hw_id(vdp_id);
	if (ret < 0)
		return;

	memcpy((void *)&vdp_hal_data[vdp_id], (const void *)vdp_hal,
		sizeof(struct vdp_hal_data_info));
	memcpy((void *)&vdp_disp_data[vdp_id], (const void *)vdp_disp,
		sizeof(struct vdp_hal_disp_info));
	memcpy((void *)&vdo_sw_reg[vdp_id], (const void *)vdp_sw_shadow,
		sizeof(struct vdo_sw_shadow));
}


