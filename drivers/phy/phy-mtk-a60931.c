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
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include "phy-mtk-u3phy-i2c.h"

#ifdef DEBUG
#define MTK_FPGA_DBG(fmt, ...)	dev_info(u3phy->dev, fmt, ##__VA_ARGS__)
#else
#define MTK_FPGA_DBG(fmt, ...)
#endif

enum mt_phy_version {
	MT_PHY_V1 = 1,
	MT_PHY_V2,
};

struct a60931_phy_pdata {
	/* avoid RX sensitivity level degradation only for mt8173 */
	bool avoid_rx_sen_degradation;
	enum mt_phy_version version;
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

struct a60931_phy_instance {
	struct phy *phy;
	void __iomem *port_base;
	union {
		struct u2phy_banks u2_banks;
		struct u3phy_banks u3_banks;
	};
	struct clk *ref_clk;	/* reference clock of anolog phy */
	u32 index;
	u8 type;
};

struct a60931_u3phy {
	struct device *dev;
	void __iomem *sif_base;	/* only shared sif */
	/* deprecated, use @ref_clk instead in phy instance */
	struct clk *u3phya_ref;	/* reference clock of usb3 anolog phy */
	const struct a60931_phy_pdata *pdata;
	struct a60931_phy_instance **phys;
	int nphys;
};

#define A60931_I2C_ADDR					0x60
#define PHY_VERSION_BANK				0x20
#define PHY_VERSION_ADDR				0xE4

static unsigned int get_phy_version(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
	unsigned int version = 0;
	void *i2c_port_base;

	i2c_port_base = instance->port_base;

	u3phy_write_reg8(i2c_port_base, A60931_I2C_ADDR, 0xff, PHY_VERSION_BANK);

	version = u3phy_read_reg32(i2c_port_base, A60931_I2C_ADDR, PHY_VERSION_ADDR);
	dev_info(u3phy->dev, "ssusb phy version: %x %p\n", version, i2c_port_base);

	return version;
}

static int a60931_init(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
	void *i2c_port_base;

	i2c_port_base = instance->port_base;

	if (get_phy_version(phy) != 0xa60931a) {
		dev_info(u3phy->dev, "get phy version failed\n");
		return -1;
	}

	/* I2C  0x60  0xFC[31:24]  0x00   RW     Change bank address to 0x00 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x00); /*change bank address to 0x00 */
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x14[14:12]  0x04   RW  RG_USB20_HSTX_SRCTRL */
	/*				set default U2 slew rate as 4 */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x14, 12, 0x7<<12, 0x4);
	MTK_FPGA_DBG("[U3P]addr: 0x14, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x14));

	/* I2C  0x60  0x18[23:23]  0x00   RW  RG_USB20_BC11_SW_EN */
	/*				RG_USB20_BC11_SW_EN, Disable BC 1.1 */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x18, 23, 0x1<<23, 0x0);
	MTK_FPGA_DBG("[U3P]addr: 0x18, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x18));

	/* I2C  0x60  0x68[18:18]  0x00   RW  force_suspendm    force_suspendm = 0 */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x68, 18, 0x1<<18, 0x0);
	MTK_FPGA_DBG("[U3P]addr: 0x68, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x68));

	/* I2C  0x60  0xFC[31:24]  0x30   RW      Change bank address to 0x30 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x30);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x04[29:29]  0x01   RW  RG_VUSB10_ON   SSUSB 1.0V power ON */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x04, 29, 0x1<<29, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x04, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x04));

	/* I2C  0x60  0x04[25:21]  0x11   RW  RG_SSUSB_XTAL_TOP_RESERVE */
	/*				RG_SSUSB_XTAL_TOP_RESERVE<15:11> =10001 */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x04, 21, 0x1f<<21, 0x11);
	MTK_FPGA_DBG("[U3P]addr: 0x04, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x04));

	/* I2C  0x60  0xFC[31:24]  0x40   RW     Change bank address to 0x40 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x40);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x38[15:00]  0x47   RW  DA_SSUSB_PLL_SSC_DELTA1 */
	/*			fine tune SSC delta1 to let SSC min average ~0ppm */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x38, 0, 0xffff<<0, 0x47);
	MTK_FPGA_DBG("[U3P]addr: 0x38, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x38));

	/* I2C  0x60  0x40[31:16]  0x44   RW  DA_SSUSB_PLL_SSC_DELTA */
	/*			fine tune SSC delta to let SSC min average ~0ppm */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x40, 16, 0xffff<<16, 0x44);
	MTK_FPGA_DBG("[U3P]addr: 0x40, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x40));

	/* I2C  0x60  0xFC[31:24]  0x30   RW    Change bank address to 0x30 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x30);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x14[15:00]  0x190  RW  RG_SSUSB_PLL_SSC_PRD */
	/*			fine tune SSC PRD to let SSC freq average 31.5KHz */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x14, 0, 0xffff<<0, 0x190);
	MTK_FPGA_DBG("[U3P]addr: 0x14, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x14));

	/* I2C  0x70  0xFC[31:24]  0x70   RW    Change bank address to 0x70 */
	u3phy_write_reg8(i2c_port_base, 0x70, 0xff, 0x70);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x70, 0xff));

	/* I2C  0x70  0x88[03:02]  0x01   RW     Pipe reset, clk driving current */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x88, 2, 0x3<<2, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x88, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x88));

	/* I2C  0x70  0x88[05:04]  0x01   RW    Data lane 0 driving current */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x88, 4, 0x3<<4, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x88, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x88));

	/* I2C  0x70  0x88[07:06]  0x01   RW     Data lane 1 driving current */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x88, 6, 0x3<<6, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x88, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x88));

	/* I2C  0x70  0x88[09:08]  0x01   RW    Data lane 2 driving current */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x88, 8, 0x3<<8, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x88, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x88));

	/* I2C  0x70  0x88[11:10]  0x01   RW     Data lane 3 driving current */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x88, 10, 0x3<<10, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x88, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x88));

	/* I2C  0x70  0x9C[04:00]  0x19   RW  rg_ssusb_ckphase  PCLK phase 0x00~0x1F */
	u3phy_writelmsk(i2c_port_base, 0x70, 0x9c, 0, 0x1f<<0, 0x19);
	MTK_FPGA_DBG("[U3P]addr: 0x9c, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x70, 0x9c));

	/*############################# */
	/*Set INTR & TX/RX Impedance	*/
	/*############################# */

	/* I2C  0x60  0xFC[31:24]  0x30   RW  Change bank address to 0x30 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x30);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x00[26:26]  0x01   RW  RG_SSUSB_INTR_EN   INTR_EN */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x00, 26, 0x1<<26, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x00, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x00));

	/* I2C  0x60  0x00[15:10]  0x26   RW  RG_SSUSB_IEXT_INTR_CTRL   Set Iext R selection */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x00, 10, 0x1f<<10, 0x26);
	MTK_FPGA_DBG("[U3P]addr: 0x00, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x00));

	/* I2C  0x60  0xFC[31:24]  0x10   RW          Change bank address to 0x10 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x10);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x10[31:31]  0x01   RW  rg_ssusb_force_tx_impsel */
	/*				 Force da_ssusb_tx_impsel enable */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x10, 31, 0x1<<31, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x10, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x10));

	/* I2C  0x60  0x10[28:24]  0x10   RW  rg_ssusb_tx_impsel   Set TX Impedance */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x10, 24, 0x1f<<24, 0x10);
	MTK_FPGA_DBG("[U3P]addr: 0x10, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x10));

	/* /I2C  0x60  0x14[31:31]  0x01   RW  rg_ssusb_force_rx_impsel Force da_ssusb_rx_impsel enable */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x14, 31, 0x1<<31, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x14, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x14));

	/* I2C  0x60  0x14[28:24]  0x10   RW  rg_ssusb_rx_impsel Set RX Impedance */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x14, 24, 0x1f<<24, 0x10);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x14));

	/* I2C  0x60  0xFC[31:24]  0x00   RW Change bank address to 0x00 */
	u3phy_write_reg8(i2c_port_base, 0x60, 0xff, 0x00);
	MTK_FPGA_DBG("[U3P]addr: 0xFF, value: %x\n", u3phy_read_reg8(i2c_port_base, 0x60, 0xff));

	/* I2C  0x60  0x00[05:05]  0x01   RW  RG_USB20_INTR_EN     U2 INTR_EN */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x00, 5, 0x1<<5, 0x01);
	MTK_FPGA_DBG("[U3P]addr: 0x00, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x00));

	/* I2C  0x60  0x04[23:19]  0x14   RW  RG_USB20_INTR_CAL   Set Iext R selection */
	u3phy_writelmsk(i2c_port_base, 0x60, 0x04, 19, 0x1f<<19, 0x14);
	MTK_FPGA_DBG("[U3P]addr: 0x04, value: %x\n", u3phy_read_reg32(i2c_port_base, 0x60, 0x04));

	return 0;
}

