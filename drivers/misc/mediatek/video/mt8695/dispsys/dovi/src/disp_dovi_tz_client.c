#define LOG_TAG "DOVI_TZ_CLIENT"

#include <kree/mem.h>
#include <kree/system.h>
#include <linux/slab.h>
#include "disp_info.h"
#include "dovi_type.h"
#include "dovi_log.h"
#include "disp_dovi_control_if.h"
#include "disp_dovi_tz_client.h"
#include "disp_hw_log.h"

struct dovi_share_memory_info_t *dovi_share_mem;

#if DOVI_TZ_OK

#include <tz_cross/ta_mem.h>
#include <tz_cross/trustzone.h>
#include "tz_cross/ta_mem.h"

static KREE_SESSION_HANDLE dovi_tz_session;
static KREE_SESSION_HANDLE dovi_tz_mem_session;
static KREE_SHAREDMEM_HANDLE dovi_tz_mem_handle;

static bool dovi_sec_share_mem_inited;
static bool dovi_sec_md_parser_inited;
static bool dovi_sec_cp_test_inited;

static enum dovi_status dovi_sec_create_session(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	TZ_RESULT ret;

	do {
		if (dovi_tz_session == 0) {
			ret = KREE_CreateSession(DOVI_TA_UUID, &dovi_tz_session);
			if (ret != TZ_RESULT_SUCCESS) {
				dovi_error("create dovi_tz_session fail:%d\n", ret);
				status = DOVI_STATUS_ERROR;
				break;
			}
		}
	} while (0);

	if (status != DOVI_STATUS_OK)
		dovi_error("create session fail:%d\n", status);
	else {
		dovi_info("create session dovi_tz_session[0x%X]\n",
			    dovi_tz_session);
	}

	return status;
}

static enum dovi_status dovi_sec_create_share_memory(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);

	if (dovi_share_mem) {
		dovi_error("share memory already created %p size %u\n",
			   dovi_share_mem, size);
		return DOVI_STATUS_OK;
	}

	dovi_share_mem = kmalloc(size, GFP_KERNEL);
	if (!dovi_share_mem) {
		dovi_error("share memory already fail %p size %u\n",
			   dovi_share_mem, size);
		return DOVI_STATUS_OK;
	}

	dovi_info("share memory create 0x%p size %u\n",
			   dovi_share_mem, size);

	memset((void *)dovi_share_mem, 0, size);
#if 0
	status = dovi_sec_service_call(DOVI_TZ_CALL_CMD_INIT_SHARE_MEMORY,
				       DOVI_TZ_CALL_DIR_INOUT, (void *)dovi_share_mem, size);
#endif

	if (status != DOVI_STATUS_OK)
		dovi_error("create share memory fail size %d 0x%p\n",
			   size, dovi_share_mem);
	else
		dovi_info("create share memory ok size %d 0x%p\n",
			    size, dovi_share_mem);

	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_create_share_mem_session(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	TZ_RESULT ret;

	do {
		if (dovi_tz_mem_session == 0) {
			ret = KREE_CreateSession(TZ_TA_MEM_UUID, &dovi_tz_mem_session);
			if (ret != TZ_RESULT_SUCCESS) {
				dovi_error("create dovi_tz_mem_session fail:%d\n", ret);
				status = DOVI_STATUS_ERROR;
				break;
			}
		}
	} while (0);

	if (status != DOVI_STATUS_OK)
		dovi_error("create session fail:%d\n", status);
	else {
		dovi_info("create session dovi_tz_mem_session[0x%X]\n",
			     dovi_tz_mem_session);
	}

	return status;
}

