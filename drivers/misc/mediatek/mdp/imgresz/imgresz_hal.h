/*
 * Copyright (c) 2016-2017 MediaTek Inc.
 * Author: Yong Wu <yong.wu@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef MTK_IMGRESZ_HAL_H
#define MTK_IMGRESZ_HAL_H

#include "imgresz_priv.h"

#define IMGALIGN(a, b) (((a)+(b)-1)&(~((b)-1)))
#define IMG_VdoPart_GetSrcW(tgW, hfactor)  ((((tgW)+8)*hfactor/0x40000)+32)/*((tgW+8)*hfactor/0x40000)*/
#define IMG_VdoPart_GetSrcPoint(tgPoint, hfactor)   ((tgPoint)*hfactor/0x40000-3)

extern int imgresz_log_level;

void imgresz_hal_HW_init(void __iomem *base);
void imgresz_hal_HW_uninit(void __iomem *base);
void imgresz_hal_clr_irq(void __iomem *base);

void
imgresz_hal_set_resample_method(void __iomem *base,
				enum IMGRESZ_RESAMPLE_METHOD_T h_method,
				enum IMGRESZ_RESAMPLE_METHOD_T v_method);
void imgresz_hal_burst_enable(void __iomem *base, bool wr_enable, bool rd_enable);

void
imgresz_hal_set_src_buf_addr(void __iomem *base, uint32_t y_buf_addr,
			     uint32_t cb_buf_addr, uint32_t cr_buf_addr);
int
imgresz_hal_set_src_buf_format(void __iomem *base,
			       const struct imgresz_buf_format *src_buf_format);

void
imgresz_hal_set_src_buf_pitch(void __iomem *base, uint32_t buf_width,
			      const struct imgresz_buf_format *src_buf_format);

void
imgresz_hal_set_src_rowbuf_height(void __iomem *base, uint32_t row_buf_hei,
				  const struct imgresz_buf_format *src_buf_format,
				  bool rm_rpr_racing_mode);
int imgresz_hal_set_src_pic_wid_hei(void __iomem *base, uint32_t width, uint32_t height,
				    const struct imgresz_buf_format *src_format);
int imgresz_hal_set_src_pic_offset(void __iomem *base,
				   unsigned int x_offset, unsigned int y_offset,
				   const struct imgresz_buf_format *src_format);
void imgresz_hal_set_src_rm_rpr(void __iomem *base, bool rpr_mode, bool rpr_racing);
void imgresz_hal_set_src_first_row(void __iomem *base, bool firstrow);
void imgresz_hal_set_src_last_row(void __iomem *base, bool lastrow);
void imgresz_hal_set_vdo_cbcr_swap(void __iomem *base, bool cbcr_swap);

int
imgresz_hal_set_dst_buf_format(void __iomem *base,
			       const struct imgresz_buf_format *src_format,
			       const struct imgresz_buf_format *dst_format);
void imgresz_hal_set_dst_buf_addr(void __iomem *base, uint32_t addr_y, uint32_t addr_c);
void imgresz_hal_set_dst_buf_pitch(void __iomem *base, uint32_t buf_width);
void imgresz_hal_set_dst_pic_wid_hei(void __iomem *base, uint32_t width, uint32_t height);
void imgresz_hal_set_dst_pic_offset(void __iomem *base,
				    unsigned int x_offset, unsigned int y_offset);

int
imgresz_hal_set_resz_mode(void __iomem *base, enum imgresz_scale_mode reszmode,
			  enum IMGRESZ_BUF_MAIN_FORMAT_T mainformat);

bool imgresz_hal_survey_linebuflen_is_ok(bool ufo, bool onephase, bool dst_blk,
		uint32_t src_width, uint32_t dst_width, unsigned int linebuflen);
int
imgresz_hal_set_linebuflen(void __iomem *base,
			   enum IMGRESZ_RESAMPLE_METHOD_T v_method,
			   uint32_t src_width, uint32_t src_height,
			   uint32_t dst_width, uint32_t dst_height,
			   bool ufo, bool rpr_mode, bool one_phase,
			   bool osd_mode, bool dst_blk, bool dst_10bit);

void imgresz_hal_set_tempbuf(void __iomem *base, uint32_t addr);

