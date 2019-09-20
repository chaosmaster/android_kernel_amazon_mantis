/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Chen Zhong <chen.zhong@mediatek.com>
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

#ifndef _DT_BINDINGS_CLK_MT8695_H
#define _DT_BINDINGS_CLK_MT8695_H

/* APMIXEDSYS */

#define CLK_APMIXED_ARMPLL		0
#define CLK_APMIXED_MAINPLL		1
#define CLK_APMIXED_UNIV2PLL		2
#define CLK_APMIXED_MMPLL		3
#define CLK_APMIXED_MSDCPLL		4
#define CLK_APMIXED_TVDPLL		5
#define CLK_APMIXED_ETHERPLL		6
#define CLK_APMIXED_VDECPLL		7
#define CLK_APMIXED_OSDPLL		8
#define CLK_APMIXED_APLL1		9
#define CLK_APMIXED_APLL2		10
#define CLK_APMIXED_HDMITX20_CKSEL	11
#define CLK_APMIXED_NR_CLK		12

/* TOPCKGEN */

#define CLK_TOP_ARMCA35PLL_600M		0
#define CLK_TOP_ARMCA35PLL_400M		1
#define CLK_TOP_SYSPLL			2
#define CLK_TOP_SYSPLL_D2		3
#define CLK_TOP_SYSPLL1_D2		4
#define CLK_TOP_SYSPLL1_D4		5
#define CLK_TOP_SYSPLL1_D16		6
#define CLK_TOP_SYSPLL_D3		7
#define CLK_TOP_SYSPLL2_D2		8
#define CLK_TOP_SYSPLL2_D4		9
#define CLK_TOP_SYSPLL_D5		10
#define CLK_TOP_SYSPLL3_D2		11
#define CLK_TOP_SYSPLL3_D4		12
#define CLK_TOP_SYSPLL_D7		13
#define CLK_TOP_SYSPLL4_D2		14
#define CLK_TOP_SYSPLL4_D4		15
#define CLK_TOP_UNIVPLL			16
#define CLK_TOP_UNIVPLL_D7		17
#define CLK_TOP_UNIVPLL_D26		18
#define CLK_TOP_UNIVPLL_D52		19
#define CLK_TOP_UNIVPLL_D2		20
#define CLK_TOP_UNIVPLL1_D2		21
#define CLK_TOP_UNIVPLL1_D4		22
#define CLK_TOP_UNIVPLL1_D8		23
#define CLK_TOP_UNIVPLL_D3		24
#define CLK_TOP_UNIVPLL2_D2		25
#define CLK_TOP_UNIVPLL2_D4		26
#define CLK_TOP_UNIVPLL2_D8		27
#define CLK_TOP_UNIVPLL_D5		28
#define CLK_TOP_UNIVPLL3_D2		29
#define CLK_TOP_UNIVPLL3_D4		30
#define CLK_TOP_UNIVPLL3_D8		31
#define CLK_TOP_F_MP0_PLL1		32
#define CLK_TOP_F_MP0_PLL2		33
#define CLK_TOP_APLL1			34
#define CLK_TOP_APLL1_D2		35
#define CLK_TOP_APLL1_D4		36
#define CLK_TOP_APLL1_D8		37
#define CLK_TOP_APLL1_D16		38
#define CLK_TOP_APLL2			39
#define CLK_TOP_APLL2_D2		40
#define CLK_TOP_APLL2_D4		41
#define CLK_TOP_APLL2_D8		42
#define CLK_TOP_APLL2_D16		43
#define CLK_TOP_OSDPLL			44
#define CLK_TOP_OSDPLL_D2		45
#define CLK_TOP_OSDPLL_D3		46
#define CLK_TOP_OSDPLL_D4		47
#define CLK_TOP_OSDPLL_D6		48
#define CLK_TOP_OSDPLL_D12		49
#define CLK_TOP_ETHERPLL_125M		50
#define CLK_TOP_ETHERPLL_50M		51
#define CLK_TOP_SYS_26M			52
#define CLK_TOP_MMPLL			53
#define CLK_TOP_MMPLL_D2		54
#define CLK_TOP_VDECPLL			55
#define CLK_TOP_TVDPLL			56
#define CLK_TOP_TVDPLL_D2		57
#define CLK_TOP_TVDPLL_D4		58
#define CLK_TOP_TVDPLL_D8		59
#define CLK_TOP_MSDCPLL			60
#define CLK_TOP_MSDCPLL_D2		61
#define CLK_TOP_MSDCPLL_D4		62
#define CLK_TOP_CLK26M_D2		63
#define CLK_TOP_DMPLL			64
#define CLK_TOP_CLKRTC_INT		65
#define CLK_TOP_NFI1X			66
#define CLK_TOP_AXI_SEL			67
#define CLK_TOP_MEM_SEL			68
#define CLK_TOP_SD_SEL			69
#define CLK_TOP_MM_SEL			70
#define CLK_TOP_VDEC_LAE_SEL		71
#define CLK_TOP_VDEC_SEL		72
#define CLK_TOP_VDEC_SLOW_SEL		73
#define CLK_TOP_VENC_SEL		74
#define CLK_TOP_MFG_SEL			75
#define CLK_TOP_RSZ_SEL			76
#define CLK_TOP_UART_SEL		77
#define CLK_TOP_SPI_SEL			78
#define CLK_TOP_USB20_SEL		79
#define CLK_TOP_USB30_SEL		80
#define CLK_TOP_MSDC50_0_HCLK_SEL	81
#define CLK_TOP_MSDC50_0_SEL		82
#define CLK_TOP_MSDC30_1_SEL		83
#define CLK_TOP_MSDC30_2_SEL		84
#define CLK_TOP_MSDC50_2_HCLK_SEL	85
#define CLK_TOP_MSDC0P_AES_SEL		86
#define CLK_TOP_INTDIR_SEL		87
#define CLK_TOP_AUDIO_SEL		88
#define CLK_TOP_AUD_INTBUS_SEL		89
#define CLK_TOP_OSD_SEL			90
#define CLK_TOP_VDO3_SEL		91
#define CLK_TOP_VDO4_SEL		92
#define CLK_TOP_HD_SEL			93
#define CLK_TOP_NR_SEL			94
#define CLK_TOP_CCI400_SEL		95
#define CLK_TOP_AUD_1_SEL		96
#define CLK_TOP_AUD_2_SEL		97
#define CLK_TOP_MEM_MFG_IN_AS_SEL	98
#define CLK_TOP_SLOW_MFG_SEL		99
#define CLK_TOP_AXI_MFG_IN_AS_SEL	100
#define CLK_TOP_HDCP_SEL		101
#define CLK_TOP_HDCP_24M_SEL		102
#define CLK_TOP_CLK32K			103
#define CLK_TOP_SPINOR_SEL		104
#define CLK_TOP_APLL_SEL		105
#define CLK_TOP_APLL2_SEL		106
#define CLK_TOP_A1SYS_HP_SEL		107
#define CLK_TOP_A2SYS_HP_SEL		108
#define CLK_TOP_ASM_L_SEL		109
#define CLK_TOP_ASM_M_SEL		110
#define CLK_TOP_ASM_H_SEL		111
#define CLK_TOP_I2SO1_SEL		112
#define CLK_TOP_I2SO2_SEL		113
#define CLK_TOP_I2SI1_SEL		114
#define CLK_TOP_I2SI2_SEL		115
#define CLK_TOP_ETHER_125M_SEL		116
#define CLK_TOP_ETHER_50M_SEL		117
#define CLK_TOP_SPISLV_SEL		118
#define CLK_TOP_I2C_SEL			119
#define CLK_TOP_PWM_INFRA_SEL		120
#define CLK_TOP_GCPU_SEL		121
#define CLK_TOP_ECC_SEL			122
#define CLK_TOP_DI_SEL			123
#define CLK_TOP_DMX_SEL			124
#define CLK_TOP_NFI2X_SEL		125
#define CLK_TOP_TDMIN_SEL		126
#define CLK_TOP_APLL_DIV0		127
#define CLK_TOP_APLL_DIV1		128
#define CLK_TOP_APLL_DIV5		129
#define CLK_TOP_APLL_DIV6		130
#define CLK_TOP_APLL_DIV_PDN0		131
#define CLK_TOP_APLL_DIV_PDN1		132
#define CLK_TOP_APLL_DIV_PDN5		133
#define CLK_TOP_APLL_DIV_PDN6		134
#define CLK_TOP_NFI2X_EN		135
#define CLK_TOP_NFI1X_EN		136
#define CLK_TOP_NR_CLK			137

