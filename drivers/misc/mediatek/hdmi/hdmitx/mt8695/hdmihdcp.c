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

#include <hdmitx.h>

#include "hdmictrl.h"
#include "hdmihdcp.h"
#include "hdmi_ctrl.h"
#include "hdmiddc.h"
#include "hdcpbin.h"

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
#include "hdmi_ca.h"
#endif

/****************************************************************************
** Local definitions
****************************************************************************/
#define REPEAT_CHECK_AUTHHDCP_VALUE 25
#define MARK_LAST_AUTH_FAIL 0xf0
#define MARK_FIRST_AUTH_FAIL 0xf1

#define USE_INT_HDCP
#define SUPPORT_SIMPLAY
#define SRM_SUPPORT

#define HDMI_OK	     (unsigned int)(0)
#define HDMI_FAIL     (unsigned int)(-1)


#define SV_OK	     (unsigned char)(0)
#define SV_FAIL      (unsigned char)(-1)

unsigned int hdcp_err_0x30_count;
unsigned int hdcp_err_0x30_flag;

/* #define SUPPORT_SOFT_SHA */
/* for debug message */
#define DEBUG_HDCP_RI
#define DEBUG_HDCP
/* #define DEBUG_HDCP_RI_AN_FIX */
#define SRM_DBG
/* #define SUPPORT_RI_SAME_WAIT_NEXT_SYNC_UPDATE 1//move it to hdmi_drv.h */
unsigned char load_bin_flag = FALSE;
static HDMI_HDCP_KEY_T bhdcpkey = EXTERNAL_KEY;
/* no encrypt key */
const unsigned char HDCP_NOENCRYPT_KEY[HDCP_KEY_RESERVE] = {
	0
};

/* encrypt key */
const unsigned char HDCP_ENCRYPT_KEY[HDCP_KEY_RESERVE] = {
	0
};

static unsigned char bHdcpKeyExternalBuff[HDCP_KEY_RESERVE] = {
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa,
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
};

/****************************************************************************
** Local structures and enumerations
****************************************************************************/
#ifdef SRM_SUPPORT
typedef struct _SRMINFO {

	/* unsigned int dwVRLAmount;    // Revocation List Length */
	/* DWRD dwIdx;    // Current Data Number */
	/* DWRD dwLBA; */
	/* DWRD dwVRLLen; */
	unsigned int dwVRLLenInDram;
	unsigned int dwVer;
	/* DWRD dwVerInDram; */
	/* BYTE bGen; */
	unsigned char bID;
	/* BYTE bHPDSt; */
	/* unsigned char bBufSt;   // KSV buffer state */
	/* unsigned char fgKSVLimited; //over 300KSVs */

} SRMINFO;

SRMINFO _rSRMInfo;

unsigned char _bHdcp_Bksv[5];
unsigned char _bLastHdcpStatus = SV_OK;

unsigned char _bHdcpStatus = SV_OK;
unsigned char _u1SRMSignatureChkFlag;
#endif

unsigned int _u14SeqMnum;

/****************************************************************************
** Function prototypes
****************************************************************************/

static unsigned char _bReAuthCnt = FALSE;
static unsigned char _bReRepeaterPollCnt;
static unsigned char _bReCertPollCnt;
static unsigned char _bReRepeaterDoneCnt;
HDMI_HDCP_BKSV_INFO hdmi_hdcp_info;
static unsigned char _bReAKEtPollCnt;

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
static unsigned char u1CaHdcpAKsv[HDCP_AKSV_COUNT];
#endif


/****************************************************************************
** Local variable
****************************************************************************/

#define HDCP_KEY_RESERVE 287	/* {0x00, AKSV, KEY_SET, ChkSum} */

unsigned char bHdcpKeyBuff[HDCP_KEY_RESERVE] = { };				/* {0x00, AKSV, KEY_SET, ChkSum} */

unsigned char HDMI_AKSV[HDCP_AKSV_COUNT];
unsigned char bKsv_buff[KSV_BUFF_SIZE];	/* 0~127 : Ksv list,max 23, 128~159: V use 20 bytes , 160~191: V' use 20byte */
unsigned char HDCPBuff[60];
unsigned char bKsvlist_buff[KSV_LIST_SIZE];
unsigned char bKsvlist[KSV_LIST_SIZE];


unsigned char bSHABuff[20];	/* SHA hash value 20 bytes */
bool _fgRepeater = FALSE;
unsigned char _bReCheckBstatusCount;
unsigned char _bReCompRiCount;
unsigned char _bReCheckReadyBit;
unsigned char _bHdcpOff = 0; //enable hdcp

unsigned char hdcp_key_load_status;
unsigned char _bSRMBuff[SRM_SIZE];

unsigned char _bTxBKAV[HDCP_AKSV_COUNT] = { 0 };

unsigned char _bDevice_Count;
unsigned int _u2TxBStatus;

static unsigned int i4HdmiShareInfo[MAX_HDMI_SHAREINFO];

unsigned long hdcp_unmute_start_time;
unsigned long hdcp_logo_start_time;
unsigned int hdcp_unmute_logo_flag = 0xff;
bool hdcp_unmute_start_flag = FALSE;

void hdcp_set_unmute_start_time(void)
{
	hdcp_unmute_start_time = jiffies;
}

bool hdcp_is_unmute_timeout(unsigned long delay_ms)
{
	unsigned long u4DeltaTime;

	u4DeltaTime = hdcp_unmute_start_time + (delay_ms)*HZ / 1000;
	if (time_after(jiffies, u4DeltaTime))
		return TRUE;
	else
		return FALSE;
}

void hdcp_set_logo_start_time(void)
{
	hdcp_logo_start_time = jiffies;
}

bool hdcp_is_logo_timeout(unsigned long delay_ms)
{
	unsigned long u4DeltaTime;

	u4DeltaTime = hdcp_logo_start_time + (delay_ms)*HZ / 1000;
	if (time_after(jiffies, u4DeltaTime))
		return TRUE;
	else
		return FALSE;
}

void hdcp_unmute_logo(void)
{
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	TX_DEF_LOG("boot unmute logo\n");
	vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa5555);
	hdcp_unmute_logo_flag = 1;
	hdcp_set_logo_start_time();
#endif
}

void hdcp_unmute_process(void)
{
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	if (hdcp_unmute_logo_flag == 1) {
		if ((hdcp_is_logo_timeout(3000))) {
			vCaHDMIWriteHdcpCtrl(0x88880000, 0x5555aaaa);
			hdcp_unmute_logo_flag = 0;
			TX_DEF_LOG("boot unmute logo done\n");
		}
	} else if ((hdcp_unmute_logo_flag != 0xff)) {
		if (hdcp_unmute_start_flag == TRUE) {
			if (hdcp_is_unmute_timeout(100)) {
				vHDMIAVUnMute();
				hdcp_unmute_start_flag = FALSE;
				TX_DEF_LOG("hdcp delay unmute\n");
			}
		} else if (hdcp_is_unmute_timeout(100)) {
			hdcp_set_unmute_start_time();
			if (_bHdcpOff != 1)
			vCaHDMIWriteHdcpCtrl(0x88880000, 0);
		}
	}
#endif
}

void vInitHdcpKey(void)
{
	/* vWriteHdmiSYSMsk(0x28, 3<<26,3<<26); */

	load_bin_flag = FALSE;
}

void vCleanAuthFailInt(void)
{
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x00020000);
	udelay(1);
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x00000000);
	HDMI_HDCP_LOG("0x14025c8c = 0x%08x\n", bReadByteHdmiGRL(HDCP2X_STATUS_0));
	_bHdcpStatus = SV_FAIL;
}

void vHDMI2xClearINT(void)
{
	HDMI_HDCP_LOG("vHDMI2xClearINT\n");

	vWriteByteHdmiGRL(TOP_INT_CLR00, 0xfffffff0);
	vWriteByteHdmiGRL(TOP_INT_CLR01, 0xffffffff);
	udelay(1);
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x0);
	vWriteByteHdmiGRL(TOP_INT_CLR01, 0x0);
}

void vHalHDCP1x_Reset(void)
{
	/* Reset hdcp 1.x */
	/* SOFT_HDCP_1P4_RST, SOFT_HDCP_1P4_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	vCaHDMIWriteHDCPRST(SOFT_HDCP_1P4_RST, SOFT_HDCP_1P4_RST);
#endif
	udelay(100);
	/* SOFT_HDCP_1P4_NOR, SOFT_HDCP_1P4_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	vCaHDMIWriteHDCPRST(SOFT_HDCP_1P4_NOR, SOFT_HDCP_1P4_RST);
#endif

	vWriteHdmiGRLMsk(HDCP1X_CTRL, 0, ANA_TOP);
	vWriteHdmiGRLMsk(HDCP1X_CTRL, 0, HDCP1X_ENC_EN);
}

void vSetHDCPState(HDCP_CTRL_STATE_T e_state)
{
	HDMI_HDCP_FUNC();

	e_hdcp_ctrl_state = e_state;
}


void vHDCPEncryptState(unsigned int u1Success)
{
	HDMI_HDCP_FUNC();

	HDMI_HDCP_LOG("u1SuccessState = %d\n", u1Success);

	hdmi_audio_signal_state(u1Success);
}

unsigned int i4SharedInfo(unsigned int u4Index)
{
	HDMI_HDCP_FUNC();
	return i4HdmiShareInfo[u4Index];
}


void vSetSharedInfo(unsigned int u4Index, unsigned int i4Value)
{
	HDMI_DRV_FUNC();
	i4HdmiShareInfo[u4Index] = i4Value;
}

void vSendHdmiCmd(unsigned char u1icmd)
{
	HDMI_DRV_FUNC();
	hdmi_hdmiCmd = u1icmd;
}

unsigned char hdmi_check_hdcp_key(void)
{
	if ((HDMI_AKSV[0] == 0) && (HDMI_AKSV[1] == 0) && (HDMI_AKSV[2] == 0) && (HDMI_AKSV[3] == 0)
	    && (HDMI_AKSV[4] == 0))
		return 0;
	return 1;
}

unsigned char hdmi_check_hdcp_state(void)
{
	if ((e_hdcp_ctrl_state == HDCP_WAIT_RI) || (e_hdcp_ctrl_state == HDCP_CHECK_LINK_INTEGRITY))
		return 1;
	return 0;
}

void hdmi_hdcpkey(unsigned char *pbhdcpkey)
{
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
#ifdef CONFIG_MTK_DRM_KEY_MNG_SUPPORT
	bool ret = FALSE;
	HDMI_HDCP_FUNC();
	fgCaHDMIInstallHdcpKey(pbhdcpkey, HDCPKEY_LENGTH_DRM);
	ret = fgCaHDMIGetAKsv(u1CaHdcpAKsv);
	if (ret == TRUE)
		hdcp_key_load_status = 1;
#else
	TX_DEF_LOG("can not get hdcp key by this case\n");
#endif
#else

	HDMI_HDCP_FUNC();
	TX_DEF_LOG("need internal HDCP\n");

#endif
}

void VHdmiMuteVideoAudio(unsigned char u1flagvideomute, unsigned char u1flagaudiomute)
{
	if (u1flagvideomute == TRUE)
		vBlackHDMIOnly();
	else
		vUnBlackHDMIOnly();

	if (u1flagaudiomute == TRUE)
		MuteHDMIAudio();
	else
		UnMuteHDMIAudio();

}

void vDrm_mutehdmi(unsigned char u1flagvideomute, unsigned char u1flagaudiomute)
{
	HDMI_HDCP_LOG("u1flagvideomute = %d, u1flagaudiomute = %d\n", u1flagvideomute,
		      u1flagaudiomute);

	VHdmiMuteVideoAudio(u1flagvideomute, u1flagaudiomute);
}

void vSvp_mutehdmi(unsigned char u1svpvideomute, unsigned char u1svpaudiomute)
{
	HDMI_HDCP_LOG("u1svpvideomute = %d, u1svpaudiomute = %d\n", u1svpvideomute, u1svpaudiomute);

	VHdmiMuteVideoAudio(u1svpvideomute, u1svpaudiomute);
}

void vClearHdmiCmd(void)
{
	HDMI_DRV_FUNC();
	hdmi_hdmiCmd = 0xff;
}

void vHDMIClearINT(void)
{
	HDMI_HDCP_FUNC();

	vWriteByteHdmiGRL(TOP_INT_CLR00, 0xfffffff0);
	vWriteByteHdmiGRL(TOP_INT_CLR01, 0xffffffff);
	udelay(1);
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x0);
	vWriteByteHdmiGRL(TOP_INT_CLR01, 0x0);
}

void vMoveHDCPInternalKey(HDMI_HDCP_KEY_T key)
{
	unsigned char *pbDramAddr;
	unsigned short i;

	HDMI_HDCP_FUNC();

	bhdcpkey = key;

	pbDramAddr = bHdcpKeyBuff;
	for (i = 0; i < 287; i++) {
		if (key == INTERNAL_ENCRYPT_KEY)
			pbDramAddr[i] = HDCP_ENCRYPT_KEY[i];
		else if (key == INTERNAL_NOENCRYPT_KEY)
			pbDramAddr[i] = HDCP_NOENCRYPT_KEY[i];
		else if (key == EXTERNAL_KEY)
			pbDramAddr[i] = bHdcpKeyExternalBuff[i];
	}
}

void vHalHDCP2x_Reset(void)
{
	/* Reset hdcp 2.x */
	HDMI_HDCP_LOG("0xc68=0x%x, %lums\n",
		      bReadByteHdmiGRL(HDCP2X_DDCM_STATUS), jiffies);

	if (bReadByteHdmiGRL(HDCP2X_CTRL_0) & HDCP2X_ENCRYPT_EN) {
		vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_ENCRYPT_EN);
		mdelay(50);
	}

	vHdcpDdcHwPoll(FALSE);

	/* SOFT_HDCP_RST, SOFT_HDCP_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_HDCP_RST, SOFT_HDCP_RST);
#endif
	/* SOFT_HDCP_CORE_RST, SOFT_HDCP_CORE_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_HDCP_CORE_RST, SOFT_HDCP_CORE_RST);
#endif
	udelay(1);
	/* SOFT_HDCP_NOR, SOFT_HDCP_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	vCaHDMIWriteHDCPRST(SOFT_HDCP_NOR, SOFT_HDCP_RST);
#endif

	/* SOFT_HDCP_CORE_NOR, SOFT_HDCP_CORE_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	vCaHDMIWriteHDCPRST(SOFT_HDCP_CORE_NOR, SOFT_HDCP_CORE_RST);
#endif

}

void vHDCPReset(void)
{
	HDMI_HDCP_LOG("vHDCPReset\n");
	if (_bHdcpOff == TRUE)
		return;
	vHalHDCP1x_Reset();

	hdmi_ddc_request(3);
	vHalHDCP2x_Reset();
	hdmi_ddc_free(3);

	vSetHDCPState(HDCP_RECEIVER_NOT_READY);
	hdmi_audio_signal_state(0);
	_bReCheckBstatusCount = 0;
}

void vHDCP14InitAuth(void)
{
	vSetHDCPTimeOut(HDCP_WAIT_RES_CHG_OK_TIMEOUE);	/* 100 ms */
	vSetHDCPState(HDCP_WAIT_RES_CHG_OK);
	_bReCheckBstatusCount = 0;
}

