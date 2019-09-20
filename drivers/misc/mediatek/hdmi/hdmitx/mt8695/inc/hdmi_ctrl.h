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

#ifndef __hdmi_ctrl_h__
#define __hdmi_ctrl_h__
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT

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

#include "internal_hdmi_drv.h"

extern unsigned int hdmidrv_log_on;

#define hdmiplllog         (0x1)
#define hdmiceccommandlog  (0x2)
#define hdmitxhotpluglog   (0x4)
#define hdmitxvideolog     (0x8)
#define hdmitxaudiolog     (0x10)
#define hdmihdcplog        (0x20)
#define hdmiceclog         (0x40)
#define hdmiddclog         (0x80)
#define hdmiedidlog        (0x100)
#define hdmidrvlog         (0x200)
#define hdmireglog         (0x400)
#define hdmicecreglog     (0x800)
#define hdmihdrlog         (0x1000)
#define hdmihdrdebuglog    (0x2000)
#define hdmitzdebuglog     (0x4000)

#define hdmialllog   (hdmiceccommandlog | hdmitxhotpluglog)

/* ////////////////////////////////////////////PLL////////////////////////////////////////////////////// */
#define HDMI_PLL_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmiplllog) {	\
		TX_DEF_LOG("[PLL] %s ,%d "fmt, __func__, __LINE__, ##arg);	\
		}	\
	} while (0)

#define HDMI_PLL_FUNC()	\
	do {	if (hdmidrv_log_on & hdmiplllog)	\
		TX_DEF_LOG("[PLL] %s\n", __func__);	\
	} while (0)

/* ////////////////////////////////////////////DGI////////////////////////////////////////////////////// */
#define HDMI_PLUG_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmitxhotpluglog)	\
		TX_DEF_LOG("[PLG] "fmt, ##arg);	\
	} while (0)
#define HDMI_HDR_LOG(fmt, arg...) \
		do {	if (hdmidrv_log_on & hdmihdrlog)	\
			TX_DEF_LOG("[HDR] "fmt, ##arg);	\
		} while (0)
#define HDMI_HDR_DBG_LOG(fmt, arg...) \
		do {	if (hdmidrv_log_on & hdmihdrdebuglog)	\
			TX_DEF_LOG("[DBGHDR] "fmt, ##arg);	\
		} while (0)
#define HDMI_PLUG_FUNC()	\
	do {	if (hdmidrv_log_on & hdmitxhotpluglog)	\
		TX_DEF_LOG("[PLG] %s\n", __func__);	\
	} while (0)

/* ////////////////////////////////////////////PLUG////////////////////////////////////////////////////// */
#define HDMI_REG_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmireglog)	\
		TX_DEF_LOG("[REG] "fmt, ##arg);	\
	} while (0)

#define HDMI_REG_FUNC()	\
	do {	if (hdmidrv_log_on & hdmireglog)	\
		TX_DEF_LOG("[REG] %s\n", __func__);	\
	} while (0)

/* //////////////////////////////////////////////VIDEO//////////////////////////////////////////////////// */

#define HDMI_VIDEO_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmitxvideolog) {	\
		TX_DEF_LOG("[VIDEO] %s ,%d "fmt, __func__, __LINE__, ##arg); \
		}	\
	} while (0)

#define HDMI_VIDEO_FUNC()	\
	do {	if (hdmidrv_log_on & hdmitxvideolog)	\
		TX_DEF_LOG("[VIDEO] %s\n", __func__);	\
	} while (0)

/* //////////////////////////////////////////////AUDIO//////////////////////////////////////////////////// */

#define HDMI_AUDIO_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmitxaudiolog) {	\
		TX_DEF_LOG("[AUDIO] %s ,%d "fmt, __func__, __LINE__, ##arg); \
		}	\
	} while (0)

#define HDMI_AUDIO_FUNC()	\
	do {	if (hdmidrv_log_on & hdmitxaudiolog)	\
		TX_DEF_LOG("[AUDIO] %s\n", __func__);	\
	} while (0)
/* ///////////////////////////////////////////////HDCP/////////////////////////////////////////////////// */

#define HDMI_HDCP_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmihdcplog) {	\
		TX_DEF_LOG("[HDCP] %s ,%d "fmt, __func__, __LINE__, ##arg); \
		}	\
	} while (0)

#define HDMI_HDCP_FUNC()	\
	do {	if (hdmidrv_log_on & hdmihdcplog)	\
		TX_DEF_LOG("[HDCP] %s\n", __func__);	\
	} while (0)

/* ///////////////////////////////////////////////CEC/////////////////////////////////////////////////// */

#define HDMI_CEC_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmiceclog) {	\
		TX_DEF_LOG("[CEC] "fmt, ##arg);	\
		}	\
	} while (0)

#define HDMI_CEC_REG_LOG(fmt, arg...) \
		do {	if (hdmidrv_log_on & hdmicecreglog) {	\
			TX_DEF_LOG("[CECREG] "fmt, ##arg);	\
		}	\
	} while (0)

#define HDMI_CEC_FUNC()	\
	do {	if (hdmidrv_log_on & hdmiceclog)	\
		TX_DEF_LOG("[CEC] %s\n", __func__);	\
	} while (0)

#define HDMI_CEC_COMMAND_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmiceccommandlog)	\
		TX_DEF_LOG("[CECCMD] "fmt, ##arg);	\
	} while (0)

/* ///////////////////////////////////////////////DDC////////////////////////////////////////////////// */
#define HDMI_DDC_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmiddclog) { \
		TX_DEF_LOG("[DDC] %s ,%d "fmt, __func__, __LINE__, ##arg); \
		} \
	} while (0)

#define HDMI_DDC_FUNC()	\
	do {	if (hdmidrv_log_on & hdmiddclog) \
		TX_DEF_LOG("[DDC] %s\n", __func__); \
	} while (0)
/* ///////////////////////////////////////////////EDID////////////////////////////////////////////////// */
#define HDMI_EDID_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmiedidlog) { \
		TX_DEF_LOG("[EDID] "fmt, ##arg); \
		} \
	} while (0)

#define HDMI_EDID_FUNC()	\
	do {	if (hdmidrv_log_on & hdmiedidlog) \
		TX_DEF_LOG("[EDID] %s\n", __func__); \
	} while (0)
/* ////////////////////////////////////////////////DRV///////////////////////////////////////////////// */

#define HDMI_DRV_LOG(fmt, arg...) \
	do {	if (hdmidrv_log_on & hdmidrvlog) { \
		TX_DEF_LOG("[DRV] %s ,%d "fmt, __func__, __LINE__, ##arg); \
		} \
	} while (0)

#define HDMI_DRV_FUNC()	\
	do {	if (hdmidrv_log_on & hdmidrvlog) \
		TX_DEF_LOG("[DRV] %s\n", __func__); \
	} while (0)
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */

extern unsigned int _u2TxBStatus;
extern bool _fgRepeater;

extern unsigned char temprgb2hdmi;
extern void hdmi_write(unsigned int u2Reg, unsigned int u4Data);
extern int hdmi_internal_power_on(void);
extern void hdmi_internal_power_off(void);
extern unsigned char hdmi_powerenable;
extern bool debug_hdr10p_force_enable_edid;
extern unsigned char debug_hdr10p_force_set_appversion;
#endif
#endif
