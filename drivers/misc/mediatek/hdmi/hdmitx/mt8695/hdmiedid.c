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


#include "hdmi_ctrl.h"

#include "hdmiddc.h"
#include "hdmiedid.h"
#include "hdmihdcp.h"

bool hdmi_force_sdr = FALSE;

bool debug_hdr10p_force_enable_edid = FALSE;
unsigned char debug_hdr10p_force_set_appversion = 0xff;

HDMI_SINK_AV_CAP_T _HdmiSinkAvCap;
static unsigned char _fgHdmiNoEdidCheck = FALSE;
static unsigned int _u4i_3D_VIC;
static unsigned int _ui4First_16_NTSC_VIC;
static unsigned int _ui4First_16_PAL_VIC;
static unsigned int _ui4First_16_VIC[16];
static unsigned char _bEdidData[EDID_SIZE];	/* 4 block 512 Bytes */
static unsigned char aEDIDHeader[EDID_HEADER_LEN] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};
static unsigned char aEDIDVSDBHeader[EDID_VSDB_LEN] = { 0x03, 0x0c, 0x00 };
static unsigned char aEDIDHFVSDBHeader[EDID_VSDB_LEN] = { 0xd8, 0x5d, 0xc4 };

static unsigned int aEDIDDoblyVSVDBHeader = 0x00d046;
static unsigned int aEDIDPhilipsVSVDBHeader = 0xea9fb1;
static unsigned int aEDIDHdr10PlusVSVDBHeader = 0x90848b;

unsigned int _ui4svd_128_VIC[128];
unsigned int _u4i_svd_420_CMDB;

static unsigned char _bEdidData2[256] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x4d, 0xd9, 0x02, 0xd7, 0x01, 0x01, 0x01,
	0x01,
	0x20, 0x16, 0x01, 0x03, 0x80, 0xa0, 0x5a, 0x78, 0x0a, 0x83, 0xad, 0xa2, 0x56, 0x49, 0x9b,
	0x25,
	0x0f, 0x47, 0x4a, 0x20, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e,
	0x28,
	0x55, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x80, 0x18, 0x71, 0x1c, 0x16,
	0x20,
	0x58, 0x2c, 0x25, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x9e, 0x00, 0x00, 0x00, 0xfc, 0x00,
	0x53,
	0x4f, 0x4e, 0x59, 0x20, 0x54, 0x56, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00,
	0xfd,
	0x00, 0x3a, 0x3e, 0x0f, 0x44, 0x0f, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01,
	0x6b,
	0x02, 0x03, 0x30, 0xf5, 0x45, 0x10, 0x04, 0x05, 0x03, 0x20, 0x35, 0x0f, 0x7f, 0x07, 0x15,
	0x07,
	0x55, 0x3d, 0x1f, 0xc0, 0x57, 0x07, 0x00, 0x67, 0x54, 0x00, 0x5f, 0x7e, 0x01, 0x4d, 0x02,
	0x00,
	0x83, 0x5f, 0x00, 0x00, 0x68, 0x03, 0x0c, 0x00, 0x21, 0x00, 0x80, 0x1e, 0x0f, 0xe2, 0x00,
	0x7b,
	0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0x40, 0x84, 0x63,
	0x00,
	0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0x40,
	0x84,
	0x63, 0x00, 0x00, 0x18, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96,
	0x00,
	0xb0, 0x84, 0x43, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x4c
};

const unsigned char _cFsStr[][7] = { {"32khz  "},
{"44khz  "},
{"48khz  "},
{"88khz  "},
{"96khz  "},
{"176khz "},
{"192khz "}
};

const unsigned char _cBitdeepStr[][7] = { {"16bit  "},
{"20bit  "},
{"24bit  "}
};

unsigned char cDstStr[50];
unsigned char cDstBitStr[21];

bool fgIsHdmiNoEDIDCheck(void)
{
	HDMI_EDID_FUNC();

	return _fgHdmiNoEdidCheck;
}

void vSetNoEdidChkInfo(void)
{
	unsigned char bInx;

	HDMI_EDID_FUNC();

	vSetSharedInfo(SI_EDID_PARSING_RESULT, TRUE);
	vSetSharedInfo(SI_EDID_VSDB_EXIST, TRUE);
	_HdmiSinkAvCap.b_sink_support_hdmi_mode = TRUE;
	_HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui4_sink_dtd_pal_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_ntsc_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_pal_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui2_sink_colorimetry = 0xffff;
	_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui4_sink_cea_pal_resolution = 0xffffffff;
	_HdmiSinkAvCap.ui2_sink_aud_dec = 0xffff;
	_HdmiSinkAvCap.ui1_sink_dsd_ch_num = 5;
	for (bInx = 0; bInx < 7; bInx++) {
		_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bInx] = 0xff;
		_HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bInx] = 0xff;
		_HdmiSinkAvCap.ui1_sink_dst_ch_sampling[bInx] = 0xff;
		_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bInx] = 0xff;
		_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bInx] = 0xff;
		_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[bInx] = 0xff;
	        _HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[bInx] = 0xff;
	}

	for (bInx = 0; bInx < 7; bInx++)
		_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bInx] = 0xff;

	_HdmiSinkAvCap.ui1_sink_spk_allocation = 0xff;


	_HdmiSinkAvCap.e_sink_rgb_color_bit =
	    (HDMI_SINK_DEEP_COLOR_10_BIT | HDMI_SINK_DEEP_COLOR_12_BIT |
	     HDMI_SINK_DEEP_COLOR_16_BIT);
	_HdmiSinkAvCap.e_sink_ycbcr_color_bit =
	    (HDMI_SINK_DEEP_COLOR_10_BIT | HDMI_SINK_DEEP_COLOR_12_BIT |
	     HDMI_SINK_DEEP_COLOR_16_BIT);
	_HdmiSinkAvCap.ui1_sink_dc420_color_bit =
	    (HDMI_SINK_DEEP_COLOR_10_BIT | HDMI_SINK_DEEP_COLOR_12_BIT |
	     HDMI_SINK_DEEP_COLOR_16_BIT);
	_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup = 0;
	_HdmiSinkAvCap.ui1_sink_support_ai = 1;

	_HdmiSinkAvCap.b_sink_edid_ready = TRUE;

	_HdmiSinkAvCap.b_sink_3D_present = TRUE;
	_HdmiSinkAvCap.ui4_sink_cea_3D_resolution = 0xFFFFFFFF;
	_HdmiSinkAvCap.ui1_sink_max_tmds_clock = 0xFFFF;

	_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic = 0x3f;
	_HdmiSinkAvCap.b_sink_SCDC_present = 1;
	_HdmiSinkAvCap.b_sink_LTE_340M_sramble = 1;
	_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = FALSE;
}

void vClearEdidInfo(void)
{
	unsigned char bInx;

	HDMI_EDID_FUNC();
	vSetSharedInfo(SI_EDID_PARSING_RESULT, FALSE);
	vSetSharedInfo(SI_EDID_VSDB_EXIST, FALSE);
	_HdmiSinkAvCap.b_sink_support_hdmi_mode = FALSE;
	_HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_dtd_pal_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_pal_resolution = 0;
	_HdmiSinkAvCap.ui2_sink_colorimetry = 0;
	_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_cea_pal_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_native_pal_resolution = 0;
	_HdmiSinkAvCap.ui2_sink_vcdb_data = 0;
	_HdmiSinkAvCap.ui2_sink_aud_dec = 0;
	_HdmiSinkAvCap.ui1_sink_dsd_ch_num = 0;
	for (bInx = 0; bInx < 7; bInx++) {
		if (bInx == 0)
			_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bInx] = 0x07;
		else
			_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_dst_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[bInx] = 0;
	}

	for (bInx = 0; bInx < 7; bInx++) {
		if (bInx == 0)
			_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bInx] = 0x07;
		else
			_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bInx] = 0;
	}

	_HdmiSinkAvCap.ui1_sink_spk_allocation = 0;
	_HdmiSinkAvCap.ui1_sink_i_latency_present = 0;
	_HdmiSinkAvCap.ui1_sink_p_latency_present = 0;	/* kenny add 2010/4/25 */
	_HdmiSinkAvCap.ui1_sink_p_audio_latency = 0;
	_HdmiSinkAvCap.ui1_sink_p_video_latency = 0;
	_HdmiSinkAvCap.ui1_sink_i_audio_latency = 0;
	_HdmiSinkAvCap.ui1_sink_i_video_latency = 0;

	_HdmiSinkAvCap.e_sink_rgb_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.e_sink_ycbcr_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.ui1_sink_dc420_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup =
	    (SINK_BASIC_AUDIO_NO_SUP | SINK_SAD_NO_EXIST | SINK_BASE_BLK_CHKSUM_ERR |
	     SINK_EXT_BLK_CHKSUM_ERR);
	_HdmiSinkAvCap.ui2_sink_cec_address = 0xffff;

	_HdmiSinkAvCap.b_sink_edid_ready = FALSE;
	_HdmiSinkAvCap.ui1_sink_support_ai = 0;
	_HdmiSinkAvCap.ui1_Display_Horizontal_Size = 0;
	_HdmiSinkAvCap.ui1_Display_Vertical_Size = 0;
	_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic = 0;
	_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb = 0;
	_HdmiSinkAvCap.b_sink_SCDC_present = 0;
	_HdmiSinkAvCap.b_sink_LTE_340M_sramble = 0;
	_HdmiSinkAvCap.ui1_sink_max_tmds_clock = 0;
	_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate = 0;
	_HdmiSinkAvCap.ui1_sink_support_static_hdr = 0;
	_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr = 0;
	_HdmiSinkAvCap.ui1_sink_hdr_content_max_luminance = 0;
	_HdmiSinkAvCap.ui1_sink_hdr_content_max_frame_average_luminance = 0;
	_HdmiSinkAvCap.ui1_sink_hdr_content_min_luminance = 0;
	_HdmiSinkAvCap.ui1_sink_hdr10plus_app_version = 0xff;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_length = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_version = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v1_low_latency = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_interface = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_low_latency_support = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444 = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_support_backlight_control = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_backlt_min_lumal = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmin = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmax = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tminPQ = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmaxPQ = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wx = 0;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wy = 0;
	for (bInx = 0; bInx < 32; bInx++)
		_HdmiSinkAvCap.ui1_sink_dolbyvision_block[bInx] = 0;
	for (bInx = 0; bInx < 7; bInx++)
		_HdmiSinkAvCap.ui1_sink_hdr_block[bInx] = 0;
	for (bInx = 0; bInx < 8; bInx++)
		_HdmiSinkAvCap.ui1_sink_hfvsdb_block[bInx] = 0;
	_HdmiSinkAvCap.ui1_sink_hf_vsdb_info = 0;
	_HdmiSinkAvCap.ui1_CNC = 0;
	_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = FALSE;

	if (fgIsHdmiNoEDIDCheck())
		vSetNoEdidChkInfo();

}

/* Remove original MTK change for EDID print as formating is not correct
 * use from abc123/abc123 to print in correct format */