void vHDCPInitAuth(void)
{
	HDMI_HDCP_LOG("vHDCPInitAuth, 0xc68=0x%x, %lums\n",
		      bReadByteHdmiGRL(HDCP2X_DDCM_STATUS), jiffies);

	vHDCPReset();
	if (hdcp2_version_flag == TRUE) {
		{
			if (load_bin_flag == FALSE)
				vSetHDCPTimeOut(HDCP2x_WAIT_LOADBIN_TIMEOUE);	/* 10 ms */
			else
				vSetHDCPTimeOut(HDCP2x_WAIT_RES_CHG_OK_TIMEOUE);	/* 500 ms */
		}
		vSetHDCPState(HDCP2x_WAIT_RES_CHG_OK);
	} else {
		vHDCP14InitAuth();
	}
}

void vRepeaterOnOff(unsigned char fgIsRep)
{
	if (fgIsRep == TRUE)
		vWriteHdmiGRLMsk(0xcd0, 1 << 5, 1 << 5);
	else
		vWriteHdmiGRLMsk(0xcd0, 0 << 5, 1 << 5);
}

void vStopAn(void)
{
	vWriteHdmiGRLMsk(0xcd0, 1 << 4, 1 << 4);
}

void vReadAn(unsigned char *AnValue)
{
	unsigned char bIndex;

	AnValue[0] = bReadByteHdmiGRL(0xcc0) & 0xff;
	AnValue[1] = (bReadByteHdmiGRL(0xcc0) & 0xff00) >> 8;
	AnValue[2] = (bReadByteHdmiGRL(0xcc0) & 0xff0000) >> 16;
	AnValue[3] = (bReadByteHdmiGRL(0xcc0) & 0xff000000) >> 24;
	AnValue[4] = bReadByteHdmiGRL(0xcc4) & 0xff;
	AnValue[5] = (bReadByteHdmiGRL(0xcc4) & 0xff00) >> 8;
	AnValue[6] = (bReadByteHdmiGRL(0xcc4) & 0xff0000) >> 16;
	AnValue[7] = (bReadByteHdmiGRL(0xcc4) & 0xff000000) >> 24;
	for (bIndex = 0; bIndex < 8; bIndex++)
		TX_DEF_LOG("[1x]AnValue[%d] =0x%02x\n", bIndex, AnValue[bIndex]);

}

void vSendAn(void)
{
	unsigned char bHDCPBuf[HDCP_AN_COUNT];
	/* Step 1: issue command to general a new An value */
	/* (1) read the value first */
	/* (2) set An control as stop to general a An first */
	vStopAn();

	/* Step 2: Read An from Transmitter */
	vReadAn(bHDCPBuf);
	/* Step 3: Send An to Receiver */
	fgDDCDataWrite(RX_ID, RX_REG_HDCP_AN, HDCP_AN_COUNT, bHDCPBuf);

}

void vWriteBksvToTx(unsigned char *bBKsv)
{
	unsigned int temp;

	HDMI_HDCP_LOG("bksv 0x%x; 0x%x; 0x%x; 0x%x; 0x%x\n", bBKsv[0], bBKsv[1], bBKsv[2],
		      bBKsv[3], bBKsv[4]);
	temp =
	    (((bBKsv[3]) & 0xff) << 24) + (((bBKsv[2]) & 0xff) << 16) + (((bBKsv[1]) & 0xff) << 8) +
	    (bBKsv[0] & 0xff);
	vWriteByteHdmiGRL(0xcb0, temp);
	udelay(10);
	vWriteByteHdmiGRL(0xcb4, bBKsv[4]);

	vWriteHdmiGRLMsk(0xcd0, 1 << 0, 1 << 0);
	udelay(100);
	vWriteHdmiGRLMsk(0xcd0, 0 << 0, 1 << 0);

	HDMI_HDCP_LOG("[1x]bksv 0xcb0 =0x%08x\n", bReadByteHdmiGRL(0xcb0));
	HDMI_HDCP_LOG("[1x]bksv 0xcb4 =0x%08x\n", bReadByteHdmiGRL(0xcb4));

}

#ifdef SRM_SUPPORT
void vCompareSRM(void)
{
	unsigned int dwKsvInx = 0, dwVRLIndex = 0;
	unsigned char *ptrSRM, bNomOfDevice = 0, bKSV_Sink_Index = 0, bIndex = 0, dwIndex = 0;

	if (_rSRMInfo.bID != 0x80)
		return;


#ifdef SRM_DBG
	{
		HDMI_HDCP_LOG("[HDCP]SRM Count = %d ", _rSRMInfo.dwVRLLenInDram);
		HDMI_HDCP_LOG("[HDCP]Key=%x, %x, %x, %x, %x", _bHdcp_Bksv[0], _bHdcp_Bksv[1],
			      _bHdcp_Bksv[2], _bHdcp_Bksv[3], _bHdcp_Bksv[4]);
	}

#endif
#ifdef REVOKE_TEST
	_bHdcp_Bksv[0] = 0x6b;
	_bHdcp_Bksv[1] = 0x3b;
	_bHdcp_Bksv[2] = 0x60;
	_bHdcp_Bksv[3] = 0xa0;
	_bHdcp_Bksv[4] = 0x54;
#endif

	vSetSharedInfo(SI_DVD_HDCP_REVOCATION_RESULT, REVOCATION_NOT_CHK);

	dwVRLIndex = 0;
	ptrSRM = &_bSRMBuff[8];
	while (_rSRMInfo.dwVRLLenInDram > dwVRLIndex) {
		bNomOfDevice = *(ptrSRM + dwVRLIndex) & 0x7F;	/* 40*N */
		dwVRLIndex++;
		for (dwKsvInx = 0; dwKsvInx < bNomOfDevice; dwKsvInx++) {
			for (dwIndex = 0; dwIndex < 5; dwIndex++) {
				if (*(ptrSRM + dwVRLIndex + (dwKsvInx * 5) + dwIndex) !=
				    _bHdcp_Bksv[dwIndex])
					break;
			}



			if (fgIsRepeater()) {
				for (bKSV_Sink_Index = 0;
				     bKSV_Sink_Index < i4SharedInfo(SI_REPEATER_DEVICE_COUNT);
				     bKSV_Sink_Index++) {
					for (bIndex = 0; bIndex < 5; bIndex++) {
					if (((bKSV_Sink_Index + 1) * 5 - bIndex - 1) < 192) {
					if (*
					    (ptrSRM + dwVRLIndex + (dwKsvInx * 5) +
					     bIndex) !=
					    bKsv_buff[(bKSV_Sink_Index + 1) * 5 -
						      bIndex - 1])
						break;
					}
					}
					if (bIndex == 5)
						break;
				}
			}

			if ((dwIndex == 5) || (bIndex == 5)) {
				vSetSharedInfo(SI_DVD_HDCP_REVOCATION_RESULT,
					       REVOCATION_IS_CHK | IS_REVOCATION_KEY);
				break;
			}
			vSetSharedInfo(SI_DVD_HDCP_REVOCATION_RESULT,
				       REVOCATION_IS_CHK | NOT_REVOCATION_KEY);
		}

		if ((dwIndex == 5) || (bIndex == 5)) {	/* Found, revolution key, break the while loop */

			break;
		}
		dwVRLIndex += bNomOfDevice * 5;
	}

#ifdef SRM_DBG
	{
		HDMI_HDCP_LOG("[HDCP]Shared Info=%x", i4SharedInfo(SI_DVD_HDCP_REVOCATION_RESULT));
		if (i4SharedInfo(SI_DVD_HDCP_REVOCATION_RESULT) & IS_REVOCATION_KEY)
			HDMI_HDCP_LOG("[HDCP]Revoked Sink Key\n");
	}
#endif

}
#endif

bool isKsvLegal(unsigned char ksv[HDCP_AKSV_COUNT])
{
	unsigned char i, bit_shift, one_cnt;

	one_cnt = 0;
	for (i = 0; i < HDCP_AKSV_COUNT; i++) {
		for (bit_shift = 0; bit_shift < 8; bit_shift++)
			if (ksv[i] & BIT(bit_shift))
				one_cnt++;
	}
	if (one_cnt == 20)
		return true;
	else {
		TX_DEF_LOG("[HDCP] %s,err ksv is:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\n",
			__func__, ksv[0], ksv[1], ksv[2], ksv[3], ksv[4]);
		return false;
	}
}

