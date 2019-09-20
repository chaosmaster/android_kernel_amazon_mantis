# drivers/dispsys
#include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y +=  -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/inc	\
							-I$(srctree)/drivers/staging/android/uapi/ \
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/vdp/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdo	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/fmt	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/videoin	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdout_sys	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/disp_sys	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/dovi/inc/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/dovi/inc/ \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/dovi/vdr_rpu/include/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/dovi/edbec/inc/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/dovi/dm/dlb_dm_process/include/	\
							-I$(srctree)/include/trustzone \
							-I$(srctree)/drivers/misc/mediatek/mmp \


#common
obj-y += disp_dovi_cmd.o
obj-y += disp_dovi_common_if.o
obj-y += disp_dovi_control_if.o
obj-y += disp_dovi_core1_if.o
obj-y += disp_dovi_core2_if.o
obj-y += disp_dovi_core3_if.o
obj-y += disp_dovi_main.o
obj-y += disp_dovi_io.o
obj-y += disp_dovi_tz_client.o

obj-y += unit/

ccflags-y += -DMET_USER_EVENT_SUPPORT


ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif

# EOF
