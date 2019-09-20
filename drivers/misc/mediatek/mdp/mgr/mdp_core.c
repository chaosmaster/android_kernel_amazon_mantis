#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>

#include "mdp_def.h"
#include "mdp_core.h"
#include "mdp_log.h"
#include "mdp_imgresz.h"
#include "mdp_rot.h"
#include "mdp_nr.h"
#include "mdp_buffer.h"
#include "mdp_fence.h"
#include "mdp_cli.h"

#include "mdp_api.h"

struct global_mdp_task_struct global_mdp_task;

/*
** enable MDP_LOG() function.
*/
bool mdp_core_should_print_log(void)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	/* config by proc/mdp/log_enable node */
	return mdp_cli_get()->enable_log;
#else
	return false;
#endif
}

static struct workqueue_struct *config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_MAX];
static struct workqueue_struct *trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_MAX];
static struct workqueue_struct *release_wq;

/* for MMP use. */
enum MDP_TASK_STATUS mdp_core_copy_mmp_data(struct mdp_task_struct *pTask, struct mmp_mdp_task_struct *pMmpData)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->enable_mmp_debug)
		return MDP_TASK_STATUS_OK;
#else
	return MDP_TASK_STATUS_OK;
#endif

	WARN_ON(pMmpData == NULL);
	WARN_ON(pTask == NULL);
	pMmpData->current_state = pTask->current_state;
	if (pTask->p_src_fence)
		pMmpData->src_release_fence = pTask->p_src_fence->fence.fence;
	if (pTask->p_dst_fence)
		pMmpData->dst_release_fence = pTask->p_dst_fence->fence.fence;
	memcpy(pMmpData->taskString, pTask->mdp_command.taskString, sizeof(pTask->mdp_command.taskString));
	memcpy(&pMmpData->mdp_command, &pTask->mdp_command, sizeof(pTask->mdp_command));

	return MDP_TASK_STATUS_OK;
}

enum MDP_TASK_STATUS mdp_core_set_mmp_tag(struct mdp_task_struct *pTask, enum MMP_DISP_EVENT event)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->enable_mmp_debug)
		return MDP_TASK_STATUS_OK;
#else
	return MDP_TASK_STATUS_OK;
#endif

	do {
		struct mmp_mdp_task_struct mmp_task;

		mdp_core_copy_mmp_data(pTask, &mmp_task);

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
		DISP_MMP_STRUCT(event, &mmp_task, struct mmp_mdp_task_struct);
#endif

	} while (0);

	return MDP_TASK_STATUS_OK;
}

/*
** mdp core init
*/
enum MDP_TASK_STATUS mdp_core_init(void)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	do {
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_ACQUIRE_TEMP_BUFFER] =
			create_singlethread_workqueue("MDP_CFG_ACQUIRE_TEMP_BUFFER");
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_NORMAL] =
			create_singlethread_workqueue("MDP_CFG_IMGRESZ_NORMAL");
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_UFO_1HW] =
			create_singlethread_workqueue("MDP_CFG_IMGRESZ_UFO_1HW");
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_UFO_2HW] =
			create_singlethread_workqueue("MDP_CFG_IMGRESZ_UFO_2HW");
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_NR] =
			create_singlethread_workqueue("MDP_CFG_NR");
		config_wq[MDP_CONFIG_WORK_QUEUE_TYPE_ROT] =
			create_singlethread_workqueue("MDP_CFG_ROT");
		trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW0] =
			create_workqueue("MDP_TRI_IMGRESZ_HW0");
		trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW1] =
			create_workqueue("MDP_TRI_IMGRESZ_HW1");
		mdp_core_init_global_mdp_task();
		status = mdp_imgresz_init();
	} while (0);

	return status;
}

