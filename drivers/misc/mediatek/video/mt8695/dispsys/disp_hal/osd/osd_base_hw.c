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
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/file.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/mutex.h>

#include "disp_osd_if.h"
#include "osd_hw.h"
#include "disp_osd_log.h"

/*todo:change support to plane counter support*/
OSD_BASE_UNION_T _rOsdBaseReg[OSD_FMT_MAX_NUM];
static OSD_BASE_UNION_T *_prHwOsdBaseReg[OSD_FMT_MAX_NUM];
OSD_PREMIX_UNION_T _osd_premix_reg;
static OSD_PREMIX_UNION_T *_pr_hw_osd_premix_reg;


void _OSD_BASE_GET_REG_BASE(uintptr_t *base_reg, uintptr_t premix_reg)
{
	_prHwOsdBaseReg[OSD_FMT_1] = (OSD_BASE_UNION_T *) base_reg[0];
	_prHwOsdBaseReg[OSD_FMT_2] = (OSD_BASE_UNION_T *) base_reg[1];
	_pr_hw_osd_premix_reg = (OSD_PREMIX_UNION_T *)premix_reg;
	OSDDBG("2:hw base register =0x%p,0x%p", _prHwOsdBaseReg[OSD_FMT_1], _prHwOsdBaseReg[OSD_FMT_2]);
}

void _OSD_AlwaysUpdateReg(unsigned int FMT_ID, BOOL fgEnable)
{
	UINT32 u4Val;

	if (fgEnable) {
		u4Val = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0);
		u4Val |= 0x02;
		IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0, u4Val);
	} else {
		u4Val = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0);
		u4Val &= (~0x02);
		IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0, u4Val);
	}
	IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0x70, 0x40);
}

INT32 _OSD_BASE_SetForceUnupdate(unsigned int FMT_ID, UINT32 u4Value)
{
	return _OSD_BASE_SetAlwaysUpdate(FMT_ID, u4Value);
}

void _OSD_UpdateReg(unsigned int FMT_ID, bool fgEnable)
{
	UINT32 u4Val;
	if (fgEnable) {
		u4Val = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0);
		u4Val |= 0x01;
		IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0, u4Val);
	} else {
		u4Val = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0);
		u4Val &= (~0x01);
		IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[FMT_ID], 0, u4Val);
	}
}

void _Osd_PustReset_Plane(UINT32 u4Plane)
{
	UINT32 u4Value;

	u4Value = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[u4Plane], 4);

	switch (u4Plane) {
	case OSD_PLANE_1:
		u4Value &= 0xFFFFFFFF;
		break;

	case OSD_PLANE_2:
		u4Value &= 0xFFFFFFFF;
		break;

	default:
		u4Value |= 0xFFFFFFFF;
		break;
	}

	IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[u4Plane], 4, u4Value);
}

void _Osd_ReleaseReset_Plane(UINT32 u4Plane)
{
	UINT32 u4Value;

	u4Value = IO_OSD_REG32((uintptr_t)_prHwOsdBaseReg[u4Plane], 4);
	u4Value &= ~OSD_RESET_PLANE_MASK;
	IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[u4Plane], 4, u4Value);
}

void _OSD_BASE_Reset(UINT32 u4Plane)
{
	UINT32 u4Val;

	u4Val = 0x03 << (u4Plane * 2 + 4);
	IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[u4Plane], 0x4, u4Val);
	IO_OSD_WRITE32((uintptr_t)_prHwOsdBaseReg[u4Plane], 0x4, 0);
}

INT32 _OSD_BASE_GetReg(unsigned int u4Plane, UINT32 *pOsdBaseReg)
{
	INT32 u4Idx = OSD_BASE_SKIP;

	if (pOsdBaseReg == NULL)
		return -(INT32) OSD_RET_INV_ARG;

	for (; u4Idx < OSD_BASE_REG_NUM; u4Idx++)
		pOsdBaseReg[u4Idx] = _rOsdBaseReg[u4Plane].au4Reg[u4Idx];

	return (INT32) OSD_RET_OK;
}

