#ifndef __HDR_SDR2HDR_H__
#define __HDR_SDR2HDR_H__
#include <linux/spinlock.h>
#include "disp_hdr_device.h"
#include "disp_hdr_def.h"
#include "disp_hdr_util.h"

#define SDR2HDR_LUTSIZE 1024 /* lut table size */

enum HDR_STATUS sdr2hdr_init(void);
enum HDR_STATUS sdr2hdr_config(int plane, struct config_info_struct *pConfig);
enum HDR_STATUS sdr2hdr_update(int plane, struct config_info_struct *pConfig);
enum HDR_STATUS sdr2hdr_default_param(struct sdr2hdr_reg_struct *pSDR2HDR);
enum HDR_STATUS sdr2hdr_write_lut(int plane, uint32_t *pLUT);

#endif /* endof __HDR_SDR2HDR_H__ */
