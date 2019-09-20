/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/version.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/regmap.h>

enum {
	MT6393_ID_VPROC = 0,
	MT6393_ID_VDDR,
	MT6393_ID_VIO18,
	MT6393_ID_VIO33,
	MT6393_ID_SD,
	MT6393_ID_RG_MAX,
};

#define MT6393_MAX_REGULATOR				MT6393_ID_RG_MAX

/* PMIC Registers */
#define MT6393_PRODUCT_ID				0x00
#define MT6393_MANUFACTURE_ID				0x01
#define MT6393_REVISION_NUMBER				0x02
#define MT6393_PG_INFO					0x03
#define MT6393_UV_INFO					0x04
#define MT6393_OV_INFO					0x05
#define MT6393_CH1_VID_SETTING				0x06
#define MT6393_CH1_SLEEP_VID_SETTING			0x07
#define MT6393_INTERNAL_ENABLE				0x08
#define MT6393_BUCK_MODE_CTRL				0x09
#define MT6393_PROTECTION_MODE				0x0A
#define MT6393_WDOG_RST_CTRL				0x0B
#define MT6393_SLEEP_MODE_CTRL				0x0C
#define MT6393_SEQUENCE_CONFIG				0x0D
#define MT6393_CH1_TIMEING_CONFIG			0x0E
#define MT6393_CH2_TIMEING_CONFIG			0x0F
#define MT6393_CH3_TIMEING_CONFIG			0x10
#define MT6393_CH4_TIMEING_CONFIG			0x11
#define MT6393_CH5_TIMEING_CONFIG			0x12
#define MT6393_EX_EN1_TIMEING_CONFIG			0x13
#define MT6393_INT_ENABLE				0x1D
#define MT6393_INT_STATUS				0x1E
#define MT6393_DIS_CHARGE_ENABLE			0x1F
#define MT6393_CH2_VOLTAGE_OFFSET			0x20

#define MT6393_BUCK_MODE_AUTO				0
#define MT6393_BUCK_MODE_FORCE_PWM			1
#define MT6393_LDO_MODE_NORMAL				0
#define MT6393_LDO_MODE_LP				1

#define mt6393_vsell_vol_reg				(MT6393_CH1_VID_SETTING)
#define mt6393_vsell_vol_mask				(0x7f)
#define mt6393_vsell_enable_reg				(MT6393_INTERNAL_ENABLE)
#define mt6393_ch1_enable_mask				(0x02)
#define mt6393_ch2_enable_mask				(0x04)
#define mt6393_ch3_enable_mask				(0x08)
#define mt6393_ch4_enable_mask				(0x10)
#define mt6393_ch5_enable_mask				(0x20)
#define mt6393_buck_mode_reg				(MT6393_BUCK_MODE_CTRL)
#define mt6393_ch1_mode_mask				(0x02)
#define mt6393_ch2_mode_mask				(0x04)
#define mt6393_ch3_mode_mask				(0x08)
#define mt6393_ch4_mode_mask				(0x10)
#define mt6393_ldo_mode_reg				(MT6393_SLEEP_MODE_CTRL)
#define mt6393_ldo_mode_mask				(0x01)

struct mt6393_regulator_info {
	struct regulator_desc desc;
	u32 modeset_reg;
	u32 modeset_mask;
};

struct mt6393_chip {
	struct device *dev;
	struct regmap *regmap;
	struct i2c_client *i2c;
};

static const struct regmap_config mt6393_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static int mt6393_get_status(struct regulator_dev *rdev);
static int mt6393_buck_set_mode(struct regulator_dev *rdev, unsigned int mode);
static unsigned int mt6393_buck_get_mode(struct regulator_dev *rdev);
static int mt6393_ldo_set_mode(struct regulator_dev *rdev, unsigned int mode);
static unsigned int mt6393_ldo_get_mode(struct regulator_dev *rdev);

static const struct regulator_ops mt6393_volt_buck_range_ops = {
	.list_voltage = regulator_list_voltage_linear_range,
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_mode = mt6393_buck_set_mode,
	.get_mode = mt6393_buck_get_mode,
	.get_status = mt6393_get_status,
};

static const struct regulator_ops mt6393_volt_buck_fixed_ops = {
	.list_voltage = regulator_list_voltage_linear,
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.set_mode = mt6393_buck_set_mode,
	.get_mode = mt6393_buck_get_mode,
	.get_status = mt6393_get_status,
};

static const struct regulator_ops mt6393_volt_ldo_ops = {
	.list_voltage = regulator_list_voltage_linear,
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.set_mode = mt6393_ldo_set_mode,
	.get_mode = mt6393_ldo_get_mode,
	.get_status = mt6393_get_status,
};