INT32 _OSD_BASE_SetReg(unsigned int u4Plane, const UINT32 *pOsdBaseReg)
{
	UINT32 u4Idx;

	if (pOsdBaseReg == NULL) {
		OSDDBG("_prHwOsdBaseReg[0]=0x%p\n", _prHwOsdBaseReg[u4Plane]);
		u4Idx = IO_OSD_REG32((uintptr_t) _prHwOsdBaseReg[u4Plane], 4);
#if !CONFIG_DRV_FAST_LOGO
		u4Idx |= OSD_RESET_PLANE_MASK;
		IO_OSD_WRITE32((uintptr_t) _prHwOsdBaseReg[u4Plane], 4, u4Idx);

		u4Idx &= (~OSD_RESET_PLANE_MASK);
		u4Idx |= 0x01F00000;
		IO_OSD_WRITE32((uintptr_t) _prHwOsdBaseReg[u4Plane], 4, u4Idx);

		IO_OSD_WRITE32((uintptr_t) _prHwOsdBaseReg[u4Plane], 0x38, 0x80008000);
		IO_OSD_WRITE32((uintptr_t) _prHwOsdBaseReg[u4Plane], 0x3c, 0x80008000);
#endif
		_prHwOsdBaseReg[u4Plane]->rField.fg_source_sync_select = 1;
		_prHwOsdBaseReg[u4Plane]->rField.fgAlwaysUpdate = 0;
		_prHwOsdBaseReg[u4Plane]->rField.fgUpdate = 1;


		for (u4Idx = OSD_BASE_SKIP; u4Idx < OSD_BASE_REG_NUM; u4Idx++)
			_rOsdBaseReg[u4Plane].au4Reg[u4Idx] = _prHwOsdBaseReg[u4Plane]->au4Reg[u4Idx];

		/*todo: get premix hardware register*/
		for (u4Idx = 0; u4Idx < OSD_PREMIX_REG_NUM; u4Idx++)
			_osd_premix_reg.au4Reg[u4Idx] = _pr_hw_osd_premix_reg->au4Reg[u4Idx];

		_rOsdBaseReg[u4Plane].au4Reg[1] &= (~OSD_RESET_PLANE_MASK);
	} else {
		for (u4Idx = OSD_BASE_SKIP; u4Idx < OSD_BASE_REG_NUM; u4Idx++)
			_rOsdBaseReg[u4Plane].au4Reg[u4Idx] = pOsdBaseReg[u4Idx];
	}

	return (INT32) OSD_RET_OK;
}

INT32 _OSD_BASE_UpdateHwReg(void)
{
	uint32_t i;

	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++)
		_OSD_BASE_SetUpdate(i, true);
	return (INT32) OSD_RET_OK;
}

INT32 _OSD_BASE_Update(uint32_t plane)
{
	UINT32 u4Update;
	UINT32 u4Idx = OSD_BASE_SKIP;

	_OSD_BASE_GetUpdate(plane, &u4Update);
	if (u4Update) {
		for (; u4Idx < OSD_BASE_COMMON_REG_NUM; u4Idx++) {
			_prHwOsdBaseReg[plane]->au4Reg[u4Idx] = _rOsdBaseReg[plane].au4Reg[u4Idx];
			OSD_PRINTF(OSD_WRITE_HW_LOG, "FMT_WRITE(%d + 'h%x, 32'h%x)\n", plane, (u4Idx*4),
				_rOsdBaseReg[plane].au4Reg[u4Idx]);
		}
		_OSD_BASE_SetUpdate(plane, false);
	}

	osd_base_get_start_update(plane, &u4Update);
	if (u4Update) {
		_prHwOsdBaseReg[plane]->au4Reg[4] = _rOsdBaseReg[plane].au4Reg[4];
		_prHwOsdBaseReg[plane]->au4Reg[5] = _rOsdBaseReg[plane].au4Reg[5];
		for (u4Idx = 9; u4Idx < 11; u4Idx++) {
			_prHwOsdBaseReg[plane]->au4Reg[u4Idx] = _rOsdBaseReg[plane].au4Reg[u4Idx];
			OSD_PRINTF(OSD_WRITE_HW_LOG, "FMT_WRITE(%d + 'h%x, 32'h%x)\n", plane, (u4Idx*4),
				_rOsdBaseReg[plane].au4Reg[u4Idx]);
		}
		osd_base_set_start_update(plane, false);
	}


	return (INT32) OSD_RET_OK;
}

