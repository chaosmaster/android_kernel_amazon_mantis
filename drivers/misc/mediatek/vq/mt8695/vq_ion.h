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

#ifndef _VQ_ION_H_
#define _VQ_ION_H_

#include <linux/types.h>

#include "ion_drv.h"

struct ion_client *vq_ion_init(void);
void vq_ion_deinit(struct ion_client *client);
struct ion_handle *vq_ion_import_handle(struct ion_client *client, int fd);
size_t vq_ion_phys_mmu_addr(struct ion_client *client, struct ion_handle *handle,
			    unsigned int *mva);
void vq_ion_free_handle(struct ion_client *client, struct ion_handle *handle);


#endif				/* _VQ_ION_H_ */
