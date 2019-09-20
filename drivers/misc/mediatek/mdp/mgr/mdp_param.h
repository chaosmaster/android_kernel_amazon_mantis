#ifndef __MDP_PARAM_H__
#define __MDP_PARAM_H__
/* NOTE this file will used by userspace
** 1/ don't use macro in this file
**
*/
#include <linux/types.h>

/* mdp task type */
enum MDP_TASK_TYPE {
	MDP_TASK_TYPE_UNKNOWN = 0, /* task type unknown */
	MDP_TASK_TYPE_NR = 1, /* nr task */
	MDP_TASK_TYPE_ROT = 2, /* ROT task */
	MDP_TASK_TYPE_IMGRESZ = 3, /* imgresz task */
	MDP_TASK_TYPE_IMGRESZ_ROT = 4, /* do imgresz then rot. */
};

#define MAX_ARRAY_SIZE 200


/* Format group: 0-RGB, 1-YUV, 2-Bayer raw, 3-compressed format */
#define DP_COLORFMT_PACK(PACKED, LOOSE, VIDEO, PLANE, COPLANE, HFACTOR, VFACTOR, BITS, GROUP, SWAP_ENABLE, UNIQUEID) (\
	((PACKED)        << 31) |                                                             \
	((LOOSE)         << 30) |                                                             \
	((VIDEO)         << 27) |                                                             \
	((PLANE)         << 24) |                                                             \
	((COPLANE)       << 22) |                                                             \
	((HFACTOR)       << 20) |                                                             \
	((VFACTOR)       << 18) |                                                             \
	((BITS)          << 8)  |                                                             \
	((GROUP)         << 6)  |                                                             \
	((SWAP_ENABLE)   << 5)  |                                                             \
	((UNIQUEID)      << 0))

#define DP_COLOR_GET_10BIT_PACKED(color)        ((0x80000000 & color) >> 31)
#define DP_COLOR_GET_10BIT_LOOSE(color)        (((0xC0000000 & color) >> 30) == 1)
#define DP_COLOR_GET_10BIT(color) (DP_COLOR_GET_10BIT_PACKED(color) | DP_COLOR_GET_10BIT_LOOSE(color))
#define DP_COLOR_GET_10BIT_TILE_MODE(color)    (((0xC0000000 & color) >> 30) == 3)
#define DP_COLOR_GET_UFP_ENABLE(color)          ((0x20000000 & color) >> 29)
#define DP_COLOR_GET_INTERLACED_MODE(color)     ((0x10000000 & color) >> 28)
#define DP_COLOR_GET_BLOCK_MODE(color)          ((0x08000000 & color) >> 27)
#define DP_COLOR_GET_PLANE_COUNT(color)         ((0x07000000 & color) >> 24)
#define DP_COLOR_IS_UV_COPLANE(color)           ((0x00C00000 & color) >> 22)
#define DP_COLOR_GET_H_SUBSAMPLE(color)         ((0x00300000 & color) >> 20)
#define DP_COLOR_GET_V_SUBSAMPLE(color)         ((0x000C0000 & color) >> 18)
#define DP_COLOR_BITS_PER_PIXEL(color)          ((0x0003FF00 & color) >>  8)
#define DP_COLOR_GET_COLOR_GROUP(color)         ((0x000000C0 & color) >>  6)
#define DP_COLOR_GET_SWAP_ENABLE(color)         ((0x00000020 & color) >>  5)
#define DP_COLOR_GET_UNIQUE_ID(color)           ((0x0000001F & color) >>  0)
#define DP_COLOR_GET_HW_FORMAT(color)           ((0x0000001F & color) >>  0)

