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


#ifndef _VDOUT_HW_H_
#define _VDOUT_HW_H_

#include <linux/types.h>

#define VIDEO_IN_CONFIG			0x0
	#define CONFIG_ON			(0x1 << 0)
	#define CONFIG_PROG			(0x1 << 3)
	#define CONFIG_PACKET_MODE	(0x3 << 16)
		#define CONFIG_8_BIT	(0x0 << 16)
		#define CONFIG_10_BIT	(0x2 << 16)
		#define CONFIG_12_BIT	(0x3 << 16)
	#define CONFIG_16_PACKET	(0x1 << 20)
	#define CONFIG_420_MODE		(0x1 << 25)
	#define CONFIG_DE_MODE		(0x1 << 29)

#define VIDEO_IN_Y_ADDR			0x8
#define VIDEO_IN_LINE			0xc
	#define H_ACTIVE_LINE		(0xfff << 0)
	#define C2_ENABLE			(0x1 << 29)
	#define C_ENABLE			(0x1 << 30)
	#define Y_ENABLE			(0x1 << 31)
#define VIDEO_IN_C_ADDR			0x10
#define VIDEO_IN_ACTIVE_LINE	0x14
#define VIDEO_IN_H_PKCNT		0x18
#define VIDEO_IN_INPUT_CTRL		0x20
	#define VIDEO_IN_Y_CHN_SEL	(0x3 << 0)
	#define VIDEO_IN_CB_CHN_SEL	(0x3 << 2)
	#define VIDEO_IN_CR_CHN_SEL	(0x3 << 4)
	#define VIDEO_IN_444_MODE	(0x1 << 9)
#define VIDEO_IN_V_PKCNT		0x24
#define VIDEO_IN_H_CNT			0x34
#define VIDEO_IN_H_CNTEND		0x38
#define VIDEO_IN_H_BACKUP		0x3c
#define VIDEO_IN_REQ_CTRL		0x7c
#define VIDEO_IN_REQ_OUT		0x80
	#define VIDEO_IN_IDX_MODE	(0x1 << 31)
	#define VIDEO_IN_EXT_FILED	(0x1 << 30)
	#define VIDEO_IN_ACT_SEL	(0x1 << 29)
	#define VIDEO_IN_END_SEL	(0x1 << 28)
#define VIDEO_IN_C2_ACTIVE		0xd0
#define VIDEO_IN_CR_ADDR		0xd4

#endif



