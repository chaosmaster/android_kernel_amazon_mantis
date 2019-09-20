/*
 * Copyright (C) 2016 Lab126, Inc.  All rights reserved.
 * Author: Akwasi Boateng <boatenga@lab126.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include "thermal_core.h"

//#include <linux/virtual_sensor_thermal.h>
#include <linux/thermal_framework.h>
//#include <linux/amlogic/aml_thermal_cooling.h>
//#include <linux/amlogic/aml_thermal_hw.h>
#include <linux/cpufreq.h>


#define BUF_SIZE 128
#define DMF 1000
#define MASK (0x001F)


static LIST_HEAD(g_virtual_sensor_dev_list);
static DEFINE_MUTEX(virtual_sensor_lock);

#ifdef CONFIG_PLATFORM_abc123_FEATURE
#include "../misc/amz_priv.h"
extern int idme_get_board_long_rev(void);
#define TMPA "tmp103a"
#define TMPC "tmp103c"
#endif

/**
 * struct virtual_thermal_zone  - Structure for each virtual thermal zone.
 * @virtual_sensor_dev_list: duplicated list of g_virtual_sensor_dev_list, so that each virtual zone has its own parameters
 *
 */
struct virtual_thermal_zone {
	struct list_head            virtual_sensor_dev_list;
	enum thermal_device_mode    virtual_sensor_mode;
	//struct aml_cool_dev        *virtual_sensor_cool_devs;
	struct thermal_zone_device *virtual_sensor_tzd;
};


static int virtual_sensor_get_temp(void *data, int *t)
{
	struct thermal_dev *tdev;
	int ret;
	int temp = 0;
	int tempv = 0;
	int alpha, offset, weight;
	struct virtual_thermal_zone *vtz = data;

	list_for_each_entry(tdev, &vtz->virtual_sensor_dev_list, node) {
		ret = tdev->dev_ops->get_temp(tdev->dev, tdev->data, &temp);
		if (ret != 0){
			pr_info("%s: virtual thermal zone:%s device:%s get temp return: %d, use last temp reading: %d.\n",
					__func__, vtz->virtual_sensor_tzd->type, dev_name(tdev->dev), ret, tdev->last_tempv);
			temp = tdev->last_tempv;
		}

		tdev->last_tempv = temp;

		alpha = tdev->tdp.alpha;
		offset = tdev->tdp.offset;
		weight = tdev->tdp.weight;

		pr_debug("%s %s t=%d a=%d o=%d w=%d\n",
				__func__, tdev->name, temp, alpha, offset, weight);

		if (!tdev->off_temp)
			tdev->off_temp = temp - offset;
		else {
			tdev->off_temp = alpha * (temp - offset) +
				(DMF - alpha) * tdev->off_temp;
			tdev->off_temp /= DMF;
		}
		tempv += (weight * tdev->off_temp)/DMF;

		pr_debug("%s tempv=%d\n", __func__, tempv);
	}

	*t = tempv;
	return 0;
}
/*
static int virtual_sensor_set_mode(struct thermal_zone_device *thermal,
					   enum thermal_device_mode mode)
{
	virtual_sensor_mode = mode;
	if (mode == THERMAL_DEVICE_DISABLED) {
	}
	return 0;
}
*/

/* Add this functio so that thermal-hal will not throw error while parsing
   thermal.policy.conf and disabled thermal-zone. Don't want user to dynamially
   configure the trip point number. nstrip is the number of trip points */
static int virtual_sensor_set_trips(void *data, int low, int high ) {
	pr_debug("%s: dummy function, ntrip won't change \n", __func__);
	return 0;
}

static struct thermal_zone_of_device_ops virtual_sensor_ops = {
	.get_temp = virtual_sensor_get_temp,
	.set_trips = virtual_sensor_set_trips,
//	.set_mode = virtual_sensor_set_mode,
};

/* can get from:
   cat sys/bus/platform/devices/virtual-sensor/params
*/
static ssize_t virtual_sensor_show_params(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	int n = 0;
	int o = 0;
	int a = 0;
	int w = 0;
	char pbufn[BUF_SIZE]; // name
	char pbufo[BUF_SIZE]; // offset
	char pbufa[BUF_SIZE]; // alpha
	char pbufw[BUF_SIZE]; // weight
	int alpha, offset, weight;
	struct thermal_dev *tdev;
	struct virtual_thermal_zone *vtz = dev_get_drvdata(dev);

	n += sprintf(pbufn + n, "device ");
	o += sprintf(pbufo + o, "offset ");
	a += sprintf(pbufa + a, "alpha ");
	w += sprintf(pbufw + w, "weight ");

	list_for_each_entry(tdev, &vtz->virtual_sensor_dev_list, node) {
		alpha = tdev->tdp.alpha;
		offset = tdev->tdp.offset;
		weight = tdev->tdp.weight;

		n += sprintf(pbufn + n, "\t%s", dev_name(tdev->dev));
		o += sprintf(pbufo + o, "\t%d", offset);
		a += sprintf(pbufa + a, "\t%d", alpha);
		w += sprintf(pbufw + w, "\t%d", weight);
	}
	return sprintf(buf, "%s\n%s\n%s\n%s\n", pbufn, pbufo, pbufa, pbufw);
}

