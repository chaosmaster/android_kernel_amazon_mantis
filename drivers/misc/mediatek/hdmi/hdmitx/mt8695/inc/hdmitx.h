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

#ifndef HDMITX_H
#define     HDMITX_H

typedef enum {
	HDMI_VIDEO_720x480i_60Hz = 0,	/* 0 */
	HDMI_VIDEO_720x576i_50Hz,	/* 1 */
	HDMI_VIDEO_720x480p_60Hz,	/* 2 */
	HDMI_VIDEO_720x576p_50Hz,	/* 3 */
	HDMI_VIDEO_1280x720p_60Hz,	/* 4 */
	HDMI_VIDEO_1280x720p_50Hz,	/* 5 */
	HDMI_VIDEO_1920x1080i_60Hz,	/* 6 */
	HDMI_VIDEO_1920x1080i_50Hz,	/* 7 */
	HDMI_VIDEO_1920x1080p_30Hz,	/* 8 */
	HDMI_VIDEO_1920x1080p_25Hz,	/* 9 */
	HDMI_VIDEO_1920x1080p_24Hz,	/* a */
	HDMI_VIDEO_1920x1080p_23Hz,	/* b */
	HDMI_VIDEO_1920x1080p_29Hz,	/* c */
	HDMI_VIDEO_1920x1080p_60Hz,	/* d */
	HDMI_VIDEO_1920x1080p_50Hz,	/* e */

	HDMI_VIDEO_1280x720p3d_60Hz,	/* f */
	HDMI_VIDEO_1280x720p3d_50Hz,	/* 10 */
	HDMI_VIDEO_1920x1080i3d_60Hz,	/* 11 */
	HDMI_VIDEO_1920x1080i3d_50Hz,	/* 12 */
	HDMI_VIDEO_1920x1080p3d_24Hz,	/* 13 */
	HDMI_VIDEO_1920x1080p3d_23Hz,	/* 14 */

	HDMI_VIDEO_3840x2160P_23_976HZ,	/* 15 */
	HDMI_VIDEO_3840x2160P_24HZ,	/* 16 */
	HDMI_VIDEO_3840x2160P_25HZ,	/* 17 */
	HDMI_VIDEO_3840x2160P_29_97HZ,	/* 18 */
	HDMI_VIDEO_3840x2160P_30HZ,	/* 19 */
	HDMI_VIDEO_4096x2160P_24HZ,	/* 1a */
	HDMI_VIDEO_3840x2160P_60HZ,	/* 1b */
	HDMI_VIDEO_3840x2160P_50HZ,	/* 1c */
	HDMI_VIDEO_4096x2160P_60HZ,	/* 1d */
	HDMI_VIDEO_4096x2160P_50HZ,	/* 1e */

	HDMI_VIDEO_1280x720p_59_94Hz,	/* 1f */
	HDMI_VIDEO_1920x1080p_59_94Hz,	/* 20 */
	HDMI_VIDEO_3840x2160P_59_94HZ,	/* 21 */
	HDMI_VIDEO_4096x2160P_59_94HZ,	/* 22 */

	HDMI_VIDEO_RESOLUTION_NUM
} HDMI_VIDEO_RESOLUTION;

typedef enum {
	HDMI_FORCE_DEFAULT,
	HDMI_FORCE_SDR,
	HDMI_FORCE_HDR,
	HDMI_FORCE_HDR10,
	HDMI_FORCE_DOLBY_VISION,
	HDMI_FORCE_HLG,
	HDMI_FORCE_HDR10_PLUS,
} HDMI_FORCE_HDR_ENABLE;

typedef enum {
	HDMI_AUDIO_PCM_16bit_48000,
	HDMI_AUDIO_PCM_16bit_44100,
	HDMI_AUDIO_PCM_16bit_32000,
	HDMI_AUDIO_SOURCE_STREAM,
} HDMI_AUDIO_FORMAT;

#define DOVI_METADATA_MAX_LENGTH 100
#define PHLP_METADATA_MAX_LENGTH 100
#define HDR10_PLUS_METADATA_MAX_LENGTH 100

typedef enum _VID_GAMMA_T {
	GAMMA_ST2084 = 1,
	GAMMA_HLG = 2,
	GAMMA_24 = 1,
	GAMMA_709 = 2,
} VID_GAMMA_T;

typedef enum {
	VID_PLA_DR_TYPE_SDR = 0,
	VID_PLA_DR_TYPE_HDR10,
	VID_PLA_DR_TYPE_DOVI,
	VID_PLA_DR_TYPE_PHLP_RESVERD,
	VID_PLA_DR_TYPE_DOVI_LOWLATENCY,
	VID_PLA_DR_TYPE_HDR10_PLUS,
	VID_PLA_DR_TYPE_HDR10_PLUS_VSIF,
} VID_PLA_DR_TYPE_T;

typedef enum LK_HDR_TYPE {
	LK_HDR_TYPE_SDR = 0,
	LK_HDR_TYPE_DOVI_STD,
	LK_HDR_TYPE_DOVI_LOWLATENCY,
	LK_HDR_TYPE_HDR10_ST2084,
	LK_HDR_TYPE_HDR10_HLG,
	LK_HDR_TYPE_HDR10_PLUS_EMP,
	LK_HDR_TYPE_HDR10_PLUS_VSIF,
} LK_HDR_TYPE_T;

typedef struct {
	unsigned short ui2_DisplayPrimariesX[3];
	unsigned short ui2_DisplayPrimariesY[3];
	unsigned short ui2_WhitePointX;
	unsigned short ui2_WhitePointY;
	unsigned short ui2_MaxDisplayMasteringLuminance;
	unsigned short ui2_MinDisplayMasteringLuminance;
	unsigned short ui2_MaxCLL;
	unsigned short ui2_MaxFALL;
	unsigned char fgNeedUpdStaticMeta;
} VID_STATIC_HDMI_MD_T;

typedef enum {
	GAMUT_TYPE_AVCHD = 0,
	GAMUT_TYPE_AVCHD_FIXED = 1,
} XVYCC_GAMUT_TYPE_T;

typedef enum {
	HDMI_4_3 = 0,
	HDMI_16_9,
} HDMI_VIDEO_ASPECT_T;

typedef struct {
	unsigned short ui2_DisplayPrimariesX[3];
	unsigned int ui4_RpuIdx;
	unsigned int ui4_RpuStartAddr;
	unsigned int ui4_RpuSize;
} VID_PLA_DOVI_METADATA_INFO_T;

typedef struct {
	unsigned int ui4_PhilipHdrAddr;
	unsigned int ui4_PhilipHdrSize;
	unsigned int ui4_Index;

} VID_PLA_PHLP_METADATA_INFO_T;

typedef struct {
	unsigned long ui4_Hdr10PlusAddr;
	unsigned int ui4_Hdr10PlusSize;
	unsigned int ui4_Hdr10PlusIdx;
	unsigned int ui4_isTrustZone;
} VID_HDR10_PLUS_METADATA_INFO_T;

typedef union {
	char dovi_metada_buffer[DOVI_METADATA_MAX_LENGTH];
	VID_PLA_DOVI_METADATA_INFO_T dovi_metadata_info;
} VID_PLA_DOVI_METADATA_UNION_T;

typedef union {
	char phlp_metada_buffer[DOVI_METADATA_MAX_LENGTH];
	VID_PLA_PHLP_METADATA_INFO_T phlp_metadata_info;
} VID_PLA_PHLP_METADATA_UNION_T;

typedef struct {
	char fgBackltCtrlMdPresent;
	unsigned int ui4_EffTmaxPQ;
} VID_DOVI_LOWLATENCY_MD_INFO_T;

