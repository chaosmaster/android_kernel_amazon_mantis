/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "osd_fence"

#include <linux/file.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/file.h>

#include "disp_osd_if.h"
#include "disp_osd_log.h"
#include "sw_sync.h"
#include "sync.h"
#include "disp_osd_fence.h"


/*#include <../drivers/staging/android/sync.h>*/


#define FENCE_STEP_COUNTER 1
#define OSD_TIMELINE_NAME_PREFIX "osd_timeline"
#define OSD_FENCE_NAME_PREFIX "osd_fence"
#define OSD_PRE_TIMELINE_NAME_PREFIX "osd_pre_timeline"
#define OSD_PRE_FENCE_NAME_PREFIX "osd_pre_fence"


static atomic_t osd_pre_timeline_counter[MAX_OSD_INPUT_CONFIG] = {ATOMIC_INIT(0)};
static atomic_t osd_pre_fence_counter[MAX_OSD_INPUT_CONFIG] = {ATOMIC_INIT(0)};
static struct sw_sync_timeline *osd_pre_timeline[MAX_OSD_INPUT_CONFIG];

static atomic_t osd_timeline_counter[MAX_OSD_INPUT_CONFIG] = {ATOMIC_INIT(0)};
static atomic_t osd_fence_counter[MAX_OSD_INPUT_CONFIG] = {ATOMIC_INIT(0)};
static struct sw_sync_timeline *osd_timeline[MAX_OSD_INPUT_CONFIG];
struct mutex osd_fence_mutex[MAX_OSD_INPUT_CONFIG];

/* * sync_timeline, sync_fence data structure */
struct fence_data {
	__u32 value;
	char name[32];
	__s32 fence;		/* fd of new fence */
};



static void osd_destroy_timeline(struct sw_sync_timeline *obj)
{
	sync_timeline_destroy(&obj->obj);
}

static unsigned int osd_get_fence_counter(unsigned int lay_id, int type)
{
	if (type == 0)
		return (unsigned int)atomic_add_return(FENCE_STEP_COUNTER, &osd_fence_counter[lay_id]);
	else
		return (unsigned int)atomic_add_return(FENCE_STEP_COUNTER, &osd_pre_fence_counter[lay_id]);
}

unsigned int osd_read_fence_counter(unsigned int lay_id)
{
	return (unsigned int)atomic_read(&osd_fence_counter[lay_id]);
}


void osd_sync_destroy(unsigned int lay_id)
{
	if (osd_timeline[lay_id] != NULL) {
		osd_destroy_timeline(osd_timeline[lay_id]);
		osd_timeline[lay_id] = NULL;
	}
	/* Reset all counter to 0 */
	atomic_set(&osd_timeline_counter[lay_id], 0);
	atomic_set(&osd_fence_counter[lay_id], 0);

	if (osd_pre_timeline[lay_id] != NULL) {
		osd_destroy_timeline(osd_pre_timeline[lay_id]);
		osd_pre_timeline[lay_id] = NULL;
	}
	/* Reset all counter to 0 */
	atomic_set(&osd_pre_timeline_counter[lay_id], 0);
	atomic_set(&osd_pre_fence_counter[lay_id], 0);
}


struct sw_sync_timeline *osd_create_timeline(unsigned int lay_id)
{
	char name[32];
	const char *prefix = OSD_TIMELINE_NAME_PREFIX;

	OSD_PRINTF(OSD_FENCE_LOG, "osd_create_timeline come in!\n");
	sprintf(name, "%s", prefix);

	osd_timeline[lay_id] = sw_sync_timeline_create(name);

	if (osd_timeline[lay_id] == NULL)
		OSD_LOG_E("error: cannot create osd_timeline!\n");
	else
		OSDDBG("osd Timeline name=%s created!\n", name);

	prefix = OSD_PRE_TIMELINE_NAME_PREFIX;
	sprintf(name, "%s", prefix);
	osd_pre_timeline[lay_id] = sw_sync_timeline_create(name);

	if (osd_pre_timeline[lay_id] == NULL)
		OSD_LOG_E("error: cannot create osd_pre_timeline!\n");
	else
		OSDDBG("osd Timeline name=%s created!\n", name);

	return osd_timeline[lay_id];
}

void osd_sync_init(unsigned int lay_id)
{

	mutex_init(&osd_fence_mutex[lay_id]);
	osd_create_timeline(lay_id);
	/* Reset all counter to 0 */
	atomic_set(&osd_timeline_counter[lay_id], 0);
	atomic_set(&osd_fence_counter[lay_id], 0);
	atomic_set(&osd_pre_timeline_counter[lay_id], 0);
	atomic_set(&osd_pre_fence_counter[lay_id], 0);
}