/*
** init global struct list & corresponding mutex
*/
void mdp_core_init_global_mdp_task(void)
{
	mutex_init(&global_mdp_task.free_task_list.mutex_lock);
	INIT_LIST_HEAD(&global_mdp_task.free_task_list.list);

	mutex_init(&global_mdp_task.acquire_temp_buffer_task_list.mutex_lock);
	INIT_LIST_HEAD(&global_mdp_task.acquire_temp_buffer_task_list.list);

	mutex_init(&global_mdp_task.imgresz_normal_prepare_task_list.mutex_lock);
	INIT_LIST_HEAD(&global_mdp_task.imgresz_normal_prepare_task_list.list);

	mutex_init(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.mutex_lock);
	INIT_LIST_HEAD(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.list);


	mutex_init(&global_mdp_task.trigger_imgresz_task_list.mutex_lock);
	spin_lock_init(&global_mdp_task.trigger_imgresz_task_list.spin_lock);
	INIT_LIST_HEAD(&global_mdp_task.trigger_imgresz_task_list.list);

	mutex_init(&global_mdp_task.imgresz_ufo_prepare_task_list.mutex_lock);
	INIT_LIST_HEAD(&global_mdp_task.imgresz_ufo_prepare_task_list.list);

	mutex_init(&global_mdp_task.get_imgresz_ticket_mutex);
	init_waitqueue_head(&global_mdp_task.mdp_core_wait_imgresz_hw_idle_queue);
	init_waitqueue_head(&global_mdp_task.mdp_core_wait_imgresz_ticket_queue);
	init_waitqueue_head(&global_mdp_task.mdp_core_wait_task_done_queue);
	init_waitqueue_head(&global_mdp_task.imgresz_wait_trigger_list_not_empty);
}

/*
** init mdp task field
*/
void mdp_core_init_mdp_task(struct mdp_task_struct *pTask)
{
	pTask->current_state = MDP_TASK_STATE_FREE;
	pTask->mdp_command.task_type = MDP_TASK_TYPE_UNKNOWN;
	INIT_LIST_HEAD(&pTask->list_entry);
	memset(&pTask->task, 0, sizeof(pTask->task));
	memset(&pTask->mdp_command, 0, sizeof(pTask->mdp_command));
	pTask->p_src_fence = NULL;
	pTask->p_dst_fence = NULL;
}

/*
** get a new mdp_task_struct from free_task_list
** if no free task, create a new one.
*/
struct mdp_task_struct *mdp_core_get_mdp_task(void)
{
	struct mdp_task_struct *pTask = NULL;

	mutex_lock(&global_mdp_task.free_task_list.mutex_lock);
	/* free_task_list is empty */
	if (list_empty(&global_mdp_task.free_task_list.list)) {
		mutex_unlock(&global_mdp_task.free_task_list.mutex_lock);
		pTask = vmalloc(sizeof(struct mdp_task_struct));
		if (pTask == NULL) {
			MDP_ERR("allocate mdp_task_struct failed\n");
			return NULL;
		}
		mdp_core_init_mdp_task(pTask);
		return pTask;
	}

	/* free_task_list is not empty */
	pTask = list_first_entry(&global_mdp_task.free_task_list.list, struct mdp_task_struct, list_entry);
	list_del_init(&pTask->list_entry);
	mutex_unlock(&global_mdp_task.free_task_list.mutex_lock);
	mdp_core_init_mdp_task(pTask);

	return pTask;
}

/*
** release mdp_task_struct & put task to free_task_list
** release temp buffer
** release task fence
*/
enum MDP_TASK_STATUS mdp_core_put_mdp_task(struct mdp_task_struct *pTask)
{
	do {
		if (mdp_proc_get_time_in_us(pTask->time.start, pTask->time.releaseFence) > 16670) {
			char show_buffer[100] = {0};

			snprintf(show_buffer, sizeof(show_buffer),
				"task:%s fence_time[%lld]us start:[0x%llx] fence:[0x%llx]\n",
				pTask->mdp_command.taskString,
				mdp_proc_get_time_in_us(pTask->time.start, pTask->time.releaseFence),
				pTask->time.start,
				pTask->time.releaseFence);

			MDP_ERR("%s", show_buffer);
			mdp_print(show_buffer);
		}

		/* 3 release ion handle */
		mdp_buffer_release_ion_handle(&pTask->mdp_command.src_buffer, &pTask->src_ion_handle);
		mdp_buffer_release_ion_handle(&pTask->mdp_command.dst_buffer, &pTask->dst_ion_handle);

	} while (0);

	pTask->time.done = sched_clock();
	mdp_proc_add_time_item(pTask);

	mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_BUF_CONFIG_END);

	/* move current task to free task list */
	pTask->current_state = MDP_TASK_STATE_FREE;

	list_del_init(&pTask->list_entry);

	mutex_lock(&global_mdp_task.free_task_list.mutex_lock);
	list_add_tail(&pTask->list_entry, &global_mdp_task.free_task_list.list);
	mutex_unlock(&global_mdp_task.free_task_list.mutex_lock);

	return MDP_TASK_STATUS_OK;
}


/*
** fill mdp_task according different type.
*/
enum MDP_TASK_STATUS mdp_core_fill_mdp_task(struct mdp_task_struct *pTask)
{
	if (pTask == NULL) {
		MDP_ERR("function:%s invalid param\n", __func__);
		return MDP_TASK_STATUS_INVALID_PARAM;
	}

	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_IMGRESZ:
		return mdp_imgresz_fill_task(pTask);
	case MDP_TASK_TYPE_ROT:
		return mdp_rot_fill_task(pTask);
	case MDP_TASK_TYPE_NR:
		return mdp_nr_fill_task(pTask);
	default:
		MDP_ERR("invalidate mdp task type\n");
		return MDP_TASK_STATUS_INVALID_TASK_TYPE;
	}

	MDP_ERR("invalidate mdp task type\n");
	return MDP_TASK_STATUS_INVALID_TASK_TYPE;
}

enum MDP_TASK_STATUS mdp_core_wait_hw_ready(struct mdp_task_struct *pTask)
{
	if (pTask == NULL) {
		MDP_ERR("function:%s invalid param\n", __func__);
		return MDP_TASK_STATUS_INVALID_PARAM;
	}

	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_IMGRESZ:
		return mdp_imgresz_wait_hardware_ready(pTask);
	case MDP_TASK_TYPE_ROT:
		return mdp_rot_wait_hw_ready(pTask);
	case MDP_TASK_TYPE_NR:
		return mdp_nr_wait_hw_ready(pTask);
	default:
		MDP_ERR("invalidate mdp task type\n");
		return MDP_TASK_STATUS_INVALID_TASK_TYPE;
	}

	MDP_ERR("invalidate mdp task type\n");
	return MDP_TASK_STATUS_INVALID_TASK_TYPE;
}

enum MDP_TASK_STATUS mdp_core_wait_buffer_ready(struct mdp_task_struct *pTask)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	do {
		status = mdp_fence_wait_fence_release(pTask->mdp_command.src_buffer.wait_fence_fd);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("wait src buffer fence fd error:%d\n", status);
			break;
		}
		status = mdp_fence_wait_fence_release(pTask->mdp_command.dst_buffer.wait_fence_fd);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("wait dst buffer fence fd error:%d\n", status);
			break;
		}
	} while (0);

	return status;
}


enum MDP_TASK_STATUS mdp_core_trigger_hw_work(struct mdp_task_struct *pTask)
{
	if (pTask == NULL) {
		MDP_ERR("function:%s invalid param\n", __func__);
		return MDP_TASK_STATUS_INVALID_PARAM;
	}

	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_IMGRESZ:
		return mdp_imgresz_trigger_hw_work(pTask);
#if 0
	case MDP_TASK_TYPE_IMGRESZ_ROT:
		return mdp_imgresz_trigger_hw_work(pTask);
#endif
	case MDP_TASK_TYPE_ROT:
		return mdp_rot_trigger_hw_work(pTask);
	case MDP_TASK_TYPE_NR:
		return mdp_nr_trigger_hw_work(pTask);
	default:
		MDP_ERR("invalidate mdp task type\n");
		return MDP_TASK_STATUS_INVALID_TASK_TYPE;
	}

	MDP_ERR("invalidate mdp task type\n");
	return MDP_TASK_STATUS_INVALID_TASK_TYPE;
}

int mdp_release_frame_count;	/* debug mdp handle done buffer */


