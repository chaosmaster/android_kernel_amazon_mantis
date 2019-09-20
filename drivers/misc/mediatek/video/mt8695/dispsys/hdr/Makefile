
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mdp/cli
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys/disp_hal/vdout_sys
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys/disp_hal/disp_sys
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys/disp_hal/fmt
ccflags-y += -I$(srctree)/drivers/misc/mediatek/video/mt8695/dispsys/vdp
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)/inc
ccflags-y += -I$(srctree)/include/trustzone


obj-y += disp_hdr2sdr.o
obj-y += disp_bt2020.o
obj-y += disp_hdr_core.o
obj-y += disp_hdr_device.o
obj-y += disp_sdr2hdr.o
obj-y += disp_hdr_table.o
obj-y += disp_hdr_util.o
obj-y += disp_hdr_path.o
obj-y += mt8697/disp_hdr_platform.o
obj-$(CONFIG_MTK_CLI_DEBUG_SUPPORT) += disp_hdr_cli.o


ifeq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT), y)
ccflags-y += -DHDR_SECURE_SUPPORT
obj-y += disp_hdr_sec.o
else
#ccflags-y += -DHDR_SECURE_SUPPORT
#obj-y += disp_hdr_sec.o
endif

# EOF