void vExchangeKSVs(void)
{
	unsigned char bHDCPBuf[HDCP_AKSV_COUNT];
#ifdef SRM_SUPPORT
	unsigned char bIndx;
#endif
	/* Step 1: read Aksv from transmitter, and send to receiver */
	TX_DEF_LOG("HDMI_AKSV:0x%x 0x%x 0x%x 0x%x 0x%x\n",
		HDMI_AKSV[0], HDMI_AKSV[1], HDMI_AKSV[2], HDMI_AKSV[3], HDMI_AKSV[4]);

	fgDDCDataWrite(RX_ID, RX_REG_HDCP_AKSV, HDCP_AKSV_COUNT, HDMI_AKSV);

	/* Step 4: read Bksv from receiver, and send to transmitter */
	fgDDCDataRead(RX_ID, RX_REG_HDCP_BKSV, HDCP_BKSV_COUNT, bHDCPBuf);
	HDMI_HDCP_LOG("bHDCPBuf 0x%x; 0x%x; 0x%x; 0x%x; 0x%x\n", bHDCPBuf[0], bHDCPBuf[1],
		      bHDCPBuf[2], bHDCPBuf[3], bHDCPBuf[4]);
	vWriteBksvToTx(bHDCPBuf);
	TX_DEF_LOG("BSKV:0x%x 0x%x 0x%x 0x%x 0x%x\n",
		bHDCPBuf[0], bHDCPBuf[1], bHDCPBuf[2], bHDCPBuf[3], bHDCPBuf[4]);

	for (bIndx = 0; bIndx < HDCP_AKSV_COUNT; bIndx++)
		_bTxBKAV[bIndx] = bHDCPBuf[bIndx];

#ifdef SRM_SUPPORT
	for (bIndx = 0; bIndx < HDCP_AKSV_COUNT; bIndx++)	/* for revocation list compare purpose. */
		_bHdcp_Bksv[bIndx] = bHDCPBuf[HDCP_AKSV_COUNT - bIndx - 1];
	/* _bHdcp_Bksv[bIndx] = bHDCPBuf[bIndx]; */

	vCompareSRM();

#endif

}

void vSendAKey(unsigned char *bAkey)
{
	unsigned int i;

	for (i = 0; i < 280; i++) {
		vWriteByteHdmiGRL(0xcc8, *(bAkey + i));
		udelay(10);
	}
}

unsigned char bCheckHDCPRiStatus(void)
{
	HDMI_HDCP_FUNC();

	if (bReadByteHdmiGRL(0xcf4) & (1 << 25))
		return TRUE;
	else
		return FALSE;

}

bool fgCompareRi(void)
{
	unsigned char bTemp;
	unsigned char bHDCPBuf[4];

	HDMI_HDCP_FUNC();

	bHDCPBuf[2] = bReadByteHdmiGRL(0xc78) & 0xff;
	bHDCPBuf[3] = (bReadByteHdmiGRL(0xc78) >> 8) & 0xff;

	/* Read R0'/ Ri' from Receiver */
	fgDDCDataRead(RX_ID, RX_REG_RI, HDCP_RI_COUNT, bHDCPBuf);

	if (e_hdcp_ctrl_state == HDCP_COMPARE_R0)
		TX_DEF_LOG("[HDCP1.x][R0]Rx_Ri=0x%x%x Tx_Ri=0x%x%x\n", bHDCPBuf[0],
			  bHDCPBuf[1], bHDCPBuf[2], bHDCPBuf[3]);
	else
	HDMI_HDCP_LOG("[HDCP1.x]Rx_Ri=0x%x%x Tx_Ri=0x%x%x\n", bHDCPBuf[0],
			  bHDCPBuf[1], bHDCPBuf[2], bHDCPBuf[3]);

	/* compare R0 and R0' */
	for (bTemp = 0; bTemp < HDCP_RI_COUNT; bTemp++) {
		if (bHDCPBuf[bTemp] == bHDCPBuf[bTemp + HDCP_RI_COUNT]) {
			continue;
		} else {	/* R0 != R0' */

			break;
		}
	}

	/* return the compare result */
	if (bTemp == HDCP_RI_COUNT) {
		_bHdcpStatus = SV_OK;
		return TRUE;
	}
	{
		_bHdcpStatus = SV_FAIL;
		TX_DEF_LOG("[HDCP][1.x]Rx_Ri=0x%x%x Tx_Ri=0x%x%x\n", bHDCPBuf[0],
			      bHDCPBuf[1], bHDCPBuf[2], bHDCPBuf[3]);
		return FALSE;
	}

}

void vEnableEncrpt(void)
{
	HDMI_HDCP_FUNC();

	vWriteHdmiGRLMsk(0xcd0, 1 << 6, 1 << 6);
}

void vHalWriteKsvListPort(unsigned char *prKsvData, unsigned char bDevice_Count,
			  unsigned char *prBstatus)
{
	unsigned char bIndex;

	HDMI_HDCP_FUNC();

	if ((bDevice_Count * 5) < KSV_BUFF_SIZE) {
		vWriteByteHdmiGRL(0xc1c, (*(prBstatus)) + ((*(prBstatus + 1)) << 8));
		vWriteHdmiGRLMsk(0xcd0, (1 << 3), (1 << 3));
		HDMI_HDCP_LOG("[HDCP]0xc1c = 0x%08x\n", bReadByteHdmiGRL(0xc1c));

		for (bIndex = 0; bIndex < (bDevice_Count * 5); bIndex++) {
			HDMI_HDCP_LOG("[HDCP]0xcd4 =0x%08x\n", (*(prKsvData + bIndex)) + (1 << 8));
			vWriteByteHdmiGRL(0xcd4, (*(prKsvData + bIndex)) + (1 << 8));
		}
	}

}

void vHalWriteHashPort(unsigned char *prHashVBuff)
{
	unsigned char bIndex;

	HDMI_HDCP_FUNC();

	for (bIndex = 0; bIndex < 5; bIndex++) {
		HDMI_HDCP_LOG("[HDCP]write v,0x%08x =0x%08x\n", (0xce0 + bIndex * 4),
			      ((*(prHashVBuff + 3 + bIndex * 4)) << 24) +
			      ((*(prHashVBuff + 2 + bIndex * 4)) << 16) +
			      ((*(prHashVBuff + 1 + bIndex * 4)) << 8) +
			      (*(prHashVBuff + 0 + bIndex * 4)));

	}
	vWriteByteHdmiGRL(0xce0,
			  ((*(prHashVBuff + 3)) << 24) + ((*(prHashVBuff + 2)) << 16) +
			  ((*(prHashVBuff + 1)) << 8) + (*(prHashVBuff + 0)));
	vWriteByteHdmiGRL(0xce4,
			  ((*(prHashVBuff + 7)) << 24) + ((*(prHashVBuff + 6)) << 16) +
			  ((*(prHashVBuff + 5)) << 8) + (*(prHashVBuff + 4)));
	vWriteByteHdmiGRL(0xce8,
			  ((*(prHashVBuff + 11)) << 24) + ((*(prHashVBuff + 10)) << 16) +
			  ((*(prHashVBuff + 9)) << 8) + (*(prHashVBuff + 8)));
	vWriteByteHdmiGRL(0xcec,
			  ((*(prHashVBuff + 15)) << 24) + ((*(prHashVBuff + 14)) << 16) +
			  ((*(prHashVBuff + 13)) << 8) + (*(prHashVBuff + 12)));
	vWriteByteHdmiGRL(0xcf0,
			  ((*(prHashVBuff + 19)) << 24) + ((*(prHashVBuff + 18)) << 16) +
			  ((*(prHashVBuff + 17)) << 8) + (*(prHashVBuff + 16)));
	for (bIndex = 0; bIndex < 5; bIndex++) {
		HDMI_HDCP_LOG("[HDCP]read v,0x%08x =0x%08x\n", (0xce0 + bIndex * 4),
			      bReadByteHdmiGRL(0xce0 + bIndex * 4));

	}
}

void vReadKSVFIFO(void)
{
	unsigned char bTemp, bIndex, bDevice_Count;	/* , bBlock; */
	unsigned char bStatus[2], bBstatus1;
	unsigned int u2TxBStatus;

	HDMI_HDCP_FUNC();

	fgDDCDataRead(RX_ID, RX_REG_BSTATUS1 + 1, 1, &bBstatus1);
	fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 1, &bDevice_Count);
	_u2TxBStatus = (((unsigned int)bBstatus1) << 8) | bDevice_Count;

	bDevice_Count &= DEVICE_COUNT_MASK;

	if ((bDevice_Count & MAX_DEVS_EXCEEDED) || (bBstatus1 & MAX_CASCADE_EXCEEDED)) {

		fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 2, bStatus);
		fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 1, &bDevice_Count);
		bDevice_Count &= DEVICE_COUNT_MASK;
		u2TxBStatus = bStatus[0] | (bStatus[1] << 8);
		vSetSharedInfo(SI_REPEATER_DEVICE_COUNT, bDevice_Count);
		if (i4SharedInfo(SI_REPEATER_DEVICE_COUNT) == 0)
			_bDevice_Count = 0;
		else
			_bDevice_Count = bDevice_Count;

		_u2TxBStatus = u2TxBStatus;
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP1.x]TX BSTATUS: bStatus[0]=%x, bStatus[1]=%x, u2TxBStatus=%x\n",
		     bStatus[0], bStatus[1], u2TxBStatus);
		vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
		vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		_bHdcpStatus = SV_FAIL;
		return;
	}

	if (bDevice_Count > 32) {
		for (bTemp = 0; bTemp < 2; bTemp++) {	/* retry 1 times */
			fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 1, &bDevice_Count);
			bDevice_Count &= DEVICE_COUNT_MASK;
			if (bDevice_Count <= 32)
				break;
		}
		if (bTemp == 2)
			bDevice_Count = 32;
	}

	vSetSharedInfo(SI_REPEATER_DEVICE_COUNT, bDevice_Count);

	if (bDevice_Count == 0) {
		for (bIndex = 0; bIndex < 5; bIndex++)
			bKsv_buff[bIndex] = 0;

		for (bIndex = 0; bIndex < 2; bIndex++)
			bStatus[bIndex] = 0;

		for (bIndex = 0; bIndex < 20; bIndex++)
			bSHABuff[bIndex] = 0;

		vWriteHdmiGRLMsk(0xcd0, (1 << 11), (1 << 11));
	} else {
		fgDDCDataRead(RX_ID, RX_REG_KSV_FIFO, bDevice_Count * 5, bKsv_buff);
		vWriteHdmiGRLMsk(0xcd0, (0 << 11), (1 << 11));
	}

	HDMI_HDCP_LOG("[1x]bDevice_Count = %d\n", bDevice_Count);

	fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 2, bStatus);
	fgDDCDataRead(RX_ID, RX_REG_REPEATER_V, 20, bSHABuff);

	u2TxBStatus = bStatus[0] | (bStatus[1] << 8);
	HDMI_HDCP_LOG("[1x]TX BSTATUS: bStatus[0]=%x, bStatus[1]=%x, u2TxBStatus=%x\n",
		   bStatus[0], bStatus[1], u2TxBStatus);


#ifdef SRM_SUPPORT
	vCompareSRM();
