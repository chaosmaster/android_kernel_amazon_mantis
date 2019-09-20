ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc   \
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys   \
			 -I$(srctree)/drivers/misc/mediatek/mmp \
			 -I$(srctree)/include/soc/mediatek/ \
			 -I$(srctree)/drivers/staging/android/ion \
			 -I$(srctree)/drivers/staging/android/mtk_ion/mtk \
             -I$(srctree)/include/linux

obj-$(CONFIG_MTK_FB) += mtk_disp_mgr.o mtkfb.o
obj-$(CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER) += mtkfb_console.o disp_assert_layer.o
