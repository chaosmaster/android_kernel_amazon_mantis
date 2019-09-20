/*
* Copyright (C) 2015-2016 MediaTek Inc.
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/pm_runtime.h>

#define LOG_TAG "MGR"
#include "disp_hw_log.h"
#include "mtk_disp_mgr.h"
#include "disp_hw_mgr.h"
#include "mtk_disp_debug.h"
#include "disp_assert_layer.h"

static dev_t disp_devno;
static struct cdev *disp_cdev;
static struct class *disp_class;


static int mtk_disp_mgr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_disp_mgr_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_disp_mgr_mmap(struct file *file, struct vm_area_struct *vma)
{
	return 0;
}

static int mtk_disp_mgr_get_info(uint32_t id, struct mtk_disp_info *info)
{
	int ret = 0;

	struct disp_hw_common_info hw_info;

	if (id >= MTK_MAX_DISPLAY_COUNT || NULL == info)
		return -EFAULT;

	if (id == MTK_DISPIF_PRIMARY_LCD) {
		ret = disp_hw_mgr_get_info(&hw_info);
		if (!ret) {
			info->isHwVsyncAvailable = 1;
			info->displayWidth = 1920;
			info->displayHeight = 1080;
			info->physicalWidth = hw_info.resolution->width;
			info->physicalHeight = hw_info.resolution->height;
			info->vsyncFPS = hw_info.resolution->frequency;
			info->disp_resolution = hw_info.resolution->res_mode;
			info->displayFormat = DISP_HW_COLOR_FORMAT_BGRA8888;
			info->isConnected = 1;
		}
	}

	return ret;
}

static int mtk_disp_mgr_get_layer_info(uint32_t id, struct mtk_disp_layer_info *info)
{
	int ret = 0;

	if (id >= MTK_MAX_DISPLAY_COUNT || NULL == info)
		return -EFAULT;

	if (id == MTK_DISPIF_PRIMARY_LCD) {
		info->supported_ui_num = 2;
		info->supported_video_num = 2;
	#ifdef CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER
		if (is_DAL_Enabled())
			info->supported_ui_num = 1;
	#endif
	}

	return ret;
}

static int mtk_disp_mgr_wait_vsync(unsigned long long *ts)
{
	return disp_hw_mgr_wait_vsync(ts);
}

static int mtk_disp_mgr_set_input_buffer(struct mtk_disp_config *config)
{
	static unsigned int set_config_cnt;
#ifdef CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER
	int i = 0;
	int ui_num = 0;
	int top_osd_layer_id = 0;

	if (is_DAL_Enabled() && config->user == DISP_USER_HWC) {
		for (i = 0; i < DISP_BUFFER_MAX; i++) {
			if (config->buffer_info[i].type == DISP_LAYER_OSD) {
				top_osd_layer_id = i;
				ui_num++;
			}
		}
		if (ui_num <= 1)
			DAL_Get_Layer_Info(&(config->buffer_info[DISP_BUFFER_MAX - 1]));
		else
			DAL_Get_Layer_Info(&(config->buffer_info[top_osd_layer_id]));
	}
#endif
	set_config_cnt++;
	if (set_config_cnt == 10)
		mtkfb_free_reserved_fb();

	return disp_hw_mgr_config(config);
}

static long mtk_disp_mgr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	long ret = 0;
	struct mtk_disp_info info;
	struct mtk_disp_config config;
	struct mtk_disp_vdp_cap vdp_cap;
	struct mtk_disp_hdmi_cap hdmi_cap;
	unsigned long long cur_time;
	struct mtk_disp_layer_info layer_info;

	switch (cmd) {
	case MTK_DISP_IOCTL_GET_INFO:
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		ret = mtk_disp_mgr_get_info(info.display_id, &info);

		if (!ret && copy_to_user(argp, &info, sizeof(info)))
			ret = -EFAULT;
		break;
	case MTK_DISP_IOCTL_WAIT_VSYNC:
		mtk_disp_mgr_wait_vsync(&cur_time);
		if (copy_to_user(argp, &cur_time, sizeof(cur_time)))
			ret = -EFAULT;
		break;
	case MTK_DISP_IOCTL_SET_INPUT_BUFFER:
		if (copy_from_user(&config, argp, sizeof(config)))
			return -EFAULT;

		ret = mtk_disp_mgr_set_input_buffer(&config);

		if (!ret && copy_to_user(argp, &config, sizeof(config)))
			ret = -EFAULT;
		break;
	case MTK_DISP_IOCTL_SUSPEND:
		ret = disp_hw_mgr_suspend();
		break;
	case MTK_DISP_IOCTL_RESUME:
		ret = disp_hw_mgr_resume();
		break;
	case MTK_DISP_IOCTL_VDP_CAP:
		if (copy_from_user(&vdp_cap, argp, sizeof(vdp_cap)))
			return -EFAULT;

		ret = disp_hw_get_vdp_cap(&vdp_cap);

		if (!ret && copy_to_user(argp, &vdp_cap, sizeof(vdp_cap)))
			ret = -EFAULT;
		break;
	case MTK_DISP_IOCTL_HDMI_CAP:
		if (copy_from_user(&hdmi_cap, argp, sizeof(hdmi_cap)))
			return -EFAULT;

		ret = disp_hw_get_hdmi_cap(&hdmi_cap);

		if (!ret && copy_to_user(argp, &hdmi_cap, sizeof(hdmi_cap)))
			ret = -EFAULT;
		break;
	case MTK_DISP_IOCTL_GET_LAYER_INFO:
		if (copy_from_user(&layer_info, argp, sizeof(layer_info)))
			return -EFAULT;

		ret = mtk_disp_mgr_get_layer_info(layer_info.display_id, &layer_info);

		if (!ret && copy_to_user(argp, &layer_info, sizeof(layer_info)))
			ret = -EFAULT;
		break;
	default:
		pr_info("disp mgr ioctl unknown ioctl=0x%x\n", cmd);
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT

struct compat_mtk_disp_dovi_md_s {
	int pts;
	uint32_t len;
	bool svp;
	uint32_t sec_handle;
	uint32_t addr;
};

struct compat_mtk_disp_buffer {
	unsigned int layer_order;
	unsigned int layer_id;
	unsigned int layer_enable;

	enum DISP_LAYER_TYPE type;
	enum DISP_HW_COLOR_FORMAT src_fmt;

	bool color_key_en;
	int src_color_key;

	bool alpha_en;
	int alpha;

	/* display region */
	struct mtk_disp_range src;
	struct mtk_disp_range crop;
	struct mtk_disp_range tgt;

	/* buffer address and fence */
	int ion_fd;
	bool secruity_en;
	int acquire_fence_fd;
	int release_fence_fd;

	/* for pre-multiple alpha */
	enum DISP_ALPHA_TYPE src_alpha;
	enum DISP_ALPHA_TYPE dst_alpha;

	/* if buffer_info is video plane, should fill video info */
	bool is_ufo;
	bool is_dolby;
	bool is_progressive;
	bool is_10bit;
	bool is_10bit_lbs2bit_tile_mode;
	bool is_hdr;
	bool is_pack_mode;
	bool is_bt2020;
	bool is_seamless;
	bool is_jumpmode;
	unsigned int pts;
	unsigned int fps;
	enum DISP_ASPECT_RATIO aspect_ratio;
	enum DISP_VIDEO_TYPE video_type;
	struct mtk_disp_hdr_info_t hdr_info;
	enum DISP_DR_TYPE_T dr_range;

	/*video buffer offset*/
	unsigned int ofst_y;
	unsigned int ofst_c;
	unsigned int ofst_c_len;
	unsigned int ofst_y_len;
	uint32_t buffer_size;

	unsigned int meta_data_size;
	struct compat_mtk_disp_dovi_md_s dolby_info;
	uint32_t meta_data;

	uint32_t src_base_addr;
	uint32_t src_phy_addr;
};