typedef union {
	char hdr10_plus_metada_buffer[HDR10_PLUS_METADATA_MAX_LENGTH];
	VID_HDR10_PLUS_METADATA_INFO_T hdr10p_metadata_info;
} VID_HDR10_PLUS_METADATA_UNION_T;

typedef union {
	VID_STATIC_HDMI_MD_T hdr10_metadata;
	VID_PLA_DOVI_METADATA_UNION_T dovi_metadata;
	VID_PLA_PHLP_METADATA_UNION_T phlp_metadata;
	VID_DOVI_LOWLATENCY_MD_INFO_T dovi_lowlatency_metadata;
	VID_HDR10_PLUS_METADATA_UNION_T hdr10_plus_metadata;
} VID_PLA_HDR_METADATA_UNION_T;

typedef struct {
	VID_PLA_DR_TYPE_T e_DynamicRangeType;
	VID_PLA_HDR_METADATA_UNION_T metadata_info;
	unsigned char fgIsMetadata;
} VID_PLA_HDR_METADATA_INFO_T;

typedef enum {
	HDMI_DEEP_COLOR_AUTO = 0,
	HDMI_NO_DEEP_COLOR,
	HDMI_DEEP_COLOR_10_BIT,
	HDMI_DEEP_COLOR_12_BIT,
	HDMI_DEEP_COLOR_16_BIT
} HDMI_DEEP_COLOR_T;

typedef enum {
	SV_I2S = 0,
	SV_SPDIF
} HDMI_AUDIO_INPUT_TYPE_T;

typedef enum {			/* new add 2007/9/12 */
	FS_16K = 0x00,
	FS_22K,
	FS_24K,
	FS_32K,
	FS_44K,
	FS_48K,
	FS_64K,
	FS_88K,
	FS_96K,
	FS_176K,
	FS_192K,
	FS512_44K,		/* for DSD */
	FS_768K,
	FS128_44k,
	FS_128K,
	FS_UNKNOWN,
	FS_48K_MAX_CH
} AUDIO_SAMPLING_T;

typedef enum {
	IEC_48K = 0,
	IEC_96K,
	IEC_192K,
	IEC_768K,
	IEC_44K,
	IEC_88K,
	IEC_176K,
	IEC_705K,
	IEC_16K,
	IEC_22K,
	IEC_24K,
	IEC_32K,


} IEC_FRAME_RATE_T;

typedef enum {
	HDMI_FS_32K = 0,
	HDMI_FS_44K,
	HDMI_FS_48K,
	HDMI_FS_88K,
	HDMI_FS_96K,
	HDMI_FS_176K,
	HDMI_FS_192K
} HDMI_AUDIO_SAMPLING_T;

typedef enum {
	PCM_16BIT = 0,
	PCM_20BIT,
	PCM_24BIT
} PCM_BIT_SIZE_T;

typedef enum {
	HDMI_RGB = 0,
	HDMI_RGB_FULL,
	HDMI_YCBCR_444,
	HDMI_YCBCR_422,
	HDMI_YCBCR_420,
	HDMI_XV_YCC,
	HDMI_YCBCR_444_FULL,
	HDMI_YCBCR_422_FULL,
	HDMI_YCBCR_420_FULL
} HDMI_OUT_COLOR_SPACE_T;

typedef enum {
	HDMI_RJT_24BIT = 0,
	HDMI_RJT_16BIT,
	HDMI_LJT_24BIT,
	HDMI_LJT_16BIT,
	HDMI_I2S_24BIT,
	HDMI_I2S_16BIT
} HDMI_AUDIO_I2S_FMT_T;

typedef enum {
	AUD_INPUT_1_0 = 0,
	AUD_INPUT_1_1,
	AUD_INPUT_2_0,
	AUD_INPUT_2_1,
	AUD_INPUT_3_0,		/* C,L,R */
	AUD_INPUT_3_1,		/* C,L,R */
	AUD_INPUT_4_0,		/* L,R,RR,RL */
	AUD_INPUT_4_1,		/* L,R,RR,RL */
	AUD_INPUT_5_0,
	AUD_INPUT_5_1,
	AUD_INPUT_6_0,
	AUD_INPUT_6_1,
	AUD_INPUT_7_0,
	AUD_INPUT_7_1,
	AUD_INPUT_3_0_LRS,	/* LRS */
	AUD_INPUT_3_1_LRS,	/* LRS */
	AUD_INPUT_4_0_CLRS,	/* C,L,R,S */
	AUD_INPUT_4_1_CLRS,	/* C,L,R,S */
	/* new layout added for DTS */
	AUD_INPUT_6_1_Cs,
	AUD_INPUT_6_1_Ch,
	AUD_INPUT_6_1_Oh,
	AUD_INPUT_6_1_Chr,
	AUD_INPUT_7_1_Lh_Rh,
	AUD_INPUT_7_1_Lsr_Rsr,
	AUD_INPUT_7_1_Lc_Rc,
	AUD_INPUT_7_1_Lw_Rw,
	AUD_INPUT_7_1_Lsd_Rsd,
	AUD_INPUT_7_1_Lss_Rss,
	AUD_INPUT_7_1_Lhs_Rhs,
	AUD_INPUT_7_1_Cs_Ch,
	AUD_INPUT_7_1_Cs_Oh,
	AUD_INPUT_7_1_Cs_Chr,
	AUD_INPUT_7_1_Ch_Oh,
	AUD_INPUT_7_1_Ch_Chr,
	AUD_INPUT_7_1_Oh_Chr,
	AUD_INPUT_7_1_Lss_Rss_Lsr_Rsr,
	AUD_INPUT_6_0_Cs,
	AUD_INPUT_6_0_Ch,
	AUD_INPUT_6_0_Oh,
	AUD_INPUT_6_0_Chr,
	AUD_INPUT_7_0_Lh_Rh,
	AUD_INPUT_7_0_Lsr_Rsr,
	AUD_INPUT_7_0_Lc_Rc,
	AUD_INPUT_7_0_Lw_Rw,
	AUD_INPUT_7_0_Lsd_Rsd,
	AUD_INPUT_7_0_Lss_Rss,
	AUD_INPUT_7_0_Lhs_Rhs,
	AUD_INPUT_7_0_Cs_Ch,
	AUD_INPUT_7_0_Cs_Oh,
	AUD_INPUT_7_0_Cs_Chr,
	AUD_INPUT_7_0_Ch_Oh,
	AUD_INPUT_7_0_Ch_Chr,
	AUD_INPUT_7_0_Oh_Chr,
	AUD_INPUT_7_0_Lss_Rss_Lsr_Rsr,
	AUD_INPUT_8_0_Lh_Rh_Cs,
	AUD_INPUT_UNKNOWN = 0xFF
} AUD_CH_NUM_T;
typedef enum {
	MCLK_128FS,
	MCLK_192FS,
	MCLK_256FS,
	MCLK_384FS,
	MCLK_512FS,
	MCLK_768FS,
	MCLK_1152FS,
} SAMPLE_FREQUENCY_T;


/* /////////////////////////////////////////////////////////// */
typedef struct _AUDIO_DEC_OUTPUT_CHANNEL_T {
	unsigned short FL:1;	/* bit0 */
	unsigned short FR:1;	/* bit1 */
	unsigned short LFE:1;	/* bit2 */
	unsigned short FC:1;	/* bit3 */
	unsigned short RL:1;	/* bit4 */
	unsigned short RR:1;	/* bit5 */
	unsigned short RC:1;	/* bit6 */
	unsigned short FLC:1;	/* bit7 */
	unsigned short FRC:1;	/* bit8 */
	unsigned short RRC:1;	/* bit9 */
	unsigned short RLC:1;	/* bit10 */

} HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T;