void vShowEdidRawData(void)
{
	unsigned short bTemp, i, j;
	unsigned char *p;

	HDMI_EDID_FUNC();

	for (bTemp = 0; bTemp < 2; bTemp++) {
		printk("===================================================================\n");
		printk("   EDID Block Number=#%d\n", bTemp);
		printk("   | 00  01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f\n");
		printk("===================================================================\n");
		j = bTemp * EDID_BLOCK_LEN;
		for (i = 0; i < 8; i++) {
			p = &_bEdidData[j + i * 16];
			printk("%02x:  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x  "
			"%02x  %02x  %02x  %02x  %02x  %02x\n",
				i * 16 + j, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		}
	}
	printk("===================================================================\n");

}

unsigned char hdmi_fgreadedid(unsigned char i1noedid)
{
	unsigned char bIdx, bBlockIdx;
	unsigned char bExtBlockNo;
	unsigned short i;

	HDMI_EDID_FUNC();

	if (i1noedid == INTERNAL_EDID) {
		_fgHdmiNoEdidCheck = FALSE;
		vSetSharedInfo(SI_EDID_EXT_BLOCK_NO, 1);

		for (i = 0; i < 256; i++)
			_bEdidData[i] = _bEdidData2[i];
		return 1;
	} else if (i1noedid == NO_EDID) {
		vSetSharedInfo(SI_EDID_EXT_BLOCK_NO, 1);
		_fgHdmiNoEdidCheck = TRUE;
		for (i = 0; i < 256; i++)
			_bEdidData[i] = 0;
		return 1;
	}
		_fgHdmiNoEdidCheck = FALSE;

	/* block 0 : standard EDID block, address 0x00 ~ 0x7F w/ device ID=0xA0 */
	bExtBlockNo = 0xff;

	for (bBlockIdx = 0; bBlockIdx <= bExtBlockNo; bBlockIdx++) {
		for (bIdx = 0; bIdx < 5; bIdx++) {
			if ((bBlockIdx * EDID_BLOCK_LEN) < EDID_SIZE) {	/* for Buffer overflow error */
				if (fgDDCDataRead
				    (EDID_ID + (bBlockIdx >> 1),
				     0x00 + (bBlockIdx & 0x01) * EDID_BLOCK_LEN, EDID_BLOCK_LEN,
				     &_bEdidData[bBlockIdx * EDID_BLOCK_LEN]) == TRUE) {
					HDMI_EDID_LOG("[HDMI]EDID block[%d]: CheckSum = 0x%x\n",
						      bBlockIdx,
						      _bEdidData[(bBlockIdx + 1) * EDID_BLOCK_LEN -
								 1]);
					break;
				}
				{
					if (bIdx == 4)
						return 0;

				}
			}
		}

		bExtBlockNo = _bEdidData[EDID_ADDR_EXT_BLOCK_FLAG];
		vSetSharedInfo(SI_EDID_EXT_BLOCK_NO, bExtBlockNo);
	}

	return 1;
}

void vAnalyzeDTD(unsigned short ui2Active, unsigned short ui2HBlanking, unsigned char bVfiq,
	unsigned char bFormat, unsigned char fgFirstDTD)
{
	unsigned int ui4NTSC = _HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution;
	unsigned int ui4PAL = _HdmiSinkAvCap.ui4_sink_dtd_pal_resolution;
	unsigned int ui41stNTSC = _HdmiSinkAvCap.ui4_sink_1st_dtd_ntsc_resolution;
	unsigned int ui41stPAL = _HdmiSinkAvCap.ui4_sink_1st_dtd_pal_resolution;

	HDMI_EDID_FUNC();

	switch (ui2Active) {
	case 0x5a0:		/* 480i */
		if (ui2HBlanking == 0x114) {	/* NTSC */
			if (bFormat == 0) {	/* p-scan */
				ui4NTSC |= SINK_480P_1440;
				if (fgFirstDTD)
					ui41stNTSC |= SINK_480P_1440;
			} else {
				ui4NTSC |= SINK_480I;
				if (fgFirstDTD)
					ui41stNTSC |= SINK_480I;
			}
		} else if (ui2HBlanking == 0x120) {	/* PAL */
			if (bFormat == 0) {	/* p-scan */
				ui4PAL |= SINK_576P_1440;
				if (fgFirstDTD)
					ui41stPAL |= SINK_576P_1440;
			} else {
				ui4PAL |= SINK_576I;
				if (fgFirstDTD)
					ui41stPAL |= SINK_576I;
			}
		}
		break;
	case 0x2d0:		/* 480p */
		if ((ui2HBlanking == 0x8a) && (bFormat == 0)) {	/* NTSC, p-scan */
			ui4NTSC |= SINK_480P;
			if (fgFirstDTD)
				ui41stNTSC |= SINK_480P;
		} else if ((ui2HBlanking == 0x90) && (bFormat == 0)) {	/* PAL, p-scan */
			ui4PAL |= SINK_576P;
			if (fgFirstDTD)
				ui41stPAL |= SINK_576P;
		}
		break;
	case 0x500:		/* 720p */
		if ((ui2HBlanking == 0x172) && (bFormat == 0)) {	/* NTSC, p-scan */
			ui4NTSC |= SINK_720P60;
			if (fgFirstDTD)
				ui41stNTSC |= SINK_720P60;
		} else if ((ui2HBlanking == 0x2bc) && (bFormat == 0)) {	/* PAL, p-scan */
			ui4PAL |= SINK_720P50;
			if (fgFirstDTD)
				ui41stPAL |= SINK_720P50;
		}
		break;
	case 0x780:		/* 1080i, 1080P */
		if ((ui2HBlanking == 0x118) && (bFormat == 1)) {
			ui4NTSC |= SINK_1080I60;
			if (fgFirstDTD) {
				ui41stNTSC |= SINK_1080I60;
			}
			HDMI_EDID_LOG("Support 1080i60\n");
		} else if ((ui2HBlanking == 0x118) && (bFormat == 0)) {
			if ((bVfiq >= 29) && (bVfiq <= 31)) {
			    ui4NTSC |= SINK_1080P30;
			    if (fgFirstDTD) {
					ui41stNTSC |= SINK_1080P30;
			    }
				HDMI_EDID_LOG("Support 1080P30\n");
			} else {
			ui4NTSC |= SINK_1080P60;
				if (fgFirstDTD) {
				ui41stNTSC |= SINK_1080P60;
				}
				HDMI_EDID_LOG("Support 1080P60\n");
			}
		} else if ((ui2HBlanking == 0x2d0) && (bFormat == 1)) {
			ui4PAL |= SINK_1080I50;
			if (fgFirstDTD) {
				ui41stPAL |= SINK_1080I50;
			}
			HDMI_EDID_LOG("Support 1080i50\n");
		} else if ((ui2HBlanking == 0x2d0) && (bFormat == 0)) {
			if ((bVfiq >= 24) && (bVfiq <= 26)) {
				ui4PAL |= SINK_1080P25;
			    if (fgFirstDTD) {
					ui41stPAL |= SINK_1080P25;
			    }
				HDMI_EDID_LOG("Support 1080P25\n");
			} else {
			ui4PAL |= SINK_1080P50;
				if (fgFirstDTD) {
				ui41stPAL |= SINK_1080P50;
		}
				HDMI_EDID_LOG("Support 1080P50\n");
			}
		} else if ((ui2HBlanking == 0x33e) && (bFormat == 0)) {
			if ((bVfiq >= 23) && (bVfiq <= 25)) {
				ui4PAL |= SINK_1080P24;
			    if (fgFirstDTD) {
					ui41stPAL |= SINK_1080P24;
			    }
				HDMI_EDID_LOG("Support 1080P24\n");
			}
		}
		break;
	}
	_HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution = ui4NTSC;
	_HdmiSinkAvCap.ui4_sink_dtd_pal_resolution = ui4PAL;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_ntsc_resolution = ui41stNTSC;
	_HdmiSinkAvCap.ui4_sink_1st_dtd_pal_resolution = ui41stPAL;
}

unsigned char fgParserEDID(unsigned char *prbData)
{
	unsigned char bIdx;
	unsigned char bTemp = 0;
	unsigned short ui2HActive, ui2HBlanking;
	unsigned char Vfiq = 1;
	unsigned int dHtotal = 1, dVtotal = 1, ui2PixClk = 1;
	unsigned short ui2VActive, ui2VBlanking;

	HDMI_EDID_FUNC();

	_HdmiSinkAvCap.ui1_Edid_Version = *(prbData + EDID_ADDR_VERSION);
	_HdmiSinkAvCap.ui1_Edid_Revision = *(prbData + EDID_ADDR_REVISION);
	_HdmiSinkAvCap.ui1_Display_Horizontal_Size = *(prbData + EDID_IMAGE_HORIZONTAL_SIZE);
	_HdmiSinkAvCap.ui1_Display_Vertical_Size = *(prbData + EDID_IMAGE_VERTICAL_SIZE);

	/* Step 1: check if EDID header pass */
	/* ie. EDID[0] ~ EDID[7] = specify header pattern */
	for (bIdx = EDID_ADDR_HEADER; bIdx < (EDID_ADDR_HEADER + EDID_HEADER_LEN); bIdx++) {
		if (*(prbData + bIdx) != aEDIDHeader[bIdx]) {
			HDMI_PLUG_LOG("fgParserEDID FALSE Header = 0x%x\n",
				      *(prbData + bIdx));
			return FALSE;
		}
	}

	/* Step 2: Check if EDID checksume pass */
	/* ie. value of EDID[0] + ... + [0x7F] = 256*n */
	for (bIdx = 0; bIdx < EDID_BLOCK_LEN; bIdx++) {
		/* add the value into checksum */
		bTemp += *(prbData + bIdx);
	}

	/* check if EDID checksume pass */
	if (bTemp) {
		HDMI_PLUG_LOG("fgParserEDID FALSE Checksum = 0x%x\n", bTemp);
		return FALSE;
	}
		_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup &= ~SINK_BASE_BLK_CHKSUM_ERR;

	/* [3.3] read-back H active line to define EDID resolution */
	for (bIdx = 0; bIdx < 2; bIdx++) {
		ui2HActive =
		    (unsigned
		     short)(*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx +
			      OFST_H_ACT_BLA_HI) & 0xf0) << 4;
		ui2HActive |= *(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_H_ACTIVE_LO);
		ui2HBlanking =
		    (unsigned
		     short)(*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx +
			      OFST_H_ACT_BLA_HI) & 0x0f) << 8;
		ui2HBlanking |=
		    *(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_H_BLANKING_LO);
		bTemp = (*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_FLAGS) & 0x80) >> 7;

		ui2VActive = (unsigned short)*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_V_ACTIVE_LO) ;
		ui2VActive |= (unsigned short)(*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_V_ACTIVE_HI) & 0xf0) << 4;
		ui2VBlanking = (unsigned short)(*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_V_ACTIVE_HI) & 0x0f) << 8;
		ui2VBlanking |= *(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_V_BLANKING_LO);
		ui2PixClk = (unsigned short)*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_PXL_CLK_LO);
		ui2PixClk |= ((unsigned short)*(prbData + EDID_ADDR_TIMING_DSPR_1 + 18 * bIdx + OFST_PXL_CLK_HI)) << 8;

		ui2PixClk = ui2PixClk * 10000;
		dHtotal = (ui2HActive + ui2HBlanking);
		dVtotal = (ui2VActive + ui2VBlanking);
		Vfiq = 1;
		if (((dHtotal * dVtotal) != 0) && (ui2PixClk != 0))
			Vfiq = ui2PixClk / (dHtotal * dVtotal);
		HDMI_EDID_LOG("base,clk=%d,h=%d,v=%d,vfiq=%d\n",
			ui2PixClk, dHtotal, dVtotal, Vfiq);
		if (bIdx == 0)
			vAnalyzeDTD(ui2HActive, ui2HBlanking, Vfiq, bTemp, TRUE);
		else
			vAnalyzeDTD(ui2HActive, ui2HBlanking, Vfiq, bTemp, FALSE);
	}

	/* if go here, ie. parsing EDID data ok !! */
	return TRUE;
}

static void vParser_Video_Data_Block(unsigned char *prData, unsigned char Len)
{
	/* Video data block */
	unsigned int ui4CEA_NTSC = 0, ui4CEA_PAL = 0, ui4OrgCEA_NTSC = 0, ui4OrgCEA_PAL =
	    0, ui4NativeCEA_NTSC = 0, ui4NativeCEA_PAL = 0;
	unsigned int ui4Temp = 0;
	unsigned int bIdx;
	unsigned char bNo = Len, b_Native_bit;

	for (bIdx = 0; bIdx < bNo; bIdx++) {
		if (*(prData + 1 + bIdx) & 0x80) {	/* Native bit */
			ui4OrgCEA_NTSC = ui4CEA_NTSC;
			ui4OrgCEA_PAL = ui4CEA_PAL;
		}

		_ui4svd_128_VIC[bIdx] = (*(prData + 1 + bIdx) & 0x7f);
		b_Native_bit = (*(prData + 1 + bIdx) & 0x80);

		switch (*(prData + 1 + bIdx) & 0x7f) {
		case 1:
			ui4CEA_NTSC |= SINK_VGA;
			ui4OrgCEA_NTSC |= SINK_VGA;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_VGA;
			break;

		case 6:
			ui4CEA_NTSC |= SINK_480I;
			ui4OrgCEA_NTSC |= SINK_480I;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480I;
			break;

		case 7:
			ui4CEA_NTSC |= SINK_480I;
			ui4OrgCEA_NTSC |= SINK_480I;	/* 16:9 */
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480I;
			break;
		case 2:
			ui4CEA_NTSC |= SINK_480P;
			ui4OrgCEA_NTSC |= SINK_480P;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480P;

			break;
		case 3:
			ui4CEA_NTSC |= SINK_480P;
			ui4OrgCEA_NTSC |= SINK_480P;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480P;

			break;
		case 14:
		case 15:
			ui4CEA_NTSC |= SINK_480P_1440;
			ui4OrgCEA_NTSC |= SINK_480P_1440;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480P_1440;

			break;
		case 4:
			ui4CEA_NTSC |= SINK_720P60;
			ui4OrgCEA_NTSC |= SINK_720P60;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_720P60;

			break;
		case 5:
			ui4CEA_NTSC |= SINK_1080I60;
			ui4OrgCEA_NTSC |= SINK_1080I60;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_1080I60;
			break;
		case 21:
			ui4CEA_PAL |= SINK_576I;
			ui4OrgCEA_PAL |= SINK_576I;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576I;
			break;

		case 22:
			ui4CEA_PAL |= SINK_576I;
			ui4OrgCEA_PAL |= SINK_576I;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576I;

			break;
		case 16:
			ui4CEA_NTSC |= SINK_1080P60;
			ui4OrgCEA_NTSC |= SINK_1080P60;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_1080P60;
			break;

		case 17:
			ui4CEA_PAL |= SINK_576P;
			ui4OrgCEA_PAL |= SINK_576P;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576P;
			break;

		case 18:
			ui4CEA_PAL |= SINK_576P;
			ui4OrgCEA_PAL |= SINK_576P;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576P;

			break;

		case 29:
		case 30:
			ui4CEA_PAL |= SINK_576P_1440;
			ui4OrgCEA_PAL |= SINK_576P_1440;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576P_1440;
			break;

		case 19:
			ui4CEA_PAL |= SINK_720P50;
			ui4OrgCEA_PAL |= SINK_720P50;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_720P50;

			break;
		case 20:
			ui4CEA_PAL |= SINK_1080I50;
			ui4OrgCEA_PAL |= SINK_1080I50;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_1080I50;

			break;

		case 31:
			ui4CEA_PAL |= SINK_1080P50;
			ui4OrgCEA_PAL |= SINK_1080P50;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_1080P50;
			break;

		case 32:
			ui4CEA_NTSC |= SINK_1080P24;
			ui4CEA_PAL |= SINK_1080P24;
			ui4CEA_NTSC |= SINK_1080P23976;
			ui4CEA_PAL |= SINK_1080P23976;
			ui4OrgCEA_PAL |= SINK_1080P24;
			ui4OrgCEA_NTSC |= SINK_1080P23976;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_1080P24;

			break;

		case 33:
			/* ui4CEA_NTSC |= SINK_1080P25; */
			ui4CEA_PAL |= SINK_1080P25;
			ui4OrgCEA_PAL |= SINK_1080P25;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_1080P25;

			break;

		case 34:
			ui4CEA_NTSC |= SINK_1080P30;
			ui4CEA_NTSC |= SINK_1080P2997;
			ui4CEA_PAL |= SINK_1080P30;
			ui4CEA_PAL |= SINK_1080P2997;
			ui4OrgCEA_PAL |= SINK_1080P30;
			ui4OrgCEA_NTSC |= SINK_1080P2997;
			if (*(prData + 1 + bIdx) & 0x80)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_1080P30;
			break;

		case 35:
			ui4CEA_NTSC |= SINK_480P_2880;
			ui4OrgCEA_NTSC |= SINK_480P_2880_4_3;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480P_2880_4_3;
			break;

		case 36:
			ui4CEA_NTSC |= SINK_480P_2880;
			ui4OrgCEA_NTSC |= SINK_480P_2880;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_NTSC |= SINK_480P_2880;
			break;

		case 37:
			ui4CEA_PAL |= SINK_576P_2880;
			ui4OrgCEA_PAL |= SINK_576P_2880_4_3;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576P_2880_4_3;
			break;

		case 38:
			ui4CEA_PAL |= SINK_576P_2880;
			ui4OrgCEA_PAL |= SINK_576P_2880;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_576P_2880;

			break;
		case 93:
		case 103:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |=
			    SINK_2160P_24HZ | SINK_2160P_23_976HZ;
			break;
		case 94:
		case 104:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2160P_25HZ;
			break;
		case 95:
		case 105:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |=
			    SINK_2160P_29_97HZ | SINK_2160P_30HZ;
			break;
		case 98:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |=
			    SINK_2161P_24HZ | SINK_2161P_23_976HZ;
			break;
		case 99:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2161P_25HZ;
			break;
		case 100:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |=
			    SINK_2161P_29_97HZ | SINK_2161P_30HZ;
			break;

		case 96:
		case 106:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2160P_50HZ;
			break;
		case 97:
		case 107:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2160P_60HZ;
			break;
		case 101:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2161P_50HZ;
			break;
		case 102:
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic |= SINK_2161P_60HZ;
			break;
		case 60:
			ui4CEA_NTSC |= SINK_720P24;
			ui4CEA_PAL |= SINK_720P24;
			ui4CEA_NTSC |= SINK_720P23976;
			ui4CEA_PAL |= SINK_720P23976;
			ui4OrgCEA_PAL |= SINK_720P24;
			ui4OrgCEA_NTSC |= SINK_720P23976;
			if (b_Native_bit)	/* Native bit */
				ui4NativeCEA_PAL |= SINK_720P24;
			break;

		default:
			break;
		}

		if (bIdx < 0x10) {
			switch (*(prData + 1 + bIdx) & 0x7f) {
			case 6:
			case 7:
				ui4Temp = SINK_480I;
				break;
			case 2:
			case 3:
				ui4Temp = SINK_480P;
				break;
			case 14:
			case 15:
				ui4Temp = SINK_480P_1440;
				break;
			case 4:
				ui4Temp = SINK_720P60;
				break;
			case 5:
				ui4Temp = SINK_1080I60;
				break;
			case 21:
			case 22:
				ui4Temp = SINK_576I;
				break;
			case 16:
				ui4Temp = SINK_1080P60;
				break;

			case 17:
			case 18:
				ui4Temp = SINK_576P;
				break;
			case 29:
			case 30:
				ui4Temp = SINK_576P_1440;
				break;
			case 19:
				ui4Temp = SINK_720P50;
				break;
			case 20:
				ui4Temp = SINK_1080I50;
				break;

			case 31:
				ui4Temp = SINK_1080P50;
				break;

			case 32:
				ui4Temp |= SINK_1080P24;
				ui4Temp |= SINK_1080P23976;
				break;

			case 33:
				/* ui4CEA_NTSC |= SINK_1080P25; */
				ui4Temp = SINK_1080P25;
				break;

			case 34:
				ui4Temp |= SINK_1080P30;
				ui4Temp |= SINK_1080P2997;

				break;

			default:
				break;


			}

			_ui4First_16_NTSC_VIC |= ui4CEA_NTSC;
			_ui4First_16_PAL_VIC |= ui4CEA_PAL;
			_ui4First_16_VIC[bIdx] = ui4Temp;
		}

		if (*(prData + 1 + bIdx) & 0x80) {
			ui4OrgCEA_NTSC = ui4CEA_NTSC & (~ui4OrgCEA_NTSC);
			ui4OrgCEA_PAL = ui4CEA_PAL & (~ui4OrgCEA_PAL);

			if (ui4OrgCEA_NTSC) {
				_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution = ui4OrgCEA_NTSC;
			} else if (ui4OrgCEA_PAL) {
				_HdmiSinkAvCap.ui4_sink_native_pal_resolution = ui4OrgCEA_PAL;
			} else {
				_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution = 0;
				_HdmiSinkAvCap.ui4_sink_native_pal_resolution = 0;
			}
		}
	}			/* for(bIdx = 0; bIdx < bNo; bIdx++) */

	_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution |= ui4CEA_NTSC;
	_HdmiSinkAvCap.ui4_sink_cea_pal_resolution |= ui4CEA_PAL;
	_HdmiSinkAvCap.ui4_sink_org_cea_ntsc_resolution |= ui4OrgCEA_NTSC;
	_HdmiSinkAvCap.ui4_sink_org_cea_pal_resolution |= ui4OrgCEA_PAL;
	_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution |= ui4NativeCEA_NTSC;
	_HdmiSinkAvCap.ui4_sink_native_pal_resolution |= ui4NativeCEA_PAL;

}

