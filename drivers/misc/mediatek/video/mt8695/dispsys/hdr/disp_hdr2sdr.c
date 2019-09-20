#include <linux/vmalloc.h>
#include "disp_hdr2sdr.h"
#include "disp_hdr_device.h"
#include "disp_hdr_util.h"
#include "disp_hdr_core.h"
#include "disp_hdr_sec.h"
#include "disp_hdr_path.h"
#include "disp_sys_hal.h"


/* static bool gUpdateHDR2SDRLutA[HDR2SDR_PLANE_MAX]; */

/* hardware register(register/lut/histograme) va address */
static char *pHDR2SDRVA[HDR2SDR_PLANE_MAX] = {NULL};
static char *pHDR2SDRLumaVA[HDR2SDR_PLANE_MAX] = {NULL};
static char *pHDR2SDRHistVA[HDR2SDR_PLANE_MAX] = {NULL};


static enum HDR_STATUS _hdr2sdr_write_register(enum WRITE_CMD_ENUM cmd, struct write_cmd_struct *value)
{
	if (HDR2SDR_VERIFY_PLANE(value->plane)) {
		HDR_ERR("invalid plane[%d] for HDR2SDR, line[%d]\n", value->plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}
	switch (cmd) {
	case WRITE_CMD_GET_MODULE_NAME:
		snprintf(value->module_name, sizeof(value->module_name), "HDR2SDR");
		break;
	case WRITE_CMD_WRITE_REGISTER:
		return hdr_write_register(pHDR2SDRVA[value->plane] + value->offset, value->value, true);
	case WRITE_CMD_WRITE_REGISTER_WITH_MASK:
		return hdr_write_register_with_mask(pHDR2SDRVA[value->plane] + value->offset,
			value->value, value->mask, true);
	}
	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr2sdr_default_param(struct hdr2sdr_reg_struct *pHDR2SDR)
{
	struct data_type_struct inputTypeBackup;
	struct data_type_struct outputTypeBackup;

	if (pHDR2SDR == NULL) {
		HDR_ERR("hdr2sdr_default_param: invalid param\n");
		return HDR_STATUS_NULL_POINTER;
	}

	inputTypeBackup = pHDR2SDR->inputType;
	outputTypeBackup = pHDR2SDR->outputType;

	memset(pHDR2SDR, 0, sizeof(*pHDR2SDR));
	pHDR2SDR->inputType = inputTypeBackup;
	pHDR2SDR->outputType = outputTypeBackup;

	pHDR2SDR->en_main_func = DISABLE_MAIN_FUNC;
	pHDR2SDR->yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
	pHDR2SDR->rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;
	#ifdef MT8695
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_r_sram_4096 = 0x0FF0;
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_g_sram_4096 = 0x0FF0;
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_b_sram_4096 = 0x0FF0;
	#else
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_r_sram_4096 = 0x00FF;
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_g_sram_4096 = 0x00FF;
	pHDR2SDR->gamma_config.gamma_max_4096.gamma_b_sram_4096 = 0x00FF;
	#endif /* endof MT8695 */

	pHDR2SDR->toneMap_config.gain_config.rgb2y_rcoef = 306;
	pHDR2SDR->toneMap_config.gain_config.rgb2y_gcoef = 601;
	pHDR2SDR->toneMap_config.gain_config.rgb2y_bcoef = 117;
	return HDR_STATUS_OK;
}

/*
** init hdr2sdr hardware register address
*/
static void _hdr2sdr_init_register_address(void)
{
	int i = 0;

	for (i = HDR2SDR_PLANE_MAIN; i < HDR2SDR_PLANE_MAX; i++)
		hdr_device_map_hdr2sdr_address_from_plane(i, (char **)&pHDR2SDRVA[i], NULL,
		(char **)&pHDR2SDRLumaVA[i], NULL,
		(char **)&pHDR2SDRHistVA[i], NULL);
}


enum HDR_STATUS hdr2sdr_init(void)
{
	_hdr2sdr_init_register_address();
	return HDR_STATUS_OK;
}

static enum HDR_STATUS _hdr2sdr_init_tone_map(GAMMA_TYPE_ENUM gammaType,
	struct tone_map_meta_data_struct *pToneMap, int plane)
{
	int curTVLuma = 30;

	WARN_ON((gammaType != GAMMA_TYPE_HLG) && (gammaType != GAMMA_TYPE_ST2084));

	if (gammaType == GAMMA_TYPE_HLG) {
		pToneMap->desaturate_config.line1_start = 4095;
		pToneMap->desaturate_config.line2_start = 4095;
		pToneMap->desaturate_config.line3_start = 4095;
		pToneMap->desaturate_config.line4_start = 4095;

		pToneMap->desaturate_config.slope0 = 4096;
		pToneMap->desaturate_config.slope1 = 4096;
		pToneMap->desaturate_config.slope2 = 4096;
		pToneMap->desaturate_config.slope3 = 4096;
		pToneMap->desaturate_config.slope4 = 4096;


		pToneMap->desaturate_config.en_de_sat = false;
		pToneMap->en_blend = false;
		pToneMap->en_fix_gain = false;
		pToneMap->en_hist = false;
		pToneMap->en_lut = false;
		pToneMap->en_xgain = false;
		pToneMap->fix_gain = 0;
		/* pToneMap->gain_curve */
	} else {
		/* 2^12 * 2^16 * (0~7) / 500 >> 4 will in int range 2^31 */
		pToneMap->desaturate_config.line1_start = (curTVLuma * 52428 / (500 << 4));
		pToneMap->desaturate_config.line2_start = (curTVLuma * 58982 / (500 << 4));
		pToneMap->desaturate_config.line3_start = (curTVLuma * 65535 / (500 << 4));
		pToneMap->desaturate_config.line4_start = (curTVLuma * 196605 / (500 << 4));
		/* ease low luminance region, make high luminance region has more slope */
		pToneMap->desaturate_config.slope0 = (500 * 52428 / (curTVLuma << 4));
		/*  2^12 * 2^16 / (2^12 <<4) */
		pToneMap->desaturate_config.slope1 = (500 * 14616 / (curTVLuma << 4));
		pToneMap->desaturate_config.slope2 = (500 * 7202 / (curTVLuma << 4));
		pToneMap->desaturate_config.slope3 = (500 * 7202 / (curTVLuma << 4));
		pToneMap->desaturate_config.slope4 = (500 * 7202 / (curTVLuma << 4));

		pToneMap->desaturate_config.en_de_sat = true;
		pToneMap->en_blend = true;
		pToneMap->en_fix_gain = false;
		pToneMap->en_hist = true;
		pToneMap->en_lut = true;
		pToneMap->en_xgain = true;
		pToneMap->fix_gain = 0;
	}

	pToneMap->mode_width = 4;
	pToneMap->nr_strength = 4;
	pToneMap->nr_type = NR_TYPE_NONE;
#ifdef HDR_SECURE_SUPPORT
		if (plane == HDR2SDR_PLANE_MAIN) {
			/*	handle hdr2sdr main path  */
			HDR_LOG("init hdr to hdr2sdr main path secure\n");
			hdr_sec_get_share_memory()->use_hdr = 1;
			hdr_sec_get_share_memory()->m_Gamma_type = gammaType;
			hdr_sec_get_share_memory()->m_TvMaxLuma = 300;
		} else {
			/*	handle hdr2sdr sub path  */
			hdr_sec_get_share_memory()->use_hdr_sub_path = 1;
			hdr_sec_get_share_memory()->m_Gamma_type_subpath = gammaType;
			hdr_sec_get_share_memory()->m_TvMaxLuma_subpath = 300;

		}
		hdr_sec_get_share_memory()->plane = plane;

		hdr_sec_service_call(SERVICE_CALL_CMD_COPY_SHARE_MEMORY,
			SERVICE_CALL_DIRECTION_INPUT,
			hdr_sec_get_share_memory(), sizeof(*hdr_sec_get_share_memory()));
#endif
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr2sdr_config_tone_map(const struct tone_map_meta_data_struct *pToneMap,
	struct tone_map_config_struct *pConfig)
{
	if (pConfig == NULL || pToneMap == NULL) {
		HDR_ERR("hdr2sdr_config_tone_map invalid NULL pointer\n");
		return HDR_STATUS_NULL_POINTER;
	}

	pConfig->desaturate_config = pToneMap->desaturate_config;
	pConfig->en_tone_map = pToneMap->en_blend || pToneMap->en_xgain || pToneMap->en_fix_gain || pToneMap->en_hist;
	pConfig->en_hist = pToneMap->en_hist;
	pConfig->gain_config.en_blend = pToneMap->en_blend;
	pConfig->gain_config.en_fix_gain = pToneMap->en_fix_gain;
	pConfig->gain_config.en_xgain = pToneMap->en_xgain;
	pConfig->gain_config.fix_gain = pToneMap->fix_gain;
	pConfig->gain_config.mode_width = pToneMap->mode_width;
	pConfig->gain_config.gain_curve_config.en_lut = pToneMap->en_lut;
	pConfig->gain_config.gain_curve_config.update = pToneMap->en_lut; /* update lut table */
	pConfig->nr_config.en_nr_r = false;
	pConfig->nr_config.en_nr_g = false;
	pConfig->nr_config.en_nr_b = (pToneMap->nr_type == NR_TYPE_NONE) ? false : true;
	pConfig->nr_config.nr_filter_no_r = false;
	pConfig->nr_config.nr_filter_no_g = false;
	pConfig->nr_config.nr_filter_no_b = (pToneMap->nr_type == NR_TYPE_9TAP) ? true : false;
	pConfig->nr_config.nr_strength_r = 0;
	pConfig->nr_config.nr_strength_g = 0;
	pConfig->nr_config.nr_strength_b = pToneMap->nr_strength;

	return HDR_STATUS_OK;
}

/* write tone-mapping related registers */
enum HDR_STATUS hdr2sdr_write_tone_map(int plane, struct tone_map_config_struct *pToneMap)
{
	uint32_t value = 0;

	if (HDR2SDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for HDR2SDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	value = pToneMap->en_tone_map << 0 |
			pToneMap->desaturate_config.en_de_sat << 1 |
			pToneMap->gain_config.en_xgain << 2 |
			pToneMap->gain_config.gain_curve_config.en_lut << 3 |
			pToneMap->nr_config.en_nr_r << 4 |
			pToneMap->nr_config.en_nr_g << 5 |
			pToneMap->nr_config.en_nr_b << 6 |
			pToneMap->en_hist << 7 |
			pToneMap->nr_config.nr_filter_no_r << 8 |
			pToneMap->nr_config.nr_filter_no_g << 9 |
			pToneMap->nr_config.nr_filter_no_b << 10 |
			pToneMap->gain_config.en_blend << 12 |
			pToneMap->desaturate_config.slope4 << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x70, value, true);

	value = pToneMap->desaturate_config.slope0 << 0 |
			pToneMap->desaturate_config.slope1 << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x74, value, true);

	value = pToneMap->desaturate_config.slope2 << 0 |
			pToneMap->desaturate_config.slope3 << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x78, value, true);

	value = pToneMap->desaturate_config.line1_start << 0 |
			pToneMap->desaturate_config.line2_start << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x7C, value, true);

