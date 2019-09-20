#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include "disp_hdr_device.h"
#include "disp_hdr_util.h"



static struct hdr_device_info gHdrDeviceInfo = {0};

static uint32_t _hdr_device_get_node_PA(struct device_node *node, int index)
{
	struct resource res;
	int status;
	uint32_t regBasePA = 0L;

	do {
		status = of_address_to_resource(node, index, &res);
		if (status < 0)
			break;

		regBasePA = res.start;
	} while (0);

	return regBasePA;
}

/*
** BT2020:
** MAIN:0x15002400 -- 0x150024FC
** SUB:0x15009400 -- 0x150094FC
** OSD:0x14002000 -- 0x140020FC

** SDR2HDR:
** OSD: 0x14002100  LUT: 0x14002200

** HDR2SDR:
** MAIN:0x15002000 -- 0x150020FC  LUT: 0x1500_2100 -- 0x1500_21FC  HIST: 0x1500_2200 -- 0x1500_22FC
** SUB: 0x15009000 -- 0x150090FC  LUT: 0x1500_9100 -- 0x1500_91FC  HIST: 0x1500_9200 -- 0x1500_92FC

** 14002000 ~ 14003000	OSD PATH: OSD SDR2HDR  OSD BT2020
** 15002000 ~ 15003000	MAIN PATH: MAIN HDR2SDR MAIN BT2020
** 15009000 ~ 1500A000	SUB PATH: SUB HDR2SDR SUB BT2020

** 14000000 ~ 1400 0100	for debug disp path
** 15000000 ~ 1500 0100 for debug osd path
*/


enum HDR_STATUS hdr_device_init_device_info(void)
{
	struct device_node *node;

	/* DEBUG 1400 0000 ~ 1400 0100
	**		 1500 0000 ~ 1500 0100
	*/
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-hdr-ctrl");
	if (node == NULL) {
		HDR_ERR("get device node mediatek, mt8695-hdr-debug fail\n");
		return HDR_STATUS_GET_HDR_DEBUG_DEVICE_TREE_FAIL;
	}
	gHdrDeviceInfo.HDR_OSD_CTRL_VA = of_iomap(node, 0);
	gHdrDeviceInfo.HDR_OSD_CTRL_PA = _hdr_device_get_node_PA(node, 0);

	gHdrDeviceInfo.HDR_DISP_CTRL_VA = of_iomap(node, 1);
	gHdrDeviceInfo.HDR_DISP_CTRL_PA = _hdr_device_get_node_PA(node, 1);

	/* OSD 14002000 ~ 14003000 */
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-hdr-osd");
	if (node == NULL) {
		HDR_ERR("get device node mediatek, mt8695-hdr-osd fail\n");
		return HDR_STATUS_GET_HDR_OSD_DEVICE_TREE_FAIL;
	}
	gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA = of_iomap(node, 0);
	gHdrDeviceInfo.SDR2HDR_OSD_REG_BASE_VA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA + 0x100;
	gHdrDeviceInfo.SDR2HDR_OSD_LUMA_REG_BASE_VA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA + 0x200;

	gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA = _hdr_device_get_node_PA(node, 0);
	gHdrDeviceInfo.SDR2HDR_OSD_REG_BASE_PA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA + 0x100;
	gHdrDeviceInfo.SDR2HDR_OSD_LUMA_REG_BASE_PA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA + 0x200;

	/* MAIN 15002000 ~ 15003000	*/
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-hdr-main");
	if (node == NULL) {
		HDR_ERR("get device node mediatek, mt8695-hdr-main fail\n");
		return HDR_STATUS_GET_HDR_MAIN_DEVICE_TREE_FAIL;
	}
	gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA = of_iomap(node, 0);
	gHdrDeviceInfo.HDR2SDR_MAIN_LUMA_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA + 0x100;
	gHdrDeviceInfo.HDR2SDR_MAIN_HIST_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA + 0x200;
	gHdrDeviceInfo.BT2020_MAIN_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA + 0x400;

	gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA = _hdr_device_get_node_PA(node, 0);
	gHdrDeviceInfo.HDR2SDR_MAIN_LUMA_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA + 0x100;
	gHdrDeviceInfo.HDR2SDR_MAIN_HIST_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA + 0x200;
	gHdrDeviceInfo.BT2020_MAIN_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA + 0x400;



