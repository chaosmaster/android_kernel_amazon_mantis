/*
 * sign_of_life_vendor.c
 *
 * vendor platform implementation
 * Copyright 2018 Amazon.com, Inc. or its Affiliates. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/sign_of_life.h>
#include <linux/spinlock.h>

extern void __iomem *toprgu_base;
#define MTK_WDT_BASE			toprgu_base
#define MTK_WDT_NONRST_REG2		(MTK_WDT_BASE+0x0024)


static DEFINE_SPINLOCK(rgu_reg_operation_spinlock);

static u32 vendor_reg_read(void)
{
	u32 tmp = 0;

	spin_lock(&rgu_reg_operation_spinlock);
	tmp = __raw_readl(MTK_WDT_NONRST_REG2);
	spin_unlock(&rgu_reg_operation_spinlock);

	return tmp;
}


static void vendor_reg_write(u32 value)
{
	spin_lock(&rgu_reg_operation_spinlock);

	writel(value, MTK_WDT_NONRST_REG2);

	spin_unlock(&rgu_reg_operation_spinlock);

}

static void vendor_set_bit(unsigned int offset)
{
	unsigned int tmp;

	spin_lock(&rgu_reg_operation_spinlock);

	tmp = __raw_readl(MTK_WDT_NONRST_REG2);

	tmp |= 1 << offset;

	writel(tmp, MTK_WDT_NONRST_REG2);

	spin_unlock(&rgu_reg_operation_spinlock);
}

static int (vendor_read_boot_reason)(life_cycle_reason_t *boot_reason)
{
	u32 vendor_breason;

	vendor_breason = vendor_reg_read();

	printk(KERN_INFO"%s: boot_reason is 0x%x\n", __func__, vendor_breason);

	if (vendor_breason & (1<<WD_SOL_WARM_BOOT_KERNEL_WDOG))
		*boot_reason = WARMBOOT_BY_KERNEL_WATCHDOG;
	else if (vendor_breason & (1<<WD_SOL_WARM_BOOT_KERNEL_PANIC))
		*boot_reason = WARMBOOT_BY_KERNEL_PANIC;
	else if (vendor_breason & (1<<WD_SOL_WARM_BOOT_HW_WDOG))
		*boot_reason = WARMBOOT_BY_HW_WATCHDOG;
	else if (vendor_breason & (1<<WD_SOL_WARM_BOOT_SW))
		*boot_reason = WARMBOOT_BY_SW;
	else if (vendor_breason & (1<<WD_SOL_COLD_BOOT_USB))
		*boot_reason = COLDBOOT_BY_USB;
	else if (vendor_breason & (1<<WD_SOL_COLD_BOOT_POWER_KEY))
		*boot_reason = COLDBOOT_BY_POWER_KEY;
	else if (vendor_breason & (1<<WD_SOL_COLD_BOOT_POWER_SUPPLY))
		*boot_reason = COLDBOOT_BY_POWER_SUPPLY;
	else {
		printk(KERN_ERR"Failed to read boot reason\n");
		return -1;
	}

	return 0;
}

static int (vendor_write_boot_reason)(life_cycle_reason_t boot_reason)
{
	u32 vendor_breason;

	vendor_breason = vendor_reg_read();

	printk(KERN_INFO"%s: current 0x%x boot_reason 0x%x\n", __func__, vendor_breason, boot_reason);
	if (boot_reason == WARMBOOT_BY_KERNEL_PANIC)
		vendor_breason = WD_SOL_WARM_BOOT_KERNEL_PANIC;
	else if (boot_reason == WARMBOOT_BY_KERNEL_WATCHDOG)
		vendor_breason = WD_SOL_WARM_BOOT_KERNEL_WDOG;
	else if (boot_reason == WARMBOOT_BY_HW_WATCHDOG)
		vendor_breason = WD_SOL_WARM_BOOT_HW_WDOG;
	else if (boot_reason == WARMBOOT_BY_SW)
		vendor_breason = WD_SOL_WARM_BOOT_SW;
	else if (boot_reason == COLDBOOT_BY_USB)
		vendor_breason = WD_SOL_COLD_BOOT_USB;
	else if (boot_reason == COLDBOOT_BY_POWER_KEY)
		vendor_breason = WD_SOL_COLD_BOOT_POWER_KEY;
	else if (boot_reason == COLDBOOT_BY_POWER_SUPPLY)
		vendor_breason = WD_SOL_COLD_BOOT_POWER_SUPPLY;

	vendor_set_bit(vendor_breason);

	return 0;
}

static int (vendor_read_shutdown_reason)(life_cycle_reason_t *shutdown_reason)
{
	u32 wd_shutdown_reason;

	wd_shutdown_reason = vendor_reg_read();
	printk(KERN_ERR"%s: shutdown reason is 0x%x\n", __func__, wd_shutdown_reason);

	if (wd_shutdown_reason & (1<<WD_SOL_SHUTDOWN_LONG_PWR_KEY_PRESS))
		*shutdown_reason = SHUTDOWN_BY_LONG_PWR_KEY_PRESS;
	else if (wd_shutdown_reason & (1<<WD_SOL_SHUTDOWN_SW))
		*shutdown_reason = SHUTDOWN_BY_SW;
	else if (wd_shutdown_reason & (1<<WD_SOL_SHUTDOWN_PWR_KEY))
		*shutdown_reason = SHUTDOWN_BY_PWR_KEY;
	else if (wd_shutdown_reason & (1<<WD_SOL_SHUTDOWN_SUDDEN_PWR_LOSS))
		*shutdown_reason = SHUTDOWN_BY_SUDDEN_POWER_LOSS;
	else if (wd_shutdown_reason & (1<<WD_SOL_SHUTDOWN_UKNOWN))
		*shutdown_reason = SHUTDOWN_BY_UNKNOWN_REASONS;
	else {
		printk(KERN_ERR"Failed to read shutdown reason\n");
		return -1;
	}

	return 0;
}

static int (vendor_write_shutdown_reason)(life_cycle_reason_t shutdown_reason)
{
	u32 wd_shutdown_reason;

	wd_shutdown_reason = vendor_reg_read();
	printk(KERN_ERR"%s: shutdown_reason 0x%x\n", __func__, wd_shutdown_reason);

	if (shutdown_reason == SHUTDOWN_BY_LONG_PWR_KEY_PRESS)
		wd_shutdown_reason = WD_SOL_SHUTDOWN_LONG_PWR_KEY_PRESS;
	else if (shutdown_reason == SHUTDOWN_BY_SW)
		wd_shutdown_reason = WD_SOL_SHUTDOWN_SW;
	else if (shutdown_reason == SHUTDOWN_BY_PWR_KEY)
		wd_shutdown_reason = WD_SOL_SHUTDOWN_PWR_KEY;
	else if (shutdown_reason == SHUTDOWN_BY_SUDDEN_POWER_LOSS)
		wd_shutdown_reason = WD_SOL_SHUTDOWN_SUDDEN_PWR_LOSS;
	else if (shutdown_reason == SHUTDOWN_BY_UNKNOWN_REASONS)
		wd_shutdown_reason = WD_SOL_SHUTDOWN_UKNOWN;
	else {
		printk(KERN_ERR"Failed to write shutdown reason\n");
		return -1;
	}


	vendor_set_bit(wd_shutdown_reason);

	return 0;
}

static int (vendor_read_thermal_shutdown_reason)(life_cycle_reason_t *thermal_shutdown_reason)
{
	u32 wd_thermal_shutdown_reason;

	wd_thermal_shutdown_reason = vendor_reg_read();

	printk(KERN_ERR"%s: thermal shutdown reason 0x%x\n", __func__, wd_thermal_shutdown_reason);

	if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_BATTERY))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_BATTERY;
	else if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_PMIC))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_PMIC;
	else if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_SOC))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_SOC;
	else if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_MODEM))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_MODEM;
	else if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_WIFI))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_WIFI;
	else if (wd_thermal_shutdown_reason & (1<<WD_SOL_THERMAL_SHUTDOWN_PCB))
		*thermal_shutdown_reason = THERMAL_SHUTDOWN_REASON_PCB;
	else {
		printk(KERN_ERR"Failed to read thermal shutdown reason\n");
		return -1;
	}

	return 0;
}

static int (vendor_write_thermal_shutdown_reason)(life_cycle_reason_t thermal_shutdown_reason)
{
	u32 wd_thermal_shutdown_reason;

	wd_thermal_shutdown_reason = vendor_reg_read();

	printk(KERN_INFO "%s: shutdown_reason 0x%0x\n", __func__, wd_thermal_shutdown_reason);


	if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_BATTERY)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_BATTERY;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_PMIC)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_PMIC;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_SOC)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_SOC;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_MODEM)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_MODEM;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_WIFI)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_WIFI;
	else if (thermal_shutdown_reason == THERMAL_SHUTDOWN_REASON_PCB)
		wd_thermal_shutdown_reason = WD_SOL_THERMAL_SHUTDOWN_PCB;
	else {
		printk(KERN_ERR"Failed to write thermal shutdown reason\n");
		return -1;
	}

	vendor_set_bit(wd_thermal_shutdown_reason);

	return 0;
}

static int (vendor_read_special_mode)(life_cycle_reason_t *special_mode)
{
	u32 wd_smode;

	wd_smode = vendor_reg_read();

	printk(KERN_ERR"%s: special mode is 0x%x\n", __func__, wd_smode);

	if (wd_smode & (1<<WD_SOL_SPECIAL_MODE_LOW_BATTERY))
		*special_mode = LIFE_CYCLE_SMODE_LOW_BATTERY;
	else if (wd_smode & (1<<WD_SOL_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED))
		*special_mode = LIFE_CYCLE_SMODE_WARM_BOOT_USB_CONNECTED;
	else if (wd_smode & (1<<WD_SOL_SPECIAL_MODE_OTA))
		*special_mode = LIFE_CYCLE_SMODE_OTA;
	else if (wd_smode & (1<<WD_SOL_SPECIAL_MODE_FACTORY_RESET))
		*special_mode = LIFE_CYCLE_SMODE_FACTORY_RESET;
	else {
		printk(KERN_ERR"Failed to read special mode\n");
		return -1;
	}
	return 0;
}

static int (vendor_write_special_mode)(life_cycle_reason_t special_mode)
{
	u32 wd_smode;

	wd_smode = vendor_reg_read();
	printk(KERN_ERR"%s: special_mode 0x%x\n", __func__, wd_smode);

	if (special_mode == LIFE_CYCLE_SMODE_LOW_BATTERY)
		wd_smode = WD_SOL_SPECIAL_MODE_LOW_BATTERY;
	else if (special_mode == LIFE_CYCLE_SMODE_WARM_BOOT_USB_CONNECTED)
		wd_smode = WD_SOL_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED;
	else if (special_mode == LIFE_CYCLE_SMODE_OTA)
		wd_smode = WD_SOL_SPECIAL_MODE_OTA;
	else if (special_mode == LIFE_CYCLE_SMODE_FACTORY_RESET)
		wd_smode = WD_SOL_SPECIAL_MODE_FACTORY_RESET;
	else {
		printk(KERN_ERR"Failed to write special mode\n");
		return -1;
	}

	vendor_set_bit(wd_smode);

	return 0;
}

int vendor_lcr_reset(void)
{
	u32 data;

	/* clean up all SoL flags */
	/* might need to add lock */
	data = vendor_reg_read();
	data &= ~(WD_SOL_BOOT_REASON_MASK | WD_SOL_SHUTDOWN_MASK);
	data &= ~(WD_SOL_THERMAL_SHUTDOWN_MASK | WD_SOL_SPECIAL_MODE_MASK);
	vendor_reg_write(data);

	return 0;
}

int life_cycle_platform_init(sign_of_life_ops *sol_ops)
{

	printk(KERN_ERR "%s: Support vendor platform\n", __func__);
	sol_ops->read_boot_reason = vendor_read_boot_reason;
	sol_ops->write_boot_reason = vendor_write_boot_reason;
	sol_ops->read_shutdown_reason = vendor_read_shutdown_reason;
	sol_ops->write_shutdown_reason = vendor_write_shutdown_reason;
	sol_ops->read_thermal_shutdown_reason = vendor_read_thermal_shutdown_reason;
	sol_ops->write_thermal_shutdown_reason = vendor_write_thermal_shutdown_reason;
	sol_ops->read_special_mode = vendor_read_special_mode;
	sol_ops->write_special_mode = vendor_write_special_mode;
	sol_ops->lcr_reset = vendor_lcr_reset;

	return 0;
}

EXPORT_SYMBOL(life_cycle_platform_init);