#define MT6393_BUCK(match, vreg, min, max, step, volt_ranges, enreg, enmask,  \
		vosel, vosel_mask, _modeset_reg, _modeset_mask)				\
[MT6393_ID_##vreg] = {							\
	.desc = {                           \
		.name = #vreg,						\
		.of_match = of_match_ptr(match),			\
		.ops = &mt6393_volt_buck_range_ops,				\
		.type = REGULATOR_VOLTAGE,				\
		.id = MT6393_ID_##vreg,					\
		.owner = THIS_MODULE,					\
		.n_voltages = (max - min)/step + 1,			\
		.linear_ranges = volt_ranges,				\
		.n_linear_ranges = ARRAY_SIZE(volt_ranges),		\
		.min_uV = min,                     \
		.uV_step = step,                    \
		.vsel_reg = vosel,					\
		.vsel_mask = vosel_mask,				\
		.enable_reg = enreg,					\
		.enable_mask = enmask,					\
		.enable_val = enmask,					\
		.disable_val = 0,					\
	},                                         \
	.modeset_reg = _modeset_reg,                \
	.modeset_mask = _modeset_mask,              \
}

#define MT6393_FIXED_BUCK(match, vreg, min, enreg, enmask, _modeset_reg, _modeset_mask)   \
[MT6393_ID_##vreg] = {												\
	.desc = {                                      \
		.name = #vreg,						\
		.of_match = of_match_ptr(match),			\
		.ops = &mt6393_volt_buck_fixed_ops,				\
		.type = REGULATOR_VOLTAGE,				\
		.id = MT6393_ID_##vreg,					\
		.owner = THIS_MODULE,					\
		.n_voltages = 1,			\
		.min_uV = min,                     \
		.enable_reg = enreg,					\
		.enable_mask = enmask,					\
		.enable_val = enmask,					\
		.disable_val = 0,					\
	},                                       \
	.modeset_reg = _modeset_reg,                \
	.modeset_mask = _modeset_mask,              \
}

#define MT6393_FIXED_LDO(match, vreg, min, enreg, enmask, _modeset_reg, _modeset_mask)		\
[MT6393_ID_##vreg] = {												\
	.desc = {                                  \
		.name = #vreg,						\
		.of_match = of_match_ptr(match),			\
		.ops = &mt6393_volt_ldo_ops,				\
		.type = REGULATOR_VOLTAGE,				\
		.id = MT6393_ID_##vreg,					\
		.owner = THIS_MODULE,					\
		.n_voltages = 1,			\
		.min_uV = min,                     \
		.enable_reg = enreg,					\
		.enable_mask = enmask,					\
		.enable_val = enmask,					\
		.disable_val = 0,					\
	},                                           \
	.modeset_reg = _modeset_reg,                \
	.modeset_mask = _modeset_mask,              \
}

static const struct regulator_linear_range buck_volt_range[] = {
	REGULATOR_LINEAR_RANGE(600000, 0, 0x7f, 6250),
};

static struct mt6393_regulator_info mt6393_regulators[] = {
	MT6393_BUCK("buck_vproc", VPROC, 600000, 1393750, 6250,
		buck_volt_range, mt6393_vsell_enable_reg, mt6393_ch1_enable_mask, mt6393_vsell_vol_reg,
		mt6393_vsell_vol_mask, mt6393_buck_mode_reg, mt6393_ch1_mode_mask),
	MT6393_FIXED_BUCK("buck_vddr", VDDR, 1100000, mt6393_vsell_enable_reg, mt6393_ch2_enable_mask,
		mt6393_buck_mode_reg, mt6393_ch2_mode_mask),
	MT6393_FIXED_BUCK("buck_vio18", VIO18, 1800000, mt6393_vsell_enable_reg, mt6393_ch3_enable_mask,
		mt6393_buck_mode_reg, mt6393_ch3_mode_mask),
	MT6393_FIXED_BUCK("buck_vio33", VIO33, 3300000, mt6393_vsell_enable_reg, mt6393_ch4_enable_mask,
		mt6393_buck_mode_reg, mt6393_ch4_mode_mask),
	MT6393_FIXED_LDO("ldo_sd", SD, 3300000, mt6393_vsell_enable_reg, mt6393_ch5_enable_mask,
		mt6393_ldo_mode_reg, mt6393_ldo_mode_mask),
};

static int mt6393_get_status(struct regulator_dev *rdev)
{
	int ret;
	u32 regval;
	struct mt6393_regulator_info *info = rdev_get_drvdata(rdev);

	ret = regmap_read(rdev->regmap, info->desc.enable_reg, &regval);
	if (ret != 0) {
		dev_info(&rdev->dev, "Failed to get enable reg: %d\n", ret);
		return ret;
	}

	return (regval & info->desc.enable_mask) ? REGULATOR_STATUS_ON : REGULATOR_STATUS_OFF;
}

