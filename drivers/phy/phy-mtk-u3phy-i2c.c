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

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/delay.h>

#define PHY_TRUE 1
#define PHY_FALSE 0

#define SDA 0
#define SCL 1

#define INPUT 0
#define OUTPUT 1

#define SSUSB_FPGA_I2C_OUT_OFFSET 0
#define SSUSB_FPGA_I2C_IN_OFFSET  0x04

#define SSUSB_FPGA_I2C_SDA_OUT (1<<0)
#define SSUSB_FPGA_I2C_SDA_OEN (1<<1)
#define SSUSB_FPGA_I2C_SCL_OUT (1<<2)
#define SSUSB_FPGA_I2C_SCL_OEN (1<<3)

#define SSUSB_FPGA_I2C_SDA_IN_OFFSET 0
#define SSUSB_FPGA_I2C_SCL_IN_OFFSET 1

#define I2C_DELAY 1000

typedef unsigned char u8;
typedef unsigned int u32;
#if 0
static void i2c_dummy_delay(unsigned int count)
{
	do {
		count--;
	} while (count > 0);
}
#else
#define i2c_dummy_delay(count)    udelay(count)
#endif

static void gpio_set_direction(void *i2c_port_base, unsigned char gpio_dir, unsigned char gpio_pin)
{
	unsigned int temp;
	void *addr;

	addr = i2c_port_base + SSUSB_FPGA_I2C_OUT_OFFSET;
	temp = readl(addr);

	if (gpio_pin == SDA) {
		if (gpio_dir == OUTPUT) {
			temp |= SSUSB_FPGA_I2C_SDA_OEN;
			writel(temp, addr);
		} else {
			temp &= ~SSUSB_FPGA_I2C_SDA_OEN;
			writel(temp, addr);
		}
	} else {
		if (gpio_dir == OUTPUT) {
			temp |= SSUSB_FPGA_I2C_SCL_OEN;
			writel(temp, addr);
		} else {
			temp &= ~SSUSB_FPGA_I2C_SCL_OEN;
			writel(temp, addr);
		}
	}
}

static void gpio_set_value(void *i2c_port_base, unsigned char value, unsigned char gpio_pin)
{
	unsigned int temp;
	void *addr;

	addr = i2c_port_base + SSUSB_FPGA_I2C_OUT_OFFSET;
	temp = readl(addr);

	if (gpio_pin == SDA) {
		if (value == 1) {
			temp |= SSUSB_FPGA_I2C_SDA_OUT;
			writel(temp, addr);
		} else {
			temp &= ~SSUSB_FPGA_I2C_SDA_OUT;
			writel(temp, addr);
		}
	} else {
		if (value == 1) {
			temp |= SSUSB_FPGA_I2C_SCL_OUT;
			writel(temp, addr);
		} else {
			temp &= ~SSUSB_FPGA_I2C_SCL_OUT;
			writel(temp, addr);
		}
	}
}

static unsigned char gpio_get_value(void *i2c_port_base, unsigned char gpio_pin)
{
	unsigned char temp;
	void *addr;

	addr = i2c_port_base + SSUSB_FPGA_I2C_IN_OFFSET;
	temp = readl(addr);

	if (gpio_pin == SDA)
		temp = (temp >> SSUSB_FPGA_I2C_SDA_IN_OFFSET) & 0x01;
	else
		temp = (temp >> SSUSB_FPGA_I2C_SCL_IN_OFFSET) & 0x01;

	return temp;
}

static void i2c_stop(void *i2c_port_base)
{
	gpio_set_direction(i2c_port_base, OUTPUT, SDA);
	gpio_set_value(i2c_port_base, 0, SCL);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 0, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 1, SCL);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 1, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_direction(i2c_port_base, INPUT, SCL);
	gpio_set_direction(i2c_port_base, INPUT, SDA);
}

static void i2c_start(void *i2c_port_base) /* Prepare the SDA and SCL for sending/receiving */
{
	gpio_set_direction(i2c_port_base, OUTPUT, SCL);
	gpio_set_direction(i2c_port_base, OUTPUT, SDA);
	gpio_set_value(i2c_port_base, 1, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 1, SCL);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 0, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 0, SCL);
	i2c_dummy_delay(I2C_DELAY);
}

static u32 i2c_send_byte(void *i2c_port_base, u8 data) /* return 0 --> ack */
{
	int i, ack;

	gpio_set_direction(i2c_port_base, OUTPUT, SDA);

	for (i = 8; --i > 0;) {
		gpio_set_value(i2c_port_base, (data>>i)&0x01, SDA);
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(i2c_port_base,  1, SCL); /* high */
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(i2c_port_base,  0, SCL); /* low */
		i2c_dummy_delay(I2C_DELAY);
	}
	gpio_set_value(i2c_port_base, (data>>i)&0x01, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base,  1, SCL); /* high */
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base,  0, SCL); /* low */
	i2c_dummy_delay(I2C_DELAY);

	gpio_set_value(i2c_port_base, 0, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_direction(i2c_port_base, INPUT, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 1, SCL);
	i2c_dummy_delay(I2C_DELAY);
	ack = gpio_get_value(i2c_port_base, SDA); /* ack 1: error , 0:ok */
	gpio_set_value(i2c_port_base, 0, SCL);
	i2c_dummy_delay(I2C_DELAY);

	if (ack == 1)
		return PHY_FALSE;
	else
		return PHY_TRUE;
}

