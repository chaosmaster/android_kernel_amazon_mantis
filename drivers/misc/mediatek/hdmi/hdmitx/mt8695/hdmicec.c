/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifdef CONFIG_MTK_INTERNAL_HDMI_SUPPORT

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/byteorder/generic.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/dma-mapping.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <hdmitx.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>

#include "hdmi_ctrl.h"
#include "hdmicec.h"

#define cec_clk_26m 0x1
#define cec_clk_32k 0x2
static unsigned int _CEC_Status;
static unsigned short _CEC_Notneed_Notify = 0xff;
unsigned char cec_clock = cec_clk_26m;
#define SetCECStatus(arg) (_CEC_Status |= (arg))
#define ClrCECStatus(arg) (_CEC_Status &= (~(arg)))
#define IsCECStatus(arg) ((_CEC_Status & (arg)) > 0)

static unsigned short _CEC_ErrStatus;
#define SetCECErrorFlag(arg) (_CEC_ErrStatus |= (arg))
#define ClrCECErrorFlag(arg) (_CEC_ErrStatus &= (~(arg)))
#define IsCECErrorFlag(arg) ((_CEC_ErrStatus & (arg)) > 0)


static CEC_FRAME_DESCRIPTION_IO ActiveRXFrame;
static CEC_FRAME_DESCRIPTION_IO CTSTestFrame;
static CEC_FRAME_DESCRIPTION_IO CEC_rx_msg_queue[RX_Q_SIZE];
static CEC_FRAME_DESCRIPTION_IO CEC_tx_msg_queue[TX_Q_SIZE];
CEC_FRAME_DESCRIPTION_IO *ActiveTXFrame;
CEC_LA_ADDRESS _rCECLaAddr;
CEC_ADDRESS_IO _rCECPhysicAddr;

static unsigned char _u1TxFailCause;
static unsigned char _u1ReTxCnt;
static unsigned char CEC_rxQ_read_idx;
static unsigned char CEC_rxQ_write_idx;
static unsigned char CEC_txQ_read_idx;
static unsigned char CEC_txQ_write_idx;
static unsigned short cec_wakeup_addr = 0xFFFF;
APK_CEC_ACK_INFO cec_send_result;
CEC_FRAME_DESCRIPTION_IO *cec_receive_msg;
static unsigned char cec_msg_report_pending;
struct mtk_cec *global_cec;

#define IS_RX_Q_EMPTY() (CEC_rxQ_read_idx == CEC_rxQ_write_idx)
#define IS_RX_Q_FULL() (((CEC_rxQ_write_idx+1)%RX_Q_SIZE) == CEC_rxQ_read_idx)
#define IS_TX_Q_EMPTY() (CEC_txQ_read_idx == CEC_txQ_write_idx)
#define IS_TX_Q_FULL() (((CEC_txQ_write_idx+1)%TX_Q_SIZE) == CEC_txQ_read_idx)

#define	RegReadFldAlign_CEC(reg16, fld) \
((CEC_REG_READ(reg16)>>(Fld_shft(fld)))&(hdmi_cec_2n(Fld_wid(fld))))
#define	vRegWriteFldAlign_CEC(reg16, val, fld) \
(CEC_REG_WRITE(reg16, (((CEC_REG_READ(reg16))& \
(hdmi_cec_maskvalue(Fld_wid(fld), Fld_shft(fld))))|(val<<(Fld_shft(fld))))))

/* CEC RX_EVENT */
#define IS_INT_DATA_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, DATA_RDY))
#define IS_INT_HEADER_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, HEADER_RDY))
#define IS_INT_MODE_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, MODE_RDY))
#define IS_INT_OV_CEC() (RegReadFldAlign_CEC(RX_EVENT, OV))
#define IS_INT_BR_SB_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, BR_SB_RDY))
#define IS_INT_SB_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, SB_RDY))
#define IS_INT_BR_RDY_CEC() (RegReadFldAlign_CEC(RX_EVENT, BR_RDY))
#define ENABLE_INT_DATA_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_DATA)
#define ENABLE_INT_HEADER_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_HEADER)
#define ENABLE_INT_MODE_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_MODE)
#define ENABLE_INT_OV_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_OV)
#define ENABLE_INT_PULSE_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_PULSE)
#define ENABLE_INT_BR_SB_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_BR_SB)
#define ENABLE_INT_SB_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_SB)
#define ENABLE_INT_BR_RDY_CEC(onoff) vRegWriteFldAlign_CEC(RX_EVENT, onoff, I_EN_BR)
#define CLR_INT_DATA_RDY_CEC() vRegWriteFldAlign_CEC(RX_EVENT, 0, DATA_RDY)
#define CLR_INT_HEADER_RDY_CEC() vRegWriteFldAlign_CEC(RX_EVENT, 0, HEADER_RDY)
#define CLR_INT_MODE_RDY_CEC() vRegWriteFldAlign_CEC(RX_EVENT, 0, MODE_RDY)
#define CLR_INT_OV_CEC() vRegWriteFldAlign_CEC(RX_EVENT, 0, OV)
#define NOTIFY_RX_HW_DATA_TAKEN_CEC() vRegWriteFldAlign_CEC(RX_EVENT, 0, BR_RDY)
#define HW_RX_DATA_ARRIVED_CEC() IS_INT_BR_RDY_CEC()
#define HW_RX_HEADER_ARRIVED_CEC() IS_INT_HEADER_RDY_CEC()

/* TX_EVENT */
#define IS_INT_UN_CEC() (RegReadFldAlign_CEC(TX_EVENT, UN))
#define IS_INT_LOW_CEC() (RegReadFldAlign_CEC(TX_EVENT, LOWB))
#define IS_TX_FINISH_CEC() (RegReadFldAlign_CEC(TX_EVENT, BS))
#define IS_INT_RB_RDY_CEC() (RegReadFldAlign_CEC(TX_EVENT, RB_RDY))
#define ENABLE_INT_UN_CEC(onoff) vRegWriteFldAlign_CEC(TX_EVENT, onoff, I_EN_UN)
#define ENABLE_INT_FAIL_CEC(onoff) vRegWriteFldAlign_CEC(TX_EVENT, onoff, I_EN_FAIL)
#define ENABLE_INT_LOW_CEC(onoff) vRegWriteFldAlign_CEC(TX_EVENT, onoff, I_EN_LOW)
#define ENABLE_INT_BS_CEC(onoff) vRegWriteFldAlign_CEC(TX_EVENT, onoff, I_EN_BS)
#define ENABLE_INT_RB_CEC(onoff) vRegWriteFldAlign_CEC(TX_EVENT, onoff, I_EN_RB)
#define CLR_INT_UN_CEC() vRegWriteFldAlign_CEC(TX_EVENT, 0, UN)
#define CLR_INT_LOW_CEC() vRegWriteFldAlign_CEC(TX_EVENT, 0, LOWB)
#define CLR_TX_FINISH_CEC() vRegWriteFldAlign_CEC(TX_EVENT, 0, BS)
#define TRIGGER_TX_HW_CEC() vRegWriteFldAlign_CEC(TX_EVENT, 1, RB_RDY)
#define IS_TX_DATA_TAKEN_CEC() (!(IS_INT_RB_RDY_CEC()))
#define IS_INT_RB_ENABLE_CEC() (RegReadFldAlign_CEC(TX_EVENT, I_EN_RB))
#define IS_INT_FAIL_ENABLE_CEC() (RegReadFldAlign_CEC(TX_EVENT, I_EN_FAIL))
#define DISABLE_ALL_TX_INT_CEC() \
do { \
	ENABLE_INT_FAIL_CEC(0); \
	ENABLE_INT_RB_CEC(0); \
	ENABLE_INT_LOW_CEC(0); \
	ENABLE_INT_UN_CEC(0); \
	ENABLE_INT_BS_CEC(0); \
} while (0)

/* RX FSM status */
#define IS_RX_FSM_IDLE_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x01)
#define IS_RX_FSM_START_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x02)
#define IS_RX_FSM_MODE_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x04)
#define IS_RX_FSM_MODE1_HEADER_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x08)
#define IS_RX_FSM_MODE1_ARB_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x10)
#define IS_RX_FSM_MODE1_FLAG_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x20)
#define IS_RX_FSM_MODE2_HEADER_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x40)
#define IS_RX_FSM_MODE2_CMD_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x80)
#define IS_RX_FSM_MODE3_ID_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x0100)
#define IS_RX_FSM_MODE3_HEADER_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x0200)
#define IS_RX_FSM_MODE3_DATA_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x0400)
#define IS_RX_FSM_GENERAL_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x0800)
#define IS_RX_FSM_ERROR_S_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x1000)
#define IS_RX_FSM_ERROR_D_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM) == 0x2000)
#define RX_FSM_STATUS_CEC() (RegReadFldAlign_CEC(RX_STATUS, RX_FSM))

/* TX FSM status */
#define IS_TX_FSM_IDLE_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x01)
#define IS_TX_FSM_INIT_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x02)
#define IS_TX_FSM_EOM_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x04)
#define IS_TX_FSM_RETRASMIT_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x08)
#define IS_TX_FSM_FAIL_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x10)
#define IS_TX_FSM_START_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x20)
#define IS_TX_FSM_MODE_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x40)
#define IS_TX_FSM_MODE1_HEADER_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x80)
#define IS_TX_FSM_MODE1_DATA_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x100)
#define IS_TX_FSM_MODE2_HEADER_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x200)
#define IS_TX_FSM_MODE2_CMD_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x400)
#define IS_TX_FSM_MODE3_ID_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x800)
#define IS_TX_FSM_MODE3_HEADER_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x1000)
#define IS_TX_FSM_MODE3_DATA_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x2000)
#define IS_TX_FSM_GENERAL_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM) == 0x4000)
#define TX_FSM_STATUS_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_FSM))

#define ENABLE_TX_EN_CEC() vRegWriteFldAlign_CEC(TR_CONFIG, 1, TX_EN)
#define DISABLE_TX_EN_CEC() vRegWriteFldAlign_CEC(TR_CONFIG, 0, TX_EN)
#define ENABLE_RX_EN_CEC() vRegWriteFldAlign_CEC(TR_CONFIG, 1, RX_EN)
#define DISABLE_RX_EN_CEC() vRegWriteFldAlign_CEC(TR_CONFIG, 0, RX_EN)

#define SET_HW_TX_LEN_CEC(num) vRegWriteFldAlign_CEC(TX_HD_NEXT, num, WTX_M3_DATA_MASK)
#define FILL_SRC_FIELD_CEC(addr) vRegWriteFldAlign_CEC(TX_HD_NEXT, addr, WTX_SRC)
#define FILL_DST_FIELD_CEC(addr) vRegWriteFldAlign_CEC(TX_HD_NEXT, addr, WTX_DST)
#define MARK_H_EOM_CEC(onoff) vRegWriteFldAlign_CEC(TX_HD_NEXT, onoff, WTX_H_EOM)
#define MARK_D_EOM_CEC(onoff) vRegWriteFldAlign_CEC(TX_HD_NEXT, onoff, WTX_D_EOM)

#define FILL_TX_DATA_CEC(data) vRegWriteFldAlign_CEC(TX_DATA_NEXT, data, WTX_DATA)

#define GET_HW_RX_LEN_CEC() (RegReadFldAlign_CEC(RX_HEADER, RXED_M3_DATA_MASK))
#define GET_SRC_FIELD_CEC() (RegReadFldAlign_CEC(RX_HEADER, RXED_SRC))
#define GET_DST_FIELD_CEC() (RegReadFldAlign_CEC(RX_HEADER, RXED_DST))
#define GET_SRC_FIELD_RECEIVING_CEC() (RegReadFldAlign_CEC(RX_HD_NEXT, RXING_SRC))
#define GET_DST_FIELD_RECEIVING_CEC() (RegReadFldAlign_CEC(RX_HD_NEXT, RXING_DST))
#define IS_RX_H_EOM_CEC() (RegReadFldAlign_CEC(RX_HEADER, RXED_H_EOM))
#define IS_RX_D_EOM_CEC() (RegReadFldAlign_CEC(RX_HEADER, RXED_D_EOM))
#define IS_RX_ERR_H_CEC() (RegReadFldAlign_CEC(RX_FAIL, ERR_H) == 0x01)

#define GET_HW_RX_DATA_CEC() (RegReadFldAlign_CEC(RX_DATA, RXED_DATA))

#define FLOW_CONTROL_ACK_CEC(onoff) \
do {\
	vRegWriteFldAlign_CEC(RX_HD_NEXT, (!(onoff)), RXING_H_ACK);\
	vRegWriteFldAlign_CEC(RX_HD_NEXT, (!(onoff)), RXING_D_ACK);\
} while (0)

#define GET_FOLLOWER_H_ACK_CEC() (RegReadFldAlign_CEC(TX_HEADER, TXING_H_ACK))
#define GET_FOLLOWER_D_ACK_CEC() (RegReadFldAlign_CEC(TX_HEADER, TXING_D_ACK))

#define TX_FAIL_MAX_CEC() (RegReadFldAlign_CEC(TX_FAIL, RETX_MAX))
#define CLR_TX_FAIL_MAX_CEC()  vRegWriteFldAlign_CEC(TX_FAIL, 0, RETX_MAX)

#define TX_FAIL_RECORD_CEC() CEC_REG_READ(TX_FAIL)
#define TX_FAIL_SOURCE_CEC() (RegReadFldAlign_CEC(TX_FAIL, SOURCE))
#define CLR_TX_FAIL_SOURCE_CEC()  vRegWriteFldAlign_CEC(TX_FAIL, 0, SOURCE)
#define CHECK_RX_EN_CEC() (RegReadFldAlign_CEC(TR_CONFIG, RX_EN))

#define SET_LA1_CEC(La1) vRegWriteFldAlign_CEC(TR_CONFIG, La1, DEVICE_ADDR)
#define SET_LA2_CEC(La2) vRegWriteFldAlign_CEC(TR_CONFIG, La2, TR_DEVICE_ADDR2)
#define SET_LA3_CEC(La3) vRegWriteFldAlign_CEC(TR_CONFIG, La3, TR_DEVICE_ADDR3)
#define GET_TX_BIT_COUNTER_CEC() (RegReadFldAlign_CEC(TX_STATUS, TX_BIT_COUNTER))

#define HW_TX_BUF_SIZE 4
#define TX_MESSAGE_BUF_SIZE 20

#define RESET_HW_TX_CEC() \
do { \
	DISABLE_TX_EN_CEC();\
	ENABLE_TX_EN_CEC();\
} while (0)

unsigned int hdmi_cec_read(unsigned short u2Reg)
{
	unsigned int u4Data;
	struct mtk_cec *cec = global_cec;

	u4Data = cec->cec_read(cec, u2Reg);
	HDMI_CEC_REG_LOG("[R]cec = 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	return u4Data;
}

void hdmi_cec_write(unsigned short u2Reg, unsigned int u4Data)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_REG_LOG("[W]cec= 0x%04x, data = 0x%08x\n", u2Reg, u4Data);
	cec->cec_write(cec, u2Reg, u4Data);
}

unsigned int hdmi_cec_2n(unsigned int u4Data)
{
	unsigned int u4resultvalue = 1;
	unsigned char u1number;

	if (u4Data == 0)
		return 0;	/* must be not 0 */
	if (u4Data == 0x20)
		return 0xffffffff;
	if (u4Data > 0x20)
		return 0;	/* must  not exceed over 0x20 */

	for (u1number = 0; u1number < u4Data; u1number++)
		u4resultvalue *= 2;
	/* HDMI_CEC_REG_LOG("hdmi_cec_2n data = 0x%08x\n", u4resultvalue-1); */

	return (u4resultvalue - 1);
}

u32 hdmi_cec_maskvalue(unsigned int u4Width, unsigned int u4Startbit)
{
	unsigned int u4Data = 0xffffffff, i;

	for (i = 0; i < u4Width; i++)
		u4Data &= (~(hdmi_cec_2n(u4Startbit + i) + 1));
	/* HDMI_CEC_LOG("hdmi_cec_maskvalue data = 0x%08x\n", u4Data); */

	return u4Data;
}

