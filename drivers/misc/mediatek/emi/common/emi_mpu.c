/*
 * Copyright (C) 2015 MediaTek Inc.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <emi.h>
#include <mt-plat/mtk_io.h>
#include <mt-plat/sync_write.h>

static unsigned int emi_physical_offset;

void __iomem *EMI_MPU_BASE;
void __iomem *CEN_EMI_BASE;
void __iomem *CH0_EMI_BASE;
void __iomem *CH1_EMI_BASE;
void __iomem *CHA_DRAMC_AO_BASE;
void __iomem *CHA_DRAMC_NAO_BASE;
void __iomem *CHA_DDRPHY_AO_BASE;
void __iomem *CHA_DDRPHY_NAO_BASE;
void __iomem *CHB_DRAMC_AO_BASE;
void __iomem *CHB_DRAMC_NAO_BASE;
void __iomem *CHB_DDRPHY_AO_BASE;
void __iomem *CHB_DDRPHY_NAO_BASE;

static unsigned int emi_reg_read(void __iomem *addr)
{
	unsigned int reg_val;

	reg_val = readl(IOMEM(addr));
	return reg_val;
}

static void emi_reg_write(unsigned int val, void __iomem *addr)
{
	mt_reg_sync_writel(val, addr);
}

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission set by SET_ACCESS_PERMISSON
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(unsigned int start, unsigned int end, int region, unsigned int access_permission)
{
	unsigned int tmp;
	unsigned int start_addr, end_addr;

	/* shift 0x40000000 */
	start_addr = start - emi_physical_offset;
	end_addr = (end - 1) - emi_physical_offset;

	switch (region) {
	case 0:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA0);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA0);
		tmp = emi_reg_read(EMI_MPU_APC0) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC0);
		break;

	case 1:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA1);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA1);
		tmp = emi_reg_read(EMI_MPU_APC1) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC1);
		break;

	case 2:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA2);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA2);
		tmp = emi_reg_read(EMI_MPU_APC2) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC2);
		break;

	case 3:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA3);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA3);
		tmp = emi_reg_read(EMI_MPU_APC3) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC3);
		break;

	case 4:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA4);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA4);
		tmp = emi_reg_read(EMI_MPU_APC4) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC4);
		break;

	case 5:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA5);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA5);
		tmp = emi_reg_read(EMI_MPU_APC5) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC5);
		break;

	case 6:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA6);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA6);
		tmp = emi_reg_read(EMI_MPU_APC6) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC6);
		break;

	case 7:
		emi_reg_write((start_addr >> 16), EMI_MPU_SA7);
		emi_reg_write((end_addr >> 16), EMI_MPU_EA7);
		tmp = emi_reg_read(EMI_MPU_APC7) & 0xFF000000;
		emi_reg_write(access_permission | tmp, EMI_MPU_APC7);
		break;
	}

	return 0;

}
EXPORT_SYMBOL(emi_mpu_set_region_protection);

void *mt_cen_emi_base_get(void)
{
	return CEN_EMI_BASE;
}
EXPORT_SYMBOL(mt_cen_emi_base_get);