/* Static hdr supported by Sink */
typedef enum {
	EDID_SUPPORT_SDR = (1 << 0),
	EDID_SUPPORT_HDR = (1 << 1),
	EDID_SUPPORT_SMPTE_ST_2084 = (1 << 2),
	EDID_SUPPORT_FUTURE_EOTF = (1 << 3),
	EDID_SUPPORT_ET_4 = (1 << 4),
	EDID_SUPPORT_ET_5 = (1 << 5),
} EDID_STATIC_HDR_T;

typedef union _AUDIO_DEC_OUTPUT_CHANNEL_UNION_T {
	HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T bit;	/* HDMI_AUDIO_DEC_OUTPUT_CHANNEL_T */
	unsigned short word;

} AUDIO_DEC_OUTPUT_CHANNEL_UNION_T;

/* Dynamic hdr supported by Sink */
typedef enum {
	EDID_SUPPORT_PHILIPS_HDR = (1 << 0),
	EDID_SUPPORT_DOLBY_HDR = (1 << 1),
	EDID_SUPPORT_YUV422_12BIT = (1 << 2),
	EDID_SUPPORT_DOLBY_HDR_2160P60 = (1 << 3),
	EDID_SUPPORT_HDR10_PLUS = (1 << 4),
} EDID_DYNAMIC_HDR_T;

extern unsigned long bsp_vaddr_to_phys(int x);

/* //////////////////////////////////////////////////////// */
typedef struct _HDMI_AV_INFO_T {
	HDMI_VIDEO_RESOLUTION e_resolution;
	unsigned char fgHdmiOutEnable;
	unsigned char u2VerFreq;
	unsigned char b_hotplug_state;
	HDMI_OUT_COLOR_SPACE_T e_video_color_space;
	HDMI_DEEP_COLOR_T e_deep_color_bit;
	unsigned char ui1_aud_out_ch_number;
	HDMI_AUDIO_SAMPLING_T e_hdmi_fs;
	unsigned char bhdmiRChstatus[6];
	unsigned char bhdmiLChstatus[6];
	unsigned char bMuteHdmiAudio;
	unsigned char u1HdmiI2sMclk;
	unsigned char u1hdcponoff;
	unsigned char u1audiosoft;
	unsigned char fgHdmiTmdsEnable;
	AUDIO_DEC_OUTPUT_CHANNEL_UNION_T ui2_aud_out_ch;

	AUDIO_SAMPLING_T e_dsp_fs;	/* from audio driver see x_aud_dec.h AUD_DEC_SAMPLE_FREQ_T */
	unsigned char bProhibit;
	unsigned char e_hdmi_aud_in;
	unsigned char e_iec_frame;
	unsigned char e_aud_code;
	unsigned char u1Aud_Input_Chan_Cnt;
	unsigned char e_I2sFmt;
} HDMI_AV_INFO_T;


typedef enum {
	AVD_BITS_NONE = 0,
	AVD_LPCM = 1,
	AVD_AC3,
	AVD_MPEG1_AUD,
	AVD_MP3,
	AVD_MPEG2_AUD,
	AVD_AAC,
	AVD_DTS,
	AVD_ATRAC,
	AVD_DSD,
	AVD_DOLBY_PLUS,
	AVD_DTS_HD,
	AVD_MAT_MLP,
	AVD_DST,
	AVD_DOLBY_ATMOS,
	AVD_WMA,
	AVD_CDDA,
	AVD_SACD_PCM,
	AVD_HDCD = 0xfe,
	AVD_BITS_OTHERS = 0xff
} AUDIO_BITSTREAM_TYPE_T;

typedef enum {
	EXTERNAL_EDID = 0,
	INTERNAL_EDID,
	NO_EDID
} GET_EDID_T;

#define RES_480i_60      (1 << 0)
#define RES_576i_50    (1 << 1)
#define RES_480p_60   (1 << 2)
#define RES_576p_50   (1 << 3)
#define RES_720p_60 (1 << 4)
#define RES_720p_50 (1 << 5)
#define RES_1080i_60      (1 << 6)
#define RES_1080i_50 (1 << 7)
#define RES_1080p_30 (1 << 8)
#define RES_1080p_25   (1 << 9)
#define RES_1080p_24      (1 << 10)
#define RES_1080p_23    (1 << 11)
#define RES_1080p_29   (1 << 12)
#define RES_1080p_60   (1 << 13)
#define RES_1080p_50 (1 << 14)
#define RES_2160P_23_976   (1 << 21)
#define RES_2160P_24   (1 << 22)
#define RES_2160P_25       (1 << 23)
#define RES_2160P_29_97   (1 << 24)
#define RES_2160P_30   (1 << 25)
#define RES_2161P_24 (1 << 26)
#define RES_2160P_60   (1 << 27)
#define RES_2160P_50   (1 << 28)
#define RES_2161P_60 (1 << 29)
#define RES_2161P_50       (1<<30)
#define RES_720P_59_94	(1LL << 31)
#define RES_1080P_59_94	(1LL << 32)
#define RES_2160P_59_94	(1LL << 33)
#define RES_2161P_59_94	(1LL << 34)

#define SINK_480P      (1 << 0)
#define SINK_720P60    (1 << 1)
#define SINK_1080I60   (1 << 2)
#define SINK_1080P60   (1 << 3)
#define SINK_480P_1440 (1 << 4)
#define SINK_480P_2880 (1 << 5)
#define SINK_480I      (1 << 6)
#define SINK_480I_1440 (1 << 7)
#define SINK_480I_2880 (1 << 8)
#define SINK_1080P30   (1 << 9)
#define SINK_576P      (1 << 10)
#define SINK_720P50    (1 << 11)
#define SINK_1080I50   (1 << 12)
#define SINK_1080P50   (1 << 13)
#define SINK_576P_1440 (1 << 14)
#define SINK_576P_2880 (1 << 15)
#define SINK_576I      (1 << 16)
#define SINK_576I_1440 (1 << 17)
#define SINK_576I_2880 (1 << 18)
#define SINK_1080P25   (1 << 19)
#define SINK_1080P24   (1 << 20)
#define SINK_1080P23976   (1 << 21)
#define SINK_1080P2997   (1 << 22)
#define SINK_VGA       (1 << 23)	/* 640x480P */
#define SINK_480I_4_3   (1 << 24)	/* 720x480I 4:3 */
#define SINK_480P_4_3   (1 << 25)	/* 720x480P 4:3 */
#define SINK_480P_2880_4_3 (1 << 26)
#define SINK_576I_4_3   (1 << 27)	/* 720x480I 4:3 */
#define SINK_576P_4_3   (1 << 28)	/* 720x480P 4:3 */
#define SINK_576P_2880_4_3 (1 << 29)
#define SINK_720P24       (1<<30)
#define SINK_720P23976 (1<<31)

/*the 2160 mean 3840x2160 */
#define SINK_2160P_23_976HZ (1 << 0)
#define SINK_2160P_24HZ (1 << 1)
#define SINK_2160P_25HZ (1 << 2)
#define SINK_2160P_29_97HZ (1 << 3)
#define SINK_2160P_30HZ (1 << 4)
/*the 2161 mean 4096x2160 */
#define SINK_2161P_24HZ (1 << 5)
#define SINK_2161P_25HZ (1 << 6)
#define SINK_2161P_30HZ (1 << 7)
#define SINK_2160P_50HZ (1 << 8)
#define SINK_2160P_60HZ (1 << 9)
#define SINK_2161P_50HZ (1 << 10)
#define SINK_2161P_60HZ (1 << 11)
#define SINK_2161P_23_976HZ (1 << 12)
#define SINK_2161P_29_97HZ (1 << 13)

