/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "mtu3.h"

struct mtu3_usbreg_set {
	unsigned int offset;
	const char *name;
	unsigned int width;
	const char *comment;
};

/*
 * Module name: ssusb_sifslv_ippc Base address: (+11280700h)
 * Address	Name	Width		Register Function
 */
static struct mtu3_usbreg_set ssusb_sifslv_ippc[] = {
	{ 0x0000, "SSUSB_IP_PW_CTRL0",	 32, "SSUSB IP Power and Clock Control Register 0" },
	{ 0x0004, "SSUSB_IP_PW_CTRL1",	 32, "SSUSB IP Power and Clock Control Register 1" },
	{ 0x0008, "SSUSB_IP_PW_CTRL2",	 32, "SSUSB IP Power and Clock Control Register 2" },
	{ 0x000c, "SSUSB_IP_PW_CTRL3",	 32, "SSUSB IP Power and Clock Control Register 3" },
	{ 0x0010, "SSUSB_IP_PW_STS1",	 32, "SSUSB IP Power and Clock Status Register 1" },
	{ 0x0014, "SSUSB_IP_PW_STS2",	 32, "SSUSB IP Power and Clock Status Register 2" },
	{ 0x0018, "SSUSB_OTG_STS",	 32, "SSUSB OTG STATUS" },
	{ 0x001c, "SSUSB_OTG_STS_CLR",	 32, "SSUSB OTG STATUS CLEAR" },
	{ 0x0020, "SSUSB_IP_MAC_CAP",	 32, "SSUSB IP MAC Capability Register" },
	{ 0x0024, "SSUSB_IP_XHCI_CAP",	 32, "SSUSB IP xHCI Capability Register" },
	{ 0x0028, "SSUSB_IP_DEV_CAP",	 32, "SSUSB IP Device Capability Register" },
	{ 0x002c, "SSUSB_OTG_INT_EN",	 32, "SSUSB OTG INTERRUPT Enable" },
	{ 0x0030, "SSUSB_U3_CTRL_0P",	 32, "SSUSB IP U3 Port 0 Control Register" },
	{ 0x0038, "SSUSB_U3_CTRL_1P",	 32, "SSUSB IP U3 Port 1 Control Register" },
	{ 0x0040, "SSUSB_U3_CTRL_2P",	 32, "SSUSB IP U3 Port 2 Control Register" },
	{ 0x0048, "SSUSB_U3_CTRL_3P",	 32, "SSUSB IP U3 Port 3 Control Register" },
	{ 0x0050, "SSUSB_U2_CTRL_0P",	 32, "SSUSB IP U2 Port 0 Control Register" },
	{ 0x0058, "SSUSB_U2_CTRL_1P",	 32, "SSUSB IP U2 Port 1 Control Register" },
	{ 0x0060, "SSUSB_U2_CTRL_2P",	 32, "SSUSB IP U2 Port 2 Control Register" },
	{ 0x0068, "SSUSB_U2_CTRL_3P",	 32, "SSUSB IP U2 Port 3 Control Register" },
	{ 0x0070, "SSUSB_U2_CTRL_4P",	 32, "SSUSB IP U2 Port 4 Control Register" },
	{ 0x0078, "SSUSB_U2_CTRL_5P",	 32, "SSUSB IP U2 Port 5 Control Register" },
	{ 0x007c, "SSUSB_U2_PHY_PLL",	 32, "SSUSB U2 PHY PLL Control Register" },
	{ 0x0080, "SSUSB_DMA_CTRL",	 32, "SSUSB DMA Control Register" },
	{ 0x0084, "SSUSB_MAC_CK_CTRL",	 32, "SSUSB MAC Clock Control Register" },
	{ 0x0088, "SSUSB_CSR_CK_CTRL",	 32, "" },
	{ 0x008c, "SSUSB_REF_CK_CTRL",	 32, "SSUSB Reference Clock Control Register" },
	{ 0x0090, "SSUSB_XHCI_CK_CTRL",	 32, "SSUSB XHCI Clock Control Register" },
	{ 0x0094, "SSUSB_XHCI_RST_CTRL", 32, "SSUSB XHCI Reset Control Register" },
	{ 0x0098, "SSUSB_DEV_RST_CTRL",	 32, "SSUSB Device Reset Control Register" },
	{ 0x009c, "SSUSB_SYS_CK_CTRL",	 32, "SSUSB System Clock Control Register" },
	{ 0x00a0, "SSUSB_HW_ID",	 32, "SSUSB HW ID" },
	{ 0x00a4, "SSUSB_HW_SUB_ID",	 32, "SSUSB HW SUB ID" },
	{ 0x00b0, "SSUSB_PRB_CTRL0",	 32, "Probe Control Register 0" },
	{ 0x00b4, "SSUSB_PRB_CTRL1",	 32, "Probe Control Register 1" },
	{ 0x00b8, "SSUSB_PRB_CTRL2",	 32, "Probe Control Register 2" },
	{ 0x00bc, "SSUSB_PRB_CTRL3",	 32, "Probe Control Register 3" },
	{ 0x00c0, "SSUSB_PRB_CTRL4",	 32, "Probe Control Register 4" },
	{ 0x00c4, "SSUSB_PRB_CTRL5",	 32, "Probe Control Register 5" },
	{ 0x00c8, "SSUSB_IP_SPARE0",	 32, "SSUSB IP Spare Register0" },
	{ 0x00cc, "SSUSB_IP_SPARE1",	 32, "SSUSB IP Spare Register1" },
	{ 0x00d0, "SSUSB_FPGA_I2C_OUT_0P",	 32, "SSUSB FPGA I2C 0P Output Register" },
	{ 0x00d4, "SSUSB_FPGA_I2C_IN_0P",	 32, "SSUSB FPGA I2C 0P Input Register" },
	{ 0x00d8, "SSUSB_FPGA_I2C_OUT_1P",	 32, "SSUSB FPGA I2C 1P Output Register" },
	{ 0x00dc, "SSUSB_FPGA_I2C_IN_1P",	 32, "SSUSB FPGA I2C 1P Input Register" },
	{ 0x00e0, "SSUSB_FPGA_I2C_OUT_2P",	 32, "SSUSB FPGA I2C 2P Output Register" },
	{ 0x00e4, "SSUSB_FPGA_I2C_IN_2P",	 32, "SSUSB FPGA I2C 2P Input Register" },
	{ 0x00e8, "SSUSB_FPGA_I2C_OUT_3P",	 32, "SSUSB FPGA I2C 3P Output Register" },
	{ 0x00ec, "SSUSB_FPGA_I2C_IN_3P",	 32, "SSUSB FPGA I2C 3P Input Register" },
	{ 0x00f0, "SSUSB_FPGA_I2C_OUT_4P",	 32, "SSUSB FPGA I2C 4P Output Register" },
	{ 0x00f4, "SSUSB_FPGA_I2C_IN_4P",	 32, "SSUSB FPGA I2C 4P Input Register" },
	{ 0x00f8, "SSUSB_IP_SLV_TMOUT",		 32, "SSUSB IP SLAVE TIMEOUT" },
	{ 0, NULL, 0, NULL }
};