INT32 OSD_PREMIX_Update(uint32_t plane)
{
	UINT32 u4Update;
	UINT32 u4Idx = 0;

	_OSD_BASE_GetPremixUpdate(plane, &u4Update);
	if (u4Update) {
		for (; u4Idx < OSD_PREMIX_REG_NUM; u4Idx++) {
			if ((u4Idx == 0xB) || (u4Idx == 0xC) || (u4Idx == 0xD))
				continue;

			_pr_hw_osd_premix_reg->au4Reg[u4Idx] = _osd_premix_reg.au4Reg[u4Idx];
			OSD_PRINTF(OSD_WRITE_HW_LOG, "PREMIX_WRITE(%d + 'h%x, 32'h%x)\n", plane, (u4Idx*4),
				_pr_hw_osd_premix_reg->au4Reg[u4Idx]);
		}
		_OSD_BASE_SetPremixUpdate(plane, false);
	}

	osd_pmx_get_window_update(plane, &u4Update);
	if (u4Update) {
		for (u4Idx = 3; u4Idx < 8; u4Idx++) {
			_pr_hw_osd_premix_reg->au4Reg[u4Idx] = _osd_premix_reg.au4Reg[u4Idx];
			OSD_PRINTF(OSD_WRITE_HW_LOG, "PREMIX_WRITE(%d + 'h%x, 32'h%x)\n", plane, (u4Idx*4),
				_pr_hw_osd_premix_reg->au4Reg[u4Idx]);
		}
		osd_pmx_set_window_update(plane, false);
	}

	return (INT32) OSD_RET_OK;
}

