#define LOG_TAG "OSD_ION"

#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>

/*#include "mtk_sync.h"*/
/*#include "mtkfb_ion.h"*/
#include "mtkfb.h"
#include "disp_osd_ion.h"
#include "disp_osd_log.h"


struct ion_client *osd_ion_client;
/*struct ion_handle *osd_handle; */

void osd_ion_init(void)
{
	if (!osd_ion_client && g_ion_device)
		osd_ion_client = ion_client_create(g_ion_device, OSD_ION_CLIENT_NAME);

	if (!osd_ion_client) {
		OSD_LOG_E("create ion client failed!\n");
		return;
	}

	OSDDBG("create ion client 0x%p\n", osd_ion_client);
}

/**
 * Import ion handle and configure this buffer
 * @client
 * @fd ion shared fd
 * @return ion handle
 */

struct ion_handle *osd_ion_import_handle(struct ion_client *client, int fd)
{
	struct ion_handle *handle = NULL;
	struct ion_mm_data mm_data;
	/* If no need Ion support, do nothing! */
	if (fd == OSD_NO_ION_FD) {
		OSD_LOG_E("NO NEED ion support\n");
		return handle;
	}

	if (!client) {
		OSD_LOG_E("invalid ion client!\n");
		return handle;
	}
#if 0
	if (fd == MTK_FB_INVALID_ION_FD) {
		OSD_LOG_E("invalid ion fd!\n");
		return handle;
	}
#endif

	handle = ion_import_dma_buf(client, fd);
	if (!handle) {
		OSD_LOG_E("import ion handle failed!\n");
		return handle;
	}
	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	mm_data.config_buffer_param.kernel_handle = handle;
	mm_data.config_buffer_param.module_id = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.coherent = 0;

	if (ion_kernel_ioctl(client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data)) {
		OSD_LOG_E("configure ion buffer failed!\n");
		return NULL;
	}

	OSD_PRINTF(OSD_FLOW_LOG, "import ion handle fd=%d,hnd=0x%p\n", fd, handle);

	return handle;
}

void osd_ion_free_handle(struct ion_client *client, struct ion_handle *handle)
{
	if (!client) {
		OSD_LOG_E("invalid ion client!\n");
		return;
	}
	if (!handle) {
		OSDDBG("invalid ion hanlde!\n");
		return;
	}
	ion_free(client, handle);
	OSDDBG("free ion handle 0x%p\n", handle);
}

size_t osd_ion_phys_mmu_addr(struct ion_client *client,
			     struct ion_handle *handle, unsigned int *mva)
{
	size_t size;

	if (!client) {
		OSD_LOG_E("invalid ion client!\n");
		return 0;
	}
	if (!handle) {
		OSD_LOG_E("invalid ion hanlde!\n");
		return 0;
	}
	ion_phys(client, handle, (ion_phys_addr_t *) mva, &size);
	OSD_PRINTF(OSD_FLOW_LOG, "alloc mmu addr handle = 0x%p,mva = 0x%08x\n", handle,
		   (unsigned int)*mva);
	return size;
}
