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

/** @file drv_osd_if.h
 *  This header file declares public function prototypes of OSD.
 */


#ifndef _OSD_IF_H_
#define _OSD_IF_H_

#include <linux/irqreturn.h>
#include "disp_info.h"
#include "disp_type.h"
#include "disp_def.h"
#include "disp_hw_mgr.h"

#define OSD_DRV_NAME  "disp_drv_osd"
#define OSD_LAYER_MAX_COUNT 4
#define OSD_DISP_TIMELINE_COUNT OSD_LAYER_MAX_COUNT
#define OSD_LAYER_SUPPORT_COUNT 1
#define CONFIG_DRV_3D_SUPPORT  1
#define CONFIG_OSD_TEST 0
#define CONFIG_DRV_FAST_LOGO 1
#define CONFIG_DRV_KTHREAD_TEST 0
#define CONFIG_DRV_LIST_TEST 1
#define MAX_OSD_INPUT_CONFIG		2
#define OSD_REGION_ALPHA_DETECT_EN 1

#define OSD_IOMMU_SUPPORT 1
#define OSD_PLANE_MAX_WINDOW 2



#if OSD_REGION_ALPHA_DETECT_EN
#define OSD_RGN_REG_NUM                8
#define OSD_RGN_CLUSTER_NUM            9
#define OSD_RGN_REG_LIST               4
#define OSD_MAX_NUM_RGN_LIST       3
#else
#define OSD_RGN_REG_NUM                12
#define OSD_RGN_CLUSTER_NUM            1
#define OSD_RGN_REG_LIST               4
#define OSD_MAX_NUM_RGN_LIST       3
#endif

#define MTK_OSD_NO_FENCE_FD        ((int)(-1))	/* ((int)(~0U>>1)) */
#define MTK_OSD_NO_ION_FD        ((int)(-1))	/* ((int)(~0U>>1)) */

#define VERIFYED(x)		((void)(x))

typedef enum {
	OSD_PLANE_1 = 0,
	OSD_PLANE_2,
	OSD_PLANE_MAX_NUM
} OSD_PLANE_T;

typedef enum {
	OSD_FMT_1 = 0,
	OSD_FMT_2,
	OSD_FMT_MAX_NUM
} OSD_FMT_T;

typedef enum {
	OSD_SCALER_1 = 0,
	OSD_SCALER_2,
	OSD_SCALER_MAX_NUM
} OSD_SCALER_T;

typedef enum {
	OSD_RET_OK,
	OSD_RET_INV_ARG,
	OSD_RET_OUT_OF_MEM,
	OSD_RET_OUT_OF_REGION,
	OSD_RET_OUT_OF_LIST,
	OSD_RET_UNINIT,
	OSD_RET_INV_PLANE,
	OSD_RET_INV_SCALER,
	OSD_RET_INV_REGION,
	OSD_RET_INV_BITMAP,
	OSD_RET_INV_LIST,
	OSD_RET_INV_DISPLAY_MODE,
	OSD_RET_REGION_COLLISION,
	OSD_RET_ERR_INTERNAL,
	OSD_RET_FENCE_FAIL,
	OSD_RET_INV_CMD,
	OSD_RET_DTS_FAIL,
	OSD_RET_UPDATE_FAIL,
	OSD_RET_UNDEF_ERR
} OSD_RET_CODE_T;

typedef enum {
	OSD_CM_YCBCR_CLUT2,
	OSD_CM_YCBCR_CLUT4,
	OSD_CM_YCBCR_CLUT8,
	OSD_CM_RESERVED_0,
	OSD_CM_CBYCR565_DIRECT16 = OSD_CM_RESERVED_0,
	OSD_CM_CBYCRY422_DIRECT16,
	OSD_CM_AYCBCR1555_DIRECT16 = OSD_CM_CBYCRY422_DIRECT16,
	OSD_CM_YCBYCR422_DIRECT16,
	OSD_CM_AYCBCR4444_DIRECT16 = OSD_CM_YCBYCR422_DIRECT16,
	OSD_CM_AYCBCR8888_DIRECT32,
	OSD_CM_RESERVED_1,
	OSD_CM_RGB_CLUT2,
	OSD_CM_RGB_CLUT4,
	OSD_CM_RGB_CLUT8,
	OSD_CM_RGB565_DIRECT16,
	OSD_CM_ARGB1555_DIRECT16,
	OSD_CM_ARGB4444_DIRECT16,
	OSD_CM_ARGB8888_DIRECT32,
	OSD_CM_RESERVED_2
} OSD_COLOR_MODE_T;

