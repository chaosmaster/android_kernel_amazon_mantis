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

#include <linux/clk.h>
#include "hdmictrl.h"
#include "hdmi_ctrl.h"
#include "hdmihdcp.h"
#include "hdmiedid.h"
#include "hdmicec.h"
#include "hdmiddc.h"

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
#include "hdmi_ca.h"
#endif


HDMI_AV_INFO_T _stAvdAVInfo = { 0 };

int _ui4GammaReg[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

int _ui4CbGainReg;
int _ui4CrGainReg;
bool _fgXVYccBitStreamOn = FALSE;
bool _fgJpegPlayOn = FALSE;
bool _fgSendAVIXVYcc = FALSE;
char _u1GamutType = GAMUT_TYPE_AVCHD_FIXED;
char _u1PictureSourceAR = HDMI_16_9;
char _u1DisaplayAFDMode = 0x08;
bool _fgxvColorOn = TRUE;
bool _fgVSyncMuteHdmi = FALSE;
unsigned char debug_force_hotplug;

static unsigned char _bAudInfoFm[5];
static unsigned char _bAviInfoFm[5];
static unsigned char _bSpdInf[25] = { 0 };

char _bStaticHdrStatus = HDR_PACKET_DISABLE;
char _bHdrType;
char _bHdrMDLength;
char *_bHdrMetadataBuff;
bool _fgBT2020Enable;
bool _fgDolbyHdrEnable;
bool _fgLowLatencyDolbyVisionEnable;
unsigned int _u4HdrDebugDisableType;
bool dovi_off_delay_needed = FALSE;

bool _fgBackltCtrlMDPresent;
unsigned int _u4EffTmaxPQ;
char _bStaticHdrType = GAMMA_ST2084;
HDMI_AV_INFO_T *_prAvdAVInfo = &_stAvdAVInfo;
unsigned int _u4SinkProductID;
unsigned int hdmi_boot_res;
unsigned int hdmi_boot_colordepth;
unsigned int hdmi_boot_colorspace;
unsigned int hdmi_boot_forcehdr;

DEFINE_SEMAPHORE(hdcp_update_mutex);
DEFINE_SEMAPHORE(color_space_mutex);
static unsigned int _u4NValue;

static const char *szHdmiResStr[HDMI_VIDEO_RESOLUTION_NUM] = {
	"RES_480I",
	"RES_576I",
	"RES_480P",
	"RES_576P",
	"RES_720P60HZ",
	"RES_720P50HZ",
	"RES_1080I60HZ",
	"RES_1080I50HZ",
	"RES_1080P30HZ",
	"RES_1080P25HZ",
	"RES_1080P24HZ",
	"RES_1080P23_976HZ",
	"RES_1080P29_97HZ",
	"RES_1080P60HZ",
	"RES_1080P50HZ",
	"RES_720p3d60Hz",
	"RES_720p3d50Hz",
	"RES_1080i3d60Hz",
	"RES_1080i3d50Hz",
	"RES_1080P3d24Hz",
	"RES_1080P3d23Hz",
	"RES_2160P_23_976HZ",
	"RES_2160P_24HZ",
	"RES_2160P_25HZ",
	"RES_2160P_29_97HZ",
	"RES_2160P_30HZ",
	"RES_2161P_24HZ",
	"RES_2160P_60HZ",
	"RES_2160P_50HZ",
	"RES_2161P_60HZ",
	"RES_2161P_50HZ",
	"RES_720P_59_94HZ",
	"RES_1080P_59_94HZ",
	"RES_2160P_59_94HZ",
	"RES_2161P_59_94HZ",
};


static const char *szHdmiDeepcolorStr[HDMI_DEEP_COLOR_16_BIT+1] = {
	"HDMI_DEEP_COLOR_AUTO",
	"HDMI_DEEP_COLOR_8_BIT",
	"HDMI_DEEP_COLOR_10_BIT",
	"HDMI_DEEP_COLOR_12_BIT",
	"HDMI_DEEP_COLOR_16_BIT",
};

static const char *szHdmiColorspaceStr[HDMI_YCBCR_420_FULL+1] = {
	"HDMI_RGB",
	"HDMI_RGB_FULL",
	"HDMI_YCBCR_444",
	"HDMI_YCBCR_422",
	"HDMI_YCBCR_420",
	"HDMI_XV_YCC",
	"HDMI_YCBCR_444_FULL",
	"HDMI_YCBCR_422_FULL",
	"HDMI_YCBCR_420_FULL",
};

static const char *cHdmiAudFsStr[7] = {
	"HDMI_FS_32K",
	"HDMI_FS_44K",
	"HDMI_FS_48K",
	"HDMI_FS_88K",
	"HDMI_FS_96K",
	"HDMI_FS_176K",
	"HDMI_FS_192K"
};


static const char *cAudCodingTypeStr[16] = {
	"Refer to Stream Header",
	"PCM",
	"AC3",
	"MPEG1",
	"MP3",
	"MPEG2",
	"AAC",
	"DTS",
	"ATRAC",
	"ONE Bit Audio",
	"Dolby Digital+",
	"DTS-HD",
	"MAT(MLP)",
	"DST",
	"WMA Pro",
	"Reserved",
};

static const char *cAudChCountStr[8] = {
	"Refer to Stream Header",
	"2ch",
	"3ch",
	"4ch",
	"5ch",
	"6ch",
	"7ch",
	"8ch",

};

static const char *cAudFsStr[8] = {
	"Refer to Stream Header",
	"32 khz",
	"44.1 khz",
	"48 khz",
	"88.2 khz",
	"96 khz",
	"176.4 khz",
	"192 khz"
};


static const char *cAudChMapStr[32] = {
	"FR,FL",
	"LFE,FR,FL",
	"FC,FR,FL",
	"FC,LFE,FR,FL",
	"RC,FR,FL",
	"RC,LFE,FR,FL",
	"RC,FC,FR,FL",
	"RC,FC,LFE,FR,FL",
	"RR,RL,FR,FL",
	"RR,RL,LFE,FR,FL",
	"RR,RL,FC,FR,FL",
	"RR,RL,FC,LFE,FR,FL",
	"RC,RR,RL,FR,FL",
	"RC,RR,RL,LFE,FR,FL",
	"RC,RR,RL,FC,FR,FL",
	"RC,RR,RL,FC,LFE,FR,FL",
	"RRC,RLC,RR,RL,FR,FL",
	"RRC,RLC,RR,RL,LFE,FR,FL",
	"RRC,RLC,RR,RL,FC,FR,FL",
	"RRC,RLC,RR,RL,FC,LFE,FR,FL",
	"FRC,FLC,FR,FL",
	"FRC,FLC,LFE,FR,FL",
	"FRC,FLC,FC,FR,FL",
	"FRC,FLC,FC,LFE,FR,FL",
	"FRC,FLC,RC,FR,FL",
	"FRC,FLC,RC,LFE,FR,FL",
	"FRC,FLC,RC,FC,FR,FL",
	"FRC,FLC,RC,FC,LFE,FR,FL",
	"FRC,FLC,RR,RL,FR,FL",
	"FRC,FLC,RR,RL,LFE,FR,FL",
	"FRC,FLC,RR,RL,FC,FR,FL",
	"FRC,FLC,RR,RL,FC,LFE,FR,FL",
};

static const char *cAudDMINHStr[2] = {
	"Permiited down mixed stereo or no information",
	"Prohibited down mixed stereo"
};

static const char *cAudSampleSizeStr[4] = {
	"Refer to Stream Header",
	"16 bit",
	"20 bit",
	"24 bit"
};

static const char *cAviRgbYcbcrStr[4] = {
	"RGB",
	"YCbCr 4:2:2",
	"YCbCr 4:4:4",
	"Future"
};

static const char *cAviActivePresentStr[2] = {
	"No data",
	"Actuve Format(R0..R3) Valid",

};

static const char *cAviBarStr[4] = {
	"Bar data not valid",
	"Vert. Bar info valid",
	"Horiz. Bar info valid",
	"Vert. and Horiz Bar info valid",
};

static const char *cAviScanStr[4] = {
	"No data",
	"Overscanned display",
	"underscanned display",
	"Future",
};

static const char *cAviColorimetryStr[4] = {
	"no data",
	"ITU601",
	"ITU709",
	"Extended Colorimetry infor valid",
};

static const char *cAviAspectStr[4] = {
	"No data",
	"4:3",
	"16:9",
	"Future",
};


static const char *cAviActiveStr[16] = {
	"reserved",
	"reserved",
	"box 16:9(top)",
	"box 14:9(top)",
	"box > 16:9(center)",
	"reserved",
	"reserved",
	"reserved",
	"Same as picture aspect ratio",
	"4:3(Center)",
	"16:9(Center)",
	"14:9(Center)",
	"reserved",
	"4:3(with shoot & protect 14:9 center)",
	"16:9(with shoot & protect 14:9 center)",
	"16:3(with shoot & protect 4:3 center)"
};

static const char *cAviItContentStr[2] = {
	"no data",
	"IT Content"
};

static const char *cAviExtColorimetryStr[2] = {
	"xvYCC601",
	"xvYCC709",
};

static const char *cAviRGBRangeStr[4] = {
	"depends on video format",
	"Limit range",
	"FULL range",
	"Reserved",
};

static const char *cAviScaleStr[4] = {
	"Unknown non-uniform scaling",
	"Picture has been scaled horizontally",
	"Picture has been scaled vertically",
	"Picture has been scaled horizontally and vertically",
};



static const char *cSPDDeviceStr[16] = {
	"unknown",
	"Digital STB",
	"DVD Player",
	"D-VHS",
	"HDD Videorecorder",
	"DVC",
	"DSC",
	"Video CD",
	"Game",
	"PC General",
	"Blu-Ray Disc",
	"Super Audio CD",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
};

typedef struct {
	HDMI_GEN_PACKET_HW_ENUM hw_num;
	unsigned int addr_header;
	unsigned int addr_pkt;
	unsigned int addr_wr_en;
	unsigned int mask_wr_en;
	unsigned int addr_rep_en;
	unsigned int mask_rep_en;
} PACKET_HW_T;

static PACKET_HW_T pkthw[GEN_PKT_HW_NUM] = {
	{
		.hw_num = GEN_PKT_HW1,
		.addr_header = TOP_GEN_HEADER,
		.addr_pkt = TOP_GEN_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN_EN | GEN_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW2,
		.addr_header = TOP_GEN2_HEADER,
		.addr_pkt = TOP_GEN2_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN2_EN | GEN2_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN2_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW3,
		.addr_header = TOP_GEN3_HEADER,
		.addr_pkt = TOP_GEN3_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN3_EN | GEN3_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN3_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW4,
		.addr_header = TOP_GEN4_HEADER,
		.addr_pkt = TOP_GEN4_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN4_EN | GEN4_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN4_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW5,
		.addr_header = TOP_GEN5_HEADER,
		.addr_pkt = TOP_GEN5_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN5_EN | GEN5_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN5_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW6,
		.addr_header = TOP_GEN6_HEADER,
		.addr_pkt = TOP_GEN6_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN6_EN | GEN6_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN6_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW7,
		.addr_header = TOP_GEN7_HEADER,
		.addr_pkt = TOP_GEN7_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN7_EN | GEN7_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN7_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW8,
		.addr_header = TOP_GEN8_HEADER,
		.addr_pkt = TOP_GEN8_PKT00,
		.addr_wr_en = TOP_INFO_EN,
		.mask_wr_en = GEN8_EN | GEN8_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN8_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW9,
		.addr_header = TOP_GEN9_HEADER,
		.addr_pkt = TOP_GEN9_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN9_EN | GEN9_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN9_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW10,
		.addr_header = TOP_GEN10_HEADER,
		.addr_pkt = TOP_GEN10_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN10_EN | GEN10_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN10_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW11,
		.addr_header = TOP_GEN11_HEADER,
		.addr_pkt = TOP_GEN11_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN11_EN | GEN11_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN11_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW12,
		.addr_header = TOP_GEN12_HEADER,
		.addr_pkt = TOP_GEN12_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN12_EN | GEN12_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN12_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW13,
		.addr_header = TOP_GEN13_HEADER,
		.addr_pkt = TOP_GEN13_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN13_EN | GEN13_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN13_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW14,
		.addr_header = TOP_GEN14_HEADER,
		.addr_pkt = TOP_GEN14_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN14_EN | GEN14_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN14_RPT_EN,
	},
	{
		.hw_num = GEN_PKT_HW15,
		.addr_header = TOP_GEN15_HEADER,
		.addr_pkt = TOP_GEN15_PKT00,
		.addr_wr_en = TOP_INFO_EN_EXPAND,
		.mask_wr_en = GEN15_EN | GEN15_EN_WR,
		.addr_rep_en = TOP_INFO_RPT,
		.mask_rep_en = GEN15_RPT_EN,
	},

};


#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
void ta_internal_hdmi_write(unsigned int u4Reg, unsigned int u4data)
{
	vCaHDMIWriteReg(u4Reg, u4data);
}
#endif
void internal_hdmi_read(unsigned long u4Reg, unsigned int *p4Data)
{
	*p4Data = (*(volatile unsigned int *)(u4Reg));
	/*HDMI_REG_LOG("[R]addr = 0x%lx, data = 0x%08x\n", u4Reg, *p4Data); */
}

void internal_hdmi_write(unsigned long u4Reg, unsigned int u4data)
{
	*(volatile unsigned int *)(u4Reg) = (u4data);
	/*HDMI_REG_LOG("[R]addr = 0x%lx, data = 0x%08x\n", u4Reg, u4data); */
}

/* //////////////////////////////////////////// */
unsigned int hdmi_drv_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_reg[REG_HDMI_DIG] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_drv_write(unsigned short u2Reg, unsigned int u4Data)
{
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	if (!((u2Reg == 0x424) || (u2Reg == 0x1C8) || (u2Reg == 0x1CC)
		|| ((u2Reg <= 0x14C) && (u2Reg >= 0x0C0))
		|| ((u2Reg <= 0x364) && (u2Reg >= 0x200))
		|| ((u2Reg <= 0xD00) && (u2Reg >= 0xC00)))) {
		HDMI_REG_LOG("[W]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
		internal_hdmi_write(hdmi_reg[REG_HDMI_DIG] + u2Reg, u4Data);
	} else {
		ta_internal_hdmi_write(u2Reg, u4Data);
	}
#else
	if (!((u2Reg == 0xca0) || (u2Reg == 0xca4)))
		HDMI_REG_LOG("[W]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_reg[REG_HDMI_DIG] + u2Reg, u4Data);
#endif
}

/* /////////////////////////////////////////////// */
unsigned int hdmi_sys_read(unsigned short u2Reg)
{
	unsigned int u4Data = 0;

	internal_hdmi_read(hdmi_ref_reg[MMSYS_CONFIG] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_sys_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[MMSYS_CONFIG] + u2Reg, u4Data);
}

/* /////////////////////////////////////////////////// */
unsigned int hdmi_hdmitopck_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[TOPCK_GEN] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_hdmitopck_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[TOPCK_GEN] + u2Reg, u4Data);
}

unsigned int hdmi_infrasys_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[INFRA_SYS] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_infrasys_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[INFRA_SYS] + u2Reg, u4Data);
}

unsigned int hdmi_perisys_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[PERISYS_REG] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_perisys_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[PERISYS_REG] + u2Reg, u4Data);
}

unsigned int hdmi_rgb2hdmi_read(unsigned int u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_reg[REG_HDMI_RGB2HDMI] + u2Reg - 0x1800, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_rgb2hdmi_write(unsigned int u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_reg[REG_HDMI_RGB2HDMI] + u2Reg - 0x1800, u4Data);
}

unsigned int hdmi_p2i_read(unsigned int u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[P2I_REG] + u2Reg - 0x1700, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_p2i_write(unsigned int u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[P2I_REG] + u2Reg - 0x1700, u4Data);
}

unsigned int hdmi_vdout10_read(unsigned int u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[VDOUT10_REG] + u2Reg - 0x1000, &u4Data);
	/* HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data); */
	return u4Data;
}

void hdmi_vdout10_write(unsigned int u2Reg, unsigned int u4Data)
{
	/* HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data); */
	internal_hdmi_write(hdmi_ref_reg[VDOUT10_REG] + u2Reg - 0x1000, u4Data);
}

unsigned int hdmi_vdout15_read(unsigned int u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[VDOUT15_REG] + u2Reg - 0x1500, &u4Data);
	/* HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data); */
	return u4Data;
}

void hdmi_vdout15_write(unsigned int u2Reg, unsigned int u4Data)
{
	/* HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data); */
	internal_hdmi_write(hdmi_ref_reg[VDOUT15_REG] + u2Reg - 0x1500, u4Data);
}

unsigned int hdmi_vdout1c_read(unsigned int u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[VDOUT1C_REG] + u2Reg - 0x1c00, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_vdout1c_write(unsigned int u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[VDOUT1C_REG] + u2Reg - 0x1c00, u4Data);

}

static unsigned int ddc_count;
/* 1: hdmi reset, 2: risc DDC, 3: hdcp2.x reset, 4: requset DDC, 5: reset DDC */
bool hdmi_ddc_request(unsigned char req)
{
	unsigned int i;

	if (down_interruptible(&hdcp_update_mutex)) {
		TX_DEF_LOG("can't get semaphore in %s() for boot time\n", __func__);
		return TRUE;
	}

	ddc_count++;

	if (req == 1)
		HDMI_DDC_LOG(">HDMI reset request DDC, %d\n", ddc_count);
	else if (req == 3)
		HDMI_DDC_LOG(">HDCP2.x rst request DDC, %d\n", ddc_count);
	else if (req == 4)
		HDMI_DDC_LOG(">request DDC, %d\n", ddc_count);

	/* if hdcp2.x enable and req is hdcp1.x, then return */
	if (req == 2)
		return TRUE;

	if (req == 5) {
		/* reset DDC */
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, DISABLE_IDLE_DDC_RESET, DISABLE_IDLE_DDC_RESET);
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(RISC_CLK_DDC_RST, RISC_CLK_DDC_RST);
#endif
		udelay(1);
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(0, RISC_CLK_DDC_RST);
#endif
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, 0, DISABLE_IDLE_DDC_RESET);

		/* send stop cmd */
		if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
			vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
			udelay(250);
		}
		return TRUE;
	}	

	if ((bReadByteHdmiGRL(HDCP2X_CTRL_0) & HDCP2X_EN) == 0)
		return TRUE;

	/* step1 : wait hdcp2.x auth finish */
	if (req == 4) {
		for (i = 0; i < 1000; i++) {
			if (fgHDMIHdcp2Auth() == FALSE)
				break;
			usleep_range(1000, 1500);
		}
		HDMI_DDC_LOG("hdcp2.x stop, %d, %d\n", i, bHDMIHDCP2Err());
	} else {
		HDMI_DDC_LOG("req %d, hdcp2.x state %d\n", req, bHDMIHDCP2Err());
	}

	/* step2 : stop polling */
	vWriteHdmiGRLMsk(HDCP2X_POL_CTRL, HDCP2X_DIS_POLL_EN, HDCP2X_DIS_POLL_EN);

	/* step4 : stop hdcp2.x */
	vWriteHdmiGRLMsk(HDCP2X_GP_IN, 0xdb << HDCP2X_GP_IN3_SHIFT, HDCP2X_GP_IN3);

	/* step5 : wait hdcp2.x msg finish */
	for (i = 0; i < 100; i++) {
		if ((bReadByteHdmiGRL(HPD_DDC_STATUS) & DDC_I2C_IN_PROG) == 0) {
			vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_EN);
			break;
		}
		usleep_range(1000, 1050);
	}

	if (i == 100) {
		TX_DEF_LOG("DDC error, %d\n", bHDMIHDCP2Err());

		/* reset DDC */
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, DISABLE_IDLE_DDC_RESET, DISABLE_IDLE_DDC_RESET);
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(RISC_CLK_DDC_RST, RISC_CLK_DDC_RST);
#endif
		udelay(1);
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(0, RISC_CLK_DDC_RST);
#endif
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, 0, DISABLE_IDLE_DDC_RESET);

		/* send stop cmd */
		if (bReadByteHdmiGRL(HDCP2X_DDCM_STATUS) & DDC_I2C_BUS_LOW) {
			vWriteHdmiGRLMsk(DDC_CTRL, (CLOCK_SCL << DDC_CMD_SHIFT), DDC_CMD);
			udelay(250);
		}
	} else if (i > 1) {
		TX_DEF_LOG("DDC stop, %d, %d\n", i, bHDMIHDCP2Err());
	}

	/* step6 : disable HDCP2.x */
	vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_EN);

	/* step7 : reset HDCP2.x */
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

	/* step8 : free hdcp2.x */
	vWriteHdmiGRLMsk(HDCP2X_GP_IN, 0 << HDCP2X_GP_IN3_SHIFT, HDCP2X_GP_IN3);

	return TRUE;
}

void hdmi_ddc_free(unsigned char req)
{
	up(&hdcp_update_mutex);

	if (req == 1)
		HDMI_DDC_LOG("HDMI reset free DDC, %d\n", ddc_count);
	else if (req == 3)
		HDMI_DDC_LOG("HDCP2.x rst free DDC, %d\n", ddc_count);
	else if (req == 4)
		HDMI_DDC_LOG("free DDC, %d\n", ddc_count);
}

/* ///////////////////////////////////////////////////// */
unsigned int hdmi_ana_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_reg[REG_HDMI_ANA] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_ana_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_reg[REG_HDMI_ANA] + u2Reg, u4Data);
}

/* ///////////////////////////////////////////////////// */

