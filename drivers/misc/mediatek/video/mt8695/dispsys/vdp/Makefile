# drivers/dispsys
#include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y +=  -I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/inc	\
							-I$(srctree)/drivers/misc/mediatek/sync/      \
							-I$(srctree)/drivers/misc/mediatek/mmp \
							-I$(srctree)/drivers/staging/android/mtk_ion \
							-I$(srctree)/drivers/staging/android/mtk_ion/mtk \
							-I$(srctree)/drivers/staging/android/uapi/ \
							-I$(srctree)/arch/arm/mach-mt8580/include/mach/inc/ \
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdo	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/fmt	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdout_sys	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/disp_sys \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/videox/	\
							-I$(srctree)/drivers/misc/mediatek/mdp/cli/	\
							-I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc/	\
							-I$(srctree)/include/trustzone \
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/dovi/inc	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/hdr	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/dovi/inc	\
							-I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/videoin	\

#common
obj-y += disp_vdp_debug.o
obj-y += disp_vdp_frc.o
#obj-y += disp_vdp_hw.o
obj-y += disp_vdp_if.o
obj-y += disp_vdp_vsync.o
obj-y += disp_vdp_sec.o
obj-$(CONFIG_MTK_CLI_DEBUG_SUPPORT) += disp_vdp_cli.o

ifeq ($(CONFIG_MMPROFILE),y)
ccflags-y += -DDEFAULT_MMP_ENABLE
endif

ccflags-y += -DMET_USER_EVENT_SUPPORT


ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif

# EOF
