/*
 * MediaTek xHCI Host Controller Driver
 *
 * Copyright (c) 2015 MediaTek Inc.
 * Author:
 *  Chunfeng Yun <chunfeng.yun@mediatek.com>
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
#include "xhci.h"
#include "xhci-mtk.h"

struct xhci_usbxx_set {
	unsigned int offset;
	const char *name;
	unsigned int width;
	const char *comment;
};

#define XHCI_INIT_VALUE 0x0

/*
 * Module name: ssusb_sifslv_ippc Base address: (+11280700h)
 * Address	Name	Width		Register Function
 */
static struct xhci_usbxx_set ssusb_sifslv_ippc[] = {
	{ 0x0000, "SSUSB_IP_PW_CTRL0",	32, "SSUSB IP Power and Clock Control Register 0" },
	{ 0x0004, "SSUSB_IP_PW_CTRL1",	32, "SSUSB IP Power and Clock Control Register 1" },
	{ 0x0008, "SSUSB_IP_PW_CTRL2",	32, "SSUSB IP Power and Clock Control Register 2" },
	{ 0x000c, "SSUSB_IP_PW_CTRL3",	32, "SSUSB IP Power and Clock Control Register 3" },
	{ 0x0010, "SSUSB_IP_PW_STS1",	32, "SSUSB IP Power and Clock Status Register 1" },
	{ 0x0014, "SSUSB_IP_PW_STS2",	32, "SSUSB IP Power and Clock Status Register 2" },
	{ 0x0018, "SSUSB_OTG_STS",	32, "SSUSB OTG STATUS" },
	{ 0x001c, "SSUSB_OTG_STS_CLR",	32, "SSUSB OTG STATUS CLEAR" },
	{ 0x0020, "SSUSB_IP_MAC_CAP",	32, "SSUSB IP MAC Capability Register" },
	{ 0x0024, "SSUSB_IP_XHCI_CAP",	32, "SSUSB IP xHCI Capability Register" },
	{ 0x0028, "SSUSB_IP_DEV_CAP",	32, "SSUSB IP Device Capability Register" },
	{ 0x002c, "SSUSB_OTG_INT_EN",	32, "SSUSB OTG INTERRUPT Enable" },
	{ 0x0030, "SSUSB_U3_CTRL_0P",	32, "SSUSB IP U3 Port 0 Control Register" },
	{ 0x0038, "SSUSB_U3_CTRL_1P",	32, "SSUSB IP U3 Port 1 Control Register" },
	{ 0x0040, "SSUSB_U3_CTRL_2P",	32, "SSUSB IP U3 Port 2 Control Register" },
	{ 0x0048, "SSUSB_U3_CTRL_3P",	32, "SSUSB IP U3 Port 3 Control Register" },
	{ 0x0050, "SSUSB_U2_CTRL_0P",	32, "SSUSB IP U2 Port 0 Control Register" },
	{ 0x0058, "SSUSB_U2_CTRL_1P",	32, "SSUSB IP U2 Port 1 Control Register" },
	{ 0x0060, "SSUSB_U2_CTRL_2P",	32, "SSUSB IP U2 Port 2 Control Register" },
	{ 0x0068, "SSUSB_U2_CTRL_3P",	32, "SSUSB IP U2 Port 3 Control Register" },
	{ 0x0070, "SSUSB_U2_CTRL_4P",	32, "SSUSB IP U2 Port 4 Control Register" },
	{ 0x0078, "SSUSB_U2_CTRL_5P",	32, "SSUSB IP U2 Port 5 Control Register" },
	{ 0x007c, "SSUSB_U2_PHY_PLL",	32, "SSUSB U2 PHY PLL Control Register" },
	{ 0x0080, "SSUSB_DMA_CTRL",	32, "SSUSB DMA Control Register" },
	{ 0x0084, "SSUSB_MAC_CK_CTRL",	32, "SSUSB MAC Clock Control Register" },
	{ 0x0088, "SSUSB_CSR_CK_CTRL",	32, "" },
	{ 0x008c, "SSUSB_REF_CK_CTRL",	32, "SSUSB Reference Clock Control Register" },
	{ 0x0090, "SSUSB_XHCI_CK_CTRL",	32, "SSUSB XHCI Clock Control Register" },
	{ 0x0094, "SSUSB_XHCI_RST_CTRL", 32, "SSUSB XHCI Reset Control Register" },
	{ 0x0098, "SSUSB_DEV_RST_CTRL",	32, "SSUSB Device Reset Control Register" },
	{ 0x009c, "SSUSB_SYS_CK_CTRL",	32, "SSUSB System Clock Control Register" },
	{ 0x00a0, "SSUSB_HW_ID",	32, "SSUSB HW ID" },
	{ 0x00a4, "SSUSB_HW_SUB_ID",	32, "SSUSB HW SUB ID" },
	{ 0x00b0, "SSUSB_PRB_CTRL0",	32, "Probe Control Register 0" },
	{ 0x00b4, "SSUSB_PRB_CTRL1",	32, "Probe Control Register 1" },
	{ 0x00b8, "SSUSB_PRB_CTRL2",	32, "Probe Control Register 2" },
	{ 0x00bc, "SSUSB_PRB_CTRL3",	32, "Probe Control Register 3" },
	{ 0x00c0, "SSUSB_PRB_CTRL4",	32, "Probe Control Register 4" },
	{ 0x00c4, "SSUSB_PRB_CTRL5",	32, "Probe Control Register 5" },
	{ 0x00c8, "SSUSB_IP_SPARE0",	32, "SSUSB IP Spare Register0" },
	{ 0x00cc, "SSUSB_IP_SPARE1",	32, "SSUSB IP Spare Register1" },
	{ 0x00f8, "SSUSB_IP_SLV_TMOUT",	32, "SSUSB IP SLAVE TIMEOUT" },
	{ 0, NULL, 0, NULL }
};


