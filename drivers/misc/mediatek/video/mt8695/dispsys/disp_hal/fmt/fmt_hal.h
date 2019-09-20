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
  * $Author: chuanfei.wang $
  * $Date: 2016/08/10 $
  * $RCSfile: fmt_hal.h,v $
  */


#ifndef _FMT_HAL_H_
#define _FMT_HAL_H_

/* ----------------------------------------------------------------------------- */
/* Include files */
/* ----------------------------------------------------------------------------- */
#include <linux/types.h>

#include "hdmitx.h"
#include "fmt_hw.h"
#include "fmt_def.h"

#define DISPFMT_H_FACTOR				0x1000


struct fmt_init_pararm {
	uintptr_t vdout_fmt_reg_base;
	uintptr_t disp_fmt_reg_base[DISP_FMT_CNT]; /*fmt1, fmt2*/
	uintptr_t io_reg_base;
};

struct fmt_active_info {
	uint32_t h_begine;
	uint32_t h_end;
	uint32_t v_odd_begine;
	uint32_t v_odd_end;
	uint32_t v_even_begine;
	uint32_t v_even_end;
};

struct fmt_delay_info {
	int adj_f;
	int v_delay;
	int h_delay;
	HDMI_VIDEO_RESOLUTION res;
};

struct fmt_hd_scl_info {
	bool hd_scl_on;
	int src_w;
	int out_w;
	int in_x_pos;
	int out_x_offset;
	int out_y_odd_pos;
	int out_y_odd_pos_e;
	int out_y_even_pos;
	int out_y_even_pos_e;
	int dispfmt_out_h_start;
	int dispfmt_out_h_end;
	int vdout_out_h_start;
	int vdout_out_h_end;
	HDMI_VIDEO_RESOLUTION res;
};
enum FMT_PLANE {
	MAIN_VIDEO_PLANE,
	SUB_VIDEO_PLANE,
	OSD_PRE_MIX_PLANE
};

enum DSD_CASE_E {
	DSD_4K_TO_480P,
	DSD_4K_TO_720P,
	DSD_4K_TO_1080P,
	DSD_1080P_TO_480P,
	DSD_1080P_TO_720P,
	DSD_720P_TO_480P,
	DSD_NONE
};


struct fmt_dsd_scl_info {
	bool fg_dsd_scl_on;
	enum DSD_CASE_E dsd_case;
	int src_w;
	int src_h;
	int src_x;
	int src_y;
	int out_w;
	int out_h;
	int out_x;
	int out_y;
	int active_start;
	int active_end;
	int original_active_start;
	int out_h_start; /*return to user*/
	int out_h_end; /*return to user*/
	HDMI_VIDEO_RESOLUTION res;
};



/************************************************************************
*    Function : fmt_hal_enable(uint32_t fmt_id, bool enable)
*    Description : enable vdout/disp fmt
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                enable: true-enable, false-disable
*    Return      : none
************************************************************************/
void fmt_hal_enable(uint32_t fmt_id, bool enable);


/************************************************************************
*    Function : fmt_hal_get_dispfmt_reg(uint32_t fmt_id);
*    Description : get disp fmt parameter
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*    Return      : The pointer of disp fmt
************************************************************************/
struct disp_fmt_t *fmt_hal_get_dispfmt_reg(uint32_t fmt_id);


/************************************************************************
*    Function : fmt_hal_get_vdoutfmt_reg(uint32_t fmt_id);
*    Description : get vdout fmt parameter
*                fmt_id: 2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*    Return      : The pointer of vdout fmt
************************************************************************/
struct vdout_fmt_t *fmt_hal_get_vdoutfmt_reg(uint32_t fmt_id);


/************************************************************************
*    Function : fmt_hal_set_mode(uint32_t fmt_id,
*                    HDMI_VIDEO_RESOLUTION resolution, bool config_hw)
*    Description : set vdout/disp fmt timing
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                resolution: the tv resolution
*                config_hw: if lk is ready , no need to set timing again when normal boot
*    Return      : none
************************************************************************/
void fmt_hal_set_mode(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution, bool config_hw);


