/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <hdmitx.h>
#include <hdmictrl.h>

#include "tz_cross/trustzone.h"
#include "tz_cross/ta_test.h"
#include "tz_cross/ta_mem.h"
#include "trustzone/kree/system.h"
#include "trustzone/kree/mem.h"
/* #include "kree_int.h" */

#include "tz_cross/ta_drmkey.h"
#include "tz_cross/keyblock.h"

#include "tz_cross/hdmi_ta.h"
#include "hdmi_ca.h"

extern int hdmi_audio_signal_state(unsigned int state);
KREE_SESSION_HANDLE ca_hdmi_handle;

bool fgCaHDMICreate(void)
{
	TZ_RESULT tz_ret = 0;

	tz_ret = KREE_CreateSession(TZ_TA_HDMI_UUID, &ca_hdmi_handle);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		/* Should provide strerror style error string in UREE. */
		TX_DEF_LOG("Create ca_hdmi_handle Error: %d\n", tz_ret);
		return false;
	}
	TX_DEF_LOG("Create ca_hdmi_handle ok: %d\n", tz_ret);


	return true;
}

bool fgCaHDMIClose(void)
{
	TZ_RESULT tz_ret = 0;

	tz_ret = KREE_CloseSession(ca_hdmi_handle);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		/* Should provide strerror style error string in UREE. */
		TX_DEF_LOG("Close ca_hdmi_handle Error: %d\n", tz_ret);
		return false;
	}
	TX_DEF_LOG("Close ca_hdmi_handle ok: %d\n", tz_ret);
	return true;
}

void vCaHDMIWriteReg(unsigned int u4addr, unsigned int u4data)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA]TEE ca_hdmi_handle=0\n");
		return;
	}

	param[0].value.a = u4addr & 0xFFF;
	param[0].value.b = 0;
	param[1].value.a = u4data;
	param[1].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_WRITE_REG,
				     TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS)
		TX_DEF_LOG("[CA] HDMI_TA_WRITE_REG err:%X\n", tz_ret);

}

void vCaHDMIWriteHDCPRST(unsigned int u4addr, unsigned int u4data)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA]TEE ca_hdmi_handle=0\n");
		return;
	}

	param[0].value.a = u4addr;
	param[0].value.b = 0;
	param[1].value.a = u4data;
	param[1].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_HDCP_RST,
				     TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS)
		TX_DEF_LOG("[CA] HDMI_TA_WRITE_REG err:%X\n", tz_ret);

}

void vCaHDMIWriteHdcpCtrl(unsigned int u4addr, unsigned int u4data)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[HDMI] TEE ca_hdmi_handle=0\n");
		return;
	}

	param[0].value.a = u4addr;
	param[0].value.b = 0;
	param[1].value.a = u4data;
	param[1].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_WRITE_REG,
				     TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS)
		TX_DEF_LOG("[CA]HDMI_TA_WRITE_REG err:%X\n", tz_ret);
}

bool fgCaHDMIInstallHdcpKey(unsigned char *pdata, unsigned int u4Len)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];
	unsigned char *ptr;
	unsigned int i;

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}

	TX_DEF_LOG("[CA]fgCaHDMIInstallHdcpKey,%d\n", u4Len);

	if (u4Len > HDCPKEY_LENGTH_DRM)
		return false;

	ptr = kmalloc(u4Len, GFP_KERNEL);

	for (i = 0; i < u4Len; i++)
		ptr[i] = pdata[i];

	param[0].mem.buffer = ptr;
	param[0].mem.size = u4Len;
	param[1].value.a = u4Len;
	param[1].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_INSTALL_HDCP_KEY,
				     TZ_ParamTypes2(TZPT_MEM_INPUT, TZPT_VALUE_INPUT), param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA] HDMI_TA_INSTALL_HDCP_KEY err:%X\n", tz_ret);
		return false;
	}

	kfree(ptr);
	return true;
}

bool fgCaHDMIGetAKsv(unsigned char *pdata)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[1];
	unsigned char *ptr;
	unsigned char i;

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}

	ptr = kmalloc(5, GFP_KERNEL);
	param[0].mem.buffer = ptr;
	param[0].mem.size = 5;
	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_GET_HDCP_AKSV,
				     TZ_ParamTypes1(TZPT_MEM_OUTPUT), param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA]HDMI_TA_GET_HDCP_AKSV err:%X\n", tz_ret);
		return false;
	}
	for (i = 0; i < 5; i++)
		pdata[i] = ptr[i];

	TX_DEF_LOG("[CA]hdcp aksv : %x %x %x %x %x\n",
		   pdata[0], pdata[1], pdata[2], pdata[3], pdata[4]);
	kfree(ptr);
	return true;
}

bool fgCaHDMIGetTAStatus(unsigned char *pdata)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[1];
	unsigned char *ptr;

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}

	ptr = kmalloc(2, GFP_KERNEL);
	if (ptr == NULL) {
		TX_DEF_LOG("[CA]fgCaHDMIGetTAStatus\n");
		return false;
	}
	param[0].mem.buffer = ptr;
	param[0].mem.size = 2;
	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_READ_STATUS,
				     TZ_ParamTypes1(TZPT_MEM_OUTPUT), param);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA]fgCaHDMIGetTAStatus err:%X\n", tz_ret);
		return false;
	}
	pdata[0] = ptr[0];
	pdata[1] = ptr[1];

	kfree(ptr);
	return true;
}

bool fgCaHDMILoadHDCPKey(void)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[1];

	TX_DEF_LOG("[CA] fgCaHDMILoadHDCPKey\n");
	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}
	param[0].value.a = 0;
	param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_LOAD_HDCP_KEY,
				     TZ_ParamTypes1(TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA] HDMI_TA_LOAD_HDCP_KEY err:%X\n", tz_ret);
		return false;
	}
	return true;
}

bool fgCaHDMILoadROM(void)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[1];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}

	param[0].value.a = 0;
	param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_LOAD_ROM,
				     TZ_ParamTypes1(TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA]HDMI_TA_LOAD_ROM err:%X\n", tz_ret);
		return false;
	}
	return true;
}


bool fgCaHDMITestHDCPVersion(void)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[1];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[CA] TEE ca_hdmi_handle=0\n");
		return false;
	}

	param[0].value.a = 0;
	param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_TEST_HDCP_VERSION,
				     TZ_ParamTypes1(TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS) {
		TX_DEF_LOG("[CA]HDMI_TA_TEST_HDCP_VERSION err:%X\n", tz_ret);
		return false;
	}
	return true;
}

void fgCaHDMISetTzLogLevel(unsigned int loglevel)
{
	TZ_RESULT tz_ret = 0;
	MTEEC_PARAM param[2];

	if (ca_hdmi_handle == 0) {
		TX_DEF_LOG("[HDMI] TEE ca_hdmi_handle=0\n");
		return;
	}

	param[0].value.a = loglevel;
	param[0].value.b = 0;

	tz_ret = KREE_TeeServiceCall(ca_hdmi_handle, HDMI_TA_SET_LOG_LEVEL,
				     TZ_ParamTypes1(TZPT_VALUE_INPUT), param);

	if (tz_ret != TZ_RESULT_SUCCESS)
		TX_DEF_LOG("[CA]HDMI_TA_SET_LOG_LEVEL err:%X\n", tz_ret);
}

#endif
