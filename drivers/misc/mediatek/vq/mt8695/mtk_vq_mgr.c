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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/pm_runtime.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/iommu.h>
#include <linux/slab.h>

#include "ion_drv.h"
#include "mtk_vq_mgr.h"
#include "vq_ion.h"
#include "nr_hal.h"
#include "smi.h"
#include "di_hal.h"

struct vq_debug vq_debug_enable;

static struct vq_data *vq_data_info;
static dev_t vq_devno;
static struct cdev *vq_cdev;
static struct class *vq_class;

struct vq_data *mtk_vq_get_data(void)
{
	return vq_data_info;
}

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
void mtk_vq_set_secure_debug_enable(unsigned int en)
{
	if (en)
		vq_debug_enable.vq_secure_debug_enable = true;
	else
		vq_debug_enable.vq_secure_debug_enable = false;

	pr_info("[VQ] log enable is %d\n", vq_debug_enable.vq_secure_debug_enable);
}

bool mtk_vq_get_secure_debug_enable(void)
{
	return vq_debug_enable.vq_secure_debug_enable;
}
#endif

void mtk_vq_set_log_enable(unsigned int en)
{
	if (en)
		vq_debug_enable.vq_log_enable = true;
	else
		vq_debug_enable.vq_log_enable = false;

	pr_info("[VQ] vq_log_enable is %d\n", vq_debug_enable.vq_log_enable);
}

void mtk_vq_set_timer_enable(unsigned int en)
{
	if (en)
		vq_debug_enable.vq_time_enable = true;
	else
		vq_debug_enable.vq_time_enable = false;

	pr_info("[VQ] vq_time_enable enable is %d\n", vq_debug_enable.vq_time_enable);
}

static int mtk_vq_mgr_prepare_buffer(struct ion_client *client,
				     struct mtk_vq_config_info *config_info)
{
	int ret = 0;
	int i = 0;
	unsigned int pre_buf_num = 0;
	unsigned int mva = 0;

	struct mtk_vq_config *config = config_info->vq_config;

	if (config->vq_mode == VQ_NR_STANDALONE)
		pre_buf_num = 1;
	else
		pre_buf_num = MTK_VQ_DI_INPUTBUFFER;

	for (i = 0; i < pre_buf_num; i++) {
		if (config->src_fd[i] <= 0) {
			VQ_ERR("ion fd is invalid [%d] fd %d\n", i, config->src_fd[i]);
			ret = -EFAULT;
			goto err_src_handle;
		}

		config_info->src_ion_handle[i] = vq_ion_import_handle(client, config->src_fd[i]);
		if (IS_ERR_OR_NULL(config_info->src_ion_handle[i])) {
			VQ_ERR("src handle error %d fd %d\n", i, config->src_fd[i]);
			ret = -EFAULT;
			goto err_src_handle;
		}
		mva = 0;
		vq_ion_phys_mmu_addr(client, config_info->src_ion_handle[i], &mva);
		if (!mva) {
			VQ_ERR("Get invalid src physical addr %d fd %d\n", i, config->src_fd[i]);
			goto err_src_handle;
		}

		config_info->src_mva[i] = mva;
		config_info->src_va[i] = ion_map_kernel(client, config_info->src_ion_handle[i]);

		VQ_INFO("the %d src mva is 0x%x va:%p\n", i, config_info->src_mva[i],
			config_info->src_va[i]);
	}

	if (config->dst_fd <= 0) {
		VQ_ERR("dst ion fd is invalid fd %d\n", config->dst_fd);
		ret = -EFAULT;
		goto err_src_handle;
	}

	config_info->dst_ion_handle = vq_ion_import_handle(client, config->dst_fd);
	if (IS_ERR_OR_NULL(config_info->dst_ion_handle)) {
		VQ_ERR("dst handle error fd %d\n", config->dst_fd);
		ret = -EFAULT;
		goto err_src_handle;
	}

	mva = 0;
	vq_ion_phys_mmu_addr(client, config_info->dst_ion_handle, &mva);
	if (!mva) {
		VQ_ERR("Get invalid dst physical addr fd %d\n", config->dst_fd);
		goto err_dst_handle;
	}

	config_info->dst_mva = mva;
	config_info->dst_va = ion_map_kernel(client, config_info->dst_ion_handle);

	VQ_INFO("the dst mva is 0x%x va:%p TFF %d field %d\n",
	config_info->dst_mva,
	config_info->dst_va,
	config->topfield_first_enable,
	config->cur_field);