/******* New CEC IP HW ********/
inline bool mtkcec_hwrx_header_arrived(struct mtk_cec *cec)
{
	bool is_rx_fsm_irq;
	bool is_rx_fsm_header;

	is_rx_fsm_irq = cec->cec_readbit(cec, CEC2_INT_STA, CEC2_RX_FSM_CHG_INT_STA);
	is_rx_fsm_header = ((cec->cec_read(cec, CEC2_RX_STATUS) & CEC2_RX_FSM) >> 16) == 0x08;
	if (is_rx_fsm_irq && is_rx_fsm_header)
		return true;
	else
		return false;
}

inline bool mtkcec_hw_input(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_RX_STATUS, CEC2_CEC_INPUT);
}

inline unsigned int mtkcec_hwrx_get_dst(struct mtk_cec *cec)
{
	return cec->cec_read(cec, CEC2_RX_BUF_HEADER) & CEC2_RX_HEADER_DST;
}

inline unsigned int mtkcec_hwrx_get_src(struct mtk_cec *cec)
{
	return (cec->cec_read(cec, CEC2_RX_BUF_HEADER) & CEC2_RX_HEADER_SRC) >> 4;
}

inline unsigned int mtkcec_hwrx_get_data_4bytes(struct mtk_cec *cec, unsigned short reg)
{
	return cec->cec_read(cec, reg);
}

inline bool mtkcec_hwrx_data_arrived(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_RX_DATA_RCVD_INT_STA);
}

inline bool mtkcec_hwrx_buffer_ready(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_RX_BUF_READY_INT_STA);
}
inline bool mtkcec_is_int_br_rdy(struct mtk_cec *cec)
{
	return mtkcec_hwrx_buffer_ready(cec);
}

inline unsigned int mtkcec_hwrx_len(struct mtk_cec *cec)
{
	unsigned int val;

	val = cec->cec_read(cec, CEC2_RX_BUF_HEADER);
	if (val & CEC2_RX_HEADER_EOM)
		return 0;
	if (val & CEC2_RX_BUF_CNT)
		return ((val & CEC2_RX_BUF_CNT) >> 12);
	else
		return 0;
}

inline void mtkcec_hwrx_notify_data_taken(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_RX_BUF_HEADER, CEC2_RX_BUF_RISC_ACK, CEC2_RX_BUF_RISC_ACK);
}

inline bool mtkcec_hwrx_int_isen_(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_EN, CEC2_RX_BUF_FULL_INT_EN);
}

inline bool mtkcec_hwrx_buffer_full(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_RX_BUF_FULL_INT_STA);
}

inline void mtkcec_hwrx_buffer_full_clear(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_RX_BUF_FULL_INT_CLR, CEC2_RX_BUF_FULL_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_RX_BUF_FULL_INT_CLR);
}

inline void mtkcec_hwrx_reset(struct mtk_cec *cec)
{
	unsigned int val;

	val = cec->cec_read(cec, CEC2_TR_CONFIG);
	cec->cec_write(cec, CEC2_TR_CONFIG, val & (~CEC2_RX_RESET_WRITE));
	udelay(1);
	val = cec->cec_read(cec, CEC2_TR_CONFIG);
	cec->cec_write(cec, CEC2_TR_CONFIG, val | (CEC2_TX_RESET_WRITE | CEC2_RX_RESET_WRITE));
}

inline bool mtkcec_hwtx_fail(struct mtk_cec *cec)
{
	return (cec->cec_read(cec, CEC2_INT_STA) & CEC2_TX_INT_FAIL) ? true : false;
}

inline bool mtkcec_hwtx_fail_fsm(struct mtk_cec *cec)
{
	return ((cec->cec_read(cec, CEC2_TX_STATUS) & CEC2_TX_FSM) >> 16) == 0x10 ? true : false;
}

inline void mtkcec_hwtx_int_fail_clear(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_INT_FAIL_CLR, CEC2_TX_INT_FAIL_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_INT_FAIL_CLR);
}

inline void mtkcec_hwtx_int_alldisable(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_EN, 0x0, CEC2_TX_INT_ALL_EN);
}

inline void mtkcec_hwtx_int_all_clear(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_INT_ALL_CLR, CEC2_TX_INT_ALL_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_INT_ALL_CLR);
}

inline void mtkcec_hw_int_alldisable(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_EN, 0x0, CEC2_INT_ALL_EN);
}

inline void mtkcec_hwtx_reset(struct mtk_cec *cec)
{
	unsigned int val;

	val = cec->cec_read(cec, CEC2_TR_CONFIG);
	cec->cec_write(cec, CEC2_TR_CONFIG, val & (~CEC2_TX_RESET_WRITE));
	udelay(1);
	val = cec->cec_read(cec, CEC2_TR_CONFIG);
	cec->cec_write(cec, CEC2_TR_CONFIG, val | (CEC2_TX_RESET_WRITE | CEC2_RX_RESET_WRITE));
}

inline void mtkcec_hwtx_detec_startbit(struct mtk_cec *cec)
{
	static unsigned int tx_fsm, pre_tx_fsm;

	tx_fsm = ((cec->cec_read(cec, CEC2_TX_STATUS)) & CEC2_TX_FSM) >> 16;
	if ((tx_fsm == 0x08) && (pre_tx_fsm == 0x20)) {
		mtkcec_hwtx_reset(cec);
		HDMI_CEC_LOG("\nDetect Start bit Err\n\n");
	}
	pre_tx_fsm = tx_fsm;
}

inline bool mtkcec_hwtx_fail_h_ack(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_HEADER_ACK_INT_STA);
}

inline void mtkcec_hwtx_fail_h_ack_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_HEADER_ACK_INT_CLR, CEC2_TX_FAIL_HEADER_ACK_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_HEADER_ACK_INT_CLR);
}

inline bool mtkcec_hwtx_fail_data_ack(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_DATA_ACK_INT_STA);
}

inline void mtkcec_hwtx_fail_data_ack_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_DATA_ACK_INT_CLR, CEC2_TX_FAIL_DATA_ACK_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_DATA_ACK_INT_CLR);
}

inline bool mtkcec_hwtx_fail_data(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_DATA_INT_STA);
}

inline void mtkcec_hwtx_fail_data_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_DATA_INT_CLR, CEC2_TX_FAIL_DATA_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_DATA_INT_CLR);
}

inline bool mtkcec_hwtx_fail_header(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_HEADER_INT_STA);
}

inline void mtkcec_hwtx_fail_header_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_HEADER_INT_CLR, CEC2_TX_FAIL_HEADER_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_HEADER_INT_CLR);
}

inline bool mtkcec_hwtx_fail_src(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_SRC_INT_STA);
}

inline void mtkcec_hwtx_fail_src_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_SRC_INT_CLR, CEC2_TX_FAIL_SRC_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_SRC_INT_CLR);
}

inline bool mtkcec_hwtx_fail_longlow(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_LINE_lOW_LONG_INT_STA);
}

inline void mtkcec_hwtx_fail_longlow_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_LINE_lOW_LONG_INT_CLR, CEC2_LINE_lOW_LONG_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_LINE_lOW_LONG_INT_CLR);
}

inline bool mtkcec_hwtx_fsm_idle(struct mtk_cec *cec)
{
	return ((cec->cec_read(cec, CEC2_TX_STATUS) & CEC2_TX_FSM) >> 16) == 0x01 ? true : false;
}

inline bool mtkcec_hwtx_finish(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_DATA_FINISH_INT_STA);
}

inline void mtkcec_hwtx_finish_clr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_DATA_FINISH_INT_CLR, CEC2_TX_DATA_FINISH_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_DATA_FINISH_INT_CLR);
}

inline void mtkcec_hwtx_setsrc(struct mtk_cec *cec, unsigned char src)
{
	cec->cec_mask(cec, CEC2_TX_HEADER, src << 4, CEC2_TX_HEADER_SRC);
}

inline void mtkcec_hwtx_setdst(struct mtk_cec *cec, unsigned char dst)
{
	cec->cec_mask(cec, CEC2_TX_HEADER, dst, CEC2_TX_HEADER_DST);
}

static void mtkcec_hwtx_setdatalen(struct mtk_cec *cec, unsigned int len)
{
	cec->cec_mask(cec, CEC2_TX_HEADER, len << 12, CEC2_TX_SEND_CNT);
}

static void mtkcec_hwtx_setdata(struct mtk_cec *cec, CEC_FRAME_BLOCK_IO *blocks, unsigned char data_len)
{
	unsigned char i, addr, byte_shift;
	unsigned char msg[16] = {0};

	msg[0] = blocks->opcode;
	memcpy(&msg[1], &(blocks->operand), data_len - 1);
	for (i = 0; i < data_len; i++) {
		addr = (i / 4) * 4 + CEC2_TX_DATA0;
		byte_shift = (i % 4 * 8);
		cec->cec_mask(cec, addr, msg[i] << byte_shift, 0xff << byte_shift);
	}
}

static void mtkcec_hwtx_set_h_eom(struct mtk_cec *cec, unsigned int heom)
{
	cec->cec_mask(cec, CEC2_TX_HEADER, heom << 8, CEC2_TX_HEADER_EOM);
}

inline void mtkcec_hwtx_fail_header_int_en(struct mtk_cec *cec, unsigned int en)
{
	cec->cec_mask(cec, CEC2_INT_EN, en << 10, CEC2_TX_FAIL_HEADER_INT_EN);
}

inline void mtkcec_hwtx_fail_header_ack_int_en(struct mtk_cec *cec, unsigned int en)
{
	cec->cec_mask(cec, CEC2_INT_EN, en << 13, CEC2_TX_FAIL_HEADER_ACK_INT_EN);
}

inline void mtkcec_hwtx_fail_retransmit_int_en(struct mtk_cec *cec, unsigned int en)
{
	cec->cec_mask(cec, CEC2_INT_EN, en << 12, CEC2_TX_FAIL_RETRANSMIT_INT_EN);
}

inline void mtkcec_hwtx_finish_int_en(struct mtk_cec *cec, unsigned int en)
{
	cec->cec_mask(cec, CEC2_INT_EN, en << 8, CEC2_TX_DATA_FINISH_INT_EN);
}

inline void mtkcec_hwtx_fail_longlow_int_en(struct mtk_cec *cec, unsigned int en)
{
	cec->cec_mask(cec, CEC2_INT_EN, en << 7, CEC2_LINE_lOW_LONG_INT_EN);
}

inline void mtkcec_hwtx_trigger(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_TX_HEADER, CEC2_TX_READY, CEC2_TX_READY);
}

inline bool mtkcec_hwtx_fsmsta_running(struct mtk_cec *cec)
{
	unsigned int val;

	val = cec->cec_read(cec, CEC2_TX_STATUS) & CEC2_TX_FSM;
	if (val == 0x01 || val == 0x04 || val == 0x10)
		return false;
	else
		return true;
}

inline bool mtkcec_hwtx_fail_isen(struct mtk_cec *cec)
{
	return (cec->cec_read(cec, CEC2_INT_EN) & CEC2_TX_INT_FAIL_EN) ? true : false;
}

inline bool mtkcec_hwtx_fail_retransmit_intsta(struct mtk_cec *cec)
{
	return cec->cec_readbit(cec, CEC2_INT_STA, CEC2_TX_FAIL_RETRANSMIT_INT_STA);
}

inline void mtkcec_hwtx_fail_retransmit_intclr(struct mtk_cec *cec)
{
	cec->cec_mask(cec, CEC2_INT_CLR, CEC2_TX_FAIL_RETRANSMIT_INT_CLR, CEC2_TX_FAIL_RETRANSMIT_INT_CLR);
	cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_TX_FAIL_RETRANSMIT_INT_CLR);
}

inline bool mtkcec_hwtx_fail_retransmit_times(struct mtk_cec *cec)
{
	return (cec->cec_read(cec, CEC2_TX_STATUS) & CEC2_TX_NUM_RETRANSMIT) >> 26;
}

inline unsigned int mtkcec_hwtx_fail_backsta(struct mtk_cec *cec)
{
	return (cec->cec_read(cec, CEC2_BACK) & CEC2_TX_LAST_ERR_STA) >> 16;
}

inline void mtkcec_hw_set_la1(struct mtk_cec *cec, unsigned char la)
{
	cec->cec_mask(cec, CEC2_TR_CONFIG, la << CEC2_LA1_SHIFT, CEC2_DEVICE_ADDR1);
}

inline void mtkcec_hw_set_la2(struct mtk_cec *cec, unsigned char la)
{
	cec->cec_mask(cec, CEC2_TR_CONFIG, la << CEC2_LA2_SHIFT, CEC2_DEVICE_ADDR2);
}

inline void mtkcec_hw_set_la3(struct mtk_cec *cec, unsigned char la)
{
	cec->cec_mask(cec, CEC2_TR_CONFIG, la << CEC2_LA3_SHIFT, CEC2_DEVICE_ADDR3);
}

static void mtkcec_hwclk_div(struct mtk_cec *cec, unsigned int clk_src)
{
	if (clk_src == cec_clk_26m) {
		/* divide the clock of 26Mhz to 100khz */
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_27M_EN, CEC2_CLK_27M_EN);
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_DIV_SEL_100K, CEC2_CLK_DIV);
		cec->cec_mask(cec, CEC2_CKGEN, 0x0, CEC2_CLK_SEL_DIV);
	} else {
		/* do not divide the clock of 32khz */
		cec->cec_mask(cec, CEC2_CKGEN, 0x0, CEC2_CLK_27M_EN);
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_32K_EN, CEC2_CLK_32K_EN);
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_SEL_DIV, CEC2_CLK_SEL_DIV);
		return;
	}
}

static void mtkcec_hwrx_check_addr(struct mtk_cec *cec, bool enable)
{
	if (enable) {
		cec->cec_mask(cec, CEC2_TR_CONFIG, CEC2_RX_CHK_DST, CEC2_RX_CHK_DST);
		cec->cec_mask(cec, CEC2_TR_CONFIG, 0, CEC2_BYPASS);
	} else {
		cec->cec_mask(cec, CEC2_TR_CONFIG, 0, CEC2_RX_CHK_DST);
		cec->cec_mask(cec, CEC2_TR_CONFIG, CEC2_BYPASS, CEC2_BYPASS);
	}
}

static void mtkcec_hwrx_timing_config(struct mtk_cec *cec, unsigned int clk_src)
{
	if (clk_src == cec_clk_26m) {
		/* use default timing value */
		cec->cec_write(cec, CEC2_RX_TIMER_START_R, 0x0186015e);
		cec->cec_write(cec, CEC2_RX_TIMER_START_F, 0x01d601ae);
		cec->cec_write(cec, CEC2_RX_TIMER_DATA, 0x011300cd);
		cec->cec_write(cec, CEC2_RX_TIMER_ACK, 0x00690082);
		cec->cec_write(cec, CEC2_RX_TIMER_ERROR, 0x01680000);
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_27M_EN, CEC2_CLK_27M_EN);
	} else {
		cec->cec_write(cec, CEC2_RX_TIMER_START_R, 0x007d0070);
		cec->cec_write(cec, CEC2_RX_TIMER_START_F, 0x0099008a);
		cec->cec_write(cec, CEC2_RX_TIMER_DATA, 0x00230040);
		cec->cec_write(cec, CEC2_RX_TIMER_ACK, 0x00000030);
		cec->cec_write(cec, CEC2_RX_TIMER_ERROR, 0x007300aa);
		return;
	}

	cec->cec_mask(cec, CEC2_TR_CONFIG, CEC2_RX_ACK_ERROR_BYPASS, CEC2_RX_ACK_ERROR_BYPASS);
	mtkcec_hwrx_reset(cec);
}

