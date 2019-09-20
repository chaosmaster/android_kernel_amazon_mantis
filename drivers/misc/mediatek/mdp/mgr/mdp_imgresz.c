#include <linux/kthread.h>
#include <linux/wait.h>
#include "mdp_fence.h"
#include "mdp_log.h"
#include "mdp_imgresz.h"
#include "mdp_cli.h"
#include "mdp_core.h"

struct workqueue_struct *imgresz_release_wq;


/*
** init mdp imgresz: create thread for dispatch imgresz hw
*/
enum MDP_TASK_STATUS mdp_imgresz_init(void)
{
#if 1
	struct task_struct *pThread;
	struct sched_param param = {2};

	pThread = kthread_run(mdp_imgresz_dispatch_hw, NULL, "mdp_imgresz_hw_dispatch_management");
	if (IS_ERR(pThread)) {
		MDP_ERR("create thread mdp_imgresz_hw_dispatch_management failed\n");
		return MDP_TASK_STATUS_CREATE_IMGRESZ_THREAD_FAIL;
	}
	/* adjust thread priority. */
	sched_setscheduler(pThread, SCHED_RR, &param);
#endif
	imgresz_release_wq = create_workqueue("MDP_RELEASE_TASK");


	return MDP_TASK_STATUS_OK;
}

/*
** map MDP color format: enum DP_COLOR_ENUM
** to imgresz source color format: enum IMGRESZ_SRC_COLOR_MODE
*/
enum IMGRESZ_SRC_COLOR_MODE mdp_imgresz_map_src_format(enum DP_COLOR_ENUM color_format)
{
	/* TODO: we need to map more color format */
	switch (color_format) {
	case DP_COLOR_420_BLKP:
			return IMGRESZ_SRC_COL_MD_420_BLK;
	case DP_COLOR_NV12:
	case DP_COLOR_NV21:
			return IMGRESZ_SRC_COL_MD_420_RS;
	case DP_COLOR_YV12:
	case DP_COLOR_YV16:
			return IMGRESZ_SRC_COL_MD_JPG_DEF;
	case DP_COLOR_422_BLKP:
			return IMGRESZ_SRC_COL_MD_422_BLK;
	case DP_COLOR_YUYV:
			return IMGRESZ_SRC_COL_MD_422_RS;
	case DP_COLOR_ARGB8888:
		return IMGRESZ_SRC_COL_MD_ARGB_8888;
	case DP_COLOR_RGB565:
		return IMGRESZ_SRC_COL_MD_RGB_565;
	case DP_COLOR_420_BLKP_UFO_10_H:
	case DP_COLOR_420_BLKP_UFO_10_V:
	case DP_COLOR_420_BLKP_UFO:
		return IMGRESZ_SRC_COL_MD_420_BLK;
	case DP_COLOR_420_BLKP_10_H:
	case DP_COLOR_420_BLKP_10_V:
		return IMGRESZ_SRC_COL_MD_420_BLK;
	case DP_COLOR_NV12_10L:
		return IMGRESZ_SRC_COL_MD_420_RS;


	default:
		return IMGRESZ_SRC_COL_MD_NONE;
	}
	return IMGRESZ_SRC_COL_MD_NONE;
}

/*
** map MDP color format: enum DP_COLOR_ENUM
** to imgresz source color format: enum IMGRESZ_DST_COLOR_MODE
*/
enum IMGRESZ_DST_COLOR_MODE mdp_imgresz_map_dst_format(enum DP_COLOR_ENUM color_format)
{
	/* TODO: we need to map more color format */
	switch (color_format) {
	case DP_COLOR_420_BLKP:
		return IMGRESZ_DST_COL_MD_420_BLK;
	case DP_COLOR_NV12:
	case DP_COLOR_NV21:
		return IMGRESZ_DST_COL_MD_420_RS;
	case DP_COLOR_422_BLKP:
		return IMGRESZ_DST_COL_MD_422_BLK;
	case DP_COLOR_YUYV:
		return IMGRESZ_DST_COL_MD_422_RS;
	case DP_COLOR_ARGB8888:
		return IMGRESZ_DST_COL_MD_ARGB_8888;
	case DP_COLOR_RGB565:
		return IMGRESZ_DST_COL_MD_RGB_565;

	default:
		return IMGRESZ_DST_COL_MD_NONE;
	}
	return IMGRESZ_DST_COL_MD_NONE;
}


enum IMGRESZ_UFO_TYPE mdp_imgresz_get_ufo_format(enum DP_COLOR_ENUM color_format)
{
	if (DP_COLOR_GET_10BIT(color_format) && DP_COLOR_GET_UFP_ENABLE(color_format))
		return IMGRESZ_UFO_10BIT_COMPACT;

