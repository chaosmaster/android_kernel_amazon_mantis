#include "mtk/ion_drv.h"
#include "mdp_buffer.h"
#include "mdp_log.h"


struct mdp_temp_buffer_global_struct g_mdp_temp_buffer_info;

void mdp_temp_buffer_init(void)
{
	int i = 0;

	for (i = 0; i < MAX_TEMP_BUFFER_NODE; i++)
		mutex_init(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
	g_mdp_temp_buffer_info.client = ion_client_create(g_ion_device, "mdp");
}

void mdp_temp_buffer_destroy(void)
{
	int i = 0;

	for (i = 0; i < MAX_TEMP_BUFFER_NODE; i++)
		mutex_destroy(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
	ion_client_destroy(g_mdp_temp_buffer_info.client);
}

bool mdp_temp_buffer_find_free_buffer(int32_t *p_index)
{
	bool buffer_finded = false;
	int i = 0;

	for (i = 0; i < MAX_TEMP_BUFFER_NODE; i++) {
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
		if (g_mdp_temp_buffer_info.buffer_pool[i].state == MDP_TEMP_BUFFER_STATE_FREE) {
			mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
			break;
		}
		mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
	}

	if (i == MAX_TEMP_BUFFER_NODE)
		buffer_finded = false;
	else {
		buffer_finded = true;
		*p_index = i;
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
		g_mdp_temp_buffer_info.buffer_pool[i].state = MDP_TEMP_BUFFER_STATE_USED_BY_IMGRESZ;
		mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
	}

	return buffer_finded;
}

enum MDP_TASK_STATUS mdp_temp_buffer_allocate_buffer(void)
{
	int i = 0;
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	size_t buffer_len;
	bool need_allocate = false;

	for (i = 0; i < MAX_TEMP_BUFFER_NODE; i++) {
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
		/* check if buffer are released to avoid buffer leak */
		if (g_mdp_temp_buffer_info.buffer_pool[i].state == MDP_TEMP_BUFFER_STATE_RELEASED) {
			mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
			need_allocate = true;
			break;
		}
	}

	if (need_allocate == false)
		return MDP_TASK_STATUS_OK;

	/* allocate 3 temp buffer */
	for (i = 0; i < MAX_TEMP_BUFFER_NODE; i++) {
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
		if (g_mdp_temp_buffer_info.buffer_pool[i].state != MDP_TEMP_BUFFER_STATE_RELEASED) {
			mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
			continue;
		}
		mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);

		g_mdp_temp_buffer_info.buffer_pool[i].handle = ion_alloc(g_mdp_temp_buffer_info.client,
				TEMP_BUFFER_SIZE, 0, ION_HEAP_TYPE_MULTIMEDIA, 0);

		if (IS_ERR(g_mdp_temp_buffer_info.buffer_pool[i].handle)) {
			MDP_ERR("allocate temp buffer failed:%d\n", i);
			ion_free(g_mdp_temp_buffer_info.client, g_mdp_temp_buffer_info.buffer_pool[i].handle);
			status = MDP_TASK_STATUS_ALLOCATE_TEMP_BUFFER_FAIL;
			break;
		}
		/* allocate success */
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);
		g_mdp_temp_buffer_info.buffer_pool[i].state = MDP_TEMP_BUFFER_STATE_FREE;
		mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[i].lock);

		ion_phys(g_mdp_temp_buffer_info.client,
					g_mdp_temp_buffer_info.buffer_pool[i].handle,
					&g_mdp_temp_buffer_info.buffer_pool[i].pa,
					&buffer_len);
		if (buffer_len != TEMP_BUFFER_SIZE) {
			MDP_ERR("allocated buffer size is not 0x%x acture size %zu\n", TEMP_BUFFER_SIZE, buffer_len);
			status = MDP_TASK_STATUS_ALLOCATE_TEMP_BUFFER_SIZE_NOT_MATCH;
			break;
		}

	}
	return status;
}

/*
** get temp buffer from buffer pool
*/
enum MDP_TASK_STATUS mdp_temp_buffer_get_buffer(struct mdp_temp_buffer_struct **ppBuffer)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	int buffer_index = 0;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);

	do {
		/* allocate temp buffer if not allocated */
		status = mdp_temp_buffer_allocate_buffer();
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("allocate temp buffer failed\n");
			break;
		}
		add_wait_queue(&g_mdp_temp_buffer_info.mdp_temp_buffer_wait_buffer_free_queue, &wait);
		/* wait available temp buffer */
		while (1) {
			long timeout = msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS);

			if (mdp_temp_buffer_find_free_buffer(&buffer_index))
				break;
			timeout = wait_woken(&wait, TASK_INTERRUPTIBLE, timeout);
			if (timeout == 0) {
				MDP_ERR("wait free temp buffer timeout\n");
				status = MDP_TASK_STATUS_WAIT_FREE_TEMP_BUFFER_TIMEOUT;
				break;
			}
		}
		remove_wait_queue(&global_mdp_task.mdp_core_wait_imgresz_hw_idle_queue, &wait);

		if (status == MDP_TASK_STATUS_WAIT_FREE_TEMP_BUFFER_TIMEOUT)
			break;
		/* get temp buffer index buffer_index for use */
		*ppBuffer = &g_mdp_temp_buffer_info.buffer_pool[buffer_index];
	} while (0);

	if (status != MDP_TASK_STATUS_OK)
		*ppBuffer = NULL;

	return status;
}

