#ifndef __MDP_ROT_H__
#define __MDP_ROT_H__
#include "mdp_def.h"

/*
** fill mdp task for rot use.
*/
enum MDP_TASK_STATUS mdp_rot_fill_task(struct mdp_task_struct *pTask);

/* wait rot hardware idle */
enum MDP_TASK_STATUS mdp_rot_wait_hw_ready(struct mdp_task_struct *pTask);

/* trigger rot hardware work */
enum MDP_TASK_STATUS mdp_rot_trigger_hw_work(struct mdp_task_struct *pTask);

/*
** handle rot task done
*/
void mdp_rot_handle_task_done(struct mdp_task_struct *pTask);

/*
** handle rot task timeout
*/
void mdp_rot_handle_task_error(struct mdp_task_struct *pTask);

/* dump rot task */
void mdp_rot_dump_task(struct mdp_task_struct *pTask);


#endif /* endof __MDP_ROT_H__ */

