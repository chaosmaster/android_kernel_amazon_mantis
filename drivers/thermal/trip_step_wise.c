/*
 *  trip_step_wise.c - A simple thermal throttling governor
 *
 *  Copyright (C) 2015 Amazon.com, Inc. or its affiliates. All Rights Reserved
 *  Author: Akwasi Boateng <boatenga@amazon.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 *
 */

#include <linux/thermal.h>
#include <linux/module.h>
#include "thermal_core.h"

/**
 * trip_step_wise_throttle
 * @tz - thermal_zone_device
 * @trip - the trip point
 *
 */
static int trip_step_wise_throttle(struct thermal_zone_device *tz, int trip)
{
	int trip_temp, trip_hyst = 0;
	struct thermal_instance *tz_instance;
	//struct thermal_instance *cd_instance;
	struct thermal_cooling_device *cdev;
	signed long target = 0;
	char data[4][32];
	char *envp[] = { data[0], data[1], data[2], data[3], NULL };
	signed long cur_state, new_state, max_state;
	signed int thermal_state; /* state0 TRIP0 state 1 TRIP1 state2 TRIP2 ... */
	//int cdev_level;

	mutex_lock(&tz->lock);
	list_for_each_entry(tz_instance, &tz->thermal_instances, tz_node) {
		if (tz_instance->trip != trip)
			continue;

		cdev = tz_instance->cdev;
		if (!cdev)
			continue;

		mutex_lock(&cdev->lock);

		#if 0
		/* remove this, it cause problem when there are multiple governors.
		   In thermal_cdev_update(), the target value is used to pick the biggest one.
		   If you reset it here, then IPA and this will set state back and forth endlessly
		 */
		list_for_each_entry(cd_instance, &cdev->thermal_instances, cdev_node) {
			cd_instance->target = THERMAL_NO_TARGET;
		}
		#endif

		if (trip == THERMAL_TRIPS_NONE)
			trip_temp = tz->forced_passive;
		else {
			tz->ops->get_trip_temp(tz, trip, &trip_temp);
			tz->ops->get_trip_hyst(tz, trip, &trip_hyst);
		}

		cdev->ops->get_cur_state(cdev, &cur_state);

		if (tz->temperature >= trip_temp) {
			if (tz_instance->upper > cur_state)
				target = tz_instance->upper;
			else
				target = cur_state;
			thermal_state = trip + 1; /* rising: state1 maps to trip0, etc. */
		} else if (tz->temperature < trip_temp - trip_hyst){
			if (cur_state > tz_instance->lower)
				target = tz_instance->lower;
			else
				target = THERMAL_NO_TARGET; //cur_state;
			thermal_state = trip; /* falling: need next trip to take effect, e.g. state1 handled in trip1 */
		}
		else {
			target = cur_state;
		}
		cdev->ops->get_max_state(cdev, &max_state);
		target = (target > max_state) ? max_state : target;

		pr_debug("thermal %s zone%d trip %d cdev %s trip_temp %d tz temp %d cur_state %ld target %ld instance-target %ld instance name %s\n",
				__func__, tz->id, trip, cdev->type, trip_temp, tz->temperature, cur_state, target, tz_instance->target, tz_instance->name);

		if (cur_state != target) {
			tz_instance->target = target;
			cdev->updated = false;
		}
		mutex_unlock(&cdev->lock);

		if (cur_state != target) {
			thermal_cdev_update(cdev);
			cdev->ops->get_cur_state(cdev, &new_state);

			/*
			pdata = cdev->devdata;
			cdev_level = -1;
			if (pdata->levels) {
				cdev_level = pdata->levels[new_state];
			}
			*/

			/* Only send  uevent when the state changes */
			if (new_state == target) {
				pr_info("thermal zone: %s, temp: %d, state change, cur %ld target %ld, id=%d cdev=%s trip=%d thermalstate=%d\n",
					 tz->type, tz->temperature, cur_state, target, tz->id, cdev->type, trip, thermal_state);

				snprintf(data[0], sizeof(data[0]), "THERMAL_STATE=%d", thermal_state);
				snprintf(data[1], sizeof(data[1]), "ID=%d", tz->id);
				snprintf(data[2], sizeof(data[2]), "CDTYPE=%s", cdev->type);
				snprintf(data[3], sizeof(data[3]), "CDLEV=%ld", new_state);
				kobject_uevent_env(&tz->device.kobj, KOBJ_CHANGE, envp);
			}
		}
	}
	mutex_unlock(&tz->lock);
	return 0;
}

static struct thermal_governor thermal_gov_trip_step_wise = {
	.name = "trip_step_wise",
	.throttle = trip_step_wise_throttle,
};

static int __init thermal_gov_trip_step_wise_init(void)
{
	return thermal_register_governor(&thermal_gov_trip_step_wise);
}

static void __exit thermal_gov_trip_step_wise_exit(void)
{
	thermal_unregister_governor(&thermal_gov_trip_step_wise);
}

fs_initcall(thermal_gov_trip_step_wise_init);
module_exit(thermal_gov_trip_step_wise_exit);

MODULE_AUTHOR("Akwasi Boateng");
MODULE_DESCRIPTION("A simple trip level throttling thermal governor");
MODULE_LICENSE("GPL");