static int mt6393_buck_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	struct mt6393_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	unsigned int val, reg_val;

	if (!info->modeset_mask) {
		dev_info(&rdev->dev, "regulator %s doesn't support set_mode\n", info->desc.name);
		return -EINVAL;
	}
	switch (mode) {
	case REGULATOR_MODE_FAST:
		val = MT6393_BUCK_MODE_FORCE_PWM;
		break;
	case REGULATOR_MODE_NORMAL:
		val = MT6393_BUCK_MODE_AUTO;
		break;
	default:
		return -EINVAL;
	}
	val <<= ffs(info->modeset_mask) - 1;

	ret = regmap_update_bits(rdev->regmap, info->modeset_reg,
						  info->modeset_mask, val);
	if (regmap_read(rdev->regmap, info->modeset_reg, &reg_val) < 0) {
		dev_info(&rdev->dev, "Failed to read register value\n");
		return -EIO;
	}
	dev_info(&rdev->dev, "info->modeset_reg 0x%x = 0x%x\n", info->modeset_reg, reg_val);

	return ret;
}

static unsigned int mt6393_buck_get_mode(struct regulator_dev *rdev)
{
	struct mt6393_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	unsigned int mode;
	unsigned int reg_val = 0;

	if (!info->modeset_mask) {
		dev_info(&rdev->dev, "regulator %s doesn't support get_mode\n", info->desc.name);
		return -EINVAL;
	}

	ret = regmap_read(rdev->regmap, info->modeset_reg, &reg_val);
	if (ret < 0)
		return ret;

	reg_val &= info->modeset_mask;
	reg_val >>= ffs(info->modeset_mask) - 1;

	if (reg_val & 0x1)
		mode = REGULATOR_MODE_FAST;
	else
		mode = REGULATOR_MODE_NORMAL;

	return mode;
}

static int mt6393_ldo_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	int ret;
	unsigned int reg_val, val;
	struct mt6393_regulator_info *info = rdev_get_drvdata(rdev);

	if (!info->modeset_mask) {
		dev_info(&rdev->dev, "regulator %s doesn't support set_mode\n",
			info->desc.name);
		return -EINVAL;
	}
	switch (mode) {
	case REGULATOR_MODE_STANDBY:
		val = MT6393_LDO_MODE_LP;
		break;
	case REGULATOR_MODE_NORMAL:
		val = MT6393_LDO_MODE_NORMAL;
		break;
	default:
		return -EINVAL;
	}

	val <<= ffs(info->modeset_mask) - 1;

	ret = regmap_update_bits(rdev->regmap, info->modeset_reg,
				  info->modeset_mask, val);

	if (regmap_read(rdev->regmap, info->modeset_reg, &reg_val) < 0) {
		dev_info(&rdev->dev, "Failed to read register value\n");
		return -EIO;
	}
	dev_info(&rdev->dev, "info->modeset_reg 0x%x = 0x%x\n", info->modeset_reg, reg_val);

	return ret;
}

static unsigned int mt6393_ldo_get_mode(struct regulator_dev *rdev)
{
	struct mt6393_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned int val;
	unsigned int mode;
	int ret;

	if (!info->modeset_mask) {
		dev_info(&rdev->dev, "regulator %s doesn't support get_mode\n",
			info->desc.name);
		return -EINVAL;
	}

	ret = regmap_read(rdev->regmap, info->modeset_reg, &val);
	if (ret < 0)
		return ret;

	val &= info->modeset_mask;
	val >>= ffs(info->modeset_mask) - 1;

	if (val & 0x1)
		mode = REGULATOR_MODE_STANDBY;
	else
		mode = REGULATOR_MODE_NORMAL;

	return mode;
}

u32 reg_value_pmic;
static ssize_t show_pmic_access(struct device *dev, struct device_attribute *attr, char *buf)
{
	pr_notice("[show_pmic_access] 0x%x\n", reg_value_pmic);
	return sprintf(buf, "%02X\n", reg_value_pmic);
}