unsigned int hdmi_pad_read(unsigned short u2Reg)
{
	unsigned int u4Data;

	internal_hdmi_read(hdmi_ref_reg[GPIO_REG] + u2Reg, &u4Data);
	HDMI_REG_LOG("[R]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_pad_write(unsigned short u2Reg, unsigned int u4Data)
{
	HDMI_REG_LOG("[W]addr = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	internal_hdmi_write(hdmi_ref_reg[GPIO_REG] + u2Reg, u4Data);
}

/* ///////////////////////////////////////////////////// */
unsigned char bResolution_SD(unsigned char ui1resindex)
{
	if ((ui1resindex == HDMI_VIDEO_720x480i_60Hz) ||
	    (ui1resindex == HDMI_VIDEO_720x576i_50Hz) ||
	    (ui1resindex == HDMI_VIDEO_720x480p_60Hz) ||
	    (ui1resindex == HDMI_VIDEO_720x576p_50Hz)) {
		return TRUE;
	} else {
		return FALSE;
	}

}

void HDMIForceHDCPHPCLevel(void)
{
	/* force HDCP HPD to 1*/
	vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_HPD_OVR, HDCP2X_HPD_OVR);
	vWriteHdmiGRLMsk(HDCP2X_CTRL_0, HDCP2X_HPD_SW, HDCP2X_HPD_SW);
}

void vHdmiSetInit(void)
{
	vWriteIoHdmiAnaMsk(HDMI_ANA_CTL, (0x1 << 16), (0x1 << 16) | (0x1 << 0));
	HDMIForceHDCPHPCLevel();
}

unsigned char fgIsVSEnable(void)
{
	if (bReadByteHdmiGRL(TOP_INFO_EN) & VSIF_EN)
		return TRUE;
	else
		return FALSE;
}

unsigned char vIsDviMode(void)
{
	unsigned char bData;

	bData = bReadByteHdmiGRL(TOP_CFG00);
	if (bData & HDMI_MODE_HDMI)
		return FALSE;
	else
		return TRUE;
}

unsigned char hdmi_get_port_hpd_value(void)
{
	unsigned char bStatus = 0;

	if (IS_HDMI_PORD())
		bStatus = STATUS_PORD;
	if (IS_HDMI_HTPLG())
		bStatus |= STATUS_HTPLG;
	return bStatus;
}

void MuteHDMIAudio(void)
{
	HDMI_AUDIO_FUNC();

	if (bReadByteHdmiGRL(AIP_CTRL) & DSD_EN)
		vWriteHdmiGRLMsk(AIP_TXCTRL, DSD_MUTE_DATA | AUD_MUTE_FIFO_EN,
				 DSD_MUTE_DATA | AUD_MUTE_FIFO_EN);
	else
		vWriteHdmiGRLMsk(AIP_TXCTRL, AUD_MUTE_FIFO_EN, AUD_MUTE_FIFO_EN);
}

void vHDMIAVMute(void)
{
	HDMI_AUDIO_FUNC();

	vBlackHDMIOnly();
	MuteHDMIAudio();
}

void vTmdsOnOffAndResetHdcp(unsigned char fgHdmiTmdsEnable)
{
	HDMI_DRV_FUNC();

	if (fgHdmiTmdsEnable == 1) {
		mdelay(1);
		vTmdsPresetOn();
		vTxSignalOnOff(SV_ON);
	} else {
		vHDMIAVMute();
		mdelay(1);
		vTxSignalOnOff(SV_OFF);
		vHDCPReset();
		mdelay(1);
	}
}

unsigned char bResolution_4K2K(unsigned char bResIndex)
{
	if ((bResIndex == HDMI_VIDEO_3840x2160P_23_976HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_24HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_25HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_29_97HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_30HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_24HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_50HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_50HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_59_94HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_59_94HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_60HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_60HZ))
		return TRUE;
	else
		return FALSE;

}

unsigned char bResolution_4K2K_50_60Hz(unsigned char bResIndex)
{
	if ((bResIndex == HDMI_VIDEO_3840x2160P_50HZ) || (bResIndex == HDMI_VIDEO_4096x2160P_50HZ)
		|| (bResIndex == HDMI_VIDEO_3840x2160P_59_94HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_59_94HZ)
	    || (bResIndex == HDMI_VIDEO_3840x2160P_60HZ)
	    || (bResIndex == HDMI_VIDEO_4096x2160P_60HZ))
		return TRUE;
	else
		return FALSE;

}

void vConfigHdmiSYS(unsigned char bResIndex)
{

}

void vResetHDMI(char bRst)
{
	HDMI_DRV_FUNC();

	if (bRst) {
		vWriteHdmiSYSMsk(MMSYS_VDOUT_SW_RST, 0, MMSYS_VDOUT_HDMI_RST);
		//vWriteHdmiGRLMsk(HDCP_TOP_CTRL, 0, OTPVMUTEOVR_SET);
		//vWriteHdmiGRLMsk(HDCP_TOP_CTRL, 0, OTP2XVOVR_EN | OTP14VOVR_EN);
	}
	else {
		vWriteHdmiSYSMsk(MMSYS_VDOUT_SW_RST, MMSYS_VDOUT_HDMI_RST, MMSYS_VDOUT_HDMI_RST);
	}
}

void vAudioPacketOff(unsigned char bOn)
{
	HDMI_AUDIO_FUNC();

	if (bOn)
		vWriteHdmiGRLMsk(AIP_TXCTRL, AUD_PACKET_DROP, AUD_PACKET_DROP);
	else
		vWriteHdmiGRLMsk(AIP_TXCTRL, 0, AUD_PACKET_DROP);
}

void vSetHdmiI2SDataFmt(unsigned char bFmt)
{
	unsigned int u4Data;

	HDMI_AUDIO_LOG("vSetHdmiI2SDataFmt bFmt = %d-------\n", bFmt);

	u4Data = bReadByteHdmiGRL(AIP_I2S_CTRL);
	HDMI_AUDIO_LOG("AIP_I2S_CTRL1 value = 0x%x-------\n", u4Data);
	u4Data &= ~(WS_HIGH | I2S_1ST_BIT_NOSHIFT | JUSTIFY_RIGHT);

	switch (bFmt) {
	case HDMI_RJT_24BIT:
		u4Data |= (WS_HIGH | I2S_1ST_BIT_NOSHIFT | JUSTIFY_RIGHT);
		break;

	case HDMI_RJT_16BIT:
		u4Data |= (WS_HIGH | I2S_1ST_BIT_NOSHIFT | JUSTIFY_RIGHT);
		break;

	case HDMI_LJT_24BIT:
		u4Data |= (WS_HIGH | I2S_1ST_BIT_NOSHIFT);
		break;

	case HDMI_LJT_16BIT:
		u4Data |= (WS_HIGH | I2S_1ST_BIT_NOSHIFT);
		break;

	case HDMI_I2S_24BIT:
		break;

	case HDMI_I2S_16BIT:
		break;

	default:
		break;
	}
	vWriteByteHdmiGRL(AIP_I2S_CTRL, u4Data);

	u4Data = bReadByteHdmiGRL(AIP_I2S_CTRL);
	HDMI_AUDIO_LOG("AIP_I2S_CTRL2 value = 0x%x-------\n", u4Data);
}

void vUpdateAudSrcChType(unsigned char u1SrcChType)
{
	_stAvdAVInfo.ui2_aud_out_ch.word = 0;

	switch (u1SrcChType) {
	case AUD_INPUT_1_0:
	case AUD_INPUT_2_0:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui1_aud_out_ch_number = 2;
		break;

	case AUD_INPUT_1_1:
	case AUD_INPUT_2_1:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 3;
		break;

	case AUD_INPUT_3_0:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui1_aud_out_ch_number = 3;
		break;

	case AUD_INPUT_3_0_LRS:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui1_aud_out_ch_number = 4;
		break;

	case AUD_INPUT_3_1_LRS:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 0;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 5;
		break;

	case AUD_INPUT_4_0_CLRS:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 5;
		break;

	case AUD_INPUT_4_1_CLRS:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 6;
		break;

	case AUD_INPUT_3_1:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 4;
		break;

	case AUD_INPUT_4_0:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui1_aud_out_ch_number = 4;
		break;

	case AUD_INPUT_4_1:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 5;
		break;

	case AUD_INPUT_5_0:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 5;
		break;

	case AUD_INPUT_5_1:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 6;
		break;

	case AUD_INPUT_6_0:
	case AUD_INPUT_6_0_Cs:
	case AUD_INPUT_6_0_Ch:
	case AUD_INPUT_6_0_Oh:
	case AUD_INPUT_6_0_Chr:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RC = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 6;
		break;

	case AUD_INPUT_6_1:
	case AUD_INPUT_6_1_Cs:
	case AUD_INPUT_6_1_Ch:
	case AUD_INPUT_6_1_Oh:
	case AUD_INPUT_6_1_Chr:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RC = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 7;
		break;

	case AUD_INPUT_7_0:
	case AUD_INPUT_7_0_Lh_Rh:
	case AUD_INPUT_7_0_Lsr_Rsr:
	case AUD_INPUT_7_0_Lc_Rc:
	case AUD_INPUT_7_0_Lw_Rw:
	case AUD_INPUT_7_0_Lsd_Rsd:
	case AUD_INPUT_7_0_Lss_Rss:
	case AUD_INPUT_7_0_Lhs_Rhs:
	case AUD_INPUT_7_0_Cs_Ch:
	case AUD_INPUT_7_0_Cs_Oh:
	case AUD_INPUT_7_0_Cs_Chr:
	case AUD_INPUT_7_0_Ch_Oh:
	case AUD_INPUT_7_0_Ch_Chr:
	case AUD_INPUT_7_0_Oh_Chr:
	case AUD_INPUT_7_0_Lss_Rss_Lsr_Rsr:
	case AUD_INPUT_8_0_Lh_Rh_Cs:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 0;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RRC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RLC = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 7;
		break;

	case AUD_INPUT_7_1:
	case AUD_INPUT_7_1_Lh_Rh:
	case AUD_INPUT_7_1_Lsr_Rsr:
	case AUD_INPUT_7_1_Lc_Rc:
	case AUD_INPUT_7_1_Lw_Rw:
	case AUD_INPUT_7_1_Lsd_Rsd:
	case AUD_INPUT_7_1_Lss_Rss:
	case AUD_INPUT_7_1_Lhs_Rhs:
	case AUD_INPUT_7_1_Cs_Ch:
	case AUD_INPUT_7_1_Cs_Oh:
	case AUD_INPUT_7_1_Cs_Chr:
	case AUD_INPUT_7_1_Ch_Oh:
	case AUD_INPUT_7_1_Ch_Chr:
	case AUD_INPUT_7_1_Oh_Chr:
	case AUD_INPUT_7_1_Lss_Rss_Lsr_Rsr:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.LFE = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RL = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RRC = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.RLC = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 8;
		break;

	default:
		_stAvdAVInfo.ui2_aud_out_ch.bit.FR = 1;
		_stAvdAVInfo.ui2_aud_out_ch.bit.FL = 1;
		_stAvdAVInfo.ui1_aud_out_ch_number = 2;
		break;
	}

}

void vSetChannelSwap(unsigned char u1SwapBit)
{
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, u1SwapBit<<20, 0x0F<<20);
}

unsigned char bGetChannelMapping(void)
{
	unsigned char bChannelMap = 0x00;

	HDMI_PLUG_LOG("u1Aud_Input_Chan_Cnt = %d; ui1_aud_out_ch_number = %d;------\n",
		      _stAvdAVInfo.u1Aud_Input_Chan_Cnt, _stAvdAVInfo.ui1_aud_out_ch_number);

	vUpdateAudSrcChType(_stAvdAVInfo.u1Aud_Input_Chan_Cnt);

	switch (_stAvdAVInfo.ui1_aud_out_ch_number) {
	case 8:

		break;

	case 7:

		break;

	case 6:
		if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FC == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.RR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.RL == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.RC == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.LFE == 0)
		    ) {
			bChannelMap = 0x0E;	/* 6.0 */
		} else if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.FC == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.RR == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.RL == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.RC == 0) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.LFE == 1)
		    ) {
			bChannelMap = 0x0B;	/* 5.1 */

		}
		break;

	case 5:
		break;

	case 4:
		if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.RR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.RL == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.LFE == 0)) {
			bChannelMap = 0x08;
		} else if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.FC == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.LFE == 1)) {
			bChannelMap = 0x03;
		}
		break;

	case 3:
		if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FC == 1)) {
			bChannelMap = 0x02;
		} else if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1) &&
			   (_stAvdAVInfo.ui2_aud_out_ch.bit.LFE == 1)
		    ) {
			bChannelMap = 0x01;
		}

		break;

	case 2:
		if ((_stAvdAVInfo.ui2_aud_out_ch.bit.FR == 1) &&
		    (_stAvdAVInfo.ui2_aud_out_ch.bit.FL == 1)) {
			bChannelMap = 0x00;
		}
		break;

	default:
		break;
	}

	HDMI_PLUG_LOG
	    ("u1Aud_Input_Chan_Cnt = 0x%x; ui1_aud_out_ch_number = 0x%x; bChannelMap = 0x%x------\n",
	     _stAvdAVInfo.u1Aud_Input_Chan_Cnt, _stAvdAVInfo.ui1_aud_out_ch_number, bChannelMap);

	return bChannelMap;
}

void vSetHdmiDsdConfig(unsigned char bChNum, bool fgRxDsdBypass)
{
	vWriteHdmiGRLMsk(AIP_CTRL, DSD_EN, SPDIF_EN | DSD_EN | HBRA_ON);
	vWriteHdmiGRLMsk(AIP_TXCTRL, DSD_MUTE_DATA, DSD_MUTE_DATA);
	if (fgRxDsdBypass)
		vWriteByteHdmiGRL(TOP_AUD_MAP, 0x75316420);	/* 0x13570246 */
	else
		vWriteByteHdmiGRL(TOP_AUD_MAP, 0x04230150);	/* 0 FL;1 SL;2 CENT;3 FR;4 SR;5 LFE 0x32400510 */

	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, 0, I2S2DSD_EN);	/* rxtx bypass */
}

void vSetHdmiI2SSckEdge(unsigned int bEdge)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bEdge, SCK_EDGE_RISE);
}

void vSetHdmiI2SCbitOrder(unsigned int bCbit)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bCbit, CBIT_ORDER_SAME);
}

void vSetHdmiI2SVbit(unsigned int bVbit)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bVbit, VBIT_COM);
}

void vSetHdmiI2SDataDir(unsigned int bDataDir)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bDataDir, DATA_DIR_LSB);
}

void vSetHdmiHbrConfig(bool fgRxDsdBypass)
{
	if (fgRxDsdBypass) {
		vWriteHdmiGRLMsk(AIP_CTRL, HBRA_ON, SPDIF_EN | DSD_EN | HBRA_ON);
		vWriteHdmiGRLMsk(AIP_CTRL, I2S_EN, I2S_EN);
	} else {
		vWriteHdmiGRLMsk(AIP_CTRL, SPDIF_EN, SPDIF_EN | DSD_EN | HBRA_ON);
		vWriteHdmiGRLMsk(AIP_CTRL, SPDIF_INTERNAL_MODULE, SPDIF_INTERNAL_MODULE);
		vWriteHdmiGRLMsk(AIP_CTRL, HBR_FROM_SPDIF, HBR_FROM_SPDIF);
		vWriteHdmiGRLMsk(AIP_CTRL, CTS_CAL_N4, CTS_CAL_N4);

		vWriteHdmiTOPCKMsk(0xe8, (0x1 << 11), (0x1 << 12) | (0x1 << 11));
	}
}

void vSetHDMIAudioIn(void)
{
	unsigned char bChMapping;

	HDMI_AUDIO_FUNC();

	vWriteByteHdmiGRL(TOP_AUD_MAP,
			  C_SD7 + C_SD6 + C_SD5 + C_SD4 + C_SD3 + C_SD2 + C_SD1 + C_SD0);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, 0, 0x0F << 20);
	vWriteHdmiGRLMsk(AIP_CTRL, 0, SPDIF_EN | DSD_EN | HBRA_ON |
			 CTS_CAL_N4 | HBR_FROM_SPDIF | SPDIF_INTERNAL_MODULE);
	vWriteHdmiGRLMsk(AIP_TXCTRL, 0, DSD_MUTE_DATA | LAYOUT1);

	if (_stAvdAVInfo.e_hdmi_aud_in == SV_I2S) {

		if (_stAvdAVInfo.e_aud_code == AVD_DSD) {
			vSetHdmiDsdConfig(_stAvdAVInfo.ui1_aud_out_ch_number, 0);
			vSetHdmiI2SChNum(_stAvdAVInfo.ui1_aud_out_ch_number, 1);
		} else {
			vSetHdmiI2SDataFmt(_stAvdAVInfo.e_I2sFmt);
			vSetHdmiI2SSckEdge(SCK_EDGE_RISE);
			vSetHdmiI2SCbitOrder(CBIT_ORDER_SAME);
			vSetHdmiI2SVbit(VBIT_PCM);
			vSetHdmiI2SDataDir(DATA_DIR_MSB);
			vEnableInputAudioType(SV_I2S);
			bChMapping = bGetChannelMapping();
			vSetHdmiI2SChNum(_stAvdAVInfo.ui1_aud_out_ch_number, bChMapping);
			vSetChannelSwap(LFE_CC_SWAP);
		}
	} else {
		if (((_stAvdAVInfo.e_hdmi_aud_in == SV_SPDIF)
		     && ((_stAvdAVInfo.e_aud_code == AVD_DTS_HD)
			 || (_stAvdAVInfo.e_aud_code == AVD_MAT_MLP))
		     && (_stAvdAVInfo.e_dsp_fs == FS_768K))) {
			vSetHdmiHbrConfig(FALSE);
		} else {
			vSetHdmiSpdifConfig();
			vSetHdmiI2SChNum(2, 0);
		}
	}
}

void vResetAudioHDMI(unsigned char bRst)
{
	if (bRst) {
		vWriteHdmiGRLMsk(AIP_TXCTRL, RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR,
				 RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR);
	} else {
		vWriteHdmiGRLMsk(AIP_TXCTRL, 0, RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR);
	}
}

void vResetVideoHDMI(unsigned char bRst)
{
	if (bRst) {
		/* SOFT_VIDEO_RST, SOFT_VIDEO_RST */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_VIDEO_RST, SOFT_VIDEO_RST);
#endif
	} else {
		/* SOFT_VIDEO_NOR, SOFT_VIDEO_NOR) */
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHDCPRST(SOFT_VIDEO_NOR, SOFT_VIDEO_RST);
#endif
	}
}

void vAudioNCTSOn(char bOn)
{
	if (bOn)
		vWriteHdmiGRLMsk(AIP_CTRL, CTS_REQ_EN, CTS_REQ_EN);
	else
		vWriteHdmiGRLMsk(AIP_CTRL, 0, CTS_REQ_EN);
}

void vChgHDMIAudioOutput(unsigned char ui1hdmifs, unsigned char ui1resindex,
			 unsigned char bdeepmode)
{
	unsigned int ui4Index;

	HDMI_AUDIO_FUNC();
	if (i4SharedInfo(SI_EDID_VSDB_EXIST) == FALSE)
		return;
	MuteHDMIAudio();
	vAudioPacketOff(TRUE);
	vResetAudioHDMI(TRUE);
	vAipCtrlInit();
	vSetHDMIAudioIn();

	vHDMI_I2S_C_Status();
	vSendAudioInfoFrame();
	vEnableAudio(TRUE);
	vResetAudioHDMI(FALSE);
	vHDMIAudioSRC(ui1hdmifs, ui1resindex, bdeepmode);

	for (ui4Index = 0; ui4Index < 5; ui4Index++)
		udelay(5);

	vHwNCTSOnOff(TRUE);

	if (_HdmiSinkAvCap.ui2_edid_chksum_and_audio_sup & SINK_BASIC_AUDIO_NO_SUP) {
		vAudioPacketOff(TRUE);
		vAudioNCTSOn(FALSE);
	} else {
		vAudioPacketOff(FALSE);
		if (_stAvdAVInfo.bMuteHdmiAudio == FALSE)	/* HDCP is OK and UI not set MUTE  */
			UnMuteHDMIAudio();
	}
}