typedef enum {
	OSD_RGN_ALLOC,
	OSD_RGN_PREV,
	OSD_RGN_NEXT,
	OSD_RGN_FLAGS,
	OSD_RGN_POS_X,
	OSD_RGN_POS_Y,
	OSD_RGN_BMP_W,
	OSD_RGN_BMP_H,
	OSD_RGN_DISP_W,
	OSD_RGN_DISP_H,
	OSD_RGN_OUT_W,
	OSD_RGN_OUT_H,
	OSD_RGN_COLORMODE,
	OSD_RGN_ALPHA,
	OSD_RGN_BMP_ADDR,
	OSD_RGN_BMP_PITCH,
	OSD_RGN_CLIP_V,
	OSD_RGN_CLIP_H,
	OSD_RGN_V_FLIP,
	OSD_RGN_H_MIRROR,
	OSD_RGN_PAL_LOAD,
	OSD_RGN_PAL_ADDR,
	OSD_RGN_PAL_LEN,
	OSD_RGN_STEP_V,
	OSD_RGN_STEP_H,
	OSD_RGN_COLOR_KEY,
	OSD_RGN_COLOR_KEY_EN,
	OSD_RGN_MIX_SEL,
	OSD_RGN_BIG_ENDIAN,
	OSD_RGN_ALPHA_SEL,
	OSD_RGN_YR_SEL,
	OSD_RGN_UG_SEL,
	OSD_RGN_VB_SEL,
	OSD_RGN_NEXT_EN,
	OSD_RGN_NEXT_HDR_ADDR,
	OSD_RGN_FIFO_EX,
	OSD_RGN_SELECT_BYTE_EN,
	OSD_RGN_RGB2YCBCR_EN,
	OSD_RGN_YCBCR709EN,
	OSD_RNG_XVYCC_EN,
	OSD_RGN_DECOMP_EN,
	OSD_RGN_DECOMP_LINE_BASED,
	OSD_RGN_WT_EN,
	OSD_RGN_DECOMP_MODE,
	OSD_RGN_ACS_FRAME_MODE,
	OSD_RGN_ACS_TOP,
	OSD_RGN_ACS_AUTO
} OSD_RGN_CMD_T;


typedef struct _OSD_AYCBCR_T {
	UINT8 u1Alpha;
	UINT8 u1Y;
	UINT8 u1Cb;
	UINT8 u1Cr;
} OSD_AYCBCR_T;

typedef enum {
	list_new,
	list_configed,
	list_updated,
	list_useless,
	list_read_done
} List_Buffer_STATE;

struct mtk_disp_layer_win {
	bool active;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

struct disp_osd_layer_config {
	unsigned int layer_id;
	unsigned int layer_enable;

	void *src_base_addr;
	unsigned int src_phy_addr;

	enum DISP_HW_COLOR_FORMAT src_fmt;
	unsigned int src_use_color_key;
	unsigned int src_pitch;
	unsigned int src_offset_x, src_offset_y;
	unsigned int src_width, src_height;
	unsigned int tgt_offset_x, tgt_offset_y;
	unsigned int tgt_width, tgt_height;
	unsigned int alpha_enable;
	unsigned int alpha;
	unsigned int sbs;
	unsigned int HDR2SDR;
	unsigned int SDR2HDR;
	unsigned int BT2020;

	int next_buff_idx;
	int identity;
	int connected_type;
	unsigned int security;