enum dovi_status dovi_sec_create_share_mem_handle(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	KREE_SHAREDMEM_PARAM dovi_param;
	TZ_RESULT ret;

	dovi_param.buffer = (void *)dovi_share_mem;
	dovi_param.size = sizeof(struct dovi_share_memory_info_t);

	do {
		if (dovi_tz_mem_handle == 0) {
			ret = KREE_RegisterSharedmem(dovi_tz_mem_session,
			&dovi_tz_mem_handle,
			&dovi_param);
			if (ret != TZ_RESULT_SUCCESS) {
				dovi_error("create dovi_tz_mem_handle fail:%d\n", ret);
				status = DOVI_STATUS_ERROR;
				break;
			}
		}
	} while (0);

	if (status != DOVI_STATUS_OK)
		dovi_error("create dovi_tz_mem_handle fail:%d\n", status);
	else {
		dovi_info("create dovi_tz_mem_handle[0x%X] share mem pointer 0x%p\n",
			     dovi_tz_mem_handle, (void *)dovi_share_mem);
	}

	return status;
}

static enum dovi_status dovi_sec_map_direction(enum DOVI_TZ_CALL_DIR direct, TZ_PARAM_TYPES *pType)
{
	switch (direct) {
	case DOVI_TZ_CALL_DIR_MEM_INPUT:
		*pType = TZPT_MEM_INPUT;
		break;
	case DOVI_TZ_CALL_DIR_MEM_OUTPUT:
		*pType = TZPT_MEM_OUTPUT;
		break;
	case DOVI_TZ_CALL_DIR_MEM_INOUT:
		*pType = TZPT_MEM_INOUT;
		break;
	case DOVI_TZ_CALL_DIR_MEMREF_INPUT:
		*pType = TZPT_MEMREF_INPUT;
		break;
	case DOVI_TZ_CALL_DIR_MEMREF_OUTPUT:
		*pType = TZPT_MEMREF_OUTPUT;
		break;
	case DOVI_TZ_CALL_DIR_MEMREF_INOUT:
		*pType = TZPT_MEMREF_INOUT;
		break;
	default:
		dovi_error("invalid service call direction:%d\n", direct);
		return DOVI_STATUS_ERROR;
	}
	return DOVI_STATUS_OK;
}