static void i2c_receive_byte(void *i2c_port_base, u8 *data, u8 ack)
{
	int i;
	u32 dataCache;

	dataCache = 0;
	gpio_set_direction(i2c_port_base, INPUT, SDA);

	for (i = 8; --i >= 0;) {
		dataCache <<= 1;
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(i2c_port_base, 1, SCL);
		i2c_dummy_delay(I2C_DELAY);
		dataCache |= gpio_get_value(i2c_port_base, SDA);
		gpio_set_value(i2c_port_base, 0, SCL);
		i2c_dummy_delay(I2C_DELAY);
	}

	gpio_set_direction(i2c_port_base, OUTPUT, SDA);
	gpio_set_value(i2c_port_base, ack, SDA);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 1, SCL);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(i2c_port_base, 0, SCL);
	i2c_dummy_delay(I2C_DELAY);
	*data = (u8)dataCache;
}


static int i2c_write_reg(void *i2c_port_base, u8 dev_id, u8 addr, u8 data)
{
	int acknowledge = 0;

	i2c_start(i2c_port_base);

	acknowledge = i2c_send_byte(i2c_port_base, (dev_id<<1) & 0xff);
	if (acknowledge)
		acknowledge = i2c_send_byte(i2c_port_base, addr);
	else
		return PHY_FALSE;

	acknowledge = i2c_send_byte(i2c_port_base, data);
	if (acknowledge) {
		i2c_stop(i2c_port_base);
		return PHY_TRUE;
	} else {
		return PHY_FALSE;
	}
}

static int i2c_read_reg(void *i2c_port_base, u8 dev_id, u8 addr, u8 *data)
{
	int acknowledge = 0;

	i2c_start(i2c_port_base);

	acknowledge = i2c_send_byte(i2c_port_base, (dev_id<<1) & 0xff);
	if (acknowledge)
		acknowledge = i2c_send_byte(i2c_port_base, addr);
	else
		return PHY_FALSE;

	i2c_start(i2c_port_base);

	acknowledge = i2c_send_byte(i2c_port_base, ((dev_id<<1) & 0xff) | 0x01);
	if (acknowledge)
		i2c_receive_byte(i2c_port_base, data, 1);  /* ack 0: ok , 1 error */
	else
		return PHY_FALSE;

	i2c_stop(i2c_port_base);

	return acknowledge;
}

int u3phy_write_reg(void *i2c_port_base, u8 dev_id, u8 address, int value)
{
	int ret;

	ret = i2c_write_reg(i2c_port_base, dev_id, address, value);
	if (ret == PHY_FALSE) {
		pr_err("Write failed(dev_id: %x, addr: 0x%x, val: 0x%x)\n", dev_id, address, value);
		return PHY_FALSE;
	}

	return PHY_TRUE;
}

unsigned char u3phy_read_reg(void *i2c_port_base, u8 dev_id,  u8 address)
{
	u8 buf;
	int ret;

	ret = i2c_read_reg(i2c_port_base, dev_id, address, &buf);
	if (ret == PHY_FALSE) {
		pr_err("Read failed(dev_id: %x, addr: 0x%x)\n", dev_id, address);
		return PHY_FALSE;
	}
	ret = buf;
	return ret;
}

int u3phy_write_reg32(void *i2c_port_base, u8 dev_id, u32 addr, u32 data)
{
	u8 addr8;
	u8 data_0, data_1, data_2, data_3;

	addr8 = addr & 0xff;
	data_0 = data & 0xff;
	data_1 = (data>>8) & 0xff;
	data_2 = (data>>16) & 0xff;
	data_3 = (data>>24) & 0xff;

	u3phy_write_reg(i2c_port_base, dev_id, addr8, data_0);
	u3phy_write_reg(i2c_port_base, dev_id, addr8+1, data_1);
	u3phy_write_reg(i2c_port_base, dev_id, addr8+2, data_2);
	u3phy_write_reg(i2c_port_base, dev_id, addr8+3, data_3);

	return 0;
}

unsigned int u3phy_read_reg32(void *i2c_port_base, u8 dev_id, u32 addr)
{
	u8 addr8;
	u32 data;

	addr8 = addr & 0xff;

	data = u3phy_read_reg(i2c_port_base, dev_id, addr8);
	data |= (u3phy_read_reg(i2c_port_base, dev_id, addr8+1) << 8);
	data |= (u3phy_read_reg(i2c_port_base, dev_id, addr8+2) << 16);
	data |= (u3phy_read_reg(i2c_port_base, dev_id, addr8+3) << 24);

	return data;
}


int u3phy_write_reg8(void *i2c_port_base, u8 dev_id, u32 addr, u8 data)
{
	u8 addr8;

	addr8 = addr & 0xff;
	u3phy_write_reg(i2c_port_base, dev_id, addr8, data);

	return PHY_TRUE;
}

unsigned char u3phy_read_reg8(void *i2c_port_base, u8 dev_id, u32 addr)
{
	u8 addr8;
	u32 data;

	addr8 = addr & 0xff;
	data = u3phy_read_reg(i2c_port_base, dev_id, addr8);

	return data;
}

unsigned int u3phy_readlmsk(void *i2c_port_base, unsigned char i2c_addr, unsigned int reg_addr32,
	unsigned int offset, unsigned int mask)
{
	return ((u3phy_read_reg32(i2c_port_base, i2c_addr, reg_addr32) & mask) >> offset);
}

int u3phy_writelmsk(void *i2c_port_base, unsigned char i2c_addr, unsigned int reg_addr32,
	unsigned int offset, unsigned int mask, unsigned int data)
{
	unsigned int cur_value;
	unsigned int new_value;

	cur_value = u3phy_read_reg32(i2c_port_base, i2c_addr, reg_addr32);
	new_value = (cur_value & (~mask)) | ((data << offset) & mask);
	u3phy_write_reg32(i2c_port_base, i2c_addr, reg_addr32, new_value);

	return 0;
}
