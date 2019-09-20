#include "disp_hdr_path.h"
#include "disp_hdr_util.h"
#include "disp_hdr_cli.h"

enum BT2020_CLOCK_BIT_ENUM {
	BT2020_CLOCK_OFF_YUV2RGB = 0,
	BT2020_CLOCK_OFF_DEGAMMA = 1,
	BT2020_CLOCK_OFF_MATRIX = 2,
	BT2020_CLOCK_OFF_GAMMA = 3,
	BT2020_CLOCK_OFF_RGB2YUV = 4,
	BT2020_CLOCK_OFF_MAX = 5,
};

enum SDR2HDR_CLOCK_BIT_ENUM {
	SDR2HDR_CLOCK_OFF_YUV2RGB = 0,
	SDR2HDR_CLOCK_OFF_LUT = 1,
	SDR2HDR_CLOCK_OFF_RGB2YUV = 2,
	SDR2HDR_CLOCK_OFF_MAX = 3,
};

enum HDR2SDR_CLOCK_BIT_ENUM {
	HDR2SDR_CLOCK_OFF_YUV2RGB = 0,
	HDR2SDR_CLOCK_OFF_DEGAMMA = 1,
	HDR2SDR_CLOCK_OFF_MATRIX = 2,
	HDR2SDR_CLOCK_OFF_TONE_MAP = 3,
	HDR2SDR_CLOCK_OFF_GAMMA = 4,
	HDR2SDR_CLOCK_OFF_RGB2YUV = 5,
	HDR2SDR_CLOCK_OFF_MAX = 6,
};

enum BT2020_PATH_SELECT_ENUM {
	BT2020_PATH_SELECT_FROM_UPPER = 0,
	BT2020_PATH_SELECT_FROM_RGB2YUV = 1,
	BT2020_PATH_SELECT_FROM_GAMMA = 2,
	BT2020_PATH_SELECT_FROM_MATRIX = 3,
	BT2020_PATH_SELECT_FROM_DEGAMMA = 4,
	BT2020_PATH_SELECT_FROM_YUV2RGB = 5,
	BT2020_PATH_SELECT_FROM_INPUT = 6,
};

enum SDR2HDR_PATH_SELECT_ENUM {
	SDR2HDR_PATH_OUT_SELECT_FROM_RGB2YUV = 0,
	SDR2HDR_PATH_OUT_SELECT_FROM_LUT = 1,
	SDR2HDR_PATH_OUT_SELECT_FROM_YUV2RGB = 2,
	SDR2HDR_PATH_OUT_SELECT_FROM_INPUT = 3,

	SDR2HDR_PATH_RGB2YUV_SELECT_FROM_LUT = 0,
	SDR2HDR_PATH_RGB2YUV_SELECT_FROM_YUV2RGB = 1,
	SDR2HDR_PATH_RGB2YUV_SELECT_FROM_INPUT = 2,

	SDR2HDR_PATH_LUT_SELECT_FROM_YUV2RGB = 0,
	SDR2HDR_PATH_LUT_SELECT_FROM_INPUT = 1,
};

enum HDR2SDR_PATH_SELECT_ENUM {
	HDR2SDR_PATH_SELECT_FROM_UPPER = 0,
	HDR2SDR_PATH_SELECT_FROM_RGB2YUV = 1,
	HDR2SDR_PATH_SELECT_FROM_GAMMA = 2,
	HDR2SDR_PATH_SELECT_FROM_TONE_MAP = 3,
	HDR2SDR_PATH_SELECT_FROM_MATRIX = 4,
	HDR2SDR_PATH_SELECT_FROM_DEGAMMA = 5,
	HDR2SDR_PATH_SELECT_FROM_YUV2RGB = 6,
	HDR2SDR_PATH_SELECT_FROM_INPUT = 7,
};

enum BT2020_PORT_ENUM {
	BT2020_PORT_YUV2RGB  = 0,
	BT2020_PORT_DEGAMMA = 1,
	BT2020_PORT_MATRIX = 2,
	BT2020_PORT_GAMMA = 3,
	BT2020_PORT_RGB2YUV = 4,
	BT2020_PORT_MAX_HW = 5,
	BT2020_PORT_OUTPUT = 5,
	BT2020_PORT_MAX = 6,
};

