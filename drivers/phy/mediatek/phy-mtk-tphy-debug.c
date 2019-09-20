/*
 * Copyright (c) 2015 MediaTek Inc.
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

#include <dt-bindings/phy/phy.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

/* version V1 sub-banks offset base address */
/* banks shared by multiple phys */
#define SSUSB_SIFSLV_V1_SPLLC		0x000	/* shared by u3 phys */
#define SSUSB_SIFSLV_V1_U2FREQ		0x100	/* shared by u2 phys */
#define SSUSB_SIFSLV_V1_CHIP		0x300	/* shared by u3 phys */
/* u2 phy bank */
#define SSUSB_SIFSLV_V1_U2PHY_COM	0x000
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V1_U3PHYD		0x000
#define SSUSB_SIFSLV_V1_U3PHYA		0x200

/* version V2 sub-banks offset base address */
/* u2 phy banks */
#define SSUSB_SIFSLV_V2_MISC		0x000
#define SSUSB_SIFSLV_V2_U2FREQ		0x100
#define SSUSB_SIFSLV_V2_U2PHY_COM	0x300
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V2_SPLLC		0x000
#define SSUSB_SIFSLV_V2_CHIP		0x100
#define SSUSB_SIFSLV_V2_U3PHYD		0x200
#define SSUSB_SIFSLV_V2_U3PHYA		0x400

#define U3P_USBPHYACR0		0x000
#define PA0_RG_U2PLL_FORCE_ON		BIT(15)
#define PA0_RG_USB20_INTR_EN		BIT(5)

#define U3P_USBPHYACR1			0x004
#define PA1_RG_USB20_INTR_CAL		GENMASK(23, 19)

#define U3P_USBPHYACR2		0x008
#define PA2_RG_SIF_U2PLL_FORCE_EN	BIT(18)

#define U3P_USBPHYACR5		0x014
#define PA5_RG_U2_HSTX_SRCAL_EN	BIT(15)
#define PA5_RG_U2_HSTX_SRCTRL		GENMASK(14, 12)
#define PA5_RG_U2_HSTX_SRCTRL_VAL(x)	((0x7 & (x)) << 12)
#define PA5_RG_U2_HS_100U_U3_EN	BIT(11)

#define U3P_USBPHYACR6		0x018
#define PA6_RG_U2_BC11_SW_EN		BIT(23)
#define PA6_RG_U2_OTG_VBUSCMP_EN	BIT(20)
#define PA6_RG_U2_SQTH		GENMASK(3, 0)
#define PA6_RG_U2_SQTH_VAL(x)	(0xf & (x))

#define U3P_U2PHYACR4		0x020
#define P2C_RG_USB20_GPIO_CTL		BIT(9)
#define P2C_USB20_GPIO_MODE		BIT(8)
#define P2C_U2_GPIO_CTR_MSK	(P2C_RG_USB20_GPIO_CTL | P2C_USB20_GPIO_MODE)

#define U3D_U2PHYDCR0		0x060
#define P2C_RG_SIF_U2PLL_FORCE_ON	BIT(24)

#define U3P_U2PHYDTM0		0x068
#define P2C_FORCE_UART_EN		BIT(26)
#define P2C_FORCE_DATAIN		BIT(23)
#define P2C_FORCE_DM_PULLDOWN		BIT(21)
#define P2C_FORCE_DP_PULLDOWN		BIT(20)
#define P2C_FORCE_XCVRSEL		BIT(19)
#define P2C_FORCE_SUSPENDM		BIT(18)
#define P2C_FORCE_TERMSEL		BIT(17)
#define P2C_RG_DATAIN			GENMASK(13, 10)
#define P2C_RG_DATAIN_VAL(x)		((0xf & (x)) << 10)
#define P2C_RG_DMPULLDOWN		BIT(7)
#define P2C_RG_DPPULLDOWN		BIT(6)
#define P2C_RG_XCVRSEL			GENMASK(5, 4)
#define P2C_RG_XCVRSEL_VAL(x)		((0x3 & (x)) << 4)
#define P2C_RG_SUSPENDM			BIT(3)
#define P2C_RG_TERMSEL			BIT(2)
#define P2C_DTM0_PART_MASK \
		(P2C_FORCE_DATAIN | P2C_FORCE_DM_PULLDOWN | \
		P2C_FORCE_DP_PULLDOWN | P2C_FORCE_XCVRSEL | \
		P2C_FORCE_TERMSEL | P2C_RG_DMPULLDOWN | \
		P2C_RG_DPPULLDOWN | P2C_RG_TERMSEL)

