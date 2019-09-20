#ifndef __HDR_DEF_H__
#define __HDR_DEF_H__
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/workqueue.h>
#include "disp_hw_mgr.h"

#define MT8695
typedef unsigned int UINT32;

/*if lk using sdr2hdr path, kernel initiation which need to open sdr2hdr clock*/
#define CONFIG_DRV_SDR2HDR_USED_FAST_LOGO 1
/*judge if sdr2hdr had used in fast logo bring up stage*/
#define CONFIG_DRV_SDR2HDR_USE_LINEAR_TABLES 0

enum HDR_PATH_ENUM {
	HDR_PATH_MAIN = 0,
	HDR_PATH_SUB = 1,
	HDR_PATH_OSD = 2,
	HDR_PATH_MAX = 3,
};

enum HDR_PLANE_STATE {
	HDR_PLANE_NOT_USE = 0, /* not use this plane */
	HDR_PLANE_USE_INVALIDATE = 1, /* use this plane, and this plane need to update register */
	HDR_PLANE_USE_VALIDATE = 2, /* use this plane, but don't need to update register except hdr2sdr */
};

/*
** enable / disable yuv2rgb
** bit[4]: CAV_on
** bit[5]: RGB_mode
** bit[6]: c_cf_on
** yuv2rgb enable = CAV_on | (RGB_mode & c_cf_on)
** note: CAV_on must = 1 no matter enable or disable yuv2rgb
*/
typedef enum YUV2RGB_ENABLE_ENUM {
	ENABLE_YUV2RGB = 0x7 << 4,
	DISABLE_YUV2RGB = 0x1 << 4,
} YUV2RGB_ENABLE_ENUM;

/*
** color space convert matrix select
** bit[10:8]
** 3'b000:limit bt601 yuv to limit rgb
** 3'b001:full bt601 yuv to full rgb
** 3'b010:limit bt601 yuv to full rgb
** 3'b011:limit bt2020 yuv to full rgb
** 3'b100:limit bt709 to limit rgb
** 3'b101:full bt709 to full rgb
** 3'b110:limit bt709 to full rgb
** 3'b111:coef from register
*/
typedef enum YUV2RGB_COEF_ENUM {
	LIMIT_BT601_LIMIT_RGB = 0x0 << 8,
	FULL_BT601_FULL_RGB = 0x1 << 8,
	LIMIT_BT601_FULL_RGB = 0x2 << 8,
	LIMIT_BT2020_FULL_RGB = 0x3 << 8,
	LIMIT_BT709_LIMIT_RGB = 0x4 << 8,
	FULL_BT709_FULL_RGB = 0x5 << 8,
	LIMIT_BT709_FULL_RGB = 0x6 << 8,
	YUV2RGB_COEF_FROM_EXT_REGISTER = 0x7 << 8,
} YUV2RGB_COEF_ENUM;

/*
** bit[30]: constant luma bt2020 source.
** bit[30] = 1 constant. special
** bit[30] = 0 non_constant. normal
*/
typedef enum CONSTANT_LUMA_ENUM {
	NON_CONSTANT_LUMA = (0 << 30),
	CONSTANT_LUMA = (1 << 30),
} CONSTANT_LUMA_ENUM;

/*
** convert yuv to rgb: Y channel(x) Cb channel(y) Cr channel(z)
** r = rmx ( x + rax ) + rmy ( y + ray ) + rmz ( z + raz ) + ra
** g = gmx ( x + gax ) + gmy ( y + gay ) + gmz ( z + gaz ) + ga
** b = bmx ( x + bax ) + bmy ( y + bay ) + bmz ( z + baz ) + ba
** s3.13 (16bits) c_cf_rmx ~ c_cf_bmz / c_cf_rmz_c02_sp / c_cf_bmy_c21_sp
** (13bits) c_cf_rax ~ c_cf_ba
*/
struct yuv2rgb_ext_coef_struct {
	int c_cf_rmx;
	int c_cf_rmy;
	int c_cf_rmz;
	int c_cf_gmx;
	int c_cf_gmy;
	int c_cf_gmz;
	int c_cf_bmx;
	int c_cf_bmy;
	int c_cf_bmz;
	int c_cf_rax;
	int c_cf_ray; /* must = 0 */
	int c_cf_raz; /* must = 0 */
	int c_cf_gax;
	int c_cf_gay; /* must = 0 */
	int c_cf_gaz; /* must = 0 */
	int c_cf_bax;
	int c_cf_bay; /* must = 0 */
	int c_cf_baz; /* must = 0 */
	int c_cf_ra;
	int c_cf_ga;
	int c_cf_ba;
	int c_cf_rmz_c02_sp;
	int c_cf_bmy_c21_sp;
};