/*
** release buffer to buffer pool
*/
enum MDP_TASK_STATUS mdp_temp_buffer_put_buffer(struct mdp_temp_buffer_struct *pBuffer)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	int buffer_index = 0;

	if (pBuffer == NULL)
		return MDP_TASK_STATUS_OK;
	do {
		buffer_index = (pBuffer - &g_mdp_temp_buffer_info.buffer_pool[0]);
		if (buffer_index >= MAX_TEMP_BUFFER_NODE) {
			MDP_ERR("wrong pTask->p_temp_buffer:%p, buffer_pool:%p\n",
				pBuffer, &g_mdp_temp_buffer_info.buffer_pool[0]);
			status = MDP_TASK_STATUS_BUFFER_POOL_OUTOF_ARRAY_INDEX;
			break;
		}
		mutex_lock(&g_mdp_temp_buffer_info.buffer_pool[buffer_index].lock);
		pBuffer->state = MDP_TEMP_BUFFER_STATE_FREE;
		mutex_unlock(&g_mdp_temp_buffer_info.buffer_pool[buffer_index].lock);
		wake_up(&g_mdp_temp_buffer_info.mdp_temp_buffer_wait_buffer_free_queue);
	} while (0);

	return status;
}

/* convert ion fd to PA */
enum MDP_TASK_STATUS mdp_buffer_convert_ion_fd(struct mdp_buffer_struct *pBuffer, struct mdp_ion_struct *pIonHandle)
{
	struct ion_handle *handle = NULL;
	struct ion_handle *handle2 = NULL;
	struct ion_mm_data mm_data = {0};
	ion_phys_addr_t mva = 0;
	ion_phys_addr_t mva2 = 0;
	size_t buffer_size;
	bool isOneChannel = false;
	int fd;

	isOneChannel = ((pBuffer->buffer_info.fd) & 0xFFFF0000) == 0;
#if 1
	fd = (pBuffer->buffer_info.fd & 0x0000FFFF);
	handle = ion_import_dma_buf(g_mdp_temp_buffer_info.client, fd);
	if (IS_ERR_OR_NULL(handle)) {
		MDP_ERR("import ion handle failed:fd1[0x%x] client:%p\n",
			pBuffer->buffer_info.fd, g_mdp_temp_buffer_info.client);
		return MDP_TASK_STATUS_IMPORT_ION_FD_FAIL;
	}
#else
	handle = (struct ion_handle *)pBuffer->buffer_info.pHanle1;
#endif

	mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
	mm_data.config_buffer_param.kernel_handle = handle;
	mm_data.config_buffer_param.module_id = 0;
	mm_data.config_buffer_param.security = 0;
	mm_data.config_buffer_param.coherent = 0;

	if (ion_kernel_ioctl(g_mdp_temp_buffer_info.client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data)) {
		MDP_ERR("convert ion fd1:0x%x fail\n", pBuffer->buffer_info.fd);
		return MDP_TASK_STATUS_CONVET_HANDLE_2_PA_FAIL;
	}

	ion_phys(g_mdp_temp_buffer_info.client, handle, &mva, &buffer_size);

	if (!isOneChannel) {
		/* 2 channel */
		#if 1
		handle2 = ion_import_dma_buf(g_mdp_temp_buffer_info.client,
			(pBuffer->buffer_info.fd & 0xFFFF0000) >> 16);
		if (IS_ERR_OR_NULL(handle2)) {
			MDP_ERR("import ion handle2 failed:fd[0x%x]!\n", pBuffer->buffer_info.fd);
			return MDP_TASK_STATUS_IMPORT_ION_FD_FAIL;
		}
		#else
		handle2  = (struct ion_handle *)pBuffer->buffer_info.pHanle2;
		#endif

		memset(&mm_data, 0, sizeof(mm_data));
		mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
		mm_data.config_buffer_param.kernel_handle = handle2;
		mm_data.config_buffer_param.module_id = 0;
		mm_data.config_buffer_param.security = 0;
		mm_data.config_buffer_param.coherent = 0;

		if (ion_kernel_ioctl(g_mdp_temp_buffer_info.client, ION_CMD_MULTIMEDIA, (unsigned long)&mm_data)) {
			MDP_ERR("convert ion fd2:0x%x fail\n", pBuffer->buffer_info.fd);
			return MDP_TASK_STATUS_CONVET_HANDLE_2_PA_FAIL;
		}
		ion_phys(g_mdp_temp_buffer_info.client, handle2, &mva2, &buffer_size);
	}