#define U3P_U2PHYDTM1		0x06C
#define P2C_RG_UART_EN			BIT(16)
#define P2C_RG_VBUSVALID		BIT(5)
#define P2C_RG_SESSEND			BIT(4)
#define P2C_RG_AVALID			BIT(2)

#define U3P_U3_CHIP_GPIO_CTLD		0x0c
#define P3C_REG_IP_SW_RST		BIT(31)
#define P3C_MCU_BUS_CK_GATE_EN		BIT(30)
#define P3C_FORCE_IP_SW_RST		BIT(29)

#define U3P_U3_CHIP_GPIO_CTLE		0x10
#define P3C_RG_SWRST_U3_PHYD		BIT(25)
#define P3C_RG_SWRST_U3_PHYD_FORCE_EN	BIT(24)

#define U3P_U3_PHYA_REG0	0x000
#define P3A_RG_CLKDRV_OFF		GENMASK(3, 2)
#define P3A_RG_CLKDRV_OFF_VAL(x)	((0x3 & (x)) << 2)

#define U3P_U3_PHYA_REG1	0x004
#define P3A_RG_CLKDRV_AMP		GENMASK(31, 29)
#define P3A_RG_CLKDRV_AMP_VAL(x)	((0x7 & (x)) << 29)

#define U3P_U3_PHYA_REG6	0x018
#define P3A_RG_TX_EIDLE_CM		GENMASK(31, 28)
#define P3A_RG_TX_EIDLE_CM_VAL(x)	((0xf & (x)) << 28)

#define U3P_U3_PHYA_REG9	0x024
#define P3A_RG_RX_DAC_MUX		GENMASK(5, 1)
#define P3A_RG_RX_DAC_MUX_VAL(x)	((0x1f & (x)) << 1)

#define U3P_U3_PHYA_DA_REG0	0x100
#define P3A_RG_XTAL_EXT_PE2H		GENMASK(17, 16)
#define P3A_RG_XTAL_EXT_PE2H_VAL(x)	((0x3 & (x)) << 16)
#define P3A_RG_XTAL_EXT_PE1H		GENMASK(13, 12)
#define P3A_RG_XTAL_EXT_PE1H_VAL(x)	((0x3 & (x)) << 12)
#define P3A_RG_XTAL_EXT_EN_U3		GENMASK(11, 10)
#define P3A_RG_XTAL_EXT_EN_U3_VAL(x)	((0x3 & (x)) << 10)

#define U3P_U3_PHYA_DA_REG4	0x108
#define P3A_RG_PLL_DIVEN_PE2H		GENMASK(21, 19)
#define P3A_RG_PLL_BC_PE2H		GENMASK(7, 6)
#define P3A_RG_PLL_BC_PE2H_VAL(x)	((0x3 & (x)) << 6)

#define U3P_U3_PHYA_DA_REG5	0x10c
#define P3A_RG_PLL_BR_PE2H		GENMASK(29, 28)
#define P3A_RG_PLL_BR_PE2H_VAL(x)	((0x3 & (x)) << 28)
#define P3A_RG_PLL_IC_PE2H		GENMASK(15, 12)
#define P3A_RG_PLL_IC_PE2H_VAL(x)	((0xf & (x)) << 12)

#define U3P_U3_PHYA_DA_REG6	0x110
#define P3A_RG_PLL_IR_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_IR_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG7	0x114
#define P3A_RG_PLL_BP_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_BP_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG20	0x13c
#define P3A_RG_PLL_DELTA1_PE2H		GENMASK(31, 16)
#define P3A_RG_PLL_DELTA1_PE2H_VAL(x)	((0xffff & (x)) << 16)

