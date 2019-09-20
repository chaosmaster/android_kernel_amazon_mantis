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
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <hdmitx.h>
#include <linux/of_gpio.h>
#include <linux/io.h>
#include <linux/fb.h>
#include <linux/notifier.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>

#include "hdmi_ctrl.h"
#include "hdmictrl.h"
#include "hdmiddc.h"
#include "hdmihdcp.h"
#include "hdmicec.h"
#include "hdmiedid.h"
#include "hdmi_tx_cod.h"

#include "internal_hdmi_drv.h"
#include <linux/pm_runtime.h>

#include "hdmiavd.h"
#include "hdmicmd.h"

#include <linux/pinctrl/consumer.h>
#include <linux/nvmem-consumer.h>
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
#include "hdmi_ca.h"
#endif
#define SAVELENGTH 100
#define SHOW_HDMISTATE_LOG_TIME 500
#define MASK_UNUSE_INTERRUPT_TIME 200
#define HPDTOGGLE_VIDEOCONFIG 0x1
#define SWTICHTOGGLE_VIDEOCONFIG 0x2
#define CLEANTOGGLE_VIDEOCONFIG 0x0

enum hdcp_version {
	NO_HDCP,
	HDCP_V1_4,
	HDCP_V2_0,
	HDCP_V2_1,
	HDCP_V2_2
};

/*----------------------------------------------------------------------------*/
/* Debug message defination */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* HDMI Timer */
/*----------------------------------------------------------------------------*/
static struct timer_list r_hdmi_timer;
static struct timer_list r_cec_timer;
unsigned int hdmistate_debug = 0xfff;
bool resolution_change = FALSE;
unsigned int hdmisave_irq_reg[SAVELENGTH][7];
unsigned char deeplength;
unsigned char temprgb2hdmi = 1;
struct platform_device *hdmi_pdev;

static unsigned int gHDMI_CHK_INTERVAL = 10;
static unsigned int gCEC_CHK_INTERVAL = 20;

unsigned int hdmidrv_log_on = hdmialllog;
/* HDMI efuse */
unsigned int caefuse0;
unsigned int caefuse1;
unsigned int caefuse2;
unsigned int caefuse3;

unsigned char efuseValue0;
unsigned char efuseValue1;
unsigned char efuseValue2;
unsigned char efuseValue3;
unsigned char efuseValue4;

unsigned char hdmi_cec_on;
unsigned char hdmi_hdmi_on;
unsigned char hdmi_powerenable;
unsigned char hdmi_clockenable;
unsigned char hdmi_sdcksel;
unsigned char hdmi_sdcksel_first;
unsigned char hdmi_sdosd_first;
unsigned char hdmi_hdtvd_first;
unsigned char hdmi_5vpower;

unsigned int hdmi_TmrValue[MAX_HDMI_TMR_NUMBER] = { 0 };

unsigned char hdmi_hdmiCmd = 0xff;
unsigned char hdmi_rxcecmode = CEC_NORMAL_MODE;
HDMI_CTRL_STATE_T e_hdmi_ctrl_state = HDMI_STATE_IDLE;
HDCP_CTRL_STATE_T e_hdcp_ctrl_state = HDCP_RECEIVER_NOT_READY;
unsigned int hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
unsigned int hdmi_hotplugout_count = 0;
long long hdmi_dispres;

#define HDMI_UNUSED_TIMER    (-1)
#define HDMI_CTRL_TMR_INX  0
#define HDCP_CTRL_TMR_INX  1
#define HDMI_DELAY_TMR_INX    2
#define HDMI_HDR_TMR_INX      3
#define HDMI_HBR_TMR_INX      4

unsigned int hdmi_audio_event = 0xff;

static unsigned char port_hpd_value_bak = 0xff;
static bool port_hpd_value_chg = FALSE;

unsigned char hdcp2_version_flag = FALSE;
unsigned char hdcp_current_level;
unsigned char hdcp_max_level;
unsigned char hpd_toggle_videoconfig_flag = 0xff;
static struct task_struct *hdmi_timer_task;
wait_queue_head_t hdmi_timer_wq;
atomic_t hdmi_timer_event = ATOMIC_INIT(0);

static struct task_struct *cec_timer_task;
wait_queue_head_t cec_timer_wq;
atomic_t cec_timer_event = ATOMIC_INIT(0);

static struct task_struct *hdmi_irq_task;
wait_queue_head_t hdmi_irq_wq;	/* NFI, LVDS, HDMI */
atomic_t hdmi_irq_event = ATOMIC_INIT(0);

static HDMI_UTIL_FUNCS hdmi_util = { 0 };

static volatile unsigned int _HdmiTmrValue[MAX_HDMI_TMR_NUMBER];
bool _fgWifiHdcpErr = FALSE;	/* for hdcp 2.x to 1.x converter */

size_t display_off = 0;
unsigned int u1hdcponoff_bak;
static unsigned char hdmi_first_hdcp = 1;

size_t display_off;
unsigned char hdmi_plug_test_mode;
unsigned char hdmi_is_boot_time;
static unsigned hdmi_port_status_in_boot = 0xff;
unsigned char check_plugin_switch_resolution = 0xff;
struct input_dev *input;

unsigned int pll_read_state;
static unsigned int monitor_count;
static unsigned int pll_read_delay;
static unsigned int pll_read_value;
static unsigned int pll_read_value_min;
static unsigned int pll_read_value_max;
static unsigned int pll_read_value_bak;
static bool pll_read_value_err;
static bool p_stable_fail;
static unsigned int p_stable_fail_pixel_clk;
static unsigned int p_stable_fail_tmds_clk;

static int hdmi_timer_kthread(void *data);
static int cec_timer_kthread(void *data);
static int hdmi_irq_kthread(void *data);
static void vPlugDetectService(HDMI_CTRL_STATE_T e_state);
static irqreturn_t hdmi_irq_handler(int irq, void *dev_id);

const char *szHdmiPordStatusStr[] = {
	"HDMI_PLUG_OUT=0",
	"HDMI_PLUG_IN_AND_SINK_POWER_ON",
	"HDMI_PLUG_IN_ONLY",
	"HDMI_PLUG_IN_EDID",
	"HDMI_PLUG_IN_CEC",
	"HDMI_PLUG_IN_POWER_EDID",
};

const char *szHdmiCecPordStatusStr[] = {
	"HDMI_CEC_STATE_PLUG_OUT=0",
	"HDMI_CEC_STATE_TX_STS",
	"HDMI_CEC_STATE_GET_CMD",
};

/* from DTS, for debug */
unsigned int hdmi_reg_pa_base[HDMI_REG_NUM] = {
	0x14001000,		/*REG_HDMI_RGB2HDMI */
	0x10002000,		/*REG_HDMI_CEC1 */
	0x10002400,		/*REG_HDMI_CEC2 */
	0x14006000,		/*REG_HDMI_DIG */
	0x11702000,		/*REG_HDMI_ANA */
};


/*//unsigned int hdmi_ref_reg_pa_base[HDMI_REF_REG_NUM] ={ */
/*0x10209000, AP_CCIF0/pll */
/*0x10000000, TOPCK_GEN */
/*0x10001000 INFRA_SYS */
/*0x14000000, MMSYS_CONFIG */
/*0x10005000}; GPIO_REG */
/*0x1401d000}; DPI0_REG */
/*0x10206000}; EFUSE_REG */
/*0x10003000}; PERISYS_REG */


unsigned int hdmi_ref_reg_pa_base[HDMI_REF_REG_NUM] = { 0 };

/* Reigster of HDMI , Include HDMI SHELL,DDC,CEC*/
unsigned long hdmi_reg[HDMI_REG_NUM] = { 0 };

/*Registers which will be used by HDMI   */
unsigned long hdmi_ref_reg[HDMI_REF_REG_NUM] = { 0 };

/* clocks that will be used by hdmi module*/
struct clk *hdmi_ref_clock[HDMI_SEL_CLOCK_NUM] = { 0 };

/*Irq of HDMI */
unsigned int hdmi_irq, cec_irq;
/* 5v ddc power control pin*/

int hdmi_filter_switch_gpio;


struct hdmi_internal_device {
	/* base address of HDMI registers */
	void __iomem *regs[HDMI_REG_NUM];
	/** HDMI interrupt */
	unsigned int irq;
	/** pointer to device parent , maybe no need??*/
	struct device *dev;
};

unsigned int resolution_v = HDMI_VIDEO_1920x1080p_50Hz;

struct tag_videolfb {
	u16		lfb_width;
	u16		lfb_height;
	u16		lfb_depth;
	u16		lfb_linelength;
	u32		lfb_base;
	u32		lfb_size;
	u32		lfb_res;
	u32		lfb_colorspace;
	u32		lfb_colordepth;
	u32		lfb_force_dolby;
	u32		lfb_hdr_type;
	u8		red_size;
	u8		red_pos;
	u8		green_size;
	u8		green_pos;
	u8		blue_size;
	u8		blue_pos;
	u8		rsvd_size;
	u8		rsvd_pos;
};

void hdmi_parse_videolfb(struct device *dev)
{
	int offset, l;
	const char *p;
	const void *fdt = initial_boot_params;
	struct tag_videolfb *videolfb_tag = NULL;

	offset = fdt_path_offset(fdt, "/chosen");
	if (offset < 0)
		offset = fdt_path_offset(fdt, "/chosen@0");
	if (offset < 0) {
		TX_DEF_LOG("can not find chosen.\n");
		return;
	}

	p = fdt_getprop(fdt, offset, "atag,videolfb", &l);
	if (!p || !l) {
		TX_DEF_LOG("can not find videolfb.\n");
		return;
	}

	videolfb_tag = (struct tag_videolfb *)(p + 8);
	if (videolfb_tag) {
		hdmi_boot_res = videolfb_tag->lfb_res;
		hdmi_boot_colorspace = videolfb_tag->lfb_colorspace;
		hdmi_boot_colordepth = videolfb_tag->lfb_colordepth;
		hdmi_boot_forcehdr = videolfb_tag->lfb_hdr_type;
	}
	TX_DEF_LOG("[DT]hdmi_boot_res=,%d,%d,%d,%d\n", hdmi_boot_res,
		hdmi_boot_colorspace, hdmi_boot_colordepth, hdmi_boot_forcehdr);
}

const char *hdmi_use_clock_name_spy(HDMI_REF_CLOCK_ENUM module)
{
	switch (module) {
	case MMSYS_HDMI_HDCLK:
		return "hdmi_hdclk";
	case MMSYS_HDMI_SDCLK:
		return "hdmi_sdclk";
	case MMSYS_HDMI_HDCP:
		return "hdmi_hdcp";
	case MMSYS_HDMI_HDCP24:
		return "hdmi_hdcp24";
	case MMSYS_HDMI_RGB2HDMIEN:
		return "hdmi_rgb2hdmien";
	case MMSYS_HDMI_HDMIEN:
		return "hdmi_hdmien";
	case MMSYS_HDMI_SDCKSEL:
		return "hdmi_sdcksel";
	case MMSYS_HDMI_HDTVD:
		return "hdmi_hdtvd";
	case MMSYS_HDMI_SDOSD:
		return "hdmi_sdosd";

	case TOP_HDMI_SEL:
		return "top_hdmi_sel";
	case TOP_HDMISEL_DIG_CTS:
		return "top_hdmisel_dig_cts";
	case TOP_HDMISEL_D2:
		return "top_hdmisel_d2";
	case TOP_HDMISEL_D3:
		return "top_hdmisel_d3";

	case TOP_HDCP_SEL:
		return "top_hdcp_sel";
	case TOP_HDCPSEL_SYS4D2:
		return "top_hdcpsel_sys4d2";
	case TOP_HDCPSEL_SYS3D4:
		return "top_hdcpsel_sys3d4";
	case TOP_HDCPSEL_UNIV2D2:
		return "top_hdcpsel_univ2d2";

	case TOP_HDCP24_SEL:
		return "top_hdcp24_sel";
	case TOP_HDCP24SEL_UNIVPD26:
		return "top_hdcp24sel_univpd26";
	case TOP_HDCP24SEL_UNIVPD52:
		return "top_hdcp24sel_univpd52";
	case TOP_HDCP24SEL_UNIVP2D8:
		return "top_hdcp24sel_univp2d8";

	default:
		return "mediatek,HDMI_UNKNOWN_CLOCK";
	}
}


const char *hdmi_use_module_name_spy(HDMI_REF_MODULE_ENUM module)
{
	switch (module) {
	case AP_CCIF0:
		return "mediatek,AP_CCIF0";	/* TVD//PLL */
	case TOPCK_GEN:
		return "mediatek,TOPCKGEN";
	case INFRA_SYS:
		return "mediatek,INFRACFG_AO";
	case MMSYS_CONFIG:
		return "mediatek,mt8695-mmsys";
	case GPIO_REG:
		return "mediatek,GPIO";
	case DPI0_REG:
		return "mediatek,DPI0";
	case EFUSE_REG:
		return "mediatek,EFUSEC";
	case GIC_REG:
		return "mediatek,MCUCFG";
	case PERISYS_REG:
		return "mediatek,PERICFG";
	case P2I_REG:
		return "mediatek,mt8695-p2i";
	case VDOUT10_REG:
		return "mediatek,mt8695-fmt";
	case VDOUT15_REG:
		return "mediatek,mt8695-vdout";
	case VDOUT1C_REG:
		return "mediatek,mt8695-vdout";

	case HDMI_REF_REG_NUM:
		return "mediatek,HDMI_UNKNOWN";
	default:
		return "mediatek,HDMI_UNKNOWN";
	}
}


