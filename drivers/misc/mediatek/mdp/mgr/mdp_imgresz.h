#ifndef __MDP_IMGRESZ_H__
#define __MDP_IMGRESZ_H__
#include "imgresz.h"
#include "mdp_def.h"

extern int mdp_release_frame_count;

/*
** init mdp imgresz: create thread for dispatch imgresz hw
*/
enum MDP_TASK_STATUS mdp_imgresz_init(void);


/*
** map MDP color format: enum DP_COLOR_ENUM
** to imgresz source color format: enum IMGRESZ_SRC_COLOR_MODE
*/
enum IMGRESZ_SRC_COLOR_MODE mdp_imgresz_map_src_format(enum DP_COLOR_ENUM color_format);

/*
** map MDP color format: enum DP_COLOR_ENUM
** to imgresz source color format: enum IMGRESZ_DST_COLOR_MODE
*/
enum IMGRESZ_DST_COLOR_MODE mdp_imgresz_map_dst_format(enum DP_COLOR_ENUM color_format);


/*
** fill mdp task for imgresz use.
*/
enum MDP_TASK_STATUS mdp_imgresz_fill_task(struct mdp_task_struct *pTask);

/*
** imgresz hardware ready or not
*/
bool mdp_imgresz_is_hardware_ready(enum imgresz_ticket_fun_type function_type, IMGRESZ_TICKET *pTicket);

/*
** May sleep
** wait a valid imgresz ticket
*/
enum MDP_TASK_STATUS mdp_imgresz_wait_hardware_ready(struct mdp_task_struct *pTask);

/*
** trigger imgresz to work
*/
enum MDP_TASK_STATUS mdp_imgresz_trigger_hw_work(struct mdp_task_struct *pTask);


/*
** imgresz task error
*/
void mdp_imgresz_handle_task_error(struct mdp_task_struct *pTask);

/*
** imgresz task done
*/
void mdp_imgresz_handle_task_done(struct mdp_task_struct *pTask);

/*
** need to handle hardware imgresz done.
** note: this function is called in imgresz irq handler.
** do not sleep in this function.
** do not call imgresz_ticket_put(ticket) in this function.
** need to handle in a new thread
*/
int mdp_imgresz_hw_done_callback(IMGRESZ_TICKET ticket, enum imgresz_cb_state hw_state, void *privdata);

int mdp_imgresz_handle_irq(IMGRESZ_TICKET ti, enum imgresz_cb_state hw_state, void *privdata);


/*
** wait imgresz hardware idle, dispatch first task use this available hardware.
** dispatch ufo hardware first.
** if have ufo task, test ufo hardware available ? dispatch ufo ticket
** else if have normal task, test normal hardware available ? dispatch normal ticket : wait
** we may need to implement a while(1) function.
*/
int32_t mdp_imgresz_dispatch_hw(void *ignore);

/* dump imgresz settings */
void mdp_imgresz_dump_task(struct mdp_task_struct *pTask);



/*
** get imgresz function type by color format
** non ufo format: IMGRESZ_FUN_NORMAL
** ufo format: IMGRESZ_FUN_UFO_2HW
*/
enum imgresz_ticket_fun_type mdp_imgresz_get_function_type(enum DP_COLOR_ENUM format, int width, int height);



/*
** release imgresz hardware
*/
enum MDP_TASK_STATUS mdp_imgresz_release_hw(struct mdp_task_struct *pTask);

#endif

