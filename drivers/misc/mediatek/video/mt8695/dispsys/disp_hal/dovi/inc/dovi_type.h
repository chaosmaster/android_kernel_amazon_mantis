/*
 * Copyright (C) 2016 MediaTek Inc.
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

#ifndef __DOVI_TYPE_H__
#define __DOVI_TYPE_H__

#include "linux/types.h"

#ifndef BOOL
#define BOOL     bool
#endif

#ifndef VOID
#define VOID     void
#endif

#ifndef CHAR
#define CHAR     char
#endif

#ifndef INT8
#define INT8     char
#endif

#ifndef INT16
#define INT16    int16_t
#endif

#ifndef INT32
#define INT32    int32_t
#endif

#ifndef INT64
#define INT64    int64_t
#endif

#ifndef UCHAR
#define UCHAR    unsigned char
#endif

#ifndef UINT8
#define UINT8    unsigned char
#endif

#ifndef UINT16
#define UINT16   uint16_t
#endif

#ifndef UINT32
#define UINT32   uint32_t
#endif

#ifndef UINT64
#define UINT64   uint64_t
#endif

#ifndef TRUE
#define TRUE   true
#endif

#ifndef FALSE
#define FALSE   false
#endif

enum dovi_status {
	DOVI_STATUS_OK = 0,
	DOVI_STATUS_ERROR = 1,
};

#endif