	if (pBuffer->buffer_info.memory_type != DP_MEMORY_SECURE) {
		/* normal buffer: convert mva */
		if (isOneChannel) {
			pBuffer->y_buffer_address = mva + pBuffer->buffer_info.y_offset;
			if (pBuffer->buffer_info.cb_offset)
				pBuffer->u_buffer_address = mva + pBuffer->buffer_info.cb_offset;
			if (pBuffer->buffer_info.cr_offset)
				pBuffer->v_buffer_address = mva + pBuffer->buffer_info.cr_offset;
			if (pBuffer->buffer_info.ylen_offset)
				pBuffer->ufo_ylen_buffer_address = mva + pBuffer->buffer_info.ylen_offset;
			if (pBuffer->buffer_info.clen_offset)
				pBuffer->ufo_clen_buffer_address = mva + pBuffer->buffer_info.clen_offset;
		} else {
			pBuffer->y_buffer_address = mva + pBuffer->buffer_info.y_offset;
			pBuffer->u_buffer_address = mva2 + pBuffer->buffer_info.cb_offset;
			if (pBuffer->buffer_info.cr_offset)
				pBuffer->v_buffer_address = mva2 + pBuffer->buffer_info.cr_offset;
			if (pBuffer->buffer_info.ylen_offset)
				pBuffer->ufo_ylen_buffer_address = mva + pBuffer->buffer_info.ylen_offset;
			if (pBuffer->buffer_info.clen_offset)
				pBuffer->ufo_clen_buffer_address = mva2 + pBuffer->buffer_info.clen_offset;
		}
	} else {
		/* secure buffer */
		pBuffer->buffer_info.secureHandle = (uint64_t)mva;
		pBuffer->y_buffer_address = pBuffer->buffer_info.y_offset;
		pBuffer->u_buffer_address = pBuffer->buffer_info.cb_offset;
		pBuffer->v_buffer_address = pBuffer->buffer_info.cr_offset;
		pBuffer->ufo_ylen_buffer_address = pBuffer->buffer_info.ylen_offset;
		pBuffer->ufo_clen_buffer_address = pBuffer->buffer_info.clen_offset;
	}

	pIonHandle->ionHandle1 = handle;
	pIonHandle->ionHandle2 = handle2;

	return MDP_TASK_STATUS_OK;
}

enum MDP_TASK_STATUS mdp_buffer_release_ion_handle(struct mdp_buffer_struct *pBuffer, struct mdp_ion_struct *pIonHandle)
{
	bool isOneChannel = ((pBuffer->buffer_info.fd) & 0xFFFF0000) == 0;

	if (IS_ERR_OR_NULL(pIonHandle->ionHandle1)) {
		MDP_ERR("invalid release ion handle:%p\n", pIonHandle->ionHandle1);
		return MDP_TASK_STATUS_INVALID_PARAM;
	}
	ion_free(g_mdp_temp_buffer_info.client, pIonHandle->ionHandle1);

	if (!isOneChannel) {
		if (IS_ERR_OR_NULL(pIonHandle->ionHandle2)) {
			MDP_ERR("invalid release ion handle2:%p\n", pIonHandle->ionHandle2);
			return MDP_TASK_STATUS_INVALID_PARAM;
		}
		ion_free(g_mdp_temp_buffer_info.client, pIonHandle->ionHandle2);
	}
	return MDP_TASK_STATUS_OK;
}


enum MDP_TASK_STATUS mdp_buffer_share_ion_fd(struct mdp_buffer_struct *pBuffer, struct mdp_ion_struct *pIonHandle)
{
	bool isOneChannel = ((pBuffer->buffer_info.fd) & 0xFFFF0000) == 0;
	struct ion_handle *handle = NULL;
	struct ion_handle *handle2 = NULL;
#if 0
	int fd1 = 0;
	int fd2 = 0;
#endif

	handle = ion_import_dma_buf(g_mdp_temp_buffer_info.client, (pBuffer->buffer_info.fd & 0x0000FFFF));
	if (IS_ERR_OR_NULL(handle)) {
		MDP_ERR("mdp_buffer_share_ion_fd import ion handle failed:fd1[0x%x] client:%p\n",
			pBuffer->buffer_info.fd, g_mdp_temp_buffer_info.client);
		return MDP_TASK_STATUS_IMPORT_ION_FD_FAIL;
	}

	if (!isOneChannel) {
		handle2 = ion_import_dma_buf(g_mdp_temp_buffer_info.client,
			(pBuffer->buffer_info.fd & 0xFFFF0000) >> 16);
		if (IS_ERR_OR_NULL(handle2)) {
			MDP_ERR("mdp_buffer_share_ion_fd import ion handle failed:fd2[0x%x] client:%p\n",
				pBuffer->buffer_info.fd, g_mdp_temp_buffer_info.client);
			return MDP_TASK_STATUS_IMPORT_ION_FD_FAIL;
		}
	}
#if 0
	fd1 = ion_share_dma_buf_fd(g_mdp_temp_buffer_info.client, handle);
	if (!isOneChannel)
		fd2 = ion_share_dma_buf_fd(g_mdp_temp_buffer_info.client, handle2);
#endif
	pIonHandle->ionHandle1 = handle;
	pIonHandle->ionHandle2 = handle2;

	return MDP_TASK_STATUS_OK;

}
