# drivers/dispsys/disp_hal/osd

ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc	\
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys \
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/osd \
             -I$(srctree)/drivers/misc/mediatek/mmp \
             -I$(srctree)/drivers/misc/mediatek/sync \
             -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/uapi \
             -I$(srctree)/include

obj-y += osd_base_hw.o
obj-y += osd_plane_hw.o
obj-y += osd_region_hw.o
obj-y += osd_scaler_hw.o
obj-y += osd_sw.o

# EOF