struct yuv2rgb_config_struct {
	bool update; /* true: need write register, false: don't need to write */
	YUV2RGB_ENABLE_ENUM en_yuv2rgb; /* bit[6:4]:enable yuv2rgb */
	YUV2RGB_COEF_ENUM yuv2rgb_coef; /* bit[10:8]:yuv2rgb color space convert matrix select */
	CONSTANT_LUMA_ENUM constant_luma; /* bit[30]: constant luma bt2020 source */
	struct yuv2rgb_ext_coef_struct yuv2rgb_ext_coef;
};

/*
** enable degamma
** bit[1]: disable main function
** bit[13]: disable degamma
*/
typedef enum DEGAMMA_ENABLE_ENUM {
	ENABLE_DEGAMMA = (0 << 13),
	DISABLE_DEGAMMA = (1 << 13),
} DEGAMMA_ENABLE_ENUM;


/* degamma curve select
** bit 22	bit 12
** 0		0	   use 2.4 anti-gamma
** 0		1	   not defined
** 1		0	   use 709 anti-gamma
** 1		1	   use srgb anti-gamma
** for st2084
**          0      use st2084 anti-gamma curve
**			1	   use hlg anti-gamma curve
*/
typedef enum DEGAMMA_CURVE_SELECT_ENUM {
	DEGAMMA_CURVE_709 = (1 << 22) |
						(0 << 12),
	DEGAMMA_CURVE_SRGB = (1 << 22) |
						(1 << 12),
	DEGAMMA_CURVE_2p4 = (0 << 22) |
						(0 << 12),
	DEGAMMA_CURVE_ST2084 = (0 << 12),
	DEGAMMA_CURVE_HLG = (1 << 12),
} DEGAMMA_CURVE_SELECT_ENUM;


struct degamma_config_struct {
	bool update;
	DEGAMMA_ENABLE_ENUM en_degamma; /* bit{1 13} enable anti-gamma */
	DEGAMMA_CURVE_SELECT_ENUM degamma_curve; /* bit{12 22} select degamma curve */
};

/*
** enable 3x3matrix
** bit[1]: disable main function
** bit[13]: disable 3x3matrix
*/
typedef enum MATRIX_ENABLE_ENUM {
	ENABLE_3X3MATRIX = (0 << 16),
	DISABLE_3X3MATRIX = (1 << 16),
} MATRIX_ENABLE_ENUM;

/*
** 3x3matrix coef
** bit[17] = 0: bt709 -> bt2020
** bit[17] = 1: bt2020 -> bt709
** bit[18]: config from external register.
** bit[18] bit[17]
** 0		0		bt709 -> bt2020
** 0		1		bt2020 -> bt709
** 1		0		config from external register.
** 1		1		config from external register.
*/
typedef enum MATRIX_COEF_ENUM {
	MATRIX_709_2020 = (0 << 18) |
						(0 << 17),
	MATRIX_2020_709 = (0 << 18) |
						(1 << 17),
	MATRIX_COEF_FROM_EXT_REGISTER = (1 << 18),
} MATRIX_COEF_ENUM;

struct matrix_ext_coef_struct {
	int matrix00;
	int matrix01;
	int matrix02;
	int matrix10;
	int matrix11;
	int matrix12;
	int matrix20;
	int matrix21;
	int matrix22;
};

struct matrix_config_struct {
	bool update;
	MATRIX_ENABLE_ENUM en_3x3matrix; /* bit{1 16} enable 3x3matrix */
	MATRIX_COEF_ENUM matrix_coef; /* bit{17 18} bt2020 <-> bt709 convert */
	struct matrix_ext_coef_struct matrix_ext_coef;
};

/*
** enable gamma
** bit[1] disable main function
** bit[20]: disable gamma
*/
typedef enum GAMMA_ENABLE_ENUM {
	ENABLE_GAMMA = (0 << 20),
	DISABLE_GAMMA = (1 << 20),
} GAMMA_ENABLE_ENUM;

