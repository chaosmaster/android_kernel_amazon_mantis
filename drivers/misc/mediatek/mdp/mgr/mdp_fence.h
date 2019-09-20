#ifndef __MDP_FENCE_H__
#define __MDP_FENCE_H__
#include <mtk_sync.h> /* mtk own fence API */
#include <linux/types.h>
#include "mdp_def.h"

/*
** create a struct sw_sync_timeline
*/
void mdp_fence_init(void);

/*
** destroy a timeline.
*/
void mdp_fence_destroy_timeline(void);

/*
** release a task fence (input buffer & output buffer) 2 fence
** WARNING !!! YOU MUST RELEASE SRC BUFFER FENCE FIRST !!!! AVOID DEAD LOCK
*/
enum MDP_TASK_STATUS mdp_fence_put_task_fence(struct mdp_task_struct *pTask);

/*
** acquire a task fence (input buffer & output buffer) 2 fence
*/
enum MDP_TASK_STATUS mdp_fence_get_task_fence(struct mdp_task_struct *pTask);


/*
** get task input & output buffer fence timeline type
*/
enum MDP_TASK_STATUS mdp_fence_get_task_timeline(struct mdp_task_struct *pTask,
			enum MDP_TIMELINE_TYPE *p_src_timeline_type,
			enum MDP_TIMELINE_TYPE *p_dst_timeline_type);


/* wait fence released, passed from vdec buffer */
enum MDP_TASK_STATUS mdp_fence_wait_fence_release(int fd);

#endif /* endof __MDP_FENCE_H__ */

