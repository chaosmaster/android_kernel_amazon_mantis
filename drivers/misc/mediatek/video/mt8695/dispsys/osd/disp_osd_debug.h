#ifndef __DRV_OSD_DEBUG_H__
#define __DRV_OSD_DEBUG_H__

#include <linux/kernel.h>


extern struct osd_context_t osd;
void disp_osd_debug_init(void);
void disp_osd_debug_deinit(void);
unsigned int osd_dbg_log_level(void);
unsigned int osd_irq_log_level(void);
void osd_dump(void);



#endif				/* __DRV_OSD_DEBUG_H__ */
