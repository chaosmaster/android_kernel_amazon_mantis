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

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT
#include "hdmiddc.h"
#include "hdmi_ctrl.h"
#include "hdmictrl.h"
#include "hdmihdcp.h"

typedef enum {
	SIF_8_BIT_HDMI,		/* /< [8 bits data address.] */
	SIF_16_BIT_HDMI,	/* /< [16 bits data address.] */
} SIF_BIT_T_HDMI;

static unsigned char _DDCMRead(unsigned char ucCurAddrMode, unsigned int u4ClkDiv,
			       unsigned char ucDev, unsigned int u4Addr, SIF_BIT_T ucAddrType,
			       unsigned char *pucValue, unsigned int u4Count)
{
	unsigned int i, temp_length, loop_counter;
	unsigned int ucReadCount, ucIdx;

	if ((pucValue == NULL) || (u4Count == 0) || (u4ClkDiv == 0))
		return 0;

	ucIdx = 0;

	if (u4Count >= 16) {
		temp_length = 16;
		loop_counter = u4Count / 16;
	} else {
		temp_length = u4Count;
		loop_counter = 1;
	}

	vWriteHdmiGRLMsk(HPD_DDC_CTRL, u4ClkDiv << DDC_DELAY_CNT_SHIFT, DDC_DELAY_CNT);
	for (i = 0; i < loop_counter; i++) {
		if (ucDev > EDID_ID) {
			vWriteHdmiGRLMsk(SCDC_CTRL, (ucDev - EDID_ID) << DDC_SEGMENT_SHIFT,
					 DDC_SEGMENT);
			vWriteByteHdmiGRL(DDC_CTRL,
					  (ENH_READ_NO_ACK << DDC_CMD_SHIFT) +
					  (temp_length << DDC_DIN_CNT_SHIFT)
					  + ((u4Addr + i * temp_length) << DDC_OFFSET_SHIFT) +
					  (EDID_ID << 1));
		} else {
			vWriteByteHdmiGRL(DDC_CTRL,
					  (SEQ_READ_NO_ACK << DDC_CMD_SHIFT) +
					  (temp_length << DDC_DIN_CNT_SHIFT)
					  + ((u4Addr + i * temp_length) << DDC_OFFSET_SHIFT) +
					  (ucDev << 1));
		}

		msleep(20);

		for (ucIdx = 0; ucIdx < temp_length; ucIdx++) {
			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[i * 16 + ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;

			ucReadCount = i * 16 + ucIdx + 1;
		}
	}
	return ucReadCount;
}


void DDC_WR_ONE(unsigned int addr_id, unsigned int offset_id, unsigned char wr_data)
{
	unsigned int i;

	vWriteHdmiGRLMsk(HPD_DDC_CTRL, DDC2_CLOK << DDC_DELAY_CNT_SHIFT, DDC_DELAY_CNT);

	if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
		vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
		udelay(250);
	}
	vWriteByteHdmiGRL(SI2C_CTRL, SI2C_ADDR_READ << SI2C_ADDR_SHIFT);
	vWriteHdmiGRLMsk(SI2C_CTRL, wr_data << SI2C_WDATA_SHIFT, SI2C_WDATA);
	vWriteHdmiGRLMsk(SI2C_CTRL, SI2C_WR, SI2C_WR);

	vWriteByteHdmiGRL(DDC_CTRL, (SEQ_WRITE_REQ_ACK << DDC_CMD_SHIFT) + (1 << DDC_DIN_CNT_SHIFT)
			  + (offset_id << DDC_OFFSET_SHIFT) + (addr_id << 1));

	for (i = 0; i < 5; i++)
		udelay(200);

	if ((bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_I2C_IN_PROG) == 0)
		HDMI_HDCP_LOG("[HDMI][DDC] error: time out\n");

	if ((bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & (DDC_I2C_NO_ACK | DDC_I2C_BUS_LOW))) {
		if ((bReadByteHdmiGRL(DDC_CTRL) & 0xFF) == (RX_ID << 1))
		TX_DEF_LOG("[HDMI][DDC][1x]err_w:0xc10=0x%08x,0xc60=0x%08x,0xc68=0x%08x\n",
			bReadByteHdmiGRL(DDC_CTRL),
			bReadByteHdmiGRL(HPD_DDC_STATUS),
			bReadByteHdmiGRL(HDCP2X_DDCM_STATUS));
		else
		HDMI_HDCP_LOG("[HDMI][DDC]err_w:0xc10=0x%08x,0xc60=0x%08x,0xc68=0x%08x\n",
			bReadByteHdmiGRL(DDC_CTRL),
			bReadByteHdmiGRL(HPD_DDC_STATUS),
			bReadByteHdmiGRL(HDCP2X_DDCM_STATUS));
		if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
			vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
			udelay(250);
		}
	}
}

