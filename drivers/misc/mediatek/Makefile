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

#$(call all-subdir-src-or-makefile)
subdir-ccflags-y += -Werror
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(CONFIG_MTK_PLATFORM)/include
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/

obj-$(CONFIG_MTK_BASE_POWER) += base/
obj-$(CONFIG_USB_MTK_HDRC) += usb20/
obj-$(CONFIG_MTK_ICE_DEBUG) +=ice_debug/
obj-$(CONFIG_RT_REGMAP) += rt-regmap/
obj-$(CONFIG_USB_C_SWITCH)	+= usb_c/

ifneq ($(wildcard $(srctree)/drivers/misc/mediatek/met_drv/),)
obj-y += met_drv/
endif

obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += trustzone/
ifeq ($(CONFIG_HAVE_MTK_IN_HOUSE_TEE_SUPPORT),y)
obj-$(CONFIG_TRUSTY) += trustzone/
endif
obj-${CONFIG_MTK_HWRNG} += hw_random/
obj-$(CONFIG_MTK_GIC_EXT) += ext_gic/
obj-$(CONFIG_MTK_HDMI_SUPPORT) += hdmi/
obj-$(CONFIG_MTK_SYS_CIRQ) += cirq/
obj-$(CONFIG_MMC_MTK_SDIO) += mtk_sdio/
obj-$(CONFIG_MTK_RAM_CONSOLE) += ram_console/
obj-$(CONFIG_MTK_AEE_FEATURE) += aee/
obj-$(CONFIG_MTK_EMI) += emi/
obj-$(CONFIG_MTK_FREQ_HOPPING) += freqhopping/
obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += gcpu/
ifeq ($(CONFIG_HAVE_MTK_IN_HOUSE_TEE_SUPPORT),y)
obj-$(CONFIG_TRUSTY) += gcpu/
obj-$(CONFIG_MTK_SECURE_EFUSE) += secure_efuse/
endif

obj-$(CONFIG_MTK_M4U)   += m4u/
obj-$(CONFIG_MTK_PSEUDO_M4U)   += pseudo_m4u/

obj-$(CONFIG_MMPROFILE) += mmp/
obj-$(CONFIG_MTK_SCHED_RQAVG_KS) += sched/
obj-$(CONFIG_MTK_TURNKEY_THERMAL) += thermal/
obj-$(CONFIG_MTK_IRRX) += irrx/
ifeq ($(CONFIG_MTK_SENSOR_SUPPORT),y)
obj-$(CONFIG_MTK_HWMON)	    += hwmon/
obj-$(CONFIG_CUSTOM_KERNEL_ALSPS) += alsps/
endif
obj-$(CONFIG_MTK_COMBO) += connectivity/
obj-$(CONFIG_MTK_BTIF) += btif/
obj-$(CONFIG_MTK_CONN_MD) += conn_md/
# for btmtk_btif repo sync from other repo
BTIF = $(wildcard drivers/misc/mediatek/btmtk_btif/*.c)
ifneq ($(BTIF),)
obj-$(CONFIG_MTK_BTMTK_BTIF) += btmtk_btif/
endif
obj-$(CONFIG_MTK_UNIFIED_DEBUG_INTERFACE) += udi/
obj-$(CONFIG_MTK_SYNC) += sync/
obj-$(CONFIG_MTK_FB) += video/
obj-$(CONFIG_MTK_VIDEOCODEC_DRIVER) += videocodec/
obj-$(CONFIG_MTK_MDP) += mdp/
obj-$(CONFIG_MTK_VQ) += vq/
obj-y += stc/
obj-y += systimer/
obj-y += devinfo/
obj-$(CONFIG_ARCH_MEDIATEK) += cmo/
obj-$(CONFIG_MTK_BOOT) += boot/
obj-$(CONFIG_MTK_BOOT_REASON) += boot_reason/
obj-$(CONFIG_MTK_SYSTRACKER) += systracker/
obj-$(CONFIG_MTK_DYNAMIC_BOOST) += dynamic_boost/
obj-$(CONFIG_MTK_FIQ_CACHE) += fiq_cache/
