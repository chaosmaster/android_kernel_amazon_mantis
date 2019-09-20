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

#if defined(CONFIG_MTK_HDMI_SUPPORT)
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/switch.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <hdmitx.h>
#include <hdmi_ctrl.h>
#include "internal_hdmi_drv.h"
#include "disp_hw_mgr.h"
#include <linux/time.h>
#include "hdmihdcp.h"
#include "hdmictrl.h"

#define HDMI_DEVNAME "hdmitx"
DEFINE_SEMAPHORE(hdmi_update_mutex);

static struct switch_dev hdmi_switch_data;
static struct switch_dev hdmires_switch_data;
static struct switch_dev hdmi_cec_switch_data;
static struct switch_dev hdmi_audio_switch_data;
static struct switch_dev hdmi_hdcp_switch_data;


static HDMI_DRIVER *hdmi_drv;
static dev_t hdmi_devno;
static struct cdev *hdmi_cdev;
static struct class *hdmi_class;
static long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int hdmi_open(struct inode *inode, struct file *file);
static int hdmi_release(struct inode *inode, struct file *file);
static int hdmi_probe(struct platform_device *pdev);
static int hdmi_remove(struct platform_device *pdev);
static void hdmi_shutdown(struct platform_device *pdev);
extern unsigned int hdmi_audio_event;
extern unsigned int hdmistate_debug;
extern unsigned char _bHdcpStatus;
bool send_fake_hpd = false;

void hdmi_log_enable(int enable)
{
	hdmi_drv->log_enable(enable);
}

#if IS_ENABLED(CONFIG_COMPAT)
static long hdmi_ioctl_compat(struct file *file, unsigned int cmd, unsigned long arg);
#endif

static const struct file_operations hdmi_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hdmi_ioctl,
#if IS_ENABLED(CONFIG_COMPAT)
	.compat_ioctl = hdmi_ioctl_compat,
#endif
	.open = hdmi_open,
	.release = hdmi_release,
};

static const struct of_device_id hdmi_of_ids[] = {
	{.compatible = "mediatek,mt8695-hdmitx20",},
	{},
};

static void hdmi_shutdown(struct platform_device *pdev)
{
	HDMI_DRV_FUNC();

	hdmi_internal_power_off();

	HDMI_DRV_LOG("leave hdmi_shutdown\n");
}

static struct platform_driver hdmi_driver = {
	.probe = hdmi_probe,
	.remove = hdmi_remove,
	.shutdown = hdmi_shutdown,
	.driver = {
		   .name = HDMI_DEVNAME,
		   .owner = THIS_MODULE,
		   .of_match_table = hdmi_of_ids,
		   },
};

int hdmi_video_config(HDMI_VIDEO_RESOLUTION vformat)
{
	bool hdmi_video_config = true;
	_stAvdAVInfo.e_resolution = vformat;

	if ((_stAvdAVInfo.e_resolution == hdmi_boot_res) && (_stAvdAVInfo.e_video_color_space == hdmi_boot_colorspace)
		&& (_stAvdAVInfo.e_deep_color_bit == hdmi_boot_colordepth) && ((!vIsDviMode()) == _HdmiSinkAvCap.b_sink_support_hdmi_mode)) {
		TX_DEF_LOG("LK and Kernel is the same resolution/colorspace/deepcolor.\n");
		hdmi_video_config = false;
	}

	hdmi_boot_res = 0xff;
	hdmi_boot_colordepth = 0xff;
	hdmi_boot_colorspace = 0xff;

	if (hdmi_video_config) {
		if (dovi_off_delay_needed) {
			dovi_off_delay_needed = FALSE;
			TX_DEF_LOG("delay 200ms for dolby off case\n");
			usleep_range(200000, 200050);
		}
		TX_DEF_LOG("av mute\n");
		/* av mute packet */
		vSend_AVMUTE();
		usleep_range(50000, 50050);
		/* av mute */
		vHDMIAVMute();
		/* disable encrypt */
		vDisable_HDCP_Encrypt();
		usleep_range(50000, 50050);

		vTmdsOnOffAndResetHdcp(0);
	}
	if (hdmi_hotplugstate != HDMI_STATE_HOT_PLUGIN_AND_POWER_ON) {
		disp_hw_mgr_send_event(DISP_EVENT_PLUG_OUT, NULL);
		switch_set_state(&hdmires_switch_data, 0);
		TX_DEF_LOG("[port]not plugin, ignore hdmi_video_config\n");
		send_fake_hpd = false;
		return 0;
	}

	hdmi_hotplugout_count = 0;
	disp_hw_mgr_send_event(DISP_EVENT_CHANGE_RES, (void *)&vformat);
	switch_set_state(&hdmires_switch_data, (vformat + 1));
	if (hdmi_hotplugout_count != 0) {
		disp_hw_mgr_send_event(DISP_EVENT_PLUG_OUT, NULL);
		switch_set_state(&hdmires_switch_data, 0);
		TX_DEF_LOG("[port]have plugout, ignore hdmi_video_config\n");
		send_fake_hpd = false;
		return 0;
	}
	TX_DEF_LOG("[port]hdmires_switch_data(%d)\n", (vformat + 1));

	if (hdmi_video_config)
		hdmi_drv->video_config(vformat);

	TX_DEF_LOG("hdmi_video_config end\n");
	hdmistate_debug = 0;
	if (send_fake_hpd) {
		switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
		switch_set_state(&hdmires_switch_data, 0);
		usleep_range(50000, 50050);
		switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
		send_fake_hpd = false;
	}
	return 0;
}

int hdmi_force_hdren(HDMI_FORCE_HDR_ENABLE enhdr)
{
	static int prev_hdr_mode = -1;

	if (enhdr == HDMI_FORCE_SDR) {
		hdmi_force_sdr = TRUE;
		if (prev_hdr_mode != enhdr) {
			send_fake_hpd = true;
		}
	} else {
		hdmi_force_sdr = FALSE;
		if (prev_hdr_mode == HDMI_FORCE_SDR) {
			send_fake_hpd = true;
		}
	}
	prev_hdr_mode = enhdr;

	disp_hw_mgr_send_event(DISP_EVENT_FORCE_HDR, (void *)&enhdr);

	return 0;
}

