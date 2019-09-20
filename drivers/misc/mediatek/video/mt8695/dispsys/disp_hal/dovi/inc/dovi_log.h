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

#ifndef __DISP_DOVI_LOG_H__
#define __DISP_DOVI_LOG_H__

#include <linux/printk.h>

#define DOVI_RET_ERROR			0
#define DOVI_RET_OK				1

extern unsigned int dovi_dbg_level;

#define DOVI_ERROR_LOG (1 << 0)
#define DOVI_FUNC_LOG (1 << 1)
#define DOVI_INFO_LOG (1 << 2)
#define DOVI_RPU_LOG (1 << 3)
#define DOVI_VSVDB_LOG (1 << 4)


#define dovi_printf(fmt, args...) pr_info("[dovi] "fmt, ##args)

#define dovi_error(format, ...) do { \
	if (1) { \
		dovi_printf("error : "format, ##__VA_ARGS__); \
	} \
} while (0)

#define dovi_func_start() do { \
	if (dovi_dbg_level & (DOVI_FUNC_LOG)) { \
		dovi_printf("func| %s start\n", __func__); \
	} \
} while (0)

#define dovi_func_end() do { \
	if (dovi_dbg_level & (DOVI_FUNC_LOG)) { \
		dovi_printf("func| %s end\n", __func__); \
	} \
} while (0)

#define dovi_func() do { \
	if (dovi_dbg_level & (DOVI_FUNC_LOG)) { \
		dovi_printf("func| %s\n", __func__); \
	} \
} while (0)

#define dovi_func_default() do { \
	if (DOVI_FUNC_LOG) { \
		dovi_printf("func| %s\n", __func__); \
	} \
} while (0)

#define dovi_info(format, ...) do { \
	if (dovi_dbg_level & (DOVI_INFO_LOG)) { \
		dovi_printf("info : "format, ##__VA_ARGS__); \
	} \
} while (0)

#define dovi_rpu(format, ...) do { \
	if (dovi_dbg_level & (DOVI_RPU_LOG)) { \
		dovi_printf("rpu : "format, ##__VA_ARGS__); \
	} \
} while (0)

#define dovi_vsvdb(format, ...) do { \
	if (dovi_dbg_level & (DOVI_VSVDB_LOG)) { \
		dovi_printf(""format, ##__VA_ARGS__); \
	} \
} while (0)

#define	dovi_default(format, ...) \
{ \
	dovi_printf(""format, ##__VA_ARGS__); \
}



#define printf(format, ...) dovi_printf(format, ##__VA_ARGS__)

#define rpu_printf(format, ...) dovi_printf("[rpu] "format, ##__VA_ARGS__)
#define dmp_printf(format, ...) dovi_printf("[dmp] "format, ##__VA_ARGS__)
#define rpu_error(format, ...) dovi_error("[rpu] "format, ##__VA_ARGS__)
#define dmp_error(format, ...) dovi_error("[dmp] "format, ##__VA_ARGS__)

#define rpu_default(format, ...) dovi_default("[rpu] "format, ##__VA_ARGS__)
#define dmp_default(format, ...) dovi_default("[dmp] "format, ##__VA_ARGS__)

#define rpu_func dovi_func
#define dmp_func dovi_func



#define assert(value) \
{ \
	if ((value) == 0) { \
	dovi_error("%s,%d value %d\n", __func__, __LINE__, value); \
	} \
	ASSERT(value); \
}


#define UTIL_Printf dovi_printf

#endif