#endif

	vHalWriteKsvListPort(bKsv_buff, bDevice_Count, bStatus);
	vHalWriteHashPort(bSHABuff);

	vSetHDCPState(HDCP_COMPARE_V);
	/* set time-out value as 0.5 sec */
	vSetHDCPTimeOut(HDCP_WAIT_V_RDY_TIMEOUE);

	for (bIndex = 0; bIndex < bDevice_Count; bIndex++) {
		if ((bIndex * 5 + 4) < KSV_BUFF_SIZE) {
			TX_DEF_LOG("[HDCP][1.x]Tx KSV List: Device[%d]= %x, %x, %x, %x, %x\n",
				   bIndex, bKsv_buff[bIndex * 5], bKsv_buff[bIndex * 5 + 1],
				   bKsv_buff[bIndex * 5 + 2], bKsv_buff[bIndex * 5 + 3],
				   bKsv_buff[bIndex * 5 + 4]);
		}
	}
	TX_DEF_LOG("[HDCP][1.x]Tx BKSV: %x, %x, %x, %x, %x\n", _bTxBKAV[0], _bTxBKAV[1],
		   _bTxBKAV[2], _bTxBKAV[3], _bTxBKAV[4]);

	if (i4SharedInfo(SI_REPEATER_DEVICE_COUNT) == 0)
		_bDevice_Count = 0;
	else
		_bDevice_Count = bDevice_Count;

	_u2TxBStatus = u2TxBStatus;

	HDMI_HDCP_LOG("[1x]_bDevice_Count = %x, _u2TxBStatus = %x\n", _bDevice_Count,
		   _u2TxBStatus);

}

unsigned int uiReadHDCPStatus(void)
{
	HDMI_HDCP_LOG("[1x]0xcf4 = 0x%08x\n", bReadByteHdmiGRL(0xcf4));
	return bReadByteHdmiGRL(0xcf4);
}

unsigned int uiReadIRQStatus01(void)
{
	HDMI_HDCP_LOG("[1x]0x1ac = 0x%08x\n", bReadByteHdmiGRL(0x1ac));
	HDMI_HDCP_LOG("[1x]0x1bc = 0x%08x\n", bReadByteHdmiGRL(0x1bc));

	return bReadByteHdmiGRL(0x1ac);
}


void vWriteAksvKeyMask(unsigned char *PrData)
{
	unsigned char bData;
	/* - write wIdx into 92. */

	bData = 0x00;		/*( *(PrData+2) & 0x0f) | ((*(PrData+3)& 0x0f) << 4); */
	vWriteHdmiGRLMsk(0xccc, (bData << 24), 0xff << 24);
	bData = 0x00;		/*(*(PrData+0) & 0x0f) | ((*(PrData+1)& 0x0f) << 4); */
	vWriteHdmiGRLMsk(0xccc, (bData << 16), 0xff << 16);
}

void vAKeyDone(void)
{
	HDMI_HDCP_FUNC();

	vWriteHdmiGRLMsk(0xcd0, 1 << 2, 1 << 2);
	udelay(100);
	vWriteHdmiGRLMsk(0xcd0, 0 << 0, 1 << 2);

}

unsigned int bHDMIHDCP2Err(void)
{
	return (bReadByteHdmiGRL(HDCP2X_STATUS_0) & HDCP2X_STATE) >> 16;
}

bool fgHDMIHdcp2Err(void)
{
	unsigned int hdcp2_txstate;

	hdcp2_txstate = bHDMIHDCP2Err();
	if ((hdcp2_txstate == 0x30)
	    || (hdcp2_txstate == 0x31)
	    || (hdcp2_txstate == 0x32)
	    || (hdcp2_txstate == 0x33)
	    || (hdcp2_txstate == 0x34)
	    || (hdcp2_txstate == 0x35)
	    || (hdcp2_txstate == 0x36)
	    || (hdcp2_txstate == 0x37)
	    || (hdcp2_txstate == 0x38)
	    || (hdcp2_txstate == 0x39)
	    || (hdcp2_txstate == 0x3a)
	    || (hdcp2_txstate == 0x3b)
	    || (hdcp2_txstate == 0x3c)
	    || (hdcp2_txstate == 0x3d)
	    || (hdcp2_txstate == 0x3e))
		return TRUE;
	return FALSE;
}

bool fgHDMIHdcp2Auth(void)
{
	unsigned int hdcp2_txstate;

	hdcp2_txstate = bHDMIHDCP2Err();
	if ((hdcp2_txstate == 3)
	    || (hdcp2_txstate == 4)
	    || (hdcp2_txstate == 11)
	    || (hdcp2_txstate == 14)
	    || (hdcp2_txstate == 16)
	    || (hdcp2_txstate == 18)
	    || (hdcp2_txstate == 41)
	    || (hdcp2_txstate == 24))
		return TRUE;
	return FALSE;
}

unsigned char u1CountNum1(unsigned char u1Data)
{
	unsigned char i, bCount = 0;

	for (i = 0; i < 8; i++) {
		if (((u1Data >> i) & 0x01) == 0x01)
			bCount++;
	}
	return bCount;
}

void dump_hdcp22(void)
{
	unsigned int temp;

	TX_DEF_LOG("dump hdcp22 start\n");

	TX_DEF_LOG("[hdcp]state=%d\n", (unsigned int)e_hdcp_ctrl_state);
	TX_DEF_LOG("[hdcp]hdcp2_version_flag=%d\n", hdcp2_version_flag);
	TX_DEF_LOG("[hdcp]hdcp_key_load_status=%d\n", hdcp_key_load_status);
	TX_DEF_LOG("[hdcp]hdcp_unmute_logo_flag=%d\n", hdcp_unmute_logo_flag);
	TX_DEF_LOG("[hdcp]_bHdcpOff=%d\n", _bHdcpOff);
	TX_DEF_LOG("[hdcp]hdcp_unmute_start_flag=%d\n", hdcp_unmute_start_flag);

	TX_DEF_LOG("0x000001a8 = 0x%08x\n", bReadByteHdmiGRL(0x1a8));
	for (temp = 0xc20; temp <= 0xc54; temp = temp + 4)
		TX_DEF_LOG("0x%08x = 0x%08x\n", temp, bReadByteHdmiGRL(temp));
	TX_DEF_LOG("0x00000c60 = 0x%08x\n", bReadByteHdmiGRL(0xc60));
	TX_DEF_LOG("0x00000c8c = 0x%08x\n", bReadByteHdmiGRL(0xc8c));
	TX_DEF_LOG("0x00000c90 = 0x%08x\n", bReadByteHdmiGRL(0xc90));
	TX_DEF_LOG("0x00000c94 = 0x%08x\n", bReadByteHdmiGRL(0xc94));
	TX_DEF_LOG("0x00000c98 = 0x%08x\n", bReadByteHdmiGRL(0xc98));
	TX_DEF_LOG("0x00000c9c = 0x%08x\n", bReadByteHdmiGRL(0xc9c));
	TX_DEF_LOG("0x00000cf4 = 0x%08x\n", bReadByteHdmiGRL(0xcf4));

	TX_DEF_LOG("dump hdcp22 done\n");
}

static int hdmi_hdcp22_monitor_state;
static int hdmi_hdcp22_state_id;

void hdmi_hdcp22_monitor_init(void)
{
	hdmi_hdcp22_monitor_state = 0;
	hdcp_err_0x30_count = 0;
}

void hdmi_hdcp22_monitor_start(void)
{
	if (hdmi_hdcp22_monitor_state == 0) {
		hdmi_hdcp22_monitor_state = 1;
		hdmi_hdcp22_state_id = 0;
	}
}

void hdmi_hdcp22_monitor_stop(void)
{
	hdmi_hdcp22_monitor_state = 0xff;
}

void hdmi_hdcp22_monitor(void)
{
	if (hdmi_hdcp22_monitor_state == 0xff)
		return;

	if (hdmi_hotplugstate != HDMI_STATE_HOT_PLUGIN_AND_POWER_ON) {
		hdmi_hdcp22_monitor_stop();
		return;
	}

	if (hdmi_hdcp22_monitor_state == 1) {
		if (fgHDMIHdcp2Err()) {
			TX_DEF_LOG("the first hdcp22 err\n");
			dump_hdcp22();
			hdmi_hdcp22_state_id = bHDMIHDCP2Err();
			hdmi_hdcp22_monitor_state = 2;
		}
	} else if (hdmi_hdcp22_monitor_state == 2) {
		if (fgHDMIHdcp2Err()) {
			if (hdmi_hdcp22_state_id != bHDMIHDCP2Err()) {
				TX_DEF_LOG("the other hdcp22 err\n");
				dump_hdcp22();
				hdmi_hdcp22_state_id = bHDMIHDCP2Err();
			}
		}
	}
}

bool hdcp_check_err_0x30(void)
{
	if (fgHDMIHdcp2Err()) {
		if (bHDMIHDCP2Err() == 0x30)
			hdcp_err_0x30_count++;
		else
			hdcp_err_0x30_count = 0;
		if (hdcp_err_0x30_count > 3) {
			hdcp_err_0x30_count = 0;
			hdcp_err_0x30_flag = 1;
			vWriteIoHdmiAnaMsk(HDMI20_CFG_0, 0, RG_HDMITX20_DRV_EN);
			TX_DEF_LOG("err=0x30, signal off\n");
			return TRUE;
		}
	}
	return FALSE;
}

void HdcpService(HDCP_CTRL_STATE_T e_hdcp_state)
{
	unsigned char bIndx, bTemp, BStatus[2] = {0};
	unsigned char bRptID[155];
	unsigned int readvalue, i, devicecnt, uitemp1, uitemp2, depth, count1;
	bool fgRepeaterError = FALSE;
	unsigned char ta_status[2];

#if 0 /* disable this as secure device is available for development, keep code for debug */
	if (hdcp_key_load_status == 0) {
		pr_info("hdmi hdcp key not loaded disable hdcp, device may be non-secure===>\n");
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaaaaaa);
#endif
		_bHdcpOff = 1;
	}
#endif
	if (_bHdcpOff == 1) {
		HDMI_HDCP_LOG("_bHdcpOff==1\n");
		vSetHDCPState(HDCP_RECEIVER_NOT_READY);
		vHDMIAVUnMute();
		hdmi_hdcp22_monitor_stop();
		return;
	}

	switch (e_hdcp_state) {
	case HDCP_RECEIVER_NOT_READY:
		HDMI_HDCP_LOG("HDCP_RECEIVER_NOT_READY\n");
		break;

	case HDCP_READ_EDID:
		break;

	case HDCP_WAIT_RES_CHG_OK:
		if (fgIsHDCPCtrlTimeOut()) {
			if (_bHdcpOff == 1) {	/* disable HDCP */
				vSetHDCPState(HDCP_RECEIVER_NOT_READY);
				vHDMIAVUnMute();
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			} else {
				vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				_bLastHdcpStatus = HDCP_RECEIVER_NOT_READY;
			}
		}
		break;

	case HDCP_INIT_AUTHENTICATION:
		/* HDCP_1P4_TCLK_EN, HDCP_1P4_TCLK_EN); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(HDCP_1P4_TCLK_EN, HDCP_1P4_TCLK_EN);
#endif
		vHDMIAVMute();
		vSetSharedInfo(SI_HDMI_HDCP_RESULT, 0);

		hdmi_ddc_request(5);
		hdmi_ddc_free(5);

		if (!fgDDCDataRead(RX_ID, RX_REG_BCAPS, 1, &bTemp)) {
			HDMI_HDCP_LOG("[1x]fail-->HDCP_INIT_AUTHENTICATION-->0\n");
			_bHdcpStatus = SV_FAIL;
			vHDCPEncryptState(0); /* for audio notify */
			vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);
			break;
		}
		if(!vIsDviMode()) {
			fgDDCDataRead(RX_ID, RX_REG_BSTATUS1, 2, BStatus);
			if ((BStatus[1] & 0x10) == 0) {
				TX_DEF_LOG("[1x]BStatus=0x%x,0x%x\n",BStatus[0], BStatus[1]);
				_bReCheckBstatusCount++;
				/* wait for upto 4.5 seconds to detect otherwise proceed anyway */
				if(_bReCheckBstatusCount < 15) {
					_bHdcpStatus = SV_FAIL;
					vHDCPEncryptState(0); /* for audio notify */
					vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);
					break;
				}
			}
		}

		TX_DEF_LOG("[1x]RX_REG_BCAPS = 0x%08x\n", bTemp);
		for (bIndx = 0; bIndx < HDCP_AKSV_COUNT; bIndx++) {
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
			HDMI_AKSV[bIndx] = u1CaHdcpAKsv[bIndx];
#else
			HDMI_AKSV[bIndx] = bHdcpKeyBuff[1 + bIndx];
#endif
			HDMI_HDCP_LOG("[1x]HDMI_AKSV[%d] = 0x%x\n", bIndx,
				   HDMI_AKSV[bIndx]);
		}

