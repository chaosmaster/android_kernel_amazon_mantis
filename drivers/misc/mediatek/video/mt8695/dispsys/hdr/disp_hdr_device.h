#ifndef __HDR_DEVICE_H__
#define __HDR_DEVICE_H__
#include <linux/types.h>
#include "disp_hdr_def.h"


/*
** 8695 bt2020 hardware count, platform related.
*/
enum BT2020_PLANE_ENUM {
	BT2020_PLANE_MAIN = 0,
	BT2020_PLANE_SUB = 1,
	BT2020_PLANE_OSD = 2,
	BT2020_PLANE_MAX,
};

/*
** 8695 sdr2hdr hardware count, platform related.
*/
enum SDR2HDR_PLANE_ENUM {
	SDR2HDR_PLANE_OSD = 0, /* only OSD path have SDR2HDR HW. */
	SDR2HDR_PLANE_MAX,
};

/*
** 8695 hdr2sdr hardware count, platform related.
*/
enum HDR2SDR_PLANE_ENUM {
	HDR2SDR_PLANE_MAIN = 0,
	HDR2SDR_PLANE_SUB = 1,
	HDR2SDR_PLANE_MAX,
};

/* VERIFY INPUT VALUE VALID */
#define HDR_VERIFY_PATH(path) ((path) >= HDR_PATH_MAX)

#define BT2020_VERIFY_PLANE(plane) ((plane) >= BT2020_PLANE_MAX)

#define SDR2HDR_VERIFY_PLANE(plane) ((plane) >= SDR2HDR_PLANE_MAX)

#define HDR2SDR_VERIFY_PLANE(plane) ((plane) >= HDR2SDR_PLANE_MAX)


struct hdr_device_info {
	char *HDR_DEBUG_VA;
	char *HDR_DISP_CTRL_VA;
	char *HDR_OSD_CTRL_VA;

	char *BT2020_MAIN_REG_BASE_VA;
	char *BT2020_SUB_REG_BASE_VA;
	char *BT2020_OSD_REG_BASE_VA;

	char *SDR2HDR_OSD_REG_BASE_VA;
	char *SDR2HDR_OSD_LUMA_REG_BASE_VA;

	char *HDR2SDR_MAIN_REG_BASE_VA;
	char *HDR2SDR_SUB_REG_BASE_VA;
	char *HDR2SDR_MAIN_LUMA_REG_BASE_VA;
	char *HDR2SDR_SUB_LUMA_REG_BASE_VA;
	char *HDR2SDR_MAIN_HIST_REG_BASE_VA;
	char *HDR2SDR_SUB_HIST_REG_BASE_VA;

	uint32_t HDR_DEBUG_PA;
	uint32_t HDR_DISP_CTRL_PA;
	uint32_t HDR_OSD_CTRL_PA;

	uint32_t BT2020_MAIN_REG_BASE_PA;
	uint32_t BT2020_SUB_REG_BASE_PA;
	uint32_t BT2020_OSD_REG_BASE_PA;


	uint32_t SDR2HDR_OSD_REG_BASE_PA;
	uint32_t SDR2HDR_OSD_LUMA_REG_BASE_PA;

	uint32_t HDR2SDR_MAIN_REG_BASE_PA;
	uint32_t HDR2SDR_SUB_REG_BASE_PA;
	uint32_t HDR2SDR_MAIN_LUMA_REG_BASE_PA;
	uint32_t HDR2SDR_SUB_LUMA_REG_BASE_PA;
	uint32_t HDR2SDR_MAIN_HIST_REG_BASE_PA;
	uint32_t HDR2SDR_SUB_HIST_REG_BASE_PA;
};

enum HDR_STATUS hdr_device_map_bt2020_address_from_plane(int plane, char **ppBt2020VA, uint32_t *pBt2020PA);

enum HDR_STATUS hdr_device_map_sdr2hdr_address_from_plane(int plane, char **ppSDR2HDRVA, uint32_t *pSDR2HDRPA,
								char **ppSDR2HDRLumaVA, uint32_t *pSDR2HDRLumaPA);

enum HDR_STATUS hdr_device_map_hdr2sdr_address_from_plane(int plane, char **ppHDR2SDRVA, uint32_t *pHDR2SDRPA,
								char **ppHDR2SDRLumaVA, uint32_t *pHDR2SDRLumaPA,
								char **ppHDR2SDRHistVA, uint32_t *pHDR2SDRHistPA);

bool hdr_device_map_PA_2_VA(uint32_t registerPA, char **registerVA);
bool hdr_device_map_VA_2_PA(char *registerVA, uint32_t *registerPA);


enum HDR_STATUS hdr_device_init_device_info(void);

bool hdr_device_map_bt2020_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane);
bool hdr_device_map_sdr2hdr_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane);
bool hdr_device_map_hdr2sdr_plane_from_path(enum HDR_PATH_ENUM path, int *pPlane);


#endif /* endof __HDR_DEVICE_H__ */