/* This HDMI_SINK_VIDEO_COLORIMETRY_T will define what kind of YCBCR */
/* can be supported by sink. */
/* And each bit also defines the colorimetry data block of EDID. */
#define SINK_YCBCR_444 (1<<0)
#define SINK_YCBCR_422 (1<<1)
#define SINK_XV_YCC709 (1<<2)
#define SINK_XV_YCC601 (1<<3)
#define SINK_METADATA0 (1<<4)
#define SINK_METADATA1 (1<<5)
#define SINK_METADATA2 (1<<6)
#define SINK_RGB       (1<<7)
#define SINK_COLOR_SPACE_BT2020_CYCC  (1<<8)
#define SINK_COLOR_SPACE_BT2020_YCC  (1<<9)
#define SINK_COLOR_SPACE_BT2020_RGB  (1<<10)
#define SINK_YCBCR_420 (1<<11)
#define SINK_YCBCR_420_CAPABILITY (1<<12)

#define SINK_METADATA3 (1<<13)
#define SINK_S_YCC601     (1<<14)
#define SINK_ADOBE_YCC601  (1<<15)
#define SINK_ADOBE_RGB  (1<<16)


/* HDMI_SINK_VCDB_T Each bit defines the VIDEO Capability Data Block of EDID. */
#define SINK_CE_ALWAYS_OVERSCANNED                  (1<<0)
#define SINK_CE_ALWAYS_UNDERSCANNED                 (1<<1)
#define SINK_CE_BOTH_OVER_AND_UNDER_SCAN            (1<<2)
#define SINK_IT_ALWAYS_OVERSCANNED                  (1<<3)
#define SINK_IT_ALWAYS_UNDERSCANNED                 (1<<4)
#define SINK_IT_BOTH_OVER_AND_UNDER_SCAN            (1<<5)
#define SINK_PT_ALWAYS_OVERSCANNED                  (1<<6)
#define SINK_PT_ALWAYS_UNDERSCANNED                 (1<<7)
#define SINK_PT_BOTH_OVER_AND_UNDER_SCAN            (1<<8)
#define SINK_RGB_SELECTABLE                         (1<<9)


/* HDMI_SINK_AUDIO_DECODER_T define what kind of audio decoder */
/* can be supported by sink. */
#define   HDMI_SINK_AUDIO_DEC_LPCM        (1<<0)
#define   HDMI_SINK_AUDIO_DEC_AC3         (1<<1)
#define   HDMI_SINK_AUDIO_DEC_MPEG1       (1<<2)
#define   HDMI_SINK_AUDIO_DEC_MP3         (1<<3)
#define   HDMI_SINK_AUDIO_DEC_MPEG2       (1<<4)
#define   HDMI_SINK_AUDIO_DEC_AAC         (1<<5)
#define   HDMI_SINK_AUDIO_DEC_DTS         (1<<6)
#define   HDMI_SINK_AUDIO_DEC_ATRAC       (1<<7)
#define   HDMI_SINK_AUDIO_DEC_DSD         (1<<8)
#define   HDMI_SINK_AUDIO_DEC_DOLBY_PLUS   (1<<9)
#define   HDMI_SINK_AUDIO_DEC_DTS_HD      (1<<10)
#define   HDMI_SINK_AUDIO_DEC_MAT_MLP     (1<<11)
#define   HDMI_SINK_AUDIO_DEC_DST         (1<<12)
#define   HDMI_SINK_AUDIO_DEC_ATMOS       (1<<13)
#define   HDMI_SINK_AUDIO_DEC_WMA         (1<<14)


/* Sink audio channel ability for a fixed Fs */
#define SINK_AUDIO_2CH   (1<<0)
#define SINK_AUDIO_3CH   (1<<1)
#define SINK_AUDIO_4CH   (1<<2)
#define SINK_AUDIO_5CH   (1<<3)
#define SINK_AUDIO_6CH   (1<<4)
#define SINK_AUDIO_7CH   (1<<5)
#define SINK_AUDIO_8CH   (1<<6)

/* Sink supported sampling rate for a fixed channel number */
#define SINK_AUDIO_32k (1<<0)
#define SINK_AUDIO_44k (1<<1)
#define SINK_AUDIO_48k (1<<2)
#define SINK_AUDIO_88k (1<<3)
#define SINK_AUDIO_96k (1<<4)
#define SINK_AUDIO_176k (1<<5)
#define SINK_AUDIO_192k (1<<6)

/* The following definition is for Sink speaker allocation data block . */
#define SINK_AUDIO_FL_FR   (1<<0)
#define SINK_AUDIO_LFE     (1<<1)
#define SINK_AUDIO_FC      (1<<2)
#define SINK_AUDIO_RL_RR   (1<<3)
#define SINK_AUDIO_RC      (1<<4)
#define SINK_AUDIO_FLC_FRC (1<<5)
#define SINK_AUDIO_RLC_RRC (1<<6)

/* The following definition is */
/* For EDID Audio Support, //HDMI_EDID_CHKSUM_AND_AUDIO_SUP_T */
#define SINK_BASIC_AUDIO_NO_SUP    (1<<0)
#define SINK_SAD_NO_EXIST          (1<<1)	/* short audio descriptor */
#define SINK_BASE_BLK_CHKSUM_ERR   (1<<2)
#define SINK_EXT_BLK_CHKSUM_ERR    (1<<3)


/* The following definition is for the output channel of */
/* audio decoder AUDIO_DEC_OUTPUT_CHANNEL_T */
#define AUDIO_DEC_FL   (1<<0)
#define AUDIO_DEC_FR   (1<<1)
#define AUDIO_DEC_LFE  (1<<2)
#define AUDIO_DEC_FC   (1<<3)
#define AUDIO_DEC_RL   (1<<4)
#define AUDIO_DEC_RR   (1<<5)
#define AUDIO_DEC_RC   (1<<6)
#define AUDIO_DEC_FLC  (1<<7)
#define AUDIO_DEC_FRC  (1<<8)


#define HDCPKEY_LENGTH_DRM 512
typedef struct {
	unsigned int u4Addr;
	unsigned int u4Data;
} hdmi_device_write;

typedef struct {
	unsigned int u4Data1;
	unsigned int u4Data2;
} hdmi_para_setting;

typedef struct {
	unsigned char current_hdcp_level;
	unsigned char max_hdcp_level;
	unsigned short ui2bstatus;
	unsigned char brepeater;
} HDCP_INFO;

typedef struct {
	unsigned char u1Hdcpkey[287];
} hdmi_hdcp_key;

typedef struct {
	unsigned char u1Hdcpkey[HDCPKEY_LENGTH_DRM];
} hdmi_hdcp_drmkey;

typedef struct {
	unsigned char u1sendsltdata[15];
} send_slt_data;

