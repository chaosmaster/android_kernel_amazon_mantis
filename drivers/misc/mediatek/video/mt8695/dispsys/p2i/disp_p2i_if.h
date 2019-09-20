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


#ifndef _P2I_IF_H_
#define _P2I_IF_H_

/* For p2i_cst mode setting mode */
#define P2I_ONLY_MODE  0
#define CST_ONLY_MODE  1
#define P2I_AND_CST_MODE  2

#define P2I_OK                  0
#define P2I_PARAM_ERR           -1
#define P2I_SET_FAIL            -2
#define P2I_GET_FAIL			-3
#define P2I_DTS_FAIL			-4

#define P2I_DRV_NAME  "disp_drv_p2i"


/* Extern API */
void disp_p2i_turnoff_cst(void);
void disp_p2i_turnon_cst(void);
uint32_t disp_p2i_get_tvfield(void);
void disp_p2i_reset(void);
struct disp_hw *disp_p2i_get_drv(void);

#endif
