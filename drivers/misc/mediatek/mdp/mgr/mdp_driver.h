#ifndef __MDP_DRIVER_H__
#define __MDP_DRIVER_H__

#include <linux/types.h>
bool mdp_driver_is_suspend(void);
void mdp_driver_set_suspend(bool suspend);

#endif /* endof __MDP_DRIVER_H__ */
