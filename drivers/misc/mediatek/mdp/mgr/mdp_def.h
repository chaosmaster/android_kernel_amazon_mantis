#ifndef __MDP_DEF_H__
#define __MDP_DEF_H__
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <mtk_sync.h>
#include <ion/ion.h>
#include <ion/ion_priv.h>
#include <mtk/mtk_ion.h>

#include "imgresz.h"
#include "mdp_param.h"

extern struct global_mdp_task_struct global_mdp_task;

/*
** imgresz has 4 HW, only 3 can support UFO format
** if we don't handle UFO format, 4HW can work at same time.
** if we use UFO_2_HW, only 1 task can handle at one time.
**
** rotate only have 1 Hardware
** we need 1 fence since only 1 hardware work and input buffer & output buffer is released at same time.
**
** NR only have 1 hardware, but for 3D NR, input buffer is released at end of this frame.
** but output buffer is useful to next frame. so output buffer is released at end of next frame.
** we need 2 fence for input buffer & output buffer
*/
enum MDP_TIMELINE_TYPE {
	MDP_TIMELINE_TYPE_INVALID = -1,

	/* imgresz & rot input buffer & output buffer are release at same time.
	** but we don't want ufo2hw affect other normal task fence release
	*/
	MDP_TIMELINE_TYPE_IMGRESZ_NORMAL = 0,
	MDP_TIMELINE_TYPE_IMGRESZ_UFO_2_HW = 1,

	MDP_TIMELINE_TYPE_ROT = 2,

	/* for nr & imgresz+rot input buffer & output buffer are not release at same time */
	MDP_TIMELINE_TYPE_NR_INPUT = 3,	/* release when current frame done */
	MDP_TIMELINE_TYPE_NR_OUTPUT = 4,	/* release when next frame done */
	MDP_TIMELINE_TYPE_IMGRESZ_ROT_INPUT = 5,	/* this is imgresz input buffer */
	MDP_TIMELINE_TYPE_IMGRESZ_ROT_OUTPUT = 6,	/* this is rot output buffer */
	MDP_TIMELINE_TYPE_MAX,
};


enum MDP_TASK_STATE {
	MDP_TASK_STATE_FREE = 0, /* unused free task in free task list */
	MDP_TASK_STATE_STARTED, /* got a new mdp_task_struct to store mdp_command_struct from userspace */
	MDP_TASK_STATE_FILLED,	/* fill task ready, begin to acquire hardware */
	MDP_TASK_STATE_ACQUIRED, /* allocate a temp buffer done */
	MDP_TASK_STATE_PREPARED, /* got a idle hardware to execute this task. */
	MDP_TASK_STATE_WORKING, /* fence is created, mdp task is summited to hardware driver. wait for done */
	MDP_TASK_STATE_DONE, /* mdp task is done. */
	MDP_TASK_STATE_NR_OUTPUT_RELEASED, /* nr task output buffer is released */
	MDP_TASK_STATE_ERROR, /* mdp task has error */
	MDP_TASK_STATE_TIMEOUT, /* wait mdp task timeout */
};