void vSendAVIInfoFrame(unsigned char ui1resindex, unsigned char ui1colorspace)
{
	HDMI_VIDEO_FUNC();
	if (i4SharedInfo(SI_EDID_VSDB_EXIST) == FALSE)
		return;

	if (down_timeout(&color_space_mutex, msecs_to_jiffies(2000)))
		TX_DEF_LOG("can't get color_space_mutex in %s()\n", __func__);

	if (_fgDolbyHdrEnable)
		_bAviInfoFm[0] = 0x00;
	else if ((ui1colorspace == HDMI_YCBCR_444) || (ui1colorspace == HDMI_YCBCR_444_FULL)
		 || (ui1colorspace == HDMI_XV_YCC)) {
		_bAviInfoFm[0] = 0x40;
	} else if ((ui1colorspace == HDMI_YCBCR_422) || (ui1colorspace == HDMI_YCBCR_422_FULL)) {
		_bAviInfoFm[0] = 0x20;
	} else {
		_bAviInfoFm[0] = 0x00;
	}

	if (ui1colorspace == HDMI_YCBCR_420)
		_bAviInfoFm[0] = 0x60;

	if (!bResolution_4K2K(ui1resindex))
		_bAviInfoFm[0] |= 0x10;	/* A0=1, Active format (R0~R3) inf valid */
	HDMI_PLUG_LOG("vSendAVIInfoFrame cs = %d; Res = 0x%x; Infofm = 0x%x-------\n",
		      ui1colorspace, ui1resindex, _bAviInfoFm[0]);

	if ((_fgSendAVIXVYcc == TRUE) || (_fgBT2020Enable))	/* vfor xvYCC or BT2020 */
		_bAviInfoFm[1] = 0xc0;
	else
		_bAviInfoFm[1] = 0x0;

	if (ui1resindex == HDMI_VIDEO_1920x1080p3d_23Hz)
		ui1resindex = HDMI_VIDEO_1920x1080p_23Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080p3d_24Hz)
		ui1resindex = HDMI_VIDEO_1920x1080p_24Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080i3d_60Hz)
		ui1resindex = HDMI_VIDEO_1920x1080i_60Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080i3d_50Hz)
		ui1resindex = HDMI_VIDEO_1920x1080i_50Hz;
	else if (ui1resindex == HDMI_VIDEO_1280x720p3d_60Hz)
		ui1resindex = HDMI_VIDEO_1280x720p_60Hz;
	else if (ui1resindex == HDMI_VIDEO_1280x720p3d_50Hz)
		ui1resindex = HDMI_VIDEO_1280x720p_50Hz;


	if ((ui1resindex == HDMI_VIDEO_720x480p_60Hz) || (ui1resindex == HDMI_VIDEO_720x576p_50Hz))
		_bAviInfoFm[1] |= AV_INFO_SD_ITU601;
	else
		_bAviInfoFm[1] |= AV_INFO_HD_ITU709;

	_bAviInfoFm[1] |= 0x20;
	_bAviInfoFm[1] |= 0x08;
	_bAviInfoFm[2] = 0;	/* bData3 */
	/* _bAviInfoFm[2] |= 0x04; //limit Range */

	if (_fgSendAVIXVYcc == TRUE) {
		if ((ui1resindex == HDMI_VIDEO_720x480p_60Hz)
		    || (ui1resindex == HDMI_VIDEO_720x576p_50Hz))
			_bAviInfoFm[2] |= 0;	/* xvYcc601 */
		else
			_bAviInfoFm[2] |= 0x10;	/* xvYcc709 */
	}

	if (_fgBT2020Enable)
		_bAviInfoFm[2] |= 0x60;	/* bt2020 110 */

	if (_fgDolbyHdrEnable)
		_bAviInfoFm[2] |= 0x08;	/* FULL Range */
	else {
		if (_HdmiSinkAvCap.ui2_sink_vcdb_data & SINK_RGB_SELECTABLE) {
			if (ui1colorspace == HDMI_RGB)
				_bAviInfoFm[2] |= 0x04;	/* Limit Range */
			else if (ui1colorspace == HDMI_RGB_FULL)
				_bAviInfoFm[2] |= 0x08;	/* FULL Range */
		}
	}

	if (!bResolution_4K2K(ui1resindex))
		_bAviInfoFm[3] = HDMI_VIDEO_ID_CODE[ui1resindex];	/* bData4 */
	else if (ui1resindex == HDMI_VIDEO_3840x2160P_50HZ)
		_bAviInfoFm[3] = 96;
	else if ((ui1resindex == HDMI_VIDEO_3840x2160P_60HZ)
		|| (ui1resindex == HDMI_VIDEO_3840x2160P_59_94HZ))
		_bAviInfoFm[3] = 97;
	else if ((ui1resindex == HDMI_VIDEO_4096x2160P_60HZ)
		|| (ui1resindex == HDMI_VIDEO_4096x2160P_59_94HZ))
		_bAviInfoFm[3] = 102;
	else if (ui1resindex == HDMI_VIDEO_4096x2160P_50HZ)
		_bAviInfoFm[3] = 101;
	else
		_bAviInfoFm[3] = 0;

	if ((_bAviInfoFm[1] & AV_INFO_16_9_OUTPUT)
	    && ((ui1resindex == HDMI_VIDEO_720x480p_60Hz)
		|| (ui1resindex == HDMI_VIDEO_720x576p_50Hz))) {
		_bAviInfoFm[3] = _bAviInfoFm[3] + 1;
	}

	_bAviInfoFm[4] = 0x00;

	HDMI_PLUG_LOG("AVIInfoFm 0x%x; 0x%x; 0x%x 0x%x; -------\n", _bAviInfoFm[0], _bAviInfoFm[1],
		      _bAviInfoFm[2], _bAviInfoFm[3]);
	vHalSendAVIInfoFrame(&_bAviInfoFm[0]);
	up(&color_space_mutex);
}

void vSendVendorSpecificInfoFrame(unsigned char ui1resindex)
{
	unsigned char bResTableIndex, b3DStruct, bVic;
	unsigned char fg3DRes;

	HDMI_DRV_FUNC();

	if (i4SharedInfo(SI_EDID_VSDB_EXIST) == FALSE)
		return;

	fg3DRes = TRUE;

	if (ui1resindex == HDMI_VIDEO_1920x1080p3d_23Hz)
		ui1resindex = HDMI_VIDEO_1920x1080p_23Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080p3d_24Hz)
		ui1resindex = HDMI_VIDEO_1920x1080p_24Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080i3d_60Hz)
		ui1resindex = HDMI_VIDEO_1920x1080i_60Hz;
	else if (ui1resindex == HDMI_VIDEO_1920x1080i3d_50Hz)
		ui1resindex = HDMI_VIDEO_1920x1080i_50Hz;
	else if (ui1resindex == HDMI_VIDEO_1280x720p3d_60Hz)
		ui1resindex = HDMI_VIDEO_1280x720p_60Hz;
	else if (ui1resindex == HDMI_VIDEO_1280x720p3d_50Hz)
		ui1resindex = HDMI_VIDEO_1280x720p_50Hz;
	else
		fg3DRes = FALSE;

	b3DStruct = 0;
	bVic = 0;

	bResTableIndex = HDMI_VIDEO_ID_CODE[ui1resindex];	/* bData4 */

	if (fg3DRes == TRUE)
		vHalSendVendorSpecificInfoFrame(fg3DRes, bResTableIndex, b3DStruct,
						_fgDolbyHdrEnable);
	else if (bResolution_4K2K(ui1resindex)) {
		if (ui1resindex == HDMI_VIDEO_3840x2160P_29_97HZ)
			bVic = 1;
		else if (ui1resindex == HDMI_VIDEO_3840x2160P_30HZ)
			bVic = 1;
		else if (ui1resindex == HDMI_VIDEO_3840x2160P_25HZ)
			bVic = 2;
		else if (ui1resindex == HDMI_VIDEO_3840x2160P_23_976HZ)
			bVic = 3;
		else if (ui1resindex == HDMI_VIDEO_3840x2160P_24HZ)
			bVic = 3;
		else if (ui1resindex == HDMI_VIDEO_4096x2160P_24HZ)
			bVic = 4;

		if (bVic != 0)
			if ((_fgDolbyHdrEnable && fgUseDolbyVSIF()) || _fgLowLatencyDolbyVisionEnable)
				vHalSendVendorSpecificInfoFrame(0, 0, 0xff, 0);
			else if (_fgDolbyHdrEnable && (!fgUseDolbyVSIF()))
				vHalSendVendorSpecificInfoFrame(0, bVic, 0xff, 1);
			else
				vHalSendVendorSpecificInfoFrame(0, bVic, 0xff, 0);
		else /* 4k@50 60 */
			vHalSendVendorSpecificInfoFrame(0, 0, 0xff, _fgDolbyHdrEnable && (!fgUseDolbyVSIF()));
	} else
		vHalSendVendorSpecificInfoFrame(0, 0, 0xff, _fgDolbyHdrEnable && (!fgUseDolbyVSIF()));

}

void vShowHpdRsenStatus(void)
{
	if (bCheckPordHotPlug(HOTPLUG_MODE) == TRUE)
		HDMI_PLUG_LOG("HPD ON\n");
	else
		HDMI_PLUG_LOG("HPD OFF\n");

	if (bCheckPordHotPlug(PORD_MODE) == TRUE)
		HDMI_PLUG_LOG("RSEN ON\n");
	else
		HDMI_PLUG_LOG("RSEN OFF\n");

	if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_IN_ONLY)
		HDMI_PLUG_LOG("SI_HDMI_RECEIVER_STATUS = HDMI_PLUG_IN_ONLY\n");
	else if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_IN_AND_SINK_POWER_ON)
		HDMI_PLUG_LOG("SI_HDMI_RECEIVER_STATUS = HDMI_PLUG_IN_AND_SINK_POWER_ON\n");
	else if (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_OUT)
		HDMI_PLUG_LOG("SI_HDMI_RECEIVER_STATUS = HDMI_PLUG_OUT\n");
	else
		HDMI_PLUG_LOG("SI_HDMI_RECEIVER_STATUS error\n");


	if ((i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_IN_ONLY) ||
	    (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) == HDMI_PLUG_IN_AND_SINK_POWER_ON)) {

		if (hdcp2_version_flag == TRUE)
			HDMI_PLUG_LOG("support hdcp2.2\n");
		else
			HDMI_PLUG_LOG("support hdcp1.x\n");
	} else {
		HDMI_PLUG_LOG("hdmi not plugged in. hdcp version unknown\n");
	}


}

void vShowOutputVideoResolution(void)
{
	HDMI_PLUG_LOG("HDMI output resolution = %s\n", szHdmiResStr[_stAvdAVInfo.e_resolution]);	/*  */

}

void vShowDviOrHdmiMode(void)
{
	if (vIsDviMode())
		HDMI_PLUG_LOG("DVI Mode\n");
	else
		HDMI_PLUG_LOG("HDMI Mode\n");

}

void vShowDeepColor(void)
{

	if (_stAvdAVInfo.e_deep_color_bit == HDMI_NO_DEEP_COLOR)
		HDMI_PLUG_LOG("HDMI output deepcolor = HDMI_DEEP_COLOR_8_BIT\n");
	else if (_stAvdAVInfo.e_deep_color_bit == HDMI_DEEP_COLOR_10_BIT)
		HDMI_PLUG_LOG("HDMI output deepcolor = HDMI_DEEP_COLOR_10_BIT\n");
	else if (_stAvdAVInfo.e_deep_color_bit == HDMI_DEEP_COLOR_12_BIT)
		HDMI_PLUG_LOG("HDMI output deepcolor = HDMI_DEEP_COLOR_12_BIT\n");
	else if (_stAvdAVInfo.e_deep_color_bit == HDMI_DEEP_COLOR_16_BIT)
		HDMI_PLUG_LOG("HDMI output deepcolor = HDMI_DEEP_COLOR_16_BIT\n");
	else
		HDMI_PLUG_LOG("HDMI output deepcolor error\n");

}

void vShowColorSpace(void)
{
	if (_stAvdAVInfo.e_video_color_space == HDMI_RGB)
		HDMI_PLUG_LOG("HDMI output colorspace = HDMI_RGB\n");
	else if (_stAvdAVInfo.e_video_color_space == HDMI_RGB_FULL)
		HDMI_PLUG_LOG("HDMI output colorspace = HDMI_RGB_FULL\n");
	else if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444)
		HDMI_PLUG_LOG("HDMI output colorspace = HDMI_YCBCR_444\n");
	else if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422)
		HDMI_PLUG_LOG("[HDMI]HDMI output colorspace = HDMI_YCBCR_422\n");
	else if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
		HDMI_PLUG_LOG("[HDMI]HDMI output colorspace = HDMI_YCBCR_420\n");
	else if (_stAvdAVInfo.e_video_color_space == HDMI_XV_YCC)
		HDMI_PLUG_LOG("HDMI output colorspace = HDMI_XV_YCC\n");
	else
		HDMI_PLUG_LOG("HDMI output colorspace error\n");

}

void vShowInforFrame(void)
{
	HDMI_PLUG_LOG
	    ("====================Audio inforFrame Start ====================================\n");
	HDMI_PLUG_LOG("Data Byte (1~5) = 0x%x  0x%x  0x%x  0x%x  0x%x\n", _bAudInfoFm[0],
		      _bAudInfoFm[1], _bAudInfoFm[2], _bAudInfoFm[3], _bAudInfoFm[4]);
	HDMI_PLUG_LOG("CC2~ CC0: 0x%x, %s\n", _bAudInfoFm[0] & 0x07,
		      cAudChCountStr[_bAudInfoFm[0] & 0x07]);
	HDMI_PLUG_LOG("CT3~ CT0: 0x%x, %s\n", (_bAudInfoFm[0] >> 4) & 0x0f,
		      cAudCodingTypeStr[(_bAudInfoFm[0] >> 4) & 0x0f]);
	HDMI_PLUG_LOG("SS1, SS0: 0x%x, %s\n", _bAudInfoFm[1] & 0x03,
		      cAudSampleSizeStr[_bAudInfoFm[1] & 0x03]);
	HDMI_PLUG_LOG("SF2~ SF0: 0x%x, %s\n", (_bAudInfoFm[1] >> 2) & 0x07,
		      cAudFsStr[(_bAudInfoFm[1] >> 2) & 0x07]);
	HDMI_PLUG_LOG("CA7~ CA0: 0x%x, %s\n", _bAudInfoFm[3] & 0xff,
		      cAudChMapStr[_bAudInfoFm[3] & 0xff]);
	HDMI_PLUG_LOG("LSV3~LSV0: %d db\n", (_bAudInfoFm[4] >> 3) & 0x0f);
	HDMI_PLUG_LOG("DM_INH: 0x%x ,%s\n", (_bAudInfoFm[4] >> 7) & 0x01,
		      cAudDMINHStr[(_bAudInfoFm[4] >> 7) & 0x01]);
	HDMI_PLUG_LOG
	    ("====================Audio inforFrame End ======================================\n");

	HDMI_PLUG_LOG
	    ("====================AVI inforFrame Start ====================================\n");
	HDMI_PLUG_LOG("Data Byte (1~5) = 0x%x  0x%x  0x%x  0x%x  0x%x\n", _bAviInfoFm[0],
		      _bAviInfoFm[1], _bAviInfoFm[2], _bAviInfoFm[3], _bAviInfoFm[4]);
	HDMI_PLUG_LOG("S1,S0: 0x%x, %s\n", _bAviInfoFm[0] & 0x03,
		      cAviScanStr[_bAviInfoFm[0] & 0x03]);
	HDMI_PLUG_LOG("B1,S0: 0x%x, %s\n", (_bAviInfoFm[0] >> 2) & 0x03,
		      cAviBarStr[(_bAviInfoFm[0] >> 2) & 0x03]);
	HDMI_PLUG_LOG("A0: 0x%x, %s\n", (_bAviInfoFm[0] >> 4) & 0x01,
		      cAviActivePresentStr[(_bAviInfoFm[0] >> 4) & 0x01]);
	HDMI_PLUG_LOG("Y1,Y0: 0x%x, %s\n", (_bAviInfoFm[0] >> 5) & 0x03,
		      cAviRgbYcbcrStr[(_bAviInfoFm[0] >> 5) & 0x03]);
	HDMI_PLUG_LOG("R3~R0: 0x%x, %s\n", (_bAviInfoFm[1]) & 0x0f,
		      cAviActiveStr[(_bAviInfoFm[1]) & 0x0f]);
	HDMI_PLUG_LOG("M1,M0: 0x%x, %s\n", (_bAviInfoFm[1] >> 4) & 0x03,
		      cAviAspectStr[(_bAviInfoFm[1] >> 4) & 0x03]);
	HDMI_PLUG_LOG("C1,C0: 0x%x, %s\n", (_bAviInfoFm[1] >> 6) & 0x03,
		      cAviColorimetryStr[(_bAviInfoFm[1] >> 6) & 0x03]);
	HDMI_PLUG_LOG("SC1,SC0: 0x%x, %s\n", (_bAviInfoFm[2]) & 0x03,
		      cAviScaleStr[(_bAviInfoFm[2]) & 0x03]);
	HDMI_PLUG_LOG("Q1,Q0: 0x%x, %s\n", (_bAviInfoFm[2] >> 2) & 0x03,
		      cAviRGBRangeStr[(_bAviInfoFm[2] >> 2) & 0x03]);
	if (((_bAviInfoFm[2] >> 4) & 0x07) <= 1)
		HDMI_PLUG_LOG("EC2~EC0: 0x%x, %s\n", (_bAviInfoFm[2] >> 4) & 0x07,
			      cAviExtColorimetryStr[(_bAviInfoFm[2] >> 4) & 0x07]);
	else
		HDMI_PLUG_LOG("EC2~EC0: resevered\n");
	HDMI_PLUG_LOG("ITC: 0x%x, %s\n", (_bAviInfoFm[2] >> 7) & 0x01,
		      cAviItContentStr[(_bAviInfoFm[2] >> 7) & 0x01]);
	HDMI_PLUG_LOG
	    ("====================AVI inforFrame End ======================================\n");

	HDMI_PLUG_LOG
	    ("====================SPD inforFrame Start ====================================\n");
	HDMI_PLUG_LOG("Data Byte (1~8)  = 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
		      _bSpdInf[0], _bSpdInf[1], _bSpdInf[2], _bSpdInf[3], _bSpdInf[4], _bSpdInf[5],
		      _bSpdInf[6], _bSpdInf[7]);
	HDMI_PLUG_LOG("Data Byte (9~16) = 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
		      _bSpdInf[8], _bSpdInf[9], _bSpdInf[10], _bSpdInf[11], _bSpdInf[12],
		      _bSpdInf[13], _bSpdInf[14], _bSpdInf[15]);
	HDMI_PLUG_LOG("Data Byte (17~24)= 0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x  0x%x\n",
		      _bSpdInf[16], _bSpdInf[17], _bSpdInf[18], _bSpdInf[19], _bSpdInf[20],
		      _bSpdInf[21], _bSpdInf[22], _bSpdInf[23]);
	HDMI_PLUG_LOG("Data Byte  25    = 0x%x\n", _bSpdInf[24]);
	HDMI_PLUG_LOG("Source Device information is %s\n", cSPDDeviceStr[_bSpdInf[24]]);
	HDMI_PLUG_LOG
	    ("====================SPD inforFrame End ======================================\n");
}

unsigned int u4ReadNValue(void)
{
	return _u4NValue;
}

unsigned int u4ReadCtsValue(void)
{
	unsigned int u4Data;

	u4Data = bReadByteHdmiGRL(AIP_STA01) & CTS_VAL_HW;
	return u4Data;
}

void vShowHdmiAudioStatus(void)
{
	HDMI_PLUG_LOG("HDMI output audio Channel Number =%d\n",
		      _stAvdAVInfo.ui1_aud_out_ch_number);
	HDMI_PLUG_LOG("HDMI output Audio Fs = %s\n", cHdmiAudFsStr[_stAvdAVInfo.e_hdmi_fs]);
	HDMI_PLUG_LOG("HDMI MCLK =%d\n", _stAvdAVInfo.u1HdmiI2sMclk);
	HDMI_PLUG_LOG("HDMI output ACR N= %d, CTS = %d\n", u4ReadNValue(), u4ReadCtsValue());
}

void vShowHdmiDrmHdcpStatus(void)
{
	HDMI_PLUG_LOG("DrmHdcp _bNeedSwHdcp =%d\n", _bHdcpOff);
	HDMI_PLUG_LOG("DrmHdcp _bHdcpOff =%d\n", _bHdcpOff);
}

void vCheckHDMICLKPIN(void)
{
	unsigned int u4Data, i;

	vWriteHdmiTOPCK(0x214, 0xffffff00);

	for (i = 0; i < 2; i++) {
		vWriteHdmiTOPCK(0x100, 0xf00);
		vWriteHdmiTOPCK(0x220, 0x81);
		/* while(dReadHdmiTOPCK(0x220)&0x1); */
		msleep(60);
		u4Data = ((dReadHdmiTOPCK(0x224) & (0x0000ffff))) * 26;
		HDMI_PLUG_LOG("sys clk = %d.%dM\n", (u4Data / 1024),
			      (u4Data - ((u4Data / 1024) * 1024)));
	}

	for (i = 0; i < 2; i++) {
		vWriteHdmiTOPCK(0x100, 0x1400);
		vWriteHdmiTOPCK(0x220, 0x81);
		/* while(dReadHdmiTOPCK(0x220)&0x1); */
		msleep(60);
		u4Data = ((dReadHdmiTOPCK(0x224) & (0x0000ffff))) * 26;
		HDMI_PLUG_LOG("tvd clk = %d.%dM\n", (u4Data / 1024),
			      (u4Data - ((u4Data / 1024) * 1024)));
	}

	for (i = 0; i < 2; i++) {
		vWriteHdmiTOPCK(0x100, 0x2100);
		vWriteHdmiTOPCK(0x220, 0x81);
		/* while(dReadHdmiTOPCK(0x220)&0x1); */
		msleep(60);
		u4Data = ((dReadHdmiTOPCK(0x224) & (0x0000ffff))) * 26 * 4;
		HDMI_PLUG_LOG("hdmi pin clk = %d.%dM\n", (u4Data / 1024),
			      (u4Data - ((u4Data / 1024) * 1024)));
	}

}

void hdmi_hdmistatus(void)
{
	vShowHpdRsenStatus();
	vShowOutputVideoResolution();
	vShowDviOrHdmiMode();
	vShowDeepColor();
	vShowColorSpace();
	vShowInforFrame();
	vShowHdmiAudioStatus();
	vShowHdmiDrmHdcpStatus();
	vShowEdidRawData();
	vShowEdidInformation();
	vShowHdcpStatus();
	vshow_hdr_status();
}

unsigned int hdmi_check_status(void)
{
	unsigned int tmp;

	tmp = 0;
	if (hdmi_check_hdcp_key() == 1)
		tmp |= (1 << 0);
	if (hdmi_check_hdcp_state() == 1)
		tmp |= (1 << 1);
	return tmp;
}

/*****************************************************************************/
/*****************************************************************************/
/********************the second hdmi driver as below******************************/
/*****************************************************************************/
/*****************************************************************************/

void vBlackHDMIOnly(void)
{
	HDMI_DRV_FUNC();
	vWriteHdmiGRLMsk(TOP_VMUTE_CFG1, REG_VMUTE_EN, REG_VMUTE_EN);
}

void MuteHDMI2Audio(void)
{
	vWriteHdmiGRLMsk(HDCP_TOP_CTRL, OTPAMUTEOVR_SET, OTPAMUTEOVR_SET | OTPADROPOVR_SET);
	vWriteHdmiGRLMsk(HDCP_TOP_CTRL, OTP2XAOVR_EN | OTP14AOVR_EN, OTP2XAOVR_EN | OTP14AOVR_EN);
}

void vUnBlackHDMIOnly(void)
{
	HDMI_DRV_FUNC();

	vWriteHdmiGRLMsk(TOP_VMUTE_CFG1, 0, REG_VMUTE_EN);
}


void UnMuteHDMIAudio(void)
{
	vWriteHdmiGRLMsk(AIP_TXCTRL, 0, AUD_MUTE_FIFO_EN);
}

unsigned char vAudioPllSelect(void)
{
	if ((_stAvdAVInfo.e_iec_frame == IEC_44K)
	    || (_stAvdAVInfo.e_iec_frame == IEC_88K)
	    || (_stAvdAVInfo.e_iec_frame == IEC_176K)
	    || (_stAvdAVInfo.e_iec_frame == IEC_22K)) {
		return 1;	/*apll1 */
	} else
		return 2;	/*apll2 */
}

