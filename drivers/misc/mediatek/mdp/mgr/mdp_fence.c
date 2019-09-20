#include <linux/vmalloc.h>
#include "mdp_fence.h"
#include "mdp_log.h"
#include "mdp_core.h"
/*
** 1 hardware <--> 1 timeline
*/

/* mutex protect g_mdp_fence_index & p_timeline & g_mdp_fence_task_list */
static struct mutex p_timeline_mutex[MDP_TIMELINE_TYPE_MAX];

/* used to allocate fence value */
static uint32_t g_mdp_fence_index[MDP_TIMELINE_TYPE_MAX];

/* timeline array */
static struct sw_sync_timeline *p_timeline[MDP_TIMELINE_TYPE_MAX];

/* task list, record all task create on this timeline, sort by create time
** the fence must released as this order.
*/
static struct list_head g_mdp_fence_list[MDP_TIMELINE_TYPE_MAX];

/* waitqueue: sleep wait for other fence released */
wait_queue_head_t g_mdp_fence_wait_fence_released_queue[MDP_TIMELINE_TYPE_MAX];


/*
** create timeline array
*/
void mdp_fence_init(void)
{
	int i = 0;

	for (i = 0; i < MDP_TIMELINE_TYPE_MAX; i++) {
		/* init mutex */
		mutex_init(&p_timeline_mutex[i]);
		/* init list_head */
		INIT_LIST_HEAD(&g_mdp_fence_list[i]);
		/* init waitqueue*/
		init_waitqueue_head(&g_mdp_fence_wait_fence_released_queue[i]);
	}

	/* create timeline */
	p_timeline[MDP_TIMELINE_TYPE_IMGRESZ_ROT_INPUT] = timeline_create("mdp_imgresz_rot_input_timeline");
	p_timeline[MDP_TIMELINE_TYPE_IMGRESZ_ROT_OUTPUT] = timeline_create("mdp_imgresz_rot_output_timeline");
	p_timeline[MDP_TIMELINE_TYPE_IMGRESZ_NORMAL] = timeline_create("mdp_imgresz_normal_timeline");
	p_timeline[MDP_TIMELINE_TYPE_IMGRESZ_UFO_2_HW] = timeline_create("mdp_imgresz_ufo_2_hw_timeline");
	p_timeline[MDP_TIMELINE_TYPE_ROT] = timeline_create("mdp_rot_timeline");
	p_timeline[MDP_TIMELINE_TYPE_NR_INPUT] = timeline_create("mdp_input_nr_timeline");
	p_timeline[MDP_TIMELINE_TYPE_NR_OUTPUT] = timeline_create("mdp_output_nr_timeline");
}

/*
** destroy timeline array.
*/
void mdp_fence_destroy_timeline(void)
{
	int i = 0;

	for (i = 0; i < MDP_TIMELINE_TYPE_MAX; i++) {
		mutex_lock(&p_timeline_mutex[i]);
		timeline_destroy(p_timeline[i]);
		p_timeline[i] = NULL;
		mutex_unlock(&p_timeline_mutex[i]);
		mutex_destroy(&p_timeline_mutex[i]);
	}
}

bool mdp_fence_is_first_fence_in_list(struct mdp_fence_struct *pFence)
{
	bool is_first = false;

	mutex_lock(&p_timeline_mutex[pFence->type]);
	is_first = (g_mdp_fence_list[pFence->type].next == &pFence->list);
	mutex_unlock(&p_timeline_mutex[pFence->type]);
	return is_first;
}