void hdmi_state_callback(HDMI_STATE state)
{
	switch (state) {
	case HDMI_STATE_NO_DEVICE:
		{
			switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
			switch_set_state(&hdmi_audio_switch_data, 0);
			hdmi_audio_event = 0xff;
			switch_set_state(&hdmires_switch_data, 0);
			disp_hw_mgr_send_event(DISP_EVENT_PLUG_OUT, NULL);
			break;
		}

	case HDMI_STATE_ACTIVE:
		{
			switch_set_state(&hdmi_switch_data, HDMI_STATE_ACTIVE);
			switch_set_state(&hdmi_audio_switch_data, 1);
			hdmi_audio_event = 0xff;
			disp_hw_mgr_send_event(DISP_EVENT_PLUG_IN, NULL);
			break;
		}

	case HDMI_STATE_PLUGIN_ONLY:
		{
			switch_set_state(&hdmi_switch_data, HDMI_STATE_NO_DEVICE);
			switch_set_state(&hdmi_audio_switch_data, 0);
			hdmi_audio_event = 0xff;
			disp_hw_mgr_send_event(DISP_EVENT_PLUG_OUT, NULL);
			break;
		}

	case HDMI_STATE_NO_DEVICE_IN_BOOT:
		{
			switch_set_state(&hdmi_audio_switch_data, 0);
			hdmi_audio_event = 0xff;
			break;
		}

	case HDMI_STATE_ACTIVE_IN_BOOT:
		{
			switch_set_state(&hdmi_audio_switch_data, 1);
			hdmi_audio_event = 0xff;
			break;
		}

	case HDMI_STATE_CHANGE_AUDIO_OFF:
		{
			HDMI_PLUG_LOG("[hdmi] HDMI_STATE_CHANGE_AUDIO_OFF\n");
			switch_set_state(&hdmi_audio_switch_data, 0);
			switch_set_state(&hdmi_hdcp_switch_data, 0);
			break;
		}

	case HDMI_STATE_CHANGE_AUDIO_ON:
		{
			HDMI_PLUG_LOG("[hdmi] HDMI_STATE_CHANGE_AUDIO_ON\n");
			switch_set_state(&hdmi_audio_switch_data, 1);
			switch_set_state(&hdmi_hdcp_switch_data, 1);
			break;
		}

	default:
		{
			break;
		}
	}

}

void hdmi_cec_state_callback(HDMI_CEC_STATE state)
{
	switch_set_state(&hdmi_cec_switch_data, 0xff);
	switch (state) {
	case HDMI_CEC_STATE_PLUG_OUT:
		switch_set_state(&hdmi_cec_switch_data, HDMI_CEC_STATE_PLUG_OUT);
		break;
	case HDMI_CEC_STATE_GET_PA:
		switch_set_state(&hdmi_cec_switch_data, HDMI_CEC_STATE_GET_PA);
		break;
	case HDMI_CEC_STATE_TX_STS:
		switch_set_state(&hdmi_cec_switch_data, HDMI_CEC_STATE_TX_STS);
		break;
	case HDMI_CEC_STATE_GET_CMD:
		switch_set_state(&hdmi_cec_switch_data, HDMI_CEC_STATE_GET_CMD);
		break;
	default:
		break;
	}
}

void hdmi_power_on(void)
{
	hdmi_drv->power_on();
#if  (defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) && defined(CONFIG_MTK_HDMI_HDCP_SUPPORT))
	hdmi_drv->enablehdcp(0xa5);
#endif
}

void hdmi_power_off(void)
{
	switch_set_state(&hdmires_switch_data, 0);
	hdmi_drv->power_off();
}

static int hdmi_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int hdmi_open(struct inode *inode, struct file *file)
{
	return 0;
}

