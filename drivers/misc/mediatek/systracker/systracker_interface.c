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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/memory.h>
#include <asm/system_misc.h>
#include <asm/signal.h>
#include <linux/cpu.h>
#include <mtk_ram_console.h>
#include <mt-plat/sync_write.h>
#include "systracker.h"

#define TRACKER_DEBUG 1

void __iomem *BUS_DBG_BASE;
static unsigned int systracker_irq;
static struct systracker_config_t track_config;
static struct systracker_entry_t track_entry;

static const struct of_device_id systracker_of_ids[] = {
	{.compatible = "mediatek,mt8695-bus_dbg",},
	{}
};

static struct mt_systracker_driver mt_systracker_drv = {
	.driver = {
		   .driver = {
			      .name = "systracker",
			      .bus = &platform_bus_type,
			      .owner = THIS_MODULE,
			      .of_match_table = systracker_of_ids,
			      },
		   .probe = systracker_probe,
		   .remove = systracker_remove,
		   .suspend = systracker_suspend,
		   .resume = systracker_resume,
		   },
	.device = {
		   .name = "systracker",
		   .id = 0,
		   .dev = {
			   },
		   },
	.reset_systracker = NULL,
	.enable_watchpoint = NULL,
	.disable_watchpoint = NULL,
	.set_watchpoint_address = NULL,
	.enable_systracker = NULL,
	.disable_systracker = NULL,
	.test_systracker = NULL,
	.systracker_probe = NULL,
	.systracker_remove = NULL,
	.systracker_suspend = NULL,
	.systracker_resume = NULL,
};


/* Some chip do not have reg dump, define a weak to avoid build error */
extern int mt_reg_dump(char *buf) __attribute__ ((weak));


static void tracker_print_ar(void)
{
	int i;
	unsigned int entry_tid;
	unsigned int entry_address;
	unsigned int entry_burst_length;

	for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
		entry_address = track_entry.ar_track_l[i];
		entry_burst_length = track_entry.ar_track_h[i];
		entry_tid = track_entry.ar_trans_tid[i];

		pr_crit("AR_TRACKER Timeout Entry[%d]: ReadAddr:0x%x, Length:0x%x, TransactionID:0x%x!\n",
				i, entry_address, entry_burst_length, entry_tid);
	}
}

static void tracker_print_aw(void)
{
	int i;
	unsigned int entry_tid;
	unsigned int entry_address;
	unsigned int entry_burst_length;

	for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
		entry_address = track_entry.aw_track_l[i];
		entry_burst_length = track_entry.aw_track_h[i];
		entry_tid = track_entry.aw_trans_tid[i];
		pr_crit("AW_TRACKER Timeout Entry[%d]: WriteAddr:0x%x, Length:0x%x, TransactionID:0x%x!\n",
				i, entry_address, entry_burst_length, entry_tid);
	}
}

static irqreturn_t systracker_isr(void)
{
	unsigned int con;
	static char reg_buf[512];

#ifdef SYSTRACKER_TEST_SUIT
	systracker_test_cleanup();
#endif

	save_entry();
	pr_err("Sys Tracker ISR\n");
	con = readl(IOMEM(BUS_DBG_CON));
	writel(con | BUS_DBG_CON_IRQ_CLR, IOMEM(BUS_DBG_CON));
	mb();

	if (con & BUS_DBG_CON_IRQ_WP_STA) {
		pr_err("[TRACKER] Watch address: 0x%x was touched\n", track_config.wp_phy_address);
		if (mt_reg_dump) {
			if (mt_reg_dump(reg_buf) == 0)
				pr_err("%s\n", reg_buf);
		}
	}

	if (con & BUS_DBG_CON_IRQ_AR_STA) {
		pr_err("[TRAKER] Read time out trigger\n");
		tracker_print_ar();
	}

	if (con & BUS_DBG_CON_IRQ_AW_STA) {
		pr_err("[TRAKER] Write time out trigger\n");
		tracker_print_aw();
	}

	return IRQ_HANDLED;
}