static unsigned char _DDCMWrite(unsigned char ucCurAddrMode, unsigned int u4ClkDiv,
				unsigned char ucDev, unsigned int u4Addr, SIF_BIT_T ucAddrType,
				const unsigned char *pucValue, unsigned int u4Count)
{
	unsigned int i;

	for (i = 0; i < u4Count; i++) {
		DDC_WR_ONE(ucDev, u4Addr + i, *(pucValue + i));
		mdelay(2);
	}

	return u4Count;
}

unsigned int DDCM_RanAddr_Write(unsigned int u4ClkDiv, unsigned char ucDev, unsigned int u4Addr,
				SIF_BIT_T ucAddrType, const unsigned char *pucValue,
				unsigned int u4Count)
{
	unsigned int u4WriteCount1;
	unsigned char ucReturnVaule;

	HDMI_DDC_FUNC();

	if ((pucValue == NULL) ||
	    (u4Count == 0) ||
	    (u4ClkDiv == 0) ||
	    (ucAddrType > SIF_16_BIT) ||
	    ((ucAddrType == SIF_8_BIT) && (u4Addr > 255)) ||
	    ((ucAddrType == SIF_16_BIT) && (u4Addr > 65535))) {
		return 0;
	}


	if (ucAddrType == SIF_8_BIT)
		u4WriteCount1 = ((255 - u4Addr) + 1);
	else if (ucAddrType == SIF_16_BIT)
		u4WriteCount1 = ((65535 - u4Addr) + 1);

	u4WriteCount1 = (u4WriteCount1 > u4Count) ? u4Count : u4WriteCount1;
	ucReturnVaule = _DDCMWrite(0, u4ClkDiv, ucDev, u4Addr, ucAddrType, pucValue, u4WriteCount1);

	return (unsigned int)ucReturnVaule;
}

unsigned int DDCM_CurAddr_Read(unsigned int u4ClkDiv, unsigned char ucDev, unsigned char *pucValue,
			       unsigned int u4Count)
{
	unsigned char ucReturnVaule;

	HDMI_DDC_FUNC();

	if ((pucValue == NULL) || (u4Count == 0) || (u4ClkDiv == 0))
		return 0;

	ucReturnVaule = _DDCMRead(1, u4ClkDiv, ucDev, 0, SIF_8_BIT, pucValue, u4Count);

	return (unsigned int)ucReturnVaule;
}

unsigned char DDCM_RanAddr_Read(unsigned int u4ClkDiv, unsigned char ucDev, unsigned int u4Addr,
				SIF_BIT_T ucAddrType, unsigned char *pucValue, unsigned int u4Count)
{
	unsigned int u4ReadCount;
	unsigned char ucReturnVaule;

	HDMI_DDC_FUNC();
	if ((pucValue == NULL) ||
	    (u4Count == 0) ||
	    (u4ClkDiv == 0) ||
	    (ucAddrType > SIF_16_BIT) ||
	    ((ucAddrType == SIF_8_BIT) && (u4Addr > 255)) ||
	    ((ucAddrType == SIF_16_BIT) && (u4Addr > 65535))) {
		return 0;
	}

	if (ucAddrType == SIF_8_BIT)
		u4ReadCount = ((255 - u4Addr) + 1);
	else if (ucAddrType == SIF_16_BIT)
		u4ReadCount = ((65535 - u4Addr) + 1);

	u4ReadCount = (u4ReadCount > u4Count) ? u4Count : u4ReadCount;
	ucReturnVaule = _DDCMRead(0, u4ClkDiv, ucDev, u4Addr, ucAddrType, pucValue, u4ReadCount);


	return ucReturnVaule;
}