/* Usage:
   cd sys/bus/platform/devices/virtual-sensor
   echo "thermistor0 offset 99" > params
   cat params
*/
static ssize_t virtual_sensor_store_params(struct device *dev,
				      struct device_attribute *devattr,
				      const char *buf,
				      size_t count)
{
	struct thermal_dev *tdev;
	char devname[BUF_SIZE];
	char param[BUF_SIZE];
	int value = 0;
	struct virtual_thermal_zone *vtz = dev_get_drvdata(dev);

	if (sscanf(buf, "%s %s %d", devname, param, &value) == 3) {
		list_for_each_entry(tdev, &vtz->virtual_sensor_dev_list, node) {
			if (!strncmp(devname, dev_name(tdev->dev), strlen(devname))) {
				if (!strcmp(param, "offset"))
					tdev->tdp.offset = value;
				if (!strcmp(param, "alpha"))
					tdev->tdp.alpha = value;
				if (!strcmp(param, "weight"))
					tdev->tdp.weight = value;
				return count;
			}
		}
	}
	else
		pr_err("%s: Error: need device_name, parameter, value\n", __func__);

	return -EINVAL;
}

static DEVICE_ATTR(params, S_IRUGO | S_IWUSR, virtual_sensor_show_params, virtual_sensor_store_params);

static int virtual_sensor_parse_components(struct device_node *np, struct virtual_thermal_zone *vtz)
{
	struct thermal_dev *tdev;
	struct device_node *child = NULL;
	const char *str; // points to existing string
	int alpha, offset, weight;
	int ret = 0;

#ifdef CONFIG_PLATFORM_abc123_FEATURE
	int board_id = 0;
	board_id = idme_get_board_long_rev();
#endif
	while ((child = of_get_next_child(np, child)) != NULL) {
		if (of_property_read_string(child, "vs_name", &str)) {
			pr_err("thermal: read vs_name failed\n");
			continue;
		}

		ret |= of_property_read_s32(child, "offset", &offset);
		ret |= of_property_read_u32(child, "alpha",  &alpha);
		ret |= of_property_read_u32(child, "weight", &weight);
		if (ret) {
			pr_err("%s: Error: need offset, alpha, weight\n", __func__);
			continue;
		}
		pr_info("%s virtual sensor %s params: offset %d alpha %d weight %d\n", __func__, str, offset, alpha, weight);

		list_for_each_entry(tdev, &vtz->virtual_sensor_dev_list, node) {
#ifdef CONFIG_PLATFORM_abc123_FEATURE
			//to support abc123 hvt device with tmp
			if(ishvt(board_id) && (!strncmp(str, TMPA, strlen(str)) || !strncmp(str, TMPC, strlen(str)))){
				tdev->tdp.offset = 8000;
				tdev->tdp.alpha  = 10;
				tdev->tdp.weight = 500;
				break;
			}
			if (!ishvt(board_id) && !strncmp(str, dev_name(tdev->dev), strlen(str))) {
				tdev->tdp.offset = offset;
				tdev->tdp.alpha  = alpha;
				tdev->tdp.weight = weight;
				break;
			}
#else
			if (!strncmp(str, dev_name(tdev->dev), strlen(str))) {
				tdev->tdp.offset = offset;
				tdev->tdp.alpha  = alpha;
				tdev->tdp.weight = weight;
				break;
			}
#endif
		}
	}
	return ret;
}