static int systracker_platform_probe_default(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "systracker probe\n");

	/* iomap register */
	BUS_DBG_BASE = of_iomap(pdev->dev.of_node, 0);
	if (BUS_DBG_BASE == NULL) {
		dev_err(&pdev->dev, "can't of_iomap for systracker!!\n");
		return -ENOMEM;
	}
	dev_notice(&pdev->dev, "of_iomap for systracker @ 0x%p\n", BUS_DBG_BASE);

	/* get irq #  */
	systracker_irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	dev_info(&pdev->dev, "%s:%d: irq # %d\n", __func__, __LINE__, systracker_irq);

	pr_err("[systracker] register isr for bus tracer\n");
	if (request_irq(systracker_irq, (irq_handler_t)systracker_isr,
					IRQF_TRIGGER_NONE, "SYSTRACKER", NULL)) {
		pr_err("SYSTRACKER IRQ LINE NOT AVAILABLE!!\n");
		return -1;
	}

	/* save entry info */
	save_entry();

	memset(&track_config, 0, sizeof(struct systracker_config_t));
	/* To latch last PC when tracker timeout, we need to enable interrupt mode */
	track_config.enable_timeout = 1;
	track_config.enable_slave_err = 1;
	track_config.enable_irq = 1;
	track_config.timeout_ms = 50;

	systracker_reset();
	systracker_enable();

	return 0;
}

int systracker_probe(struct platform_device *pdev)
{
	if (mt_systracker_drv.systracker_probe)
		return mt_systracker_drv.systracker_probe(pdev);
	else
		return systracker_platform_probe_default(pdev);
}

int systracker_remove(struct platform_device *pdev)
{
	if (mt_systracker_drv.systracker_remove)
		return mt_systracker_drv.systracker_remove(pdev);
	return 0;
}

int systracker_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (mt_systracker_drv.systracker_suspend)
		return mt_systracker_drv.systracker_suspend(pdev, state);
	return 0;
}

static int systracker_resume_default(struct platform_device *pdev)
{
	if (track_config.state || track_config.enable_wp)
		systracker_enable();
	return 0;
}

int systracker_resume(struct platform_device *pdev)
{
	if (mt_systracker_drv.systracker_resume)
		return mt_systracker_drv.systracker_resume(pdev);
	else
		return systracker_resume_default(pdev);
}


/*
 * save entry info early
 */
void save_entry(void)
{
	int i = 0;

	track_entry.dbg_con = readl(IOMEM(BUS_DBG_CON));

	for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
		track_entry.ar_track_l[i] = readl(IOMEM(BUS_DBG_AR_TRACK_L(i)));
		track_entry.ar_track_h[i] = readl(IOMEM(BUS_DBG_AR_TRACK_H(i)));
		track_entry.ar_trans_tid[i] = readl(IOMEM(BUS_DBG_AR_TRANS_TID(i)));
		track_entry.aw_track_l[i] = readl(IOMEM(BUS_DBG_AW_TRACK_L(i)));
		track_entry.aw_track_h[i] = readl(IOMEM(BUS_DBG_AW_TRACK_H(i)));
		track_entry.aw_trans_tid[i] = readl(IOMEM(BUS_DBG_AW_TRANS_TID(i)));
	}
}


#ifdef SYSTRACKER_TEST_SUIT
void systracker_test_cleanup(void)
{
	if (mt_systracker_drv.systracker_test_cleanup)
		return mt_systracker_drv.systracker_test_cleanup();
}
#endif


static int systracker_watchpoint_enable_default(void)
{
	track_config.enable_wp = 1;
	writel(track_config.wp_phy_address, IOMEM(BUS_DBG_WP));
	writel(0x0000000F, IOMEM(BUS_DBG_WP_MASK));
	writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_WP_EN, IOMEM(BUS_DBG_CON));
	pr_notice("track_config.wp_phy_address = 0x%x\n", track_config.wp_phy_address);
	mb();

	return 0;
}

int systracker_watchpoint_enable(void)
{
	if (mt_systracker_drv.enable_watchpoint)
		return mt_systracker_drv.enable_watchpoint();
	else
		return systracker_watchpoint_enable_default();
}

static int systracker_watchpoint_disable_default(void)
{
	track_config.enable_wp = 0;
	writel(readl(IOMEM(BUS_DBG_CON)) & ~BUS_DBG_CON_WP_EN, IOMEM(BUS_DBG_CON));
	mb();

	return 0;
}

