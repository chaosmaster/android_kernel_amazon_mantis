#include "mdp_rot.h"


/*
** fill mdp task for rot use.
*/
enum MDP_TASK_STATUS mdp_rot_fill_task(struct mdp_task_struct *pTask)
{
	/* TODO */
	return MDP_TASK_STATUS_OK;
}

/* wait rot hardware idle */
enum MDP_TASK_STATUS mdp_rot_wait_hw_ready(struct mdp_task_struct *pTask)
{
	/* TODO */
	return MDP_TASK_STATUS_OK;
}

/* trigger rot hardware work */
enum MDP_TASK_STATUS mdp_rot_trigger_hw_work(struct mdp_task_struct *pTask)
{
	/* TODO */
	return MDP_TASK_STATUS_OK;
}

/*
** handle rot task done
*/
void mdp_rot_handle_task_done(struct mdp_task_struct *pTask)
{

}

/*
** handle rot task timeout
*/
void mdp_rot_handle_task_error(struct mdp_task_struct *pTask)
{

}

/* dump rot task */
void mdp_rot_dump_task(struct mdp_task_struct *pTask)
{

}