#define EDID_LENGTH 256
typedef struct _HDMI_EDID_T {
	unsigned int ui4_ntsc_resolution;	/* use EDID_VIDEO_RES_T, there are many resolution */
	unsigned int ui4_pal_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_native_ntsc_resolution;
	/* use EDID_VIDEO_RES_T, only one NTSC resolution, Zero means none native NTSC resolution is aviable */
	unsigned int ui4_sink_native_pal_resolution;
	/* use EDID_VIDEO_RES_T, only one resolution, Zero means none native PAL resolution is aviable */
	unsigned int ui4_sink_cea_ntsc_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_cea_pal_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_dtd_ntsc_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_dtd_pal_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_1st_dtd_ntsc_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned int ui4_sink_1st_dtd_pal_resolution;	/* use EDID_VIDEO_RES_T */
	unsigned short ui2_sink_colorimetry;	/* use EDID_VIDEO_COLORIMETRY_T */
	unsigned char ui1_sink_rgb_color_bit;	/* color bit for RGB */
	unsigned char ui1_sink_ycbcr_color_bit;	/* color bit for YCbCr */
	unsigned char ui1_sink_dc420_color_bit;
	unsigned short ui2_sink_aud_dec;	/* use EDID_AUDIO_DECODER_T */
	unsigned char ui1_sink_is_plug_in;	/* 1: Plug in 0:Plug Out */
	unsigned int ui4_hdmi_pcm_ch_type;	/* use EDID_A_FMT_CH_TYPE */
	unsigned int ui4_hdmi_pcm_ch3ch4ch5ch7_type;	/* use EDID_A_FMT_CH_TYPE1 */
	unsigned int ui4_hdmi_ac3_ch_type;	/* use AVD_AC3_CH_TYPE */
	unsigned int ui4_hdmi_ac3_ch3ch4ch5ch7_type;	/* use AVD_AC3_CH_TYPE1 */
	unsigned int ui4_hdmi_ec3_ch_type;	/* AVD_DOLBY_PLUS_CH_TYPE */
	unsigned int ui4_hdmi_ec3_ch3ch4ch5ch7_type;	/* AVD_DOLBY_PLUS_CH_TYPE1 */
	unsigned int ui4_hdmi_dts_ch_type;
	unsigned int ui4_hdmi_dts_ch3ch4ch5ch7_type;
	unsigned int ui4_hdmi_dts_hd_ch_type;
	unsigned int ui4_hdmi_dts_hd_ch3ch4ch5ch7_type;
	unsigned int ui4_hdmi_pcm_bit_size;
	unsigned int ui4_hdmi_pcm_ch3ch4ch5ch7_bit_size;
	unsigned int ui4_dac_pcm_ch_type;	/* use EDID_A_FMT_CH_TYPE */
	unsigned char ui1_sink_support_dolby_atoms;
	unsigned char ui1_sink_i_latency_present;
	unsigned int ui1_sink_p_audio_latency;
	unsigned int ui1_sink_p_video_latency;
	unsigned int ui1_sink_i_audio_latency;
	unsigned int ui1_sink_i_video_latency;
	unsigned char ui1ExtEdid_Revision;
	unsigned char ui1Edid_Version;
	unsigned char ui1Edid_Revision;
	unsigned char ui1_Display_Horizontal_Size;
	unsigned char ui1_Display_Vertical_Size;
	unsigned int ui4_ID_Serial_Number;
	unsigned int ui4_sink_cea_3D_resolution;
	unsigned char ui1_sink_support_ai;	/* 0: not support AI, 1:support AI */
	unsigned short ui2_sink_cec_address;
	unsigned short ui1_sink_max_tmds_clock;
	unsigned short ui2_sink_3D_structure;
	unsigned int ui4_sink_cea_FP_SUP_3D_resolution;
	unsigned int ui4_sink_cea_TOB_SUP_3D_resolution;
	unsigned int ui4_sink_cea_SBS_SUP_3D_resolution;
	unsigned short ui2_sink_ID_manufacturer_name;	/* (08H~09H) */
	unsigned short ui2_sink_ID_product_code;	/* (0aH~0bH) */
	unsigned int ui4_sink_ID_serial_number;	/* (0cH~0fH) */
	unsigned char ui1_sink_week_of_manufacture;	/* (10H) */
	unsigned char ui1_sink_year_of_manufacture;	/* (11H)  base on year 1990 */
	unsigned char b_sink_SCDC_present;
	unsigned char b_sink_LTE_340M_sramble;
	unsigned int ui4_sink_hdmi_4k2kvic;
	unsigned short ui2_sink_max_tmds_character_rate;
	unsigned char ui1_sink_support_static_hdr;
	unsigned char ui1_sink_support_dynamic_hdr;
	unsigned char ui1_sink_hdr10plus_app_version;
	unsigned char ui1_sink_hdr_content_max_luminance;
	unsigned char ui1_sink_hdr_content_max_frame_average_luminance;
	unsigned char ui1_sink_hdr_content_min_luminance;
	unsigned int ui4_sink_dolbyvision_vsvdb_length;
	unsigned int ui4_sink_dolbyvision_vsvdb_version;
	unsigned int ui4_sink_dolbyvision_vsvdb_v1_low_latency;
	unsigned int ui4_sink_dolbyvision_vsvdb_v2_interface;
	unsigned int ui4_sink_dolbyvision_vsvdb_low_latency_support;
	unsigned int ui4_sink_dolbyvision_vsvdb_v2_supports_10b_12b_444;
	unsigned int ui4_sink_dolbyvision_vsvdb_support_backlight_control;
	unsigned int ui4_sink_dolbyvision_vsvdb_backlt_min_lumal;
	unsigned int ui4_sink_dolbyvision_vsvdb_tmin;
	unsigned int ui4_sink_dolbyvision_vsvdb_tmax;
	unsigned int ui4_sink_dolbyvision_vsvdb_tminPQ;
	unsigned int ui4_sink_dolbyvision_vsvdb_tmaxPQ;
	unsigned int ui4_sink_dolbyvision_vsvdb_Rx;
	unsigned int ui4_sink_dolbyvision_vsvdb_Ry;
	unsigned int ui4_sink_dolbyvision_vsvdb_Gx;
	unsigned int ui4_sink_dolbyvision_vsvdb_Gy;
	unsigned int ui4_sink_dolbyvision_vsvdb_Bx;
	unsigned int ui4_sink_dolbyvision_vsvdb_By;
	unsigned int ui4_sink_dolbyvision_vsvdb_Wx;
	unsigned int ui4_sink_dolbyvision_vsvdb_Wy;
	unsigned char ui1_sink_dolbyvision_block[32];
	unsigned char ui1rawdata_edid[EDID_LENGTH];
} HDMI_EDID_T;

typedef enum {
	EDID_HF_VSDB_3D_OSD_DISPARITY = (1 << 0),
	EDID_HF_VSDB_DUAL_VIEW = (1 << 1),
	EDID_HF_VSDB_INDEPENDENT_VIEW = (1 << 2),
	EDID_HF_VSDB_LTE_340MCSC_SCRAMBLE = (1 << 3),
	EDID_HF_VSDB_RR_CAPABLE = (1 << 6),
	EDID_HF_VSDB_SCDC_PRESENT = (1 << 7),
} EDID_HF_VSDB_INFO_T;


#define CEC_MAX_DEV_LA_NUM 3
#define CEC_MAX_OPERAND_SIZE       14
#define LOCAL_CEC_MAX_OPERAND_SIZE 14

typedef struct {
	unsigned char ui1_la_num;
	unsigned char e_la[CEC_MAX_DEV_LA_NUM];
	unsigned short ui2_pa;
	unsigned short h_cecm_svc;
} CEC_DRV_ADDR_CFG;

typedef struct {
	unsigned char destination:4;
	unsigned char initiator:4;
} CEC_HEADER_BLOCK_IO;

typedef struct {
	CEC_HEADER_BLOCK_IO header;
	unsigned char opcode;
	unsigned char operand[15];
} CEC_FRAME_BLOCK_IO;