#ifndef NO_ENCRYPT_KEY_TEST
		vWriteAksvKeyMask(&HDMI_AKSV[0]);
#endif

		fgDDCDataRead(RX_ID, RX_REG_BCAPS, 1, &bTemp);
		vSetSharedInfo(SI_REPEATER_DEVICE_COUNT, 0);
		if (bTemp & RX_BIT_ADDR_RPTR) {
			_fgRepeater = TRUE;
		} else {
			_fgRepeater = FALSE;
		}

		if (fgIsRepeater())
			vRepeaterOnOff(TRUE);
		else
			vRepeaterOnOff(FALSE);

		vSendAn();
		vExchangeKSVs();
		if ((!isKsvLegal(HDMI_AKSV)) || (!isKsvLegal(_bTxBKAV))) {
			HDMI_HDCP_LOG("[1x]fail-->HDCP_INIT_AUTHENTICATION-->isKsvLegal\n");
			vHDMIAVMute();
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			_bHdcpStatus = SV_FAIL;
			vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);
			break;
		}

#ifdef SRM_SUPPORT
		if ((i4SharedInfo(SI_DVD_HDCP_REVOCATION_RESULT) & IS_REVOCATION_KEY)
		    && (_rSRMInfo.bID == 0x80)) {
			HDMI_HDCP_LOG("[1x]fail-->HDCP_INIT_AUTHENTICATION-->1\n");
			vHDMIAVMute();
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			_bHdcpStatus = SV_FAIL;
			vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);	/* 2007/12/27 add 300 ms issue next coomand */
			break;

		}
#endif
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		fgCaHDMILoadHDCPKey();
#else
		vSendAKey(&bHdcpKeyBuff[6]);	/* around 190msec */
#endif

		/* set time-out value as 100 ms */
		vSetHDCPTimeOut(HDCP_WAIT_R0_TIMEOUT);
		vAKeyDone();

		/* change state as waiting R0 */
		vSetHDCPState(HDCP_WAIT_R0);

		break;


	case HDCP_WAIT_R0:
		bTemp = bCheckHDCPRiStatus();
		if (bTemp == TRUE) {
			vSetHDCPState(HDCP_COMPARE_R0);
		} else {
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			_bHdcpStatus = SV_FAIL;
			vHDCPEncryptState(0); /* for audio notify */
			break;
		}

	case HDCP_COMPARE_R0:

		if (fgCompareRi() == TRUE) {

			vEnableEncrpt();	/* Enabe encrption */

			/* change state as check repeater */
			vSetHDCPState(HDCP_CHECK_REPEATER);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			vSetSharedInfo(SI_HDMI_HDCP_RESULT, 0x01);	/* step 1 OK. */
		} else {
			vSetHDCPState(HDCP_RE_COMPARE_R0);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			TX_DEF_LOG("[1x]fail-->HDCP_WAIT_R0-->1\n");
			vHDCPEncryptState(0); /* for audio notify */
			_bReCompRiCount = 0;
		}

		break;

	case HDCP_RE_COMPARE_R0:

		_bReCompRiCount++;
		if (fgIsHDCPCtrlTimeOut() && _bReCompRiCount > 3) {
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			_bHdcpStatus = SV_FAIL;
			_bReCompRiCount = 0;
			vHDCPEncryptState(0); /* for audio notify */
			TX_DEF_LOG("[1x]fail-->HDCP_WAIT_R0-->2\n");

		} else {
			if (fgCompareRi() == TRUE) {
				vEnableEncrpt();	/* Enabe encrption */

				/* change state as check repeater */
				vSetHDCPState(HDCP_CHECK_REPEATER);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				vSetSharedInfo(SI_HDMI_HDCP_RESULT, 0x01);	/* step 1 OK. */
			} else {
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			}

		}
		break;

	case HDCP_CHECK_REPEATER:
		HDMI_HDCP_LOG("[1x]HDCP_CHECK_REPEATER\n");
		/* if the device is a Repeater, */
		if (fgIsRepeater()) {
			_bReCheckReadyBit = 0;
			vSetHDCPState(HDCP_WAIT_KSV_LIST);
			vSetHDCPTimeOut(HDCP_WAIT_KSV_LIST_TIMEOUT);
		} else {

			_bDevice_Count = 0;
			_u2TxBStatus = 0;
			vSetHDCPState(HDCP_WAIT_RI);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		}

		break;

	case HDCP_WAIT_KSV_LIST:

		fgDDCDataRead(RX_ID, RX_REG_BCAPS, 1, &bTemp);
		if ((bTemp & RX_BIT_ADDR_READY)) {
			_bReCheckReadyBit = 0;
			vSetHDCPState(HDCP_READ_KSV_LIST);
		} else if (_bReCheckReadyBit > HDCP_CHECK_KSV_LIST_RDY_RETRY_COUNT) {
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			_bReCheckReadyBit = 0;
			_bHdcpStatus = SV_FAIL;
			TX_DEF_LOG("[1x]HDCP_WAIT_KSV_LIST, fail\n");
			break;
		} else {
			_bReCheckReadyBit++;
			vSetHDCPState(HDCP_WAIT_KSV_LIST);
			vSetHDCPTimeOut(HDCP_WAIT_KSV_LIST_RETRY_TIMEOUT);
			break;
		}

	case HDCP_READ_KSV_LIST:

		vReadKSVFIFO();
#ifdef SRM_SUPPORT
		if ((i4SharedInfo(SI_DVD_HDCP_REVOCATION_RESULT) & IS_REVOCATION_KEY)
		    && (_rSRMInfo.bID == 0x80)) {
			vHDMIAVMute();
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			_bHdcpStatus = SV_FAIL;
			vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);	/* 2007/12/27 add 300 ms  issue next coomand */
			break;
		}
#endif
		break;

	case HDCP_COMPARE_V:

		uitemp1 = uiReadHDCPStatus();
		uitemp2 = uiReadIRQStatus01();
		vWriteByteHdmiGRL(TOP_INT_CLR01, 0x00004000);
		udelay(10);
		vWriteByteHdmiGRL(TOP_INT_CLR01, 0x00000000);
		if ((uitemp2 & (1 << 14)) || (uitemp1 & (1 << 28))) {
			if ((uitemp2 & (1 << 14))) {	/* for Simplay #7-20-5 */
				vSetHDCPState(HDCP_WAIT_RI);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				HDMI_HDCP_LOG("[1x]HDCP_COMPARE_V, pass\n");
				vSetSharedInfo(SI_HDMI_HDCP_RESULT,
					       (i4SharedInfo(SI_HDMI_HDCP_RESULT) | 0x02));
				/* step 2 OK. */
			} else {
				vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				TX_DEF_LOG("[1x]HDCP_COMPARE_V, fail\n");
				_bHdcpStatus = SV_FAIL;
			}
		} else {
			HDMI_HDCP_LOG("[HDCP]V Not RDY\n");
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			_bHdcpStatus = SV_FAIL;
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		}

		break;

	case HDCP_WAIT_RI:

		vHDMIAVUnMute();
		hdmi_audio_signal_state(1);
		hdcp_unmute_start_flag = TRUE;
		hdcp_set_unmute_start_time();
		TX_DEF_LOG("[HDCP1.x]pass, %lums\n", jiffies);
		vHDCPEncryptState(1); /* for audio notify */
		break;

	case HDCP_CHECK_LINK_INTEGRITY:
#ifdef SRM_SUPPORT
		if ((i4SharedInfo(SI_DVD_HDCP_REVOCATION_RESULT) & IS_REVOCATION_KEY) && (_rSRMInfo.bID == 0x80)) {
			vHDMIAVMute();
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			_bHdcpStatus = SV_FAIL;
			vHDCPEncryptState(0); /* for audio notify */
			vSetHDCPTimeOut(HDCP_WAIT_300MS_TIMEOUT);
			break;

		}
