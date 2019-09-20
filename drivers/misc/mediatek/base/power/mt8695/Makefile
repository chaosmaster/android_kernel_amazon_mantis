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

ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/mt8695
ccflags-y += -I$(srctree)/drivers/misc/mediatek/thermal/mt8695/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/hps_v1/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/mt8695/include
obj-y += mt8695-cpufreq.o
obj-y += mtk_static_power.o
obj-y += mtk_power_throttle.o
obj-$(CONFIG_MTK_GPU_SUPPORT) += mtk_gpufreq.o
obj-y += mtk_svs.o