/************************************************************************
*    Function : fmt_hal_reset_in_vsync(uint32_t fmt_id)
*    Description : enable the reset flag
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*    Return      : none
************************************************************************/
void fmt_hal_reset_in_vsync(uint32_t fmt_id);


/************************************************************************
*    Function : fmt_hal_hw_shadow_enable(uint32_t fmt_id)
*    Description : enable shadow register
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*    Return      : none
************************************************************************/
void fmt_hal_hw_shadow_enable(uint32_t fmt_id);


/************************************************************************
*    Function : fmt_hal_update_hw_register(void)
*    Description : update vdout/disp fmt register
*    Return      : none
************************************************************************/
void fmt_hal_update_hw_register(void);


/************************************************************************
*    Function : fmt_hal_reset(fmt_id)
*    Description : reset vdout/disp fmt hw
*    Return      : none
************************************************************************/
void fmt_hal_reset(uint32_t fmt_id);


/************************************************************************
*    Function : fmt_hal_set_pllgp_hdmidds(void)
*    Description : set hdmi pll
*    Return      : none
************************************************************************/
void fmt_hal_set_pllgp_hdmidds(uint32_t eRes, bool en, bool set_pll, bool fractional);


/************************************************************************
*    Function : fmt_hal_set_tv_type(uint32_t fmt_id, uint32_t tv_type)
*    Description : set tv type
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                tv_type: the tv type
*    Return      : none
************************************************************************/
void fmt_hal_set_tv_type(uint32_t fmt_id, uint32_t tv_type);


/************************************************************************
*    Function : fmt_hal_set_background(uint32_t fmt_id, uint32_t background)
*    Description : set fmtter background
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                background: the color of background
*    Return      : none
************************************************************************/
void fmt_hal_set_background(uint32_t fmt_id, uint32_t background);


/************************************************************************
*    Function : fmt_hal_not_mix_plane(uint32_t plane)
*    Description : mix off plane
*                plane: 1-plane2(sub video) 2-plane3(osd), main vdo can
*                       not be mixoff
*    Return      : none
************************************************************************/
void fmt_hal_not_mix_plane(uint32_t plane);


/************************************************************************
*    Function : fmt_hal_mix_plane(uint32_t plane)
*    Description : mix plane
*                plane: 1-plane2(sub video) 2-plane3(osd)
*    Return      : none
************************************************************************/
void fmt_hal_mix_plane(uint32_t plane);


/************************************************************************
*    Function : fmt_hal_plane_is_mix(uint32_t plane)
*    Description : if the plane is mix or not
*                plane: 0-plane1(main vdo) 1-plane2(sub video) 2-plane3(osd)
*    Return      : none
************************************************************************/
bool fmt_hal_plane_is_mix(uint32_t plane);


/************************************************************************
*    Function : fmt_hal_disable_video_plane(uint32_t videoId)
*    Description : disable video plane
*                videoId: for mix off sub video
*    Return      : none
************************************************************************/
void fmt_hal_disable_video_plane(uint32_t videoId);


/************************************************************************
*    Function : fmt_hal_set_alpha(uint32_t fmt_id, uint32_t alpha)
*    Description : set plane alpha
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                alpha: the alpha value
*    Return      : none
************************************************************************/
void fmt_hal_set_alpha(uint32_t fmt_id, uint32_t alpha);


/************************************************************************
*    Function : fmt_hal_set_active_zone_by_res(...)
*    Description : set active zone ioformation by resolution
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                resolution: resolution of tv
*                width: h active
*                height: v active
*    Return      : int
************************************************************************/
int32_t fmt_hal_set_active_zone_by_res
		(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution, uint16_t width, uint16_t height);


