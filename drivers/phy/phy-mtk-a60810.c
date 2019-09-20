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

struct a60810_phy_pdata {
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

struct a60810_phy_instance {
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

struct a60810_u3phy {
	struct device *dev;
	void __iomem *sif_base;	/* only shared sif */
	/* deprecated, use @ref_clk instead in phy instance */
	struct clk *u3phya_ref;	/* reference clock of usb3 anolog phy */
	const struct a60810_phy_pdata *pdata;
	struct a60810_phy_instance **phys;
	int nphys;
};

#define A60810_I2C_ADDR					0x60
#define PHY_VERSION_BANK				0x20
#define PHY_VERSION_ADDR				0xE4

static void *g_ippc_port_addr;
static int usb_phy_writeb(unsigned char data, unsigned char addr)
{
	u3phy_write_reg(g_ippc_port_addr, A60810_I2C_ADDR, addr, data);

	return 0;
}

#ifdef DEBUG
static unsigned char usb_phy_readb(unsigned char addr)
{
	unsigned char data;

	data = u3phy_read_reg(g_ippc_port_addr, A60810_I2C_ADDR, addr);

	return 0;
}
#endif
static unsigned int get_phy_version(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
	unsigned int version = 0;
	void *i2c_port_base;

	i2c_port_base = instance->port_base;

	u3phy_write_reg8(i2c_port_base, A60810_I2C_ADDR, 0xff, PHY_VERSION_BANK);

	version = u3phy_read_reg32(i2c_port_base, A60810_I2C_ADDR, PHY_VERSION_ADDR);
	dev_info(u3phy->dev, "ssusb phy version: %x %p\n", version, i2c_port_base);

	return version;
}


static int a60810_init(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	g_ippc_port_addr = instance->port_base;

	if (get_phy_version(phy) != 0xa60810a) {
		dev_info(u3phy->dev, "get phy version failed\n");
		return -1;
	}

	/* usb phy initial sequence */
	usb_phy_writeb(0x00, 0xFF);
	MTK_FPGA_DBG("*********** before bank 0x00 ***********\n");
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x05, value: %x\n", usb_phy_readb(0x05));
	MTK_FPGA_DBG("[U2P]addr: 0x18, value: %x\n", usb_phy_readb(0x18));
	MTK_FPGA_DBG("*********** after ***********\n");
	usb_phy_writeb(0x55, 0x05);
	usb_phy_writeb(0x84, 0x18);

	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x05, value: %x\n", usb_phy_readb(0x05));
	MTK_FPGA_DBG("[U2P]addr: 0x18, value: %x\n", usb_phy_readb(0x18));
	MTK_FPGA_DBG("*********** before bank 0x10 ***********\n");
	usb_phy_writeb(0x10, 0xFF);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x0A, value: %x\n", usb_phy_readb(0x0A));
	MTK_FPGA_DBG("*********** after ***********\n");

	usb_phy_writeb(0x84, 0x0A);

	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x0A, value: %x\n", usb_phy_readb(0x0A));
	MTK_FPGA_DBG("*********** before bank 0x40 ***********\n");
	usb_phy_writeb(0x40, 0xFF);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x38, value: %x\n", usb_phy_readb(0x38));
	MTK_FPGA_DBG("[U2P]addr: 0x42, value: %x\n", usb_phy_readb(0x42));
	MTK_FPGA_DBG("[U2P]addr: 0x08, value: %x\n", usb_phy_readb(0x08));
	MTK_FPGA_DBG("[U2P]addr: 0x09, value: %x\n", usb_phy_readb(0x09));
	MTK_FPGA_DBG("[U2P]addr: 0x0C, value: %x\n", usb_phy_readb(0x0C));
	MTK_FPGA_DBG("[U2P]addr: 0x0E, value: %x\n", usb_phy_readb(0x0E));
	MTK_FPGA_DBG("[U2P]addr: 0x10, value: %x\n", usb_phy_readb(0x10));
	MTK_FPGA_DBG("[U2P]addr: 0x14, value: %x\n", usb_phy_readb(0x14));
	MTK_FPGA_DBG("*********** after ***********\n");

	usb_phy_writeb(0x46, 0x38);
	usb_phy_writeb(0x40, 0x42);
	usb_phy_writeb(0xAB, 0x08);
	usb_phy_writeb(0x0C, 0x09);
	usb_phy_writeb(0x71, 0x0C);
	usb_phy_writeb(0x4F, 0x0E);
	usb_phy_writeb(0xE1, 0x10);
	usb_phy_writeb(0x5F, 0x14);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x38, value: %x\n", usb_phy_readb(0x38));
	MTK_FPGA_DBG("[U2P]addr: 0x42, value: %x\n", usb_phy_readb(0x42));
	MTK_FPGA_DBG("[U2P]addr: 0x08, value: %x\n", usb_phy_readb(0x08));
	MTK_FPGA_DBG("[U2P]addr: 0x09, value: %x\n", usb_phy_readb(0x09));
	MTK_FPGA_DBG("[U2P]addr: 0x0C, value: %x\n", usb_phy_readb(0x0C));
	MTK_FPGA_DBG("[U2P]addr: 0x0E, value: %x\n", usb_phy_readb(0x0E));
	MTK_FPGA_DBG("[U2P]addr: 0x10, value: %x\n", usb_phy_readb(0x10));
	MTK_FPGA_DBG("[U2P]addr: 0x14, value: %x\n", usb_phy_readb(0x14));
	MTK_FPGA_DBG("*********** before bank 0x60 ***********\n");
	usb_phy_writeb(0x60, 0xFF);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x10, value: %x\n", usb_phy_readb(0x14));
	MTK_FPGA_DBG("*********** after ***********\n");

	usb_phy_writeb(0x03, 0x14);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x10, value: %x\n", usb_phy_readb(0x14));
	MTK_FPGA_DBG("*********** before bank 0x00 ***********\n");
	usb_phy_writeb(0x00, 0xFF);
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x6A, value: %x\n", usb_phy_readb(0x6A));
	MTK_FPGA_DBG("[U2P]addr: 0x68, value: %x\n", usb_phy_readb(0x68));
	MTK_FPGA_DBG("[U2P]addr: 0x6C, value: %x\n", usb_phy_readb(0x6C));
	MTK_FPGA_DBG("[U2P]addr: 0x6D, value: %x\n", usb_phy_readb(0x6D));
	usb_phy_writeb(0x04, 0x6A);
	usb_phy_writeb(0x08, 0x68);
	usb_phy_writeb(0x26, 0x6C);
	usb_phy_writeb(0x36, 0x6D);
	MTK_FPGA_DBG("*********** after ***********\n");
	MTK_FPGA_DBG("[U2P]addr: 0xFF, value: %x\n", usb_phy_readb(0xFF));
	MTK_FPGA_DBG("[U2P]addr: 0x6A, value: %x\n", usb_phy_readb(0x6A));
	MTK_FPGA_DBG("[U2P]addr: 0x68, value: %x\n", usb_phy_readb(0x68));
	MTK_FPGA_DBG("[U2P]addr: 0x6C, value: %x\n", usb_phy_readb(0x6C));
	MTK_FPGA_DBG("[U2P]addr: 0x6D, value: %x\n", usb_phy_readb(0x6D));

	MTK_FPGA_DBG("[U2P]%s, end\n", __func__);
	return 0;
}

static int a60810_phy_init(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
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
		a60810_init(phy);
		break;
	default:
		dev_err(u3phy->dev, "unsupported device type: %d\n", instance->type);
		break;
	}

