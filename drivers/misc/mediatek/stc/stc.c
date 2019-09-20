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

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include "stc.h"


static dev_t stc_devno;
static struct cdev *stc_cdev;
static struct class *stc_class;
static void __iomem *stc_reg_base;
static bool stc_id_using[DMX_STC_NS];


static int alloc_stc(int *id)
{
	int i;

	for (i = 0; i < DMX_STC_NS; i++) {
		if (stc_id_using[i] == false) {
			stc_id_using[i] = true;
			*id = i;
			return 0;
		}
	}
	*id = -1;
	pr_notice("[STC] Error: No free stc hw.\n");

	return -1;
}

static int free_stc(int id)
{
	if (id < DMX_STC_NS)
		stc_id_using[id] = false;
	else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", id, __func__);
		return -1;
	}

	return 0;
}

static void reset_stc_and_run(void)
{
	unsigned int RegValue;

	/* here reset sequence discussed with DE by Huayang */
	RegValue = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	RegValue &= (~STC_CFG_REST_BIT);
	iowrite32(RegValue, (stc_reg_base + STC_SYSTOP_CONFIG));

	RegValue = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	RegValue |= STC_CFG_REST_BIT;
	iowrite32(RegValue, (stc_reg_base + STC_SYSTOP_CONFIG));
}

static void init_stc_hw(void)
{
	unsigned int RegValue;

	/* First enable STC_27M_CLK_ON bit and maybe default on */
	RegValue = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	RegValue |= STC_CFG_CLK_ON_BIT;
	iowrite32(RegValue, (stc_reg_base + STC_SYSTOP_CONFIG));

	reset_stc_and_run();
	pr_debug("[%s] STC start...\n", __func__);
}

static int stop_stc(int id)
{
	unsigned int RegValue;

	RegValue = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	if (id == 0)
		RegValue |= STC_CFG_HOLD_1_BIT;
	else if (id == 1)
		RegValue |= STC_CFG_HOLD_2_BIT;
	else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", id, __func__);
		return -1;
	}

	iowrite32(RegValue, (stc_reg_base + STC_SYSTOP_CONFIG));

	return 0;
}

static int start_stc(int id)
{
	unsigned int RegValue;

	RegValue = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	if (id == 0)
		RegValue &= (~STC_CFG_HOLD_1_BIT);
	else if (id == 1)
		RegValue &= (~STC_CFG_HOLD_2_BIT);
	else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", id, __func__);
		return -1;
	}

	iowrite32(RegValue, (stc_reg_base + STC_SYSTOP_CONFIG));

	return 0;
}

static int get_stc(struct mtk_stc_info *info)
{
	unsigned int StcL = 0;
	uint64_t StcH = 0;

	if (info->stc_id == 0) {
		StcH = ioread32((stc_reg_base + STC_SYSTOP1_VALUE_HIGH));
		StcL = ioread32((stc_reg_base + STC_SYSTOP1_VALUE_LOW));
	} else if (info->stc_id == 1) {
		StcH = ioread32((stc_reg_base + STC_SYSTOP2_VALUE_HIGH));
		StcL = ioread32((stc_reg_base + STC_SYSTOP2_VALUE_LOW));
	} else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);
		return -1;
	}

	StcH &= STC_HIGH_VALUE_MASK;
	StcL &= STC_LOW_VALUE_MASK;
	/*force be 64 bits*/
	info->stc_value = (StcH << 32) | StcL;

	return 0;
}

static bool check_write_ok(struct mtk_stc_info *info)
{
	unsigned int value;

	value = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));

	if (info->stc_id == 0) {
		value &= STC_CFG_SYSTOP_WRITE_STC1_OK_BIT;
	} else if (info->stc_id == 1) {
		value &= STC_CFG_SYSTOP_WRITE_STC2_OK_BIT;
	} else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);
		return false;
	}

	return value ? true : false;
}

static void clear_write_ok_bit(struct mtk_stc_info *info)
{
	unsigned int value;

	/* Enable write clear bit first */
	value = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	if (info->stc_id == 0)
		value |= STC_CFG_SYSTOP_WRITE_STC1_OK_CLR_BIT;
	else if (info->stc_id == 1)
		value |= STC_CFG_SYSTOP_WRITE_STC2_OK_CLR_BIT;
	else
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);

	iowrite32(value, (stc_reg_base + STC_SYSTOP_CONFIG));

	/* Must disable write clear bit secondly */
	value = ioread32((stc_reg_base + STC_SYSTOP_CONFIG));
	if (info->stc_id == 0)
		value &= (~STC_CFG_SYSTOP_WRITE_STC1_OK_CLR_BIT);
	else if (info->stc_id == 1)
		value &= (~STC_CFG_SYSTOP_WRITE_STC2_OK_CLR_BIT);
	else
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);

	iowrite32(value, (stc_reg_base + STC_SYSTOP_CONFIG));
}

