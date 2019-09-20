/****************************************************************
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
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED
 * IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 *******************************************************/
#define LOG_TAG "osd_plane"

#include <linux/types.h>
#include "osd_hw.h"
#include "disp_osd_log.h"

/* software register */
OSD_PLA_CORE_UNION_T _rOsdPlaneCoreReg[OSD_PLANE_MAX_NUM];
/* hardware register map */
static volatile OSD_PLA_CORE_UNION_T *_prHwOsdPlaneCoreReg[OSD_PLANE_MAX_NUM];

struct osd_fading_ratio_para fading_ratio[MAX_OSD_INPUT_CONFIG];
struct osd_fading_ratio_para fading_ratio_shadow[MAX_OSD_INPUT_CONFIG];

void _OSD_PLA_GET_REG_BASE(uintptr_t *osd_pln_base_reg)
{
	_prHwOsdPlaneCoreReg[OSD_PLANE_1] = (OSD_PLA_CORE_UNION_T *) osd_pln_base_reg[0];
	_prHwOsdPlaneCoreReg[OSD_PLANE_2] = (OSD_PLA_CORE_UNION_T *) osd_pln_base_reg[1];
}

INT32 _OSD_PLA_GetReg(UINT32 u4Plane, UINT32 *pOsdPlaneReg)
{
	UINT32 u4Idx = 0;

	OSD_VERIFY_PLANE(u4Plane);
	if (pOsdPlaneReg == NULL)
		return -(INT32) OSD_RET_INV_ARG;

	for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++)
		pOsdPlaneReg[u4Idx] = _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];

	return (INT32) OSD_RET_OK;
}

INT32 _OSD_PLA_SetReg(UINT32 u4Plane, const UINT32 *pOsdPlaneReg)
{
	UINT32 u4Idx = 0;

	OSD_VERIFY_PLANE(u4Plane);
	if (pOsdPlaneReg == NULL)
		for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++)
			_rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx] =
			    _prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx];
	else
		for (; u4Idx < OSD_CORE_REG_NUM; u4Idx++)
			_rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx] = pOsdPlaneReg[u4Idx];

	return (INT32) OSD_RET_OK;
}

INT32 _OSD_PLA_SetUpdateStatus(UINT32 u4Plane, UINT32 u4Update)
{
	UINT32 u4PlaneStatus;

	_OSD_BASE_GetPlaneUpdate(u4Plane, &u4PlaneStatus);
	u4PlaneStatus = u4Update ? (u4PlaneStatus | (1 << u4Plane)) :
	    (u4PlaneStatus & ~(1 << u4Plane));
	_OSD_BASE_SetPlaneUpdate(u4Plane, u4PlaneStatus);
	return (INT32) OSD_RET_OK;
}

INT32 _OSD_PLA_GetUpdateStatus(UINT32 u4Plane)
{
	UINT32 u4PlaneStatus;

	_OSD_BASE_GetPlaneUpdate(u4Plane, &u4PlaneStatus);
	return ((u4PlaneStatus & (1 << u4Plane)) >> u4Plane);
}

INT32 _OSD_PLA_SetReflip(UINT32 u4Plane, BOOL u4Update)
{
	UINT32 u4PlaneStatus;

	_OSD_BASE_GetPlaneReflip(u4Plane, &u4PlaneStatus);
	u4PlaneStatus = u4Update ? (u4PlaneStatus | (1 << u4Plane)) :
	    (u4PlaneStatus & ~(1 << u4Plane));
	_OSD_BASE_SetPlaneReflip(u4Plane, u4PlaneStatus);
	return (INT32) OSD_RET_OK;
}

INT32 _OSD_PLA_GetReflip(UINT32 u4Plane)
{
	UINT32 u4PlaneStatus;

	_OSD_BASE_GetPlaneReflip(u4Plane, &u4PlaneStatus);
	return ((u4PlaneStatus & (1 << u4Plane)) >> u4Plane);
}

INT32 _OSD_PLA_UpdateHwReg(UINT32 u4Plane)
{

	_OSD_PLA_SetUpdateStatus(u4Plane, true);

#if 0				/*osd update shadow register in active area */
	UINT32 u4Idx;

	for (u4Idx = 0; u4Idx < OSD_CORE_REG_NUM; u4Idx++) {
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx] =
		    _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
		if (u4Idx < 8)
			OSD_PRINTF(OSD_WRITE_HW_LOG, "_OSD_PLA_Set u4Value =0x%x\n ",
				   _prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx]);
	}
	_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x39] = _rOsdPlaneCoreReg[u4Plane].au4Reg[0x39];
#endif
	return (INT32) OSD_RET_OK;
}

