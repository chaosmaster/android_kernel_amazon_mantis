/*
 * Thermal Framework Driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 * Author: Dan Murphy <DMurphy@ti.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#ifndef __LINUX_THERMAL_FRAMEWORK_H__
#define __LINUX_THERMAL_FRAMEWORK_H__

#define NUM_COOLERS 10
#include <linux/seq_file.h>

struct thermal_dev;
struct cooling_device;

/**
 * struct thermal_dev_params  - Structure for each thermal sensor devicev params.
 * @alpha:  Moving average coefficient
 * @offset: Temperature offset
 * @weight: Weight
 */
struct thermal_dev_params {
	int offset;
	int alpha;
	int weight;
};

/**
 * struct thermal_dev_ops  - Structure for device operation call backs
 * @get_temp: A temp sensor call back to get the current temperature.
 *		temp is reported in milli degrees.
 * @int: channel number
 * @int *: temp value returns
 *
 * return 0 if success
 */
struct thermal_dev_ops {
	int (*get_temp) (struct device *dev, int ch_n, int *temp);
};

/**
 * struct thermal_dev  - Structure for each thermal device.
 * @name: The name of the device that is registering to the framework
 * @dev: Device node
 * @dev_ops: The device specific operations for the sensor, governor and cooling
 *           agents.
 * @node: The list node of the
 * @current_temp: The current temperature reported for the specific domain
 * @vs: The virtual sensor to which thermal sensor links to.
 *
 */
struct thermal_dev {
	const char *name;
	struct device *dev;
	struct thermal_dev_ops *dev_ops;
	struct list_head node;
	struct thermal_dev_params tdp;
	//int current_temp;
	int off_temp;
	int last_tempv;
	int data;
	//int vs;
};
/**
 * API to register temperature sensors with a virtual sensor device
 */
int virtual_sensor_dev_register(struct device *dev, struct thermal_dev_ops *dev_ops, int data);

#endif /* __LINUX_THERMAL_FRAMEWORK_H__ */