enum SDR2HDR_PORT_ENUM {
	SDR2HDR_PORT_YUV2RGB = 0,
	SDR2HDR_PORT_LUT = 1,
	SDR2HDR_PORT_RGB2YUV = 2,
	SDR2HDR_PORT_MAX_HW = 3,
	SDR2HDR_PORT_OUTPUT = 3,
	SDR2HDR_PORT_MAX = 4,
};

enum HDR2SDR_PORT_ENUM {
	HDR2SDR_PORT_YUV2RGB = 0,
	HDR2SDR_PORT_DEGAMMA = 1,
	HDR2SDR_PORT_MATRIX = 2,
	HDR2SDR_PORT_TONE_MAP = 3,
	HDR2SDR_PORT_GAMMA = 4,
	HDR2SDR_PORT_RGB2YUV = 5,
	HDR2SDR_PORT_MAX_HW = 6,
	HDR2SDR_PORT_OUTPUT = 7,
	HDR2SDR_PORT_MAX = 8,
};

static enum BT2020_PATH_SELECT_ENUM _hdr_path_bt2020_convert_index(int hwIndex)
{
	switch (hwIndex) {
	case 0:
		return BT2020_PATH_SELECT_FROM_YUV2RGB;
	case 1:
		return BT2020_PATH_SELECT_FROM_DEGAMMA;
	case 2:
		return BT2020_PATH_SELECT_FROM_MATRIX;
	case 3:
		return BT2020_PATH_SELECT_FROM_GAMMA;
	case 4:
		return BT2020_PATH_SELECT_FROM_RGB2YUV;
	default:
		return BT2020_PATH_SELECT_FROM_INPUT;
	}
}

/* yuv2rgb -> degamma -> matrix -> gamma -> rgb2yuv */
static uint32_t _hdr_path_calc_bt2020_path(const struct bt2020_reg_struct *pConfig)
{
	uint32_t path_value = 0;
	bool bypassHWArray[BT2020_PORT_MAX_HW] = {0}; /* record which HW need to bypass */
	enum BT2020_PATH_SELECT_ENUM selectFromSetting[BT2020_PORT_MAX] = {0};
	int checkHWIndex = BT2020_PORT_RGB2YUV;
	int fillPortIndex = BT2020_PORT_OUTPUT;

	bypassHWArray[BT2020_PORT_YUV2RGB] = (pConfig->yuv2rgb_config.update == false);
	bypassHWArray[BT2020_PORT_DEGAMMA] = (pConfig->degamma_config.update == false);
	bypassHWArray[BT2020_PORT_MATRIX] = (pConfig->matrix_config.update == false);
	bypassHWArray[BT2020_PORT_GAMMA] = (pConfig->gamma_config.update == false);
	bypassHWArray[BT2020_PORT_RGB2YUV] = (pConfig->rgb2yuv_config.update == false);

	for (fillPortIndex = BT2020_PORT_OUTPUT; fillPortIndex >= 0 && checkHWIndex >= 0; checkHWIndex--) {
		/* find the first not bypass HW ID */
		while ((checkHWIndex >= 0) && (bypassHWArray[checkHWIndex] == true))
			checkHWIndex--;
		/* if we don't found the bypass HW, checkHWIndex = -1, handle in _hdr_path_bt2020_convert_index */
		selectFromSetting[fillPortIndex] = _hdr_path_bt2020_convert_index(checkHWIndex);
		fillPortIndex = checkHWIndex;
	}

	path_value |= selectFromSetting[BT2020_PORT_YUV2RGB] << 20;
	path_value |= selectFromSetting[BT2020_PORT_DEGAMMA] << 16;
	path_value |= selectFromSetting[BT2020_PORT_MATRIX] << 12;
	path_value |= selectFromSetting[BT2020_PORT_GAMMA] << 8;
	path_value |= selectFromSetting[BT2020_PORT_RGB2YUV] << 4;
	path_value |= selectFromSetting[BT2020_PORT_OUTPUT] << 0;
	return path_value;
}
static uint32_t _hdr_path_calc_bt2020_clock(const struct bt2020_reg_struct *pConfig)
{
	uint32_t clock_value = 0;

	if (pConfig->disable_bt2020)
		return 0xFFFF; /* close every clock bit. */

	clock_value |= (pConfig->yuv2rgb_config.update == false) << BT2020_CLOCK_OFF_YUV2RGB;
	clock_value |= (pConfig->degamma_config.update == false) << BT2020_CLOCK_OFF_DEGAMMA;
	clock_value |= (pConfig->matrix_config.update == false) << BT2020_CLOCK_OFF_MATRIX;
	clock_value |= (pConfig->gamma_config.update == false) << BT2020_CLOCK_OFF_GAMMA;
	clock_value |= (pConfig->rgb2yuv_config.update == false) << BT2020_CLOCK_OFF_RGB2YUV;

	return clock_value;
}