typedef struct {
	unsigned char size;
	unsigned char sendidx;
	unsigned char reTXcnt;
	void *txtag;
	CEC_FRAME_BLOCK_IO blocks;
} CEC_FRAME_DESCRIPTION_IO;

typedef struct _CEC_FRAME_INFO {
	unsigned char ui1_init_addr;
	unsigned char ui1_dest_addr;
	unsigned short ui2_opcode;
	unsigned char aui1_operand[CEC_MAX_OPERAND_SIZE];
	unsigned int z_operand_size;
} CEC_FRAME_INFO;

/* ACK condition */
typedef enum {
	APK_CEC_ACK_COND_OK = 0,
	APK_CEC_ACK_COND_NO_RESPONSE,
} APK_CEC_ACK_COND;

/* ACK info */
typedef struct _APK_CEC_ACK_INFO {
	void *pv_tag;
	APK_CEC_ACK_COND e_ack_cond;
} APK_CEC_ACK_INFO;

typedef struct _CEC_SEND_MSG {
	void *pv_tag;
	CEC_FRAME_INFO t_frame_info;
	unsigned char b_enqueue_ok;
    APK_CEC_ACK_INFO result;
} CEC_SEND_MSG;

typedef struct {
	unsigned char ui1_la;
	unsigned short ui2_pa;
} CEC_ADDRESS_IO;

typedef struct {
	unsigned char u1Size;
	unsigned char au1Data[14];
} CEC_SLT_DATA;

typedef struct {
	unsigned int cmd;
	unsigned int result;
} CEC_USR_CMD_T;


typedef struct {
	unsigned int u1address;
	unsigned int pu1Data;
} READ_REG_VALUE;

typedef struct {
	unsigned char e_hdmi_aud_in;
	unsigned char e_iec_frame;
	unsigned char e_hdmi_fs;
	unsigned char e_aud_code;
	unsigned char u1Aud_Input_Chan_Cnt;
	unsigned char e_I2sFmt;
	unsigned char u1HdmiI2sMclk;
	unsigned char bhdmi_LCh_status[5];
	unsigned char bhdmi_RCh_status[5];
} HDMITX_AUDIO_PARA;

enum HDMI_CAPABILITY {
	HDMI_SCALE_ADJUSTMENT_SUPPORT = 0x01,
	HDMI_ONE_RDMA_LIMITATION = 0x02,
	HDMI_PHONE_GPIO_REUSAGE  = 0x04,
	HDMI_FACTORY_MODE_NEW    = 0x1000,
	HDMI_FACTORY_TEST_BOX	  = 0x2000,
	HDMI_FACTORY_TEST_HDCP	 = 0x4000,
};

#define HDMI_IOW(num, dtype)     _IOW('H', num, dtype)
#define HDMI_IOR(num, dtype)     _IOR('H', num, dtype)
#define HDMI_IOWR(num, dtype)    _IOWR('H', num, dtype)
#define HDMI_IO(num)             _IO('H', num)

#define MTK_HDMI_AUDIO_VIDEO_ENABLE             HDMI_IO(1)
#define MTK_HDMI_AUDIO_ENABLE                   HDMI_IO(2)
#define MTK_HDMI_VIDEO_ENABLE                   HDMI_IO(3)
#define MTK_HDMI_VIDEO_CONFIG                   HDMI_IOWR(4, int)
#define MTK_HDMI_POWER_ENABLE                   HDMI_IOW(5, int)
#define MTK_HDMI_AUDIO_SETTING                  HDMI_IOWR(6, HDMITX_AUDIO_PARA)
#define MTK_HDMI_FACTORY_MODE_ENABLE            HDMI_IOW(7, int)
#define MTK_HDMI_FACTORY_GET_STATUS             HDMI_IOWR(8, int)
#define MTK_HDMI_WRITE_DEV                      HDMI_IOWR(9, hdmi_device_write)
#define MTK_HDMI_READ_DEV                       HDMI_IOWR(10, unsigned int)
#define MTK_HDMI_ENABLE_LOG                     HDMI_IOWR(11, unsigned int)
#define MTK_HDMI_CHECK_EDID                     HDMI_IOWR(12, unsigned int)
#define MTK_HDMI_INFOFRAME_SETTING              HDMI_IOWR(13, hdmi_para_setting)
#define MTK_HDMI_COLOR_DEEP                     HDMI_IOWR(14, hdmi_para_setting)
#define MTK_HDMI_ENABLE_HDCP                    HDMI_IOWR(15, unsigned int)
#define MTK_HDMI_STATUS                         HDMI_IOWR(16, unsigned int)
#define MTK_HDMI_HDCP_KEY                       HDMI_IOWR(17, hdmi_hdcp_key)
#define MTK_HDMI_GET_EDID                       HDMI_IOWR(18, HDMI_EDID_T)
#define MTK_HDMI_SETLA                          HDMI_IOWR(19, CEC_DRV_ADDR_CFG)
#define MTK_HDMI_GET_CECCMD                     HDMI_IOWR(20, CEC_FRAME_DESCRIPTION_IO)
#define MTK_HDMI_SET_CECCMD                     HDMI_IOWR(21, CEC_SEND_MSG)
#define MTK_HDMI_CEC_ENABLE                     HDMI_IOWR(22, unsigned int)
#define MTK_HDMI_GET_CECADDR                    HDMI_IOWR(23, CEC_ADDRESS_IO)
#define MTK_HDMI_CECRX_MODE                     HDMI_IOWR(24, unsigned int)
#define MTK_HDMI_SENDSLTDATA                    HDMI_IOWR(25, send_slt_data)
#define MTK_HDMI_GET_SLTDATA                    HDMI_IOWR(27, CEC_SLT_DATA)
#define MTK_HDMI_VIDEO_MUTE                     HDMI_IOWR(28, int)
#define MTK_HDMI_GET_CECSTS                     HDMI_IOWR(29, APK_CEC_ACK_INFO)
#define MTK_HDMI_CEC_USR_CMD                    HDMI_IOWR(30, CEC_USR_CMD_T)
#define MTK_HDMI_HDCP_INFO                      HDMI_IOWR(31, HDCP_INFO)
#define MTK_HDMI_HDR_ENABLE                     HDMI_IOWR(32, HDMI_FORCE_HDR_ENABLE)
#define MTK_HDMI_GET_CAPABILITY                 HDMI_IOWR(33, int)
#define MTK_HDMI_FACTORY_CHIP_INIT              HDMI_IOWR(34, int)
#define MTK_HDMI_AUDIO_CONFIG                   HDMI_IOWR(35, int)
#define MTK_HDMI_FACTORY_JUDGE_CALLBACK         HDMI_IOWR(36, int)
#define MTK_HDMI_FACTORY_START_DPI_AND_CONFIG   HDMI_IOWR(37, int)
#define MTK_HDMI_FACTORY_DPI_TEST               HDMI_IOWR(38, int)
#define MTK_HDMI_CEC_OPTION_SYSTEM_CONTROL      HDMI_IOWR(39, int)
#define MTK_HDMI_HDCP_AUTH_STATUS               HDMI_IOWR(40, int)
#define MTK_HDMI_HPD_ONOFF                      HDMI_IOWR(41, int)


