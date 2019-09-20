/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#ifndef _P2I_HAL_H_
#define _P2I_HAL_H_
#include "disp_type.h"
#include "disp_hw_mgr.h"


#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef NULL
#define NULL    0
#endif




/* P2I and CST mode */
#define P2I_ONLY  0
#define CST_ONLY  1
#define P2I_AND_CST  2

/*#define P2I_SYS_BASE 0xf0003700*/

/* #define vWriteP2I(dAddr, dVal)  WriteREG32(((uint64_t)(P2I_SYS_BASE+dAddr)), dVal) */
/* #define dReadP2I(dAddr)        ReadREG32(((uint64_t)(P2I_SYS_BASE+dAddr))) */
/* #define vWriteP2IMsk(dAddr, dVal, dMsk) vWriteP2I((dAddr), (dReadP2I(dAddr) & (~(dMsk))) | ((dVal) & (dMsk))) */

#define P2I_ERR(fmt, arg...)  pr_err("[P2I] error:"fmt, ##arg)
#define P2I_WARN(fmt, arg...)  pr_warn("[P2I]:"fmt, ##arg)
#define P2I_INFO(fmt, arg...)  pr_info("[P2I]:"fmt, ##arg)
#define P2I_LOG(fmt, arg...)  pr_debug("[P2I]:"fmt, ##arg)
#define P2I_DEBUG(fmt, arg...) pr_debug("[P2I]:"fmt, ##arg)

#define P2I_FUNC() pr_debug("[P2I] func: %s, line: %d\n", __func__, __LINE__)


/* API */
int p2i_hal_init(void);
void p2i_hal_set_cstmode(unsigned char mode);
void p2i_hal_enable_cst(bool is_on);
void p2i_hal_set_time(HDMI_VIDEO_RESOLUTION res);
void p2i_hal_turnoff_cst(bool turn_off);
void p2i_hal_enable_clk(bool on);
void p2i_hal_set_rwnotsametime(bool is_on);
uint32_t p2i_hal_get_tv_field(void);
void pi2_hal_reset(void);
#endif