bool hdmi_show_ioctl(unsigned int cmd, unsigned long arg)
{
	bool ret = true;

	switch (cmd) {
	case MTK_HDMI_AUDIO_VIDEO_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_AUDIO_VIDEO_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_AUDIO_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_AUDIO_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_VIDEO_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_VIDEO_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_VIDEO_CONFIG:
		TX_DEF_LOG("[ioctl]MTK_HDMI_VIDEO_CONFIG, arg = %ld\n", arg);
		break;
	case MTK_HDMI_POWER_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_POWER_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_AUDIO_SETTING:
		TX_DEF_LOG("[ioctl]MTK_HDMI_AUDIO_SETTING, arg = %ld\n", arg);
		break;
	case MTK_HDMI_FACTORY_MODE_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_FACTORY_MODE_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_FACTORY_GET_STATUS:
		TX_DEF_LOG("[ioctl]MTK_HDMI_FACTORY_GET_STATUS, arg = %ld\n", arg);
		break;
	case MTK_HDMI_CHECK_EDID:
		TX_DEF_LOG("[ioctl]MTK_HDMI_CHECK_EDID, arg = %ld\n", arg);
		break;
	case MTK_HDMI_INFOFRAME_SETTING:
		TX_DEF_LOG("[ioctl]MTK_HDMI_INFOFRAME_SETTING, arg = %ld\n", arg);
		break;
	case MTK_HDMI_COLOR_DEEP:
		TX_DEF_LOG("[ioctl]MTK_HDMI_COLOR_DEEP, arg = %ld\n", arg);
		break;
	case MTK_HDMI_ENABLE_HDCP:
		TX_DEF_LOG("[ioctl]MTK_HDMI_ENABLE_HDCP, arg = %ld\n", arg);
		break;
	case MTK_HDMI_HDCP_KEY:
		TX_DEF_LOG("[ioctl]MTK_HDMI_HDCP_KEY, arg = %ld\n", arg);
		break;
	case MTK_HDMI_GET_EDID:
		TX_DEF_LOG("[ioctl]MTK_HDMI_GET_EDID\n");
		break;
	case MTK_HDMI_VIDEO_MUTE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_VIDEO_MUTE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_HDR_ENABLE:
		TX_DEF_LOG("[ioctl]MTK_HDMI_HDR_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_GET_CAPABILITY:
		TX_DEF_LOG("[ioctl]MTK_HDMI_GET_CAPABILITY, arg = %ld\n", arg);
		break;
	case MTK_HDMI_SETLA:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_SETLA, arg = %ld\n", arg);
		break;
	case MTK_HDMI_GET_CECCMD:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_GET_CECCMD, arg = %ld\n", arg);
		break;
	case MTK_HDMI_SET_CECCMD:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_SET_CECCMD, arg = %ld\n", arg);
		break;
	case MTK_HDMI_CEC_ENABLE:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_CEC_ENABLE, arg = %ld\n", arg);
		break;
	case MTK_HDMI_GET_CECADDR:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_GET_CECADDR, arg = %ld\n", arg);
		break;
	case MTK_HDMI_GET_CECSTS:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_GET_CECSTS, arg = %ld\n", arg);
		break;
	case MTK_HDMI_CEC_USR_CMD:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_CEC_USR_CMD, arg = %ld\n", arg);
		break;
	case MTK_HDMI_CEC_OPTION_SYSTEM_CONTROL:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_CEC_OPTION_SYSTEM_CONTROL, arg = %ld\n", arg);
		break;
	case MTK_HDMI_HDCP_AUTH_STATUS:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_HDCP_AUTH_STATUS, arg = %ld\n", arg);
		break;
	case MTK_HDMI_HPD_ONOFF:
		HDMI_CEC_LOG("[ioctl]MTK_HDMI_HPD_ONOFF, arg = %ld\n", arg);
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}

static long hdmi_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	HDMITX_AUDIO_PARA audio_para;
	HDMI_EDID_T pv_get_info;
	hdmi_para_setting data_info;
	CEC_DRV_ADDR_CFG cecsetAddr;
	CEC_SEND_MSG cecsendframe;
	CEC_FRAME_DESCRIPTION_IO cec_frame;
	CEC_USR_CMD_T cec_usr_cmd;
	APK_CEC_ACK_INFO cec_tx_status;
	CEC_ADDRESS_IO cecaddr;
	HDCP_INFO hdcp_information;
	int r = 0;
#if (defined(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) && defined(CONFIG_MTK_HDMI_HDCP_SUPPORT)\
		&& defined(CONFIG_MTK_DRM_KEY_MNG_SUPPORT))
		hdmi_hdcp_drmkey key;
#else
		hdmi_hdcp_key key;
#endif

	if (!hdmi_show_ioctl(cmd, arg))
		HDMI_DRV_LOG(">> hdmi_ioctl: %d\n", cmd);

	switch (cmd) {
	case MTK_HDMI_AUDIO_VIDEO_ENABLE:
		{
			HDMI_DRV_LOG(">> MTK_HDMI_AUDIO_VIDEO_ENABLE arg = %ld\n", arg);
			if (arg) {
				hdmi_power_on();
			} else
				hdmi_power_off();
			break;
		}
	case MTK_HDMI_AUDIO_SETTING:
		{
			if (copy_from_user(&audio_para, (void __user *)arg, sizeof(audio_para)))
				r = -EFAULT;
			else
				hdmi_drv->audiosetting(&audio_para);

			/*hdmi_audio_setting( struct HDMITX_AUDIO_PARA arg); */
			break;
		}
	case MTK_HDMI_WRITE_DEV:
		{
			break;
		}

	case MTK_HDMI_HDCP_INFO:
		{
			if (copy_from_user(&hdcp_information, (void __user *)arg, sizeof(hdcp_information))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			} else {
				hdmi_drv->hdcp_info(&hdcp_information);
			}
			if (copy_to_user((void __user *)arg, &hdcp_information, sizeof(hdcp_information))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}


	case MTK_HDMI_INFOFRAME_SETTING:
		{
			break;
		}

	case MTK_HDMI_HDCP_KEY:
		{
			TX_DEF_LOG("[HDCP] MTK_HDMI_HDCP_KEY! line:%d\n", __LINE__);
			if (copy_from_user(&key, (void __user *)arg, sizeof(key))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			} else {
				hdmi_drv->hdcpkey((unsigned char *) &key);
			}
			break;
		}

	case MTK_HDMI_SETLA:
		{
			if (copy_from_user(&cecsetAddr, (void __user *)arg, sizeof(cecsetAddr))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			} else {
				hdmi_drv->setcecla(&cecsetAddr);
			}
			break;
		}

	case MTK_HDMI_SENDSLTDATA:
		{
			break;
		}

	case MTK_HDMI_SET_CECCMD:
		{
			if (copy_from_user(&cecsendframe, (void __user *)arg, sizeof(cecsendframe))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			} else {
				hdmi_drv->setceccmd(&cecsendframe);
				if (copy_to_user((void __user *)arg, &cecsendframe, sizeof(cecsendframe))) {
					TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
					r = -EFAULT;
				}
			}
			break;
		}

	case MTK_HDMI_CEC_ENABLE:
		{
			hdmi_drv->cecenable(arg & 0xFF);
			break;
		}

	case MTK_HDMI_GET_CECCMD:
		{
			hdmi_drv->getceccmd(&cec_frame);
			if (copy_to_user((void __user *)arg, &cec_frame, sizeof(cec_frame))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}

	case MTK_HDMI_GET_CECSTS:
		{
			hdmi_drv->getcectxstatus(&cec_tx_status);
			if (copy_to_user((void __user *)arg, &cec_tx_status, sizeof(APK_CEC_ACK_INFO))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}

	case MTK_HDMI_CEC_USR_CMD:
		{
			if (copy_from_user(&cec_usr_cmd, (void __user *)arg, sizeof(CEC_USR_CMD_T)))
				r = -EFAULT;
			else
				hdmi_drv->cecusrcmd(cec_usr_cmd.cmd, &(cec_usr_cmd.result));

			if (copy_to_user((void __user *)arg, &cec_usr_cmd, sizeof(CEC_USR_CMD_T))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}

	case MTK_HDMI_GET_SLTDATA:
		{
			break;
		}

	case MTK_HDMI_GET_CECADDR:
		{
			hdmi_drv->getcecaddr(&cecaddr);
			if (copy_to_user((void __user *)arg, &cecaddr, sizeof(cecaddr))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}

	case MTK_HDMI_COLOR_DEEP:
		{
			if (copy_from_user(&data_info, (void __user *)arg, sizeof(data_info))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			} else {

				TX_DEF_LOG("MTK_HDMI_COLOR_DEEP: %d %d\n", data_info.u4Data1,
					   data_info.u4Data2);

				hdmi_drv->colordeep(data_info.u4Data1 & 0xFF,
						    data_info.u4Data2 & 0xFF);
			}
			break;
		}

	case MTK_HDMI_READ_DEV:
		{
			break;
		}

	case MTK_HDMI_ENABLE_LOG:
		{
			break;
		}

	case MTK_HDMI_ENABLE_HDCP:
		{
			break;
		}

	case MTK_HDMI_CECRX_MODE:
		{
			break;
		}

	case MTK_HDMI_STATUS:
		{
			break;
		}

	case MTK_HDMI_CHECK_EDID:
		{
			break;
		}

	case MTK_HDMI_POWER_ENABLE:
		{
			break;
		}

	case MTK_HDMI_VIDEO_CONFIG:
		{
			r = hdmi_video_config(arg);
			break;
		}

	case MTK_HDMI_HDR_ENABLE:
		{
			r = hdmi_force_hdren(arg);
			break;
		}

	case MTK_HDMI_FACTORY_GET_STATUS:
		{
			int hdmi_status;

			/* check_res:
			* 0:HDMI Plug in;
			* 1 HDMI Plug in,EDID OK;
			* -1: No HDMI Plug in;
			* -2: DPI Clock OFF;
			* -3: HDMI Plug in, EDID Error ;
			* -4: cec error;
			*/
			if (hdmi_drv->get_state() == HDMI_STATE_NO_DEVICE) {
				TX_DEF_LOG("[hdmi]TV disconnected\n");
				hdmi_status = -1;
			} else if (hdmi_drv->get_state() == HDMI_STATE_PLUGIN_ONLY) {
				TX_DEF_LOG("[hdmi]TV plug in only\n");
				hdmi_status = 0;
			} else if (hdmi_drv->checkedidheader() == FALSE) {
				TX_DEF_LOG("[hdmi]edid error\n");
				hdmi_status = -3;
			} else if (!hdmi_cec_factory_test()) {
				TX_DEF_LOG("[hdmi]cec error\n");
				hdmi_status = -4;
			} else
				hdmi_status = 1;

			TX_DEF_LOG("MTK_HDMI_FACTORY_GET_STATUS is %d\n", hdmi_status);
			if (copy_to_user((void __user *)arg, &hdmi_status, sizeof(hdmi_status))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				r = -EFAULT;
			}
			break;
		}

	case MTK_HDMI_GET_EDID:
		{
			memset(&pv_get_info, 0, sizeof(pv_get_info));
			if (hdmi_drv->getedid)
				hdmi_drv->getedid(&pv_get_info);

			if (copy_to_user((void __user *)arg, &pv_get_info, sizeof(pv_get_info)))
				r = -EFAULT;
			/*app_get_edid( struct HDMI_EDID_T arg); */
			break;
		}

	case MTK_HDMI_HDCP_AUTH_STATUS:
	   {
			int hdcp_auth_status = -1;
			/* convert _bHdcpStatue return value to hwcec value (1:auth ok, 0:auth fail)
			    #define SV_OK	     (unsigned char)(0)
				#define SV_FAIL      (unsigned char)(-1) */
			if (!_bHdcpStatus)
				hdcp_auth_status = 1;
			else if (_bHdcpStatus == 0xFF)
				hdcp_auth_status = 0;
			else {
				HDMI_DRV_LOG("MTK_HDMI_HDCP_AUTH_STATUS unknown state");
				hdcp_auth_status = -1;
			}
			HDMI_DRV_LOG("_bHdcpStatus:%d hdcp_auth_status:%d\n", _bHdcpStatus, hdcp_auth_status);
			if (copy_to_user((void __user *)arg, &hdcp_auth_status, sizeof(hdcp_auth_status))) {
				HDMI_DRV_LOG("copy_to_user failed! line:%d \n", __LINE__);
				r = -EFAULT;
			}
			break;
	    }

	case MTK_HDMI_HPD_ONOFF:
	    {
			int hdmi_hpd_onoff = -1;

			switch (hdmi_hotplugstate) {
			case HDMI_STATE_HOT_PLUG_OUT:
				hdmi_hpd_onoff = 0;
				break;
			case HDMI_STATE_HOT_PLUG_IN_ONLY:
				hdmi_hpd_onoff = 0;
				break;
			case HDMI_STATE_HOT_PLUGIN_AND_POWER_ON:
				hdmi_hpd_onoff = 1;
				break;
			default:
				HDMI_DRV_LOG("MTK_HDMI_HPD_ONOFF unknown state");
				break;
			}

			if (copy_to_user((void __user *)arg, &hdmi_hpd_onoff, sizeof(hdmi_hpd_onoff))) {
				HDMI_DRV_LOG("copy_to_user failed! line:%d \n", __LINE__);
				r = -EFAULT;
			}
			break;
	    }

	case MTK_HDMI_GET_CAPABILITY:
		{
			int ret = 0;
			int query_type = 0;

			HDMI_DRV_LOG(">> MTK_HDMI_GET_CAPABILITY arg = %ld\n", arg);
			query_type |= (HDMI_FACTORY_MODE_NEW | HDMI_FACTORY_TEST_BOX | HDMI_FACTORY_TEST_HDCP);

			if (copy_to_user((void __user *)arg, &query_type, sizeof(query_type)))
				ret = -EFAULT;
			break;
		}
	case MTK_HDMI_FACTORY_MODE_ENABLE:
		{
			HDMI_DRV_LOG(">> MTK_HDMI_FACTORY_MODE_ENABLE arg = %ld\n", arg);
			break;
		}
	case MTK_HDMI_FACTORY_CHIP_INIT:
		{
			HDMI_DRV_LOG(">> MTK_HDMI_FACTORY_CHIP_INIT arg = %ld\n", arg);
			break;
		}
	case MTK_HDMI_AUDIO_ENABLE:
		{
			HDMI_DRV_LOG(">> MTK_HDMI_AUDIO_ENABLE arg = %ld\n", arg);
			break;
		}
	case MTK_HDMI_CEC_OPTION_SYSTEM_CONTROL:
		{
			if (arg & 0xFF)
				hdmi_drv->setcecrxmode(CEC_NORMAL_MODE);
			else
				hdmi_drv->setcecrxmode(CEC_KER_HANDLE_MODE);
			break;
		}
	default:
		{
			r = -EFAULT;
			TX_DEF_LOG("Unknown HDMI ioctl cmd: 0x%x\n", cmd);
			break;
		}
	}

	return r;
}

#if IS_ENABLED(CONFIG_COMPAT)
#if 0
static int compat_get_hdmitx_audio_para(COMPAT_HDMITX_AUDIO_PARA __user *data32,
					HDMITX_AUDIO_PARA __user *data)
{
	unsigned char c;
	int err;

	err = get_user(c, &data32->e_hdmi_aud_in);
	err |= put_user(c, &data->e_hdmi_aud_in);

	err |= get_user(c, &data32->e_iec_frame);
	err |= put_user(c, &data->e_iec_frame);

	err |= get_user(c, &data32->e_hdmi_fs);
	err |= put_user(c, &data->e_hdmi_fs);

	err |= get_user(c, &data32->e_aud_code);
	err |= put_user(c, &data->e_aud_code);

	err = get_user(c, &data32->u1Aud_Input_Chan_Cnt);
	err |= put_user(c, &data->u1Aud_Input_Chan_Cnt);

	err |= get_user(c, &data32->e_I2sFmt);
	err |= put_user(c, &data->e_I2sFmt);

	err |= get_user(c, &data32->u1HdmiI2sMclk);
	err |= put_user(c, &data->u1HdmiI2sMclk);

	err |= get_user(c, &data32->bhdmi_LCh_status);
	err |= put_user(c, &data->bhdmi_LCh_status);

	return err;
}
#endif

#if 0
static int compat_put_edid(struct COMPAT_HDMI_EDID_T __user *data32, HDMI_EDID_T __user *data)
{
	compat_uint_t u;
	int err;

	err = get_user(u, &data->ui4_ntsc_resolution);
	err |= put_user(u, &data32->ui4_ntsc_resolution);

	err |= get_user(u, &data->ui4_pal_resolution);
	err |= put_user(u, &data32->ui4_pal_resolution);

	err |= get_user(u, &data->ui4_sink_native_ntsc_resolution);
	err |= put_user(u, &data32->ui4_sink_native_ntsc_resolution);

	err |= get_user(u, &data->ui4_sink_native_pal_resolution);
	err |= put_user(u, &data32->ui4_sink_native_pal_resolution);

	err |= get_user(u, &data->ui4_sink_cea_ntsc_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_ntsc_resolution);

	err |= get_user(u, &data->ui4_sink_cea_pal_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_pal_resolution);

	err |= get_user(u, &data->ui4_sink_dtd_ntsc_resolution);
	err |= put_user(u, &data32->ui4_sink_dtd_ntsc_resolution);

	err |= get_user(u, &data->ui4_sink_dtd_pal_resolution);
	err |= put_user(u, &data32->ui4_sink_dtd_pal_resolution);

	err |= get_user(u, &data->ui4_sink_1st_dtd_ntsc_resolution);
	err |= put_user(u, &data32->ui4_sink_1st_dtd_ntsc_resolution);

	err |= get_user(u, &data->ui4_sink_1st_dtd_pal_resolution);
	err |= put_user(u, &data32->ui4_sink_1st_dtd_pal_resolution);

	err |= get_user(u, &data->ui2_sink_colorimetry);
	err |= put_user(u, &data32->ui2_sink_colorimetry);

	err |= get_user(u, &data->ui1_sink_rgb_color_bit);
	err |= put_user(u, &data32->ui1_sink_rgb_color_bit);

	err |= get_user(u, &data->ui1_sink_ycbcr_color_bit);
	err |= put_user(u, &data32->ui1_sink_ycbcr_color_bit);

	err |= get_user(u, &data->ui1_sink_dc420_color_bit);
	err |= put_user(u, &data32->ui1_sink_dc420_color_bit);

	err |= get_user(u, &data->ui2_sink_aud_dec);
	err |= put_user(u, &data32->ui2_sink_aud_dec);

	err |= get_user(u, &data->ui1_sink_is_plug_in);
	err |= put_user(u, &data32->ui1_sink_is_plug_in);

	err |= get_user(u, &data->ui4_hdmi_pcm_ch_type);
	err |= put_user(u, &data32->ui4_hdmi_pcm_ch_type);

	err |= get_user(u, &data->ui4_hdmi_pcm_ch3ch4ch5ch7_type);
	err |= put_user(u, &data32->ui4_hdmi_pcm_ch3ch4ch5ch7_type);

	err |= get_user(u, &data->ui4_hdmi_ac3_ch_type);
	err |= put_user(u, &data32->ui4_hdmi_ac3_ch_type);

	err |= get_user(u, &data->ui4_hdmi_ac3_ch3ch4ch5ch7_type);
	err |= put_user(u, &data32->ui4_hdmi_ac3_ch3ch4ch5ch7_type);

	err |= get_user(u, &data->ui4_hdmi_ec3_ch_type);
	err |= put_user(u, &data32->ui4_hdmi_ec3_ch_type);

	err |= get_user(u, &data->ui4_hdmi_ec3_ch3ch4ch5ch7_type);
	err |= put_user(u, &data32->ui4_hdmi_ec3_ch3ch4ch5ch7_type);

	err |= get_user(u, &data->ui4_hdmi_dts_ch_type);
	err |= put_user(u, &data32->ui4_hdmi_dts_ch_type);

	err |= get_user(u, &data->ui4_hdmi_dts_ch3ch4ch5ch7_type);
	err |= put_user(u, &data32->ui4_hdmi_dts_ch3ch4ch5ch7_type);

	err |= get_user(u, &data->ui4_hdmi_dts_hd_ch_type);
	err |= put_user(u, &data32->ui4_hdmi_dts_hd_ch_type);

	err |= get_user(u, &data->ui4_hdmi_dts_hd_ch3ch4ch5ch7_type);
	err |= put_user(u, &data32->ui4_hdmi_dts_hd_ch3ch4ch5ch7_type);

	err |= get_user(u, &data->ui4_dac_pcm_ch_type);
	err |= put_user(u, &data32->ui4_dac_pcm_ch_type);

	err |= get_user(u, &data->ui1_sink_i_latency_present);
	err |= put_user(u, &data32->ui1_sink_i_latency_present);

	err |= get_user(u, &data->ui1_sink_p_audio_latency);
	err |= put_user(u, &data32->ui1_sink_p_audio_latency);

	err |= get_user(u, &data->ui1_sink_p_video_latency);
	err |= put_user(u, &data32->ui1_sink_p_video_latency);

	err |= get_user(u, &data->ui1_sink_i_audio_latency);
	err |= put_user(u, &data32->ui1_sink_i_audio_latency);

	err |= get_user(u, &data->ui1_sink_i_video_latency);
	err |= put_user(u, &data32->ui1_sink_i_video_latency);

	err |= get_user(u, &data->ui1ExtEdid_Revision);
	err |= put_user(u, &data32->ui1ExtEdid_Revision);

	err |= get_user(u, &data->ui1Edid_Version);
	err |= put_user(u, &data32->ui1Edid_Version);

	err |= get_user(u, &data->ui1Edid_Revision);
	err |= put_user(u, &data32->ui1Edid_Revision);

	err |= get_user(u, &data->ui1_Display_Horizontal_Size);
	err |= put_user(u, &data32->ui1_Display_Horizontal_Size);

	err |= get_user(u, &data->ui1_Display_Vertical_Size);
	err |= put_user(u, &data32->ui1_Display_Vertical_Size);

	err |= get_user(u, &data->ui4_ID_Serial_Number);
	err |= put_user(u, &data32->ui4_ID_Serial_Number);

	err |= get_user(u, &data->ui4_sink_cea_3D_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_3D_resolution);

	err |= get_user(u, &data->ui1_sink_support_ai);
	err |= put_user(u, &data32->ui1_sink_support_ai);

	err |= get_user(u, &data->ui2_sink_cec_address);
	err |= put_user(u, &data32->ui2_sink_cec_address);

	err |= get_user(u, &data->ui1_sink_max_tmds_clock);
	err |= put_user(u, &data32->ui1_sink_max_tmds_clock);

	err |= get_user(u, &data->ui2_sink_3D_structure);
	err |= put_user(u, &data32->ui2_sink_3D_structure);

	err |= get_user(u, &data->ui4_sink_cea_FP_SUP_3D_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_FP_SUP_3D_resolution);

	err |= get_user(u, &data->ui4_sink_cea_TOB_SUP_3D_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_TOB_SUP_3D_resolution);

	err |= get_user(u, &data->ui4_sink_cea_SBS_SUP_3D_resolution);
	err |= put_user(u, &data32->ui4_sink_cea_SBS_SUP_3D_resolution);

	err |= get_user(u, &data->ui2_sink_ID_manufacturer_name);
	err |= put_user(u, &data32->ui2_sink_ID_manufacturer_name);

	err |= get_user(u, &data->ui2_sink_ID_product_code);
	err |= put_user(u, &data32->ui2_sink_ID_product_code);

	err |= get_user(u, &data->ui4_sink_ID_serial_number);
	err |= put_user(u, &data32->ui4_sink_ID_serial_number);

	err |= get_user(u, &data->ui1_sink_week_of_manufacture);
	err |= put_user(u, &data32->ui1_sink_week_of_manufacture);

	err |= get_user(u, &data->ui1_sink_year_of_manufacture);
	err |= put_user(u, &data32->ui1_sink_year_of_manufacture);

	err |= get_user(u, &data->b_sink_SCDC_present);
	err |= put_user(u, &data32->b_sink_SCDC_present);

	err |= get_user(u, &data->b_sink_LTE_340M_sramble);
	err |= put_user(u, &data32->b_sink_LTE_340M_sramble);

	err |= get_user(u, &data->ui4_sink_hdmi_4k2kvic);
	err |= put_user(u, &data32->ui4_sink_hdmi_4k2kvic);

	err |= get_user(u, &data->ui1_sink_support_static_hdr);
	err |= put_user(u, &data32->ui1_sink_support_static_hdr);

	err |= get_user(u, &data->ui1_sink_support_dynamic_hdr);
	err |= put_user(u, &data32->ui1_sink_support_dynamic_hdr);

	err |= get_user(u, &data->ui1_sink_hdr10plus_app_version);
	err |= put_user(u, &data32->ui1_sink_hdr10plus_app_version);

	err |= get_user(u, &data->ui1_sink_dolbyvision_block[32]);
	err |= put_user(u, &data32->ui1_sink_dolbyvision_block[32]);

	err |= get_user(u, &data->ui1rawdata_edid[EDID_LENGTH]);
	err |= put_user(u, &data32->ui1rawdata_edid[EDID_LENGTH]);

	return err;
}
#endif

static long hdmi_ioctl_compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	HDMI_DRV_LOG(">> hdmi_ioctl_compat: 0x%x\n", cmd);

	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;

	switch (cmd) {
	case COMPAT_MTK_HDMI_AUDIO_SETTING:
		{
			struct COMPAT_HDMITX_AUDIO_PARA __user *data32;	/* userspace passed argument */
			HDMITX_AUDIO_PARA __user *data;	/* kernel used */

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_AUDIO_SETTING\n");
			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(*data));

			if (data == NULL)
				return -EFAULT;

			/*For hdmi_audio_setting, unsigned char type dont need to be convert */
			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_AUDIO_SETTING,
						       (unsigned long)data32);
			return ret;
		}
#if 0
	case COMPAT_MTK_HDMI_GET_EDID:
		{
			struct COMPAT_HDMI_EDID_T __user *data32;
			HDMI_EDID_T __user *data;
			int err = 0;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_GET_EDID\n");
			data32 = compat_ptr(arg);	/* userspace passed argument */
			data = compat_alloc_user_space(sizeof(*data));

			if (data == NULL)
				return -EFAULT;

			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_GET_EDID,
						       (unsigned long)data);
			err = compat_put_edid(data32, data);

			return ret ? ret : err;
		}
#endif
	case COMPAT_MTK_HDMI_AUDIO_VIDEO_ENABLE:
		{
			int __user *data32;	/* userspace passed argument */

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_AUDIO_VIDEO_ENABLE %ld\n", arg);
			data32 = compat_ptr(arg);

			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_AUDIO_VIDEO_ENABLE,
						       (unsigned long)data32);
			break;
		}
	case COMPAT_MTK_HDMI_WRITE_DEV:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_WRITE_DEV\n");
			break;
		}

	case COMPAT_MTK_HDMI_INFOFRAME_SETTING:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_INFOFRAME_SETTING\n");
			break;
		}


	case COMPAT_MTK_HDMI_HDCP_KEY:
		{
			TX_DEF_LOG("COMPAT_MTK_HDMI_HDCP_KEY! line:%d\n", __LINE__);
			break;
		}

	case COMPAT_MTK_HDMI_SETLA:
		{
			CEC_DRV_ADDR_CFG __user *data32;

			data32 = compat_ptr(arg);
			ret = file->f_op->unlocked_ioctl(file, MTK_HDMI_SETLA, (unsigned long)data32);
			break;
		}

	case COMPAT_MTK_HDMI_SENDSLTDATA:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_SENDSLTDATA\n");
			break;
		}


	case COMPAT_MTK_HDMI_SET_CECCMD:
		{
			COMPAT_CEC_SEND_MSG __user *arg_u2k = compat_ptr(arg);
			COMPAT_CEC_SEND_MSG __user *param_u2k =
				compat_alloc_user_space(sizeof(*param_u2k));
			CEC_SEND_MSG __user *cecsendframe =
				compat_alloc_user_space(sizeof(*cecsendframe));

			if (copy_from_user(param_u2k, arg_u2k, sizeof(*param_u2k))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				ret = -EFAULT;
			} else {
				cecsendframe->pv_tag = (void *)(unsigned long)(param_u2k->pv_tag);
				memcpy(&cecsendframe->t_frame_info, &param_u2k->t_frame_info,
					sizeof(param_u2k->t_frame_info));
				cecsendframe->b_enqueue_ok = param_u2k->b_enqueue_ok;
				ret = file->f_op->unlocked_ioctl(file,
					MTK_HDMI_SET_CECCMD, (unsigned long)cecsendframe);
			}
			break;
		}

	case COMPAT_MTK_HDMI_CEC_ENABLE:
		{
			unsigned int __user *arg_u2k = compat_ptr(arg);

			ret = file->f_op->unlocked_ioctl(file,
				MTK_HDMI_CEC_ENABLE, (unsigned long)arg_u2k);
			break;
		}

	case COMPAT_MTK_HDMI_GET_CECCMD:
		{
			COMPAT_CEC_FRAME_DESCRIPTION_IO __user *arg_u2k;
			CEC_FRAME_DESCRIPTION_IO __user *param_u2k;
			CEC_FRAME_BLOCK_IO block;
			compat_uint_t u;
			compat_uptr_t p;
			char c;
			void *ptr;
			int err = 0;

			arg_u2k = compat_ptr(arg);
			param_u2k = compat_alloc_user_space(sizeof(*param_u2k));
			err |= get_user(c, &(arg_u2k->size));
			err |= put_user(c, &(param_u2k->size));
			err |= get_user(c, &(arg_u2k->sendidx));
			err |= put_user(c, &(param_u2k->sendidx));
			err |= get_user(c, &(arg_u2k->reTXcnt));
			err |= put_user(c, &(param_u2k->reTXcnt));
			err |= get_user(p, &(arg_u2k->txtag));
			err |= put_user(compat_ptr(p), &(param_u2k->txtag));
			if (copy_from_user(&block, &(arg_u2k->blocks), sizeof(CEC_FRAME_BLOCK_IO))) {
				TX_DEF_LOG("copy_from_user failed! line:%d\n", __LINE__);
				ret = -EFAULT;
			}
			if (copy_to_user((void __user *)(&(param_u2k->blocks)),
				&block, sizeof(CEC_FRAME_BLOCK_IO))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				ret = -EFAULT;
			}
			ret = file->f_op->unlocked_ioctl(file,
				MTK_HDMI_GET_CECCMD, (unsigned long)param_u2k);
			err |= get_user(u, &(param_u2k->size));
			err |= put_user(u, &(arg_u2k->size));
			err |= get_user(u, &(param_u2k->sendidx));
			err |= put_user(u, &(arg_u2k->sendidx));
			err |= get_user(u, &(param_u2k->reTXcnt));
			err |= put_user(u, &(arg_u2k->reTXcnt));
			err |= get_user(ptr, &(param_u2k->txtag));
			err |= put_user(ptr_to_compat(ptr), &(arg_u2k->txtag));
			if (copy_in_user(&(arg_u2k->blocks), &(param_u2k->blocks),
				sizeof(CEC_FRAME_BLOCK_IO))) {
				TX_DEF_LOG("copy_in_user failed! line:%d\n", __LINE__);
				ret = -EFAULT;
			}
			break;
		}

	case COMPAT_MTK_HDMI_GET_CECSTS:
		{
			COMPAT_APK_CEC_ACK_INFO __user *arg_u2k;
			APK_CEC_ACK_INFO __user *param_u2k;
			compat_uint_t u;
			compat_uptr_t p;
			void *ptr;
			int err = 0;

			arg_u2k = compat_ptr(arg);
			param_u2k = compat_alloc_user_space(sizeof(APK_CEC_ACK_INFO));
			err |= get_user(p, &(arg_u2k->pv_tag));
			err |= put_user(compat_ptr(p), &(param_u2k->pv_tag));
			err |= get_user(u, &(arg_u2k->e_ack_cond));
			err |= put_user(u, &(param_u2k->e_ack_cond));
			ret = file->f_op->unlocked_ioctl(file,
				MTK_HDMI_GET_CECSTS, (unsigned long)param_u2k);
			err |= get_user(ptr, &(param_u2k->pv_tag));
			err |= put_user(ptr_to_compat(ptr), &(arg_u2k->pv_tag));
			err |= get_user(u, &(param_u2k->e_ack_cond));
			err |= put_user(u, &(arg_u2k->e_ack_cond));
			break;
		}

	case COMPAT_MTK_HDMI_CEC_USR_CMD:
		{
			CEC_USR_CMD_T __user *data32;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_CEC_USR_CMD %ld\n", arg);
			data32 = compat_ptr(arg);
			ret = file->f_op->unlocked_ioctl(file,
				MTK_HDMI_CEC_USR_CMD, (unsigned long)data32);
			break;
		}

	case COMPAT_MTK_HDMI_GET_SLTDATA:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_GET_SLTDATA\n");
			break;
		}

	case COMPAT_MTK_HDMI_GET_CECADDR:
		{
			CEC_ADDRESS_IO __user *data32;
			CEC_ADDRESS_IO __user *data;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_GET_CECADDR %ld\n", arg);
			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(*data));
			ret = file->f_op->unlocked_ioctl(file,
				MTK_HDMI_GET_CECADDR, (unsigned long)data);
			if (copy_to_user((void __user *)data32, data, sizeof(*data))) {
				TX_DEF_LOG("copy_to_user failed! line:%d\n", __LINE__);
				ret = -EFAULT;
			}
			break;
		}

	case COMPAT_MTK_HDMI_COLOR_DEEP:
		{
			struct compat_hdmi_para_setting __user *data32;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_COLOR_DEEP\n");
			data32 = compat_ptr(arg);	/* userspace passed argument */

			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_COLOR_DEEP,
						       (unsigned long)data32);
			break;
		}

	case COMPAT_MTK_HDMI_HDR_ENABLE:
		{
			struct compat_hdmi_force_hdr_enable __user *data32;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_HDR_ENABLE\n");
			data32 = compat_ptr(arg);	/* userspace passed argument */

			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_HDR_ENABLE,
						       (unsigned long)data32);
			break;
		}


	case COMPAT_MTK_HDMI_READ_DEV:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_READ_DEV\n");
			break;
		}

	case COMPAT_MTK_HDMI_ENABLE_LOG:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_ENABLE_LOG\n");
			break;
		}

	case COMPAT_MTK_HDMI_ENABLE_HDCP:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_ENABLE_HDCP\n");
			break;
		}

	case COMPAT_MTK_HDMI_CECRX_MODE:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_CECRX_MODE\n");
			break;
		}

	case COMPAT_MTK_HDMI_STATUS:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_STATUS\n");
			break;
		}

	case COMPAT_MTK_HDMI_CHECK_EDID:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_CHECK_EDID\n");
			break;
		}

	case COMPAT_MTK_HDMI_POWER_ENABLE:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_POWER_ENABLE\n");
			break;
		}

	case COMPAT_MTK_HDMI_VIDEO_CONFIG:
		{
			int __user *data32;

			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_VIDEO_CONFIG arg = %ld\n", arg);
			data32 = compat_ptr(arg);	/* userspace passed argument */

			ret =
			    file->f_op->unlocked_ioctl(file, MTK_HDMI_VIDEO_CONFIG,
						       (unsigned long)data32);
			break;
		}

	case COMPAT_MTK_HDMI_FACTORY_GET_STATUS:
		{
			HDMI_DRV_LOG(">> COMPAT_MTK_HDMI_FACTORY_GET_STATUS\n");
			break;

		}

	default:
	        HDMI_DRV_LOG(">> calling default cmd=0x%x\n", cmd);
		hdmi_ioctl(file, cmd, arg);
		break;

	}
	return ret;
}
#endif