INT32 osd_pla_non_shadow_update(UINT32 u4Plane)
{
	if ((fading_ratio[u4Plane].fg_fading_ratio_update ||
		fading_ratio_shadow[u4Plane].fg_fading_ratio_update)) {
		if (fading_ratio[u4Plane].header_addr ==
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x1]) {
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x2] =
				fading_ratio[u4Plane].fading_ratio_alpha;

			fading_ratio[u4Plane].fg_fading_ratio_update = false;
		} else if (fading_ratio_shadow[u4Plane].header_addr ==
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x1]) {
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x2] =
				fading_ratio_shadow[u4Plane].fading_ratio_alpha;

			fading_ratio_shadow[u4Plane].fg_fading_ratio_update = false;
			fading_ratio[u4Plane].fg_fading_ratio_update = false;
		} else {
			OSD_LOG_E("fading ratio not match[%d] %x,%x(%x %x)\n", u4Plane,
				fading_ratio_shadow[u4Plane].fading_ratio_alpha,
				fading_ratio[u4Plane].fading_ratio_alpha,
				_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x1],
				fading_ratio[u4Plane].header_addr);
		}
	}

	return (INT32) OSD_RET_OK;
}


INT32 _OSD_PLA_Update(UINT32 u4Plane)
{
	UINT32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < OSD_CORE_COMMON_REG_NUM; u4Idx++) {
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx] =
		    _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
		OSD_PRINTF(OSD_WRITE_HW_LOG, "PLA_WRITE(%d + 'h%x, 32'h%x)\n", u4Plane, (u4Idx*4),
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx]);
	}


	if (_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x2] !=
		    _rOsdPlaneCoreReg[u4Plane].au4Reg[0x2])
	{
		/* OSD_LOG_I("update shadow:%x\n", _rOsdPlaneCoreReg[u4Plane].au4Reg[0x2]); */
		if (fading_ratio[u4Plane].fg_fading_ratio_update == true &&
			fading_ratio_shadow[u4Plane].fg_fading_ratio_update == false) {
			fading_ratio_shadow[u4Plane].header_addr = _rOsdPlaneCoreReg[u4Plane].au4Reg[0x1];
			fading_ratio_shadow[u4Plane].fading_ratio_alpha =	_rOsdPlaneCoreReg[u4Plane].au4Reg[0x2];
			fading_ratio_shadow[u4Plane].fg_fading_ratio_update = true;
		} else if (fading_ratio[u4Plane].fg_fading_ratio_update == false) {
			fading_ratio[u4Plane].header_addr = _rOsdPlaneCoreReg[u4Plane].au4Reg[0x1];
			fading_ratio[u4Plane].fading_ratio_alpha =	_rOsdPlaneCoreReg[u4Plane].au4Reg[0x2];
			fading_ratio[u4Plane].fg_fading_ratio_update = true;
		} else {
			OSD_LOG_E("osd update 3 frame 1 vsync\n");
			fading_ratio[u4Plane].fg_fading_ratio_update = false;
			fading_ratio_shadow[u4Plane].fg_fading_ratio_update = false;
		}
	} else {
		fading_ratio_shadow[u4Plane].fg_fading_ratio_update = false;
		fading_ratio[u4Plane].fg_fading_ratio_update = false;
	}

	OSD_PRINTF(OSD_WRITE_HW_LOG, "PLA_WRITE(%d + 'h8, 32'h%x)\n", u4Plane,
		_rOsdPlaneCoreReg[u4Plane].au4Reg[0x2]);

	return (INT32) OSD_RET_OK;
}

INT32 osd_plane_update_extend(UINT32 u4Plane)
{
	UINT32 u4Idx = 0;

	/*include burst 8 enable*/
	_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[0x3] =
		_rOsdPlaneCoreReg[u4Plane].au4Reg[0x3];

	for (u4Idx = 0x2B; u4Idx < 0x30; u4Idx++) {
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx] =
		    _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
		OSD_PRINTF(OSD_WRITE_HW_LOG, "PLA_WRITE(%d + 'h%x, 32'h%x)\n", u4Plane, (u4Idx*4),
			_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx]);
	}
	return (INT32) OSD_RET_OK;
}

INT32 osd_plane_update_sync_threshold(UINT32 u4Plane)
{
	UINT32 u4Idx = 0x39;

	_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx] =
	    _rOsdPlaneCoreReg[u4Plane].au4Reg[u4Idx];
	OSD_PRINTF(OSD_WRITE_HW_LOG, "PLA_WRITE(%d + 'h%x, 32'h%x)\n", u4Plane, (u4Idx*4),
		_prHwOsdPlaneCoreReg[u4Plane]->au4Reg[u4Idx]);
	return (INT32) OSD_RET_OK;
}


