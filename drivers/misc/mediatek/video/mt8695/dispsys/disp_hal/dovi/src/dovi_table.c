/*
 * Copyright (C) 2016 MediaTek Inc.
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

#include "dovi_type.h"
#include "dovi_log.h"
#include "dovi_table.h"
#include "dovi_common_hal.h"

/*
* htotal, vtotal, width , height,  freq, progressive,  hd,  resolution
*/
struct dobly_resolution dobly_resolution_table[] = {
	{858, 525, 720, 480, 60, false, false, HDMI_VIDEO_720x480i_60Hz},
	{864, 625, 720, 576, 50, false, false, HDMI_VIDEO_720x576i_50Hz},
	{858, 525, 720, 480, 60, true, false, HDMI_VIDEO_720x480p_60Hz},
	{864, 625, 720, 576, 50, true, false, HDMI_VIDEO_720x576p_50Hz},
	{1650, 750, 1280, 720, 60, true, true, HDMI_VIDEO_1280x720p_60Hz},
	{1980, 750, 1280, 720, 50, true, true, HDMI_VIDEO_1280x720p_50Hz},
	{2200, 1125, 1920, 1080, 60, false, true, HDMI_VIDEO_1920x1080i_60Hz},
	{2640, 1125, 1920, 1080, 50, false, true, HDMI_VIDEO_1920x1080i_50Hz},
	{2200, 1125, 1920, 1080, 30, true, true, HDMI_VIDEO_1920x1080p_30Hz},
	{2640, 1125, 1920, 1080, 25, true, true, HDMI_VIDEO_1920x1080p_25Hz},
	{2750, 1125, 1920, 1080, 24, true, true, HDMI_VIDEO_1920x1080p_24Hz},
	{2750, 1125, 1920, 1080, 23, true, true, HDMI_VIDEO_1920x1080p_23Hz},
	{2200, 1125, 1920, 1080, 29, true, true, HDMI_VIDEO_1920x1080p_29Hz},
	{2200, 1125, 1920, 1080, 60, true, true, HDMI_VIDEO_1920x1080p_60Hz},
	{2640, 1125, 1920, 1080, 50, true, true, HDMI_VIDEO_1920x1080p_50Hz},
	{1650, 750, 1280, 720, 60, true, true, HDMI_VIDEO_1280x720p3d_60Hz},
	{1980, 750, 1280, 720, 50, true, true, HDMI_VIDEO_1280x720p3d_50Hz},
	{2200, 1125, 1920, 1080, 60, false, true, HDMI_VIDEO_1920x1080i3d_60Hz},
	{2640, 1125, 1920, 1080, 50, false, true, HDMI_VIDEO_1920x1080i3d_50Hz},
	{2750, 1125, 1920, 1080, 24, true, true, HDMI_VIDEO_1920x1080p3d_24Hz},
	{2750, 1125, 1920, 1080, 23, true, true, HDMI_VIDEO_1920x1080p3d_23Hz},
	{5500, 2250, 3840, 2160, 23, true, true, HDMI_VIDEO_3840x2160P_23_976HZ},
	{5500, 2250, 3840, 2160, 24, true, true, HDMI_VIDEO_3840x2160P_24HZ},
	{5280, 2250, 3840, 2160, 25, true, true, HDMI_VIDEO_3840x2160P_25HZ},
	{4400, 2250, 3840, 2160, 29, true, true, HDMI_VIDEO_3840x2160P_29_97HZ},
	{4400, 2250, 3840, 2160, 30, true, true, HDMI_VIDEO_3840x2160P_30HZ},
	{5500, 2250, 4096, 2160, 24, true, true, HDMI_VIDEO_4096x2160P_24HZ},
	{4400, 2250, 3840, 2160, 60, true, true, HDMI_VIDEO_3840x2160P_60HZ},
	{5280, 2250, 3840, 2160, 50, true, true, HDMI_VIDEO_3840x2160P_50HZ},
	{4400, 2250, 4096, 2160, 60, true, true, HDMI_VIDEO_4096x2160P_60HZ},
	{5280, 2250, 4096, 2160, 50, true, true, HDMI_VIDEO_4096x2160P_50HZ},
	{1650, 750, 1280, 720, 60, true, true, HDMI_VIDEO_1280x720p_59_94Hz},
	{2200, 1125, 1920, 1080, 60, true, true, HDMI_VIDEO_1920x1080p_59_94Hz},
	{4400, 2250, 3840, 2160, 60, true, true, HDMI_VIDEO_3840x2160P_59_94HZ},
	{4400, 2250, 4096, 2160, 60, true, true, HDMI_VIDEO_4096x2160P_59_94HZ},
};