bool hdmi_suspend_en = 0;
static ssize_t hdmi_suspend_show_enable(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	return sprintf(buf, "%d\n", hdmi_suspend_en);
}

static ssize_t hdmi_suspend_store_enable(struct device *dev,
				      struct device_attribute *devattr,
				      const char *buf,
				      size_t count)
{
	if (!strncmp(buf, "1", 1)) {
		hdmi_suspend_en = 1;
		pr_info("enable hdmi suspend\n");
	}
	else if(!strncmp(buf, "0", 1)) {
		hdmi_suspend_en = 0;
		pr_info("disable hdmi suspend\n");
	}
	return count;
}

char hdmi_debug_normal_buffer[256];

static ssize_t hdmi_debug_normal_show(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	TX_DEF_LOG("%s buffer:\n%s\n", __func__, hdmi_debug_normal_buffer);
	return sprintf(buf, "%s\n", hdmi_debug_normal_buffer);
}

static ssize_t hdmi_debug_normal_store(struct device *dev,
				      struct device_attribute *devattr,
				      const char *buf,
				      size_t count)
{
	int ret, arg;

	HDMI_DRV_FUNC();
	ret = count;
	if (strncmp(buf, "dbglevel:", 9) == 0) {
		memset(hdmi_debug_normal_buffer, 0, sizeof(hdmi_debug_normal_buffer));
		ret = sscanf(buf, "dbglevel:0x%x", &arg);
		if (ret == 1) {
			sprintf(hdmi_debug_normal_buffer, "log old=0x%x\n", hdmidrv_log_on);
			hdmi_drvlog_enable(arg);
			sprintf(hdmi_debug_normal_buffer, "%slog new=0x%x\n",
				hdmi_debug_normal_buffer, hdmidrv_log_on);
		} else {
			sprintf(hdmi_debug_normal_buffer, "error,buf=%s\n", buf);
			TX_DEF_LOG("%s,%d,%s\n", __func__, __LINE__, hdmi_debug_normal_buffer);
			ret = -EINVAL;
		}
	} else {
		TX_DEF_LOG("%s,%d,not handle str:%s\n", __func__, __LINE__, buf);
		ret = -EINVAL;
	}

	if (ret != -EINVAL)
		return count;

	return ret;
}