#endif
		if (fgCompareRi() == TRUE) {
			vSetSharedInfo(SI_HDMI_HDCP_RESULT,
				       (i4SharedInfo(SI_HDMI_HDCP_RESULT) | 0x04));
			/* step 3 OK. */
			if (fgIsRepeater()) {
				if (i4SharedInfo(SI_HDMI_HDCP_RESULT) == 0x07) {	/* step 1, 2, 3. */
					vSetSharedInfo(SI_HDMI_HDCP_RESULT,
						       (i4SharedInfo(SI_HDMI_HDCP_RESULT) | 0x08));
					/* all ok. */
				}
			} else {	/* not repeater, don't need step 2. */

				if (i4SharedInfo(SI_HDMI_HDCP_RESULT) == 0x05) {	/* step 1, 3. */
					vSetSharedInfo(SI_HDMI_HDCP_RESULT,
						       (i4SharedInfo(SI_HDMI_HDCP_RESULT) | 0x08));
					/* all ok. */
				}
			}
		} else {
			_bReCompRiCount = 0;
			vSetHDCPState(HDCP_RE_COMPARE_RI);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			vHDCPEncryptState(0); /* for audio notify */
			TX_DEF_LOG("[1x]fai-->HDCP_CHECK_LINK_INTEGRITY\n");

		}
		break;

	case HDCP_RE_COMPARE_RI:
		HDMI_HDCP_LOG("[1x]HDCP_RE_COMPARE_RI\n");
		_bReCompRiCount++;
		if (_bReCompRiCount > 5) {
			vSetHDCPState(HDCP_RE_DO_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			_bReCompRiCount = 0;
			_bHdcpStatus = SV_FAIL;
			TX_DEF_LOG("[1x]fai-->HDCP_RE_COMPARE_RI\n");
			vHDCPEncryptState(0); /* for audio notify */
		} else {
			if (fgCompareRi() == TRUE) {
				_bReCompRiCount = 0;
				vSetHDCPState(HDCP_CHECK_LINK_INTEGRITY);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				vSetSharedInfo(SI_HDMI_HDCP_RESULT,
					       (i4SharedInfo(SI_HDMI_HDCP_RESULT) | 0x04));
				/* step 3 OK. */
				if (fgIsRepeater()) {
					if (i4SharedInfo(SI_HDMI_HDCP_RESULT) == 0x07) {	/* step 1, 2, 3. */
						vSetSharedInfo(SI_HDMI_HDCP_RESULT,
							       (i4SharedInfo(SI_HDMI_HDCP_RESULT) |
								0x08));
						/* all ok. */
					}
				} else {
					if (i4SharedInfo(SI_HDMI_HDCP_RESULT) == 0x05) {	/* step 1, 3. */
						vSetSharedInfo(SI_HDMI_HDCP_RESULT,
							       (i4SharedInfo(SI_HDMI_HDCP_RESULT) |
								0x08));
						/* all ok. */
					}
				}

			} else {
				vSetHDCPState(HDCP_RE_COMPARE_RI);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			}
		}
		break;

	case HDCP_RE_DO_AUTHENTICATION:
		HDMI_HDCP_LOG("[1x]HDCP_RE_DO_AUTHENTICATION\n");
		vHDMIAVMute();
		vHDCPReset();
		if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) != HDMI_PLUG_IN_AND_SINK_POWER_ON) {
			vSetHDCPState(HDCP_RECEIVER_NOT_READY);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		} else {
			vSetHDCPState(HDCP_WAIT_RESET_OK);
			vSetHDCPTimeOut(HDCP_WAIT_RE_DO_AUTHENTICATION);

		}
		break;

	case HDCP_WAIT_RESET_OK:
		if (fgIsHDCPCtrlTimeOut()) {
			vSetHDCPState(HDCP_INIT_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		}
		break;

		/*hdcp2 code start here */
	case HDCP2x_WAIT_RES_CHG_OK:
		HDMI_HDCP_LOG("HDCP2x_WAIT_RES_CHG_OK, %lums\n", jiffies);
		if (fgIsHDCPCtrlTimeOut()) {
			if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) != HDMI_PLUG_IN_AND_SINK_POWER_ON) {
				vSetHDCPState(HDCP_RECEIVER_NOT_READY);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			} else if (_bHdcpOff == 1) {
				vSetHDCPState(HDCP_RECEIVER_NOT_READY);
				vHDMIAVUnMute();
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			} else {
				vHDMIAVMute();
				if (hdcp_err_0x30_flag == 1) {
					hdcp_err_0x30_flag = 0;
					hdmi_ddc_request(3);
					hdmi_ddc_free(3);
					if (bOver340M() == TRUE)
						vSendTMDSConfiguration(SCRAMBLING_ENABLE | TMDS_BIT_CLOCK_RATION);
					else
						vSendTMDSConfiguration(0);
					TX_DEF_LOG("err=0x30, signal on\n");
					vWriteIoHdmiAnaMsk(HDMI20_CFG_0, RG_HDMITX20_DRV_EN, RG_HDMITX20_DRV_EN);
				}
				_bReRepeaterPollCnt = 0;
				_bReCertPollCnt = 0;
				_bReAuthCnt = 0;
				_u14SeqMnum = 0;
				vHDMI2xClearINT();
				vSetHDCPState(HDCP2x_LOAD_BIN);
				/* vSetHDCPTimeOut(HDCP2x_WAIT_LOADBIN_TIMEOUE); */
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);

				vWriteHdmiGRLMsk(HDCP2X_RPT_SEQ_NUM, 0, HDCP2X_RPT_SEQ_NUM_M);
				vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_ENCRYPT_EN);
				vWriteHdmiGRLMsk(HPD_DDC_CTRL, DDC2_CLOK << DDC_DELAY_CNT_SHIFT,
						 DDC_DELAY_CNT);
				_bLastHdcpStatus = HDCP_RECEIVER_NOT_READY;
			}
		}
		break;

	case HDCP2x_LOAD_BIN:
		HDMI_HDCP_LOG("HDCP2x_LOAD_BIN, flag = %d, %lums\n", load_bin_flag,
			      jiffies);

		/* SOFT_HDCP_CORE_RST, SOFT_HDCP_CORE_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_HDCP_CORE_RST, SOFT_HDCP_CORE_RST);
#endif

		if (load_bin_flag == FALSE) {
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
			TX_DEF_LOG("[HDMI]CONFIG_MTK_IN_HOUSE_TEE_SUPPORT fgCaHDMILoadROM\n");
			fgCaHDMILoadROM();
#else
			TX_DEF_LOG("[HDMI]Normal Mode fgCaHDMILoadROM\n");
			for (readvalue = 0; readvalue < 0x8000; readvalue++) {
				udelay(1);	/* Trustzone no need this delay */
				vWriteByteHdmiGRL(PROM_CTRL,
						  (readvalue << PROM_ADDR_SHIFT) +
						  (hdcp_prom[readvalue] << PROM_WDATA_SHIFT) +
						  PROM_CS + PROM_WR);
			}

			vWriteByteHdmiGRL(PROM_CTRL, 0);

			for (readvalue = 0; readvalue < 0x4000; readvalue++) {
				udelay(1);	/* Trustzone no need this delay */
				vWriteByteHdmiGRL(PRAM_CTRL,
						  (readvalue << PRAM_ADDR_SHIFT) +
						  (hdcp_pram[readvalue] << PRAM_WDATA_SHIFT) +
						  PRAM_CTRL_SEL + PRAM_CS + PRAM_WR);
			}
			udelay(5);
			vWriteByteHdmiGRL(PRAM_CTRL, 0);
#endif
			load_bin_flag = TRUE;
		} else {
			mdelay(1);
			vWriteByteHdmiGRL(PRAM_CTRL,
					  (0x3fff << PRAM_ADDR_SHIFT) +
					  (hdcp_pram[0x3fff] << PRAM_WDATA_SHIFT) +
					  PRAM_CTRL_SEL + PRAM_CS + PRAM_WR);

			vWriteByteHdmiGRL(PROM_CTRL, 0);
			vWriteByteHdmiGRL(PRAM_CTRL, 0);
		}
		vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_CUPD_START, HDCP2X_CUPD_START);
		/* SOFT_HDCP_CORE_NOR, SOFT_HDCP_CORE_RST); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_HDCP_CORE_NOR, SOFT_HDCP_CORE_RST);
#endif

		/* HDCP_TCLK_EN, HDCP_TCLK_EN); */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(HDCP_TCLK_EN, HDCP_TCLK_EN);
#endif

		if (e_hdcp_ctrl_state != HDCP2x_LOAD_BIN) {
			TX_DEF_LOG("hdcp state changed by other thread\n");
			break;
		}

		vSetHDCPState(HDCP2x_INITAIL_OK);
		vSetHDCPTimeOut(HDCP2x_WAIT_INITAIL_TIMEOUE);

		break;

	case HDCP2x_INITAIL_OK:
		HDMI_HDCP_LOG("HDCP2x_INITAIL_OK, %lums\n", jiffies);
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		HDMI_HDCP_LOG("readvalue 1a8 = 0x%08x\n", readvalue);
		fgCaHDMIGetTAStatus(ta_status);
		if ((readvalue & HDCP2X_CCHK_DONE_INT_STA) &&
			((ta_status[0] & 0x01) == 0)) {
			HDMI_HDCP_LOG("hdcp2.2 ram/rom check is done\n");
			vSetHDCPState(HDCP2x_AUTHENTICATION);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		} else {
			TX_DEF_LOG("hdcp2.2 ram/rom check is fail, %x\n", ta_status[0]);
			vHDCPInitAuth();
			_bHdcpStatus = SV_FAIL;
			load_bin_flag = FALSE;
			vHDCPEncryptState(0); /* for audio notify */
		}
		break;

	case HDCP2x_AUTHENTICATION:
		HDMI_HDCP_LOG("HDCP2x_AUTHENTICATION, %lums\n", jiffies);
		/* enable reauth_req irq */
		vWriteHdmiGRLMsk(TOP_INT_MASK00, 0x02000000, 0x02000000);
		vWriteHdmiGRLMsk(HDCP2X_TEST_TP0, 0x75 << HDCP2X_TP1_SHIFT, HDCP2X_TP1);
		/* vWriteHdmiGRLMsk(HDCP2X_GP_IN, 0x22<<HDCP2X_GP_IN1_SHIFT, HDCP2X_GP_IN1); */
		vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_EN | HDCP2X_HDMIMODE,
				 HDCP2X_EN | HDCP2X_HDMIMODE | HDCP2X_ENCRYPT_EN);
		vWriteHdmiGRLMsk(SI2C_CTRL, RX_CAP_RD_TRIG, RX_CAP_RD_TRIG);

		vWriteHdmiGRLMsk(HDCP2X_POL_CTRL, 0x3402, HDCP2X_POL_VAL1 | HDCP2X_POL_VAL0);
		/* vWriteHdmiGRLMsk(HPD_DDC_CTRL, 0x62, DDC_DELAY_CNT); */
		vWriteByteHdmiGRL(HDCP2X_TEST_TP0, 0x2a019803);
		vWriteByteHdmiGRL(HDCP2X_TEST_TP1, 0x09026411);
		vWriteByteHdmiGRL(HDCP2X_TEST_TP2, 0xa7111110);
		vWriteByteHdmiGRL(HDCP2X_TEST_TP3, 0x00fa0d7d);
		vWriteHdmiGRLMsk(HDCP2X_GP_IN, 0x0 << HDCP2X_GP_IN2_SHIFT, HDCP2X_GP_IN2);
		vWriteHdmiGRLMsk(HDCP2X_GP_IN, 0x0 << HDCP2X_GP_IN3_SHIFT, HDCP2X_GP_IN3);

#if 1
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		/* set SM */
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa0000);
#endif
#else
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, HDCP2X_RPT_SMNG_WR_START, HDCP2X_RPT_SMNG_WR_START);

		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 1 << HDCP2X_RPT_SMNG_K_SHIFT, HDCP2X_RPT_SMNG_K);

		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0 << HDCP2X_RPT_SMNG_IN_SHIFT, HDCP2X_RPT_SMNG_IN);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, HDCP2X_RPT_SMNG_WR, HDCP2X_RPT_SMNG_WR);
		udelay(1);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0, HDCP2X_RPT_SMNG_WR);

		/* enforce type 1 only in 4K mode */
		if (/*(_u14kContentType) && */fgFMTis4k2k(_prAvdAVInfo->e_resolution))
			vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 1 << HDCP2X_RPT_SMNG_IN_SHIFT,
					 HDCP2X_RPT_SMNG_IN);
		else
			vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0 << HDCP2X_RPT_SMNG_IN_SHIFT,
					 HDCP2X_RPT_SMNG_IN);

		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, HDCP2X_RPT_SMNG_WR, HDCP2X_RPT_SMNG_WR);
		udelay(1);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0, HDCP2X_RPT_SMNG_WR);