int systracker_watchpoint_disable(void)
{
	if (mt_systracker_drv.disable_watchpoint)
		return mt_systracker_drv.disable_watchpoint();
	else
		return systracker_watchpoint_disable_default();
}

static void systracker_reset_default(void)
{
	writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_SW_RST, IOMEM(BUS_DBG_CON));
	writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_IRQ_CLR, IOMEM(BUS_DBG_CON));
	mb();
}

void systracker_reset(void)
{
	if (mt_systracker_drv.reset_systracker)
		mt_systracker_drv.reset_systracker();
	else
		systracker_reset_default();
}

static unsigned int systracker_timeout_value_default(void)
{
	/* prescale = (133 * (10 ^ 6)) / 16 = 8312500/s */
	return (unsigned int)(BUS_DBG_BUS_MHZ * 1000 / 16)
			* (unsigned int)track_config.timeout_ms;
}

static unsigned int systracker_timeout_value(void)
{
	if (mt_systracker_drv.systracker_timeout_value)
		return mt_systracker_drv.systracker_timeout_value();
	else
		return systracker_timeout_value_default();
}

static void systracker_enable_default(void)
{
	unsigned int con;
	unsigned int timer_control_value;

	timer_control_value = systracker_timeout_value();
	writel(timer_control_value, IOMEM(BUS_DBG_TIMER_CON));

	track_config.state = 1;
	con = BUS_DBG_CON_BUS_DBG_EN | BUS_DBG_CON_BUS_OT_EN;
	if (track_config.enable_timeout)
		con |= BUS_DBG_CON_TIMEOUT_EN;

	if (track_config.enable_slave_err)
		con |= BUS_DBG_CON_SLV_ERR_EN;

	if (track_config.enable_irq)
		con |= BUS_DBG_CON_IRQ_EN;

	con &= ~BUS_DBG_CON_IRQ_WP_EN;

	con |= BUS_DBG_CON_HALT_ON_EN;
	writel(con, IOMEM(BUS_DBG_CON));
	mb();
	con |= BUS_DBG_CON_IRQ_WP_EN;
	writel(con, IOMEM(BUS_DBG_CON));
	mb();
}

void systracker_enable(void)
{
	if (mt_systracker_drv.enable_systracker)
		mt_systracker_drv.enable_systracker();
	else
		systracker_enable_default();
}

void enable_systracker(void)
{
	systracker_enable();
}

static void systracker_disable_default(void)
{
	track_config.state = 0;
	writel(readl(IOMEM(BUS_DBG_CON)) & ~BUS_DBG_CON_BUS_DBG_EN, IOMEM(BUS_DBG_CON));
	mb();

}

void systracker_disable(void)
{
	if (mt_systracker_drv.disable_systracker)
		mt_systracker_drv.disable_systracker();
	else
		systracker_disable_default();
}

static int systracker_hook_fault(void)
{
	if (mt_systracker_drv.systracker_hook_fault)
		return mt_systracker_drv.systracker_hook_fault();

	pr_warn("mt_systracker_drv.systracker_hook_fault is NULL");
	return -1;
}

int systracker_test_init(void)
{
	if (mt_systracker_drv.systracker_test_init)
		return mt_systracker_drv.systracker_test_init();

	pr_warn("mt_systracker_drv.systracker_test_init is NULL");
	return -1;
}

struct mt_systracker_driver *get_mt_systracker_drv(void)
{
	return &mt_systracker_drv;
}