char hdmi_hdr_status_buffer[256];

static ssize_t hdmi_hdr_status_show(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	int ret;

	memset(hdmi_hdr_status_buffer, 0, sizeof(hdmi_hdr_status_buffer));
	ret = hdr_status(hdmi_hdr_status_buffer);
	sprintf(hdmi_hdr_status_buffer, "%s\nret=%d\n",
		hdmi_hdr_status_buffer, ret);
	TX_DEF_LOG("%s,%d,\n%s\n", __func__, __LINE__, hdmi_hdr_status_buffer);

	return sprintf(buf, "%s\n", hdmi_hdr_status_buffer);
}

static ssize_t hdmi_hdr_status_store(struct device *dev,
				      struct device_attribute *devattr,
				      const char *buf,
				      size_t count)
{
	return count;
}

char hdmi_common_status_buffer[256];

static ssize_t hdmi_common_status_show(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	memset(hdmi_common_status_buffer, 0, sizeof(hdmi_common_status_buffer));
	vcommon_status(hdmi_common_status_buffer);
	TX_DEF_LOG("%s,%d,\n%s\n", __func__, __LINE__, hdmi_common_status_buffer);

	return sprintf(buf, "%s\n", hdmi_common_status_buffer);
}

static ssize_t hdmi_common_status_store(struct device *dev,
				      struct device_attribute *devattr,
				      const char *buf,
				      size_t count)
{
	return count;
}

