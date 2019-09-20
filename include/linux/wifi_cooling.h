/*
 *  linux/include/linux/wifi_cooling.h
 */

#ifndef __wifi_COOLING_H__
#define __wifi_COOLING_H__

#ifdef CONFIG_WIFI_THERMAL

struct thermal_cooling_device *wifi_cooling_register(struct device_node *np, int id);


#endif	/* CONFIG_WIFI_THERMAL */
#endif /* __wifi_COOLING_H__ */
