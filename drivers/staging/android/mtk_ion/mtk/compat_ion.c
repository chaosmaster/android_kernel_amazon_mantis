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
#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "ion.h"
#include "ion_drv.h"
#include "compat_ion.h"

static int compat_get_ion_mm_config_buffer_param(
			struct compat_ion_mm_config_buffer_param __user *data32,
			struct ion_mm_config_buffer_param __user *data)
{
	compat_ulong_t handle;
	compat_uint_t module_id;
	compat_uint_t security;
	compat_uint_t coherent;
	compat_uint_t iova_start;
	compat_uint_t iova_end;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	err |= get_user(module_id, &data32->module_id);
	err |= put_user(module_id, &data->module_id);
	err |= get_user(security, &data32->security);
	err |= put_user(security, &data->security);
	err |= get_user(coherent, &data32->coherent);
	err |= put_user(coherent, &data->coherent);
	err |= get_user(iova_start, &data32->reserve_iova_start);
	err |= put_user(iova_start, &data->reserve_iova_start);
	err |= get_user(iova_end, &data32->reserve_iova_end);
	err |= put_user(iova_end, &data->reserve_iova_end);

	return err;
}

static int compat_get_ion_mm_buf_debug_info_set(
			struct compat_ion_mm_buf_debug_info __user *data32,
			struct ion_mm_buf_debug_info __user *data)
{
	compat_ulong_t handle;
	char dbg_name;
	compat_uint_t value1;
	compat_uint_t value2;
	compat_uint_t value3;
	compat_uint_t value4;

	int i, err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	for (i = 0; i < ION_MM_DBG_NAME_LEN; i++) {
		err = get_user(dbg_name, &data32->dbg_name[i]);
		err |= put_user(dbg_name, &data->dbg_name[i]);
	}
	err |= get_user(value1, &data32->value1);
	err |= put_user(value1, &data->value1);
	err |= get_user(value2, &data32->value2);
	err |= put_user(value2, &data->value2);
	err |= get_user(value3, &data32->value3);
	err |= put_user(value3, &data->value3);
	err |= get_user(value4, &data32->value4);
	err |= put_user(value4, &data->value4);

	return err;
}

static int compat_get_ion_mm_buf_debug_info(
			struct compat_ion_mm_buf_debug_info __user *data32,
			struct ion_mm_buf_debug_info __user *data)
{
	compat_ulong_t handle;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);

	return err;
}

static int compat_put_ion_mm_buf_debug_info(
			struct compat_ion_mm_buf_debug_info __user *data32,
			struct ion_mm_buf_debug_info __user *data)
{
	compat_ulong_t handle;
	char dbg_name;
	compat_uint_t value1;
	compat_uint_t value2;
	compat_uint_t value3;
	compat_uint_t value4;

	int i, err = 0;

	err = get_user(handle, &data->handle);
	err |= put_user(handle, &data32->handle);
	for (i = 0; i < ION_MM_DBG_NAME_LEN; i++) {
		err = get_user(dbg_name, &data->dbg_name[i]);
		err |= put_user(dbg_name, &data32->dbg_name[i]);
	}
	err |= get_user(value1, &data->value1);
	err |= put_user(value1, &data32->value1);
	err |= get_user(value2, &data->value2);
	err |= put_user(value2, &data32->value2);
	err |= get_user(value3, &data->value3);
	err |= put_user(value3, &data32->value3);
	err |= get_user(value4, &data->value4);
	err |= put_user(value4, &data32->value4);

	return err;
}

static int compat_get_ion_mm_sf_buf_info_set(
			struct compat_ion_mm_sf_buf_info __user *data32,
			struct ion_mm_sf_buf_info __user *data)
{
	compat_ulong_t handle;
	compat_uint_t info;

	int i, err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	for (i = 0; i < ION_MM_SF_BUF_INFO_LEN; i++) {
		err |= get_user(info, &data32->info[i]);
		err |= put_user(info, &data->info[i]);
	}

	return err;
}

static int compat_get_ion_mm_sf_buf_info(
			struct compat_ion_mm_sf_buf_info __user *data32,
			struct ion_mm_sf_buf_info __user *data)
{
	compat_ulong_t handle;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);

	return err;
}