static DEVICE_ATTR(hdmi_suspend_enable, S_IRUGO | S_IWUSR, hdmi_suspend_show_enable, hdmi_suspend_store_enable);
static DEVICE_ATTR(hdmi_debug_normal, S_IRUGO | S_IWUSR, hdmi_debug_normal_show, hdmi_debug_normal_store);
static DEVICE_ATTR(hdmi_hdr_status, S_IRUGO | S_IWUSR, hdmi_hdr_status_show, hdmi_hdr_status_store);
static DEVICE_ATTR(hdmi_common_status, S_IRUGO | S_IWUSR, hdmi_common_status_show, hdmi_common_status_store);


static int hdmi_remove(struct platform_device *pdev)
{
	HDMI_DRV_LOG("hdmi_remove\n");
	return 0;
}

static void hdmi_udelay(unsigned int us)
{
	udelay(us);
}

static void hdmi_mdelay(unsigned int ms)
{
	msleep(ms);
}

static bool hdmi_drv_init_context(void)
{
	static const HDMI_UTIL_FUNCS hdmi_utils = {
		.udelay = hdmi_udelay,
		.mdelay = hdmi_mdelay,
		.state_callback = hdmi_state_callback,
		.cec_state_callback = hdmi_cec_state_callback,
	};

	if (hdmi_drv != NULL)
		return TRUE;

	hdmi_drv = (HDMI_DRIVER *) HDMI_GetDriver();

	if (hdmi_drv == NULL)
		return FALSE;

	hdmi_drv->set_util_funcs(&hdmi_utils);

	return TRUE;
}