static void vInitAvInfoVar(void)
{
	if(hdmi_boot_res >= 0 && hdmi_boot_res < HDMI_VIDEO_RESOLUTION_NUM)
		_stAvdAVInfo.e_resolution = hdmi_boot_res;
	else
		_stAvdAVInfo.e_resolution = HDMI_VIDEO_1920x1080p_60Hz;
	_stAvdAVInfo.fgHdmiOutEnable = TRUE;
	_stAvdAVInfo.fgHdmiTmdsEnable = TRUE;
	_stAvdAVInfo.bMuteHdmiAudio = FALSE;
	if(hdmi_boot_colorspace >= 0 && hdmi_boot_colorspace <= HDMI_YCBCR_420_FULL)
		_stAvdAVInfo.e_video_color_space = hdmi_boot_colorspace;
	else
		_stAvdAVInfo.e_video_color_space = HDMI_RGB;
	if(hdmi_boot_colordepth >= 0 && hdmi_boot_colordepth <= HDMI_DEEP_COLOR_16_BIT)
		_stAvdAVInfo.e_deep_color_bit = hdmi_boot_colordepth;
	else
		_stAvdAVInfo.e_deep_color_bit = HDMI_NO_DEEP_COLOR;
	_stAvdAVInfo.ui1_aud_out_ch_number = 2;
	_stAvdAVInfo.e_hdmi_fs = HDMI_FS_44K;

	_stAvdAVInfo.bhdmiRChstatus[0] = 0x00;
	_stAvdAVInfo.bhdmiRChstatus[1] = 0x00;
	_stAvdAVInfo.bhdmiRChstatus[2] = 0x02;
	_stAvdAVInfo.bhdmiRChstatus[3] = 0x00;
	_stAvdAVInfo.bhdmiRChstatus[4] = 0x00;
	_stAvdAVInfo.bhdmiRChstatus[5] = 0x00;
	_stAvdAVInfo.bhdmiLChstatus[0] = 0x00;
	_stAvdAVInfo.bhdmiLChstatus[1] = 0x00;
	_stAvdAVInfo.bhdmiLChstatus[2] = 0x02;
	_stAvdAVInfo.bhdmiLChstatus[3] = 0x00;
	_stAvdAVInfo.bhdmiLChstatus[4] = 0x00;
	_stAvdAVInfo.bhdmiLChstatus[5] = 0x00;

	hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
	vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);

	if ((hdmi_boot_colorspace == HDMI_YCBCR_444) || (hdmi_boot_colorspace == HDMI_YCBCR_444_FULL)
	    || (hdmi_boot_colorspace == HDMI_YCBCR_420) || (hdmi_boot_colorspace == HDMI_YCBCR_420_FULL)
	    || (hdmi_boot_colorspace == HDMI_XV_YCC)) {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x80001000);
	} else if ((hdmi_boot_colorspace == HDMI_YCBCR_422) || (hdmi_boot_colorspace == HDMI_YCBCR_422_FULL)) {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x8000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x00001000);
	} else {
		vWriteByteHdmiGRL(TOP_VMUTE_CFG1, 0x1000);
		vWriteByteHdmiGRL(TOP_VMUTE_CFG2, 0x10001000);
	}

	u1hdcponoff_bak = 0;
}

void vSetHDMIMdiTimeOut(unsigned int i4_count)
{
	HDMI_DRV_FUNC();
	hdmi_TmrValue[HDMI_PLUG_DETECT_CMD] = i4_count;

}

void hdmi_timing_monitor_start(void)
{
	pll_read_state = 0;
	pll_read_delay = 0;
	monitor_count = 0;
	pll_read_value_err = FALSE;
	p_stable_fail = FALSE;
}

void hdmi_timing_monitor_stop(void)
{
	pll_read_state = 0xff;
}

void hdmi_timing_monitor(void)
{
	unsigned int temp;

	if (pll_read_state == 0xff)
		return;

	if ((dReadIoHdmiAna(HDMI20_CFG_0) & RG_HDMITX20_DRV_EN) != RG_HDMITX20_DRV_EN) {
		hdmi_timing_monitor_stop();
		return;
	}

	if (pll_read_state == 0) {
		/* delay 100ms */
		if (pll_read_delay > 10) {
			pll_read_state = 1;
			hdmi_clock_read(0);
			usleep_range(1000, 1050);
			hdmi_timing_monitor_sel(0);
			hdmi_clock_read(2);
		}
		pll_read_delay++;
	} else if (pll_read_state == 1) {
		pll_read_value = hdmi_clock_read(1);
		pll_read_value_min = pll_read_value - 8;
		pll_read_value_max = pll_read_value + 8;
		pll_read_value_err = FALSE;
		p_stable_fail = FALSE;
		pll_read_state = 2;
		TX_DEF_LOG("pixel base clk:%d/%dkhz, 0x1c0=0x%08x\n",
			pll_read_value,
			((pll_read_value * 52000) / 1024),
			bReadByteHdmiGRL(TOP_STA));
		hdmi_timing_monitor_sel(0);
		hdmi_clock_read(2);
	} else if (pll_read_state == 2) {
		/* pixel clk */
		temp = hdmi_clock_read(1);
		if ((temp < pll_read_value_min) ||
			(temp > pll_read_value_max)) {
			pll_read_value_err = TRUE;
			pll_read_value_bak = temp;
		}

		if ((bReadByteHdmiGRL(TOP_STA) & P_STABLE) == 0) {
			p_stable_fail = TRUE;
			p_stable_fail_pixel_clk = temp;
			hdmi_timing_monitor_sel(1);
			hdmi_clock_read(2);
			usleep_range(1000, 1050);
			p_stable_fail_tmds_clk = hdmi_clock_read(1);
		}

		pll_read_state = 2;
		hdmi_timing_monitor_sel(0);
		hdmi_clock_read(2);

		/* print 3s */
		if (monitor_count > 300) {
			if (p_stable_fail) {
				p_stable_fail = FALSE;
				TX_DEF_LOG("p_stable fail, pix:%d/%dkhz, tmds:%d/%dkhz\n",
					p_stable_fail_pixel_clk,
					((p_stable_fail_pixel_clk * 52000) / 1024),
					p_stable_fail_tmds_clk,
					((p_stable_fail_tmds_clk * 52000) / 1024));
			}

			if (pll_read_value_err) {
				TX_DEF_LOG("pixel clk change, base:%d/%dkhz, err:%d/%dkhz, temp:%d/%dkhz\n",
					pll_read_value,
					((pll_read_value * 52000) / 1024),
					pll_read_value_bak,
					((pll_read_value_bak * 52000) / 1024),
					temp,
					((temp * 52000) / 1024));
				pll_read_value_err = FALSE;
				pll_read_value_bak = 0;
			}

			monitor_count = 0;
		}
	}

	monitor_count++;
}

/*----------------------------------------------------------------------------*/

static void hdmi_set_util_funcs(const HDMI_UTIL_FUNCS *util)
{
	memcpy(&hdmi_util, util, sizeof(HDMI_UTIL_FUNCS));
}

/*----------------------------------------------------------------------------*/

static void hdmi_get_params(HDMI_PARAMS *params)
{
	memset(params, 0, sizeof(HDMI_PARAMS));

	HDMI_DRV_LOG("720p\n");
	params->init_config.vformat = HDMI_VIDEO_1280x720p_50Hz;
	params->init_config.aformat = HDMI_AUDIO_PCM_16bit_48000;

	params->clk_pol = HDMI_POLARITY_FALLING;
	params->de_pol = HDMI_POLARITY_RISING;
	params->vsync_pol = HDMI_POLARITY_RISING;
	params->hsync_pol = HDMI_POLARITY_RISING;

	params->hsync_pulse_width = 40;
	params->hsync_back_porch = 220;
	params->hsync_front_porch = 440;
	params->vsync_pulse_width = 5;
	params->vsync_back_porch = 20;
	params->vsync_front_porch = 5;

	params->rgb_order = HDMI_COLOR_ORDER_RGB;

	params->io_driving_current = IO_DRIVING_CURRENT_2MA;
	params->intermediat_buffer_num = 4;
	params->output_mode = HDMI_OUTPUT_MODE_LCD_MIRROR;
	params->is_force_awake = 1;
	params->is_force_landscape = 1;

	params->scaling_factor = 0;
#ifndef CONFIG_MTK_HDMI_HDCP_SUPPORT
	params->NeedSwHDCP = 1;
#endif
}

static int hdmi_internal_enter(void)
{
	HDMI_DRV_FUNC();
	return 0;

}

static int hdmi_internal_exit(void)
{
	HDMI_DRV_FUNC();
	return 0;
}

/*----------------------------------------------------------------------------*/

static void hdmi_internal_suspend(void)
{
	HDMI_DRV_FUNC();

	/* _stAvdAVInfo.fgHdmiTmdsEnable = 0; */
	/* av_hdmiset(HDMI2_SET_TURN_OFF_TMDS, &_stAvdAVInfo, 1); */
}

/*----------------------------------------------------------------------------*/

static void hdmi_internal_resume(void)
{
	HDMI_DRV_FUNC();


}

/*----------------------------------------------------------------------------*/


void HDMI_DisableIrq(void)
{
	vWriteByteHdmiGRL(TOP_INT_MASK00, 0x00000000);
	vWriteByteHdmiGRL(TOP_INT_MASK01, 0x00000000);
}

void HDMI_EnableIrq(void)
{
	vWriteByteHdmiGRL(TOP_INT_MASK00, 0x02000000);
	vWriteByteHdmiGRL(TOP_INT_MASK01, 0x00000000);
}

void read_hdmi1x_reg(void)
{
	unsigned int i;

	HDMI_PLUG_FUNC();
	for (i = 0; i < 0x9; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14025014 + i * 4,
			      bReadByteHdmiGRL(0x14 + i * 4));
	for (i = 0; i < 0x2; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x140250b8 + i * 4,
			      bReadByteHdmiGRL(0xb8 + i * 4));
	for (i = 0; i < 0xa; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14025154 + i * 4,
			      bReadByteHdmiGRL(0x154 + i * 4));
	for (i = 0; i < 0x4; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x1402518c + i * 4,
			      bReadByteHdmiGRL(0x18c + i * 4));
	for (i = 0; i < 0x1; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x140251c4 + i * 4,
			      bReadByteHdmiGRL(0x1c4 + i * 4));
	for (i = 0; i < 0x2; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14025200 + i * 4,
			      bReadByteHdmiGRL(0x200 + i * 4));
	for (i = 0; i < 0x2; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14025260 + i * 4,
			      bReadByteHdmiGRL(0x260 + i * 4));
}

void read_hdmi2x_reg(void)
{
	unsigned int i;

	HDMI_PLUG_FUNC();
	for (i = 0; i < 0x4; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14006000 + i * 4, bReadByteHdmiGRL(i * 4));
	for (i = 0; i < 0x9; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x140061a0 + i * 4,
			      bReadByteHdmiGRL(0x1a0 + i * 4));
	for (i = 0; i < 0x10; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14006400 + i * 4,
			      bReadByteHdmiGRL(0x400 + i * 4));
	for (i = 0; i < 0x1; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14006c00 + i * 4,
			      bReadByteHdmiGRL(0xc00 + i * 4));
	for (i = 0; i < 0x1; i++)
		HDMI_PLUG_LOG("0x%08x = 0x%08x\n", 0x14006c20 + i * 4,
			      bReadByteHdmiGRL(0xc20 + i * 4));
}

void Check_HdmiPath_Clock(void)
{
	unsigned char i;
	unsigned int temp;

	vWriteHdmiSYS(0x80, 0x0);
	vWriteHdmiSYS(0x78, 0x10000000);

	for (i = 0; i < 5; i++)
		mdelay(2);

	vWriteHdmiSYS(0x80, 0x1);
	for (i = 0; i < 200; i++)
		mdelay(2);

	temp = (dReadHdmiSYS(0x80) >> 16) * 27;
	HDMI_PLUG_LOG("xtal clk = %d.%d\n", temp / 1024, (temp - (temp / 1024) * 1024));

	vWriteHdmiSYS(0x80, 0x0);
	vWriteHdmiSYS(0x78, 0x40070000);
	vWriteHdmiSYS(0x94, 0x00008000);

	for (i = 0; i < 5; i++)
		mdelay(2);

	vWriteHdmiSYS(0x80, 0x1);
	for (i = 0; i < 200; i++)
		mdelay(2);

	temp = (dReadHdmiSYS(0x80) >> 16) * 27;
	HDMI_PLUG_LOG("syspll1 clk = %d.%d\n", temp / 1024, (temp - (temp / 1024) * 1024));

	vWriteHdmiSYS(0x80, 0x0);
	vWriteHdmiSYS(0x78, 0x50000000);
	vWriteHdmiSYS(0x9c, 0x00040000);
	vWriteHdmiSYS(0x154, 0x00000002);

	for (i = 0; i < 5; i++)
		mdelay(2);

	vWriteHdmiSYS(0x80, 0x1);
	for (i = 0; i < 200; i++)
		mdelay(2);

	temp = (dReadHdmiSYS(0x80) >> 16) * 27 * 4;
	HDMI_PLUG_LOG("pll-gp clk = %d.%d\n", temp / 1024, (temp - (temp / 1024) * 1024));

}

