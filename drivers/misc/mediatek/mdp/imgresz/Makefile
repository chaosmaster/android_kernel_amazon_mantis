$(warning imgresz build $(srctree))

ccflags-y += -I$(srctree)/include/                              \
             -I$(srctree)/drivers/staging/android/mtk_ion/mtk/  \
             -I$(srctree)/include/soc/mediatek/                 \
             -I$(srctree)/drivers/staging/android/ion/          \
             -I$(srctree)/include/trustzone/ \
             -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys \
             -I$(srctree)/drivers/misc/mediatek/mmp \
             -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/mt8695/inc \

obj-y += imgresz_hal.o imgresz.o