static enum SDR2HDR_PATH_SELECT_ENUM _hdr_path_sdr2hdr_convert_index(int fillPortIndex, int checkHWIndex)
{
	if (fillPortIndex == SDR2HDR_PORT_LUT)
		switch (checkHWIndex) {
		case 0:
			return SDR2HDR_PATH_LUT_SELECT_FROM_YUV2RGB;
		default:
			return SDR2HDR_PATH_LUT_SELECT_FROM_INPUT;
		}
	if (fillPortIndex == SDR2HDR_PORT_RGB2YUV)
		switch (checkHWIndex) {
		case 0:
			return SDR2HDR_PATH_RGB2YUV_SELECT_FROM_YUV2RGB;
		case 1:
			return SDR2HDR_PATH_RGB2YUV_SELECT_FROM_LUT;
		default:
			return SDR2HDR_PATH_RGB2YUV_SELECT_FROM_INPUT;
		}
	if (fillPortIndex == SDR2HDR_PORT_OUTPUT)
		switch (checkHWIndex) {
		case 0:
			return SDR2HDR_PATH_OUT_SELECT_FROM_YUV2RGB;
		case 1:
			return SDR2HDR_PATH_OUT_SELECT_FROM_LUT;
		case 2:
			return SDR2HDR_PATH_OUT_SELECT_FROM_RGB2YUV;
		default:
			return SDR2HDR_PATH_OUT_SELECT_FROM_INPUT;
		}
	return 0;
}

static uint32_t _hdr_path_calc_sdr2hdr_path(const struct sdr2hdr_reg_struct *pConfig)
{
	uint32_t path_value = 0;

	bool bypassHWArray[SDR2HDR_PORT_MAX_HW] = {0}; /* record which HW need to bypass */
	uint32_t selectFromSetting[SDR2HDR_PORT_MAX] = {0};
	int checkHWIndex = SDR2HDR_PORT_RGB2YUV;
	int fillPortIndex = SDR2HDR_PORT_OUTPUT;

	bypassHWArray[SDR2HDR_PORT_YUV2RGB] = (pConfig->yuv2rgb_config.update == false);
	bypassHWArray[SDR2HDR_PORT_LUT] = (pConfig->disable_sdr2hdr == true);
	bypassHWArray[SDR2HDR_PORT_RGB2YUV] = (pConfig->rgb2yuv_config.update == false);

	for (fillPortIndex = SDR2HDR_PORT_OUTPUT; fillPortIndex >= 0 && checkHWIndex >= 0; checkHWIndex--) {
		/* find the first not bypass HW ID */
		while ((checkHWIndex >= 0) && (bypassHWArray[checkHWIndex] == true))
			checkHWIndex--;
		/* if we don't found the bypass HW, checkHWIndex = -1, handle in _hdr_path_sdr2hdr_convert_index */
		selectFromSetting[fillPortIndex] = _hdr_path_sdr2hdr_convert_index(fillPortIndex, checkHWIndex);
		fillPortIndex = checkHWIndex;
	}

	path_value |= selectFromSetting[SDR2HDR_PORT_LUT] << 4;
	path_value |= selectFromSetting[SDR2HDR_PORT_RGB2YUV] << 2;
	path_value |= selectFromSetting[SDR2HDR_PORT_OUTPUT] << 0;

	return path_value;
}