int force_video_config(HDMI_VIDEO_RESOLUTION vformat)
{
	HDMI_PLUG_LOG("force vformat = 0x%x\n", vformat);

	_stAvdAVInfo.e_resolution = vformat;

	/* TVD_config_pll(vformat); */
	hdmi_fmtsetting(vformat);
	rgb2hdmi_setting(vformat);

	_stAvdAVInfo.fgHdmiTmdsEnable = 0;
	av_hdmiset(HDMI_SET_TURN_OFF_TMDS, &_stAvdAVInfo, 1);
	av_hdmiset(HDMI_SET_VPLL, &_stAvdAVInfo, 1);
	av_hdmiset(HDMI_SET_SOFT_NCTS, &_stAvdAVInfo, 1);
	av_hdmiset(HDMI_SET_VIDEO_RES_CHG, &_stAvdAVInfo, 1);

	av_hdmiset(HDMI_SET_HDCP_INITIAL_AUTH, &_stAvdAVInfo, 1);
	if (hpd_toggle_videoconfig_flag == HPDTOGGLE_VIDEOCONFIG) {
		hpd_toggle_videoconfig_flag = CLEANTOGGLE_VIDEOCONFIG;
		HDMI_HDCP_LOG("set video_config from hpd\n");
	} else {
		hpd_toggle_videoconfig_flag = SWTICHTOGGLE_VIDEOCONFIG;
		HDMI_HDCP_LOG("set video_config from swtich resolution\n");
	}
	hdmistate_debug = 0;

	return 0;
}

int force_audio_config(void)
{
	HDMI_PLUG_LOG("force audio setting!!\n");
	av_hdmiset(HDMI_SET_AUDIO_CHG_SETTING, &_stAvdAVInfo, 1);

	return 0;
}

int pmx_parameter(HDMI_VIDEO_RESOLUTION vformat, int temp)
{
	HDMI_PLUG_LOG("pmx vformat = 0x%x\n", vformat);

	_stAvdAVInfo.e_resolution = vformat;

	/* TVD_config_pll(vformat); */
	hdmi_fmtsetting(vformat);

	if (temp == 2)
		temprgb2hdmi = 1;
	else if (temp == 3)
		temprgb2hdmi = 2;

	if (temp != 0)
		rgb2hdmi_setting(vformat);
	return 0;
}

static void hdmi_config_ref_clock(unsigned int resolutionmode)
{
	HDMI_DRV_FUNC();

	switch (resolutionmode) {
	case HDMI_VIDEO_720x480p_60Hz:
	case HDMI_VIDEO_720x576p_50Hz:
	case HDMI_VIDEO_720x480i_60Hz:
	case HDMI_VIDEO_720x576i_50Hz:
		vWriteHdmiSYSMsk(0x228, 1 << 16, 1 << 16);
		break;

	default:
		vWriteHdmiSYSMsk(0x228, 0 << 16, 1 << 16);
		break;
	}
}

void hdmitx_confighdmisetting(unsigned int resolutionmode)
{

	HDMI_DRV_FUNC();

	vWriteIoHdmiAnaMsk(HDMI_ANA_CTL, REG_ANA_HDMI20_FIFO_EN, REG_ANA_HDMI20_FIFO_EN);
	/* output from analog20 port */

	if (((resolutionmode == HDMI_VIDEO_3840x2160P_50HZ)
	     || (resolutionmode == HDMI_VIDEO_3840x2160P_60HZ)
	     || (resolutionmode == HDMI_VIDEO_3840x2160P_59_94HZ)
	     || (resolutionmode == HDMI_VIDEO_4096x2160P_59_94HZ))
	    && !(_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)) {
		HDMI_PLUG_LOG("hdmitx_configsetting Over 3G\n");
		vWriteIoHdmiAnaMsk(HDMI20_CLK_CFG, (0x2 << REG_TXC_DIV_SHIFT), REG_TXC_DIV);
	} else {
		HDMI_PLUG_LOG("hdmitx_configsetting Under 3G\n");
		vWriteIoHdmiAnaMsk(HDMI20_CLK_CFG, 0, REG_TXC_DIV);
	}

	if (resolutionmode >= HDMI_VIDEO_3840x2160P_23_976HZ) {
		pr_info("%s: Set hdmi filter HIGH\n", __func__);
		gpio_direction_output(hdmi_filter_switch_gpio, 1);
	} else {
		pr_info("%s: Set hdmi filter LOW\n", __func__);
		gpio_direction_output(hdmi_filter_switch_gpio, 0);
	}
}



int hdmi_internal_video_config(HDMI_VIDEO_RESOLUTION vformat)
{
	HDMI_DRV_FUNC();

	TX_DEF_LOG("%s, format=0x%x\n", __func__, vformat);

	if (hdcp2_version_flag == TRUE) {
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa0002);
#endif
	} else {
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		vCaHDMIWriteHdcpCtrl(0x88880000, 0xaaaa0001);
#endif
	}

	if (vformat <= HDMI_VIDEO_720x576p_50Hz) {
		if (hdmi_sdcksel == 0) {
			HDMI_DRV_LOG("[hdmi]hdmi SD clock select\n");
			hdmi_sdck_select(TRUE);
		}
		hdmi_sdcksel = 1;
	} else {
		if (hdmi_sdcksel_first == 0) {
			HDMI_DRV_LOG("[hdmi]hdmi HD clock select\n");
			hdmi_sdck_select(FALSE);
			hdmi_sdcksel_first = 1;
		} else if (hdmi_sdcksel == 1) {
			HDMI_DRV_LOG("[hdmi]hdmi HD clock select\n");
			hdmi_sdck_select(FALSE);
		}
		hdmi_sdcksel = 0;
	}

	_stAvdAVInfo.e_resolution = vformat;
	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
		vdout_sys_422_to_420(0x1);
	else
		vdout_sys_422_to_420(0x0);
	hdmi_config_ref_clock(vformat);	/* need vdout to setting 480P/576P NG */
	hdmitx_confighdmisetting(vformat);

	av_hdmiset(HDMI_SET_VPLL, &_stAvdAVInfo, 1);
	av_hdmiset(HDMI_SET_VIDEO_RES_CHG, &_stAvdAVInfo, 1);
	av_hdmiset(HDMI_SET_AUDIO_CHG_SETTING, &_stAvdAVInfo, 1);

	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420) {
		HDMI_DRV_LOG("e_video_color_space =%d-------\n", _stAvdAVInfo.e_video_color_space);
		vWriteByteHdmiGRL(VID_DOWNSAMPLE_CONFIG, 0x00000351);
		vWriteByteHdmiGRL(VID_OUT_FORMAT, 0x00000400);
		vWriteRGB2HDMI(HDMI_RGB_CTRL, 0xd41);
		if (_stAvdAVInfo.e_deep_color_bit == 1)
			vWriteIoHdmiAnaMsk(HDMI20_CLK_CFG, 0, REG_TXC_DIV);
	}

	HDMI_DRV_LOG("AVIINFO Res = 0x%x; Color = %d\n", _stAvdAVInfo.e_resolution,
		      _stAvdAVInfo.e_video_color_space);
	vSendAVIInfoFrame(_stAvdAVInfo.e_resolution, _stAvdAVInfo.e_video_color_space);
	vSendVendorSpecificInfoFrame(_stAvdAVInfo.e_resolution);
	vTrySendDolbyVSIF(_stAvdAVInfo.e_resolution);
	vSendStaticHdrInfoFrame();

	HDMI_DRV_LOG("e_deep_color_bit=%d-------\n", _stAvdAVInfo.e_deep_color_bit);

	msleep(100);
	vTxSignalOnOff(SV_ON);
	av_hdmiset(HDMI_SET_HDCP_INITIAL_AUTH, &_stAvdAVInfo, 1);

	HDMI_DRV_LOG("end-------\n");

	hdmi_timing_monitor_start();
	hdmi_hdcp22_monitor_init();

	return 0;
}
EXPORT_SYMBOL(hdmi_internal_video_config);

