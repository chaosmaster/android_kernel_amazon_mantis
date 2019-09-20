/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Qing Li <qing.li@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#ifndef __MTK_VQ_DEF_H__
#define __MTK_VQ_DEF_H__

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mem2mem.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-dma-contig.h>
#include <media/videobuf2-vmalloc.h>

#define MTK_VQ_SUPPORT_NR       1

#define MTK_VQ_MODULE_NAME      "mtk-vq"

#define MTK_VQ_SHUTDOWN_TIMEOUT ((100*HZ)/1000)

#define MTK_VQ_PARAMS           (1 << 0)
#define MTK_VQ_SRC_FMT          (1 << 1)
#define MTK_VQ_DST_FMT          (1 << 2)
#define MTK_VQ_CTX_M2M          (1 << 3)
#define MTK_VQ_CTX_STOP_REQ     (1 << 6)
#define MTK_VQ_CTX_ABORT        (1 << 7)

#define S_BUF_COUNT             3

enum mtk_vq_dev_flags {
	/* for global */
	ST_SUSPEND,

	/* for m2m node */
	ST_M2M_OPEN,
	ST_M2M_RUN,
	ST_M2M_PEND,
	ST_M2M_SUSPENDED,
	ST_M2M_SUSPENDING,
};

enum mtk_vq_irq {
	MTK_VQ_IRQ_DONE,
	MTK_VQ_IRQ_OVERRUN
};

enum mtk_vq_color_fmt {
	MTK_VQ_YUV420BLK = 0x1,
	MTK_VQ_YUV420SCL = 0x2,
	MTK_VQ_YUV422BLK = 0x4,
	MTK_VQ_YUV422SCL = 0x8,
};

enum mtk_mdp_yuv_fmt {
	MTK_VQ_LSB_Y = 0x10,
	MTK_VQ_LSB_C,
	MTK_VQ_CBCR = 0x20,
	MTK_VQ_CRCB,
};

#define mtk_vq_fmt_is_420(fmt)      ((fmt == MTK_VQ_YUV420BLK) || (fmt == MTK_VQ_YUV420SCL))
#define mtk_vq_fmt_is_422(fmt)      ((fmt == MTK_VQ_YUV422BLK) || (fmt == MTK_VQ_YUV422SCL))

#define mtk_vq_m2m_active(dev)      test_bit(ST_M2M_RUN, &(dev)->state)
#define mtk_vq_m2m_pending(dev) test_bit(ST_M2M_PEND, &(dev)->state)
#define mtk_vq_m2m_opened(dev)      test_bit(ST_M2M_OPEN, &(dev)->state)

/**
 * struct mtk_vq_fmt - the driver's internal color format data
 * @mbus_code:          Media Bus pixel code, -1 if not applicable
 * @name:               format description
 * @pixelformat:        the fourcc code for this format, 0 if not applicable
 * @yorder:             Y/C order
 * @corder:             Chrominance order control
 * @num_planes:         number of physically non-contiguous data planes
 * @nr_comp:            number of physically contiguous data planes
 * @depth:              per plane driver's private 'number of bits per pixel'
 * @flags:              flags indicating which operation mode format applies to
 */
struct mtk_vq_fmt {
	enum v4l2_mbus_pixelcode mbus_code;
	char *name;
	u32 pixelformat;
	u32 color;
	u32 yorder;
	u32 corder;
	u16 num_planes;
	u16 num_comp;
	u8 depth[VIDEO_MAX_PLANES];
	u32 flags;
};

/**
 * struct mtk_vq_addr - the video quality physical address set
 * @y:   luminance plane address
 * @cb:  cbcr plane address
 */
struct mtk_vq_addr {
	dma_addr_t y;
	dma_addr_t c;
};

/* struct mtk_vq_ctrls - the video quality control set
 * @di:                     deinterlace
 * @nr:                     noise reduction
 */
struct mtk_vq_ctrls {
	struct v4l2_ctrl *di;
	struct v4l2_ctrl *nr;
};

/**
 * struct mtk_vq_frame - source/target frame properties
 * @f_width:                SRC : SRCIMG_WIDTH, DST : OUTPUTDMA_WHOLE_IMG_WIDTH
 * @f_height:               SRC : SRCIMG_HEIGHT, DST : OUTPUTDMA_WHOLE_IMG_HEIGHT
 * @crop:                   cropped(source)/scaled(destination) size
 * @payload:                image size in bytes (w x h x bpp)
 * @pitch:                  bytes per line of image in memory
 * @fmt:                    color format pointer
 * @colorspace:             value indicating v4l2_colorspace
 * @alpha:                  frame's alpha value
 */
struct mtk_vq_frame {
	u32 f_width;
	u32 f_height;
	struct v4l2_rect crop;
	unsigned long payload[VIDEO_MAX_PLANES];
	unsigned int pitch[VIDEO_MAX_PLANES];
	const struct mtk_vq_fmt *fmt;
	u32 colorspace;
	u8 alpha;
};

/*
 * struct mtk_vq_m2m_device - v4l2 memory-to-memory device data
 * @vfd:                        the video device node for v4l2 m2m mode
 * @m2m_dev:                    v4l2 memory-to-memory device data
 * @ctx:                        hardware context data
 * @refcnt:                     the reference counter
 */
struct mtk_vq_m2m_device {
	struct v4l2_m2m_dev *m2m_dev;
	int refcnt;
};

/*
 *  struct mtk_vq_pix_max - picture pixel size limits in various IP configurations
 *  @org_w:                 max  source pixel width
 *  @org_h:                 max  source pixel height
 *  @target_w:              max  output pixel height
 *  @target_h:              max  output pixel height
 */