int tracker_dump(char *buf)
{

	char *ptr = buf;
	unsigned int reg_value;
	int i;
	unsigned int entry_valid;
	unsigned int entry_tid;
	unsigned int entry_id;
	unsigned int entry_address;
	unsigned int entry_data_size;
	unsigned int entry_burst_length;

	{
		/* Get tracker info and save to buf */

		/* BUS_DBG_AR_TRACK_L(__n)
		 * [31:0] ARADDR: DBG read tracker entry read address
		 */

		/* BUS_DBG_AR_TRACK_H(__n)
		 * [14] Valid:DBG read tracker entry valid
		 * [13:7] ARID:DBG read tracker entry read ID
		 * [6:4] ARSIZE:DBG read tracker entry read data size
		 * [3:0] ARLEN: DBG read tracker entry read burst length
		 */

		/* BUS_DBG_AR_TRACK_TID(__n)
		 * [2:0] BUS_DBG_AR_TRANS0_ENTRY_ID: DBG read tracker entry ID of 1st transaction
		 */

#ifdef TRACKER_DEBUG
		pr_debug("Sys Tracker Dump\n");
#endif

		for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
			entry_address = track_entry.ar_track_l[i];
			reg_value = track_entry.ar_track_h[i];
			entry_valid = extract_n2mbits(reg_value, 19, 19);
			entry_id = extract_n2mbits(reg_value, 7, 18);
			entry_data_size = extract_n2mbits(reg_value, 4, 6);
			entry_burst_length = extract_n2mbits(reg_value, 0, 3);
			entry_tid = track_entry.ar_trans_tid[i];

			ptr += sprintf(ptr,
				"read entry = %d, valid = 0x%x, tid = 0x%x, read id = 0x%x, address = 0x%x, data_size = 0x%x, burst_length = 0x%x\n",
				i, entry_valid, entry_tid, entry_id,
				entry_address, entry_data_size, entry_burst_length);

#ifdef TRACKER_DEBUG
			pr_debug("read entry = %d, valid = 0x%x, tid = 0x%x, read id = 0x%x, address = 0x%x, data_size = 0x%x, burst_length = 0x%x\n",
				i, entry_valid, entry_tid, entry_id,
				entry_address, entry_data_size, entry_burst_length);
#endif
		}

		/* BUS_DBG_AW_TRACK_L(__n)
		 * [31:0] AWADDR: DBG write tracker entry write address
		 */

		/* BUS_DBG_AW_TRACK_H(__n)
		 * [14] Valid:DBG   write tracker entry valid
		 * [13:7] ARID:DBG  write tracker entry write ID
		 * [6:4] ARSIZE:DBG write tracker entry write data size
		 * [3:0] ARLEN: DBG write tracker entry write burst length
		 */

		/* BUS_DBG_AW_TRACK_TID(__n)
		 * [2:0] BUS_DBG_AW_TRANS0_ENTRY_ID: DBG write tracker entry ID of 1st transaction
		 */

		for (i = 0; i < BUS_DBG_NUM_TRACKER; i++) {
			entry_address = track_entry.aw_track_l[i];
			reg_value = track_entry.aw_track_h[i];
			entry_valid = extract_n2mbits(reg_value, 19, 19);
			entry_id = extract_n2mbits(reg_value, 7, 18);
			entry_data_size = extract_n2mbits(reg_value, 4, 6);
			entry_burst_length = extract_n2mbits(reg_value, 0, 3);
			entry_tid = track_entry.aw_trans_tid[i];

			ptr += sprintf(ptr,
				"write entry = %d, valid = 0x%x, tid = 0x%x, write id = 0x%x, address = 0x%x, data_size = 0x%x, burst_length = 0x%x\n",
				i, entry_valid, entry_tid, entry_id, entry_address,
				entry_data_size, entry_burst_length);

#ifdef TRACKER_DEBUG
			pr_debug("write entry = %d, valid = 0x%x, tid = 0x%x, write id = 0x%x, address = 0x%x, data_size = 0x%x, burst_length = 0x%x\n",
				i, entry_valid, entry_tid, entry_id, entry_address,
				entry_data_size, entry_burst_length);
#endif
		}

		return strlen(buf);
	}

	return -1;
}

static ssize_t tracker_run_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%x\n", readl(IOMEM(BUS_DBG_CON)));
}

static ssize_t tracker_run_store(struct device_driver *driver, const char *buf, size_t count)
{
	unsigned int value;

	if (unlikely(kstrtou32(buf, 10, &value) != 1))
		return -EINVAL;

	if (value == 1)
		systracker_enable();
	else if (value == 0)
		systracker_disable();
	else
		return -EINVAL;

	return count;
}

DRIVER_ATTR(tracker_run, 0644, tracker_run_show, tracker_run_store);

static ssize_t enable_wp_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%x\n", track_config.enable_wp);
}

static ssize_t enable_wp_store(struct device_driver *driver, const char *buf, size_t count)
{
	unsigned int value;

	if (kstrtou32(buf, 10, &value))
		return -EINVAL;

	if (value == 1)
		systracker_watchpoint_enable();
	else if (value == 0)
		systracker_watchpoint_disable();
	else
		return -EINVAL;

	return count;
}

