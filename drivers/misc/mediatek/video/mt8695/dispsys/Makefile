
ccflags-y += -DMET_USER_EVENT_SUPPORT
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc	\
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/vdout_sys \
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/disp_sys \
             -I$(srctree)/drivers/misc/mediatek/video/$(CONFIG_MTK_PLATFORM)/dispsys/disp_hal/fmt \
             -I$(srctree)/drivers/misc/mediatek/mmp \
             -I$(srctree)/drivers/misc/mediatek/sync \
             -I$(srctree)/drivers/staging/android/ion \
             -I$(srctree)/drivers/staging/android/mtk_ion/mtk \
             -I$(srctree)/drivers/staging/android/uapi \
			 -I$(srctree)/include/soc/mediatek \
             -I$(srctree)/include/linux \
             -I$(srctree)/include/trustzone \


#common
obj-y += disp_hw_mgr.o
obj-y += disp_hw_log.o
obj-y += disp_hw_debug.o
obj-y += disp_clk.o
obj-y += disp_path.o
obj-y += disp_irq.o

obj-y += osd/
obj-y += vdp/
obj-y += disp_hal/
obj-y += pmx/
obj-y += p2i/
obj-y += pp/
obj-y += videoin/
obj-y += hdr/
obj-y += dovi/
# EOF
