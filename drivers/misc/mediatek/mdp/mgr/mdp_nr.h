#ifndef __MDP_NR_H__
#define __MDP_NR_H__
#include "mdp_def.h"

/*
** fill mdp task for nr use.
*/
enum MDP_TASK_STATUS mdp_nr_fill_task(struct mdp_task_struct *pTask);

/*
** wait nr hardware idle
*/
enum MDP_TASK_STATUS mdp_nr_wait_hw_ready(struct mdp_task_struct *pTask);

/* trigger nr hardware work */
enum MDP_TASK_STATUS mdp_nr_trigger_hw_work(struct mdp_task_struct *pTask);


/* handle nr task done */
void mdp_nr_handle_task_done(struct mdp_task_struct *pTask);

/* handle nr task timeout */
void mdp_nr_handle_task_error(struct mdp_task_struct *pTask);

/* dump nr task */
void mdp_nr_dump_task(struct mdp_task_struct *pTask);


#endif /* endof __MDP_NR__ */