/*
** gamma curve select
** bit[21] = 0: gamma 2.4
** bit[21] = 1: gamma 709
*/
typedef enum GAMMA_CURVE_SELECT_ENUM {
	GAMMA_CURVE_709 = (1 << 21),
	GAMMA_CURVE_2p4 = (0 << 21),
} GAMMA_CURVE_SELECT_ENUM;


struct gamma_max_4096_struct {
	bool update;
	int gamma_r_sram_4096;
	int gamma_g_sram_4096;
	int gamma_b_sram_4096;
};

struct gamma_config_struct {
	bool update;
	GAMMA_ENABLE_ENUM en_gamma; /* bit{1 20} enable gamma */
	GAMMA_CURVE_SELECT_ENUM gamma_curve; /* bit[21] gamma curve select */
	struct gamma_max_4096_struct gamma_max_4096;
};

/*
** enable rgb2yuv
** bit[24]: halt_b
** bit[27]: trans_on
** rgb2yuv enable = trans_on & halt_b
** note: halt_b must = 1 no matter enable or disable rgb2yuv
*/
typedef enum RGB2YUV_ENABLE_ENUM {
	ENABLE_RGB2YUV = (1 << 24) |
					(1 << 27),
	DISABLE_RGB2YUV = (1 << 24) |
					(0 << 27),
} RGB2YUV_ENABLE_ENUM;

/*
** bit[29] rgb2yuv coef from external register
** bit{28 25 26} rgb2yuv inner coef
** bt2020_en bit[28] ycc709_en bit[25] xvycc_en  bit[26]
** 000 full rgb to limit 601
** 001 full rgb to full 601
** 010 full rgb to limit 709
** 011 full rgb to full 709
** 100 full rgb to limit 2020
** 101 full rgb to full 2020
** 110 full rgb to limit 2020 (others same as 100)
** 111 full rgb to limit 2020 (others same as 100)
*/
typedef enum RGB2YUV_COEF_ENUM {
	FULL_RGB_LIMIT_BT601 = (0 << 28) | (0 << 25) | (0 << 26),
	FULL_RGB_FULL_BT601 = (0 << 28) | (0 << 25) | (1 << 26),
	FULL_RGB_LIMIT_BT709 = (0 << 28) | (1 << 25) | (0 << 26),
	FULL_RGB_FULL_BT709 = (0 << 28) | (1 << 25) | (1 << 26),
	FULL_RGB_LIMIT_BT2020 = (1 << 28) | (0 << 25) | (0 << 26),
	FULL_RGB_FULL_BT2020 = (1 << 28) | (0 << 25) | (1 << 26),
	RGB2YUV_COEF_FROM_EXT_REGISTER = (1 << 29),
} RGB2YUV_COEF_ENUM;

struct rgb2yuv_ext_coef_struct {
	int c_cf_y_mul_r;
	int c_cf_y_mul_g;
	int c_cf_y_mul_b;
	int c_cf_cb_mul_r;
	int c_cf_cb_mul_g;
	int c_cf_cb_mul_b;
	int c_cf_cr_mul_r;
	int c_cf_cr_mul_g;
	int c_cf_cr_mul_b;
};

struct rgb2yuv_config_struct {
	bool update;
	RGB2YUV_ENABLE_ENUM en_rgb2yuv; /* bit{27 24} enable rgb2yuv */
	RGB2YUV_COEF_ENUM rgb2yuv_coef; /* bit{28 25 26}:rgb2yuv color space convert matrix select */
	struct rgb2yuv_ext_coef_struct rgb2yuv_ext_coef;
};

/*
** main function: top control of degamma/3x3matrix/gamma
** bit[1] disable main function
** bit[1] = 0 enable main function
** bit[1] = 1 disable main function
*/
typedef enum MAIN_FUNC_ENABLE_ENUM {
	ENABLE_MAIN_FUNC = (0 << 1),
	DISABLE_MAIN_FUNC = (1 << 1),
} MAIN_FUNC_ENABLE_ENUM;


struct lut_config_struct {
	bool update; /* lut table has changed, need to rewrite lut table */
	bool en_lut;
	bool use_lut_A;
	bool use_lut_B;
	int phaseSelect; /* update count for sdr2hdr update 8 times, for hdr2sdr only 2 times */
	int oddSelect; /* 0: update even, 1: update odd */
	int rgbSelect; /* 0: update R channel, 1: update G channel, 2: update B channel */
	bool update_lut_A;
	bool update_lut_B;
	bool update_enable; /* only used in hdr2sdr lut config */
};

