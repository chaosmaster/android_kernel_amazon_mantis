#include <kree/mem.h>
#include <kree/system.h>
#include <tz_cross/ta_mem.h>
#include <tz_cross/trustzone.h>
#include <linux/slab.h>

#include "disp_hdr_sec.h"
#include "disp_hdr_util.h"

#ifdef HDR_SECURE_SUPPORT

static KREE_SESSION_HANDLE hdr_session;
static KREE_SESSION_HANDLE memory_session;


static struct hdr_share_memory_struct gShareMemory;

static enum HDR_STATUS _hdr_sec_create_session(void)
{
	enum HDR_STATUS status = HDR_STATUS_OK;
	TZ_RESULT ret;

	do {
		if (hdr_session == 0) {
			ret = KREE_CreateSession("hdr2sdr_ta_uuid_07318", &hdr_session);
			if (ret != TZ_RESULT_SUCCESS) {
				HDR_ERR("create hdr_session fail:%d\n", ret);
				status = HDR_STATUS_CREATE_HDR_SESSION_FAIL;
				break;
			}
		}

		if (memory_session == 0) {
			ret = KREE_CreateSession(TZ_TA_MEM_UUID, &memory_session);
			if (ret != TZ_RESULT_SUCCESS) {
				HDR_ERR("create memory session fail:%d\n", ret);
				status = HDR_STATUS_CREATE_MEMORY_SESSION_FAIL;
				break;
			}
		}
	} while (0);

	if (status != HDR_STATUS_OK)
		HDR_ERR("create session fail:%d\n", status);
	else
		HDR_LOG("create session success: hdr_session[%d] memory_session[%d]\n", hdr_session, memory_session);

	return status;
}
#if 0
static enum HDR_STATUS _hdr_sec_create_share_memory(void)
{

#if 0
	enum HDR_STATUS status = HDR_STATUS_OK;

	memset(&gShareMemory, 0, sizeof(gShareMemory));
	status = hdr_sec_service_call(SERVICE_CALL_CMD_INIT_SHARE_MEMORY,
		SERVICE_CALL_DIRECTION_INOUT, &gShareMemory, sizeof(gShareMemory));
	if (status != HDR_STATUS_OK)
		HDR_ERR("create share memory fail\n");
	else
		HDR_LOG("create share memory ok\n");
#endif
#if 0
	if (memory_session == 0) {
		HDR_ERR("memory_session not created\n");
		return HDR_STATUS_MEMORY_SESSION_NOT_CREATED;
	}

	memory_param.buffer = kzalloc(sizeof(struct hdr_share_memory_struct), GFP_KERNEL);
	memory_param.size = sizeof(struct hdr_share_memory_struct);

	ret = KREE_RegisterSharedmem(memory_session, &memory_handle, &memory_param);
	if (ret != TZ_RESULT_SUCCESS) {
		HDR_ERR("register share memory fail[%d]\n", ret);
		return HDR_STATUS_CREATE_SHARE_MEMORY_FAIL;
	}

	hdr_param[0].memref.handle = memory_handle;
	hdr_param[0].memref.offset = 0;
	hdr_param[0].memref.size = memory_param.size;
	paramTypes = TZ_ParamTypes1(TZPT_MEMREF_INOUT);
	ret = KREE_TeeServiceCall(hdr_session, SERVICE_CALL_CMD_INIT_SHARE_MEMORY, paramTypes, hdr_param);
	if (ret != TZ_RESULT_SUCCESS) {
		HDR_ERR("share memory service call fail[%d]\n", ret);
		return HDR_STATUS_CREATE_SHARE_MEMORY_FAIL;
	}
#endif
	return HDR_STATUS_OK;
}
#endif


enum HDR_STATUS hdr_sec_init(void)
{
	static bool hdr_sec_inited;
	enum HDR_STATUS status = HDR_STATUS_OK;

	if (hdr_sec_inited)
		return HDR_STATUS_OK;
	hdr_sec_inited = true; /* only init sec once */