int hdmi_audiosetting(HDMITX_AUDIO_PARA *audio_para)
{
	static HDMITX_AUDIO_PARA last_audio_para;
	int skip_audio_setting = 0;
	HDMI_DRV_FUNC();

	_stAvdAVInfo.e_hdmi_aud_in = audio_para->e_hdmi_aud_in;	/* SV_I2S; */
	_stAvdAVInfo.e_iec_frame = audio_para->e_iec_frame;	/* IEC_48K; */
	_stAvdAVInfo.e_hdmi_fs = audio_para->e_hdmi_fs;	/* HDMI_FS_48K; */
	_stAvdAVInfo.e_aud_code = audio_para->e_aud_code;	/* AVD_LPCM; */
	_stAvdAVInfo.u1Aud_Input_Chan_Cnt = audio_para->u1Aud_Input_Chan_Cnt;	/* AUD_INPUT_2_0; */
	_stAvdAVInfo.e_I2sFmt = audio_para->e_I2sFmt;	/* HDMI_I2S_24BIT; */
	_stAvdAVInfo.u1HdmiI2sMclk = audio_para->u1HdmiI2sMclk;	/* MCLK_128FS; */
	_stAvdAVInfo.bhdmiLChstatus[0] = audio_para->bhdmi_LCh_status[0];
	_stAvdAVInfo.bhdmiLChstatus[1] = audio_para->bhdmi_LCh_status[1];
	_stAvdAVInfo.bhdmiLChstatus[2] = audio_para->bhdmi_LCh_status[2];
	_stAvdAVInfo.bhdmiLChstatus[3] = audio_para->bhdmi_LCh_status[3];
	_stAvdAVInfo.bhdmiLChstatus[4] = audio_para->bhdmi_LCh_status[4];
	_stAvdAVInfo.bhdmiRChstatus[0] = audio_para->bhdmi_RCh_status[0];
	_stAvdAVInfo.bhdmiRChstatus[1] = audio_para->bhdmi_RCh_status[1];
	_stAvdAVInfo.bhdmiRChstatus[2] = audio_para->bhdmi_RCh_status[2];
	_stAvdAVInfo.bhdmiRChstatus[3] = audio_para->bhdmi_RCh_status[3];
	_stAvdAVInfo.bhdmiRChstatus[4] = audio_para->bhdmi_RCh_status[4];
	/* back & compare start */
	if ((_stAvdAVInfo.e_hdmi_aud_in == last_audio_para.e_hdmi_aud_in) &&
		(_stAvdAVInfo.e_iec_frame == last_audio_para.e_iec_frame) &&
		(_stAvdAVInfo.e_hdmi_fs ==  last_audio_para.e_hdmi_fs) &&
		(_stAvdAVInfo.e_aud_code == last_audio_para.e_aud_code) &&
		(_stAvdAVInfo.u1Aud_Input_Chan_Cnt == last_audio_para.u1Aud_Input_Chan_Cnt) &&
		(_stAvdAVInfo.e_I2sFmt == last_audio_para.e_I2sFmt) &&
		(_stAvdAVInfo.u1HdmiI2sMclk == last_audio_para.u1HdmiI2sMclk) &&
		(_stAvdAVInfo.bhdmiLChstatus[0] == last_audio_para.bhdmi_LCh_status[0]) &&
		(_stAvdAVInfo.bhdmiLChstatus[1] == last_audio_para.bhdmi_LCh_status[1]) &&
		(_stAvdAVInfo.bhdmiLChstatus[2] == last_audio_para.bhdmi_LCh_status[2]) &&
		(_stAvdAVInfo.bhdmiLChstatus[3] == last_audio_para.bhdmi_LCh_status[3]) &&
		(_stAvdAVInfo.bhdmiLChstatus[4] == last_audio_para.bhdmi_LCh_status[4]) &&
		(_stAvdAVInfo.bhdmiRChstatus[0] == last_audio_para.bhdmi_RCh_status[0]) &&
		(_stAvdAVInfo.bhdmiRChstatus[1] == last_audio_para.bhdmi_RCh_status[1]) &&
		(_stAvdAVInfo.bhdmiRChstatus[2] == last_audio_para.bhdmi_RCh_status[2]) &&
		(_stAvdAVInfo.bhdmiRChstatus[3] == last_audio_para.bhdmi_RCh_status[3]) &&
		(_stAvdAVInfo.bhdmiRChstatus[4] == last_audio_para.bhdmi_RCh_status[4])) {
		skip_audio_setting = 1;
	} else {
		skip_audio_setting = 0;
		last_audio_para.e_hdmi_aud_in = audio_para->e_hdmi_aud_in;	/* SV_I2S; */
		last_audio_para.e_iec_frame = audio_para->e_iec_frame;	/* IEC_48K; */
		last_audio_para.e_hdmi_fs = audio_para->e_hdmi_fs;	/* HDMI_FS_48K; */
		last_audio_para.e_aud_code = audio_para->e_aud_code;	/* AVD_LPCM; */
		last_audio_para.u1Aud_Input_Chan_Cnt = audio_para->u1Aud_Input_Chan_Cnt;	/* AUD_INPUT_2_0; */
		last_audio_para.e_I2sFmt = audio_para->e_I2sFmt;	/* HDMI_I2S_24BIT; */
		last_audio_para.u1HdmiI2sMclk = audio_para->u1HdmiI2sMclk;	/* MCLK_128FS; */
		last_audio_para.bhdmi_LCh_status[0] = audio_para->bhdmi_LCh_status[0];
		last_audio_para.bhdmi_LCh_status[1] = audio_para->bhdmi_LCh_status[1];
		last_audio_para.bhdmi_LCh_status[2] = audio_para->bhdmi_LCh_status[2];
		last_audio_para.bhdmi_LCh_status[3] = audio_para->bhdmi_LCh_status[3];
		last_audio_para.bhdmi_LCh_status[4] = audio_para->bhdmi_LCh_status[4];
		last_audio_para.bhdmi_RCh_status[0] = audio_para->bhdmi_RCh_status[0];
		last_audio_para.bhdmi_RCh_status[1] = audio_para->bhdmi_RCh_status[1];
		last_audio_para.bhdmi_RCh_status[2] = audio_para->bhdmi_RCh_status[2];
		last_audio_para.bhdmi_RCh_status[3] = audio_para->bhdmi_RCh_status[3];
		last_audio_para.bhdmi_RCh_status[4] = audio_para->bhdmi_RCh_status[4];
	}

	/* back & compare end */

	if (skip_audio_setting == 0) {
		TX_DEF_LOG("hdmi apply new audio setting\n");

		av_hdmiset(HDMI_SET_AUDIO_CHG_SETTING, &_stAvdAVInfo, 1);
		TX_DEF_LOG("e_hdmi_aud_in=%d,e_iec_frame=%d,e_hdmi_fs=%d\n", _stAvdAVInfo.e_hdmi_aud_in,
		     _stAvdAVInfo.e_iec_frame, _stAvdAVInfo.e_hdmi_fs);
		TX_DEF_LOG("e_aud_code=%d,u1Aud_Input_Chan_Cnt=%d,e_I2sFmt=%d\n", _stAvdAVInfo.e_aud_code,
		     _stAvdAVInfo.u1Aud_Input_Chan_Cnt, _stAvdAVInfo.e_I2sFmt);
		TX_DEF_LOG("u1HdmiI2sMclk=%d\n", _stAvdAVInfo.u1HdmiI2sMclk);

		TX_DEF_LOG("bhdmiLChstatus0=%d\n", _stAvdAVInfo.bhdmiLChstatus[0]);
		TX_DEF_LOG("bhdmiLChstatus1=%d\n", _stAvdAVInfo.bhdmiLChstatus[1]);
		TX_DEF_LOG("bhdmiLChstatus2=%d\n", _stAvdAVInfo.bhdmiLChstatus[2]);
		TX_DEF_LOG("bhdmiLChstatus3=%d\n", _stAvdAVInfo.bhdmiLChstatus[3]);
		TX_DEF_LOG("bhdmiLChstatus4=%d\n", _stAvdAVInfo.bhdmiLChstatus[4]);
		TX_DEF_LOG("bhdmiRChstatus0=%d\n", _stAvdAVInfo.bhdmiRChstatus[0]);
		TX_DEF_LOG("bhdmiRChstatus1=%d\n", _stAvdAVInfo.bhdmiRChstatus[1]);
		TX_DEF_LOG("bhdmiRChstatus2=%d\n", _stAvdAVInfo.bhdmiRChstatus[2]);
		TX_DEF_LOG("bhdmiRChstatus3=%d\n", _stAvdAVInfo.bhdmiRChstatus[3]);
		TX_DEF_LOG("bhdmiRChstatus4=%d\n", _stAvdAVInfo.bhdmiRChstatus[4]);
	} else {
		TX_DEF_LOG("hdmi skip hdmi audio setting\n");
	}

	return 0;
}

int hdmi_tmdsonoff(unsigned char u1ionoff)
{
	HDMI_DRV_FUNC();

	_stAvdAVInfo.fgHdmiTmdsEnable = u1ionoff;
	av_hdmiset(HDMI_SET_TURN_OFF_TMDS, &_stAvdAVInfo, 1);

	return 0;
}

/*----------------------------------------------------------------------------*/

