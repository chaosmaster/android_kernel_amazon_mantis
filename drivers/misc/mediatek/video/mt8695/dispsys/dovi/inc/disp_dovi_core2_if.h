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

#ifndef _DOVI_CORE2_IF_H_
#define _DOVI_CORE2_IF_H_

int dovi_core2_init(char *reg_base);
int dovi_core2_config_lut(uint32_t *p_core2_lut);
int dovi_core2_lut_status(void);
int dovi_core2_status(void);

int dovi_core2_uninit(void);
extern struct device *dovi_dev;
#endif				/* _DOVI_CORE3_DRV_H_ */