/*
** wait hardware task done
*/
enum MDP_TASK_STATUS mdp_core_wait_task_done(struct mdp_task_struct *pTask)
{
	int ret = 0;
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	if (pTask == NULL) {
		MDP_ERR("%s invalid param\n", __func__);
		return MDP_TASK_STATUS_NULL_POINTER_ACCESS;
	}
	ret = wait_event_timeout(global_mdp_task.mdp_core_wait_task_done_queue,
			pTask->current_state == MDP_TASK_STATE_DONE ||
			pTask->current_state == MDP_TASK_STATE_ERROR ||
			pTask->current_state == MDP_TASK_STATE_NR_OUTPUT_RELEASED,
			msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS));


	if (ret == 0) {
		pTask->current_state = MDP_TASK_STATE_TIMEOUT;
		mdp_core_handle_task_error(pTask);
		status = MDP_TASK_STATUS_WAIT_TASK_TIMEOUT;
	} else if (pTask->current_state == MDP_TASK_STATE_ERROR) {
		mdp_core_handle_task_error(pTask);
		status = MDP_TASK_STATUS_HANDLE_TASK_ERROR;
	} else {
		mdp_core_handle_task_done(pTask);
		status = MDP_TASK_STATUS_OK;
	}

	return status;
}

/*
** handle task done
*/
void mdp_core_handle_task_done(struct mdp_task_struct *pTask)
{
	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_NR:
		mdp_nr_handle_task_done(pTask);
		break;
	case MDP_TASK_TYPE_ROT:
		mdp_rot_handle_task_done(pTask);
		break;
	case MDP_TASK_TYPE_IMGRESZ:
		mdp_imgresz_handle_task_done(pTask);
		break;
	case MDP_TASK_TYPE_IMGRESZ_ROT:
		/* imgresz + rot case, imgresz hw is done */
		break;
	default:
		/* TODO: imgresz + rot is not supported */
		MDP_ERR("unrecognised task type:%d\n", pTask->mdp_command.task_type);
		break;
	}
}


/*
** handle task timeout
*/
void mdp_core_handle_task_error(struct mdp_task_struct *pTask)
{
	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_NR:
		mdp_nr_handle_task_error(pTask);
		break;
	case MDP_TASK_TYPE_ROT:
		mdp_rot_handle_task_error(pTask);
		break;
	case MDP_TASK_TYPE_IMGRESZ:
		mdp_imgresz_handle_task_error(pTask);
		break;
	default:
		/* TODO: imgresz + rot is not supported */
		MDP_ERR("unrecognised task type:%d\n", pTask->mdp_command.task_type);
		break;
	}
}

/*
** TODO: we need to allocate temp buffer.
** call imgresz output to temp buffer & wait imgresz task done & recall rot to output
*/
int32_t mdp_core_handle_imgresz_rot_task(void *p)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct mdp_command_struct command;
	struct mdp_fence_struct *p_src_fence = NULL;
	struct mdp_fence_struct *p_dst_fence = NULL;
	struct mdp_temp_buffer_struct *p_temp_buffer = NULL;
	struct mdp_task_struct *pTask = (struct mdp_task_struct *)p;
	bool is_secure = false;

	do {
		/* for imgresz + rot case, allocate temp buffer */
		status = mdp_temp_buffer_get_buffer(&p_temp_buffer);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("acquire mdp temp buffer error:%d\n", status);
			break;
		}

		/* save information */
		command = pTask->mdp_command;
		p_src_fence = pTask->p_src_fence;
		p_dst_fence = pTask->p_dst_fence;
		is_secure = pTask->mdp_command.is_secure;

		/* 2 modify mdp_command_struct to mdp_task_struct */
		pTask->mdp_command.task_type = MDP_TASK_TYPE_IMGRESZ;
		pTask->mdp_command.is_secure = is_secure;
		pTask->p_src_fence = p_src_fence;
		pTask->p_dst_fence = NULL; /* temp buffer no need to release fence */
		pTask->mdp_command.dst_buffer.wait_fence_fd = -1; /* temp buffer no need to wait fence */
		/* TODO */
		/* we need to add dst_buffer info */
		/* pTask->mdp_command.dst_buffer. */



		status = mdp_core_handle_task(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("execute imgresz work fail:%d\n", status);
			break;
		}

		/* 1 acquire a free mdp_task_struct */
		pTask = mdp_core_get_mdp_task();

		/* 2 modify mdp_command_struct */
		pTask->mdp_command.task_type = MDP_TASK_TYPE_ROT;
		pTask->mdp_command.is_secure = is_secure;
		pTask->p_src_fence = NULL;	 /* temp buffer no need to release fence */
		pTask->p_dst_fence = p_dst_fence;
		pTask->mdp_command.src_buffer.wait_fence_fd = -1; /* temp buffer no need to wait fence */
		/* TODO */
		/* we need to add src_buffer info */
		/* pTask->mdp_command.src_buffer. */

		status = mdp_core_handle_task(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("execute nr work fail:%d\n", status);
			break;
		}
	} while (0);

	mdp_temp_buffer_put_buffer(p_temp_buffer);

	return 0;
}