int hdmi_internal_audio_config(HDMI_AUDIO_FORMAT aformat)
{
	unsigned int bData = 0;

	HDMI_DRV_FUNC();

	HDMI_PLUG_LOG("Aformat = 0x%x\n", aformat);

	vWriteByteHdmiGRL(AIP_CTRL, 0x0001031A);	/* 0x400 */
	vWriteByteHdmiGRL(AIP_N_VAL, 0x000016c0);
	vWriteByteHdmiGRL(AIP_CTS_SVAL, 0x00022551);
	vWriteByteHdmiGRL(AIP_SPDIF_CTRL, 0x44290400);
	vWriteByteHdmiGRL(AIP_I2S_CTRL, 0x000b69e4);	/* 0x410 */
	vWriteByteHdmiGRL(AIP_I2S_CHST0, 0x02108900);
	vWriteByteHdmiGRL(AIP_I2S_CHST1, 0x000000db);
	vWriteByteHdmiGRL(AIP_DOWNSAMPLE_CTRL, 0x00000000);
	vWriteByteHdmiGRL(AIP_PAR_CTRL, 0x00000000);	/* 0x420 */
	vWriteByteHdmiGRL(AIP_TXCTRL, 0x00000000);
	vWriteByteHdmiGRL(AIP_TPI_CTRL, 0x00000000);
	vWriteByteHdmiGRL(AIP_INT_CTRL, 0xffff0000);
	vWriteByteHdmiGRL(AIP_STA00, 0x000016c0);	/* 0x430 */
	vWriteByteHdmiGRL(AIP_STA01, 0x00d2254F);
	vWriteByteHdmiGRL(AIP_STA02, 0x00020402);
	vWriteByteHdmiGRL(AIP_STA03, 0x000000fc);

	vHalSendAudioInfoFrame(0x01, 0x00, 0x00, 0x00);

	bData = bReadByteHdmiGRL(AIP_CTRL);
	bData = bReadByteHdmiGRL(AIP_N_VAL);
	bData = bReadByteHdmiGRL(AIP_CTS_SVAL);
	bData = bReadByteHdmiGRL(AIP_SPDIF_CTRL);
	bData = bReadByteHdmiGRL(AIP_I2S_CTRL);
	bData = bReadByteHdmiGRL(AIP_I2S_CHST0);
	bData = bReadByteHdmiGRL(AIP_I2S_CHST1);
	bData = bReadByteHdmiGRL(AIP_DOWNSAMPLE_CTRL);
	bData = bReadByteHdmiGRL(AIP_PAR_CTRL);
	bData = bReadByteHdmiGRL(AIP_TXCTRL);
	bData = bReadByteHdmiGRL(AIP_TPI_CTRL);
	bData = bReadByteHdmiGRL(AIP_INT_CTRL);
	bData = bReadByteHdmiGRL(AIP_STA00);
	bData = bReadByteHdmiGRL(AIP_STA01);
	bData = bReadByteHdmiGRL(AIP_STA02);
	bData = bReadByteHdmiGRL(AIP_STA03);

	bData = bReadByteHdmiGRL(TOP_INFO_EN);
	bData = bReadByteHdmiGRL(TOP_INFO_RPT);
	bData = bReadByteHdmiGRL(TOP_AIF_HEADER);
	bData = bReadByteHdmiGRL(TOP_AIF_PKT00);
	bData = bReadByteHdmiGRL(TOP_AIF_PKT01);
	bData = bReadByteHdmiGRL(TOP_AIF_PKT02);
	bData = bReadByteHdmiGRL(TOP_AIF_PKT03);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int hdmi_internal_video_enable(unsigned char enable)
{
	HDMI_DRV_FUNC();

	return 0;
}

/*----------------------------------------------------------------------------*/

static int hdmi_internal_audio_enable(unsigned char enable)
{
	HDMI_DRV_FUNC();

	return 0;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

void hdmi_internal_set_mode(unsigned char ucMode)
{
	HDMI_DRV_FUNC();

}

int hdmi_internal_power_on(void)
{
	struct pinctrl *p;
	struct pinctrl_state *s;
	int ret = 0;

	HDMI_PLUG_FUNC();

	HDMI_DRV_LOG("[hdmi]hdmi_internal_power_on\n");

	hdmi_is_boot_time = 0;


	if (hdmi_powerenable == 1)
		return 0;
	hdmi_powerenable = 1;
	hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;

	if (hdmi_hdmi_on == 0) {
		ret = pm_runtime_get_sync(&hdmi_pdev->dev);
		TX_DEF_LOG("[pm]Power Domain On %d\n", ret);

		HDMI_DRV_LOG("[clock]hdmitx Power On\n");
		hdmi_hdmi_on = 1;
		clk_prepare(hdmi_ref_clock[MMSYS_HDMI_HDMIEN]);
		clk_enable(hdmi_ref_clock[MMSYS_HDMI_HDMIEN]);
	}

	port_hpd_value_bak = 0xff;

	memset((void *)&r_hdmi_timer, 0, sizeof(r_hdmi_timer));
	r_hdmi_timer.expires = jiffies + 1000 / (1000 / HZ);	/* wait 1s to stable */
	r_hdmi_timer.function = hdmi_poll_isr;
	r_hdmi_timer.data = 0;
	init_timer(&r_hdmi_timer);
	add_timer(&r_hdmi_timer);

	if (hdmi_5vpower == 0) {
		TX_DEF_LOG("Enable 5v power high\n");
		p = devm_pinctrl_get(&hdmi_pdev->dev);
		if (IS_ERR(p)) {
			TX_DEF_LOG("devm_pinctrl_get 5vhigh fail!!\n");
		}
		s = pinctrl_lookup_state(p, "5vhigh");
		if (IS_ERR(s)) {
			TX_DEF_LOG("pinctrl_lookup_state 5vhigh fail!!\n");
		}
		ret = pinctrl_select_state(p, s);
		if (ret < 0) {
			TX_DEF_LOG("pinctrl_select_state 5vhigh fail!!\n");
		}
		hdmi_5vpower = 1;
	}
	if (request_irq(hdmi_irq, hdmi_irq_handler, IRQF_TRIGGER_HIGH, "hdmiirq", NULL) < 0)
		TX_DEF_LOG("request hdmi interrupt failed.\n");
	else
		TX_DEF_LOG("request hdmi interrupt success\n");

	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);

	return 0;
}

/*----------------------------------------------------------------------------*/
void vReadHdcpVersion(void)
{
	unsigned char bTemp;

	if (!fgDDCDataRead(RX_ID, RX_REG_HDCP2VERSION, 1, &bTemp)) {
		TX_DEF_LOG("read hdcp version fail from sink\n");
		hdcp2_version_flag = FALSE;
		hdcp_current_level = NO_HDCP;
		hdcp_max_level = NO_HDCP;
	} else if (bTemp & 0x4) {
		hdcp2_version_flag = TRUE;
		hdcp_current_level = HDCP_V2_2;
		hdcp_max_level = HDCP_V2_2;
		HDMI_PLUG_LOG("sink support hdcp2.2 version\n");
	} else {
		hdcp2_version_flag = FALSE;
		hdcp_current_level = HDCP_V1_4;
		hdcp_max_level = HDCP_V1_4;
		HDMI_PLUG_LOG("sink support hdcp1.x version\n");
	}
}

void hdmi_internal_power_off(void)
{
	struct pinctrl *p;
	struct pinctrl_state *s;
	int ret = 0;

	HDMI_PLUG_FUNC();

	HDMI_DRV_LOG("[hdmi]hdmi_internal_power_off\n");

	hdmi_is_boot_time = 0;

	if (hdmi_powerenable == 0)
		return;

	hdmi_powerenable = 0;

	hdcp2_version_flag = FALSE;

	_stAvdAVInfo.fgHdmiTmdsEnable = 0;
	_stAvdAVInfo.e_resolution = HDMI_VIDEO_RESOLUTION_NUM;
	av_hdmiset(HDMI_SET_TURN_OFF_TMDS, &_stAvdAVInfo, 1);

	hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
	vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);

	free_irq(hdmi_irq, NULL);
	TX_DEF_LOG("Free hdmi interrupt\n");

	TX_DEF_LOG("hdmi_clockenable = %d\n", hdmi_clockenable);
	TX_DEF_LOG("hdmi_hdtvd_first = %d\n", hdmi_hdtvd_first);
	TX_DEF_LOG("hdmi_sdosd_first = %d\n", hdmi_sdosd_first);
	/*Use CCF APIs to disable clocsk */
	if (hdmi_clockenable == 1) {
		TX_DEF_LOG("[clock]rgb2hdmi Power Off\n");
		hdmi_clockenable = 0;
		hdmi_clock_enable(false);
	}

	if (hdmi_hdtvd_first != 0) {
	clk_disable(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
	clk_unprepare(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
	TX_DEF_LOG("disable HD clock MMSYS_HDMI_HDTVD\n");
	hdmi_hdtvd_first = 0;
	}
	if (hdmi_sdosd_first != 0) {
	clk_disable(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
	clk_unprepare(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
	TX_DEF_LOG("disable SD clock MMSYS_HDMI_SDOSD\n");
	hdmi_sdosd_first = 0;
	}

	if (r_hdmi_timer.function)
		del_timer_sync(&r_hdmi_timer);

	memset((void *)&r_hdmi_timer, 0, sizeof(r_hdmi_timer));
	/*vCec_poweron_32k(); */
	hdmistate_debug = 0xfff;

	if (hdmi_5vpower == 1) {
		TX_DEF_LOG("disable 5v power low\n");
		p = devm_pinctrl_get(&hdmi_pdev->dev);
		if (IS_ERR(p)) {
			TX_DEF_LOG("devm_pinctrl_get 5vlow fail!!\n");
		}

		s = pinctrl_lookup_state(p, "5vlow");
		if (IS_ERR(s)) {
			TX_DEF_LOG("pinctrl_lookup_state 5vlow fail!!\n");
		}

		ret = pinctrl_select_state(p, s);
		if (ret < 0) {
			TX_DEF_LOG("pinctrl_select_state 5vlow fail!!\n");
		}
		hdmi_5vpower = 0;
	}

	mdelay(500);
	if (hdmi_hdmi_on == 1) {
		TX_DEF_LOG("[clock]hdmitx clock Power Off\n");
		hdmi_hdmi_on = 0;
		clk_disable(hdmi_ref_clock[MMSYS_HDMI_HDMIEN]);
		clk_unprepare(hdmi_ref_clock[MMSYS_HDMI_HDMIEN]);

		pm_runtime_put_sync(&hdmi_pdev->dev);
		TX_DEF_LOG("[pm]Power Domain Off\n");
	}
}

/*----------------------------------------------------------------------------*/

void hdmi_internal_dump(void)
{
	HDMI_DRV_FUNC();
	/* hdmi_dump_reg(); */
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

HDMI_STATE hdmi_get_state(void)
{
	HDMI_DRV_FUNC();

	if (bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE) == TRUE)
		return HDMI_STATE_ACTIVE;
	else if (bCheckPordHotPlug(HOTPLUG_MODE) == TRUE)
		return HDMI_STATE_PLUGIN_ONLY;
	else
		return HDMI_STATE_NO_DEVICE;
}

void hdmi_enablehdcp(unsigned char u1hdcponoff)
{
	if (hdmi_first_hdcp == 1) {
		if (_bHdcpOff == 1)
		return;

		hdcp_unmute_logo();
#if  (defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) && defined(CONFIG_MTK_HDMI_HDCP_SUPPORT))
		if (bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE) == TRUE) {
			vReadHdcpVersion();
			TX_DEF_LOG("boot hdcp, ver = %d\n", hdcp2_version_flag);
			vHDCPInitAuth();
			hdmi_hdcp22_monitor_init();
		}
#endif
		hdmi_first_hdcp = 0;
	}
}

void hdmi_setcecrxmode(unsigned char u1cecrxmode)
{
	HDMI_DRV_FUNC();
	hdmi_rxcecmode = u1cecrxmode;
}

void hdmi_colordeep(unsigned char u1colorspace, unsigned char u1deepcolor)
{
	HDMI_DRV_FUNC();

	if ((u1colorspace == 0xff) && (u1deepcolor == 0xff)) {
		TX_DEF_LOG("color_space:HDMI_RGB = 0\n");
		TX_DEF_LOG("color_space:HDMI_YCBCR_444 = 2\n");
		TX_DEF_LOG("color_space:HDMI_YCBCR_422 = 3\n");
		TX_DEF_LOG("color_space:HDMI_YCBCR_420 = 4\n");

		TX_DEF_LOG("deep_color:HDMI_NO_DEEP_COLOR = 1\n");
		TX_DEF_LOG("deep_color:HDMI_DEEP_COLOR_10_BIT = 2\n");
		TX_DEF_LOG("deep_color:HDMI_DEEP_COLOR_12_BIT = 3\n");
		TX_DEF_LOG("deep_color:HDMI_DEEP_COLOR_16_BIT = 4\n");

		return;
	}

	TX_DEF_LOG("hdmi_colordeep:u1colorspace = %d; u1deepcolor = %d\n", u1colorspace, u1deepcolor);

	_stAvdAVInfo.e_video_color_space = u1colorspace;
	_stAvdAVInfo.e_deep_color_bit = (HDMI_DEEP_COLOR_T) u1deepcolor;

	if ((u1colorspace == hdmi_boot_colorspace) && (u1deepcolor == hdmi_boot_colordepth)) {
		TX_DEF_LOG("LK & Kernel same colorspcace and deepcolor: %d;  %d\n",
			hdmi_boot_colorspace, hdmi_boot_colordepth);
		return;
	}
/*
	if (_stAvdAVInfo.e_video_color_space == HDMI_YCBCR_420)
		vdout_sys_422_to_420(0x1);
	else
		vdout_sys_422_to_420(0x0);
*/
}

void hdmi_read(unsigned int u2Reg, unsigned int *p4Data)
{
	switch (u2Reg & 0xffffff00) {
	case 0x14006000:
		internal_hdmi_read(hdmi_reg[REG_HDMI_DIG] + u2Reg - 0x14006000, p4Data);
		break;

	case 0x10002000:
		internal_hdmi_read(hdmi_reg[REG_HDMI_CEC1] + u2Reg - 0x10002000, p4Data);
		break;

	case 0x10002400:
		internal_hdmi_read(hdmi_reg[REG_HDMI_CEC2] + u2Reg - 0x10002400, p4Data);
		break;

	case 0x10000000:
		internal_hdmi_read(hdmi_ref_reg[MMSYS_CONFIG] + u2Reg - 0x10000000, p4Data);
		break;

	case 0x14001800:
		internal_hdmi_read(hdmi_reg[REG_HDMI_RGB2HDMI] + u2Reg - 0x14001800, p4Data);
		break;

	case 0x11702000:
		internal_hdmi_read(hdmi_reg[REG_HDMI_ANA] + u2Reg - 0x11702000, p4Data);
		break;

	default:
		TX_DEF_LOG("hdmi_read fail\n");
		break;
	}

}

void vWriteReg(unsigned int u2Reg, unsigned int u4Data)
{
	switch (u2Reg & 0xffffff00) {
	case 0x10000000:
		internal_hdmi_write(hdmi_ref_reg[MMSYS_CONFIG] + u2Reg - 0x10000000, u4Data);
		break;

	case 0x14001800:
		internal_hdmi_write(hdmi_reg[REG_HDMI_RGB2HDMI] + u2Reg - 0x14001800, u4Data);
		break;

	case 0x11702000:
		internal_hdmi_write(hdmi_reg[REG_HDMI_ANA] + u2Reg - 0x11702000, u4Data);
		break;

	case 0x14006000:
		internal_hdmi_write(hdmi_reg[REG_HDMI_DIG] + u2Reg - 0x14006000, u4Data);
		break;

	case 0x14006c00:
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
		HDMI_DRV_LOG("CONFIG_MTK_IN_HOUSE_TEE_SUPPORT\n");
		vCaHDMIWriteReg(u2Reg - 0x14006c00, u4Data);
#else
		internal_hdmi_write(hdmi_reg[REG_HDMI_DIG] + u2Reg - 0x14006c00, u4Data);
#endif
		break;

	case 0x10002000:
		internal_hdmi_write(hdmi_reg[REG_HDMI_CEC1] + u2Reg - 0x10002000, u4Data);
		break;

	case 0x10002400:
		internal_hdmi_write(hdmi_reg[REG_HDMI_CEC2] + u2Reg - 0x10002400, u4Data);
		break;

	default:
		TX_DEF_LOG("vWriteReg fail\n");
		break;
	}

}

void dbg_wakeup_hdmi_irqwq(void)
{
	HDMI_DRV_FUNC();

	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);
}

void vShowIrqRegister(void)
{
	unsigned char i;

	for (i = 0; i < deeplength; i++) {
		TX_DEF_LOG("irq counter = %d\n", i);
		TX_DEF_LOG("0x221A8 = 0x%08x\n", hdmisave_irq_reg[i][0]);
		TX_DEF_LOG("0x221AC = 0x%08x\n", hdmisave_irq_reg[i][1]);
		TX_DEF_LOG("0x221B0 = 0x%08x\n", hdmisave_irq_reg[i][2]);
		TX_DEF_LOG("0x221B4 = 0x%08x\n", hdmisave_irq_reg[i][3]);
		TX_DEF_LOG("0x221B8 = 0x%08x\n", hdmisave_irq_reg[i][4]);
		TX_DEF_LOG("0x221BC = 0x%08x\n", hdmisave_irq_reg[i][5]);
		TX_DEF_LOG("port_hpd_value = 0x%08x\n", hdmisave_irq_reg[i][6]);
	}
	deeplength = 0;
}

void hdmi_write(unsigned int u2Reg, unsigned int u4Data)
{
	u2Reg = 0x0;
	u4Data = 0x0;
}

void read_int_status(unsigned char savevalue)
{
	if (deeplength > (SAVELENGTH - 1)) {
		deeplength = 0;
		return;
	}

	hdmisave_irq_reg[deeplength][0] = bReadByteHdmiGRL(TOP_INT_STA00);
	hdmisave_irq_reg[deeplength][1] = bReadByteHdmiGRL(TOP_INT_STA01);
	hdmisave_irq_reg[deeplength][2] = bReadByteHdmiGRL(TOP_INT_MASK00);
	hdmisave_irq_reg[deeplength][3] = bReadByteHdmiGRL(TOP_INT_MASK01);
	hdmisave_irq_reg[deeplength][4] = bReadByteHdmiGRL(TOP_INT_CLR00);
	hdmisave_irq_reg[deeplength][5] = bReadByteHdmiGRL(TOP_INT_CLR01);
	hdmisave_irq_reg[deeplength][6] = (savevalue << 16) + port_hpd_value_bak;
	deeplength++;

}

void vClear_pordhpd_irq(void)
{
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0xf);
	udelay(1);
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x0);
}

static irqreturn_t hdmi_irq_handler(int irq, void *dev_id)
{
	unsigned char port_hpd_value;
	unsigned int bStatus;

	bStatus = bReadByteHdmiGRL(TOP_INT_STA00);
	/* 0x0200000f is the same as  TOP_INT_MASK00 */
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x0200000f);
	vWriteByteHdmiGRL(TOP_INT_CLR00, 0x00000000);

	if (bStatus & 0x02000000) {
		if ((hdmi_hotplugstate == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)
	    && (e_hdcp_ctrl_state != HDCP2x_AUTHENTICATION)) {
			if (bStatus & HDCP2X_RX_REAUTH_REQ_DDCM_INT_STA) {
				vSetHDCPState(HDCP2x_AUTHENTICATION);
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
				atomic_set(&hdmi_timer_event, 1);
				wake_up_interruptible(&hdmi_timer_wq);
				TX_DEF_LOG("reauth_req\n");
			}
		}
	}

	if (bStatus & 0x0000000f) {
	port_hpd_value = hdmi_get_port_hpd_value();
	read_int_status(port_hpd_value);
	TX_DEF_LOG("port_hpd_value = %d, No: %d\n", port_hpd_value, deeplength);

		/* mark hpd and port irq */
		vWriteHdmiGRLMsk(TOP_INT_MASK00, 0x00000000, 0x0000000f);

	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);
	}

	return IRQ_HANDLED;
}


extern bool hdmi_suspend_en;


static int hdmi_event_notifier_callback(struct notifier_block *self,
					unsigned long action, void *data)
{
	struct fb_event *event = data;
	int blank_mode;

	if ((event == NULL) || (event->data == NULL))
		return 0;

	if (action != FB_EARLY_EVENT_BLANK)
		return 0;

	blank_mode = *(int *)event->data;
#if 1
	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
	case FB_BLANK_NORMAL:
		printk("hdmi_late_resume \n");
		display_off = 0;
		/*hdmi_internal_power_on();*/
		if(hdmi_suspend_en)
			hdmi_internal_power_on();
		break;
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
		break;
	case FB_BLANK_POWERDOWN:
		printk("hdmi_early_suspend %d\n", hdmi_suspend_en);
		display_off = 1;
		/*hdmi_internal_power_off();*/
		if(hdmi_suspend_en) {
			hdmi_internal_power_off();
		}
		break;
	default:
		return -EINVAL;
	}