struct compat_mtk_disp_config {
	enum DISP_MGR_USER user;
	unsigned int buffer_num;
	struct compat_mtk_disp_buffer buffer_info[DISP_BUFFER_MAX];
};


#define COMPAT_MTK_DISP_IOCTL_SET_INPUT_BUFFER		MTK_DISP_IOWR(0xd2, struct compat_mtk_disp_config)

void assignment(struct compat_mtk_disp_config *compat_config,	struct mtk_disp_config *config)
{
	int i = 0;

	for (i = 0; i < config->buffer_num; i++) {
		compat_config->buffer_info[i].release_fence_fd = config->buffer_info[i].release_fence_fd;
	}
}

void compat_assignment(struct mtk_disp_config *config, struct compat_mtk_disp_config *compat_config)
{
	int i = 0;

	config->user = compat_config->user;
	config->buffer_num = compat_config->buffer_num;
	for (i = 0; i < compat_config->buffer_num; i++) {

		memcpy(&(config->buffer_info[i]),
			&(compat_config->buffer_info[i]), sizeof(struct compat_mtk_disp_buffer));
		memcpy(&(config->buffer_info[i].dolby_info),
			&(compat_config->buffer_info[i].dolby_info), sizeof(struct compat_mtk_disp_dovi_md_s));
		config->buffer_info[i].dolby_info.addr =
				(void *)((long)(compat_config->buffer_info[i].dolby_info.addr & 0xffffffff));
		config->buffer_info[i].meta_data =
			(void *)((long)(compat_config->buffer_info[i].meta_data & 0xffffffff));
		config->buffer_info[i].src_base_addr =
			(void *)((long)(compat_config->buffer_info[i].src_base_addr & 0xffffffff));
		config->buffer_info[i].src_phy_addr =
			(void *)((long)(compat_config->buffer_info[i].src_phy_addr & 0xffffffff));

	}
}

