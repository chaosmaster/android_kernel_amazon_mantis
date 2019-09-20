#ifndef __MDP_LOG_H__
#define __MDP_LOG_H__

#include <linux/types.h>
#include <linux/printk.h>
#include "mdp_proc_print.h"

#define LOG_TAG "MDP"

#include "disp_hw_log.h"

bool mdp_core_should_print_log(void);

#define MDP_ERR(string, args...) do { \
	pr_err("MDP_ERR: "string, ##args); \
	} while (0)


#define MDP_LOG(string, args...) do { \
	if (mdp_core_should_print_log()) { \
		pr_err("MDP_LOG: "string, ##args); \
	} \
} while (0)

#endif /* endof __MDP_LOG_H__ */