	/* SUB 15009000 ~ 1500A000 */
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-hdr-sub");
	if (node == NULL) {
		HDR_ERR("get device node mediatek, mt8695-hdr-sub fail\n");
		return HDR_STATUS_GET_HDR_MAIN_DEVICE_TREE_FAIL;
	}
	gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA = of_iomap(node, 0);
	gHdrDeviceInfo.HDR2SDR_SUB_LUMA_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA + 0x100;
	gHdrDeviceInfo.HDR2SDR_SUB_HIST_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA + 0x200;
	gHdrDeviceInfo.BT2020_SUB_REG_BASE_VA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA + 0x400;

	gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA = _hdr_device_get_node_PA(node, 0);
	gHdrDeviceInfo.HDR2SDR_SUB_LUMA_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA + 0x100;
	gHdrDeviceInfo.HDR2SDR_SUB_HIST_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA + 0x200;
	gHdrDeviceInfo.BT2020_SUB_REG_BASE_PA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA + 0x400;

	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr_device_map_bt2020_address_from_plane(int plane, char **ppBt2020VA, uint32_t *pBt2020PA)
{
	if (BT2020_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane:%d for BT2020, LINE:%d\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	switch (plane) {
	case BT2020_PLANE_MAIN:
		if (ppBt2020VA != NULL)
			*ppBt2020VA = gHdrDeviceInfo.BT2020_MAIN_REG_BASE_VA;
		if (pBt2020PA != NULL)
			*pBt2020PA = gHdrDeviceInfo.BT2020_MAIN_REG_BASE_PA;
		break;
	case BT2020_PLANE_SUB:
		if (ppBt2020VA != NULL)
			*ppBt2020VA = gHdrDeviceInfo.BT2020_SUB_REG_BASE_VA;
		if (pBt2020PA != NULL)
			*pBt2020PA = gHdrDeviceInfo.BT2020_SUB_REG_BASE_PA;
		break;
	case BT2020_PLANE_OSD:
		if (ppBt2020VA != NULL)
			*ppBt2020VA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA;
		if (pBt2020PA != NULL)
			*pBt2020PA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA;
		break;
	default:
		HDR_ERR("invalid plane[%d] for BT2020\n", plane);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr_device_map_sdr2hdr_address_from_plane(int plane, char **ppSDR2HDRVA, uint32_t *pSDR2HDRPA,
						char **ppSDR2HDRLumaVA, uint32_t *pSDR2HDRLumaPA)
{
	if (SDR2HDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for SDR2HDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	switch (plane) {
	case SDR2HDR_PLANE_OSD:
		if (ppSDR2HDRVA != NULL)
			*ppSDR2HDRVA = gHdrDeviceInfo.SDR2HDR_OSD_REG_BASE_VA;
		if (pSDR2HDRPA != NULL)
			*pSDR2HDRPA = gHdrDeviceInfo.SDR2HDR_OSD_REG_BASE_PA;
		if (ppSDR2HDRLumaVA != NULL)
			*ppSDR2HDRLumaVA = gHdrDeviceInfo.SDR2HDR_OSD_LUMA_REG_BASE_VA;
		if (pSDR2HDRLumaPA != NULL)
			*pSDR2HDRLumaPA = gHdrDeviceInfo.SDR2HDR_OSD_LUMA_REG_BASE_PA;
		break;
	default:
		HDR_ERR("invalid plane[%d] for sdr2hdr\n", plane);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	return HDR_STATUS_OK;
}

enum HDR_STATUS hdr_device_map_hdr2sdr_address_from_plane(int plane, char **ppHDR2SDRVA, uint32_t *pHDR2SDRPA,
						char **ppHDR2SDRLumaVA, uint32_t *pHDR2SDRLumaPA,
						char **ppHDR2SDRHistVA, uint32_t *pHDR2SDRHistPA)
{
	if (HDR2SDR_VERIFY_PLANE(plane)) {
		HDR_ERR("invalid plane[%d] for HDR2SDR, line[%d]\n", plane, __LINE__);
		return HDR_STATUS_INVALID_PLANE_NUMBER;
	}

	switch (plane) {
	case HDR2SDR_PLANE_MAIN:
		if (ppHDR2SDRVA != NULL)
			*ppHDR2SDRVA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA;
		if (pHDR2SDRPA != NULL)
			*pHDR2SDRPA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA;
		if (ppHDR2SDRLumaVA != NULL)
			*ppHDR2SDRLumaVA = gHdrDeviceInfo.HDR2SDR_MAIN_LUMA_REG_BASE_VA;
		if (pHDR2SDRLumaPA != NULL)
			*pHDR2SDRLumaPA = gHdrDeviceInfo.HDR2SDR_MAIN_LUMA_REG_BASE_PA;
		if (ppHDR2SDRHistVA != NULL)
			*ppHDR2SDRHistVA = gHdrDeviceInfo.HDR2SDR_MAIN_HIST_REG_BASE_VA;
		if (pHDR2SDRHistPA != NULL)
			*pHDR2SDRHistPA = gHdrDeviceInfo.HDR2SDR_MAIN_HIST_REG_BASE_PA;
		break;
	case HDR2SDR_PLANE_SUB:
		if (ppHDR2SDRVA != NULL)
			*ppHDR2SDRVA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA;
		if (pHDR2SDRPA != NULL)
			*pHDR2SDRPA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA;
		if (ppHDR2SDRLumaVA != NULL)
			*ppHDR2SDRLumaVA = gHdrDeviceInfo.HDR2SDR_SUB_LUMA_REG_BASE_VA;
		if (pHDR2SDRLumaPA != NULL)
			*pHDR2SDRLumaPA = gHdrDeviceInfo.HDR2SDR_SUB_LUMA_REG_BASE_PA;
		if (ppHDR2SDRHistVA != NULL)
			*ppHDR2SDRHistVA = gHdrDeviceInfo.HDR2SDR_SUB_HIST_REG_BASE_VA;
		if (pHDR2SDRHistPA != NULL)
			*pHDR2SDRHistPA = gHdrDeviceInfo.HDR2SDR_SUB_HIST_REG_BASE_PA;

		break;
	}
	return HDR_STATUS_OK;
}

/*
** main path:	HDR2SDR BT2020
** sub path:	BT2020	SDR2HDR
** osd path:	BT2020	SDR2HDR
*/
bool hdr_device_map_bt2020_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane)
{
	switch (path) {
	case HDR_PATH_MAIN:
		*pPlane = BT2020_PLANE_MAIN;
		break;
	case HDR_PATH_SUB:
		*pPlane = BT2020_PLANE_SUB;
		break;
	case HDR_PATH_OSD:
		*pPlane = BT2020_PLANE_OSD;
		break;
	default:
		return false;
	}
	return true;
}

bool hdr_device_map_sdr2hdr_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane)
{
	switch (path) {
	case HDR_PATH_OSD:
		*pPlane = SDR2HDR_PLANE_OSD;
		break;
	default:
		return false;
	}
	return true;
}

bool hdr_device_map_hdr2sdr_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane)
{
	switch (path) {
	case HDR_PATH_MAIN:
		*pPlane = HDR2SDR_PLANE_MAIN;
		break;
	case HDR_PATH_SUB:
		*pPlane = HDR2SDR_PLANE_SUB;
		break;
	default:
		return false;
	}
	return true;
}


/* check if register address is hdr register */
static bool _hdr_device_register_PA_valid(uint32_t registerPA)
{
	/*
	** HDR_DEBUG_PA:			14000000 ~ 14000100
	**							15000000 ~ 15000100
	** MAIN: 15002000 ~ 15003000
	** SUB: 15009000 ~ 1500A000
	** OSD: 14002000 ~ 14003000
	*/
	if ((registerPA >= gHdrDeviceInfo.HDR_OSD_CTRL_PA &&
		registerPA < gHdrDeviceInfo.HDR_OSD_CTRL_PA + 0x100) ||
		(registerPA >= gHdrDeviceInfo.HDR_DISP_CTRL_PA &&
		registerPA < gHdrDeviceInfo.HDR_DISP_CTRL_PA + 0x100) ||
		(registerPA >= gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA + 0x1000) ||
		(registerPA >= gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA + 0x1000) ||
		(registerPA >= gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA + 0x1000))
		return true;

	HDR_ERR("invalid HDR register[0x%08x]", registerPA);
	return false;
}

static bool _hdr_device_register_VA_valid(char *registerVA)
{
	/* HDR_DEBUG_PA:			14000000 ~ 14000100
	**							15000000 ~ 15000100
	** MAIN: 15002000 ~ 15003000
	** SUB: 15009000 ~ 1500A000
	** OSD: 14002000 ~ 14003000
	*/
	if ((registerVA >= gHdrDeviceInfo.HDR_OSD_CTRL_VA &&
		registerVA < gHdrDeviceInfo.HDR_OSD_CTRL_VA + 0x100) ||
		(registerVA >= gHdrDeviceInfo.HDR_DISP_CTRL_VA &&
		registerVA < gHdrDeviceInfo.HDR_DISP_CTRL_VA + 0x100) ||
		(registerVA >= gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA + 0x1000) ||
		(registerVA >= gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA + 0x1000) ||
		(registerVA >= gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA + 0x1000))
		return true;

	HDR_ERR("invalid HDR register[%p]", registerVA);
	return false;
}

bool hdr_device_map_PA_2_VA(uint32_t registerPA, char **registerVA)
{
	if (false == _hdr_device_register_PA_valid(registerPA))
		return false;

	if (registerPA >= gHdrDeviceInfo.HDR_OSD_CTRL_PA &&
		registerPA < gHdrDeviceInfo.HDR_OSD_CTRL_PA + 0x100)
		*registerVA = gHdrDeviceInfo.HDR_OSD_CTRL_VA +
			(registerPA - gHdrDeviceInfo.HDR_OSD_CTRL_PA);
	else if (registerPA >= gHdrDeviceInfo.HDR_DISP_CTRL_PA &&
		registerPA < gHdrDeviceInfo.HDR_DISP_CTRL_PA + 0x100)
		*registerVA = gHdrDeviceInfo.HDR_DISP_CTRL_VA +
			(registerPA - gHdrDeviceInfo.HDR_DISP_CTRL_PA);
	else if (registerPA >= gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA + 0x1000)
		*registerVA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA +
			(registerPA - gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA);
	else if (registerPA >= gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA + 0x1000)
		*registerVA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA +
			(registerPA - gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA);
	else if (registerPA >= gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA &&
		registerPA < gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA + 0x1000)
		*registerVA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA +
		(registerPA - gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA);
	else
		WARN_ON(1);

	return true;
}

bool hdr_device_map_VA_2_PA(char *registerVA, uint32_t *registerPA)
{
	if (false == _hdr_device_register_VA_valid(registerVA))
		return false;

	if (registerVA >= gHdrDeviceInfo.HDR_OSD_CTRL_VA &&
		registerVA < gHdrDeviceInfo.HDR_OSD_CTRL_VA + 0x100)
		*registerPA = gHdrDeviceInfo.HDR_OSD_CTRL_PA +
			(registerVA - gHdrDeviceInfo.HDR_OSD_CTRL_VA);
	else if (registerVA >= gHdrDeviceInfo.HDR_DISP_CTRL_VA &&
		registerVA < gHdrDeviceInfo.HDR_DISP_CTRL_VA + 0x100)
		*registerPA = gHdrDeviceInfo.HDR_DISP_CTRL_PA +
			(registerVA - gHdrDeviceInfo.HDR_DISP_CTRL_VA);
	else if (registerVA >= gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA + 0x1000)
		*registerPA = gHdrDeviceInfo.BT2020_OSD_REG_BASE_PA +
			(registerVA - gHdrDeviceInfo.BT2020_OSD_REG_BASE_VA);
	else if (registerVA >= gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA + 0x1000)
		*registerPA = gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_PA +
			(registerVA - gHdrDeviceInfo.HDR2SDR_MAIN_REG_BASE_VA);
	else if (registerVA >= gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA &&
		registerVA < gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA + 0x1000)
		*registerPA = gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_PA +
			(registerVA - gHdrDeviceInfo.HDR2SDR_SUB_REG_BASE_VA);
	else
		WARN_ON(1);
	return true;
}

