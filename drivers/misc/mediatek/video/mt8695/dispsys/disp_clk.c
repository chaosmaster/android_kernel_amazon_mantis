/*
*
*  Copyright (C) 2015-2016 MediaTek Inc.
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

#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include "smi.h"

#define LOG_TAG "CLK"
#include "disp_clk.h"
#include "disp_hw_log.h"

#define DISP_PM_NUM DISP_PM_UNKNOWN

static struct clk *disp_clk_map[DISP_CLK_NUM];
static int clock_status[DISP_CLK_NUM];
static struct DISP_PM_INFO pm_info[DISP_PM_NUM];
static struct device *smi_larb_dev[DISP_SMI_LARB_NUM];
static bool register_success;
const char *disp_get_clk_name(enum DISP_HW_CLK disp_clk)
{
	switch (disp_clk) {
	case DISP_CLK_TVDPLL:
		return "tvdpll";
	case DISP_CLK_TVDPLL_D2:
		return "tvdpll_d2";
	case DISP_CLK_OSDPLL:
		return "osdpll";
	case DISP_CLK_OSDPLL_D2:
		return "osdpll_d2";
	case DISP_CLK_OSDPLL_D4:
		return "osdpll_d4";
	case DISP_CLK_HD_SEL:
		return "hd_sel";
	case DISP_CLK_SD_SEL:
		return "sd_sel";
	case DISP_CLK_OSD_SEL:
		return "osd_sel";
	case DISP_CLK_VDO3_SEL:
		return "vdo3_sel";
	case DISP_CLK_VDO4_SEL:
		return "vdo4_sel";
	case DISP_CLK_26M:
		return "clk26m";
	case DISP_CLK_VDOUT_SYS:
		return "vdout_sys";
	case DISP_CLK_FMT_TG:
		return "fmt_tg";
	case DISP_CLK_OSD_TVE:
		return "osd_tve";
	case DISP_CLK_OSD_FHD:
		return "osd_fhd";
	case DISP_CLK_OSD_UHD:
		return "osd_uhd";
	case DISP_CLK_P2I:
		return "p2i";
	case DISP_CLK_SCLER:
		return "scler";
	case DISP_CLK_SDPPF:
		return "sdppf";
	case DISP_CLK_VDOIN:
		return "vdoin";
	case DISP_CLK_SDR2HDR:
		return "sdr2hdr";
	case DISP_CLK_OSD_PREMIX:
		return "osd_premix";
	case DISP_CLK_DOLBY_MIX:
		return "dolby_mix";
	case DISP_CLK_VM:
		return "vm";

	case DISP_CLK_DOLBY1:
		return "dolby1";
	case DISP_CLK_DOLBY2:
		return "dolby2";
	case DISP_CLK_DOLBY3:
		return "dolby3";

	case DISP_CLK_VDO3:
		return "vdo3";
	case DISP_CLK_DISPFMT3:
		return "dispfmt3";
	case DISP_CLK_MVDO_HDR2SDR:
		return "mvdo_hdr2sdr";
	case DISP_CLK_MVDO_BT2020:
		return "mvdo_bt2020";

	case DISP_CLK_VDO4:
		return "vdo4";
	case DISP_CLK_DISPFMT4:
		return "dispfmt4";
	case DISP_CLK_SVDO_HDR2SDR:
		return "svdo_hdr2sdr";
	case DISP_CLK_SVDO_BT2020:
		return "svdo_bt2020";

	default:
		DISP_LOG_E("invalid clk id=%d\n", disp_clk);
		return "unknown";
	}
}

static enum DISP_PM_TYPE disp_get_pm_type(enum DISP_HW_CLK hw_clk)
{
	enum DISP_PM_TYPE type = DISP_PM_UNKNOWN;

	switch (hw_clk) {
	case DISP_CLK_TVDPLL:
	case DISP_CLK_TVDPLL_D2:
	case DISP_CLK_OSDPLL:
	case DISP_CLK_OSDPLL_D2:
	case DISP_CLK_OSDPLL_D4:
	case DISP_CLK_HD_SEL:
	case DISP_CLK_SD_SEL:
	case DISP_CLK_OSD_SEL:
	case DISP_CLK_VDO3_SEL:
	case DISP_CLK_VDO4_SEL:
	case DISP_CLK_26M:
	case DISP_CLK_VDOUT_SYS:
	case DISP_CLK_FMT_TG:
	case DISP_CLK_OSD_TVE:
	case DISP_CLK_OSD_FHD:
	case DISP_CLK_OSD_UHD:
	case DISP_CLK_P2I:
	case DISP_CLK_SCLER:
	case DISP_CLK_SDPPF:
	case DISP_CLK_VDOIN:
	case DISP_CLK_SDR2HDR:
	case DISP_CLK_OSD_PREMIX:
	case DISP_CLK_DOLBY_MIX:
	case DISP_CLK_VM:
		type = DISP_PM_MMSYS_TOP;
		break;

	case DISP_CLK_DOLBY1:
	case DISP_CLK_DOLBY2:
	case DISP_CLK_DOLBY3:
		type = DISP_PM_DOLBY;
		break;

	case DISP_CLK_VDO3:
	case DISP_CLK_DISPFMT3:
	case DISP_CLK_MVDO_HDR2SDR:
	case DISP_CLK_MVDO_BT2020:
		type = DISP_PM_DISP_MAIN;
		break;

	case DISP_CLK_VDO4:
	case DISP_CLK_DISPFMT4:
	case DISP_CLK_SVDO_HDR2SDR:
	case DISP_CLK_SVDO_BT2020:
		type = DISP_PM_DISP_SUB;
		break;

	default:
		type = DISP_PM_UNKNOWN;
		break;
	}

	return type;
}

static int disp_enable_disable_pm(enum DISP_HW_CLK hw_clk, bool en)
{
	enum DISP_PM_TYPE type = disp_get_pm_type(hw_clk);

	if (type == DISP_PM_UNKNOWN)
		return 0;

	if (en && pm_info[type].pm_clk_cnt == 0 && pm_info[type].pm_dev) {
		int ret = 0;

		ret = pm_runtime_get_sync(pm_info[type].pm_dev);
		DISP_LOG_DEBUG("enable pm= %d ret %d\n", type, ret);
	} else if (!en && pm_info[type].pm_clk_cnt == 0 && pm_info[type].pm_dev) {
		DISP_LOG_DEBUG("disbale pm= %d\n", type);
		pm_runtime_put_sync(pm_info[type].pm_dev);
	}

	return 0;
}

static int disp_subsys_ref_cnt(enum DISP_HW_CLK hw_clk, bool en)
{
	enum DISP_PM_TYPE type = disp_get_pm_type(hw_clk);

	if (type == DISP_PM_UNKNOWN)
		return 0;

	if (en)
		pm_info[type].pm_clk_cnt++;
	else
		pm_info[type].pm_clk_cnt--;

	return 0;
}

static int disp_get_clk_by_pm(struct device *dev, enum DISP_PM_TYPE type)
{
	int i = 0;
	int pm_type = DISP_PM_UNKNOWN;

	for (i = 0; i < DISP_CLK_NUM; i++) {
		pm_type = disp_get_pm_type(i);
		if (pm_type != type)
			continue;

		disp_clk_map[i] = devm_clk_get(dev, disp_get_clk_name(i));

		if (IS_ERR(disp_clk_map[i])) {
			DISP_LOG_D("***DT|DISPSYS clock|%s ID %d: 0x%lx\n", disp_get_clk_name(i), i,
			  (unsigned long)disp_clk_map[i]);
				return -EPROBE_DEFER;
		}
	}

	return 0;
}

int disp_get_clk(struct device *dev)
{
	int i = 0;

	for (i = 0; i < DISP_CLK_NUM; i++) {
		disp_clk_map[i] = devm_clk_get(dev, disp_get_clk_name(i));

		if (IS_ERR(disp_clk_map[i]))
			DISP_LOG_E("***DT|DISPSYS clock|%s ID %d: 0x%lx\n", disp_get_clk_name(i), i,
			  (unsigned long)disp_clk_map[i]);
	}

	return 0;
}

int disp_clock_set_larb_dev(struct device *dev, int larb_id)
{
	if (!dev || larb_id >= DISP_SMI_LARB_NUM) {
		DISP_LOG_E("dev[%p] is NULL or larb id(%d) err\n",
			dev, larb_id);
		return -1;
	}
	smi_larb_dev[larb_id] = dev;

	return 0;
}

int disp_clock_smi_larb_en(enum DISP_SMI_LARB_ID larb_id, bool en)
{
	if (larb_id >= DISP_SMI_LARB_NUM) {
		DISP_LOG_E("disp_clock_smi_larb_en id err, larb_id=%d\n", larb_id);
		return -1;
	}
	if (!smi_larb_dev[larb_id]) {
		DISP_LOG_E("disp_clock_smi_larb_en can not find larb dev,id=%d\n", larb_id);
		return -1;
	}
	DISP_LOG_D("disp_clock_smi_larb_en id=%d, en=%d\n", larb_id, en);
	if (en)
		mtk_smi_larb_get(smi_larb_dev[larb_id]);
	else
		mtk_smi_larb_put(smi_larb_dev[larb_id]);

	return 0;
}

int disp_clock_set_pm_dev(struct device *dev, enum DISP_PM_TYPE type)
{
	DISP_LOG_D("set pm device , type=%d\n", type);

	if (!dev) {
		DISP_LOG_E("dev is NULL!!\n");
		return -1;
	}

	if (type >= DISP_PM_UNKNOWN) {
		DISP_LOG_E("set pm dev, err type=%d\n", type);
		return -1;
	}

	if (pm_info[type].pm_dev != NULL) {
		DISP_LOG_D("pm has been set already, type=%d\n", type);
	}

	pm_info[type].type = type;
	pm_info[type].pm_dev = dev;
	pm_info[type].pm_clk_cnt = 0;

	return 0;
}

int disp_clock_enable(enum DISP_HW_CLK hw_clk, bool enable)
{
	int ret;

	if (hw_clk >= DISP_CLK_NUM) {
		DISP_LOG_E("hw clk[%d] fail\n", hw_clk);
		return -1;
	}

	if (IS_ERR(disp_clk_map[hw_clk]) || !disp_clk_map[hw_clk]) {
		DISP_LOG_E("get hw clk -%s- fail\n", disp_get_clk_name(hw_clk));
		return -1;
	}

	if (enable) {
		/* enable pm */
		if (disp_get_pm_type(hw_clk) != DISP_PM_DOLBY)
			disp_enable_disable_pm(hw_clk, true);

		ret = clk_prepare_enable(disp_clk_map[hw_clk]);
		if (ret != 0) {
			DISP_LOG_E("clk_prepare %s fail\n", disp_get_clk_name(hw_clk));
			/* disable pm if open clock fail */
			disp_enable_disable_pm(hw_clk, false);
			return -1;
		}

		if (disp_get_pm_type(hw_clk) == DISP_PM_DOLBY)
			disp_enable_disable_pm(hw_clk, true);

		clock_status[hw_clk]++;
		/* pm ref count */
		disp_subsys_ref_cnt(hw_clk, true);

		DISP_LOG_D("CLK/enable-%d-%s\n", clock_status[hw_clk], disp_get_clk_name(hw_clk));
	} else {
		if (disp_get_pm_type(hw_clk) != DISP_PM_DOLBY)
			clk_disable_unprepare(disp_clk_map[hw_clk]);

		clock_status[hw_clk]--;
		/* pm ref count */
		disp_subsys_ref_cnt(hw_clk, false);
		/* dsiable pm */
		disp_enable_disable_pm(hw_clk, false);
		if (disp_get_pm_type(hw_clk) == DISP_PM_DOLBY)
			clk_disable_unprepare(disp_clk_map[hw_clk]);

		DISP_LOG_D("CLK/disable-%d-%s\n", clock_status[hw_clk],
		       disp_get_clk_name(hw_clk));
	}

	return 0;
}