	if (DP_COLOR_GET_UFP_ENABLE(color_format))
		return IMGRESZ_UFO_8BIT;

	if (DP_COLOR_GET_10BIT(color_format))
		return IMGRESZ_UFO_10BIT_COMPACT_UNCOMPRESS;

	return IMGRESZ_UFO_MIN;
}


/*
** get imgresz function type by color format
** non ufo format: IMGRESZ_FUN_NORMAL
** ufo format: IMGRESZ_FUN_UFO_2HW
*/
enum imgresz_ticket_fun_type mdp_imgresz_get_function_type(enum DP_COLOR_ENUM format, int width, int height)
{
	/* ufo format */
	if (DP_COLOR_GET_UFP_ENABLE(format) && (width * height > 1920 * 1088)) {
#if 0
		MDP_LOG("use 1 hw to convert ufo\n");
		return IMGRESZ_FUN_UFO;
#else
		MDP_LOG("use 2 hw to convert ufo\n");
		return IMGRESZ_FUN_UFO_2HW;
#endif
	}

	/* non ufo format */
	return IMGRESZ_FUN_NORMAL;
}



/*
** fill mdp task for imgresz use.
** fill src_buffer & dst_buffer info in struct imgresz_task_struct
*/
enum MDP_TASK_STATUS mdp_imgresz_fill_task(struct mdp_task_struct *pTask)
{
	/* FILL imgresz field */
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct imgresz_src_buf_info *p_task_src_buffer = NULL;
	struct imgresz_dst_buf_info *p_task_dst_buffer = NULL;
	struct mdp_buffer_struct *p_command_buffer = NULL;
	bool need_cbcr_swap = false;

