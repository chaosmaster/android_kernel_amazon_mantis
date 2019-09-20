#include <linux/vmalloc.h>
#include "disp_bt2020.h"
#include "disp_hdr_device.h"
#include "disp_hdr_core.h"
#include "disp_hdr_util.h"
#include "disp_hdr_path.h"
#include "disp_sys_hal.h"


/* BT2020 register VA address */
static char *pBT2020VA[BT2020_PLANE_MAX] = {NULL};

/* write register callback function, use in hdr_core */
static enum HDR_STATUS _bt2020_write_register(enum WRITE_CMD_ENUM cmd, struct write_cmd_struct *value)
{
	if (BT2020_VERIFY_PLANE(value->plane)) {
		HDR_ERR("invalid plane:%d for BT2020, LINE:%d\n", value->plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}
	switch (cmd) {
	case WRITE_CMD_GET_MODULE_NAME:
		snprintf(value->module_name, sizeof(value->module_name), "BT2020");
		break;
	case WRITE_CMD_WRITE_REGISTER:
		return hdr_write_register(pBT2020VA[value->plane] + value->offset, value->value, true);
	case WRITE_CMD_WRITE_REGISTER_WITH_MASK:
		return hdr_write_register_with_mask(pBT2020VA[value->plane] + value->offset,
			value->value, value->mask, true);
	}
	return HDR_STATUS_OK;
}

/* bt2020_reg_struct default register value */
enum HDR_STATUS bt2020_default_param(struct bt2020_reg_struct *pBT2020)
{
	struct data_type_struct inputTypeBackup;
	struct data_type_struct outputTypeBackup;

	if (pBT2020 == NULL) {
		HDR_ERR("bt2020_init_param: invalid param\n");
		return HDR_STATUS_NULL_POINTER;
	}
	inputTypeBackup = pBT2020->inputType;
	outputTypeBackup = pBT2020->outputType;

	memset(pBT2020, 0, sizeof(*pBT2020));
	pBT2020->inputType = inputTypeBackup;
	pBT2020->outputType = outputTypeBackup;
	pBT2020->en_main_func = DISABLE_MAIN_FUNC;
	pBT2020->yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
	pBT2020->rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;

#ifdef MT8695
	pBT2020->gamma_config.gamma_max_4096.gamma_r_sram_4096 = 0xFF0;
	pBT2020->gamma_config.gamma_max_4096.gamma_g_sram_4096 = 0xFF0;
	pBT2020->gamma_config.gamma_max_4096.gamma_b_sram_4096 = 0xFF0;
#else
	pBT2020->gamma_config.gamma_max_4096.gamma_r_sram_4096 = 0xFF;
	pBT2020->gamma_config.gamma_max_4096.gamma_g_sram_4096 = 0xFF;
	pBT2020->gamma_config.gamma_max_4096.gamma_b_sram_4096 = 0xFF;
#endif /*endof MT8695 */

	return HDR_STATUS_OK;
}

/* get BT2020 regsiter VA address from dts */
static void _bt2020_init_register_address(void)
{
	int i = 0;

	for (i = BT2020_PLANE_MAIN; i < BT2020_PLANE_MAX; i++)
		hdr_device_map_bt2020_address_from_plane(i, (char **)&pBT2020VA[i], NULL);
}

/* calculate BT2020 control register value */
static uint32_t _bt2020_config_control_register(const struct bt2020_reg_struct *pBT2020Config)
{
	uint32_t value = 0;

	value = pBT2020Config->bypass_bt2020 |
			pBT2020Config->en_main_func |

			pBT2020Config->yuv2rgb_config.en_yuv2rgb |
			pBT2020Config->yuv2rgb_config.yuv2rgb_coef |
			pBT2020Config->yuv2rgb_config.constant_luma |

			pBT2020Config->degamma_config.en_degamma |
			pBT2020Config->degamma_config.degamma_curve |

			pBT2020Config->matrix_config.en_3x3matrix |
			pBT2020Config->matrix_config.matrix_coef |

			pBT2020Config->gamma_config.en_gamma |
			pBT2020Config->gamma_config.gamma_curve |

			pBT2020Config->rgb2yuv_config.en_rgb2yuv |
			pBT2020Config->rgb2yuv_config.rgb2yuv_coef;
	return value;
}


enum HDR_STATUS bt2020_init(void)
{
	_bt2020_init_register_address();
	return HDR_STATUS_OK;
}

enum HDR_STATUS bt2020_config(int plane, struct config_info_struct *pConfig)
{
	if (BT2020_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane:%d for BT2020, LINE:%d\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	bt2020_default_param(&pConfig->BT2020Config);

	pConfig->BT2020Config.need_update = true;
	if (pConfig->BT2020Config.inputType.bypassModule) {
		HDR_LOG("bypass BT2020\n");
		pConfig->BT2020Config.disable_bt2020 = true;
		hdr_path_handle_bt2020(&pConfig->BT2020Config);
		return HDR_STATUS_OK;
	}

	/* config yuv2rgb */
	hdr_core_config_yuv2rgb(&pConfig->BT2020Config.inputType, &pConfig->BT2020Config.yuv2rgb_config);

	/* config degamma */
	hdr_core_config_degamma(&pConfig->BT2020Config.inputType, &pConfig->BT2020Config.degamma_config);
	/*pConfig->BT2020Config.degamma_config.degamma_curve = DEGAMMA_CURVE_SRGB;*/

	/* config 3x3matrix */
	hdr_core_config_3x3matrix(&pConfig->BT2020Config.inputType,
							&pConfig->BT2020Config.outputType,
							&pConfig->BT2020Config.matrix_config);

	/* config gamma */
	hdr_core_config_gamma(&pConfig->BT2020Config.outputType, &pConfig->BT2020Config.gamma_config);
	/*pConfig->BT2020Config.gamma_config.gamma_curve = GAMMA_CURVE_2p4;*/

	/* config rgb2yuv */
	hdr_core_config_rgb2yuv(&pConfig->BT2020Config.outputType, &pConfig->BT2020Config.rgb2yuv_config);

	if (pConfig->BT2020Config.degamma_config.en_degamma == ENABLE_DEGAMMA ||
		pConfig->BT2020Config.matrix_config.en_3x3matrix == ENABLE_3X3MATRIX ||
		pConfig->BT2020Config.gamma_config.en_gamma == ENABLE_GAMMA)
		pConfig->BT2020Config.en_main_func = ENABLE_MAIN_FUNC;
	else
		pConfig->BT2020Config.en_main_func = DISABLE_MAIN_FUNC;

	hdr_path_handle_bt2020(&pConfig->BT2020Config);

#if 0
	/* always update register when call bt2020_config
	** because we diff input info with former frame to decide whether call bt2020_config.
	*/
	pConfig->BT2020Config.need_update = pConfig->BT2020Config.yuv2rgb_config.update ||
		pConfig->BT2020Config.degamma_config.update ||
		pConfig->BT2020Config.matrix_config.update ||
		pConfig->BT2020Config.gamma_config.update ||
		pConfig->BT2020Config.rgb2yuv_config.update;
#endif
	return HDR_STATUS_OK;
}

enum HDR_STATUS bt2020_update(int plane, struct config_info_struct *pConfig)
{
	const struct bt2020_reg_struct *pBT2020Config = &(pConfig->BT2020Config);