typedef enum LUT_ACTION_ENUM {
	LUT_ACTION_READ_PHASE_COUNT = 0,
	LUT_ACTION_READ_LUT_TABLE = 1,
	LUT_ACTION_CHANGE_LUT_TABLE = 2,
	LUT_ACTION_WRITE_LUT_CONFIG = 3,
	LUT_ACTION_WRITE_LUT_TABLE = 4,
	LUT_ACTION_READ_RGB_COUNT = 5,
	LUT_ACTION_MAX,
} LUT_ACTION_ENUM;

typedef enum HDR_STATUS (*pWriteLutFunc) (int plane, LUT_ACTION_ENUM action, const void **ppData);


enum HDR_STATUS {
	HDR_STATUS_OK = 0,
	HDR_STATUS_INVALID_PLANE_NUMBER = 1,
	HDR_STATUS_INVALID_PARAM = 2,
	HDR_STATUS_INVALID_INPUT_FORMAT = 3,
	HDR_STATUS_INVALID_GAMMA_TYPE = 4,
	HDR_STATUS_INVALID_OUTPUT_FORMAT = 5,
	HDR_STATUS_INTERNAL_LUT_ERROR = 6,
	HDR_STATUS_NULL_POINTER = 7,
	HDR_STATUS_INVALID_LUT_ACTION = 8,
	HDR_STATUS_GET_HDR_MAIN_DEVICE_TREE_FAIL = 9,
	HDR_STATUS_GET_HDR_SUB_DEVICE_TREE_FAIL = 10,
	HDR_STATUS_GET_HDR_OSD_DEVICE_TREE_FAIL = 11,
	HDR_STATUS_GET_HDR_DEBUG_DEVICE_TREE_FAIL = 12,
	HDR_STATUS_INVALID_PATH_NUMBER = 13,
	HDR_STATUS_CREATE_HDR_SESSION_FAIL = 14,
	HDR_STATUS_CREATE_MEMORY_SESSION_FAIL = 15,
	HDR_STATUS_HDR_SESSION_NOT_CREATED = 16,
	HDR_STATUS_MEMORY_SESSION_NOT_CREATED = 17,
	HDR_STATUS_SERVICE_CALL_FAIL = 18,
	HDR_STATUS_CREATE_SHARE_MEMORY_FAIL = 19,
	HDR_STATUS_SERVICE_CALL_SIZE_TOO_LARGE = 20,
	HDR_STATUS_RELEASE_SHARE_MEMORY_FAIL = 21,
	HDR_STATUS_INVALID_SERVICE_CALL_DIRECTION = 22,
};

typedef struct MATRIX_PARAM {
	int c00;
	int c01;
	int c02;
	int c10;
	int c11;
	int c12;
	int c20;
	int c21;
	int c22;
	int pre_add_0;
	int pre_add_1;
	int pre_add_2;
	int c02_sp; /* for constant luma bt2020 source */
	int c21_sp; /* for constant luma bt2020 source */
} MATRIX_PARAM;


enum WRITE_CMD_ENUM {
	WRITE_CMD_GET_MODULE_NAME = 1, /* get module name */
	WRITE_CMD_WRITE_REGISTER = 2, /* write register */
	WRITE_CMD_WRITE_REGISTER_WITH_MASK = 3, /* write register with mask */
};

#define MAX_MODULE_NAME_LEN 100
struct write_cmd_struct {
	int plane; /* WRITE_CMD_WRITE_REGISTER */
	int offset;
	uint32_t value;
	uint32_t mask; /*value mask, set 1 if need write */
	char module_name[MAX_MODULE_NAME_LEN]; /* WRITE_CMD_GET_MODULE_NAME */
};

typedef enum HDR_STATUS (*pWriteRegisterFunc) (enum WRITE_CMD_ENUM cmd, struct write_cmd_struct *value);



typedef enum COLOR_FORMAT_ENUM {
	COLOR_FORMAT_LIMIT_709	= 0,
	COLOR_FORMAT_LIMIT_601	= 1,
	COLOR_FORMAT_LIMIT_2020	= 2,
	COLOR_FORMAT_FULL_709	= 3,
	COLOR_FORMAT_FULL_601	= 4,
	COLOR_FORMAT_FULL_2020	= 5,
	COLOR_FORMAT_RGB		= 6,
	COLOR_FORMAT_MAX,
} COLOR_FORMAT_ENUM;