DRIVER_ATTR(enable_wp, 0644, enable_wp_show, enable_wp_store);

static ssize_t set_wp_address_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%x\n", track_config.wp_phy_address);
}

int systracker_set_watchpoint_addr(unsigned int addr)
{
	if (mt_systracker_drv.set_watchpoint_address)
		return mt_systracker_drv.set_watchpoint_address(addr);

	track_config.wp_phy_address = addr;

	return 0;
}

static ssize_t set_wp_address_store(struct device_driver *driver, const char *buf, size_t count)
{
	unsigned int value;

	if (unlikely(kstrtou32(buf, 10, &value) != 1))
		return -EINVAL;

	pr_notice("watch address:0x%x\n", value);
	systracker_set_watchpoint_addr(value);

	return count;
}

DRIVER_ATTR(set_wp_address, 0644, set_wp_address_show, set_wp_address_store);

static ssize_t tracker_entry_dump_show(struct device_driver *driver, char *buf)
{
	int ret = tracker_dump(buf);

	if (ret == -1)
		pr_crit("Dump error in %s, %d\n", __func__, __LINE__);

	return strlen(buf);
}


static ssize_t tracker_swtrst_show(struct device_driver *driver, char *buf)
{
	return 0;
}

static ssize_t tracker_swtrst_store(struct device_driver *driver, const char *buf, size_t count)
{
	writel(readl(IOMEM(BUS_DBG_CON)) | BUS_DBG_CON_SW_RST, IOMEM(BUS_DBG_CON));
	return count;
}

DRIVER_ATTR(tracker_swtrst, 0664, tracker_swtrst_show, tracker_swtrst_store);
#ifdef SYSTRACKER_TEST_SUIT
void systracker_wp_test(void)
{
	if (mt_systracker_drv.systracker_wp_test)
		mt_systracker_drv.systracker_wp_test();
	else
		pr_notice("mt_systracker_drv.systracker_wp_test is NULL\n");
}

/* this function expects */
void systracker_read_timeout_test(void)
{
	pr_notice("we are going to have read timeout\n");

	if (mt_systracker_drv.systracker_read_timeout_test)
		mt_systracker_drv.systracker_read_timeout_test();
	else
		pr_notice("mt_systracker_drv.systracker_read_timeout_test is NULL\n");
}

void systracker_write_timeout_test(void)
{
	pr_notice("we are going to have write timeout\n");

	if (mt_systracker_drv.systracker_write_timeout_test)
		mt_systracker_drv.systracker_write_timeout_test();
	else
		pr_notice("mt_systracker_drv.systracker_write_timeout_test is NULL\n");
}

void systracker_timeout_withrecord_test(void)
{
	pr_notice("we are going to have read timeout, and then wdt happens\n");
	pr_notice("Please check if there is related backtrace info in aee\n");

	if (mt_systracker_drv.systracker_withrecord_test)
		mt_systracker_drv.systracker_withrecord_test();
	else
		pr_notice("mt_systracker_drv.systracker_withrecord_test is NULL\n");
}

void systracker_notimeout_test(void)
{
	pr_notice("should hang forever from now on, never come back...\n");
	pr_notice("ICE should not connect anymore\n");

	if (mt_systracker_drv.systracker_notimeout_test)
		mt_systracker_drv.systracker_notimeout_test();
	else
		pr_notice("mt_systracker_drv.systracker_notimeout_test is NULL\n");
}

static ssize_t test_suit_show(struct device_driver *driver, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "==Systracker test==\n"
			"1.Systracker show dump test\n"
			"2.Systracker watchpoint test\n"
			"3.Systracker read timeout test\n"
			"4.Systracker write timeout test\n"
			"5.Systracker timeout with record test\n"
			"6.Systracker no timeout test\n");
}