static void mtkcec_hwtx_timing_config(struct mtk_cec *cec, unsigned int clk_src)
{
	if (clk_src == cec_clk_26m) {
		/* use default timing value */
		cec->cec_write(cec, CEC2_TX_TIMER_START, 0x01c20172);
		cec->cec_write(cec, CEC2_TX_TIMER_DATA_R, 0x003c0096);
		cec->cec_write(cec, CEC2_TX_T_DATA_F, 0x00f000f0);
		cec->cec_write(cec, TX_TIMER_DATA_S, 0x00040069);
		cec->cec_write(cec, CEC2_LINE_DET, 0x1f4004b0);

		cec->cec_mask(cec, CEC2_TX_TIMER_ACK, 0x00aa0028, CEC2_TX_TIMER_ACK_MAX | CEC2_TX_TIMER_ACK_MIN);
		cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_27M_EN, CEC2_CLK_27M_EN);
	} else {
		cec->cec_write(cec, CEC2_TX_TIMER_START, 0x00920079);
		cec->cec_write(cec, CEC2_TX_TIMER_DATA_R, 0x00140031);
		cec->cec_write(cec, CEC2_TX_T_DATA_F, 0x004f004f);
		cec->cec_write(cec, TX_TIMER_DATA_S, 0x00010022);
		cec->cec_write(cec, CEC2_LINE_DET, 0x0a000180);

		cec->cec_mask(cec, CEC2_TX_TIMER_ACK, 0x0036000d, CEC2_TX_TIMER_ACK_MAX | CEC2_TX_TIMER_ACK_MIN);
		return;
	}
	cec->cec_mask(cec, CEC2_TX_ARB, 0x01 << 24, CEC2_TX_MAX_RETRANSMIT_NUM_ARB);
	cec->cec_mask(cec, CEC2_TX_ARB, 0x01 << 20, CEC2_TX_MAX_RETRANSMIT_NUM_COL);
	cec->cec_mask(cec, CEC2_TX_ARB, 0x01 << 16, CEC2_TX_MAX_RETRANSMIT_NUM_NAK);
	cec->cec_mask(cec, CEC2_TX_ARB, 0x03 << 8, CEC2_TX_BCNT_RETRANSMIT);
	cec->cec_mask(cec, CEC2_TX_ARB, 0x07 << 4, CEC2_TX_BCNT_NEW_MSG);
	cec->cec_mask(cec, CEC2_TX_ARB, 0x05, CEC2_TX_BCNT_NEW_INIT);

	cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_TX_EN, CEC2_CLK_TX_EN);
	mtkcec_hwtx_reset(cec);
}

inline void mtkcec_hwtxrx_interrupts_config(struct mtk_cec *cec)
{
	/*disable all irq config*/
	cec->cec_write(cec, CEC2_INT_CLR, CEC2_INT_CLR_ALL);
	cec->cec_write(cec, CEC2_INT_CLR, 0);

	/*enable all rx irq config*/
	cec->cec_mask(cec, CEC2_INT_EN, CEC2_RX_INT_ALL_EN, CEC2_RX_INT_ALL_EN);

	/*enable some of tx irq config*/
	cec->cec_mask(cec, CEC2_INT_EN, CEC2_TX_FAIL_RETRANSMIT_INT_EN, CEC2_TX_FAIL_RETRANSMIT_INT_EN);
	cec->cec_mask(cec, CEC2_INT_EN, CEC2_TX_DATA_FINISH_INT_EN, CEC2_TX_DATA_FINISH_INT_EN);
}

inline void mtkcec_hw_switch(struct mtk_cec *cec, CEC_HW_IP ip)
{
	if (ip == CECHW_IP_OLD) {
		cec->hwip_switch = CECHW_IP_OLD;
		vRegWriteFldAlign_CEC(TR_TEST, 0, TR_HWIP_SWITCH);
		cec->common_mask(cec, cec->cec2_base + CEC2_CKGEN, CEC2_CLK_PDN, CEC2_CLK_PDN);
	} else {
		cec->hwip_switch = CECHW_IP_NEW;
		vRegWriteFldAlign_CEC(TR_TEST, 1, TR_HWIP_SWITCH);
		cec->common_mask(cec, cec->cec1_base + CEC_CKGEN, PDN, PDN);
	}
}
/***** New CEC IP HW END*******/

void mtkcec_irq2gic_clear(struct mtk_cec *cec)
{
	if (cec->hwip_switch == CECHW_IP_OLD) {
		/* 27mhz, clear HPD/PORT and cec int */
		cec->cec_mask(cec, TR_CONFIG, 1, CLEAR_CEC_IRQ);
		udelay(2);
		cec->cec_mask(cec, TR_CONFIG, 0, CLEAR_CEC_IRQ);

		/* 32khz, clear HPD/PORT and CEC RX int */
		cec->cec_mask(cec, RX_GEN_WD, 1, HDMI_PORD_INT_CLR);
		cec->cec_mask(cec, RX_GEN_WD, 1, RX_INT_CLR);
		cec->cec_mask(cec, RX_GEN_WD, 1, HDMI_HTPLG_INT_CLR);
	} else {
		/* irq of TX has clearn */
		cec->cec_mask(cec, CEC2_INT_CLR, CEC2_RX_INT_ALL_CLR, CEC2_RX_INT_ALL_CLR);
		cec->cec_mask(cec, CEC2_INT_CLR, 0, CEC2_RX_INT_ALL_CLR);
	}
}

void hdmi_cec_init(struct mtk_cec *cec)
{
	HDMI_CEC_FUNC();
	cec_msg_report_pending = 0;

	/*_rCECPhysicAddr.ui2_pa = 0xffff;*/
	_rCECPhysicAddr.ui1_la = 0x0;

	_CEC_Status = 0;
	_CEC_ErrStatus = 0;
	_u1TxFailCause = FAIL_NONE;
	_u1ReTxCnt = 0;
	CEC_rxQ_write_idx = 0;
	CEC_rxQ_read_idx = 0;
	CEC_txQ_write_idx = 0;
	CEC_txQ_read_idx = 0;

	_rCECLaAddr.aui1_la[0] = 0x0F;
	_rCECLaAddr.aui1_la[1] = 0x0F;
	_rCECLaAddr.aui1_la[2] = 0x0F;
	if (cec->hwip_switch == CECHW_IP_OLD) {

		/*3MHz, different from BD 100k*/
		CEC_REG_WRITE(CEC_CKGEN, 0x000a0082);
		/*Bpad enable Tx compared timing 0x19*/
		CEC_REG_WRITE(TR_TEST, 0x40004019);

		/* CYJ.NOTE TX_EN, RX_EN: disable it */
		CEC_REG_WRITE(TR_CONFIG, 0x00000001);

		CEC_REG_WRITE(RX_T_START_R, 0x01980154);
		CEC_REG_WRITE(RX_T_START_F, 0x01e801a9);
		/* C8->C7,for CTS8.2.4 */
		CEC_REG_WRITE(RX_T_DATA, 0x006e00c8);
		CEC_REG_WRITE(RX_T_ACK, 0x00000096);
		CEC_REG_WRITE(RX_T_ERROR, 0x01680212);
		CEC_REG_WRITE(TX_T_START, 0x01c20172);
		CEC_REG_WRITE(TX_T_DATA_R, 0x003c0096);
		CEC_REG_WRITE(TX_T_DATA_F, 0x00f000f0);
		CEC_REG_WRITE(TX_ARB, 0x00000596);

		/* turn off interrupt of general mode */
		CEC_REG_WRITE(TX_GEN_INTR, 0x00000000);
		CEC_REG_WRITE(RX_CAP_90, 0x00000000);
		CEC_REG_WRITE(TX_GEN_MASK, 0x00000000);
		CEC_REG_WRITE(RX_GEN_WD, 0x00000000);
		CEC_REG_WRITE(RX_GEN_MASK, 0x00000000);
		CEC_REG_WRITE(RX_GEN_INTR, 0x00000000);

		FLOW_CONTROL_ACK_CEC(1);

		vRegWriteFldAlign_CEC(TX_HD_NEXT, 0, WTX_M3_ID);
		vRegWriteFldAlign_CEC(TX_HD_NEXT, 0, WTX_M1_DIR);
		vRegWriteFldAlign_CEC(TX_HD_NEXT, 0, WTX_M1_PAS);
		vRegWriteFldAlign_CEC(TX_HD_NEXT, 0, WTX_M1_NAS);
		vRegWriteFldAlign_CEC(TX_HD_NEXT, 0, WTX_M1_DES);
		vRegWriteFldAlign_CEC(TX_HD_NEXT, 3, WTX_MODE);

		CEC_REG_WRITE(TR_CONFIG, 0x8fff1101);

		CEC_REG_WRITE_MASK(TX_EVENT, 0x00, 0xff);
		CEC_REG_WRITE_MASK(RX_EVENT, 0x00, 0xff);
		/* RX_EVENT */
		ENABLE_INT_OV_CEC(1);
		ENABLE_INT_BR_RDY_CEC(1);
		ENABLE_INT_HEADER_RDY_CEC(1);
		/* TX_EVENT */
		ENABLE_INT_UN_CEC(0);
		ENABLE_INT_LOW_CEC(0);
		ENABLE_INT_FAIL_CEC(0);
		ENABLE_INT_BS_CEC(0);
		ENABLE_INT_RB_CEC(0);
		/* la */
		SET_LA1_CEC(0x0F);
		SET_LA2_CEC(0x0F);
		SET_LA3_CEC(0x0F);
	} else {
		cec->common_mask(cec, cec->cec1_base + TR_TEST, 0xc0004019, 0xffffffff);
		mtkcec_hwclk_div(cec, cec_clock);
		mtkcec_hwrx_check_addr(cec, true);
		mtkcec_hwrx_timing_config(cec, cec_clock);
		mtkcec_hwtx_timing_config(cec, cec_clock);
		mtkcec_hwtxrx_interrupts_config(cec);
		mtkcec_hw_set_la3(cec, 0x0F);
		mtkcec_hw_set_la2(cec, 0x0F);
		mtkcec_hw_set_la1(cec, 0x0F);
	}
}

void hdmi_cec_deinit(struct mtk_cec *cec)
{
	if (cec->hwip_switch == CECHW_IP_OLD) {
		ENABLE_INT_OV_CEC(0);
		ENABLE_INT_BR_RDY_CEC(0);
		ENABLE_INT_HEADER_RDY_CEC(0);
		DISABLE_RX_EN_CEC();
		DISABLE_TX_EN_CEC();
	} else {
		mtkcec_hw_int_alldisable(cec);
	}
}

void mtkcec_clk_enable(struct mtk_cec *cec, unsigned char en)
{
	static bool is_off = true;

	if (en && is_off) {
		clk_prepare_enable(cec->clock);
		is_off = false;
	} else if ((en == false) && (is_off == false)) {
		clk_disable_unprepare(cec->clock);
		is_off = true;
	} else
		HDMI_CEC_LOG("CEC clk warning, en=%d,state=%d\n", en, is_off);
}

void mtkcec_clk_probe(struct mtk_cec *cec)
{
	cec->clock = devm_clk_get(&cec->pdev->dev, "hdmi_cec");
	WARN_ON(IS_ERR(cec->clock));
	mtkcec_clk_enable(cec, true);
	wake_lock_init(&cec->wakelock, WAKE_LOCK_SUSPEND, "cec_wakelock");
}

void mtkcec_wake_lock(struct mtk_cec *cec)
{
	if (wake_lock_active(&cec->wakelock))
		wake_unlock(&cec->wakelock);
	wake_lock_timeout(&cec->wakelock, msecs_to_jiffies(500));
}

void mtkcec_wake_unlock(struct mtk_cec *cec)
{
	if (wake_lock_active(&cec->wakelock))
		wake_unlock(&cec->wakelock);
}

void hdmi_cec_power_on(struct mtk_cec *cec, bool pwr)
{
	HDMI_CEC_FUNC();

	if (cec->hwip_switch == CECHW_IP_OLD)
		if (pwr == false) {
			vRegWriteFldAlign_CEC(CEC_CKGEN, 1, PDN);
			mtkcec_clk_enable(cec, false);
		} else {
			mtkcec_clk_enable(cec, true);
			vRegWriteFldAlign_CEC(CEC_CKGEN, 0, PDN);
		}
	else
		if (pwr == false) {
			cec->cec_mask(cec, CEC2_CKGEN, CEC2_CLK_PDN, CEC2_CLK_PDN);
			mtkcec_clk_enable(cec, false);
		} else {
			mtkcec_clk_enable(cec, true);
			cec->cec_mask(cec, CEC2_CKGEN, 0, CEC2_CLK_PDN);
		}
}

static unsigned char CEC_rx_enqueue(CEC_FRAME_DESCRIPTION_IO *frame)
{
	/* HDMI_CEC_FUNC(); */
	if (IS_RX_Q_FULL())
		return FALSE;

	memcpy(&(CEC_rx_msg_queue[CEC_rxQ_write_idx]), frame, sizeof(CEC_FRAME_DESCRIPTION_IO));
	/* CYJ.NOTE: no critical section */
	CEC_rxQ_write_idx = (CEC_rxQ_write_idx + 1) % RX_Q_SIZE;
	HDMI_CEC_COMMAND_LOG(" rxing opcode-->start 0x%x\n", frame->blocks.opcode);
	memset(frame, 0, sizeof(CEC_FRAME_DESCRIPTION_IO));
	return TRUE;
}

static void _CEC_Receiving_Old(void)
{
	static unsigned char *size;
	static CEC_FRAME_DESCRIPTION_IO *frame = &ActiveRXFrame;
	unsigned int data;
	unsigned char i, rxlen, is_d_eom, ret;

	/* no data available */
	if (!IS_INT_BR_RDY_CEC())
		return;

	/* <polling message> only */
	if (GET_HW_RX_LEN_CEC() == 0) {
		NOTIFY_RX_HW_DATA_TAKEN_CEC();
		ClrCECStatus(STATE_RX_GET_NEW_HEADER);	/* CM 20081210 */
		return;
	}

	/* new incoming message */
	if (IsCECStatus(STATE_RX_GET_NEW_HEADER)) {
		ClrCECStatus(STATE_RX_GET_NEW_HEADER);
		if (IsCECStatus(STATE_WAIT_RX_FRAME_COMPLETE)) {
			HDMI_CEC_LOG("Lost EOM:2\n");
			SetCECErrorFlag(ERR_RX_LOST_EOM);
		}
		SetCECStatus(STATE_WAIT_RX_FRAME_COMPLETE);

		size = &(frame->size);
		(*size) = 0;
		frame->blocks.header.initiator = GET_SRC_FIELD_CEC();
		frame->blocks.header.destination = GET_DST_FIELD_CEC();
		(*size)++;
	}

	if (!IsCECStatus(STATE_WAIT_RX_FRAME_COMPLETE)) {
		NOTIFY_RX_HW_DATA_TAKEN_CEC();
		SetCECErrorFlag(ERR_RX_LOST_HEADER);
		return;
	}

	rxlen = GET_HW_RX_LEN_CEC();
	data = GET_HW_RX_DATA_CEC();
	is_d_eom = IS_RX_D_EOM_CEC();
	NOTIFY_RX_HW_DATA_TAKEN_CEC();

	if (rxlen == 0x3) {
		rxlen = 2;
	} else if (rxlen == 0x7) {
		rxlen = 3;
	} else if (rxlen == 0xf) {
		rxlen = 4;
	} else if (rxlen != 0x1) {
		HDMI_CEC_LOG("invalid rx length occurs\n");
		/* assert(0); */
	}
	/* for opcode */
	if ((*size) == 1) {
		frame->blocks.opcode = data & 0xff;
		data >>= 8;
		(*size)++;
		rxlen--;
	}
	/* for operand */
	for (i = 0; i < rxlen; i++) {

		if ((*size) > 15) {
			HDMI_CEC_LOG("Receive Data Length is wrong !\n");
			break;
		}
		{
			frame->blocks.operand[(*size) - 2] = data & 0xff;
			data >>= 8;
			(*size)++;
		}
	}

	if (is_d_eom) {
		ClrCECStatus(STATE_WAIT_RX_FRAME_COMPLETE);
		SetCECStatus(STATE_RX_COMPLETE_NEW_FRAME);

		/* push into rx_queue */
		ret = CEC_rx_enqueue(frame);
		if (ret == FALSE) {
			SetCECErrorFlag(ERR_RXQ_OVERFLOW);
			HDMI_CEC_LOG("cec rx buffer overflow\n");
		}
	}
}