/* mdp error code */
enum MDP_TASK_STATUS {
	MDP_TASK_STATUS_OK = 0, /* OK case */
	MDP_TASK_STATUS_GENERAL_ERROR = 1, /* general error */
	MDP_TASK_STATUS_INVALID_PARAM = 2, /* invalid param */
	MDP_TASK_STATUS_INVALID_TASK_TYPE = 3,
	MDP_TASK_STATUS_SET_SCALE_MODE_FAIL = 4,
	MDP_TASK_STATUS_SET_SRC_BUFFER_FAIL = 5,
	MDP_TASK_STATUS_SET_DST_BUFFER_FAIL = 6,
	MDP_TASK_STATUS_SET_CALLBACK_FUNC_FAIL = 7,
	MDP_TASK_STATUS_TRIGGER_SCALE_FAIL = 8,
	MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_SRC_FORMAT = 9,
	MDP_TASK_STATUS_UNSUPPORTED_IMGRESZ_DST_FORMAT = 10,
	MDP_TASK_STATUS_INVALID_FENCE_TYPE = 11,
	MDP_TASK_STATUS_GET_IMGRESZ_TICKET_TIMEOUT = 12,
	MDP_TASK_STATUS_WAIT_TASK_TIMEOUT = 13,
	MDP_TASK_STATUS_COPY_FROM_USER_FAIL = 14,
	MDP_TASK_STATUS_UNDEFINED_IOCTL = 15,
	MDP_TASK_STATUS_CREATE_IMGRESZ_THREAD_FAIL = 16,
	MDP_TASK_STATUS_RELEASE_IMGRESZ_HW_FAIL = 17,
	MDP_TASK_STATUS_WAIT_INTERNAL_FENCE_TIMEOUT = 18,
	MDP_TASK_STATUS_WRONG_FENCE_ORDER = 19,
	MDP_TASK_STATUS_INVALID_FENCE_WAIT_FD = 20,
	MDP_TASK_STATUS_WAIT_FENCE_TIMEOUT = 21,
	MDP_TASK_STATUS_UNKNOWN_TASK_TYPE = 22,
	MDP_TASK_STATUS_COPY_TO_USER_FAIL = 23,
	MDP_TASK_STATUS_CREATE_THREAD_FAIL = 24,
	MDP_TASK_STATUS_HANDLE_TASK_ERROR = 25,
	MDP_TASK_STATUS_ALLOCATE_TEMP_BUFFER_FAIL = 27,
	MDP_TASK_STATUS_ALLOCATE_TEMP_BUFFER_SIZE_NOT_MATCH = 28,
	MDP_TASK_STATUS_WAIT_FREE_TEMP_BUFFER_TIMEOUT = 29,
	MDP_TASK_STATUS_BUFFER_POOL_OUTOF_ARRAY_INDEX = 30,
	MDP_TASK_STATUS_WAIT_NR_OUTPUT_BUFFER_RELEASE_TIMEOUT = 31,
	MDP_TASK_STATUS_NULL_POINTER_ACCESS = 32,
	MDP_TASK_STATUS_ALLOCATE_ERROR_STRUCT_FAIL = 33,
	MDP_TASK_STATUS_ALLOCATE_ERROR_BUFFER_FAIL = 34,
	MDP_TASK_STATUS_ALLOCATE_TIME_ITEM_FAIL = 35,
	MDP_TASK_STATUS_CREATE_MDP_DIR_FAIL = 36,
	MDP_TASK_STATUS_CREATE_MDP_DEVICE_FAIL = 37,
	MDP_TASK_STATUS_CREATE_MDP_ERROR_FAIL = 38,
	MDP_TASK_STATUS_CREATE_MDP_RECORD_FAIL = 39,
	MDP_TASK_STATUS_IMPORT_ION_FD_FAIL = 40,
	MDP_TASK_STATUS_CONVET_HANDLE_2_PA_FAIL = 41,
	MDP_TASK_STATUS_ALLOCATE_TASK_FAIL = 42,
	MDP_TASK_STATUS_FILL_MDP_TASK_FAIL = 43,
	MDP_TASK_STATUS_CONVERT_SRC_ION_HANDLE_FAIL = 44,
	MDP_TASK_STATUS_CONVERT_DST_ION_HANDLE_FAIL = 45,
	MDP_TASK_STATUS_GET_TASK_FENCE_FAIL = 46,
};

enum MDP_TEMP_BUFFER_STATE {
	MDP_TEMP_BUFFER_STATE_RELEASED = 0,
	MDP_TEMP_BUFFER_STATE_FREE = 1,
	MDP_TEMP_BUFFER_STATE_USED_BY_IMGRESZ = 2,
	MDP_TEMP_BUFFER_STATE_USED_BY_ROT = 3,
	MDP_TEMP_BUFFER_STATE_DONE = 4,
};

#define MDP_WAIT_TASK_TIME_MS 1000

struct task_time_item_struct {
	char taskString[MAX_ARRAY_SIZE];
	struct list_head list;
	uint64_t start; /* got ioctl */
	uint64_t trigger; /* triiger hardware work */
	uint64_t gotIRQ; /* got a irq */
	uint64_t releaseFence; /* release fence */
	uint64_t done; /* done */
};

struct task_list_struct {
	struct list_head list;
	struct mutex mutex_lock;
	spinlock_t spin_lock;
};

struct mdp_fence_struct {
	struct fence_data fence;
	struct list_head list; /* ensure fence release in create order */
	enum MDP_TIMELINE_TYPE type;
	char taskString[MAX_ARRAY_SIZE];
};


struct global_mdp_task_struct {
	/* free task put to this list. when task is used, remove from this list. */
	struct task_list_struct free_task_list;

	/* acquire task list  task need to allocate temp buffer put on this list. */
	/* when acquired, remove from this list. */
	struct task_list_struct acquire_temp_buffer_task_list;

	/* imgresz normal case prepare task list */
	/* task need to wait a idle hardware put on this list. ensure execute sequence */
	/* when prepared, remove from this list. */
	struct task_list_struct imgresz_normal_prepare_task_list;

	/* imgresz ufo case prepare task list*/
	/*task need to wait a idle hardware put on this list. ensure execute sequence */
	/* when prepared, remove from this list. */
	struct task_list_struct imgresz_ufo_prepare_task_list;

