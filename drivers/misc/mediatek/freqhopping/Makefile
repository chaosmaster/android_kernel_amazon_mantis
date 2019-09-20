subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/freqhopping/

obj-$(CONFIG_MTK_FREQ_HOPPING) := mt_freqhopping_drv.o

ifeq ($(CONFIG_MTK8695_FREQ_HOPPING),y)
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/freqhopping/mt8695/
obj-y += mt8695/
endif