#define U3P_U3_PHYA_DA_REG25	0x148
#define P3A_RG_PLL_DELTA_PE2H		GENMASK(15, 0)
#define P3A_RG_PLL_DELTA_PE2H_VAL(x)	(0xffff & (x))

#define U3P_U3_PHYD_LFPS1		0x00c
#define P3D_RG_FWAKE_TH		GENMASK(21, 16)
#define P3D_RG_FWAKE_TH_VAL(x)	((0x3f & (x)) << 16)

#define U3P_U3_PHYD_CDR1		0x05c
#define P3D_RG_CDR_BIR_LTD1		GENMASK(28, 24)
#define P3D_RG_CDR_BIR_LTD1_VAL(x)	((0x1f & (x)) << 24)
#define P3D_RG_CDR_BIR_LTD0		GENMASK(12, 8)
#define P3D_RG_CDR_BIR_LTD0_VAL(x)	((0x1f & (x)) << 8)

#define U3P_U3_PHYD_RXDET1		0x128
#define P3D_RG_RXDET_STB2_SET		GENMASK(17, 9)
#define P3D_RG_RXDET_STB2_SET_VAL(x)	((0x1ff & (x)) << 9)

#define U3P_U3_PHYD_RXDET2		0x12c
#define P3D_RG_RXDET_STB2_SET_P3	GENMASK(8, 0)
#define P3D_RG_RXDET_STB2_SET_P3_VAL(x)	(0x1ff & (x))

#define U3P_SPLLC_XTALCTL3		0x018
#define XC3_RG_U3_XTAL_RX_PWD		BIT(9)
#define XC3_RG_U3_FRC_XTAL_RX_PWD	BIT(8)

#define U3P_U2FREQ_FMCR0	0x00
#define P2F_RG_MONCLK_SEL	GENMASK(27, 26)
#define P2F_RG_MONCLK_SEL_VAL(x)	((0x3 & (x)) << 26)
#define P2F_RG_FREQDET_EN	BIT(24)
#define P2F_RG_CYCLECNT		GENMASK(23, 0)
#define P2F_RG_CYCLECNT_VAL(x)	((P2F_RG_CYCLECNT) & (x))

#define U3P_U2FREQ_VALUE	0x0c

#define U3P_U2FREQ_FMMONR1	0x10
#define P2F_USB_FM_VALID	BIT(0)
#define P2F_RG_FRCK_EN		BIT(8)

#define U3P_REF_CLK		48	/* MHZ */
#define U3P_SLEW_RATE_COEF	18
#define U3P_SR_COEF_DIVISOR	1000
#define U3P_FM_DET_CYCLE_CNT	1024

/* SATA register setting */
#define PHYD_CTRL_SIGNAL_MODE4		0x1c
/* CDR Charge Pump P-path current adjustment */
#define RG_CDR_BICLTD1_GEN1_MSK		GENMASK(23, 20)
#define RG_CDR_BICLTD1_GEN1_VAL(x)	((0xf & (x)) << 20)
#define RG_CDR_BICLTD0_GEN1_MSK		GENMASK(11, 8)
#define RG_CDR_BICLTD0_GEN1_VAL(x)	((0xf & (x)) << 8)

#define PHYD_DESIGN_OPTION2		0x24
/* Symbol lock count selection */
#define RG_LOCK_CNT_SEL_MSK		GENMASK(5, 4)
#define RG_LOCK_CNT_SEL_VAL(x)		((0x3 & (x)) << 4)

#define PHYD_DESIGN_OPTION9	0x40
/* COMWAK GAP width window */
#define RG_TG_MAX_MSK		GENMASK(20, 16)
#define RG_TG_MAX_VAL(x)	((0x1f & (x)) << 16)
/* COMINIT GAP width window */
#define RG_T2_MAX_MSK		GENMASK(13, 8)
#define RG_T2_MAX_VAL(x)	((0x3f & (x)) << 8)
/* COMWAK GAP width window */
#define RG_TG_MIN_MSK		GENMASK(7, 5)
#define RG_TG_MIN_VAL(x)	((0x7 & (x)) << 5)
/* COMINIT GAP width window */
#define RG_T2_MIN_MSK		GENMASK(4, 0)
#define RG_T2_MIN_VAL(x)	(0x1f & (x))