void mdp_core_release_task(struct work_struct *work)
{
	struct mdp_task_struct *pTask = NULL;
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	pTask = container_of(work, struct mdp_task_struct, task_work);
	if (pTask == NULL) {
		MDP_ERR("get Task NULL\n");
		return;
	}
	status = mdp_core_put_mdp_task(pTask);
	if (status != MDP_TASK_STATUS_OK)
		MDP_ERR("release task %s fail:%d\n", pTask->mdp_command.taskString, status);
}


void mdp_core_trigger_wait_task_done(struct work_struct *work)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct mdp_task_struct *pTask = NULL;

	pTask = container_of(work, struct mdp_task_struct, task_work);
	if (pTask == NULL) {
		MDP_ERR("get Task NULL\n");
		return;
	}

	status = mdp_core_trigger_hw_work(pTask);

	if (status == MDP_TASK_STATUS_OK)
		MDP_LOG("mdp_core_handle_task %s OK\n", pTask->mdp_command.taskString);
	else
		MDP_ERR("mdp_core_handle_task %s fail, status:%d\n", pTask->mdp_command.taskString, status);

	MDP_LOG("mdp exec time:%lld\n", mdp_proc_get_time_in_us(pTask->time.trigger, pTask->time.gotIRQ));

	INIT_WORK(&pTask->task_work, mdp_core_release_task);
	queue_work(release_wq, &pTask->task_work);
	if (status != MDP_TASK_STATUS_OK)
		MDP_ERR("release task %s fail:%d\n", pTask->mdp_command.taskString, status);

	DISP_MMP(MMP_DISP_MDP, MMP_END, 0, 0);
}

/*
** handle task
*/
int32_t mdp_core_handle_task(struct mdp_task_struct *pTask)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	do {
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_BUF_FILL);
		/* fill mdp task info, prepare buffer info */
		status = mdp_core_fill_mdp_task(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("task %s fill mdp task error:%d\n", pTask->mdp_command.taskString, status);
			break;
		}

		/* wait hardware ready(idle) */
#if 0
		status = mdp_core_wait_hw_ready(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("task %s wait hardware idle error:%d\n", pTask->mdp_command.taskString, status);
			break;
		}
#endif
#if 0
		/* wait buffer fence passed from userspace release */
		status = mdp_core_wait_buffer_ready(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("task %s wait buffer ready error:%d\n", pTask->mdp_command.taskString, status);
			break;
		}
#endif

		/* switch thread to trigger & wait task done. */
		INIT_WORK(&pTask->task_work, mdp_core_trigger_wait_task_done);
		queue_work(trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW0], &pTask->task_work);
		#if 0
		if ((pTask->task.imgresz_task.ticket & 0x1) == 1)	/* use HW0 */
			queue_work(trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW0], &pTask->task_work);
		else
			queue_work(trigger_wq[MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW1], &pTask->task_work);
		#endif
	} while (0);

	/* for error handle release pTask */
	if (status != MDP_TASK_STATUS_OK) {
		status = mdp_core_put_mdp_task(pTask);
		if (status != MDP_TASK_STATUS_OK)
			MDP_ERR("release task %s fail:%d\n", pTask->mdp_command.taskString, status);
	}

	return 0;
}