const char *dobly_resstr[HDMI_VIDEO_RESOLUTION_NUM] = {
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
};

unsigned int core3_select_external_timing;

struct core3_timing_info_t core3_timing_info[CORE3_MAX_TIMING] = {
	{720, 576, 0x40, 0x2d},
	{720, 480, 0x36, 0x2b},
	{1280, 720, 0xc5, 0x1a},
	{1920, 1080, 0x81, 0x2a},
	{3840, 2160, 0x141, 0x53},
	{4096, 2160, 0x99, 0x53},
};

struct dovi_timing_info core2_timing_info[HDMI_VIDEO_RESOLUTION_NUM] = {
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 0 HDMI_VIDEO_720x480i_60Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 1 HDMI_VIDEO_720x576i_50Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 2 HDMI_VIDEO_720x480p_60Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 3 HDMI_VIDEO_720x576p_50Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 4 HDMI_VIDEO_1280x720p_60Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 5 HDMI_VIDEO_1280x720p_50Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 6 */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 7 */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 8 */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 9 */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 10 */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 11 HDMI_VIDEO_1920x1080p_23Hz */
	{0x10001, 0x34A007B, 0x20A002B, 0x10001}, /* 12 HDMI_VIDEO_1920x1080p_60Hz */
	{0x10001, 0x84100C1, 0x461002A, 0x10001}, /* 13 HDMI_VIDEO_1920x1080p_60Hz */
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001}, /* 15 */
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001}, /* 20 */
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001}, /* 25 */
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001},
	{0x10001, 0x84100C1, 0x461002A, 0x10001}, /* 30 */
};

struct dovi_out_format_table_t dovi_out_format[MAX_DOVI_TEST_CASE_ID] = {
	{1, OUT_FORMAT_DOVI},
	{2, OUT_FORMAT_DOVI},
	{3, OUT_FORMAT_DOVI},
	{4, OUT_FORMAT_DOVI},
	{5, OUT_FORMAT_DOVI},
	{6, OUT_FORMAT_DOVI},
	{7, OUT_FORMAT_DOVI},
	{8, OUT_FORMAT_DOVI},

	{11, OUT_FORMAT_DOVI},
	{12, OUT_FORMAT_DOVI},
	{13, OUT_FORMAT_DOVI},
	{14, OUT_FORMAT_DOVI},
	{15, OUT_FORMAT_DOVI},
	{16, OUT_FORMAT_DOVI},

	{5000, OUT_FORMAT_SDR},
	{5001, OUT_FORMAT_SDR},
	{5002, OUT_FORMAT_SDR},
	{5003, OUT_FORMAT_SDR},
	{5004, OUT_FORMAT_SDR},
	{5005, OUT_FORMAT_SDR},
	{5006, OUT_FORMAT_SDR},
	{5007, OUT_FORMAT_SDR},
	{5008, OUT_FORMAT_SDR},
	{5009, OUT_FORMAT_SDR},

	{5020, OUT_FORMAT_SDR},
	{5021, OUT_FORMAT_SDR},
	{5022, OUT_FORMAT_SDR},
	{5023, OUT_FORMAT_SDR},
	{5024, OUT_FORMAT_SDR},

	{5100, OUT_FORMAT_SDR},
	{5101, OUT_FORMAT_SDR},
	{5102, OUT_FORMAT_SDR},

	{5103, OUT_FORMAT_SDR},
	{5104, OUT_FORMAT_SDR},
	{5105, OUT_FORMAT_SDR},
	{5106, OUT_FORMAT_SDR},

	{5107, OUT_FORMAT_SDR},

	{5025, OUT_FORMAT_SDR},
	{5026, OUT_FORMAT_SDR},
	{5027, OUT_FORMAT_SDR},
	{5028, OUT_FORMAT_DOVI},

	{5030, OUT_FORMAT_SDR},
	{5031, OUT_FORMAT_SDR},
	{5032, OUT_FORMAT_SDR},
	{5033, OUT_FORMAT_SDR},
	{5034, OUT_FORMAT_SDR},
	{5035, OUT_FORMAT_SDR},
	{5036, OUT_FORMAT_SDR},

