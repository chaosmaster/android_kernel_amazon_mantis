# drivers/dispsys
#include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y +=  -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/inc	\
							-I$(srctree)/drivers/staging/android/uapi/ \
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdo	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/fmt	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/videoin	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/dovi/vdr_rpu/include/	\


#common
obj-y += src/

ccflags-y += -DMET_USER_EVENT_SUPPORT


ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif

# EOF