void vConfigHdmi2SYS(unsigned char bResIndex)
{



}

void vSetHDMITxPLL(unsigned int bResIndex, unsigned int bClockRate)
{
	unsigned char u4Feq = 0;
	unsigned int u4Data;
	/*vSetPLLGP_HDMIDDS_14_20_Select(bResIndex,1);  pmx setting */
	HDMI_PLL_LOG("bResIndex = %d, bClockRate = %d\n", bResIndex, bClockRate);
	HDMI_DRV_FUNC();

	switch (bResIndex) {
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x576i_50Hz:
	case HDMI_VIDEO_720x480p_60Hz:
	case HDMI_VIDEO_720x576p_50Hz:
		u4Feq = 0;	/* 27M */
		break;

	case HDMI_VIDEO_1280x720p_60Hz:
	case HDMI_VIDEO_1280x720p_59_94Hz:
	case HDMI_VIDEO_1280x720p_50Hz:
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_30Hz:
	case HDMI_VIDEO_1920x1080p_25Hz:
	case HDMI_VIDEO_1920x1080p_24Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
	case HDMI_VIDEO_1920x1080p_29Hz:
		u4Feq = 1;	/* 74M */
		break;
	case HDMI_VIDEO_1920x1080p_60Hz:
	case HDMI_VIDEO_1920x1080p_59_94Hz:
	case HDMI_VIDEO_1920x1080p_50Hz:
		u4Feq = 2;	/* 148M */
		break;
	case HDMI_VIDEO_1280x720p3d_60Hz:	/* 59.4*2=118.8M pixel clock */
	case HDMI_VIDEO_1280x720p3d_50Hz:
	case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080i3d_50Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
	case HDMI_VIDEO_1920x1080p3d_23Hz:
		u4Feq = 2;	/* 148M */
		break;
	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_24HZ:
	case HDMI_VIDEO_3840x2160P_25HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:
	case HDMI_VIDEO_3840x2160P_30HZ:
	case HDMI_VIDEO_4096x2160P_24HZ:
		u4Feq = 3;	/* 297M */
		break;
	case HDMI_VIDEO_3840x2160P_60HZ:
	case HDMI_VIDEO_3840x2160P_59_94HZ:
	case HDMI_VIDEO_3840x2160P_50HZ:
	case HDMI_VIDEO_4096x2160P_60HZ:
	case HDMI_VIDEO_4096x2160P_59_94HZ:
	case HDMI_VIDEO_4096x2160P_50HZ:
		u4Feq = 3;	/* 297M */
		if (!(_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420))
			bClockRate = TMDS_CLK_X2;
		break;

	default:
		break;
	}

	HDMI_PLL_LOG("u4Feq = %d, bClockRate = %d\n", u4Feq, bClockRate);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0,
		       ((BAND[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_BAND_SHIFT),
		       RG_HDMITX20PLL_BAND);

	u4Data = dReadIoHdmiAna(HDMI20_PLL_CFG_1);
	HDMI_PLUG_LOG("HDMI20_PLL_CFG_1 11702044 = 0x%x\n", u4Data);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       ((HTPLLBC[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_BC_SHIFT),
		       RG_HDMITX20PLL_BC);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       ((HTPLLBP[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_BP_SHIFT),
		       RG_HDMITX20PLL_BP);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       ((HTPLLBP[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_BP2_SHIFT),
		       RG_HDMITX20PLL_BP2);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       ((HTPLLBR[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_BR_SHIFT),
		       RG_HDMITX20PLL_BR);

	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       (HTPLLIC[u4Feq][bClockRate - 1] << RG_HDMITX20PLL_IC_SHIFT),
		       RG_HDMITX20PLL_IC);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_1,
		       (HTPLLIR[u4Feq][bClockRate - 1] << RG_HDMITX20PLL_IR_SHIFT),
		       RG_HDMITX20PLL_IR);
	u4Data = dReadIoHdmiAna(HDMI20_PLL_CFG_1);
	HDMI_PLUG_LOG("111HDMI20_PLL_CFG_1 11702044 = 0x%x\n", u4Data);

	u4Data = dReadIoHdmiAna(HDMI20_PLL_CFG_0);
	HDMI_PLUG_LOG("HDMI20_PLL_CFG_0 11702040 = 0x%x\n", u4Data);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0,
		       ((FBKDIV[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_FBKDIV_SHIFT),
		       RG_HDMITX20PLL_FBKDIV);
	u4Data = dReadIoHdmiAna(HDMI20_PLL_CFG_0);
	HDMI_PLUG_LOG("111HDMI20_PLL_CFG_0 11702040 = 0x%x\n", u4Data);


	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0,
		       ((FBKSEL[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_FBKSEL_SHIFT),
		       RG_HDMITX20PLL_FBKSEL);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_8,
		       ((TXDIV[u4Feq][bClockRate - 1]) << RG_HDMITX20_TX_POSDIV_SHIFT),
		       RG_HDMITX20_TX_POSDIV);
	HDMI_PLUG_LOG("[efuseValue] 0x%x\n", efuseValue0);
	if (efuseValue0 != 0)
	vWriteIoHdmiAnaMsk(HDMI20_CFG_8,
		       (efuseValue0 << RG_HDMITX20_INTR_CAL_SHIFT),
		       RG_HDMITX20_INTR_CAL);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0,
		       ((PREDIV[u4Feq][bClockRate - 1]) << RG_HDMITX20PLL_PREDIV_SHIFT),
		       RG_HDMITX20PLL_PREDIV);

	if ((u4Feq == 3) && (bClockRate != TMDS_CLK_X1))
		vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, 0, RG_HDMITX20PLL_POSDIV);
	else
		vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, RG_HDMITX20PLL_POSDIV, RG_HDMITX20PLL_POSDIV);

	if ((u4Feq == 3) && (bClockRate != TMDS_CLK_X1)) { /*3G~6G*/
		HDMI_PLUG_LOG("[hdmi]3G~6G\n");
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x2 << RG_HDMITX20_PRED_IMP_CLK_SHIFT,
			       RG_HDMITX20_PRED_IMP_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x2 << RG_HDMITX20_PRED_IMP_D0_SHIFT,
			       RG_HDMITX20_PRED_IMP_D0);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x2 << RG_HDMITX20_PRED_IMP_D1_SHIFT,
			       RG_HDMITX20_PRED_IMP_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x2 << RG_HDMITX20_PRED_IMP_D2_SHIFT,
			       RG_HDMITX20_PRED_IMP_D2);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0xc << RG_HDMITX20_PRD_IBIAS_CLK_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0xc << RG_HDMITX20_PRD_IBIAS_D2_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0xc << RG_HDMITX20_PRD_IBIAS_D1_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0xc << RG_HDMITX20_PRD_IBIAS_D0_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D0);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, (0xf << RG_HDMITX20_DRV_IMP_EN_SHIFT),
			       RG_HDMITX20_DRV_IMP_EN);

		if (bClockRate == TMDS_CLK_X1_25) {
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1a << RG_HDMITX20_DRV_IBIAS_CLK_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_CLK);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1c << RG_HDMITX20_DRV_IBIAS_D2_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D2);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1c << RG_HDMITX20_DRV_IBIAS_D1_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D1);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1c << RG_HDMITX20_DRV_IBIAS_D0_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D0);
		} else {
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1a << RG_HDMITX20_DRV_IBIAS_CLK_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_CLK);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1e << RG_HDMITX20_DRV_IBIAS_D2_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D2);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1e << RG_HDMITX20_DRV_IBIAS_D1_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D1);
			vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1e << RG_HDMITX20_DRV_IBIAS_D0_SHIFT),
				       RG_HDMITX20_DRV_IBIAS_D0);
		}

		HDMI_PLUG_LOG("[efuseValue6G] 0x%x, 0x%x, 0x%x, 0x%x\n",
			efuseValue1, efuseValue2, efuseValue3, efuseValue4);
		if ((efuseValue1 != 0) && (efuseValue2 != 0) && (efuseValue3 != 0) && (efuseValue4 != 0)) {
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue4 << RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue3 << RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue2 << RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue1 << RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN1);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue4 << RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue3 << RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue2 << RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue1 << RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN2);
			} else {
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x36 << RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN1);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x36 << RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN2);
		}
	} else if (((u4Feq == 2) && (bClockRate != TMDS_CLK_X1))/*1.65G~3G*/
		   || ((u4Feq == 3) && (bClockRate == TMDS_CLK_X1))) {
		HDMI_PLUG_LOG("[hdmi]1.65G~3G\n");
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x1 << RG_HDMITX20_PRED_IMP_CLK_SHIFT,
			       RG_HDMITX20_PRED_IMP_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x1 << RG_HDMITX20_PRED_IMP_D0_SHIFT,
			       RG_HDMITX20_PRED_IMP_D0);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x1 << RG_HDMITX20_PRED_IMP_D1_SHIFT,
			       RG_HDMITX20_PRED_IMP_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x1 << RG_HDMITX20_PRED_IMP_D2_SHIFT,
			       RG_HDMITX20_PRED_IMP_D2);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x8 << RG_HDMITX20_PRD_IBIAS_CLK_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x8 << RG_HDMITX20_PRD_IBIAS_D2_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x8 << RG_HDMITX20_PRD_IBIAS_D1_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x8 << RG_HDMITX20_PRD_IBIAS_D0_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D0);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, (0xf << RG_HDMITX20_DRV_IMP_EN_SHIFT),
			       RG_HDMITX20_DRV_IMP_EN);

		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x16 << RG_HDMITX20_DRV_IBIAS_CLK_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_CLK);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1a << RG_HDMITX20_DRV_IBIAS_D2_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D2);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1a << RG_HDMITX20_DRV_IBIAS_D1_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D1);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x1a << RG_HDMITX20_DRV_IBIAS_D0_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D0);

		HDMI_PLUG_LOG("[efuseValue3G] 0x%x, 0x%x, 0x%x, 0x%x\n",
			efuseValue1, efuseValue2, efuseValue3, efuseValue4);
		if ((efuseValue1 != 0) && (efuseValue2 != 0) && (efuseValue3 != 0) && (efuseValue4 != 0)) {
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue4 << RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue3 << RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue2 << RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (efuseValue1 << RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN1);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue4 << RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue3 << RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue2 << RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (efuseValue1 << RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN2);
			} else {
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x36 << RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0x31 << RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN1);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x36 << RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0x31 << RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN2);
		}
	} else { /*<1.65G*/
		HDMI_PLUG_LOG("[hdmi]<1.65G\n");
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x0 << RG_HDMITX20_PRED_IMP_CLK_SHIFT,
			       RG_HDMITX20_PRED_IMP_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x0 << RG_HDMITX20_PRED_IMP_D0_SHIFT,
			       RG_HDMITX20_PRED_IMP_D0);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x0 << RG_HDMITX20_PRED_IMP_D1_SHIFT,
			       RG_HDMITX20_PRED_IMP_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0x0 << RG_HDMITX20_PRED_IMP_D2_SHIFT,
			       RG_HDMITX20_PRED_IMP_D2);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x3 << RG_HDMITX20_PRD_IBIAS_CLK_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_CLK);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x3 << RG_HDMITX20_PRD_IBIAS_D2_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x3 << RG_HDMITX20_PRD_IBIAS_D1_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_1, (0x3 << RG_HDMITX20_PRD_IBIAS_D0_SHIFT),
			       RG_HDMITX20_PRD_IBIAS_D0);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, (0x0 << RG_HDMITX20_DRV_IMP_EN_SHIFT),
			       RG_HDMITX20_DRV_IMP_EN);

		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x10 << RG_HDMITX20_DRV_IBIAS_CLK_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_CLK);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x10 << RG_HDMITX20_DRV_IBIAS_D2_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D2);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x10 << RG_HDMITX20_DRV_IBIAS_D1_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D1);
		vWriteIoHdmiAnaMsk(HDM20_CFG_2, (0x10 << RG_HDMITX20_DRV_IBIAS_D0_SHIFT),
			       RG_HDMITX20_DRV_IBIAS_D0);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0 << RG_HDMITX20_DRV_IMP_CLK_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0 << RG_HDMITX20_DRV_IMP_D2_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0 << RG_HDMITX20_DRV_IMP_D1_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN1);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, (0 << RG_HDMITX20_DRV_IMP_D0_EN1_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN1);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0 << RG_HDMITX20_DRV_IMP_CLK_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_CLK_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0 << RG_HDMITX20_DRV_IMP_D2_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D2_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0 << RG_HDMITX20_DRV_IMP_D1_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D1_EN2);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, (0 << RG_HDMITX20_DRV_IMP_D0_EN2_SHIFT),
			       RG_HDMITX20_DRV_IMP_D0_EN2);

	}

}

void vChangeVpll(unsigned int bRes, unsigned int bdeepmode)
{
	unsigned int bClockRate = 0;

	HDMI_PLL_FUNC();

	if (bdeepmode == HDMI_DEEP_COLOR_16_BIT)
		bClockRate = TMDS_CLK_X2;
	else if (bdeepmode == HDMI_DEEP_COLOR_12_BIT)
		bClockRate = TMDS_CLK_X1_5;
	else if (bdeepmode == HDMI_DEEP_COLOR_10_BIT)
		bClockRate = TMDS_CLK_X1_25;
	else if (bdeepmode == HDMI_NO_DEEP_COLOR)	/* No deep color */
		bClockRate = TMDS_CLK_X1;
	else
		bClockRate = TMDS_CLK_X1;

	vSetHDMITxPLL(bRes, bClockRate);	/* set PLL */
}

void vEnableHdmiMode(char bOn)
{
	if (bOn == 1)
		vWriteHdmiGRLMsk(TOP_CFG00, HDMI_MODE_HDMI, HDMI_MODE_HDMI);
	else
		vWriteHdmiGRLMsk(TOP_CFG00, HDMI_MODE_DVI, HDMI_MODE_HDMI);

}

void vResetAudioHDMI2(unsigned char bRst)
{
	if (bRst) {
		vWriteHdmiGRLMsk(AIP_TXCTRL, RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR,
				 RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR);
	} else {
		vWriteHdmiGRLMsk(AIP_TXCTRL, 0, RST4AUDIO | RST4AUDIO_FIFO | RST4AUDIO_ACR);
	}
}

void vAipCtrlInit(void)
{
	vWriteHdmiGRLMsk(AIP_CTRL, AUD_SEL_OWRT | NO_MCLK_CTSGEN_SEL | CTS_REQ_EN,
			 AUD_SEL_OWRT | NO_MCLK_CTSGEN_SEL | MCLK_EN | CTS_REQ_EN);
	vWriteHdmiGRLMsk(AIP_TPI_CTRL, TPI_AUDIO_LOOKUP_DIS, TPI_AUDIO_LOOKUP_EN);
}

void vSetHdmi2I2SDataFmt(unsigned int bLength)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bLength << I2S_IN_LENGTH_SHIFT, I2S_IN_LENGTH);
}

void vSetHdmi2I2SSckEdge(unsigned int bEdge)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bEdge, SCK_EDGE_RISE);
}

void vSetHdmi2I2SCbitOrder(unsigned int bCbit)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bCbit, CBIT_ORDER_SAME);
}

void vSetHdmi2I2SVbit(unsigned int bVbit)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bVbit, VBIT_COM);
}

void vSetHdmi2I2SWS(unsigned int bWS)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bWS, WS_HIGH);
}

void vSetHdmi2I2SJustify(unsigned int bJustify)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bJustify, JUSTIFY_RIGHT);
}

void vSetHdmi2I2SDataDir(unsigned int bDataDir)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bDataDir, DATA_DIR_LSB);
}

void vSetHdmi2I2S1stbit(unsigned int b1stbit)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, b1stbit, I2S_1ST_BIT_NOSHIFT);
}

void vSetHdmi2I2SfifoMap(unsigned int bFifoMap)
{
	vWriteHdmiGRLMsk(AIP_I2S_CTRL, bFifoMap, FIFO3_MAP | FIFO2_MAP | FIFO1_MAP | FIFO0_MAP);
}

void vSetHdmi2I2SCH(unsigned int bCH)
{
	vWriteHdmiGRLMsk(AIP_CTRL, bCH << I2S_EN_SHIFT, I2S_EN);
}

void vEnableInputAudioType(unsigned int bspdifi2s)
{
	vWriteHdmiGRLMsk(AIP_CTRL, bspdifi2s << SPDIF_EN_SHIFT, SPDIF_EN);
}

void vSetHdmiI2SChNum(unsigned char bChNum, unsigned char bChMapping)
{
	unsigned int bData, bData1, bData2, bData3;

	if (bChNum == 2) {	/* I2S 2ch */
		bData = 0x1;	/* 2ch data */
		bData1 = 0x50;	/* data0 */


	} else if ((bChNum == 3) || (bChNum == 4)) {	/* I2S 2ch */
		if ((bChNum == 4) && (bChMapping == 0x08))
			bData = 0x3;	/* 4ch data */
		else
			bData = 0x03;	/* 4ch data */

		bData1 = 0x50;	/* data0 */


	} else if ((bChNum == 6) || (bChNum == 5)) {	/* I2S 5.1ch */
		if ((bChNum == 6) && (bChMapping == 0x0E)) {
			bData = 0xf;	/* 6.0 ch data */
			bData1 = 0x50;	/* data0 */
		} else {
			bData = 0x7;	/* 5.1ch data, 5/0ch */
			bData1 = 0x50;	/* data0 */
		}


	} else if (bChNum == 8) {	/* I2S 5.1ch */
		bData = 0xf;	/* 7.1ch data */
		bData1 = 0x50;	/* data0 */
	} else if (bChNum == 7) {	/* I2S 6.1ch */
		bData = 0xf;	/* 6.1ch data */
		bData1 = 0x50;	/* data0 */
	} else {
		bData = 0x01;	/* 2ch data */
		bData1 = 0x50;	/* data0 */
	}

	bData2 = 0xc6;
	bData3 = 0xfa;

	/* vWriteByteHdmiGRL(GRL_CH_SW0, bData1); */
	/* vWriteByteHdmiGRL(GRL_CH_SW1, bData2); */
	/* vWriteByteHdmiGRL(GRL_CH_SW2, bData3); */
	/* vWriteByteHdmiGRL(GRL_I2S_UV, bData); */

	vSetHdmi2I2SfifoMap((MAP_SD3 << 6) | (MAP_SD2 << 4) | (MAP_SD1 << 2) | (MAP_SD0 << 0));
	vSetHdmi2I2SCH(bData);

	if (bChNum == 2)
		vWriteHdmiGRLMsk(AIP_TXCTRL, LAYOUT0, LAYOUT1);
	else
		vWriteHdmiGRLMsk(AIP_TXCTRL, LAYOUT1, LAYOUT1);

}

void vSetHdmiSpdifConfig(void)
{
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, WR_1UI_UNLOCK, WR_1UI_LOCK);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, FS_UNOVERRIDE, FS_OVERRIDE_WRITE);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, WR_2UI_UNLOCK, WR_2UI_LOCK);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, 0x4 << MAX_1UI_WRITE_SHIFT, MAX_1UI_WRITE);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, 0x9 << MAX_2UI_WRITE_SHIFT, MAX_2UI_WRITE);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, 0x4 << AUD_ERR_THRESH_SHIFT, AUD_ERR_THRESH);
	vWriteHdmiGRLMsk(AIP_SPDIF_CTRL, I2S2DSD_EN, I2S2DSD_EN);
}

void vHwSet_Hdmi_I2S_C_Status(unsigned char *prLChData, unsigned char *prRChData)
{
	vWriteByteHdmiGRL(AIP_I2S_CHST0,
			  (prLChData[3] << 24) + (prLChData[2] << 16) + (prLChData[1] << 8) +
			  prLChData[0]);
	vWriteByteHdmiGRL(AIP_I2S_CHST1, prLChData[4]);
}

void vHDMI_I2S_C_Status(void)
{
	unsigned char bData = 0;
	unsigned char bhdmi_RCh_status[5];
	unsigned char bhdmi_LCh_status[5];


	bhdmi_LCh_status[0] = _stAvdAVInfo.bhdmiLChstatus[0];
	bhdmi_LCh_status[1] = _stAvdAVInfo.bhdmiLChstatus[1];
	bhdmi_LCh_status[2] = _stAvdAVInfo.bhdmiLChstatus[2];
	bhdmi_RCh_status[0] = _stAvdAVInfo.bhdmiRChstatus[0];
	bhdmi_RCh_status[1] = _stAvdAVInfo.bhdmiRChstatus[1];
	bhdmi_RCh_status[2] = _stAvdAVInfo.bhdmiRChstatus[2];

	bData = _stAvdAVInfo.bhdmiLChstatus[3] & 0xf0;

	switch (_stAvdAVInfo.e_hdmi_fs) {
	case HDMI_FS_32K:
		bData |= 0x03;
		break;
	case HDMI_FS_44K:
		break;
	case HDMI_FS_88K:
		bData |= 0x08;
		break;
	case HDMI_FS_96K:
		bData |= 0x0A;
		break;
	case HDMI_FS_176K:
		bData |= 0x0C;
		break;
	case HDMI_FS_192K:
		bData |= 0x0E;
		break;
	case HDMI_FS_48K:
	default:
		bData |= 0x02;
		break;

	}


	bhdmi_LCh_status[3] = bData;
	bhdmi_RCh_status[3] = bData;

	bData = _stAvdAVInfo.bhdmiLChstatus[4];

	bData |= ((~(bhdmi_LCh_status[3] & 0x0f)) << 4);

	bhdmi_LCh_status[4] = bData;
	bhdmi_RCh_status[4] = bData;

	vHwSet_Hdmi_I2S_C_Status(&bhdmi_LCh_status[0], &bhdmi_RCh_status[0]);


}

