#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

obj-$(CONFIG_MACH_MT8521P) += mt8127/
ifeq ($(CONFIG_MTK_PLATFORM),"mt8695")
obj-y += hps_v1/
endif
obj-$(CONFIG_MTK_BASE_POWER_MT7622) += mt7622/
obj-$(CONFIG_MTK_BASE_POWER) += mt8695/

ifeq ($(CONFIG_MTK_PLATFORM),"mt8695")
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/hps_v1/
endif