/*
**	HDR have 3 types: Normal HDR is HDR10, HDR module support
**	Philips HDR & Dolby HDR is not supported. So HDR type must be 0 or 1
*/
typedef enum HDR_TYPE_ENUM {
	HDR_TYPE_SDR = 0,		/* SDR */
	HDR_TYPE_HDR10 = 1,		/* HDR10 */
	HDR_TYPE_PHILIPS = 2,	/* Philips HDR*/
	HDR_TYPE_DOLBY = 3,		/* Dolby HDR */
	HDR_TYPE_MAX,
} HDR_TYPE_ENUM;

typedef enum GAMMA_TYPE_ENUM {
	GAMMA_TYPE_BYPASS = 0,
	GAMMA_TYPE_ST2084 = 1,
	GAMMA_TYPE_HLG	 = 2,
	GAMMA_TYPE_2p4	 = 3,
	GAMMA_TYPE_709	 = 4,
	GAMMA_TYPE_MAX,
} GAMMA_TYPE_ENUM;

struct data_type_struct {
	HDR_TYPE_ENUM hdrType;
	GAMMA_TYPE_ENUM gammaType;
	COLOR_FORMAT_ENUM colorFormat;
	bool constant_luma_bt2020_input; /* yuv2rgb input source = BT2020 can use. */
	int sdr2hdrGain; /* only for sdr2hdr */
	bool bypassModule; /* bypass HW BT2020/SDR2HDR/HDR2SDR. only use in inputType */
};

/* BT2020 structure
** yuv2rgb -> de-gamma -> 3x3matrix -> gamma -> rgb2yuv
*/
struct bt2020_reg_struct {
	struct data_type_struct inputType; /* pass from disp buffer & TV info */
	struct data_type_struct outputType; /* pass from disp buffer & TV info */

	bool need_update; /* need to set register to hw */
	bool disable_bt2020; /* disable bt2020 */

	bool bypass_bt2020; /* bit[0] bypass bt2020 always false, bypass only use for verification */
	MAIN_FUNC_ENABLE_ENUM en_main_func; /* bit[1]:disable other functions except yuv2rgb & rgb2yuv */

	/* yuv2rgb */
	struct yuv2rgb_config_struct yuv2rgb_config;

	/* degamma */
	struct degamma_config_struct degamma_config;

	/* 3x3matrix */
	struct matrix_config_struct matrix_config;

	/* gamma */
	struct gamma_config_struct gamma_config;

	/* rgb2yuv */
	struct rgb2yuv_config_struct rgb2yuv_config;

	/* path & clock control */
	uint32_t path_config_value;
	uint32_t clock_config_value;

};



/* SDR2HDR structure
** yuv2rgb -> LUT -> rgb2yuv
*/
struct sdr2hdr_reg_struct {
	struct data_type_struct inputType; /* pass from disp buffer & TV info */
	struct data_type_struct outputType; /* pass from disp buffer & TV info */


	bool need_update; /* need to set register to HW */
	bool disable_sdr2hdr; /* disable sdr2hdr */
	bool bypass_sdr2hdr; /* bit[0]:bypass bt2020 always set 0, cause bypass only for verification */
	MAIN_FUNC_ENABLE_ENUM en_main_func; /* bit[1]:disable other functions except yuv2rgb & rgb2yuv */

	/* yuv2rgb */
	struct yuv2rgb_config_struct yuv2rgb_config;

	struct gamma_max_4096_struct gamma_max_4096;
	struct lut_config_struct lut_config;

	/* rgb2yuv */
	struct rgb2yuv_config_struct rgb2yuv_config;

	/* path & clock control */
	uint32_t path_config_value;
	uint32_t clock_config_value;

};

/*
** bit 12: reg_use_hlg, 1: use HLG curve, 0:use ST2084 curve
*/
enum ST2084_CURVE_ENUM {
	ST2084_CURVE_ST2084 = (0 << 12),
	ST2084_CURVE_HLG = (1 << 12),
};

/*
** bit 13: reg_disable_agamma, 1: disable anti-gamma, 0: enable anti-gamma.
*/
enum ST2084_ENABLE_ENUM {
	ENABLE_ST2084 = (0 << 13),
	DISABLE_ST2084 = (1 << 13),
};

