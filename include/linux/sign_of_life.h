/*
 * sign_of_life.h
 *
 * Device sign of life header file
 *
 * Copyright (C) 2015-2018 Amazon Technologies Inc. All rights reserved.
 * Yang Liu (yangliu@lab126.com)
 * Jiangli Yuan (jly@amazon.com)
 * TODO: Add additional contributor's names.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SIGN_OF_LIFE_H
#define __SIGN_OF_LIFE_H

typedef enum {
	/* Device Boot Reason */
	LIFE_CYCLE_NOT_AVAILABLE     = -1,
	WARMBOOT_BY_KERNEL_PANIC     = 0x100,
	WARMBOOT_BY_KERNEL_WATCHDOG  = 0x101,
	WARMBOOT_BY_HW_WATCHDOG      = 0x102,
	WARMBOOT_BY_SW               = 0x103,
	COLDBOOT_BY_USB              = 0x104,
	COLDBOOT_BY_POWER_KEY        = 0x105,
	COLDBOOT_BY_POWER_SUPPLY     = 0x106,
	/* Device Shutdown Reason */
	SHUTDOWN_BY_LONG_PWR_KEY_PRESS = 0x201,
	SHUTDOWN_BY_SW                 = 0x202,
	SHUTDOWN_BY_PWR_KEY            = 0x203,
	SHUTDOWN_BY_SUDDEN_POWER_LOSS  = 0x204,
	SHUTDOWN_BY_UNKNOWN_REASONS    = 0x205,
	THERMAL_SHUTDOWN_REASON_BATTERY = 0x301,
	THERMAL_SHUTDOWN_REASON_PMIC    = 0x302,
	THERMAL_SHUTDOWN_REASON_SOC     = 0x303,
	THERMAL_SHUTDOWN_REASON_MODEM   = 0x304,
	THERMAL_SHUTDOWN_REASON_WIFI    = 0x305,
	THERMAL_SHUTDOWN_REASON_PCB     = 0x306,
	/* LIFE CYCLE Special Mode */
	LIFE_CYCLE_SMODE_NONE		= 0x400,
	LIFE_CYCLE_SMODE_LOW_BATTERY    = 0x401,
	LIFE_CYCLE_SMODE_WARM_BOOT_USB_CONNECTED  = 0x402,
	LIFE_CYCLE_SMODE_OTA            = 0x403,
	LIFE_CYCLE_SMODE_FACTORY_RESET  = 0x404,
}life_cycle_reason_t;

/* sign_of_life_reason_data */
struct sign_of_life_reason_data {
        life_cycle_reason_t reason_value;
        char  *life_cycle_reasons;
        char  *life_cycle_type;
};

/* used to store persist data in wdt, each one is one bit */
typedef enum wd_persist_data {
	WD_BOOTMODE_FASTBOOT,      /* 0 */
	WD_BOOTMODE_RPMB_PROGRAM,  /* 1 */
	WD_BOOTMODE_RECOVERY,      /* 2 */

	/* Sign of Life flags, taken from life_cycle_reasons_mtk.c */

	WD_SOL_WARM_BOOT_KERNEL_PANIC, /* 3 */
	WD_SOL_WARM_BOOT_KERNEL_WDOG, 
	WD_SOL_WARM_BOOT_HW_WDOG, 
	WD_SOL_WARM_BOOT_SW, 
	WD_SOL_COLD_BOOT_USB, 
	WD_SOL_COLD_BOOT_POWER_KEY, 
	WD_SOL_COLD_BOOT_POWER_SUPPLY, 

	WD_SOL_SHUTDOWN_LONG_PWR_KEY_PRESS, /* 10 */
	WD_SOL_SHUTDOWN_SW, 
	WD_SOL_SHUTDOWN_PWR_KEY, 
	WD_SOL_SHUTDOWN_SUDDEN_PWR_LOSS, 
	WD_SOL_SHUTDOWN_UKNOWN, 

	WD_SOL_THERMAL_SHUTDOWN_BATTERY, /* 15 */
	WD_SOL_THERMAL_SHUTDOWN_PMIC, 
	WD_SOL_THERMAL_SHUTDOWN_SOC, 
	WD_SOL_THERMAL_SHUTDOWN_MODEM, 
	WD_SOL_THERMAL_SHUTDOWN_WIFI, 
	WD_SOL_THERMAL_SHUTDOWN_PCB, 

	WD_SOL_SPECIAL_MODE_LOW_BATTERY, /* 21 */
	WD_SOL_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED, 
	WD_SOL_SPECIAL_MODE_OTA, 
	WD_SOL_SPECIAL_MODE_FACTORY_RESET, 

} WD_PERSIST_DATA;

