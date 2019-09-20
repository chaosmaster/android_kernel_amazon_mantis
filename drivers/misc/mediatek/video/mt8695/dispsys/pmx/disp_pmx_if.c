/*
 * Copyright (C) 2017 MediaTek Inc.
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


#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "disp_hw_mgr.h"
#include "disp_pmx_if.h"
#include "disp_def.h"
#include "fmt_hal.h"
#include "vdout_sys_hal.h"
#include "disp_sys_hal.h"
#include "disp_irq.h"
/** Plane mixer configuration.
 */
struct pmx_context_t {
	int u4BgColor;
	int u4PlaneOrder;
	int tv_type;
	struct disp_hw_resolution hdmi_res;
	struct vdout_init_param vdout_reg;
	struct fmt_init_pararm fmt_reg;
	struct disp_sys_init_param disp_reg;
};


static struct pmx_context_t pmx = {
	0x108080, 0x76543210,
};

static int _pmx_parse_dev_node(void)
{
	struct device_node *np;
	unsigned int reg_value;
	unsigned int irq_value;
	struct disp_hw *pmx_drv = disp_pmx_get_drv();

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-fmt");
	if (np == NULL) {
		PMX_ERR("dts error, no fmtter device node.\n");
		return PMX_DTS_FAIL;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);
	of_property_read_u32_index(np, "interrupts", 1, &irq_value);

	pmx.fmt_reg.io_reg_base = (uintptr_t)of_iomap(np, 0);			/* 10000000 */
	pmx.fmt_reg.vdout_fmt_reg_base = (uintptr_t)of_iomap(np, 1);		/* 14001000 */
	pmx.disp_reg.dispsys_reg_base = (uintptr_t)of_iomap(np, 2);		/* 15000000 */
	pmx.fmt_reg.disp_fmt_reg_base[0] = (uintptr_t)of_iomap(np, 3);	/* 15001000 */
	pmx.fmt_reg.disp_fmt_reg_base[1] = (uintptr_t)of_iomap(np, 4);	/* 15008000 */
#if 1
	pmx_drv->irq[0].value = irq_of_parse_and_map(np, 0);  /* vsync irq*/
	pmx_drv->irq[1].value = irq_of_parse_and_map(np, 1);  /* active start*/
	if (pmx_drv->irq[0].value == 0 ||
		pmx_drv->irq[1].value == 0) {
		PMX_ERR("dts error, get interrupt id fail.\n");
		return PMX_DTS_FAIL;
	}
	PMX_DEBUG("irq 1 value: %d, irq 2 value: %d\n",
		pmx_drv->irq[0].value, pmx_drv->irq[1].value);
	pmx_drv->irq[0].irq = DISP_IRQ_FMT_VSYNC;
	pmx_drv->irq[1].irq = DISP_IRQ_FMT_ACTIVE_START;
	pmx_drv->irq_num = 2;
#endif
	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-vdout");
	if (np == NULL) {
		PMX_ERR("dts error, no vdout device node.\n");
		return PMX_DTS_FAIL;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);

	pmx.vdout_reg.sys1_reg_base = (uintptr_t)of_iomap(np, 0);	/* 3500 */
	pmx.vdout_reg.sys2_reg_base = (uintptr_t)of_iomap(np, 1);	/* 3c00 */

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-mmsys");
	if (np) {
		of_property_read_u32_index(np, "reg", 1, &reg_value);
		pmx.disp_reg.mmsys_reg_base = (uintptr_t)of_iomap(np, 0);
	}

	return PMX_OK;
}