struct st2084_config_struct {
	enum ST2084_ENABLE_ENUM en_st2084;
	enum ST2084_CURVE_ENUM st2084_curve;
};

struct nr_config_struct {
	bool en_nr_r;
	bool en_nr_g;
	bool en_nr_b;
	int nr_filter_no_r;
	int nr_filter_no_g;
	int nr_filter_no_b;
	int nr_strength_r;
	int nr_strength_g;
	int nr_strength_b;
};

struct desaturate_config_struct {
	bool en_de_sat;
	int slope0;
	int slope1;
	int slope2;
	int slope3;
	int slope4;
	int line1_start;
	int line2_start;
	int line3_start;
	int line4_start;
};

struct gain_config_struct {
	bool en_xgain; /* gain enable */

	bool en_blend; /* 1: add Y mode into maxRGB with weight, 0: only use maxRGB */
	int mode_width; /* 0 ~ 8: maxRGB weight. (8 - mode_width): Y mode weight */
	int rgb2y_rcoef; /* R weight/1024 for Y mode */
	int rgb2y_gcoef; /* G weight/1024 for Y mode */
	int rgb2y_bcoef; /* B weight/1024 for Y mode */

	bool en_fix_gain; /* use fixed gain, not gainCurve */
	int fix_gain; /* gain value */

	struct lut_config_struct gain_curve_config; /* gainCurve config */
};

struct tone_map_config_struct {
	bool en_tone_map; /* top switch for hist/nr/desatrurate/gain */
	bool en_hist; /* enable histogram */
	struct nr_config_struct nr_config;
	struct desaturate_config_struct desaturate_config;
	struct gain_config_struct gain_config;
};

enum NR_TYPE_ENUM {
	NR_TYPE_NONE = 0,
	NR_TYPE_9TAP = 1,
	NR_TYPE_17TAP = 2,
	NR_TYPE_MAX,
};

struct tone_map_meta_data_struct {
	enum NR_TYPE_ENUM nr_type;
	int nr_strength;
	struct desaturate_config_struct desaturate_config;
	uint32_t gain_curve[256];

	bool en_xgain; /* gain enable */
	bool en_blend; /* 1: add Y mode into maxRGB with weight, 0: only use maxRGB */
	int mode_width; /* 0 ~ 8: maxRGB weight. (8 - mode_width): Y mode weight */

	bool en_fix_gain; /* use fixed gain, not gainCurve */
	int fix_gain; /* gain value */
	bool en_hist; /* enable histogram */
	bool en_lut; /* enable gain curve */
};

/* HDR2SDR structure
** yuv2rgb -> st2084 -> 3x3matrix -> toneMapping -> lut -> gamma -> rgb2yuv
*/
struct hdr2sdr_reg_struct {
	struct data_type_struct inputType; /* pass from disp buffer & TV info */
	struct data_type_struct outputType; /* pass from disp buffer & TV info */


	bool need_update; /* need to set register to HW */
	bool disable_hdr2hdr; /* disable hdr2sdr */
	bool bypass_hdr2sdr; /* bit[0]: bypass hdr2sdr, always set 0, cause bypass only use for verification */
	MAIN_FUNC_ENABLE_ENUM en_main_func; /* bit[1]:disable other functions except yuv2rgb & rgb2yuv */

	struct yuv2rgb_config_struct yuv2rgb_config; /* yuv2rgb config */

	struct degamma_config_struct st2084_config; /* degamma config */

	struct matrix_config_struct matrix_config; /* 3x3 matrix config */

	struct gamma_config_struct gamma_config; /* gamma config */

	struct rgb2yuv_config_struct rgb2yuv_config; /* rgb2yuv config */

	struct tone_map_config_struct toneMap_config; /* tone map config */

	/* path & clock control */
	uint32_t path_config_value;
	uint32_t clock_config_value;

};

struct config_info_struct {
	struct work_struct workItem;
	struct list_head listEntry;

	/* struct data_type_struct inputType; */
	/* struct data_type_struct outputType; */
	enum HDR_PATH_ENUM path;

	/* disp hardware resolution information*/
	struct disp_hw_resolution resolution;

	struct bt2020_reg_struct BT2020Config;
	struct sdr2hdr_reg_struct SDR2HDRConfig;
	struct hdr2sdr_reg_struct HDR2SDRConfig;
};