#endif

		vHdcpDdcHwPoll(TRUE);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_REAUTH_SW, HDCP2X_REAUTH_SW);
		udelay(1);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_REAUTH_SW);
		vSetHDCPState(HDCP2x_CHECK_CERT_OK);
		vSetHDCPTimeOut(HDCP2x_WAIT_CERT_TIMEOUE);
		hdmi_hdcp22_monitor_start();
		break;

	case HDCP2x_CHECK_AKE_OK:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_CHECK_AKE_OK, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		if (readvalue & HDCP2X_AKE_SENT_RCVD_INT_STA) {
			vSetHDCPState(HDCP2x_CHECK_CERT_OK);
			vSetHDCPTimeOut(HDCP2x_WAIT_CERT_TIMEOUE);
			HDMI_HDCP_LOG
			    ("[HDMI][HDCP2.x]HDCP2x_CHECK_AKE_OK, _bReAKEtPollCnt = %d, %lums\n",
			     _bReAKEtPollCnt, jiffies);
			_bReAKEtPollCnt = 0;
		} else {
			vSetHDCPState(HDCP2x_CHECK_AKE_OK);
			vSetHDCPTimeOut(HDCP2x_WAIT_AKE_TIMEOUE);
			_bReAKEtPollCnt++;
		}
		break;

	case HDCP2x_CHECK_CERT_OK:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_CHECK_CERT_OK, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		if (readvalue & HDCP2X_CERT_SEND_RCVD_INT_STA) {
			vSetHDCPState(HDCP2x_REPEATER_CHECK);
			vSetHDCPTimeOut(HDCP2x_WAIT_REPEATER_CHECK_TIMEOUE);
			_bReCertPollCnt = 0;
			hdcp_err_0x30_count = 0;
		} else if (_bReCertPollCnt < 20) {
			_bReCertPollCnt++;
			if (hdcp_check_err_0x30() == TRUE) {
				hdmi_ddc_request(3);
				hdmi_ddc_free(3);
				vSetHDCPState(HDCP2x_WAIT_RES_CHG_OK);
				vSetHDCPTimeOut(100);
				break;
			}
			if (fgHDMIHdcp2Err()) {
				_bReCertPollCnt = 0;
				_bHdcpStatus = SV_FAIL;
				vSetHDCPState(HDCP2x_WAIT_RES_CHG_OK);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				vHdcpDdcHwPoll(FALSE);
				TX_DEF_LOG
				    ("[HDMI][HDCP2.x]HDCP2x_CHECK_CERT_OK, hdcp2_err=%x, HDCP2X_STATE = %d\n",
				     bHDMIHDCP2Err(), _bReCertPollCnt);
				break;
			}

			HDMI_HDCP_LOG("_bReCertPollCnt=%d\n", _bReCertPollCnt);

			vSetHDCPState(HDCP2x_CHECK_CERT_OK);
			vSetHDCPTimeOut(10);
		} else {
			TX_DEF_LOG
		    ("[HDMI][HDCP2.x]hdcp2.2 receive cert failure, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %d\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), _bReCertPollCnt);
			_bReCertPollCnt = 0;
			_bHdcpStatus = SV_FAIL;
			hdcp_err_0x30_count = 0;
			vHDCPInitAuth();
		}
		break;

	case HDCP2x_REPEATER_CHECK:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_REPEATER_CHECK, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		readvalue = bReadByteHdmiGRL(HDCP2X_STATUS_0);
		if (readvalue & HDCP2X_RPT_REPEATER) {
			HDMI_HDCP_LOG("downstream device is repeater\n");
			_fgRepeater = TRUE;
			vSetHDCPState(HDCP2x_REPEATER_CHECK_OK);
			vSetHDCPTimeOut(HDCP2x_WAIT_REPEATER_POLL_TIMEOUE);
		} else {
			HDMI_HDCP_LOG("downstream device is receiver\n");
			_fgRepeater = FALSE;
			vSetHDCPState(HDCP2x_AUTHEN_CHECK);
			vSetHDCPTimeOut(HDCP2x_WAIT_AUTHEN_TIMEOUE);

			_bDevice_Count = 0;
			_u2TxBStatus = 0;
			_bTxBKAV[4] = bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff;
			_bTxBKAV[3] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff00) >> 8;
			_bTxBKAV[2] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff0000) >> 16;
			_bTxBKAV[1] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff000000) >> 24;
			_bTxBKAV[0] = (bReadByteHdmiGRL(HDCP2X_RPT_SEQ) & 0xff000000) >> 24;

			_u2TxBStatus = ((bReadByteHdmiGRL(HDCP2X_STATUS_0) & 0x3c) >> 2)
			    + (((bReadByteHdmiGRL(HDCP2X_STATUS_1) & 0xff00) >> 8) << 4)
			    + ((bReadByteHdmiGRL(HDCP2X_STATUS_1) & 0xff) << 9);
			TX_DEF_LOG("[2.x]_bTxBKAV=0x%x;0x%x;0x%x;0x%x;0x%x\n",
			_bTxBKAV[0], _bTxBKAV[1], _bTxBKAV[2], _bTxBKAV[3], _bTxBKAV[4]);
			TX_DEF_LOG("[2.x]_u2TxBStatus=0x%x\n", _u2TxBStatus);
		}
		break;

	case HDCP2x_REPEATER_CHECK_OK:
		HDMI_HDCP_LOG
		("[HDMI][HDCP2.x]HDCP2x_REPEATER_CHECK_OK,0x1a8=0x%08x,0xc60=0x%08x,0xc8c=0x%08x,0xc90=0x%08x,%lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), bReadByteHdmiGRL(HDCP2X_STATUS_1),
		     jiffies);
		/*  0x1a0[23] can not work sometime, so add 0x1a0[24][16]  */
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		if ((readvalue & HDCP2X_RPT_RCVID_CHANGED_INT_STA)
		    || (readvalue & HDCP2X_RPT_SMNG_XFER_DONE_INT_STA)
		    || (readvalue & HDCP2X_AUTH_DONE_INT_STA)) {
			_bReRepeaterPollCnt = 0;
			vSetHDCPState(HDCP2x_RESET_RECEIVER);
			vSetHDCPTimeOut(HDCP2x_WAIT_RESET_RECEIVER_TIMEOUE);

		} else if ((_bReRepeaterPollCnt <= 30) && (fgHDMIHdcp2Err() == FALSE)) {
			_bReRepeaterPollCnt++;
			HDMI_HDCP_LOG("_bReRepeaterPollCnt=%d\n",
				      _bReRepeaterPollCnt);
			vSetHDCPState(HDCP2x_REPEATER_CHECK_OK);
			vSetHDCPTimeOut(HDCP2x_WAIT_REPEATER_POLL_TIMEOUE);
		} else {
			TX_DEF_LOG
			    ("[HDMI][HDCP2.x]hdcp2.2 assume repeater failure, hdcp2_err=%x\n",
			     bHDMIHDCP2Err());
			vHDCPInitAuth();
			_bHdcpStatus = SV_FAIL;
			_bReRepeaterPollCnt = 0;
			vSetHDCPTimeOut(HDCP2x_WAIT_LOADBIN_TIMEOUE);
			vSetHDCPState(HDCP2x_WAIT_RES_CHG_OK);
		}
		break;

	case HDCP2x_RESET_RECEIVER:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_RESET_RECEIVER, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, HDCP2X_RPT_RCVID_RD_START,
				 HDCP2X_RPT_RCVID_RD_START);
		udelay(1);
		vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0, HDCP2X_RPT_RCVID_RD_START);
		devicecnt =
		    (bReadByteHdmiGRL(HDCP2X_STATUS_1) & HDCP2X_RPT_DEVCNT) >>
		    HDCP2X_RPT_DEVCNT_SHIFT;

		depth = bReadByteHdmiGRL(HDCP2X_STATUS_1) & HDCP2X_RPT_DEPTH;
		if ((depth == 0) && (devicecnt != 0))
			fgRepeaterError = TRUE;
		count1 = 0;

		bRptID[0] =
		    (bReadByteHdmiGRL(HDCP2X_STATUS_1) & HDCP2X_RPT_RCVID_OUT) >>
		    HDCP2X_RPT_RCVID_OUT_SHIFT;
		count1 = count1 + u1CountNum1(bRptID[0]);
		for (i = 1; i < 5 * devicecnt; i++) {
			vWriteHdmiGRLMsk(HDCP2X_CTRL_2, HDCP2X_RPT_RCVID_RD, HDCP2X_RPT_RCVID_RD);
			udelay(1);
			vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0, HDCP2X_RPT_RCVID_RD);
			if (i < 155) {
				bRptID[i] =
				    (bReadByteHdmiGRL(HDCP2X_STATUS_1) & HDCP2X_RPT_RCVID_OUT) >>
				    HDCP2X_RPT_RCVID_OUT_SHIFT;
				count1 = count1+u1CountNum1(bRptID[i]);
				if ((i % 5) == 4) {
					if (count1 != 20)
						fgRepeaterError = TRUE;
					count1 = 0;
				}
			} else
				HDMI_HDCP_LOG("device count exceed\n");
		}

		for (i = 0; i < 5 * devicecnt; i++) {
			if ((i % 5) == 0)
				HDMI_HDCP_LOG("ID[%d]:", i / 5);

			HDMI_HDCP_LOG("0x%x,", bRptID[i]);

			if ((i % 5) == 4)
				HDMI_HDCP_LOG("\n");
		}
		if (fgRepeaterError) {
			HDMI_HDCP_LOG("repeater parameter invaild\n");
			vHDMIAVMute();
			vHDCPReset();
			vHDCPInitAuth();
			break;
		}

		_bDevice_Count = devicecnt;
		vSetHDCPState(HDCP2x_REPEAT_MSG_DONE);
		vSetHDCPTimeOut(HDCP2x_WAIT_REPEATER_DONE_TIMEOUE);

		_bTxBKAV[4] = bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff;
		_bTxBKAV[3] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff00) >> 8;
		_bTxBKAV[2] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff0000) >> 16;
		_bTxBKAV[1] = (bReadByteHdmiGRL(HDCP2X_RCVR_ID) & 0xff000000) >> 24;
		_bTxBKAV[0] = (bReadByteHdmiGRL(HDCP2X_RPT_SEQ) & 0xff000000) >> 24;
		_u2TxBStatus = ((bReadByteHdmiGRL(HDCP2X_STATUS_0) & 0x3c) >> 2)
		    + (((bReadByteHdmiGRL(HDCP2X_STATUS_1) & 0xff00) >> 8) << 4)
		    + ((bReadByteHdmiGRL(HDCP2X_STATUS_1) & 0xff) << 9);
		break;

	case HDCP2x_REPEAT_MSG_DONE:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_REPEAT_MSG_DONE, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		if ((readvalue & HDCP2X_RPT_SMNG_XFER_DONE_INT_STA)
		    || (readvalue & HDCP2X_AUTH_DONE_INT_STA)) {
			_bReRepeaterDoneCnt = 0;
			vWriteHdmiGRLMsk(HDCP2X_CTRL_2, 0, HDCP2X_RPT_SMNG_WR_START);
			vSetHDCPState(HDCP2x_AUTHEN_CHECK);
			vSetHDCPTimeOut(HDCP2x_WAIT_AUTHEN_TIMEOUE);
		} else if ((_bReRepeaterDoneCnt < 10) && (fgHDMIHdcp2Err() == FALSE)) {
			_bReRepeaterDoneCnt++;
			HDMI_HDCP_LOG("_bReRepeaterDoneCnt=%d\n",
				      _bReRepeaterDoneCnt);
			vSetHDCPState(HDCP2x_REPEAT_MSG_DONE);
			vSetHDCPTimeOut(HDCP2x_WAIT_REPEATER_DONE_TIMEOUE);
		} else {
			TX_DEF_LOG("repeater smsg done failure, hdcp2_err=%x\n",
				      bHDMIHDCP2Err());
			vHDCPInitAuth();

			_bReRepeaterDoneCnt = 0;
			_bHdcpStatus = SV_FAIL;
		}
		break;

	case HDCP2x_AUTHEN_CHECK:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_AUTHEN_CHECK, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		readvalue = bReadByteHdmiGRL(TOP_INT_STA00);
		if ((readvalue & HDCP2X_AUTH_DONE_INT_STA) && (fgHDMIHdcp2Err() == FALSE)) {
			vSetHDCPState(HDCP2x_ENCRYPTION);
			vSetHDCPTimeOut(HDCP2x_WAIT_AITHEN_DEALY_TIMEOUE);
			_bReAuthCnt = 0;
		} else if (((readvalue & HDCP2X_AUTH_FAIL_INT_STA) && (_bReAuthCnt != 0))
			   || (_bReAuthCnt > REPEAT_CHECK_AUTHHDCP_VALUE) || fgHDMIHdcp2Err()) {
			TX_DEF_LOG
			    ("[HDMI][HDCP2.x]hdcp2.2 authentication fail-->1, hdcp2_err=%x\n",
			     bHDMIHDCP2Err());
			vHDCPInitAuth();
			_bReAuthCnt = 0;
			vHDCPEncryptState(0); /* for audio notify */
			if (readvalue & HDCP2X_AUTH_FAIL_INT_STA) {
				vCleanAuthFailInt();
				vHDCPEncryptState(0); /* for audio notify */
				HDMI_HDCP_LOG("hdcp2.2 authentication fail-->2\n");
			}
		} else {
			if ((readvalue & HDCP2X_AUTH_FAIL_INT_STA) && (_bReAuthCnt == 0)) {
				vCleanAuthFailInt();
				vHDCPEncryptState(0); /* for audio notify */
				TX_DEF_LOG("hdcp2.2 authentication fail-->3\n");
			}
			_bReAuthCnt++;
			HDMI_HDCP_LOG("hdcp2.2 authentication wait=%d\n",
				      _bReAuthCnt);
			vSetHDCPState(HDCP2x_AUTHEN_CHECK);
			vSetHDCPTimeOut(HDCP2x_WAIT_AUTHEN_TIMEOUE);
		}
		break;

	case HDCP2x_ENCRYPTION:
		HDMI_HDCP_LOG
		    ("[HDMI][HDCP2.x]HDCP2x_ENCRYPTION, 0x1a8 = 0x%08x, 0xc60 = 0x%08x, 0xc8c = 0x%08x, %lums\n",
		     bReadByteHdmiGRL(TOP_INT_STA00), bReadByteHdmiGRL(HPD_DDC_STATUS),
		     bReadByteHdmiGRL(HDCP2X_STATUS_0), jiffies);
		if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) != HDMI_PLUG_IN_AND_SINK_POWER_ON) {
			vSetHDCPState(HDCP_RECEIVER_NOT_READY);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		} else {
			vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_ENCRYPT_EN, HDCP2X_ENCRYPT_EN);
			_bReAuthCnt = 0;
			_bHdcpStatus = SV_OK;
			vHDMIAVUnMute();
			hdmi_audio_signal_state(1);
			hdcp_unmute_start_flag = TRUE;
			hdcp_set_unmute_start_time();
			TX_DEF_LOG("[HDCP2.x]pass, %lums\n", jiffies);
			vHDCPEncryptState(1); /* for audio notify */
		}
		break;

	default:
		break;
	}
}

