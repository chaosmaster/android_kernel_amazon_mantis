/*
* Copyright (C) 2015-2016 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*/

#define STC_SESSION_DEVICE "mtk_stc"


/*******************************************************************************
 * REGISTER OFFSET DEFINITIONS
 ******************************************************************************/
#define STC_SYSTOP_CONFIG           (0x0000)
#define STC_AUDIO_STATUS            (0x0004)
#define STC_SYSTOP1_VALUE_HIGH      (0x0008)
#define STC_SYSTOP1_VALUE_LOW       (0x000C)
#define STC_AUDIO1_VALUE_HIGH       (0x0010)
#define STC_AUDIO1_VALUE_LOW        (0x0014)
#define STC_SYSTOP2_VALUE_HIGH      (0x0018)
#define STC_SYSTOP2_VALUE_LOW       (0x001C)
#define STC_AUDIO2_VALUE_HIGH       (0x0020)
#define STC_AUDIO2_VALUE_LOW        (0x0024)

/*******************************************************************************
 * REGISTER MASKS
 ******************************************************************************/
/* STC CONFIG mask*/
#define STC_CFG_CLK_ON_BIT            (0x1 << 0)            /* STC1 27M clk on/off. 0:off, 1:on*/
#define STC_CFG_REST_BIT              (0x1 << 1)            /* Reset counter. 0:reset, 1:not reset, default 1*/
#define STC_CFG_HOLD_1_BIT            (0x1 << 2)            /* Hold STC1 counter. 0:no hold, 1:hold */
#define STC_CFG_HOLD_2_BIT            (0x1 << 3)            /* Hold STC2 counter. 0:no hold, 1:hold  */
#define STC_CFG_SYSTOP_WRITE_STC1_OK_BIT        (0x1 << 4)    /* Write STC1 value OK status bit*/
#define STC_CFG_SYSTOP_WRITE_STC2_OK_BIT        (0x1 << 5)    /* Write STC2 value OK status bit*/
#define STC_CFG_SYSTOP_WRITE_STC1_OK_CLR_BIT    (0x1 << 6)    /* Write STC1 value OK status clr bit */
#define STC_CFG_SYSTOP_WRITE_STC2_OK_CLR_BIT    (0x1 << 7)    /* Write STC2 value OK status clr bit */

#define STC_CFG_AUDIO_WRITE_STC1_OK_BIT         (0x1 << 0)    /* Write STC1 value OK status bit*/
#define STC_CFG_AUDIO_WRITE_STC2_OK_BIT         (0x1 << 1)    /* Write STC2 value OK status bit*/
#define STC_CFG_AUDIO_WRITE_STC1_OK_CLR_BIT     (0x1 << 2)    /* Write STC1 value OK status clr bit */
#define STC_CFG_AUDIO_WRITE_STC2_OK_CLR_BIT     (0x1 << 3)    /* Write STC2 value OK status clr bit */

#define STC_SYSTOP_OK_STATUS_MASK               (0x3 << 4)        /* bit[5-4]*/
#define STC_AUDIO_OK_STATUS_MASK                (0x3 << 0)        /* bit[1-0]*/
#define STC_HIGH_VALUE_MASK                     (0x1FFFF)         /* bit[16-0]*/
#define STC_LOW_VALUE_MASK                      (0xFFFFFFFF)      /* bit[31-0]*/
#define STC_MAX_VALUE_MASK                      (0x1FFFFFFFFFFFF) /* bit[47-0]*/

struct mtk_stc_info {
	int     stc_id;
	int64_t stc_value;
};

#define stc_test_enable     0

#define DMX_STC_NS       2

#define STC_OK      0
#define STC_FAIL    -1

#define MTK_STC_IOW(num, dtype)     _IOW('B', num, dtype)
#define MTK_STC_IOR(num, dtype)     _IOR('B', num, dtype)
#define MTK_STC_IOWR(num, dtype)    _IOWR('B', num, dtype)
#define MTK_STC_IO(num)             _IO('B', num)

#define MTK_STC_IOCTL_ALLOC		MTK_STC_IO(0x0)
#define MTK_STC_IOCTL_FREE	    MTK_STC_IO(0x1)
#define MTK_STC_IOCTL_START	    MTK_STC_IOW(0x2, int)
#define MTK_STC_IOCTL_STOP		MTK_STC_IOW(0x3, int)
#define MTK_STC_IOCTL_SET		MTK_STC_IOW(0x4, struct mtk_stc_info)
#define MTK_STC_IOCTL_GET		MTK_STC_IOR(0x5, struct mtk_stc_info)
#define MTK_STC_IOCTL_ADJUST    MTK_STC_IOW(0x6, struct mtk_stc_info)