INT32 OSD_PREMIX_Update_vdo4(void)
{
	/*0x14001C38*/
	_pr_hw_osd_premix_reg->au4Reg[0x7] = _osd_premix_reg.au4Reg[0x7];
	_pr_hw_osd_premix_reg->au4Reg[0x8] = _osd_premix_reg.au4Reg[0x8];
	_pr_hw_osd_premix_reg->au4Reg[0xE] = _osd_premix_reg.au4Reg[0xE];
	OSDDBG("14001C38=0x%x\n", _osd_premix_reg.au4Reg[0xE]);
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetUpdate(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgUpdate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetUpdate(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgUpdate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetPlaneUpdate(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgPlaneUpdate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetPlaneUpdate(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgPlaneUpdate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetSclUpdate(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgSclUpdate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetSclUpdate(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgSclUpdate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetPremixUpdate(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgPremixUpdate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetPremixUpdate(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgPremixUpdate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetPlaneReflip(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgPlaneReflip = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetPlaneReflip(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgPlaneReflip;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetScalerReCfg(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgScalerReCfg = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScalerReCfg(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgScalerReCfg;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetAlwaysUpdate(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgAlwaysUpdate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetAlwaysUpdate(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _prHwOsdBaseReg[u4Plane]->rField.fgAlwaysUpdate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetResetMainPath(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgRstMainFmt = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetResetMainPath(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgRstMainFmt;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetResetOsd1(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgRstOsd1 = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetResetOsd1(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgRstOsd1;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetResetOsd2(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgRstOsd2 = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetResetOsd2(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgRstOsd2;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetIOMonSel(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4IOMonSel = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetIOMonSel(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4IOMonSel;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetAlphaSel(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4AlphaSel = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetAlphaSel(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4AlphaSel;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetSRamType(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4SRamType = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetSRamType(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4SRamType;
	return (INT32) OSD_RET_OK;
}

/* 08h OSD mode configuration register */
inline INT32 _OSD_BASE_SetHsEdge(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgHsEdge = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetHsEdge(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgHsEdge;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetVsEdge(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgVsEdge = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetVsEdge(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgVsEdge;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetFldPol(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgFldPol = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetFldPol(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgFldPol;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd3Prgs(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgOsd3Prgs = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd3Prgs(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgOsd3Prgs;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd2Prgs(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgOsd2Prgs = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2Prgs(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgOsd2Prgs;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd1Path(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgOsd1Aux = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd1Path(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgOsd1Aux;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd2Path(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgOsd2Aux = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2Path(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgOsd2Aux;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd1Dotctl(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd1Dotctl = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd1Dotctl(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd1Dotctl;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd2Dotctl(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd2Dotctl = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2Dotctl(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd2Dotctl;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetAutoSwEn(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgAutoSwEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetAutoSwEn(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgAutoSwEn;
	return (INT32) OSD_RET_OK;
}

/* 0Ch Main FMT Vsync Timing Configuration Register */
inline INT32 _OSD_BASE_SetOvtMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4OvtMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOvtMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4OvtMain;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetVsWidthMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4VsWidthMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetVsWidthMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4VsWidthMain;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetHsWidthMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4HsWidthMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetHsWidthMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4HsWidthMain;
	return (INT32) OSD_RET_OK;
}

/* 10h FMT H-Timing Configuration Register#1 */
inline INT32 _OSD_BASE_SetScrnHStartOsd2(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnHStartOsd2 = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnHStartOsd2(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnHStartOsd2;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetScrnHStartOsd3(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnHStartOsd3 = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnHStartOsd3(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnHStartOsd3;
	return (INT32) OSD_RET_OK;
}

/* 18h Main FMT V-Timing Configuration Register #1 */
inline INT32 _OSD_BASE_SetScrnVStartBotMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnVStartBotMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnVStartBotMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnVStartBotMain;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetScrnVStartTopMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnVStartTopMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnVStartTopMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnVStartTopMain;
	return (INT32) OSD_RET_OK;
}

/* 1Ch Main FMT Timing Configuration Register #2 */
inline INT32 _OSD_BASE_SetScrnVSizeMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnVSizeMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnVSizeMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnVSizeMain;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetScrnHSizeMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4ScrnHSizeMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetScrnHSizeMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4ScrnHSizeMain;
	return (INT32) OSD_RET_OK;
}

/* 20h OSD1 Window Position Configuration Register */
inline INT32 _OSD_BASE_SetOsd3VStart(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd3VStart = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd3VStart(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd3VStart;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd3HStart(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd3HStart = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd3HStart(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd3HStart;
	return (INT32) OSD_RET_OK;
}

/* 24h OSD2 Window Position Configuration Register  */
inline INT32 _OSD_BASE_SetOsd2VStart(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd2VStart = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2VStart(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd2VStart;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsd2HStart(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4Osd2HStart = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2HStart(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4Osd2HStart;
	return (INT32) OSD_RET_OK;
}

/* DWORD - 064 OSD_FMT_64 */
inline INT32 _OSD_BASE_SetOhtMain(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.u4OhtMain = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOhtMain(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.u4OhtMain;
	return (INT32) OSD_RET_OK;
}

/* DWORD - 088 OSD_FMT_70 */
inline INT32 _OSD_BASE_SetIntTGen(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgIntTGen = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetIntTGen(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgIntTGen;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsdShareSram(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgOsdShareSram = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsdShareSram(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.fgOsdShareSram;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_SetOsdChkSumEn(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.fgCheckSumEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd3ChkSum(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _prHwOsdBaseReg[u4Plane]->rField.u4Osd3CheckSum;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsdSc3ChkSum(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _prHwOsdBaseReg[u4Plane]->rField.u4Osd3ScCheckSum;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsd2ChkSum(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _prHwOsdBaseReg[u4Plane]->rField.u4Osd2CheckSum;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_BASE_GetOsdSc2ChkSum(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _prHwOsdBaseReg[u4Plane]->rField.u4Osd2ScCheckSum;
	return (INT32) OSD_RET_OK;
}

inline void osd_base_set_start_update(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.osd_start_update = u4Value;
}

inline int osd_base_get_start_update(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.osd_start_update;
	return (INT32) OSD_RET_OK;
}

inline void osd_pmx_set_window_update(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.osd_window_update = u4Value;
}

inline int osd_pmx_get_window_update(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.osd_window_update;
	return (INT32) OSD_RET_OK;
}

inline void osd_plane_set_extend_update(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.osd_plane_update = u4Value;
}

inline int osd_plane_get_extend_update(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.osd_plane_update;
	return (INT32) OSD_RET_OK;
}

inline void osd_plane_set_sync_threshold_update(unsigned int u4Plane, UINT32 u4Value)
{
	_rOsdBaseReg[u4Plane].rField.osd_sync_threshold_update = u4Value;
}

inline int osd_plane_get_sync_threshold_update(unsigned int u4Plane, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdBaseReg[u4Plane].rField.osd_sync_threshold_update;
	return (INT32) OSD_RET_OK;
}


inline void osd_pmx_set_rg_htotal(uint32_t value)
{
	_osd_premix_reg.rField.rg_htotal = value;
}

inline void osd_pmx_set_rg_vtotal(uint32_t value)
{
	_osd_premix_reg.rField.rg_vtotal = value;
}

inline void osd_pmx_set_rg_win_hend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win_hend = value;
}

inline void osd_pmx_set_rg_win_hst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win_hst = value;
}

inline void osd_pmx_set_rg_win_vend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win_vend = value;
}

inline void osd_pmx_set_rg_win_vst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win_vst = value;
}

inline void osd_pmx_set_rg_win1_hend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win1_hend = value;
}

inline void osd_pmx_set_rg_win1_hst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win1_hst = value;
}

inline void osd_pmx_set_rg_win1_vend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win1_vend = value;
}

inline void osd_pmx_set_rg_win1_vst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win1_vst = value;
}

inline void osd_pmx_set_rg_win2_hend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win2_hend = value;
}

inline void osd_pmx_set_rg_win2_hst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win2_hst = value;
}

inline void osd_pmx_set_rg_win2_vend(uint32_t value)
{
	_osd_premix_reg.rField.rg_win2_vend = value;
}

inline void osd_pmx_set_rg_win2_vst(uint32_t value)
{
	_osd_premix_reg.rField.rg_win2_vst = value;
}

inline void osd_pmx_set_rg_pln2_max_a_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_pln2_max_a_sel = value;
}

inline void osd_pmx_set_rg_pln1_max_a_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_pln1_max_a_sel = value;
}

inline void osd_pmx_set_rg_pln0_max_a_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_pln0_max_a_sel = value;
}

inline void osd_pmx_set_rg_vs_pol_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_vs_pol_sel = value;
}

inline void osd_pmx_set_shadow_update(uint32_t value)
{
	_osd_premix_reg.rField.shadow_update = value;
}

inline void osd_pmx_set_shadow_en(uint32_t value)
{
	_osd_premix_reg.rField.shadow_en = value;
}

inline void osd_pmx_set_rg_win2_en(uint32_t value)
{
	_osd_premix_reg.rField.rg_win2_en = value;
}

inline void osd_pmx_set_rg_win1_en(uint32_t value)
{
	_osd_premix_reg.rField.rg_win1_en = value;
}

inline void osd_pmx_set_rg_win0_en(uint32_t value)
{
	_osd_premix_reg.rField.rg_win0_en = value;
}

inline void osd_pmx_set_rg_alpha_val(uint32_t value)
{
	_osd_premix_reg.rField.rg_alpha_val = value;
}

inline void osd_pmx_set_rg_dbg_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_dbg_sel = value;
}

inline void osd_pmx_set_rg_max_a_sel(uint32_t value)
{
	_osd_premix_reg.rField.rg_max_a_sel = value;
}

inline void osd_pmx_set_rg_max_a_sel_div(uint32_t value)
{
	_osd_premix_reg.rField.rg_max_a_sel_div = value;
}

inline void osd_pmx_set_rg_premul(uint32_t value)
{
	_osd_premix_reg.rField.rg_premul = value;
}

inline void osd_pmx_set_plane2_cr_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane2_cr_sel = value;
}

inline void osd_pmx_set_plane2_cb_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane2_cb_sel = value;
}

inline void osd_pmx_set_plane2_y_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane2_y_sel = value;
}

inline void osd_pmx_set_swap_2(uint32_t value)
{
	_osd_premix_reg.rField.swap_2 = value;
}

inline void osd_pmx_set_plane1_cr_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane1_cr_sel = value;
}

inline void osd_pmx_set_plane1_cb_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane1_cb_sel = value;
}

inline void osd_pmx_set_plane1_y_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane1_y_sel = value;
}

inline void osd_pmx_set_swap_1(uint32_t value)
{
	_osd_premix_reg.rField.swap_1 = value;
}

inline void osd_pmx_set_plane0_cr_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane0_cr_sel = value;
}

inline void osd_pmx_set_plane0_cb_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane0_cb_sel = value;
}

inline void osd_pmx_set_plane0_y_sel(uint32_t value)
{
	_osd_premix_reg.rField.plane0_y_sel = value;
}

inline void osd_pmx_set_swap_0(uint32_t value)
{
	_osd_premix_reg.rField.swap_0 = value;
}

inline void osd_pmx_set_mix1_premul_set_rgb(uint32_t value)
{
	_osd_premix_reg.rField.mix1_premul_set_rgb = value;
}

inline void osd_pmx_set_mix1_sign_en(uint32_t value)
{
	_osd_premix_reg.rField.mix1_sign_en = value;
}

inline void osd_pmx_set_mix1_bypass(uint32_t value)
{
	_osd_premix_reg.rField.mix1_bypass = value;
}

inline void osd_pmx_set_mix1_bot_mask(uint32_t value)
{
	_osd_premix_reg.rField.mix1_bot_mask = value;
}

inline void osd_pmx_set_mix1_top_mask(uint32_t value)
{
	_osd_premix_reg.rField.mix1_top_mask = value;
}

inline void osd_pmx_set_mix1_top_mix_bg(uint32_t value)
{
	_osd_premix_reg.rField.mix1_top_mix_bg = value;
}

inline void osd_pmx_set_mix1_pre_mul_a(uint32_t value)
{
	_osd_premix_reg.rField.mix1_pre_mul_a = value;
}

inline void osd_pmx_set_mix1_de_sel(uint32_t value)
{
	_osd_premix_reg.rField.mix1_de_sel = value;
}

inline void osd_pmx_set_mix2_premul_set_rgb(uint32_t value)
{
	_osd_premix_reg.rField.mix2_premul_set_rgb = value;
}

inline void osd_pmx_set_mix2_sign_en(uint32_t value)
{
	_osd_premix_reg.rField.mix2_sign_en = value;
}

inline void osd_pmx_set_mix2_bypass(uint32_t value)
{
	_osd_premix_reg.rField.mix2_bypass = value;
}

inline void osd_pmx_set_mix2_bot_mask(uint32_t value)
{
	_osd_premix_reg.rField.mix2_bot_mask = value;
}

inline void osd_pmx_set_mix2_top_mask(uint32_t value)
{
	_osd_premix_reg.rField.mix2_top_mask = value;
}

inline void osd_pmx_set_mix2_top_mix_bg(uint32_t value)
{
	_osd_premix_reg.rField.mix2_top_mix_bg = value;
}

inline void osd_pmx_set_mix2_pre_mul_a(uint32_t value)
{
	_osd_premix_reg.rField.mix2_pre_mul_a = value;
}

inline void osd_pmx_set_mix2_de_sel(uint32_t value)
{
	_osd_premix_reg.rField.mix2_de_sel = value;
}

inline void osd_pmx_set_pln0_msk(uint32_t value)
{
	_osd_premix_reg.rField.rg_pln0_mask = value;
	_osd_premix_reg.rField.rg_b_y_0_2 = 0x0;
	_osd_premix_reg.rField.rg_b_y_3_5 = 0x0;
	_osd_premix_reg.rField.rg_b_y_6_7 = 0x0;
	_osd_premix_reg.rField.rg_b_cb_0_2 = 0x0;
	_osd_premix_reg.rField.rg_b_cb_3_5 = 0x0;
	_osd_premix_reg.rField.rg_b_cb_6_7 = 0x0;
	_osd_premix_reg.rField.rg_b_cr_0_2 = 0x0;
	_osd_premix_reg.rField.rg_b_cr_3_5 = 0x0;
	_osd_premix_reg.rField.rg_b_cr_6_7 = 0x0;
}

inline void osd_pmx_set_vdo4_alpha(uint32_t value)
{
	_osd_premix_reg.rField.vdo4_cfg_alpha = value;
}