void *mt_chn_emi_base_get(int channel)
{
	switch (channel) {
	case 0:
		return CH0_EMI_BASE;
	case 1:
		return CH1_EMI_BASE;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(mt_chn_emi_base_get);

void *mt_dramc_chn_base_get(int channel)
{
	switch (channel) {
	case 0:
		return CHA_DRAMC_AO_BASE;
	case 1:
		return CHB_DRAMC_AO_BASE;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(mt_dramc_chn_base_get);

void *mt_dramc_nao_chn_base_get(int channel)
{
	switch (channel) {
	case 0:
		return CHA_DRAMC_NAO_BASE;
	case 1:
		return CHB_DRAMC_NAO_BASE;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(mt_dramc_nao_chn_base_get);

void *mt_ddrphy_chn_base_get(int channel)
{
	switch (channel) {
	case 0:
		return CHA_DDRPHY_AO_BASE;
	case 1:
		return CHB_DDRPHY_AO_BASE;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(mt_ddrphy_chn_base_get);
void *mt_ddrphy_nao_chn_base_get(int channel)
{
	switch (channel) {
	case 0:
		return CHA_DDRPHY_NAO_BASE;
	case 1:
		return CHB_DDRPHY_NAO_BASE;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(mt_ddrphy_nao_chn_base_get);

int get_dram_data_rate(void)
{
	return 3200;
}
EXPORT_SYMBOL(get_dram_data_rate);

int get_ddr_type(void)
{
	return TYPE_LPDDR4;
}
EXPORT_SYMBOL(get_ddr_type);

static ssize_t emi_mpu_show(struct device_driver *driver, char *buf)
{
	char *ptr = buf;
	unsigned int start, end;
	unsigned int reg_value;
	unsigned int d0, d1, d2, d3;
	static const char *permission[8] = {
		"No protect",
		"Only R/W for secure access",
		"Only R/W for secure access, and non-secure read access",
		"Only R/W for non-secure access",
		"Only R for secure/non-secure",
		"Both R/W are forbidden",
		"Only secure W is forbidden",
		"Only R for secure access"
	};

	reg_value = emi_reg_read(EMI_MPU_SA0);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA0);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 0 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA1);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA1);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 1 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA2);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA2);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 2 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA3);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA3);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 3 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA4);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA4);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 4 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA5);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA5);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 5 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA6);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA6);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 6 --> 0x%x to 0x%x\n", start, end);

	reg_value = emi_reg_read(EMI_MPU_SA7);
	start = ((reg_value << 16) + emi_physical_offset);
	reg_value = emi_reg_read(EMI_MPU_EA7);
	end = ((reg_value  << 16) + emi_physical_offset + 0xFFFF);
	ptr += sprintf(ptr, "Region 7 --> 0x%x to 0x%x\n", start, end);

	ptr += sprintf(ptr, "\n");

	reg_value = emi_reg_read(EMI_MPU_APC0);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 0 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC1);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 1 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC2);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 2 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC3);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 3 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC4);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 4 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC5);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 5 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC6);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 6 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	reg_value = emi_reg_read(EMI_MPU_APC7);
	d0 = (reg_value & 0x7);
	d1 = (reg_value >> 3) & 0x7;
	d2 = (reg_value >> 6) & 0x7;
	d3 = (reg_value >> 9) & 0x7;
	ptr += sprintf(ptr, "Region 7 --> d0 = %s, d1 = %s, d2 = %s, d3 = %s\n",
					permission[d0],  permission[d1],  permission[d2], permission[d3]);

	return ptr-buf;
}

static ssize_t emi_mpu_store(struct device_driver *driver, const char *buf, size_t count)
{
	int i;
	int ret = 0;
	unsigned int start_addr;
	unsigned int end_addr;
	unsigned int region;
	unsigned int access_permission;
	char *command;
	char *ptr;
	char *token[5];

	if ((strlen(buf) + 1) > MAX_EMI_MPU_STORE_CMD_LEN) {
		pr_err("emi_mpu_store command overflow.");
		return count;
	}
	pr_err("emi_mpu_store: %s\n", buf);

	command = kmalloc((size_t)MAX_EMI_MPU_STORE_CMD_LEN, GFP_KERNEL);
	if (!command)
		return count;

	strcpy(command, buf);
	ptr = (char *)buf;

	if (!strncmp(buf, EN_MPU_STR, strlen(EN_MPU_STR))) {
		i = 0;
		while (ptr != NULL) {
			ptr = strsep(&command, " ");
			token[i] = ptr;
			pr_devel("token[%d] = %s\n", i, token[i]);
			i++;
		}
		for (i = 0; i < 5; i++)
			pr_devel("token[%d] = %s\n", i, token[i]);

		ret += kstrtoul(token[1], 16, (unsigned long *) &start_addr);
		ret += kstrtoul(token[2], 16, (unsigned long *) &end_addr);
		ret += kstrtoul(token[3], 16, (unsigned long *) &region);
		ret += kstrtoul(token[4], 16, (unsigned long *) &access_permission);

		if (ret) {
			pr_err("fail to parse command.\n");
			return -1;
		}

		emi_mpu_set_region_protection(start_addr, end_addr, region, access_permission);
		pr_err("Set EMI_MPU: start: 0x%x, end: 0x%x, region: %d, permission: 0x%x.\n",
				start_addr, end_addr, region, access_permission);
	} else if (!strncmp(buf, DIS_MPU_STR, strlen(DIS_MPU_STR))) {
		i = 0;
		while (ptr != NULL) {
			ptr = strsep(&command, " ");
			token[i] = ptr;
			pr_devel("token[%d] = %s\n", i, token[i]);
			i++;
		}
		for (i = 0; i < 5; i++)
			pr_devel("token[%d] = %s\n", i, token[i]);

		ret += kstrtoul(token[1], 16, (unsigned long *) &start_addr);
		ret += kstrtoul(token[2], 16, (unsigned long *) &end_addr);
		ret += kstrtoul(token[3], 16, (unsigned long *) &region);

		if (ret) {
			pr_err("fail to parse command.\n");
			return -1;
		}

		emi_mpu_set_region_protection(0x0, 0x0, region,
							SET_ACCESS_PERMISSON(
							NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
							NO_PROTECTION));
	} else
		pr_err("Unknown emi_mpu command.\n");

	kfree(command);

	return count;
}