	do {
		/* source buffer info */
		p_task_src_buffer = &pTask->task.imgresz_task.src_buffer;
		p_command_buffer = &pTask->mdp_command.src_buffer;

		/* color format */
		p_task_src_buffer->src_mode = mdp_imgresz_map_src_format(p_command_buffer->color_format);
		if (p_task_src_buffer->src_mode == IMGRESZ_SRC_COL_MD_NONE) {
			MDP_ERR("unsupported mdp format for imgresz src format:0x%x\n", p_command_buffer->color_format);
			status = MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_SRC_FORMAT;
			break;
		} else if (p_task_src_buffer->src_mode == IMGRESZ_SRC_COL_MD_JPG_DEF) {
			uint32_t tempPA = 0;

			switch (p_command_buffer->color_format) {
			case DP_COLOR_YV12:
				/* swap cb & cr, since YV12 is Y V U memory layout */
				tempPA = p_command_buffer->u_buffer_address;
				p_command_buffer->u_buffer_address = p_command_buffer->v_buffer_address;
				p_command_buffer->v_buffer_address = tempPA;

				p_task_src_buffer->jpg_comp.y_comp_sample_h = 2;
				p_task_src_buffer->jpg_comp.y_comp_sample_v = 2;
				p_task_src_buffer->jpg_comp.cb_comp_sample_h = 1;
				p_task_src_buffer->jpg_comp.cb_comp_sample_v = 1;
				p_task_src_buffer->jpg_comp.cr_comp_sample_h = 1;
				p_task_src_buffer->jpg_comp.cr_comp_sample_v = 1;
				break;
			case DP_COLOR_YV16:
				/* swap cb & cr, since YV12 is Y V U memory layout */
				tempPA = p_command_buffer->u_buffer_address;
				p_command_buffer->u_buffer_address = p_command_buffer->v_buffer_address;
				p_command_buffer->v_buffer_address = tempPA;

				p_task_src_buffer->jpg_comp.y_comp_sample_h = 4;
				p_task_src_buffer->jpg_comp.y_comp_sample_v = 1;
				p_task_src_buffer->jpg_comp.cb_comp_sample_h = 2;
				p_task_src_buffer->jpg_comp.cb_comp_sample_v = 1;
				p_task_src_buffer->jpg_comp.cr_comp_sample_h = 2;
				p_task_src_buffer->jpg_comp.cr_comp_sample_v = 1;
				break;
			default:
				MDP_ERR("3 plane unsupport mdp format:0x%x\n", p_command_buffer->color_format);
				status = MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_SRC_FORMAT;
				break;
			}
			if (status == MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_SRC_FORMAT)
				break;
		}

		if (p_command_buffer->color_format == DP_COLOR_NV21)
			need_cbcr_swap = !need_cbcr_swap;

		p_task_src_buffer->buf_info.mem_type = p_command_buffer->buffer_info.memory_type;

		p_task_src_buffer->buf_info.fd = p_command_buffer->buffer_info.memory_type == DP_MEMORY_SECURE ?
			p_command_buffer->buffer_info.secureHandle :
			p_command_buffer->buffer_info.fd;

		p_task_src_buffer->buf_info.y_offset = p_command_buffer->buffer_info.y_offset;
		p_task_src_buffer->buf_info.cb_offset = p_command_buffer->buffer_info.cb_offset;
		p_task_src_buffer->buf_info.cr_offset = p_command_buffer->buffer_info.cr_offset;
		p_task_src_buffer->buf_info.ylen_offset = p_command_buffer->buffer_info.ylen_offset;
		p_task_src_buffer->buf_info.clen_offset = p_command_buffer->buffer_info.clen_offset;

		p_task_src_buffer->y_buf_addr = p_command_buffer->y_buffer_address; /* y buffer */
		p_task_src_buffer->cb_buf_addr = p_command_buffer->u_buffer_address; /* u buffer */
		p_task_src_buffer->cr_buf_addr = p_command_buffer->v_buffer_address; /* v buffer (not used)*/
		p_task_src_buffer->buf_width = p_command_buffer->y_pitch; /* pitch */
		p_task_src_buffer->buf_height = p_command_buffer->buffer_height; /* buffer height align value */
		p_task_src_buffer->pic_width = p_command_buffer->width; /* width */
		p_task_src_buffer->pic_height = p_command_buffer->height; /* height */
		p_task_src_buffer->pic_x_offset = p_command_buffer->pic_x_offset; /* default 0 */
		p_task_src_buffer->pic_y_offset = p_command_buffer->pic_y_offset; /* default 0 */

		p_task_src_buffer->interlaced = DP_COLOR_GET_INTERLACED_MODE(p_command_buffer->color_format);
		p_task_src_buffer->topfield = (p_command_buffer->interlace_format == eTop_Field);
		p_task_src_buffer->bottomfield = (p_command_buffer->interlace_format == eBottom_Field);


		p_task_src_buffer->ufo_type = mdp_imgresz_get_ufo_format(p_command_buffer->color_format);
		p_task_src_buffer->ufo_ylen_buf = p_command_buffer->ufo_ylen_buffer_address;
		p_task_src_buffer->ufo_clen_buf = p_command_buffer->ufo_clen_buffer_address;
		p_task_src_buffer->ufo_ylen_buf_len = p_command_buffer->ufo_ylen_buffer_len;
		p_task_src_buffer->ufo_ybuf_len = p_command_buffer->ufo_clen_buffer_len;

		p_task_src_buffer->ufo_jump = (p_command_buffer->metaData == DP_METADATA_JUMP_MODE);

		/* dst buffer info */
		p_task_dst_buffer = &pTask->task.imgresz_task.dst_buffer;
		p_command_buffer = &pTask->mdp_command.dst_buffer;

		p_task_dst_buffer->dst_mode = mdp_imgresz_map_dst_format(p_command_buffer->color_format);
		if (p_task_dst_buffer->dst_mode == IMGRESZ_DST_COL_MD_NONE) {
			MDP_ERR("unsupported mdp format for imgresz dst format:0x%x\n", p_command_buffer->color_format);
			status = MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_DST_FORMAT;
			break;
		}

		p_task_dst_buffer->buf_info.mem_type = p_command_buffer->buffer_info.memory_type;

		p_task_dst_buffer->buf_info.fd = p_command_buffer->buffer_info.memory_type == DP_MEMORY_SECURE ?
			p_command_buffer->buffer_info.secureHandle :
			p_command_buffer->buffer_info.fd;

		p_task_dst_buffer->buf_info.y_offset = p_command_buffer->buffer_info.y_offset;
		p_task_dst_buffer->buf_info.cb_offset = p_command_buffer->buffer_info.cb_offset;
		p_task_dst_buffer->buf_info.cr_offset = p_command_buffer->buffer_info.cr_offset;
		p_task_dst_buffer->buf_info.ylen_offset = p_command_buffer->buffer_info.ylen_offset;
		p_task_dst_buffer->buf_info.clen_offset = p_command_buffer->buffer_info.clen_offset;

		p_task_dst_buffer->y_buf_addr = p_command_buffer->y_buffer_address;
		p_task_dst_buffer->c_buf_addr = p_command_buffer->u_buffer_address;
		p_task_dst_buffer->pic_width = p_command_buffer->width;
		p_task_dst_buffer->pic_height = p_command_buffer->height;
		p_task_dst_buffer->buf_width = p_command_buffer->y_pitch;
		p_task_dst_buffer->pic_x_offset = p_command_buffer->pic_x_offset;
		p_task_dst_buffer->pic_y_offset = p_command_buffer->pic_y_offset;
		p_task_dst_buffer->cbcr_swap = (p_command_buffer->color_format == DP_COLOR_NV21) ?
			!need_cbcr_swap : need_cbcr_swap;

		pTask->current_state = MDP_TASK_STATE_FILLED;
	} while (0);

