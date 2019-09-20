/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Linjie Zhang <Linjie.Zhang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DI_TABLE_H
#define DI_TABLE_H

#include <linux/ioctl.h>
#include "mtk_vq_info.h"

#include "di_util.h"

#ifndef UINT32
#define UINT32 unsigned int
#endif

#define MAX_MOTION_LEVEL    5

typedef enum
{
  MA_RES_ZERO = 0,
  MA_RES_0,
  MA_RES_1,
  MA_RES_2,
  MA_RES_3,
  MA_RES_4,
  MA_MAX_RES,
} MA_RES_T;


typedef struct
{
  UINT32 DIFF_THR3;
  UINT32 DIFF_THR2;
  UINT32 DIFF_THR1;
  UINT32 SAW_TH;
  
  UINT32 TH_MIN_XZ;
  UINT32 TH_MED_XZ;
  UINT32 TH_NM_XZ;
  UINT32 TH_ED_XZ;
  
  UINT32 H_ED_TH;
  UINT32 TH_MIN_YW;
  UINT32 TH_MED_YW;
  UINT32 TH_NM_YW;
  
  UINT32 TH_ED_YW;
  UINT32 WH_TX_TH;
  UINT32 FCH_MIN_XZ;
  UINT32 FCH_NM_XZ;
  
  UINT32 VMV_FCH;
  UINT32 FCH_MIN_YW;
  UINT32 FCH_NM_YW;
  UINT32 EDGE_3LINE_GRAD_TH;
  
  UINT32 MA_EDGE_MODE6;
  UINT32 MA_EDGE_REJECT_TH_NM;
  UINT32 MA_EDGE_REJECT_TH_TIGHT;

} MA_THRESHOLD_T;


extern MA_THRESHOLD_T MA_FHD_TH;
extern MA_THRESHOLD_T MA_HD_TH;
extern MA_THRESHOLD_T MA_PAL_TH;
extern MA_THRESHOLD_T MA_NTSC_TH;
extern MA_THRESHOLD_T MA_OTHER_TH;
#endif				/* DI_HAL_H */