typedef struct _TONE_MAPPING_PQ_FIELD_T {
	UINT32 dynamic_rendering;                                                  /* SW register, 1b */

	UINT32 static_mode_gain_curve_idx;                                         /* 4b, 0~8 for st2084, 0~1 for hlg */
	UINT32 gain_curve_boost_speed;                                             /* 8b, 0~255 */

	UINT32 hlg_gain_curve_idx;                                                 /* 2b, 0~3 for hlg */

	UINT32 tgt_hist_idx_array[10];  /* 0~32, 6b.  {20, 22, 24, 24, 25, 26, 27, 28, 29, 29} */

	UINT32 high_bright;                                                        /* 32b, 0~400000, SW register */

#ifdef __BD8581__
	UINT32 tone_mapping_idx;
#endif

	UINT32 dark_scene_slope1, dark_scene_slope2, dark_scene_darken_rate;       /* 9b, 0~256 */
	UINT32 dark_scene_p1, dark_scene_p2;                                       /* 32b, 0~400000 */

	UINT32 normal_scene_slope1, normal_scene_slope2, normal_scene_darken_rate; /* 9b, 0~256 */
	UINT32 normal_scene_p1, normal_scene_p2;                                   /* 32b, 0~400000 */

	UINT32 bright_scene_slope1, bright_scene_slope2;                           /* 9b, 0~256 */
	UINT32 bright_scene_p1, bright_scene_p2;                                   /* 32b, 0~400000 */

	UINT32 non_bright_scene_slope, non_bright_scene_lighten_rate;              /* 9b, 0~256 */
	UINT32 non_bright_scene_p1, non_bright_scene_p2;                           /* 32b, 0~400000 */

	UINT32 panel_nits_change_rate;                                             /* SW register, 9b, 0.03 = 15/512 */
	UINT32 tgt_nits_change_step;                                               /* SW register, 4b, 0~15 */
	UINT32 fade_hist_change_rate;                                              /* SW register, 9b, 0.06 = 31/512 */
	UINT32 fade_tgt_nits_change_rate;                                          /* SW register, 9b, 0.1 = 51/512 */
	UINT32 tgt_nits_assign_factor;                                             /* SW register, 5b, 0~16 */
	UINT32 fade_tgt_nits_assign_factor;                                        /* SW register, 5b, 0~16 */

	UINT32 p1, p2, p3, p4, p5;                                                     /* 16+3 = 19b */
	UINT32 slope0, slope1, slope2, slope3, slope4;                             /* 12+4 = 16b */

	UINT32 nr_strength_b;
	UINT32 mode_weight; /* mode_weith */

	UINT32 low_flicker_mode_en;
	UINT32 low_flicker_mode_scene_change_nits_diff;
	UINT32 low_flicker_mode_different_scene_light_decrease;
	UINT32 low_flicker_mode_different_scene_light_increase;
	UINT32 low_flicker_mode_fade_decrease;
	UINT32 low_flicker_mode_fade_increase;
	UINT32 low_flicker_mode_same_scene_chase_gap;
	UINT32 low_flicker_mode_same_scene_chase_converge_period;
	UINT32 low_flicker_mode_same_scene_chase_max_speed;

	UINT32 xpercent_histogram_tuning_en;
	UINT32 max_hist_70_100_percent_nits;
	UINT32 approach_saturate_region;
	UINT32 tone_mapping_truncate_mode;
	UINT32 min_p_end;
	UINT32 tone_mapping_fw_refine_test_en;
	UINT32 dynamic_mode_fix_gain_curve_en;
	UINT32 dynamic_mode_fixed_gain_curve_idx;

} TONE_MAPPING_PQ_FIELD_T;

/*dynamic clock on/off whthin vsync interval */
struct disp_hdr_dynamic_clock_on_off {
	uint16_t clk_dyn_off_start_main;
	uint16_t clk_dyn_off_end_main;
	uint16_t clk_dyn_off_start_sub;
	uint16_t clk_dyn_off_end_sub;
	uint16_t clk_dyn_ctrl_main;
	uint16_t clk_dyn_ctrl_sub;
	HDMI_VIDEO_RESOLUTION res_mode;
};

/*adjust osd path delayed value when using sdr2hdr&bt2020l */
struct disp_sdr2hdr_osd_path_delay {
	uint32_t osd_delay;
	HDMI_VIDEO_RESOLUTION res_mode;
};

#endif /* endof __HDR_DEF_H__ */
