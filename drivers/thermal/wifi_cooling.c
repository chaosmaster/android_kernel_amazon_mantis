/*
 *  linux/drivers/thermal/wifi_cooling.c
 *
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <thermal_core.h>

/* mt8695 has wifi state 0,1,2,3(HT20 is not used) */
#define MAX_WIFI_STATE 4
unsigned int wifi_state;
struct thermal_cooling_device *wifi_cool_dev;

/**
 * wifi_get_max_state - callback function to get the max cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the max cooling state.
 *
 * Callback for the thermal cooling device to return the
 * max cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int wifi_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = MAX_WIFI_STATE;
	pr_debug("%s state=%ld\n", __func__, *state);
	return 0;
}

/**
 * wifi_get_cur_state - callback function to get the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the current cooling state.
 *
 * Callback for the thermal cooling device to return the
 * current cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int wifi_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	*state = wifi_state;
	pr_debug("%s state=%ld\n", __func__, *state);
	return 0;
}

/**
 * wifi_set_cur_state - callback function to set the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: set this variable to the current cooling state.
 *
 * Callback for the thermal cooling device to change the
 * current cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int wifi_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	if (MAX_WIFI_STATE - state > 0) {
		if (wifi_state != state)
			pr_info("%s set state=%ld\n", __func__, state);
		wifi_state = state;
//		wifi_set_state(state);
	}

	return 0;
}

/*
 * Simple mathematics model for power:
 * just for ipa hook, nothing to do;
 */
static int wifi_state2power(struct thermal_cooling_device *cdev,
			       struct thermal_zone_device *tz,
			       unsigned long state, u32 *power)
{
	*power = 0;

	return 0;
}

static int wifi_power2state(struct thermal_cooling_device *cdev,
			       struct thermal_zone_device *tz, u32 power,
			       unsigned long *state)
{
	cdev->ops->get_cur_state(cdev, state);
	return 0;
}

/* Bind callbacks to thermal cooling device ops */
static struct thermal_cooling_device_ops const wifi_cooling_ops = {
	.get_max_state = wifi_get_max_state,
	.get_cur_state = wifi_get_cur_state,
	.set_cur_state = wifi_set_cur_state,
	.state2power   = wifi_state2power,
	.power2state   = wifi_power2state,
};

/**
 * wifi_cooling_register - function to create cooling device.
 *
 * This interface function registers the wifi cooling device with the name
 * "wifi".
 *
 * Return: a valid struct thermal_cooling_device pointer on success,
 */
struct thermal_cooling_device *
wifi_cooling_register(struct device_node *np, int id)
{
	wifi_cool_dev = thermal_of_cooling_device_register(np, "wifi", NULL,
						      &wifi_cooling_ops);
	if (!wifi_cool_dev) {
		return ERR_PTR(-EINVAL);
	}

	return wifi_cool_dev;
}
EXPORT_SYMBOL(wifi_cooling_register);

/**
 * wifi_cooling_unregister - function to remove wifi cooling device.
 * @cdev: thermal cooling device pointer.
 *
 */
void wifi_cooling_unregister(struct thermal_cooling_device *cdev)
{
	if (!cdev)
		return;

	thermal_cooling_device_unregister(wifi_cool_dev);
}
EXPORT_SYMBOL(wifi_cooling_unregister);