void vHalSendAudioInfoFrame(unsigned char bData1, unsigned char bData2, unsigned char bData4,
			    unsigned char bData5)
{
	unsigned char bAUDIO_CHSUM;
	unsigned int bData = 0, bData3 = 0;

	bAUDIO_CHSUM = AUDIO_TYPE + AUDIO_VERS + AUDIO_LEN;

	bAUDIO_CHSUM += bData1;
	bAUDIO_CHSUM += bData2;
	bAUDIO_CHSUM += bData4;
	bAUDIO_CHSUM += bData5;

	bAUDIO_CHSUM = 0x100 - bAUDIO_CHSUM;

	vWriteHdmiGRLMsk(TOP_INFO_EN, AUD_DIS, AUD_EN);

	vWriteByteHdmiGRL(TOP_AIF_HEADER, (AUDIO_LEN << 16) + (AUDIO_VERS << 8) + AUDIO_TYPE);
	vWriteByteHdmiGRL(TOP_AIF_PKT00,
			  (bData3 << 24) + (bData2 << 16) + (bData1 << 8) + bAUDIO_CHSUM);
	vWriteByteHdmiGRL(TOP_AIF_PKT01, (bData5 << 8) + bData4);
	vWriteByteHdmiGRL(TOP_AIF_PKT02, 0x00000000);
	vWriteByteHdmiGRL(TOP_AIF_PKT03, 0x00000000);

	bData = bReadByteHdmiGRL(TOP_INFO_EN);
	bData |= AUD_EN;
	vWriteByteHdmiGRL(TOP_INFO_EN, bData);

	bData = bReadByteHdmiGRL(TOP_INFO_RPT);
	bData |= AUD_RPT_EN;
	vWriteByteHdmiGRL(TOP_INFO_RPT, bData);

	bData = bReadByteHdmiGRL(TOP_INFO_EN);
	bData |= AUD_EN_WR;
	vWriteByteHdmiGRL(TOP_INFO_EN, bData);

}

void vSendAudioInfoFrame(void)
{
	HDMI_AUDIO_FUNC();
	if (i4SharedInfo(SI_EDID_VSDB_EXIST) == FALSE)
		return;

	if (_stAvdAVInfo.e_hdmi_aud_in == SV_SPDIF) {
		_bAudInfoFm[0] = 0x00;	/* CC as 0, */
		_bAudInfoFm[3] = 0x00;	/* CA 2ch */
	} else {		/* pcm */

		switch (_stAvdAVInfo.ui2_aud_out_ch.word & 0x7fb) {
		case 0x03:	/* FL/FR */
			_bAudInfoFm[0] = 0x01;
			_bAudInfoFm[3] = 0x00;
			break;

		case 0x0b:	/* FL/FR/FC */
			_bAudInfoFm[0] = 0x02;
			_bAudInfoFm[3] = 0x02;
			break;

		case 0x13:	/* FL/FR/RC */
			_bAudInfoFm[0] = 0x02;
			_bAudInfoFm[3] = 0x04;
			break;

		case 0x1b:	/* FL/FR/FC/RC */
			_bAudInfoFm[0] = 0x03;
			_bAudInfoFm[3] = 0x06;
			break;

		case 0x33:	/* FL/FR/RL/RR */
			_bAudInfoFm[0] = 0x03;
			_bAudInfoFm[3] = 0x08;
			break;

		case 0x3b:	/* FL/FR/FC/RL/RR */
			_bAudInfoFm[0] = 0x04;
			_bAudInfoFm[3] = 0x0A;
			break;

		case 0x73:	/* FL/FR/RL/RR/RC */
			_bAudInfoFm[0] = 0x04;
			_bAudInfoFm[3] = 0x0C;
			break;

		case 0x7B:	/* FL/FR/FC/RL/RR/RC */
			_bAudInfoFm[0] = 0x05;
			_bAudInfoFm[3] = 0x0E;
			break;

		case 0x633:	/* FL/FR/RL/RR/RLC/RRC */
			_bAudInfoFm[0] = 0x05;
			_bAudInfoFm[3] = 0x10;
			break;

		case 0x63B:	/* FL/FR/FC/RL/RR/RLC/RRC */
			_bAudInfoFm[0] = 0x06;
			_bAudInfoFm[3] = 0x12;
			break;

		case 0x183:	/* FL/FR/FLC/FRC */
			_bAudInfoFm[0] = 0x03;
			_bAudInfoFm[3] = 0x14;
			break;

		case 0x18B:	/* FL/FR/FC/FLC/FRC */
			_bAudInfoFm[0] = 0x04;
			_bAudInfoFm[3] = 0x16;
			break;

		case 0x1C3:	/* FL/FR/RC/FLC/FRC */
			_bAudInfoFm[0] = 0x04;
			_bAudInfoFm[3] = 0x18;
			break;

		case 0x1CB:	/* FL/FR/FC/RC/FLC/FRC */
			_bAudInfoFm[0] = 0x05;
			_bAudInfoFm[3] = 0x1A;
			break;

		default:
			_bAudInfoFm[0] = 0x01;
			_bAudInfoFm[3] = 0x00;
			break;
		}

		if (_stAvdAVInfo.ui2_aud_out_ch.word & 0x04) {
			_bAudInfoFm[0]++;
			 /*LFE*/ _bAudInfoFm[3]++;
		}
	}

	_bAudInfoFm[1] = 0;
	_bAudInfoFm[4] = 0x0;

	vHalSendAudioInfoFrame(_bAudInfoFm[0], _bAudInfoFm[1], _bAudInfoFm[3], _bAudInfoFm[4]);

}

void vEnableAudio(unsigned int bOn)
{
	vWriteHdmiGRLMsk(AIP_CTRL, bOn << AUD_IN_EN_SHIFT, AUD_IN_EN);
}

void vHwNCTSOnOff(unsigned char bHwNctsOn)
{
	unsigned int bData;

	bData = bReadByteHdmiGRL(AIP_CTRL);

	if (bHwNctsOn == 0)
		bData |= CTS_SW_SEL;
	else
		bData &= ~CTS_SW_SEL;

	vWriteByteHdmiGRL(AIP_CTRL, bData);

}

void vHalHDMI_NCTS(unsigned char bAudioFreq, unsigned char bPix, unsigned char bDeepMode)
{
	unsigned char bTemp, bData, bData1[NCTS_BYTES];
	unsigned int u4Temp, u4NTemp = 0;

	bData = 0;

	for (bTemp = 0; bTemp < NCTS_BYTES; bTemp++)
		bData1[bTemp] = 0;

	if (bDeepMode == HDMI_NO_DEEP_COLOR) {
		for (bTemp = 0; bTemp < NCTS_BYTES; bTemp++) {

			if ((bAudioFreq < 7) && (bPix < 9))

				bData1[bTemp] = HDMI_NCTS[bAudioFreq][bPix][bTemp];
		}

		u4NTemp = (bData1[4] << 16) | (bData1[5] << 8) | (bData1[6]);	/* N */
		u4Temp = (bData1[0] << 24) | (bData1[1] << 16) | (bData1[2] << 8) | (bData1[3]);	/* CTS */

	} else {
		for (bTemp = 0; bTemp < NCTS_BYTES; bTemp++) {
			if ((bAudioFreq < 7) && (bPix < 9))

				bData1[bTemp] = HDMI_NCTS[bAudioFreq][bPix][bTemp];
		}

		u4NTemp = (bData1[4] << 16) | (bData1[5] << 8) | (bData1[6]);	/* N */
		u4Temp = (bData1[0] << 24) | (bData1[1] << 16) | (bData1[2] << 8) | (bData1[3]);

		if (bDeepMode == HDMI_DEEP_COLOR_10_BIT)
			u4Temp = (u4Temp >> 2) * 5;	/* (*5/4) */
		else if (bDeepMode == HDMI_DEEP_COLOR_12_BIT)
			u4Temp = (u4Temp >> 1) * 3;	/* (*3/2) */
		else if (bDeepMode == HDMI_DEEP_COLOR_16_BIT)
			u4Temp = (u4Temp << 1);	/* (*2) */

		bData1[0] = (u4Temp >> 24) & 0xff;
		bData1[1] = (u4Temp >> 16) & 0xff;
		bData1[2] = (u4Temp >> 8) & 0xff;
		bData1[3] = (u4Temp) & 0xff;

	}

	vWriteByteHdmiGRL(AIP_N_VAL, (bData1[4] << 16) + (bData1[5] << 8) + (bData1[6] << 0));
	vWriteByteHdmiGRL(AIP_CTS_SVAL,
			  (bData1[0] << 24) + (bData1[1] << 16) + (bData1[2] << 8) +
			  (bData1[3] << 0));
	_u4NValue = u4NTemp;

}

void vHDMI_NCTS(unsigned char bHDMIFsFreq, unsigned char bResolution, unsigned char bdeepmode)
{
	unsigned char bPix;

	switch (bResolution) {
	case HDMI_VIDEO_720x480p_60Hz:
	case HDMI_VIDEO_720x576p_50Hz:
	default:
		bPix = 0;
		break;

	case HDMI_VIDEO_1280x720p_60Hz:	/* 74.175M pixel clock */
	case HDMI_VIDEO_1280x720p_59_94Hz:
	case HDMI_VIDEO_1920x1080i_60Hz:
	case HDMI_VIDEO_1920x1080p_23Hz:
		bPix = 2;
		break;

	case HDMI_VIDEO_1280x720p_50Hz:	/* 74.25M pixel clock */
	case HDMI_VIDEO_1920x1080i_50Hz:
	case HDMI_VIDEO_1920x1080p_24Hz:
		bPix = 3;
		break;
	case HDMI_VIDEO_1920x1080p_60Hz:	/* 148.35M pixel clock */
	case HDMI_VIDEO_1920x1080p_59_94Hz:
	case HDMI_VIDEO_1280x720p3d_60Hz:
	case HDMI_VIDEO_1920x1080i3d_60Hz:
	case HDMI_VIDEO_1920x1080p3d_23Hz:
		bPix = 4;
		break;
	case HDMI_VIDEO_1920x1080p_50Hz:	/* 148.50M pixel clock */
	case HDMI_VIDEO_1280x720p3d_50Hz:
	case HDMI_VIDEO_1920x1080i3d_50Hz:
	case HDMI_VIDEO_1920x1080p3d_24Hz:
		bPix = 5;
		break;

	case HDMI_VIDEO_3840x2160P_23_976HZ:
	case HDMI_VIDEO_3840x2160P_29_97HZ:	/* 296.976m pixel clock */
		bPix = 7;
		break;

	case HDMI_VIDEO_3840x2160P_24HZ:	/* 297m pixel clock */
	case HDMI_VIDEO_3840x2160P_25HZ:	/* 297m pixel clock */
	case HDMI_VIDEO_3840x2160P_30HZ:	/* 297m pixel clock */
	case HDMI_VIDEO_4096x2160P_24HZ:	/* 297m pixel clock */
		bPix = 8;
		break;
	}
	if (bResolution_4K2K_50_60Hz(bResolution)
	    && (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)) {
		if ((bResolution == HDMI_VIDEO_3840x2160P_50HZ)
		    || (bResolution == HDMI_VIDEO_4096x2160P_50HZ))
			bPix = 8;
		if ((bResolution == HDMI_VIDEO_3840x2160P_60HZ)
		    || (bResolution == HDMI_VIDEO_4096x2160P_60HZ)
		    || (bResolution == HDMI_VIDEO_3840x2160P_59_94HZ)
		    || (bResolution == HDMI_VIDEO_4096x2160P_59_94HZ))
			bPix = 7;
	}

	vHalHDMI_NCTS(bHDMIFsFreq, bPix, bdeepmode);

}

void vHDMIAudioSRC(unsigned char ui1hdmifs, unsigned char ui1resindex, unsigned char bdeepmode)
{
	vHwNCTSOnOff(FALSE);

	switch (ui1hdmifs) {
	case HDMI_FS_44K:

		break;

	case HDMI_FS_48K:

		break;

	default:

		break;
	}

	vHDMI_NCTS(ui1hdmifs, ui1resindex, bdeepmode);
}

void vHalSendAVIInfoFrame(unsigned char *pr_bData)
{
	unsigned char bAVI_CHSUM = 0;
	unsigned char bData1 = 0, bData2 = 0, bData3 = 0, bData4 = 0, bData5 = 0;

	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, AVI_EN_WR | AVI_EN);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, AVI_RPT_EN);

	bData1 = *pr_bData;
	bData2 = *(pr_bData + 1);
	bData3 = *(pr_bData + 2);
	bData4 = *(pr_bData + 3);
	bData5 = *(pr_bData + 4);
	bAVI_CHSUM = AVI_TYPE + AVI_VERS + AVI_LEN;
	bAVI_CHSUM += bData1;
	bAVI_CHSUM += bData2;
	bAVI_CHSUM += bData3;
	bAVI_CHSUM += bData4;
	bAVI_CHSUM += bData5;
	bAVI_CHSUM = 0x100 - bAVI_CHSUM;

	vWriteByteHdmiGRL(TOP_AVI_HEADER, (AVI_LEN << 16) + (AVI_VERS << 8) + (AVI_TYPE << 0));
	vWriteByteHdmiGRL(TOP_AVI_PKT00,
			  (bData3 << 24) + (bData2 << 16) + (bData1 << 8) + (bAVI_CHSUM << 0));
	vWriteByteHdmiGRL(TOP_AVI_PKT01, (bData5 << 8) + (bData4 << 0));
	vWriteByteHdmiGRL(TOP_AVI_PKT02, 0);
	vWriteByteHdmiGRL(TOP_AVI_PKT03, 0);
	vWriteByteHdmiGRL(TOP_AVI_PKT04, 0);

	vWriteHdmiGRLMsk(TOP_INFO_RPT, AVI_RPT_EN, AVI_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, AVI_EN_WR | AVI_EN, AVI_EN_WR | AVI_EN);

}

unsigned char vic_4k5060(unsigned char ui1resindex)
{
	unsigned char vic_value = 0;

	if (ui1resindex == HDMI_VIDEO_3840x2160P_50HZ)
		vic_value = 96;
	else if ((ui1resindex == HDMI_VIDEO_3840x2160P_60HZ)
		|| (ui1resindex == HDMI_VIDEO_3840x2160P_59_94HZ))
		vic_value = 97;
	else if ((ui1resindex == HDMI_VIDEO_4096x2160P_60HZ)
		|| (ui1resindex == HDMI_VIDEO_4096x2160P_59_94HZ))
		vic_value = 102;
	else if (ui1resindex == HDMI_VIDEO_4096x2160P_50HZ)
		vic_value = 101;
	else
		TX_DEF_LOG("4k error\n");
	return vic_value;
}

void vHalSendVendorSpecificInfoFrame(unsigned char fg3DRes, unsigned char bVIC,
				     unsigned char b3dstruct, unsigned char fgDolbyHdrEnable)
{
	unsigned char bVS_CHSUM;
	unsigned char bPB1, bPB2, bPB3, bPB4, bPB5;

	bPB1 = 0x03;
	bPB2 = 0x0C;
	bPB3 = 0x00;

	if (fg3DRes == TRUE)
		bPB4 = 0x40;
	else
		bPB4 = 0x20;	/* for 4k2k */

	bPB5 = 0x00;
	if (b3dstruct != 0xff)
		bPB5 |= b3dstruct << 4;
	else			/* for4k2k */
		bPB5 = bVIC;

	if (fgDolbyHdrEnable)
		bVS_CHSUM =
		    VS_VERS + VS_TYPE + VS_DOLBYVISION_LEN + bPB1 + bPB2 + bPB3 + bPB4 + bPB5;
	else
		bVS_CHSUM = VS_VERS + VS_TYPE + VS_LEN + bPB1 + bPB2 + bPB3 + bPB4 + bPB5;
	bVS_CHSUM = 0x100 - bVS_CHSUM;

	vWriteHdmiGRLMsk(TOP_INFO_EN, VSIF_DIS | VSIF_DIS_WR, VSIF_EN | VSIF_EN_WR);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, VSIF_RPT_DIS, VSIF_RPT_EN);

	if (fgDolbyHdrEnable)
		vWriteByteHdmiGRL(TOP_VSIF_HEADER,
				  (VS_DOLBYVISION_LEN << 16) + (VS_VERS << 8) + VS_TYPE);
	else
		vWriteByteHdmiGRL(TOP_VSIF_HEADER, (VS_LEN << 16) + (VS_VERS << 8) + VS_TYPE);

	vWriteByteHdmiGRL(TOP_VSIF_PKT00, (bPB3 << 24) + (bPB2 << 16) + (bPB1 << 8) + bVS_CHSUM);
	vWriteByteHdmiGRL(TOP_VSIF_PKT01, (bPB5 << 8) + bPB4);
	vWriteByteHdmiGRL(TOP_VSIF_PKT02, 0x00000000);
	vWriteByteHdmiGRL(TOP_VSIF_PKT03, 0x00000000);
	vWriteByteHdmiGRL(TOP_VSIF_PKT04, 0x00000000);
	vWriteByteHdmiGRL(TOP_VSIF_PKT05, 0x00000000);
	vWriteByteHdmiGRL(TOP_VSIF_PKT06, 0x00000000);
	vWriteByteHdmiGRL(TOP_VSIF_PKT07, 0x00000000);

	if ((fg3DRes != 0) || (bVIC != 0) || (fgDolbyHdrEnable != 0)) {
		vWriteHdmiGRLMsk(TOP_INFO_RPT, VSIF_RPT_EN, VSIF_RPT_EN);
		vWriteHdmiGRLMsk(TOP_INFO_EN, VSIF_EN | VSIF_EN_WR, VSIF_EN | VSIF_EN_WR);
	}
}

void vDisable_AVMUTE_Packet(void)
{
	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_CLR_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_SET_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, CP_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, CP_EN | CP_EN_WR);
}

void vDisable_HDCP_Encrypt(void)
{
	vWriteHdmiGRLMsk(HDCP2X_CTRL_0, 0, HDCP2X_ENCRYPT_EN);
	vWriteHdmiGRLMsk(0xcd0, 0, 1 << 6);
}

void vSend_AVUNMUTE(void)
{
	/*GCP packet */
	HDMI_DRV_FUNC();

	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_CLR_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_SET_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, CP_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, CP_EN | CP_EN_WR);

	vWriteHdmiGRLMsk(TOP_CFG01, CP_CLR_MUTE_EN, CP_CLR_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_SET_MUTE_DIS);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, CP_RPT_EN, CP_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, CP_EN | CP_EN_WR, CP_EN | CP_EN_WR);
}

void vSend_AVMUTE(void)
{
	/*GCP packet */
	HDMI_DRV_FUNC();

	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_CLR_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_SET_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, CP_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, CP_EN | CP_EN_WR);

	vWriteHdmiGRLMsk(TOP_CFG01, 0, CP_CLR_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_CFG01, CP_SET_MUTE_EN, CP_SET_MUTE_EN);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, CP_RPT_EN, CP_RPT_EN);
	vWriteHdmiGRLMsk(TOP_INFO_EN, CP_EN | CP_EN_WR, CP_EN | CP_EN_WR);
}

void vSendTMDSConfiguration(unsigned char enscramble)
{
	HDMI_DRV_FUNC();

	HDMI_PLL_LOG("enscramble = %d\n", enscramble);
	fgDDCDataWrite(RX_REG_SCRAMBLE >> 1, RX_REG_TMDS_CONFIG, 1, &enscramble);
}

unsigned char bOver340M(void)
{
	char _bver340m = FALSE;

	HDMI_PLL_LOG("Res = %d, deepcolor = %d\n", _stAvdAVInfo.e_resolution,
		     _stAvdAVInfo.e_deep_color_bit);
	if (fgFMTis4k2k(_stAvdAVInfo.e_resolution)) {
		_bver340m = TRUE;
		HDMI_PLL_LOG("_bver340m = true\n");
	}

	if (fgFMTis4k2k(_stAvdAVInfo.e_resolution)
	    && (_stAvdAVInfo.e_deep_color_bit == HDMI_NO_DEEP_COLOR)
	    && (!fgFMTis4k2k_6G(_stAvdAVInfo.e_resolution))) {
		HDMI_PLL_LOG("para1 = %d, para2 = %d, para3 = %d\n",
			     fgFMTis4k2k(_stAvdAVInfo.e_resolution),
			     (_stAvdAVInfo.e_deep_color_bit == HDMI_NO_DEEP_COLOR),
			     (!fgFMTis4k2k_6G(_stAvdAVInfo.e_resolution)));
		_bver340m = FALSE;
		HDMI_PLL_LOG("_bver340m = false1\n");
	}

	if (fgFMTis4k2k_6G(_stAvdAVInfo.e_resolution) && (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
	    && (_stAvdAVInfo.e_deep_color_bit == HDMI_NO_DEEP_COLOR)) {
		HDMI_PLL_LOG("para1 = %d, para2 = %d, para3 = %d\n",
			     fgFMTis4k2k_6G(_stAvdAVInfo.e_resolution),
			     (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420),
			     (_stAvdAVInfo.e_deep_color_bit == HDMI_NO_DEEP_COLOR));
		_bver340m = FALSE;
		HDMI_PLL_LOG("_bver340m = false2\n");
	}

	HDMI_PLL_LOG("bOver340M = %d\n", _bver340m);
	return _bver340m;
}

void v4k6G_set_clock(void)
{
	unsigned int u4Data;

	HDMI_DRV_FUNC();

	if (bOver340M() == TRUE) {
		vWriteIoHdmiAnaMsk(HDMI20_CLK_CFG, (0x2 << REG_TXC_DIV_SHIFT), REG_TXC_DIV);
		u4Data = dReadIoHdmiAna(HDMI20_CLK_CFG);
		HDMI_PLUG_LOG("HDMI20_CLK_CFG bOver340M 1170207c = 0x%x\n", u4Data);
	} else {
		vWriteIoHdmiAnaMsk(HDMI20_CLK_CFG, 0, REG_TXC_DIV);
		u4Data = dReadIoHdmiAna(HDMI20_CLK_CFG);
		HDMI_PLUG_LOG("HDMI20_CLK_CFG !bOver340M 1170207c = 0x%x\n", u4Data);
		}
}

void vSetRGB2HdmiYUV709(bool bOn)
{
	if (bOn == TRUE)
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_709_ORG, YCBCR2RGB_3X3_BY_COEFFICIENTS);
	else
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, 0, YCBCR2RGB_3X3_BY_COEFFICIENTS);
}