static void __exit hdmi_exit(void)
{
	device_destroy(hdmi_class, hdmi_devno);
	class_destroy(hdmi_class);
	cdev_del(hdmi_cdev);
	unregister_chrdev_region(hdmi_devno, 1);
}

static int hdmi_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct class_device *class_dev = NULL;
	struct mtk_hdmi *hdmi;

	/* Allocate device number for hdmi driver */
	ret = alloc_chrdev_region(&hdmi_devno, 0, 1, HDMI_DEVNAME);

	if (ret) {
		TX_DEF_LOG("alloc_chrdev_region fail\n");
		return -1;
	}

	/* For character driver register to system, device number binded to file operations */
	hdmi_cdev = cdev_alloc();
	hdmi_cdev->owner = THIS_MODULE;
	hdmi_cdev->ops = &hdmi_fops;
	ret = cdev_add(hdmi_cdev, hdmi_devno, 1);

	/* For device number binded to device name(hdmitx), one class is corresponeded to one node */
	hdmi_class = class_create(THIS_MODULE, HDMI_DEVNAME);
	/* mknod /dev/hdmitx */
	class_dev =
	    (struct class_device *)device_create(hdmi_class, NULL, hdmi_devno, NULL, HDMI_DEVNAME);

	if (!hdmi_drv_init_context()) {
		TX_DEF_LOG("%s, hdmi_drv_init_context fail\n", __func__);
		return 0;
	}
	device_create_file((struct device *)class_dev, &dev_attr_hdmi_suspend_enable);
	device_create_file((struct device *)class_dev, &dev_attr_hdmi_debug_normal);
	device_create_file((struct device *)class_dev, &dev_attr_hdmi_hdr_status);
	device_create_file((struct device *)class_dev, &dev_attr_hdmi_common_status);

	hdmi = devm_kzalloc(&(pdev->dev), sizeof(*hdmi), GFP_KERNEL);
	if (!hdmi)
		return -ENOMEM;
	hdmi->dev = &(pdev->dev);
	hdmi->cdev = hdmi_cdev;
	platform_set_drvdata(pdev, hdmi);

	hdmi_parse_videolfb(hdmi->dev);
	ret = hdmi_drv->hdmidrv_probe(pdev, hdmi_boot_res);
	if (ret)
		return 0;

	return 0;
}