	return status;
}

bool mdp_imgresz_get_ticket(struct mdp_task_struct *pTask, IMGRESZ_TICKET *pTicket)
{
	enum imgresz_ticket_fun_type type;

	type = mdp_imgresz_get_function_type(pTask->mdp_command.src_buffer.color_format,
		pTask->mdp_command.src_buffer.width,
		pTask->mdp_command.src_buffer.height);
	return mdp_imgresz_is_hardware_ready(type,	pTicket);
}


/*
** May sleep
** wait a valid imgresz ticket
*/
enum MDP_TASK_STATUS mdp_imgresz_wait_hardware_ready(struct mdp_task_struct *pTask)
{
	IMGRESZ_TICKET ticket = 0;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);
	long timeout = msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS);
	#if 0
	/* wakeup thread: imgresz hw dispatch thread */
	wake_up(&global_mdp_task.mdp_core_wait_imgresz_hw_idle_queue);

	/* waiting get a imgresz ticket */

	status = wait_event_timeout(global_mdp_task.mdp_core_wait_imgresz_ticket_queue,
			pTask->current_state == MDP_TASK_STATE_PREPARED,
			msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS));
	#endif

#if 0
	status = wait_event_timeout(global_mdp_task.mdp_core_wait_imgresz_ticket_queue,
		mdp_imgresz_is_hardware_ready(type,	&ticket),
		msecs_to_jiffies(MDP_WAIT_TASK_TIME_MS));
#else
	add_wait_queue(&global_mdp_task.mdp_core_wait_imgresz_ticket_queue, &wait);

	while (1) {
		if (mdp_imgresz_get_ticket(pTask, &ticket))
			break;
		timeout = wait_woken(&wait, TASK_INTERRUPTIBLE, timeout);
		if (timeout == 0)
			break;
	}
	remove_wait_queue(&global_mdp_task.mdp_core_wait_imgresz_ticket_queue, &wait);
#endif

	/* timeout */
	if (timeout == 0) {
		MDP_ERR("task %s get imgresz ticket timeout state:%d\n",
			pTask->mdp_command.taskString, pTask->current_state);
		return MDP_TASK_STATUS_GET_IMGRESZ_TICKET_TIMEOUT;
	}

	MDP_LOG("get a free ticket[%d] for task %s\n", ticket, pTask->mdp_command.taskString);
	pTask->task.imgresz_task.ticket = ticket;
	pTask->current_state = MDP_TASK_STATE_PREPARED;

	/* get a ticket success */
	return MDP_TASK_STATUS_OK;
}