	{5040, OUT_FORMAT_DOVI},
	{5041, OUT_FORMAT_DOVI},
	{5042, OUT_FORMAT_DOVI},
	{5043, OUT_FORMAT_DOVI},
	{5044, OUT_FORMAT_DOVI},
	{5045, OUT_FORMAT_DOVI},
	{5046, OUT_FORMAT_DOVI},
	{5047, OUT_FORMAT_DOVI},
	{5048, OUT_FORMAT_DOVI},
	{5049, OUT_FORMAT_DOVI},

	{5200, OUT_FORMAT_SDR},
	{5201, OUT_FORMAT_DOVI},
	{5203, OUT_FORMAT_DOVI},
	{5204, OUT_FORMAT_SDR},
	{5205, OUT_FORMAT_HDR10},
	{5207, OUT_FORMAT_HDR10},

	{5210, OUT_FORMAT_SDR},
	{5211, OUT_FORMAT_DOVI},
	{5212, OUT_FORMAT_SDR},
	{5213, OUT_FORMAT_DOVI},
	{5214, OUT_FORMAT_SDR},
	{5215, OUT_FORMAT_HDR10},
	{5217, OUT_FORMAT_HDR10},

	{5220, OUT_FORMAT_SDR},
	{5221, OUT_FORMAT_DOVI},
	{5223, OUT_FORMAT_DOVI},
	{5224, OUT_FORMAT_SDR},
	{5225, OUT_FORMAT_HDR10},
	{5227, OUT_FORMAT_HDR10},

	{5230, OUT_FORMAT_SDR},
	{5231, OUT_FORMAT_DOVI},
	{5233, OUT_FORMAT_DOVI},
	{5234, OUT_FORMAT_SDR},
	{5235, OUT_FORMAT_HDR10},
	{5237, OUT_FORMAT_HDR10},

	{5240, OUT_FORMAT_SDR},
	{5241, OUT_FORMAT_DOVI},
	{5243, OUT_FORMAT_DOVI},
	{5244, OUT_FORMAT_SDR},
	{5245, OUT_FORMAT_HDR10},
	{5247, OUT_FORMAT_HDR10},

	{5250, OUT_FORMAT_SDR},
	{5251, OUT_FORMAT_DOVI},
	{5253, OUT_FORMAT_DOVI},
	{5254, OUT_FORMAT_SDR},
	{5255, OUT_FORMAT_HDR10},
	{5257, OUT_FORMAT_HDR10},

	{5260, OUT_FORMAT_SDR},
	{5261, OUT_FORMAT_DOVI},
	{5263, OUT_FORMAT_DOVI},
	{5264, OUT_FORMAT_SDR},
	{5265, OUT_FORMAT_HDR10},
	{5267, OUT_FORMAT_HDR10},
	{5270, OUT_FORMAT_SDR},
	{5271, OUT_FORMAT_DOVI},
	{5273, OUT_FORMAT_DOVI},
	{5274, OUT_FORMAT_SDR},
	{5275, OUT_FORMAT_HDR10},
	{5277, OUT_FORMAT_HDR10},

};

struct dovi_case_info_t dovi_case_info[MAX_DOVI_TEST_CASE_ID] = {
	{1, "vsvdb_v0.bin"},
	{2, "vsvdb_v1-15b.bin"},
	{3, "vsvdb_v1-12b-ll.bin"},
	{4, "vsvdb_v2-if2.bin"},
	{5, "vsvdb_v1-12b-ll.bin", 1},
	{6, "vsvdb_v1-12b-ll.bin", 1},
	{7, "vsvdb_v2-if2.bin", 1},
	{8, "vsvdb_v2-if2.bin", 1},

	{11, "vsvdb_v0.bin"},
	{12, "vsvdb_v1-15b.bin"},
	{13, "vsvdb_v1-12b-ll.bin"},
	{14, "vsvdb_v2-if2.bin"},
	{15, "vsvdb_v1-12b-ll.bin", 1},
	{16, "vsvdb_v2-if2.bin", 1},

	{5000, ""},
	{5001, ""},
	{5002, ""},
	{5003, ""},
	{5004, ""},
	{5005, ""},
	{5006, ""},
	{5007, ""},
	{5008, ""},
	{5009, ""},

	{5020, ""},
	{5021, ""},
	{5022, ""},
	{5023, ""},
	{5024, ""},
	{5025, "vsvdb_v1-12b-ll.bin"},
	{5026, ""},
	{5027, ""},
	{5028, "vsvdb_v1-12b-ll.bin"},

