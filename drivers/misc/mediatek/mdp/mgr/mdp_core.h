#ifndef __MDP_CORE_H__
#define __MDP_CORE_H__
#include "mdp_def.h"
#define LOG_TAG "MDP"
#include "disp_hw_log.h"

/*
** enable MDP_LOG() function.
*/
bool mdp_core_should_print_log(void);

/*
** init global struct list & corresponding mutex
*/
void mdp_core_init_global_mdp_task(void);

/*
** init mdp task field
*/
void mdp_core_init_mdp_task(struct mdp_task_struct *pTask);

/*
** get a new mdp_task_struct from free_task_list
** if no free task, create a new one.
*/
struct mdp_task_struct *mdp_core_get_mdp_task(void);

/*
** release mdp_task_struct & put task to free_task_list
*/
enum MDP_TASK_STATUS mdp_core_put_mdp_task(struct mdp_task_struct *pTask);



/*
** fill mdp_task according different type.
*/
enum MDP_TASK_STATUS mdp_core_fill_mdp_task(struct mdp_task_struct *pTask);



/*
** call imgresz output to temp buffer & wait imgresz task done & recall rot to output
*/
int32_t mdp_core_handle_imgresz_rot_task(void *p);



/*
** mdp core init
*/
enum MDP_TASK_STATUS mdp_core_init(void);

/*
** handle mdp ioctl: MDP_IOCTL_EXEC_COMMAND
*/
enum MDP_TASK_STATUS mdp_core_handle_exec_command(struct mdp_command_struct *pCommand);


/*
** wait hardware task done
*/
enum MDP_TASK_STATUS mdp_core_wait_task_done(struct mdp_task_struct *pTask);

/*
** handle task done
*/
void mdp_core_handle_task_done(struct mdp_task_struct *pTask);

/*
** handle task error
*/
void mdp_core_handle_task_error(struct mdp_task_struct *pTask);

/*
** handle task
*/
int32_t mdp_core_handle_task(struct mdp_task_struct *pTask);

/* dump mdp task */
void mdp_core_dump_task(struct mdp_task_struct *pTask);


/* dump mdp_command_struct */
void mdp_core_dump_command(struct mdp_command_struct *pCommand);

/* mmp debug set mmp tag */
enum MDP_TASK_STATUS mdp_core_set_mmp_tag(struct mdp_task_struct *pTask, enum MMP_DISP_EVENT event);

void mdp_core_release_task(struct work_struct *work);


#endif /* endif __MDP_CORE_H__ */