static uint32_t _hdr_path_calc_sdr2hdr_clock(const struct sdr2hdr_reg_struct *pConfig)
{
	uint32_t clock_value = 0;

	if (pConfig->disable_sdr2hdr)
		return 0xFFFF; /* close every clock bit */

	clock_value |= (pConfig->yuv2rgb_config.update == false) << SDR2HDR_CLOCK_OFF_YUV2RGB;
	/* clock_value |= (pConfig->lut_config.en_lut == false) << SDR2HDR_CLOCK_OFF_LUT; */
	clock_value |= 0 << SDR2HDR_CLOCK_OFF_LUT; /* always enable lut */
	clock_value |= (pConfig->rgb2yuv_config.update == false) << SDR2HDR_CLOCK_OFF_RGB2YUV;
	return clock_value;
}

static enum HDR2SDR_PATH_SELECT_ENUM _hdr_path_hdr2sdr_convert_index(int hwIndex)
{
	switch (hwIndex) {
	case 0:
		return HDR2SDR_PATH_SELECT_FROM_YUV2RGB;
	case 1:
		return HDR2SDR_PATH_SELECT_FROM_DEGAMMA;
	case 2:
		return HDR2SDR_PATH_SELECT_FROM_MATRIX;
	case 3:
		return HDR2SDR_PATH_SELECT_FROM_TONE_MAP;
	case 4:
		return HDR2SDR_PATH_SELECT_FROM_GAMMA;
	case 5:
		return HDR2SDR_PATH_SELECT_FROM_RGB2YUV;
	default:
		return HDR2SDR_PATH_SELECT_FROM_INPUT;
	}
}


static uint32_t _hdr_path_calc_hdr2sdr_path(const struct hdr2sdr_reg_struct *pConfig)
{
	uint32_t path_value = 0;
	bool bypassHWArray[HDR2SDR_PORT_MAX_HW] = {0}; /* record which HW need to bypass */
	enum HDR2SDR_PATH_SELECT_ENUM selectFromSetting[HDR2SDR_PORT_MAX] = {0};
	int checkHWIndex = HDR2SDR_PORT_RGB2YUV;
	int fillPortIndex = HDR2SDR_PORT_OUTPUT;

	bypassHWArray[HDR2SDR_PORT_YUV2RGB] = (pConfig->yuv2rgb_config.update == false);
	bypassHWArray[HDR2SDR_PORT_DEGAMMA] = (pConfig->st2084_config.update == false);
	bypassHWArray[HDR2SDR_PORT_MATRIX] = (pConfig->matrix_config.update == false);
	bypassHWArray[HDR2SDR_PORT_TONE_MAP] = (pConfig->disable_hdr2hdr == true);
	bypassHWArray[HDR2SDR_PORT_GAMMA] = (pConfig->gamma_config.update == false);
	bypassHWArray[HDR2SDR_PORT_RGB2YUV] = (pConfig->rgb2yuv_config.update == false);

	for (fillPortIndex = HDR2SDR_PORT_OUTPUT; fillPortIndex >= 0 && checkHWIndex >= 0; checkHWIndex--) {
		/* find the first not bypass HW ID */
		while ((checkHWIndex >= 0) && (bypassHWArray[checkHWIndex] == true))
			checkHWIndex--;
		/* if we don't found the bypass HW, checkHWIndex = -1, handle in _hdr_path_hdr2sdr_convert_index */
		selectFromSetting[fillPortIndex] = _hdr_path_hdr2sdr_convert_index(checkHWIndex);
		fillPortIndex = checkHWIndex;
	}

	path_value |= selectFromSetting[HDR2SDR_PORT_YUV2RGB] << 24;
	path_value |= selectFromSetting[HDR2SDR_PORT_DEGAMMA] << 20;
	path_value |= selectFromSetting[HDR2SDR_PORT_MATRIX] << 16;
	path_value |= selectFromSetting[HDR2SDR_PORT_TONE_MAP] << 12;
	path_value |= selectFromSetting[HDR2SDR_PORT_GAMMA] << 8;
	path_value |= selectFromSetting[HDR2SDR_PORT_RGB2YUV] << 4;
	path_value |= selectFromSetting[HDR2SDR_PORT_OUTPUT] << 0;
	return path_value;
}