#define ANA_RG_CTRL_SIGNAL1		0x4c
/* TX driver tail current control for 0dB de-empahsis mdoe for Gen1 speed */
#define RG_IDRV_0DB_GEN1_MSK		GENMASK(13, 8)
#define RG_IDRV_0DB_GEN1_VAL(x)		((0x3f & (x)) << 8)

#define ANA_RG_CTRL_SIGNAL4		0x58
#define RG_CDR_BICLTR_GEN1_MSK		GENMASK(23, 20)
#define RG_CDR_BICLTR_GEN1_VAL(x)	((0xf & (x)) << 20)
/* Loop filter R1 resistance adjustment for Gen1 speed */
#define RG_CDR_BR_GEN2_MSK		GENMASK(10, 8)
#define RG_CDR_BR_GEN2_VAL(x)		((0x7 & (x)) << 8)

#define ANA_RG_CTRL_SIGNAL6		0x60
/* I-path capacitance adjustment for Gen1 */
#define RG_CDR_BC_GEN1_MSK		GENMASK(28, 24)
#define RG_CDR_BC_GEN1_VAL(x)		((0x1f & (x)) << 24)
#define RG_CDR_BIRLTR_GEN1_MSK		GENMASK(4, 0)
#define RG_CDR_BIRLTR_GEN1_VAL(x)	(0x1f & (x))

#define ANA_EQ_EYE_CTRL_SIGNAL1		0x6c
/* RX Gen1 LEQ tuning step */
#define RG_EQ_DLEQ_LFI_GEN1_MSK		GENMASK(11, 8)
#define RG_EQ_DLEQ_LFI_GEN1_VAL(x)	((0xf & (x)) << 8)

#define ANA_EQ_EYE_CTRL_SIGNAL4		0xd8
#define RG_CDR_BIRLTD0_GEN1_MSK		GENMASK(20, 16)
#define RG_CDR_BIRLTD0_GEN1_VAL(x)	((0x1f & (x)) << 16)

#define ANA_EQ_EYE_CTRL_SIGNAL5		0xdc
#define RG_CDR_BIRLTD0_GEN3_MSK		GENMASK(4, 0)
#define RG_CDR_BIRLTD0_GEN3_VAL(x)	(0x1f & (x))

enum mtk_phy_version {
	MTK_PHY_V1 = 1,
	MTK_PHY_V2,
};

struct mtk_phy_pdata {
	/* avoid RX sensitivity level degradation only for mt8173 */
	bool avoid_rx_sen_degradation;
	enum mtk_phy_version version;
};

struct u2phy_banks {
	void __iomem *misc;
	void __iomem *fmreg;
	void __iomem *com;
};

struct u3phy_banks {
	void __iomem *spllc;
	void __iomem *chip;
	void __iomem *phyd; /* include u3phyd_bank2 */
	void __iomem *phya; /* include u3phya_da */
};

struct mtk_phy_instance {
	struct phy *phy;
	void __iomem *port_base;
	union {
		struct u2phy_banks u2_banks;
		struct u3phy_banks u3_banks;
	};
	struct clk *ref_clk;	/* reference clock of anolog phy */
	u32 index;
	u8 type;
	void *privatedata;
};

struct mtk_tphy {
	struct device *dev;
	void __iomem *sif_base;	/* only shared sif */
	/* deprecated, use @ref_clk instead in phy instance */
	struct clk *u3phya_ref;	/* reference clock of usb3 anolog phy */
	const struct mtk_phy_pdata *pdata;
	struct mtk_phy_instance **phys;
	int nphys;

	char cmd_buf[1024];
	struct dentry *debugfs_root;
};

struct tphy_usbreg_set {
	unsigned int offset;
	const char  *name;
	unsigned int width;
	const char *comment;
};