struct mtk_vq_pix_max {
	u16 org_w;
	u16 org_h;
	u16 target_w;
	u16 target_h;
};

/*
 *  struct mtk_vq_pix_min - picture pixel size limits in various IP configurations
 *
 *  @org_w: minimum source pixel width
 *  @org_h: minimum source pixel height
 *  @target_w: minimum output pixel height
 *  @target_h: minimum output pixel height
 */
struct mtk_vq_pix_min {
	u16 org_w;
	u16 org_h;
	u16 target_w;
	u16 target_h;
};

struct mtk_vq_pix_align {
	u16 org_w;
	u16 org_h;
	u16 target_w;
	u16 target_h;
};

/*
 * struct mtk_vq_variant - video quality variant information
 */
struct mtk_vq_variant {
	struct mtk_vq_pix_max *pix_max;
	struct mtk_vq_pix_min *pix_min;
	struct mtk_vq_pix_align *pix_align;
	u16 in_buf_cnt;
	u16 out_buf_cnt;
};

/*
 * struct mtk_vq_hw_clks - video quality clock
 */
struct mtk_vq_hw_clks {
	struct clk *clk_bdp_f27m;
	struct clk *clk_bdp_f27m_vdout;
	struct clk *clk_bdp_f27_74_74;
	struct clk *clk_bdp_f2fs;
	struct clk *clk_bdp_f2fs74_148;
	struct clk *clk_bdp_fb;
	struct clk *clk_bdp_vdo_dram;
	struct clk *clk_bdp_vdo_2fs;
	struct clk *clk_bdp_vdo_b;
	struct clk *clk_bdp_wr_di_pxl;
	struct clk *clk_bdp_wr_di_dram;
	struct clk *clk_bdp_wr_di_b;
	struct clk *clk_bdp_nr_pxl;
	struct clk *clk_bdp_nr_dram;
	struct clk *clk_bdp_nr_b;
};

/*
 * struct mtk_vq_dev - abstraction for image processor entity
 * @lock:               the mutex protecting this data structure
 * @vqlock:             the mutex protecting the communication with vq
 * @pdev:               pointer to the image processor platform device
 * @variant:            the IP variant information
 * @clks:               clocks required for the video quality operation
 * @irq_queue:          interrupt handler waitqueue
 * @m2m:                memory-to-memory V4L2 device information
 * @alloc_ctx:          videobuf2 memory allocator context
 * @vdev:               video device for image processor instance
 * @larb:               clocks required for the video quality operation
 * @workqueue:          decode work queue
 * @vq_dev:             vq platform device
 * @state:              flags used to synchronize m2m and capture mode operation
 * @id:                 the video quality device index (0..MTK_VQ_MAX_DEVS)
 */
struct mtk_vq_dev {
	struct mutex lock;
	struct mutex vqlock;
	struct platform_device *pdev;
	struct mtk_vq_variant *variant;
	struct mtk_vq_hw_clks clks;
	wait_queue_head_t irq_queue;
	struct mtk_vq_m2m_device m2m;
	struct vb2_alloc_ctx *alloc_ctx;
	struct video_device vdev;
	struct v4l2_device v4l2_dev;
	struct device *larb[2];
	struct workqueue_struct *workqueue;
	struct platform_device *vq_dev;

	unsigned long state;
	u16 id;
};

/*
 * struct mtk_vq_buf - buffer information record
 * @vb:                 vb record point
 * @addr:               addr record
 */
struct mtk_vq_buf {
	struct vb2_buffer *vb;
	struct mtk_vq_addr addr;
};

/*
 * mtk_vq_ctx - the device context data
 * @s_frame:        source frame properties
 * @d_frame:        destination frame properties
 * @vq_dev:         the video quality device which this context applies to
 * @m2m_ctx:        memory-to-memory device context
 * @fh:             v4l2 file handle
 * @ctrl_handler:   v4l2 controls handler
 * @ctrls           image processor control set
 * @qlock:          vb2 queue lock
 * @slock:          the mutex protecting this data structure
 * @work:           worker for image processing
 * @s_buf:          source buffer information record
 * @d_addr:         dest frame buffer physical addresses
 * @s_idx_for_next: source buffer idx for next
 * @flags:          additional flags for image conversion
 * @state:          flags to keep track of user configuration
 * @ctrls_rdy:      true if the control handler is initialized
 * @field_mode:     deinterlace by specified field mode
 */
struct mtk_vq_ctx {
	struct mtk_vq_frame s_frame;
	struct mtk_vq_frame d_frame;
	struct mtk_vq_dev *vq_dev;
	struct v4l2_m2m_ctx *m2m_ctx;
	struct v4l2_fh fh;
	struct v4l2_ctrl_handler ctrl_handler;
	struct mtk_vq_ctrls ctrls;
	struct mutex qlock;
	struct mutex slock;
	struct work_struct work;
	struct mtk_vq_buf s_buf[S_BUF_COUNT];
	struct mtk_vq_addr d_addr;

	u32 s_idx_for_next;
	u32 flags;
	u32 state;
	u32 process_idx;
	bool ctrls_rdy;
	enum v4l2_field field_mode;
};

extern irqreturn_t MTK_VQ_Dispfmt_IrqHandler(int irq, void *dev_id);
extern int mtk_vq_register_m2m_device(struct mtk_vq_dev *vq);
extern void mtk_vq_unregister_m2m_device(struct mtk_vq_dev *vq);

extern void mtk_vq_debug_init(void);
extern void mtk_vq_debug_exit(void);

#endif				/* __MTK_VQ_DEF_H__ */