	if (BT2020_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane:%d for BT2020, LINE:%d\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	if (!pBT2020Config->need_update) {
		HDR_ERR("BT2020 don't need to update\n");
		return HDR_STATUS_OK;
	}

	/* config control register */
	hdr_write_register(pBT2020VA[plane] + 0x00, _bt2020_config_control_register(pBT2020Config), true);

	/* path select & clock control */
	/*for osd path, bt2020 direct links osd, it did not need to bypass and it is also didn't need to turn off internal clock */
	if (pConfig->path != HDR_PATH_OSD) {
		hdr_write_register(pBT2020VA[plane] + 0x70, pBT2020Config->path_config_value, true);
		hdr_write_register(pBT2020VA[plane] + 0x74, pBT2020Config->clock_config_value, true);
	}

	if (pBT2020Config->disable_bt2020) {
#if 0
		/* bypass bt2020, disable bt2020 main/sub/osd clock or disp_video_out selects vdo3 out */
		if (plane < BT2020_PLANE_OSD) {
			disp_clock_enable((plane == BT2020_PLANE_MAIN) ? DISP_CLK_MVDO_BT2020 : DISP_CLK_SVDO_BT2020,
				false);
			disp_clock_enable((plane == BT2020_PLANE_MAIN) ? DISP_CLK_MVDO_HDR2SDR : DISP_CLK_SVDO_HDR2SDR,
				false);
			disp_sys_hal_set_disp_out((plane == BT2020_PLANE_MAIN) ? DISP_MAIN : DISP_SUB,
				DISP_OUT_SEL_VDO);
		} else
			disp_clock_enable(DISP_CLK_SDR2HDR, false);
#endif
		return HDR_STATUS_OK;
	}
#if 0
    /* enable bt2020 main/sub/osd clock or disp_video_out select bt2020 video out */
	if (plane < BT2020_PLANE_OSD) {
		disp_clock_enable((plane == BT2020_PLANE_MAIN) ? DISP_CLK_MVDO_BT2020 : DISP_CLK_SVDO_BT2020, true);
		disp_clock_enable((plane == BT2020_PLANE_MAIN) ? DISP_CLK_MVDO_HDR2SDR : DISP_CLK_SVDO_HDR2SDR,
			true);
		disp_sys_hal_set_disp_out((plane == BT2020_PLANE_MAIN) ? DISP_MAIN : DISP_SUB, DISP_OUT_SEL_BT2020);
	} else
		disp_clock_enable(DISP_CLK_SDR2HDR, true);
#endif
	/* yuv2rgb coef from external register */
	hdr_core_write_yuv2rgb_ext_register(plane, &pBT2020Config->yuv2rgb_config, _bt2020_write_register);

	/* matrix coef from external register */
	hdr_core_write_matrix_ext_register(plane, &pBT2020Config->matrix_config, _bt2020_write_register);

	/* config gamma max 4096 input register */
	hdr_core_write_gamma4096_register(plane, &pBT2020Config->gamma_config.gamma_max_4096, _bt2020_write_register);

	/* rgb2yuv coef from external register */
	hdr_core_write_rgb2yuv_ext_register(plane, &pBT2020Config->rgb2yuv_config, _bt2020_write_register);

	return HDR_STATUS_OK;
}


