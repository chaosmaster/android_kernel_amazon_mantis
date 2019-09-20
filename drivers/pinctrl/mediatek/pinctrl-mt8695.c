/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author:  Biao Huang <biao.huang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/regmap.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <dt-bindings/pinctrl/mt65xx.h>

#include "pinctrl-mtk-common.h"
#include "pinctrl-mtk-mt8695.h"

static const struct mtk_pin_spec_driving_set_samereg mt8695_spec_driving[] = {
	MTK_PIN_SPEC_DRIVING(56, 0xd80, 0, 2, 1),
	MTK_PIN_SPEC_DRIVING(57, 0xd80, 8, 10, 9),
	MTK_PIN_SPEC_DRIVING(76, 0xd50, 0, 2, 1),
	MTK_PIN_SPEC_DRIVING(77, 0xd50, 8, 10, 9),
	MTK_PIN_SPEC_DRIVING(117, 0xd40, 0, 2, 1),
	MTK_PIN_SPEC_DRIVING(118, 0xd40, 8, 10, 9),
};

static int mt8695_spec_driving_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, bool isup, unsigned int d1d0)
{
	return mtk_pctrl_spec_driving_set_samereg(regmap, mt8695_spec_driving,
		ARRAY_SIZE(mt8695_spec_driving), pin, align, isup, d1d0);
}

static int mt8695_spec_driving_get(struct regmap *regmap, unsigned int pin)
{
	return mtk_spec_driving_get_samereg(regmap, mt8695_spec_driving,
		ARRAY_SIZE(mt8695_spec_driving), pin);
}

static const struct mtk_pin_spec_rsel_set_samereg mt8695_spec_rsel[] = {
	MTK_PIN_SPEC_RSEL(56, 0xd80, 0, 4, 3),
	MTK_PIN_SPEC_RSEL(57, 0xd80, 8, 12, 11),
	MTK_PIN_SPEC_RSEL(76, 0xd50, 0, 4, 3),
	MTK_PIN_SPEC_RSEL(77, 0xd50, 8, 12, 11),
	MTK_PIN_SPEC_RSEL(117, 0xd40, 0, 4, 3),
	MTK_PIN_SPEC_RSEL(118, 0xd40, 8, 12, 11),
};

static int mt8695_spec_rsel_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, bool isup, unsigned int s1s0)
{
	return mtk_pctrl_spec_rsel_set_samereg(regmap, mt8695_spec_rsel,
		ARRAY_SIZE(mt8695_spec_rsel), pin, align, isup, s1s0);
}

static int mt8695_spec_rsel_get(struct regmap *regmap, unsigned int pin)
{
	return mtk_spec_rsel_get_samereg(regmap, mt8695_spec_rsel,
		ARRAY_SIZE(mt8695_spec_rsel), pin);
}