typedef enum DP_COLOR_ENUM {
	DP_COLOR_UNKNOWN        = 0,
	DP_COLOR_FULLG8         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 3,  0, 20),
	DP_COLOR_FULLG10        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 3,  0, 21),
	DP_COLOR_FULLG12        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 12, 3,  0, 22),
	DP_COLOR_FULLG14        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 14, 3,  0, 26),
	DP_COLOR_UFO10          = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 3,  0, 27),

	DP_COLOR_BAYER8         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 2,  0, 20),
	DP_COLOR_BAYER10        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 10, 2,  0, 21),
	DP_COLOR_BAYER12        = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 12, 2,  0, 22),
	DP_COLOR_RGB48          = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 48, 2,  0, 23),
	DP_COLOR_RGB565_RAW     = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 16, 2,  0, 0),/*for Bayer+Mono raw-16 */

	/* Unified format */
	DP_COLOR_GREY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0,  8, 1,  0, 7),

	DP_COLOR_RGB565         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 16, 0,  0, 0),
	DP_COLOR_BGR565         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 16, 0,  1, 0),
	DP_COLOR_RGB888         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24, 0,  1, 1),
	DP_COLOR_BGR888         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24, 0,  0, 1),
	DP_COLOR_RGBA8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  1, 2),
	DP_COLOR_BGRA8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  0, 2),
	DP_COLOR_ARGB8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  1, 3),
	DP_COLOR_ABGR8888       = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32, 0,  0, 3),

	DP_COLOR_UYVY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  0, 4),
	DP_COLOR_VYUY           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  1, 4),
	DP_COLOR_YUYV           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  0, 5),
	DP_COLOR_YVYU           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 1, 0, 16, 1,  1, 5),

	DP_COLOR_I420           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 1,  8, 1,  0, 8),
	DP_COLOR_YV12           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 1,  8, 1,  1, 8),
	DP_COLOR_I422           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 0,  8, 1,  0, 9),
	DP_COLOR_YV16           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 1, 0,  8, 1,  1, 9),
	DP_COLOR_I444           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 0, 0,  8, 1,  0, 10),
	DP_COLOR_YV24           = DP_COLORFMT_PACK(0, 0,  0,   3,  0, 0, 0,  8, 1,  1, 10),

	DP_COLOR_NV12           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 1,  8, 1,  0, 12),
	DP_COLOR_NV21           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 1,  8, 1,  1, 12),
	DP_COLOR_NV16           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 0,  8, 1,  0, 13),
	DP_COLOR_NV61           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 1, 0,  8, 1,  1, 13),
	DP_COLOR_NV24           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 0, 0,  8, 1,  0, 14),
	DP_COLOR_NV42           = DP_COLORFMT_PACK(0, 0,  0,   2,  1, 0, 0,  8, 1,  1, 14),

	/* Mediatek proprietary format */
	DP_COLOR_420_BLKP_UFO   = DP_COLORFMT_PACK(0, 0,  5,   2,  1, 1, 1, 256, 1, 0, 12),/*Frame mode + Block mode */
	DP_COLOR_420_BLKP       = DP_COLORFMT_PACK(0, 0,  1,   2,  1, 1, 1, 256, 1, 0, 12),/*Frame mode + Block mode */
	DP_COLOR_420_BLKI       = DP_COLORFMT_PACK(0, 0,  3,   2,  1, 1, 1, 256, 1, 0, 12),/*Field mode + Block mode */
	DP_COLOR_422_BLKP       = DP_COLORFMT_PACK(0, 0,  1,   1,  0, 1, 0, 512, 1, 0, 4), /*Frame mode */

	DP_COLOR_PARGB8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 26),
	DP_COLOR_XARGB8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 27),
	DP_COLOR_PABGR8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 28),
	DP_COLOR_XABGR8888      = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 32,  0, 0, 29),

	DP_COLOR_IYU2           = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24,  1, 0, 25),
	DP_COLOR_YUV444         = DP_COLORFMT_PACK(0, 0,  0,   1,  0, 0, 0, 24,  1, 0, 30),

	/* Mediatek proprietary 10bit format */
	DP_COLOR_RGBA1010102    = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 0, 0, 32,  0, 1, 2),
	DP_COLOR_BGRA1010102    = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 0, 0, 32,  0, 0, 2),
	DP_COLOR_UYVY_10P       = DP_COLORFMT_PACK(1, 0,  0,   1,  0, 1, 0, 20,  1, 0, 4),
	DP_COLOR_NV21_10P       = DP_COLORFMT_PACK(1, 0,  0,   2,  1, 1, 1, 10,  1, 1, 12),
	DP_COLOR_420_BLKP_10_H  = DP_COLORFMT_PACK(1, 0,  1,   2,  1, 1, 1, 320, 1, 0, 12),
	DP_COLOR_420_BLKP_10_V  = DP_COLORFMT_PACK(1, 1,  1,   2,  1, 1, 1, 320, 1, 0, 12),
	DP_COLOR_420_BLKP_UFO_10_H  = DP_COLORFMT_PACK(1, 0,  5,   2,  1, 1, 1, 320, 1, 0, 12),
	DP_COLOR_420_BLKP_UFO_10_V  = DP_COLORFMT_PACK(1, 1,  5,   2,  1, 1, 1, 320, 1, 0, 12),

	/* Loose 10bit format */
	DP_COLOR_UYVY_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 0, 4),
	DP_COLOR_VYUY_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 1, 4),
	DP_COLOR_YUYV_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 0, 5),
	DP_COLOR_YVYU_10L       = DP_COLORFMT_PACK(0, 1,  0,   1,  0, 1, 0, 20,  1, 1, 5),
	DP_COLOR_NV12_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 1, 10,  1, 0, 12),
	DP_COLOR_NV21_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 1, 10,  1, 1, 12),
	DP_COLOR_NV16_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 0, 10,  1, 0, 13),
	DP_COLOR_NV61_10L       = DP_COLORFMT_PACK(0, 1,  0,   2,  1, 1, 0, 10,  1, 1, 13),
	DP_COLOR_YV12_10L       = DP_COLORFMT_PACK(0, 1,  0,   3,  0, 1, 1, 10,  1, 1, 8),
	DP_COLOR_I420_10L       = DP_COLORFMT_PACK(0, 1,  0,   3,  0, 1, 1, 10,  1, 0, 8),

