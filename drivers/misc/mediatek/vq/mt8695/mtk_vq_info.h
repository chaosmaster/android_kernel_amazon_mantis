/*
 * Copyright (c) 2017 MediaTek Inc.
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

#ifndef _MTK_VQ_INFO_H_
#define _MTK_VQ_INFO_H_

#include "linux/types.h"

#define DI_ASYNC_SUPPORT   0

#define MTK_VQ_BUFFER_COUINT  (5)

#define MTK_VQ_DI_INPUTBUFFER  (3)

enum VQ_FIELD_TYPE {
	VQ_FIELD_TYPE_TOP,
	VQ_FIELD_TYPE_BOTTOM,
	VQ_FIELD_TYPE_MAX
};

enum VQ_DI_MODE {
	VQ_DI_MODE_FRAME,
	VQ_DI_MODE_4_FIELD,
	VQ_DI_MODE_FIELD,
	VQ_DI_MODE_MAX
};

enum VQ_COLOR_FMT {
	VQ_COLOR_FMT_420BLK,
	VQ_COLOR_FMT_420SCL,
	VQ_COLOR_FMT_422BLK,
	VQ_COLOR_FMT_422SCL,
	VQ_COLOR_FMT_MAX
};

enum VQ_PATH_MODE {
	VQ_DI_NR_DIRECTLINK_ALL_ENABLE = 0,
	VQ_DI_NR_DIRECTLINK_DI_BYPASS,
	VQ_DI_NR_DIRECTLINK_NR_BYPASS,
	VQ_DI_STANDALONE,
	VQ_NR_STANDALONE
};

enum VQ_TIMING_TYPE {
	VQ_TIMING_TYPE_480P,
	VQ_TIMING_TYPE_576P,
	VQ_TIMING_TYPE_720P,
	VQ_TIMING_TYPE_1080P,
	VQ_TIMING_TYPE_MAX
};
enum VDO_DI_MODE {
	VDO_FRAME_MODE,
	VDO_FIELD_MODE,
	VDO_INTRA_MODE_WITH_EDGE_PRESERVING,
	VDO_4FIELD_MA_MODE,
	VDO_FUSION_MODE,
	VDO_8FIELD_MA_MODE,
	VDO_DI_MODE_NUM
};

struct mtk_vq_config {
	/*for secure mode */
	bool secruity_en;

	enum VQ_PATH_MODE vq_mode;

	enum VQ_COLOR_FMT src_fmt;
	enum VQ_COLOR_FMT dst_fmt;

	int src_width;
	int src_height;

	int src_align_width;
	int src_align_height;

	unsigned int src_ofset_y_len[MTK_VQ_BUFFER_COUINT];
	unsigned int src_ofset_c_len[MTK_VQ_BUFFER_COUINT];
	unsigned int dst_ofset_y_len;
	unsigned int dst_ofset_c_len;

	/*ion fd */
	int src_fd[MTK_VQ_BUFFER_COUINT];
	int dst_fd;

	/*NR config */
	unsigned int bnr_level;
	unsigned int mnr_level;
	enum VQ_DI_MODE di_mode;
	enum VQ_FIELD_TYPE cur_field;


	bool topfield_first_enable;
	bool h265_enable;
};

struct mtk_vq_config_info {
	struct mtk_vq_config *vq_config;

	struct ion_handle *src_ion_handle[MTK_VQ_BUFFER_COUINT];
	struct ion_handle *dst_ion_handle;

	unsigned int src_mva[MTK_VQ_BUFFER_COUINT];
	unsigned int dst_mva;

	void *src_va[MTK_VQ_BUFFER_COUINT];
	void *dst_va;


};

#define MTK_VQ_IOW(num, dtype)     _IOW('O', num, dtype)
#define MTK_VQ_IOR(num, dtype)     _IOR('O', num, dtype)
#define MTK_VQ_IOWR(num, dtype)    _IOWR('O', num, dtype)
#define MTK_VQ_IO(num)             _IO('O', num)

#define MTK_VQ_IOCTL_SET_INPUT_CONFIG    MTK_VQ_IOWR(0xe0, struct mtk_vq_config)

#endif				/* _MTK_VQ_INFO_H_ */