static uint32_t _hdr_path_calc_hdr2sdr_clock(const struct hdr2sdr_reg_struct *pConfig)
{
	uint32_t clock_value = 0;

	if (pConfig->disable_hdr2hdr)
		return 0xFFFF; /* close every bit */

	clock_value |= (pConfig->yuv2rgb_config.update == false) << HDR2SDR_CLOCK_OFF_YUV2RGB;
	clock_value |= (pConfig->st2084_config.update == false) << HDR2SDR_CLOCK_OFF_DEGAMMA;
	clock_value |= (pConfig->matrix_config.update == false) << HDR2SDR_CLOCK_OFF_MATRIX;
	clock_value |= 0 << HDR2SDR_CLOCK_OFF_TONE_MAP; /* always enable lut */
	clock_value |= (pConfig->gamma_config.update == false) << HDR2SDR_CLOCK_OFF_GAMMA;
	clock_value |= (pConfig->rgb2yuv_config.update == false) << HDR2SDR_CLOCK_OFF_RGB2YUV;

	return clock_value;
}

enum HDR_STATUS hdr_path_handle_bt2020(struct bt2020_reg_struct *pConfig)
{
	bool disable_path_clock_control = true;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	disable_path_clock_control = hdr_cli_get_info()->disable_path_clock_control;
#endif

	if (!disable_path_clock_control) {
		pConfig->path_config_value = _hdr_path_calc_bt2020_path(pConfig);
		pConfig->clock_config_value = _hdr_path_calc_bt2020_clock(pConfig);
		HDR_LOG("BT2020:yuv2rgb[%d] degamma[%d] matrix[%d] gamma[%d] rgb2yuv[%d]\n",
			pConfig->yuv2rgb_config.update,
			pConfig->degamma_config.update,
			pConfig->matrix_config.update,
			pConfig->gamma_config.update,
			pConfig->rgb2yuv_config.update);
		HDR_LOG("BT2020: path[0x%x] clock[0x%x]\n",
			pConfig->path_config_value,
			pConfig->clock_config_value);
	} else {
		HDR_LOG("new function: path & clock control disable\n");
		pConfig->path_config_value = 0;
		pConfig->clock_config_value = 0;
	}
	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_path_handle_sdr2hdr(struct sdr2hdr_reg_struct *pConfig)
{
	bool disable_path_clock_control = true;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	disable_path_clock_control = hdr_cli_get_info()->disable_path_clock_control;
#endif
	if (!disable_path_clock_control) {
		pConfig->path_config_value = _hdr_path_calc_sdr2hdr_path(pConfig);
		pConfig->clock_config_value = _hdr_path_calc_sdr2hdr_clock(pConfig);
		HDR_LOG("SDR2HDR: yuv2rgb[%d] lut[%d] rgb2yuv[%d]\n",
			pConfig->yuv2rgb_config.update,
			(pConfig->disable_sdr2hdr == false),
			pConfig->rgb2yuv_config.update);
		HDR_LOG("SDR2HDR: path[0x%x] clock[0x%x]\n",
			pConfig->path_config_value,
			pConfig->clock_config_value);
	} else {
		HDR_LOG("new function: path & clock control disable\n");
		pConfig->path_config_value = 0;
		pConfig->clock_config_value = 0;
	}

	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_path_handle_hdr2sdr(struct hdr2sdr_reg_struct *pConfig)
{
	bool disable_path_clock_control = true;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	disable_path_clock_control = hdr_cli_get_info()->disable_path_clock_control;
#endif

	if (!disable_path_clock_control) {
		pConfig->path_config_value = _hdr_path_calc_hdr2sdr_path(pConfig);
		pConfig->clock_config_value = _hdr_path_calc_hdr2sdr_clock(pConfig);
		HDR_LOG("HDR2SDR: yuv2rgb[%d] degamma[%d] matrix[%d] tonemap[%d] gamma[%d] rgb2yuv[%d]\n",
			pConfig->yuv2rgb_config.update,
			pConfig->st2084_config.update,
			pConfig->matrix_config.update,
			(pConfig->disable_hdr2hdr == false),
			pConfig->gamma_config.update,
			pConfig->rgb2yuv_config.update);
		HDR_LOG("HDR2SDR: path[0x%x] clock[0x%x]\n",
			pConfig->path_config_value,
			pConfig->clock_config_value);
	} else {
		HDR_LOG("new function: path & clock control disable\n");
		pConfig->path_config_value = 0;
		pConfig->clock_config_value = 0;
	}
	return HDR_STATUS_OK;
}