	/* imgresz ufo_2_hw case prepare task list*/
	/*task need to wait a idle hardware put on this list. ensure execute sequence */
	/* when prepared, remove from this list. */
	struct task_list_struct imgresz_ufo_2_hw_prepare_task_list;

	/* buffer list: task is prepared, need to trigger imgresz */
	struct task_list_struct trigger_imgresz_task_list;

	/* nr prepare task list, task need to wait nr hardware put on this list, ensure execute sequence */
	/* when prepared, remove from this list */
	struct task_list_struct nr_prepare_task_list;

	/* rot prepare task list, task need to wait rot hardware put on this list, ensure execute sequence*/
	struct task_list_struct rot_prepare_task_list;

	/* waitqueue: sleep wait for trigger_imgresz_task_list not empty */
	wait_queue_head_t imgresz_wait_trigger_list_not_empty;

	/* waitqueue: sleep wait for aquire a imgresz ticket, wait on main thread */
	wait_queue_head_t mdp_core_wait_imgresz_ticket_queue;

	/* waitqueue: sleep wait for get a idle imgresz hardware, wait on imgresz hw management thread*/
	wait_queue_head_t mdp_core_wait_imgresz_hw_idle_queue;

	/* waitqueue: sleep wait for task done */
	wait_queue_head_t mdp_core_wait_task_done_queue;

	/* ensure serialize call imgresz_ticket_get() function */
	struct mutex get_imgresz_ticket_mutex;
};

/*
** temp buffer struct
*/
struct mdp_temp_buffer_struct {
	struct mutex lock; /* serialize access buffer state */
	enum MDP_TEMP_BUFFER_STATE state;
	struct mdp_buffer_struct buffer;
	struct ion_handle *handle;
	ion_phys_addr_t pa;
};

/*
** imgresz task info
*/
struct imgresz_task_struct {
	/* imgresz task id, need to get from imgresz */
	IMGRESZ_TICKET ticket;

	struct imgresz_src_buf_info src_buffer; /* src buffer info */
	struct imgresz_dst_buf_info dst_buffer; /* dst buffer info */
};

/*
** rot task info
*/
struct rot_task_struct {
	/* not defined yet */
};

/*
** nr task info
*/
struct nr_task_struct {
	/* struct NR_BUFINFO src_buffer; */
	/* struct NR_BUFINFO dst_buffer; */

};

union MDP_TASK_INFO {
	struct imgresz_task_struct imgresz_task;
	struct rot_task_struct rot_task;
	struct nr_task_struct nr_task;
};

/* store ion handle convert from ion fd. */
struct mdp_ion_struct {
	struct ion_handle *ionHandle1;
	struct ion_handle *ionHandle2;
};

/*
** one mdp task info copy from struct mdp_command_struct.
*/
struct mdp_task_struct {
	struct work_struct task_work; /* for workqueue */
	enum MDP_TASK_STATE current_state; /* record current task state. */
	struct list_head list_entry; /* mdp_task_struct will put to list */
	struct mdp_command_struct mdp_command; /* input data from userspace */
	struct mdp_ion_struct src_ion_handle; /* store ion handle convert from ion fd. */
	struct mdp_ion_struct dst_ion_handle; /* store ion handle convert from ion fd. */
	struct task_time_item_struct time; /* record task time */

	/* buffer fence */
	struct mdp_fence_struct *p_src_fence;
	struct mdp_fence_struct *p_dst_fence;

	/* invalidate part */
	union MDP_TASK_INFO task;
};

struct mmp_mdp_task_struct {
	enum MDP_TASK_STATE current_state;
	char taskString[MAX_ARRAY_SIZE];
	int src_release_fence;
	int dst_release_fence;
	struct mdp_command_struct mdp_command;
	/* struct imgresz_task_struct imgresz_task; */
};

enum MDP_CONFIG_WORK_QUEUE_TYPE {
	MDP_CONFIG_WORK_QUEUE_TYPE_ACQUIRE_TEMP_BUFFER = 0,
	MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_NORMAL = 1,
	MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_UFO_1HW = 2,
	MDP_CONFIG_WORK_QUEUE_TYPE_IMGRESZ_UFO_2HW = 3,
	MDP_CONFIG_WORK_QUEUE_TYPE_NR = 4,
	MDP_CONFIG_WORK_QUEUE_TYPE_ROT = 5,
	MDP_CONFIG_WORK_QUEUE_TYPE_MAX,
};

enum MDP_TRIGGER_WORK_QUEUE_TYPE {
	MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW0 = 0,
	MDP_TRIGGER_WORK_QUEUE_TYPE_IMGRESZ_HW1 = 1,
	MDP_TRIGGER_WORK_QUEUE_TYPE_MAX,
};


#endif /* end of __MDP_DEF_H__ */
