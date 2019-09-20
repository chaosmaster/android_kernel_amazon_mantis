#ifndef __DISP_HW_LOG_H__
#define __DISP_HW_LOG_H__

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
#include "mmprofile.h"
#endif
#include "disp_hw_mgr.h"

extern int down_sample_x;
extern int down_sample_y;
extern int dump_enable;
/* sort by thread */
enum MMP_DISP_EVENT {
	MMP_DISP_ROOT = 0,
	/* hwc thread */
	MMP_DISP_HWC = 1,
	MMP_DISP_CONFIG = 2,
	MMP_DISP_WAIT_VSYNC = 3,

	/* hw mgr thread*/
	MMP_DISP_HW = 4,
	MMP_DISP_HW_CONFIG = 5,
	MMP_DISP_HW_CONFIG_DONE = 6,
	MMP_DISP_HW_CHANGE_RES = 7,
	MMP_DISP_HW_EVENT = 8,

	MMP_DISP_BITMAP = 9,
	MMP_DISP_BITMAP_MVDO = 10,
	MMP_DISP_BITMAP_SVDO = 11,
	MMP_DISP_BITMAP_OSD1 = 12,
	MMP_DISP_BITMAP_OSD2 = 13,

	/* hw irq */
	MMP_DISP_IRQ = 14,
	MMP_DISP_FMT = 15,
	MMP_DISP_VDP = 16,

	/* MDP event */
	MMP_DISP_MDP = 17,
	MMP_DISP_MDP_BUF_CONFIG = 18,
	MMP_DISP_MDP_BUF_FILL,
	MMP_DISP_MDP_CREATE_FENCE,
	MMP_DISP_MDP_GET_TICKET,
	MMP_DISP_MDP_SET_SCALE_MODE,
	MMP_DISP_MDP_SCALE_ASYNC,
	MMP_DISP_MDP_SET_HW,
	MMP_DISP_MDP_GOT_IRQ,
	MMP_DISP_MDP_HW_DONE,
	MMP_DISP_MDP_RELEASE_FENCE,
	MMP_DISP_MDP_BUF_CONFIG_END,

	MMP_EVENTS_MAX,
};

enum MMP_DISP_EVENT_TYPE {
	MMP_START,
	MMP_END,
	MMP_PULSE,
};

enum DISP_MMP_EVENT_LEVEL {
	MMP_LEVEL_0,
	MMP_LEVEL_1,
	MMP_LEVEL_2,
	MMP_LEVEL_3,
	MMP_LEVEL_4,
	MMP_LEVEL_MAX,
};

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
struct mmp_disp_event {
	enum DISP_MMP_EVENT_LEVEL level;
	char *name;
	MMP_Event parent;
	MMP_Event value;
};
#endif

struct mtk_disp_log {
	struct device *dev;
	uint32_t mgr_level;
	uint32_t sub_level[DISP_MODULE_NUM];
#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
	MMP_Event parent[MMP_LEVEL_MAX];
	struct mmp_disp_event mmp_event[MMP_EVENTS_MAX];
#endif
};

extern struct mtk_disp_log disp_log;

extern bool print_video_fence_history;


enum DISP_LOG_MASK {
	DISP_LOG_MSG_MASK	= (1 << 0),
	DISP_LOG_FUNC_MASK	= (1 << 1),
	DISP_LOG_IRQ_MASK	= (1 << 2),
	DISP_LOG_SYNC_MASK	= (1 << 3),
	DISP_LOG_MMP_MASK	= (1 << 4),
	DISP_LOG_MGR_MASK	= (1 << 5),
	DISP_LOG_HW_MASK	= (1 << 6),
	DISP_LOG_DEBUG      = (1 << 7),
};

#define DISP_LOG_LEVEL_DEBUG1 (DISP_LOG_MSG_MASK)
#define DISP_LOG_LEVEL_DEBUG2 (DISP_LOG_MSG_MASK | DISP_LOG_FUNC_MASK)
#define DISP_LOG_LEVEL_DEBUG3 (DISP_LOG_MSG_MASK | DISP_LOG_FUNC_MASK \
				| DISP_LOG_SYNC_MASK | DISP_LOG_MMP_MASK)
#define DISP_LOG_LEVEL_DEBUG4 (DISP_LOG_MSG_MASK | DISP_LOG_FUNC_MASK | DISP_LOG_IRQ_MASK\
				| DISP_LOG_SYNC_MASK | DISP_LOG_MMP_MASK | DISP_LOG_MGR_MASK | DISP_LOG_HW_MASK)
#define DISP_LOG_LEVEL_DEBUG5 (DISP_LOG_DEBUG)

#ifndef LOG_TAG
#warning "should define LOG_TAG before include disp_hw_log.h"
#define LOG_TAG
#endif

#define DISP_DEBUG
#ifdef DISP_DEBUG
#define DISP_LOG_D(fmt, args...)
#define DISP_LOG_I(fmt, args...) dev_info(disp_log.dev, "["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_N(fmt, args...) dev_notice(disp_log.dev, "["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_W(fmt, args...) dev_warn(disp_log.dev, "["LOG_TAG"]warn: "fmt, ##args)
#define DISP_LOG_E(fmt, args...) dev_err(disp_log.dev, "["LOG_TAG"]error: "fmt, ##args)