	return 0;

err_dst_handle:
	vq_ion_free_handle(client, config_info->dst_ion_handle);

err_src_handle:
	for (i -= 1; i >= 0; i--)
		vq_ion_free_handle(client, config_info->src_ion_handle[i]);

	return ret;
}

static int mtk_vq_mgr_free_buffer_handle(struct ion_client *client,
					 struct mtk_vq_config_info *config_info)
{
	int i = 0;
	unsigned int pre_buf_num = 0;

	if (config_info->vq_config->vq_mode == VQ_NR_STANDALONE)
		pre_buf_num = 1;
	else
		pre_buf_num = MTK_VQ_DI_INPUTBUFFER;

	for (i = 0; i < pre_buf_num; i++) {
		if (!(IS_ERR_OR_NULL(config_info->src_ion_handle[i])))
			vq_ion_free_handle(client, config_info->src_ion_handle[i]);
	}

	vq_ion_free_handle(client, config_info->dst_ion_handle);

	return 0;
}

static void mtk_vq_smi_larb_en(struct vq_data *data, bool en)
{
	if (en)
		mtk_smi_larb_get(data->larb_dev);
	else
		mtk_smi_larb_put(data->larb_dev);
}

static void mtk_vq_pm_en(struct vq_data *data, bool en)
{
	if (en)
		pm_runtime_get_sync(data->dev);
	else
		pm_runtime_put_sync(data->dev);
}

int mtk_vq_power_on(struct vq_data *data, enum VQ_PATH_MODE vq_mode)
{
	int ret;

	mtk_vq_pm_en(data, true);

	if (vq_mode == VQ_NR_STANDALONE) {
		ret = nr_hal_power_on(&data->nr);
		if (ret) {
			mtk_vq_pm_en(data, false);
			return ret;
		}
	} else if (vq_mode == VQ_DI_STANDALONE) {
		ret = di_hal_power_on(&data->di);
		if (ret) {
			mtk_vq_pm_en(data, false);
			return ret;
		}
	} else {
		ret = nr_hal_power_on(&data->nr);
		if (ret) {
			mtk_vq_pm_en(data, false);
			return ret;
		}
		ret = di_hal_power_on(&data->di);
		if (ret) {
			mtk_vq_pm_en(data, false);
			return ret;
		}
	}

	mtk_vq_smi_larb_en(data, true);

	VQ_INFO("mtk_vq_power_on end\n");

	return ret;
}

int mtk_vq_power_off(struct vq_data *data, enum VQ_PATH_MODE vq_mode)
{
	mtk_vq_smi_larb_en(data, false);

	if (vq_mode == VQ_NR_STANDALONE)
		nr_hal_power_off(&data->nr);
	else if (vq_mode == VQ_DI_STANDALONE)
		di_hal_power_off(&data->di);
	else {
		nr_hal_power_off(&data->nr);
		di_hal_power_off(&data->di);
	}

	mtk_vq_pm_en(data, false);

	VQ_INFO("mtk_vq_power_off end\n");

	return 0;
}

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
static int mtk_vq_mgr_request_irq(struct vq_data *data, struct mtk_vq_config *config)
{
	if (config->vq_mode == VQ_NR_STANDALONE)
		return nr_hal_request_irq(&data->nr);
	else
		return 0;
}

static void mtk_vq_mgr_free_irq(struct vq_data *data, struct mtk_vq_config *config)
{
	if (config->vq_mode == VQ_NR_STANDALONE)
		nr_hal_free_irq(&data->nr);
	else if (config->vq_mode == VQ_DI_STANDALONE)
		di_hal_free_irq(&data->di);
}

static int mtk_vq_mgr_set_input_buffer_secure(struct vq_data *data,
					      struct mtk_vq_config_info *vq_config_info)
{
	unsigned int paramTypes = 0;
	MTEEC_PARAM vq_param[4];
	TZ_RESULT ret = 0;
	unsigned int i;
	unsigned int pre_buf_num = 0;

	if (vq_config_info->vq_config->vq_mode == VQ_DI_STANDALONE) {
		VQ_ERR("DI not need do SVP\n");
		return ret;
	}
	if (vq_config_info->vq_config->vq_mode == VQ_NR_STANDALONE)
		pre_buf_num = 1;
	else
		pre_buf_num = MTK_VQ_DI_INPUTBUFFER;

	for (i = 0; i < pre_buf_num; i++)
		vq_config_info->vq_config->src_fd[i] = vq_config_info->src_mva[i];
	vq_config_info->vq_config->dst_fd = vq_config_info->dst_mva;

