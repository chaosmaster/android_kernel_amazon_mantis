/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
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

#ifndef MTK_VQ_UTIL_H
#define MTK_VQ_UTIL_H

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/regmap.h>

/* switch define start */
/* default define 0 */
#define VQ_MVA_MAP_VA                   0
#define DI_LOG_ALL                      1
#define VQ_SUPPORT_ION                  0
#define VQ_SUPPORT_IOMMU_ATTACH         1

/* default define 1 */
#define VQ_WAIT_IRQ                     1
#define VQ_WH_TIMING                    1
#define VQ_TIME_CHECK                   0
#define VQ_SUPPORT_LARB                 0
#define VQ_SUPPORT_REGMAP               0
/* switch define end */

#define VQ_INVALID_DW                   0xffffffff
#define VQ_INVALID_DECIMAL              99

#define DI_RET_OK                       0
#define DI_RET_ERR_PARAM                -1
#define DI_RET_ERR_EXCEPTION            -2

#if VQ_TIME_CHECK
#define VQ_TIME_CHECK_COUNT             10
#endif

/* debug */
#if DI_LOG_ALL
#define DI_LOG_DEFAULT      0
#define DI_LOG_CMD          0
#define DI_LOG_ERROR        1
#define DI_LOG_WARN         0
#define DI_LOG_IOCTL        0
#define DI_LOG_M2M          0
#define DI_LOG_PARAM        0
#define DI_LOG_ADDRESS      1
#define DI_LOG_FLOW         1
#define DI_LOG_TIME         1
#define DI_LOG_IRQ          1
#define DI_LOG_CTP          1
#define DI_LOG_REG          1
#else
#define DI_LOG_DEFAULT      0
#define DI_LOG_CMD          0
#define DI_LOG_ERROR        0
#define DI_LOG_WARN         0
#define DI_LOG_IOCTL        1
#define DI_LOG_M2M          2
#define DI_LOG_PARAM        3
#define DI_LOG_ADDRESS      4
#define DI_LOG_FLOW         5
#define DI_LOG_TIME         6
#define DI_LOG_IRQ          7
#define DI_LOG_CTP          8
#define DI_LOG_REG          9
#endif

extern unsigned int di_dbg_level;
#define DI_Printf(level, string, args...)  \
{ \
if (di_dbg_level & (1 << level)) { \
pr_err("[DI] "string"\n", ##args); \
} \
}

#define DBG_LINE        DI_Printf(DI_LOG_DEFAULT, "%s[%d]", __func__, __LINE__)

#if VQ_TIME_CHECK
extern unsigned int _au4VqTimeRec[];

#define VQ_TIME_REC(x) \
{ \
struct timeval TimeRec; \
do_gettimeofday(&TimeRec); \
_au4VqTimeRec[x] = TimeRec.tv_sec * 1000000 + TimeRec.tv_usec; \
}
#endif

#if VQ_SUPPORT_REGMAP
#define MTK_VQ_REG_BASE_COUNT       4
#else
#define MTK_VQ_REG_BASE_COUNT       5
#endif

#define MTK_VQ_SUPPORT_FIELD_TYPE(x) \
((V4L2_FIELD_NONE == (x)) || (V4L2_FIELD_TOP == (x)) || (V4L2_FIELD_BOTTOM == (x)) || \
(V4L2_FIELD_INTERLACED_TB == (x)) || (V4L2_FIELD_INTERLACED_BT == (x)))

/**********************************************************************************************************************/

extern unsigned int _vq_reg_base[MTK_VQ_REG_BASE_COUNT];

#if VQ_SUPPORT_REGMAP
extern struct regmap *_vq_regmap;
#endif

#endif				/* MTK_VQ_UTIL_H */