static int _pmx_init(struct disp_hw_common_info *info)
{
	enum FMT_TV_TYPE tv_type;
	struct fmt_active_info active_info = {0};
	HDMI_VIDEO_RESOLUTION res = info->resolution->res_mode;

	PMX_DEBUG("_pmx_init begin, res: %d\n\n", res);

	/* hdmi service is not ready , so use temporary solution*/
#if 0
	hdmi_internal_video_config(info->resolution->res_mode);

	hdmi_internal_audio_config(0x0);
#endif

	if (info->resolution->frequency == 0 || res >= HDMI_VIDEO_RESOLUTION_NUM)
		return PMX_PARAM_ERR;

	if (info->resolution->frequency == 25 || info->resolution->frequency == 50)
		tv_type = FMT_TV_TYPE_PAL;
	else
		tv_type = FMT_TV_TYPE_NTSC;

	fmt_hal_set_pllgp_hdmidds(info->resolution->res_mode, true, false,
		info->resolution->is_fractional);

	vdout_sys_hal_clock_on_off(true); /* open the vdout sys clock */

	fmt_hal_clock_on_off(VDOUT_FMT, true);

	fmt_hal_set_mode(VDOUT_FMT, info->resolution->res_mode, false);

	fmt_hal_set_tv_type(VDOUT_FMT, tv_type);

	fmt_hal_set_background(VDOUT_FMT, pmx.u4BgColor);

	fmt_hal_disable_video_plane(DISP_FMT_SUB);  /*disable video plane 2*/
	fmt_hal_set_active_zone(VDOUT_FMT, &active_info);
	vdout_sys_hal_set_hdmi(info->resolution->res_mode);   /* set hdmi pll */
	vdout_sys_hal_vm_bypass(true);

	fmt_hal_hw_shadow_enable(VDOUT_FMT);

	pmx.hdmi_res.res_mode = info->resolution->res_mode;
	pmx.hdmi_res.frequency = info->resolution->frequency;
	pmx.hdmi_res.width = info->resolution->width;
	pmx.hdmi_res.height = info->resolution->height;
	pmx.hdmi_res.is_fractional = info->resolution->is_fractional;
	pmx.tv_type = tv_type;

	PMX_DEBUG("_pmx_init end.\n");

	return PMX_OK;
}

static int disp_pmx_suspend(void)
{
	PMX_FUNC();
	fmt_hal_enable(VDOUT_FMT, false);

	fmt_hal_clock_on_off(VDOUT_FMT, false);

	vdout_sys_hal_clock_on_off(false); /* open the vdout sys clock */

	fmt_hal_set_pllgp_hdmidds(pmx.hdmi_res.res_mode, false, false,
		pmx.hdmi_res.is_fractional);

	PMX_INFO("disp_pmx_suspend - leave.\n");
	return PMX_OK;
}

static int disp_pmx_resume(void)
{
	struct fmt_active_info active_info = {0};

	PMX_FUNC();

	fmt_hal_set_pllgp_hdmidds(pmx.hdmi_res.res_mode, true, true,
		pmx.hdmi_res.is_fractional);

	vdout_sys_hal_clock_on_off(true); /* open the vdout sys clock */

	fmt_hal_clock_on_off(VDOUT_FMT, true);

	fmt_hal_set_mode(VDOUT_FMT, pmx.hdmi_res.res_mode, true);
	fmt_hal_hw_shadow_enable(VDOUT_FMT);

	fmt_hal_set_tv_type(VDOUT_FMT, pmx.tv_type);

	fmt_hal_set_background(VDOUT_FMT, pmx.u4BgColor);

	fmt_hal_disable_video_plane(DISP_FMT_SUB);  /*disable video plane 2*/

	fmt_hal_set_active_zone(VDOUT_FMT, &active_info);

	fmt_hal_reset(VDOUT_FMT);

	vdout_sys_hal_set_hdmi(pmx.hdmi_res.res_mode);   /* set hdmi pll */
	vdout_sys_hal_vm_bypass(true);

	PMX_INFO("disp_pmx_resume - leave.\n");

	return PMX_OK;
}


static int disp_pmx_get_info(struct disp_hw_common_info *info)
{
	return PMX_OK;
}