static ssize_t test_suit_store(struct device_driver *driver, const char *buf, size_t count)
{
	char *p = (char *)buf;
	unsigned long int num;
	ssize_t ret;

	ret = kstrtoul(p, 10, &num);
	if (ret == 0) {
		switch (num) {
			/* Test Systracker Function */
		case 1:
			return tracker_entry_dump_show(driver, p);
		case 2:
			systracker_wp_test();
			break;
		case 3:
			systracker_read_timeout_test();
			break;
		case 4:
			systracker_write_timeout_test();
			break;
		case 5:
			systracker_timeout_withrecord_test();
			break;
		case 6:
			systracker_notimeout_test();
			break;
		default:
			break;
		}
	}

	return count;
}

DRIVER_ATTR(test_suit, 0664, test_suit_show, test_suit_store);
#endif


static ssize_t tracker_entry_dump_store(struct device_driver *driver, const char *buf, size_t count)
{
	return count;
}

DRIVER_ATTR(tracker_entry_dump, 0664, tracker_entry_dump_show, tracker_entry_dump_store);

static ssize_t tracker_last_status_show(struct device_driver *driver, char *buf)
{

	if (track_entry.dbg_con & (BUS_DBG_CON_IRQ_AR_STA | BUS_DBG_CON_IRQ_AW_STA))
		return snprintf(buf, PAGE_SIZE, "1\n");
	else
		return snprintf(buf, PAGE_SIZE, "0\n");
}

static ssize_t tracker_last_status_store(struct device_driver *driver, const char *buf,
					 size_t count)
{
	return count;
}

DRIVER_ATTR(tracker_last_status, 0664, tracker_last_status_show, tracker_last_status_store);


static int systracker_panic_callback(struct notifier_block *nb,
			       unsigned long reason, void *arg)
{
	unsigned int con = 0;

	save_entry();

	con = readl(BUS_DBG_CON);
	writel(con | BUS_DBG_CON_IRQ_CLR, BUS_DBG_CON);
	mb(); /* memory barriar */

	if ((con & BUS_DBG_CON_IRQ_AR_STA) != 0U) {
		pr_crit("[%s] Read timeout trigger\n", __func__);
		tracker_print_ar();
	}

	if ((con & BUS_DBG_CON_IRQ_AW_STA) != 0U) {
		pr_crit("[%s] Write timeout trigger\n", __func__);
		tracker_print_aw();
	}

	return 0;
}

static struct notifier_block systracker_panic_notifier = {
	.notifier_call = systracker_panic_callback,
};


/*
 * driver initialization entry point
 */
static int __init systracker_init(void)
{
	int err;
	int ret = 0;

#ifdef SYSTRACKER_TEST_SUIT
	systracker_test_init();
#endif

	printk("systracker_init enter! \n");

	err = platform_driver_register(&mt_systracker_drv.driver);
	if (err != 0) {
		pr_err("systracker_init register fail %d! \n", err);
		return err;
	}
	
	/* Create sysfs entry */

	ret = driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_tracker_entry_dump);
	ret |= driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_tracker_run);
	ret |= driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_enable_wp);
	ret |= driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_set_wp_address);
	ret |= driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_tracker_swtrst);
	ret |=
	    driver_create_file(&mt_systracker_drv.driver.driver, &driver_attr_tracker_last_status);
		if (ret != 0)
		pr_err("Fail at driver_attr_tracker_run!\n");

#ifdef SYSTRACKER_TEST_SUIT
	ret = driver_create_file(&mt_systracker_drv.driver.driver,
				&driver_attr_test_suit);
	if (ret != 0)
		pr_err("Fail at driver_attr_test_suit!\n");
#endif

	ret = systracker_hook_fault();
	if (ret != 0)
		pr_err("mt_systracker_drv.systracker_hook_fault is NULL\n");

	ret = atomic_notifier_chain_register(&panic_notifier_list,
				       &systracker_panic_notifier);
	if (ret != 0)
		pr_err("Fail to register notifier chain!\n");

	pr_info("systracker init done!\n");

	return 0;
}

/*
 * driver exit point
 */
static void __exit systracker_exit(void)
{
	int ret = 0;

	ret = atomic_notifier_chain_unregister(&panic_notifier_list,
					 &systracker_panic_notifier);
	if (ret != 0)
		pr_err("Fail to unregister notifier chain!\n");
}

module_init(systracker_init);
module_exit(systracker_exit);

MODULE_DESCRIPTION("system tracker driver v1.0");
MODULE_LICENSE("GPL v2");