#endif
	return 0;
}

static struct notifier_block hdmi_event_notifier = {
	.notifier_call = hdmi_event_notifier_callback,
};


static int hdmi_internal_init(void)
{
	HDMI_DRV_FUNC();

	init_waitqueue_head(&hdmi_timer_wq);
	hdmi_timer_task = kthread_create(hdmi_timer_kthread, NULL, "hdmi_timer_kthread");
	wake_up_process(hdmi_timer_task);

	init_waitqueue_head(&cec_timer_wq);
	cec_timer_task = kthread_create(cec_timer_kthread, NULL, "cec_timer_kthread");
	wake_up_process(cec_timer_task);

	init_waitqueue_head(&hdmi_irq_wq);
	hdmi_irq_task = kthread_create(hdmi_irq_kthread, NULL, "hdmi_irq_kthread");
	wake_up_process(hdmi_irq_task);

	fb_register_client(&hdmi_event_notifier);

#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	fgCaHDMICreate();
#endif

	return 0;
}


void vNotifyAppHdmiState(unsigned char u1hdmistate)
{
	HDMI_EDID_T get_info;
	static unsigned char pre_state;

	HDMI_EDID_FUNC();

	hdmi_AppGetEdidInfo(&get_info);
	hdmi_dispres = hdmi_DispGetEdidInfo();

	TX_DEF_LOG("[port]Notify APP HDMI state = %d, res = 0x%llx, %s\n",
		u1hdmistate,
		hdmi_dispres,
		szHdmiPordStatusStr[u1hdmistate]);

	switch (u1hdmistate) {
	case HDMI_PLUG_OUT:
		hdmi_util.state_callback(HDMI_STATE_NO_DEVICE);
		hdmi_SetPhysicCECAddress(0xffff, 0x0);
		hdmi_util.cec_state_callback(HDMI_CEC_STATE_PLUG_OUT);
		break;

	case HDMI_PLUG_IN_AND_SINK_POWER_ON:
		hdmi_util.state_callback(HDMI_STATE_ACTIVE);
		hdmi_SetPhysicCECAddress(get_info.ui2_sink_cec_address, 0x4);
		hdmi_util.cec_state_callback(HDMI_CEC_STATE_GET_PA);
		cod_test();
		break;

	case HDMI_PLUG_IN_ONLY:
		hdmi_SetPhysicCECAddress(get_info.ui2_sink_cec_address, 0xf);
		if (pre_state != HDMI_PLUG_IN_AND_SINK_POWER_ON)
			hdmi_util.cec_state_callback(HDMI_CEC_STATE_GET_PA);
		hdmi_util.state_callback(HDMI_STATE_NO_DEVICE);
		break;

	case HDMI_PLUG_IN_CEC:
		hdmi_util.state_callback(HDMI_STATE_CEC_UPDATE);
		break;

	case HDMI_PLUG_OUT_POWEROFF:
		hdmi_util.state_callback(HDMI_STATE_NO_DEVICE);
		break;

	default:
		break;

	}
	pre_state = u1hdmistate;
}

int hdmi_audio_signal_state(unsigned int state)
{
	if(state) {
		if (hdmi_audio_event != 1) {
			hdmi_util.state_callback(HDMI_STATE_CHANGE_AUDIO_ON);
			hdmi_audio_event = 1;
		}
	} else {
		if (hdmi_audio_event != 0) {
			hdmi_util.state_callback(HDMI_STATE_CHANGE_AUDIO_OFF);
			hdmi_audio_event = 0;
		}
	}

	return 0;
}
EXPORT_SYMBOL(hdmi_audio_signal_state);

long long vDispGetHdmiResolution(void)
{
	return hdmi_dispres;
}

void vNotifyAppHdmiCecState(HDMI_NFY_CEC_STATE_T u1hdmicecstate)
{
	HDMI_CEC_LOG("u1hdmicecstate = %d, %s\n", u1hdmicecstate,
		     szHdmiCecPordStatusStr[u1hdmicecstate]);
	switch (u1hdmicecstate) {
	case HDMI_CEC_TX_STATUS:
		hdmi_util.cec_state_callback(HDMI_CEC_STATE_TX_STS);
		break;

	case HDMI_CEC_GET_CMD:
		hdmi_util.cec_state_callback(HDMI_CEC_STATE_GET_CMD);
		break;

	default:
		break;
	}
}

static void vPlugDetectService_edid(HDMI_CTRL_STATE_T e_state)
{
	HDMI_PLUG_FUNC();

	/* clear port change info before reading EDID */
	/* driver read EDID only and don't do others, if reading EDID is successful, then do others */

	switch (e_state) {
	case HDMI_STATE_HOT_PLUGIN_AND_POWER_ON:
		if (hdmi_clockenable == 0) {
			HDMI_DRV_LOG("hdmi irq for clock:hdmi plug in and power on\n");
			hdmi_clockenable = 1;
			hdmi_clock_enable(true);
		}
		vClearEdidInfo();
		hdmi_clear_edid_data();
		hdmi_checkedid(0x0);
		vReadHdcpVersion();
		break;

	case HDMI_STATE_HOT_PLUG_IN_ONLY:
		if (hdmi_clockenable == 0) {
			HDMI_DRV_LOG("hdmi irq for clock:hdmi plug in only\n");
			hdmi_clockenable = 1;
			hdmi_clock_enable(true);
		}
		vClearEdidInfo();
		hdmi_clear_edid_data();
		hdmi_checkedid(0x0);
		break;
	case HDMI_STATE_IDLE:
		break;

	default:
		break;
	}
}

static void vPlugDetectService(HDMI_CTRL_STATE_T e_state)
{
	unsigned char bData = 0xff;

	HDMI_PLUG_FUNC();
	e_hdmi_ctrl_state = HDMI_STATE_IDLE;

	switch (e_state) {
	case HDMI_STATE_HOT_PLUG_OUT:
		vClearEdidInfo();
		hdmi_clear_edid_data();
		hdmi_boot_res = 0xff;
		hdmi_boot_colordepth = 0xff;
		hdmi_boot_colorspace = 0xff;
		hdcp_current_level = NO_HDCP;
		hdcp_max_level = NO_HDCP;
		_u2TxBStatus = 0;
		_fgRepeater = false;
		vTmdsOnOffAndResetHdcp(0);
		if (hdmi_clockenable == 1) {
			HDMI_DRV_LOG("[hdmi]hdmi irq for clock:hdmi plug out\n");
			hdmi_clockenable = 0;
			hdmi_clock_enable(false);
		}
		bData = HDMI_PLUG_OUT;
		break;

	case HDMI_STATE_HOT_PLUGIN_AND_POWER_ON:
		if (hdmi_clockenable == 0) {
			HDMI_DRV_LOG("[hdmi]hdmi irq for clock:hdmi plug in and power on\n");
			hdmi_clockenable = 1;
			hdmi_clock_enable(true);
		}
		/* hdmi_checkedid(0x0); */
		/* vReadHdcpVersion(); */
		bData = HDMI_PLUG_IN_AND_SINK_POWER_ON;
		break;

	case HDMI_STATE_HOT_PLUG_IN_ONLY:
		if (hdmi_clockenable == 0) {
			HDMI_DRV_LOG("hdmi irq for clock:hdmi plug in only\n");
			hdmi_clockenable = 1;
			hdmi_clock_enable(true);
		}
		/* vClearEdidInfo(); */
		hdmi_boot_res = 0xff;
		hdmi_boot_colordepth = 0xff;
		hdmi_boot_colorspace = 0xff;
		hdcp_current_level = NO_HDCP;
		hdcp_max_level = NO_HDCP;
		_u2TxBStatus = 0;
		_fgRepeater = false;
		vTmdsOnOffAndResetHdcp(0);
		/* hdmi_checkedid(0x0); */
		bData = HDMI_PLUG_IN_ONLY;
		break;

	case HDMI_STATE_POWER_OFF_HOT_PLUG_OUT:
		hdcp_current_level = NO_HDCP;
		hdcp_max_level = NO_HDCP;
		_u2TxBStatus = 0;
		_fgRepeater = false;
		vTmdsOnOffAndResetHdcp(0);
		if (hdmi_clockenable == 1) {
			HDMI_DRV_LOG("[hdmi]hdmi irq for clock:hdmi plug out and hpd off\n");
			hdmi_clockenable = 0;
			hdmi_clock_enable(false);
		}
		hdmi_boot_res = 0xff;
		hdmi_boot_colordepth = 0xff;
		hdmi_boot_colorspace = 0xff;
		bData = HDMI_PLUG_OUT_POWEROFF;
		break;

	case HDMI_STATE_IDLE:
		break;

	default:
		break;
	}

	if (bData != 0xff)
		vNotifyAppHdmiState(bData);
}

void hdmi_force_plug_out(void)
{
	hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
	vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);
	vPlugDetectService(HDMI_STATE_POWER_OFF_HOT_PLUG_OUT);
}

void hdmi_force_plug_in(void)
{
	hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
	vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);
	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);
}


void hdmi_drvlog_enable(unsigned int enable)
{
	HDMI_DRV_FUNC();

	if (enable == 0) {
		TX_DEF_LOG("hdmiplllog =   0x1\n");
		TX_DEF_LOG("hdmiceccommandlog =   0x2\n");
		TX_DEF_LOG("hdmitxhotpluglog =  0x4\n");
		TX_DEF_LOG("hdmitxvideolog = 0x8\n");
		TX_DEF_LOG("hdmitxaudiolog = 0x10\n");
		TX_DEF_LOG("hdmihdcplog =  0x20\n");
		TX_DEF_LOG("hdmiceclog =   0x40\n");
		TX_DEF_LOG("hdmiddclog =   0x80\n");
		TX_DEF_LOG("hdmiedidlog =  0x100\n");
		TX_DEF_LOG("hdmidrvlog =   0x200\n");
		TX_DEF_LOG("hdmireglog =	0x400\n");
		TX_DEF_LOG("hdmicecreglog =	0x800\n");
		TX_DEF_LOG("hdmihdrlog =	0x1000\n");
		TX_DEF_LOG("hdmihdrdebuglog =	0x2000\n");
		TX_DEF_LOG("hdmitzdebuglog =	0x4000\n");
		TX_DEF_LOG("hdmi_all_log =   0x7fff\n");
	}

	hdmidrv_log_on = enable;
#if defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT)
	fgCaHDMISetTzLogLevel(hdmidrv_log_on);
#endif
}

void hdmi_timer_impl(void)
{
	unsigned int bStatus;

	if (!hdmi_hdmi_on) {
		HDMI_DRV_LOG("hdmi_hdmi_on is false, return.\n");
		return;
	}

	bStatus = bReadByteHdmiGRL(TOP_INT_STA00);
	if (bStatus & HDCP_RI_128_INT_STA) {
		/* clear ri irq */
		vWriteHdmiGRLMsk(TOP_INT_CLR00, HDCP_RI_128_INT_CLR, HDCP_RI_128_INT_CLR);
		vWriteHdmiGRLMsk(TOP_INT_CLR00, 0x0, HDCP_RI_128_INT_CLR);

	if ((hdmi_hotplugstate == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)
	    && ((e_hdcp_ctrl_state == HDCP_WAIT_RI)
		|| (e_hdcp_ctrl_state == HDCP_CHECK_LINK_INTEGRITY))) {
			vSetHDCPState(HDCP_CHECK_LINK_INTEGRITY);
			vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
		}
	}

	hdmi_hdcp22_monitor();
	hdmi_timing_monitor();

	if (hdmi_hdmiCmd == HDMI_PLUG_DETECT_CMD) {
		vClearHdmiCmd();
		/* vcheckhdmiplugstate(); */
		/* vPlugDetectService(e_hdmi_ctrl_state); */
	} else if ((hdmi_hdmiCmd == HDMI_HDCP_PROTOCAL_CMD)
		   && (hdmi_hotplugstate == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)) {
		vClearHdmiCmd();
		HdcpService(e_hdcp_ctrl_state);
		if (resolution_change == true && e_hdcp_ctrl_state == HDCP2x_ENCRYPTION) {
			resolution_change = FALSE;
			hdmi_util.state_callback(HDMI_STATE_CHANGE_RESOLUTION);
		}
	} else if ((hdmi_hdmiCmd == HDMI_HDR10_DELAY_OFF_CMD)
		   && (hdmi_hotplugstate == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)) {
		vClearHdmiCmd();
		Hdr10DelayOffHandler();
	} else if ((hdmi_hdmiCmd == HDMI_HDR10P_VSIF_DELAY_OFF_CMD)
		   && (hdmi_hotplugstate == HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)) {
		vClearHdmiCmd();
		Hdr10pVsifDelayOffHandler();
	}

	hdmistate_debug++;
	if (hdmistate_debug == SHOW_HDMISTATE_LOG_TIME) {
		hdmi_hdmistatus();
		if (resolution_change == true) {
			resolution_change = FALSE;
			hdmi_util.state_callback(HDMI_STATE_CHANGE_RESOLUTION);
		}
	}
	if ((check_plugin_switch_resolution == 1)
	    && (hdmistate_debug > MASK_UNUSE_INTERRUPT_TIME)) {
		check_plugin_switch_resolution = 0;
	}

	hdcp_unmute_process();
}