	if (data->vq_session == 0) {
		VQ_ERR("kree session is error\n");
		return -ENXIO;
	}

	mtk_vq_mgr_free_irq(data, vq_config_info->vq_config);

	paramTypes = TZ_ParamTypes1(TZPT_MEM_INPUT);

	vq_param[0].mem.buffer = (void *)vq_config_info->vq_config;
	vq_param[0].mem.size = sizeof(struct mtk_vq_config);

	ret =
	    KREE_TeeServiceCall(data->vq_session, VQ_SERVICE_CALL_CMD_CONFIG, paramTypes, vq_param);
	if (ret != TZ_RESULT_SUCCESS)
		VQ_ERR("service call fail: cmd[%d] ret[%d]\n", VQ_SERVICE_CALL_CMD_CONFIG, ret);

	ret = mtk_vq_mgr_request_irq(data, vq_config_info->vq_config);
	if (ret)
		VQ_ERR("[failed to request irq (%d)\n", ret);

	return ret;
}
#endif

static int mtk_vq_mgr_set_input_buffer_normal(struct vq_data *data,
					      struct mtk_vq_config_info *vq_config_info)
{
	int ret = 0;
	unsigned long start_timer, end_timer;

	switch (vq_config_info->vq_config->vq_mode) {
	case VQ_DI_NR_DIRECTLINK_ALL_ENABLE:
	case VQ_DI_NR_DIRECTLINK_DI_BYPASS:
	case VQ_DI_NR_DIRECTLINK_NR_BYPASS:
		start_timer = sched_clock();
		ret = nr_hal_set_info(data->nr.nr_reg_base, vq_config_info);
		ret = di_hal_config(vq_config_info);
		end_timer = sched_clock();
		if (vq_debug_enable.vq_time_enable)
			pr_info("[VQ] hw time is %ld di->nr hal_config s:%ld,end:%ld\n",
				(end_timer - start_timer) / 1000000, start_timer / 1000000,
				end_timer / 1000000);
		/* set di->nr config */
		nr_hal_hw_reset(data->nr.nr_reg_base);
		break;

	case VQ_DI_STANDALONE:

		start_timer = sched_clock();
		ret = di_hal_config(vq_config_info);
		end_timer = sched_clock();
		if (vq_debug_enable.vq_time_enable)
			pr_info("[DI] hw time is %ld di_hal_config s:%ld,end:%ld\n",
				(end_timer - start_timer) / 1000000, start_timer / 1000000,
				end_timer / 1000000);

		break;

	case VQ_NR_STANDALONE:
		start_timer = sched_clock();
		ret = nr_hal_set_info(data->nr.nr_reg_base, vq_config_info);
		ret = nr_hal_wait_complete_timeout(&data->nr, 1000);
		end_timer = sched_clock();
		if (vq_debug_enable.vq_time_enable)
			pr_info("[NR] hw timer is %ld\n", end_timer - start_timer);
		break;
	default:
		VQ_ERR("unknown vq mode %d\n", vq_config_info->vq_config->vq_mode);
		break;
	}

	return ret;
}