void mdp_core_handle_ioctl_command(struct work_struct *work)
{
	struct mdp_task_struct *pTask = NULL;

	pTask = container_of(work, struct mdp_task_struct, task_work);
	if (pTask == NULL) {
		MDP_ERR("get Task NULL\n");
		return;
	}


	if (unlikely(pTask->mdp_command.task_type == MDP_TASK_TYPE_IMGRESZ_ROT))
		mdp_core_handle_imgresz_rot_task(pTask);
	else
		mdp_core_handle_task(pTask);
#if 0
	do {
		/* 6 trigger a new thread */
		if (pTask->mdp_command.task_type == MDP_TASK_TYPE_IMGRESZ_ROT)
			pThread = kthread_create(mdp_core_handle_imgresz_rot_task, pTask, "handle_imgresz_rot_task");
		else
			pThread = kthread_create(mdp_core_handle_task, pTask, "handle_task");
		if (!IS_ERR(pThread)) {
			wake_up_process(pThread);
		} else {
			MDP_ERR("create thread fail\n");
			status = MDP_TASK_STATUS_CREATE_THREAD_FAIL;
			/* release fence since fence is already created */
			if (mdp_fence_put_task_fence(pTask) != MDP_TASK_STATUS_OK)
				MDP_ERR("release mdp fence error\n");
			if (mdp_core_put_mdp_task(pTask) != MDP_TASK_STATUS_OK)
				MDP_ERR("put mdp task error\n");
		}
	} while (0);
#endif
}


/*
** handle mdp ioctl: MDP_IOCTL_EXEC_COMMAND
** 1 get a free task
** 2 copy mdp_command_struct to mdp_task_struct
** 3 create fence for input buffer & output buffer
** 4 copy created fence fd to mdp_command_struct
** 5 trigger a new thread
** 6 return to userspace
*/
enum MDP_TASK_STATUS mdp_core_handle_exec_command(struct mdp_command_struct *pCommand)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct mdp_task_struct *pTask;
	unsigned long flags;

	do {
		/* 1 acquire a free mdp_task_struct */
		pTask = mdp_core_get_mdp_task();
		if (pTask == NULL) {
			MDP_ERR("mdp_task_struct allocate failed");
			status = MDP_TASK_STATUS_ALLOCATE_TASK_FAIL;
			break;
		}
		pTask->time.start = sched_clock();

		/* 2 copy mdp_command_struct to mdp_task_struct */
		memcpy(&pTask->mdp_command, pCommand, sizeof(pTask->mdp_command));

		DISP_MMP(MMP_DISP_MDP, MMP_START, 0, 0);
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_BUF_CONFIG);

		/* convert ion fd to mva or secure handle */
		status = mdp_buffer_convert_ion_fd(&pTask->mdp_command.src_buffer, &pTask->src_ion_handle);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("convert source buffer fail:%d\n", status);
			status = MDP_TASK_STATUS_CONVERT_SRC_ION_HANDLE_FAIL;
			break;
		}

		status = mdp_buffer_convert_ion_fd(&pTask->mdp_command.dst_buffer, &pTask->dst_ion_handle);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("convert dst buffer fail:%d\n", status);
			status = MDP_TASK_STATUS_CONVERT_DST_ION_HANDLE_FAIL;
			break;
		}

		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_BUF_FILL);
		/* fill mdp task info, prepare buffer info */
		status = mdp_core_fill_mdp_task(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("task %s fill mdp task error:%d\n", pTask->mdp_command.taskString, status);
			status = MDP_TASK_STATUS_FILL_MDP_TASK_FAIL;
			break;
		}

		/* create fence for input buffer & output buffer */
		status = mdp_fence_get_task_fence(pTask);
		if (status != MDP_TASK_STATUS_OK) {
			MDP_ERR("task %s acquire task fence fail:%d\n", pTask->mdp_command.taskString, status);
			spin_unlock_irqrestore(&global_mdp_task.trigger_imgresz_task_list.spin_lock, flags);
			status = MDP_TASK_STATUS_GET_TASK_FENCE_FAIL;
			break;
		}

		/* copy created fence fd to mdp_command_struct */
		pCommand->src_buffer.release_fence_fd = pTask->p_src_fence->fence.fence;
		pCommand->dst_buffer.release_fence_fd = pTask->p_dst_fence->fence.fence;

		wake_up(&global_mdp_task.imgresz_wait_trigger_list_not_empty);
	} while (0);

	if (status != MDP_TASK_STATUS_OK) {
		/* reelase resource */
		switch (status) {
		case MDP_TASK_STATUS_GET_TASK_FENCE_FAIL:
		case MDP_TASK_STATUS_FILL_MDP_TASK_FAIL:
			/*convert dst ion handle success, need to free */
			mdp_buffer_release_ion_handle(&pTask->mdp_command.dst_buffer, &pTask->dst_ion_handle);
		case MDP_TASK_STATUS_CONVERT_DST_ION_HANDLE_FAIL:
			/*convert src ion handle success, need to free */
			mdp_buffer_release_ion_handle(&pTask->mdp_command.src_buffer, &pTask->src_ion_handle);
		case MDP_TASK_STATUS_CONVERT_SRC_ION_HANDLE_FAIL:
			/* allocate buffer success, we need to release buffer */
			list_del_init(&pTask->list_entry);

			mutex_lock(&global_mdp_task.free_task_list.mutex_lock);
			list_add_tail(&pTask->list_entry, &global_mdp_task.free_task_list.list);
			mutex_unlock(&global_mdp_task.free_task_list.mutex_lock);
		case MDP_TASK_STATUS_ALLOCATE_TASK_FAIL:
			/* allocate buffer fail, no need to do anything */
		default:
			break;
		}
	}

	return status;
}


