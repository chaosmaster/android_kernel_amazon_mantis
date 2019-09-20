/*
 * Copyright (c) 2017 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _NR_HW_H_
#define _NR_HW_H_

/*nr register */
#define RW_NR_CURR_Y_RD_ADDR             0x000
#define RW_NR_CURR_C_RD_ADDR             0x004
#define RW_NR_HD_RANGE_MAP               0x008
#define RW_NR_HD_ACTIVE                  0x01C
#define RW_NR_HD_HDE_RATIO               0x024
#define RW_NR_HD_LINE_OFST               0x028
#define RW_NR_HD_MODE_CTRL               0x02C
#define BURST_READ                          (0x1 << 23)
#define RW_NR_HD_SYNC_TRIGGER            0x030
#define RW_NR_HD_STATUS                  0x03C
#define RW_NR_HD_PATH_ENABLE             0x040

#define RW_NR_MISC_CTRL                  0x300
#define RW_NR_INT_CLR                    0x30C
#define RW_NR_DRAM_CTRL_00               0x400
#define RW_NR_DRAM_CTRL_01               0x404
#define RW_NR_Y_WR_SADDR                 0x410
#define RW_NR_Y_WR_EADDR                 0x414
#define RW_NR_C_WR_SADDR                 0x418
#define RW_NR_C_WR_EADDR                 0x41C
#define RW_NR_MISC_CTRL_01               0x43C
#define USE_HD_SRAM                         (0x1 << 3)
#define RW_NR_DRAM_CTRL_10               0x440
#define LAST_BURST_READ                     (0x1 << 1)
#define RW_NR_LAST_SIZE_CTRL           0x444
#define RW_NR_LAST_Y_RD_ADDR             0x450
#define RW_NR_LAST_C_RD_ADDR             0x458
#define RO_NR_WCHKSM_Y_00                0x500
#define RO_NR_WCHKSM_Y_01                0x504
#define RO_NR_WCHKSM_Y_02                0x508
#define RO_NR_WCHKSM_Y_03                0x50C
#define RO_NR_WCHKSM_Y_04                0x510
#define RO_NR_WCHKSM_Y_05                0x514
#define RO_NR_WCHKSM_Y_06                0x518
#define RO_NR_WCHKSM_Y_07                0x51C
#define RO_NR_WCHKSM_C_00                0x520
#define RO_NR_WCHKSM_C_01                0x524
#define RO_NR_WCHKSM_C_02                0x528
#define RO_NR_WCHKSM_C_03                0x52C
#define RO_NR_WCHKSM_C_04                0x530
#define RO_NR_WCHKSM_C_05                0x534
#define RO_NR_WCHKSM_C_06                0x538
#define RO_NR_WCHKSM_C_07                0x53C
#define RW_NR_MAIN_CTRL_00               0x800
#define RW_NR_MAIN_CTRL_01               0x804
#define RW_NR_YCBCR2YC_MAIN_00           0x874
#define RW_NR_CRC_SETTING                0x8A4
#define RW_NR_CRC_STATUS                 0x9E4
#define RW_NR_3D_00                      0xC00
#define C_LINE_HALF_WIDTH                   (0x3FF << 0)
#define RW_NR_3D_0D                      0xC34
#define C_LINE_MANUAL_LEN                   (0x3FF << 20)
#define RW_NR_2DNR_CTRL_00               0xD80
#define RW_NR_2DNR_CTRL_01               0xD84
#define RW_NR_2DNR_CTRL_02               0xD88
#define SLICE_X_POSITION                    (0x3FF << 16)
#define RW_NR_2DNR_CTRL_03               0xD8C
#define RW_NR_2DNR_CTRL_04               0xD90
#define RW_NR_2DNR_CTRL_05               0xD94
#define RW_NR_2DNR_CTRL_06               0xD98
#define RW_NR_2DNR_CTRL_07               0xD9C
#define RW_NR_2DNR_CTRL_08               0xDA0
#define RW_NR_2DNR_CTRL_09               0xDA4
#define RW_NR_2DNR_CTRL_0A               0xDA8
#define RW_NR_2DNR_CTRL_0B               0xDAC
#define RW_NR_2DNR_CTRL_0C               0xDB0
#define RW_NR_2DNR_CTRL_0D               0xDB4
#define RW_NR_2DNR_CTRL_0E               0xDB8
#define RW_NR_2DNR_CTRL_0F               0xDBC
#define RW_NR_2DNR_CTRL_10               0xDC0
#define RW_NR_2DNR_CTRL_11               0xDC4
#define RW_NR_2DNR_CTRL_12               0xDC8
#define RW_NR_2DNR_CTRL_13               0xDCC
#define RW_NR_2DNR_CTRL_14               0xDD0
#define RW_NR_2DNR_CTRL_15               0xDD4
#define RW_NR_2DNR_CTRL_16               0xDD8
#define RW_NR_2DNR_CTRL_17               0xDDC
#define RW_NR_2DNR_CTRL_18               0xDE0
#define RW_NR_2DNR_CTRL_19               0xDE4
#define RW_NR_2DNR_CTRL_1A               0xDE8
#define RW_NR_2DNR_CTRL_1B               0xDEC
#define RW_NR_2DNR_CTRL_1C               0xDF0
#define RW_NR_2DNR_CTRL_1D               0xDF4
#define RW_NR_2DNR_CTRL_1E               0xDF8
#define RW_NR_2DNR_CTRL_1F               0xE00
#define SLICE_DEMO_CTRL                     (0x1 << 13)
#define SLICE_DEMO_ENABLE                   (0x1 << 12)
#define BLOCK_METER_ENABLE                  (0x1 << 8)
#define BLOCK_PROC_ENABLE                   (0x1 << 7)
#define RW_NR_2DNR_CTRL_20               0xE04
#define RW_NR_2DNR_CTRL_21               0xE08
#define RW_NR_2DNR_CTRL_22               0xE0C
#define RW_NR_2DNR_CTRL_23               0xE10
#define RW_NR_2DNR_CTRL_24               0xE14
#define RW_NR_2DNR_CTRL_25               0xE18
#define RW_NR_2DNR_CTRL_26               0xE1C
#define RW_NR_2DNR_CTRL_27               0xE20
#define RW_NR_2DNR_CTRL_28               0xE24
#define RW_NR_2DNR_CTRL_29               0xE28
#define RW_NR_2DNR_CTRL_2A               0xE2C
#define RW_NR_2DNR_CTRL_2B               0xE30
#define RW_NR_2DNR_CTRL_2C               0xE34
#define RW_NR_2DNR_CTRL_2D               0xE38
#define RW_NR_2DNR_CTRL_2E               0xE3C
#define RW_NR_2DNR_CTRL_2F               0xE40
#define RW_NR_2DNR_CTRL_30               0xE44
#define RW_NR_2DNR_CTRL_31               0xE48
#define RW_NR_2DNR_CTRL_32               0xE4C
#define RW_NR_2DNR_CTRL_33               0xE50
#define RW_NR_2DNR_CTRL_34               0xE54
#define RW_NR_2DNR_CTRL_35               0xE58
#define RW_NR_2DNR_CTRL_36               0xE5C
#define RW_NR_2DNR_CTRL_37               0xE60
#define MESSSFT_SMOOTH_CO1MO                (0x3F << 24)
#define MESSTHL_SMOOTH_CO1MO                (0x3F << 16)
#define MESSSFT_EDGE_CO1MO                  (0x3F << 8)
#define MESSTHL_EDGE_CO1MO                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_38               0xE64
#define MESSSFT_MESS_CO1MO                  (0x3F << 24)
#define MESSTHL_MESS_CO1MO                  (0x3F << 16)
#define MESSSFT_MOS_CO1MO                   (0x3F << 8)
#define MESSTHL_MOS_CO1MO                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_39               0xE68
#define MESSSFT_SMOOTH_CO1ST                (0x3F << 24)
#define MESSTHL_SMOOTH_CO1ST                (0x3F << 16)
#define MESSSFT_EDGE_CO1ST                  (0x3F << 8)
#define MESSTHL_EDGE_CO1ST                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_3A               0xE6C
#define MESSSFT_MESS_CO1ST                  (0x3F << 24)
#define MESSTHL_MESS_CO1ST                  (0x3F << 16)
#define MESSSFT_MOS_CO1ST                   (0x3F << 8)
#define MESSTHL_MOS_CO1ST                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_3B               0xE70
#define MESSSFT_SMOOTH_CO2MO                (0x3F << 24)
#define MESSTHL_SMOOTH_CO2MO                (0x3F << 16)
#define MESSSFT_EDGE_CO2MO                  (0x3F << 8)
#define MESSTHL_EDGE_CO2MO                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_3C               0xE74
#define MESSSFT_MESS_CO2MO                  (0x3F << 24)
#define MESSTHL_MESS_CO2MO                  (0x3F << 16)
#define MESSSFT_MOS_CO2MO                   (0x3F << 8)
#define MESSTHL_MOS_CO2MO                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_3D               0xE78
#define MESSSFT_SMOOTH_CO2ST                (0x3F << 24)
#define MESSTHL_SMOOTH_CO2ST                (0x3F << 16)
#define MESSSFT_EDGE_CO2ST                  (0x3F << 8)
#define MESSTHL_EDGE_CO2ST                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_3E               0xE7C
#define MESSSFT_MESS_CO2ST                  (0x3F << 24)
#define MESSTHL_MESS_CO2ST                  (0x3F << 16)
#define MESSSFT_MOS_CO2ST                   (0x3F << 8)
#define MESSTHL_MOS_CO2ST                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_3F               0xE80
#define MESSSFT_SMOOTH_CO3MO                (0x3F << 24)
#define MESSTHL_SMOOTH_CO3MO                (0x3F << 16)
#define MESSSFT_EDGE_CO3MO                  (0x3F << 8)
#define MESSTHL_EDGE_CO3MO                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_40               0xE84
#define MESSSFT_MESS_CO3MO                  (0x3F << 24)
#define MESSTHL_MESS_CO3MO                  (0x3F << 16)
#define MESSSFT_MOS_CO3MO                   (0x3F << 8)
#define MESSTHL_MOS_CO3MO                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_41               0xE88
#define MESSSFT_SMOOTH_CO3ST                (0x3F << 24)
#define MESSTHL_SMOOTH_CO3ST                (0x3F << 16)
#define MESSSFT_EDGE_CO3ST                  (0x3F << 8)
#define MESSTHL_EDGE_CO3ST                  (0x3F << 0)
#define RW_NR_2DNR_CTRL_42               0xE8C
#define MESSSFT_MESS_CO3ST                  (0x3F << 24)
#define MESSTHL_MESS_CO3ST                  (0x3F << 16)
#define MESSSFT_MOS_CO3ST                   (0x3F << 8)
#define MESSTHL_MOS_CO3ST                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_43               0xE90
#define RW_NR_2DNR_CTRL_44               0xE94
#define RW_NR_2DNR_CTRL_45               0xE98
#define RW_NR_2DNR_CTRL_46               0xE9C
#define RW_NR_2DNR_CTRL_47               0xEA0
#define RW_NR_2DNR_CTRL_48               0xEA4
#define RW_NR_2DNR_CTRL_49               0xEA8
#define RW_NR_2DNR_CTRL_4A               0xEAC
#define RW_NR_2DNR_CTRL_4B               0xEB0
#define RW_NR_2DNR_CTRL_4C               0xEB4
#define RW_NR_2DNR_CTRL_4D               0xEB8
#define RW_NR_2DNR_CTRL_4E               0xEBC
#define RW_NR_2DNR_CTRL_4F               0xEC0
#define MESSSFT_SMOOTH_FRST                 (0x3F << 24)
#define MESSTHL_SMOOTH_FRST                 (0x3F << 16)
#define MESSSFT_EDGE_FRST                   (0x3F << 8)
#define MESSTHL_EDGE_FRST                   (0x3F << 0)
#define RW_NR_2DNR_CTRL_50               0xEC4
#define MESSSFT_MESS_FRST                   (0x3F << 24)
#define MESSTHL_MESS_FRST                   (0x3F << 16)
#define MESSSFT_MOS_FRST                    (0x3F << 8)
#define MESSTHL_MOS_FRST                    (0x3F << 0)
#define RW_NR_2DNR_CTRL_51               0xEC8
#define MESSSFT_SMOOTH_MO                   (0x3F << 24)
#define MESSTHL_SMOOTH_MO                   (0x3F << 16)
#define MESSSFT_EDGE_MO                     (0x3F << 8)
#define MESSTHL_EDGE_MO                     (0x3F << 0)
#define RW_NR_2DNR_CTRL_52               0xECC
#define MESSSFT_MESS_MO                     (0x3F << 24)
#define MESSTHL_MESS_MO                     (0x3F << 16)
#define MESSSFT_MOS_MO                      (0x3F << 8)
#define MESSTHL_MOS_MO                      (0x3F << 0)
#define RW_NR_2DNR_CTRL_53               0xED0
#define MESSSFT_SMOOTH_ST                   (0x3F << 24)
#define MESSTHL_SMOOTH_ST                   (0x3F << 16)
#define MESSSFT_EDGE_ST                     (0x3F << 8)
#define MESSTHL_EDGE_ST                     (0x3F << 0)
#define RW_NR_2DNR_CTRL_54               0xED4
#define MESSSFT_MESS_ST                     (0x3F << 24)
#define MESSTHL_MESS_ST                     (0x3F << 16)
#define MESSSFT_MOS_ST                      (0x3F << 8)
#define MESSTHL_MOS_ST                      (0x3F << 0)
#define RW_NR_2DNR_CTRL_55               0xED8
#define MESSSFT_SMOOTH_BK                   (0x3F << 24)
#define MESSTHL_SMOOTH_BK                   (0x3F << 16)
#define MESSSFT_EDGE_BK                     (0x3F << 8)
#define MESSTHL_EDGE_BK                     (0x3F << 0)
#define RW_NR_2DNR_CTRL_56               0xEDC
#define MESSSFT_MESS_BK                     (0x3F << 24)
#define MESSTHL_MESS_BK                     (0x3F << 16)
#define MESSSFT_MOS_BK                      (0x3F << 8)
#define MESSTHL_MOS_BK                      (0x3F << 0)
#define RW_NR_2DNR_CTRL_57               0xEE0
#define MESSSFT_SMOOTH_DEF                  (0x3F << 24)
#define MESSTHL_SMOOTH_DEF                  (0x3F << 16)
#define MESSSFT_EDGE_DEF                    (0x3F << 8)
#define MESSTHL_EDGE_DEF                    (0x3F << 0)
#define RW_NR_2DNR_CTRL_58               0xEE4
#define MESSSFT_MESS_DEF                    (0x3F << 24)
#define MESSTHL_MESS_DEF                    (0x3F << 16)
#define MESSSFT_MOS_DEF                     (0x3F << 8)
#define MESSTHL_MOS_DEF                     (0x3F << 0)
#define RW_NR_2DNR_CTRL_92               0xEF8
#define RW_NR_2DNR_CTRL_59               0xEFC
#define RW_NR_2DNR_CTRL_5A               0xF00
#define RW_NR_2DNR_CTRL_5B               0xF04
#define BK_METER_WIDTH                      (0x3FF << 16)
#define BK_METER_HEIGHT                     (0x7FF << 0)
#define RW_NR_2DNR_CTRL_5C               0xF08
#define RW_NR_2DNR_CTRL_5D               0xF0C
#define RW_NR_2DNR_CTRL_5E               0xF10
#define RW_NR_2DNR_CTRL_5F               0xF14
#define RW_NR_2DNR_CTRL_60               0xF18
#define RW_NR_2DNR_CTRL_61               0xF1C
#define RW_NR_2DNR_CTRL_62               0xF20
#define RW_NR_2DNR_CTRL_63               0xF24
#define RW_NR_2DNR_CTRL_64               0xF28
#define RW_NR_2DNR_CTRL_65               0xF2C
#define MNR_SM_THR                          (0xFF << 24)
#define MNR_EDGE_THR                        (0xFF << 16)
#define SM_NUM_THR                          (0xF << 8)
#define NEAREDGE_SEL_WIDTH                  (0xF << 0)
#define RW_NR_2DNR_CTRL_66               0xF30
#define RW_NR_2DNR_CTRL_67               0xF34
#define RW_NR_2DNR_CTRL_68               0xF38
#define RW_NR_2DNR_CTRL_69               0xF3C
#define RW_NR_2DNR_CTRL_6A               0xF40
#define RW_NR_2DNR_CTRL_6B               0xF44
#define RW_NR_2DNR_CTRL_6C               0xF48
#define RW_NR_2DNR_CTRL_6E               0xF50
#define Y_GLOBAL_BLEND                      (0xF << 28)
#define RW_NR_2DNR_CTRL_6F               0xF54
#define RW_NR_2DNR_CTRL_70               0xF58
#define RW_NR_2DNR_CTRL_71               0xF60
#define RW_NR_2DNR_CTRL_72               0xF64
#define RW_NR_2DNR_CTRL_73               0xF68
#define RW_NR_2DNR_CTRL_74               0xF6C
#define RW_NR_2DNR_CTRL_75               0xF70
#define RW_NR_2DNR_CTRL_76               0xF74
#define RW_NR_2DNR_CTRL_77               0xF78
#define RW_NR_2DNR_CTRL_78               0xF7C
#define RW_NR_2DNR_CTRL_79               0xF80
#define RW_NR_2DNR_CTRL_93               0xF84
#define RW_NR_2DNR_CTRL_94               0xF88
#define RW_NR_2DNR_CTRL_95               0xF8C
#define RW_NR_2DNR_CTRL_96               0xF90
#define RW_NR_2DNR_CTRL_97               0xF94
#define RW_NR_2DNR_CTRL_7E               0xF98
#define BLDLV_BK_ST                         (0xF << 28)
#define BLDLV_SM_ST                         (0xF << 24)
#define BLDLV_MESS_ST                       (0xF << 20)
#define BLDLV_EDGE_ST                       (0xF << 16)
#define BLDLV_BK_DEF                        (0xF << 12)
#define BLDLV_SM_DEF                        (0xF << 8)
#define BLDLV_MESS_DEF                      (0xF << 4)
#define BLDLV_EDGE_DEF                      (0xF << 0)
#define RW_NR_2DNR_CTRL_7F               0xF9C
#define BLDLV_BK_BK                         (0xF << 28)
#define BLDLV_SM_BK                         (0xF << 24)
#define BLDLV_MESS_BK                       (0xF << 20)
#define BLDLV_EDGE_BK                       (0xF << 16)
#define BLDLV_BK_MO                         (0xF << 12)
#define BLDLV_SM_MO                         (0xF << 8)
#define BLDLV_MESS_MO                       (0xF << 4)
#define BLDLV_EDGE_MO                       (0xF << 0)
#define RW_NR_2DNR_CTRL_80               0xFA0
#define BLDLV_BK_FRST                       (0xF << 28)
#define BLDLV_SM_FRST                       (0xF << 24)
#define BLDLV_MESS_FRST                     (0xF << 20)
#define BLDLV_EDGE_FRST                     (0xF << 16)
#define BLDLV_BK_CO1                        (0xF << 12)
#define BLDLV_SM_CO1                        (0xF << 8)
#define BLDLV_MESS_CO1                      (0xF << 4)
#define BLDLV_EDGE_CO1                      (0xF << 0)
#define RW_NR_2DNR_CTRL_81               0xFA4
#define BLDLV_BK_CO2                        (0xF << 28)
#define BLDLV_SM_CO2                        (0xF << 24)
#define BLDLV_MESS_CO2                      (0xF << 20)
#define BLDLV_EDGE_CO2                      (0xF << 16)
#define BLDLV_BK_CO3                        (0xF << 12)
#define BLDLV_SM_CO3                        (0xF << 8)
#define BLDLV_MESS_CO3                      (0xF << 4)
#define BLDLV_EDGE_CO3                      (0xF << 0)
#define RW_NR_2DNR_CTRL_82               0xFA8
#define RW_NR_2DNR_CTRL_83               0xFAC
#define BLDLV_MOS_BK                        (0xF << 28)
#define BLDLV_MOS_MO                        (0xF << 24)
#define BLDLV_MOS_ST                        (0xF << 20)
#define BLDLV_MOS_DEF                       (0xF << 16)
#define RW_NR_2DNR_CTRL_84               0xFB0
#define BLDLV_MOS_CO3                       (0xF << 12)
#define BLDLV_MOS_CO2                       (0xF << 8)
#define BLDLV_MOS_CO1                       (0xF << 4)
#define BLDLV_NEAR_FRST                     (0xF << 0)
#define RW_NR_2DNR_CTRL_85               0xFB4
#define RW_NR_2DNR_CTRL_86               0xFB8
#define RW_NR_2DNR_CTRL_87               0xFBC
#define RW_NR_2DNR_CTRL_88               0xFC0
#define RW_NR_2DNR_CTRL_89               0xFC4
#define RW_NR_2DNR_CTRL_8A               0xFC8
#define RW_NR_2DNR_CTRL_8B               0xFCC
#define RW_NR_2DNR_CTRL_8C               0xFD0
#define RW_NR_2DNR_CTRL_8D               0xFD4
#define RW_NR_2DNR_CTRL_8E               0xFD8
#define RW_NR_2DNR_CTRL_98               0xFDC
#define RW_NR_2DNR_CTRL_99               0xFE0
#define RW_NR_2DNR_CTRL_9A               0xFE4
#define RW_NR_2DNR_CTRL_9B               0xFE8
#define RW_NR_2DNR_CTRL_9C               0xFEC
#define RW_NR_2DNR_CTRL_9D               0xFF0
#define RW_NR_2DNR_CTRL_8F               0xFF4
#define RW_NR_2DNR_CTRL_90               0xFF8
#define RW_NR_2DNR_CTRL_91               0xFFC

#endif				/* _NR_HW_H_ */