	int index;	/* fence count */
	int pre_index;	/* present fence count */
	int acquire_fence_fd;	/* fence fd */
	int ion_fd;
	int release_fence_fd;
	int present_fence_fd;
	struct mtk_disp_layer_win win[OSD_PLANE_MAX_WINDOW];
	struct ion_handle *ion_handle;
	uint32_t region_id;
	HDMI_VIDEO_RESOLUTION res_mode;
	bool osd_swap;
};

typedef struct {
	int fence_fd;
	struct disp_osd_layer_config config;
} disp_osd_input_config;

typedef struct _osd_buffer_list {
	List_Buffer_STATE list_buf_state;
	unsigned int index;	/* fence count */
	int fence_fd;		/*acquire fence */
	struct sync_fence *fences;
	struct ion_handle *ion_handles;
	unsigned int *phy_addr;
	disp_osd_input_config buffer_info;
	struct list_head list;
	unsigned long vsync_cnt;
	int buffer_paired;
} Osd_buffer_list;

struct osd_resolution_change {
	bool fg_need_change_res;
	bool fg_res_vsync1;
	bool fg_res_vsync2;
	bool fg_res_vsync3;
	bool is_progressive;
	bool is_hd;
	uint16_t htotal;
	uint16_t vtotal;
	uint16_t width;
	uint16_t height;
	uint16_t frequency;
	HDMI_VIDEO_RESOLUTION osd_res_mode;
	HDMI_VIDEO_RESOLUTION cur_res_mode;
	wait_queue_head_t event_res;
	atomic_t event_res_flag;
};

struct osd_hdmi_plug {
	bool fg_hdmi_plug_out;
	bool fg_vsync1;
	bool fg_vsync2;
};

struct osd_reg_pararm {
	uintptr_t osd_fmt_reg_base[OSD_FMT_MAX_NUM];
	uintptr_t osd_pln_reg_base[OSD_PLANE_MAX_NUM];
	uintptr_t osd_scl_reg_base[OSD_SCALER_MAX_NUM];
	uintptr_t osd_pmx_reg_base;
};

enum OSD_LAYER_STATUS {
	OSD_LAYER_STOP = 0,
	OSD_LAYER_DISABLE = 1,
	OSD_LAYER_STOPPED = 2,
	OSD_LAYER_START = 3,
	OSD_LAYER_STOPPING = 4,
};

struct osd_stop_control {
	enum OSD_LAYER_STATUS osd_stop_status[OSD_PLANE_MAX_NUM];
	unsigned int osd_start_pts[OSD_PLANE_MAX_NUM];
	unsigned int osd_stop_pts[OSD_PLANE_MAX_NUM];
	unsigned int osd_stop_rel_fence_idx[OSD_PLANE_MAX_NUM];
};

struct osd_rgn_info {
	unsigned int osd_rgn_idx[OSD_PLANE_MAX_NUM];
	int osd_multi_region_rdy[OSD_PLANE_MAX_NUM];
};

struct osd_drop_check {
	unsigned long last_buffer_addr[OSD_PLANE_MAX_NUM];
	unsigned int last_alpha_en[OSD_PLANE_MAX_NUM];
	unsigned int last_alpha[OSD_PLANE_MAX_NUM];
	unsigned int last_tgt_x_offset[OSD_PLANE_MAX_NUM];
	unsigned int last_tgt_y_offset[OSD_PLANE_MAX_NUM];
	unsigned int last_tgt_width[OSD_PLANE_MAX_NUM];
	unsigned int last_tgt_height[OSD_PLANE_MAX_NUM];
	unsigned int last_src_x_offset[OSD_PLANE_MAX_NUM];
	unsigned int last_src_y_offset[OSD_PLANE_MAX_NUM];
	unsigned int last_src_width[OSD_PLANE_MAX_NUM];
	unsigned int last_src_height[OSD_PLANE_MAX_NUM];
};


struct osd_context_t {
	HDMI_VIDEO_RESOLUTION res;
	struct osd_reg_pararm osd_reg;
	struct osd_resolution_change res_chg[OSD_PLANE_MAX_WINDOW];
	struct osd_stop_control osd_stop_ctl;
	enum OSD_LAYER_STATUS osd_stop_status;
	struct osd_rgn_info rgn;
	struct mtk_disp_layer_win win[OSD_PLANE_MAX_WINDOW];
	unsigned char win_cnt;
	int update_tl_idx[OSD_PLANE_MAX_WINDOW];