/*
** get task input & output buffer fence timeline type
*/
enum MDP_TASK_STATUS mdp_fence_get_task_timeline(struct mdp_task_struct *pTask,
		enum MDP_TIMELINE_TYPE *p_src_timeline_type,
		enum MDP_TIMELINE_TYPE *p_dst_timeline_type)
{
	enum MDP_TIMELINE_TYPE src_timeline_type;
	enum MDP_TIMELINE_TYPE dst_timeline_type;

	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_NR:
		src_timeline_type = MDP_TIMELINE_TYPE_NR_INPUT;
		dst_timeline_type = MDP_TIMELINE_TYPE_NR_OUTPUT;
		break;
	case MDP_TASK_TYPE_ROT:
		src_timeline_type = MDP_TIMELINE_TYPE_ROT;
		dst_timeline_type = MDP_TIMELINE_TYPE_ROT;
		break;
	case MDP_TASK_TYPE_IMGRESZ:
		if (DP_COLOR_GET_UFP_ENABLE(pTask->mdp_command.src_buffer.color_format)) {
			src_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_UFO_2_HW;
			dst_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_UFO_2_HW;
		} else {
			src_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_NORMAL;
			dst_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_NORMAL;
		}
		break;
	case MDP_TASK_TYPE_IMGRESZ_ROT:
			src_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_ROT_INPUT;
			dst_timeline_type = MDP_TIMELINE_TYPE_IMGRESZ_ROT_OUTPUT;
		break;
	default:
		MDP_ERR("unknown task type:%d\n", pTask->mdp_command.task_type);
		return MDP_TASK_STATUS_UNKNOWN_TASK_TYPE;
	}
	*p_src_timeline_type = src_timeline_type;
	*p_dst_timeline_type = dst_timeline_type;

	return MDP_TASK_STATUS_OK;
}



/*
** release a task fence (input buffer & output buffer) 2 fence
** WARNING !!! YOU MUST RELEASE SRC BUFFER FENCE FIRST !!!! AVOID DEAD LOCK
*/
enum MDP_TASK_STATUS mdp_fence_put_task_fence(struct mdp_task_struct *pTask)
{
	struct mdp_fence_struct *p_src_fence = pTask->p_src_fence;
	struct mdp_fence_struct *p_dst_fence = pTask->p_dst_fence;
	int count = 0;
	struct mdp_fence_struct *p_temp_fence = NULL;

	DEFINE_WAIT_FUNC(srcwait, woken_wake_function);
	DEFINE_WAIT_FUNC(dstwait, woken_wake_function);


	/* release buffer fence */

	/* 1 wait the fence is the first fence in fence list */
	long timeout = msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS);

	add_wait_queue(&g_mdp_fence_wait_fence_released_queue[p_src_fence->type], &srcwait);
	while (1) {
		if (mdp_fence_is_first_fence_in_list(p_src_fence))
			break;
		timeout = wait_woken(&srcwait, TASK_INTERRUPTIBLE, timeout);
		if (timeout == 0)
			break;
	}
	remove_wait_queue(&g_mdp_fence_wait_fence_released_queue[p_src_fence->type], &srcwait);

	if (timeout == 0) {
		MDP_ERR("task %s wait fence release timeout\n", pTask->mdp_command.taskString);
		mutex_lock(&p_timeline_mutex[p_src_fence->type]);
		list_for_each_entry(p_temp_fence, &g_mdp_fence_list[p_src_fence->type], list) {
			MDP_ERR("task %s fence type[%d] name[%s] value[%d]\n",
				p_temp_fence->taskString,
				p_temp_fence->type,
				p_temp_fence->fence.name,
				p_temp_fence->fence.value);
		}
		mutex_unlock(&p_timeline_mutex[p_src_fence->type]);
		return MDP_TASK_STATUS_WAIT_INTERNAL_FENCE_TIMEOUT;
	}
	/* 2 ok to release buffer fence */
	mutex_lock(&p_timeline_mutex[p_src_fence->type]);
	mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_RELEASE_FENCE);

	/* remove fence from fence list */
	list_del_init(&p_src_fence->list);
	list_del_init(&p_dst_fence->list);
	wake_up(&g_mdp_fence_wait_fence_released_queue[p_src_fence->type]);

	/* fence is not released */
	count = p_src_fence->fence.value - p_timeline[p_src_fence->type]->value;
	MDP_LOG("task %s release src fence counter[%d]\n", pTask->mdp_command.taskString, count);
	while (count-- > 0)
		timeline_inc(p_timeline[p_src_fence->type], 1);

	count = p_dst_fence->fence.value - p_timeline[p_dst_fence->type]->value;
	MDP_LOG("task %s release fence counter[%d]\n", pTask->mdp_command.taskString, count);
	while (count-- > 0)
		timeline_inc(p_timeline[p_dst_fence->type], 1);

	mutex_unlock(&p_timeline_mutex[p_src_fence->type]);
	vfree(pTask->p_src_fence);
	vfree(pTask->p_dst_fence);
	pTask->p_src_fence = NULL;
	pTask->p_dst_fence = NULL;

	return MDP_TASK_STATUS_OK;
}


