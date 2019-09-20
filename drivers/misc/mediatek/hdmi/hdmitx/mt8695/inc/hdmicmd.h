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

#ifndef __hdmihdmicmd_h__
#define __hdmihdmicmd_h__
#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
#include "hdmihdcp.h"

extern void hdmi_InfoframeSetting(unsigned char i1typemode, unsigned char i1typeselect);
extern void hdmi_fmtsetting(unsigned int resolutionmode);
extern void rgb2hdmi_setting(unsigned int resolutionmode);
extern void hdmitx_configsetting(unsigned int resolutionmode);
extern const unsigned char _cFsStr[][7];
extern unsigned char cDstStr[50];
extern void HDMI_EnableIrq(void);
extern void HDMI_DisableIrq(void);
extern int hdmi_audiosetting(HDMITX_AUDIO_PARA *audio_para);
extern void HdcpService(HDCP_CTRL_STATE_T e_hdcp_state);
extern void vHDCP2XInitAuth(void);


#endif
#endif