void vSetHdmiFullRGB(char bOn)
{
	int ui4Data;

	ui4Data = dReadRGB2HDMI(HDMI_RGB_CTRL);

	if (bOn == TRUE)
		ui4Data |= RGB_FULL_RANGE;
	else
		ui4Data &= ~RGB_FULL_RANGE;

	vWriteRGB2HDMI(HDMI_RGB_CTRL, ui4Data);
}

void vSetHdmiFullYUV(char bOn)
{
	int ui4Data;

	ui4Data = dReadRGB2HDMI(HDMI_RGB_CTRL);

	if (bOn == TRUE)
		ui4Data |= YVU_FULL;
	else
		ui4Data &= ~YVU_FULL;

	vWriteRGB2HDMI(HDMI_RGB_CTRL, ui4Data);
}

void vHalSetHdmiLimitRange(char bOn, int u2TopLimit, int u1BottomLimit)
{
	vWriteRGB2HDMI(HDMI_RGB2HDMI_50, (u2TopLimit << 16) | u1BottomLimit);
	if (bOn)
		vWriteRGB2HDMIMsk(HDMI_RGB_CTRL, LMT_EN, LMT_EN);
	else
		vWriteRGB2HDMIMsk(HDMI_RGB_CTRL, 0, LMT_EN);
}

void vSetRGBYCbCrDelay(char bRDelay, char bGDelay, char bBDelay, char bYDelay, char bCbDelay,
		       char bCrDelay)
{
	vWriteRGB2HDMI(HDMI_RGB2HDMI_4O,
		    ((bRDelay & 0x07)) | ((bGDelay & 0x07) << 4) | ((bBDelay & 0x07) << 8) |
		    ((bYDelay & 0x07) << 12) | ((bCbDelay & 0x07) << 16) | ((bCrDelay & 0x07) <<
									    20));
}

void vSetHDMIVideoPixelRepeat(char bColorSpace, bool fgPixRep, bool fgYuv422Delay)
{
	int ui4Data;
	int ui4DataDelay = 0;

	ui4Data = dReadRGB2HDMI(HDMI_RGB_CTRL);
	ui4Data &= ~(DOUBLE_EN|REPEAT_EN|YCBCR422_CBCR_INV);
	ui4Data |= (RGB2HDMI_ON|UV_OFFSET|ADJ_SYNC_EN);
	ui4DataDelay |= HSYNC_DELAY;

	if (bColorSpace == YCBCR_444) {
		ui4Data &= ~(YUV_422|RGB_MOD);
		if (fgPixRep) {
			ui4Data |= REPEAT_EN; /* enable 4:4:4	repeater */
			ui4DataDelay &= ~HSYNC_DELAY;
		}
	} else if (bColorSpace == YCBCR_422) {
		ui4Data &= ~(RGB_MOD);
		ui4Data |= YUV_422;
		if (fgPixRep) {
			if (fgYuv422Delay) {
				ui4Data |= (DOUBLE_EN|REPEAT_EN);
				ui4DataDelay |= (DOUBLE422_DELAY<<1);
				ui4DataDelay &= ~HSYNC_DELAY;
			} else {
				ui4Data |= (DOUBLE_EN|REPEAT_EN);
				ui4Data &= ~(YCBCR422_CBCR_INV);
				ui4DataDelay |= (DOUBLE422_DELAY);
				ui4DataDelay &= ~HSYNC_DELAY;
			}
		} else {
			ui4DataDelay |= (DOUBLE422_DELAY);
		}
	} else {
		ui4Data &= ~(YUV_422);
		ui4Data |= RGB_MOD;
		if (fgPixRep) {
			ui4Data |= REPEAT_EN; /* enable 4:4:4	repeater */
			ui4DataDelay &= ~HSYNC_DELAY;
		}
	}
	vWriteRGB2HDMI(HDMI_RGB_CTRL, ui4Data);
	vWriteRGB2HDMI(HDMI_RGB2HDMI_C0, ui4DataDelay);
}

void vHDMIVideoOutput(char ui1Res, char ui1ColorSpace)
{
	bool fgPixRep, fgYuv422Delay;
	char bColorSpace;
	if (down_timeout(&color_space_mutex, msecs_to_jiffies(2000)))
		TX_DEF_LOG("can't get color_space_mutex in %s()\n", __func__);

	if (_fgDolbyHdrEnable)
		ui1ColorSpace = HDMI_YCBCR_444;

	fgPixRep = FALSE;
	fgYuv422Delay = FALSE;

	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420_FULL)
		ui1ColorSpace = HDMI_YCBCR_444_FULL;

	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
		ui1ColorSpace = HDMI_YCBCR_444;

	if ((ui1ColorSpace == HDMI_YCBCR_444) || (ui1ColorSpace == HDMI_YCBCR_444_FULL)
	    || (ui1ColorSpace == HDMI_XV_YCC)) {
		bColorSpace = YCBCR_444;
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x80001000);
	} else if ((ui1ColorSpace == HDMI_YCBCR_422) || (ui1ColorSpace == HDMI_YCBCR_422_FULL)) {
		bColorSpace = YCBCR_422;
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x00001000);
	} else {
		bColorSpace = RGB_444;
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x1000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x10001000);
	}
	HDMI_PLUG_LOG("vHDMIVideoOutput ui1Res=%d,bColorSpace=%d,ui1ColorSpace=%d\n", ui1Res,
		      bColorSpace, ui1ColorSpace);

	vSetHDMIVideoPixelRepeat(ui1ColorSpace, fgPixRep, fgYuv422Delay);

	if (fgIsHDRes(ui1Res))
		vSetRGB2HdmiYUV709(TRUE);
	else
		vSetRGB2HdmiYUV709(FALSE);

	vSetHdmiFullRGB(TRUE);
	vSetHdmiFullYUV(TRUE);

	if (ui1ColorSpace == HDMI_RGB_FULL) {
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_60, YCBCR2RGB_3X3_ON, YCBCR2RGB_3X3_ON);	/* Y offset 16 */
		if (_fgBT2020Enable) {
			vWriteRGB2HDMI(HDMI_RGB2HDMI_64, 0x70eb0100);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_6C, 0x0C000400);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_70, 0xF5B20017);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_74, 0xD9003B89);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_78, 0x3A);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_7C, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_80, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_84, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_88, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_8C, 0);
		} else if (fgIsHDRes(ui1Res))	/* 709 */
			vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_709_NEW2,
				       YCBCR2RGB_3X3_BY_COEFFICIENTS);
		/* limit to 16 ~ 235 */
		else
			vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_601_NEW2,
				       YCBCR2RGB_3X3_BY_COEFFICIENTS);
		/* limit to 16 ~ 235 */
		vHalSetHdmiLimitRange(TRUE, 0xfeff, 0x100);	/* 16bit */
	} else if (ui1ColorSpace == HDMI_RGB) {
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_60, YCBCR2RGB_3X3_ON, YCBCR2RGB_3X3_ON);	/* Y offset 16 */
		if (_fgBT2020Enable) {
			vWriteRGB2HDMI(HDMI_RGB2HDMI_64, 0x70eb0100);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_6C, 0x0C000400);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_70, 0xF5B20017);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_74, 0xD9003B89);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_78, 0x3A);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_7C, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_80, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_84, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_88, 0);
			vWriteRGB2HDMI(HDMI_RGB2HDMI_8C, 0);
		} else if (fgIsHDRes(ui1Res))	/* 709 */
			vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_709_ORG,
				       YCBCR2RGB_3X3_BY_COEFFICIENTS);
		/* limit to 16 ~ 235 */
		else
			vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_601_ORG,
				       YCBCR2RGB_3X3_BY_COEFFICIENTS);
		/* limit to 16 ~ 235 */
		vHalSetHdmiLimitRange(FALSE, 0xebff, 0x0000);	/* 16bit */
	} else {
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_60, 0, YCBCR2RGB_3X3_ON);	/* Y offset 16 */
		vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_64, YCBCR2RGB_601_ORG, YCBCR2RGB_3X3_BY_COEFFICIENTS);
		/* limit to 16 ~ 235 */
		if (_fgDolbyHdrEnable)
			vHalSetHdmiLimitRange(FALSE, 0xfeff, 0x100);	/* 16bit */
		else
			vHalSetHdmiLimitRange(TRUE, 0xfeff, 0x100);	/* 16bit */
	}

	if ((ui1Res == HDMI_VIDEO_1920x1080p_30Hz) || (ui1Res == HDMI_VIDEO_1920x1080p_25Hz)
	    || (ui1Res == HDMI_VIDEO_1920x1080p_24Hz) || (ui1Res == HDMI_VIDEO_1920x1080p_23Hz)
	    || (ui1Res == HDMI_VIDEO_1920x1080p_29Hz) || (ui1Res == HDMI_VIDEO_1920x1080p_60Hz)
	    || (ui1Res == HDMI_VIDEO_1920x1080p_50Hz) || (ui1Res == HDMI_VIDEO_1920x1080p3d_24Hz)
	    || (ui1Res == HDMI_VIDEO_1920x1080p3d_23Hz) || (ui1Res == HDMI_VIDEO_1920x1080p_59_94Hz))
		vSetRGBYCbCrDelay(0, 0, 0, 0, 0, 0);
	else if ((ui1Res == HDMI_VIDEO_1920x1080i_60Hz) || (ui1Res == HDMI_VIDEO_1920x1080i_50Hz))
		vSetRGBYCbCrDelay(0, 0, 0, 1, 0, 1);
	else
		vSetRGBYCbCrDelay(0, 0, 0, 1, 1, 1);
	up(&color_space_mutex);
}

/*
 *Function : BOOL fgTVisHDMI(void)
 *Description : Check if the receiver is HDMI or DVI
 *Parameter   : None
 *Return      : True  -- HDMI, and connecting
 *		False -- DVI
*/
bool fgTVisHDMI(void)
{

	if ((i4SharedInfo(SI_EDID_VSDB_EXIST) == TRUE) && (i4SharedInfo(SI_HDMI_RECEIVER_STATUS) != HDMI_PLUG_OUT))
		return TRUE;
	else
		return FALSE;
}

void vEnableDeepColor(bool fgEnable, unsigned int ui1Mode)
{
	unsigned int u4Data;

	HDMI_DRV_FUNC();

	if (ui1Mode == HDMI_DEEP_COLOR_10_BIT)
		u4Data = DEEPCOLOR_MODE_10BIT;
	else if (ui1Mode == HDMI_DEEP_COLOR_12_BIT)
		u4Data = DEEPCOLOR_MODE_12BIT;
	else if (ui1Mode == HDMI_DEEP_COLOR_16_BIT)
		u4Data = DEEPCOLOR_MODE_16BIT;
	else
		u4Data = DEEPCOLOR_MODE_8BIT;

	vWriteHdmiGRLMsk(TOP_CFG00, u4Data, DEEPCOLOR_MODE_MASKBIT);

	/* GCP */
	vWriteHdmiGRLMsk(TOP_CFG00, 0, DEEPCOLOR_PAT_EN);
	if ((fgEnable == TRUE) && (u4Data != DEEPCOLOR_MODE_8BIT)) {
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, DEEP_COLOR_ADD, DEEP_COLOR_ADD);
	} else {
		vWriteHdmiGRLMsk(TOP_MISC_CTLR, 0, DEEP_COLOR_ADD);
	}
}

void vSendHdmiDeepColorModePhase(char bDeepColor)
{
	if (bDeepColor == HDMI_NO_DEEP_COLOR)
		/* SEND GCP */
		vEnableDeepColor(FALSE, HDMI_NO_DEEP_COLOR);
	else
		vEnableDeepColor(TRUE, bDeepColor);
}

void vSetHDMIDataEnable(int bResIndex)
{
	/* Set Vsync start, hsync width, v_polar, h_polar */
	if (bResIndex < MAX_RES) {
		vWriteRGB2HDMI(HDMI_RGB_TIME0, HDMI_SCL_HDMISYNC[bResIndex][0]);
		/* Write hdmi data enable */
		vWriteRGB2HDMIMsk(HDMI_RGB_TIME1, HDMI_DATA_ENABLE[bResIndex][2], 0x9FFF1FFF);
		/* 0xABC~0xABF */
		vWriteRGB2HDMIMsk(HDMI_RGB_TIME2, HDMI_DATA_ENABLE[bResIndex][1], 0x7FFFFFF);
		/* 0xAC0~0xAC3 */
		vWriteRGB2HDMIMsk(HDMI_RGB_TIME3, HDMI_DATA_ENABLE[bResIndex][0], 0x7FFFFFF);
		/* 0xAC4~0xAC7 */
	}
}

void vSetHDMI4K2KDataEnable(void)
{
	/* Set Vsync start, hsync width, v_polar, h_polar */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME0, (88 << 16) | (10 << 0), 0xffffffff);
	/* Write hdmi data enable */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME1, ((88 + 296 + 1) << 16) | (88 + 296 + 3840), 0x1FFF1FFF);
	/* 0xABC~0xABF */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME2, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC0~0xAC3 */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME3, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC4~0xAC7 */
}

void vSetHDMI4K2K4096DataEnable(void)
{
	/* Set Vsync start, hsync width, v_polar, h_polar */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME0, (88 << 16) | (10 << 0), 0xffffffff);
	/* Write hdmi data enable */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME1, ((88 + 296 + 1) << 16) | (88 + 296 + 4096), 0x1FFF1FFF);
	/* 0xABC~0xABF */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME2, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC0~0xAC3 */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME3, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC4~0xAC7 */
}

void vSetHDMI4K2K4096DataEnable_60Hz(void)
{
	/* Set Vsync start, hsync width, v_polar, h_polar */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME0, (88 << 16) | (10 << 0), 0xffffffff);
	/* Write hdmi data enable */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME1, ((88 + 128 + 1) << 16) | (88 + 128 + 4096), 0x1FFF1FFF);
	/* 0xABC~0xABF */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME2, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC0~0xAC3 */
	vWriteRGB2HDMIMsk(HDMI_RGB_TIME3, ((10 + 72 + 1) << 16) | (10 + 72 + 2160), 0x7FFFFFF);
	/* 0xAC4~0xAC7 */
}

void vSetHDMISyncDelay(int bResIndex)
{
	if (bResIndex < MAX_RES)
		vWriteRGB2HDMI(HDMI_RGB2HDMI_B4, HDMI_SYNC_DELAY[bResIndex][0]);
}

void vSet85553DSyncDelay(int dwIndex)
{
	vWriteRGB2HDMIMsk(HDMI_RGB2HDMI_B4, dwIndex, 0xFFFFFFFF);
}

void vHDMIResetGenReg(void)
{
	char bResTableIndx = 0;

	v4k6G_set_clock();

	hdmi_ddc_request(1);
	vResetHDMI(1);		/* reset HDMI, all general register will be reset */
	hdmi_ddc_free(1);

	HDMI_PLUG_LOG("vHDMIResetGenReg = %d, %lums\n", _stAvdAVInfo.e_resolution,
		      jiffies * 10);

	vHDMIVideoOutput(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
	vResetHDMI(0);		/* HDMI normally */
	HAL_Delay_us(2);
	vWriteByteHdmiGRL(HDCP_TOP_CTRL, 0x0);
	HDMI_EnableIrq();
	HDMIForceHDCPHPCLevel();

	if ((_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444) || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444_FULL)
		|| (_stAvdAVInfo.e_video_color_space == HDMI_XV_YCC) || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
		|| (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420_FULL)) {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x80001000);
	} else if ((_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422) || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422_FULL)) {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x00001000);
	} else {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x1000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x10001000);
	}

	if (fgTVisHDMI()) {
		vSendHdmiDeepColorModePhase(_stAvdAVInfo.e_deep_color_bit);
		vEnableHdmiMode(TRUE);
	} else {
		vSendHdmiDeepColorModePhase(HDMI_NO_DEEP_COLOR);
		vEnableHdmiMode(FALSE);
	}

	bResTableIndx = _stAvdAVInfo.e_resolution - HDMI_VIDEO_720x480i_60Hz;
	if (fgFMTis4k2k(bResTableIndx)) {
		if (_stAvdAVInfo.e_resolution == HDMI_VIDEO_4096x2160P_24HZ)
			vSetHDMI4K2K4096DataEnable();
		else if ((_stAvdAVInfo.e_resolution == HDMI_VIDEO_4096x2160P_60HZ)
			|| (_stAvdAVInfo.e_resolution == HDMI_VIDEO_4096x2160P_59_94HZ)
			 || (_stAvdAVInfo.e_resolution == HDMI_VIDEO_4096x2160P_50HZ))
			vSetHDMI4K2K4096DataEnable_60Hz();
		else
			vSetHDMI4K2KDataEnable();
	} else {
		vSetHDMIDataEnable(bResTableIndx);
	}

	vSetHDMISyncDelay(bResTableIndx);

	if (fgFMTis4k2k(_stAvdAVInfo.e_resolution))
		vSet85553DSyncDelay(0x00010026);
	else
		vSetHDMISyncDelay(bResTableIndx);
}

void vTmdsPresetOn(void)
{
	HDMI_PLUG_LOG("vTmdsPresetOn = %lums\n", jiffies);

	vWriteIoHdmiAnaMsk(HDMI20_CFG_7, RG_HDMITX20_RESERVE, RG_HDMITX20_RESERVE);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, RG_HDMITX20PLL_BG_EN, RG_HDMITX20PLL_BG_EN);
	vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, RG_HDMITX20PLL_EN, RG_HDMITX20PLL_EN);
	udelay(100);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_8, RG_HDMITX20_BIAS_EN, RG_HDMITX20_BIAS_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_5, RG_HDMITX20_CKLDO_EN, RG_HDMITX20_CKLDO_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_4, RG_HDMITX20_SLDO_EN, RG_HDMITX20_SLDO_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_8, RG_HDMITX20_TX_POSDIV_EN, RG_HDMITX20_TX_POSDIV_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_0, RG_HDMITX20_SER_EN, RG_HDMITX20_SER_EN);

	vWriteIoHdmiAnaMsk(HDMI20_CFG_0, RG_HDMITX20_PRD_EN, RG_HDMITX20_PRD_EN);

	vWriteIoHdmiAnaMsk(HDMI20_CFG_8, 0, RG_HDMITX20_BIAS_LPF_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_8, RG_HDMITX20_INTR_EN, RG_HDMITX20_INTR_EN);

	vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0, RG_HDMITX20_CKLDOLPF_EN);
	vWriteIoHdmiAnaMsk(HDMI20_CFG_4, 0, RG_HDMITX20_SLDOLPF_EN);

	vWriteIoHdmiAnaMsk(0x7c, (1 << 16), (1 << 16));
	udelay(100);
}

void vTxSignalOnOff(unsigned char bOn)
{
	HDMI_DRV_FUNC();

	if (bOn) {

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, RG_HDMITX20_DRV_EN, RG_HDMITX20_DRV_EN);
	} else {
		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, 0, RG_HDMITX20_DRV_EN);
		udelay(100);
		vWriteIoHdmiAnaMsk(0x7c, 0, (1 << 16));
		vWriteIoHdmiAnaMsk(HDMI20_CFG_4, 0, RG_HDMITX20_SLDOLPF_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0, RG_HDMITX20_CKLDOLPF_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_8, 0, RG_HDMITX20_BIAS_LPF_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, 0, RG_HDMITX20_PRD_EN);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, 0, RG_HDMITX20_SER_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_8, 0, RG_HDMITX20_TX_POSDIV_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_4, 0, RG_HDMITX20_SLDO_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_5, 0, RG_HDMITX20_CKLDO_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_8, 0, RG_HDMITX20_BIAS_EN);

		vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, 0, RG_HDMITX20PLL_EN);
		vWriteIoHdmiAnaMsk(HDMI20_PLL_CFG_0, 0, RG_HDMITX20PLL_BG_EN);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_7, 0, RG_HDMITX20_RESERVE);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_3, 0, 0xffffffff);
		vWriteIoHdmiAnaMsk(HDMI20_CFG_6, 0, 0xffffffff);

		vWriteIoHdmiAnaMsk(HDMI20_CFG_0, 0, RG_HDMITX20_DRV_IMP_EN);

		HDMI_PLUG_LOG("vTxSignalOff = %lums\n", jiffies);
	}

}

bool vCheckSendxvYCC(void)
{
	if (((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC709)
	     || (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC601))
	    && (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_METADATA0))
		HDMI_PLUG_LOG("[XVYCC]TV Support XvYCC!!!\n");
	else
		HDMI_PLUG_LOG("[XVYCC]TV Not Support XvYCC!!!\n");

	if (_stAvdAVInfo.e_video_color_space == HDMI_XV_YCC)
		HDMI_PLUG_LOG("[XVYCC]UI color space is HDMI_XV_YCC!!!\n");
	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444)
		HDMI_PLUG_LOG("[XVYCC]UI color space is HDMI_YCBCR_444!!!\n");
	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422)
		HDMI_PLUG_LOG("[XVYCC]UI color space is HDMI_YCBCR_422!!!\n");

	if (((_stAvdAVInfo.e_video_color_space == HDMI_XV_YCC)
	     || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444)
	     || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422)
	     || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_422_FULL)
	     || (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_444_FULL)) &&
	    ((_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC709)
	     || (_HdmiSinkAvCap.ui2_sink_colorimetry & SINK_XV_YCC601))
	    ) {
		return TRUE;
	} else
		return FALSE;
}

void vChgHDMIVideoResolution(void)
{
	int u4Index;

	vTmdsPresetOn();
	/*vHDMIAVMute(); */
	vHDMIResetGenReg();
	/*vChgHDMIAudioOutput(CHG_NCTS_AND_INPUT);  */

	for (u4Index = 0; u4Index < 5; u4Index++)
		HAL_Delay_us(1);

	vHDMIAVMute();
	vSend_AVUNMUTE();
	vWriteHdmiGRLMsk(TOP_CFG01, NULL_PKT_VSYNC_HIGH_EN, NULL_PKT_VSYNC_HIGH_EN | NULL_PKT_EN);

	if (bOver340M() == TRUE) {
		vSendTMDSConfiguration(SCRAMBLING_ENABLE | TMDS_BIT_CLOCK_RATION);
		HAL_Delay_us(100);
		vWriteHdmiGRLMsk(TOP_CFG00, SCR_ON | HDMI2_ON, SCR_ON | HDMI2_ON);
	} else {
		vSendTMDSConfiguration(0);
		HAL_Delay_us(100);
		vWriteHdmiGRLMsk(TOP_CFG00, 0, SCR_ON | HDMI2_ON);
	}

	/* vMutevideoAudio(0); */
}