/*
** The function is called after get imgresz ticket
** need to acquire fence before call imgresz api
** trigger imgresz to work
*/
enum MDP_TASK_STATUS mdp_imgresz_trigger_hw_work(struct mdp_task_struct *pTask)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	bool async_mode = false;
	int ret = 0;

	mdp_imgresz_dump_task(pTask);

	do {
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_GET_TICKET);
		/* get ticket */
		if (!mdp_imgresz_get_ticket(pTask, &pTask->task.imgresz_task.ticket)) {
			MDP_ERR("task: %s get ticket timeout\n", pTask->mdp_command.taskString);
			status = MDP_TASK_STATUS_GET_IMGRESZ_TICKET_TIMEOUT;
			break;
		}
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_SET_SCALE_MODE);
		/* set scale mode */
		ret = imgresz_set_scale_mode(pTask->task.imgresz_task.ticket, IMGRESZ_FRAME_SCALE);
		if (ret < 0) {
			MDP_ERR("call imgresz_set_scale_mode failed\n");
			status = MDP_TASK_STATUS_SET_SCALE_MODE_FAIL;
			break;
		}

		/* set input buffer info */
		ret = imgresz_set_src_bufinfo(pTask->task.imgresz_task.ticket,
				&pTask->task.imgresz_task.src_buffer);/* set src info */
		if (ret < 0) {
			MDP_ERR("call imgresz_set_src_bufinfo failed\n");
			status = MDP_TASK_STATUS_SET_SRC_BUFFER_FAIL;
			break;
		}

		/* set output buffer info */
		ret = imgresz_set_dst_bufinfo(pTask->task.imgresz_task.ticket,
				&pTask->task.imgresz_task.dst_buffer);/* set dst info */
		if (ret < 0) {
			MDP_ERR("call imgresz_set_dst_bufinfo failed\n");
			status = MDP_TASK_STATUS_SET_DST_BUFFER_FAIL;
			break;
		}

		async_mode = false;	/* block mode */

		/* set callback function. */
		imgresz_set_callback_fun(pTask->task.imgresz_task.ticket,
			async_mode ? mdp_imgresz_handle_irq : NULL,
			async_mode ? pTask : NULL);

		/* trigger imgresz hw work */
		pTask->time.trigger = sched_clock();
		pTask->current_state = MDP_TASK_STATE_WORKING;
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_SCALE_ASYNC);
		ret = imgresz_trigger_scale_async(pTask->task.imgresz_task.ticket, async_mode);
		if (ret < 0) {
			MDP_ERR("call imgresz_trigger_scale_async failed\n");
			status = MDP_TASK_STATUS_TRIGGER_SCALE_FAIL;
			break;
		}
	} while (0);

	if (!async_mode) {
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_HW_DONE);
		pTask->time.gotIRQ = sched_clock();
		pTask->current_state = MDP_TASK_STATE_DONE;
		mdp_release_frame_count++;
		if (status != MDP_TASK_STATUS_GET_IMGRESZ_TICKET_TIMEOUT)
			imgresz_ticket_put(pTask->task.imgresz_task.ticket);
	}

	return status;
}


/*
** imgresz task error
*/
void mdp_imgresz_handle_task_error(struct mdp_task_struct *pTask)
{
	/* Imgresz dump itself if imgresz timeout. */
	/* imgresz_debug_dump(pTask->task.imgresz_task.ticket); */
}

/*
** imgresz task done
*/
void mdp_imgresz_handle_task_done(struct mdp_task_struct *pTask)
{
	MDP_LOG("task %s done\n", pTask->mdp_command.taskString);
}

/*
** imgresz ready or not
** critical hardware resource. we need add mutex_lock to get ticket.
** maybe muti threads call this function at same time.
** and imgresz_ticket_get API doesn't use lock.
** so we need to add lock to ensure only one thread is getting ticket at one time.
*/
bool mdp_imgresz_is_hardware_ready(enum imgresz_ticket_fun_type function_type, IMGRESZ_TICKET *pTicket)
{
	IMGRESZ_TICKET ticket;

	if (pTicket == NULL) {
		MDP_ERR("null pointer is passed to func:%s\n", __func__);
		return false;
	}

	ticket = imgresz_ticket_get(function_type);
	if (ticket < 0)
		return false;

	/* got a free imgresz hardware */
	*pTicket = ticket;
	return true;
}

/*
** check if imgresz_normal_prepare_task_list is empty
** note use mutex_lock to ensure serialize list access
*/
bool mdp_imgresz_normal_list_first_task_filled(void)
{
	bool normal_list_filled = false;
	struct mdp_task_struct *pTask = NULL;

	mutex_lock(&global_mdp_task.imgresz_normal_prepare_task_list.mutex_lock);
	if (!list_empty(&global_mdp_task.imgresz_normal_prepare_task_list.list)) {
		pTask = list_first_entry(&global_mdp_task.imgresz_normal_prepare_task_list.list,
			struct mdp_task_struct, list_entry);
		normal_list_filled = (pTask->current_state == MDP_TASK_STATE_FILLED);
	}
	mutex_unlock(&global_mdp_task.imgresz_normal_prepare_task_list.mutex_lock);

	return normal_list_filled;
}

/*
** check if imgresz_ufo_prepare_task_list first task is filled &&
** note use mutex_lock to ensure serialize list access
*/
bool mdp_imgresz_ufo_list_first_task_filled(void)
{
	bool ufo_list_filled = false;
	struct mdp_task_struct *pTask = NULL;

	mutex_lock(&global_mdp_task.imgresz_ufo_prepare_task_list.mutex_lock);
	if (!list_empty(&global_mdp_task.imgresz_ufo_prepare_task_list.list)) {
		pTask = list_first_entry(&global_mdp_task.imgresz_ufo_prepare_task_list.list,
			struct mdp_task_struct, list_entry);
		ufo_list_filled = (pTask->current_state == MDP_TASK_STATE_FILLED);
	}
	mutex_unlock(&global_mdp_task.imgresz_ufo_prepare_task_list.mutex_lock);

	return ufo_list_filled;
}