inline INT32 _OSD_PLA_SetEnable(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgOsdEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetEnable(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgOsdEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetFakeHdr(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgFakeHdr = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetFakeHdr(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgFakeHdr;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetPrngEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgPrngEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetPrngEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgPrngEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetOutRngColorMode(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgOutRngColorMode = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetOutRngColorMode(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgOutRngColorMode;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetHeaderAddr(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4HeaderAddr = (u4Value == 0) ? 0 : (u4Value) >> 4;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetHeaderAddr(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = (UINT32) VIRTUAL(_rOsdPlaneCoreReg[u4Plane].rField.u4HeaderAddr << 4);
	return (INT32) OSD_RET_OK;
}


inline INT32 _OSD_PLA_SetBlending(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4GobalBlending = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetBlending(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4GobalBlending;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetFading(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4FadingRatio = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetFading(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4FadingRatio;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetHFilter(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgHFilter = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetHFilter(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgHFilter;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetColorExpSel(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgColorExpSel = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetColorExpSel(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgColorExpSel;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetAlphaRatioEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgAlphaRatioEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetAlphaRatioEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgAlphaRatioEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetContReqLmt(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4ContReqLmt = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetContReqLmt(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4ContReqLmt;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetFifoSize(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4FifoSize = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetFifoSize(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4FifoSize;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetPauseCnt(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4PauseCnt = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetPauseCnt(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4PauseCnt;
	return (INT32) OSD_RET_OK;
}


inline INT32 _OSD_PLA_SetContReqLmt0(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4ContReqLmt0 = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetContReqLmt0(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4ContReqLmt0;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetBurstDis(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgBurstDis = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetBurstDis(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgBurstDis;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetRgbMode(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgRgbMode = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetRgbMode(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgRgbMode;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetVacancyThr(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4VacancyThr = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetVacancyThr(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4VacancyThr;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetPreMultiMode(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4PreMultiMode = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetPreMultiMode(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4PreMultiMode;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetHMirrorEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgHMirrorEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetHMirrorEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgHMirrorEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetVFlipEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgVFlipEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetVFlipEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgVFlipEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetRgb2YcbrbEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgRgb2YcbrbEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetRgb2YcbrbEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgRgb2YcbrbEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetXVYCCEn(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgXVYCCEn = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetXVYCCEn(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgXVYCCEn;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetYCbCr709En(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgYCbCr709En = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetYCbCr709En(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgYCbCr709En;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetNonAlphaShift(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.fgNonAlphaShift = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetNonAlphaShift(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.fgNonAlphaShift;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetAlphaCoordinate(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4AlphaCoordinate = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetAlphaCoordinate(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4AlphaCoordinate;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetEableRGB2YCC(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.u4EableRGB2YCC = u4Value;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_GetEableRGB2YCC(UINT32 u4Plane, UINT32 *pu4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;
	*pu4Value = _rOsdPlaneCoreReg[u4Plane].rField.u4EableRGB2YCC;
	return (INT32) OSD_RET_OK;
}

inline INT32 _OSD_PLA_SetBurst8_Enable(UINT32 u4Plane, UINT32 u4Value)
{
	OSD_VERIFY_PLANE(u4Plane);
	_rOsdPlaneCoreReg[u4Plane].rField.burst_8_enable = u4Value;
	return (INT32) OSD_RET_OK;
}

inline int osd_pla_set_alpha_detect_en(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.rg_alpha_det_en = u4Value;
	return (INT32) OSD_RET_OK;
}

inline int osd_pla_set_alpha_detect_clear(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.rg_alpha_det_clear = u4Value;
	return (INT32) OSD_RET_OK;
}

inline int osd_pla_set_alpha_detect_threshold(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.alpha_threshold = u4Value;
	return (INT32) OSD_RET_OK;
}

inline int osd_pla_set_region_width(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.region_width = u4Value;
	return (INT32) OSD_RET_OK;
}

inline int osd_pla_get_alpha_region(UINT32 u4Plane, UINT32 *pu4Value)
{
	*pu4Value = _prHwOsdPlaneCoreReg[u4Plane]->rField.alpha_region_flag;
	return (INT32) OSD_RET_OK;
}

int osd_pla_get_osd_en_effect(uint32_t layer_id)
{
	return _prHwOsdPlaneCoreReg[layer_id]->rField.fgOsdEnEffect;
}

int osd_pla_get_line_cnt(uint32_t layer_id)
{
	if (_prHwOsdPlaneCoreReg[layer_id]->rField.fgOsdEnEffect)
		return _prHwOsdPlaneCoreReg[layer_id]->rField.u4line_cnt;
	else
		return -1;
}

int osd_pla_set_sync_threshold(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.u4SyncThres = u4Value;
	return (INT32) OSD_RET_OK;
}

int osd_pla_set_auto_ultra(UINT32 u4Plane, UINT32 u4Value)
{
	_rOsdPlaneCoreReg[u4Plane].rField.auto_ultra = u4Value;
	return (INT32) OSD_RET_OK;
}