/*
** acquire a task fence (input buffer & output buffer) 2 fence
*/
enum MDP_TASK_STATUS mdp_fence_get_task_fence(struct mdp_task_struct *pTask)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct mdp_fence_struct *p_src_fence = NULL;
	struct mdp_fence_struct *p_dst_fence = NULL;

	enum MDP_TIMELINE_TYPE src_timeline_type = MDP_TIMELINE_TYPE_INVALID;
	enum MDP_TIMELINE_TYPE dst_timeline_type = MDP_TIMELINE_TYPE_INVALID;

	unsigned long flags;

	status = mdp_fence_get_task_timeline(pTask, &src_timeline_type, &dst_timeline_type);
	if (status != MDP_TASK_STATUS_OK)
		return status;


	p_src_fence = vmalloc(sizeof(struct mdp_fence_struct));
	if (p_src_fence == NULL)
		return MDP_TASK_STATUS_ALLOCATE_ERROR_BUFFER_FAIL;

	p_dst_fence = vmalloc(sizeof(struct mdp_fence_struct));
	if (p_dst_fence == NULL) {
		vfree(p_src_fence);
		return MDP_TASK_STATUS_ALLOCATE_ERROR_BUFFER_FAIL;
	}


	p_src_fence->type = src_timeline_type;
	p_dst_fence->type = dst_timeline_type;

	snprintf(p_src_fence->taskString, sizeof(p_src_fence->taskString), "%s", pTask->mdp_command.taskString);
	snprintf(p_dst_fence->taskString, sizeof(p_dst_fence->taskString), "%s", pTask->mdp_command.taskString);

	/* create input buffer fence */
	mutex_lock(&p_timeline_mutex[p_src_fence->type]);
	p_src_fence->fence.value = g_mdp_fence_index[p_src_fence->type]++;
	snprintf(p_src_fence->fence.name, sizeof(p_src_fence->fence.name), "mdp_input_%x_0x%x",
		p_timeline[p_src_fence->type]->value, p_src_fence->fence.value);
	fence_create(p_timeline[p_src_fence->type], &p_src_fence->fence);

	p_dst_fence->fence.value = g_mdp_fence_index[p_dst_fence->type]++;
	snprintf(p_dst_fence->fence.name, sizeof(p_dst_fence->fence.name), "mdp_output_%x_0x%x",
		p_timeline[p_dst_fence->type]->value, p_dst_fence->fence.value);
	fence_create(p_timeline[p_dst_fence->type], &p_dst_fence->fence);

	mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_CREATE_FENCE);

	/* add fence to fence list
	** note first add src fence, then add dst fence.
	** you should first release src fence, then release dst fence at the same order.
	*/
	list_add_tail(&p_src_fence->list, &g_mdp_fence_list[p_src_fence->type]);
	list_add_tail(&p_dst_fence->list, &g_mdp_fence_list[p_dst_fence->type]);

	pTask->p_src_fence = p_src_fence;
	pTask->p_dst_fence = p_dst_fence;

	/* add task to task list */
	spin_lock_irqsave(&global_mdp_task.trigger_imgresz_task_list.spin_lock, flags);
	list_add_tail(&pTask->list_entry, &global_mdp_task.trigger_imgresz_task_list.list);
	spin_unlock_irqrestore(&global_mdp_task.trigger_imgresz_task_list.spin_lock, flags);
	mutex_unlock(&p_timeline_mutex[p_src_fence->type]);

	return MDP_TASK_STATUS_OK;
}


/* wait fence released, passed from vdec buffer */
enum MDP_TASK_STATUS mdp_fence_wait_fence_release(int fd)
{
	struct sync_fence *fence;
	int ret = 0;

	/* fd < 0, means we don't need to wait this fence fd, this buffer has no fence, return OK */
	if (fd < 0)
		return MDP_TASK_STATUS_OK;

	/* get sync_fence structure by fence fd */
	fence = sync_fence_fdget(fd);

	/* wait fence for 1000ms */
	ret = sync_fence_wait(fence, 1000);
	if (ret < 0) {
		MDP_ERR("wait fence fd:%d timeout\n", fd);
		sync_fence_put(fence);
		return MDP_TASK_STATUS_WAIT_FENCE_TIMEOUT;
	}

	sync_fence_put(fence);

	return MDP_TASK_STATUS_OK;
}