void cec_timer_impl(void)
{
	if (hdmi_cec_on == 1)
		hdmi_cec_mainloop(hdmi_rxcecmode);
}

void hdmi_irq_impl(void)
{
	unsigned char port_hpd_value;

	msleep(50);

	if (down_interruptible(&hdmi_update_mutex)) {
		TX_DEF_LOG("can't get semaphore in %s() for boot time\n", __func__);
		return;
	}

	port_hpd_value = hdmi_get_port_hpd_value();
	TX_DEF_LOG("[port]hdmi_hotplugstate = %d, hpd_port = %d, L:%d\n",
		hdmi_hotplugstate, port_hpd_value, __LINE__);

	if (hdmi_is_boot_time == 1) {
		HDMI_PLUG_LOG("hdmi_is_boot_time: pre-status = 0x%x cur-status = 0x%x\n",
			hdmi_port_status_in_boot, bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE));
		if (hdmi_port_status_in_boot != bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE)) {
			hdmi_port_status_in_boot = bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE);
			if (bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE) == TRUE) {
				hdmi_hotplugstate = HDMI_STATE_HOT_PLUGIN_AND_POWER_ON;
				TX_DEF_LOG("HDMI_STATE_HOT_PLUGIN_AND_POWER_ON\n");
				vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_IN_AND_SINK_POWER_ON);
				hdmi_checkedid(0x0);
				hdmi_util.state_callback(HDMI_STATE_ACTIVE_IN_BOOT);
				hdmi_timing_monitor_start();
			} else {
				hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
				TX_DEF_LOG("HDMI_STATE_HOT_PLUG_OUT\n");
				vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);
				vClearEdidInfo();
				hdmi_util.state_callback(HDMI_STATE_NO_DEVICE_IN_BOOT);
			}
		}
	} else if ((hdmi_hotplugstate != HDMI_STATE_HOT_PLUG_OUT)
		   && (bCheckPordHotPlug(HOTPLUG_MODE) == FALSE)
		   && (hdmi_powerenable == 1)) {
		hdcp2_version_flag = FALSE;
		hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_OUT;
		hdmi_hotplugout_count = 1;
		vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_OUT);
		vPlugDetectService(HDMI_STATE_HOT_PLUG_OUT);
		TX_DEF_LOG("hdmi plug out\n");
	} else if ((hdmi_hotplugstate != HDMI_STATE_HOT_PLUGIN_AND_POWER_ON)
		   && (bCheckPordHotPlug(PORD_MODE | HOTPLUG_MODE) == TRUE)
		   && (hdmi_powerenable == 1)) {
		/* clear  port_hpd_value_chg before reading EDID */
		port_hpd_value_chg = FALSE;
		/* read EDID */
		vPlugDetectService_edid(HDMI_STATE_HOT_PLUGIN_AND_POWER_ON);
		/* if port have changed when reading EDID, don't change hdmi_hotplugstate state and don't send uevent */
		/* if port change after reading, then we need to read EDID again  */
		if (port_hpd_value_chg == TRUE) {
			TX_DEF_LOG("[port]HPD change when driver read EDID, plugin, read again\n");
			atomic_set(&hdmi_irq_event, 1);
			wake_up_interruptible(&hdmi_irq_wq);
		} else {
			hdmi_hotplugstate = HDMI_STATE_HOT_PLUGIN_AND_POWER_ON;
			vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_PLUG_IN_AND_SINK_POWER_ON);
			vPlugDetectService(HDMI_STATE_HOT_PLUGIN_AND_POWER_ON);
			TX_DEF_LOG("hdmi plug in\n");
		}
	} else if ((hdmi_hotplugstate != HDMI_STATE_HOT_PLUG_IN_ONLY)
		   && (bCheckPordHotPlug(HOTPLUG_MODE) == TRUE)
		   && (bCheckPordHotPlug(PORD_MODE) == FALSE)) {
		port_hpd_value_chg = FALSE;
		vPlugDetectService_edid(HDMI_STATE_HOT_PLUG_IN_ONLY);
		if (port_hpd_value_chg == TRUE) {
			TX_DEF_LOG("[port]HPD change when driver read EDID, plugin only, read again\n");
			atomic_set(&hdmi_irq_event, 1);
			wake_up_interruptible(&hdmi_irq_wq);
		} else {
			hdcp2_version_flag = FALSE;
			hdmi_hotplugstate = HDMI_STATE_HOT_PLUG_IN_ONLY;
			hdmi_hotplugout_count = 1;
			vSetSharedInfo(SI_HDMI_RECEIVER_STATUS, HDMI_STATE_HOT_PLUG_IN_ONLY);
			vPlugDetectService(HDMI_STATE_HOT_PLUG_IN_ONLY);
			TX_DEF_LOG("hdmi plug in only\n");
		}
	} else if ((hdmi_hotplugstate == HDMI_STATE_HOT_PLUG_OUT)
		   && (bCheckPordHotPlug(HOTPLUG_MODE) == FALSE)
		   && (bCheckPordHotPlug(PORD_MODE) == FALSE)
		   && (hdmi_powerenable == 1) && (hdmi_clockenable == 1)) {
		TX_DEF_LOG("first power on from api-->plugout\n");
		vPlugDetectService(HDMI_STATE_HOT_PLUG_OUT);
		hdcp2_version_flag = FALSE;
	} else {
		TX_DEF_LOG("[port]hdmi plug no action,same to before\n");
	}

	up(&hdmi_update_mutex);

}

static int hdmi_timer_kthread(void *data)
{
#if 0
	struct sched_param param = {.sched_priority = RTPM_PRIO_CAMERA_PREVIEW };

	sched_setscheduler(current, SCHED_RR, &param);
#endif
	for (;;) {
		wait_event_interruptible(hdmi_timer_wq, atomic_read(&hdmi_timer_event));
		atomic_set(&hdmi_timer_event, 0);
		hdmi_timer_impl();
		if (kthread_should_stop())
			break;
	}
	return 0;
}

static int cec_timer_kthread(void *data)
{
	for (;;) {
		wait_event_interruptible(cec_timer_wq, atomic_read(&cec_timer_event));
		atomic_set(&cec_timer_event, 0);
		cec_timer_impl();
		if (kthread_should_stop())
			break;
	}
	return 0;
}

static int hdmi_irq_kthread(void *data)
{
#if 0
	struct sched_param param = {.sched_priority = RTPM_PRIO_SCRN_UPDATE };

	sched_setscheduler(current, SCHED_RR, &param);
#endif
	for (;;) {
		wait_event_interruptible(hdmi_irq_wq, atomic_read(&hdmi_irq_event));
		atomic_set(&hdmi_irq_event, 0);
		hdmi_irq_impl();
		if (kthread_should_stop())
			break;
	}
	return 0;
}

void hdmi_poll_isr(unsigned long n)
{
	unsigned int i;
	unsigned char port_hpd_value;

	/* port detection here */
	port_hpd_value = hdmi_get_port_hpd_value();
	if (port_hpd_value_bak != port_hpd_value) {
		port_hpd_value_bak = port_hpd_value;
		port_hpd_value_chg = TRUE;
		atomic_set(&hdmi_irq_event, 1);
		wake_up_interruptible(&hdmi_irq_wq);
	}

	for (i = 0; i < MAX_HDMI_TMR_NUMBER; i++) {
		if (hdmi_TmrValue[i] >= AVD_TMR_ISR_TICKS) {
			hdmi_TmrValue[i] -= AVD_TMR_ISR_TICKS;

			if ((i == HDMI_PLUG_DETECT_CMD)
			    && (hdmi_TmrValue[HDMI_PLUG_DETECT_CMD] == 0))
				vSendHdmiCmd(HDMI_PLUG_DETECT_CMD);
			else if ((i == HDMI_HDCP_PROTOCAL_CMD)
				 && (hdmi_TmrValue[HDMI_HDCP_PROTOCAL_CMD] == 0))
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			else if ((i == HDMI_HDR10_DELAY_OFF_CMD)
				 && (hdmi_TmrValue[HDMI_HDR10_DELAY_OFF_CMD] == 0))
				vSendHdmiCmd(HDMI_HDR10_DELAY_OFF_CMD);
			else if ((i == HDMI_HDR10P_VSIF_DELAY_OFF_CMD)
				 && (hdmi_TmrValue[HDMI_HDR10P_VSIF_DELAY_OFF_CMD] == 0))
				vSendHdmiCmd(HDMI_HDR10P_VSIF_DELAY_OFF_CMD);

		} else if (hdmi_TmrValue[i] > 0) {
			hdmi_TmrValue[i] = 0;

			if ((i == HDMI_PLUG_DETECT_CMD)
			    && (hdmi_TmrValue[HDMI_PLUG_DETECT_CMD] == 0))
				vSendHdmiCmd(HDMI_PLUG_DETECT_CMD);
			else if ((i == HDMI_HDCP_PROTOCAL_CMD)
				 && (hdmi_TmrValue[HDMI_HDCP_PROTOCAL_CMD] == 0))
				vSendHdmiCmd(HDMI_HDCP_PROTOCAL_CMD);
			else if ((i == HDMI_HDR10_DELAY_OFF_CMD)
				 && (hdmi_TmrValue[HDMI_HDR10_DELAY_OFF_CMD] == 0))
				vSendHdmiCmd(HDMI_HDR10_DELAY_OFF_CMD);
			else if ((i == HDMI_HDR10P_VSIF_DELAY_OFF_CMD)
				 && (hdmi_TmrValue[HDMI_HDR10P_VSIF_DELAY_OFF_CMD] == 0))
				vSendHdmiCmd(HDMI_HDR10P_VSIF_DELAY_OFF_CMD);
		}
	}

	atomic_set(&hdmi_timer_event, 1);
	wake_up_interruptible(&hdmi_timer_wq);
	mod_timer(&r_hdmi_timer, jiffies + gHDMI_CHK_INTERVAL / (1000 / HZ));
}

void cec_timer_wakeup(void)
{
	memset((void *)&r_cec_timer, 0, sizeof(r_cec_timer));
	r_cec_timer.expires = jiffies + 1000 / (1000 / HZ);	/* wait 1s to stable */
	r_cec_timer.function = cec_poll_isr;
	r_cec_timer.data = 0;
	init_timer(&r_cec_timer);
	add_timer(&r_cec_timer);

	hdmi_is_boot_time = 0;
	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);
}

void cec_timer_sleep(void)
{
	if (r_cec_timer.function)
		del_timer_sync(&r_cec_timer);
	memset((void *)&r_cec_timer, 0, sizeof(r_cec_timer));
}

void cec_poll_isr(unsigned long n)
{
	atomic_set(&cec_timer_event, 1);
	wake_up_interruptible(&cec_timer_wq);
	mod_timer(&r_cec_timer, jiffies + gCEC_CHK_INTERVAL / (1000 / HZ));
}

