#include <linux/vmalloc.h>
#include "disp_sdr2hdr.h"
#include "disp_hdr_device.h"
#include "disp_hdr_core.h"
#include "disp_hdr_table.h"
#include "disp_hdr_path.h"
#include "vdout_sys_hal.h"


/* SDR2HDR lut table */
static uint32_t SDR2HDR_LUT[SDR2HDR_PLANE_MAX][SDR2HDR_LUTSIZE] = { {0} };

#if CONFIG_DRV_SDR2HDR_USE_LINEAR_TABLES
static uint32_t SDR2HDR_LUT_LINEAR[SDR2HDR_LUTSIZE] = { {0} };
#endif

static char *pSDR2HDRVA[SDR2HDR_PLANE_MAX] = {NULL};
static char *pSDR2HDRLumaVA[SDR2HDR_PLANE_MAX] = {0};

/* write register callback function */
static enum HDR_STATUS _sdr2hdr_write_register(enum WRITE_CMD_ENUM cmd, struct write_cmd_struct *value)
{
	if (SDR2HDR_VERIFY_PLANE(value->plane)) {
		HDR_ERR("invalid plane[%d] for SDR2HDR, line[%d]\n", value->plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	switch (cmd) {
	case WRITE_CMD_GET_MODULE_NAME:
		snprintf(value->module_name, sizeof(value->module_name), "SDR2HDR");
		break;
	case WRITE_CMD_WRITE_REGISTER:
		return hdr_write_register(pSDR2HDRVA[value->plane] + value->offset, value->value, true);
	case WRITE_CMD_WRITE_REGISTER_WITH_MASK:
		return hdr_write_register_with_mask(pSDR2HDRVA[value->plane] + value->offset,
			value->value, value->mask, true);
	}
	return HDR_STATUS_OK;
}

/* SDR2HDR default register value */
enum HDR_STATUS sdr2hdr_default_param(struct sdr2hdr_reg_struct *pSDR2HDR)
{
	struct data_type_struct inputTypeBackup;
	struct data_type_struct outputTypeBackup;

	if (pSDR2HDR == NULL) {
		HDR_ERR("sdr2hdr_default_param: invalid param\n");
		return HDR_STATUS_NULL_POINTER;
	}

	inputTypeBackup = pSDR2HDR->inputType;
	outputTypeBackup = pSDR2HDR->outputType;
	memset(pSDR2HDR, 0, sizeof(*pSDR2HDR));
	pSDR2HDR->inputType = inputTypeBackup;
	pSDR2HDR->outputType = outputTypeBackup;
	pSDR2HDR->en_main_func = DISABLE_MAIN_FUNC;
	pSDR2HDR->yuv2rgb_config.en_yuv2rgb = DISABLE_YUV2RGB;
	pSDR2HDR->rgb2yuv_config.en_rgb2yuv = DISABLE_RGB2YUV;

/*
For DE propasal, about gamma_r/g/b_sram value shoule be set as followed
when check LUT table, if input values >= 0xffc, the output value will equal to  gamma_r/g/b_sram.
Currently, the maximum LUT table value is 0x88a,
if the input value is 0xffb, look up output value in this lut table, the output value is 0X88A,
if the input value is 0xffd, the outputs value is  equal to  gamma_r/g/b_sram, it is 0xff0,
so there is going to be a jump, the jump here causes the color jump in the picture to appear as some blue nosie block.
current the max lut table value is 0x88A, so change the  gamma_r/g/b_sram default value to 0x88a.
it's also need to update gamma_r/g/b_sram value, because it's default value is 0xff0.
*/
#ifdef MT8695
	/*
	pSDR2HDR->gamma_max_4096.gamma_r_sram_4096 = 0xFF0;
	pSDR2HDR->gamma_max_4096.gamma_g_sram_4096 = 0xFF0;
	pSDR2HDR->gamma_max_4096.gamma_b_sram_4096 = 0xFF0;
	*/
	pSDR2HDR->gamma_max_4096.update = true;
	pSDR2HDR->gamma_max_4096.gamma_r_sram_4096 = 0x88A;
	pSDR2HDR->gamma_max_4096.gamma_g_sram_4096 = 0x88A;
	pSDR2HDR->gamma_max_4096.gamma_b_sram_4096 = 0x88A;
#else
	pSDR2HDR->gamma_max_4096.gamma_r_sram_4096 = 0xFF;
	pSDR2HDR->gamma_max_4096.gamma_g_sram_4096 = 0xFF;
	pSDR2HDR->gamma_max_4096.gamma_b_sram_4096 = 0xFF;
#endif /*endof MT8695 */

	return HDR_STATUS_OK;
}


/*
** init sdr2hdr hardware register address
*/
static void _sdr2hdr_init_register_address(void)
{
	int i = 0;

	for (i = SDR2HDR_PLANE_OSD; i < SDR2HDR_PLANE_MAX; i++)
		hdr_device_map_sdr2hdr_address_from_plane(i, (char **)&pSDR2HDRVA[i], NULL,
		(char **)&pSDR2HDRLumaVA[i], NULL);
}

/* SDR2HDR default luma table */
enum HDR_STATUS _sdr2hdr_init_lut(int plane)
{
	int i = 0;

	if (SDR2HDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for SDR2HDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	for (i = 0; i < SDR2HDR_LUTSIZE; i++)
		SDR2HDR_LUT[plane][i] = eotf4[i];
	
#if CONFIG_DRV_SDR2HDR_USE_LINEAR_TABLES
	for (i = 0; i < SDR2HDR_LUTSIZE; i++)
		SDR2HDR_LUT_LINEAR[i] = i * 4;
	SDR2HDR_LUT_LINEAR[SDR2HDR_LUTSIZE] = 4095;
#endif
	return HDR_STATUS_OK;
}

/* generate SDR2HDR luma table
** calclulate luma table SDR2HDR_LUT[] with gain & OETF_10b
*/

static enum HDR_STATUS _sdr2hdr_config_lut(int plane, struct sdr2hdr_reg_struct *pConfig, uint32_t *pLUT)
{
	static uint32_t currentGain[SDR2HDR_PLANE_MAX] = {0};
	uint32_t gain = pConfig->inputType.sdr2hdrGain;
	uint32_t i, idx, idx1, fraction, mod;
	uint32_t dividend = 10000;
	uint64_t result;
#if CONFIG_DRV_SDR2HDR_USE_LINEAR_TABLES
	uint32_t lut_max_value = 0;
#endif
	pConfig->en_main_func = ENABLE_MAIN_FUNC;
	pConfig->lut_config.en_lut = true;

	if (currentGain[plane] == gain) {
		pConfig->lut_config.update = false;
		return HDR_STATUS_OK;
	}
	pConfig->lut_config.update = true;

	/* gain value has changed we need to regenerate LUT table */
	currentGain[plane] = gain;

	HDR_LOG("SDR2HDR[%d] gain value changed, regenerate LUT table\n", plane);

	for (i = 0; i < SDR2HDR_LUTSIZE; i++) {
		result = (uint64_t)OETF_10b[i] * 10 * gain;
		mod = do_div(result, dividend);
		idx = ((uint32_t)(result)) >> (22 - 10); /* 31-9=22 */
		if (idx < 1024) {
			idx = idx;
			result = (uint64_t)OETF_10b[i] * 10 * gain;
			mod = do_div(result, dividend);
			fraction = (((uint32_t)(result)) >> (22 - 10 - 7)) % 128;
			idx1 = idx + 1;
			pLUT[i] = (OETF_2084_12b[idx] * (128 - fraction) + OETF_2084_12b[idx1] * fraction) >> 7;
		} else {
			result = (uint64_t)OETF_10b[i] * 10 * gain;
			mod = do_div(result, dividend);
			idx = (((int)(result)) >> 22)  + 1024 - 1;
			result = (uint64_t)OETF_10b[i] * 10 * gain;
			mod = do_div(result, dividend);
			fraction = (((uint32_t)(result)) >> (22 - 4)) % 16;
			idx1 = (idx + 1) > 1534 ? 1534 : idx + 1;
			pLUT[i] = (OETF_2084_12b[idx] * (16 - fraction) + OETF_2084_12b[idx1] * fraction) >> 4;
		}
	}

#if CONFIG_DRV_SDR2HDR_USE_LINEAR_TABLES
	for (i = 0; i < SDR2HDR_LUTSIZE; i++)
		if (pLUT[i] > lut_max_value)
			lut_max_value = pLUT[i];

	for (i = 0; i < SDR2HDR_LUTSIZE; i++) {
		pLUT[i] = (SDR2HDR_LUT_LINEAR[i] * (235 - 16) / 255) * ((lut_max_value - 256) / (3760 - 256));
		pLUT[i] += 256;
		if (pLUT[i] > 4095)
			pLUT[i] = 4095;
	}	
#endif

	return HDR_STATUS_OK;
}



/*
** RGB format: R / G / B each has 1024 datas need to write to LUT
** but we only have 64 register to write.
** we use rgbSelect to choose current write R / G / B  rgbSelect = {0(R) 1(G) 2(B)}
** we use oddSelect to devide 1024 datas to two groups {0->odd(0 2 4 ... 1022) 1->even(1 3 5 ... 1023)}
** every group has 512 datas.
** for each group we use phaseSelect to devide 512 datas into 8 phases.
** ecach pahses has 64 datas.
** eg. for even groups
**	phase 0: {0   2   4   6   ... 126}
**	phase 1: {128 130 132 134 ... 254}
**	phase 2: {256 258 260 262 ... 382}
**	phase 3: {384 386 388 390 ... 510}
**	phase 4: {512 514 516 518 ... 638}
**	phase 5: {640 642 644 646 ... 766}
**	phase 6: {768 770 772 774 ... 894}
**	phase 7: {896 898 900 902 ... 1022}
**
**	for odd group
**	phase 0: {1   3   5   7   ... 127}
**	phase 1: {129 131 133 135 ... 255}
**	phase 2: {257 259 261 263 ... 383}
**	phase 3: {385 387 389 391 ... 511}
**	phase 4: {513 515 517 519 ... 639}
**	phase 5: {641 643 645 647 ... 767}
**	phase 6: {769 771 773 775 ... 895}
**	phase 7: {897 899 901 903 ... 1023}
*/

/*
** This testcase contains two cases
** case 0 (param == 0): write rgb 3*1024 datas into LUTA & read it back. compare whether write & read values are same.
** case 1 (param == 1): write rgb 3*1024 datas into LUTB & read it back. compare whether write & read values are same.
**
** register:
** bit 13 update LUT B
** bit 12 update LUT A
** bit 9:8 update RGB LUT selection(rgbSelect)
**	2'b00: LUT R
**	2'b01: LUT G
**	2'b10: LUT B
**	2'b11: LUT RGB (will not test)
** bit 7 update ODD/EVEN LUT selection(oddSelect)  {1'b0: update even 1'b1: update odd}
** bit 6:4 update LUT ADDR MSB 3BITS [8:6](phaseSelect)
** bit 1 change gamma use LUT B
** bit 0 change gamma use LUT A
*/

enum HDR_STATUS sdr2hdr_write_lut(int plane, uint32_t *pLUT)
{
	static bool curve_use_table_A[SDR2HDR_PLANE_MAX] = {0}; /* 0: use table B, 1: use table A */
	int use_lut_A = 0; /* bit[0]*/
	int use_lut_B = 0; /* bit[1]*/
	int parse = 0; /* bit[6:4]*/
	int update_odd = 0; /* bit[7]*/
	int select_RGB = 0; /* bit[9:8]*/
	int update_lut_A = 0; /* bit[12]*/
	int update_lut_B = 0; /* bit[13]*/

	int i = 0;

	if (curve_use_table_A[plane] == 0) { /* table B is in use, we need to use table A */
		update_lut_A = 1;
		update_lut_B = 0;
	} else { /* table A is in use, we need to use table B */
		update_lut_A = 0;
		update_lut_B = 1;
	}

	for (select_RGB = 0; select_RGB < 3; select_RGB++)
		for (update_odd = 0; update_odd < 2; update_odd++)
			for (parse = 0; parse < 8; parse++) {
				/* use mask, don't affect use_lut_A & use_lut_B */
				hdr_write_register_with_mask(pSDR2HDRVA[plane] + 0x40,
					parse << 4 | update_odd << 7 | select_RGB << 8 |
					update_lut_A << 12 | update_lut_B << 13,
					0x7 << 4 | 1 << 7 | 0x3 << 8 | 0x3 << 12, false);
				for (i = 0; i < 64; i++)
					hdr_write_register(pSDR2HDRLumaVA[plane] + i * 4,
						pLUT[parse * 128 + i * 2 + update_odd], false);
			}

	/* update table done, need to use this table */
	update_lut_A = 0;
	update_lut_B = 0;
	parse = 0;
	update_odd = 0;
	select_RGB = 0;
	if (curve_use_table_A[plane] == 0) { /* table B is in use, we need to use table A */
		use_lut_A = 1;
		use_lut_B = 0;
	} else { /* table A is in use, we need to use table B */
		use_lut_A = 0;
		use_lut_B = 1;
	}

	curve_use_table_A[plane] = !curve_use_table_A[plane];
	hdr_write_register(pSDR2HDRVA[plane] + 0x40, use_lut_A << 0 | use_lut_B << 1, true);

	return HDR_STATUS_OK;
}

enum HDR_STATUS sdr2hdr_init(void)
{
	int i = 0;

	_sdr2hdr_init_register_address();

	for (i = SDR2HDR_PLANE_OSD; i < SDR2HDR_PLANE_MAX; i++)
		_sdr2hdr_init_lut(i);

	/* need to bypass sdr2hdr when bring up inited  */
	/*vdout_sys_hal_sdr2hdr_bypass(true);*/

	return HDR_STATUS_OK;
}


/*
** enable SDR2HDR
*/
enum HDR_STATUS sdr2hdr_config(int plane, struct config_info_struct *pConfig)
{
	if (SDR2HDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for SDR2HDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	sdr2hdr_default_param(&pConfig->SDR2HDRConfig);
	pConfig->SDR2HDRConfig.need_update = true;
	if (pConfig->SDR2HDRConfig.inputType.bypassModule) {
		HDR_LOG("bypass SDR2HDR\n");
		pConfig->SDR2HDRConfig.disable_sdr2hdr = true;
		/* handle path & control */
		hdr_path_handle_sdr2hdr(&pConfig->SDR2HDRConfig);
		return HDR_STATUS_OK;
	}


	/* config yuv2rgb */
	hdr_core_config_yuv2rgb(&pConfig->SDR2HDRConfig.inputType, &pConfig->SDR2HDRConfig.yuv2rgb_config);

	/* if gain value has changed, we need to regenerate SDR2HDR LumaTable */
	_sdr2hdr_config_lut(plane, &pConfig->SDR2HDRConfig, SDR2HDR_LUT[plane]);

	/* config rgb2yuv */
	hdr_core_config_rgb2yuv(&pConfig->SDR2HDRConfig.outputType, &pConfig->SDR2HDRConfig.rgb2yuv_config);

	/* handle path & control */
	hdr_path_handle_sdr2hdr(&pConfig->SDR2HDRConfig);

#if 0
	pConfig->SDR2HDRConfig.need_update = pConfig->SDR2HDRConfig.yuv2rgb_config.update ||
		pConfig->SDR2HDRConfig.lut_config.update ||
		pConfig->SDR2HDRConfig.rgb2yuv_config.update;
#endif
	return HDR_STATUS_OK;
}


/*
** update config value to hardware register.
*/
enum HDR_STATUS sdr2hdr_update(int plane, struct config_info_struct *pConfig)
{
	uint32_t value = 0;
	struct sdr2hdr_reg_struct *pSDR2HDRConfig = &pConfig->SDR2HDRConfig;

	if (SDR2HDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for SDR2HDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	if (!pSDR2HDRConfig->need_update) {
		HDR_ERR("SDR2HDR don't need to update\n");
		return HDR_STATUS_OK;
	}

	/* config control register */
	do {
		value = pSDR2HDRConfig->bypass_sdr2hdr |
				pSDR2HDRConfig->en_main_func |

				pSDR2HDRConfig->yuv2rgb_config.en_yuv2rgb |
				pSDR2HDRConfig->yuv2rgb_config.yuv2rgb_coef |

				(!pSDR2HDRConfig->lut_config.en_lut << 20) |
				pSDR2HDRConfig->rgb2yuv_config.en_rgb2yuv |
				pSDR2HDRConfig->rgb2yuv_config.rgb2yuv_coef;
		hdr_write_register(pSDR2HDRVA[plane] + 0x00, value, true);
	} while (0);

	/* config path & clock */
	/*now sdr2hdr direct links osd, it did not need to bypass and it is also didn't need to turn off internal clock */
	/*hdr_write_register(pSDR2HDRVA[plane] + 0x70, pSDR2HDRConfig->path_config_value, true);*/
	/*hdr_write_register(pSDR2HDRVA[plane] + 0x74, pSDR2HDRConfig->clock_config_value, true);*/

	if (pSDR2HDRConfig->disable_sdr2hdr) {
#if 0
		/* disable sdr2hdr osd path external clock */
		disp_clock_enable(DISP_CLK_SDR2HDR, false);

		/* bypss sdr2hdr , disp_vdo_out bypass sdr2hdr path */
		vdout_sys_hal_sdr2hdr_bypass(true);
#endif
		return HDR_STATUS_OK;

	}
#if 0
	/* enable sdr2hdr osd path external clock */
	disp_clock_enable(DISP_CLK_SDR2HDR, true);

	/* disp_video_out which needs to through sdr2hdr path */
	vdout_sys_hal_sdr2hdr_bypass(false);
#endif

	/* yuv2rgb coef from external register */
	hdr_core_write_yuv2rgb_ext_register(plane, &pSDR2HDRConfig->yuv2rgb_config, _sdr2hdr_write_register);

	/* config gamma max 4096 input register */
	hdr_core_write_gamma4096_register(plane, &pSDR2HDRConfig->gamma_max_4096, _sdr2hdr_write_register);

	/* write lut table */
	if (pSDR2HDRConfig->lut_config.update)
		sdr2hdr_write_lut(plane, SDR2HDR_LUT[plane]);

	/* rgb2yuv coef from external register */
	hdr_core_write_rgb2yuv_ext_register(plane, &pSDR2HDRConfig->rgb2yuv_config, _sdr2hdr_write_register);

	return HDR_STATUS_OK;
}