/************************************************************************
*    Function : fmt_hal_set_active_zone(uint32_t fmt_id, struct fmt_active_info *active_info);
*    Description : set active zone ioformation
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                active_info: active information(h active,v active,htotal, vtotal, etc..)
*    Return      : int
************************************************************************/
int32_t fmt_hal_set_active_zone(uint32_t fmt_id, struct fmt_active_info *active_info);


/************************************************************************
*    Function : fmt_hal_set_delay_by_res(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution);
*    Description : set delay information via resolution
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                resolution: tv resolution
*    Return      : int
************************************************************************/
int32_t fmt_hal_set_delay_by_res(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution);


/************************************************************************
*    Function : fmt_hal_set_delay(...);
*    Description : set delay information
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                adj_f: adjustment forward
*                h_delay: horizontal shift of output vsync
*                v_delay: vertical shift of output hsync
*    Return      : int
************************************************************************/
int32_t fmt_hal_set_delay(uint32_t fmt_id, uint32_t adj_f, uint32_t h_delay, uint32_t v_delay);


/************************************************************************
*    Function : fmt_hal_set_pixel_factor(...);
*    Description : set pixel length and horizontal scaling factor
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                pixel_len: pixel length
*                hsfactor: horizontal scaling factor
*                enable: horizontal scaling enable
*    Return      : int
************************************************************************/
int32_t fmt_hal_set_pixel_factor(uint32_t fmt_id, uint32_t pixel_len, uint32_t hsfactor, uint32_t enable, uint32_t factor_linear_coef);
int32_t fmt_hal_set_h_scale_coef(uint32_t fmt_id, uint32_t enable_16phase);
int32_t fmt_hal_dsd_set_mode(uint32_t fmt_id, enum DSD_CASE_E dsd_case, int h_total, int v_total);
int32_t fmt_hal_dsd_enable(uint32_t fmt_id, struct fmt_dsd_scl_info *info);
int32_t fmt_hal_dsd_set_delay(uint32_t fmt_id, HDMI_VIDEO_RESOLUTION resolution);
int32_t fmt_hal_h_down_enable(uint32_t fmt_id, struct fmt_hd_scl_info *info);
int32_t fmt_hal_h_down_disable(uint32_t fmt_id);
int32_t fmt_hal_h_down_cap(uint32_t fmt_id, uint32_t src_w, uint32_t out_w, uint32_t out_h);
int32_t fmt_hal_set_output_444(uint32_t fmt_id, bool is_444);
int32_t fmt_hal_set_uv_swap(uint32_t fmt_id, bool swap);
int32_t fmt_hal_set_secure(uint32_t fmt_id, bool is_secure);
char *fmt_hal_get_sw_register(uint32_t fmt_id, int *size, uint64_t **reg_mode);
int32_t fmt_hal_show_colorbar(bool en);
int32_t fmt_hal_multiple_alpha(bool en);
/************************************************************************
*    Function : fmt_hal_shadow_update(void);
*    Description : shadow update
*    Return      : none
************************************************************************/
void fmt_hal_shadow_update(void);


/************************************************************************
*    Function : fmt_hal_clock_on_off(uint32_t fmt_id, bool on);
*    Description : clock on/off fmt
*                fmt_id: 0-DISP_FMT_MAIN, 1-DISP_FMT_SUB
*                        2-VDOUT_FMT, 3-VDOUT_FMT_SUB
*                on: enable or disable
*    Return      : int
************************************************************************/
int32_t fmt_hal_clock_on_off(uint32_t fmt_id, bool on);


/************************************************************************
*    Function : fmt_hal_init(struct fmt_init_pararm *param);
*    Description : initialization fmt
*                param: parameter
*    Return      : int
************************************************************************/
int32_t fmt_hal_init(struct fmt_init_pararm *param);


/************************************************************************
*    Function : fmt_hal_deinit(void);
*    Description : de-initialization fmt
*    Return      : int
************************************************************************/
int32_t fmt_hal_deinit(void);


#endif
