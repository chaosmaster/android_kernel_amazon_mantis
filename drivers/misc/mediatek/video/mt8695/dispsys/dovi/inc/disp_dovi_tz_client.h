/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __DISP_DOVI_SEC_H__
#define __DISP_DOVI_SEC_H__

#include "disp_info.h"
#include "dovi_type.h"
#include "dovi_log.h"
#include "disp_dovi_control_if.h"
#include "dovi_common_hal.h"
#include "dovi_core1_hw.h"
#include "dovi_core2_hw.h"
#include "dovi_core3_hw.h"
#include "disp_dovi_md_parser.h"

#define DOVI_TA_UUID "dovi_ta_uuid_40418"

#define BS_BUF_SIZE          1024

enum DOVI_TZ_CALL_CMD {
	DOVI_TZ_CALL_CMD_SEND_CLI_INFO,
	DOVI_TZ_CALL_CMD_INIT_SHARE_MEMORY,
	DOVI_TZ_CALL_CMD_DEBUG_LEVEL_INIT,
	DOVI_TZ_CALL_CMD_REGISTER_IRQ,
	DOVI_TZ_CALL_CMD_MD_PARSER_INIT,
	DOVI_TZ_CALL_CMD_MD_PARSER_MAIN,
	DOVI_TZ_CALL_CMD_MD_PARSER_UNINIT,
	DOVI_TZ_CALL_CMD_CP_TEST_INIT,
	DOVI_TZ_CALL_CMD_CP_TEST_MAIN,
	DOVI_TZ_CALL_CMD_CP_TEST_UNINIT,
	DOVI_TZ_CALL_CMD_SHARE_MEMORY_INIT,
	DOVI_TZ_CALL_CMD_SEC_MEM_COPY,
	DOVI_TZ_CALL_CMD_MAX,
};

enum DOVI_TZ_CALL_DIR {
	DOVI_TZ_CALL_DIR_NONE = 0,
	DOVI_TZ_CALL_DIR_VALUE_INPUT = 1,
	DOVI_TZ_CALL_DIR_VALUE_OUTPUT = 2,
	DOVI_TZ_CALL_DIR_VALUE_INOUT = 3,
	DOVI_TZ_CALL_DIR_MEM_INPUT = 4,
	DOVI_TZ_CALL_DIR_MEM_OUTPUT = 5,
	DOVI_TZ_CALL_DIR_MEM_INOUT = 6,
	DOVI_TZ_CALL_DIR_MEMREF_INPUT = 7,
	DOVI_TZ_CALL_DIR_MEMREF_OUTPUT = 8,
	DOVI_TZ_CALL_DIR_MEMREF_INOUT = 9,
};

struct dovi_share_memory_info_t {
	enum DISP_DR_TYPE_T dr_type; /* in: sdr/hdr10 md/dovi rpu stream */
	struct mtk_disp_hdr10_md_t hdr10_md;
	unsigned int frame_num;
	unsigned int rpu_bs_len;
	bool svp;
	uint32_t sec_handle;
	unsigned char rpu_bs_buffer[BITSTREAM_BUFFER_SIZE];
	struct cp_param_t cp_param; /* in: control_path_test in param */
	uint32_t comp_md[DOVI_COMP_DW_SIZE]; /* out: metadata praser out comp info */
	uint32_t orig_md[DOVI_MD_DW_SIZE]; /* out: metadata praser out dm info */
	uint32_t hdmi_md[DOVI_MD_DW_SIZE]; /* out: control_path_test out dm info */
	uint32_t orig_md_len;
	uint32_t hdmi_md_len;
	struct mtk_disp_hdr10_md_t hdr10_info_frame; /* out: control_path_test hdr10 md */
	uint32_t core1_reg[CORE1_DPM_REG_NUM]; /* out: control_path_test core1 reg */
	uint32_t core2_reg[CORE2_DPM_REG_NUM]; /* out: control_path_test core2 reg */
	uint32_t core3_reg[CORE3_DPM_REG_NUM]; /* out: control_path_test core3 reg */
	uint32_t core1_lut[DOVI_LUT_DW_SIZE]; /* out: control_path_test core1 lut */
	uint32_t core2_lut[DOVI_LUT_DW_SIZE]; /* out: control_path_test core2 lut */
	uint32_t log_level;
	uint32_t sec_handle_in;
	uint32_t sec_handle_out;
	uint32_t sec_handle_len;
	bool profile4;
};

extern struct dovi_share_memory_info_t *dovi_share_mem;
enum dovi_status dovi_sec_service_call(enum DOVI_TZ_CALL_CMD cmd,
				       enum DOVI_TZ_CALL_DIR direct,
				       void *buffer,
				       uint32_t size);
enum dovi_status dovi_sec_share_mem_service_call(enum DOVI_TZ_CALL_CMD cmd,
enum DOVI_TZ_CALL_DIR direct,
void *buffer,
uint32_t size);

enum dovi_status dovi_sec_share_memory_init(void);

enum dovi_status dovi_sec_md_parser_init(void);
enum dovi_status dovi_sec_md_parser_uninit(void);
enum dovi_status dovi_sec_cp_test_init(void);
enum dovi_status dovi_sec_cp_test_main(void);
enum dovi_status dovi_sec_cp_test_uninit(void);
enum dovi_status dovi_sec_status(void);
enum dovi_status dovi_sec_debug_level_init(uint32_t dovi_tz_level);
enum dovi_status dovi_sec_sec_handle_copy(uint32_t *sec_handle, uint32_t len);

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) || defined(CONFIG_TRUSTY)
#define DOVI_TZ_OK 1
#else
#define DOVI_TZ_OK 0
#endif

#endif				/* __DISP_DOVI_SEC_H__ */