static ssize_t store_pmic_access(struct device *dev, struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int ret = 0;
	char temp_buf[32];
	char *pvalue;
	unsigned int reg_value = 0;
	unsigned int reg_address = 0;
	struct mt6393_chip *mt6393_drivedata;

	mt6393_drivedata = dev_get_drvdata(dev);

	strncpy(temp_buf, buf, sizeof(temp_buf));
	temp_buf[sizeof(temp_buf) - 1] = 0;
	pvalue = temp_buf;

	if (size != 0) {
		if (size > 5) {
			ret = kstrtouint(strsep(&pvalue, " "), 16, &reg_address);
			if (ret)
				return ret;
			ret = kstrtouint(pvalue, 16, &reg_value);
			if (ret)
				return ret;
			pr_notice("[store_pmic_access] write PMU reg 0x%x with value 0x%x !\n",
				  reg_address, reg_value);
			ret = regmap_write(mt6393_drivedata->regmap, reg_address, reg_value);
		} else {
			ret = kstrtouint(pvalue, 16, &reg_address);
			if (ret)
				return ret;
			ret = regmap_read(mt6393_drivedata->regmap, reg_address, &reg_value_pmic);
			pr_notice("[store_pmic_access] read PMU reg 0x%x with value 0x%x !\n",
				  reg_address, reg_value_pmic);
			pr_notice("[store_pmic_access] Please use \"cat pmic_access\" to get value\r\n");
		}
	}
	return size;
}

static DEVICE_ATTR(pmic_access, 0664, show_pmic_access, store_pmic_access);	/* 664 */

static int mt6393_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct mt6393_regulator_info *info;
	struct mt6393_chip *mt6393;
	struct regulator_dev *rdev;
	struct regulator_config config = {};
	struct regulation_constraints *c;
	struct device_node *np;
	int ret;
	int i;
	int suspend_gpio;
	u32 val;

	device_create_file(&i2c->dev, &dev_attr_pmic_access);

	info = devm_kzalloc(&i2c->dev, sizeof(struct mt6393_regulator_info), GFP_KERNEL);
		if (!info)
			return -ENOMEM;

	mt6393 = devm_kzalloc(&i2c->dev, sizeof(struct mt6393_chip), GFP_KERNEL);
		if (!mt6393)
			return -ENOMEM;

	/*allocate regmap*/
	mt6393->regmap = devm_regmap_init_i2c(i2c, &mt6393_regmap_config);
	if (IS_ERR(mt6393->regmap)) {
		ret = PTR_ERR(mt6393->regmap);
		dev_info(&i2c->dev, "Failed to allocate register map: %d\n", ret);
		return ret;
	}

	/*set other information of chip*/
	mt6393->i2c = i2c;
	mt6393->dev = &i2c->dev;

	i2c_set_clientdata(i2c, mt6393);

	/*regist regulators*/
	for (i = 0; i < MT6393_MAX_REGULATOR; i++) {
		config.dev = &i2c->dev;
		config.driver_data = &mt6393_regulators[i];
		config.regmap = mt6393->regmap;

		rdev = devm_regulator_register(&i2c->dev, &mt6393_regulators[i].desc, &config);

		if (IS_ERR(rdev)) {
			dev_info(&i2c->dev, "fail to register mt6393 regulator : %s\n",
				i2c->dev.of_node->name);
			return ret;
		}

		/* Constrain board-specific capabilities according to what
		 * this driver and the chip itself can actually do.
		 */
		c = rdev->constraints;
		c->valid_modes_mask |= REGULATOR_MODE_NORMAL|
			REGULATOR_MODE_STANDBY | REGULATOR_MODE_FAST;
		c->valid_ops_mask |= REGULATOR_CHANGE_MODE;

	}

	/*set pmic suspend gpio */
	np = of_find_node_by_name(NULL, "mt6393regulator");
	suspend_gpio = of_get_named_gpio(np, "pmic,suspend_gpio", 0);

	if (suspend_gpio < 0) {
		pr_info("%s: no pmic suspend_gpio info\n", __func__);
		return 0;
	}

	ret = gpio_request(suspend_gpio, "suspend_gpio");
	if (ret < 0) {
		dev_info(&i2c->dev, "%s : fail to request pmic_suspend gpio\n", __func__);
		return ret;
	}

	ret = of_property_read_u32(np, "normal_mode", &val);
	if (val)
		gpio_direction_output(suspend_gpio, 1);
	else
		gpio_direction_output(suspend_gpio, 0);

	return 0;
}

static const struct of_device_id mt6393_match_table[] = {
	{.compatible = "mediatek,mt6393-regulator",},
	{},
};

MODULE_DEVICE_TABLE(of, mt6393_match_table);

static const struct i2c_device_id mt6393_dev_id[] = {
	{"mt6393-regulator", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, mt6393_dev_id);

static struct i2c_driver mt6393_i2c_driver = {
	.driver = {
		   .name = "mt6393-regulator",
		   .owner = THIS_MODULE,
		   .of_match_table = mt6393_match_table,
		   },
	.probe = mt6393_i2c_probe,
	.id_table = mt6393_dev_id,
};

module_i2c_driver(mt6393_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Qiqi Wang <Qiqi.Wang@mediatek.com>");
MODULE_DESCRIPTION("Regulator driver for MT6393");