void hdmi_sdck_select(bool bEnable)
{
	if (bEnable) {
		HDMI_DRV_LOG("Select SD clock\n");
		clk_prepare(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
		clk_enable(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
		HDMI_DRV_LOG("enable SD clock MMSYS_HDMI_SDOSD\n");
		hdmi_sdosd_first = 1;

		clk_set_parent(hdmi_ref_clock[MMSYS_HDMI_SDCKSEL], hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
		HDMI_DRV_LOG("set parent MMSYS_HDMI_SDCKSEL: MMSYS_HDMI_SDOSD\n");

		if (hdmi_hdtvd_first != 0) {
		clk_disable(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
		clk_unprepare(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
		HDMI_DRV_LOG("disable HD clock MMSYS_HDMI_HDTVD\n");
		hdmi_hdtvd_first = 0;
		}
	} else {
		HDMI_DRV_LOG("Select HD clock\n");
		clk_prepare(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
		clk_enable(hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
		HDMI_DRV_LOG("enable HD clock MMSYS_HDMI_HDTVD\n");
		hdmi_hdtvd_first = 1;

		clk_set_parent(hdmi_ref_clock[MMSYS_HDMI_SDCKSEL], hdmi_ref_clock[MMSYS_HDMI_HDTVD]);
		HDMI_DRV_LOG("set parent MMSYS_HDMI_SDCKSEL: MMSYS_HDMI_HDTVD\n");

		if (hdmi_sdosd_first != 0) {
		clk_disable(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
		clk_unprepare(hdmi_ref_clock[MMSYS_HDMI_SDOSD]);
		HDMI_DRV_LOG("disable SD clock MMSYS_HDMI_SDOSD\n");
		hdmi_sdosd_first = 0;
		}
	}
}

void hdmi_clock_debug(bool bEnable, unsigned char bItem)
{
	if (bItem < MMSYS_HDMI_HDTVD) {
		if (bEnable) {
			HDMI_DRV_LOG("Enable hdmi clocks(0-8) %d\n", bItem);
			clk_prepare(hdmi_ref_clock[bItem]);
			clk_enable(hdmi_ref_clock[bItem]);
			HDMI_DRV_LOG("Enable hdmi clocks i = %d\n", bItem);
		} else {
			HDMI_DRV_LOG("Disable hdmi clocks(0-8) %d\n", bItem);
			clk_disable(hdmi_ref_clock[bItem]);
			clk_unprepare(hdmi_ref_clock[bItem]);
			HDMI_DRV_LOG("Disable hdmi clocks i = %d\n", bItem);
		}
	} else if (bItem == MMSYS_HDMI_HDTVD) {
		hdmi_sdck_select(0);
	} else if (bItem == MMSYS_HDMI_SDOSD) {
		hdmi_sdck_select(1);
	}
}


void hdmi_clock_enable(bool bEnable)
{
	int i;

	if (bEnable) {
		HDMI_DRV_LOG("Enable hdmi clocks(include rgb2hdmi)\n");
		for (i = 0; i < MMSYS_HDMI_HDTVD; i++) {
			HDMI_DRV_LOG("1Enable hdmi clocks i = %d\n", i);
			clk_prepare(hdmi_ref_clock[i]);
			clk_enable(hdmi_ref_clock[i]);
			HDMI_DRV_LOG("Enable hdmi clocks i = %d\n", i);
		}
	} else {
		HDMI_DRV_LOG("Disable hdmi clocks\n");
		for (i = MMSYS_HDMI_HDTVD - 1; i >= 0; i--) {
			HDMI_DRV_LOG("1Disable hdmi clocks i = %d\n", i);
			clk_disable(hdmi_ref_clock[i]);
			clk_unprepare(hdmi_ref_clock[i]);
			HDMI_DRV_LOG("Disable hdmi clocks i = %d\n", i);
		}
	}
}

void hdmi_clock_probe(struct platform_device *pdev)
{
	int i;

	HDMI_DRV_LOG("Probe clocks start\n");
	if (pdev == NULL) {
		pr_err("[HDMI] pdev Error\n");
		return;
	}

	pm_runtime_enable(&pdev->dev);
	hdmi_pdev = pdev;

	for (i = 0; i < TOP_HDMI_SEL; i++) {
		hdmi_ref_clock[i] = devm_clk_get(&pdev->dev, hdmi_use_clock_name_spy(i));
		WARN_ON(IS_ERR(hdmi_ref_clock[i]));

		HDMI_DRV_LOG("Get Clock %s\n", hdmi_use_clock_name_spy(i));
	}
}

static int attach_input_hdmidev(void)
{
	int ret = 0;

	input = input_allocate_device();
	if (!input) {
		ret = -ENOMEM;
		goto err_free_mem;
	}

	/* Indicate that we generate key events */
	set_bit(EV_KEY, input->evbit);
	__set_bit(KEY_POWER, input->keybit);

	input->name = "hdmipower";
	ret = input_register_device(input);
	if (ret)
		goto err_free_mem;

	return ret;

err_free_mem:
	input_free_device(input);
	return ret;
}

int report_virtual_hdmikey(void)
{
	if ((display_off == 0) || (display_off == 2)) {
		TX_DEF_LOG("[CECWAKE] display is already on, display_off=%d\n", display_off);
		return 0;
	}

	/* avoid sending duplicate keys in case two power on commands came too
	   close to each other such as <routing info> and <set stream path>.
	   Using a value of 2 to indicate KEY_POWER is being sent already. */
	display_off = 2;
	input_report_key(input, KEY_POWER, 1);
	input_sync(input);

	input_report_key(input, KEY_POWER, 0);
	input_sync(input);
	TX_DEF_LOG("[CECWAKE] display was off and is being turned on\n");
	return 0;
}

int hdmi_internal_probe(struct platform_device *pdev, unsigned long u8Res)
{
	int i;
	int ret;
	struct device_node *np;
	unsigned int reg_value;
	int *buf;
	struct nvmem_cell *cell;
	size_t len;

	HDMI_DRV_LOG("[hdmi_internal_probe] probe start u8Res = %ld\n", u8Res);

	cell = of_nvmem_cell_get(pdev->dev.of_node, "hdmi_calibration");
	if (IS_ERR(cell))
		return -1;
	buf = nvmem_cell_read(cell, &len);
	nvmem_cell_put(cell);
	if (IS_ERR(buf))
		return -1;
	caefuse0 = buf[0];
	caefuse1 = buf[1];
	caefuse2 = buf[2];
	caefuse3 = buf[3];
	kfree(buf);

	efuseValue0 = (caefuse0 >> 9) & 0x1F;
	efuseValue1 = (caefuse1 >> 0) & 0x3F;
	efuseValue2 = (caefuse1 >> 6) & 0x3F;
	efuseValue3 = (caefuse1 >> 12) & 0x3F;
	efuseValue4 = (caefuse1 >> 18) & 0x3F;

	HDMI_DRV_LOG("[calefuse] 0x%x, 0x%x, 0x%x, 0x%x\n", caefuse0, caefuse1, caefuse2, caefuse3);
	TX_DEF_LOG("[efuseValue] 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		efuseValue0, efuseValue1, efuseValue2, efuseValue3, efuseValue4);

	if (pdev->dev.of_node == NULL) {
		TX_DEF_LOG("[hdmi_internal_probe] Device Node Error\n");
		return -1;
	}
	/* iomap registers and irq of HDMI Module */
	for (i = 0; i < HDMI_REG_NUM; i++) {
		hdmi_reg[i] = (unsigned long)of_iomap(pdev->dev.of_node, i);
		if (!hdmi_reg[i]) {
			HDMI_DRV_LOG("Unable to ioremap registers, of_iomap fail, i=%d\n", i);
			return -ENOMEM;
		}
		HDMI_DRV_LOG("DT, i=%d, map_addr=0x%lx, reg_pa=0x%x\n",
			     i, hdmi_reg[i], hdmi_reg_pa_base[i]);
	}

	hdmi_filter_switch_gpio = of_get_named_gpio(pdev->dev.of_node, "hdmi,filter_switch_gpio", 0);

	if (hdmi_filter_switch_gpio < 0)
		pr_info("%s: no hdmi filter switch info\n", __func__);

	ret = gpio_request(hdmi_filter_switch_gpio, "hdmi_filter_switch_gpio");
	if (ret < 0) {
		pr_info("%s : fail to request hdmi filter switch gpio\n", __func__);
		hdmi_filter_switch_gpio = -1;
	}
	gpio_direction_output(hdmi_filter_switch_gpio, 0);

        cod_init(pdev);
        cod_test();

	/*get IRQ ID and request IRQ */
	HDMI_PLUG_LOG("get IRQ ID and request IRQ\n");
	hdmi_irq = irq_of_parse_and_map(pdev->dev.of_node, 0);

	/* Get PLL/TOPCK/INFRA_SYS/MMSSYS /GPIO/EFUSE  phy base address and iomap virtual address */
	for (i = 0; i < HDMI_REF_REG_NUM; i++) {
		np = of_find_compatible_node(NULL, NULL, hdmi_use_module_name_spy(i));
		if (np == NULL)
			continue;

		of_property_read_u32_index(np, "reg", 0, &reg_value);
		hdmi_ref_reg_pa_base[i] = reg_value;
		if ((i == VDOUT10_REG) || (i == VDOUT1C_REG))
			hdmi_ref_reg[i] = (unsigned long)of_iomap(np, 1);
		else
			hdmi_ref_reg[i] = (unsigned long)of_iomap(np, 0);
		HDMI_DRV_LOG("DT HDMI_Ref|%s, reg base: 0x%x --> map:0x%lx\n",
			     np->name, reg_value, hdmi_ref_reg[i]);
	}
	hdmi_clock_probe(pdev);
	/*   debugfs init   */
	ret = hdmitx_debug_init();

	vInitHdr();
	vHdmiSetInit();
	vInitAvInfoVar();
	hdmi_cec_probe(pdev);

	HDMI_EnableIrq();
	if (hdmi_clockenable == 0) {
		HDMI_DRV_LOG("[hdmi]turn on hdmi clock: hdmi_internal_probe");
		hdmi_clockenable = 1;
		hdmi_clock_enable(true);
	}

	hdmi_5vpower = 1;
	hdmi_is_boot_time = 1;
	hdmi_timing_monitor_stop();
	atomic_set(&hdmi_irq_event, 1);
	wake_up_interruptible(&hdmi_irq_wq);

	attach_input_hdmidev();
	/* Passive switches may not assert HPD so test current overdraw anyway */
	return 0;

}

unsigned int i4GetHotPlugStatus(void)
{
	return i4SharedInfo(SI_HDMI_RECEIVER_STATUS);
}

unsigned char fgIsHDCPCtrlTimeOut(void)
{
	if (_HdmiTmrValue[HDCP_CTRL_TMR_INX] <= 0)
		return TRUE;
	else
		return FALSE;
}

void vSetHDCPTimeOut(unsigned int i4_count)
{
	HDMI_HDCP_FUNC();
	hdmi_TmrValue[HDMI_HDCP_PROTOCAL_CMD] = i4_count;
}

void hdmi_hdcp_information(HDCP_INFO *hdcp_information)
{
	hdcp_information->current_hdcp_level = hdcp_current_level;
	hdcp_information->max_hdcp_level = hdcp_max_level;
	hdcp_information->ui2bstatus = _u2TxBStatus;
	hdcp_information->brepeater = _fgRepeater;

	HDMI_HDCP_LOG("hdmi_hdcp_information curver:%d; maxver:%d; status:0x%x; repeater:%d\n",
		hdcp_information->current_hdcp_level, hdcp_information->max_hdcp_level,
		hdcp_information->ui2bstatus, hdcp_information->brepeater);
}

void hdmi_show_hdcp_information(void)
{
	HDMI_HDCP_LOG("hdmi_hdcp_information curver:%d; maxver:%d; status:0x%x; repeater:%d\n",
		hdcp_current_level, hdcp_max_level,
		_u2TxBStatus, _fgRepeater);
}

const HDMI_DRIVER *HDMI_GetDriver(void)
{
	static const HDMI_DRIVER HDMI_DRV = {
		.set_util_funcs = hdmi_set_util_funcs,
		.get_params = hdmi_get_params,
		.hdmidrv_probe = hdmi_internal_probe,
		.init = hdmi_internal_init,
		.enter = hdmi_internal_enter,
		.exit = hdmi_internal_exit,
		.suspend = hdmi_internal_suspend,
		.resume = hdmi_internal_resume,
		.video_config = hdmi_internal_video_config,
		.audio_config = hdmi_internal_audio_config,
		.video_enable = hdmi_internal_video_enable,
		.audio_enable = hdmi_internal_audio_enable,
		.power_on = hdmi_internal_power_on,
		.power_off = hdmi_internal_power_off,
		.set_mode = hdmi_internal_set_mode,
		.dump = hdmi_internal_dump,
		.read = hdmi_read,
		.write = hdmi_write,
		.get_state = hdmi_get_state,
		.log_enable = hdmi_drvlog_enable,
		.InfoframeSetting = hdmi_InfoframeSetting,
		.checkedid = hdmi_checkedid,
		.colordeep = hdmi_colordeep,
		.enablehdcp = hdmi_enablehdcp,
		.setcecrxmode = hdmi_setcecrxmode,
		.hdmistatus = hdmi_hdmistatus,
		.hdcpkey = hdmi_hdcpkey,
		.getedid = hdmi_AppGetEdidInfo,
		.setcecla = hdmi_CECMWSetLA,
		.sendsltdata = hdmi_u4CecSendSLTData,
		.getceccmd = hdmi_CECMWGet,
		.getsltdata = hdmi_GetSLTData,
		.setceccmd = hdmi_CECMWSend,
		.cecenable = hdmi_CECMWSetEnableCEC,
		.getcecaddr = hdmi_NotifyApiCECAddress,
		.getcectxstatus = hdmi_cec_api_get_txsts,
		.audiosetting = hdmi_audiosetting,
		.tmdsonoff = hdmi_tmdsonoff,
		.mutehdmi = vDrm_mutehdmi,
		.svpmutehdmi = vSvp_mutehdmi,
		.cecusrcmd = hdmi_cec_usr_cmd,
		.checkedidheader = hdmi_check_edid_header,
		.gethdmistatus = hdmi_check_status,
		.hdcp_info = hdmi_hdcp_information,
	};

	return &HDMI_DRV;
}
EXPORT_SYMBOL(HDMI_GetDriver);
#endif
