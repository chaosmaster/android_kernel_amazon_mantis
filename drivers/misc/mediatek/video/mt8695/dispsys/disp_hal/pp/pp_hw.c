/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


#include "disp_reg.h"
#include "pp_hw.h"

uint8_t u1RegRd1B(uintptr_t reg)
{
	return u1IO32Rd1B(reg);
}


uint16_t u2RegRd2B(uintptr_t reg)
{
	return u2IO32Rd2B(reg);
}

void vRegWt1B(uintptr_t reg, uint8_t val8)
{
	vIO32Wt1B(reg, val8);
}

void vRegWt1BMsk(uintptr_t reg, uint8_t val8, uint8_t msk8)
{
	vIO32Wt1BMsk(reg, val8, msk8);
}

void vRegWt2B(uintptr_t reg, uint16_t val16)
{
	vIO32Wt2B(reg, val16);
}

void vRegWt2BMsk(uintptr_t reg, uint16_t val16, uint16_t msk16)
{
	vIO32Wt2BMsk(reg, val16, msk16);
}

void vRegWt4BMsk(uintptr_t reg, uint32_t val32, uint32_t msk32)
{
	vIO32Wt4BMsk(reg, val32, msk32);
}

uint8_t u1IO32Rd1B(uintptr_t reg)
{
	unsigned long addr = reg & ~3;

	switch (reg & 3) {
	default:
	case 0:
		return (uint8_t) (ReadREG32(addr) >> 0) & 0xff;
	case 1:
		return (uint8_t) (ReadREG32(addr) >> 8) & 0xff;
	case 2:
		return (uint8_t) (ReadREG32(addr) >> 16) & 0xff;
	case 3:
		/* ASSERT((reg & 3) < 3); */
		return (uint8_t) (ReadREG32(addr) >> 24) & 0xff;
	}
}



uint16_t u2IO32Rd2B(uintptr_t reg)
{
	unsigned long addr = reg & ~3;

	switch (reg & 3) {
	default:
	case 0:
		return (uint16_t) (ReadREG32(addr) >> 0) & 0xffff;
	case 2:
		return (uint16_t) (ReadREG32(addr) >> 16) & 0xffff;
	case 1:
		return (uint16_t) (ReadREG32(addr) >> 8) & 0xffff;
	case 3:
		/* ASSERT((reg32 & 3) < 3); */
		return (uint16_t) (ReadREG32(addr) >> 24) & 0xff;
	}
}

void vIO32Wt1BMsk(uintptr_t reg, uint8_t val8, uint8_t msk8)
{
	uint32_t u4Val, u4Msk;
	uint8_t bByte;

	bByte = reg & 3;
	reg &= ~3;
	val8 &= msk8;
	u4Msk = ~(uint32_t) (msk8 << ((uint32_t) bByte << 3));

	u4Val = ReadREG32(reg);
	u4Val = ((u4Val & u4Msk) | ((uint32_t) val8 << (bByte << 3)));
	WriteREG32(reg, u4Val);

}


void vIO32Wt2BMsk(uintptr_t reg, uint16_t val16, uint16_t msk16)
{
	uint32_t u4Val, u4Msk;
	uint8_t bByte;

	bByte = reg & 3;
	/* ASSERT(bByte < 3); */

	reg &= ~3;
	val16 &= msk16;
	u4Msk = ~(uint32_t) (msk16 << ((uint32_t) bByte << 3));

	u4Val = ReadREG32(reg);
	u4Val = ((u4Val & u4Msk) | ((uint32_t) val16 << (bByte << 3)));
	WriteREG32(reg, u4Val);

}

void vIO32Wt4BMsk(uintptr_t reg, uint32_t val32, uint32_t msk32)
{
	/* ASSERT((reg & 3) == 0); */
	WriteREG32Msk(reg, val32, msk32);
}


void vIO32Wt1B(uintptr_t reg, uint8_t val8)
{
	vIO32Wt1BMsk(reg, val8, 0xff);
}

void vIO32Wt2B(uintptr_t reg, uint16_t val16)
{
	vIO32Wt2BMsk(reg, val16, 0xffff);
}
