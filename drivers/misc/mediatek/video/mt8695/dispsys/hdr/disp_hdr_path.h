#ifndef __DISP_HDR_PATH_H__
#define __DISP_HDR_PATH_H__
#include "disp_hdr_def.h"
/*
** This file is for HW path select & clock control(for every sub_module)
*/
/* handle BT2020 path */
enum HDR_STATUS hdr_path_handle_bt2020(struct bt2020_reg_struct *pConfig);

/* handle SDR2HDR path */
enum HDR_STATUS hdr_path_handle_sdr2hdr(struct sdr2hdr_reg_struct *pConfig);

/* handle HDR2SDR path */
enum HDR_STATUS hdr_path_handle_hdr2sdr(struct hdr2sdr_reg_struct *pConfig);

#endif /* __DISP_HDR_PATH_H__ */