static int compat_put_ion_mm_sf_buf_info(
			struct compat_ion_mm_sf_buf_info __user *data32,
			struct ion_mm_sf_buf_info __user *data)
{
	compat_ulong_t handle;
	compat_uint_t info;

	int i, err;

	err = get_user(handle, &data->handle);
	err |= put_user(handle, &data32->handle);

	for (i = 0; i < ION_MM_SF_BUF_INFO_LEN; i++) {
		err |= get_user(info, &data->info[i]);
		err |= put_user(info, &data32->info[i]);
	}

	return err;
}

int compat_get_ion_mm_data(struct compat_ion_mm_data *data32, struct ion_mm_data *data)
{
	compat_uint_t mm_cmd;

	int err;

	err = get_user(mm_cmd, &data32->mm_cmd);
	err |= put_user(mm_cmd, &data->mm_cmd);

	switch (mm_cmd) {
	case ION_MM_CONFIG_BUFFER:
	case ION_MM_CONFIG_BUFFER_EXT:
	{
		err |= compat_get_ion_mm_config_buffer_param(&data32->config_buffer_param, &data->config_buffer_param);
		break;
	}
	case ION_MM_SET_DEBUG_INFO:
	{
		err |= compat_get_ion_mm_buf_debug_info_set(&data32->buf_debug_info_param, &data->buf_debug_info_param);
		break;
	}
	case ION_MM_GET_DEBUG_INFO:
	{
		err |= compat_get_ion_mm_buf_debug_info(&data32->buf_debug_info_param, &data->buf_debug_info_param);
		break;
	}
	case ION_MM_SET_SF_BUF_INFO:
	{
		err |= compat_get_ion_mm_sf_buf_info_set(&data32->sf_buf_info_param, &data->sf_buf_info_param);
		break;
	}
	case ION_MM_GET_SF_BUF_INFO:
	{
		err |= compat_get_ion_mm_sf_buf_info(&data32->sf_buf_info_param, &data->sf_buf_info_param);
		break;
	}
	}

	return err;
}

int compat_put_ion_mm_data(struct compat_ion_mm_data *data32, struct ion_mm_data *data)
{
	compat_uint_t mm_cmd;

	int err = 0;

	err = get_user(mm_cmd, &data->mm_cmd);
	err |= put_user(mm_cmd, &data32->mm_cmd);

	switch (mm_cmd) {
	case ION_MM_GET_DEBUG_INFO:
	{
		err |= compat_put_ion_mm_buf_debug_info(&data32->buf_debug_info_param, &data->buf_debug_info_param);
		break;
	}
	case ION_MM_GET_SF_BUF_INFO: {
		err |= compat_put_ion_mm_sf_buf_info(&data32->sf_buf_info_param, &data->sf_buf_info_param);
		break;
	}
	default:
		err = 0;
	}

	return err;
}

static int compat_get_ion_sys_cache_sync_param(
			struct compat_ion_sys_cache_sync_param __user *data32,
			struct ion_sys_cache_sync_param __user *data)
{
	compat_int_t handle;
	compat_uptr_t va;
	compat_size_t size;
	compat_uint_t sync_type;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	err |= get_user(va, &data32->va);
	err |= put_user(compat_ptr(va), &data->va);
	err |= get_user(size, &data32->size);
	err |= put_user(size, &data->size);
	err |= get_user(sync_type, &data32->sync_type);
	err |= put_user(sync_type, &data->sync_type);

	return err;
}

static int compat_get_ion_sys_dma_op_param(
			struct compat_ion_dma_param __user *data32,
			struct ion_dma_param __user *data)
{
	compat_int_t handle;
	compat_uptr_t va;
	compat_size_t size;
	compat_uint_t dma_type;
	compat_uint_t dma_dir;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	err |= get_user(va, &data32->va);
	err |= put_user(compat_ptr(va), &data->va);
	err |= get_user(size, &data32->size);
	err |= put_user(size, &data->size);
	err |= get_user(dma_type, &data32->dma_type);
	err |= put_user(dma_type, &data->dma_type);
	err |= get_user(dma_dir, &data32->dma_dir);
	err |= put_user(dma_dir, &data->dma_dir);

	return err;
}