int mtk_vq_mgr_set_input_buffer(struct vq_data *data, struct mtk_vq_config *config)
{
	int ret = 0;
	unsigned long start_timer, end_timer;

	struct mtk_vq_config_info vq_config_info;

	VQ_INFO("type %d, mnr %d bnr %d\n", config->vq_mode, config->mnr_level, config->bnr_level);

	vq_config_info.vq_config = config;
	start_timer = sched_clock();

	ret = mtk_vq_mgr_prepare_buffer(data->client, &vq_config_info);
	if (ret) {
		VQ_ERR("prepare buffer error\n");
		return -EFAULT;
	}
	end_timer = sched_clock();
	if (vq_debug_enable.vq_time_enable)
		pr_info("[VQ] prepare total time is %ld,s:%ld,end:%ld\n",
			(end_timer - start_timer) / 1000000, start_timer / 1000000,
			end_timer / 1000000);

	start_timer = sched_clock();
	ret = mtk_vq_power_on(data, config->vq_mode);
	if (ret) {
		VQ_ERR("power on clk error\n");
		return -EFAULT;
	}

	nr_hal_switch_mode(data->disp_top_reg_base, config->vq_mode);

	VQ_INFO("the size (%d, %d), align size (%d, %d), mode %d\n",
		config->src_width, config->src_height,
		config->src_align_width, config->src_align_height, config->vq_mode);

	VQ_INFO("src offset(%d, %d), dst offset(%d %d), level(%d %d)\n",
		config->src_ofset_y_len[0], config->src_ofset_c_len[0],
		config->dst_ofset_y_len, config->dst_ofset_c_len,
		config->bnr_level, config->mnr_level);
	end_timer = sched_clock();
	if (vq_debug_enable.vq_time_enable)
		pr_info("[VQ] power total time is %ld,s:%ld,end:%ld\n",
			(end_timer - start_timer) / 1000000, start_timer / 1000000,
			end_timer / 1000000);

	start_timer = sched_clock();
#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	if (config->secruity_en)
		ret = mtk_vq_mgr_set_input_buffer_secure(data, &vq_config_info);
	else
		ret = mtk_vq_mgr_set_input_buffer_normal(data, &vq_config_info);
#else
	ret = mtk_vq_mgr_set_input_buffer_normal(data, &vq_config_info);
#endif
	end_timer = sched_clock();
	if (vq_debug_enable.vq_time_enable)
		pr_info("mtk_vq_mgr_set_input_buffer_normal end  %ld,s:%ld,end:%ld\n",
			(end_timer - start_timer) / 1000000, start_timer / 1000000,
			end_timer / 1000000);

	start_timer = sched_clock();
	mtk_vq_power_off(data, config->vq_mode);
	end_timer = sched_clock();
	if (vq_debug_enable.vq_time_enable)
		pr_info("mtk_vq_power_off end  %ld,s:%ld,end:%ld\n",
			(end_timer - start_timer) / 1000000, start_timer / 1000000,
			end_timer / 1000000);

	start_timer = sched_clock();
	ret = mtk_vq_mgr_free_buffer_handle(data->client, &vq_config_info);
	end_timer = sched_clock();
	if (vq_debug_enable.vq_time_enable)
		pr_info("mtk_vq_mgr_free_buffer_handle end  %ld,s:%ld,end:%ld\n",
			(end_timer - start_timer) / 1000000, start_timer / 1000000,
			end_timer / 1000000);

	return ret;
}


static long mtk_vq_mgr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long start_timer, end_timer;
	void __user *argp = (void __user *)arg;
	long ret = 0;
	struct mtk_vq_config config;
	struct vq_data *data = mtk_vq_get_data();

	start_timer = sched_clock();

	switch (cmd) {
	case MTK_VQ_IOCTL_SET_INPUT_CONFIG:
		{
			if (copy_from_user(&config, argp, sizeof(config)))
				return -EFAULT;

			ret = mtk_vq_mgr_set_input_buffer(data, &config);

			if (!ret && copy_to_user(argp, &config, sizeof(config)))
				ret = -EFAULT;
			break;
		}
	default:
		VQ_ERR("error ioctl cmd %d\n", cmd);
		break;
	}

	end_timer = sched_clock();

	if (vq_debug_enable.vq_time_enable)
		pr_info("[VQ] mtk_vq_mgr_ioctl total time is %ld\n",
			(end_timer - start_timer) / 1000000);

	return 0;
}



#ifdef CONFIG_COMPAT
static long mtk_vq_mgr_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return mtk_vq_mgr_ioctl(file, cmd, arg);
}
#endif

static int mtk_vq_mgr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_vq_mgr_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations mtk_vq_mgr_fops = {
	.owner = THIS_MODULE,
	.open = mtk_vq_mgr_open,
	.unlocked_ioctl = mtk_vq_mgr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mtk_vq_mgr_compat_ioctl,
#endif
	.release = mtk_vq_mgr_release,
};

static int mtk_vq_create_device(struct platform_device *pdev)
{
	int ret = 0;

	VQ_INFO("[VQ]mtk_vq_create_device\n");

	alloc_chrdev_region(&vq_devno, 0, 1, VQ_SESSION_DEVICE);
	vq_cdev = cdev_alloc();
	vq_cdev->owner = THIS_MODULE;
	vq_cdev->ops = &mtk_vq_mgr_fops;
	cdev_add(vq_cdev, vq_devno, 1);
	vq_class = class_create(THIS_MODULE, VQ_SESSION_DEVICE);
	device_create(vq_class, NULL, vq_devno, NULL, VQ_SESSION_DEVICE);

	return ret;
}