/*    DP_COLOR_YUV422I        = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 41),*/ /*Dup to DP_COLOR_YUYV */
/*    DP_COLOR_Y800           = DP_COLORFMT_PACK(1,  0, 1, 0, 8, 1, 42),*/ /*Dup to DP_COLOR_GREY */
/*    DP_COLOR_COMPACT_RAW1   = DP_COLORFMT_PACK(1,  0, 1, 0, 10, 2, 43),*/ /*Dup to Bayer10 */
/*    DP_COLOR_420_3P_YVU     = DP_COLORFMT_PACK(3,  0, 1, 1,  8, 1, 44),*/ /*Dup to DP_COLOR_YV12 */
} DP_COLOR_ENUM;
/*
** interlace color format
*/
enum DpInterlaceFormat {
	eInterlace_None,	/* no interlace */
	eTop_Field,			/* top field interlace */
	eBottom_Field		/* bottom field interlace */
};

typedef enum DP_MEMORY_ENUM {
	DP_MEMORY_VA,
	DP_MEMORY_ION,
	DP_MEMORY_PHY,
	DP_MEMORY_MVA,
	DP_MEMORY_SECURE,
} DP_MEMORY_ENUM;

enum DP_METADATA_ENUM {
	DP_METADATA_JUMP_MODE = 1 << 0,	/* bit 0 for jump mode */
};

struct mdp_buffer_info_struct {
	/* pass from userspace */
	DP_MEMORY_ENUM memory_type;
	int32_t fd;
	uint32_t y_offset;
	uint32_t cb_offset;
	uint32_t cr_offset;
	uint32_t ylen_offset;
	uint32_t clen_offset;

	/* convert in kernel */
	uint32_t secureHandle;
};

/*
** buffer info passed from userspace
*/
struct mdp_buffer_struct {
	/* only used in kernel */

	/* only used in userspace */
	/* buffer fence fd: created in kernel, return to userspace for other module to wait */
	int release_fence_fd;

	/* passed from userspace kernel need to wait this fence before use this buffer */
	int wait_fence_fd;

	/* useed in both kernel & userspace */
	/* buffer address(PA) */
	struct mdp_buffer_info_struct buffer_info; /* buffer info */
	uint32_t y_buffer_address; /* Y plane buffer address */
	uint32_t u_buffer_address; /* u plane buffer address */
	uint32_t v_buffer_address; /* v plane buffer address */

	/* ufo buffer info */
	uint32_t ufo_ylen_buffer_address; /* Y Len buffer address */
	uint32_t ufo_clen_buffer_address; /* C Len buffer address */
	uint32_t ufo_ylen_buffer_len; /* ufo Y Len size */
	uint32_t ufo_clen_buffer_len; /* ufo C Len size */

	/* buffer config info */
	enum DP_COLOR_ENUM color_format;  /* buffer color format */
	uint32_t metaData;	/* extra info */
	uint32_t width; /* width */
	uint32_t height; /* height */
	uint32_t y_pitch; /* pitch */
	uint32_t buffer_height; /* buffer height align */
	uint32_t pic_x_offset;	/* buffer crop */
	uint32_t pic_y_offset;	/* buffer crop */

	/* interlace format */
	enum DpInterlaceFormat interlace_format;
};

/*
** receive ioctl command
*/
struct mdp_command_struct {
	/* do not use pointer in this structure */
	bool is_secure;
	char taskString[MAX_ARRAY_SIZE];
	enum MDP_TASK_TYPE task_type; /* mdp_task_struct use which hardware */
	struct mdp_buffer_struct src_buffer;
	struct mdp_buffer_struct dst_buffer;
};

/* define MDP ioctl command */
#define MDP_IOCTL_MAGIC_NUMBER 'z'
#define MDP_IOCTL_EXEC_COMMAND   _IOW(MDP_IOCTL_MAGIC_NUMBER, 1, struct mdp_command_struct)


#endif /* endof __MDP_PARAM_H__ */