unsigned char _DDCMRead_hdmi(unsigned char ucCurAddrMode, unsigned int u4ClkDiv,
			     unsigned char ucDev, unsigned int u4Addr, SIF_BIT_T_HDMI ucAddrType,
			     unsigned char *pucValue, unsigned int u4Count)
{
	unsigned int i, temp_length, loop_counter, temp_ksvlist, device_n;
	unsigned int ucReadCount, ucIdx;
	unsigned long DdcStartTime, DdcEndTime, DdcTimeOut;

	if ((pucValue == NULL) || (u4Count == 0) || (u4ClkDiv == 0))
		return 0;

	ucIdx = 0;
	if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
		vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
		udelay(250);
	}

	vWriteHdmiGRLMsk(DDC_CTRL, (CLEAR_FIFO << DDC_CMD_SHIFT), DDC_CMD);

	if (u4Addr == 0x43) {
		vWriteByteHdmiGRL(DDC_CTRL,
				  (SEQ_READ_NO_ACK << DDC_CMD_SHIFT) +
				  (u4Count << DDC_DIN_CNT_SHIFT)
				  + (u4Addr << DDC_OFFSET_SHIFT) + (ucDev << 1));
		udelay(250);
		udelay(250);
		udelay(200);

		if (u4Count > 10)
			temp_ksvlist = 10;
		else
			temp_ksvlist = u4Count;

		for (ucIdx = 0; ucIdx < temp_ksvlist; ucIdx++) {
			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;
			udelay(100);

		}

		if (u4Count == temp_ksvlist)
			return (ucIdx + 1);

		udelay(250);
		udelay(250);

		if (u4Count / 5 == 3)
			device_n = 5;
		else
			device_n = 10;

		for (ucIdx = 10; ucIdx < (10 + device_n); ucIdx++) {

			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;
			udelay(100);
		}

		if (u4Count == (10 + device_n))
			return (ucIdx + 1);

		udelay(250);
		udelay(250);

		if (u4Count / 5 == 5)
			device_n = 5;
		else
			device_n = 10;

		for (ucIdx = 20; ucIdx < (20 + device_n); ucIdx++) {

			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;
			udelay(100);
		}

		if (u4Count == (20 + device_n))
			return (ucIdx + 1);

		udelay(250);
		udelay(250);

		if (u4Count / 5 == 7)
			device_n = 5;
		else
			device_n = 10;

		for (ucIdx = 30; ucIdx < (30 + device_n); ucIdx++) {

			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;
			udelay(100);
		}

		if (u4Count == (30 + device_n))
			return (ucIdx + 1);

		udelay(250);
		udelay(250);

		for (ucIdx = 40; ucIdx < (40 + 5); ucIdx++) {

			vWriteByteHdmiGRL(SI2C_CTRL, (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
			vWriteByteHdmiGRL(SI2C_CTRL,
					  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_CONFIRM_READ);

			pucValue[ucIdx] =
			    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >> DDC_DATA_OUT_SHIFT;
			udelay(100);
		}

		if (u4Count == 45)
			return (ucIdx + 1);
	} else {
		if (u4Count >= 16) {
			temp_length = 16;
			loop_counter = u4Count / 16 + ((u4Count % 16 == 0) ? 0 : 1);
		} else {
			temp_length = u4Count;
			loop_counter = 1;
		}
		if (ucDev >= EDID_ID) {
			if (u4ClkDiv < DDC2_CLOK_EDID)
				u4ClkDiv = DDC2_CLOK_EDID;
		}
		vWriteHdmiGRLMsk(HPD_DDC_CTRL, u4ClkDiv << DDC_DELAY_CNT_SHIFT, DDC_DELAY_CNT);
		for (i = 0; i < loop_counter; i++) {
			if ((i == (loop_counter - 1)) && (i != 0) && (u4Count % 16))
				temp_length = u4Count % 16;

			if (ucDev > EDID_ID) {
				vWriteHdmiGRLMsk(SCDC_CTRL, (ucDev - EDID_ID) << DDC_SEGMENT_SHIFT,
						 DDC_SEGMENT);
				vWriteByteHdmiGRL(DDC_CTRL,
						  (ENH_READ_NO_ACK << DDC_CMD_SHIFT) +
						  (temp_length << DDC_DIN_CNT_SHIFT)
						  +
						  ((u4Addr + i * temp_length) << DDC_OFFSET_SHIFT) +
						  (EDID_ID << 1));
			} else {
				vWriteByteHdmiGRL(DDC_CTRL,
						  (SEQ_READ_NO_ACK << DDC_CMD_SHIFT) +
						  (temp_length << DDC_DIN_CNT_SHIFT)
						  +
						  ((u4Addr +
						    ((u4Addr ==
						      0x43) ? 0 : (i * 16))) << DDC_OFFSET_SHIFT) +
						  (ucDev << 1));
			}
			mdelay(2);
			DdcStartTime = jiffies;
			DdcTimeOut = temp_length + 5;
			DdcEndTime = DdcStartTime + (DdcTimeOut) * HZ / 1000;
			while (1) {
				if ((bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_I2C_IN_PROG) == 0)
					break;

				if (time_after(jiffies, DdcEndTime)) {
					HDMI_HDCP_LOG("[HDMI][DDC] error: time out\n");
					return 0;
				}
				mdelay(1);
			}
			if ((bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) &
			     (DDC_I2C_NO_ACK | DDC_I2C_BUS_LOW))) {
				if ((bReadByteHdmiGRL(DDC_CTRL) & 0xFF) == (RX_ID << 1))
				TX_DEF_LOG("[HDMI][DDC][1x]err_r:0xc10=0x%08x,0xc60=0x%08x,0xc68=0x%08x\n",
					bReadByteHdmiGRL(DDC_CTRL),
					bReadByteHdmiGRL(HPD_DDC_STATUS),
					bReadByteHdmiGRL(HDCP2X_DDCM_STATUS));
				else
				HDMI_HDCP_LOG("[HDMI][DDC]err_r:0xc10=0x%08x,0xc60=0x%08x,0xc68=0x%08x\n",
					bReadByteHdmiGRL(DDC_CTRL),
					bReadByteHdmiGRL(HPD_DDC_STATUS),
					bReadByteHdmiGRL(HDCP2X_DDCM_STATUS));
				if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
					vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT),
							 DDC_CMD);
					udelay(250);
				}
				return 0;
			}
			for (ucIdx = 0; ucIdx < temp_length; ucIdx++) {
				vWriteByteHdmiGRL(SI2C_CTRL,
						  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) + SI2C_RD);
				vWriteByteHdmiGRL(SI2C_CTRL,
						  (SI2C_ADDR_READ << SI2C_ADDR_SHIFT) +
						  SI2C_CONFIRM_READ);

				pucValue[i * 16 + ucIdx] =
				    (bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_DATA_OUT) >>
				    DDC_DATA_OUT_SHIFT;
				/*
				* when reading edid, if hdmi module been reset,
				* ddc will fail and it's speed will be set to 400.
				*/
				if (((bReadByteHdmiGRL(HPD_DDC_CTRL) >> 16) & 0xFFFF) < DDC2_CLOK) {
					HDMI_DDC_LOG("error: speed dev=0x%x; addr=0x%x\n", ucDev, u4Addr);
					return 0;
				}

				ucReadCount = i * 16 + ucIdx + 1;
			}
		}
		return ucReadCount;
	}
	return 0;
}