static void _CEC_Receiving(struct mtk_cec *cec)
{
	static CEC_FRAME_DESCRIPTION_IO *frame = &ActiveRXFrame;
	unsigned int data[4];
	unsigned char rxlen, i, shift, oper_index, data_index;

	/* no data available */
	if (!mtkcec_is_int_br_rdy(cec))
		return;

	/* <polling message> only */
	if (mtkcec_hwrx_len(cec) == 0) {
		mtkcec_hwrx_notify_data_taken(cec);
		ClrCECStatus(STATE_RX_GET_NEW_HEADER);	/* CM 20081210 */
		return;
	}

	frame->blocks.header.initiator = mtkcec_hwrx_get_src(cec);
	frame->blocks.header.destination = mtkcec_hwrx_get_dst(cec);

	rxlen = mtkcec_hwrx_len(cec);
	frame->size = 1 + rxlen;

	for (i = 0; i < rxlen; i++) {
		data_index = i / 4;
		shift = (i % 4) << 3;
		if (shift == 0)
			data[data_index] = mtkcec_hwrx_get_data_4bytes(cec, CEC2_RX_DATA0 + 4 * data_index);
		oper_index = i > 1 ? (i - 1) : 0;
		frame->blocks.operand[oper_index] = (data[data_index] >> shift) & 0xff;
	}
	frame->blocks.opcode = data[0] & 0xff;
	mtkcec_hwrx_notify_data_taken(cec);

	ClrCECStatus(STATE_WAIT_RX_FRAME_COMPLETE);
	SetCECStatus(STATE_RX_COMPLETE_NEW_FRAME);

	/* push into rx_queue */
	if (true != CEC_rx_enqueue(frame)) {
		SetCECErrorFlag(ERR_RXQ_OVERFLOW);
		HDMI_CEC_LOG("cec rx buffer overflow\n");
	}
}

static unsigned char _CEC_SendRemainingDataBlocks(void)
{
	CEC_FRAME_DESCRIPTION_IO *frame;
	unsigned char errcode = 0;
	unsigned char size;
	unsigned char *sendidx;
	unsigned char *blocks;
	unsigned int data;
	unsigned char i, j;

	if (!IsCECStatus(STATE_TXING_FRAME))
		return 0;

	if (IsCECStatus(STATE_WAIT_TX_DATA_TAKEN)) {
		if (IS_INT_RB_ENABLE_CEC() && IS_TX_DATA_TAKEN_CEC()) {
			ClrCECStatus(STATE_WAIT_TX_DATA_TAKEN);
		} else {
			/* tx buffer is not emply */
			return 0;
		}
	} else {
		return 0;
	}

	/* request current active TX frame */
	frame = ActiveTXFrame;

	size = frame->size;
	sendidx = &(frame->sendidx);
	blocks = &(frame->blocks.opcode);

	/* CYJ.NOTE: Leave "TX hardware error handling" to _CEC_Mainloop */
	if (IS_TX_FSM_FAIL_CEC() | (TX_FAIL_RECORD_CEC() > 0)) {
		HDMI_CEC_LOG("Detect TX FAIL in %s\n", __func__);
		return 3;
		/* RESET_HW_TX(); */
	}

	size -= ((*sendidx) + 1);

	if (size == 0)
		return 0;

	/* CYJ:TODO duplicate (as _CEC_SendFrame())! */
	/* fill data */
	if (size > 4) {
		SET_HW_TX_LEN_CEC(0xf);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(0);
	} else if (size == 4) {
		SET_HW_TX_LEN_CEC(0xf);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 3) {
		SET_HW_TX_LEN_CEC(0x7);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 2) {
		SET_HW_TX_LEN_CEC(0x3);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 1) {
		SET_HW_TX_LEN_CEC(0x1);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	}

	data = 0;
	for (i = 0, j = size; i < 4; i++) {
		data >>= 8;
		data |= (blocks[(*sendidx)]) << 24;
		if (i < j) {
			(*sendidx)++;
			size--;
		}
	}

	/* EOM */
	if (size == 0) {
		ENABLE_INT_FAIL_CEC(1);
		ENABLE_INT_RB_CEC(0);
		ENABLE_INT_LOW_CEC(0);
		ENABLE_INT_UN_CEC(0);
		ENABLE_INT_BS_CEC(0);
	} else {
		ENABLE_INT_FAIL_CEC(1);
		ENABLE_INT_RB_CEC(1);
		ENABLE_INT_LOW_CEC(1);
		ENABLE_INT_UN_CEC(1);
		ENABLE_INT_BS_CEC(0);

		SetCECStatus(STATE_WAIT_TX_DATA_TAKEN);
	}

	FILL_TX_DATA_CEC(data);
	HDMI_CEC_LOG("TRIGGER_TX_HW in %s, size: %x\n", __func__, size);

	CLR_TX_FINISH_CEC();

	TRIGGER_TX_HW_CEC();

	return errcode;
}

static void _CECCheck_Active_Tx_Result_Old(struct mtk_cec *cec)
{
	HDMI_CEC_FUNC();

	if (cec->hwip_switch != CECHW_IP_OLD)
		return;

	if (IsCECStatus(STATE_TXING_FRAME)) {
		if (IsCECStatus(STATE_HW_RETX)) {
			if (TX_FAIL_SOURCE_CEC()) {
				_u1TxFailCause = FAIL_SOURCE;
				CLR_TX_FAIL_SOURCE_CEC();
			}
			if ((TX_FAIL_RECORD_CEC() != 0)) {
				DISABLE_ALL_TX_INT_CEC();

				SetCECStatus(STATE_TX_NOACK);
				ClrCECStatus(STATE_HW_RETX);

				ClrCECStatus(STATE_TXING_FRAME);
				if (IS_TX_FSM_FAIL_CEC())
					HDMI_CEC_LOG("[hdmi_cec]TX NO ACK\n");
				else
					HDMI_CEC_LOG("[hdmi_cec]other TX error\n");

				HDMI_CEC_LOG("H ACK: %x, D ACK: %x\n", GET_FOLLOWER_H_ACK_CEC(),
					     GET_FOLLOWER_D_ACK_CEC());

				RESET_HW_TX_CEC();
			}
		} else if ((ActiveTXFrame->sendidx + 1) == (ActiveTXFrame->size)) {

			if (IS_TX_FSM_IDLE_CEC() && IS_TX_FINISH_CEC()) {
				DISABLE_ALL_TX_INT_CEC();
				SetCECStatus(STATE_TX_FRAME_SUCCESS);
				ClrCECStatus(STATE_TXING_FRAME);
				HDMI_CEC_LOG("TX is COMPLETED with: H ACK: %x and D ACK %x\n",
					     (unsigned int)GET_FOLLOWER_H_ACK_CEC(),
					     (unsigned int)GET_FOLLOWER_D_ACK_CEC());
			}
		}
	}

}

static CEC_FRAME_DESCRIPTION_IO *_CEC_Get_Cur_TX_Q_Msg(void)
{
	HDMI_CEC_FUNC();

	if (IS_TX_Q_EMPTY())
		return NULL;

	return (&(CEC_tx_msg_queue[CEC_txQ_read_idx]));
}

static unsigned char _CEC_TX_Dequeue(void)
{
	HDMI_CEC_FUNC();

	if (IS_TX_Q_EMPTY())
		return FALSE;
	/* CYJ.NOTE: no critical section */
	CEC_txQ_read_idx = (CEC_txQ_read_idx + 1) % TX_Q_SIZE;

	return TRUE;
}

static void _CEC_tx_msg_notify(unsigned char result, CEC_FRAME_DESCRIPTION_IO *frame)
{
	if (result == 0x00) {
		/* HDMI_CEC_LOG("cec tx result ok\n"); */
		cec_send_result.pv_tag = frame->txtag;
		cec_send_result.e_ack_cond = APK_CEC_ACK_COND_OK;
		if (_CEC_Notneed_Notify == 0xff)
			hdmi_CECSendNotify(HDMI_CEC_TX_STATUS);
		if (hdmi_rxcecmode == CEC_KER_HANDLE_MODE)
			CEC_KHandle_TXNotify(HDMI_CEC_TX_STATUS);
		HDMI_CEC_COMMAND_LOG("sending opcode-->success: 0x%x\n",
				     frame->blocks.opcode);
	} else if (result == 0x01) {
		if ((frame->size == 0x2) && (frame->blocks.header.initiator == 4)
		    && (frame->blocks.header.destination == 4) && (frame->blocks.opcode == 0))
			HDMI_CEC_COMMAND_LOG("get LA of player\n");
		else
			TX_DEF_LOG("[CEC] tx result fail, op=0x%x\n", frame->blocks.opcode);

		cec_send_result.pv_tag = frame->txtag;
		cec_send_result.e_ack_cond = APK_CEC_ACK_COND_NO_RESPONSE;
		if (_CEC_Notneed_Notify == 0xff)
			hdmi_CECSendNotify(HDMI_CEC_TX_STATUS);
		if (hdmi_rxcecmode == CEC_KER_HANDLE_MODE)
			CEC_KHandle_TXNotify(HDMI_CEC_TX_STATUS);
	}
}

void hdmi_cec_api_get_txsts(APK_CEC_ACK_INFO *pt)
{
	memcpy(pt, &cec_send_result, sizeof(APK_CEC_ACK_INFO));
}

static void vApiNotifyCECDataArrival(CEC_FRAME_DESCRIPTION_IO *frame)
{
	cec_receive_msg = frame;
	if (_CEC_Notneed_Notify == 0xff)
		vNotifyAppHdmiCecState(HDMI_CEC_GET_CMD);
}

static unsigned char vGetcecmsg_poweron(CEC_FRAME_DESCRIPTION_IO *frame)
{
	unsigned short msg_phy_addr = 0xFFFF;

	if (frame->blocks.opcode == OPCODE_USER_CONTROL_PRESSED) {
		if ((frame->blocks.operand[0] == 0x40) || (frame->blocks.operand[0] == 0x6D) ||
		   ((frame->blocks.operand[0] >= 0x00) && (frame->blocks.operand[0] <= 0x08))) {
			TX_DEF_LOG("[CECWAKE]waking up on power command(0x%x:0x%x) from tv\n",frame->blocks.opcode, frame->blocks.operand[0]);
			return 1;
		}
	}

	if ((frame->blocks.opcode == OPCODE_SET_STREAM_PATH) || (frame->blocks.opcode == OPCODE_ROUTING_INFORMATION))
		msg_phy_addr = ((frame->blocks.operand[0] & 0xff) << 8) | (frame->blocks.operand[1] & 0xff);

	if (frame->blocks.opcode == OPCODE_ROUTING_CHANGE)
		msg_phy_addr = ((frame->blocks.operand[2] & 0xff) << 8) | (frame->blocks.operand[3] & 0xff);

	if (msg_phy_addr != 0xFFFF) {
		TX_DEF_LOG("[CECWAKE]received power command(0x%x), msg_phy_addr=0x%x\n", frame->blocks.opcode, msg_phy_addr);
		TX_DEF_LOG("[CECWAKE]_rCECPhysicAddr.ui2_pa=0x%x, cec_wakeup_addr=0x%x\n", _rCECPhysicAddr.ui2_pa, cec_wakeup_addr);

		if (msg_phy_addr == _rCECPhysicAddr.ui2_pa) {
			TX_DEF_LOG("[CECWAKE]waking up on power command(0x%x) from tv\n",frame->blocks.opcode);
			return 1;
		}

		msleep(100);	/*wait for reading edid */
		TX_DEF_LOG("[CECWAKE]updated _rCECPhysicAddr.ui2_pa=0x%x, cec_wakeup_addr=0x%x\n", _rCECPhysicAddr.ui2_pa, cec_wakeup_addr);
		/* Compare with previous physical address stored in cec_wakeup_addr in case EDID parsed address
		   is still not available. This can happen as HPD can go down and up during input switches
		   resulting in physical address not being available. User device will most likely be still
		   on the same port. If it is not, we will falsely turn on the screen without it being active
		   as active port address will be the one coming in messages <Set Stream Path> or <Routing Change>.
		   This should not hamper user experience in general but will improve it for most cases. */
		if ((msg_phy_addr == _rCECPhysicAddr.ui2_pa) ||
			(msg_phy_addr == cec_wakeup_addr)) {
			TX_DEF_LOG("[CECWAKE]waking up on power command(0x%x) from tv\n",frame->blocks.opcode);
			return 1;
		}
	}

	return 0;
}