int compat_mtk_disp_mgr_set_input_buffer(struct compat_mtk_disp_config *compat_config)
{
	int ret = 0;
	struct mtk_disp_config config;

	memset(&config, 0, sizeof(struct mtk_disp_config));

	compat_assignment(&config, compat_config);

	ret = mtk_disp_mgr_set_input_buffer(&config);
	if (ret)
		return -EFAULT;

	assignment(compat_config, &config);

	return ret;
}

static long mtk_disp_mgr_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	struct compat_mtk_disp_config compat_config;

	if (cmd == COMPAT_MTK_DISP_IOCTL_SET_INPUT_BUFFER) {
		int ret = 0;
		void __user *argp = (void __user *)arg;

		if (copy_from_user(&compat_config, argp, sizeof(struct compat_mtk_disp_config)))
			return -EFAULT;

		ret = compat_mtk_disp_mgr_set_input_buffer(&compat_config);
		if (ret)
			return -EFAULT;

		ret = copy_to_user(argp, &compat_config, sizeof(struct compat_mtk_disp_config));

		return ret;
	}

	return mtk_disp_mgr_ioctl(file, cmd, arg);
}
#endif

static ssize_t mtk_disp_mgr_dump(struct device *dev, struct device_attribute *attr, char *buf)
{
	disp_hw_mgr_dump(0);
	return 0;
}

static ssize_t mtk_disp_mgr_ut(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return 0;
}

static DEVICE_ATTR(ut, 0644, mtk_disp_mgr_dump, mtk_disp_mgr_ut);

static const struct file_operations mtk_disp_mgr_fops = {
	.owner = THIS_MODULE,
	.open = mtk_disp_mgr_open,
	.mmap = mtk_disp_mgr_mmap,
	.unlocked_ioctl = mtk_disp_mgr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mtk_disp_mgr_compat_ioctl,
#endif
	.release = mtk_disp_mgr_release,
};

static int mtk_disp_mgr_probe(struct platform_device *pdev)
{
	int ret = 0;

	alloc_chrdev_region(&disp_devno, 0, 1, DISP_SESSION_DEVICE);
	disp_cdev = cdev_alloc();
	disp_cdev->owner = THIS_MODULE;
	disp_cdev->ops = &mtk_disp_mgr_fops;
	cdev_add(disp_cdev, disp_devno, 1);
	disp_class = class_create(THIS_MODULE, DISP_SESSION_DEVICE);
	device_create(disp_class, NULL, disp_devno, NULL, DISP_SESSION_DEVICE);

	device_create_file(&pdev->dev, &dev_attr_ut);

	return ret;
}

static int mtk_disp_mgr_remove(struct platform_device *pdev)
{
	int ret = 0;

	device_remove_file(&pdev->dev, &dev_attr_ut);

	cdev_del(disp_cdev);
	unregister_chrdev_region(disp_devno, 1);
	device_destroy(disp_class, disp_devno);
	class_destroy(disp_class);
	return ret;
}

static void mtk_disp_mgr_shutdown(struct platform_device *pdev)
{
}


static const struct of_device_id mgr_of_ids[] = {
	{.compatible = "mediatek,mt8695-dispmgr",},
	{}
};

static struct platform_driver mtk_disp_mgr_driver = {
	.probe = mtk_disp_mgr_probe,
	.remove = mtk_disp_mgr_remove,
	.shutdown = mtk_disp_mgr_shutdown,
	.driver = {
		   .name = DISP_SESSION_DEVICE,
		   .owner = THIS_MODULE,
		   .of_match_table = mgr_of_ids,
		   },
};

static void mtk_disp_mgr_device_release(struct device *dev)
{

}

static u64 mtk_disp_mgr_dmamask = ~(u32) 0;

static struct platform_device mtk_disp_mgr_device = {
	.name = DISP_SESSION_DEVICE,
	.id = 0,
	.dev = {
		.release = mtk_disp_mgr_device_release,
		.dma_mask = &mtk_disp_mgr_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = 0,
};


static int __init mtk_disp_mgr_init(void)
{
	pr_debug("mtk_disp_mgr_init in\n");

	if (platform_device_register(&mtk_disp_mgr_device))
		return -ENODEV;

	if (platform_driver_register(&mtk_disp_mgr_driver)) {
		platform_device_unregister(&mtk_disp_mgr_device);
		return -ENODEV;
	}

	pr_debug("mtk_disp_mgr_init out\n");
	return 0;
}

static void __exit mtk_disp_mgr_exit(void)
{
	cdev_del(disp_cdev);
	unregister_chrdev_region(disp_devno, 1);

	platform_driver_unregister(&mtk_disp_mgr_driver);

	device_destroy(disp_class, disp_devno);
	class_destroy(disp_class);
}

module_init(mtk_disp_mgr_init);
module_exit(mtk_disp_mgr_exit);
MODULE_DESCRIPTION("mediatek display manager");
MODULE_LICENSE("GPL");