static struct tphy_usbreg_set mt8695_usb20_phys[] = {
	{ 0x0000, "USBPHYACR0",	32, ""},
	{ 0x0004, "USBPHYACR1",	32, ""},
	{ 0x0008, "USBPHYACR2",	32, ""},
	{ 0x0010, "USBPHYACR4",	32, ""},
	{ 0x0014, "USBPHYACR5",	32, ""},
	{ 0x0018, "USBPHYACR6",	32, ""},
	{ 0x001C, "U2PHYACR3",	32, "USB20 PHYA Control 3 Register"},
	{ 0x0020, "U2PHYACR4",	32, "USB20 PHYA Control 4 Register"},
	{ 0x0024, "U2PHYAMON0",	32, "USB20 PHYA Monitor 0 Register"},
	{ 0x0060, "U2PHYDCR0",	32, "USB20 PHYD Control 0 Register"},
	{ 0x0064, "U2PHYDCR1",	32, "USB20 PHYD Control 1 Register"},
	{ 0x0068, "U2PHYDTM0",	32, "USB20 PHYD Control UTMI 0 Register"},
	{ 0x006C, "U2PHYDTM1",	32, "USB20 PHYD Control UTMI 1 Register"},
	{ 0x0070, "U2PHYDMON0",	32, "USB20 PHYD Monitor 0 Register"},
	{ 0x0074, "U2PHYDMON1",	32, "USB20 PHYD Monitor 1 Register"},
	{ 0x0078, "U2PHYDMON2",	32, "USB20 PHYD Monitor 2 Register"},
	{ 0x007C, "U2PHYDMON3",	32, "USB20 PHYD Monitor 3 Register"},
	{ 0x0080, "U2PHYBC12C",	32, "USB20 PHYD BC12 Control Register"},
	{ 0x0084, "U2PHYBC12C1", 32, "USB20 PHYD BC12 Control Register 1"},
	{ 0x00E0, "REGFPPC",	32, "USB Feature Register"},
	{ 0x00F0, "VERSIONC",	32, "USB Regfile version Register"},
	{ 0x00FC, "REGFCOM",	32, "USB Common Register"},
	{ 0, NULL, 0, NULL}
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
		pr_err("[tphy debug][Register RMW] Invalid Register field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		value &= (1 << len) - 1;
		pr_err("[tphy debug][Register RMW]Original:0x%p (0x%x)\n", address, readl(address));
		IO_SET_FIELD(address, field, value);
		pr_err("[tphy debug][Register RMW]Modified:0x%p (0x%x)\n", address, readl(address));
	}
}

static void register_get_field(void __iomem *address, unsigned int start_bit,
						unsigned int len, unsigned int value)
{
	unsigned long field;

	if (start_bit > 31 || start_bit < 0 || len > 31 || len <= 0
							|| (start_bit + len > 31))
		pr_err("[tphy debug][Register RMW]Invalid reg field range or length\n");
	else {
		field = ((1 << len) - 1) << start_bit;
		IO_GET_FIELD(address, field, value);
		pr_err("[tphy debug][Register RMW]Reg:0x%p start_bit(%d)len(%d)(0x%x)\n",
						address, start_bit, len, value);
	}
}