	return 0;
}

static int a60810_phy_exit(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

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

static int a60810_phy_power_on(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

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

static int a60810_phy_power_off(struct phy *phy)
{
	struct a60810_phy_instance *instance = phy_get_drvdata(phy);
	struct a60810_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

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


static const struct phy_ops a60810_u3phy_ops = {
	.init		= a60810_phy_init,
	.exit		= a60810_phy_exit,
	.power_on	= a60810_phy_power_on,
	.power_off	= a60810_phy_power_off,
	.owner		= THIS_MODULE,
};

static const struct a60810_phy_pdata a60810_pdata = {
	.avoid_rx_sen_degradation = false,
	.version = MT_PHY_V2,
};

static const struct of_device_id a60810_u3phy_id_table[] = {
	{ .compatible = "mediatek,mt8695-u3phy-a60810", .data = &a60810_pdata },
	{ },
};
MODULE_DEVICE_TABLE(of, a60810_u3phy_id_table);

static void phy_v1_banks_init(struct a60810_u3phy *u3phy,
	struct a60810_phy_instance *instance)
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

static void phy_v2_banks_init(struct a60810_u3phy *u3phy,
	struct a60810_phy_instance *instance)
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

static struct phy *a60810_phy_xlate(struct device *dev,
					struct of_phandle_args *args)
{
	struct a60810_u3phy *u3phy = dev_get_drvdata(dev);
	struct a60810_phy_instance *instance = NULL;
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

static int u3phy_fpga_a60810_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *child_np;
	struct phy_provider *provider;
	struct resource *sif_res;
	struct a60810_u3phy *u3phy;
	struct resource res;
	int port, retval;

	match = of_match_node(a60810_u3phy_id_table, pdev->dev.of_node);
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
		struct a60810_phy_instance *instance;
		struct phy *phy;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance) {
			retval = -ENOMEM;
			goto put_child;
		}

		u3phy->phys[port] = instance;

		phy = devm_phy_create(dev, child_np, &a60810_u3phy_ops);
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

	provider = devm_of_phy_provider_register(dev, a60810_phy_xlate);

	return PTR_ERR_OR_ZERO(provider);
put_child:
	of_node_put(child_np);
	return retval;
}

static struct platform_driver a60810_u3phy_driver = {
	.probe		= u3phy_fpga_a60810_probe,
	.driver		= {
		.name	= "a60810-u3phy",
		.of_match_table = a60810_u3phy_id_table,
	},
};

module_platform_driver(a60810_u3phy_driver);

MODULE_AUTHOR("Chunfeng Yun <chunfeng.yun@mediatek.com>");
MODULE_DESCRIPTION("A60810 USB PHY driver");
MODULE_LICENSE("GPL v2");