static int fence_create(struct sw_sync_timeline *obj, struct fence_data *data)
{
	int fd = get_unused_fd_flags(0);
	int err;
	struct sync_pt *pt;
	struct sync_fence *fence;

	if (fd < 0)
		return fd;

	pt = sw_sync_pt_create(obj, data->value);
	if (pt == NULL) {
		err = -ENOMEM;
		goto err;
	}

	data->name[sizeof(data->name) - 1] = '\0';
	fence = sync_fence_create(data->name, pt);
	if (fence == NULL) {
		sync_pt_free(pt);
		err = -ENOMEM;
		goto err;
	}

	data->fence = fd;
	sync_fence_install(fence, fd);
	return 0;

err:
	put_unused_fd(fd);
	return err;
}

int osd_create_fence(int *pfence, int *rfence, unsigned int *pvalue,
			unsigned int *rvalue, unsigned int lay_id)
{
	const char *prefix = OSD_FENCE_NAME_PREFIX;
	struct fence_data data;

	*rfence = MTK_OSD_NO_FENCE_FD;
	mutex_lock(&osd_fence_mutex[lay_id]);
	data.value = osd_get_fence_counter(lay_id, 0);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	sprintf(data.name, "%s-%d", prefix, data.value);

	if (osd_timeline[lay_id] != NULL) {
		if (fence_create(osd_timeline[lay_id], &data)) {
			data.fence = MTK_OSD_NO_FENCE_FD;
			OSD_LOG_E("cannot create Fence Obj %d\n", -OSD_RET_FENCE_FAIL);
			return -OSD_RET_FENCE_FAIL;
		}

		*rfence = data.fence;
		*rvalue = data.value;
	} else {
		OSD_LOG_E("no Timeline to create Fence!\n");
	}

	prefix = OSD_PRE_FENCE_NAME_PREFIX;
	*pfence = MTK_OSD_NO_FENCE_FD;
	mutex_lock(&osd_fence_mutex[lay_id]);
	data.value = osd_get_fence_counter(lay_id, 1);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	sprintf(data.name, "%s-%d", prefix, data.value);
	if (osd_pre_timeline[lay_id] != NULL) {
		if (fence_create(osd_pre_timeline[lay_id], &data)) {
			data.fence = MTK_OSD_NO_FENCE_FD;
			OSD_LOG_E("cannot create Fence Obj %d\n", -OSD_RET_FENCE_FAIL);
			return -OSD_RET_FENCE_FAIL;
		}

		*pfence = data.fence;
		*pvalue = data.value;
	} else {
		OSD_LOG_E("no Timeline to create Pre-Fence!\n");
	}
	return OSD_RET_OK;
}


void osd_release_all_fence(unsigned int lay_id, unsigned int fence_count, unsigned int pre_fence_count)
{
	int inc = fence_count - atomic_read(&osd_timeline_counter[lay_id]);
	int pre_inc = fence_count - atomic_read(&osd_pre_timeline_counter[lay_id]);

	if (inc > 0) {
		OSD_PRINTF(OSD_FENCE_LOG, "rel all fence:lay_id=%d, inc=%d, fence1=%d, fence2=%d!\n", lay_id, inc,
			atomic_read(&osd_fence_counter[lay_id]), atomic_read(&osd_timeline_counter[lay_id]));

		if (osd_timeline[lay_id] != NULL) {
			/* timeline_inc(hdmi_timeline, inc); */
			sw_sync_timeline_inc(osd_timeline[lay_id], inc);
		}

		mutex_lock(&osd_fence_mutex[lay_id]);
		atomic_add(inc, &osd_timeline_counter[lay_id]);
		mutex_unlock(&osd_fence_mutex[lay_id]);
	}

	if (pre_inc > 0) {
		OSD_PRINTF(OSD_FENCE_LOG, "rel all pre-fence:lay_id=%d, inc=%d, fence1=%d, fence2=%d!\n", lay_id, pre_inc,
			atomic_read(&osd_pre_fence_counter[lay_id]), atomic_read(&osd_pre_timeline_counter[lay_id]));

		if (osd_pre_timeline[lay_id] != NULL) {
			/* timeline_inc(hdmi_timeline, inc); */
			sw_sync_timeline_inc(osd_pre_timeline[lay_id], pre_inc);
		}

		mutex_lock(&osd_fence_mutex[lay_id]);
		atomic_add(pre_inc, &osd_pre_timeline_counter[lay_id]);
		mutex_unlock(&osd_fence_mutex[lay_id]);
	}

}