static void tphy_print_phy_regs(struct seq_file *s, int index)
{
	int i;
	unsigned int temp;
	struct mtk_tphy *tphy = s->private;
	struct mtk_phy_instance *instance = tphy->phys[index];
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct tphy_usbreg_set *usbxxreg;
	void __iomem *misc;
	void __iomem *fmreg;
	void __iomem *com;

	if (instance->type == PHY_TYPE_USB2) {
		com   = u2_banks->com;
		fmreg = u2_banks->fmreg;
		misc  = u2_banks->misc;
		seq_printf(s, "\nMTK USB2.0 PHY%i:MTK_PHY_%s(com: 0x%p, fmreg: 0x%p, misc: 0x%p)\n",
			index, (tphy->pdata->version == MTK_PHY_V1) ? "V1" : "V2", com, fmreg, misc);

		if (PTR_ERR(com)) {
			temp = readl(com + U3P_USBPHYACR0);
			seq_printf(s, "  0x%p: U3P_USBPHYACR0	= 0x%08X\n",
					com + U3P_USBPHYACR0, (unsigned int) temp);
			temp = readl(com + U3P_USBPHYACR1);
			seq_printf(s, "  0x%p: U3P_USBPHYACR1	= 0x%08X\n",
					com + U3P_USBPHYACR1, (unsigned int) temp);
			temp = readl(com + U3P_USBPHYACR2);
			seq_printf(s, "  0x%p: U3P_USBPHYACR2	= 0x%08X\n",
					com + U3P_USBPHYACR2, (unsigned int) temp);
			temp = readl(com + U3P_USBPHYACR5);
			seq_printf(s, "  0x%p: U3P_USBPHYACR5	= 0x%08X\n",
					com + U3P_USBPHYACR5, (unsigned int) temp);
			temp = readl(com + U3P_USBPHYACR6);
			seq_printf(s, "  0x%p: U3P_USBPHYACR6	= 0x%08X\n",
					com + U3P_USBPHYACR6, (unsigned int) temp);
			temp = readl(com + U3P_U2PHYACR4);
			seq_printf(s, "  0x%p: U3P_U2PHYACR4	= 0x%08X\n",
					com + U3P_U2PHYACR4, (unsigned int) temp);
			temp = readl(com + U3D_U2PHYDCR0);
			seq_printf(s, "  0x%p: U3D_U2PHYDCR0	= 0x%08X\n",
					com + U3D_U2PHYDCR0, (unsigned int) temp);
			temp = readl(com + U3P_U2PHYDTM0);
			seq_printf(s, "  0x%p: U3P_U2PHYDTM0	= 0x%08X\n",
					com + U3P_U2PHYDTM0, (unsigned int) temp);
			temp = readl(com + U3P_U2PHYDTM1);
			seq_printf(s, "  0x%p: U3P_U2PHYDTM1	= 0x%08X\n",
					com + U3P_U2PHYDTM1, (unsigned int) temp);
			temp = readl(com + U3P_U3_PHYA_REG0);
			seq_printf(s, "  0x%p: U3P_U3_PHYA_REG0 = 0x%08X\n",
					com + U3P_U3_PHYA_REG0, (unsigned int) temp);
			temp = readl(com + U3P_U3_PHYA_REG6);
			seq_printf(s, "  0x%p: U3P_U3_PHYA_REG6 = 0x%08X\n",
					com + U3P_U3_PHYA_REG6, (unsigned int) temp);
			temp = readl(com + U3P_U3_PHYA_REG9);
			seq_printf(s, "  0x%p: U3P_U3_PHYA_REG9 = 0x%08X\n",
					com + U3P_U3_PHYA_REG9, (unsigned int) temp);
		}

		if (PTR_ERR(fmreg)) {
			seq_printf(s, "\nMTK USB3.0 PHY%i(fmreg_base) registers at 0x%p:\n", index, fmreg);
			temp = readl(fmreg + U3P_U2FREQ_FMCR0);
			seq_printf(s, "  0x%p: U3P_U2FREQ_FMCR0 = 0x%08X\n",
					fmreg + U3P_U2FREQ_FMCR0, (unsigned int) temp);
			temp = readl(fmreg + U3P_U2FREQ_VALUE);
			seq_printf(s, "  0x%p: U3P_U2FREQ_VALUE	= 0x%08X\n",
					fmreg + U3P_U2FREQ_VALUE, (unsigned int) temp);
			temp = readl(fmreg + U3P_U2FREQ_FMMONR1);
			seq_printf(s, " 0x%p: U3P_U2FREQ_FMMONR1= 0x%08X\n",
					fmreg + U3P_U2FREQ_FMMONR1, (unsigned int) temp);
		}

		if (PTR_ERR(com)) {
			seq_printf(s, "\nMTK USB2.0 Port%i PHY20 phyicial Register Dump range %s\n", index,
				index ? "[0x11500300 ~ 0x11500400)" : "[0x11501300 ~ 0x11501400)");
			for (i = 0; i < ARRAY_SIZE(mt8695_usb20_phys); i++) {
				usbxxreg = &mt8695_usb20_phys[i];
				switch (usbxxreg->width) {
				case 8:
					seq_printf(s, "%-12s(0x%p): 0x%02X\n",
						usbxxreg->name, com + usbxxreg->offset,
						readb(com + usbxxreg->offset));
					break;
				case 16:
					seq_printf(s, "%-12s(0x%p): 0x%04X\n",
						usbxxreg->name, com + usbxxreg->offset,
						readw(com + usbxxreg->offset));
					break;
				case 32:
					seq_printf(s, "%-12s(0x%p): 0x%08X\n",
						usbxxreg->name, com + usbxxreg->offset,
						readl(com + usbxxreg->offset));
					break;
				}
			}
		}
	}
}