#if 0
static int a60931_u3phy_change_pclk_phase(void __iomem *i2c_port_base, unsigned int pclk_phase)
{
	u3phy_write_reg8(i2c_port_base, 0x70, 0xff, 0x70);
	u3phy_writelmsk(i2c_port_base, 0x70, 0x9c, 0, 0x1f<<0, pclk_phase);

	return 0;
}

/* for OTG test */
static int a60931_u3phy_u2_reg_w(void __iomem *i2c_port_base, u8 addr, u8 data)
{
	u3phy_write_reg8(i2c_port_base, 0x60, addr, data);
	return 0;
}

static u8 a60931_u3phy_u2_reg_r(void __iomem *i2c_port_base, u8 addr)
{
	return u3phy_read_reg8(i2c_port_base, 0x60, addr);
}
#endif

static int a60931_phy_init(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
	int ret;

	ret = clk_prepare_enable(u3phy->u3phya_ref);
	if (ret) {
		dev_err(u3phy->dev, "failed to enable u3phya_ref\n");
		return ret;
	}

	ret = clk_prepare_enable(instance->ref_clk);
	if (ret) {
		dev_err(u3phy->dev, "failed to enable ref_clk\n");
		return ret;
	}

	switch (instance->type) {
	case PHY_TYPE_USB2:
	case PHY_TYPE_USB3:
		a60931_init(phy);
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

	return 0;
}

static int a60931_phy_exit(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	switch (instance->type) {
	case PHY_TYPE_USB2:
	case PHY_TYPE_USB3:
		/* phy_instance_exit(u3phy, instance); */
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

	clk_disable_unprepare(instance->ref_clk);
	clk_disable_unprepare(u3phy->u3phya_ref);
	return 0;
}

static int a60931_phy_power_on(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	switch (instance->type) {
	case PHY_TYPE_USB2:
	case PHY_TYPE_USB3:
		/* phy_instance_power_on(u3phy, instance); */
		/* hs_slew_rate_calibrate(u3phy, instance); */
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

	return 0;
}

static int a60931_phy_power_off(struct phy *phy)
{
	struct a60931_phy_instance *instance = phy_get_drvdata(phy);
	struct a60931_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	switch (instance->type) {
	case PHY_TYPE_USB2:
	case PHY_TYPE_USB3:
		/* phy_instance_power_off(u3phy, instance); */
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

	return 0;
}


static const struct phy_ops a60931_u3phy_ops = {
	.init		= a60931_phy_init,
	.exit		= a60931_phy_exit,
	.power_on	= a60931_phy_power_on,
	.power_off	= a60931_phy_power_off,
	.owner		= THIS_MODULE,
};

static const struct a60931_phy_pdata a60931_pdata = {
	.avoid_rx_sen_degradation = false,
	.version = MT_PHY_V2,
};

static const struct of_device_id a60931_u3phy_id_table[] = {
	{ .compatible = "mediatek,mt8695-u3phy-a60931", .data = &a60931_pdata },
	{ },
};
MODULE_DEVICE_TABLE(of, a60931_u3phy_id_table);

static void phy_v1_banks_init(struct a60931_u3phy *u3phy,
	struct a60931_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc	= NULL;
		u2_banks->fmreg = u3phy->sif_base;
		u2_banks->com	= instance->port_base;
		break;
	case PHY_TYPE_USB3:
		u3_banks->spllc	= u3phy->sif_base;
		u3_banks->chip	= NULL;
		u3_banks->phyd	= instance->port_base;
		u3_banks->phya	= instance->port_base;
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

}

static void phy_v2_banks_init(struct a60931_u3phy *u3phy,
	struct a60931_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc	= instance->port_base;
		u2_banks->fmreg	= instance->port_base;
		u2_banks->com	= instance->port_base;
		break;
	case PHY_TYPE_USB3:
		u3_banks->spllc	= instance->port_base;
		u3_banks->chip	= instance->port_base;
		u3_banks->phyd	= instance->port_base;
		u3_banks->phya	= instance->port_base;
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

}

static struct phy *a60931_phy_xlate(struct device *dev,
					struct of_phandle_args *args)
{
	struct a60931_u3phy *u3phy = dev_get_drvdata(dev);
	struct a60931_phy_instance *instance = NULL;
	struct device_node *phy_np = args->np;
	int index;

	if (args->args_count != 1) {
		dev_err(dev, "invalid number of cells in 'phy' property\n");
		return ERR_PTR(-EINVAL);
	}

	for (index = 0; index < u3phy->nphys; index++)
		if (phy_np == u3phy->phys[index]->phy->dev.of_node) {
			instance = u3phy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(dev, "failed to find appropriate phy\n");
		return ERR_PTR(-EINVAL);
	}

	instance->type = args->args[0];
	switch (instance->type) {
	case PHY_TYPE_USB2:
	case PHY_TYPE_USB3:
		/* phy_instance_power_off(u3phy, instance); */
		break;
	default:
		dev_err(dev, "unsupported device type: %d\n", instance->type);
		return ERR_PTR(-EINVAL);
	}

	if (u3phy->pdata->version == MT_PHY_V1) {
		phy_v1_banks_init(u3phy, instance);
	} else if (u3phy->pdata->version == MT_PHY_V2) {
		phy_v2_banks_init(u3phy, instance);
	} else {
		dev_err(dev, "phy version is not supported\n");
		return ERR_PTR(-EINVAL);
	}

	return instance->phy;
}

static int u3phy_fpga_a60931_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *child_np;
	struct phy_provider *provider;
	struct resource *sif_res;
	struct a60931_u3phy *u3phy;
	struct resource res;
	int port, retval;

	match = of_match_node(a60931_u3phy_id_table, pdev->dev.of_node);
	if (!match)
		return -EINVAL;

	u3phy = devm_kzalloc(dev, sizeof(*u3phy), GFP_KERNEL);
	if (!u3phy)
		return -ENOMEM;

	u3phy->pdata = match->data;
	u3phy->nphys = of_get_child_count(np);
	u3phy->phys = devm_kcalloc(dev, u3phy->nphys,
					sizeof(*u3phy->phys), GFP_KERNEL);
	if (!u3phy->phys)
		return -ENOMEM;

	u3phy->dev = dev;
	platform_set_drvdata(pdev, u3phy);

	if (u3phy->pdata->version == MT_PHY_V1) {
		/* get banks shared by multiple phys */
		sif_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		u3phy->sif_base = devm_ioremap_resource(dev, sif_res);
		if (IS_ERR(u3phy->sif_base)) {
			dev_err(dev, "failed to remap sif regs\n");
			return PTR_ERR(u3phy->sif_base);
		}
	}

	/* it's deprecated, make it optional for backward compatibility */
	u3phy->u3phya_ref = devm_clk_get(dev, "u3phya_ref");
	if (IS_ERR(u3phy->u3phya_ref)) {
		if (PTR_ERR(u3phy->u3phya_ref) == -EPROBE_DEFER)
			return -EPROBE_DEFER;

		u3phy->u3phya_ref = NULL;
	}

	port = 0;
	for_each_child_of_node(np, child_np) {
		struct a60931_phy_instance *instance;
		struct phy *phy;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance) {
			retval = -ENOMEM;
			goto put_child;
		}

		u3phy->phys[port] = instance;

		phy = devm_phy_create(dev, child_np, &a60931_u3phy_ops);
		if (IS_ERR(phy)) {
			dev_err(dev, "failed to create phy\n");
			retval = PTR_ERR(phy);
			goto put_child;
		}

		retval = of_address_to_resource(child_np, 0, &res);
		if (retval) {
			dev_err(dev, "failed to get address resource(id-%d)\n",
				port);
			goto put_child;
		}

		instance->port_base = devm_ioremap_resource(&phy->dev, &res);
		if (IS_ERR(instance->port_base)) {
			dev_err(dev, "failed to remap phy regs\n");
			retval = PTR_ERR(instance->port_base);
			goto put_child;
		}

		instance->phy = phy;
		instance->index = port;
		phy_set_drvdata(phy, instance);
		port++;

		/* if deprecated clock is provided, ignore instance's one */
		if (u3phy->u3phya_ref)
			continue;

		instance->ref_clk = devm_clk_get(&phy->dev, "ref");
		if (IS_ERR(instance->ref_clk)) {
			dev_err(dev, "failed to get ref_clk(id-%d)\n", port);
			retval = PTR_ERR(instance->ref_clk);
			goto put_child;
		}
	}

	provider = devm_of_phy_provider_register(dev, a60931_phy_xlate);

	return PTR_ERR_OR_ZERO(provider);
put_child:
	of_node_put(child_np);
	return retval;
}

static struct platform_driver a60931_u3phy_driver = {
	.probe		= u3phy_fpga_a60931_probe,
	.driver		= {
		.name	= "a60931-u3phy",
		.of_match_table = a60931_u3phy_id_table,
	},
};

module_platform_driver(a60931_u3phy_driver);

MODULE_AUTHOR("Chunfeng Yun <chunfeng.yun@mediatek.com>");
MODULE_DESCRIPTION("a60931 USB PHY driver");
MODULE_LICENSE("GPL v2");