enum dovi_status dovi_sec_service_call(enum DOVI_TZ_CALL_CMD cmd,
				       enum DOVI_TZ_CALL_DIR direct,
				       void *buffer,
				       uint32_t size)
{
	unsigned int paramTypes = 0;
	MTEEC_PARAM dovi_param[4];
	TZ_PARAM_TYPES type;
	TZ_RESULT ret;
	enum dovi_status status = DOVI_STATUS_OK;

	if (dovi_tz_session == 0) {
		dovi_error("dovi_tz_session not created\n");
		return DOVI_STATUS_ERROR;
	}

	status = dovi_sec_map_direction(direct, &type);
	if (status != DOVI_STATUS_OK)
		return status;

	paramTypes = TZ_ParamTypes1(type);

	dovi_param[0].mem.buffer = buffer;
	dovi_param[0].mem.size = size;

	dovi_info("service call cmd %d, paramtype %d buffer %p, size %u\n",
	cmd, paramTypes, buffer, size);

	ret = KREE_TeeServiceCall(dovi_tz_session, cmd, paramTypes, dovi_param);
	if (ret != TZ_RESULT_SUCCESS) {
		dovi_error("service call fail: cmd[%d] ret[%d]\n", cmd, ret);
		return DOVI_STATUS_ERROR;
	}

	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_share_mem_service_call(enum DOVI_TZ_CALL_CMD cmd,
				       enum DOVI_TZ_CALL_DIR direct,
				       void *buffer, uint32_t size)
{
	unsigned int paramTypes = 0;
	MTEEC_PARAM dovi_param[4];
	TZ_PARAM_TYPES type;
	TZ_RESULT ret;
	enum dovi_status status = DOVI_STATUS_OK;

	if (dovi_tz_session == 0) {
		dovi_error("dovi_tz_session not created\n");
		return DOVI_STATUS_ERROR;
	}

	status = dovi_sec_map_direction(direct, &type);
	if (status != DOVI_STATUS_OK)
		return status;

	paramTypes = TZ_ParamTypes1(type);

	dovi_param[0].memref.handle = (uint32_t)(*(uint32_t *)buffer);
	dovi_param[0].memref.offset = 0;
	dovi_param[0].memref.size = size;

	dovi_info("share_mem_service_call cmd %d buf 0x%p size %u handle 0x%X\n",
	cmd, buffer, size, dovi_param[0].memref.handle);

	ret = KREE_TeeServiceCall(dovi_tz_session, cmd, paramTypes, dovi_param);
	if (ret != TZ_RESULT_SUCCESS) {
		dovi_error("service call fail: cmd[%d] ret[0x%X]\n", cmd, ret);
		return DOVI_STATUS_ERROR;
	}

	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_init(void)
{
	static bool dovi_sec_inited;
	enum dovi_status status = DOVI_STATUS_OK;

	dovi_func_default();
	if (dovi_sec_inited)
		return DOVI_STATUS_OK;
	dovi_sec_inited = true;	/* only init sec once */

	do {
		status = dovi_sec_create_session();
		if (status != DOVI_STATUS_OK)
			break;

		status = dovi_sec_create_share_memory();
		if (status != DOVI_STATUS_OK)
			break;

		status = dovi_sec_create_share_mem_session();
		if (status != DOVI_STATUS_OK)
			break;

		status = dovi_sec_create_share_mem_handle();
		if (status != DOVI_STATUS_OK)
			break;

		status = dovi_sec_share_memory_init();
		if (status != DOVI_STATUS_OK)
			break;

	} while (0);

	dovi_sec_status();

	if (status != DOVI_STATUS_OK)
		dovi_error("init dovi secure fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_share_memory_init(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);

	if (dovi_sec_share_mem_inited) {
		dovi_error("share mem already init\n");
		return DOVI_STATUS_OK;
	}

	status = dovi_sec_share_mem_service_call(DOVI_TZ_CALL_CMD_SHARE_MEMORY_INIT,
				       DOVI_TZ_CALL_DIR_MEMREF_INOUT,
				       (void *)&dovi_tz_mem_handle,
				       size);

	if (status != DOVI_STATUS_OK)
		dovi_error("init share mem fail [%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_md_parser_init(void)
{
	enum dovi_status status = DOVI_STATUS_OK;

	if (dovi_sec_md_parser_inited) {
		dovi_error("dovi_sec_md_parser already inited\n");
		return DOVI_STATUS_OK;
	}
	dovi_sec_md_parser_inited = true;	/* only init sec once */

#if 1
	status = dovi_sec_service_call(DOVI_TZ_CALL_CMD_MD_PARSER_INIT,
				       DOVI_TZ_CALL_DIR_MEM_INPUT, NULL, 0);
#else
#endif
	if (status != DOVI_STATUS_OK)
		dovi_error("init dovi secure fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_md_parser_uninit(void)
{
	enum dovi_status status = DOVI_STATUS_OK;

	if (!dovi_sec_md_parser_inited) {
		dovi_error("dovi_sec_md_parser already uninited\n");
		return DOVI_STATUS_OK;
	}
	dovi_sec_md_parser_inited = false;	/* only init sec once */

	status = dovi_sec_service_call(DOVI_TZ_CALL_CMD_MD_PARSER_UNINIT,
				       DOVI_TZ_CALL_DIR_MEM_INPUT, NULL, 0);

	if (status != DOVI_STATUS_OK)
		dovi_error("init dovi secure fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_cp_test_init(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);
	char *p_dovi_share_mem_handle = (char *) &dovi_tz_mem_handle;

	if (dovi_sec_cp_test_inited) {
		DISP_LOG_DEBUG("dovi_sec_cp_test init again!\n");
	}
	dovi_sec_cp_test_inited = true;	/* only init sec once */

	dovi_info("size %d share mem %p frame %u, dr_type %u, rpu_len %u 0x%p\n",
		size,
		dovi_share_mem,
		dovi_share_mem->frame_num,
		dovi_share_mem->dr_type,
		dovi_share_mem->rpu_bs_len,
		dovi_share_mem->rpu_bs_buffer);

	status = dovi_sec_share_mem_service_call(DOVI_TZ_CALL_CMD_CP_TEST_INIT,
				       DOVI_TZ_CALL_DIR_MEMREF_INPUT,
				       (void *)p_dovi_share_mem_handle,
				       size);

	if (status != DOVI_STATUS_OK)
		dovi_error("dovi_sec_cp_test init fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_cp_test_main(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);
	char *p_dovi_share_mem_handle = (char *) &dovi_tz_mem_handle;

	status = dovi_sec_share_mem_service_call(DOVI_TZ_CALL_CMD_CP_TEST_MAIN,
				       DOVI_TZ_CALL_DIR_MEMREF_INOUT,
				       (void *)p_dovi_share_mem_handle,
				       size);

	if (status != DOVI_STATUS_OK)
		dovi_error("dovi_sec_cp_test_main fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_cp_test_uninit(void)
{
	enum dovi_status status = DOVI_STATUS_OK;

	if (!dovi_sec_cp_test_inited) {
		dovi_error("dovi_sec_cp_test already uninited\n");
		return DOVI_STATUS_OK;
	}
	dovi_sec_cp_test_inited = false;	/* only init sec once */

	status = dovi_sec_service_call(DOVI_TZ_CALL_CMD_CP_TEST_UNINIT,
				       DOVI_TZ_CALL_DIR_MEM_INPUT, NULL, 0);

	if (status != DOVI_STATUS_OK)
		dovi_error("dovi_sec_cp_test uninit fail[%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_status(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t share_size = sizeof(struct dovi_share_memory_info_t);

	dovi_func();

	dovi_info("sec session 0x%X, share mem session 0x%X, handle 0x%X\n",
	dovi_tz_session, dovi_tz_mem_session, dovi_tz_mem_handle);

	dovi_info("share mem %p size %d\n", dovi_share_mem, share_size);


	return status;
}

enum dovi_status dovi_sec_debug_level_init(uint32_t dovi_log_level)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);
	char *p_dovi_share_mem_handle = (char *) &dovi_tz_mem_handle;


	dovi_share_mem->log_level = dovi_log_level;
	status = dovi_sec_share_mem_service_call(DOVI_TZ_CALL_CMD_DEBUG_LEVEL_INIT,
					DOVI_TZ_CALL_DIR_MEMREF_INOUT,
					(void *)p_dovi_share_mem_handle,
					size);

	if (status != DOVI_STATUS_OK)
		dovi_error("init debug level init fail [%d]\n", status);

	return status;
}

enum dovi_status dovi_sec_sec_handle_copy(uint32_t *sec_handle, uint32_t len)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);
	char *p_dovi_share_mem_handle = (char *) &dovi_tz_mem_handle;

	dovi_share_mem->sec_handle_in = *sec_handle;
	dovi_share_mem->sec_handle_len = len;
	status = dovi_sec_share_mem_service_call(DOVI_TZ_CALL_CMD_SEC_MEM_COPY,
					DOVI_TZ_CALL_DIR_MEMREF_INOUT,
					(void *)p_dovi_share_mem_handle,
					size);

	if (status != DOVI_STATUS_OK)
		dovi_error("dovi_sec_sec_handle_copy fail [%d]\n", status);

	*sec_handle = dovi_share_mem->sec_handle_out;

	return status;
}

#else

enum dovi_status dovi_sec_md_parser_init(void)
{
	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_md_parser_uninit(void)
{
	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_cp_test_init(void)
{
	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_cp_test_main(void)
{
	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_cp_test_uninit(void)
{
	return DOVI_STATUS_OK;
}

enum dovi_status dovi_sec_init(void)
{
	enum dovi_status status = DOVI_STATUS_OK;
	uint32_t size = sizeof(struct dovi_share_memory_info_t);

	if (dovi_share_mem) {
		dovi_error("share memory already created %p size %u\n",
			   dovi_share_mem, size);
		return DOVI_STATUS_OK;
	}

	dovi_share_mem = kmalloc(size, GFP_KERNEL);
	if (!dovi_share_mem) {
		dovi_error("share memory already fail %p size %u\n",
			   dovi_share_mem, size);
		return DOVI_STATUS_OK;
	}

	memset((void *)dovi_share_mem, 0, size);

	if (status != DOVI_STATUS_OK)
		dovi_error("create share memory fail size %d 0x%p\n",
			   size, dovi_share_mem);

	return DOVI_STATUS_OK;
}

#endif