/* INFRACFG */

#define CLK_INFRA_DBGCLK		0
#define CLK_INFRA_GCE			1
#define CLK_INFRA_M4U			2
#define CLK_INFRA_KP			3
#define CLK_INFRA_CEC			4
#define CLK_INFRA_TRNG			5
#define CLK_INFRA_NR_CLK		6

/* PERICFG */

#define CLK_PERI_UART0_SEL		0
#define CLK_PERI_UART1_SEL		1
#define CLK_PERI_UART2_SEL		2
#define CLK_PERI_NFI			3
#define CLK_PERI_THERM			4
#define CLK_PERI_PWM0			5
#define CLK_PERI_PWM1			6
#define CLK_PERI_PWM2			7
#define CLK_PERI_PWM3			8
#define CLK_PERI_PWM4			9
#define CLK_PERI_PWM5			10
#define CLK_PERI_PWM6			11
#define CLK_PERI_PWM7			12
#define CLK_PERI_PWM			13
#define CLK_PERI_USB			14
#define CLK_PERI_AP_DMA			15
#define CLK_PERI_MSDC30_0		16
#define CLK_PERI_MSDC30_1		17
#define CLK_PERI_MSDC30_2		18
#define CLK_PERI_UART0			19
#define CLK_PERI_UART1			20
#define CLK_PERI_UART2			21
#define CLK_PERI_I2C0			22
#define CLK_PERI_I2C1			23
#define CLK_PERI_I2C2			24
#define CLK_PERI_I2C3			25
#define CLK_PERI_I2C4			26
#define CLK_PERI_AUXADC			27
#define CLK_PERI_SPI0			28
#define CLK_PERI_SFLASH			29
#define CLK_PERI_SPI2			30
#define CLK_PERI_HAXI_SFLASH		31
#define CLK_PERI_GMAC			32
#define CLK_PERI_GMAC_PCLK		33
#define CLK_PERI_PTP_THERM		34
#define CLK_PERI_MSDC50_0_EN		35
#define CLK_PERI_MSDC30_1_EN		36
#define CLK_PERI_MSDC30_2_EN		37
#define CLK_PERI_MSDC50_0_HCLK_EN	38
#define CLK_PERI_MSDC50_2_HCLK_EN	39
#define CLK_PERI_NR_CLK			40

