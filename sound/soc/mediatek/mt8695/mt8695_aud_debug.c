/*
 * Mediatek audio debug function
 *
 * Copyright (c) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include "mt8695-afe-common.h"

struct mt8695_afe_debug_fs {
	char *fs_name;
	const struct file_operations *fops;
};

static struct dentry *debugfs_dentry[2];

static ssize_t mt8695_afe_reg_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos)
{

	return 0;
}


static ssize_t mt8695_afe_reg_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos)
{
	char buf[1024];
	size_t buf_size;
	char *start = buf;
	char *reg_str, *value_str, *flag;
	const char delim[] = " ";
	unsigned int reg, value;
	struct mtk_afe *afe = file->private_data;
	int ret, i;

	buf_size = min(count, (sizeof(buf) - 1));

	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = 0;

	reg_str = strsep(&start, delim);
	if (!reg_str || !strlen(reg_str)) {
		dev_info(afe->dev, "reg_str get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	value_str = strsep(&start, delim);
	if (!value_str || !strlen(value_str)) {
		dev_info(afe->dev, "value_str get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	flag = strsep(&start, delim);
	if (!flag || !strlen(flag)) {
		dev_info(afe->dev, "flag get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	if (strncmp(reg_str, "0x", 2) == 0) {
		if (kstrtoul(&reg_str[2], 16, (unsigned long *)&reg)) {
			dev_info(afe->dev, "reg_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	} else {
		if (kstrtoul(reg_str, 10, (unsigned long *)&reg)) {
			dev_info(afe->dev, "reg_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	}

	if (reg & 0x3)
		reg = reg & (~0x3);

	if (strncmp(value_str, "0x", 2) == 0) {
		if (kstrtoul(&value_str[2], 16, (unsigned long *)&value)) {
			dev_info(afe->dev, "value_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	} else {
		if (kstrtoul(value_str, 10, (unsigned long *)&value)) {
			dev_info(afe->dev, "value_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	}

	if (value == 0)
		value = 1;

	if (strncmp(flag, "w", 1) == 0) {
		regmap_write(afe->regmap, reg, value);
		pr_info("Written done!\n");
		return buf_size;
	} else if (strncmp(flag, "r", 1) == 0) {
		unsigned int reg_value;

		pr_info("Read Result:");
		for (i = 0; i < value; i++) {
			regmap_read(afe->regmap, (reg + i * 4), &reg_value);
			pr_info("0x%4x | 0x%x\n", (reg + i * 4), reg_value);
		}
		pr_info("\n");
		return buf_size;
	}
usage:
	dev_info(afe->dev, "usage: write reg\n");
	dev_info(afe->dev, "      echo [reg offset] [reg value] w > /sys/kernel/debug/afe_reg\n");
	dev_info(afe->dev, "      ex: echo 0x123456789 0x77777777 w > /sys/kernel/debug/afe_reg\n");
	dev_info(afe->dev, "       read reg\n");
	dev_info(afe->dev, "      echo [reg offset] [read count] r > /sys/kernel/debug/afe_reg\n");
	dev_info(afe->dev, "      ex: echo 0x123456789 8 r > /sys/kernel/debug/afe_reg\n");
	return ret;
}

static ssize_t mt8695_top_reg_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos)
{
	return 0;
}

static ssize_t mt8695_top_reg_write(struct file *file, const char __user *user_buf, size_t count, loff_t *pos)
{
	char buf[1024];
	size_t buf_size;
	char *start = buf;
	char *reg_str, *value_str, *flag;
	const char delim[] = " ";
	unsigned int reg, value;
	struct mtk_afe *afe = file->private_data;
	int ret, i;

	buf_size = min(count, (sizeof(buf) - 1));

	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = 0;

	reg_str = strsep(&start, delim);
	if (!reg_str || !strlen(reg_str)) {
		dev_info(afe->dev, "reg_str get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	value_str = strsep(&start, delim);
	if (!value_str || !strlen(value_str)) {
		dev_info(afe->dev, "value_str get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	flag = strsep(&start, delim);
	if (!flag || !strlen(flag)) {
		dev_info(afe->dev, "flag get failed!\n");
		ret = -EINVAL;
		goto usage;
	}

	if (strncmp(reg_str, "0x", 2) == 0) {
		if (kstrtoul(&reg_str[2], 16, (unsigned long *)&reg)) {
			dev_info(afe->dev, "reg_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	} else {
		if (kstrtoul(reg_str, 10, (unsigned long *)&reg)) {
			dev_info(afe->dev, "reg_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	}
	if (reg & 0x3)
		reg = reg & (~0x3);

	if (strncmp(value_str, "0x", 2) == 0) {
		if (kstrtoul(&value_str[2], 16, (unsigned long *)&value)) {
			dev_info(afe->dev, "value_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	} else {
		if (kstrtoul(value_str, 10, (unsigned long *)&value)) {
			dev_info(afe->dev, "value_str not number!\n");
			ret = -EINVAL;
			goto usage;
		}
	}
	if (value == 0)
		value = 1;

	if (strncmp(flag, "w", 1) == 0) {
		regmap_write(afe->topregmap, reg, value);
		pr_info("Written done!\n");
		return buf_size;
	} else if (strncmp(flag, "r", 1) == 0) {
		unsigned int reg_value;

		pr_info("Read Result:");
		for (i = 0; i < value; i++) {
			regmap_read(afe->topregmap, (reg + i * 4), &reg_value);
			pr_info("0x%4x | 0x%x\n", (reg + i * 4), reg_value);
		}
		pr_info("\n");
		return buf_size;
	}

usage:
	dev_info(afe->dev, "usage: write reg\n");
	dev_info(afe->dev, "      echo [reg offset] [reg value] w > /sys/kernel/debug/top_reg\n");
	dev_info(afe->dev, "      ex: echo 0x123456789 0x77777777 w > /sys/kernel/debug/top_reg\n");
	dev_info(afe->dev, "       read reg\n");
	dev_info(afe->dev, "      echo [reg offset] [read count] r > /sys/kernel/debug/top_reg\n");
	dev_info(afe->dev, "      ex: echo 0x123456789 8 r > /sys/kernel/debug/top_reg\n");
	return ret;
}

static const struct file_operations afe_reg_ops = {
	.open = simple_open,
	.read = mt8695_afe_reg_read,
	.write = mt8695_afe_reg_write,
	.llseek = default_llseek,
};

static const struct file_operations top_reg_ops = {
	.open = simple_open,
	.read = mt8695_top_reg_read,
	.write = mt8695_top_reg_write,
	.llseek = default_llseek,
};

struct mt8695_afe_debug_fs debug_fs[] = {
	{
		.fs_name = "afe_reg",
		.fops = &afe_reg_ops,
	},
	{
		.fs_name = "top_reg",
		.fops = &top_reg_ops,
	},
};


void mt8695_init_debugfs(struct mtk_afe *afe)
{
#ifdef CONFIG_DEBUG_FS
	int i;

	for (i = 0; i < 2; i++) {
		debugfs_dentry[i] = debugfs_create_file(debug_fs[i].fs_name, 0644, NULL, afe, debug_fs[i].fops);
		if (!debugfs_dentry[i]) {
			dev_err(afe->dev, "[%s]Can't create debug fs file(%s)\n", __func__, debug_fs[i].fs_name);
			break;
		}
	}
	dev_dbg(afe->dev, "%s is start!\n", __func__);
#endif
}

void mt8695_uninit_debugfs(struct mtk_afe *afe)
{
#ifdef CONFIG_DEBUG_FS
	int i;

	for (i = 0; i < 2; i++)
		debugfs_remove(debugfs_dentry[i]);
	dev_info(afe->dev, "%s is stop!\n", __func__);
#endif
}