	{5040, "vsvdb_v1-12b-ll.bin"},
	{5041, "vsvdb_v1-12b-ll.bin", 1},
	{5042, "vsvdb_v2-if0.bin"},
	{5043, "vsvdb_v2-if1.bin"},
	{5044, "vsvdb_v2-if1.bin", 0, 1},
	{5045, "vsvdb_v2-if2.bin"},
	{5046, "vsvdb_v2-if2.bin", 1},
	{5047, "vsvdb_v2-if3.bin"},
	{5048, "vsvdb_v2-if3.bin", 1},
	{5049, "vsvdb_v2-if3.bin", 1, 1},

	{5100, ""},
	{5102, ""},

	{5103, ""},
	{5104, ""},
	{5105, ""},
	{5106, ""},

	{5107, ""},

	{5200, ""},
	{5201, "vsvdb_v1-12b-ll.bin"},
	{5203, "vsvdb_v1-12b-ll.bin"},
	{5204, ""},
	{5205, ""},
	{5207, ""},

	{5210, ""},
	{5211, "vsvdb_v1-12b-ll.bin"},
	{5212, ""},
	{5213, "vsvdb_v1-12b-ll.bin"},
	{5214, ""},
	{5215, ""},
	{5217, ""},

	{5220, ""},
	{5221, "vsvdb_v1-12b-ll.bin"},
	{5223, "vsvdb_v1-12b-ll.bin"},
	{5224, ""},
	{5225, ""},
	{5227, ""},

	{5230, ""},
	{5231, "vsvdb_v1-12b-ll.bin"},
	{5233, "vsvdb_v1-12b-ll.bin"},
	{5234, ""},
	{5235, ""},
	{5237, ""},

	{5240, ""},
	{5241, "vsvdb_v1-12b-ll.bin"},
	{5243, "vsvdb_v1-12b-ll.bin"},
	{5244, ""},
	{5245, ""},
	{5247, ""},

	{5250, ""},
	{5251, "vsvdb_v1-12b-ll.bin"},
	{5253, "vsvdb_v1-12b-ll.bin"},
	{5254, ""},
	{5255, ""},
	{5257, ""},

	{5260, ""},
	{5261, "vsvdb_v1-12b-ll.bin"},
	{5263, "vsvdb_v1-12b-ll.bin"},
	{5264, ""},
	{5265, ""},
	{5267, ""},

	{5270, ""},
	{5271, "vsvdb_v1-12b-ll.bin"},
	{5273, "vsvdb_v1-12b-ll.bin"},
	{5274, ""},
	{5275, ""},
	{5277, ""},
};

struct dovi2hdr10_out_map_table_t dovi2hdr10_out_mapping[MAX_DOVI_TEST_CASE_ID] = {
	{1, 0},
	{2, 0},
	{3, 0},
	{4, 0},
	{5, 0},
	{6, 0},
	{7, 0},
	{8, 0},

	{11, 0},
	{12, 0},
	{13, 0},
	{14, 0},
	{15, 0},
	{16, 0},

	{5000, 0},
	{5001, 0},
	{5002, 0},
	{5003, 0},
	{5004, 0},
	{5005, 0},
	{5006, 0},
	{5007, 0},
	{5008, 0},
	{5009, 0},

	{5020, 0},
	{5021, 0},
	{5022, 0},
	{5023, 0},
	{5024, 0},

	{5100, 0},
	{5101, 0},
	{5102, 0},

	{5103, 0},
	{5104, 0},
	{5105, 0},
	{5106, 0},

	{5107, 0},

	{5025, 0},
	{5026, 0},
	{5027, 0},
	{5028, 0},

	{5030, 0},
	{5031, 0},
	{5032, 0},
	{5033, 0},
	{5034, 0},
	{5035, 0},
	{5036, 0},

	{5040, 0},
	{5041, 0},
	{5042, 0},
	{5043, 0},
	{5044, 0},
	{5045, 0},
	{5046, 0},
	{5047, 0},
	{5048, 0},
	{5049, 0},

	{5200, 0},
	{5201, 0},
	{5203, 0},
	{5204, 0},
	{5205, 0},
	{5207, 0},

	{5210, 0},
	{5211, 0},
	{5212, 0},
	{5213, 0},
	{5214, 0},
	{5215, 1},
	{5217, 1},

	{5220, 0},
	{5221, 0},
	{5223, 0},
	{5224, 0},
	{5225, 1},
	{5227, 1},