static void vParser_Audio_Data_Block(unsigned char *prData, unsigned char Len)
{
	unsigned int bIdx;
	unsigned char bLengthSum;
	unsigned char bNo, bAudCode, bPcmChNum;

	bNo = Len;
	/* Audio data block */
	_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup &= ~(SINK_SAD_NO_EXIST);
	for (bIdx = 0; bIdx < (bNo / 3); bIdx++) {
		bLengthSum = bIdx * 3;
		bAudCode = (*(prData + bLengthSum + 1) & 0x78) >> 3;	/* get audio code */

		if (bAudCode == AVD_DOLBY_PLUS) {	/* AVD_DOLBY_PLUS */
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bPcmChNum -
									2] |=
					(*(prData + bLengthSum + 2) & 0x7f);
			}
		}
		if (((*(prData + bLengthSum + 3) & 0x01) == 0x1) && (bAudCode == AVD_DOLBY_PLUS)) {
			bAudCode = AVD_DOLBY_ATMOS;
			_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = TRUE;
		}

		if ((bAudCode >= AVD_LPCM) && bAudCode <= AVD_WMA) {
			_HdmiSinkAvCap.ui2_sink_aud_dec |= (1 << (bAudCode - 1));	/* PCM:1 HDMI_SINK_AUDIO_DEC_LPCM AC3:2 HDMI_SINK_AUDIO_DEC_AC3 */
			/*must support dolby plus if support atmos according to spec*/

		if (bAudCode == AVD_DOLBY_ATMOS)
				_HdmiSinkAvCap.ui2_sink_aud_dec |= (1 << (AVD_DOLBY_PLUS - 1));
		}

		if (bAudCode == AVD_LPCM) {	/* LPCM */
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
				_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bPcmChNum -
								     2] |=
				    (*(prData + bLengthSum + 3) & 0x07);
			}
		}

		if (bAudCode == AVD_AC3) {	/* AVD_AC3 */
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_DST) {	/* DST */
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_dst_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_DSD) {	/* DSD */
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_MPEG1_AUD) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_mpeg1_ch_sampling[bPcmChNum -
									  2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_MP3) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_mp3_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_MPEG2_AUD) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_mpeg2_ch_sampling[bPcmChNum -
									  2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_AAC) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_aac_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_DTS) {
			_HdmiSinkAvCap.ui4_sink_dts_fs = (*(prData + bLengthSum + 2) & 0x7f);
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[bPcmChNum -
									2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_ATRAC) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_atrac_ch_sampling[bPcmChNum - 2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_DOLBY_PLUS) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_dolby_plus_ch_sampling[bPcmChNum - 2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
			if ((*(prData + bLengthSum + 3) & 0x01) == 0x1) {
				/* SAD DD+, Byte-3 Bit-0 is Dolby Atmos */
				_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = TRUE;
			}
			if ((*(prData + bLengthSum + 3) & 0x02) == 0x2) {
				/* SAD DD+, Byte-3 Bit-1 is Dolby Atmos ACMOD 28 */
				_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = TRUE;
			}

		}

		if (bAudCode == AVD_DTS_HD) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[bPcmChNum - 2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_MAT_MLP) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_mat_mlp_ch_sampling[bPcmChNum - 2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}

		if (bAudCode == AVD_WMA) {
			bPcmChNum = (*(prData + bLengthSum + 1) & 0x07) + 1;
			if (bPcmChNum >= 2) {
				_HdmiSinkAvCap.ui1_sink_wma_ch_sampling[bPcmChNum - 2] |=
				    (*(prData + bLengthSum + 2) & 0x7f);
			}
		}
	}			/* for(bIdx = 0; bIdx < bNo/3; bIdx++) */
}

static void vParser_Speaker_Allocation(unsigned char *prData, unsigned char Len)
{
	_HdmiSinkAvCap.ui1_sink_spk_allocation = *(prData + 1) & 0x7f;

}

static void vParser_VendorSpecfic_Data_Block(unsigned char *prData, unsigned char Len)
{
	unsigned char bTemp13;
	unsigned char bTemp8;
	unsigned char bTemp;
	unsigned char bNo = Len;
	unsigned char bLatency_offset = 0;
	unsigned char b3D_Multi_present = 0, b3D_Structure_7_0 = 1, b3D_MASK_15_8 =
	    1, b3D_MASK_7_0 = 1, b2D_VIC_order_Index = 0;
	unsigned char i, bTemp14 = 1, bDataTemp = 1;
	unsigned int u23D_MASK_ALL;

	/* HF VDSB exit */
	for (bTemp = 0; bTemp < EDID_VSDB_LEN; bTemp++) {
		if (*(prData + bTemp + 1) != aEDIDHFVSDBHeader[bTemp])
			break;
	}
	if (bTemp == EDID_VSDB_LEN) {
		vSetSharedInfo(SI_EDID_VSDB_EXIST, TRUE);
		_HdmiSinkAvCap.b_sink_support_hdmi_mode = TRUE;
		for (i = 0; i < 8; i++) {
			_HdmiSinkAvCap.ui1_sink_hfvsdb_block[i] =
			    *(prData + i);
		}

		_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate = *(prData + 5) * 5;
		bTemp13 = *(prData + 6);
		if (bTemp13 & 0x88)
			_HdmiSinkAvCap.b_sink_LTE_340M_sramble = TRUE;
		if (bTemp13 & 0x80) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
				EDID_HF_VSDB_SCDC_PRESENT;
			_HdmiSinkAvCap.b_sink_SCDC_present = TRUE;
		}
		if (bTemp13 & 0x40) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
			    EDID_HF_VSDB_RR_CAPABLE;
		}
		if (bTemp13 & 0x08) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
			    EDID_HF_VSDB_LTE_340MCSC_SCRAMBLE;
		}
		if (bTemp13 & 0x04) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
			    EDID_HF_VSDB_INDEPENDENT_VIEW;
		}
		if (bTemp13 & 0x02) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
			    EDID_HF_VSDB_DUAL_VIEW;
		}
		if (bTemp13 & 0x01) {
			_HdmiSinkAvCap.ui1_sink_hf_vsdb_info |=
			    EDID_HF_VSDB_3D_OSD_DISPARITY;
		}
		bTemp13 = *(prData + 7);
		_HdmiSinkAvCap.ui1_sink_dc420_color_bit = (bTemp13 & 0x07);
	}


	/* VDSB exit */
	for (bTemp = 0; bTemp < EDID_VSDB_LEN; bTemp++) {
		if (*(prData + bTemp + 1) != aEDIDVSDBHeader[bTemp])
			break;
	}
	/* for loop to end, is. VSDB header match */
	if (bTemp == EDID_VSDB_LEN) {
		vSetSharedInfo(SI_EDID_VSDB_EXIST, TRUE);
		_HdmiSinkAvCap.b_sink_support_hdmi_mode = TRUE;
		/* Read CEC physis address */
		if (bNo >= 5) {
			_HdmiSinkAvCap.ui2_sink_cec_address =
			    (*(prData + 4) << 8) | (*(prData + 5));
		} else {
			_HdmiSinkAvCap.ui2_sink_cec_address = 0xFFFF;
		}

		/* Read Support AI */
		if (bNo >= 6) {
			bTemp = *(prData + 6);
			if (bTemp & 0x80) {
				_HdmiSinkAvCap.ui1_sink_support_ai = 1;
				vSetSharedInfo(SI_HDMI_SUPPORTS_AI, 1);
			} else {
				_HdmiSinkAvCap.ui1_sink_support_ai = 0;
				vSetSharedInfo(SI_HDMI_SUPPORTS_AI, 0);
			}

			/* kenny add 2010/4/25 for repeater EDID check */
			_HdmiSinkAvCap.u1_sink_support_ai = i4SharedInfo(SI_HDMI_SUPPORTS_AI);
			_HdmiSinkAvCap.e_sink_rgb_color_bit = ((bTemp >> 4) & 0x07);

			_HdmiSinkAvCap.u1_sink_max_tmds = *(prData + 7);

			if (bTemp & 0x08) {	/* support YCbCr Deep Color */
				_HdmiSinkAvCap.e_sink_ycbcr_color_bit = ((bTemp >> 4) & 0x07);
			}
		} else {
			_HdmiSinkAvCap.ui1_sink_support_ai = 0;
			vSetSharedInfo(SI_HDMI_SUPPORTS_AI, 0);
		}

		/* max tmds clock */
		if (bNo >= 7) {
			bTemp = *(prData + 7);
			_HdmiSinkAvCap.ui1_sink_max_tmds_clock = ((unsigned short)bTemp) * 5;
			/* _HdmiSinkAvCap.ui1_sink_max_tmds_clock = 190; */
		} else {
			_HdmiSinkAvCap.ui1_sink_max_tmds_clock = 0;
		}

		/* Read Latency data */
		if (bNo >= 8) {
			bTemp = *(prData + 8);
			if (bTemp & 0x20)
				_HdmiSinkAvCap.b_sink_hdmi_video_present = 1;
			else
				_HdmiSinkAvCap.b_sink_hdmi_video_present = 0;
			_HdmiSinkAvCap.ui1_sink_content_cnc = bTemp & 0x0f;

			if (bTemp & 0x80) {	/* Latency Present */
				/* kenny add 2010/4/25 */
				_HdmiSinkAvCap.ui1_sink_p_latency_present = TRUE;
				if ((bNo >= 9) && (*(prData + 9) != 0x00) && (*(prData + 9) != 0xFF))
					_HdmiSinkAvCap.ui1_sink_p_video_latency = (*(prData + 9) - 1) << 1;
				if ((bNo >= 10) && (*(prData + 10) != 0x00) && (*(prData + 10) != 0xFF))
					_HdmiSinkAvCap.ui1_sink_p_audio_latency = (*(prData + 10) - 1) << 1;

			}
			if (bTemp & 0x40) {	/* Interlace Latency present */
				_HdmiSinkAvCap.ui1_sink_i_latency_present = TRUE;
				if ((bNo >= 11) && (*(prData + 11) != 0x00) && (*(prData + 11) != 0xFF))
					_HdmiSinkAvCap.ui1_sink_i_video_latency = (*(prData + 11) - 1) << 1;
				if ((bNo >= 12) && (*(prData + 12) != 0x00) && (*(prData + 12) != 0xFF))
					_HdmiSinkAvCap.ui1_sink_i_audio_latency = (*(prData + 12) - 1) << 1;
			}

			_HdmiSinkAvCap.ui1_CNC = bTemp & 0x0F;
		}

		if (bNo >= 8) {
			bTemp = *(prData + 8);

			if (!(bTemp & 0x80))	/* Latency Present */
				bLatency_offset = bLatency_offset + 2;
			if (!(bTemp & 0x40))	/* Interlace Latency present */
				bLatency_offset = bLatency_offset + 2;
		}
		if (bNo >= 13) {	/* kenny add */
			bTemp = *(prData + 13);
			if (bTemp & 0x80)
				_HdmiSinkAvCap.b_sink_3D_present = 1;
			else
				_HdmiSinkAvCap.b_sink_3D_present = 0;

		}
		if (bNo >= 8) {
			bTemp8 = *(prData + 8);

			if (bTemp8 & 0x20)
				;
		}

		if (bNo >= (13 - bLatency_offset)) {
			bTemp13 = *(prData + 13 - bLatency_offset);

			if (bTemp13 & 0x80) {
				_u4i_3D_VIC |= SINK_720P50;
				_u4i_3D_VIC |= SINK_720P60;
				_u4i_3D_VIC |= SINK_1080P23976;
				_u4i_3D_VIC |= SINK_1080P24;
				_HdmiSinkAvCap.b_sink_3D_present = TRUE;
			} else
				_HdmiSinkAvCap.b_sink_3D_present = FALSE;
		} else
			_HdmiSinkAvCap.b_sink_3D_present = FALSE;

		if (bNo >= (13 - bLatency_offset)) {
			bTemp13 = *(prData + 13 - bLatency_offset);

			if ((bTemp13 & 0x60) == 0x20)
				b3D_Multi_present = 0x20;
			else if ((bTemp13 & 0x60) == 0x40)
				b3D_Multi_present = 0x40;
			else
				b3D_Multi_present = 0x00;
		}

		if (bNo >= (14 - bLatency_offset))
			bTemp14 = *(prData + 14 - bLatency_offset);

		if (_HdmiSinkAvCap.b_sink_hdmi_video_present == TRUE) {
			/* hdmi_vic */
			if ((bNo > (14 - bLatency_offset))
				&& (((bTemp14 & 0xE0) >> 5) > 0)) {
				for (bTemp = 0; bTemp < ((bTemp14 & 0xE0) >> 5);
					 bTemp++) {
					if ((*
						 (prData + 15 - bLatency_offset +
						  bTemp)) == 0x01)
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic
							|=
							SINK_2160P_29_97HZ +
							SINK_2160P_30HZ;
					if ((*
						 (prData + 15 - bLatency_offset +
						  bTemp)) == 0x02)
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic
							|= SINK_2160P_25HZ;
					if ((*
						 (prData + 15 - bLatency_offset +
						  bTemp)) == 0x03)
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic
							|=
							SINK_2160P_23_976HZ +
							SINK_2160P_24HZ;
					if ((*
						 (prData + 15 - bLatency_offset +
						  bTemp)) == 0x04)
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic
							|= SINK_2161P_24HZ;
				}
			}
		}

		if (bNo > (14 - bLatency_offset + ((bTemp14 & 0xE0) >> 5))) {
			if (b3D_Multi_present == 0x20) {
				if (((15 - bLatency_offset + ((bTemp14 & 0xE0) >> 5)) +
				     (bTemp14 & 0x1F)) >=
				    (15 - bLatency_offset + ((bTemp14 & 0xE0) >> 5) + 2))
					/* b3D_Structure_15_8=*(prData+15+((bTemp&0xE0)>>5)); */
					b3D_Structure_7_0 = *(prData + 15 - bLatency_offset +
							      ((bTemp14 & 0xE0) >> 5) + 1);

				/* support frame packet */
				if ((b3D_Structure_7_0 & 0x01) == 0x01)
					for (i = 0; i < 0x10; i++)
						_u4i_3D_VIC |= _ui4First_16_VIC[i];

				while (((15 - bLatency_offset +
					 ((bTemp14 & 0xE0) >> 5)) +
					(bTemp14 & 0x1F)) >
				       ((15 - bLatency_offset +
					 ((bTemp14 & 0xE0) >> 5)) + 2 + b2D_VIC_order_Index)) {
					/* 2 is 3D_structure */
					bDataTemp =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + 2 + b2D_VIC_order_Index);
					if ((bDataTemp & 0x0F) < 0x08) {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 1;
					/* 3D_Structure=0,              support frame packet */
					if ((bDataTemp & 0x0F) == 0x00)
						_u4i_3D_VIC |=
						    _ui4First_16_VIC[((bDataTemp & 0xF0) >>
								      4)];
					} else {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 2;
					}
				}
			} else if (b3D_Multi_present == 0x40) {
				if (((15 - bLatency_offset +
				      ((bTemp14 & 0xE0) >> 5)) +
				     (bTemp14 & 0x1F)) >=
				    ((15 - bLatency_offset + ((bTemp14 & 0xE0) >> 5)) + 4)) {
					/* 4 is 3D_structure+3D_MASK */
					/* b3D_Structure_15_8=*(prData+15+((bTemp&0xE0)>>5)); */
					b3D_Structure_7_0 =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + 1);
					b3D_MASK_15_8 =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + 2);
					b3D_MASK_7_0 =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + 3);
					/* support frame packet */
					if ((b3D_Structure_7_0 & 0x01) == 0x01) {
						u23D_MASK_ALL =
						    (((unsigned short)(b3D_MASK_15_8)) << 8)
						    | ((unsigned short)(b3D_MASK_7_0));
					for (i = 0; i < 0x10; i++) {
					if (u23D_MASK_ALL & 0x0001)
						_u4i_3D_VIC |= _ui4First_16_VIC[i];

						u23D_MASK_ALL = u23D_MASK_ALL >> 1;
					}
					}

				}
				while (((15 - bLatency_offset +
					 ((bTemp14 & 0xE0) >> 5)) +
					(bTemp14 & 0x1F)) >
				       (15 - bLatency_offset +
					((bTemp14 & 0xE0) >> 5) + 4 + b2D_VIC_order_Index)) {
					bDataTemp =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + 4 + b2D_VIC_order_Index);
					if ((bDataTemp & 0x0F) < 0x08) {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 1;
						/* 3D_Structure=0 */
					if ((bDataTemp & 0x0F) == 0x00) {
						_u4i_3D_VIC |=
						    _ui4First_16_VIC[((bDataTemp & 0xF0) >>
								      4)];
					}

					} else {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 2;
					}
				}

			} else {
				b3D_Structure_7_0 = 0;
				while (((15 - bLatency_offset +
					 ((bTemp14 & 0xE0) >> 5)) +
					(bTemp14 & 0x1F)) >
				       ((15 - bLatency_offset +
					 ((bTemp14 & 0xE0) >> 5)) + b2D_VIC_order_Index)) {
					bDataTemp =
					    *(prData + 15 - bLatency_offset +
					      ((bTemp14 & 0xE0) >> 5) + b2D_VIC_order_Index);
					if ((bDataTemp & 0x0F) < 0x08) {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 1;
					/* 3D_Structure=0 */
					if ((bDataTemp & 0x0F) == 0x00) {
						_u4i_3D_VIC |=
						    _ui4First_16_VIC[((bDataTemp & 0xF0) >>
								      4)];
					}

					} else {
						b2D_VIC_order_Index = b2D_VIC_order_Index + 2;
					}
				}
			}

		}
		_HdmiSinkAvCap.ui4_sink_cea_3D_resolution = _u4i_3D_VIC;
	} /* if(bTemp==EDID_VSDB_LEN) */
	else {
		/* vSetSharedInfo(SI_EDID_VSDB_EXIST, FALSE); */
		/* _HdmiSinkAvCap.b_sink_support_hdmi_mode = FALSE; */
	}

}