/* MCUCFG */

#define CLK_MCU_BUS_SEL			0
#define CLK_MCU_NR_CLK			1

/* MFGCFG */

#define CLK_MFG_BAXI			0
#define CLK_MFG_BMEM			1
#define CLK_MFG_BG3D			2
#define CLK_MFG_B26M			3
#define CLK_MFG_NR_CLK			4

/* MMSYS */

#define CLK_MM_SMI_COMMON		0
#define CLK_MM_SMI_LARB0		1
#define CLK_MM_DRAM_LARB		2
#define CLK_MM_FAKE_ENG			3
#define CLK_MM_SMI_LARB4		4
#define CLK_MM_SMI_LARB1		5
#define CLK_MM_SMI_LARB5		6
#define CLK_MM_SMI_LARB6		7
#define CLK_MM_SMI_LARB7		8
#define CLK_MM_VDEC2IMG			9
#define CLK_MM_VDOUT			10
#define CLK_MM_FMT_TG			11
#define CLK_MM_FMT_DGO			12
#define CLK_MM_TVE			13
#define CLK_MM_CRC			14
#define CLK_MM_OSD_TVE			15
#define CLK_MM_OSD_FHD			16
#define CLK_MM_OSD_UHD			17
#define CLK_MM_P2I			18
#define CLK_MM_HDMITX			19
#define CLK_MM_RGB2HDMI			20
#define CLK_MM_SCLER			21
#define CLK_MM_SDPPF			22
#define CLK_MM_VDOIN			23
#define CLK_MM_DOLBY1			24
#define CLK_MM_DOLBY2			25
#define CLK_MM_DOLBY3			26
#define CLK_MM_OSD_SDR2HDR		27
#define CLK_MM_OSD_PREMIX		28
#define CLK_MM_DOLBY_MIX		29
#define CLK_MM_VM			30
#define CLK_MM_NR_CLK			31

/* DISP */

#define CLK_DISP_VDO3			0
#define CLK_DISP_FMT3			1
#define CLK_DISP_MV_HDR2SDR		2
#define CLK_DISP_MV_BT2020		3
#define CLK_DISP_SMI_LARB5		4
#define CLK_DISP_SMI_LARB6		5
#define CLK_DISP_DRAMC_LARB8		6
#define CLK_DISP_VDO4			7
#define CLK_DISP_FMT4			8
#define CLK_DISP_SV_HDR2SDR		9
#define CLK_DISP_SV_BT2020		10
#define CLK_DISP_IRT_DMA		11
#define CLK_DISP_RSZ0			12
#define CLK_DISP_VDO_DI			13
#define CLK_DISP_FMT_DI			14
#define CLK_DISP_NR			15
#define CLK_DISP_WR_CHANNEL		16
#define CLK_DISP_NR_CLK			17

/* VDECSOC */

#define CLK_VDSOC_SOC			0
#define CLK_VDSOC_LARB7			1
#define CLK_VDSOC_NR_CLK		2

/* VDECCORE */

#define CLK_VDCORE_VDEC			0
#define CLK_VDCORE_LARB1		1
#define CLK_VDCORE_NR_CLK		2

/* VENCSYS */

#define CLK_VENC_SMI			0
#define CLK_VENC			1
#define CLK_VENC_NR_CLK			2

/* DEMUXSYS */

#define CLK_DEMUX_DMX_SMI		0
#define CLK_DEMUX			1
#define CLK_DEMUX_NR_CLK		2

#endif /* _DT_BINDINGS_CLK_MT8695_H */