/**
 * timeline_counter records counter of this timeline.
 * It should be always posterior to fence_counter when enable is true, otherwise
 * they're equaled
 * timeline_counter will step forward and present current hw used buff counter
 * NOTICE:
 *     Frame dropping maybe happen, we has no cache FIFO now!
 *     When a new buffer is coming, all prior to it will be released
 *     Buf will be released immediately if ovl_layer is disabled
 */
unsigned int osd_timeline_inc(unsigned int lay_id)
{
	unsigned int fence_cnt, timeline_cnt, inc;

	mutex_lock(&osd_fence_mutex[lay_id]);
	fence_cnt = atomic_read(&osd_fence_counter[lay_id]);
	timeline_cnt = atomic_read(&osd_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	inc = fence_cnt - timeline_cnt;

	if (inc < 0 || inc > 3) {
		OSD_LOG_I("fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d!\n", inc, fence_cnt,
		       timeline_cnt);
		inc = 0;
	}
	mutex_lock(&osd_fence_mutex[lay_id]);
	atomic_add(1, &osd_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	return atomic_read(&osd_timeline_counter[lay_id]);
}

/**
 * step forward timeline
 * all fence(sync_point) will be signaled prior to it's counter
 * refer to {@link sw_sync_timeline_inc}
 */
void osd_signal_fence(unsigned int lay_id, unsigned int index)
{
	unsigned int fence_cnt, timeline_cnt, inc;

	mutex_lock(&osd_fence_mutex[lay_id]);
	fence_cnt = atomic_read(&osd_fence_counter[lay_id]);
	timeline_cnt = atomic_read(&osd_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	inc = fence_cnt - timeline_cnt;

	if (inc <= 0)
		return;

	if (inc > 3) {
		OSDDBG("fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d!\n", inc, fence_cnt,
		       timeline_cnt);
	}

	inc = index - timeline_cnt;

	mutex_lock(&osd_fence_mutex[lay_id]);
	atomic_add(inc, &osd_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);

	if (osd_timeline[lay_id] != NULL) {
		sw_sync_timeline_inc(osd_timeline[lay_id], inc);
		/*OSDDBG("fence  : value=%d\n", osd_timeline[lay_id]->value);*/
		OSD_PRINTF(OSD_FENCE_LOG, "fence %ld : inc=%d, fence_cnt=%d, timeline_cnt=%d!\n",
			vsync_cnt, inc, atomic_read(&osd_fence_counter[lay_id]),
			atomic_read(&osd_timeline_counter[lay_id]));
	} else {
		OSD_LOG_E("no Timeline to inc tl %d, fd %d\n", atomic_read(&osd_timeline_counter[lay_id]),
			  atomic_read(&osd_fence_counter[lay_id]));
	}
}

void osd_signal_pre_fence(unsigned int lay_id, unsigned int index)
{
	unsigned int fence_cnt, timeline_cnt, inc;

	mutex_lock(&osd_fence_mutex[lay_id]);
	fence_cnt = atomic_read(&osd_pre_fence_counter[lay_id]);
	timeline_cnt = atomic_read(&osd_pre_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);
	inc = fence_cnt - timeline_cnt;

	if (inc < 0 || inc > 3) {
		OSDDBG("pre-fence error: inc=%d, fence_cnt=%d, timeline_cnt=%d!\n", inc, fence_cnt,
		       timeline_cnt);
	}

	inc = index - timeline_cnt;

	mutex_lock(&osd_fence_mutex[lay_id]);
	atomic_add(inc, &osd_pre_timeline_counter[lay_id]);
	mutex_unlock(&osd_fence_mutex[lay_id]);

	if (osd_pre_timeline[lay_id] != NULL) {
		sw_sync_timeline_inc(osd_pre_timeline[lay_id], inc);
		/*OSDDBG("fence  : value=%d\n", osd_timeline[lay_id]->value);*/
		OSD_PRINTF(OSD_FENCE_LOG, "fence %ld : inc=%d, fence_cnt=%d, timeline_cnt=%d!\n",
			vsync_cnt, inc, atomic_read(&osd_pre_fence_counter[lay_id]),
			atomic_read(&osd_pre_timeline_counter[lay_id]));
	} else {
		OSD_LOG_E("no Timeline to inc tl %d, fd %d\n", atomic_read(&osd_pre_timeline_counter[lay_id]),
			  atomic_read(&osd_pre_fence_counter[lay_id]));
	}
}