static int tphy_phy_regdump_show(struct seq_file *m, void *unused)
{
	unsigned i = 0;
	struct mtk_tphy *tphy = m->private;

	seq_printf(m, "\n===tphy_print_phy_regs(%i) help===\n", tphy->nphys);
	for (i = 0; i < tphy->nphys; i++)
		tphy_print_phy_regs(m, i);

	return 0;
}


static int tphy_phy_regdump(struct inode *inode, struct file *file)
{
	return single_open(file, tphy_phy_regdump_show, inode->i_private);
}

static const struct file_operations mtk_tphy_regdump_fops = {
	.open		= tphy_phy_regdump,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int tphy_phy_debug_proc_show(struct seq_file *m, void *v)
{
	seq_puts(m, "\n===tphy_phy_debug help===\n");
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

static ssize_t tphy_phy_debug_proc_write(struct file *file, const char *buf, size_t count, loff_t *data)
{
	int ret;
	int cmd, p1, p3, p4, p5;
	unsigned int reg_value;
	int sscanf_num;
	struct seq_file	*s = file->private_data;
	struct mtk_tphy *tphy = s->private;
	void __iomem *iomem = NULL;

	unsigned int long long p2;

	p1 = p2 = p3 = p4 = p5 = -1;

	if (count == 0)
		return -1;

	if (count > 255)
		count = 255;

	ret = copy_from_user(tphy->cmd_buf, buf, count);
	if (ret < 0)
		return -1;

	tphy->cmd_buf[count] = '\0';
	pr_err("[tphy debug]debug received:%s\n", tphy->cmd_buf);

	sscanf_num = sscanf(tphy->cmd_buf, "%x %x %llx %x %x %x", &cmd, &p1, &p2, &p3, &p4, &p5);
	if (sscanf_num < 1)
		return count;

	if (cmd == 0)
		pr_err("[tphy debug] zone <0x%.8x> is not exist yet\n", p1);
	else if (cmd == 1) {
		iomem += p2;
		if (p1 == 0) {
			reg_value = p3;
			pr_err("[tphy debug][Register Write]Original:0x%p (0x%08X)\n",
								iomem, readl(iomem));

			writel(reg_value, iomem);

			pr_err("[tphy debug][Register Write]Writed:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		} else if (p1 == 1)
			pr_err("[tphy debug][Register Read]Register:0x%p (0x%08X)\n",
								iomem, readl(iomem));
		else if (p1 == 2)
			register_set_field(iomem, p3, p4, p5);
		else if (p1 == 3)
			register_get_field(iomem, p3, p4, p5);
		else
			pr_err("[tphy debug] todo\n");
	}

	return count;
}

static int tphy_phy_debug_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, tphy_phy_debug_proc_show, inode->i_private);
}

static const struct file_operations mtk_tphy_debug_proc_fops = {
	.open   = tphy_phy_debug_proc_open,
	.write  = tphy_phy_debug_proc_write,
	.read   = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


int tphy_phy_init_debugfs(struct mtk_tphy *tphy)
{
	int	ret;
	struct dentry *file;
	struct dentry *root;

	root = debugfs_create_dir(dev_driver_string(tphy->dev), NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	file = debugfs_create_file("regdump", S_IRUGO | S_IWUSR, root, tphy,
						&mtk_tphy_regdump_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	file = debugfs_create_file("debug", S_IRUGO | S_IWUSR, root, tphy,
						&mtk_tphy_debug_proc_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	tphy->debugfs_root = root;

	return 0;

err1:
	debugfs_remove_recursive(root);

err0:
	return ret;
}

void tphy_phy_exit_debugfs(struct mtk_tphy *tphy)
{
	debugfs_remove_recursive(tphy->debugfs_root);
}