/*
 * Module name: ssusb2_xhci_exclude_port_csr Base address: (+11270000h)
 * Address		Name	Width		Register Function
 */
static struct xhci_usbxx_set ssusb2_xhci_exclude_port_csr[] = {
	{ 0x0000, "CAPLENGTH",	32, "Capability Register Length" },
	{ 0x0004, "HCSPARAMS1",	32, "Structural Parameters 1" },
	{ 0x0008, "HCSPARAMS2",	32, "Structural Parameters 2" },
	{ 0x000c, "HCSPARAMS3",	32, "Structural Parameters 3" },
	{ 0x0010, "HCCPARAMS",	32, "Capability Parameters" },
	{ 0x0014, "DBSOFF",	32, "Doorbell Offset" },
	{ 0x0018, "RTSOFF",	32, "Runtime Register Space Offset" },
	{ 0x0020, "USBCMD",	32, "USB Command" },
	{ 0x0024, "USBSTS",	32, "USB Status" },
	{ 0x0028, "PAGESIZE",	32, "Page Size" },
	{ 0x0034, "DNCTRL",	32, "Device Notification Control" },
	{ 0x0038, "CRCR1",	32, "Command Ring Control 1" },
	{ 0x003c, "CRCR2",	32, "Command Ring Control 2" },
	{ 0x0050, "DCBAAP_LO",	32, "Device Context Base Address Array Pointer Lo" },
	{ 0x0054, "DCBAAP_HI",	32, "Device Context Base Address Array Pointer High" },
	{ 0x0058, "CONFIG",	32, "Configure" },
	{ 0x0500, "SUPP_PTCL_REG1", 32, "xHCI Supported Protocol Capability 1 Register" },
	{ 0x0504, "SUPP_PTCL_REG2", 32, "xHCI Supported Protocol Capability 2 Register" },
	{ 0x0508, "SUPP_PTCL_REG3", 32, "xHCI Supported Protocol Capability 3 Register" },
	{ 0x0510, "SUPP_PTCL_REG4", 32, "xHCI Supported Protocol Capability 4 Register" },
	{ 0x0514, "SUPP_PTCL_REG5", 32, "xHCI Supported Protocol Capability 5 Register" },
	{ 0x0518, "SUPP_PTCL_REG6", 32, "xHCI Supported Protocol Capability 6 Register" },
	{ 0x0600, "MFINDEX",	32, "Microframe Index" },
	{ 0x0620, "IMAN",	32, "Interrupter Management" },
	{ 0x0624, "IMOD",	32, "Interrupter Moderation" },
	{ 0x0628, "ERSTSZ",	32, "Event Ring Segment Table Size" },
	{ 0x0630, "ERSTBA_LO",	32, "Event Ring Segment Table Base Address Lo" },
	{ 0x0634, "ERSTBA_HI",	32, "Event Ring Segment Table Base Address Hi" },
	{ 0x0638, "ERDP_LO",	32, "Event Ring Segment Table Base Address Lo" },
	{ 0x063c, "ERDP_HI",	32, "Event Ring Segment Table Base Address Hi" },
	{ 0x0800, "HOST_CMD_DB", 32, "Host Controller Doorbell Registers" },
	{ 0x0804, "DEVICE1_DB",	32, "Device 1 Doorbell Registers" },
	{ 0x0808, "DEVICE2_DB",	32, "Device 2 Doorbell Registers" },
	{ 0x080c, "DEVICE3_DB",	32, "Device 3 Doorbell Registers" },
	{ 0x0810, "DEVICE4_DB",	32, "Device 4 Doorbell Registers" },
	{ 0x0814, "DEVICE5_DB",	32, "Device 5 Doorbell Registers" },
	{ 0x0818, "DEVICE6_DB",	32, "Device 6 Doorbell Registers" },
	{ 0x081c, "DEVICE7_DB",	32, "Device 7 Doorbell Registers" },
	{ 0x0820, "DEVICE8_DB",	32, "Device 8 Doorbell Registers" },
	{ 0x0824, "DEVICE9_DB",	32, "Device 9 Doorbell Registers" },
	{ 0x0828, "DEVICE10_DB", 32, "Device 10 Doorbell Registers" },
	{ 0x082c, "DEVICE11_DB", 32, "Device 11 Doorbell Registers" },
	{ 0x0830, "DEVICE12_DB", 32, "Device 12 Doorbell Registers" },
	{ 0x0834, "DEVICE13_DB", 32, "Device 13 Doorbell Registers" },
	{ 0x0838, "DEVICE14_DB", 32, "Device 14 Doorbell Registers" },
	{ 0x083c, "DEVICE15_DB", 32, "Device 15 Doorbell Registers" },
	{ 0x0900, "HSRAM_DBGCTL", 32, "Host SRAM Debug Control Register" },
	{ 0x0904, "HSRAM_DBGMODE", 32, "Host SRAM Debug Mode Register" },
	{ 0x0908, "HSRAM_DBGSEL", 32, "Host SRAM Debug Select Register" },
	{ 0x090c, "HSRAM_DBGADR", 32, "Host SRAM Debug Address Register" },
	{ 0x0910, "HSRAM_DBGDR", 32, "Host SRAM Debug Data Register" },
	{ 0x0920, "HSRAM_DELSEL_0", 32, "Host SRAM  Delay Select 0" },
	{ 0x0924, "HSRAM_DELSEL_1", 32, "Host SRAM  Delay Select 1" },
	{ 0x0930, "LS_EOF",	32, "Low Speed EOF Start Offset" },
	{ 0x0934, "FS_EOF",	32, "Full Speed EOF Start Offset" },
	{ 0x0938, "SYNC_HS_EOF", 32, "Synchronous High Speed EOF Start Offset" },
	{ 0x093c, "SS_EOF",	32, "Super Speed EOF Start Offset" },
	{ 0x0940, "SOF_OFFSET",	32, "SOF Offset" },
	{ 0x0944, "HFCNTR_CFG",	32, "Host Frame Counter Configuration" },
	{ 0x0948, "XACT3_CFG",	32, "Super Speed Transaction Configuration" },
	{ 0x094c, "XACT2_CFG",	32, "USB2 Transaction Configuration" },
	{ 0x0950, "HDMA_CFG",	32, "Host DMA Configuration" },
	{ 0x0954, "ASYNC_HS_EOF", 32, "Asynchronous High Speed EOF Start Offset" },
	{ 0x0958, "AXI_WR_DMA_CFG", 32, "AXI WR DMA configuration register." },
	{ 0x095c, "AXI_RD_DMA_CFG", 32, "AXI RD DMA configuration register." },
	{ 0x0960, "HSCH_CFG1",	32, "Host Scheduler Configuration Register 1" },
	{ 0x0964, "CMD_CFG",	32, "Command Configuration" },
	{ 0x0968, "EP_CFG",	32, "Endpoint Status Configuration" },
	{ 0x096c, "EVT_CFG",	32, "Event Configuration" },
	{ 0x0970, "TRBQ_CFG",	32, "TRBQ Configuration" },
	{ 0x0974, "U3PORT_CFG",	32, "USB3 Port Configuration" },
	{ 0x0978, "U2PORT_CFG",	32, "USB2 Port Configuration" },
	{ 0x097c, "HSCH_CFG2",	32, "Host Scheduler Configuration Register 2" },
	{ 0x0980, "SW_ERDY",	32, "Software ERDY" },
	{ 0x09a0, "SLOT_EP_STS0", 32, "Slot and EP Resource Status0" },
	{ 0x09a4, "SLOT_EP_STS1", 32, "Slot and EP Resource Status1" },
	{ 0x09a8, "SLOT_EP_STS2", 32, "Slot and EP Resource Status2" },
	{ 0x09b0, "RST_CTRL0",	32, "Host reset control Register 2" },
	{ 0x09b4, "RST_CTRL1",	32, "Host reset control Register 3" },
	{ 0x09f0, "SPARE0",	32, "Spare Register 0" },
	{ 0x09f4, "SPARE1",	32, "Spare Register 1" },
	{ 0, NULL, 0, NULL }
};
/*
 * Module name: ssusb2_xhci_u2_port_csr Base address: (+11270420h)
 * Address		Name	Width		Register Function
 */