static int set_stc(struct mtk_stc_info *info)
{
	unsigned int StcL = (info->stc_value) & STC_LOW_VALUE_MASK;
	unsigned int StcH = (info->stc_value >> 32) & STC_HIGH_VALUE_MASK;

	/* Set register to write STC H & L to STC counter,
	** then must write SYSTOP_STC_H first, and write SYSTOP_STC_L.
	*/
	if (info->stc_id == 0) {
		iowrite32(StcH, (stc_reg_base + STC_SYSTOP1_VALUE_HIGH));
		iowrite32(StcL, (stc_reg_base + STC_SYSTOP1_VALUE_LOW));
	} else if (info->stc_id == 1) {
		iowrite32(StcH, (stc_reg_base + STC_SYSTOP2_VALUE_HIGH));
		iowrite32(StcL, (stc_reg_base + STC_SYSTOP2_VALUE_LOW));
	} else {
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);
		return -1;
	}

	/* Check write success? */
	while (!check_write_ok(info))
		;
	clear_write_ok_bit(info);

	return 0;
}

static int adjust_stc(struct mtk_stc_info *info)
{
	int64_t StcAdjustValue;

	if (info->stc_id > DMX_STC_NS) {
		pr_notice("[STC] Wrong stc id: %d in %s\n", info->stc_id, __func__);
		return -1;
	}

	StcAdjustValue = info->stc_value;
	get_stc(info);
	info->stc_value += StcAdjustValue;
	set_stc(info);

	return 0;
}

static long mtk_stc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int ret = 0;
	int stc_id = 0;
	struct mtk_stc_info info = {0};

	switch (cmd) {
	case MTK_STC_IOCTL_ALLOC:
		ret = alloc_stc(&stc_id);
		if (!ret && put_user(stc_id, (int __user *)arg))
			ret = -EFAULT;
		break;

	case MTK_STC_IOCTL_FREE:
		if (get_user(stc_id, (int __user *)arg))
			return -EFAULT;
		ret = free_stc(stc_id);
		break;

	case MTK_STC_IOCTL_START:
		if (get_user(stc_id, (int __user *)arg))
			return -EFAULT;
		ret = start_stc(stc_id);
		break;

	case MTK_STC_IOCTL_STOP:
		if (get_user(stc_id, (int __user *)arg))
			return -EFAULT;
		ret = stop_stc(stc_id);
		break;

	case MTK_STC_IOCTL_SET:
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		ret = set_stc(&info);
		break;

	case MTK_STC_IOCTL_GET:
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		ret = get_stc(&info);
		if (!ret && copy_to_user(argp, &info, sizeof(info)))
			ret = -EFAULT;
		break;

	case MTK_STC_IOCTL_ADJUST:
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		ret = adjust_stc(&info);
		break;

	default:
		break;
	}

	return ret;
}

static int mtk_stc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_stc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_stc_mmap(struct file *file, struct vm_area_struct *vma)
{
	return 0;
}