/*
** check if imgresz_ufo_2_hw_prepare_task_list first task is filled &&
** note use mutex_lock to ensure serialize list access
*/
bool mdp_imgresz_ufo_2_hw_list_first_task_filled(void)
{
	bool ufo_2_hw_list_filled = false;
	struct mdp_task_struct *pTask = NULL;

	mutex_lock(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.mutex_lock);
	if (!list_empty(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.list)) {
		pTask = list_first_entry(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.list,
			struct mdp_task_struct, list_entry);
		ufo_2_hw_list_filled = (pTask->current_state == MDP_TASK_STATE_FILLED);
	}
	mutex_unlock(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.mutex_lock);

	return ufo_2_hw_list_filled;
}

bool mdp_imgresz_set_type(enum imgresz_ticket_fun_type *pType, enum imgresz_ticket_fun_type value)
{
	*pType = value;
	return true;
}
/*
** wait imgresz hardware idle, dispatch first task use this available hardware.
** dispatch ufo hardware first.
** if have ufo task, test ufo hardware available ? dispatch ufo ticket
** else if have normal task, test normal hardware available ? dispatch normal ticket : wait
** we may need to implement a while(1) function.
*/
#if 0
int32_t mdp_imgresz_dispatch_hw(void *ignore)
{
	IMGRESZ_TICKET ticket;
	struct mdp_task_struct *pTask = NULL;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);


	add_wait_queue(&global_mdp_task.mdp_core_wait_imgresz_hw_idle_queue, &wait);
	while (1) {
		enum imgresz_ticket_fun_type type = IMGRESZ_FUN_MIN;
		/* Caution */
		/* we wait imgresz hardwareidle on waitqueue: mdp_core_wait_imgresz_hw_idle_queue */
		/* only the condition is different, This may have issue */
		if (!((mdp_imgresz_ufo_2_hw_list_first_task_filled() &&
				mdp_imgresz_is_hardware_ready(IMGRESZ_FUN_UFO_2HW, &ticket) &&
				mdp_imgresz_set_type(&type, IMGRESZ_FUN_UFO_2HW)) ||
				(mdp_imgresz_ufo_list_first_task_filled() &&
				mdp_imgresz_is_hardware_ready(IMGRESZ_FUN_UFO, &ticket) &&
				mdp_imgresz_set_type(&type, IMGRESZ_FUN_UFO)) ||
				(mdp_imgresz_normal_list_first_task_filled() &&
				mdp_imgresz_is_hardware_ready(IMGRESZ_FUN_NORMAL, &ticket) &&
				mdp_imgresz_set_type(&type, IMGRESZ_FUN_NORMAL)))) {
			wait_woken(&wait, TASK_INTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT);
			continue;
		}

		/* got a free imgresz hardware ticket */
		if (type == IMGRESZ_FUN_UFO_2HW) {
			/* put ticket to first filled state task in ufo_2_hw task list */
			mutex_lock(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.mutex_lock);
			pTask = list_first_entry(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.list,
										struct mdp_task_struct, list_entry);
			list_del_init(&pTask->list_entry);
			mutex_unlock(&global_mdp_task.imgresz_ufo_2_hw_prepare_task_list.mutex_lock);

		} else if (type == IMGRESZ_FUN_UFO) {
			/* put ticket to first filled state task in ufo task list */
			mutex_lock(&global_mdp_task.imgresz_ufo_prepare_task_list.mutex_lock);
			pTask = list_first_entry(&global_mdp_task.imgresz_ufo_prepare_task_list.list,
										struct mdp_task_struct, list_entry);
			list_del_init(&pTask->list_entry);
			mutex_unlock(&global_mdp_task.imgresz_ufo_prepare_task_list.mutex_lock);

		} else if (type == IMGRESZ_FUN_NORMAL) {
			/* put ticket to first filled state task in normal task list */
			mutex_lock(&global_mdp_task.imgresz_normal_prepare_task_list.mutex_lock);
			pTask = list_first_entry(&global_mdp_task.imgresz_normal_prepare_task_list.list,
										struct mdp_task_struct, list_entry);
			list_del_init(&pTask->list_entry);
			mutex_unlock(&global_mdp_task.imgresz_normal_prepare_task_list.mutex_lock);
		} else {
			MDP_ERR("invalid imgresz type:%d\n", type);
			continue;
		}

		MDP_LOG("get a free ticket[%d] for task %s\n", ticket, pTask->mdp_command.taskString);

		pTask->task.imgresz_task.ticket = ticket;
		pTask->current_state = MDP_TASK_STATE_PREPARED;
		/* wakeup main thread: ioctl thread */
		wake_up(&global_mdp_task.mdp_core_wait_imgresz_ticket_queue);
	} /* endof while(1) */
	remove_wait_queue(&global_mdp_task.mdp_core_wait_imgresz_hw_idle_queue, &wait);

	return 0;
}
#endif

