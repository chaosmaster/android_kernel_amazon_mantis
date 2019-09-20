# drivers/dispsys/osd
#include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y +=  -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/videox \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/osd \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/pmx \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/osd \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdout_sys \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/disp_sys \
							-I$(srctree)/drivers/misc/mediatek/sync/ \
							-I$(srctree)/drivers/misc/mediatek/include/ \
							-I$(srctree)/drivers/staging/android \
							-I$(srctree)/drivers/staging/android/mtk_ion \
							-I$(srctree)/drivers/staging/android/ion/ \
							-I$(srctree)/drivers/staging/android/uapi/ \
							-I$(srctree)/include \
							-I$(srctree)/drivers/misc/mediatek/mmp \
							-I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc

#common
	obj-y += disp_osd_if.o
	obj-y += disp_osd_env.o
	obj-y += disp_osd_debug.o
	obj-y += disp_osd_fence.o
  obj-y += disp_osd_ion.o




ifeq ($(CONFIG_MMPROFILE),y)
ccflags-y += -DDEFAULT_MMP_ENABLE
endif

ccflags-y += -DMET_USER_EVENT_SUPPORT


ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif

# EOF