void vHDMIAVUnMute(void)
{
	HDMI_AUDIO_FUNC();
	vUnBlackHDMIOnly();
	UnMuteHDMIAudio();
}

unsigned int IS_HDMI_HTPLG(void)
{

	if (debug_force_hotplug == HDMI_STATE_HOT_PLUG_OUT)
		return FALSE;
	else if (debug_force_hotplug == HDMI_STATE_HOT_PLUG_IN_ONLY)
		return TRUE;
	else if (debug_force_hotplug == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)
		return TRUE;

	if (bReadByteHdmiGRL(HPD_DDC_STATUS) & HPD_PIN_STA)
		return TRUE;
	else
		return FALSE;
}

unsigned int IS_HDMI_PORD(void)
{

	if (debug_force_hotplug == HDMI_STATE_HOT_PLUG_OUT)
		return FALSE;
	else if (debug_force_hotplug == HDMI_STATE_HOT_PLUG_IN_ONLY)
		return FALSE;
	else if (debug_force_hotplug == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)
		return TRUE;

	if (bReadByteHdmiGRL(HPD_DDC_STATUS) & PORD_PIN_STA)
		return TRUE;
	else
		return FALSE;
}

unsigned char bCheckStatus(unsigned char bMode)
{
	unsigned char bStatus = 0;

	bStatus = IS_HDMI_PORD() << 1;

	if ((STATUS_HTPLG & bMode) == STATUS_HTPLG)
		bStatus = IS_HDMI_HTPLG() ? (bStatus | STATUS_HTPLG) : (bStatus & (~STATUS_HTPLG));

	if ((bStatus & bMode) == bMode)
		return TRUE;
	else
		return FALSE;
}

unsigned char bCheckPordHotPlug(unsigned char bMode)
{
	unsigned char bStatus = FALSE;

	if (bMode == (PORD_MODE | HOTPLUG_MODE))
		bStatus = bCheckStatus(STATUS_PORD | STATUS_HTPLG);
	else if (bMode == HOTPLUG_MODE)
		bStatus = bCheckStatus(STATUS_HTPLG);
	else if (bMode == PORD_MODE)
		bStatus = bCheckStatus(STATUS_PORD);

	return bStatus;
}

void force_plugin(unsigned int colorspace)
{
	HDMI_DRV_FUNC();
	vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_IN_AND_SINK_POWER_ON);
	vSetSharedInfo(SI_EDID_VSDB_EXIST, TRUE);

	if (colorspace == HDMI_YCBCR_420)
		_HdmiSinkAvCap.ui2_sink_colorimetry = SINK_YCBCR_420;
	else
		_HdmiSinkAvCap.ui2_sink_colorimetry = SINK_RGB;
}

void vHalSendStaticHdrInfoFrame(char bEnable, char *pr_bData)
{

	char bHDR_CHSUM = 0;
	char i;

	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, GEN4_EN | GEN4_EN_WR);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, GEN4_RPT_EN);

	HDMI_PLUG_LOG("vHalSendStaticHdrInfoFrame bEnable = %d\n", bEnable);

	if (bEnable == HDR_PACKET_ACTIVE) {
		vWriteByteHdmiGRL(TOP_GEN4_HEADER,
				  (HDR_LEN << 16) + (HDR_VERS << 8) + (HDR_TYPE << 0));
		bHDR_CHSUM = HDR_LEN + HDR_VERS + HDR_TYPE;
		for (i = 0; i < HDR_LEN; i++)
			bHDR_CHSUM += (*(pr_bData + i));
		bHDR_CHSUM = 0x100 - bHDR_CHSUM;
		vWriteByteHdmiGRL(TOP_GEN4_PKT00,
				  ((*(pr_bData + 2)) << 24) + (*(pr_bData + 1) << 16) +
				  ((*(pr_bData + 0)) << 8) + (bHDR_CHSUM << 0));
		HDMI_HDR_LOG(" 1 data0=0x%x, data1=0x%x, data2=0x%x, data3=0x%x\n",
				  (bHDR_CHSUM << 0), ((*(pr_bData + 0)) << 8),
				  (*(pr_bData + 1) << 16), ((*(pr_bData + 2)) << 24));
		vWriteByteHdmiGRL(TOP_GEN4_PKT01,
				  (((*(pr_bData + 5)) << 16) + ((*(pr_bData + 4)) << 8) +
				   ((*(pr_bData + 3)) << 0)));
		HDMI_HDR_LOG(" 2 data0=0x%x, data1=0x%x, data2=0x%x\n",
				  ((*(pr_bData + 3)) << 0), ((*(pr_bData + 4)) << 8),
				  ((*(pr_bData + 5)) << 16));
		vWriteByteHdmiGRL(TOP_GEN4_PKT02,
				  ((*(pr_bData + 9)) << 24) + ((*(pr_bData + 8)) << 16) +
				  ((*(pr_bData + 7)) << 8) + ((*(pr_bData + 6)) << 0));
		HDMI_HDR_LOG(" 3 data0=0x%x, data1=0x%x, data2=0x%x, data3=0x%x\n",
				  ((*(pr_bData + 6)) << 0), ((*(pr_bData + 7)) << 8),
				  ((*(pr_bData + 8)) << 16), ((*(pr_bData + 9)) << 24));
		vWriteByteHdmiGRL(TOP_GEN4_PKT03,
				  ((*(pr_bData + 12)) << 16) + ((*(pr_bData + 11)) << 8) +
				  ((*(pr_bData + 10)) << 0));
		HDMI_HDR_LOG(" 4 data0=0x%x, data1=0x%x, data2=0x%x\n",
				  ((*(pr_bData + 10)) << 0), ((*(pr_bData + 11)) << 8),
				  ((*(pr_bData + 12)) << 16));
		vWriteByteHdmiGRL(TOP_GEN4_PKT04,
				  ((*(pr_bData + 16)) << 24) + (*(pr_bData + 15) << 16) +
				  ((*(pr_bData + 14)) << 8) + ((*(pr_bData + 13)) << 0));
		HDMI_HDR_LOG(" 5 data0=0x%x, data1=0x%x, data2=0x%x, data3=0x%x\n",
				  ((*(pr_bData + 13)) << 0), ((*(pr_bData + 14)) << 8),
				  (*(pr_bData + 15) << 16), ((*(pr_bData + 16)) << 24));
		vWriteByteHdmiGRL(TOP_GEN4_PKT05,
				  (*(pr_bData + 19) << 16) + ((*(pr_bData + 18)) << 8) +
				  ((*(pr_bData + 17)) << 0));
		HDMI_HDR_LOG(" 6 data0=0x%x, data1=0x%x, data2=0x%x\n",
				  ((*(pr_bData + 17)) << 0), ((*(pr_bData + 18)) << 8),
				  (*(pr_bData + 19) << 16));
		vWriteByteHdmiGRL(TOP_GEN4_PKT06,
				  ((*(pr_bData + 23)) << 24) + (*(pr_bData + 22) << 16) +
				  ((*(pr_bData + 21)) << 8) + ((*(pr_bData + 20)) << 0));
		HDMI_HDR_LOG(" 7 data0=0x%x, data1=0x%x, data2=0x%x, data3=0x%x\n",
				  ((*(pr_bData + 20)) << 0), ((*(pr_bData + 21)) << 8),
				  (*(pr_bData + 22) << 16), ((*(pr_bData + 23)) << 24));
		vWriteByteHdmiGRL(TOP_GEN4_PKT07,
				  ((*(pr_bData + 25)) << 8) + ((*(pr_bData + 24)) << 0));
		HDMI_HDR_LOG(" 8 data0=0x%x, data1=0x%x\n", ((*(pr_bData + 24)) << 0),
				  ((*(pr_bData + 25)) << 8));
		vWriteHdmiGRLMsk(TOP_INFO_RPT, GEN4_RPT_EN, GEN4_RPT_EN);
		vWriteHdmiGRLMsk(TOP_INFO_EN, GEN4_EN | GEN4_EN_WR, GEN4_EN | GEN4_EN_WR);

	} else if (bEnable == HDR_PACKET_ZERO) {
		vWriteByteHdmiGRL(TOP_GEN4_HEADER,
				  (HDR_LEN << 16) + (HDR_VERS << 8) + (HDR_TYPE << 0));
		bHDR_CHSUM = HDR_LEN + HDR_VERS + HDR_TYPE;
		bHDR_CHSUM = 0x100 - bHDR_CHSUM;
		vWriteByteHdmiGRL(TOP_GEN4_PKT00, bHDR_CHSUM);
		vWriteByteHdmiGRL(TOP_GEN4_PKT01, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT02, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT03, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT04, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT05, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT06, 0);
		vWriteByteHdmiGRL(TOP_GEN4_PKT07, 0);

		vWriteHdmiGRLMsk(TOP_INFO_RPT, GEN4_RPT_EN, GEN4_RPT_EN);
		vWriteHdmiGRLMsk(TOP_INFO_EN, GEN4_EN | GEN4_EN_WR, GEN4_EN | GEN4_EN_WR);

	}

}

void vSendStaticHdrInfoFrame(void)
{
	vHalSendStaticHdrInfoFrame(_bStaticHdrStatus, _bHdrMetadataBuff);
}

void vSetHdr10TimeDelayOff(unsigned int i4_count)
{
	HDMI_DRV_FUNC();
	hdmi_TmrValue[HDMI_HDR10_DELAY_OFF_CMD] = i4_count;
}

bool isHdr10TimeDelayOffActivity(void)
{
	if (hdmi_TmrValue[HDMI_HDR10_DELAY_OFF_CMD] != 0)
		return true;
	else
		return false;
}

void Hdr10OffImmediately(void)
{
	if (_bHdrType == VID_PLA_DR_TYPE_HDR10
		 || _bHdrType == VID_PLA_DR_TYPE_HDR10_PLUS_VSIF
		 || isHdr10TimeDelayOffActivity()) {
		vSetHdr10TimeDelayOff(0);
		_bStaticHdrStatus = HDR_PACKET_DISABLE;
		vSendStaticHdrInfoFrame();
		if (_bHdrType == VID_PLA_DR_TYPE_HDR10)
			_bHdrType = VID_PLA_DR_TYPE_SDR;
		HDMI_HDR_LOG("[imd]HDR10 off done\n");
	} else
		HDMI_HDR_DBG_LOG("[imd]HDR10 already off\n");
}

void Hdr10DelayOffHandler(void)
{
	HDMI_DRV_FUNC();
	if (_bStaticHdrStatus == HDR_PACKET_ZERO) {
		Hdr10OffImmediately();
		HDMI_HDR_LOG("[delay]HDR10 off done\n");
	} else
		HDMI_HDR_LOG("[delay]HDR10 already off\n");

}

void HdrDelayAllOffImmediately(void)
{
	Hdr10OffImmediately();
	Hdr10pVsifOffImmediately();
}

void vHdrEnable(bool fgEnable)
{
	if (_u4HdrDebugDisableType & HDR_DEBUG_DISABLE_HDR)
		return;
	HDMI_PLUG_LOG(" fgEnable = %d, _bStaticHdrStatus =%d,  _bHdrType=%d\n", fgEnable,
		      _bStaticHdrStatus, _bHdrType);
	if (fgEnable) {
		HdrDelayAllOffImmediately();
		_bStaticHdrStatus = HDR_PACKET_ACTIVE;
		vSendStaticHdrInfoFrame();
		_bHdrType = VID_PLA_DR_TYPE_HDR10;
	} else {
		if (_bStaticHdrStatus == HDR_PACKET_ACTIVE) {
			_bStaticHdrStatus = HDR_PACKET_ZERO;
			vSendStaticHdrInfoFrame();
			vSetHdr10TimeDelayOff(2000);
			HDMI_HDR_LOG("HDR10 SET TIMER\n");
		}
	}
}

void vHalEnablePacket(PACKET_HW_T hw, bool en)
{
	if (en) {
		vWriteHdmiGRLMsk(hw.addr_rep_en, hw.mask_rep_en, hw.mask_rep_en);
		vWriteHdmiGRLMsk(hw.addr_wr_en, hw.mask_wr_en, hw.mask_wr_en);
	} else {
		vWriteHdmiGRLMsk(hw.addr_wr_en, 0, hw.mask_wr_en);
		vWriteHdmiGRLMsk(hw.addr_rep_en, 0, hw.mask_rep_en);
	}
}

void vHalSendPacket(PACKET_HW_T hw, unsigned char hb[3], unsigned char pb[28])
{
	unsigned char i;

	vHalEnablePacket(hw, false);
	vWriteByteHdmiGRL(hw.addr_header, (hb[2] << 16) + (hb[1] << 8) + (hb[0] << 0));
	for (i = 0; i < 4; i++) {
		vWriteByteHdmiGRL(hw.addr_pkt + 8 * i,
			(pb[3 + 7 * i] << 24) + (pb[2 + 7 * i] << 16) + (pb[1 + 7 * i] << 8) + (pb[0 + 7 * i] << 0));
		vWriteByteHdmiGRL(hw.addr_pkt + 8 * i + 4,
			(pb[6 + 7 * i] << 16) + (pb[5 + 7 * i] << 8) + (pb[4 + 7 * i] << 0));
	}
	vHalEnablePacket(hw, true);

	HDMI_HDR_DBG_LOG("hw_num=%d, hb[0-2]=0x%02x,0x%02x,0x%02x\n", hw.hw_num, hb[0], hb[1], hb[2]);
	HDMI_HDR_DBG_LOG("pb[0-27]=");
	for (i = 0; i < EMP_SIZE_MAX; i++)
		HDMI_HDR_DBG_LOG(",0x%02x", pb[i]);
	HDMI_HDR_DBG_LOG("\n");
}

void vDynamicHdrEMP_HeaderInit(unsigned char *hb, unsigned int *size,
	unsigned char *sequence_index, char *first, char *last)
{
	*(hb + 0) = DYNAMIC_HDR_EMP_TYPE;
	if (*size <= EMP_SIZE_FIRST) {
		*first = 1;
		*last = 1;
		*sequence_index = 0;
	} else if ((EMP_SIZE_FIRST + (*sequence_index) * EMP_SIZE_MAX) < *size) {
		if (*sequence_index == 0)
			*first = 1;
		else
			*first = 0;
		*last = 0;
	} else if ((EMP_SIZE_FIRST + (*sequence_index) * EMP_SIZE_MAX) >= *size) {
		*first = 0;
		*last = 1;
	}
	*(hb + 1) = (((*first) << 7) | ((*last) << 6)) & 0xc0;
	*(hb + 2) = *sequence_index;
}

void vHalSendDynamicHdrEMPs(char bEnable, VID_HDR10_PLUS_METADATA_UNION_T *pr_bData)
{
	unsigned char HB[3] = {0};
	unsigned char PB[EMP_SIZE_MAX] = {0};
	unsigned char i, pkthw_index;
	unsigned char first, last, pb_new, pb_end, dataset_tagm, dataset_tagl, dataset_lenm, dataset_lenl;
	unsigned char sequence_index = 0;
	unsigned int size = pr_bData->hdr10p_metadata_info.ui4_Hdr10PlusSize;
	unsigned char *meta = (unsigned char *)(unsigned long)pr_bData->hdr10p_metadata_info.ui4_Hdr10PlusAddr;
	PACKET_HW_T pkthw_list[GEN_PKT_HW_NUM] = {
		pkthw[GEN_PKT_HW1], pkthw[GEN_PKT_HW2], pkthw[GEN_PKT_HW3], pkthw[GEN_PKT_HW4],
		pkthw[GEN_PKT_HW5], pkthw[GEN_PKT_HW6], pkthw[GEN_PKT_HW7], pkthw[GEN_PKT_HW8],
		pkthw[GEN_PKT_HW9], pkthw[GEN_PKT_HW10], pkthw[GEN_PKT_HW11], pkthw[GEN_PKT_HW12],
		pkthw[GEN_PKT_HW13], pkthw[GEN_PKT_HW14], pkthw[GEN_PKT_HW15],
	};

	HDMI_HDR_DBG_LOG("%s(),%d, meta=%p, size=%d\n", __func__, __LINE__, meta, size);
	/*tset_size[0-3]=78,56,34,12  */

	for (pkthw_index = 0; pkthw_index < GEN_PKT_HW_NUM; pkthw_index++)
		vHalEnablePacket(pkthw_list[pkthw_index], false);
	vDynamicHdrEMP_HeaderInit(HB, &size, &sequence_index, &first, &last);

	if (first) {
		pb_new = 1;
		pb_end = 0;
		dataset_tagm = 0x00;
		dataset_tagl = 0x04;
		dataset_lenm = (unsigned char)((size >> 8) & 0xff);
		dataset_lenl = (unsigned char)(size & 0xff);
		PB[0] = (pb_new << 7) | (pb_end << 6) | (DYNAMIC_HDR_EMP_DS_TYPE << 4) |
			(DYNAMIC_HDR_EMP_AFR  << 3) | (DYNAMIC_HDR_EMP_VFR << 2) | (DYNAMIC_HDR_EMP_SYNC << 1);
		PB[1] = 0;
		PB[2] = DYNAMIC_HDR_EMP_ORGID;
		PB[3] = dataset_tagm;
		PB[4] = dataset_tagl;
		PB[5] = dataset_lenm;
		PB[6] = dataset_lenl;

		if (last)
			for (i = 0; i < size; i++)
				*(PB + 7 + i) = *(meta + i);
		else
			for (i = 0; i < EMP_SIZE_FIRST; i++)
				*(PB + 7 + i) = *(meta + i);

		pkthw_index = (sequence_index % GEN_PKT_HW_NUM);
		vHalSendPacket(pkthw_list[pkthw_index], HB, PB);
		sequence_index++;
		if (last) {
			HDMI_HDR_DBG_LOG("%s(),%d, Return in first\n", __func__, __LINE__);
			return;
		}
	}

	vDynamicHdrEMP_HeaderInit(HB, &size, &sequence_index, &first, &last);
	while (last == 0) {
		for (i = 0; i < EMP_SIZE_MAX; i++)
			*(PB + i) = (*(meta + ((sequence_index - 1) * EMP_SIZE_MAX) + EMP_SIZE_FIRST + i));
		pkthw_index = (sequence_index % GEN_PKT_HW_NUM);
		vHalSendPacket(pkthw_list[pkthw_index], HB, PB);
		sequence_index++;
		vDynamicHdrEMP_HeaderInit(HB, &size, &sequence_index, &first, &last);
	};

	for (i = 0; i < EMP_SIZE_MAX; i++)
		*(PB + i) = 0;

	HDMI_HDR_DBG_LOG("%s(),%d ,left size=%d\n", __func__, __LINE__,
		(size - ((sequence_index - 1) * EMP_SIZE_MAX) - EMP_SIZE_FIRST));
	for (i = 0; i < (size - ((sequence_index - 1) * EMP_SIZE_MAX) - EMP_SIZE_FIRST); i++)
		*(PB + i) = (*(meta + ((sequence_index - 1) * EMP_SIZE_MAX) + EMP_SIZE_FIRST + i));
	pkthw_index = (sequence_index % GEN_PKT_HW_NUM);
	vHalSendPacket(pkthw_list[pkthw_index], HB, PB);
	sequence_index = 0;
}

void vSetHdrDebugType(unsigned int u4Data)
{
	_u4HdrDebugDisableType = u4Data;
}

void vHalSendHdr10PlusVSIF(char bEnable, VID_HDR10_PLUS_METADATA_UNION_T *pr_bData)
{
	unsigned char i;
	unsigned char HB[3] = {0};
	unsigned char PB[DYNAMIC_HDR10P_VSIF_MAXLEN + 1] = {0};
	unsigned char chsum = 0;
	unsigned char size = (unsigned char)(pr_bData->hdr10p_metadata_info.ui4_Hdr10PlusSize & 0xff);
	unsigned char *meta = (unsigned char *)(unsigned long)pr_bData->hdr10p_metadata_info.ui4_Hdr10PlusAddr;

	if (size > DYNAMIC_HDR10P_VSIF_MAXLEN) {
		TX_DEF_LOG("Error in %s,size=%d", __func__, size);
		return;
	}

	HB[0] = 0x80 | DYNAMIC_HDR10P_VSIF_TYPE;
	HB[1] = DYNAMIC_HDR10P_VSIF_VERSION;
	HB[2] = size & 0x1f;
	chsum = HB[0] + HB[1] + HB[2];
	for (i = 0; i < size; i++) {
		PB[i + 1] = *(meta + i);
		chsum += PB[i + 1];
	}
	PB[0] = 0x100 - chsum;
	vHalSendPacket(pkthw[DYNAMIC_HDR10P_VSIF_PKTHW], HB, PB);
}

void vSetHdr10pVsifTimeDelayOff(unsigned int i4_count)
{
	HDMI_DRV_FUNC();
	hdmi_TmrValue[HDMI_HDR10P_VSIF_DELAY_OFF_CMD] = i4_count;
}

bool isHdr10pVsifTimeDelayOffActivity(void)
{
	if (hdmi_TmrValue[HDMI_HDR10P_VSIF_DELAY_OFF_CMD] != 0)
		return true;
	else
		return false;
}

void Hdr10pVsifOffImmediately(void)
{
	PACKET_HW_T mpkthw = pkthw[DYNAMIC_HDR10P_VSIF_PKTHW];

	if (_bHdrType == VID_PLA_DR_TYPE_HDR10_PLUS_VSIF
		|| isHdr10pVsifTimeDelayOffActivity()) {
		vSetHdr10pVsifTimeDelayOff(0);
		vHalEnablePacket(mpkthw, false);
		if (_bHdrType == VID_PLA_DR_TYPE_HDR10_PLUS_VSIF)
			_bHdrType = VID_PLA_DR_TYPE_SDR;
		HDMI_HDR_LOG("[imd]Hdr10pVsif off done\n");
	} else
		HDMI_HDR_DBG_LOG("[imd]Hdr10pVsif already off\n");
}