	do {
		status = _hdr_sec_create_session();
		if (status != HDR_STATUS_OK)
			break;
		#if 0
		status = _hdr_sec_create_share_memory();
		if (status != HDR_STATUS_OK)
			break;
		#endif
	} while (0);

	if (status != HDR_STATUS_OK)
		HDR_ERR("init hdr secure fail[%d]\n", status);

	return status;
}

struct hdr_share_memory_struct *hdr_sec_get_share_memory(void)
{
	return &gShareMemory;
}

static enum HDR_STATUS _hdr_sec_map_direction(enum SERVICE_CALL_DIRECTION direct, TZ_PARAM_TYPES *pType)
{
	switch (direct) {
	case SERVICE_CALL_DIRECTION_INPUT:
		*pType = TZPT_MEM_INPUT;
		break;
	case SERVICE_CALL_DIRECTION_OUTPUT:
		*pType = TZPT_MEM_OUTPUT;
		break;
	case SERVICE_CALL_DIRECTION_INOUT:
		*pType = TZPT_MEM_INOUT;
		break;
	default:
		HDR_ERR("invalid service call direction:%d\n", direct);
		return HDR_STATUS_INVALID_SERVICE_CALL_DIRECTION;
	}
	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr_sec_service_call(enum SERVICE_CALL_CMD cmd, enum SERVICE_CALL_DIRECTION direct,
	void *buffer, uint32_t size)
{
	unsigned int paramTypes = 0;
	MTEEC_PARAM hdr_param[4];
	TZ_PARAM_TYPES type;
	TZ_RESULT ret;
	enum HDR_STATUS status = HDR_STATUS_OK;

	if (hdr_session == 0) {
		HDR_ERR("hdr_sec_service_call error: hdr_session not created\n");
		return HDR_STATUS_HDR_SESSION_NOT_CREATED;
	}

	status = _hdr_sec_map_direction(direct, &type);
	if (status != HDR_STATUS_OK)
		return status;

	paramTypes = TZ_ParamTypes1(type);

	hdr_param[0].mem.buffer = buffer;
	hdr_param[0].mem.size = size;

	ret = KREE_TeeServiceCall(hdr_session, cmd, paramTypes, hdr_param);
	if (ret != TZ_RESULT_SUCCESS) {
		HDR_ERR("service call fail: cmd[%d] ret[%d]\n", cmd, ret);
		return HDR_STATUS_SERVICE_CALL_FAIL;
	}

	return HDR_STATUS_OK;
}


enum HDR_STATUS hdr_sec_service_call_2(enum SERVICE_CALL_CMD cmd,
	enum SERVICE_CALL_DIRECTION direct_1, void *buffer_1, uint32_t size_1,
	enum SERVICE_CALL_DIRECTION direct_2, void *buffer_2, uint32_t size_2)
{
	enum HDR_STATUS status = HDR_STATUS_OK;
	unsigned int paramTypes = 0;
	MTEEC_PARAM hdr_param[4];
	TZ_RESULT ret;
	TZ_PARAM_TYPES type_1, type_2;

	if (hdr_session == 0) {
		HDR_ERR("hdr_sec_service_call error: hdr_session not created\n");
		return HDR_STATUS_HDR_SESSION_NOT_CREATED;
	}

	status = _hdr_sec_map_direction(direct_1, &type_1);
	if (status != HDR_STATUS_OK)
		return status;
	status = _hdr_sec_map_direction(direct_2, &type_2);
	if (status != HDR_STATUS_OK)
		return status;

	paramTypes = TZ_ParamTypes2(type_1, type_2);


	hdr_param[0].mem.buffer = buffer_1;
	hdr_param[0].mem.size = size_1;

	hdr_param[1].mem.buffer = buffer_2;
	hdr_param[1].mem.size = size_2;

	ret = KREE_TeeServiceCall(hdr_session, cmd, paramTypes, hdr_param);
	if (ret != TZ_RESULT_SUCCESS) {
		HDR_ERR("service call fail: cmd[%d] ret[%d]\n", cmd, ret);
		return HDR_STATUS_SERVICE_CALL_FAIL;
	}

	return HDR_STATUS_OK;
}

#endif		/* ifdef HDR_SECURE_SUPPORT */