static void PrintFrameDescription(CEC_FRAME_DESCRIPTION_IO *frame)
{
	unsigned char i;

	HDMI_CEC_LOG(">>>>>>>>>>>>>>>>>>>>\n");
	HDMI_CEC_LOG("frame description:\n");
	HDMI_CEC_LOG("size: 0x%x\n", frame->size);
	HDMI_CEC_LOG("sendidx: 0x%x\n", frame->sendidx);
	HDMI_CEC_LOG("reTXcnt: 0x%x\n", frame->reTXcnt);
	HDMI_CEC_LOG("pv_tag=%p\n", frame->txtag);
	HDMI_CEC_LOG("initiator: 0x%x\n", frame->blocks.header.initiator);
	HDMI_CEC_LOG("destination: 0x%x\n", frame->blocks.header.destination);
	if (frame->size > 1)
		HDMI_CEC_LOG("opcode: 0x%x\n", frame->blocks.opcode);

	if ((frame->size > 2) && (frame->size <= 16)) {
		for (i = 0; i < (frame->size - 2); i++)
			HDMI_CEC_LOG("0x%02x\n", frame->blocks.operand[i]);
	}
	HDMI_CEC_LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

static unsigned char _CEC_SendFrame_Old(struct mtk_cec *cec, CEC_FRAME_DESCRIPTION_IO *frame)
{
	unsigned char errcode = 0;
	unsigned char size;
	unsigned char *sendidx;
	unsigned char *blocks;
	unsigned int data;
	unsigned char i, j;

	HDMI_CEC_FUNC();
	if (cec->hwip_switch != CECHW_IP_OLD)
		return 1;

	if (IsCECStatus(STATE_TXING_FRAME))
		return 1;

	SetCECStatus(STATE_TXING_FRAME);

	/* CYJ.NOTE: Leave "TX hardware error handling" to _CEC_Mainloop */
	if (IS_TX_FSM_FAIL_CEC() | (TX_FAIL_RECORD_CEC() > 0)) {
		HDMI_CEC_LOG("Detect TX FAIL in %s\n", __func__);
		/* return 3; */
		RESET_HW_TX_CEC();
	}

	size = frame->size;
	sendidx = &(frame->sendidx);
	blocks = &(frame->blocks.opcode);

	if (size == 0) {
		ClrCECStatus(STATE_TXING_FRAME);
		return 2;
	} else if (size > 16) {
		ClrCECStatus(STATE_TXING_FRAME);
		return 2;
	}
	/* CYJ.NOTE: TX HW is not idle */
	if (!IS_TX_FSM_IDLE_CEC())
		RESET_HW_TX_CEC();
	ActiveTXFrame = frame;

	ClrCECStatus(STATE_TX_FRAME_SUCCESS);
	ClrCECStatus(STATE_TX_NOACK);


	/* fill header */
	FILL_SRC_FIELD_CEC(frame->blocks.header.initiator);
	FILL_DST_FIELD_CEC(frame->blocks.header.destination);
	size -= 1;

	/* header-only */
	if (size == 0) {
		SET_HW_TX_LEN_CEC(0);
		MARK_H_EOM_CEC(1);
		MARK_D_EOM_CEC(0);
		/* TRIGGER_TX_HW(); */
		size = 0;
	}

	/* fill data */
	if (size > 4) {
		SET_HW_TX_LEN_CEC(0xf);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(0);
	} else if (size == 4) {
		SET_HW_TX_LEN_CEC(0xf);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 3) {
		SET_HW_TX_LEN_CEC(0x7);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 2) {
		SET_HW_TX_LEN_CEC(0x3);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	} else if (size == 1) {
		SET_HW_TX_LEN_CEC(0x1);
		MARK_H_EOM_CEC(0);
		MARK_D_EOM_CEC(1);
	}

	data = 0;
	for (i = 0, j = size; i < 4; i++) {
		data >>= 8;
		data |= (blocks[(*sendidx)]) << 24;
		if (i < j) {
			(*sendidx)++;
			size--;
		}
	}

	/* EOM */
	if (size == 0) {
		ENABLE_INT_FAIL_CEC(1);
		ENABLE_INT_RB_CEC(0);
		ENABLE_INT_LOW_CEC(0);
		ENABLE_INT_UN_CEC(0);
		ENABLE_INT_BS_CEC(0);
	} else {
		ENABLE_INT_FAIL_CEC(1);
		ENABLE_INT_RB_CEC(1);
		ENABLE_INT_LOW_CEC(1);
		ENABLE_INT_UN_CEC(1);
		ENABLE_INT_BS_CEC(0);

		SetCECStatus(STATE_WAIT_TX_DATA_TAKEN);
	}

	FILL_TX_DATA_CEC(data);
	HDMI_CEC_LOG("TRIGGER_TX_HW in %s, size: %x\n", __func__, size);

	CLR_TX_FINISH_CEC();
	_u1TxFailCause = FAIL_NONE;
	_u1ReTxCnt = 0;
	TRIGGER_TX_HW_CEC();

	return errcode;
}

static unsigned char _CEC_SendFrame(struct mtk_cec *cec,	CEC_FRAME_DESCRIPTION_IO *frame)
{
	CEC_FRAME_BLOCK_IO *blocks;
	unsigned char data_len;

	HDMI_CEC_FUNC();

	if (IsCECStatus(STATE_TXING_FRAME | STATE_WAIT_TX_CHECK_RESULT)) {
		HDMI_CEC_LOG("TX FAIL in %s,line=%d\n", __func__, __LINE__);
		return 1;
	}
	/* CYJ.NOTE: Leave "TX hardware error handling" to _CEC_Mainloop */
	if (mtkcec_hwtx_fail_fsm(cec) || mtkcec_hwtx_fail(cec)) {
		HDMI_CEC_LOG("Detect TX FAIL in %s,line=%d\n", __func__, __LINE__);
		mtkcec_hwtx_reset(cec);
		if (mtkcec_hw_input(cec))
			mtkcec_hwtx_fail_longlow_intclr(cec);
		return 2;
	}
	if ((frame->size <= 0) || (frame->size > 16)) {
		ClrCECStatus(STATE_TXING_FRAME);
		HDMI_CEC_LOG("TX FAIL in %s,line=%d\n", __func__, __LINE__);
		return 3;
	}
	/* CYJ.NOTE: TX HW is not idle */
	if (!mtkcec_hwtx_fsm_idle(cec))
		mtkcec_hwtx_reset(cec);

	ActiveTXFrame = frame;
	data_len = frame->size - 1;
	blocks = &(frame->blocks);

	/* fill header */
	mtkcec_hwtx_setsrc(cec, frame->blocks.header.initiator);
	mtkcec_hwtx_setdst(cec, frame->blocks.header.destination);

	mtkcec_hwtx_int_alldisable(cec);
	mtkcec_hwtx_int_all_clear(cec);
	/* header-only */
	if (data_len == 0)
		mtkcec_hwtx_set_h_eom(cec, 1);
	else {
		mtkcec_hwtx_set_h_eom(cec, 0);
		mtkcec_hwtx_setdatalen(cec, data_len);
		mtkcec_hwtx_setdata(cec, blocks, data_len);
	}

	ClrCECStatus(STATE_TX_FRAME_SUCCESS | STATE_TX_NOACK | STATE_WAIT_TX_CHECK_RESULT);
	ClrCECStatus(STATE_TXFAIL_DNAK | STATE_TXFAIL_HNAK | STATE_TXFAIL_RETR);
	ClrCECStatus(STATE_TXFAIL_DATA | STATE_TXFAIL_HEAD | STATE_TXFAIL_SRC | STATE_TXFAIL_LOW);
	SetCECStatus(STATE_TXING_FRAME);

	mtkcec_hwtx_finish_int_en(cec, 1);
	mtkcec_hwtx_fail_longlow_int_en(cec, 1);
	mtkcec_hwtx_fail_retransmit_int_en(cec, 1);
	mtkcec_hwtx_trigger(cec);
	HDMI_CEC_LOG("TRIGGER_TX_HW in %s, data_len: %x\n", __func__, data_len);

	return 0;
}

static void _CEC_TX_Queue_Loop_Old(struct mtk_cec *cec)
{
	CEC_FRAME_DESCRIPTION_IO *frame;
	/*HDMI_CEC_FUNC();*/

	if (cec->hwip_switch != CECHW_IP_OLD)
		return;

	/* if the tx message queue is empty */
	if (IS_TX_Q_EMPTY())
		return;

	/* if the tx is active, check the result */
	if (IsCECStatus(STATE_TXING_FRAME)) {
		if (IsCECErrorFlag(ERR_TX_MISALARM) && (TX_FAIL_RECORD_CEC() != 0) && IS_TX_FSM_FAIL_CEC()) {
			DISABLE_ALL_TX_INT_CEC();
			ClrCECErrorFlag(ERR_TX_MISALARM);
			SetCECStatus(STATE_TX_NOACK);
			ClrCECStatus(STATE_HW_RETX);
			ClrCECStatus(STATE_TXING_FRAME);
			RESET_HW_TX_CEC();
			TX_DEF_LOG("[CEC]ERR_TX_MISALARM\n");
		}

		_CECCheck_Active_Tx_Result_Old(cec);
		if (IsCECStatus(STATE_TX_FRAME_SUCCESS)) {
			/* HDMI_CEC_LOG("This message is successful\n"); */
			frame = _CEC_Get_Cur_TX_Q_Msg();
			if (frame == NULL)
				TX_DEF_LOG("[CEC] ack msg frame null\n");

			_CEC_tx_msg_notify(0x00, frame);
			_CEC_TX_Dequeue();
			ClrCECStatus(STATE_TX_FRAME_SUCCESS);
		}
		if (IsCECStatus(STATE_TX_NOACK)) {
			frame = _CEC_Get_Cur_TX_Q_Msg();
			if (frame == NULL)
				TX_DEF_LOG("[CEC] noack msg frame null\n");

			HDMI_CEC_LOG("[hdmi_cec]This message is failed: %d\n", frame->reTXcnt);
			frame->reTXcnt++;
			frame->sendidx = 0;
			ClrCECStatus(STATE_TX_NOACK);
			/* CYJ.NOTE: retransmission */
			if (frame->reTXcnt == RETX_MAX_CNT) {
				_u1TxFailCause = FAIL_NONE;
				HDMI_CEC_LOG("ReTX reach MAX\n");
				_CEC_tx_msg_notify(0x01, frame);
				_CEC_TX_Dequeue();
			}
		}
	} else {
		/* if the tx is not active, send the next message */
		frame = _CEC_Get_Cur_TX_Q_Msg();
		if (frame == NULL)
			TX_DEF_LOG("[CEC] next msg frame null\n");

		if (_u1TxFailCause == FAIL_SOURCE) {
			if (_u1ReTxCnt < 15) {
				_u1ReTxCnt++;
				return;
			} else
				_u1ReTxCnt = 0;
		}
		/* HDMI_CEC_LOG("Send a new message\n"); */
		PrintFrameDescription(frame);
		_CEC_SendFrame_Old(cec, frame);
		HDMI_CEC_COMMAND_LOG("sending opcode-->start: 0x%x\n",
				     frame->blocks.opcode);
	}
}

static void _CEC_TX_check_result(struct mtk_cec *cec)
{
	CEC_FRAME_DESCRIPTION_IO *frame;
	unsigned char result = 3;

	HDMI_CEC_FUNC();
	frame = _CEC_Get_Cur_TX_Q_Msg();

	if (IsCECStatus(STATE_TXFAIL_RETR)) {
		if ((frame->blocks.header.destination == CEC_LOG_ADDR_UNREGED_BRDCST)) {
			if (IsCECStatus(STATE_TX_NOACK)) {
				result = 1;
				ClrCECStatus(STATE_TX_NOACK);
				HDMI_CEC_LOG("CEC Brdcst msg success\n");
			} else {
				result = 0;
				HDMI_CEC_LOG("CEC Brdcst msg fail\n");
			}
		} else {
			result = 0;
			HDMI_CEC_LOG("CEC Direct msg fail: ");
			if (IsCECStatus(STATE_TXFAIL_HNAK)) {
				ClrCECStatus(STATE_TX_NOACK | STATE_TXFAIL_HNAK);
				HDMI_CEC_LOG("err:HNack, ");
			}
			if (IsCECStatus(STATE_TXFAIL_DNAK)) {
				ClrCECStatus(STATE_TX_NOACK | STATE_TXFAIL_DNAK);
				HDMI_CEC_LOG("err:DNack, ");
			}
			if (IsCECStatus(STATE_TXFAIL_DATA)) {
				ClrCECStatus(STATE_TXFAIL_DATA);
				HDMI_CEC_LOG("err:data, ");
			}
			if (IsCECStatus(STATE_TXFAIL_HEAD)) {
				ClrCECStatus(STATE_TXFAIL_HEAD);
				HDMI_CEC_LOG("err:header, ");
			}
			if (IsCECStatus(STATE_TXFAIL_SRC)) {
				ClrCECStatus(STATE_TXFAIL_SRC);
				HDMI_CEC_LOG("err:src, ");
			}
			HDMI_CEC_LOG("err:Retransmit\n");
		}
		ClrCECStatus(STATE_TXFAIL_RETR);
	}

	if (IsCECStatus(STATE_TXFAIL_LOW)) {
		result = 0;
		mtkcec_hwtx_reset(cec);
		HDMI_CEC_LOG("CEC Direct/Brdcst msg fail:long low");
		ClrCECStatus(STATE_TXFAIL_LOW);
	}

	if (IsCECStatus(STATE_TX_FRAME_SUCCESS)) {
		result = 1;
		ClrCECStatus(STATE_TX_FRAME_SUCCESS);
		if ((frame->blocks.header.destination == CEC_LOG_ADDR_UNREGED_BRDCST))
			HDMI_CEC_LOG("CEC Brdcst message success\n");
		else
			HDMI_CEC_LOG("CEC Direct message success\n");
	}

	if (result == 1) {
		_CEC_tx_msg_notify(0x00, frame);
		_CEC_TX_Dequeue();
	} else if (result == 0) {
		_CEC_tx_msg_notify(0x01, frame);
		_CEC_TX_Dequeue();
	} else
		HDMI_CEC_LOG("CEC message unknown result\n");
}

static void _CEC_TX_Queue_Loop(struct mtk_cec *cec)
{
	CEC_FRAME_DESCRIPTION_IO *frame;

	if (cec->hwip_switch != CECHW_IP_NEW)
		return;

	if (IS_TX_Q_EMPTY())
		return;

	if (IsCECStatus(STATE_WAIT_TX_CHECK_RESULT)) {
		_CEC_TX_check_result(cec);
		ClrCECStatus(STATE_WAIT_TX_CHECK_RESULT);
	} else if (!IsCECStatus(STATE_TXING_FRAME)) {
		frame = _CEC_Get_Cur_TX_Q_Msg();
		if (frame != NULL) {
			PrintFrameDescription(frame);
			if (!_CEC_SendFrame(cec, frame))
				HDMI_CEC_COMMAND_LOG("sending opcode:0x%x\n",
					frame->blocks.opcode);
		}
	}
}

static CEC_FRAME_DESCRIPTION_IO *CEC_rx_dequeue(void)
{
	CEC_FRAME_DESCRIPTION_IO *ret;
	/* HDMI_CEC_FUNC(); */

	/* check if queue is empty */
	if (IS_RX_Q_EMPTY())
		return NULL;

	/* return next available entry for middleware */
	ret = &(CEC_rx_msg_queue[CEC_rxQ_read_idx]);

	/* CYJ.NOTE: no critical section */
	CEC_rxQ_read_idx = (CEC_rxQ_read_idx + 1) % RX_Q_SIZE;

	return ret;
}

static int hdmi_cec_txmsgq_dump(char *str)
{
	CEC_FRAME_DESCRIPTION_IO *txmsg;
	unsigned char tx_read_idx = CEC_txQ_read_idx;
	unsigned char tx_write_idx = CEC_txQ_write_idx;
	int len = 0;
	char j, i = 0;

	if (tx_read_idx <= 0)
		tx_read_idx = TX_Q_SIZE;
	else
		tx_read_idx--;

	while (!(tx_read_idx == tx_write_idx)) {
		txmsg = &(CEC_tx_msg_queue[tx_read_idx]);
		tx_read_idx = (tx_read_idx + 1) % TX_Q_SIZE;

		if (txmsg->size > 0)
			len += sprintf(str, "%s txmsgq[%02d]=0x%02x", str, (i - 1),
			(txmsg->blocks.header.initiator << 4) | txmsg->blocks.header.destination);
		if (txmsg->size > 1)
			len += sprintf(str, "%s,0x%02x", str, txmsg->blocks.opcode);
		for (j = 2; j < txmsg->size; j++)
			len += sprintf(str, "%s,0x%02x", str, txmsg->blocks.operand[j - 2]);

		len += sprintf(str, "%s\n", str);
		i++;
	}

	return len;
}

static int hdmi_cec_rxmsgq_dump(char *str)
{
	CEC_FRAME_DESCRIPTION_IO *rxmsg;
	unsigned char rx_read_idx = CEC_rxQ_read_idx;
	unsigned char rx_write_idx = CEC_rxQ_write_idx;
	int len = 0;
	char j, i = 0;

	if (rx_read_idx <= 0)
		rx_read_idx = RX_Q_SIZE;
	else
		rx_read_idx--;

	while (!(rx_read_idx == rx_write_idx)) {
		rxmsg = &(CEC_rx_msg_queue[rx_read_idx]);
		rx_read_idx = (rx_read_idx + 1) % RX_Q_SIZE;

		if (rxmsg->size > 0)
			len += sprintf(str, "%s rxmsgq[%02d]=0x%02x", str, (i - 1),
			(rxmsg->blocks.header.initiator << 4) | rxmsg->blocks.header.destination);
		if (rxmsg->size > 1)
			len += sprintf(str, "%s,0x%02x", str, rxmsg->blocks.opcode);
		for (j = 2; j < rxmsg->size; j++)
			len += sprintf(str, "%s,0x%02x", str, rxmsg->blocks.operand[j - 2]);

		len += sprintf(str, "%s\n", str);
		i++;
	}

	return len;
}

static int hdmi_cec_msgq_dump(char *str)
{
	int len;

	len = hdmi_cec_txmsgq_dump(str);
	len += hdmi_cec_rxmsgq_dump(str);
	return len;
}

static int hdmi_cec_level_dump(char *str)
{
	int len;

	if (global_cec == NULL) {
		TX_DEF_LOG("[CEC] NULL in %s\n", __func__);
		len = sprintf(str, "%s \nNULL in %s\n", str, __func__);
		return len;
	}
	len = sprintf(str, "%s cec level=%d\n", str, mtkcec_hw_input(global_cec));

	return len;
}

static int hdmi_cec_args_dump(char *str)
{
	struct mtk_cec *cec = global_cec;
	char name[15] = {0};
	int len;

	if (cec == NULL) {
		TX_DEF_LOG("[CEC] NULL in %s\n", __func__);
		len = sprintf(str, "%s \nNULL in %s\n", str, __func__);
		return len;
	}

	memcpy(&name, &cec->kh.saved_arg.osd_name, cec->kh.saved_arg.osd_namelen);
	name[cec->kh.saved_arg.osd_namelen] = '\0';
	len = sprintf(str, "%s pa[0-1]=0x%04x\n", str, *cec->kh.saved_arg.pa);
	len += sprintf(str, "%s version=0x%02x\n", str, cec->kh.saved_arg.version);
	len += sprintf(str, "%s device_type=0x%02x\n", str, cec->kh.saved_arg.device_type);
	len += sprintf(str, "%s vendor_id[0-2]=0x%02x,0x%02x,0x%02x\n", str,
		cec->kh.saved_arg.vendor_id[0], cec->kh.saved_arg.vendor_id[1], cec->kh.saved_arg.vendor_id[2]);
	len += sprintf(str, "%s osd_name=%s\n", str, name);

	return len;
}

int hdmi_cec_status_dump(char *str)
{
	int len;

	len = hdmi_cec_level_dump(str);
	len += hdmi_cec_msgq_dump(str);
	len += hdmi_cec_args_dump(str);

	return len;
}

static unsigned char CEC_frame_validation(CEC_FRAME_DESCRIPTION_IO *frame)
{
	unsigned char size = frame->size;
	unsigned char i1ret = TRUE;

	HDMI_CEC_FUNC();

	/* opcode-aware */
	/* CYJ.NOTE: code size issue */
	switch (frame->blocks.opcode) {
		/* length == 2 */
	case OPCODE_IMAGE_VIEW_ON:
	case OPCODE_TEXT_VIEW_ON:
	case OPCODE_REQUEST_ACTIVE_SOURCE:
	case OPCODE_STANDBY:
	case OPCODE_RECORD_OFF:
	case OPCODE_RECORD_TV_SCREEN:
	case OPCODE_GET_CEC_VERSION:
	case OPCODE_GIVE_PHYSICAL_ADDRESS:
	case OPCODE_GET_MENU_LANGUAGE:
	case OPCODE_TUNER_STEP_DECREMENT:
	case OPCODE_TUNER_STEP_INCREMENT:
	case OPCODE_GIVE_DEVICE_VENDOR_ID:
	case OPCODE_VENDOR_REMOTE_BUTTON_UP:
	case OPCODE_GIVE_OSD_NAME:
	case OPCODE_USER_CONTROL_RELEASED:
	case OPCODE_GIVE_DEVICE_POWER_STATUS:
	case OPCODE_ABORT:
	case OPCODE_GIVE_AUDIO_STATUS:
	case OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS:
		if (size != 2)
			i1ret = FALSE;
		break;
	case OPCODE_SYSTEM_AUDIO_MODE_REQUEST:
		if ((size != 2) && (size != 4))
			i1ret = FALSE;
		break;
		/* length == 3 */
	case OPCODE_RECORD_STATUS:
	case OPCODE_TIMER_CLEARED_STATUS:
	case OPCODE_CEC_VERSION:
	case OPCODE_DECK_CONTROL:
	case OPCODE_DECK_STATUS:
	case OPCODE_GIVE_DECK_STATUS:
	case OPCODE_PLAY:
	case OPCODE_GIVE_TUNER_DEVICE_STATUS:
	case OPCODE_MENU_REQUEST:
	case OPCODE_MENU_STATUS:
	case OPCODE_REPORT_POWER_STATUS:
	case OPCODE_REPORT_AUDIO_STATUS:
	case OPCODE_SET_SYSTEM_AUDIO_MODE:
	case OPCODE_SYSTEM_AUDIO_MODE_STATUS:
	case OPCODE_SET_AUDIO_RATE:
		if (size != 3)
			i1ret = FALSE;
		break;
	case OPCODE_USER_CONTROL_PRESSED:
		if ((size != 3) && (size != 4))
			i1ret = FALSE;
		break;
		/* length == 4 */
	case OPCODE_ACTIVE_SOURCE:
	case OPCODE_INACTIVE_SOURCE:
	case OPCODE_ROUTING_INFORMATION:
	case OPCODE_SET_STREAM_PATH:
	case OPCODE_FEATURE_ABORT:
	case OPCODE_REQUEST_CURRENT_LATENCY:
		if (size != 4)
			i1ret = FALSE;
		break;
		/* length == 5 */
	case OPCODE_REPORT_PHYSICAL_ADDRESS:
	case OPCODE_SET_MENU_LANGUAGE:
	case OPCODE_DEVICE_VENDOR_ID:
		if (size != 5)
			i1ret = FALSE;
		break;
		/* length == 6 */
	case OPCODE_ROUTING_CHANGE:
	case OPCODE_SELECT_ANALOGUE_SERVICE:
		if (size != 6)
			i1ret = FALSE;
		break;
		/* length == 9 */
	case OPCODE_SELECT_DIGITAL_SERVICE:
		if (size != 9)
			i1ret = FALSE;
		break;
		/* length == 13 */
	case OPCODE_CLEAR_ANALOGUE_TIMER:
	case OPCODE_SET_ANALOGUE_TIMER:
		if (size != 13)
			i1ret = FALSE;
		break;
		/* length == 16 */
	case OPCODE_CLEAR_DIGITAL_TIMER:
	case OPCODE_SET_DIGITAL_TIMER:
		if (size != 16)
			i1ret = FALSE;
		break;
	case OPCODE_RECORD_ON:
		if ((size < 3) || (size > 10))
			i1ret = FALSE;
		break;
		/* length == 10 ~ 11 */
	case OPCODE_CLEAR_EXTERNAL_TIMER:
	case OPCODE_SET_EXTERNAL_TIMER:
		if ((size < 10) || (size > 11))
			i1ret = FALSE;
		break;
	case OPCODE_TIMER_STATUS:
		if ((size != 3) && (size != 5))
			i1ret = FALSE;
		break;
	case OPCODE_TUNER_DEVICE_STATUS:
		if ((size != 7) && (size != 10))
			i1ret = FALSE;
		break;
	case OPCODE_VENDOR_COMMAND:
	case OPCODE_VENDOR_COMMAND_WITH_ID:
	case OPCODE_VENDOR_REMOTE_BUTTON_DOWN:
		if (size > 16)
			i1ret = FALSE;
		break;
	case OPCODE_SET_OSD_STRING:
		if ((size < 3) || (size > 16))
			i1ret = FALSE;
		break;
	case OPCODE_SET_TIMER_PROGRAM_TITLE:
	case OPCODE_SET_OSD_NAME:
		if ((size < 3) || (size > 16))
			i1ret = FALSE;
		break;
	case OPCODE_GET_CURRENT_LATENCY:
		if ((size < 6) || (size > 7))
			i1ret = FALSE;
		break;
	}
	if (i1ret == FALSE) {
		HDMI_CEC_LOG("receive invalid frame: %x\n", frame->blocks.opcode);
		PrintFrameDescription(frame);
	}
	return i1ret;
}

static unsigned char check_and_init_tx_frame(CEC_FRAME_DESCRIPTION_IO *frame)
{
	unsigned char ret = 0x00;

	HDMI_CEC_FUNC();

	if ((frame->size > CEC_MAX_MESG_SIZE) || (frame->size == 0)) {
		HDMI_CEC_LOG("Tx fram size is not correct\n");
		ret = 0x01;
	}
	/* valid tx frame */
	if (ret == 0x00) {
		frame->reTXcnt = 0;
		frame->sendidx = 0;
	}

	return ret;
}

unsigned char _CEC_TX_Enqueue(CEC_FRAME_DESCRIPTION_IO *frame)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();
	if (frame->size == 1) {
		HDMI_CEC_LOG("Polling LA = 0x%x\n",
			     (unsigned char)((frame->blocks.header.initiator << 4) | frame->blocks.
					     header.destination));
	} else
		HDMI_CEC_LOG("Opcode = 0x%x, size = 0x%x\n", frame->blocks.opcode, frame->size);

	if (check_and_init_tx_frame(frame))
		return 0x01;

	if (IS_TX_Q_FULL()) {
		HDMI_CEC_LOG("Tx queue is full\n");
		return 0x01;
	}

	memcpy(&(CEC_tx_msg_queue[CEC_txQ_write_idx]), frame, sizeof(CEC_FRAME_DESCRIPTION_IO));
	/* CYJ.NOTE: no critical section */
	CEC_txQ_write_idx = (CEC_txQ_write_idx + 1) % TX_Q_SIZE;
	mtkcec_wake_lock(cec);

	return 0x00;
}

