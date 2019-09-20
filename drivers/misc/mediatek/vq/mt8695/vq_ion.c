/*
 * Copyright (c) 2017 MediaTek Inc.
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

#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include "vq_ion.h"
#include "vq_def.h"

struct ion_client *vq_ion_init(void)
{
	struct ion_client *client = NULL;

	if (!client && g_ion_device)
		client = ion_client_create(g_ion_device, "vq");

	return client;
}

void vq_ion_deinit(struct ion_client *client)
{
	if (!client)
		ion_client_destroy(client);
}

struct ion_handle *vq_ion_import_handle(struct ion_client *client, int fd)
{
	struct ion_handle *handle = NULL;
	struct ion_mm_data mm_data;
	/* If no need Ion support, do nothing! */
	if (fd < 0) {
		pr_notice("[VQ]NO NEED ion support\n");
		return handle;
	}

	if (!client) {
		pr_notice("[VQ]invalid ion client!\n");
		return handle;
	}

	handle = ion_import_dma_buf(client, fd);
	if (IS_ERR_OR_NULL(handle)) {
		pr_notice("[VQ]import ion handle failed!\n");
		return handle;
	}
	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	mm_data.config_buffer_param.kernel_handle = handle;
	mm_data.config_buffer_param.module_id = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.coherent = 0;

	if (ion_kernel_ioctl(client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data))
		pr_notice("[VQ]configure ion buffer failed!\n");

	return handle;

}

size_t vq_ion_phys_mmu_addr(struct ion_client *client, struct ion_handle *handle,
			    unsigned int *mva)
{
	size_t size;

	if (!client) {
		pr_notice("[VQ]invalid ion client!\n");
		return 0;
	}
	if (IS_ERR_OR_NULL(handle))
		return 0;

	ion_phys(client, handle, (ion_phys_addr_t *) mva, &size);

	VQ_INFO("mva[0x%08x] va:[%p]\n", *mva, ion_map_kernel(client, handle));

	return size;

}

void vq_ion_free_handle(struct ion_client *client, struct ion_handle *handle)
{
	if (!client) {
		pr_notice("[VQ]invalid ion client!\n");
		return;
	}
	if (IS_ERR_OR_NULL(handle))
		return;

	ion_free(client, handle);
}
