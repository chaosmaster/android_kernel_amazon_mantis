MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/base/power/$(MTK_PLATFORM)

snd-soc-mt8695-pcm-objs := \
        mt8695-afe-pcm.o mt8695_evb.o mt8695-afe-util.o mt8695_aud_debug.o\


obj-$(CONFIG_SND_SOC_MT8695) += snd-soc-mt8695-pcm.o