void hdmi_u4CecSendSLTData(unsigned char *pu1Data)
{
	unsigned char i;

	if (*pu1Data > 14)
		*pu1Data = 14;

	CTSTestFrame.size = *pu1Data + 2;
	CTSTestFrame.sendidx = 0;
	CTSTestFrame.reTXcnt = 0;
	CTSTestFrame.txtag = NULL;
	CTSTestFrame.blocks.header.destination = 0x00;
	CTSTestFrame.blocks.header.initiator = 0x04;
	CTSTestFrame.blocks.opcode = 0x01;
	for (i = 0; i < *pu1Data; i++)
		CTSTestFrame.blocks.operand[i] = *(pu1Data + i + 1);

	_CEC_TX_Enqueue(&CTSTestFrame);

}

void hdmi_GetSLTData(CEC_SLT_DATA *rCecSltData)
{
	unsigned char i;
	CEC_FRAME_DESCRIPTION_IO *frame;

	HDMI_CEC_FUNC();

	frame = CEC_rx_dequeue();

	if (frame == NULL) {
		rCecSltData->u1Size = 5;
		for (i = 0; i < rCecSltData->u1Size; i++)
			rCecSltData->au1Data[i] = i;

		ClrCECStatus(STATE_RX_COMPLETE_NEW_FRAME);
		return;
	}

	if (frame->blocks.opcode == 0x01) {
		if (CEC_frame_validation(frame))
			PrintFrameDescription(frame);

		rCecSltData->u1Size = frame->size - 2;
		if (rCecSltData->u1Size > 14)
			rCecSltData->u1Size = 14;

		for (i = 0; i < rCecSltData->u1Size; i++)
			rCecSltData->au1Data[i] = frame->blocks.operand[i];

		HDMI_CEC_LOG("[CEC SLT] Receive data\n");
		HDMI_CEC_LOG("[CEC SLT] size = 0x%x\n", rCecSltData->u1Size);
		HDMI_CEC_LOG("[CEC SLT] data = ");

		for (i = 0; i < rCecSltData->u1Size; i++)
			HDMI_CEC_LOG(" 0x%x  ", rCecSltData->au1Data[i]);

		HDMI_CEC_LOG("\n");
	}
}

void CTS_RXProcess(CEC_FRAME_DESCRIPTION_IO *frame)
{
	HDMI_CEC_FUNC();

	if (frame->blocks.opcode == OPCODE_ABORT) {
		CTSTestFrame.size = 4;
		CTSTestFrame.sendidx = 0;
		CTSTestFrame.reTXcnt = 0;
		CTSTestFrame.txtag = NULL;
		CTSTestFrame.blocks.header.destination = frame->blocks.header.initiator;
		CTSTestFrame.blocks.header.initiator = 4;
		CTSTestFrame.blocks.opcode = OPCODE_FEATURE_ABORT;
		CTSTestFrame.blocks.operand[0] = OPCODE_ABORT;
		CTSTestFrame.blocks.operand[1] = 4;

		/*Test */
		CTSTestFrame.blocks.operand[2] = 0x41;
		CTSTestFrame.blocks.operand[3] = 0x0;
		CTSTestFrame.blocks.operand[4] = 0xff;
		CTSTestFrame.blocks.operand[5] = 4;
		CTSTestFrame.blocks.operand[6] = 0x41;
		CTSTestFrame.blocks.operand[7] = 0x0;
		CTSTestFrame.blocks.operand[8] = 0xff;
		CTSTestFrame.blocks.operand[9] = 4;
		CTSTestFrame.blocks.operand[10] = 10;
		CTSTestFrame.blocks.operand[11] = 11;
		CTSTestFrame.blocks.operand[12] = 12;
		CTSTestFrame.blocks.operand[13] = 13;
		/* Test */

		_CEC_TX_Enqueue(&CTSTestFrame);
		HDMI_CEC_LOG("CTS Send: OPCODE_FEATURE_ABORT\n");
	} else if (frame->blocks.opcode == OPCODE_GIVE_PHYSICAL_ADDRESS) {
		CTSTestFrame.size = 5;
		CTSTestFrame.sendidx = 0;
		CTSTestFrame.reTXcnt = 0;
		CTSTestFrame.txtag = NULL;
		CTSTestFrame.blocks.header.destination = 0xf;
		CTSTestFrame.blocks.header.initiator = 4;
		CTSTestFrame.blocks.opcode = OPCODE_REPORT_PHYSICAL_ADDRESS;
		CTSTestFrame.blocks.operand[0] = 0x10;
		CTSTestFrame.blocks.operand[1] = 0x00;
		CTSTestFrame.blocks.operand[2] = 0x04;

		_CEC_TX_Enqueue(&CTSTestFrame);
		HDMI_CEC_LOG("CTS Send: OPCODE_REPORT_PHYSICAL_ADDRESS\n");
	}
}

void FAC_RXProcess(CEC_FRAME_DESCRIPTION_IO *frame)
{
	cec_receive_msg = frame;
	if (frame->blocks.opcode == OPCODE_FEATURE_ABORT)
		TX_DEF_LOG("CEC FAC_RXProcess pass:0x%x\n", frame->blocks.opcode);
	else
		TX_DEF_LOG("CEC FAC_RXProcess err:0x%x\n", frame->blocks.opcode);
}

static void CEC_KHandle_RXNotify(CEC_FRAME_DESCRIPTION_IO *frame)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();
	cec_receive_msg = frame;
	schedule_work(&cec->kh.cec_work);
}

void CEC_KHandle_TXNotify(HDMI_NFY_CEC_STATE_T u1hdmicecstate)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();
	hdmi_cec_api_get_txsts(&cec->kh.tx_result);
	wake_up_interruptible(&cec->kh.waitq_tx);
}

static int CEC_KHandle_TxWaitResult(struct mtk_cec *cec)
{
	int ret;

	HDMI_CEC_FUNC();
	ret = wait_event_interruptible_timeout(cec->kh.waitq_tx,
		(cec->kh.tx_result.pv_tag == cec->kh.txing_frame.txtag), msecs_to_jiffies(2000));

	if ((ret > 0) && (cec->kh.tx_result.e_ack_cond == APK_CEC_ACK_COND_OK))
		return 1;
	else
		return 0;
}

static unsigned char CEC_KHandle_GetBestLa(struct mtk_cec *cec)
{
	unsigned char dstla;

	if (_rCECLaAddr.aui1_la[0] != 0xff)
		dstla = _rCECLaAddr.aui1_la[0];
	else if (_rCECLaAddr.aui1_la[1] != 0xff)
		dstla = _rCECLaAddr.aui1_la[1];
	else if (_rCECLaAddr.aui1_la[2] != 0xff)
		dstla = _rCECLaAddr.aui1_la[2];
	else
		dstla = CEC_LOG_ADDR_PLAYBACK_DEV_1;
	return dstla;
}

static void CEC_KHandle_BuildReplyMsg(struct mtk_cec *cec)
{
	static unsigned int sequence;
	CEC_FRAME_DESCRIPTION_IO *rx_frame = &(cec->kh.rx_frame);

	HDMI_CEC_FUNC();
	memset(&cec->kh.txing_frame, 0, sizeof(cec->kh.txing_frame));
	cec->kh.txing_frame.blocks.header.initiator = CEC_KHandle_GetBestLa(cec);
	cec->kh.txing_frame.blocks.header.destination = rx_frame->blocks.header.initiator;
	sequence = (sequence + 1) >= UINT_MAX? 0 : (sequence + 1);

	switch (rx_frame->blocks.opcode) {
	case OPCODE_FEATURE_ABORT:
	case OPCODE_CEC_VERSION:
	case OPCODE_DEVICE_VENDOR_ID:
	case OPCODE_REQUEST_ACTIVE_SOURCE:
		HDMI_CEC_LOG("[KH] bypass op=0x%02x\n", rx_frame->blocks.opcode);
		break;
	case OPCODE_GIVE_OSD_NAME:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_SET_OSD_NAME;
		memcpy(&cec->kh.txing_frame.blocks.operand,
			&cec->kh.saved_arg.osd_name, cec->kh.saved_arg.osd_namelen);
		cec->kh.txing_frame.size = cec->kh.saved_arg.osd_namelen + 2;
		break;
	case OPCODE_STANDBY:
		HDMI_CEC_LOG("[KH] allready in Standby\n");
		break;
	case OPCODE_GET_CEC_VERSION:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_CEC_VERSION;
		cec->kh.txing_frame.blocks.operand[0] = cec->kh.saved_arg.version;
		cec->kh.txing_frame.size = 3;
		break;
	case OPCODE_GIVE_PHYSICAL_ADDRESS:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_REPORT_PHYSICAL_ADDRESS;
		cec->kh.txing_frame.blocks.operand[0] =
			(unsigned char)(((*cec->kh.saved_arg.pa) & 0xff00) >> 8);
		cec->kh.txing_frame.blocks.operand[1] =
			(unsigned char)((*cec->kh.saved_arg.pa) & 0x00ff);
		cec->kh.txing_frame.blocks.operand[2] = cec->kh.saved_arg.device_type;
		cec->kh.txing_frame.size = 5;
		break;
	case OPCODE_GIVE_DEVICE_VENDOR_ID:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_DEVICE_VENDOR_ID;
		cec->kh.txing_frame.blocks.operand[0] = cec->kh.saved_arg.vendor_id[0];
		cec->kh.txing_frame.blocks.operand[1] = cec->kh.saved_arg.vendor_id[1];
		cec->kh.txing_frame.blocks.operand[2] = cec->kh.saved_arg.vendor_id[2];
		cec->kh.txing_frame.size = 5;
		break;
	case OPCODE_GIVE_DEVICE_POWER_STATUS:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_REPORT_POWER_STATUS;
		cec->kh.txing_frame.blocks.operand[0] = 0x01;
		cec->kh.txing_frame.size = 3;
		break;
	case OPCODE_ABORT:
		cec->kh.txing_frame.txtag = (void *)sequence;
		cec->kh.txing_frame.blocks.opcode = OPCODE_FEATURE_ABORT;
		cec->kh.txing_frame.blocks.operand[0] = OPCODE_ABORT;
		cec->kh.txing_frame.blocks.operand[1] = 4;
		cec->kh.txing_frame.size = 4;
		break;
	default:
		HDMI_CEC_LOG("[KH] not reply to op=0x%02x\n", rx_frame->blocks.opcode);
		break;
	}

}

