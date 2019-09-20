#ifndef __DISP_HDR_SEC_H__
#define __DISP_HDR_SEC_H__

#include "disp_hdr_def.h"

enum SERVICE_CALL_CMD {
	SERVICE_CALL_CMD_SEND_CLI_INFO = 0,
	SERVICE_CALL_CMD_INIT_SHARE_MEMORY = 1,
	SERVICE_CALL_CMD_DEBUG = 2,
	SERVICE_CALL_CMD_REGISTER_IRQ = 3,
	SERVICE_CALL_CMD_COPY_SHARE_MEMORY = 4,
	SERVICE_CALL_CMD_MAX,
};

enum SERVICE_CALL_DIRECTION {
	SERVICE_CALL_DIRECTION_INPUT = 0,
	SERVICE_CALL_DIRECTION_OUTPUT = 1,
	SERVICE_CALL_DIRECTION_INOUT = 2,
};

struct hdr_share_memory_struct {
	int plane;
	int use_hdr;
	int use_hdr_sub_path;
	GAMMA_TYPE_ENUM m_Gamma_type;
	GAMMA_TYPE_ENUM m_Gamma_type_subpath;
	uint32_t m_TvMaxLuma;
	uint32_t m_TvMaxLuma_subpath;
	TONE_MAPPING_PQ_FIELD_T vHDRPQINFO;
	TONE_MAPPING_PQ_FIELD_T vHDRPQINFOSubpath;
};


enum HDR_STATUS hdr_sec_init(void);
enum HDR_STATUS hdr_sec_service_call(enum SERVICE_CALL_CMD cmd, enum SERVICE_CALL_DIRECTION direct,
	void *buffer, uint32_t size);
enum HDR_STATUS hdr_sec_service_call_2(enum SERVICE_CALL_CMD cmd,
	enum SERVICE_CALL_DIRECTION direct_1, void *buffer_1, uint32_t size_1,
	enum SERVICE_CALL_DIRECTION direct_2, void *buffer_2, uint32_t size_2);
struct hdr_share_memory_struct *hdr_sec_get_share_memory(void);

#endif /* __DISP_HDR_SEC_H__ */