static int mtk_vq_mgr_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct vq_data *data;
	struct device_node *larb_node;
	struct resource *res;
	struct platform_device *larb_pdev;

	/* mtk_vq_set_timer_enable(1); */
	/* mtk_vq_set_log_enable(1); */

	larb_node = of_parse_phandle(pdev->dev.of_node, "mediatek,larb", 0);
	if (!larb_node) {
		dev_notice(dev, "[VQ] get larbs np fail\n");
		return -EINVAL;
	}

	larb_pdev = of_find_device_by_node(larb_node);
	of_node_put(larb_node);
	if ((!larb_pdev) || (!larb_pdev->dev.driver)) {
		dev_notice(dev, "[VQ]vq_probe is earlier than SMI\n");
		return -EPROBE_DEFER;
	}

	ret = mtk_vq_create_device(pdev);

	data = devm_kzalloc(dev, sizeof(struct vq_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->larb_dev = &larb_pdev->dev;
	data->dev = dev;
	VQ_INFO("[VQ]nr_hal_hw_init\n");

	ret = nr_hal_hw_init(pdev, &data->nr);
	if (ret == -EPROBE_DEFER) {
		dev_notice(dev, "[VQ] get nr clk earlier than dispsys\n");
		return -EPROBE_DEFER;
	} else if (ret < 0) {
		dev_notice(dev, "[VQ] error get nr info fail\n");
		return -EINVAL;
	}

	VQ_INFO("[VQ]di_hal_hw_init\n");
	ret = di_hal_hw_init(pdev, &data->di);
	if (ret == -EPROBE_DEFER) {
		dev_notice(dev, "[VQ] get nr clk earlier than dispsys\n");
		return -EPROBE_DEFER;
	} else if (ret < 0) {
		dev_notice(dev, "[VQ] error get nr info fail\n");
		return -EINVAL;
	}

	data->client = vq_ion_init();
	if (!data->client)
		dev_notice(dev, "[VQ] alloc ion client error\n");

	VQ_INFO("[VQ]vq ion client is 0x%p\n", data->client);

	res = platform_get_resource(pdev, IORESOURCE_MEM, MTK_VQ_REG_DISP_TOP);
	if (!res) {
		dev_notice(dev, "[VQ]failed to get disp top resource 0\n");
		return -ENXIO;
	}
	data->disp_top_reg_base = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(data->disp_top_reg_base)) {
		dev_notice(dev, "[VQ]get disp top reg base err\n");
		return PTR_ERR(data->disp_top_reg_base);
	}
	VQ_INFO("[VQ] disp top reg base is 0x%lx\n", (unsigned long)data->disp_top_reg_base);

#ifdef CONFIG_MTK_IN_HOUSE_TEE_SUPPORT
	if (data->vq_session == 0) {
		ret = KREE_CreateSession(TZ_TA_VQ_UUID, &data->vq_session);
		if (ret != TZ_RESULT_SUCCESS) {
			VQ_ERR("create vq_session fail:%d\n", ret);
			return -ENXIO;
		}
	}

	VQ_INFO("create KREE session is %d\n", data->vq_session);
#endif

	dev_set_drvdata(dev, data);

	pm_runtime_enable(dev);

	vq_data_info = data;

	return 0;
}

static int mtk_vq_mgr_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;

	struct vq_data *data = dev_get_drvdata(dev);

	pm_runtime_disable(dev);

	vq_ion_deinit(data->client);

	return ret;
}

static const struct of_device_id mgr_of_ids[] = {
	{.compatible = "mediatek,mt8695-vq",},
	{}
};

static struct platform_driver mtk_vq_mgr_driver = {
	.probe = mtk_vq_mgr_probe,
	.remove = mtk_vq_mgr_remove,
	.driver = {
		   .name = VQ_SESSION_DEVICE,
		   .owner = THIS_MODULE,
		   .of_match_table = mgr_of_ids,
		   },
};

static int __init mtk_vq_mgr_init(void)
{
	VQ_INFO("[VQ]mtk_nr_mgr_init in\n");

	if (platform_driver_register(&mtk_vq_mgr_driver))
		return -ENODEV;

	VQ_INFO("[VQ]mtk_vq_mgr_init out\n");
	return 0;
}

static void __exit mtk_vq_mgr_exit(void)
{
	device_destroy(vq_class, vq_devno);
	class_destroy(vq_class);
	cdev_del(vq_cdev);
	unregister_chrdev_region(vq_devno, 1);

	platform_driver_unregister(&mtk_vq_mgr_driver);
}
module_init(mtk_vq_mgr_init);
module_exit(mtk_vq_mgr_exit);
MODULE_DESCRIPTION("mediatek vq manager");
MODULE_LICENSE("GPL");