static void CEC_KHandle_Reply(struct mtk_cec *cec)
{
	char max_retry;

	HDMI_CEC_FUNC();
	if (cec->kh.txing_frame.size > 0) {
		cec->kh.txing_frame.sendidx = 0;
		cec->kh.txing_frame.reTXcnt = 0;
		max_retry = cec->kh.max_retry;
		do {
			_CEC_TX_Enqueue(&cec->kh.txing_frame);
			if (CEC_KHandle_TxWaitResult(cec) > 0) {
				HDMI_CEC_LOG("%s, op=0x%02x,success\n", __func__,
					cec->kh.txing_frame.blocks.opcode);
				break;
			} else
				TX_DEF_LOG("[CEC][KH][Error]reTXcnt=%d\n",
					cec->kh.txing_frame.reTXcnt);
			cec->kh.txing_frame.reTXcnt += 2;
		} while (max_retry--);
		memset(&cec->kh.txing_frame, 0, sizeof(cec->kh.txing_frame));
	}
}

static int CEC_KHandle_WorkMsg(struct mtk_cec *cec)
{
	HDMI_CEC_FUNC();
	hdmi_CECMWGet(&cec->kh.rx_frame);
	if (cec->kh.rx_frame.size == 1) {
		HDMI_CEC_LOG("KHandle get polling msg only\n");
		return -1;
	}
	CEC_KHandle_BuildReplyMsg(cec);
	CEC_KHandle_Reply(cec);

	return 0;
}

static void CEC_KHandle_Work(struct work_struct *work)
{
	struct mtk_cec *cec;
	struct cec_kernel_handle *cec_kh;

	HDMI_CEC_FUNC();
	cec_kh = container_of(work, struct cec_kernel_handle, cec_work);
	cec = container_of(cec_kh, struct mtk_cec, kh);

	CEC_KHandle_WorkMsg(cec);
}

static void CEC_rx_msg_notify(unsigned char u1rxmode)
{
	CEC_FRAME_DESCRIPTION_IO *frame;

	/* HDMI_CEC_FUNC(); */
	if (u1rxmode == CEC_SLT_MODE)
		return;

	while (1) {
		if (cec_msg_report_pending == 1) {
			/* HDMI_CEC_LOG("wait user get cmd\n"); */
			return;
		}

		frame = CEC_rx_dequeue();

		if (frame == NULL) {
			ClrCECStatus(STATE_RX_COMPLETE_NEW_FRAME);
			return;
		}

		if (CEC_frame_validation(frame)) {
			/* HDMI_CEC_LOG("Receive message\n"); */
			PrintFrameDescription(frame);

			HDMI_CEC_COMMAND_LOG("rxing opcode-->success: 0x%x\n",
					     frame->blocks.opcode);
			cec_msg_report_pending = 1;
			if (u1rxmode == CEC_CTS_MODE)
				CTS_RXProcess(frame);
			else if (u1rxmode == CEC_FAC_MODE)
				FAC_RXProcess(frame);
			else if (u1rxmode == CEC_KER_HANDLE_MODE) {
				/* process only required messages - it could be a wakeup
				   message or a message needing mandatory response back.
				   But a message cant be both - use if else to optimize */
				if (vGetcecmsg_poweron(frame)) {
					report_virtual_hdmikey();
					cec_msg_report_pending = 0;
				} else
					CEC_KHandle_RXNotify(frame);
			} else
				vApiNotifyCECDataArrival(frame);
		}
	}
}

void hdmi_cec_mainloop(unsigned char u1rxmode)
{
	struct mtk_cec *cec;

	/* HDMI_CEC_FUNC(); */
	if (global_cec == NULL)
		return;

	cec = global_cec;
	if (cec->hwip_switch == CECHW_IP_OLD)
		_CEC_TX_Queue_Loop_Old(cec);
	else
		_CEC_TX_Queue_Loop(cec);

	/* NOTE: the priority between tx and rx */
	if (!IsCECStatus(STATE_TXING_FRAME))
		CEC_rx_msg_notify(u1rxmode);

}

unsigned char hdmi_cec_isrprocess_old(unsigned char u1rxmode)
{
	unsigned char u1ReceivedDst;
	HDMI_CEC_FUNC();

	if (HW_RX_HEADER_ARRIVED_CEC()) {
		u1ReceivedDst = GET_DST_FIELD_RECEIVING_CEC();
		HDMI_CEC_LOG("u1ReceivedDst = 0x%08x\n", u1ReceivedDst);
		if ((u1ReceivedDst == _rCECLaAddr.aui1_la[0])
		    || (u1ReceivedDst == _rCECLaAddr.aui1_la[1])
		    || (u1ReceivedDst == _rCECLaAddr.aui1_la[2]) || (u1ReceivedDst == 0xf)) {
			HDMI_CEC_LOG("RX:H\n");
			if (IsCECStatus(STATE_RX_GET_NEW_HEADER)) {
				TX_DEF_LOG("[CEC]Lost EOM:1\n");
				SetCECErrorFlag(ERR_RX_LOST_EOM);
			}
			SetCECStatus(STATE_RX_GET_NEW_HEADER);
		} else {
			ClrCECStatus(STATE_RX_GET_NEW_HEADER);
			HDMI_CEC_LOG("[hdmi_cec]RX:H False\n");
		}
	}
	if (HW_RX_DATA_ARRIVED_CEC()) {
		HDMI_CEC_LOG("RX:D\n");
		_CEC_Receiving_Old();
	}
	if (IS_INT_OV_CEC()) {
		TX_DEF_LOG("[CEC]Overflow\n");
		CLR_INT_OV_CEC();
		SetCECStatus(STATE_HW_RX_OVERFLOW);
	}
	/* TX_EVENT */
	if (IsCECStatus(STATE_TXING_FRAME)) {
		if (IS_INT_UN_CEC()) {
			TX_DEF_LOG("[CEC]Underrun\n");
			CLR_INT_UN_CEC();
			SetCECErrorFlag(ERR_TX_UNDERRUN);
		}
		if (IS_INT_LOW_CEC()) {
			HDMI_CEC_LOG("[hdmi_cec]Buffer Low\n");
			CLR_INT_LOW_CEC();
			if (!IS_INT_RB_RDY_CEC()) {
				TX_DEF_LOG("[CEC]FW is slow to trigger the following blocks\n");
				SetCECErrorFlag(ERR_TX_BUFFER_LOW);
			}
		}
		if (IS_INT_RB_ENABLE_CEC() && IS_TX_DATA_TAKEN_CEC()) {
			/* HDMI_CEC_LOG("TX Data Taken\n"); */
			_CEC_SendRemainingDataBlocks();
		}
		/* CYJ.NOTE TX Failure Detection */
		if (IS_INT_FAIL_ENABLE_CEC() && (TX_FAIL_RECORD_CEC() != 0)) {
			DISABLE_ALL_TX_INT_CEC();
			SetCECStatus(STATE_HW_RETX);

			if (TX_FAIL_MAX_CEC() | IS_TX_FSM_FAIL_CEC())
				HDMI_CEC_LOG("[hdmi_cec]TX MAX or Fail: %x\n", TX_FAIL_RECORD_CEC());
			else
				HDMI_CEC_LOG("[hdmi_cec]TX Fail: %x\n", TX_FAIL_RECORD_CEC());
		}
		/* HDMI_CEC_LOG("[hdmi_cec]TX HW FSM: %x\n", TX_FSM_STATUS_CEC()); */
	}

	if (IS_RX_Q_EMPTY())
		return 0;
	else
		return 1;
}

unsigned char hdmi_cec_isrprocess_new(struct mtk_cec *cec, unsigned char u1rxmode)
{
	unsigned char u1ReceivedDst;

	HDMI_CEC_REG_LOG("cecisr=0x%08x,0x%08x,0x%08x\n", cec->cec_read(cec, CEC2_INT_STA),
		cec->cec_read(cec, CEC2_INT_EN), (cec->cec_read(cec, CEC2_INT_STA) & cec->cec_read(cec, CEC2_INT_EN)));

	if (mtkcec_hwrx_header_arrived(cec)) {
		u1ReceivedDst = mtkcec_hwrx_get_dst(cec);
		HDMI_CEC_LOG("header_addr=0x%08x\n", (mtkcec_hwrx_get_src(cec) << 4) + u1ReceivedDst);
		if ((u1ReceivedDst == _rCECLaAddr.aui1_la[0])
		    || (u1ReceivedDst == _rCECLaAddr.aui1_la[1])
		    || (u1ReceivedDst == _rCECLaAddr.aui1_la[2])
		    || (u1ReceivedDst == 0xf)) {
			HDMI_CEC_LOG("RX:H\n");
			SetCECStatus(STATE_RX_GET_NEW_HEADER);
		} else {
			ClrCECStatus(STATE_RX_GET_NEW_HEADER);
			HDMI_CEC_LOG("[hdmi_cec]RX:H False\n");
		}
	}
	if (mtkcec_hwrx_data_arrived(cec) || mtkcec_hwrx_buffer_ready(cec)) {
		HDMI_CEC_LOG("RX:D\n");
		_CEC_Receiving(cec);
	}
	if (mtkcec_hwrx_buffer_full(cec)) {
		HDMI_CEC_LOG("[hdmi_cec]Overflow\n");
		SetCECStatus(STATE_HW_RX_OVERFLOW);
	}

	/* TX_EVENT */
	mtkcec_hwtx_detec_startbit(cec);
	if (IsCECStatus(STATE_TXING_FRAME)) {
		if (mtkcec_hwtx_fail_longlow(cec)) {
			mtkcec_hwtx_fail_longlow_int_en(cec, 0);
			mtkcec_hwtx_fail_longlow_intclr(cec);
			SetCECStatus(STATE_TXFAIL_LOW);
			SetCECStatus(STATE_WAIT_TX_CHECK_RESULT);
		}
		if (mtkcec_hwtx_fail_retransmit_intsta(cec)) {
			if (mtkcec_hwtx_fail_h_ack(cec)) {
				mtkcec_hwtx_fail_h_ack_intclr(cec);
				SetCECStatus(STATE_TX_NOACK);
				SetCECStatus(STATE_TXFAIL_HNAK);
			}
			if (mtkcec_hwtx_fail_data_ack(cec)) {
				mtkcec_hwtx_fail_data_ack_intclr(cec);
				SetCECStatus(STATE_TX_NOACK);
				SetCECStatus(STATE_TXFAIL_DNAK);
			}
			if (mtkcec_hwtx_fail_data(cec)) {
				mtkcec_hwtx_fail_data_intclr(cec);
				SetCECStatus(STATE_TXFAIL_DATA);
			}
			if (mtkcec_hwtx_fail_header(cec)) {
				mtkcec_hwtx_fail_header_intclr(cec);
				SetCECStatus(STATE_TXFAIL_HEAD);
			}
			if (mtkcec_hwtx_fail_src(cec)) {
				mtkcec_hwtx_fail_src_intclr(cec);
				SetCECStatus(STATE_TXFAIL_SRC);
			}

			mtkcec_hwtx_fail_retransmit_intclr(cec);
			SetCECStatus(STATE_TXFAIL_RETR);
			SetCECStatus(STATE_WAIT_TX_CHECK_RESULT);
			ClrCECStatus(STATE_TXING_FRAME);
			mtkcec_wake_unlock(cec);
		}

		if (mtkcec_hwtx_finish(cec)) {
			mtkcec_hwtx_finish_clr(cec);
			mtkcec_hwtx_int_alldisable(cec);
			mtkcec_hwtx_int_all_clear(cec);
			SetCECStatus(STATE_TX_FRAME_SUCCESS);
			SetCECStatus(STATE_WAIT_TX_CHECK_RESULT);
			ClrCECStatus(STATE_TXING_FRAME);
			mtkcec_wake_unlock(cec);
		}
	}
	if (IS_RX_Q_EMPTY())
		return 0;
	else
		return 1;
}

void CECMWSetLA(CEC_LA_ADDRESS *prLA)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();
	memcpy(&_rCECLaAddr, prLA, sizeof(CEC_LA_ADDRESS));

	if (_rCECLaAddr.ui1_num == 0) {
		if (cec->hwip_switch == CECHW_IP_OLD) {
			SET_LA3_CEC(0x0F);
			SET_LA2_CEC(0x0F);
			SET_LA1_CEC(0x0F);
		} else {
			mtkcec_hw_set_la3(cec, 0x0F);
			mtkcec_hw_set_la2(cec, 0x0F);
			mtkcec_hw_set_la1(cec, 0x0F);
		}
		_rCECLaAddr.aui1_la[0] = 0x0F;
		_rCECLaAddr.aui1_la[1] = 0x0F;
		_rCECLaAddr.aui1_la[2] = 0x0F;
	} else if (_rCECLaAddr.ui1_num == 1) {
		if (cec->hwip_switch == CECHW_IP_OLD) {
			SET_LA3_CEC(0x0F);
			SET_LA2_CEC(0x0F);
			SET_LA1_CEC(_rCECLaAddr.aui1_la[0]);
		} else {
			mtkcec_hw_set_la3(cec, 0x0F);
			mtkcec_hw_set_la2(cec, 0x0F);
			mtkcec_hw_set_la1(cec, _rCECLaAddr.aui1_la[0]);
		}
		_rCECLaAddr.aui1_la[1] = 0x0F;
		_rCECLaAddr.aui1_la[2] = 0x0F;
	} else if (_rCECLaAddr.ui1_num == 2) {
		if (cec->hwip_switch == CECHW_IP_OLD) {
			SET_LA3_CEC(0x0F);
			SET_LA2_CEC(_rCECLaAddr.aui1_la[1]);
			SET_LA1_CEC(_rCECLaAddr.aui1_la[0]);
		} else {
			mtkcec_hw_set_la3(cec, 0x0F);
			mtkcec_hw_set_la2(cec, _rCECLaAddr.aui1_la[1]);
			mtkcec_hw_set_la1(cec, _rCECLaAddr.aui1_la[0]);
		}
		_rCECLaAddr.aui1_la[2] = 0x0F;
	} else if (_rCECLaAddr.ui1_num == 3) {
		if (cec->hwip_switch == CECHW_IP_OLD) {
			SET_LA3_CEC(_rCECLaAddr.aui1_la[2]);
			SET_LA2_CEC(_rCECLaAddr.aui1_la[1]);
			SET_LA1_CEC(_rCECLaAddr.aui1_la[0]);
		} else {
			mtkcec_hw_set_la3(cec, _rCECLaAddr.aui1_la[2]);
			mtkcec_hw_set_la2(cec, _rCECLaAddr.aui1_la[1]);
			mtkcec_hw_set_la1(cec, _rCECLaAddr.aui1_la[0]);
		}
	}

	HDMI_CEC_LOG("LA num = 0x%x , LA = 0x%x 0x%x 0x%x\n", _rCECLaAddr.ui1_num,
		     _rCECLaAddr.aui1_la[0], _rCECLaAddr.aui1_la[1], _rCECLaAddr.aui1_la[2]);
	HDMI_CEC_LOG("PA = %04x\n", _rCECLaAddr.ui2_pa);
}