#define fgIsHDRes(u1Res) ((u1Res == HDMI_VIDEO_1280x720p_60Hz) || (u1Res == HDMI_VIDEO_1280x720p_50Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080i_60Hz) || (u1Res == HDMI_VIDEO_1920x1080i_50Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080p_30Hz) || (u1Res == HDMI_VIDEO_1920x1080p_25Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080p_24Hz) || (u1Res == HDMI_VIDEO_1920x1080p_23Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080p_29Hz) || (u1Res == HDMI_VIDEO_1920x1080p_60Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080p_50Hz) || (u1Res == HDMI_VIDEO_1280x720p3d_60Hz) ||\
	(u1Res == HDMI_VIDEO_1280x720p3d_50Hz) || (u1Res == HDMI_VIDEO_1920x1080i3d_60Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080i3d_50Hz) || (u1Res == HDMI_VIDEO_1920x1080p3d_24Hz) || \
	(u1Res == HDMI_VIDEO_1920x1080p3d_23Hz) || (u1Res == HDMI_VIDEO_3840x2160P_23_976HZ) || \
	(u1Res == HDMI_VIDEO_3840x2160P_24HZ) || (u1Res == HDMI_VIDEO_3840x2160P_25HZ) ||\
	(u1Res == HDMI_VIDEO_3840x2160P_29_97HZ) || (u1Res == HDMI_VIDEO_3840x2160P_30HZ) || \
	(u1Res == HDMI_VIDEO_4096x2160P_24HZ) || (u1Res == HDMI_VIDEO_3840x2160P_60HZ) || \
	(u1Res == HDMI_VIDEO_3840x2160P_50HZ) || (u1Res == HDMI_VIDEO_4096x2160P_60HZ) ||\
	(u1Res == HDMI_VIDEO_4096x2160P_50HZ) || (u1Res == HDMI_VIDEO_1280x720p_59_94Hz) ||\
	(u1Res == HDMI_VIDEO_1920x1080p_59_94Hz) || (u1Res == HDMI_VIDEO_3840x2160P_59_94HZ) ||\
	(u1Res == HDMI_VIDEO_4096x2160P_59_94HZ))

#define  fgVideoIsNtsc(ucFmt)  ((ucFmt == HDMI_VIDEO_720x480i_60Hz) || (ucFmt == HDMI_VIDEO_720x480p_60Hz) ||\
	(ucFmt == HDMI_VIDEO_1280x720p_60Hz) || (ucFmt == HDMI_VIDEO_1920x1080i_60Hz) || \
	(ucFmt == HDMI_VIDEO_1920x1080p_30Hz) || (ucFmt == HDMI_VIDEO_1920x1080p_24Hz) || \
	(ucFmt == HDMI_VIDEO_1920x1080p_23Hz) || (ucFmt == HDMI_VIDEO_1920x1080p_29Hz) ||\
	(ucFmt == HDMI_VIDEO_1920x1080p_60Hz) || /*(ucFmt == RES_1080P30HZ)||*/\
	(ucFmt == HDMI_VIDEO_1280x720p3d_60Hz) || (ucFmt == HDMI_VIDEO_1920x1080i3d_60Hz) || \
	(ucFmt == HDMI_VIDEO_1920x1080p3d_24Hz) || (ucFmt == HDMI_VIDEO_1920x1080p3d_23Hz) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_23_976HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_24HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_29_97HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_30HZ) || \
	(ucFmt == HDMI_VIDEO_4096x2160P_24HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_60HZ) || \
	(ucFmt == HDMI_VIDEO_4096x2160P_60HZ) || (ucFmt == HDMI_VIDEO_1280x720p_59_94Hz) ||\
	(ucFmt == HDMI_VIDEO_1920x1080p_59_94Hz) || (ucFmt == HDMI_VIDEO_3840x2160P_59_94HZ) ||\
	(ucFmt == HDMI_VIDEO_4096x2160P_59_94HZ))

#define fgFMTis4k2k(ucFmt) ((ucFmt == HDMI_VIDEO_3840x2160P_23_976HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_24HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_25HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_29_97HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_30HZ) || (ucFmt == HDMI_VIDEO_4096x2160P_24HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_60HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_50HZ) || \
	(ucFmt == HDMI_VIDEO_4096x2160P_60HZ) || (ucFmt == HDMI_VIDEO_4096x2160P_50HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_59_94HZ) || (ucFmt == HDMI_VIDEO_4096x2160P_59_94HZ))

#define fgFMTis4k2k_6G(ucFmt) ((ucFmt == HDMI_VIDEO_3840x2160P_60HZ) || (ucFmt == HDMI_VIDEO_3840x2160P_50HZ) || \
	(ucFmt == HDMI_VIDEO_4096x2160P_60HZ) || (ucFmt == HDMI_VIDEO_4096x2160P_50HZ) || \
	(ucFmt == HDMI_VIDEO_3840x2160P_59_94HZ) || (ucFmt == HDMI_VIDEO_4096x2160P_59_94HZ))

#if IS_ENABLED(CONFIG_COMPAT)
#include <linux/compat.h>
#include <linux/uaccess.h>

struct COMPAT_HDMI_EDID_T {
	compat_uint_t ui4_ntsc_resolution;
	compat_uint_t ui4_pal_resolution;
	compat_uint_t ui4_sink_native_ntsc_resolution;
	compat_uint_t ui4_sink_native_pal_resolution;
	compat_uint_t ui4_sink_cea_ntsc_resolution;
	compat_uint_t ui4_sink_cea_pal_resolution;
	compat_uint_t ui4_sink_dtd_ntsc_resolution;
	compat_uint_t ui4_sink_dtd_pal_resolution;
	compat_uint_t ui4_sink_1st_dtd_ntsc_resolution;
	compat_uint_t ui4_sink_1st_dtd_pal_resolution;
	unsigned short ui2_sink_colorimetry;
	unsigned char ui1_sink_rgb_color_bit;
	unsigned char ui1_sink_ycbcr_color_bit;
	unsigned char ui1_sink_dc420_color_bit;
	unsigned short ui2_sink_aud_dec;
	unsigned char ui1_sink_is_plug_in;
	compat_uint_t ui4_hdmi_pcm_ch_type;
	compat_uint_t ui4_hdmi_pcm_ch3ch4ch5ch7_type;
	compat_uint_t ui4_hdmi_ac3_ch_type;	/* use AVD_AC3_CH_TYPE */
	compat_uint_t ui4_hdmi_ac3_ch3ch4ch5ch7_type;	/* use AVD_AC3_CH_TYPE1 */
	compat_uint_t ui4_hdmi_ec3_ch_type;	/* AVD_DOLBY_PLUS_CH_TYPE */
	compat_uint_t ui4_hdmi_ec3_ch3ch4ch5ch7_type;	/* AVD_DOLBY_PLUS_CH_TYPE1 */
	compat_uint_t ui4_hdmi_dts_ch_type;
	compat_uint_t ui4_hdmi_dts_ch3ch4ch5ch7_type;
	compat_uint_t ui4_hdmi_dts_hd_ch_type;
	compat_uint_t ui4_hdmi_dts_hd_ch3ch4ch5ch7_type;
	compat_uint_t ui4_hdmi_pcm_bit_size;
	compat_uint_t ui4_hdmi_pcm_ch3ch4ch5ch7_bit_size;
	compat_uint_t ui4_dac_pcm_ch_type;
	unsigned char ui1_sink_support_dolby_atoms;
	unsigned char ui1_sink_i_latency_present;
	compat_uint_t ui1_sink_p_audio_latency;
	compat_uint_t ui1_sink_p_video_latency;
	compat_uint_t ui1_sink_i_audio_latency;
	compat_uint_t ui1_sink_i_video_latency;
	unsigned char ui1ExtEdid_Revision;
	unsigned char ui1Edid_Version;
	unsigned char ui1Edid_Revision;
	unsigned char ui1_Display_Horizontal_Size;
	unsigned char ui1_Display_Vertical_Size;
	compat_uint_t ui4_ID_Serial_Number;
	compat_uint_t ui4_sink_cea_3D_resolution;
	unsigned char ui1_sink_support_ai;
	unsigned short ui2_sink_cec_address;
	unsigned short ui1_sink_max_tmds_clock;
	unsigned short ui2_sink_3D_structure;
	compat_uint_t ui4_sink_cea_FP_SUP_3D_resolution;
	compat_uint_t ui4_sink_cea_TOB_SUP_3D_resolution;
	compat_uint_t ui4_sink_cea_SBS_SUP_3D_resolution;
	unsigned short ui2_sink_ID_manufacturer_name;
	unsigned short ui2_sink_ID_product_code;
	compat_uint_t ui4_sink_ID_serial_number;
	unsigned char ui1_sink_week_of_manufacture;
	unsigned char ui1_sink_year_of_manufacture;
	unsigned char b_sink_SCDC_present;
	unsigned char b_sink_LTE_340M_sramble;
	compat_uint_t ui4_sink_hdmi_4k2kvic;
	unsigned char ui1_sink_support_static_hdr;
	unsigned char ui1_sink_support_dynamic_hdr;
	unsigned char ui1_sink_hdr10plus_app_version;
	unsigned char ui1_sink_dolbyvision_block[32];
	unsigned char ui1rawdata_edid[EDID_LENGTH];
};