int disp_clock_set_pll(enum DISP_HW_CLK hw_clk, unsigned int rate)
{
	if (hw_clk >= DISP_CLK_NUM) {
		DISP_LOG_E("set pll, clk[%d] fail\n", hw_clk);
		return -1;
	}

	if (IS_ERR(disp_clk_map[hw_clk]) || !disp_clk_map[hw_clk]) {
		DISP_LOG_E("set pll clk -%s- fail\n", disp_get_clk_name(hw_clk));
		return -1;
	}

	DISP_LOG_I("disp_clock_set_pll %s - %d\n", disp_get_clk_name(hw_clk), rate);

	disp_clock_enable(hw_clk, true);
	clk_set_rate(disp_clk_map[hw_clk], rate);
	disp_clock_enable(hw_clk, false);

	return 0;
}

int disp_clock_select_pll(enum DISP_HW_CLK hw_clk, enum DISP_HW_CLK sel_clk)
{
	if (hw_clk >= DISP_CLK_NUM || sel_clk >= DISP_CLK_NUM) {
		DISP_LOG_E("select pll, clk[%d] fail\n", hw_clk);
		return -1;
	}

	if (IS_ERR(disp_clk_map[hw_clk]) || IS_ERR(disp_clk_map[sel_clk]) ||
		!disp_clk_map[hw_clk] || !disp_clk_map[sel_clk]) {
		DISP_LOG_E("select pll clk -%s- fail\n", disp_get_clk_name(hw_clk));
		return -1;
	}

	DISP_LOG_D("disp_clock_select_pll %s - %s\n", disp_get_clk_name(hw_clk), disp_get_clk_name(sel_clk));

	/* clk_prepare_enable(disp_clk_map[hw_clk]); */
	clk_set_parent(disp_clk_map[hw_clk], disp_clk_map[sel_clk]);
	/* clk_disable_unprepare(disp_clk_map[hw_clk]); */

	return 0;
}