void hdmi_CECMWSetLA(CEC_DRV_ADDR_CFG *prAddr)
{
	CEC_LA_ADDRESS rLA;

	HDMI_CEC_FUNC();

	if (prAddr->ui1_la_num > 3)
		return;

	rLA.ui1_num = prAddr->ui1_la_num;
	rLA.aui1_la[0] = prAddr->e_la[0];
	rLA.aui1_la[1] = prAddr->e_la[1];
	rLA.aui1_la[2] = prAddr->e_la[2];
	rLA.ui2_pa = prAddr->ui2_pa;
	CECMWSetLA(&rLA);
}

void hdmi_CECMWGet(CEC_FRAME_DESCRIPTION_IO *frame)
{
	HDMI_CEC_FUNC();
	if (cec_msg_report_pending == 0) {
		TX_DEF_LOG("[CEC]get cec msg fail\n");
		return;
	}
	memcpy(frame, cec_receive_msg, sizeof(CEC_FRAME_DESCRIPTION_IO));
	cec_msg_report_pending = 0;
}


void hdmi_CECSendNotify(HDMI_NFY_CEC_STATE_T u1hdmicecstate)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();
	hdmi_cec_api_get_txsts(&cec->txmsg_usr->result);
	wake_up_interruptible(&cec->waitq_tx);
}

bool hdmi_CECSendWaitResult(struct mtk_cec *cec)
{
	int ret;

	HDMI_CEC_FUNC();
	ret = wait_event_interruptible_timeout(cec->waitq_tx,
		(cec->txmsg_usr->result.pv_tag == cec->txmsg_usr->pv_tag), msecs_to_jiffies(2000));

	if ((ret > 0) && (cec->txmsg_usr->result.e_ack_cond == APK_CEC_ACK_COND_OK)) {
		HDMI_CEC_LOG("send msg ACK\n");
		return true;
	} else if (ret == 0) {
		cec->txmsg_usr->result.pv_tag = cec->txmsg_usr->pv_tag;
		cec->txmsg_usr->result.e_ack_cond = APK_CEC_ACK_COND_NO_RESPONSE;
		HDMI_CEC_LOG("send msg NACK, timeout\n");
		return false;
	} else {
		cec->txmsg_usr->result.e_ack_cond = APK_CEC_ACK_COND_NO_RESPONSE;
		HDMI_CEC_LOG("send msg NACK\n");
		return false;
	}
}

void hdmi_CECMWSend(CEC_SEND_MSG *msg)
{
	unsigned int i4Ret;
	unsigned char i;
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();

	cec->txmsg_usr = msg;
	if ((cec->txmsg_usr->t_frame_info.ui1_init_addr > 0xf)
		|| (cec->txmsg_usr->t_frame_info.ui1_dest_addr > 0xf)
	    || (cec->txmsg_usr->t_frame_info.z_operand_size > LOCAL_CEC_MAX_OPERAND_SIZE)) {
	    cec->txmsg_usr->b_enqueue_ok = FALSE;
		cec->txmsg_usr->result.pv_tag = cec->txmsg_usr->pv_tag;
		cec->txmsg_usr->result.e_ack_cond = APK_CEC_ACK_COND_NO_RESPONSE;
		HDMI_CEC_LOG("apk send msg error\n");
		return;
	}

	cec->txmsg_ker.txtag = cec->txmsg_usr->pv_tag;
	cec->txmsg_ker.blocks.header.initiator = cec->txmsg_usr->t_frame_info.ui1_init_addr;
	cec->txmsg_ker.blocks.header.destination = cec->txmsg_usr->t_frame_info.ui1_dest_addr;
	cec->txmsg_ker.sendidx = 0;
	cec->txmsg_ker.reTXcnt = 0;
	if (cec->txmsg_usr->t_frame_info.ui2_opcode == 0xffff)
		cec->txmsg_ker.size = 1;
	else {
		cec->txmsg_ker.blocks.opcode = cec->txmsg_usr->t_frame_info.ui2_opcode;
		cec->txmsg_ker.size = cec->txmsg_usr->t_frame_info.z_operand_size + 2;
	}
	for (i = 0; i < cec->txmsg_ker.size - 2; i++)
		cec->txmsg_ker.blocks.operand[i] = cec->txmsg_usr->t_frame_info.aui1_operand[i];

	i4Ret = _CEC_TX_Enqueue(&cec->txmsg_ker);
	if (i4Ret == 0x01) {
		cec->txmsg_usr->b_enqueue_ok = FALSE;
		cec->txmsg_usr->result.pv_tag = cec->txmsg_usr->pv_tag;
		cec->txmsg_usr->result.e_ack_cond = APK_CEC_ACK_COND_NO_RESPONSE;
		HDMI_CEC_LOG("MW Set cmd fail\n");
		return;
	} else {
		cec->txmsg_usr->b_enqueue_ok = TRUE;
		HDMI_CEC_LOG("MW Set cmd success\n");
	}
	hdmi_CECSendWaitResult(cec);
}

void hdmi_CECMWSetEnableCEC(unsigned char u1EnCec)
{
	struct mtk_cec *cec = global_cec;

	HDMI_CEC_FUNC();

	if ((u1EnCec == 1) && (hdmi_cec_on == 0)) {
		HDMI_CEC_LOG("UI ON\n");
		hdmi_cec_on = 1;
		hdmi_cec_power_on(cec, true);
		hdmi_cec_init(cec);
		cec_timer_wakeup();
	} else if ((u1EnCec == 0) && (hdmi_cec_on == 1)) {
		HDMI_CEC_LOG("UI off\n");
		hdmi_cec_on = 0;
		cec_timer_sleep();
		hdmi_cec_deinit(cec);
		hdmi_cec_power_on(cec, false);
	} else {
		HDMI_CEC_LOG("[hdmi_cec]on/off fail,en=%d,hdmi_cec_on=%d\n", u1EnCec, hdmi_cec_on);
	}
}

void hdmi_NotifyApiCECAddress(CEC_ADDRESS_IO *cecaddr)
{
	HDMI_CEC_FUNC();

	cecaddr->ui2_pa = _rCECPhysicAddr.ui2_pa;
	cecaddr->ui1_la = _rCECPhysicAddr.ui1_la;
}

void hdmi_SetPhysicCECAddress(unsigned short u2pa, unsigned char u1la)
{
	/* To support CEC wake up, we need to store previous address
	   as we might have to use it for wakeup in case <Routing Change> or
	   <Set Stream Path> is received with HPD down - very typical as these
	   commands are sent during input switches or while turning on the TV
	   when HPD keeps toggling before stabilizing to high state and we need
	   to have a valid address to compare against in such cases */
	cec_wakeup_addr = (u2pa == 0xFFFF) ? _rCECPhysicAddr.ui2_pa : u2pa;
	HDMI_CEC_LOG("cec_wakeup_addr=0x%x, u2pa=0x%x,u1la=0x%x\n", cec_wakeup_addr, u2pa, u1la);
	_rCECPhysicAddr.ui2_pa = u2pa;
	_rCECPhysicAddr.ui1_la = u1la;
}

bool hdmi_cec_factory_test(void)
{
	CEC_DRV_ADDR_CFG cecsetAddr;
	CEC_SEND_MSG txdata;
	unsigned int result;
	CEC_FRAME_DESCRIPTION_IO rxdata;
	unsigned char backup_rxcecmode = hdmi_rxcecmode;
	int wait_times, retry = 2;

	memset(&rxdata, 0, sizeof(rxdata));
	rxdata.blocks.opcode = OPCODE_ABORT;

	cecsetAddr.ui1_la_num = 0x01;
	cecsetAddr.e_la[0] = CEC_LOG_ADDR_PLAYBACK_DEV_1;
	txdata.t_frame_info.ui1_init_addr = CEC_LOG_ADDR_PLAYBACK_DEV_1;
	txdata.t_frame_info.ui1_dest_addr = CEC_LOG_ADDR_TV;
	txdata.t_frame_info.ui2_opcode = OPCODE_ABORT;
	txdata.t_frame_info.z_operand_size = 0;

	hdmi_rxcecmode = CEC_FAC_MODE;
	hdmi_CECMWSetEnableCEC(0x01);
	hdmi_CECMWSetLA(&cecsetAddr);
	while (!IS_RX_Q_EMPTY())
		CEC_rx_dequeue();
	while (!IS_TX_Q_EMPTY())
		_CEC_TX_Dequeue();

	do {
		hdmi_CECMWSend(&txdata);
		wait_times = 10;
		do {
			msleep(100);
			hdmi_cec_usr_cmd(0x00, &result);
		} while ((wait_times--) && (result <= 0));
	} while ((retry--) && (result <= 0));

	if (result > 0)
		msleep(100);

	hdmi_CECMWGet(&rxdata);
	if (rxdata.blocks.opcode == OPCODE_FEATURE_ABORT) {
		TX_DEF_LOG("%s()pass!\n", __func__);
		hdmi_rxcecmode = backup_rxcecmode;
		return true;
	}

	TX_DEF_LOG("%s()failed! %d,%d,%d\n", __func__, retry, wait_times, result);
	while (!IS_RX_Q_EMPTY())
		CEC_rx_dequeue();

	hdmi_rxcecmode = backup_rxcecmode;
	return false;
}

void hdmi_cec_usr_cmd(unsigned int cmd, unsigned int *result)
{
	switch (cmd) {
	case 0:
		*result = cec_msg_report_pending;
		break;
	case 1:
		*result = hdmi_hotplugstate;
		TX_DEF_LOG("[CEC] hdmi_hotplugstate=%d\n", hdmi_hotplugstate);
		break;
	case 2:
		hdmi_hotplugstate = 1;
		break;
	case 3:
		hdmi_CECMWSetEnableCEC(1);
		break;
	case 4:
		hdmi_CECMWSetEnableCEC(0);
		break;
	case 5:
		_CEC_Notneed_Notify = 1;
		TX_DEF_LOG("[CEC] _CEC_Notneed_Notify = %d\n", _CEC_Notneed_Notify);
		break;
	case 6:
		_CEC_Notneed_Notify = 0xff;
		TX_DEF_LOG("[CEC] _CEC_Notneed_Notify = %d\n", _CEC_Notneed_Notify);
		break;
	case 7:
		cec_clock = cec_clk_26m;
		TX_DEF_LOG("[CEC] cec_clock = %d\n", cec_clock);
		break;
	case 8:
		cec_clock = cec_clk_32k;
		TX_DEF_LOG("[CEC] cec_clock = %d\n", cec_clock);
		break;

	default:
		break;
	}
}

inline bool impl_cec_readbit(struct mtk_cec *cec, unsigned short reg, unsigned int offset)
{
	if (cec->hwip_switch == CECHW_IP_OLD)
		return (readl(cec->cec1_base + reg) & offset) ? true : false;
	else
		return (readl(cec->cec2_base + reg) & offset) ? true : false;
}

inline unsigned int impl_cec_read(struct mtk_cec *cec, unsigned short reg)
{
	if (cec->hwip_switch == CECHW_IP_OLD)
		return readl(cec->cec1_base + reg);
	else
		return readl(cec->cec2_base + reg);
}

inline void impl_cec_write(struct mtk_cec *cec, unsigned short reg, unsigned int val)
{
	if (cec->hwip_switch == CECHW_IP_OLD)
		writel(val, cec->cec1_base + reg);
	else
		writel(val, cec->cec2_base + reg);
}

inline void impl_cec_mask(struct mtk_cec *cec, unsigned int reg,
			 unsigned int val, unsigned int mask)
{
	unsigned int tmp;

	if (cec->hwip_switch == CECHW_IP_OLD) {
		tmp = readl(cec->cec1_base + reg) & ~mask;
		tmp |= (val & mask);
		writel(tmp, cec->cec1_base + reg);
	} else {
		tmp = readl(cec->cec2_base + reg) & ~mask;
		tmp |= (val & mask);
		writel(tmp, cec->cec2_base + reg);
	}
}

inline void impl_cec_common_mask(struct mtk_cec *cec, void __iomem *addr,
			 unsigned int val, unsigned int mask)
{
	unsigned int tmp;

	tmp = readl(addr) & ~mask;
	tmp |= (val & mask);
	writel(tmp, addr);
}

inline void mtkcec_regops_probe(struct mtk_cec *cec)
{
	cec->cec_readbit = impl_cec_readbit;
	cec->cec_read = impl_cec_read;
	cec->cec_write = impl_cec_write;
	cec->cec_mask = impl_cec_mask;
	cec->common_mask = impl_cec_common_mask;
}

inline void mtkcec_kernel_hanlde_probe(struct mtk_cec *cec)
{
	const char name[15] = "Amazon FireTV";

	cec->kh.max_retry = 1;
	cec->kh.saved_arg.version = 0x05;
	cec->kh.saved_arg.pa = &_rCECPhysicAddr.ui2_pa;
	cec->kh.saved_arg.device_type = 0x04;
	cec->kh.saved_arg.vendor_id[0] = 0x00;
	cec->kh.saved_arg.vendor_id[1] = 0x0c;
	cec->kh.saved_arg.vendor_id[2] = 0xe7;
	/* 1 <= Osd Name length <= 14*/
	cec->kh.saved_arg.osd_namelen = strlen(name);
	memcpy(&cec->kh.saved_arg.osd_name, name, cec->kh.saved_arg.osd_namelen);
	init_waitqueue_head(&cec->waitq_tx);
	init_waitqueue_head(&cec->kh.waitq_tx);
	INIT_WORK(&cec->kh.cec_work, CEC_KHandle_Work);
}

static irqreturn_t cec_irq_handler(int irq, void *dev_id)
{
	struct mtk_cec *cec = global_cec;

	if (hdmi_cec_on == 1) {
		if (cec->hwip_switch == CECHW_IP_OLD)
			hdmi_cec_isrprocess_old(hdmi_rxcecmode);
		else
			hdmi_cec_isrprocess_new(cec, hdmi_rxcecmode);
	}
	mtkcec_irq2gic_clear(cec);

	return IRQ_HANDLED;
}

int hdmi_cec_probe(struct platform_device *pdev)
{
	struct mtk_hdmi *hdmi = platform_get_drvdata(pdev);
	struct mtk_cec *cec;
	struct device_node	*of_node;
	unsigned int cec_irq_type;

	/* cec does not have device of itself, therefor use the device of hdmi */
	cec = devm_kzalloc(hdmi->dev, sizeof(*cec), GFP_KERNEL);
	if (!cec)
		return -ENOMEM;

	cec->hdmi = hdmi;
	global_cec = hdmi->cec = cec;
	cec->pdev = pdev;
	of_node = cec->pdev->dev.of_node;
	cec->cec1_base = of_iomap(of_node, REG_HDMI_CEC1);
	cec->cec2_base = of_iomap(of_node, REG_HDMI_CEC2);
	if (cec->cec1_base == NULL || cec->cec2_base == NULL) {
		HDMI_CEC_LOG("Unable to ioremap registers\n");
		return -ENOMEM;
	}

	mtkcec_clk_probe(cec);
	mtkcec_regops_probe(cec);
	mtkcec_hw_switch(cec, CECHW_IP_NEW);
	mtkcec_kernel_hanlde_probe(cec);
	cec->irq = irq_of_parse_and_map(of_node, 1);
	cec_irq_type = irq_get_trigger_type(cec->irq);
	if (request_irq(cec->irq, cec_irq_handler, cec_irq_type, "cecirq", cec) < 0)
		HDMI_CEC_LOG("request cec interrupt failed.\n");
	else
		HDMI_CEC_LOG("request CEC interrupt success\n");

	return 0;
}
#endif