static const struct mtk_pin_spec_pupd_set_samereg mt8695_spec_pupd[] = {
	MTK_PIN_PUPD_SPEC_SR(0, 0xc70, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(1, 0xc80, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(2, 0xc80, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(3, 0xc80, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(4, 0xc80, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(5, 0xc80, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(6, 0xc90, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(7, 0xc90, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(8, 0xc90, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(9, 0xc90, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(10, 0xc90, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(11, 0xca0, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(12, 0xca0, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(13, 0xcc0, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(14, 0xcc0, 8, 7, 6),

	MTK_PIN_PUPD_SPEC_SR(28, 0xc40, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(29, 0xc50, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(30, 0xc50, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(31, 0xc50, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(32, 0xc50, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(33, 0xc50, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(34, 0xc40, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(35, 0xca0, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(36, 0xc70, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(37, 0xca0, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(38, 0xca0, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(39, 0xcb0, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(40, 0xcb0, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(41, 0xcb0, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(42, 0xcb0, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(43, 0xcb0, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(44, 0xcc0, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(45, 0xc70, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(46, 0xc70, 8, 7, 6),

	MTK_PIN_PUPD_SPEC_SR(52, 0xcd0, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(53, 0xcc0, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(54, 0xcd0, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(55, 0xcd0, 5, 4, 3),

	MTK_PIN_PUPD_SPEC_SR(60, 0xcd0, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(61, 0xcd0, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(62, 0xce0, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(63, 0xce0, 5, 4, 3),
	MTK_PIN_PUPD_SPEC_SR(64, 0xce0, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(65, 0xce0, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(66, 0xc60, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(67, 0xc70, 2, 1, 0),
	MTK_PIN_PUPD_SPEC_SR(68, 0xc60, 11, 10, 9),
	MTK_PIN_PUPD_SPEC_SR(69, 0xc60, 14, 13, 12),
	MTK_PIN_PUPD_SPEC_SR(70, 0xc60, 8, 7, 6),
	MTK_PIN_PUPD_SPEC_SR(71, 0xc60, 5, 4, 3),
};

static int mt8695_spec_pull_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, bool isup, unsigned int r1r0)
{
	return mtk_pctrl_spec_pull_set_samereg(regmap, mt8695_spec_pupd,
		ARRAY_SIZE(mt8695_spec_pupd), pin, align, isup, r1r0);
}

static int mt8695_spec_pull_get(struct regmap *regmap, unsigned int pin)
{
	return mtk_spec_pull_get_samereg(regmap, mt8695_spec_pupd,
		ARRAY_SIZE(mt8695_spec_pupd), pin);
}

static const struct mtk_pin_ies_smt_set mt8695_smt_set[] = {
	MTK_PIN_IES_SMT_SPEC(0, 0, 0x880, 1),
	MTK_PIN_IES_SMT_SPEC(1, 8, 0x880, 2),
	MTK_PIN_IES_SMT_SPEC(9, 12, 0x880, 3),
	MTK_PIN_IES_SMT_SPEC(13, 14, 0x880, 9),
	MTK_PIN_IES_SMT_SPEC(19, 20, 0x880, 13),
	MTK_PIN_IES_SMT_SPEC(21, 24, 0x880, 8),
	MTK_PIN_IES_SMT_SPEC(25, 25, 0x880, 10),
	MTK_PIN_IES_SMT_SPEC(26, 26, 0x8b0, 2),
	MTK_PIN_IES_SMT_SPEC(27, 27, 0x8b0, 1),
	MTK_PIN_IES_SMT_SPEC(28, 28, 0x8b0, 5),
	MTK_PIN_IES_SMT_SPEC(29, 29, 0x8b0, 0),
	MTK_PIN_IES_SMT_SPEC(30, 33, 0x8b0, 7),
	MTK_PIN_IES_SMT_SPEC(34, 34, 0x8b0, 6),
	MTK_PIN_IES_SMT_SPEC(35, 35, 0x8b0, 14),
	MTK_PIN_IES_SMT_SPEC(36, 36, 0x8b0, 11),
	MTK_PIN_IES_SMT_SPEC(37, 44, 0x8b0, 13),
	MTK_PIN_IES_SMT_SPEC(45, 45, 0x8b0, 12),
	MTK_PIN_IES_SMT_SPEC(46, 46, 0x8b0, 15),
	MTK_PIN_IES_SMT_SPEC(47, 50, 0x880, 10),
	MTK_PIN_IES_SMT_SPEC(51, 51, 0x880, 7),
	MTK_PIN_IES_SMT_SPEC(52, 55, 0x880, 11),
	MTK_PIN_IES_SMT_SPEC(56, 57, 0x880, 12),
	MTK_PIN_IES_SMT_SPEC(58, 58, 0x8a0, 15),
	MTK_PIN_IES_SMT_SPEC(59, 59, 0x8a0, 14),
	MTK_PIN_IES_SMT_SPEC(60, 60, 0x890, 3),
	MTK_PIN_IES_SMT_SPEC(61, 61, 0x890, 4),
	MTK_PIN_IES_SMT_SPEC(62, 62, 0x890, 3),
	MTK_PIN_IES_SMT_SPEC(63, 65, 0x890, 4),
	MTK_PIN_IES_SMT_SPEC(66, 66, 0x8b0, 8),
	MTK_PIN_IES_SMT_SPEC(67, 70, 0x8b0, 10),
	MTK_PIN_IES_SMT_SPEC(71, 71, 0x8b0, 9),
	MTK_PIN_IES_SMT_SPEC(72, 73, 0x880, 14),
	MTK_PIN_IES_SMT_SPEC(74, 75, 0x880, 15),
	MTK_PIN_IES_SMT_SPEC(76, 77, 0x880, 5),
	MTK_PIN_IES_SMT_SPEC(78, 81, 0x8b0, 4),
	MTK_PIN_IES_SMT_SPEC(82, 82, 0x890, 0),
	MTK_PIN_IES_SMT_SPEC(83, 83, 0x890, 1),
	MTK_PIN_IES_SMT_SPEC(84, 85, 0x890, 6),
	MTK_PIN_IES_SMT_SPEC(86, 87, 0x8a0, 0),
	MTK_PIN_IES_SMT_SPEC(88, 88, 0x8a0, 3),
	MTK_PIN_IES_SMT_SPEC(89, 89, 0x8a0, 4),
	MTK_PIN_IES_SMT_SPEC(90, 90, 0x8a0, 3),
	MTK_PIN_IES_SMT_SPEC(91, 91, 0x8a0, 4),
	MTK_PIN_IES_SMT_SPEC(92, 92, 0x890, 7),
	MTK_PIN_IES_SMT_SPEC(93, 93, 0x890, 8),
	MTK_PIN_IES_SMT_SPEC(94, 95, 0x890, 7),
	MTK_PIN_IES_SMT_SPEC(96, 99, 0x890, 5),
	MTK_PIN_IES_SMT_SPEC(100, 100, 0x8a0, 8),
	MTK_PIN_IES_SMT_SPEC(101, 101, 0x8a0, 9),
	MTK_PIN_IES_SMT_SPEC(102, 102, 0x8a0, 8),
	MTK_PIN_IES_SMT_SPEC(103, 104, 0x8a0, 9),
	MTK_PIN_IES_SMT_SPEC(105, 105, 0x8a0, 6),
	MTK_PIN_IES_SMT_SPEC(106, 106, 0x8a0, 5),
	MTK_PIN_IES_SMT_SPEC(107, 107, 0x8a0, 6),
	MTK_PIN_IES_SMT_SPEC(108, 108, 0x8a0, 5),
	MTK_PIN_IES_SMT_SPEC(109, 110, 0x8a0, 12),
	MTK_PIN_IES_SMT_SPEC(111, 112, 0x8a0, 13),
	MTK_PIN_IES_SMT_SPEC(113, 116, 0x880, 6),
	MTK_PIN_IES_SMT_SPEC(117, 118, 0x880, 4),
	MTK_PIN_IES_SMT_SPEC(119, 119, 0x880, 7),
	MTK_PIN_IES_SMT_SPEC(120, 123, 0x890, 9),
	MTK_PIN_IES_SMT_SPEC(124, 125, 0x890, 10),
	MTK_PIN_IES_SMT_SPEC(126, 129, 0x890, 11),
	MTK_PIN_IES_SMT_SPEC(130, 135, 0x890, 10),
	MTK_PIN_IES_SMT_SPEC(136, 136, 0x890, 11),
	MTK_PIN_IES_SMT_SPEC(137, 137, 0x890, 10),
};

static const struct mtk_pin_ies_smt_set mt8695_ies_set[] = {
	MTK_PIN_IES_SMT_SPEC(0, 0, 0x840, 1),
	MTK_PIN_IES_SMT_SPEC(1, 8, 0x840, 2),
	MTK_PIN_IES_SMT_SPEC(9, 12, 0x840, 3),
	MTK_PIN_IES_SMT_SPEC(13, 14, 0x840, 9),
	MTK_PIN_IES_SMT_SPEC(19, 20, 0x840, 13),
	MTK_PIN_IES_SMT_SPEC(21, 24, 0x840, 8),
	MTK_PIN_IES_SMT_SPEC(25, 25, 0x840, 10),
	MTK_PIN_IES_SMT_SPEC(26, 26, 0x870, 2),
	MTK_PIN_IES_SMT_SPEC(27, 27, 0x870, 1),
	MTK_PIN_IES_SMT_SPEC(28, 28, 0x870, 5),
	MTK_PIN_IES_SMT_SPEC(29, 29, 0x870, 0),
	MTK_PIN_IES_SMT_SPEC(30, 33, 0x870, 7),
	MTK_PIN_IES_SMT_SPEC(34, 34, 0x870, 6),
	MTK_PIN_IES_SMT_SPEC(35, 35, 0x870, 14),
	MTK_PIN_IES_SMT_SPEC(36, 36, 0x870, 11),
	MTK_PIN_IES_SMT_SPEC(37, 44, 0x870, 13),
	MTK_PIN_IES_SMT_SPEC(45, 45, 0x870, 12),
	MTK_PIN_IES_SMT_SPEC(46, 46, 0x870, 15),
	MTK_PIN_IES_SMT_SPEC(47, 50, 0x840, 10),
	MTK_PIN_IES_SMT_SPEC(51, 51, 0x840, 7),
	MTK_PIN_IES_SMT_SPEC(52, 55, 0x840, 11),
	MTK_PIN_IES_SMT_SPEC(56, 57, 0x840, 12),
	MTK_PIN_IES_SMT_SPEC(58, 58, 0x860, 15),
	MTK_PIN_IES_SMT_SPEC(59, 59, 0x860, 14),
	MTK_PIN_IES_SMT_SPEC(60, 60, 0x850, 3),
	MTK_PIN_IES_SMT_SPEC(61, 61, 0x850, 4),
	MTK_PIN_IES_SMT_SPEC(62, 62, 0x850, 3),
	MTK_PIN_IES_SMT_SPEC(63, 65, 0x850, 4),
	MTK_PIN_IES_SMT_SPEC(66, 66, 0x870, 8),
	MTK_PIN_IES_SMT_SPEC(67, 70, 0x870, 10),
	MTK_PIN_IES_SMT_SPEC(71, 71, 0x870, 9),
	MTK_PIN_IES_SMT_SPEC(72, 73, 0x840, 14),
	MTK_PIN_IES_SMT_SPEC(74, 75, 0x840, 15),
	MTK_PIN_IES_SMT_SPEC(76, 77, 0x840, 5),
	MTK_PIN_IES_SMT_SPEC(78, 81, 0x870, 4),
	MTK_PIN_IES_SMT_SPEC(82, 82, 0x850, 0),
	MTK_PIN_IES_SMT_SPEC(83, 83, 0x850, 1),
	MTK_PIN_IES_SMT_SPEC(84, 85, 0x850, 6),
	MTK_PIN_IES_SMT_SPEC(86, 87, 0x860, 0),
	MTK_PIN_IES_SMT_SPEC(88, 88, 0x860, 3),
	MTK_PIN_IES_SMT_SPEC(89, 89, 0x860, 4),
	MTK_PIN_IES_SMT_SPEC(90, 90, 0x860, 3),
	MTK_PIN_IES_SMT_SPEC(91, 91, 0x860, 4),
	MTK_PIN_IES_SMT_SPEC(92, 92, 0x850, 7),
	MTK_PIN_IES_SMT_SPEC(93, 93, 0x850, 8),
	MTK_PIN_IES_SMT_SPEC(94, 95, 0x850, 7),
	MTK_PIN_IES_SMT_SPEC(96, 99, 0x850, 5),
	MTK_PIN_IES_SMT_SPEC(100, 100, 0x860, 8),
	MTK_PIN_IES_SMT_SPEC(101, 101, 0x860, 9),
	MTK_PIN_IES_SMT_SPEC(102, 102, 0x860, 8),
	MTK_PIN_IES_SMT_SPEC(103, 104, 0x860, 9),
	MTK_PIN_IES_SMT_SPEC(105, 105, 0x860, 6),
	MTK_PIN_IES_SMT_SPEC(106, 106, 0x860, 5),
	MTK_PIN_IES_SMT_SPEC(107, 107, 0x860, 6),
	MTK_PIN_IES_SMT_SPEC(108, 108, 0x860, 5),
	MTK_PIN_IES_SMT_SPEC(109, 110, 0x860, 12),
	MTK_PIN_IES_SMT_SPEC(111, 112, 0x860, 13),
	MTK_PIN_IES_SMT_SPEC(113, 116, 0x840, 6),
	MTK_PIN_IES_SMT_SPEC(117, 118, 0x840, 4),
	MTK_PIN_IES_SMT_SPEC(119, 119, 0x840, 7),
	MTK_PIN_IES_SMT_SPEC(120, 123, 0x850, 9),
	MTK_PIN_IES_SMT_SPEC(124, 125, 0x850, 10),
	MTK_PIN_IES_SMT_SPEC(126, 129, 0x850, 11),
	MTK_PIN_IES_SMT_SPEC(130, 135, 0x850, 10),
	MTK_PIN_IES_SMT_SPEC(136, 136, 0x850, 11),
	MTK_PIN_IES_SMT_SPEC(137, 137, 0x850, 10),
};

static int mt8695_ies_smt_set(struct regmap *regmap, unsigned int pin,
		unsigned char align, int value, enum pin_config_param arg)
{
	if (arg == PIN_CONFIG_INPUT_ENABLE)
		return mtk_pconf_spec_set_ies_smt_range(regmap, mt8695_ies_set,
			ARRAY_SIZE(mt8695_ies_set), pin, align, value);
	else if (arg == PIN_CONFIG_INPUT_SCHMITT_ENABLE)
		return mtk_pconf_spec_set_ies_smt_range(regmap, mt8695_smt_set,
			ARRAY_SIZE(mt8695_smt_set), pin, align, value);
	return -EINVAL;
}

static int mt8695_spec_ies_get(struct regmap *regmap, unsigned int pin)
{
	return mtk_spec_get_ies_smt_range(regmap, mt8695_ies_set,
		ARRAY_SIZE(mt8695_ies_set), pin);
}

static int mt8695_spec_smt_get(struct regmap *regmap, unsigned int pin)
{
	return mtk_spec_get_ies_smt_range(regmap, mt8695_smt_set,
		ARRAY_SIZE(mt8695_smt_set), pin);
}

static const struct mtk_drv_group_desc mt8695_drv_grp[] =  {
	/* E8E4E2  2/4/6/8/10/12/14/16 */
	MTK_DRV_GRP(2, 16, 0, 2, 2)
};

static const struct mtk_pin_drv_grp mt8695_pin_drv[] = {
	MTK_PIN_DRV_GRP(0, 0xb40, 0, 0),
	MTK_PIN_DRV_GRP(1, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(2, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(3, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(4, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(5, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(6, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(7, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(8, 0xb40, 4, 0),
	MTK_PIN_DRV_GRP(9, 0xb40, 8, 0),
	MTK_PIN_DRV_GRP(10, 0xb40, 8, 0),
	MTK_PIN_DRV_GRP(11, 0xb40, 8, 0),
	MTK_PIN_DRV_GRP(12, 0xb40, 8, 0),
	MTK_PIN_DRV_GRP(13, 0xbd0, 4, 0),
	MTK_PIN_DRV_GRP(14, 0xbd0, 4, 0),
	MTK_PIN_DRV_GRP(19, 0xb60, 4, 0),
	MTK_PIN_DRV_GRP(20, 0xb60, 8, 0),
	MTK_PIN_DRV_GRP(21, 0xb40, 12, 0),
	MTK_PIN_DRV_GRP(22, 0xb40, 12, 0),
	MTK_PIN_DRV_GRP(23, 0xb40, 12, 0),
	MTK_PIN_DRV_GRP(24, 0xb40, 12, 0),
	MTK_PIN_DRV_GRP(25, 0xb80, 0, 0),
	MTK_PIN_DRV_GRP(26, 0xbe0, 12, 0),
	MTK_PIN_DRV_GRP(27, 0xbe0, 8, 0),
	MTK_PIN_DRV_GRP(28, 0xbf0, 12, 0),
	MTK_PIN_DRV_GRP(29, 0xc00, 0, 0),
	MTK_PIN_DRV_GRP(30, 0xc00, 4, 0),
	MTK_PIN_DRV_GRP(31, 0xc00, 4, 0),
	MTK_PIN_DRV_GRP(32, 0xc00, 4, 0),
	MTK_PIN_DRV_GRP(33, 0xc00, 4, 0),
	MTK_PIN_DRV_GRP(34, 0xc00, 8, 0),
	MTK_PIN_DRV_GRP(35, 0xc00, 12, 0),
	MTK_PIN_DRV_GRP(36, 0xc10, 0, 0),
	MTK_PIN_DRV_GRP(37, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(38, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(39, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(40, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(41, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(42, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(43, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(44, 0xc10, 4, 0),
	MTK_PIN_DRV_GRP(45, 0xc10, 8, 0),
	MTK_PIN_DRV_GRP(46, 0xc10, 12, 0),
	MTK_PIN_DRV_GRP(47, 0xb50, 8, 0),
	MTK_PIN_DRV_GRP(48, 0xb50, 8, 0),
	MTK_PIN_DRV_GRP(49, 0xb50, 8, 0),
	MTK_PIN_DRV_GRP(50, 0xb50, 8, 0),
	MTK_PIN_DRV_GRP(51, 0xb60, 12, 0),
	MTK_PIN_DRV_GRP(52, 0xb50, 12, 0),
	MTK_PIN_DRV_GRP(53, 0xb50, 12, 0),
	MTK_PIN_DRV_GRP(54, 0xb50, 12, 0),
	MTK_PIN_DRV_GRP(55, 0xb50, 12, 0),
	MTK_PIN_DRV_GRP(56, 0xb60, 0, 0),
	MTK_PIN_DRV_GRP(57, 0xb60, 0, 0),
	MTK_PIN_DRV_GRP(58, 0xbe0, 0, 0),
	MTK_PIN_DRV_GRP(59, 0xbe0, 4, 0),
	MTK_PIN_DRV_GRP(60, 0xb80, 4, 0),
	MTK_PIN_DRV_GRP(61, 0xb80, 4, 0),
	MTK_PIN_DRV_GRP(62, 0xb80, 8, 0),
	MTK_PIN_DRV_GRP(63, 0xb80, 8, 0),
	MTK_PIN_DRV_GRP(64, 0xb80, 8, 0),
	MTK_PIN_DRV_GRP(65, 0xb80, 8, 0),
	MTK_PIN_DRV_GRP(66, 0xbf0, 0, 0),
	MTK_PIN_DRV_GRP(67, 0xbf0, 4, 0),
	MTK_PIN_DRV_GRP(68, 0xbf0, 4, 0),
	MTK_PIN_DRV_GRP(69, 0xbf0, 4, 0),
	MTK_PIN_DRV_GRP(70, 0xbf0, 4, 0),
	MTK_PIN_DRV_GRP(71, 0xbf0, 8, 0),
	MTK_PIN_DRV_GRP(72, 0xb70, 0, 1),
	MTK_PIN_DRV_GRP(73, 0xb70, 0, 1),
	MTK_PIN_DRV_GRP(74, 0xb70, 4, 0),
	MTK_PIN_DRV_GRP(75, 0xb70, 4, 0),
	MTK_PIN_DRV_GRP(76, 0xb50, 0, 0),
	MTK_PIN_DRV_GRP(77, 0xb50, 0, 0),
	MTK_PIN_DRV_GRP(78, 0xc20, 0, 0),
	MTK_PIN_DRV_GRP(79, 0xc20, 0, 0),
	MTK_PIN_DRV_GRP(80, 0xc20, 0, 0),
	MTK_PIN_DRV_GRP(81, 0xc20, 0, 0),
	MTK_PIN_DRV_GRP(82, 0xb70, 8, 0),
	MTK_PIN_DRV_GRP(83, 0xb70, 12, 0),
	MTK_PIN_DRV_GRP(84, 0xb90, 0, 0),
	MTK_PIN_DRV_GRP(85, 0xb90, 0, 0),
	MTK_PIN_DRV_GRP(86, 0xb90, 4, 0),
	MTK_PIN_DRV_GRP(87, 0xb90, 4, 0),
	MTK_PIN_DRV_GRP(88, 0xb90, 8, 0),
	MTK_PIN_DRV_GRP(89, 0xb90, 12, 0),
	MTK_PIN_DRV_GRP(90, 0xb90, 8, 0),
	MTK_PIN_DRV_GRP(91, 0xb90, 12, 0),
	MTK_PIN_DRV_GRP(92, 0xba0, 8, 0),
	MTK_PIN_DRV_GRP(93, 0xba0, 12, 0),
	MTK_PIN_DRV_GRP(94, 0xba0, 8, 0),
	MTK_PIN_DRV_GRP(95, 0xba0, 8, 0),
	MTK_PIN_DRV_GRP(96, 0xb80, 12, 0),
	MTK_PIN_DRV_GRP(97, 0xb80, 12, 0),
	MTK_PIN_DRV_GRP(98, 0xb80, 12, 0),
	MTK_PIN_DRV_GRP(99, 0xb80, 12, 0),
	MTK_PIN_DRV_GRP(100, 0xba0, 0, 0),
	MTK_PIN_DRV_GRP(101, 0xba0, 4, 0),
	MTK_PIN_DRV_GRP(102, 0xba0, 0, 0),
	MTK_PIN_DRV_GRP(103, 0xba0, 4, 0),
	MTK_PIN_DRV_GRP(104, 0xba0, 4, 0),
	MTK_PIN_DRV_GRP(105, 0xbc0, 0, 0),
	MTK_PIN_DRV_GRP(106, 0xbc0, 4, 0),
	MTK_PIN_DRV_GRP(107, 0xbc0, 0, 0),
	MTK_PIN_DRV_GRP(108, 0xbc0, 4, 0),
	MTK_PIN_DRV_GRP(109, 0xbb0, 0, 0),
	MTK_PIN_DRV_GRP(110, 0xbb0, 0, 0),
	MTK_PIN_DRV_GRP(111, 0xbb0, 4, 0),
	MTK_PIN_DRV_GRP(112, 0xbb0, 4, 0),
	MTK_PIN_DRV_GRP(113, 0xb50, 4, 0),
	MTK_PIN_DRV_GRP(114, 0xb50, 4, 0),
	MTK_PIN_DRV_GRP(115, 0xb50, 4, 0),
	MTK_PIN_DRV_GRP(116, 0xb50, 4, 0),
	MTK_PIN_DRV_GRP(117, 0xbb0, 8, 0),
	MTK_PIN_DRV_GRP(118, 0xbb0, 8, 0),
	MTK_PIN_DRV_GRP(119, 0xbb0, 12, 0),
	MTK_PIN_DRV_GRP(120, 0xbc0, 8, 0),
	MTK_PIN_DRV_GRP(121, 0xbc0, 8, 0),
	MTK_PIN_DRV_GRP(122, 0xbc0, 8, 0),
	MTK_PIN_DRV_GRP(123, 0xbc0, 8, 0),
	MTK_PIN_DRV_GRP(124, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(125, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(126, 0xbd0, 0, 0),
	MTK_PIN_DRV_GRP(127, 0xbd0, 0, 0),
	MTK_PIN_DRV_GRP(128, 0xbd0, 0, 0),
	MTK_PIN_DRV_GRP(129, 0xbd0, 0, 0),
	MTK_PIN_DRV_GRP(130, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(131, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(132, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(133, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(134, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(135, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(136, 0xbc0, 12, 0),
	MTK_PIN_DRV_GRP(137, 0xbc0, 12, 0),
};

static const struct mtk_pinctrl_devdata mt8695_pinctrl_data = {
	.pins = mtk_pins_mt8695,
	.npins = ARRAY_SIZE(mtk_pins_mt8695),
	.grp_desc = mt8695_drv_grp,
	.n_grp_cls = ARRAY_SIZE(mt8695_drv_grp),
	.pin_drv_grp = mt8695_pin_drv,
	.n_pin_drv_grps = ARRAY_SIZE(mt8695_pin_drv),
	.spec_driving_set = mt8695_spec_driving_set,
	.spec_driving_get = mt8695_spec_driving_get,
	.spec_rsel_set = mt8695_spec_rsel_set,
	.spec_rsel_get = mt8695_spec_rsel_get,
	.spec_pull_set = mt8695_spec_pull_set,
	.spec_pull_get = mt8695_spec_pull_get,
	.spec_ies_smt_set = mt8695_ies_smt_set,
	.spec_ies_get = mt8695_spec_ies_get,
	.spec_smt_get = mt8695_spec_smt_get,
	.dir_offset = 0x0000,
	.pullen_offset = 0x0100,
	.pullsel_offset = 0x0200,
	.dout_offset = 0x0300,
	.din_offset = 0x0400,
	.pinmux_offset = 0x0500,
	.type1_start = 138,
	.type1_end = 138,
	.port_shf = 4,
	.port_mask = 0xf,
	.port_align = 4,
	.eint_offsets = {
		.name = "mt8695_eint",
		.stat      = 0x000,
		.ack       = 0x040,
		.mask      = 0x080,
		.mask_set  = 0x0c0,
		.mask_clr  = 0x100,
		.sens      = 0x140,
		.sens_set  = 0x180,
		.sens_clr  = 0x1c0,
		.soft      = 0x200,
		.soft_set  = 0x240,
		.soft_clr  = 0x280,
		.pol       = 0x300,
		.pol_set   = 0x340,
		.pol_clr   = 0x380,
		.dom_en    = 0x400,
		.dbnc_ctrl = 0x500,
		.dbnc_set  = 0x600,
		.dbnc_clr  = 0x700,
		.port_mask = 0x7,
		.ports     = 5,
	},
	.ap_num = 145,
	.db_cnt = 100,
};

static int mt8695_pinctrl_probe(struct platform_device *pdev)
{
	return mtk_pctrl_init(pdev, &mt8695_pinctrl_data, NULL);
}

static const struct of_device_id mt8695_pctrl_match[] = {
	{
		.compatible = "mediatek,mt8695-pinctrl",
	},
	{ }
};
MODULE_DEVICE_TABLE(of, mt8695_pctrl_match);

static struct platform_driver mtk_pinctrl_driver = {
	.probe = mt8695_pinctrl_probe,
	.driver = {
		.name = "mediatek-mt8695-pinctrl",
		.of_match_table = mt8695_pctrl_match,
		.pm = &mtk_eint_pm_ops,
	},
};

static int __init mtk_pinctrl_init(void)
{
	return platform_driver_register(&mtk_pinctrl_driver);
}

arch_initcall(mtk_pinctrl_init);