	value = pToneMap->desaturate_config.line3_start << 0 |
			pToneMap->desaturate_config.line4_start << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x80, value, true);


	value = pToneMap->gain_config.fix_gain << 0 |
			pToneMap->gain_config.en_fix_gain << 31;
	hdr_write_register(pHDR2SDRVA[plane] + 0x84, value, true);

	value = pToneMap->gain_config.mode_width << 0 |
			pToneMap->nr_config.nr_strength_r << 4 |
			pToneMap->nr_config.nr_strength_g << 8 |
			pToneMap->nr_config.nr_strength_b << 12 |
			pToneMap->gain_config.rgb2y_rcoef << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x88, value, true);

	value = pToneMap->gain_config.rgb2y_gcoef << 0 |
			pToneMap->gain_config.rgb2y_bcoef << 16;
	hdr_write_register(pHDR2SDRVA[plane] + 0x8C, value, true);

	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr2sdr_config(int plane, struct config_info_struct *pConfig)
{
	struct tone_map_meta_data_struct tone_map_meta_data;

	if (HDR2SDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for HDR2SDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	hdr2sdr_default_param(&pConfig->HDR2SDRConfig);
	pConfig->HDR2SDRConfig.need_update = true;
	if (pConfig->HDR2SDRConfig.inputType.bypassModule) {
		HDR_LOG("bypass HDR2SDR\n");
		pConfig->HDR2SDRConfig.disable_hdr2hdr = true;
		/* config path & clock */
		hdr_path_handle_hdr2sdr(&pConfig->HDR2SDRConfig);
		return HDR_STATUS_OK;
	}
	_hdr2sdr_init_tone_map(pConfig->HDR2SDRConfig.inputType.gammaType, &tone_map_meta_data, plane);

	/* config yuv2rgb */
	hdr_core_config_yuv2rgb(&pConfig->HDR2SDRConfig.inputType, &pConfig->HDR2SDRConfig.yuv2rgb_config);

	/* config degamma(st2084) */
	hdr_core_config_degamma(&pConfig->HDR2SDRConfig.inputType, &pConfig->HDR2SDRConfig.st2084_config);


	/* config 3x3matrix */
	hdr_core_config_3x3matrix(&pConfig->HDR2SDRConfig.inputType,
							&pConfig->HDR2SDRConfig.outputType,
							&pConfig->HDR2SDRConfig.matrix_config);

	/* config tone-map */
	hdr2sdr_config_tone_map(&tone_map_meta_data, &pConfig->HDR2SDRConfig.toneMap_config);

	/* config gamma */
	hdr_core_config_gamma(&pConfig->HDR2SDRConfig.outputType, &pConfig->HDR2SDRConfig.gamma_config);


	/* config rgb2yuv */
	hdr_core_config_rgb2yuv(&pConfig->HDR2SDRConfig.outputType, &pConfig->HDR2SDRConfig.rgb2yuv_config);

	if (pConfig->HDR2SDRConfig.gamma_config.en_gamma == ENABLE_GAMMA ||
		pConfig->HDR2SDRConfig.st2084_config.en_degamma == ENABLE_DEGAMMA ||
		pConfig->HDR2SDRConfig.matrix_config.en_3x3matrix == ENABLE_3X3MATRIX ||
		pConfig->HDR2SDRConfig.toneMap_config.en_tone_map == true)
		pConfig->HDR2SDRConfig.en_main_func = ENABLE_MAIN_FUNC;

	/* config path & clock */
	hdr_path_handle_hdr2sdr(&pConfig->HDR2SDRConfig);

#if 0
	pConfig->HDR2SDRConfig.need_update = pConfig->HDR2SDRConfig.yuv2rgb_config.update ||
		pConfig->HDR2SDRConfig.st2084_config.update ||
		pConfig->HDR2SDRConfig.matrix_config.update ||
		pConfig->HDR2SDRConfig.gamma_config.update ||
		pConfig->HDR2SDRConfig.rgb2yuv_config.update;
#endif
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr2sdr_update(int plane, struct config_info_struct *pConfig, bool bt2020_need_update)
{
	uint32_t value = 0;
	struct hdr2sdr_reg_struct *pHDR2SDRConfig = &pConfig->HDR2SDRConfig;

	if (HDR2SDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for HDR2SDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	if (!pHDR2SDRConfig->need_update) {
		HDR_ERR("HDR2SDR don't need to update\n");
		return HDR_STATUS_OK;
	}

	/* config control register */
	do {
		value = pHDR2SDRConfig->bypass_hdr2sdr |
				pHDR2SDRConfig->en_main_func |
				pHDR2SDRConfig->yuv2rgb_config.en_yuv2rgb |
				pHDR2SDRConfig->yuv2rgb_config.yuv2rgb_coef |
				pHDR2SDRConfig->st2084_config.en_degamma |
				pHDR2SDRConfig->st2084_config.degamma_curve |
				pHDR2SDRConfig->matrix_config.en_3x3matrix |
				pHDR2SDRConfig->matrix_config.matrix_coef |
				pHDR2SDRConfig->gamma_config.en_gamma |
				pHDR2SDRConfig->gamma_config.gamma_curve |
				pHDR2SDRConfig->rgb2yuv_config.en_rgb2yuv |
				pHDR2SDRConfig->rgb2yuv_config.rgb2yuv_coef |
				pHDR2SDRConfig->yuv2rgb_config.constant_luma;
		hdr_write_register(pHDR2SDRVA[plane] + 0x00, value, true);
	} while (0);

	/* config path & clock value */
	hdr_write_register(pHDR2SDRVA[plane] + 0x90, pHDR2SDRConfig->path_config_value, true);
	hdr_write_register(pHDR2SDRVA[plane] + 0x94, pHDR2SDRConfig->clock_config_value, true);

	if (pHDR2SDRConfig->disable_hdr2hdr) {
#if 0
		/* disable HDR2SDR main/sub path external clock */
		disp_clock_enable((plane == HDR2SDR_PLANE_MAIN) ? DISP_CLK_MVDO_HDR2SDR : DISP_CLK_SVDO_HDR2SDR,
			false);

		/*bypass hdr2sdr, disp main/sub video out selects vdo3  */
		if (!bt2020_need_update)
			disp_sys_hal_set_disp_out((plane == HDR2SDR_PLANE_MAIN) ? DISP_MAIN : DISP_SUB,
					DISP_OUT_SEL_VDO);
#endif
		return HDR_STATUS_OK;
	}
#if 0
	/* enable HDR2SDR main/sub path external clock */
	disp_clock_enable((plane == HDR2SDR_PLANE_MAIN) ? DISP_CLK_MVDO_HDR2SDR : DISP_CLK_SVDO_HDR2SDR, true);

	/* disp main/sub video out selects hdr2sdr  */
	if (!bt2020_need_update)
		disp_sys_hal_set_disp_out(((plane == HDR2SDR_PLANE_MAIN) ? DISP_MAIN : DISP_SUB), DISP_OUT_SEL_HDR2SDR);
#endif
	/* yuv2rgb coef from external register */
	hdr_core_write_yuv2rgb_ext_register(plane, &pHDR2SDRConfig->yuv2rgb_config, _hdr2sdr_write_register);

	/* config 3x3matrix */
	hdr_core_write_matrix_ext_register(plane, &pHDR2SDRConfig->matrix_config, _hdr2sdr_write_register);

#if 1
	/* config tone map register */
	hdr2sdr_write_tone_map(plane, &pHDR2SDRConfig->toneMap_config);
#endif

	/* config gamma max 4096 input register */
	hdr_core_write_gamma4096_register(plane, &pHDR2SDRConfig->gamma_config.gamma_max_4096, _hdr2sdr_write_register);

	/* rgb2yuv coef from external register */
	hdr_core_write_rgb2yuv_ext_register(plane, &pHDR2SDRConfig->rgb2yuv_config, _hdr2sdr_write_register);

	return HDR_STATUS_OK;
}



