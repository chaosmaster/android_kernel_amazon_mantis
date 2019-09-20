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

#ifndef _HDMI_CA_H_
#define _HDMI_CA_H_
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT

bool fgCaHDMICreate(void);
bool fgCaHDMIClose(void);
void vCaHDMIWriteReg(unsigned int u4addr, unsigned int u4data);
bool fgCaHDMIInstallHdcpKey(unsigned char *pdata, unsigned int u4Len);
bool fgCaHDMIGetAKsv(unsigned char *pdata);
bool fgCaHDMILoadHDCPKey(void);
bool fgCaHDMILoadROM(void);
void vCaHDMIWriteHdcpCtrl(unsigned int u4addr, unsigned int u4data);

bool fgCaHDMITestHDCPVersion(void);
extern int hdmi_audio_signal_state(unsigned int state);
extern unsigned int _Cmd_GCPU_KP_Set(unsigned int bIndex);
extern void vCaHDMIWriteHDCPRST(unsigned int u4addr, unsigned int u4data);
void fgCaHDMISetTzLogLevel(unsigned int loglevel);
extern bool fgCaHDMIGetTAStatus(unsigned char *pdata);

#endif
#endif