static ssize_t mtk_stc_dump(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t mtk_stc_program(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return 0;
}

static DEVICE_ATTR(stc, 0644, mtk_stc_dump, mtk_stc_program);

static const struct file_operations mtk_stc_fops = {
	.owner = THIS_MODULE,
	.open = mtk_stc_open,
	.mmap = mtk_stc_mmap,
	.unlocked_ioctl = mtk_stc_ioctl,
	.compat_ioctl = mtk_stc_ioctl,
	.release = mtk_stc_release,
};

static int stc_parse_dev_node(void)
{
	struct device_node *np;
	unsigned int reg_value;

	np = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-stc");
	if (np == NULL) {
		pr_notice("stc error, no stc device node.\n");
		return STC_FAIL;
	}

	of_property_read_u32_index(np, "reg", 1, &reg_value);

	stc_reg_base = of_iomap(np, 0);
	if (!stc_reg_base) {
		pr_notice("%s: Can't find STC Base!\n", STC_SESSION_DEVICE);
		return STC_FAIL;
	}

	pr_debug("stc reg base PA:0x%016x, VA:0x%p\n", reg_value, stc_reg_base);

	return STC_OK;
}

#if (stc_test_enable)
static int stc_test(void *data)
{
	int i;
	struct mtk_stc_info info1 = {0}, info2 = {0};

	for (i = 0; i < 50; i++) {
		alloc_stc(&info1.stc_id);
		alloc_stc(&info2.stc_id);

		get_stc(&info1);
		get_stc(&info2);
		pr_notice("[STC]T1 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T1 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
		stop_stc(info1.stc_id);
		stop_stc(info2.stc_id);
		get_stc(&info1);
		get_stc(&info2);
		pr_notice("[STC]T2 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T2 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(3*HZ);
		pr_notice("[STC]T3 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T3 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		info1.stc_value = 90000*100;
		info2.stc_value = 90000*100;
		set_stc(&info1);
		get_stc(&info1);
		set_stc(&info2);
		get_stc(&info2);
		pr_notice("[STC]T4 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T4 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		info1.stc_value = 90000*500;
		info2.stc_value = 90000*500;
		adjust_stc(&info1);
		adjust_stc(&info2);
		get_stc(&info1);
		get_stc(&info2);
		pr_notice("[STC]T5 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T5 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		start_stc(info1.stc_id);
		start_stc(info2.stc_id);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(5*HZ);
		get_stc(&info1);
		get_stc(&info2);
		pr_notice("[STC]T6 stc: %lld, id: %d.\n", info1.stc_value, info1.stc_id);
		pr_notice("[STC]T6 stc: %lld, id: %d.\n", info2.stc_value, info2.stc_id);

		free_stc(info1.stc_id);
		free_stc(info2.stc_id);
	}
	return 0;
}
#endif

static int mtk_stc_probe(struct platform_device *pdev)
{
	int ret = 0;

	alloc_chrdev_region(&stc_devno, 0, 1, STC_SESSION_DEVICE);
	stc_cdev = cdev_alloc();
	stc_cdev->owner = THIS_MODULE;
	stc_cdev->ops = &mtk_stc_fops;
	cdev_add(stc_cdev, stc_devno, 1);
	stc_class = class_create(THIS_MODULE, STC_SESSION_DEVICE);
	device_create(stc_class, NULL, stc_devno, NULL, STC_SESSION_DEVICE);

	device_create_file(&pdev->dev, &dev_attr_stc);
	ret = stc_parse_dev_node();
	if (ret == STC_OK)
		init_stc_hw();  /* HW reset maybe need here */

#if (stc_test_enable)
	{
		kthread_run(stc_test, NULL, "stc_test");
	}
#endif

	return ret;
}


static int mtk_stc_remove(struct platform_device *pdev)
{
	int ret = 0;

	iounmap(stc_reg_base);

	device_remove_file(&pdev->dev, &dev_attr_stc);

	cdev_del(stc_cdev);
	unregister_chrdev_region(stc_devno, 1);
	device_destroy(stc_class, stc_devno);
	class_destroy(stc_class);

	return ret;
}

static void mtk_stc_shutdown(struct platform_device *pdev)
{

}

static void mtk_stc_device_release(struct device *dev)
{

}

static u64 mtk_stc_dmamask = ~(u32) 0;

static struct platform_device mtk_stc_device = {
	.name = STC_SESSION_DEVICE,
	.id = 0,
	.dev = {
		.release = mtk_stc_device_release,
		.dma_mask = &mtk_stc_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = 0,
};


static struct platform_driver mtk_stc_driver = {
	.probe = mtk_stc_probe,
	.remove = mtk_stc_remove,
	.shutdown = mtk_stc_shutdown,
	.driver = {
		   .name = STC_SESSION_DEVICE,
		   .owner = THIS_MODULE,
		   },
};


static int __init mtk_stc_init(void)
{
	pr_debug("mtk_stc_init in\n");

	if (platform_device_register(&mtk_stc_device))
		return -ENODEV;

	if (platform_driver_register(&mtk_stc_driver))
		return -ENODEV;

	pr_debug("mtk_stc_init out\n");
	return 0;
}

static void __exit mtk_stc_exit(void)
{
	cdev_del(stc_cdev);
	unregister_chrdev_region(stc_devno, 1);

	platform_driver_unregister(&mtk_stc_driver);

	device_destroy(stc_class, stc_devno);
	class_destroy(stc_class);
}

module_init(mtk_stc_init);
module_exit(mtk_stc_exit);
MODULE_DESCRIPTION("mediatek stc");
MODULE_LICENSE("GPL");