void vHdcpDdcHwPoll(unsigned char _bhw)
{
	if (_bhw == TRUE)
		vWriteHdmiGRLMsk(HDCP2X_POL_CTRL, 0, HDCP2X_DIS_POLL_EN);
	else
		vWriteHdmiGRLMsk(HDCP2X_POL_CTRL, HDCP2X_DIS_POLL_EN, HDCP2X_DIS_POLL_EN);
}

bool fgIsRepeater(void)
{
	return (_fgRepeater == TRUE);

}

void vCliSetSRMSignatureChkFlag(unsigned char u1Flag)
{
	_u1SRMSignatureChkFlag = u1Flag;
}

void vHdcpSrmGetSignatureChkFunc(unsigned char *prData)
{
	TX_DEF_LOG("Middleware get SRM signature CHeck Flag=%d\n", _u1SRMSignatureChkFlag);
	*prData = _u1SRMSignatureChkFlag;
}
EXPORT_SYMBOL(vHdcpSrmGetSignatureChkFunc);

void vHdcpSrmSetFunc(unsigned int u4ByteCount, unsigned char *prData)
{
	unsigned int u4Inx;
	/* unsigned char u1Data; */


#ifdef SRM_DBG
	HDMI_HDCP_LOG("Update SRM\n");
#endif

	if (u4ByteCount > SRM_SIZE)
		u4ByteCount = SRM_SIZE;

	if (copy_from_user(&_bSRMBuff[0], prData, u4ByteCount))
		HDMI_HDCP_LOG("Update SRM Fail\n");

#ifdef SRM_DBG
	HDMI_HDCP_LOG("[HDCP]Update SRM\n");
#endif
#if 0
	for (u4Inx = 0; u4Inx < u4ByteCount; u4Inx++) {
		if (u4Inx >= SRM_SIZE)
			break;
		u1Data = *(prData + u4Inx);
		vSetHdcpSrmBuff(u4Inx, u1Data);

	}
#endif

	u4Inx = ((_bSRMBuff[5] << 16) | (_bSRMBuff[6] << 8) | (_bSRMBuff[7]));

	_rSRMInfo.dwVRLLenInDram = (u4Inx - 3 - 40);

	_rSRMInfo.bID = (_bSRMBuff[0] & 0xf0);
	_rSRMInfo.dwVer = ((_bSRMBuff[2] << 8) | (_bSRMBuff[3]));

#ifdef SRM_SUPPORT
	vCompareSRM();
#endif

}
EXPORT_SYMBOL(vHdcpSrmSetFunc);

void vHDCPBStatus(void)
{
/* _u2TxBStatus */
	unsigned int u2Temp = 0;

	if (hdcp2_version_flag == TRUE) {
		TX_DEF_LOG("hdcp2.2\n");
		if (fgIsRepeater()) {
			TX_DEF_LOG("Bstatus = 0x%x\n", _u2TxBStatus);

			if (_u2TxBStatus & (0x1 << 2))
				TX_DEF_LOG("MAX_CASCADE_EXCEEDED = 1\n");
			else
				TX_DEF_LOG("MAX_CASCADE_EXCEEDED = 0\n");

			u2Temp = (_u2TxBStatus >> 9) & (0x7);
			TX_DEF_LOG("DEPTH = %d\n", u2Temp);

			if (_u2TxBStatus & (0x1 << 3))
				TX_DEF_LOG("MAX_DEVS_EXCEEDED = 1\n");
			else
				TX_DEF_LOG("MAX_DEVS_EXCEEDED = 0\n");

			u2Temp = (_u2TxBStatus >> 4) & (0x1f);
			TX_DEF_LOG("DEVICE_COUNT = %d\n", u2Temp);

			u2Temp = (_u2TxBStatus >> 1) & (0x1);
			if (u2Temp)
				TX_DEF_LOG
				    ("presence of an hdcp20 compliant repeater in the topology\n");

			u2Temp = (_u2TxBStatus >> 0) & (0x1);
			if (u2Temp)
				TX_DEF_LOG
				    ("presence of an hdcp1x compliant repeater in the topology\n");
		} else {
			TX_DEF_LOG("A Connected device is only Sink!!!\n");
		}

	} else {
		TX_DEF_LOG("hdcp1.4\n");
		if (fgIsRepeater()) {
			TX_DEF_LOG("Bstatus = 0x%x\n", _u2TxBStatus);
			if (_u2TxBStatus & (0x1 << 12))
				TX_DEF_LOG("HDMI_MODE = 1\n");
			else
				TX_DEF_LOG("HDMI_MODE = 0\n");

			if (_u2TxBStatus & (0x1 << 11))
				TX_DEF_LOG("MAX_CASCADE_EXCEEDED = 1\n");
			else
				TX_DEF_LOG("MAX_CASCADE_EXCEEDED = 0\n");

			u2Temp = (_u2TxBStatus >> 8) & (0x7);
			TX_DEF_LOG("DEPTH = %d\n", u2Temp);

			if (_u2TxBStatus & (0x1 << 7))
				TX_DEF_LOG("MAX_DEVS_EXCEEDED = 1\n");
			else
				TX_DEF_LOG("MAX_DEVS_EXCEEDED = 0\n");

			u2Temp = _u2TxBStatus & 0x7F;
			TX_DEF_LOG("DEVICE_COUNT = %d\n", u2Temp);
		} else {
			TX_DEF_LOG("A Connected device is only Sink!!!\n");
		}
	}
}


int hdmi_tx_get_Bksv(char *Bksv, int *count, int *depth)
{
	unsigned char i, j;
	unsigned char _u1Depth = 0;
	unsigned char bKsvlist_temp[KSV_LIST_SIZE];

	HDMI_HDCP_LOG("[HDCP]  enter %s,_fgWifiHdcpErr = %d\n", __func__, _fgWifiHdcpErr);
	if (_fgWifiHdcpErr) {
		TX_DEF_LOG("[HDCP]HDCP FAIL....\n");
		_fgWifiHdcpErr = FALSE;
		return -1;
	}

	memset(bKsvlist_buff, 0, KSV_LIST_SIZE);
	for (i = 0; i < 5; i++)
		bKsvlist_buff[i] = _bTxBKAV[i];

	if (_bDevice_Count <= 9) {
		for (i = 0; i < 5 * _bDevice_Count; i++)
			bKsvlist_buff[i + 5] = bKsv_buff[i];
	} else {
		{
			TX_DEF_LOG("[HDCP] !!!Error!! TX dowstream over 9 = %d\n",
				   _bDevice_Count);
		}

	}

	if ((hdcp2_version_flag == FALSE) && (_bDevice_Count <= 9)) {
		for (j = 0; j < _bDevice_Count + 1; j++) {
			for (i = 0; i < 5; i++)
				bKsvlist_temp[4 - i + j * 5] = bKsvlist_buff[i + j * 5];
		}

		for (j = 0; j < (_bDevice_Count + 1) * 5; j++)
			bKsvlist_buff[j] = bKsvlist_temp[j];
	}

	memcpy(Bksv, bKsvlist_buff, KSV_LIST_SIZE);

	if (fgIsRepeater()) {
		TX_DEF_LOG("[HDCP]HDMI Sink is repeater\n");
		*count = _bDevice_Count + 1;
	} else {
		if (i4GetHotPlugStatus() == HDMI_PLUG_OUT)
			*count = 0;
		else
			*count = 1;
	}
	if (_bDevice_Count <= 9) {
		TX_DEF_LOG("[HDCP] vGetTxBKsv\n");
		TX_DEF_LOG("[HDCP] _bDevice_Down_Count = %d\n", _bDevice_Count);
		TX_DEF_LOG("[HDCP] count = %d\n", *count);

		for (i = 0; i < 5 * (_bDevice_Count + 1); i++) {
			if ((i % 5) == 0)
				HDMI_HDCP_LOG("ID[%d]:", i / 5);

			HDMI_HDCP_LOG("0x%x,", bKsvlist_buff[i]);

			if ((i % 5) == 4)
				HDMI_HDCP_LOG("\n");
		}
	}

	if (hdcp2_version_flag == TRUE)
		_u1Depth = (bReadByteHdmiGRL(HDCP2X_STATUS_1) & 0xff);
	else
		_u1Depth = (_u2TxBStatus >> 8) & (0x7);

	if (fgIsRepeater()) {
		TX_DEF_LOG("[HDCP]HDMI Sink is repeater\n");
		*depth = _u1Depth + 1;
	} else {
		if (i4GetHotPlugStatus() == HDMI_PLUG_OUT)
			*depth = 0;
		else
			*depth = 1;
	}
	TX_DEF_LOG("[HDCP]*depth = 0x%x\n", *depth);

	return 0;
}
EXPORT_SYMBOL(hdmi_tx_get_Bksv);

void vShowHdcpStatus(void)
{
	TX_DEF_LOG("[hdcp]state=%d\n", (unsigned int)e_hdcp_ctrl_state);
	TX_DEF_LOG("[hdcp]hdcp2_version_flag=%d\n", hdcp2_version_flag);
	TX_DEF_LOG("[hdcp]hdcp_key_load_status=%d\n", hdcp_key_load_status);
	TX_DEF_LOG("[hdcp]hdcp_unmute_logo_flag=%d\n", hdcp_unmute_logo_flag);
	TX_DEF_LOG("[hdcp]_bHdcpOff=%d\n", _bHdcpOff);
	TX_DEF_LOG("[hdcp]hdcp_unmute_start_flag=%d\n", hdcp_unmute_start_flag);

	if ((bReadByteHdmiGRL(HDCP1x_STATUS) & HDCP_ENCRYPTING_ON) == HDCP_ENCRYPTING_ON)
		TX_DEF_LOG("[hdcp1]HDCP_ENCRYPTING_ON\n");
	else
		TX_DEF_LOG("[hdcp1]HDCP_ENCRYPTING_OFF\n");
	if ((bReadByteHdmiGRL(HDCP2X_STATUS_0) & HDCP2X_ENCRYPTING_ON) == HDCP2X_ENCRYPTING_ON)
		TX_DEF_LOG("[hdcp2]HDCP2X_ENCRYPTING_ON\n");
	else
		TX_DEF_LOG("[hdcp2]HDCP2X_ENCRYPTING_OFF\n");
}