unsigned char fgDDCBusy;
unsigned char vDDCRead(unsigned int u4ClkDiv, unsigned char ucDev, unsigned int u4Addr,
		       SIF_BIT_T_HDMI ucAddrType, unsigned char *pucValue, unsigned int u4Count)
{
	unsigned int u4ReadCount;
	unsigned char ucReturnVaule;


	if ((pucValue == NULL) ||
	    (u4Count == 0) ||
	    (u4ClkDiv == 0) ||
	    (ucAddrType > SIF_16_BIT_HDMI) ||
	    ((ucAddrType == SIF_8_BIT_HDMI) && (u4Addr > 255)) ||
	    ((ucAddrType == SIF_16_BIT_HDMI) && (u4Addr > 65535))) {
		return 0;
	}

	if (ucAddrType == SIF_8_BIT_HDMI)
		u4ReadCount = ((255 - u4Addr) + 1);
	else if (ucAddrType == SIF_16_BIT_HDMI)
		u4ReadCount = ((65535 - u4Addr) + 1);

	u4ReadCount = (u4ReadCount > u4Count) ? u4Count : u4ReadCount;
	ucReturnVaule =
	    _DDCMRead_hdmi(0, u4ClkDiv, ucDev, u4Addr, ucAddrType, pucValue, u4ReadCount);
	return ucReturnVaule;
}

unsigned char fgDDCDataRead(unsigned char bDevice, unsigned char bData_Addr,
			    unsigned char bDataCount, unsigned char *prData)
{
	bool flag;

	HDMI_DDC_FUNC();

	while (fgDDCBusy == 1) {
		HDMI_HDCP_LOG("[HDMI][DDC]DDC read busy\n");
		mdelay(2);
	}
	fgDDCBusy = 1;

	hdmi_ddc_request(2);
	if (vDDCRead(DDC2_CLOK, (unsigned char)bDevice, (unsigned int)bData_Addr, SIF_8_BIT_HDMI,
	     (unsigned char *)prData, (unsigned int)bDataCount) == bDataCount) {
		fgDDCBusy = 0;
		flag = TRUE;
	} else {
		fgDDCBusy = 0;
		flag = FALSE;
}
	hdmi_ddc_free(2);

	return flag;
}

unsigned char fgDDCDataWrite(unsigned char bDevice, unsigned char bData_Addr,
			     unsigned char bDataCount, unsigned char *prData)
{
	unsigned int i;

	HDMI_DDC_FUNC();
	while (fgDDCBusy == 1) {
		HDMI_HDCP_LOG("[HDMI][DDC]DDC write busy\n");
		mdelay(2);
	}
	fgDDCBusy = 1;

	hdmi_ddc_request(2);
	for (i = 0; i < bDataCount; i++)
		DDC_WR_ONE(bDevice, bData_Addr + i, *(prData + i));
	hdmi_ddc_free(2);

	fgDDCBusy = 0;
	return 1;
}


#endif