void imgresz_hal_jpg_picmode_enable(void __iomem *base);
void imgresz_hal_jpg_preload_enable(void __iomem *base);
void imgresz_hal_jpg_component(void __iomem *base, bool y_exist,
			       bool cb_exist, bool cr_exist);
void imgresz_hal_jpg_component_ex(void __iomem *base, bool y_exist,
				  bool cb_exist, bool cr_exist, bool rpr_racing);
void imgresz_hal_jpg_cbcr_pad(void __iomem *base, bool cb_exist, bool cr_exist);
void imgresz_hal_jpg_preload_buf(void __iomem *base, uint32_t addr_y, uint32_t addr_c);

void imgresz_hal_set_scaling_type(void __iomem *base, unsigned int type);

unsigned int imgresz_hal_coeff_get_h8_y(void __iomem *base);
unsigned int imgresz_hal_coeff_get_h8_cb(void __iomem *base);

int imgresz_hal_coeff_set_h_factor(void __iomem *base,
			       const struct imgresz_src_buf_info *src_buf,
			       const struct imgresz_dst_buf_info *dst_buf,
			       const struct imgresz_buf_format *src_format,
			       enum IMGRESZ_RESAMPLE_METHOD_T resample_method,
			       const struct imgresz_hal_info *hal_info);

int imgresz_hal_coeff_set_v_factor(void __iomem *base,
			       const struct imgresz_src_buf_info *src_buf,
			       const struct imgresz_dst_buf_info *dst_buf,
			       const struct imgresz_buf_format *src_format,
			       enum IMGRESZ_RESAMPLE_METHOD_T resample_method,
			       const struct imgresz_hal_info *hal_info);

int imgresz_hal_coeff_get_v4_factor(void __iomem *base);

void imgresz_hal_coeff_h8tap_vdo_partition_offset(void __iomem *base,
	unsigned int tg_offset, unsigned int hfactor_y, unsigned int hfactor_c);

void
imgresz_hal_coeff_v4tap_vdo_partition_offset(void __iomem *base, unsigned int part1_offset);

void imgresz_hal_set_src_pic_wid_hei_vdo_partition(void __iomem *base,
		unsigned int y_width, unsigned int c_width, unsigned int height);
void imgresz_hal_coeff_set_rpr_H_factor(void __iomem *base, uint32_t src_width,
					uint32_t dst_width);
void imgresz_hal_coeff_set_rpr_V_factor(void __iomem *base, uint32_t src_height,
					uint32_t dst_height);
void imgresz_hal_print_reg(void __iomem *base);
void imgresz_hal_trigger_hw(void __iomem *base, bool ufo);
bool imgresz_hal_is_done(void __iomem *base);
int imgresz_hal_grace_reset(void __iomem *base);
u32 imgresz_checksum_write(void __iomem *base);
u32 imgresz_checksum_read(void __iomem *base);

/* UFO hal interfaces. */
void imgresz_ufo_partition_set_start_point(void __iomem *base, unsigned int x, unsigned int y);
void imgresz_ufo_pagesz(void __iomem *base, uint32_t srcwid, uint32_t srchei, uint32_t dstwid);
void imgresz_ufo_poweron(void __iomem *base);
void imgresz_ufo_config(void __iomem *base, enum IMGRESZ_UFO_TYPE type);
void imgresz_ufo_picsz(void __iomem *base, uint32_t buf_width, uint32_t buf_height);
void imgresz_ufo_src_buf(void __iomem *base, uint32_t y_addr, uint32_t c_addr);
void imgresz_ufo_srclen_buf(void __iomem *base, uint32_t y_len_addr, uint32_t c_len_addr);
int imgresz_ufo_vdo_v_partition_offset_update(
		void __iomem *base, uint32_t src_buf_wid, uint32_t part1_src_bgn);
void imgresz_hal_set_src_offset_vdo_partition(void __iomem *base,
		unsigned int y_h_offset, unsigned int y_v_offset, unsigned int c_h_offset);
void imgresz_debug_vdo_partition_setting(void __iomem *base);
void imgresz_ufo_10bit_output_enable(void __iomem *base);
void imgresz_ufo_10bit_jump_mode_enable(void __iomem *base);
void imgresz_ufo_idle_int_on(void __iomem *base);
void imgresz_ufo_outstanding_enable(void __iomem *base);
#endif
