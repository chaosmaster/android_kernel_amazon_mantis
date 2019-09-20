#ifndef _DISP_REG_H_
#define _DISP_REG_H_

#include "disp_type.h"
#include <linux/types.h>


#define WriteREG32(_reg32_, _val_) (*(volatile unsigned int *)(_reg32_) = (_val_))

#define ReadREG32(reg32) (*(volatile unsigned int *)(reg32))


#define WriteREG32Msk(arg, val, msk) \
	WriteREG32((arg), (ReadREG32(arg) & (~((uint32_t)(msk)))) | \
	(((uint32_t)(val)) & ((uint32_t)(msk))))
#define REG_MASK(idx)           ((uint64_t)1LL << (idx))

#define IS_REG_SET(r, mask)     ((r) & (mask))
#define REG_SET(r, mask)        ((r) |= (mask))
#define REG_RESET(r, mask)      ((r) &= ~(mask))

#endif