int disp_clock_dump(enum DISP_HW_CLK hw_clk)
{
	int i = 0;

	DISP_LOG_N("disp_clock_dump clk= %s\n", disp_get_clk_name(hw_clk));
	if (hw_clk >= DISP_CLK_NUM) {
		DISP_LOG_N("--------dump all clock-------\n");
		for (i = 0; i < DISP_CLK_NUM; i++)
			DISP_LOG_N("--%s--: %d\n", disp_get_clk_name(i), clock_status[i]);
	} else {
		DISP_LOG_N("--------dump one clock-------\n");
		DISP_LOG_N("--%s--: %d\n", disp_get_clk_name(hw_clk), clock_status[hw_clk]);
	}

	return 0;
}

bool disp_clock_register_success(void)
{
	return register_success;
}

static int mtk_disp_clk_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	enum DISP_PM_TYPE type = DISP_PM_UNKNOWN;

	register_success = false;
	ret = of_property_read_u32(dev->of_node, "mediatek,pm_type", &type);
	if (ret) {
		DISP_LOG_W("read device node warning, ret= %d\n", ret);
		return ret;
	}

	disp_clock_set_pm_dev(dev, type);
	ret = disp_get_clk_by_pm(dev, type);
	if (ret != 0) {
		DISP_LOG_D("mtk_disp_clk_probe get clk fail, ret= 0x%x\n", ret);
		return ret;
	}

	DISP_LOG_D("disp_clock probe, pm type=%d\n", type);

	pm_runtime_enable(dev);
	if (type == DISP_PM_DOLBY) {
		DISP_LOG_I("disp_clock probe power on dolby, pm type=%d\n", type);
		pm_runtime_get_sync(dev);
	}
	register_success = true;
	return 0;
}