	{5230, 0},
	{5231, 0},
	{5233, 0},
	{5234, 0},
	{5235, 0},
	{5237, 1},

	{5240, 0},
	{5241, 0},
	{5243, 0},
	{5244, 0},
	{5245, 1},
	{5247, 1},

	{5250, 0},
	{5251, 0},
	{5253, 0},
	{5254, 0},
	{5255, 1},
	{5257, 1},

	{5260, 0},
	{5261, 0},
	{5263, 0},
	{5264, 0},
	{5265, 1},
	{5267, 1},
	{5270, 0},
	{5271, 0},
	{5273, 0},
	{5274, 0},
	{5275, 0},
	{5277, 0},

};

uint32_t force_priority_mode;
uint32_t priority_mode;
uint32_t force_g_format;
uint32_t g_format;

char dovi_graphic_name[][MAX_FILE_NAME_LEN] = {
	"/sdcard/dovi/03_colorSquare_FHD_Rec709_Gamma2.2.bgra",
	"/sdcard/dovi/04_colorSquare_UHD_Rec709_Gamma2.2.bgra",
};

struct dovi_graphic_info_table_t dovi_graphic_info[MAX_DOVI_TEST_CASE_ID] = {
	{1, "/sdcard/dovi/01_colorBar_FHD_Rec709_Gamma2.2.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{2, "/sdcard/dovi/03_colorSquare_FHD_Rec709_Gamma2.2.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{3, "/sdcard/dovi/05_colorSquare_FHD_Rec2020_PQ.rgba", 1, V_PRIORITY, GRAPHIC_HDR_RGB},
	{4, "/sdcard/dovi/05_colorSquare_FHD_Rec2020_PQ.rgba", 1, G_PRIORITY, GRAPHIC_HDR_RGB},
	{5, "/sdcard/dovi/01_colorBar_FHD_Rec709_Gamma2.2.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{6, "/sdcard/dovi/03_colorSquare_FHD_Rec709_Gamma2.2.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{7, "/sdcard/dovi/05_colorSquare_FHD_Rec2020_PQ.rgba", 1, V_PRIORITY, GRAPHIC_HDR_RGB},
	{8, "/sdcard/dovi/05_colorSquare_FHD_Rec2020_PQ.rgba", 1, G_PRIORITY, GRAPHIC_HDR_RGB},

	{1, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{2, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{3, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{4, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{6, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5000, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5001, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5002, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5003, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5004, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5005, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5006, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5007, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5008, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5009, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5020, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5021, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5022, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5023, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5024, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5100, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5101, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5102, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5103, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5104, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5105, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5106, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5107, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5025, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5026, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5027, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5028, "/sdcard/dovi/Graphic-IG_1920x1080_P444_8b_PQ_R2020.rgba", 1, V_PRIORITY, GRAPHIC_HDR_RGB},

	{5030, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5031, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5032, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5033, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5034, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5035, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5036, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5040, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5041, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5042, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5043, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5044, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5045, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5046, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5047, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5048, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5049, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5200, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5201, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5203, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5204, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5205, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5207, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5210, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5211, "", 0, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5212, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5213, "/sdcard/dovi/Graphic_720x480_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5214, "/sdcard/dovi/Graphic_720x480_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5215, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5217, "/sdcard/dovi/Graphic_720x480_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5220, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5221, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5223, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5224, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5225, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5227, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5230, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5231, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5233, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5234, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5235, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5237, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5240, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5241, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5243, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5244, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5245, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5247, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5250, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5251, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5253, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5254, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5255, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5257, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5260, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5261, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5263, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5264, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, G_PRIORITY, GRAPHIC_SDR_RGB},
	{5265, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5267, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

	{5270, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5271, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5273, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5274, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5275, "", 0, V_PRIORITY, GRAPHIC_SDR_RGB},
	{5277, "/sdcard/dovi/Graphic_1920x1080_P444_8b_SDR.rgba", 1, V_PRIORITY, GRAPHIC_SDR_RGB},

};


uint32_t get_dovi_out_format(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_out_format[idx].test_case_id)
			return dovi_out_format[idx].out_format;
	}

	return OUT_FORMAT_SDR;
}

uint32_t get_dovi_out_format_status(void)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		dovi_default("%d, %d\n",
			dovi_out_format[idx].test_case_id,
			dovi_out_format[idx].out_format);
	}

	return OUT_FORMAT_SDR;
}

uint32_t set_dovi_out_format(uint32_t test_case_id, uint32_t out_format)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_out_format[idx].test_case_id) {
			dovi_out_format[idx].out_format = out_format;

			return dovi_out_format[idx].out_format;
		}
	}