#define DISP_LOG_MSG(fmt, args...) \
	do { \
		if (DISP_LOG_MSG_MASK & disp_log.mgr_level) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#define DISP_LOG_IRQ(fmt, args...) \
	do { \
		if (DISP_LOG_IRQ_MASK & disp_log.mgr_level) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#define DISP_LOG_SYNC(fmt, args...) \
	do { \
		if (DISP_LOG_SYNC_MASK & disp_log.mgr_level) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#define DISP_LOG_MMP(fmt, args...) \
	do { \
		if (DISP_LOG_MMP_MASK & disp_log.mgr_level) \
			DISP_LOG_I("MMP "fmt, ##args); \
	} while (0)

#define DISP_LOG_MGR(fmt, args...) \
	do { \
		if (DISP_LOG_MGR_MASK & disp_log.mgr_level) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#define DISP_LOG_HW(fmt, args...) \
	do { \
		if (DISP_LOG_HW_MASK & disp_log.mgr_level) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#define DISP_LOG_FUNC_ENTER()\
	do { \
		if (DISP_LOG_FUNC_MASK & disp_log.mgr_level) \
			DISP_LOG_I("Enter %s:%s line_%d\n", __FILE__, __func__, __LINE__); \
	} while (0)

#define DISP_LOG_FUNC_LEAVE()\
	do { \
		if (DISP_LOG_FUNC_MASK & disp_log.mgr_level) \
			DISP_LOG_I("Leave %s:%s line_%d\n", __FILE__, __func__, __LINE__); \
	} while (0)

#define DISP_LOG_DEBUG(fmt, args...) \
	do { \
		if (disp_log.mgr_level == DISP_LOG_LEVEL_DEBUG5) \
			DISP_LOG_I(fmt, ##args); \
	} while (0)

#else
#define DISP_LOG_D(fmt, args...)
#define DISP_LOG_I(fmt, args...)
#define DISP_LOG_N(fmt, args...) dev_notice(disp_log.dev, "["LOG_TAG"]"fmt, ##args)
#define DISP_LOG_W(fmt, args...) dev_warn(disp_log.dev, "["LOG_TAG"]warn: "fmt, ##args)
#define DISP_LOG_E(fmt, args...) dev_err(disp_log.dev, "["LOG_TAG"]error: "fmt, ##args)
#define DISP_LOG_MSG(fmt, args...)
#define DISP_LOG_IRQ(fmt, args...)
#define DISP_LOG_SYNC(fmt, args...)
#define DISP_LOG_ION(fmt, args...)
#define DISP_LOG_MGR(fmt, args...)
#define DISP_LOG_HW(fmt, args...)
#define DISP_LOG_FUNC_ENTER()
#define DISP_LOG_FUNC_LEAVE()
#define DISP_LOG_DEBUG(fmt, args...)
#endif

void disp_log_init(struct device *dev);
void disp_log_set_level(uint32_t level);
void disp_log_set_sub_level(enum DISP_MODULE_ENUM module, uint32_t level);

#if defined(CONFIG_MTK_MMPROFILE_SUPPORT) && defined(CONFIG_MMPROFILE)
void DISP_MMP(enum MMP_DISP_EVENT event, enum MMP_DISP_EVENT_TYPE type,
			uint64_t data1, uint64_t data2);
void DISP_MMP_DUMP(enum MMP_DISP_EVENT event, unsigned long addr, uint32_t size,
			enum DISP_HW_COLOR_FORMAT fmt, uint32_t w, uint32_t h);
#define DISP_MMP_STRING(event, str, data1, data2)	\
	mmprofile_log_meta_string_ex(disp_log.mmp_event[event].value, MMPROFILE_FLAG_PULSE, data1, data2, str)
#define DISP_MMP_STRUCT(event, pStruct, struct_type) \
	{ \
		mmp_metadata_structure_t meta_data; \
		meta_data.data1 = 0; \
		meta_data.data2 = 0; \
		strcpy(meta_data.struct_name, #struct_type); \
		meta_data.struct_size = sizeof(struct_type); \
		meta_data.p_data = (void *)(pStruct); \
		mmprofile_log_meta_structure(disp_log.mmp_event[event].value, MMPROFILE_FLAG_PULSE, &meta_data); \
	}

#else
static inline void DISP_MMP(enum MMP_DISP_EVENT event, enum MMP_DISP_EVENT_TYPE type,
			uint64_t data1, uint64_t data2)
{
}

static inline void DISP_MMP_DUMP(enum MMP_DISP_EVENT event, unsigned long addr, uint32_t size,
			enum DISP_HW_COLOR_FORMAT fmt, uint32_t w, uint32_t h)
{
}

#define DISP_MMP_STRUCT(event, pStruct, struct_type)
#define DISP_MMP_STRING(event, str, data1, data2)
#endif

extern void MMProfileEnable(int enable);
extern void MMProfileStart(int start);

#endif