#if 1
int32_t mdp_imgresz_dispatch_hw(void *ignore)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;
	struct mdp_task_struct *pTask = NULL;
	DEFINE_WAIT_FUNC(wait, woken_wake_function);
	unsigned long flags;

	add_wait_queue(&global_mdp_task.imgresz_wait_trigger_list_not_empty, &wait);
	do {
		/* find a task in trigger buffer list */
		spin_lock_irqsave(&global_mdp_task.trigger_imgresz_task_list.spin_lock, flags);
		pTask = list_first_entry_or_null(&global_mdp_task.trigger_imgresz_task_list.list,
			struct mdp_task_struct, list_entry);
		spin_unlock_irqrestore(&global_mdp_task.trigger_imgresz_task_list.spin_lock, flags);

		/* if no task, sleep */
		if (pTask == NULL) {
			wait_woken(&wait, TASK_INTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT);
			continue;
		}


		/* trigger current task. */
		status = mdp_imgresz_trigger_hw_work(pTask);
		if (status == MDP_TASK_STATUS_OK)
			MDP_LOG("mdp_core_handle_task %s OK\n", pTask->mdp_command.taskString);
		else
			MDP_ERR("mdp_core_handle_task %s fail, status:%d\n", pTask->mdp_command.taskString, status);

		/* find a task */
		/* remove task from trigger buffer list. */
		list_del_init(&pTask->list_entry);
		wake_up(&global_mdp_task.mdp_core_wait_task_done_queue);


		/* release pTask */
		mdp_fence_put_task_fence(pTask);
		pTask->time.releaseFence = sched_clock();

		/* release current tsk*/
		INIT_WORK(&pTask->task_work, mdp_core_release_task);
		queue_work(imgresz_release_wq, &pTask->task_work);
		mdp_core_set_mmp_tag(pTask, MMP_DISP_MDP_BUF_CONFIG_END);
		DISP_MMP(MMP_DISP_MDP, MMP_END, 0, 0);
	} while (1);
	remove_wait_queue(&global_mdp_task.imgresz_wait_trigger_list_not_empty, &wait);

	return 0;
}
#endif

int mdp_imgresz_handle_irq(IMGRESZ_TICKET ti, enum imgresz_cb_state hw_state, void *privdata)
{
	return 0;
}



/* dump imgresz settings */
void mdp_imgresz_dump_task(struct mdp_task_struct *pTask)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->dump_imgresz_setting)
		return;
#else
	return;