/*
 * Module name: ssusb_device_ssusb_dev Base address: (+11271000h)
 * Address	Name	Width		Register Function
 */
static struct mtu3_usbreg_set ssusb_device_ssusb_dev[] = {
	{ 0x000, "LV1ISR", 32, "Level1 Interrupt Status Register" },
	{ 0x004, "LV1IER", 32, "Level1 Interrupt Enable Register" },
	{ 0x008, "LV1IESR", 32, "Level1 Interrupt Enable Set Register" },
	{ 0x00C, "LV1IECR", 32, "Level1 Interrupt Enable Clear Register" },
	{ 0x030, "MAC_U1_EN_CTRL", 32, "MAC U1 Enable Control Register" },
	{ 0x034, "MAC_U2_EN_CTRL", 32, "MAC U2 Enable Control Register" },
	{ 0x040, "SRAM_DBG_CTRL", 32, "SRAM debug mode" },
	{ 0x044, "SRAM_DBG_CTRL_1", 32, "SRAM debug mode_1" },
	{ 0x050, "RISC_SIZE", 32, "RISC_SIZE" },
	{ 0x070, "WRBUF_ERR_STS", 32, "Buffer Error Flag" },
	{ 0x074, "BUF_ERR_EN", 32, "Buffer Error Enable" },
	{ 0x080, "EPISR", 32, "Endpoint Interrupt Status Register" },
	{ 0x084, "EPIER", 32, "Endpoint Interrupt Enable Register" },
	{ 0x088, "EPIESR", 32, "Endpoint Interrupt Enable Set Register" },
	{ 0x08C, "EPIECR", 32, "Endpoint Interrupt Enable Clear Register" },
	{ 0x090, "DMAISR", 32, "DMA Interrupt Status Register" },
	{ 0x094, "DMAIER", 32, "DMA Interrupt Enable Register" },
	{ 0x098, "DMAIESR", 32, "DMA Interrupt Enable Set Register" },
	{ 0x09C, "DMAIECR", 32, "DMA Interrupt Enable Clear Register" },
	{ 0x0C0, "EP0DMACTRL", 32, "EP0 DMA Control and FIFO Start Address Register" },
	{ 0x0C4, "EP0DMASTRADDR", 32, "EP0 DMA Start address" },
	{ 0x0C8, "EP0DMATFRCOUNT", 32, "EP0 DMA Transfer Count Register" },
	{ 0x0CC, "EP0DMARLCOUNT", 32, "EP0 DMA Real Count Register" },
	{ 0x0D0, "TXDMACTRL", 32, "Tx DMA Control and FIFO Start Address Register" },
	{ 0x0D4, "TXDMASTRADDR", 32, "Tx DMA Start address" },
	{ 0x0D8, "TXDMATRDCNT", 32, "Tx DMA Transfer Count Register" },
	{ 0x0DC, "TXDMARLCOUNT", 32, "Tx DMA Real Count Register" },
	{ 0x0E0, "RXDMACTRL", 32, "Rx DMA Control Register" },
	{ 0x0E4, "RXDMASTRADDR", 32, "Rx DMA Start address" },
	{ 0x0E8, "RXDMATRDCNT", 32, "Rx DMA Transfer Count Register" },
	{ 0x0EC, "RXDMARLCOUNT", 32, "Rx DMA Real Count Register" },
	{ 0x100, "EP0CSR", 32, "EP0 Control Status Register" },
	{ 0x108, "RXCOUNT0", 32, "EP0 Received bytes Register" },
	{ 0x10C, "RESERVED", 32, "EP0 Reserved Byte" },
	{ 0x110, "TX1CSR0", 32, "Tx EP 1 Control Status Register 0" },
	{ 0x114, "TX1CSR1", 32, "Tx EP 1 Control Status Register 1" },
	{ 0x118, "TX1CSR2", 32, "Tx EP 1 Control Status Register 2" },
	{ 0x120, "TX2CSR0", 32, "Tx EP 2 Control Status Register 0" },
	{ 0x124, "TX2CSR1", 32, "Tx EP 2 Control Status Register 1" },
	{ 0x128, "TX2CSR2", 32, "Tx EP 2 Control Status Register 2" },
	{ 0x130, "TX3CSR0", 32, "Tx EP 3 Control Status Register 0" },
	{ 0x134, "TX3CSR1", 32, "Tx EP 3 Control Status Register 1" },
	{ 0x138, "TX3CSR2", 32, "Tx EP 3 Control Status Register 2" },
	{ 0x140, "TX4CSR0", 32, "Tx EP 4 Control Status Register 0" },
	{ 0x144, "TX4CSR1", 32, "Tx EP 4 Control Status Register 1" },
	{ 0x148, "TX4CSR2", 32, "Tx EP 4 Control Status Register 2" },
	{ 0x150, "TX5CSR0", 32, "Tx EP 5 Control Status Register 0" },
	{ 0x154, "TX5CSR1", 32, "Tx EP 5 Control Status Register 1" },
	{ 0x158, "TX5CSR2", 32, "Tx EP 5 Control Status Register 2" },
	{ 0x160, "TX6CSR0", 32, "Tx EP 6 Control Status Register 0" },
	{ 0x164, "TX6CSR1", 32, "Tx EP 6 Control Status Register 1" },
	{ 0x168, "TX6CSR2", 32, "Tx EP 6 Control Status Register 2" },
	{ 0x170, "TX7CSR0", 32, "Tx EP 7 Control Status Register 0" },
	{ 0x174, "TX7CSR1", 32, "Tx EP 7 Control Status Register 1" },
	{ 0x178, "TX7CSR2", 32, "Tx EP 7 Control Status Register 2" },
	{ 0x180, "TX8CSR0", 32, "Tx EP 8 Control Status Register 0" },
	{ 0x184, "TX8CSR1", 32, "Tx EP 8 Control Status Register 1" },
	{ 0x188, "TX8CSR2", 32, "Tx EP 8 Control Status Register 2" },
	{ 0x210, "RX1CSR0", 32, "RX EP 1 Control Status Register 0" },
	{ 0x214, "RX1CSR1", 32, "RX EP 1 Control Status Register 1" },
	{ 0x218, "RX1CSR2", 32, "RX EP 1 Control Status Register 2" },
	{ 0x21C, "RX1CSR3", 32, "RX EP 1 Control Status Register 3" },
	{ 0x220, "RX2CSR0", 32, "RX EP 2 Control Status Register 0" },
	{ 0x224, "RX2CSR1", 32, "RX EP 2 Control Status Register 1" },
	{ 0x228, "RX2CSR2", 32, "RX EP 2 Control Status Register 2" },
	{ 0x22C, "RX2CSR3", 32, "RX EP 2 Control Status Register 3" },
	{ 0x230, "RX3CSR0", 32, "RX EP 3 Control Status Register 0" },
	{ 0x234, "RX3CSR1", 32, "RX EP 3 Control Status Register 1" },
	{ 0x238, "RX3CSR2", 32, "RX EP 3 Control Status Register 2" },
	{ 0x23C, "RX3CSR3", 32, "RX EP 3 Control Status Register 3" },
	{ 0x240, "RX4CSR0", 32, "RX EP 4 Control Status Register 0" },
	{ 0x244, "RX4CSR1", 32, "RX EP 4 Control Status Register 1" },
	{ 0x248, "RX4CSR2", 32, "RX EP 4 Control Status Register 2" },
	{ 0x24C, "RX4CSR3", 32, "RX EP 4 Control Status Register 3" },
	{ 0x250, "RX5CSR0", 32, "RX EP 5 Control Status Register 0" },
	{ 0x254, "RX5CSR1", 32, "RX EP 5 Control Status Register 1" },
	{ 0x258, "RX5CSR2", 32, "RX EP 5 Control Status Register 2" },
	{ 0x25C, "RX5CSR3", 32, "RX EP 5 Control Status Register 3" },
	{ 0x260, "RX6CSR0", 32, "RX EP 6 Control Status Register 0" },
	{ 0x264, "RX6CSR1", 32, "RX EP 6 Control Status Register 1" },
	{ 0x268, "RX6CSR2", 32, "RX EP 6 Control Status Register 2" },
	{ 0x26C, "RX6CSR3", 32, "RX EP 6 Control Status Register 3" },
	{ 0x270, "RX7CSR0", 32, "RX EP 7 Control Status Register 0" },
	{ 0x274, "RX7CSR1", 32, "RX EP 7 Control Status Register 1" },
	{ 0x278, "RX7CSR2", 32, "RX EP 7 Control Status Register 2" },
	{ 0x27C, "RX7CSR3", 32, "RX EP 7 Control Status Register 3" },
	{ 0x280, "RX8CSR0", 32, "RX EP 8 Control Status Register 0" },
	{ 0x284, "RX8CSR1", 32, "RX EP 8 Control Status Register 1" },
	{ 0x288, "RX8CSR2", 32, "RX EP 8 Control Status Register 2" },
	{ 0x28C, "RX8CSR3", 32, "RX EP 8 Control Status Register 3" },
	{ 0x300, "FIFO0", 32, "USB Endpoint 0 FIFO Register" },
	{ 0x310, "FIFO1", 32, "USB Endpoint 1 FIFO Register" },
	{ 0x320, "FIFO2", 32, "USB Endpoint 2 FIFO Register" },
	{ 0x330, "FIFO3", 32, "USB Endpoint 3 FIFO Register" },
	{ 0x340, "FIFO4", 32, "USB Endpoint 4 FIFO Register" },
	{ 0x350, "FIFO5", 32, "USB Endpoint 5 FIFO Register" },
	{ 0x360, "FIFO6", 32, "USB Endpoint 6 FIFO Register" },
	{ 0x370, "FIFO7", 32, "USB Endpoint 7 FIFO Register" },
	{ 0x380, "FIFO8", 32, "USB Endpoint 8 FIFO Register" },
	{ 0x390, "FIFO9", 32, "USB Endpoint 9 FIFO Register" },
	{ 0x3A0, "FIFO10", 32, "USB Endpoint 10 FIFO Register" },
	{ 0x3B0, "FIFO11", 32, "USB Endpoint 11 FIFO Register" },
	{ 0x3C0, "FIFO12", 32, "USB Endpoint 12 FIFO Register" },
	{ 0x3D0, "FIFO13", 32, "USB Endpoint 13 FIFO Register" },
	{ 0x3E0, "FIFO14", 32, "USB Endpoint 14 FIFO Register" },
	{ 0x3F0, "FIFO15", 32, "USB Endpoint 15 FIFO Register" },
	{ 0x400, "QCR0", 32, "Queue Control Register 0" },
	{ 0x404, "QCR1", 32, "Queue Control Register 1" },
	{ 0x408, "QCR2", 32, "Queue Control Register 2" },
	{ 0x40C, "QCR3", 32, "Queue Control Register 3" },
	{ 0x510, "TXQCSR1", 32, "TX Queue Command and Status Register 1" },
	{ 0x514, "TXQSAR1", 32, "TX Queue Starting Address Register 1" },
	{ 0x518, "TXQCPR1", 32, "TX Queue Current Pointer Register 1" },
	{ 0x520, "TXQCSR2", 32, "TX Queue Command and Status Register 2" },
	{ 0x524, "TXQSAR2", 32, "TX Queue Starting Address Register 2" },
	{ 0x528, "TXQCPR2", 32, "TX Queue Current Pointer Register 2" },
	{ 0x530, "TXQCSR3", 32, "TX Queue Command and Status Register 3" },
	{ 0x534, "TXQSAR3", 32, "TX Queue Starting Address Register 3" },
	{ 0x538, "TXQCPR3", 32, "TX Queue Current Pointer Register 3" },
	{ 0x540, "TXQCSR4", 32, "TX Queue Command and Status Register 4" },
	{ 0x544, "TXQSAR4", 32, "TX Queue Starting Address Register 4" },
	{ 0x548, "TXQCPR4", 32, "TX Queue Current Pointer Register 4" },
	{ 0x550, "TXQCSR5", 32, "TX Queue Command and Status Register 5" },
	{ 0x554, "TXQSAR5", 32, "TX Queue Starting Address Register 5" },
	{ 0x558, "TXQCPR5", 32, "TX Queue Current Pointer Register 5" },
	{ 0x560, "TXQCSR6", 32, "TX Queue Command and Status Register 6" },
	{ 0x564, "TXQSAR6", 32, "TX Queue Starting Address Register 6" },
	{ 0x568, "TXQCPR6", 32, "TX Queue Current Pointer Register 6" },
	{ 0x570, "TXQCSR7", 32, "TX Queue Command and Status Register 7" },
	{ 0x574, "TXQSAR7", 32, "TX Queue Starting Address Register 7" },
	{ 0x578, "TXQCPR7", 32, "TX Queue Current Pointer Register 7" },
	{ 0x580, "TXQCSR8", 32, "TX Queue Command and Status Register 8" },
	{ 0x584, "TXQSAR8", 32, "TX Queue Starting Address Register 8" },
	{ 0x588, "TXQCPR8", 32, "TX Queue Current Pointer Register 8" },
	{ 0x610, "RXQCSR1", 32, "RX Queue Command and Status Register 1" },
	{ 0x614, "RXQSAR1", 32, "RX Queue Starting Address Register 1" },
	{ 0x618, "RXQCPR1", 32, "RX Queue Current Pointer Register 1" },
	{ 0x61C, "RXQLDPR1", 32, "RX Queue Last Done Pointer Register 1" },
	{ 0x620, "RXQCSR2", 32, "RX Queue Command and Status Register 2" },
	{ 0x624, "RXQSAR2", 32, "RX Queue Starting Address Register 2" },
	{ 0x628, "RXQCPR2", 32, "RX Queue Current Pointer Register 2" },
	{ 0x62C, "RXQLDPR2", 32, "RX Queue Last Done Pointer Register 2" },
	{ 0x630, "RXQCSR3", 32, "RX Queue Command and Status Register 3" },
	{ 0x634, "RXQSAR3", 32, "RX Queue Starting Address Register 3" },
	{ 0x638, "RXQCPR3", 32, "RX Queue Current Pointer Register 3" },
	{ 0x63C, "RXQLDPR3", 32, "RX Queue Last Done Pointer Register 3" },
	{ 0x640, "RXQCSR4", 32, "RX Queue Command and Status Register 4" },
	{ 0x644, "RXQSAR4", 32, "RX Queue Starting Address Register 4" },
	{ 0x648, "RXQCPR4", 32, "RX Queue Current Pointer Register 4" },
	{ 0x64C, "RXQLDPR4", 32, "RX Queue Last Done Pointer Register 4" },
	{ 0x650, "RXQCSR5", 32, "RX Queue Command and Status Register 5" },
	{ 0x654, "RXQSAR5", 32, "RX Queue Starting Address Register 5" },
	{ 0x658, "RXQCPR5", 32, "RX Queue Current Pointer Register 5" },
	{ 0x65C, "RXQLDPR5", 32, "RX Queue Last Done Pointer Register 5" },
	{ 0x660, "RXQCSR6", 32, "RX Queue Command and Status Register 6" },
	{ 0x664, "RXQSAR6", 32, "RX Queue Starting Address Register 6" },
	{ 0x668, "RXQCPR6", 32, "RX Queue Current Pointer Register 6" },
	{ 0x66C, "RXQLDPR6", 32, "RX Queue Last Done Pointer Register 6" },
	{ 0x670, "RXQCSR7", 32, "RX Queue Command and Status Register 7" },
	{ 0x674, "RXQSAR7", 32, "RX Queue Starting Address Register 7" },
	{ 0x678, "RXQCPR7", 32, "RX Queue Current Pointer Register 7" },
	{ 0x67C, "RXQLDPR7", 32, "RX Queue Last Done Pointer Register 7" },
	{ 0x680, "RXQCSR8", 32, "RX Queue Command and Status Register 8" },
	{ 0x684, "RXQSAR8", 32, "RX Queue Starting Address Register 8" },
	{ 0x688, "RXQCPR8", 32, "RX Queue Current Pointer Register 8" },
	{ 0x68C, "RXQLDPR8", 32, "RX Queue Last Done Pointer Register 8" },
	{ 0x700, "QISAR0", 32,
		"QMU Interrupt Status and Acknowledgment Register 0 (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x704, "QIER0", 32, "QMU Interrupt Enable Register 0 (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x708, "QIESR0", 32, "QMU Interrupt Enable Set Register 0 (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x70C, "QIECR0", 32, "QMU Interrupt Enable Clear Register 0 (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x710, "QISAR1", 32, "QMU Interrupt Status and Acknowledgment Register" },
	{ 0x714, "QIER1", 32, "QMU Interrupt Enable Register 1" },
	{ 0x718, "QIESR1", 32, "QMU Interrupt Enable Set Register 1" },
	{ 0x71C, "QIECR1", 32, "QMU Interrupt Enable Clear Register 1" },
	{ 0x740, "QEMIR", 32, "Queue Empty Indication Register (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x744, "QEMIER", 32, "Queue Empty Indication Enable Register (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x748, "QEMIESR", 32, "Queue Empty Indication Enable Set Register (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x74C, "QEMIECR", 32, "Queue Empty Indication Enable Clear Register (n from 1 to maximum TXQ/RXQ number)" },
	{ 0x780, "TQERRIR0", 32, "TX Queue Error Indication Register 0" },
	{ 0x784, "TQERRIER0", 32, "TX Queue Error Indication Enable Register 0" },
	{ 0x788, "TQERRIESR0", 32, "TX Queue Error Indication Enable Set Register 0" },
	{ 0x78C, "TQERRIECR0", 32, "TX Queue Error Indication Enable Clear Register 0" },
	{ 0x7C0, "RQERRIR0", 32, "RX Queue Error Indication Register 0" },
	{ 0x7C4, "RQERRIER0", 32, "RX Queue Error Indication Enable Register 0" },
	{ 0x7C8, "RQERRIESR0", 32, "RX Queue Error Indication Enable Set Register 0" },
	{ 0x7CC, "RQERRIECR0", 32, "RX Queue Error Indication Enable Clear Register 0" },
	{ 0x7D0, "RQERRIR1", 32, "RX Queue Error Indication Register 1" },
	{ 0x7D4, "RQERRIER1", 32, "RX Queue Error Indication Enable Register 1" },
	{ 0x7D8, "RQERRIESR1", 32, "RX Queue Error Indication Enable Set Register 1" },
	{ 0x7DC, "RQERRIECR1", 32, "RX Queue Error Indication Enable Clear Register 1" },
	{ 0xC04, "CAP_EP0FFSZ", 32, "EP0 FIFO size Capability Register" },
	{ 0xC08, "CAP_EPNTXFFSZ", 32, "EPN TX FIFO size Capability Register" },
	{ 0xC0C, "CAP_EPNRXFFSZ", 32, "EPN RX FIFO size Capability Register" },
	{ 0xC10, "CAP_EPINFO", 32, "EP and Queue number Capability Register" },
	{ 0xC20, "CAP_TX_SLOT1", 32, "EP TX SLOT Capability Register 1" },
	{ 0xC24, "CAP_TX_SLOT2", 32, "EP TX SLOT Capability Register 2" },
	{ 0xC28, "CAP_TX_SLOT3", 32, "EP TX SLOT Capability Register 3" },
	{ 0xC2C, "CAP_TX_SLOT4", 32, "EP TX SLOT Capability Register 4" },
	{ 0xC30, "CAP_RX_SLOT1", 32, "EP RX SLOT Capability Register 1" },
	{ 0xC34, "CAP_RX_SLOT2", 32, "EP RX SLOT Capability Register 2" },
	{ 0xC38, "CAP_RX_SLOT3", 32, "EP RX SLOT Capability Register 3" },
	{ 0xC3C, "CAP_RX_SLOT4", 32, "EP RX SLOT Capability Register 4" },
	{ 0xC84, "MISC_CTRL", 32, "Device miscellaneous control" },
	{ 0, NULL, 0, NULL }
};

/*
 * Module name: ssusb_epctl_csr Base address: (+11271800h)
 * Address	Name	Width		Register Function
 */
static struct mtu3_usbreg_set ssusb_epctl_csr[] = {
	{ 0x800, "DEVICE_CONF", 32, NULL },
	{ 0x804, "EP_RST", 32, NULL },
	{ 0x808, "USB3_ERDY_TIMING_PARAMETER", 32, NULL },
	{ 0x80C, "USB3_EPCTRL_CAP", 32, NULL },
	{ 0x810, "USB2_ISOINEP_INCOMP_INTR", 32, NULL },
	{ 0x814, "USB2_ISOOUTEP_INCOMP_ERR", 32, NULL },
	{ 0x818, "ISO_UNDERRUN_INTR", 32, NULL },
	{ 0x81C, "ISO_OVERRUN_INTR", 32, NULL },
	{ 0x820, "USB2_RX_EP_DATAERR_INTR", 32, NULL },
	{ 0x824, "USB2_EPCTRL_CAP", 32, NULL },
	{ 0x828, "USB2_EPCTL_LPM", 32, NULL },
	{ 0x830, "USB3_SW_ERDY", 32, NULL },
	{ 0x840, "EP_FLOW_CTRL", 32, NULL },
	{ 0x844, "USB3_EP_ACT", 32, NULL },
	{ 0x848, "USB3_EP_PACKET_PENDING", 32, NULL },
	{ 0x850, "DEV_LINK_INTR_ENABLE", 32, NULL },
	{ 0x854, "DEV_LINK_INTR", 32, NULL },
	{ 0x860, "USB2_EPCTL_LPM_FC_CHK", 32, NULL },
	{ 0x864, "DEVICE_MONITOR", 32, NULL },
	{ 0, NULL, 0, NULL }
};


/*
 * Module name: ssusb_usb2_csr Base address: (+11273400h)
 * Address	Name	Width		Register Function
 */
static struct mtu3_usbreg_set ssusb_usb2_csr[] = {
	{ 0x2400, "XHCI_PORT_CTRL", 32, NULL },
	{ 0x2404, "POWER_MANAGEMENT", 32, NULL },
	{ 0x2408, "TIMING_TEST_MODE", 32, NULL },
	{ 0x240C, "DEVICE_CONTROL", 32, NULL },
	{ 0x2410, "POWER_UP_COUNTER", 32, NULL },
	{ 0x2414, "USB2_TEST_MODE", 32, NULL },
	{ 0x2418, "COMMON_USB_INTR_ENABLE", 32, NULL },
	{ 0x241C, "COMMON_USB_INTR", 32, NULL },
	{ 0x2420, "USB_BUS_PERFORMANCE", 32, NULL },
	{ 0x2424, "LINK_RESET_INFO", 32, NULL },
	{ 0x2434, "RESET_RESUME_TIME_VALUE", 32, NULL },
	{ 0x2438, "UTMI_SIGNAL_SEL", 32, NULL },
	{ 0x243C, "USB20_FRAME_NUM", 32, NULL },
	{ 0x2440, "USB20_TIMING_PARAMETER", 32, NULL },
	{ 0x2444, "USB20_LPM_PARAMETER", 32, NULL },
	{ 0x2448, "USB20_LPM_ENTRY_COUNT", 32, NULL },
	{ 0x244C, "USB20_MISC_CONTROL", 32, NULL },
	{ 0x2450, "USB20_LPM_TIMING_PARAM", 32, NULL },
	{ 0, NULL, 0, NULL }
};


/*
 * Module name: ssusb_usb2_csr_1p Base address: (+11273600h)
 * Address	Name	Width		Register Function
 */
static struct mtu3_usbreg_set ssusb_usb2_csr_1p[] = {
	{ 0x2600, "XHCI_PORT_CTRL", 32, NULL },
	{ 0x2604, "POWER_MANAGEMENT", 32, NULL },
	{ 0x2608, "TIMING_TEST_MODE", 32, NULL },
	{ 0x260C, "DEVICE_CONTROL", 32, NULL },
	{ 0x2610, "POWER_UP_COUNTER", 32, NULL },
	{ 0x2614, "USB2_TEST_MODE", 32, NULL },
	{ 0x2618, "COMMON_USB_INTR_ENABLE", 32, NULL },
	{ 0x261C, "COMMON_USB_INTR", 32, NULL },
	{ 0x2620, "USB_BUS_PERFORMANCE", 32, NULL },
	{ 0x2624, "LINK_RESET_INFO", 32, NULL },
	{ 0x2634, "RESET_RESUME_TIME_VALUE", 32, NULL },
	{ 0x2638, "UTMI_SIGNAL_SEL", 32, NULL },
	{ 0x263C, "USB20_FRAME_NUM", 32, NULL },
	{ 0x2640, "USB20_TIMING_PARAMETER", 32, NULL },
	{ 0x2644, "USB20_LPM_PARAMETER", 32, NULL },
	{ 0x2648, "USB20_LPM_ENTRY_COUNT", 32, NULL },
	{ 0x264C, "USB20_MISC_CONTROL", 32, NULL },
	{ 0x2650, "USB20_LPM_TIMING_PARAM", 32, NULL },
	{ 0, NULL, 0, NULL }
};

static inline unsigned int uffs(unsigned int x)
{
	unsigned int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		 r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}

	return r;
}

#define IO_SET_FIELD(reg, field, val) \
		do { \
			unsigned int tv = readl(reg); \
			tv &= ~(field); \
			tv |= ((val) << (uffs((unsigned int)field) - 1)); \
			writel(tv, reg); \
		} while (0)

#define IO_GET_FIELD(reg, field, val) \
		do { \
			unsigned int tv = readl(reg); \
			val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
		} while (0)

static void register_set_field(void __iomem *address, unsigned int start_bit,
						unsigned int len, unsigned int value)
{
	unsigned long field;

	if (start_bit > 31 || start_bit < 0 || len > 31 || len <= 0
							|| (start_bit + len > 31))
		pr_err("[ssusb debug][Register RMW] Invalid Register field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		value &= (1 << len) - 1;
		pr_err("[ssusb debug][Register RMW]Original:0x%p (0x%x)\n", address, readl(address));
		IO_SET_FIELD(address, field, value);
		pr_err("[ssusb debug][Register RMW]Modified:0x%p (0x%x)\n", address, readl(address));
	}
}

static void register_get_field(void __iomem *address, unsigned int start_bit,
						unsigned int len, unsigned int value)
{
	unsigned long field;

	if (start_bit > 31 || start_bit < 0 || len > 31 || len <= 0
							|| (start_bit + len > 31))
		pr_err("[ssusb debug][Register RMW]Invalid reg field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		IO_GET_FIELD(address, field, value);
		pr_err("[ssusb debug][Register RMW]Reg:0x%p start_bit(%d)len(%d)(0x%x)\n",
								address, start_bit, len, value);
	}
}

/* Add verbose debugging later, just print everything for now */
static int mtk_ssusb_regdump_show(struct seq_file *s, void *unused)
{
	int i;
	struct mtu3_usbreg_set *usbxxreg;
	struct ssusb_mtk *ssusb = s->private;
	void __iomem *mbase = ssusb->mac_base;
	void __iomem *ibase = ssusb->ippc_base;

	seq_printf(s, "MTK SSUSB(ippc_base: 0x%p, mac_base: 0x%p)\n", ibase, mbase);
	if (PTR_ERR(ibase)) {
		seq_puts(s, "\nssusb_sifslv_ippc Register Dump range [0x11280700 ~ 0x11280800)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_sifslv_ippc); i++) {
			usbxxreg = &ssusb_sifslv_ippc[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-25s(0x%p): 0x%02X\n", usbxxreg->name, ibase + usbxxreg->offset,
									readb(ibase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-25s(0x%p): 0x%04X\n", usbxxreg->name, ibase + usbxxreg->offset,
									readw(ibase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-25s(0x%p): 0x%08X\n", usbxxreg->name, ibase + usbxxreg->offset,
									readl(ibase + usbxxreg->offset));
				break;
			}
		}
	}

	if (PTR_ERR(mbase)) {
		seq_puts(s, "\nssusb_device_ssusb_dev Register Dump range [0x11271000 ~ 0x11272000)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_device_ssusb_dev); i++) {
			usbxxreg = &ssusb_device_ssusb_dev[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-25s(0x%p): 0x%02X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readb(mbase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-25s(0x%p): 0x%04X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readw(mbase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-25s(0x%p): 0x%08X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readl(mbase + usbxxreg->offset));
				break;
			}
		}

		seq_puts(s, "\nssusb_epctl_csr Register Dump range [0x11271800 ~ 0x11271900)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_epctl_csr); i++) {
			usbxxreg = &ssusb_epctl_csr[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-25s(0x%p): 0x%02X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readb(mbase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-25s(0x%p): 0x%04X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readw(mbase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-25s(0x%p): 0x%08X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readl(mbase + usbxxreg->offset));
				break;
			}
		}

		seq_puts(s, "\nssusb_usb2_csr Register Dump range [0x11273400 ~ 0x11273450]\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_usb2_csr); i++) {
			usbxxreg = &ssusb_usb2_csr[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-25s(0x%p): 0x%02X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readb(mbase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-25s(0x%p): 0x%04X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readw(mbase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-25s(0x%p): 0x%08X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readl(mbase + usbxxreg->offset));
				break;
			}
		}

		seq_puts(s, "\nssusb_usb2_csr_1p Register Dump range [0x11273600 ~ 0x11273650]\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_usb2_csr_1p); i++) {
			usbxxreg = &ssusb_usb2_csr_1p[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-25s(0x%p): 0x%02X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readb(mbase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-25s(0x%p): 0x%04X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readw(mbase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-25s(0x%p): 0x%08X\n", usbxxreg->name, mbase + usbxxreg->offset,
									readl(mbase + usbxxreg->offset));
				break;
			}
		}
	}

	return 0;
}

static int mtk_ssusb_regdump(struct inode *inode, struct file *file)
{
	return single_open(file, mtk_ssusb_regdump_show, inode->i_private);
}

static const struct file_operations mtk_ssusb_regdump_fops = {
	.open	= mtk_ssusb_regdump,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int mtk_ssusb_debug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "\n===mtk_ssusb_debug help===\n");
	seq_puts(m, "\n   LOG control:        echo 0 [debug_zone] > debug\n");

	seq_puts(m, "\n   REGISTER control usage:\n");
	seq_puts(m, "       write register:   echo 1 0 [io_addr] [value] > debug\n");
	seq_puts(m, "       read  register:   echo 1 1 [io_addr] > debug\n");
	seq_puts(m, "       write mask:       echo 1 2 [io_addr] [start_bit] [len] [value] > debug\n");
	seq_puts(m, "       read  mask:       echo 1 3 [io_addr] [start_bit] [len] > debug\n");
	seq_puts(m, "       dump all regiters echo 1 4 > debug\n");
	seq_puts(m, "=========================================\n\n");

	return 0;
}

static ssize_t mtk_ssusb_debug_proc_write(struct file *file, const char *buf, size_t count, loff_t *data)
{
	int ret;
	int cmd, p1, p3, p4, p5;
	unsigned int reg_value;
	int sscanf_num;
	struct seq_file	*s = file->private_data;
	struct ssusb_mtk *ssusb = s->private;
	void __iomem *iomem = NULL;

	unsigned int long long p2;

	p1 = p2 = p3 = p4 = p5 = -1;

	if (count == 0)
		return -1;

	if (count > 255)
		count = 255;

	ret = copy_from_user(ssusb->cmd_buf, buf, count);
	if (ret < 0)
		return -1;

	ssusb->cmd_buf[count] = '\0';
	pr_err("[ssusb debug]debug received:%s\n", ssusb->cmd_buf);

	sscanf_num = sscanf(ssusb->cmd_buf, "%x %x %llx %x %x %x", &cmd, &p1, &p2, &p3, &p4, &p5);
	if (sscanf_num < 1)
		return count;

	if (cmd == 0)
		pr_err("[ssusb debug] zone <0x%.8x> is not exist yet\n", p1);
	else if (cmd == 1) {
		iomem += p2;
		if (p1 == 0) {
			reg_value = p3;
			pr_err("[ssusb debug][Register Write]Original:0x%p (0x%08X)\n",
								iomem, readl(iomem));

			writel(reg_value, iomem);

			pr_err("[ssusb debug][Register Write]Writed:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		} else if (p1 == 1)
			pr_err("[ssusb debug][Register Read]Register:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		else if (p1 == 2)
			register_set_field(iomem, p3, p4, p5);
		else if (p1 == 3)
			register_get_field(iomem, p3, p4, p5);
		else
			pr_err("[ssusb debug] todo\n");
	}

	return count;
}

static int mtk_ssusb_debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mtk_ssusb_debug_proc_show, inode->i_private);
}

static const struct file_operations mtk_ssusb_debug_proc_fops = {
	.open   = mtk_ssusb_debug_proc_open,
	.write  = mtk_ssusb_debug_proc_write,
	.read   = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int mtk_ssusb_init_debugfs(struct ssusb_mtk *ssusb)
{
	int	ret;
	struct dentry *file;
	struct dentry *root;

	root = debugfs_create_dir(dev_driver_string(ssusb->dev), NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	file = debugfs_create_file("regdump", S_IRUGO, root, ssusb, &mtk_ssusb_regdump_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	file = debugfs_create_file("debug", S_IRUGO | S_IWUSR, root, ssusb, &mtk_ssusb_debug_proc_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	ssusb->debugfs_root = root;

	return 0;

err1:
	debugfs_remove_recursive(root);

err0:
	return ret;
}

void mtk_ssusb_exit_debugfs(struct ssusb_mtk *ssusb)
{
	debugfs_remove_recursive(ssusb->debugfs_root);
}