static struct xhci_usbxx_set ssusb2_xhci_u2_port_csr[] = {
	{ 0x0420, "USB2_PORT_SC",	32, "USB2_PORT Port Status and Control" },
	{ 0x0424, "USB2_PORT_PMSC",	32, "USB2_PORT PM Status and Control" },
	{ 0x0428, "USB2_PORT_LI",	32, "USB2_PORT Link Info" },
	{ 0x042c, "USB2_PORT_HLPMC", 32, "USB2_PORT Hardware LPM Control Register" },
	{ 0, NULL, 0, NULL }
};

/*
 * Module name: ssusb2_xhci_u2_port_csr_1p Base address: (+11270430h)
 * Address	Name	Width		Register Function
 */
static struct xhci_usbxx_set ssusb2_xhci_u2_port_csr_1p[] = {
	{ 0x0430, "USB2_PORT_SC",	32, "USB2_PORT Port Status and Control" },
	{ 0x0434, "USB2_PORT_PMSC",	32, "USB2_PORT PM Status and Control" },
	{ 0x0438, "USB2_PORT_LI",	32, "USB2_PORT Link Info" },
	{ 0x043c, "USB2_PORT_HLPMC", 32, "USB2_PORT Hardware LPM Control Register" },
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
		pr_err("[xhci debug][Register RMW] Invalid Register field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		value &= (1 << len) - 1;
		pr_err("[xhci debug][Register RMW]Original:0x%p (0x%x)\n", address, readl(address));
		IO_SET_FIELD(address, field, value);
		pr_err("[xhci debug][Register RMW]Modified:0x%p (0x%x)\n", address, readl(address));
	}
}