void Hdr10pVsifDelayOffHandler(void)
{
	HDMI_DRV_FUNC();
	Hdr10OffImmediately();
	Hdr10pVsifOffImmediately();
}

void vHdr10PlusEnable(bool fgEnable)
{
	unsigned char pkthw_index;

	HDMI_PLUG_LOG("[vHdr10PlusEnable],%d\n", fgEnable);
	if (fgEnable) {
		HdrDelayAllOffImmediately();
		for (pkthw_index = 0; pkthw_index < GEN_PKT_HW_NUM; pkthw_index++)
			vHalEnablePacket(pkthw[pkthw_index], true);
		_bHdrType = VID_PLA_DR_TYPE_HDR10_PLUS;
	} else {
		for (pkthw_index = 0; pkthw_index < GEN_PKT_HW_NUM; pkthw_index++)
			vHalEnablePacket(pkthw[pkthw_index], false);
		_bHdrType = VID_PLA_DR_TYPE_SDR;
	}
}

void vHdr10PlusVSIF_ZeroPacket(void)
{
	char testbuf[27] = {0};
	VID_PLA_HDR_METADATA_INFO_T hdr_metadata;

	hdr_metadata.e_DynamicRangeType = VID_PLA_DR_TYPE_HDR10_PLUS_VSIF;
	hdr_metadata.metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusSize =
		27;
	hdr_metadata.metadata_info.hdr10_plus_metadata.hdr10p_metadata_info.ui4_Hdr10PlusAddr =
		(unsigned long)(&testbuf);
	vVdpSetHdrMetadata(true, hdr_metadata);
}

void vHdr10PlusVSIFEnable(bool fgEnable, unsigned int forcedHdrType, int dolbyOpFmt)
{
	PACKET_HW_T mpkthw = pkthw[DYNAMIC_HDR10P_VSIF_PKTHW];

	HDMI_PLUG_LOG("vHdr10PlusVSIFEnable:fgEnable=%d, forcedHdrType=%u, dolbyOpFmt=%d\n", fgEnable, forcedHdrType, dolbyOpFmt);
	if (fgEnable) {
		HdrDelayAllOffImmediately();
		_bStaticHdrStatus = HDR_PACKET_ACTIVE;
		vSendStaticHdrInfoFrame();
		vHalEnablePacket(mpkthw, true);
		_bHdrType = VID_PLA_DR_TYPE_HDR10_PLUS_VSIF;
	} else {
		if ((forcedHdrType > 1) && (dolbyOpFmt == 1)) {
			/* if VS10 is enabled, we will be going back to HDR10
			mode from HDR10+. For this transition, we should not
			send zero VSIF and should not disable HDR10 DRMIF. We
			only need to disable HDR10+ VSIF. HDR10 DRMIF will be
			updated as soon as VS10 path updates the metadata */
			Hdr10pVsifOffImmediately();
		} else {
			_bStaticHdrStatus = HDR_PACKET_ZERO;
			vSendStaticHdrInfoFrame();
			vHdr10PlusVSIF_ZeroPacket();
			vSetHdr10pVsifTimeDelayOff(2000);
			HDMI_HDR_LOG("HDR10+VSIF SET TIMER\n");
		}
	}
}

void vBT2020Enable(bool fgEnable)
{
	if (_u4HdrDebugDisableType & HDR_DEBUG_DISABLE_BT2020)
		return;

	HDMI_PLUG_LOG(" _fgBT2020Enable =%d\n", fgEnable);
	_fgBT2020Enable = fgEnable;
	vSendAVIInfoFrame(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
	vHDMIVideoOutput(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
}

void vSetStaticHdrType(char bType)
{
	if (bType == GAMMA_HLG) {
		_bStaticHdrType = GAMMA_HLG;
		*_bHdrMetadataBuff = 0x03;	/* EOTF */
	} else {
		_bStaticHdrType = GAMMA_ST2084;
		*_bHdrMetadataBuff = 0x02;	/* EOTF */
	}

	HDMI_PLUG_LOG(" bType = %d, _bStaticHdrType =%d\n", bType, _bStaticHdrType);

}

void vHalSendDolbyVSIF(bool fgEnable, bool fgLowLatency, bool fgDolbyVisionSignal,
			 bool fgBackltCtrlMdPresent, unsigned int u4EfftmaxPQ)
{
	unsigned char bData[6];
	unsigned char bHDR_CHSUM = 0;
	unsigned char i;

	vWriteHdmiGRLMsk(TOP_INFO_EN, 0, GEN5_EN | GEN5_EN_WR);
	vWriteHdmiGRLMsk(TOP_INFO_RPT, 0, GEN5_RPT_EN);

	if (fgEnable) {
		bData[0] = 0x46;
		bData[1] = 0xD0;
		bData[2] = 0x00;
		bData[3] = (fgDolbyVisionSignal << 1) | fgLowLatency;
		bData[4] = (fgBackltCtrlMdPresent << 7) | ((u4EfftmaxPQ >> 8) & 0x0F);
		bData[5] = u4EfftmaxPQ & 0xFF;
		if (fgLowLatency)
			bData[3] |= 1;
		if (fgDolbyVisionSignal)
			bData[3] |= (1 << 1);
		if (fgBackltCtrlMdPresent)
			bData[4] |= (1 << 7);

		vWriteByteHdmiGRL(TOP_GEN5_HEADER,
			(DOLBYVSIF_LEN << 16) + (DOLBYVSIF_VERS << 8) + (DOLBYVSIF_TYPE << 0));
		bHDR_CHSUM = DOLBYVSIF_LEN + DOLBYVSIF_VERS + DOLBYVSIF_TYPE;
		for (i = 0; i < 6; i++)
			bHDR_CHSUM += bData[i];
		bHDR_CHSUM = 0x100 - bHDR_CHSUM;
		vWriteByteHdmiGRL(TOP_GEN5_PKT00,
			(bData[2] << 24) + (bData[1] << 16) + (bData[0] << 8) + (bHDR_CHSUM << 0));
		vWriteByteHdmiGRL(TOP_GEN5_PKT01, (bData[5] << 16) + (bData[4] << 8) + (bData[3] << 0));
		vWriteByteHdmiGRL(TOP_GEN5_PKT02, 0);
		vWriteByteHdmiGRL(TOP_GEN5_PKT03, 0);
		vWriteByteHdmiGRL(TOP_GEN5_PKT04, 0);
		vWriteByteHdmiGRL(TOP_GEN5_PKT05, 0);
		vWriteByteHdmiGRL(TOP_GEN5_PKT06, 0);
		vWriteByteHdmiGRL(TOP_GEN5_PKT07, 0);
		vWriteHdmiGRLMsk(TOP_INFO_RPT, GEN5_RPT_EN, GEN5_RPT_EN);
		vWriteHdmiGRLMsk(TOP_INFO_EN, GEN5_EN | GEN5_EN_WR, GEN5_EN | GEN5_EN_WR);
	}
}

bool fgUseDolbyVSIF(void)
{
	if (_HdmiSinkAvCap.ui4_sink_dolbyvision_vsvdb_low_latency_support)
		return TRUE;
	else
		return FALSE;
}

void vTrySendDolbyVSIF(HDMI_VIDEO_RESOLUTION res)
{
	if (_fgLowLatencyDolbyVisionEnable || (_fgDolbyHdrEnable && fgUseDolbyVSIF()))
		vHalSendDolbyVSIF(TRUE, _fgLowLatencyDolbyVisionEnable,
		TRUE, _fgBackltCtrlMDPresent, _u4EffTmaxPQ);
}

void vDolbyHdrEnable(bool fgEnable)
{
	if (_u4HdrDebugDisableType & HDR_DEBUG_DISABLE_DOLBY_HDR)
		return;

	dovi_off_delay_needed = FALSE;
	if ((_fgDolbyHdrEnable == TRUE) && (fgEnable == FALSE))
		dovi_off_delay_needed = TRUE;

	HDMI_PLUG_LOG(" _fgDolbyHdrEnable =%d\n", fgEnable);
	_fgDolbyHdrEnable = fgEnable;

	if (fgEnable) {
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa5551);
		HdrDelayAllOffImmediately();
		vSendVendorSpecificInfoFrame(_stAvdAVInfo.e_resolution);
		vTrySendDolbyVSIF(_stAvdAVInfo.e_resolution);
		_bHdrType = VID_PLA_DR_TYPE_DOVI;
	} else {
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa5550);
		if (!fgUseDolbyVSIF())
			vSendVendorSpecificInfoFrame(_stAvdAVInfo.e_resolution);
		else
			vHalSendDolbyVSIF(TRUE, 0, 0, 0, 0);
		_bHdrType = VID_PLA_DR_TYPE_SDR;
	}
	vSendAVIInfoFrame(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
	vHDMIVideoOutput(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
}

void vLowLatencyDolbyVisionEnable(bool fgEnable)
{
	if (fgEnable == _fgLowLatencyDolbyVisionEnable)
		return;

	HDMI_PLUG_LOG(" is_enable =%d\n", fgEnable);
	_fgLowLatencyDolbyVisionEnable = fgEnable;

	if (fgEnable) {
		HdrDelayAllOffImmediately();
		vBT2020Enable(TRUE);
		vHalSendDolbyVSIF(TRUE, TRUE, TRUE, _fgBackltCtrlMDPresent, _u4EffTmaxPQ);
		vSendVendorSpecificInfoFrame(_stAvdAVInfo.e_resolution);
		_bHdrType = VID_PLA_DR_TYPE_DOVI_LOWLATENCY;
	} else {
		vBT2020Enable(FALSE);
		vHalSendDolbyVSIF(TRUE, 0, 0, 0, 0);
		vSendVendorSpecificInfoFrame(_stAvdAVInfo.e_resolution);
		_bHdrType = VID_PLA_DR_TYPE_SDR;
	}
}

void vInitHdr(void)
{
	HDMI_STATIC_METADATA_INFO_T initbuf;

	_bHdrMetadataBuff = kmalloc(256, GFP_KERNEL);
	if (_bHdrMetadataBuff == NULL)
		HDMI_HDR_LOG
		    ("[HDR] vInitHdrMetadata Buffer Big error allocate Mem == NULL\n");

	initbuf.ui1_EOTF = 0x02;
	initbuf.ui1_Static_Metadata_ID = 0;
	initbuf.ui2_DisplayPrimariesX0 = 35400;
	initbuf.ui2_DisplayPrimariesY0 = 14600;
	initbuf.ui2_DisplayPrimariesX1 = 8500;
	initbuf.ui2_DisplayPrimariesY1 = 39850;
	initbuf.ui2_DisplayPrimariesX2 = 6550;
	initbuf.ui2_DisplayPrimariesY2 = 2300;
	initbuf.ui2_WhitePointX = 15635;
	initbuf.ui2_WhitePointY = 16450;
	initbuf.ui2_MaxDisplayMasteringLuminance = 1000;
	initbuf.ui2_MinDisplayMasteringLuminance = 50;
	initbuf.ui2_MaxCLL = 0;
	initbuf.ui2_MaxFALL = 0;
	memcpy(_bHdrMetadataBuff, &initbuf, sizeof(HDMI_STATIC_METADATA_INFO_T));

	_u4HdrDebugDisableType = 0;

	HDMI_HDR_LOG("%s,hdmi_boot_forcehdr=%d\n", __func__, hdmi_boot_forcehdr);

	switch (hdmi_boot_forcehdr) {
	case LK_HDR_TYPE_SDR:
		_fgBT2020Enable = 0;
		_bHdrType = VID_PLA_DR_TYPE_SDR;
		break;
	case LK_HDR_TYPE_HDR10_ST2084:
		_fgBT2020Enable = 1;
		vSetStaticHdrType(GAMMA_ST2084);
		_bHdrType = VID_PLA_DR_TYPE_HDR10;
		_bStaticHdrStatus = HDR_PACKET_ACTIVE;
		break;
	case LK_HDR_TYPE_HDR10_HLG:
		_fgBT2020Enable = 0;
		vSetStaticHdrType(GAMMA_HLG);
		_bHdrType = VID_PLA_DR_TYPE_HDR10;
		_bStaticHdrStatus = HDR_PACKET_ACTIVE;
		break;
	case LK_HDR_TYPE_DOVI_STD:
		_bHdrType = VID_PLA_DR_TYPE_DOVI;
		_fgDolbyHdrEnable = 1;
		_fgBT2020Enable = 0;
		break;
	case LK_HDR_TYPE_DOVI_LOWLATENCY:
		_bHdrType = VID_PLA_DR_TYPE_DOVI_LOWLATENCY;
		_fgBT2020Enable = 1;
		_fgLowLatencyDolbyVisionEnable = 1;
		break;
	default:
		break;
	}

}

void vVdpSetHdrMetadata(bool enable, VID_PLA_HDR_METADATA_INFO_T hdr_metadata)
{
	HDMI_STATIC_METADATA_INFO_T _bStaticHdrMetadata;
	bool fgMetadataSame = false;

	if (_u4HdrDebugDisableType & HDR_DEBUG_DISABLE_METADATA)
		return;

	HDMI_HDR_LOG(" set HDR metadata Type=%d\n", hdr_metadata.e_DynamicRangeType);

	if (hdr_metadata.e_DynamicRangeType == VID_PLA_DR_TYPE_HDR10) {
		if (_bStaticHdrType == GAMMA_HLG)
			_bStaticHdrMetadata.ui1_EOTF = 0x03;	/* HLG */
		else
			_bStaticHdrMetadata.ui1_EOTF = 0x02;	/* 2084 */

		_bStaticHdrMetadata.ui1_Static_Metadata_ID = 0;
		_bStaticHdrMetadata.ui2_DisplayPrimariesX0 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[0];
		_bStaticHdrMetadata.ui2_DisplayPrimariesY0 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[0];
		_bStaticHdrMetadata.ui2_DisplayPrimariesX1 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[1];
		_bStaticHdrMetadata.ui2_DisplayPrimariesY1 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[1];
		_bStaticHdrMetadata.ui2_DisplayPrimariesX2 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesX[2];
		_bStaticHdrMetadata.ui2_DisplayPrimariesY2 =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_DisplayPrimariesY[2];
		_bStaticHdrMetadata.ui2_WhitePointX =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_WhitePointX;
		_bStaticHdrMetadata.ui2_WhitePointY =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_WhitePointY;
		_bStaticHdrMetadata.ui2_MaxDisplayMasteringLuminance =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_MaxDisplayMasteringLuminance;
		_bStaticHdrMetadata.ui2_MinDisplayMasteringLuminance =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_MinDisplayMasteringLuminance;
		_bStaticHdrMetadata.ui2_MaxCLL =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_MaxCLL;
		_bStaticHdrMetadata.ui2_MaxFALL =
		    hdr_metadata.metadata_info.hdr10_metadata.ui2_MaxFALL;

		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesX0=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesX0);
		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesY0=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesY0);
		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesX1=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesX1);
		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesY1=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesY1);
		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesX2=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesX2);
		HDMI_HDR_LOG(" set HDR ui2_DisplayPrimariesY2=0x%x\n",
			      _bStaticHdrMetadata.ui2_DisplayPrimariesX2);
		HDMI_HDR_LOG(" set HDR ui2_WhitePointX=0x%x\n",
			      _bStaticHdrMetadata.ui2_WhitePointX);
		HDMI_HDR_LOG(" set HDR ui2_WhitePointY=0x%x\n",
			      _bStaticHdrMetadata.ui2_WhitePointY);
		HDMI_HDR_LOG(" set HDR ui2_MaxDisplayMasteringLuminance=0x%x\n",
			      _bStaticHdrMetadata.ui2_MaxDisplayMasteringLuminance);
		HDMI_HDR_LOG(" set HDR ui2_MinDisplayMasteringLuminance=0x%x\n",
			      _bStaticHdrMetadata.ui2_MinDisplayMasteringLuminance);
		HDMI_HDR_LOG(" set HDR ui2_MaxCLL=0x%x\n",
			      _bStaticHdrMetadata.ui2_MaxCLL);
		memcpy(_bHdrMetadataBuff, &_bStaticHdrMetadata, sizeof(HDMI_STATIC_METADATA_INFO_T));
		vSendStaticHdrInfoFrame();
	} else if (hdr_metadata.e_DynamicRangeType == VID_PLA_DR_TYPE_DOVI_LOWLATENCY) {
		if ((_fgBackltCtrlMDPresent ==
			 hdr_metadata.metadata_info.dovi_lowlatency_metadata.fgBackltCtrlMdPresent)
			&& (_u4EffTmaxPQ ==
			 hdr_metadata.metadata_info.dovi_lowlatency_metadata.ui4_EffTmaxPQ)) {
			fgMetadataSame = TRUE;
		} else {
			_fgBackltCtrlMDPresent  =
				 hdr_metadata.metadata_info.dovi_lowlatency_metadata.fgBackltCtrlMdPresent;
			if (_fgBackltCtrlMDPresent)
				_u4EffTmaxPQ = hdr_metadata.metadata_info.dovi_lowlatency_metadata.ui4_EffTmaxPQ;
			else
				_u4EffTmaxPQ = 0;
			HDMI_HDR_LOG(" %d, _u4EffTmaxPQ =0x%x\n ", _fgBackltCtrlMDPresent, _u4EffTmaxPQ);
		}
		if ((!fgMetadataSame) && _fgLowLatencyDolbyVisionEnable)
			vHalSendDolbyVSIF(TRUE, TRUE, TRUE, _fgBackltCtrlMDPresent, _u4EffTmaxPQ);
	} else if (hdr_metadata.e_DynamicRangeType == VID_PLA_DR_TYPE_HDR10_PLUS) {
		HDMI_HDR_LOG("[HDR10+] [vVdpSetHdrMetadata]\n");
		vHalSendDynamicHdrEMPs(enable, &(hdr_metadata.metadata_info.hdr10_plus_metadata));
	} else if (hdr_metadata.e_DynamicRangeType == VID_PLA_DR_TYPE_HDR10_PLUS_VSIF) {
		HDMI_HDR_LOG("[HDR10+] [vVdpSetHdrMetadata] VSIF\n");
		vHalSendHdr10PlusVSIF(enable, &(hdr_metadata.metadata_info.hdr10_plus_metadata));
	}
}

VID_PLA_DR_TYPE_T hdr_status(char *str)
{
	VID_PLA_DR_TYPE_T status;

	switch (_bHdrType) {
	case VID_PLA_DR_TYPE_SDR:
		status = VID_PLA_DR_TYPE_SDR;
		sprintf(str, "%s", "SDR");
		break;
	case VID_PLA_DR_TYPE_DOVI:
		status = VID_PLA_DR_TYPE_DOVI;
		sprintf(str, "%s", "DolbyVision-Std");
		break;
	case VID_PLA_DR_TYPE_DOVI_LOWLATENCY:
		status = VID_PLA_DR_TYPE_DOVI_LOWLATENCY;
		sprintf(str, "%s", "DolbyVision-Lowlatency");
		break;
	case VID_PLA_DR_TYPE_HDR10:
		status = VID_PLA_DR_TYPE_HDR10;
		if (_bStaticHdrType == GAMMA_HLG)
			sprintf(str, "%s", "HDR10-GAMMA_HLG");
		else if (_bStaticHdrType == GAMMA_ST2084)
			sprintf(str, "%s", "HDR10-GAMMA_ST2084");
		else
			sprintf(str, "%s", "HDR10-others");
		break;
	case VID_PLA_DR_TYPE_HDR10_PLUS:
		status = VID_PLA_DR_TYPE_HDR10_PLUS;
		sprintf(str, "%s", "HDR10Plus-EMP");
		break;
	case VID_PLA_DR_TYPE_HDR10_PLUS_VSIF:
		status = VID_PLA_DR_TYPE_HDR10_PLUS_VSIF;
		sprintf(str, "%s", "HDR10Plus-VSIF");
		break;
	default:
		sprintf(str, "%s", "Error HDR type");
		HDMI_HDR_LOG("%s, error type of hdr\n", __func__);
	}

	return status;
}

void vshow_hdr_status(void)
{
	char str[50];
	VID_PLA_DR_TYPE_T ret;

	ret = hdr_status(str);
	HDMI_PLUG_LOG("%s\n", str);
	HDMI_PLUG_LOG("ret=%d\n", ret);
}

void vcommon_status(char *str)
{
	sprintf(str, "%s\n%s\n%s\n\n",
		szHdmiResStr[_stAvdAVInfo.e_resolution], szHdmiDeepcolorStr[_stAvdAVInfo.e_deep_color_bit],
		szHdmiColorspaceStr[_stAvdAVInfo.e_video_color_space]);

}

void hdmi_timing_monitor_sel(unsigned int sel)
{
	unsigned int temp;

	temp = hdmi_vdout15_read(0x159c);
	if (sel == 0) /* pixel clk */
		temp = temp & (~(1 << 17));
	else /* tmds clk */
		temp = temp | (1 << 17);
	hdmi_vdout15_write(0x159c, temp);
}

unsigned int hdmi_clock_read(unsigned char sel)
{
	unsigned int temp = 0;

	if (sel == 0) {
		vWriteHdmiGRLMsk(HDMI_PLLMETER, 0, HDMI_PLLMETER_ENABLE);
		vWriteHdmiGRLMsk(HDMI_PLLMETER_TRIGGER, 0, HDMI_PLL_FREQ_TRGGIER);
		vWriteHdmiGRLMsk(HDMI_PLLMETER, HDMI_PLLMETER_ENABLE, HDMI_PLLMETER_ENABLE);
	} else if (sel == 1)
		temp = bReadByteHdmiGRL(HDMI_PLLMETER) >> 16;
	else if (sel == 2)
		vWriteHdmiGRLMsk(HDMI_PLLMETER_TRIGGER, HDMI_PLL_FREQ_TRGGIER, HDMI_PLL_FREQ_TRGGIER);

	return temp;
}

#endif