/* dump mdp_command_struct */
void mdp_core_dump_command(struct mdp_command_struct *pCommand)
{
	MDP_LOG("task %s dump mdp command begin\n", pCommand->taskString);
	MDP_LOG("DpCommand: is_secure[%d] task_type[%d]\n", pCommand->is_secure, pCommand->task_type);
	MDP_LOG("DpCommand: src buffer: wait_fence_fd[%d] y_buffer[0x%x] u_buffer[0x%x] v_buffer[0x%x]\n",
			pCommand->src_buffer.wait_fence_fd,
			pCommand->src_buffer.y_buffer_address,
			pCommand->src_buffer.u_buffer_address,
			pCommand->src_buffer.v_buffer_address);

	MDP_LOG("DpCommand: src buffer: format[0x%x] w[%d] h[%d] y_pitch[%d]\n",
			pCommand->src_buffer.color_format,
			pCommand->src_buffer.width,
			pCommand->src_buffer.height,
			pCommand->src_buffer.y_pitch);


	MDP_LOG("ufo_ylen[0x%x] ufo_clen[0x%x] ufo_ylen_length[%d] ufo_clen_length[%d]\n",
			pCommand->src_buffer.ufo_ylen_buffer_address,
			pCommand->src_buffer.ufo_clen_buffer_address,
			pCommand->src_buffer.ufo_ylen_buffer_len,
			pCommand->src_buffer.ufo_clen_buffer_len);

	MDP_LOG("DpCommand: dst buffer: wait_fence_fd[%d] y_buffer[0x%x] u_buffer[0x%x] v_buffer[0x%x]\n",
			pCommand->dst_buffer.wait_fence_fd,
			pCommand->dst_buffer.y_buffer_address,
			pCommand->dst_buffer.u_buffer_address,
			pCommand->dst_buffer.v_buffer_address);
	MDP_LOG("DpCommand: dst buffer: format[0x%x] w[%d] h[%d] y_pitch[%d]\n",
			pCommand->dst_buffer.color_format,
			pCommand->dst_buffer.width,
			pCommand->dst_buffer.height,
			pCommand->dst_buffer.y_pitch);

	MDP_LOG("ufo_ylen[0x%x] ufo_clen[0x%x] ufo_ylen_length[%d] ufo_clen_length[%d]\n",
			pCommand->dst_buffer.ufo_ylen_buffer_address,
			pCommand->dst_buffer.ufo_clen_buffer_address,
			pCommand->dst_buffer.ufo_ylen_buffer_len,
			pCommand->dst_buffer.ufo_clen_buffer_len);


	MDP_LOG("task %s dump mdp command end\n", pCommand->taskString);
}

/* dump mdp task */
void mdp_core_dump_task(struct mdp_task_struct *pTask)
{
	switch (pTask->mdp_command.task_type) {
	case MDP_TASK_TYPE_IMGRESZ:
		mdp_imgresz_dump_task(pTask);
		break;
	case MDP_TASK_TYPE_ROT:
		mdp_rot_dump_task(pTask);
		break;
	case MDP_TASK_TYPE_NR:
		mdp_nr_dump_task(pTask);
		break;
	default:
		break;
	}
}
