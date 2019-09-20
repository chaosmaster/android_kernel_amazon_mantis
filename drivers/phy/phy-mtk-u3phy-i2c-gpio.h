/*
 * Copyright (c) 2013-2015, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _U3PHY_IIC_GPIO_H_
#define _U3PHY_IIC_GPIO_H_

int u3phy_write_reg(void __iomem *i2c_port_base, unsigned char dev_id, unsigned char address, int value);
unsigned char u3phy_read_reg(void __iomem *i2c_port_base, unsigned char dev_id,  unsigned char address);
int u3phy_write_reg32(void __iomem *i2c_port_base, unsigned char dev_id, unsigned int addr, unsigned int data);
unsigned int u3phy_read_reg32(void __iomem *i2c_port_base, unsigned char dev_id, unsigned int addr);
unsigned int u3phy_read_reg32(void __iomem *i2c_port_base, unsigned char dev_id, unsigned int addr);
int u3phy_write_reg8(void __iomem *i2c_port_base, unsigned char dev_id, unsigned int addr, unsigned char data);
unsigned char u3phy_read_reg8(void __iomem *i2c_port_base, u8 dev_id, u32 addr);
unsigned int u3phy_readlmsk(void __iomem *i2c_port_base, unsigned char i2c_addr,
				unsigned int reg_addr32, unsigned int offset, unsigned int mask);
int u3phy_writelmsk(void __iomem *i2c_port_base, unsigned char i2c_addr,
			unsigned int reg_addr32, unsigned int offset, unsigned int mask, unsigned int data);
#endif