#define WD_BOOTMODE_MASK           ((1<<WD_BOOTMODE_FASTBOOT) | (1<<WD_BOOTMODE_RPMB_PROGRAM) | (1<<WD_BOOTMODE_RECOVERY))
#define WD_SOL_BOOT_REASON_MASK    ((1<<WD_SOL_WARM_BOOT_KERNEL_PANIC) | (1<<WD_SOL_WARM_BOOT_KERNEL_WDOG) | \
									(1<<WD_SOL_WARM_BOOT_HW_WDOG) | (1<<WD_SOL_WARM_BOOT_SW) | \
									(1<<WD_SOL_COLD_BOOT_USB) | (1<<WD_SOL_COLD_BOOT_POWER_KEY) | \
									(1<<WD_SOL_COLD_BOOT_POWER_SUPPLY))
#define WD_SOL_SHUTDOWN_MASK       ((1<<WD_SOL_SHUTDOWN_LONG_PWR_KEY_PRESS) | (1<<WD_SOL_SHUTDOWN_SW) | \
									(1<<WD_SOL_SHUTDOWN_PWR_KEY) | (1<<WD_SOL_SHUTDOWN_SUDDEN_PWR_LOSS) | \
									(1<<WD_SOL_SHUTDOWN_UKNOWN))
#define WD_SOL_THERMAL_SHUTDOWN_MASK   ((1<<WD_SOL_THERMAL_SHUTDOWN_BATTERY) | (1<<WD_SOL_THERMAL_SHUTDOWN_PMIC) | \
										(1<<WD_SOL_THERMAL_SHUTDOWN_SOC) | (1<<WD_SOL_THERMAL_SHUTDOWN_MODEM) | \
										(1<<WD_SOL_THERMAL_SHUTDOWN_WIFI) | (1<<WD_SOL_THERMAL_SHUTDOWN_PCB))
#define WD_SOL_SPECIAL_MODE_MASK   ((1<<WD_SOL_SPECIAL_MODE_LOW_BATTERY) | (1<<WD_SOL_SPECIAL_MODE_WARM_BOOT_USB_CONNECTED) | \
									(1<<WD_SOL_SPECIAL_MODE_OTA) | (1<<WD_SOL_SPECIAL_MODE_FACTORY_RESET))

/* sign of life operations */
typedef struct sign_of_life_ops {
	int (*read_boot_reason)(life_cycle_reason_t *boot_reason);
	int (*write_boot_reason)(life_cycle_reason_t boot_reason);
	int (*read_shutdown_reason)(life_cycle_reason_t *shutdown_reason);
	int (*write_shutdown_reason)(life_cycle_reason_t shutdown_reason);
	int (*read_thermal_shutdown_reason)(life_cycle_reason_t *thermal_shutdown_reason);
	int (*write_thermal_shutdown_reason)(life_cycle_reason_t thermal_shutdown_reason);
	int (*read_special_mode)(life_cycle_reason_t *special_mode);
	int (*write_special_mode)(life_cycle_reason_t special_mode);
	int (*lcr_reset)(void);
} sign_of_life_ops;


/*
 * life_cycle_set_boot_reason
 * Description: this function will set the boot reason which causing device booting
 */
int life_cycle_set_boot_reason(life_cycle_reason_t boot_reason);

/*
 * life_cycle_set_shutdown_reason
 * Description: this function will set the Shutdown reason which causing device shutdown
 */
int life_cycle_set_shutdown_reason(life_cycle_reason_t shutdown_reason);

/*
 * life_cycle_set_thermal_shutdown_reason
 * Description: this function will set the Thermal Shutdown reason which causing device shutdown
 */
int life_cycle_set_thermal_shutdown_reason(life_cycle_reason_t thermal_shutdown_reason);

/*
 * life_cycle_set_special_mode
 * Description: this function will set the special mode which causing device life cycle change
 */
int life_cycle_set_special_mode(life_cycle_reason_t life_cycle_special_mode);

#endif