void vParserDolbyVisionBlock(unsigned char *prData)
{
	unsigned char bVersion;
	unsigned char bLength;
	unsigned char bDataLen;

	bLength = (*prData) & 0x1F;
	bVersion = ((*(prData + 5)) >> 5) & 0x07;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_length = bLength + 1;
	_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_version = bVersion;

	if(bLength > 0x1A){
		bDataLen = 0x1A;
		HDMI_PLUG_LOG("[HDMI TX]Dolby Vision length > 0x1A, %d\n", bDataLen);
	} else {
		bDataLen = bLength;
	}
	memcpy(_HdmiSinkAvCap.ui1_sink_dolbyvision_block, prData, (bDataLen + 1));


	if ((bLength == 0x19) && (bVersion == 0)) {
		_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_DOLBY_HDR;
		if ((*(prData + 5)) & 0x01)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_YUV422_12BIT;
		if ((*(prData + 5)) & 0x02)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=  EDID_SUPPORT_DOLBY_HDR_2160P60;

		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tminPQ =
			((*(prData + 19)) << 4) | (((*(prData + 18)) >> 4) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmaxPQ =
			((*(prData + 20)) << 4) | ((*(prData + 18)) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx =
			((*(prData + 7)) << 4) | (((*(prData + 6)) >> 4) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry =
			((*(prData + 8)) << 4) | ((*(prData + 6)) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx =
			((*(prData + 10)) << 4) | (((*(prData + 9)) >> 4) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy =
			((*(prData + 11)) << 4) | ((*(prData + 9)) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx =
			((*(prData + 13)) << 4) | (((*(prData + 12)) >> 4) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By =
			((*(prData + 14)) << 4) | ((*(prData + 12)) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wx =
			((*(prData + 16)) << 4) | (((*(prData + 15)) >> 4) & 0x0F);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wy =
			((*(prData + 17)) << 4) | ((*(prData + 15)) & 0x0F);
		HDMI_PLUG_LOG("[HDMI TX]Dovi: vsdb v0, length=0x19, dynamic_hdr=0x%x\n", _HdmiSinkAvCap.ui1_sink_support_dynamic_hdr);
	} else if ((bLength == 0x0E) && (bVersion == 1)) {
		_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_DOLBY_HDR;
		if ((*(prData + 5)) & 0x01)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_YUV422_12BIT;
		if ((*(prData + 5)) & 0x02)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=  EDID_SUPPORT_DOLBY_HDR_2160P60;

		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmin = (*(prData + 7)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmax = (*(prData + 6)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx = *(prData + 9);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry = *(prData + 10);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx = *(prData + 11);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy = *(prData + 12);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx = *(prData + 13);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By = *(prData + 14);
		HDMI_PLUG_LOG("[HDMI TX]Dovi: vsdb v1, length=0x0E, dynamic_hdr=0x%x\n", _HdmiSinkAvCap.ui1_sink_support_dynamic_hdr);
	} else if ((bLength == 0x0B) && (bVersion == 1)) {
		_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_DOLBY_HDR;
		if ((*(prData + 5)) & 0x01)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_YUV422_12BIT;
		if ((*(prData + 5)) & 0x02)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=  EDID_SUPPORT_DOLBY_HDR_2160P60;

		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v1_low_latency = (*(prData + 8)) & 0x03;
		if (_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v1_low_latency == 1)
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_low_latency_support = 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmin = (*(prData + 7)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmax = (*(prData + 6)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx = ((*(prData + 11)) >> 3) | 0xA0;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry = (((*(prData + 11)) & 0x07) << 2)
						 | (((*(prData + 10)) & 0x01) << 1) | ((*(prData + 9)) & 0x01) | 0x40;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx = (*(prData + 9)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy = ((*(prData + 10)) >> 1) | 0x80;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx = ((*(prData + 8)) >> 5) | 0x20;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By = (((*(prData + 8)) >> 2) & 0x07) | 0x08;
		HDMI_PLUG_LOG("[HDMI TX]Dovi: vsdb v1, length=0x0B, dynamic_hdr=0x%x, v1 low-latency=0x%x\n",
					   _HdmiSinkAvCap.ui1_sink_support_dynamic_hdr, _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v1_low_latency);
	} else if ((bLength == 0x0B) && (bVersion == 2)) {
		_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_DOLBY_HDR;
		if ((*(prData + 5)) & 0x01)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=	EDID_SUPPORT_YUV422_12BIT;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_interface = (*(prData + 7)) & 0x03;

		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_low_latency_support = 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444 =
			(((*(prData + 8)) & 0x01) << 1) | ((*(prData + 9)) & 0x01);
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_support_backlight_control =
			((*(prData + 5)) >> 1) & 0x01;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_backlt_min_lumal = (*(prData + 6)) & 0x03;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tminPQ = ((*(prData + 6)) >> 3) * 20;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmaxPQ = ((*(prData + 7)) >> 3) * 65 + 2055;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx = ((*(prData + 10)) >> 3) | 0xA0;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry = ((*(prData + 11)) >> 3) | 0x40;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx = (*(prData + 8)) >> 1;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy = ((*(prData + 9)) >> 1) | 0x80;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx = ((*(prData + 10)) & 0x07) | 0x20;
		_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By = ((*(prData + 11)) & 0x07) | 0x08;
		HDMI_PLUG_LOG("[HDMI TX]Dovi: vsdb v2, length=0x0B, dynamic_hdr=0x%x, v2 low-latency=1, v2 interface=0x%x\n",
					   _HdmiSinkAvCap.ui1_sink_support_dynamic_hdr, _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_interface);
	}
}


static void vParser_User_Extension_Tag(unsigned char *prData, unsigned char Len)
{
	unsigned char bTemp, bIdx, i;
	unsigned int u4IEEE = 0;
	unsigned char u2svd_420_cmdb;

/* Use Extended Tag */
	if (*(prData + 1) == 0x05) {	/* Extend Tag code ==0x05 */
		if (*(prData + 2) & 0x1) {
			/* Suppot xvYcc601 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_XV_YCC601;
		}

		if (*(prData + 2) & 0x2) {
			/* Suppot xvYcc709 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_XV_YCC709;
		}

		if (*(prData + 2) & 0x4) {
			/* Suppot sYcc601 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_S_YCC601;
		}

		if (*(prData + 2) & 0x8) {
			/* Suppot adobe Ycc709 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_ADOBE_YCC601;
		}

		if (*(prData + 2) & 0x10) {
			/* Suppot adobe RGB */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_ADOBE_RGB;
		}

		if (*(prData + 2) & 0x20) {
			/* Suppot BT2020 Cycc */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_COLOR_SPACE_BT2020_CYCC;
		}

		if (*(prData + 2) & 0x40) {
			/* Suppot BT2020 Ycc */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_COLOR_SPACE_BT2020_YCC;
		}

		if (*(prData + 2) & 0x80) {
			/* Suppot BT2020 RGB */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_COLOR_SPACE_BT2020_RGB;
		}

		if (*(prData + 3) & 0x1) {
			/* support Gamut data P0 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_METADATA0;
		}

		if (*(prData + 3) & 0x2) {
			/* support Gamut data P1 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_METADATA1;
		}

		if (*(prData + 3) & 0x4) {
			/* support Gamut data P1 */
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_METADATA2;
		}
	} else if (*(prData + 1) == 0x6) {
		/* Extend Tag code ==0x6 */
		/* support SDR */
		if (*(prData + 2) & 0x1) {
			/* Suppot SDR */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_SDR;
		}

		if (*(prData + 2) & 0x2) {
			/* Suppot HDR */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_HDR;
		}

		if (*(prData + 2) & 0x4) {
			/* Suppot ST 2084 */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_SMPTE_ST_2084;
		}

		if (*(prData + 2) & 0x8) {
			/* Suppot Future EOTF */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_FUTURE_EOTF;
		}

		if (*(prData + 2) & 0x10) {
			/* Suppot ET4 */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_ET_4;
		}

		if (*(prData + 2) & 0x20) {
			/* Suppot ET5 */
			_HdmiSinkAvCap.ui1_sink_support_static_hdr |= EDID_SUPPORT_ET_5;
		}

		if ((*prData & 0x1F) >= 4) {
			/* Max luminance */
			_HdmiSinkAvCap.ui1_sink_hdr_content_max_luminance = *(prData + 4);
		}

		if ((*prData & 0x1F) >= 5) {
			/* support avarage luminance */
			_HdmiSinkAvCap.ui1_sink_hdr_content_max_frame_average_luminance =
			    *(prData + 5);
		}

		if ((*prData & 0x1F) >= 6) {
			/* support Min luminance */
			_HdmiSinkAvCap.ui1_sink_hdr_content_min_luminance = *(prData + 6);
		}

		for (bTemp = 0; bTemp < ((*prData & 0x1F) + 1); bTemp++) {
			if (bTemp <= 6)
				_HdmiSinkAvCap.ui1_sink_hdr_block[bTemp] = *(prData + bTemp);
		}
	} else if (*(prData + 1) == 0x7) {
		unsigned char version, type_len = 0;
		unsigned int type;

		if (Len >= 1)
		/* Extend Tag code ==0x7 */
		for (bIdx = 0; bIdx < Len - 1; bIdx += type_len) {
			type_len = *(prData + 2 + bIdx);
			if (type_len >= 2) {
				type = (*(prData + 2 + bIdx + 1)) + ((*(prData + 2 + bIdx + 2)) << 8);
				switch (type) {
				case 0x0001:
				case 0x0002:
				case 0x0003:
					break;
				case 0x0004:
					_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |= EDID_SUPPORT_HDR10_PLUS;
					if (type_len >= 3)
						version = (*(prData + 2 + bIdx + 3)) && 0x0f;
					break;
				default:
					break;
				}
			}
		}
	} else if (*(prData + 1) == 0x01) {
		/* Extend Tag code ==0x01 philips vs-vsdb HDR dobly */
		u4IEEE = ((*(prData + 4)) << 16) | ((*(prData + 3)) << 8) | (*(prData + 2));
		HDMI_EDID_LOG("u4IEEE = 0x%x, Len=%d\n", u4IEEE, Len);
		if (u4IEEE == aEDIDPhilipsVSVDBHeader) {
			if (Len >= 5) {
				bTemp = *(prData + 5);
				if (bTemp & 0x1)
					_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |=
					    EDID_SUPPORT_PHILIPS_HDR;
			}
		} else if (u4IEEE == aEDIDDoblyVSVDBHeader) {
			vParserDolbyVisionBlock(prData);
		} else if ((u4IEEE == aEDIDHdr10PlusVSVDBHeader) && (Len >= 5)) {
			_HdmiSinkAvCap.ui1_sink_hdr10plus_app_version = (*(prData + 5)) & 0x03;
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |= EDID_SUPPORT_HDR10_PLUS;
		}

	} else if (*(prData + 1) == 0xE) {
		/* Extend Tag code ==0xE */
		/* support ycbcr420 Only SVDs */
		HDMI_EDID_LOG("Support YCBCR 420 only video data block, %d\n", Len - 1);
		for (bIdx = 0; bIdx < Len - 1; bIdx++) {
			_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_YCBCR_420;
			HDMI_EDID_LOG(" vic = %d\n", (*(prData + 2 + bIdx) & 0x7f));
			switch (*(prData + 2 + bIdx) & 0x7f) {
			case 96:
			case 106:
				_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb |= SINK_2160P_50HZ;
				break;
			case 97:
			case 107:
				_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb |= SINK_2160P_60HZ;
				break;
			case 101:
				_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb |= SINK_2161P_50HZ;
				break;
			case 102:
				_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb |= SINK_2161P_60HZ;
				break;
			default:
				break;
			}
		}
	} else if (*(prData + 1) == 0xF) {
		/* Extend Tag code ==0xF */
		/* support ycbcr420 capability Map data block */
		HDMI_EDID_LOG("Support YCBCR 420 Capability Map data block, %d\n", Len - 1);
		_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_YCBCR_420_CAPABILITY;
		if (Len == 1)
			_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb =
			    SINK_2160P_50HZ | SINK_2160P_60HZ | SINK_2161P_50HZ | SINK_2161P_60HZ;
		else {
			for (bIdx = 0; bIdx < Len - 1; bIdx++) {
				u2svd_420_cmdb = *(prData + 2 + bIdx);
				HDMI_EDID_LOG(" u2svd_420_cmdb = %d\n", u2svd_420_cmdb);
				for (i = 0; i < 8; i++) {
					if (u2svd_420_cmdb & 0x0001) {
					switch (_ui4svd_128_VIC[bIdx * 8 + i] & 0x7f) {
					case 96:
					case 106:
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb
						    |= SINK_2160P_50HZ;
						break;
					case 97:
					case 107:
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb
						    |= SINK_2160P_60HZ;
						break;
					case 101:
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb
						    |= SINK_2161P_50HZ;
						break;
					case 102:
						_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb
						    |= SINK_2161P_60HZ;
						break;
					default:
						break;
					}
					}
					u2svd_420_cmdb = u2svd_420_cmdb >> 1;
				}
			}
		}

	} else if (*(prData + 1) == 0x0) {
		/* Extend Tag code ==0x0 */
		if (*(prData + 2) & 0x40) {
			/* support selectable, QS=1 */
			_HdmiSinkAvCap.ui2_sink_vcdb_data |= SINK_RGB_SELECTABLE;
		}
	}
}

void vSetEdidChkError(void)
{
	unsigned char bInx;

	HDMI_EDID_FUNC();
	vSetSharedInfo(SI_EDID_PARSING_RESULT, TRUE);
	vSetSharedInfo(SI_EDID_VSDB_EXIST, FALSE);
	_HdmiSinkAvCap.b_sink_support_hdmi_mode = TRUE;
	_HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution = SINK_480P;	/* 0x1fffff; */
	_HdmiSinkAvCap.ui4_sink_dtd_pal_resolution = SINK_576P;	/* 0x1fffff; */
	_HdmiSinkAvCap.ui2_sink_colorimetry = 0;
	_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_cea_pal_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution = 0;
	_HdmiSinkAvCap.ui4_sink_native_pal_resolution = 0;
	_HdmiSinkAvCap.ui2_sink_vcdb_data = 0;
	_HdmiSinkAvCap.ui2_sink_aud_dec = 1;	/* PCM only */
	_HdmiSinkAvCap.ui1_sink_dsd_ch_num = 0;
	for (bInx = 0; bInx < 7; bInx++) {
		if (bInx == 0)
			_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bInx] = 0x07;	/* 2ch max 48khz */
		else
			_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bInx] = 0x0;
		_HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_dst_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bInx] = 0;
		_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bInx] = 0;
	}

	for (bInx = 0; bInx < 7; bInx++) {
		if (bInx == 0)
			_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bInx] = 0x07;	/* 2ch 24 bits */
		else
			_HdmiSinkAvCap.ui1_sink_pcm_bit_size[bInx] = 0;
	}

	_HdmiSinkAvCap.ui1_sink_spk_allocation = 0;
	_HdmiSinkAvCap.ui1_sink_i_latency_present = 0;
	_HdmiSinkAvCap.ui1_sink_p_latency_present = 0;	/* kenny add 2010/4/25 */
	_HdmiSinkAvCap.ui1_sink_p_audio_latency = 0;
	_HdmiSinkAvCap.ui1_sink_p_video_latency = 0;
	_HdmiSinkAvCap.ui1_sink_i_audio_latency = 0;
	_HdmiSinkAvCap.ui1_sink_i_video_latency = 0;

	_HdmiSinkAvCap.e_sink_rgb_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.e_sink_ycbcr_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.ui1_sink_dc420_color_bit = HDMI_SINK_NO_DEEP_COLOR;
	_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup =
	    (SINK_BASIC_AUDIO_NO_SUP | SINK_SAD_NO_EXIST | SINK_BASE_BLK_CHKSUM_ERR |
	     SINK_EXT_BLK_CHKSUM_ERR);
	_HdmiSinkAvCap.ui2_sink_cec_address = 0xffff;

	_HdmiSinkAvCap.b_sink_edid_ready = FALSE;
	_HdmiSinkAvCap.ui1_sink_support_dolby_atoms = FALSE;
	_HdmiSinkAvCap.ui1_sink_support_ai = 0;

	_HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic = 0;
	_HdmiSinkAvCap.b_sink_SCDC_present = 0;
	_HdmiSinkAvCap.b_sink_LTE_340M_sramble = 0;
	_HdmiSinkAvCap.ui1_sink_max_tmds_clock = 0;
	_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate = 0;
}

void vParserCEADataBlock(unsigned char *prData, unsigned char bLen)
{
	unsigned char Header;
	int bIdx;
	unsigned char bType, dataBlock_Len;

	HDMI_EDID_FUNC();
	while (bLen) {
		if (bLen > 0x80)
			break;

		/* Step 1: get 1st data block type & total number of this data type */
		Header = *prData;
		bType = Header >> 5;	/* bit[7:5] */
		dataBlock_Len = Header & 0x1F;	/* bit[4:0] */

		if (bType == 0x02)
			vParser_Video_Data_Block(prData, dataBlock_Len);
		else if (bType == 0x01)
			vParser_Audio_Data_Block(prData, dataBlock_Len);
		else if (bType == 0x04)
			vParser_Speaker_Allocation(prData, dataBlock_Len);
		else if (bType == 0x03)
			vParser_VendorSpecfic_Data_Block(prData, dataBlock_Len);
		else if (bType == 0x07)
			vParser_User_Extension_Tag(prData, dataBlock_Len);
		else
			TX_DEF_LOG("vParserCEADataBlock: Invalid Data Block\n");

		/* re-assign the next data block address */
		prData += (dataBlock_Len + 1);	/* '1' means the tag byte */
		bLen -= (dataBlock_Len + 1);
	}			/* while(bLen) */

	for (bIdx = 6; bIdx >= 1; bIdx--) {
		_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[bIdx];
		_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[bIdx];
		_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[bIdx];
		_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_dts_ch_sampling[bIdx];
		_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[bIdx];
		_HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bIdx - 1] |=
		    _HdmiSinkAvCap.ui1_sink_dsd_ch_sampling[bIdx];
	}

	if (_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup & SINK_EXT_BLK_CHKSUM_ERR) {
		vSetSharedInfo(SI_EDID_VSDB_EXIST, FALSE);
		_HdmiSinkAvCap.b_sink_support_hdmi_mode = FALSE;
	}

}


unsigned char fgParserExtEDID(unsigned char *prData)
{
	unsigned char bIdx;
	unsigned char bTemp = 0;
	unsigned short ui2HActive, ui2HBlanking, ui2VActive, ui2VBlanking;
	unsigned char bOfst, *prCEAaddr;
	unsigned char Vfiq = 1;
	unsigned int dHtotal = 1,dVtotal = 1,ui2PixClk = 1;

	HDMI_EDID_FUNC();

	_HdmiSinkAvCap.ui1_ExtEdid_Revision = *(prData + EXTEDID_ADDR_REVISION);

	for (bIdx = 0; bIdx < EDID_BLOCK_LEN; bIdx++) {
		/* add the value into checksum */
		bTemp += *(prData + bIdx);	/* i4SharedInfo(wPos+bIdx); */
	}


	bTemp = 0;
	/* check if EDID checksume pass */
	if (bTemp)
		return FALSE;

	_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup &= ~SINK_EXT_BLK_CHKSUM_ERR;

	/* Step 1: get the offset value of 1st detail timing description within extension block */
	bOfst = *(prData + EXTEDID_ADDR_OFST_TIME_DSPR);

	if (*(prData + EDID_ADDR_EXTEND_BYTE3) & 0x40)	/* Support basic audio */
		_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup &= ~SINK_BASIC_AUDIO_NO_SUP;

	/* Max'0528'04, move to here, after read 0x80 ~ 0xFF because it is 0x83... */
	if (*(prData + EDID_ADDR_EXTEND_BYTE3) & 0x20) {	/* receiver support YCbCr 4:4:4 */
		_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_YCBCR_444;
	}

	if (*(prData + EDID_ADDR_EXTEND_BYTE3) & 0x10)	/* receiver support YCbCr 4:2:2 */
		_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_YCBCR_422;

	_HdmiSinkAvCap.ui2_sink_colorimetry |= SINK_RGB;
	/* Step 3: read-back the pixel clock of each timing descriptor */

	/* Step 4: read-back V active line to define EDID resolution */
	for (bIdx = 0; bIdx < 6; bIdx++) {
		if (((bOfst + 18 * bIdx) > 109) || (*(prData + bOfst + 18 * bIdx) == 0))
			break;

		ui2HActive =
		    (unsigned short)(*(prData + bOfst + 18 * bIdx + OFST_H_ACT_BLA_HI) & 0xf0) << 4;
		ui2HActive |= *(prData + bOfst + 18 * bIdx + OFST_H_ACTIVE_LO);
		ui2HBlanking =
		    (unsigned short)(*(prData + bOfst + 18 * bIdx + OFST_H_ACT_BLA_HI) & 0x0f) << 8;
		ui2HBlanking |= *(prData + bOfst + 18 * bIdx + OFST_H_BLANKING_LO);
		ui2VBlanking =
		    (unsigned short)(*(prData + bOfst + 18 * bIdx + OFST_V_ACTIVE_HI) & 0x0f) << 8;
		ui2VBlanking |= *(prData + bOfst + 18 * bIdx + OFST_V_BLANKING_LO);
		bTemp = (*(prData + bOfst + 18 * bIdx + OFST_FLAGS) & 0x80) >> 7;

		ui2PixClk = (unsigned short)*(prData+bOfst + 18 * bIdx+OFST_PXL_CLK_LO);
		ui2PixClk |= ((unsigned short)*(prData+bOfst + 18 * bIdx+OFST_PXL_CLK_HI)) << 8;
		ui2VActive = (unsigned short)*(prData+bOfst + 18 * bIdx+OFST_V_ACTIVE_LO);
		ui2VActive |= (unsigned short)(*(prData+bOfst + 18 * bIdx+OFST_V_ACTIVE_HI) & 0xf0) << 4;
		ui2PixClk = ui2PixClk * 10000;
		dHtotal = (ui2HActive + ui2HBlanking);
		dVtotal = (ui2VActive + ui2VBlanking);
		Vfiq = 1;
		if (((dHtotal * dVtotal) != 0) && (ui2PixClk != 0))
			Vfiq = ui2PixClk / (dHtotal * dVtotal);
		HDMI_EDID_LOG("clk=%d,h=%d,v=%d,vfiq=%d\n",
			ui2PixClk, dHtotal, dVtotal, Vfiq);
		vAnalyzeDTD(ui2HActive, ui2HBlanking, Vfiq, bTemp, FALSE);
	}

	if (*(prData + EXTEDID_ADDR_REVISION) >= 0x03) {	/* for simplay #7-37, #7-36 */

		prCEAaddr = prData + 4;
		vParserCEADataBlock(prCEAaddr, bOfst - 4);
	}

	TX_DEF_LOG("parsing EDID data ok\n");
	/* if go here, ie. parsing EDID data ok !! */
	return TRUE;
}

void vParserExtEDIDState(unsigned char *prEdid)
{
	unsigned char bTemp;
	unsigned char *prData;

	HDMI_EDID_FUNC();

	if (i4SharedInfo(SI_EDID_PARSING_RESULT) == TRUE) {
		/* parsing EDID extension block if it exist */
		for (bTemp = 0; bTemp < i4SharedInfo(SI_EDID_EXT_BLOCK_NO); bTemp++) {
			if ((EDID_BLOCK_LEN + bTemp * EDID_BLOCK_LEN) < EDID_SIZE) {	/* for Buffer Overflow error */
				if (*(prEdid + EDID_BLOCK_LEN + bTemp * EDID_BLOCK_LEN) == 0x02) {
					prData = (prEdid + EDID_BLOCK_LEN + bTemp * EDID_BLOCK_LEN);
					fgParserExtEDID(prData);
				} else if (*(prEdid + EDID_BLOCK_LEN + bTemp * EDID_BLOCK_LEN) ==
					   0xF0) {

				}
			} else {

			}
		}
	}
}

void hdmi_checkedid(unsigned char i1noedid)
{
	unsigned char bTemp;
	unsigned char bRetryCount = 10;
	unsigned char i;


	HDMI_EDID_FUNC();

	vClearEdidInfo();

	for (i = 0; i < 0x10; i++)
		_ui4First_16_VIC[i] = 0;

	_ui4First_16_NTSC_VIC = 0;
	_ui4First_16_PAL_VIC = 0;
	_u4i_3D_VIC = 0;
	_HdmiSinkAvCap.b_sink_hdmi_video_present = FALSE;
	_HdmiSinkAvCap.b_sink_3D_present = FALSE;
	_HdmiSinkAvCap.ui4_sink_cea_3D_resolution = 0;

	for (bTemp = 0; bTemp < bRetryCount; bTemp++) {
		if (hdmi_fgreadedid(i1noedid) == TRUE) {
			if (fgParserEDID(&_bEdidData[0]) == TRUE) {
				vSetSharedInfo(SI_EDID_PARSING_RESULT, TRUE);
				_HdmiSinkAvCap.b_sink_edid_ready = TRUE;

				break;
			}

			if (bTemp == bRetryCount - 1)
				vSetSharedInfo(SI_EDID_PARSING_RESULT, TRUE);

			if (bTemp == bRetryCount - 1)
				break;

		} else {

			if (bTemp == bRetryCount - 1) {
				if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_IN_ONLY)
					vClearEdidInfo();
				else if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) ==
					 HDMI_PLUG_IN_AND_SINK_POWER_ON)
					vSetEdidChkError();

				if (fgIsHdmiNoEDIDCheck())
					vSetNoEdidChkInfo();

				goto Done;
			}
		}
		/* msleep(5); */
		msleep(20);
	}

	if ((i4SharedInfo(SI_EDID_EXT_BLOCK_NO) * EDID_BLOCK_LEN) < EDID_SIZE)	/* for Buffer Overflow error */
		vParserExtEDIDState(&_bEdidData[0]);

	if (fgIsHdmiNoEDIDCheck())
		vSetNoEdidChkInfo();

Done:

	return;

}

unsigned char vCheckPcmBitSize(unsigned char ui1ChNumInx)
{
	unsigned char ui1Data, u1MaxBit;
	int i;

	u1MaxBit = PCM_16BIT;
	for (i = 6; i >= ui1ChNumInx; i--) {
		ui1Data = _HdmiSinkAvCap.ui1_sink_pcm_bit_size[i];

		if (ui1Data & (1 << PCM_24BIT)) {
			if (u1MaxBit < PCM_24BIT)
				u1MaxBit = PCM_24BIT;
		} else if (ui1Data & (1 << PCM_20BIT)) {
			if (u1MaxBit < PCM_20BIT)
				u1MaxBit = PCM_20BIT;
		}
	}

	return u1MaxBit;
}

void vShowEdidInformation(void)
{
	unsigned int u4Res = 0;
	unsigned char bInx = 0;
	unsigned int maxTMDSRate = 0;
	unsigned int yuv420Supported = 0;

	HDMI_EDID_FUNC();

	HDMI_PLUG_LOG("EDID ver:%d/rev:%d\n", _HdmiSinkAvCap.ui1_Edid_Version,
		      _HdmiSinkAvCap.ui1_Edid_Revision);
	HDMI_PLUG_LOG("EDID Extend Rev:%d\n", _HdmiSinkAvCap.ui1_ExtEdid_Revision);
	if (_HdmiSinkAvCap.b_sink_support_hdmi_mode)
		HDMI_PLUG_LOG("SINK Device is HDMI\n");
	else
		HDMI_PLUG_LOG("SINK Device is DVI\n");

	if (_HdmiSinkAvCap.b_sink_support_hdmi_mode)
		HDMI_PLUG_LOG("CEC ADDRESS:%x\n", _HdmiSinkAvCap.ui2_sink_cec_address);

	HDMI_PLUG_LOG("max clock limit : %d\n", _HdmiSinkAvCap.ui1_sink_max_tmds_clock);

	u4Res = (_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution |
		 _HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution |
		 _HdmiSinkAvCap.ui4_sink_cea_pal_resolution |
		 _HdmiSinkAvCap.ui4_sink_dtd_pal_resolution);

	if (u4Res & SINK_480I)
		HDMI_PLUG_LOG("SUPPORT 1440x480I 59.94hz\n");
	if (u4Res & SINK_480I_1440)
		HDMI_PLUG_LOG("SUPPORT 2880x480I 59.94hz\n");
	if (u4Res & SINK_480P)
		HDMI_PLUG_LOG("SUPPORT 720x480P 59.94hz\n");
	if (u4Res & SINK_480P_1440)
		HDMI_PLUG_LOG("SUPPORT 1440x480P 59.94hz\n");
	if (u4Res & SINK_480P_2880)
		HDMI_PLUG_LOG("SUPPORT 2880x480P 59.94hz\n");
	if (u4Res & SINK_720P60)
		HDMI_PLUG_LOG("SUPPORT 1280x720P 59.94hz\n");
	if (u4Res & SINK_1080I60)
		HDMI_PLUG_LOG("SUPPORT 1920x1080I 59.94hz\n");
	if (u4Res & SINK_1080P60)
		HDMI_PLUG_LOG("SUPPORT 1920x1080P 59.94hz\n");

	if (u4Res & SINK_576I)
		HDMI_PLUG_LOG("SUPPORT 1440x576I 50hz\n");
	if (u4Res & SINK_576I_1440)
		HDMI_PLUG_LOG("SUPPORT 2880x576I 50hz\n");
	if (u4Res & SINK_576P)
		HDMI_PLUG_LOG("SUPPORT 720x576P 50hz\n");
	if (u4Res & SINK_576P_1440)
		HDMI_PLUG_LOG("SUPPORT 1440x576P 50hz\n");
	if (u4Res & SINK_576P_2880)
		HDMI_PLUG_LOG("SUPPORT 2880x576P 50hz\n");
	if (u4Res & SINK_720P50)
		HDMI_PLUG_LOG("SUPPORT 1280x720P 50hz\n");
	if (u4Res & SINK_1080I50)
		HDMI_PLUG_LOG("SUPPORT 1920x1080I 50hz\n");
	if (u4Res & SINK_1080P50)
		HDMI_PLUG_LOG("SUPPORT 1920x1080P 50hz\n");
	if (u4Res & SINK_1080P30)
		HDMI_PLUG_LOG("SUPPORT 1920x1080P 30hz\n");
	if (u4Res & SINK_1080P24)
		HDMI_PLUG_LOG("SUPPORT 1920x1080P 24hz\n");
	if (u4Res & SINK_1080P25)
		HDMI_PLUG_LOG("SUPPORT 1920x1080P 25hz\n");

	u4Res =
	    (_HdmiSinkAvCap.ui4_sink_native_ntsc_resolution | _HdmiSinkAvCap.
	     ui4_sink_native_pal_resolution);
	HDMI_PLUG_LOG("NTSC Native =%x\n", _HdmiSinkAvCap.ui4_sink_native_ntsc_resolution);
	HDMI_PLUG_LOG("PAL Native =%x\n", _HdmiSinkAvCap.ui4_sink_native_pal_resolution);
	if (u4Res & SINK_480I)
		HDMI_PLUG_LOG("Native resolution is 1440x480I 59.94hz\n");
	if (u4Res & SINK_480I_1440)
		HDMI_PLUG_LOG("Native resolution is 2880x480I 59.94hz\n");
	if (u4Res & SINK_480P)
		HDMI_PLUG_LOG("Native resolution is 720x480P 59.94hz\n");
	if (u4Res & SINK_480P_1440)
		HDMI_PLUG_LOG("Native resolution is 1440x480P 59.94hz\n");
	if (u4Res & SINK_480P_2880)
		HDMI_PLUG_LOG("Native resolution is 2880x480P 59.94hz\n");
	if (u4Res & SINK_720P60)
		HDMI_PLUG_LOG("Native resolution is 1280x720P 59.94hz\n");
	if (u4Res & SINK_1080I60)
		HDMI_PLUG_LOG("Native resolution is 1920x1080I 59.94hz\n");
	if (u4Res & SINK_1080P60)
		HDMI_PLUG_LOG("Native resolution is 1920x1080P 59.94hz\n");
	if (u4Res & SINK_576I)
		HDMI_PLUG_LOG("Native resolution is 1440x576I 50hz\n");
	if (u4Res & SINK_576I_1440)
		HDMI_PLUG_LOG("Native resolution is 2880x576I 50hz\n");
	if (u4Res & SINK_576P)
		HDMI_PLUG_LOG("Native resolution is 720x576P 50hz\n");
	if (u4Res & SINK_576P_1440)
		HDMI_PLUG_LOG("Native resolution is 1440x576P 50hz\n");
	if (u4Res & SINK_576P_2880)
		HDMI_PLUG_LOG("Native resolution is 2880x576P 50hz\n");
	if (u4Res & SINK_720P50)
		HDMI_PLUG_LOG("Native resolution is 1280x720P 50hz\n");
	if (u4Res & SINK_1080I50)
		HDMI_PLUG_LOG("Native resolution is 1920x1080I 50hz\n");
	if (u4Res & SINK_1080P50)
		HDMI_PLUG_LOG("Native resolution is 1920x1080P 50hz\n");
	if (u4Res & SINK_1080P30)
		HDMI_PLUG_LOG("Native resolution is 1920x1080P 30hz\n");
	if (u4Res & SINK_1080P24)
		HDMI_PLUG_LOG("Native resolution is 1920x1080P 24hz\n");
	if (u4Res & SINK_1080P25)
		HDMI_PLUG_LOG("Native resolution is 1920x1080P 25hz\n");

	u4Res = _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic | _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb;
	if (u4Res & SINK_2160P_23_976HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2160P_23_976HZ\n");
	if (u4Res & SINK_2160P_24HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2160P_24HZ\n");
	if (u4Res & SINK_2160P_25HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2160P_25HZ\n");
	if (u4Res & SINK_2160P_29_97HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2160P_29_97HZ\n");
	if (u4Res & SINK_2160P_30HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2160P_30HZ\n");
	if (u4Res & SINK_2161P_24HZ)
		HDMI_PLUG_LOG("4k2k support to SINK_2161P_24HZ\n");
	/* For HDMI2 modes, TMDS character rate should be >= 340 atleast or >=297 for 420modes. However,
	   some HDMI1.x AVRs when providing the combined EDID for the AVR and sink incorrectly report
	   support for HDMI2.0 modes indicating 4k@60/50 modes as available. Check TMDS character rate
	   for the same. If it is not set or < 340, support for 4k@60/50 is not there and EDID is
	   incorrectly indicating 4k@60/50 support. Remove 4k@60/50 support for such cases. */
	if (_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate != 0) {
		maxTMDSRate = _HdmiSinkAvCap.ui2_sink_max_tmds_character_rate;
	} else {
		maxTMDSRate = _HdmiSinkAvCap.ui1_sink_max_tmds_clock;
	}

	yuv420Supported = ((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420) ||
					   (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420_CAPABILITY)) ? 1 : 0;
	if (((yuv420Supported) && (maxTMDSRate >= 297)) || (maxTMDSRate >= 340)/* HDMI2 modes*/) {
		if (u4Res & SINK_2160P_50HZ)
			HDMI_PLUG_LOG("4k2k support to SINK_2160P_50HZ\n");
		if (u4Res & SINK_2160P_60HZ)
			HDMI_PLUG_LOG("4k2k support to SINK_2160P_60HZ\n");
		if (u4Res & SINK_2161P_50HZ)
			HDMI_PLUG_LOG("4k2k support to SINK_2161P_50HZ\n");
		if (u4Res & SINK_2161P_60HZ)
			HDMI_PLUG_LOG("4k2k support to SINK_2161P_60HZ\n");
	}

	HDMI_PLUG_LOG("SUPPORT RGB\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_444)
		HDMI_PLUG_LOG("SUPPORT YCBCR 444\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_422)
		HDMI_PLUG_LOG("SUPPORT YCBCR 422\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420)
		HDMI_PLUG_LOG("SUPPORT YCBCR ONLY 420, 0x%x\n",
			      _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb);
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420_CAPABILITY)
		HDMI_PLUG_LOG("SUPPORT YCBCR 420 and other colorspace, 0x%x\n",
			      _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb);
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC709)
		HDMI_PLUG_LOG("SUPPORT xvYCC 709\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC601)
		HDMI_PLUG_LOG("SUPPORT xvYCC 601\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_S_YCC601)
		HDMI_PLUG_LOG("SINK_S_YCC601\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_ADOBE_YCC601)
		HDMI_PLUG_LOG("SINK_ADOBE_YCC601\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_ADOBE_RGB)
		HDMI_PLUG_LOG("SINK_ADOBE_RGB\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_CYCC)
		HDMI_PLUG_LOG("SINK_COLOR_SPACE_BT2020_CYCC\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_YCC)
		HDMI_PLUG_LOG("SINK_COLOR_SPACE_BT2020_YCC\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_COLOR_SPACE_BT2020_RGB)
		HDMI_PLUG_LOG("SINK_COLOR_SPACE_BT2020_RGB\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA0)
		HDMI_PLUG_LOG("SUPPORT metadata P0\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA1)
		HDMI_PLUG_LOG("SUPPORT metadata P1\n");
	if (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA2)
		HDMI_PLUG_LOG("SUPPORT metadata P2\n");


	if (_HdmiSinkAvCap.e_sink_ycbcr_color_bit & HDMI_SINK_DEEP_COLOR_10_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR 30 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_ycbcr_color_bit & HDMI_SINK_DEEP_COLOR_12_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR 36 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_ycbcr_color_bit & HDMI_SINK_DEEP_COLOR_16_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR 48 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_ycbcr_color_bit == HDMI_SINK_NO_DEEP_COLOR)
		HDMI_PLUG_LOG("Not SUPPORT YCBCR Deep Color\n");

	if (_HdmiSinkAvCap.ui1_sink_dc420_color_bit & HDMI_SINK_DEEP_COLOR_16_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR420 48 Bits Deep Color\n");
	if (_HdmiSinkAvCap.ui1_sink_dc420_color_bit & HDMI_SINK_DEEP_COLOR_12_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR420 36 Bits Deep Color\n");
	if (_HdmiSinkAvCap.ui1_sink_dc420_color_bit & HDMI_SINK_DEEP_COLOR_10_BIT)
		HDMI_PLUG_LOG("SUPPORT YCBCR420 30 Bits Deep Color\n");
	if (_HdmiSinkAvCap.ui1_sink_dc420_color_bit == HDMI_SINK_NO_DEEP_COLOR)
		HDMI_PLUG_LOG("Not SUPPORT YCBCR420 Deep Color\n");

	if (_HdmiSinkAvCap.e_sink_rgb_color_bit & HDMI_SINK_DEEP_COLOR_10_BIT)
		HDMI_PLUG_LOG("SUPPORT RGB 30 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_rgb_color_bit & HDMI_SINK_DEEP_COLOR_12_BIT)
		HDMI_PLUG_LOG("SUPPORT RGB 36 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_rgb_color_bit & HDMI_SINK_DEEP_COLOR_16_BIT)
		HDMI_PLUG_LOG("SUPPORT RGB 48 Bits Deep Color\n");
	if (_HdmiSinkAvCap.e_sink_rgb_color_bit == HDMI_SINK_NO_DEEP_COLOR)
		HDMI_PLUG_LOG("Not SUPPORT RGB Deep Color\n");

	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_LPCM)
		HDMI_PLUG_LOG("SUPPORT LPCM\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_AC3)
		HDMI_PLUG_LOG("SUPPORT AC3 Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_MPEG1)
		HDMI_PLUG_LOG("SUPPORT MPEG1 Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_MP3)
		HDMI_PLUG_LOG("SUPPORT AC3 Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_MPEG2)
		HDMI_PLUG_LOG("SUPPORT MPEG2 Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_AAC)
		HDMI_PLUG_LOG("SUPPORT AAC Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_DTS)
		HDMI_PLUG_LOG("SUPPORT DTS Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_ATRAC)
		HDMI_PLUG_LOG("SUPPORT ATRAC Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_DSD)
		HDMI_PLUG_LOG("SUPPORT SACD DSD Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_DOLBY_PLUS)
		HDMI_PLUG_LOG("SUPPORT Dolby Plus Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_DTS_HD)
		HDMI_PLUG_LOG("SUPPORT DTS HD Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_MAT_MLP)
		HDMI_PLUG_LOG("SUPPORT MAT MLP Decode\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_DST)
		HDMI_PLUG_LOG("SUPPORT SACD DST Decode\n");
	if ((_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_ATMOS) || (_HdmiSinkAvCap.ui1_sink_support_dolby_atoms == TRUE))
		HDMI_PLUG_LOG("SUPPORT Dolby ATMOS\n");
	if (_HdmiSinkAvCap.ui2_sink_aud_dec & HDMI_SINK_AUDIO_DEC_WMA)
		HDMI_PLUG_LOG("SUPPORT  WMA Decode\n");

	if (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[0] != 0) {

		for (bInx = 0; bInx < 50; bInx++) {
			memcpy(&cDstStr[0 + bInx], " ", 1);
			if (bInx < 21)
				memcpy(&cDstBitStr[0 + bInx], " ", 1);
		}

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[0] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		for (bInx = 0; bInx < 3; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_bit_size[0] >> bInx) & 0x01)
				memcpy(&cDstBitStr[0 + bInx * 7], &_cBitdeepStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT PCM Max 2CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[4] != 0) {

		for (bInx = 0; bInx < 50; bInx++) {
			memcpy(&cDstStr[0 + bInx], " ", 1);
			if (bInx < 21)
				memcpy(&cDstBitStr[0 + bInx], " ", 1);
		}

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[4] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		for (bInx = 0; bInx < 3; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_bit_size[4] >> bInx) & 0x01)
				memcpy(&cDstBitStr[0 + bInx * 7], &_cBitdeepStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT PCM Max 6CH, Fs is: %s; bitdeep is: %s\n", &cDstStr[0], &cDstBitStr[0]);
	}

	if (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[5] != 0) {

		for (bInx = 0; bInx < 50; bInx++) {
			memcpy(&cDstStr[0 + bInx], " ", 1);
			if (bInx < 21)
				memcpy(&cDstBitStr[0 + bInx], " ", 1);
		}

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[5] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		for (bInx = 0; bInx < 3; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_bit_size[5] >> bInx) & 0x01)
				memcpy(&cDstBitStr[0 + bInx * 7], &_cBitdeepStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT PCM Max 7CH, Fs is: %s; bitdeep is: %s\n", &cDstStr[0], &cDstBitStr[0]);
	}

	if (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[6] != 0) {

		for (bInx = 0; bInx < 50; bInx++) {
			memcpy(&cDstStr[0 + bInx], " ", 1);
			if (bInx < 21)
				memcpy(&cDstBitStr[0 + bInx], " ", 1);
		}

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[6] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		for (bInx = 0; bInx < 3; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_pcm_bit_size[6] >> bInx) & 0x01)
				memcpy(&cDstBitStr[0 + bInx * 7], &_cBitdeepStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT PCM Max 8CH, FS is: %s; bitdeep is: %s\n", &cDstStr[0], &cDstBitStr[0]);
	}

	if (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[0] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);


		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[0] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT AC3 Max 2CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[4] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[4] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT AC3 Max 6CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[5] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[5] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT AC3 Max 7CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[6] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[6] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT AC3 Max 8CH, FS is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[0] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);


		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[0] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT E-AC3 Max 2CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[4] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[4] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT E-AC3 Max 6CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[5] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[5] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT E-AC3 Max 7CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[6] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[6] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("SUPPORT E-AC3 Max 8CH, FS is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[0] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);


		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[0] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT DTS Max 2CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[4] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[4] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT dts Max 6CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[5] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[5] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT dts Max 7CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[6] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[6] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT dts Max 8CH, FS is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[0] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);


		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[0] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT DTS HD Max 2CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[4] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[4] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT DTS HD Max 6CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[5] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[5] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT DTS HD Max 7CH, Fs is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[6] != 0) {

		for (bInx = 0; bInx < 50; bInx++)
			memcpy(&cDstStr[0 + bInx], " ", 1);

		for (bInx = 0; bInx < 7; bInx++) {
			if ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[6] >> bInx) & 0x01)
				memcpy(&cDstStr[0 + bInx * 7], &_cFsStr[bInx][0], 7);
		}
		HDMI_PLUG_LOG("[HDMI]SUPPORT DTS HD Max 8CH, FS is: %s\n", &cDstStr[0]);

	}

	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_FL_FR)
		HDMI_PLUG_LOG("Speaker FL/FR allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_LFE)
		HDMI_PLUG_LOG("Speaker LFE allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_FC)
		HDMI_PLUG_LOG("Speaker FC allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_RL_RR)
		HDMI_PLUG_LOG("Speaker RL/RR allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_RC)
		HDMI_PLUG_LOG("Speaker RC allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_FLC_FRC)
		HDMI_PLUG_LOG("Speaker FLC/FRC allocated\n");
	if (_HdmiSinkAvCap.ui1_sink_spk_allocation & SINK_AUDIO_RLC_RRC)
		HDMI_PLUG_LOG("Speaker RLC/RRC allocated\n");

	HDMI_PLUG_LOG("HDMI edid support content type =%x\n",
		      _HdmiSinkAvCap.ui1_sink_content_cnc);
	HDMI_PLUG_LOG("Lip Sync Progressive audio latency = %d\n",
		      _HdmiSinkAvCap.ui1_sink_p_audio_latency);
	HDMI_PLUG_LOG("Lip Sync Progressive video latency = %d\n",
		      _HdmiSinkAvCap.ui1_sink_p_video_latency);
	if (_HdmiSinkAvCap.ui1_sink_i_latency_present) {
		HDMI_PLUG_LOG("Lip Sync Interlace audio latency = %d\n",
			      _HdmiSinkAvCap.ui1_sink_i_audio_latency);
		HDMI_PLUG_LOG("Lip Sync Interlace video latency = %d\n",
			      _HdmiSinkAvCap.ui1_sink_i_video_latency);
	}

	if (_HdmiSinkAvCap.ui1_sink_support_ai == 1)
		HDMI_PLUG_LOG("Support AI\n");
	else
		HDMI_PLUG_LOG("Not Support AI\n");

	HDMI_PLUG_LOG("Monitor Max horizontal size = %d\n",
		      _HdmiSinkAvCap.ui1_Display_Horizontal_Size);
	HDMI_PLUG_LOG("Monitor Max vertical size = %d\n",
		      _HdmiSinkAvCap.ui1_Display_Vertical_Size);


	if (_HdmiSinkAvCap.b_sink_hdmi_video_present == TRUE)
		HDMI_PLUG_LOG("HDMI_Video_Present\n");
	else
		HDMI_PLUG_LOG("No HDMI_Video_Present\n");

	if (_HdmiSinkAvCap.b_sink_3D_present == TRUE)
		HDMI_PLUG_LOG("3D_present\n");
	else
		HDMI_PLUG_LOG("No 3D_present\n");

	if (_HdmiSinkAvCap.b_sink_SCDC_present == TRUE)
		HDMI_PLUG_LOG("HDMI_SCDC_Present\n");

	if (_HdmiSinkAvCap.b_sink_LTE_340M_sramble == TRUE)
		HDMI_PLUG_LOG("LTE_340M_sramble\n");

	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_SCDC_PRESENT)
		HDMI_PLUG_LOG("EDID_HF_VSDB_SCDC_PRESENT EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_RR_CAPABLE)
		HDMI_PLUG_LOG("EDID_HF_VSDB_RR_CAPABLE EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_LTE_340MCSC_SCRAMBLE)
		HDMI_PLUG_LOG("EDID_HF_VSDB_LTE_340MCSC_SCRAMBLE EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_INDEPENDENT_VIEW)
		HDMI_PLUG_LOG("EDID_HF_VSDB_INDEPENDENT_VIEW EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_DUAL_VIEW)
		HDMI_PLUG_LOG("EDID_HF_VSDB_DUAL_VIEW EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_hf_vsdb_info & EDID_HF_VSDB_3D_OSD_DISPARITY)
		HDMI_PLUG_LOG("EDID_HF_VSDB_3D_OSD_DISPARITY EXIST\n");
	if (_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_PHILIPS_HDR)
		HDMI_PLUG_LOG("EDID_SUPPORT_PHILIPS_HDR\n");
	if (_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR)
		HDMI_PLUG_LOG("EDID_SUPPORT_DOLBY_HDR(Dolby HDR Enable Bit)\n");
	if (_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_YUV422_12BIT)
		HDMI_PLUG_LOG("EDID_SUPPORT_YUV422_12BIT\n");
	if (_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR_2160P60)
		HDMI_PLUG_LOG("EDID_SUPPORT_DOLBY_HDR_2160P60\n");
	if (!(_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR_2160P60) &&
		(_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_DOLBY_HDR))
		HDMI_PLUG_LOG("[HDMI TX]Dolby enable and Dolby 2160P60 disable, max Dolby 2160P30\n");
	if (_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr & EDID_SUPPORT_HDR10_PLUS)
		HDMI_PLUG_LOG("EDID_SUPPORT_HDR10_PLUS\n");
	if (_HdmiSinkAvCap.ui1_sink_hdr10plus_app_version)
		HDMI_PLUG_LOG("ui1_sink_hdr10plus_app_version=0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hdr10plus_app_version);
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_SDR)
		HDMI_PLUG_LOG("EDID_SUPPORT_SDR\n");
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_HDR)
		HDMI_PLUG_LOG("EDID_SUPPORT_HDR\n");
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_SMPTE_ST_2084)
		HDMI_PLUG_LOG("EDID_SUPPORT_SMPTE_ST_2084(HDR10 Enable Bit)\n");
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_FUTURE_EOTF)
		HDMI_PLUG_LOG("EDID_SUPPORT_FUTURE_EOTF\n");
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_ET_4)
		HDMI_PLUG_LOG("EDID_SUPPORT_ET_4\n");
	if (_HdmiSinkAvCap.ui1_sink_support_static_hdr & EDID_SUPPORT_ET_5)
		HDMI_PLUG_LOG("EDID_SUPPORT_ET_5\n");

	HDMI_PLUG_LOG("ui1_sink_hdr_content_max_luminance = 0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hdr_content_max_luminance);
	HDMI_PLUG_LOG("ui1_sink_hdr_content_max_frame_average_luminance = 0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hdr_content_max_frame_average_luminance);
	HDMI_PLUG_LOG("ui1_sink_hdr_content_min_luminance = 0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hdr_content_min_luminance);

	HDMI_PLUG_LOG("HDR block=0x%x;0x%x;0x%x;0x%x;0x%x;0x%x;0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hdr_block[0], _HdmiSinkAvCap.ui1_sink_hdr_block[1],
		_HdmiSinkAvCap.ui1_sink_hdr_block[2], _HdmiSinkAvCap.ui1_sink_hdr_block[3],
		_HdmiSinkAvCap.ui1_sink_hdr_block[4], _HdmiSinkAvCap.ui1_sink_hdr_block[5],
		_HdmiSinkAvCap.ui1_sink_hdr_block[6]);
	HDMI_PLUG_LOG("HFVSDB block=0x%x;0x%x;0x%x;0x%x;0x%x;0x%x;0x%x;0x%x\n",
		_HdmiSinkAvCap.ui1_sink_hfvsdb_block[0], _HdmiSinkAvCap.ui1_sink_hfvsdb_block[1],
		_HdmiSinkAvCap.ui1_sink_hfvsdb_block[2], _HdmiSinkAvCap.ui1_sink_hfvsdb_block[3],
		_HdmiSinkAvCap.ui1_sink_hfvsdb_block[4], _HdmiSinkAvCap.ui1_sink_hfvsdb_block[5],
		_HdmiSinkAvCap.ui1_sink_hfvsdb_block[6], _HdmiSinkAvCap.ui1_sink_hfvsdb_block[7]);

	HDMI_PLUG_LOG("[HDMI TX]ui2_sink_max_tmds_character_rate = 0x%x\n",
		_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate);
	HDMI_PLUG_LOG("[HDMI TX]ui1_sink_hf_vsdb_info = 0x%x\n", _HdmiSinkAvCap.ui1_sink_hf_vsdb_info);
}

long long hdmi_DispGetEdidInfo(void)
{
	long long u4Resolution = 0x0;
	long long u4Res = 0x0;
	unsigned int maxTMDSRate = 0;
	unsigned int yuv420Supported = 0;

	HDMI_EDID_FUNC();

	u4Res = (_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution |
		 _HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution |
		 _HdmiSinkAvCap.ui4_sink_cea_pal_resolution |
		 _HdmiSinkAvCap.ui4_sink_dtd_pal_resolution);

	if(u4Res & (SINK_480I | SINK_480I_1440 | SINK_480I_2880 | SINK_480I_4_3))
		u4Resolution |= RES_480i_60;
	if(u4Res & (SINK_576I | SINK_576I_1440 | SINK_576I_2880 | SINK_576I_4_3))
		u4Resolution |= RES_576i_50;
	if(u4Res & (SINK_576I | SINK_576I_1440 | SINK_576I_2880 | SINK_576I_4_3))
		u4Resolution |= RES_576i_50;
	if(u4Res & (SINK_480P | SINK_480P_1440 | SINK_480P_2880 | SINK_480P_2880_4_3 | SINK_480P_4_3))
		u4Resolution |= RES_480p_60;
	if(u4Res & (SINK_576P | SINK_576P_1440 | SINK_576P_2880 | SINK_576P_2880_4_3 | SINK_576P_4_3))
		u4Resolution |= RES_576p_50;
	if(u4Res & SINK_720P60)
		u4Resolution |= (RES_720p_60 | RES_720P_59_94);
	if(u4Res & SINK_720P50)
		u4Resolution |= RES_720p_50;
	if(u4Res & SINK_1080I60)
		u4Resolution |= RES_1080i_60;
	if(u4Res & SINK_1080I50)
		u4Resolution |= RES_1080i_50;
	if(u4Res & SINK_1080P30)
		u4Resolution |= RES_1080p_30;
	if (u4Res & SINK_1080P2997)
		u4Resolution |= RES_1080p_29;
	if (u4Res & SINK_1080P25)
		u4Resolution |= RES_1080p_25;
	if(u4Res & SINK_1080P24)
		u4Resolution |= RES_1080p_24;
	if (u4Res & SINK_1080P23976)
		u4Resolution |= RES_1080p_23;
	if (u4Res & SINK_1080P60)
		u4Resolution |= (RES_1080p_60 | RES_1080P_59_94);
	if(u4Res & SINK_1080P50)
		u4Resolution |= RES_1080p_50;

	TX_DEF_LOG("hdmi_DispGetEdidInfo u4Res = 0x%llx\n", u4Res);

	u4Res = _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic | _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb;

	if(u4Res & SINK_2160P_23_976HZ)
		u4Resolution |= RES_2160P_23_976;
	if(u4Res & SINK_2160P_24HZ)
		u4Resolution |= RES_2160P_24;
	if(u4Res & SINK_2160P_25HZ)
		u4Resolution |= RES_2160P_25;
	if(u4Res & SINK_2160P_29_97HZ)
		u4Resolution |= RES_2160P_29_97;
	if(u4Res & SINK_2160P_30HZ)
		u4Resolution |= RES_2160P_30;
	if(u4Res & SINK_2161P_24HZ)
		u4Resolution |= RES_2161P_24;

	/* For HDMI2 modes, TMDS character rate should be >= 340 atleast or >=297 for 420modes. However,
	   some HDMI1.x AVRs when providing the combined EDID for the AVR and sink incorrectly report
	   support for HDMI2.0 modes indicating 4k@60/50 modes as available. Check TMDS character rate
	   for the same. If it is not set or < 340, support for 4k@60/50 is not there and EDID is
	   incorrectly indicating 4k@60/50 support. Remove 4k@60/50 support for such cases. */
	if (_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate != 0) {
		maxTMDSRate = _HdmiSinkAvCap.ui2_sink_max_tmds_character_rate;
	} else {
		maxTMDSRate = _HdmiSinkAvCap.ui1_sink_max_tmds_clock;
	}

	yuv420Supported = ((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420) ||
					   (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420_CAPABILITY)) ? 1 : 0;
	if (((yuv420Supported) && (maxTMDSRate >= 297)) || (maxTMDSRate >= 340)/* HDMI2 modes*/) {
		if(u4Res & SINK_2160P_60HZ)
			u4Resolution |= (RES_2160P_60 | RES_2160P_59_94);
		if(u4Res & SINK_2160P_50HZ)
			u4Resolution |= RES_2160P_50;
		if(u4Res & SINK_2161P_60HZ)
			u4Resolution |= (RES_2161P_60 | RES_2161P_59_94);
		if(u4Res & SINK_2161P_50HZ)
			u4Resolution |= RES_2161P_50;
	} else {
		TX_DEF_LOG("hdmi_DispGetEdidInfo: 4k@60/50 modes are not supported");
	}
	return u4Resolution;
}

bool isThomsonUD9_India_TV(void) {
	/* THOMSON UD9 43TH6000 TV (India only TV) has Dolby audio playback issue.
	 * black list Dolby audio capability of this TV based on Manufacturer ID
	 * and Manufacturer product code.
	 * 00  ff  ff  ff  ff  ff  ff  00  0e  96  03  b1  01  00  00  00 */
	if ((_bEdidData[0x08] == 0x0e) && (_bEdidData[0x09] ==  0x96) &&
	    (_bEdidData[0x0a] ==  0x03) && (_bEdidData[0x0b] ==  0xb1))
		return true;

	return false;
}

void hdmi_AppGetEdidInfo(HDMI_EDID_T *pv_get_info)
{
	unsigned int i;
	long long u4Res = 0x0;
	unsigned int maxTMDSRate = 0;
	unsigned int yuv420Supported = 0;

	HDMI_EDID_FUNC();

	pv_get_info->ui4_ntsc_resolution =
	    (_HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution | _HdmiSinkAvCap.
	     ui4_sink_dtd_ntsc_resolution);
	pv_get_info->ui4_pal_resolution =
	    (_HdmiSinkAvCap.ui4_sink_cea_pal_resolution | _HdmiSinkAvCap.
	     ui4_sink_dtd_pal_resolution);
	pv_get_info->ui4_sink_native_ntsc_resolution =
	    _HdmiSinkAvCap.ui4_sink_native_ntsc_resolution;
	pv_get_info->ui4_sink_native_pal_resolution = _HdmiSinkAvCap.ui4_sink_native_pal_resolution;
	pv_get_info->ui4_sink_cea_ntsc_resolution = _HdmiSinkAvCap.ui4_sink_cea_ntsc_resolution;
	pv_get_info->ui4_sink_cea_pal_resolution = _HdmiSinkAvCap.ui4_sink_cea_pal_resolution;
	pv_get_info->ui4_sink_dtd_ntsc_resolution = _HdmiSinkAvCap.ui4_sink_dtd_ntsc_resolution;
	pv_get_info->ui4_sink_dtd_pal_resolution = _HdmiSinkAvCap.ui4_sink_dtd_pal_resolution;
	pv_get_info->ui4_sink_1st_dtd_ntsc_resolution =
	    _HdmiSinkAvCap.ui4_sink_1st_dtd_ntsc_resolution;
	pv_get_info->ui4_sink_1st_dtd_pal_resolution =
	    _HdmiSinkAvCap.ui4_sink_1st_dtd_pal_resolution;
	pv_get_info->ui2_sink_colorimetry = _HdmiSinkAvCap.ui2_sink_colorimetry;
	pv_get_info->ui1_sink_rgb_color_bit = _HdmiSinkAvCap.e_sink_rgb_color_bit;
	pv_get_info->ui1_sink_ycbcr_color_bit = _HdmiSinkAvCap.e_sink_ycbcr_color_bit;
	pv_get_info->ui1_sink_dc420_color_bit = _HdmiSinkAvCap.ui1_sink_dc420_color_bit;
	pv_get_info->ui2_sink_aud_dec = _HdmiSinkAvCap.ui2_sink_aud_dec | HDMI_SINK_AUDIO_DEC_LPCM;
	pv_get_info->ui1_sink_is_plug_in = _stAvdAVInfo.b_hotplug_state;
	pv_get_info->ui4_hdmi_pcm_ch_type =
	    ((_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[0]) |
	     (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[4] << 8) | (_HdmiSinkAvCap.
								  ui1_sink_pcm_ch_sampling[6] <<
								  16));
	pv_get_info->ui4_hdmi_pcm_ch3ch4ch5ch7_type =
	    ((_HdmiSinkAvCap.
	      ui1_sink_pcm_ch_sampling[1]) | (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[2] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[3] << 16)
	     | (_HdmiSinkAvCap.ui1_sink_pcm_ch_sampling[5] << 24));

	pv_get_info->ui4_hdmi_ac3_ch_type =
	    ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[0]) |
	     (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[4] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[6] << 16));
	pv_get_info->ui4_hdmi_ac3_ch3ch4ch5ch7_type =
	    ((_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[1]) |
	     (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[2] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[3] << 16) |
	     (_HdmiSinkAvCap.ui1_sink_ac3_ch_sampling[5] << 24));

	pv_get_info->ui4_hdmi_ec3_ch_type =
	    ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[0]) |
	     (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[4] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[6] << 16));
	pv_get_info->ui4_hdmi_ec3_ch3ch4ch5ch7_type =
	    ((_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[1]) |
	     (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[2] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[3] << 16) |
	     (_HdmiSinkAvCap.ui1_sink_ec3_ch_sampling[5] << 24));

	pv_get_info->ui4_hdmi_dts_ch_type =
	    ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[0]) |
	     (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[4] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[6] << 16));
	pv_get_info->ui4_hdmi_dts_ch3ch4ch5ch7_type =
	    ((_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[1]) |
	     (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[2] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[3] << 16) |
	     (_HdmiSinkAvCap.ui1_sink_dts_ch_sampling[5] << 24));

	pv_get_info->ui4_hdmi_dts_hd_ch_type =
	    ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[0]) |
	     (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[4] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[6] << 16));
	pv_get_info->ui4_hdmi_dts_hd_ch3ch4ch5ch7_type =
	    ((_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[1]) |
	     (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[2] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[3] << 16) |
	     (_HdmiSinkAvCap.ui1_sink_dts_hd_ch_sampling[5] << 24));

	pv_get_info->ui4_hdmi_pcm_bit_size =
	    ((_HdmiSinkAvCap.ui1_sink_pcm_bit_size[0]) |
	     (_HdmiSinkAvCap.ui1_sink_pcm_bit_size[4] << 8) |
	     (_HdmiSinkAvCap.ui1_sink_pcm_bit_size[6] << 16));

	pv_get_info->ui1_sink_support_dolby_atoms = _HdmiSinkAvCap.ui1_sink_support_dolby_atoms;
	pv_get_info->ui1_sink_i_latency_present = _HdmiSinkAvCap.ui1_sink_i_latency_present;
	pv_get_info->ui1_sink_p_audio_latency = _HdmiSinkAvCap.ui1_sink_p_audio_latency;
	pv_get_info->ui1_sink_p_video_latency = _HdmiSinkAvCap.ui1_sink_p_video_latency;
	pv_get_info->ui1_sink_i_audio_latency = _HdmiSinkAvCap.ui1_sink_i_audio_latency;
	pv_get_info->ui1_sink_i_video_latency = _HdmiSinkAvCap.ui1_sink_i_video_latency;

	pv_get_info->ui1ExtEdid_Revision = _HdmiSinkAvCap.ui1_ExtEdid_Revision;
	pv_get_info->ui1Edid_Version = _HdmiSinkAvCap.ui1_Edid_Version;
	pv_get_info->ui1Edid_Revision = _HdmiSinkAvCap.ui1_Edid_Revision;
	pv_get_info->ui1_Display_Horizontal_Size = _HdmiSinkAvCap.ui1_Display_Horizontal_Size;
	pv_get_info->ui1_Display_Vertical_Size = _HdmiSinkAvCap.ui1_Display_Vertical_Size;
	pv_get_info->ui2_sink_cec_address = _HdmiSinkAvCap.ui2_sink_cec_address;
	HDMI_EDID_LOG("[edid]to app,ntsc:%x,pal:%x,pa:%x\n", pv_get_info->ui4_ntsc_resolution,
		      pv_get_info->ui4_pal_resolution, pv_get_info->ui2_sink_cec_address);
	HDMI_EDID_LOG("[edid]to app,aud dec:%x,pcm ch:%x\n", pv_get_info->ui2_sink_aud_dec,
		      pv_get_info->ui4_hdmi_pcm_ch_type);

	pv_get_info->b_sink_SCDC_present = _HdmiSinkAvCap.b_sink_SCDC_present;
	pv_get_info->b_sink_LTE_340M_sramble = _HdmiSinkAvCap.b_sink_LTE_340M_sramble;

	u4Res = _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic | _HdmiSinkAvCap.ui4_sink_hdmi_4k2kvic_420_vdb;

	/* For HDMI2 modes, TMDS character rate should be >= 340 atleast or >=297 for 420modes. However,
	   some HDMI1.x AVRs when providing the combined EDID for the AVR and sink incorrectly report
	   support for HDMI2.0 modes indicating 4k@60/50 modes as available. Check TMDS character rate
	   for the same. If it is not set or < 340, support for 4k@60/50 is not there and EDID is
	   incorrectly indicating 4k@60/50 support. Remove 4k@60/50 support for such cases. */
	if (_HdmiSinkAvCap.ui2_sink_max_tmds_character_rate != 0) {
		maxTMDSRate = _HdmiSinkAvCap.ui2_sink_max_tmds_character_rate;
	} else {
		maxTMDSRate = _HdmiSinkAvCap.ui1_sink_max_tmds_clock;
	}

	yuv420Supported = ((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420) ||
					   (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_YCBCR_420_CAPABILITY)) ? 1 : 0;
	if (((yuv420Supported) && (maxTMDSRate >= 297)) || (maxTMDSRate >= 340)/* HDMI2 modes*/) {
		pv_get_info->ui4_sink_hdmi_4k2kvic = u4Res;
	} else {
		if (u4Res != 0) {
			TX_DEF_LOG("hdmi_AppGetEdidInfo: u4Res=0x%llx, maxTMDSRate = %u, max_tmds_clock=%u, max_tmds_char_rate = %u\n",
			    u4Res, maxTMDSRate, _HdmiSinkAvCap.ui1_sink_max_tmds_clock, _HdmiSinkAvCap.ui2_sink_max_tmds_character_rate);
			if(u4Res & SINK_2160P_60HZ)
				u4Res &= ~SINK_2160P_60HZ;
			if(u4Res & SINK_2160P_50HZ)
				u4Res &= ~SINK_2160P_50HZ;
			if(u4Res & SINK_2161P_60HZ)
				u4Res &= ~SINK_2161P_60HZ;
			if(u4Res & SINK_2161P_50HZ)
				u4Res &= ~SINK_2161P_50HZ;
			TX_DEF_LOG("hdmi_AppGetEdidInfo: 4k@60/50 modes are not supported, 4K u4Res = 0x%llx\n", u4Res);
		}
		pv_get_info->ui4_sink_hdmi_4k2kvic = u4Res;
	}

	pv_get_info->ui1_sink_max_tmds_clock = _HdmiSinkAvCap.ui1_sink_max_tmds_clock;
	pv_get_info->ui2_sink_max_tmds_character_rate = _HdmiSinkAvCap.ui2_sink_max_tmds_character_rate;

	if (hdmi_force_sdr == FALSE) {
		pv_get_info->ui1_sink_support_static_hdr = _HdmiSinkAvCap.ui1_sink_support_static_hdr;
		if (debug_hdr10p_force_enable_edid)
			_HdmiSinkAvCap.ui1_sink_support_dynamic_hdr |= EDID_SUPPORT_HDR10_PLUS;
		if (debug_hdr10p_force_set_appversion != 0xff) /*if it is not 0xff, we received a command to set a forced version - set it*/
			_HdmiSinkAvCap.ui1_sink_hdr10plus_app_version = debug_hdr10p_force_set_appversion;
		pv_get_info->ui1_sink_support_dynamic_hdr = _HdmiSinkAvCap.ui1_sink_support_dynamic_hdr;
		pv_get_info->ui1_sink_hdr10plus_app_version = _HdmiSinkAvCap.ui1_sink_hdr10plus_app_version;
		pv_get_info->ui1_sink_hdr_content_max_luminance =
			_HdmiSinkAvCap.ui1_sink_hdr_content_max_luminance;
		pv_get_info->ui1_sink_hdr_content_max_frame_average_luminance =
			_HdmiSinkAvCap.ui1_sink_hdr_content_max_frame_average_luminance;
		pv_get_info->ui1_sink_hdr_content_min_luminance =
			_HdmiSinkAvCap.ui1_sink_hdr_content_min_luminance;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_length =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_length;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_version =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_version;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v1_low_latency =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v1_low_latency;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v2_interface =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_interface;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_low_latency_support =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_low_latency_support;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444 =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_support_backlight_control =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_support_backlight_control;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_backlt_min_lumal =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_backlt_min_lumal;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmin =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmin;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmax =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmax;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tminPQ =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tminPQ;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmaxPQ =
			_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_tmaxPQ;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Rx = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Rx;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Ry = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Ry;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Gx = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gx;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Gy = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Gy;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Bx = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Bx;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_By = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_By;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Wx = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wx;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Wy = _HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_Wy;
		for (i = 0; i < 32; i++)
			pv_get_info->ui1_sink_dolbyvision_block[i] = _HdmiSinkAvCap.ui1_sink_dolbyvision_block[i];
	} else {
		unsigned int removeBT2020 = ~(SINK_COLOR_SPACE_BT2020_CYCC | SINK_COLOR_SPACE_BT2020_YCC | SINK_COLOR_SPACE_BT2020_RGB);
		HDMI_PLUG_LOG("removeBT2020=0x%x, pv_get_info->ui2_sink_colorimetry=0x%x\n", removeBT2020, pv_get_info->ui2_sink_colorimetry);
		pv_get_info->ui2_sink_colorimetry &= ~(SINK_COLOR_SPACE_BT2020_CYCC | SINK_COLOR_SPACE_BT2020_YCC | SINK_COLOR_SPACE_BT2020_RGB);
		HDMI_PLUG_LOG("after removeBT2020=0x%x, pv_get_info->ui2_sink_colorimetry=0x%x\n", removeBT2020, pv_get_info->ui2_sink_colorimetry);
		pv_get_info->ui1_sink_support_static_hdr = 0;
		pv_get_info->ui1_sink_support_dynamic_hdr = 0;
		pv_get_info->ui1_sink_hdr_content_max_luminance = 0;
		pv_get_info->ui1_sink_hdr_content_max_frame_average_luminance = 0;
		pv_get_info->ui1_sink_hdr_content_min_luminance = 0;
		pv_get_info->ui1_sink_hdr10plus_app_version = 0xff;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_length = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_version = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v1_low_latency = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v2_interface = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_low_latency_support = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444 = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_support_backlight_control = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_backlt_min_lumal = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmin = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmax = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tminPQ = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_tmaxPQ = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Rx = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Ry = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Gx = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Gy = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Bx = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_By = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Wx = 0;
		pv_get_info->ui4_sink_dolbyvision_vsvdb_Wy = 0;
		for (i = 0; i < 32; i++)
			pv_get_info->ui1_sink_dolbyvision_block[i] = 0;
	}

	for (i = 0; i < EDID_LENGTH; i++)
		pv_get_info->ui1rawdata_edid[i] = _bEdidData[i];

	if (isThomsonUD9_India_TV()) {
		HDMI_PLUG_LOG("THOMSON UD9 43TH6000 TV (India only TV) remove Dolby audio capability\n");
		pv_get_info->ui4_hdmi_ac3_ch_type = 0x00;
		pv_get_info->ui4_hdmi_ac3_ch3ch4ch5ch7_type = 0x00;
		pv_get_info->ui4_hdmi_ec3_ch_type = 0x00;
		pv_get_info->ui4_hdmi_ec3_ch3ch4ch5ch7_type = 0x00;
	}
}

unsigned char hdmi_check_edid_header(void)
{
	unsigned char bIdx;

	for (bIdx = EDID_ADDR_HEADER; bIdx < (EDID_ADDR_HEADER + EDID_HEADER_LEN); bIdx++) {
		if (_bEdidData[bIdx] != aEDIDHeader[bIdx])
			return 0;
	}
	return 1;
}

void hdmi_clear_edid_data(void)
{
	memset(_bEdidData, 0, EDID_SIZE);
}
#endif
