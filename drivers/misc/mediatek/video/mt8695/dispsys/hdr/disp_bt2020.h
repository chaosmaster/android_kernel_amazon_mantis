#ifndef __HDR_BT2020_H__
#define __HDR_BT2020_H__
#include <linux/spinlock.h>
#include "disp_hdr_device.h"
#include "disp_hdr_def.h"
#include "disp_hdr_util.h"



enum HDR_STATUS bt2020_init(void);
enum HDR_STATUS bt2020_config(int plane, struct config_info_struct *pConfig);
enum HDR_STATUS bt2020_update(int plane, struct config_info_struct *pConfig);
enum HDR_STATUS bt2020_default_param(struct bt2020_reg_struct *pBT2020);


#endif /* endof __HDR_BT2020_H__ */
