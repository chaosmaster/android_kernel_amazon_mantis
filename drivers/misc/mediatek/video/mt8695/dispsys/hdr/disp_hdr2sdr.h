#ifndef __HDR2SDR_H__
#define __HDR2SDR_H__

#include "disp_hdr_device.h"
#include "disp_hdr_def.h"
#include "disp_hdr_util.h"

#define HDR2SDR_GAIN_CURVE_SIZE 256

enum HDR_STATUS hdr2sdr_init(void);
enum HDR_STATUS hdr2sdr_config_tone_map(const struct tone_map_meta_data_struct *pToneMap,
	struct tone_map_config_struct *pConfig);
enum HDR_STATUS hdr2sdr_config(int plane, struct config_info_struct *pConfig);
enum HDR_STATUS hdr2sdr_update(int plane, struct config_info_struct *pConfig, bool bt2020_need_update);
enum HDR_STATUS hdr2sdr_default_param(struct hdr2sdr_reg_struct *pHDR2SDR);


#endif /* endif __HDR2SDR_H__ */