	return OUT_FORMAT_SDR;
}

char *get_dovi_vsvdb_file_name(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_case_info[idx].test_case_id)
			return dovi_case_info[idx].vsvdb_file_name;
	}

	return 0;
}

uint32_t get_dovi2hdr10_mapping_type(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi2hdr10_out_mapping[idx].test_case_id)
			return dovi2hdr10_out_mapping[idx].dovi2hdr10_mapping;
	}

	return 0;
}

uint32_t get_dovi2hdr10_mapping_type_Status(void)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		dovi_default("%d, %d\n",
			dovi2hdr10_out_mapping[idx].test_case_id,
			dovi2hdr10_out_mapping[idx].dovi2hdr10_mapping);
	}

	return 0;
}

uint32_t set_dovi2hdr10_mapping_type(uint32_t test_case_id, uint32_t dovi2hdr10_mapping)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_out_format[idx].test_case_id) {
			dovi2hdr10_out_mapping[idx].dovi2hdr10_mapping = dovi2hdr10_mapping;
			return dovi2hdr10_out_mapping[idx].dovi2hdr10_mapping;
		}
	}

	return 0;
}

uint32_t get_dovi_use_ll(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_case_info[idx].test_case_id)
			return dovi_case_info[idx].use_ll;
	}

	return 0;
}

uint32_t get_dovi_ll_rgb_desired(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_case_info[idx].test_case_id)
			return dovi_case_info[idx].ll_rgb_desired;
	}

	return 0;
}

uint32_t set_dovi_ll_mode(uint32_t test_case_id, uint32_t use_ll, uint32_t ll_rgb_desired)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_case_info[idx].test_case_id) {
			dovi_case_info[idx].use_ll = use_ll;
			dovi_case_info[idx].ll_rgb_desired = ll_rgb_desired;

			return dovi_case_info[idx].use_ll;
		}
	}

	return 0;
}

uint32_t get_dovi_ll_mode_status(void)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		dovi_default("[%4d]  %d %d\n",
			dovi_case_info[idx].test_case_id,
			dovi_case_info[idx].use_ll,
			dovi_case_info[idx].ll_rgb_desired);
	}

	return OUT_FORMAT_SDR;
}

int get_dovi_graphic_on(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_graphic_info[idx].test_case_id)
			return dovi_graphic_info[idx].f_graphic_on;
	}

	return 0;
}

enum pri_mode_t get_dovi_priority_mode(uint32_t test_case_id)
{
	uint32_t idx = 0;

	if (force_priority_mode)
		return (enum pri_mode_t)priority_mode;
	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_graphic_info[idx].test_case_id)
			return dovi_graphic_info[idx].priority_mode;
	}

	return V_PRIORITY;
}

enum graphic_format_t get_dovi_g_format(uint32_t test_case_id)
{
	uint32_t idx = 0;

	if (force_g_format)
		return (enum graphic_format_t)g_format;
	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_graphic_info[idx].test_case_id)
			return dovi_graphic_info[idx].g_format;
	}

	return GRAPHIC_SDR_RGB;
}

void set_dovi_priority_mode(uint32_t force_pri_mode, uint32_t pri_mode)
{
	force_priority_mode = force_pri_mode;
	priority_mode = pri_mode;
}

void set_dovi_g_format(uint32_t force_gformat, uint32_t gformat)
{
	force_g_format = force_gformat;
	g_format = gformat;
}

char *get_dovi_graphic_file_name(uint32_t test_case_id)
{
	uint32_t idx = 0;

	for (idx = 0; idx < MAX_DOVI_TEST_CASE_ID; idx++) {
		if (test_case_id == dovi_graphic_info[idx].test_case_id)
			return dovi_graphic_info[idx].graphic_file_name;
	}

	return 0;
}

void dovi_get_core3_x_start(uint16_t width, uint16_t height, uint16_t *x_start, uint16_t *y_start)
{
	uint16_t idx = 0;

	for (idx = 0; idx < CORE3_MAX_TIMING; idx++) {
		if ((width == core3_timing_info[idx].width)
			&& (height == core3_timing_info[idx].height)) {
			*x_start = core3_timing_info[idx].x_start;
			*y_start = core3_timing_info[idx].y_start;
			break;
		}
	}
}