static int compat_get_ion_sys_get_phys_param(
			struct compat_ion_sys_get_phys_param __user *data32,
			struct ion_sys_get_phys_param __user *data)
{
	compat_int_t handle;
	compat_uint_t phy_addr;
	compat_size_t len;

	int err;

	err = get_user(handle, &data32->handle);
	err |= put_user(handle, &data->handle);
	err |= get_user(phy_addr, &data32->phy_addr);
	err |= put_user(phy_addr, &data->phy_addr);
	err |= get_user(len, &data32->len);
	err |= put_user(len, &data->len);

	return err;
}

static int compat_put_ion_sys_get_phys_param(
			struct compat_ion_sys_get_phys_param __user *data32,
			struct ion_sys_get_phys_param __user *data)
{
	compat_int_t handle;
	compat_uint_t phy_addr;
	compat_size_t len;

	int err = 0;

	err = get_user(handle, &data->handle);
	err |= put_user(handle, &data32->handle);
	err |= get_user(phy_addr, &data->phy_addr);
	err |= put_user(phy_addr, &data32->phy_addr);
	err |= get_user(len, &data->len);
	err |= put_user(len, &data32->len);

	return err;
}

static int compat_get_ion_sys_client_name(
			struct compat_ion_sys_client_name __user *data32,
			struct ion_sys_client_name __user *data)
{
	char name;

	int i, err;

	for (i = 0; i < ION_MM_DBG_NAME_LEN; i++) {
		err = get_user(name, &data32->name[i]);
		err |= put_user(name, &data->name[i]);
	}

	return err;
}

static int compat_get_ion_sys_get_client_param(
			struct compat_ion_sys_get_client_param __user *data32,
			struct ion_sys_get_client_param __user *data)
{
	compat_uint_t client;

	int err;

	err = get_user(client, &data32->client);
	err |= put_user(client, &data->client);

	return err;
}

static int compat_put_ion_sys_get_client_param(
			struct compat_ion_sys_get_client_param __user *data32,
			struct ion_sys_get_client_param __user *data)
{
	compat_uint_t client;

	int err = 0;

	err = get_user(client, &data->client);
	err |= put_user(client, &data32->client);

	return err;
}

int compat_get_ion_sys_data(struct compat_ion_sys_data __user *data32,
			    struct ion_sys_data __user *data)
{
	compat_uint_t sys_cmd;

	int err;

	err = get_user(sys_cmd, &data32->sys_cmd);
	err |= put_user(sys_cmd, &data->sys_cmd);
	switch (sys_cmd) {
	case ION_SYS_CACHE_SYNC:
	{
		err |= compat_get_ion_sys_cache_sync_param(&data32->cache_sync_param, &data->cache_sync_param);
		break;
	}
	case ION_SYS_GET_PHYS:
	{
		err |= compat_get_ion_sys_get_phys_param(&data32->get_phys_param, &data->get_phys_param);
		break;
	}
	case ION_SYS_GET_CLIENT:
	{
		err |= compat_get_ion_sys_get_client_param(&data32->get_client_param, &data->get_client_param);
		break;
	}
	case ION_SYS_SET_CLIENT_NAME:
	{
		err |= compat_get_ion_sys_client_name(&data32->client_name_param, &data->client_name_param);
		break;
	}
	case ION_SYS_DMA_OP:
	{
		err |= compat_get_ion_sys_dma_op_param(&data32->dma_param, &data->dma_param);
		break;
	}
	}

	return err;
}

int compat_put_ion_sys_data(struct compat_ion_sys_data __user *data32,
			    struct ion_sys_data __user *data)
{
	compat_uint_t sys_cmd;

	int err = 0;

	err = get_user(sys_cmd, &data->sys_cmd);
	err |= put_user(sys_cmd, &data32->sys_cmd);

	switch (sys_cmd) {
	case ION_SYS_GET_PHYS:
	{
		err |= compat_put_ion_sys_get_phys_param(&data32->get_phys_param, &data->get_phys_param);
		break;
	}
	case ION_SYS_GET_CLIENT:
	{
		err |= compat_put_ion_sys_get_client_param(&data32->get_client_param, &data->get_client_param);
		break;
	}
	default:
		err = 0;
	}

	return err;
}

