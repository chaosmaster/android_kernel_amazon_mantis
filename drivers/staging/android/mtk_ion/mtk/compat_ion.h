/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Honghui Zhang <honghui.zhang@mediatek.com>
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

#ifndef __MTK_COMPAT_ION_H__
#define __MTK_COMPAT_ION_H__

/* ion_custom_ioctl */
struct compat_ion_sys_cache_sync_param {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	compat_uptr_t va;
	compat_size_t size;
	compat_uint_t sync_type;
};

struct compat_ion_sys_get_phys_param {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	compat_uint_t phy_addr;
	compat_size_t len;
};

struct compat_ion_dma_param {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	compat_uptr_t va;
	compat_size_t size;
	compat_uint_t dma_type;
	compat_uint_t dma_dir;
};

struct compat_ion_sys_client_name {
	char name[ION_MM_DBG_NAME_LEN];
};

struct compat_ion_sys_get_client_param {
	compat_uint_t client;
};

struct compat_ion_sys_data {
	compat_uint_t sys_cmd;
	union {
		struct compat_ion_sys_cache_sync_param cache_sync_param;
		struct compat_ion_sys_get_phys_param   get_phys_param;
		struct compat_ion_sys_get_client_param get_client_param;
		struct compat_ion_sys_client_name client_name_param;
		struct compat_ion_dma_param dma_param;
	};
};

struct compat_ion_mm_config_buffer_param {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	compat_uint_t module_id;
	compat_uint_t security;
	compat_uint_t coherent;
	compat_uint_t reserve_iova_start;
	compat_uint_t reserve_iova_end;
};

struct compat_ion_mm_buf_debug_info {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	char dbg_name[ION_MM_DBG_NAME_LEN];
	compat_uint_t value1;
	compat_uint_t value2;
	compat_uint_t value3;
	compat_uint_t value4;
};

struct compat_ion_mm_sf_buf_info {
	union {
		compat_int_t handle;
		compat_uptr_t kernel_handle;
	};
	unsigned int info[ION_MM_SF_BUF_INFO_LEN];
};

struct compat_ion_mm_data {
	compat_uint_t mm_cmd;
	union {
		struct compat_ion_mm_config_buffer_param config_buffer_param;
		struct compat_ion_mm_buf_debug_info  buf_debug_info_param;
		struct compat_ion_mm_sf_buf_info sf_buf_info_param;
	};
};

int compat_put_ion_sys_data(struct compat_ion_sys_data __user *data32,
			    struct ion_sys_data __user *data);
int compat_get_ion_sys_data(struct compat_ion_sys_data __user *data32,
			    struct ion_sys_data __user *data);

int compat_get_ion_mm_data(struct compat_ion_mm_data *data32, struct ion_mm_data *data);
int compat_put_ion_mm_data(struct compat_ion_mm_data *data32, struct ion_mm_data *data);

#endif
