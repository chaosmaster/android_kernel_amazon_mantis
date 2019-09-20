#ifndef __DRV_OSD_ION_H__
#define __DRV_OSD_ION_H__

#include <linux/mutex.h>
#include <linux/list.h>

#include "ion.h"
#include "mtk/ion_drv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSD_INVALID_ION_FD	   (-1)
#define OSD_INVALID_FENCE_FD    (-1)
#define OSD_NO_ION_FD ((int)(~0U>>1))

#define OSD_ION_CLIENT_NAME "OSD_ion_client"

#define OSD_FRAME_BUFFER_ALIGN    (1024)

void osd_ion_init(void);
void osd_ion_test(void);
struct ion_handle *osd_ion_import_handle(struct ion_client *client, int fd);
size_t osd_ion_phys_mmu_addr(struct ion_client *client,
			     struct ion_handle *handle, unsigned int *mva);

void osd_ion_free_handle(struct ion_client *client, struct ion_handle *handle);
void osd_ion_cache_flush(struct ion_client *client, struct ion_handle *handle);
ion_phys_addr_t osd_ion_alloc(struct ion_client *client,
			      unsigned int heap_id_mask, size_t size);

extern struct ion_client *osd_ion_client;
extern struct ion_handle *osd_handle;

extern struct ion_client *osd_ion_client;

#ifdef __cplusplus
}
#endif
#endif