#endif

	MDP_ERR("dump imgresz task %s ticket[%d] begin\n",
		pTask->mdp_command.taskString,
		pTask->task.imgresz_task.ticket);
	MDP_ERR("IMGREZ_SRC: buffer type[%d->%s] fd[0x%08x] offset:y[0x%x] cb[0x%x] cr[0x%x] ylen[0x%x] clen[0x%x]\n",
		pTask->task.imgresz_task.src_buffer.buf_info.mem_type,
		(pTask->task.imgresz_task.src_buffer.buf_info.mem_type != IMGRZ_MEM_SECURE) ? "ion" : "secure",
		pTask->task.imgresz_task.src_buffer.buf_info.fd,
		pTask->task.imgresz_task.src_buffer.buf_info.y_offset,
		pTask->task.imgresz_task.src_buffer.buf_info.cb_offset,
		pTask->task.imgresz_task.src_buffer.buf_info.cr_offset,
		pTask->task.imgresz_task.src_buffer.buf_info.ylen_offset,
		pTask->task.imgresz_task.src_buffer.buf_info.clen_offset);

	MDP_ERR("IMGREZ_SRC: format[%d] Y[0x%x] CB[0x%x] CR[0x%x] W[%u] H[%u] YPitch[%u] bufferHeight[%u]\n",
		pTask->task.imgresz_task.src_buffer.src_mode,
		(unsigned int)pTask->task.imgresz_task.src_buffer.y_buf_addr,
		(unsigned int)pTask->task.imgresz_task.src_buffer.cb_buf_addr,
		(unsigned int)pTask->task.imgresz_task.src_buffer.cr_buf_addr,
		pTask->task.imgresz_task.src_buffer.pic_width,
		pTask->task.imgresz_task.src_buffer.pic_height,
		pTask->task.imgresz_task.src_buffer.buf_width,
		pTask->task.imgresz_task.src_buffer.buf_height);

	MDP_ERR("IMGRESZ_SRC: xOffset[%d] yOffset[%d] interlace[%d] topfield[%d] bottomfield[%d]\n",
		pTask->task.imgresz_task.src_buffer.pic_x_offset,
		pTask->task.imgresz_task.src_buffer.pic_y_offset,
		pTask->task.imgresz_task.src_buffer.interlaced,
		pTask->task.imgresz_task.src_buffer.topfield,
		pTask->task.imgresz_task.src_buffer.bottomfield);

	MDP_ERR("IMGRESZ_SRC: ufo_type[%d] ylen[0x%x] clen[0x%x] ylen_len[%u] clen_len[%u] ufo_jump[%d]\n",
		pTask->task.imgresz_task.src_buffer.ufo_type,
		(unsigned int)pTask->task.imgresz_task.src_buffer.ufo_ylen_buf,
		(unsigned int)pTask->task.imgresz_task.src_buffer.ufo_clen_buf,
		pTask->task.imgresz_task.src_buffer.ufo_ylen_buf_len,
		pTask->task.imgresz_task.src_buffer.ufo_ybuf_len,
		pTask->task.imgresz_task.src_buffer.ufo_jump);

	MDP_ERR("IMGRESZ_SRC: jpg_comp y_h[%d] y_v[%d] cb_h[%d] cb_v[%d] cr_h[%d] cr_v[%d]\n",
		pTask->task.imgresz_task.src_buffer.jpg_comp.y_comp_sample_h,
		pTask->task.imgresz_task.src_buffer.jpg_comp.y_comp_sample_v,
		pTask->task.imgresz_task.src_buffer.jpg_comp.cb_comp_sample_h,
		pTask->task.imgresz_task.src_buffer.jpg_comp.cb_comp_sample_v,
		pTask->task.imgresz_task.src_buffer.jpg_comp.cr_comp_sample_h,
		pTask->task.imgresz_task.src_buffer.jpg_comp.cr_comp_sample_v);

	MDP_ERR("IMGREZ_DST: buffer type[%d->%s] fd[0x%08x] offset:y[0x%x] cb[0x%x] cr[0x%x] ylen[0x%x] clen[0x%x]\n",
		pTask->task.imgresz_task.dst_buffer.buf_info.mem_type,
		(pTask->task.imgresz_task.dst_buffer.buf_info.mem_type == IMGRZ_MEM_ION) ? "ion" : "secure",
		pTask->task.imgresz_task.dst_buffer.buf_info.fd,
		pTask->task.imgresz_task.dst_buffer.buf_info.y_offset,
		pTask->task.imgresz_task.dst_buffer.buf_info.cb_offset,
		pTask->task.imgresz_task.dst_buffer.buf_info.cr_offset,
		pTask->task.imgresz_task.dst_buffer.buf_info.ylen_offset,
		pTask->task.imgresz_task.dst_buffer.buf_info.clen_offset);

	MDP_ERR("IMGREZ_DST: format[%d] Y[0x%x] CB[0x%x] YPitch[%u] W[%u] WCb[%u] WCr[%u] H[%u]\n",
				pTask->task.imgresz_task.dst_buffer.dst_mode,
				(unsigned int)pTask->task.imgresz_task.dst_buffer.y_buf_addr,
				(unsigned int)pTask->task.imgresz_task.dst_buffer.c_buf_addr,
				pTask->task.imgresz_task.dst_buffer.buf_width,
				pTask->task.imgresz_task.dst_buffer.pic_width,
				pTask->task.imgresz_task.dst_buffer.pic_width_cb,
				pTask->task.imgresz_task.dst_buffer.pic_width_cr,
				pTask->task.imgresz_task.dst_buffer.pic_height);

	MDP_ERR("IMGREZ_DST: xOffset[%d] yOffset[%d] cbcr_swap[%d] 10Bit[%d]\n",
		pTask->task.imgresz_task.dst_buffer.pic_x_offset,
		pTask->task.imgresz_task.dst_buffer.pic_y_offset,
		pTask->task.imgresz_task.dst_buffer.cbcr_swap,
		pTask->task.imgresz_task.dst_buffer.bit10);

	MDP_ERR("dump imgresz task %s ticket[%d] end\n",
		pTask->mdp_command.taskString,
		pTask->task.imgresz_task.ticket);
}