static int __init hdmi_init(void)
{
	int ret = 0;

	if (!hdmi_drv_init_context()) {
		TX_DEF_LOG("%s, hdmi_drv_init_context fail\n", __func__);
		return 0;
	}

	hdmi_drv->init();

	hdmi_switch_data.name = "hdmi";
	hdmi_switch_data.index = 0;
	hdmi_switch_data.state = 0;

	/* for support hdmi hotplug, inform AP the event */
	ret += switch_dev_register(&hdmi_switch_data);

	hdmires_switch_data.name = "res_hdmi";
	hdmires_switch_data.index = 0;
	hdmires_switch_data.state = 0;

	/* for support hdmi hotplug, inform AP the event */
	ret += switch_dev_register(&hdmires_switch_data);

	hdmi_cec_switch_data.name = "cec_hdmi";
	hdmi_cec_switch_data.index = 0;
	hdmi_cec_switch_data.state = 0;
	ret += switch_dev_register(&hdmi_cec_switch_data);

	hdmi_audio_switch_data.name = "hdmi_audio";
	hdmi_audio_switch_data.index = 0;
	hdmi_audio_switch_data.state = 0;
	ret += switch_dev_register(&hdmi_audio_switch_data);

	hdmi_hdcp_switch_data.name = "hdcp";
	hdmi_hdcp_switch_data.index = 0;
	hdmi_hdcp_switch_data.state = 0;
	ret += switch_dev_register(&hdmi_hdcp_switch_data);

	if (ret) {
		TX_DEF_LOG("switch_dev_register returned:%d!\n", ret);
		return 1;
	}

	if (platform_driver_register(&hdmi_driver)) {
		TX_DEF_LOG("failed to register mtkfb driver\n");
		return -1;
	}

	return 0;
}
module_init(hdmi_init);
module_exit(hdmi_exit);
MODULE_AUTHOR("www.mediatek.com>");
MODULE_DESCRIPTION("HDMI Driver");
MODULE_LICENSE("GPL");

#endif