static int virtual_sensor_thermal_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np, *child;
	struct virtual_thermal_zone *vtz;
	struct thermal_dev *tdev, *tdev_new;
        if (!cpufreq_frequency_get_table(0)) {
                dev_info(&pdev->dev,
                        "Frequency table not initialized. Deferring probe...\n");
                return -EPROBE_DEFER;
        }
	np = pdev->dev.of_node;
	if (!np) {
		pr_err("%s: Error No of_node\n", __func__);
		return -EINVAL;
	}

	vtz = kzalloc(sizeof(struct virtual_thermal_zone), GFP_KERNEL);
	if (!vtz) {
		pr_err("%s: No memory\n", __func__);
		return -ENOMEM;
	}

	/* duplicate the virtual thermal devices list into this thermal zone */
	INIT_LIST_HEAD(&vtz->virtual_sensor_dev_list);
	list_for_each_entry(tdev, &g_virtual_sensor_dev_list, node) {
		tdev_new = kzalloc(sizeof(struct thermal_dev), GFP_KERNEL);
		if (!tdev_new) {
			pr_err("%s: No memory\n", __func__);
			return -ENOMEM;
		}
		memcpy(tdev_new, tdev, sizeof(*tdev));
		list_add_tail(&tdev_new->node, &vtz->virtual_sensor_dev_list);
	}


	/* virtual sensor components */
	child = of_get_child_by_name(np, "vs_components");
	if (child == NULL) {
		pr_err("thermal: can't find vs_components\n");
		return -EINVAL;
	}
	if (virtual_sensor_parse_components(child, vtz))
		return -EINVAL;

	/* cooling devices */
	child = of_get_child_by_name(np, "cooling_devices");
	if (child == NULL) {
		pr_err("thermal: can't find cooling_devices\n");
	}
	/*else {
		vtz->virtual_sensor_cool_devs = kzalloc(sizeof(struct aml_cool_dev) * of_get_child_count(child), GFP_KERNEL);
		if (aml_parse_register_cool_device(child, vtz->virtual_sensor_cool_devs)) {
			pr_err("thermal: parse cooling devices failed\n");
			return -EINVAL;
		}
	}*/

	vtz->virtual_sensor_tzd = thermal_zone_of_sensor_register(&pdev->dev,
							  0,
							  vtz, /* private data */
							  &virtual_sensor_ops);

	if (IS_ERR(vtz->virtual_sensor_tzd)) {
		pr_err("%s Failed to register sensor\n", __func__);
		return -EINVAL;
	}
	thermal_zone_device_update(vtz->virtual_sensor_tzd);

	ret = device_create_file(&pdev->dev, &dev_attr_params);
	if (ret)
		pr_err("%s Failed to create params attr\n", __func__);

	dev_set_drvdata(&pdev->dev, vtz);

	return ret;
}
static int virtual_sensor_thermal_remove(struct platform_device *pdev)
{
	//struct virtual_thermal_zone *vtz = dev_get_drvdata(&pdev->dev);
//	if (vtz && vtz->virtual_sensor_cool_devs)
//		kfree(vtz->virtual_sensor_cool_devs);
	return 0;
}

int virtual_sensor_dev_register(struct device *dev, struct thermal_dev_ops *dev_ops, int data)
{
	struct thermal_dev *vthermal_dev;

	if (unlikely(IS_ERR_OR_NULL(dev))) {
		pr_err("%s: NULL device\n", __func__);
		return -ENODEV;
	}
	if (!dev_ops->get_temp) {
		pr_err("%s: Error getting get_temp()\n", __func__);
		return -EINVAL;
	}

	vthermal_dev = kzalloc(sizeof(struct thermal_dev), GFP_KERNEL);
	if (!vthermal_dev) {
		pr_err("%s: No memory\n", __func__);
		return -ENOMEM;
	}

	mutex_lock(&virtual_sensor_lock);
	vthermal_dev->dev = dev;
	vthermal_dev->dev_ops = dev_ops;
	vthermal_dev->data = data;
	list_add_tail(&vthermal_dev->node, &g_virtual_sensor_dev_list);
	mutex_unlock(&virtual_sensor_lock);

	pr_info("%s: added virtual sensor %s\n", __func__, dev_name(dev));
	return 0;
}
EXPORT_SYMBOL(virtual_sensor_dev_register);

static struct of_device_id virtual_sensor_driver_of_match[] = {
	{ .compatible = "amazon,virtual_sensor" },
	{},
};

static struct platform_driver virtual_sensor_driver = {
	.probe = virtual_sensor_thermal_probe,
	.remove = virtual_sensor_thermal_remove,
	.driver     = {
		.name  = "virtual_sensor",
		.owner = THIS_MODULE,
		.of_match_table = virtual_sensor_driver_of_match,
	},
};

static int __init virtual_sensor_thermal_init(void)
{
	return platform_driver_register(&virtual_sensor_driver);
}
static void __exit virtual_sensor_thermal_exit(void)
{
	platform_driver_unregister(&virtual_sensor_driver);
}

late_initcall(virtual_sensor_thermal_init);
module_exit(virtual_sensor_thermal_exit);

MODULE_DESCRIPTION("VIRTUAL_SENSOR pcb virtual sensor thermal zone driver");
MODULE_AUTHOR("Akwasi Boateng <boatenga@amazon.com>");
MODULE_LICENSE("GPL");