DRIVER_ATTR(mpu_config, 0644, emi_mpu_show, emi_mpu_store);


static struct platform_driver emi_mpu_ctrl_platform_drv = {
	.driver = {
		.name = "emi_mpu_ctrl",
		.bus = &platform_bus_type,
		.owner = THIS_MODULE,
	}
};

static int __init emi_mpu_mod_init(void)
{
	int ret;
	struct device_node *node;

	/* DTS version */
	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-emi_mpu");
	if (node)
		EMI_MPU_BASE = of_iomap(node, 0);
	else
		return -ENODEV;

	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-emi");
	if (!node)
		return -ENODEV;

	CEN_EMI_BASE = of_iomap(node, 0);
	if (!CEN_EMI_BASE) {
		pr_err("Fail to get CEN_EMI_BASE.\n");
		return -ENOMEM;
	}

	CH0_EMI_BASE = of_iomap(node, 1);
	if (!CH0_EMI_BASE) {
		pr_err("Fail to get CH0_EMI_BASE.\n");
		return -ENOMEM;
	}

	CH1_EMI_BASE = of_iomap(node, 2);
	if (!CH1_EMI_BASE) {
		pr_err("Fail to get CH1_EMI_BASE.\n");
		return -ENOMEM;
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-dramc");
	if (!node)
		return -ENODEV;

	CHA_DDRPHY_AO_BASE = of_iomap(node, 0);
	if (!CHA_DDRPHY_AO_BASE) {
		pr_err("Fail to get CHA_DDRPHY_AO_BASE.\n");
		return -ENOMEM;
	}

	CHA_DRAMC_AO_BASE = of_iomap(node, 1);
	if (!CHA_DRAMC_AO_BASE) {
		pr_err("Fail to get CHA_DRAMC_AO_BASE.\n");
		return -ENOMEM;
	}

	CHA_DRAMC_NAO_BASE = of_iomap(node, 2);
	if (!CHA_DRAMC_NAO_BASE) {
		pr_err("Fail to get CHA_DRAMC_NAO_BASE.\n");
		return -ENOMEM;
	}

	CHA_DDRPHY_NAO_BASE = of_iomap(node, 3);
	if (!CHA_DDRPHY_NAO_BASE) {
		pr_err("Fail to get CHA_DDRPHY_NAO_BASE.\n");
		return -ENOMEM;
	}

	CHB_DDRPHY_AO_BASE = of_iomap(node, 4);
	if (!CHB_DDRPHY_AO_BASE) {
		pr_err("Fail to get CHB_DDRPHY_AO_BASE.\n");
		return -ENOMEM;
	}

	CHB_DRAMC_AO_BASE = of_iomap(node, 5);
	if (!CHB_DRAMC_AO_BASE) {
		pr_err("Fail to get CHB_DRAMC_AO_BASE.\n");
		return -ENOMEM;
	}

	CHB_DRAMC_NAO_BASE = of_iomap(node, 6);
	if (!CHB_DRAMC_NAO_BASE) {
		pr_err("Fail to get CHB_DRAMC_NAO_BASE.\n");
		return -ENOMEM;
	}

	CHB_DDRPHY_NAO_BASE = of_iomap(node, 7);
	if (!CHB_DDRPHY_NAO_BASE) {
		pr_err("Fail to get CHB_DDRPHY_NAO_BASE.\n");
		return -ENOMEM;
	}

	emi_physical_offset = 0x40000000;

	/* register driver and create sysfs files */
	ret = platform_driver_register(&emi_mpu_ctrl_platform_drv);
	if (ret) {
		pr_err("Fail to register EMI_MPU driver, ret: %d\n", ret);
		return ret;
	}

	ret = driver_create_file(&emi_mpu_ctrl_platform_drv.driver, &driver_attr_mpu_config);
	if (ret) {
		pr_err("Fail to create MPU config sysfs file, ret: %d\n", ret);
		return ret;
	}

	return 0;
}

static void __exit emi_mpu_mod_exit(void)
{
}

module_init(emi_mpu_mod_init);
module_exit(emi_mpu_mod_exit);
