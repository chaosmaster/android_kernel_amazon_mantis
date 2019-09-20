#ifndef __HDR_CORE_H__
#define __HDR_CORE_H__
#include <linux/workqueue.h>
#include "disp_hdr_def.h"
#include "disp_vdp_if.h"


struct HDR_BUFFER_INFO {
	bool is_hdr;
	bool is_hdr10plus;
	bool is_dolby;
	bool constant_bt2020;
	COLOR_FORMAT_ENUM color_format;
	GAMMA_TYPE_ENUM gamma_type;
	/* disp hardware resolution information*/
	struct disp_hw_resolution resolution;
};

enum HDR_CLOCK_MODULE_ENUM {
	HDR_CLOCK_MODULE_MAIN_BT2020 = 0,
	HDR_CLOCK_MODULE_MAIN_HDR2SDR = 1,
	HDR_CLOCK_MODULE_SUB_BT2020 = 2,
	HDR_CLOCK_MODULE_SUB_HDR2SDR = 3,
	HDR_CLOCK_MODULE_OSD = 4,
	HDR_CLOCK_MODULE_MAX = 5,
};

enum HDR_STATUS hdr_core_config_ext_yuv2rgb_coef(struct yuv2rgb_ext_coef_struct *pYuv2rgbExtCoef,
	const MATRIX_PARAM *pMatrix);
enum HDR_STATUS hdr_core_config_constant_luma_input(struct yuv2rgb_config_struct *pConfig);
enum HDR_STATUS hdr_core_config_yuv2rgb(const struct data_type_struct *pInputType,
	struct yuv2rgb_config_struct *pConfig);
enum HDR_STATUS hdr_core_config_degamma(const struct data_type_struct *pInputType,
	struct degamma_config_struct *pConfig);
enum HDR_STATUS hdr_core_config_gamma(const struct data_type_struct *pOutputType,
	struct gamma_config_struct *pConfig);

enum HDR_STATUS hdr_core_config_3x3matrix(const struct data_type_struct *pInputType,
	const struct data_type_struct *pOutputType,
	struct matrix_config_struct *pConfig);

enum HDR_STATUS hdr_core_config_rgb2yuv(const struct data_type_struct *pOutputType,
	struct rgb2yuv_config_struct *pConfig);

enum HDR_STATUS hdr_core_config_lut(int plane,
	struct lut_config_struct *pLutConfig,
	pWriteLutFunc pCallBackFunc);


enum HDR_STATUS hdr_core_write_yuv2rgb_ext_register(int plane,
	const struct yuv2rgb_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc);

enum HDR_STATUS hdr_core_write_matrix_ext_register(int plane,
	const struct matrix_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc);

enum HDR_STATUS hdr_core_write_gamma4096_register(int plane,
	const struct gamma_max_4096_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc);

enum HDR_STATUS hdr_core_write_rgb2yuv_ext_register(int plane,
	const struct rgb2yuv_config_struct *pConfig,
	pWriteRegisterFunc pCallBackFunc);

void hdr_core_enable_dynamic_clock_on(HDMI_VIDEO_RESOLUTION res_mode, enum HDR_PATH_ENUM path);

void hdr_core_adjust_osd_delay(HDMI_VIDEO_RESOLUTION res_mode, enum HDR_PATH_ENUM path);

bool hdr_core_need_through_hdr2sdr_bt2020_sdr2hdr(enum HDR_PATH_ENUM path,
	struct HDR_BUFFER_INFO bufferInfo, struct disp_hw_tv_capbility tv);

bool hdr_core_had_through_hdr2sdr_bt2020_sdr2hdr(enum HDR_PATH_ENUM path);

int hdr_core_handle_tv_display_source(enum HDR10_TYPE_ENUM TVTiming);

int hdr_core_handle_disp_stop(unsigned int layer_id);

int hdr_core_handle_disp_suspend(void);

int hdr_core_handle_disp_resume(void);

enum HDR_STATUS disp_path_set_delay_back_for_hdr2sdr(void);

enum HDR_STATUS sdr2hdr_boot_kenel_init_stage_special_prcess(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig);
enum HDR_STATUS sdr2hdr_bt2020_for_sub_path_video(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig);
enum HDR_STATUS hdr2sdr_bt2020_for_dolby_pip(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig);

enum HDR_STATUS hdr2sdr_bt2020_sub_path_clock_path_ctrl(enum HDR_PATH_ENUM path,
	struct config_info_struct *pConfig);
#endif /* endof __HDR_CORE_H__ */