static int mtk_disp_clk_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);
	return 0;
}

static const struct of_device_id mtk_disp_clk_of_ids[] = {
	{ .compatible = "mediatek,mt8695-disp-clk", },
	{}
};

static struct platform_driver mtk_disp_clk_driver = {
	.probe	= mtk_disp_clk_probe,
	.remove = mtk_disp_clk_remove,
	.driver	= {
		.name = "mtk-disp-clk",
		.of_match_table = mtk_disp_clk_of_ids,
	}
};


static int __init disp_clk_init(void)
{
	DISP_LOG_D("mtk_disp_clk_init in\n");

	if (platform_driver_register(&mtk_disp_clk_driver))
		return -ENODEV;

	DISP_LOG_D("mtk_disp_clk_init out\n");
	return 0;
}

static void __exit disp_clk_exit(void)
{
	platform_driver_unregister(&mtk_disp_clk_driver);
}

static int __init disp_clk_init_late(void)
{
	DISP_LOG_I("disp_clock probe power down dolby, pm type=%d\n", DISP_PM_DOLBY);
	pm_runtime_put_sync(pm_info[DISP_PM_DOLBY].pm_dev);

	return 0;
}

module_init(disp_clk_init);
module_exit(disp_clk_exit);
late_initcall(disp_clk_init_late);
MODULE_DESCRIPTION("mediatek display clock manager");
MODULE_LICENSE("GPL");