struct COMPAT_HDMITX_AUDIO_PARA {
	unsigned char e_hdmi_aud_in;
	unsigned char e_iec_frame;
	unsigned char e_hdmi_fs;
	unsigned char e_aud_code;
	unsigned char u1Aud_Input_Chan_Cnt;
	unsigned char e_I2sFmt;
	unsigned char u1HdmiI2sMclk;
	unsigned char bhdmi_LCh_status[5];
	unsigned char bhdmi_RCh_status[5];
};

typedef struct _COMPAT_CEC_SEND_MSG {
	compat_uptr_t pv_tag;
	CEC_FRAME_INFO t_frame_info;
	unsigned char b_enqueue_ok;
} COMPAT_CEC_SEND_MSG;

typedef struct {
	unsigned char destination:4;
	unsigned char initiator:4;
} COMPAT_CEC_HEADER_BLOCK_IO;

typedef struct {
	COMPAT_CEC_HEADER_BLOCK_IO header;
	unsigned char opcode;
	unsigned char operand[15];
} COMPAT_CEC_FRAME_BLOCK_IO;

typedef struct {
	unsigned char size;
	unsigned char sendidx;
	unsigned char reTXcnt;
	compat_uptr_t txtag;
	COMPAT_CEC_FRAME_BLOCK_IO blocks;
} COMPAT_CEC_FRAME_DESCRIPTION_IO;


/* ACK info */
typedef struct _COMPAT_APK_CEC_ACK_INFO {
	compat_uptr_t pv_tag;
	compat_uint_t e_ack_cond;
} COMPAT_APK_CEC_ACK_INFO;

typedef struct {
	unsigned char u1Hdcpkey[HDCPKEY_LENGTH_DRM];
} compat_hdmi_hdcp_drmkey;

typedef struct {
	unsigned char u1Hdcpkey[287];
} compat_hdmi_hdcp_key;

typedef struct {
	compat_uint_t u4Data1;
	compat_uint_t u4Data2;
} compat_hdmi_para_setting;

typedef enum {
	COMPAT_HDMI_FORCE_DEFAULT,
	COMPAT_HDMI_FORCE_SDR,
	COMPAT_HDMI_FORCE_HDR,
	COMPAT_HDMI_FORCE_HDR10,
	COMPAT_HDMI_FORCE_DOLBY_VISION,
	COMPAT_HDMI_FORCE_HLG,
	COMPAT_HDMI_FORCE_HDR10_PLUS,
} compat_hdmi_force_hdr_enable;


#define COMPAT_MTK_HDMI_AUDIO_VIDEO_ENABLE             HDMI_IO(1)
#define COMPAT_MTK_HDMI_AUDIO_ENABLE                   HDMI_IO(2)
#define COMPAT_MTK_HDMI_VIDEO_ENABLE                   HDMI_IO(3)
#define COMPAT_MTK_HDMI_VIDEO_CONFIG                   HDMI_IOWR(4, int)
#define COMPAT_MTK_HDMI_POWER_ENABLE                   HDMI_IOW(5, int)
#define COMPAT_MTK_HDMI_AUDIO_SETTING                  HDMI_IOWR(6, HDMITX_AUDIO_PARA)
#define COMPAT_MTK_HDMI_FACTORY_MODE_ENABLE            HDMI_IOW(7, int)
#define COMPAT_MTK_HDMI_FACTORY_GET_STATUS             HDMI_IOWR(8, int)
#define COMPAT_MTK_HDMI_WRITE_DEV                      HDMI_IOWR(9, hdmi_device_write)
#define COMPAT_MTK_HDMI_READ_DEV                       HDMI_IOWR(10, unsigned int)
#define COMPAT_MTK_HDMI_ENABLE_LOG                     HDMI_IOWR(11, unsigned int)
#define COMPAT_MTK_HDMI_CHECK_EDID                     HDMI_IOWR(12, unsigned int)
#define COMPAT_MTK_HDMI_INFOFRAME_SETTING              HDMI_IOWR(13, hdmi_para_setting)
#define COMPAT_MTK_HDMI_COLOR_DEEP                     HDMI_IOWR(14, hdmi_para_setting)
#define COMPAT_MTK_HDMI_ENABLE_HDCP                    HDMI_IOWR(15, unsigned int)
#define COMPAT_MTK_HDMI_STATUS                         HDMI_IOWR(16, unsigned int)
#define COMPAT_MTK_HDMI_HDCP_KEY                       HDMI_IOWR(17, hdmi_hdcp_key)
#define COMPAT_MTK_HDMI_GET_EDID                       HDMI_IOWR(18, HDMI_EDID_T)
#define COMPAT_MTK_HDMI_SETLA                          HDMI_IOWR(19, CEC_DRV_ADDR_CFG)
#define COMPAT_MTK_HDMI_GET_CECCMD                     HDMI_IOWR(20, COMPAT_CEC_FRAME_DESCRIPTION_IO)
#define COMPAT_MTK_HDMI_SET_CECCMD                     HDMI_IOWR(21, COMPAT_CEC_SEND_MSG)
#define COMPAT_MTK_HDMI_CEC_ENABLE                     HDMI_IOWR(22, unsigned int)
#define COMPAT_MTK_HDMI_GET_CECADDR                    HDMI_IOWR(23, CEC_ADDRESS_IO)
#define COMPAT_MTK_HDMI_CECRX_MODE                     HDMI_IOWR(24, unsigned int)
#define COMPAT_MTK_HDMI_SENDSLTDATA                    HDMI_IOWR(25, send_slt_data)
#define COMPAT_MTK_HDMI_GET_SLTDATA                    HDMI_IOWR(27, CEC_SLT_DATA)
#define COMPAT_MTK_HDMI_VIDEO_MUTE                     HDMI_IOWR(28, int)
#define COMPAT_MTK_HDMI_GET_CECSTS                     HDMI_IOWR(29, COMPAT_APK_CEC_ACK_INFO)
#define COMPAT_MTK_HDMI_CEC_USR_CMD                    HDMI_IOWR(30, CEC_USR_CMD_T)
#define COMPAT_MTK_HDMI_HDR_ENABLE                     HDMI_IOWR(32, HDMI_FORCE_HDR_ENABLE)

#endif

#endif
