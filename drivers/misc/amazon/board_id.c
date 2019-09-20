/*
 * board_id.c
 *
 * Copyright 2018 Amazon Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/gpio/consumer.h>
#include "board_id.h"
#define BOARD_ID_PROC_NAME	"board_id"

static struct proc_dir_entry *board_id_file;

struct board_id {
	int br_id;
	struct gpio_desc *gpio_b0;
	struct gpio_desc *gpio_b1;
	struct gpio_desc *gpio_b2;
};


static struct board_id *gboard_id;

static int board_id_show(struct seq_file *m, void *v)
{
	if (gboard_id) {
		seq_printf(m, "board_id:%d\n", gboard_id->br_id);
	} else
		seq_printf(m, "board_id driver is not initialized");
	return 0;
}

static int board_id_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, board_id_show, NULL);
}

static const struct file_operations board_id_proc_fops = {
	.open = board_id_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int get_board_id(void)
{
	if (!gboard_id)
		return -1;

	pr_info("%s: board_id:%d\n", __func__, gboard_id->br_id);
	return gboard_id->br_id;
}
EXPORT_SYMBOL(get_board_id);

static void board_id_proc_done(void)
{
	remove_proc_entry(BOARD_ID_PROC_NAME, NULL);
}

static int amazon_board_id_probe(struct platform_device *pdev)
{
	int ret = 0;

	gboard_id = devm_kzalloc(&pdev->dev, sizeof(*gboard_id), GFP_KERNEL);
	if (!gboard_id)
		return -ENOMEM;

	/* Look up the GPIO pins to use */
	gboard_id->gpio_b0 = devm_gpiod_get(&pdev->dev, "id0", GPIOD_IN);
	if (IS_ERR(gboard_id->gpio_b0))
		return PTR_ERR(gboard_id->gpio_b0);

	gboard_id->gpio_b1 = devm_gpiod_get(&pdev->dev, "id1", GPIOD_IN);
	if (IS_ERR(gboard_id->gpio_b1))
		return PTR_ERR(gboard_id->gpio_b1);

	gboard_id->gpio_b2 = devm_gpiod_get(&pdev->dev, "id2", GPIOD_IN);
	if (IS_ERR(gboard_id->gpio_b2))
		return PTR_ERR(gboard_id->gpio_b2);


	gboard_id->br_id =  (gpiod_get_value(gboard_id->gpio_b0) |
		gpiod_get_value(gboard_id->gpio_b1) << BOARD_ID_BIT_1 |
		gpiod_get_value(gboard_id->gpio_b2) << BOARD_ID_BIT_2 );

	pr_info("%s: board id:%d\n", __func__, gboard_id->br_id);

	board_id_file = proc_create(BOARD_ID_PROC_NAME,  0444, NULL, &board_id_proc_fops);
	if (!board_id_file) {
		pr_err("can't create board id proc entry\n");
		ret = -ENOMEM;
		goto fail;
	}

	return ret;

fail:
	kfree(gboard_id);
	return ret;
}


static const struct of_device_id of_board_id_match[] = {
	{ .compatible = "amazon,board-id", },
	{ },
};

static struct platform_driver board_id_driver = {
	.probe = amazon_board_id_probe,
	.driver = {
		.name = "board-id",
		.of_match_table = of_board_id_match,
	},
};

static void __exit board_id_exit(void)
{
	board_id_proc_done();
	kfree(gboard_id);
	platform_driver_unregister(&board_id_driver);
}

module_platform_driver(board_id_driver);
module_exit(board_id_exit);

MODULE_LICENSE("GPL v2");