static void register_get_field(void __iomem *address, unsigned int start_bit,
						unsigned int len, unsigned int value)
{
	unsigned long field;

	if (start_bit > 31 || start_bit < 0 || len > 31 || len <= 0
							|| (start_bit + len > 31))
		pr_err("[xhci debug][Register RMW]Invalid reg field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		IO_GET_FIELD(address, field, value);
		pr_err("[xhci debug][Register RMW]Reg:0x%p start_bit(%d)len(%d)(0x%x)\n",
								address, start_bit, len, value);
	}
}

static int mtk_xhci_regdump_show(struct seq_file *s, void *unused)
{
	int i;
	struct xhci_usbxx_set *usbxxreg;
	struct xhci_hcd *xhci = s->private;
	struct usb_hcd *hcd = xhci_to_hcd(xhci);
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);
	void __iomem *iobase = xhci->main_hcd->regs;
	void __iomem *ibase = mtk->ippc_regs;

	seq_printf(s, "\nMTK xHCI:(version:0x%X) (ippc_base: 0x%p, mac_base: 0x%p)\n",
				HC_VERSION(readl(&xhci->cap_regs->hc_capbase)), ibase, iobase);

	if (PTR_ERR(ibase)) {
		seq_puts(s, "ssusb_sifslv_ippc Register Dump range [0x11280700 ~ 0x11280800)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb_sifslv_ippc); i++) {
			usbxxreg = &ssusb_sifslv_ippc[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-15s(0x%p): %02x\n", usbxxreg->name, ibase + usbxxreg->offset,
									readb(ibase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-15s(0x%p): %04x\n", usbxxreg->name, ibase + usbxxreg->offset,
									readw(ibase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-15s(0x%p): %08x\n", usbxxreg->name, ibase + usbxxreg->offset,
									readl(ibase + usbxxreg->offset));
				break;
			}
		}
	}

	if (PTR_ERR(iobase)) {
		seq_puts(s, "\nssusb2_xhci_exclude_port_csr Register Dump range [0x11270000 ~ 0x11271000)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb2_xhci_exclude_port_csr); i++) {
			usbxxreg = &ssusb2_xhci_exclude_port_csr[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-15s(0x%p): 0x%02X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readb(iobase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-15s(0x%p): 0x%04X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readw(iobase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-15s(0x%p): 0x%08X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readl(iobase + usbxxreg->offset));
				break;
			}
		}

		seq_puts(s, "\nssusb2_xhci_u2_port_csr Register Dump range [0x11270420 ~ 0x11270430)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb2_xhci_u2_port_csr); i++) {
			usbxxreg = &ssusb2_xhci_u2_port_csr[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-15s(0x%p): 0x%02X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readb(iobase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-15s(0x%p): 0x%04X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readw(iobase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-15s(0x%p): 0x%08X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readl(iobase + usbxxreg->offset));
				break;
			}
		}

		seq_puts(s, "\nssusb2_xhci_u2_port_csr_1p Register Dump range [0x11270430 ~ 0x11270440)\n");
		for (i = 0; i < ARRAY_SIZE(ssusb2_xhci_u2_port_csr_1p); i++) {
			usbxxreg = &ssusb2_xhci_u2_port_csr_1p[i];
			switch (usbxxreg->width) {
			case 8:
				seq_printf(s, "%-15s(0x%p): 0x%02X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readb(iobase + usbxxreg->offset));
				break;
			case 16:
				seq_printf(s, "%-15s(0x%p): 0x%04X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readw(iobase + usbxxreg->offset));
				break;
			case 32:
				seq_printf(s, "%-15s(0x%p): 0x%08X\n", usbxxreg->name, iobase + usbxxreg->offset,
									readl(iobase + usbxxreg->offset));
				break;
			}
		}
	}

	return 0;
}


static int mtk_xhci_regdump(struct inode *inode, struct file *file)
{
	return single_open(file, mtk_xhci_regdump_show, inode->i_private);
}

static const struct file_operations mtk_xhci_regdump_fops = {
	.open	= mtk_xhci_regdump,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int mtk_xhci_debug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "\n===mtk_xhci_debug help===\n");
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

static ssize_t mtk_xhci_debug_proc_write(struct file *file, const char *buf, size_t count, loff_t *data)
{
	int ret;
	int cmd, p1, p3, p4, p5;
	unsigned int reg_value;
	int sscanf_num;
	struct seq_file	*s = file->private_data;
	struct xhci_hcd *xhci = s->private;
	struct usb_hcd *hcd = xhci_to_hcd(xhci);
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);
	void __iomem *iomem = NULL;

	unsigned int long long p2;

	p1 = p2 = p3 = p4 = p5 = -1;

	if (count == 0)
		return -1;

	if (count > 255)
		count = 255;

	ret = copy_from_user(mtk->cmd_buf, buf, count);
	if (ret < 0)
		return -1;

	mtk->cmd_buf[count] = '\0';
	pr_err("[xhci debug]debug received:%s\n", mtk->cmd_buf);

	sscanf_num = sscanf(mtk->cmd_buf, "%x %x %llx %x %x %x", &cmd, &p1, &p2, &p3, &p4, &p5);
	if (sscanf_num < 1)
		return count;

	if (cmd == 0)
		pr_err("[xhci debug] zone <0x%.8x> yet\n", p1);
	else if (cmd == 1) {
		iomem += p2;
		if (p1 == 0) {
			reg_value = p3;
			pr_err("[xhci debug][Register Write]Original:0x%p (0x%08X)\n",
								iomem, readl(iomem));

			writel(reg_value, iomem);

			pr_err("[xhci debug][Register Write]Writed:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		} else if (p1 == 1)
			pr_err("[xhci debug][Register Read]Register:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		else if (p1 == 2)
			register_set_field(iomem, p3, p4, p5);
		else if (p1 == 3)
			register_get_field(iomem, p3, p4, p5);
		else
			pr_err("[xhci debug] todo\n");
	}

	return count;
}

static int mtk_xhci_debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mtk_xhci_debug_proc_show, inode->i_private);
}

static const struct file_operations mtk_xhci_debug_proc_fops = {
	.open   = mtk_xhci_debug_proc_open,
	.write  = mtk_xhci_debug_proc_write,
	.read   = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int mtk_xhci_init_debugfs(struct xhci_hcd *xhci)
{
	int	ret;
	struct dentry *file;
	struct dentry *root;
	struct usb_hcd *hcd = xhci_to_hcd(xhci);
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);

	root = debugfs_create_dir(dev_driver_string(xhci_to_hcd(xhci)->self.controller), NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	file = debugfs_create_file("regdump", S_IRUGO, root, xhci, &mtk_xhci_regdump_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	file = debugfs_create_file("debug", S_IRUGO | S_IWUSR, root, xhci, &mtk_xhci_debug_proc_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	mtk->debugfs_root = root;

	return 0;

err1:
	debugfs_remove_recursive(root);

err0:
	 return ret;
}

void mtk_xhci_exit_debugfs(struct xhci_hcd *xhci)
{
	struct usb_hcd *hcd = xhci_to_hcd(xhci);
	struct xhci_hcd_mtk *mtk = hcd_to_mtk(hcd);

	debugfs_remove_recursive(mtk->debugfs_root);
}
