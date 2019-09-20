#ifndef __DDP_DEBUG_H__
#define __DDP_DEBUG_H__

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include "ddp_mmp.h"
#include "ddp_log.h"

#define STR_CONVERT(p, val, base, action)\
	do {			\
		int ret = 0;	\
		const char *tmp;	\
		tmp = strsep(p, ","); \
		if (tmp == NULL) \
			break; \
		if (strcmp(#base, "int") == 0)\
			ret = kstrtoint(tmp, 0, (int *)val); \
		else if (strcmp(#base, "uint") == 0)\
			ret = kstrtouint(tmp, 0, (unsigned int *)val); \
		else if (strcmp(#base, "ul") == 0)\
			ret = kstrtoul(tmp, 0, (unsigned long *)val); \
		if (ret != 0) {\
			DDPMSG("[ERROR]kstrtoint/kstrtouint/kstrtoul return error: %d\n" \
				"  file : %s, line : %d\n",		\
				ret, __FILE__, __LINE__);\
			action; \
		} \
	} while (0)



#define DDP_DEBUG_ROOT_DIR "dispsys"

void ddp_debug_init(void);
void ddp_debug_exit(void);
struct dentry *ddp_debug_get_root_dir(void);



#endif				/* __DDP_DEBUG_H__ */