	unsigned int osd_try_in_suspend[OSD_PLANE_MAX_WINDOW];
	bool osd_in_suspend[OSD_PLANE_MAX_WINDOW];
	bool osd_initial;
	char *osd_suspend_save_info;
	wait_queue_head_t suspend_event_res[OSD_PLANE_MAX_WINDOW];
	atomic_t suspend_event_res_flag[OSD_PLANE_MAX_WINDOW];
	wait_queue_head_t irq_res;
	atomic_t irq_res_flag;

	struct mutex stop_sync_lock[OSD_PLANE_MAX_WINDOW];
	struct mutex config_setting_lock[OSD_PLANE_MAX_WINDOW];
	struct mutex osd_queue_lock[OSD_PLANE_MAX_WINDOW];
	struct mutex osd_config_queue_lock[OSD_PLANE_MAX_WINDOW];
	struct mutex osd_release_buf_lock[OSD_PLANE_MAX_WINDOW];
	unsigned long osd_irq_thread_update[OSD_PLANE_MAX_WINDOW];
	unsigned long osd_config_thread_update[OSD_PLANE_MAX_WINDOW];
	Osd_buffer_list *osd_current_frame[OSD_PLANE_MAX_WINDOW];
	int osd_layer_paired;
	atomic_t last_vsync_trigger[OSD_PLANE_MAX_WINDOW];
	unsigned int last_cmd_fence_idx[OSD_PLANE_MAX_WINDOW];
	unsigned int dim_layer_phy_addr;
	void *dim_layer_virt_addr;
	struct osd_drop_check drop_check_info;
	atomic_t config_cmd_count;
	struct osd_hdmi_plug plug_out[OSD_PLANE_MAX_NUM];
	bool osd_swap;
};

struct osd_fading_ratio_para {
	bool  fg_fading_ratio_update;
	uint32_t fading_ratio_alpha;
	uint32_t header_addr;
};

#define OSD_VERIFY_PLANE(X) \
	do { \
		if (X >= OSD_PLANE_MAX_NUM) \
			; \
	} while (0)

#define OSD_VERIFY_SCALER(X) \
	do { \
		if (X >= OSD_SCALER_MAX_NUM) \
			; \
	} while (0)

#define OSD_VERIFY_REGION(X) \
	do { \
		if ((X & 0xffff) >= OSD_RGN_REG_NUM) \
			; \
	} while (0)

#define IGNORE_RET(X) \
	do { \
		INT32 i4Ignore; \
		i4Ignore = (INT32)(X); \
	} while (0)

int disp_osd_update_register(uint32_t plane);
void disp_osd_trigger_hw(uint32_t plane);
int	disp_osd_update_register_for_alpha_det(uint32_t plane);
extern void osd_dump(void);
extern int test_osd_buffer(disp_osd_input_config *buffer_info);
extern void osd_engine_clk_enable(HDMI_VIDEO_RESOLUTION res_mode, bool enable);
extern void osd_clk_enable(HDMI_VIDEO_RESOLUTION res_mode, bool enable, unsigned int layer_id);
int osd_reassemble_multi_region(uint32_t plane);
int disp_osd_start(struct disp_hw_common_info *info, unsigned int layer_id);
int disp_osd_stop(unsigned int layer_id);
void disp_osd_vsync_stastic(unsigned int idx);
void osd_release_buffer(unsigned int layer_id);
void disp_osd_dim_layer_buffer_init(void);
void osd_dim_layer_uninit(void);


#endif /*_OSD_IF_H_*/