static int disp_pmx_change_resolution(const struct disp_hw_resolution *info)
{
	enum FMT_TV_TYPE tv_type;
	HDMI_VIDEO_RESOLUTION res = info->res_mode;

	PMX_FUNC();
	PMX_INFO("disp_fmt_change_resolution res_mode: %d\n", info->res_mode);

	if (pmx.hdmi_res.res_mode == info->res_mode &&
		pmx.hdmi_res.is_fractional == info->is_fractional) {
		PMX_INFO("res mode is not change, res_mode: %d\n", info->res_mode);
		return PMX_OK;
	}
	/* hdmi service is not ready , so use temporary solution*/
	/* hdmi_internal_video_config(info->res_mode); */
	if (info->frequency == 0 || res >= HDMI_VIDEO_RESOLUTION_NUM)
		return PMX_PARAM_ERR;

	if (info->frequency == 25 || info->frequency == 50)
		tv_type = FMT_TV_TYPE_PAL;
	else
		tv_type = FMT_TV_TYPE_NTSC;

	fmt_hal_set_pllgp_hdmidds(pmx.hdmi_res.res_mode, false, false,
		pmx.hdmi_res.is_fractional);
	fmt_hal_set_pllgp_hdmidds(info->res_mode, true, true, info->is_fractional);

	fmt_hal_set_mode(VDOUT_FMT, info->res_mode, true);
	fmt_hal_hw_shadow_enable(VDOUT_FMT);

	fmt_hal_set_tv_type(VDOUT_FMT, tv_type);
	fmt_hal_reset(VDOUT_FMT);

	vdout_sys_hal_set_hdmi(info->res_mode);   /* set hdmi pll */
	vdout_sys_hal_vm_bypass(true);

	pmx.hdmi_res.res_mode = info->res_mode;
	pmx.hdmi_res.frequency = info->frequency;
	pmx.hdmi_res.width = info->width;
	pmx.hdmi_res.height = info->height;
	pmx.hdmi_res.is_fractional = info->is_fractional;
	pmx.tv_type = tv_type;

	PMX_DEBUG("disp_fmt_change_resolution done.\n");

	return PMX_OK;
}

static int disp_pmx_config(struct mtk_disp_buffer *config, struct disp_hw_common_info *info)
{
	/*do nothing*/
	return PMX_OK;
}

static int disp_pmx_irq_handler(uint32_t irq)
{
	if (irq == DISP_IRQ_FMT_VSYNC)
		fmt_hal_update_hw_register();

	return PMX_OK;
}

static int disp_pmx_dump(uint32_t level)
{
	return PMX_OK;
}

/*hw manager need tell fmt init at which resolution*/
bool pmx_initiated;
static int disp_pmx_init(struct disp_hw_common_info *info)
{
	int ret = PMX_OK;

	PMX_FUNC();

	if (!pmx_initiated) {
		ret = _pmx_parse_dev_node();
		if (ret != PMX_OK) {
			PMX_ERR("parse device node fail.\n");
			return ret;
		}

		ret = vdout_sys_hal_init(&(pmx.vdout_reg));
		if (ret != PMX_OK) {
			PMX_ERR("set vdout sys register base fail.\n");
			return ret;
		}

		ret = disp_sys_hal_init(&(pmx.disp_reg));
		if (ret != PMX_OK) {
			PMX_ERR("set disp sys register base fail.\n");
			return ret;
		}

		ret = fmt_hal_init(&(pmx.fmt_reg));
		if (ret != PMX_OK) {
			PMX_ERR("set fmt register base fail.\n");
			return ret;
		}

		ret = _pmx_init(info);
		if (ret != PMX_OK) {
			PMX_ERR("init pmx fail.\n");
			return ret;
		}

		pmx_initiated = true;
	}

	return ret;

}

static int disp_pmx_deinit(void)
{
	/*release sw resouce*/
	fmt_hal_set_pllgp_hdmidds(pmx.hdmi_res.res_mode, false, false,
		pmx.hdmi_res.is_fractional);
	fmt_hal_clock_on_off(VDOUT_FMT, false);
	vdout_sys_hal_clock_on_off(false);
	pmx_initiated = false;
	return PMX_OK;
}

/***************** driver************/
struct disp_hw disp_pmx_driver = {
	.name = PMX_DRV_NAME,
	.init = disp_pmx_init,
	.deinit = disp_pmx_deinit,
	.start = NULL,
	.stop = NULL,
	.suspend = disp_pmx_suspend,
	.resume = disp_pmx_resume,
	.get_info = disp_pmx_get_info,
	.change_resolution = disp_pmx_change_resolution,
	.config = disp_pmx_config,
	.irq_handler = disp_pmx_irq_handler,
	.set_listener = NULL,
	.wait_event = NULL,
	.dump = disp_pmx_dump,
};

struct disp_hw *disp_pmx_get_drv(void)
{
	return &disp_pmx_driver;
}
