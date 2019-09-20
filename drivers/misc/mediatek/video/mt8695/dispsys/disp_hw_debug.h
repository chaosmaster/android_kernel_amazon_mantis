#ifndef __DISP_HW_DEBUG_H__
#define __DISP_HW_DEBUG_H__

#include <linux/types.h>

extern unsigned long fb_bus_addr;
extern void *fb_va_base;

void disp_hw_debug_init(void);
void disp_hw_debug_deinit(void);

#endif
