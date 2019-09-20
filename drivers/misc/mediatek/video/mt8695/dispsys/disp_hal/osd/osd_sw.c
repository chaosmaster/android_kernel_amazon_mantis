/********************************************************************************************
 *	 LEGAL DISCLAIMER
 *
 *	 (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *	 BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *	 THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *	 FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *	 ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *	 INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *	 A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *	 WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *	 INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *	 ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *	 NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *	 OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *	 BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *	 RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *	 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *	 FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *	 THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *	 OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "osd_sw"

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include "disp_osd_if.h"
#include "osd_sw.h"
#include "osd_hw.h"
#include "disp_osd_log.h"
#include "disp_osd_env.h"
#include "disp_osd_fence.h"
#include "disp_path.h"

#define OSD_COLORSPACE_601			   0
#define OSD_COLORSPACE_709			   1


#ifndef MAX
#define MAX(x, y)   (((x) >= (y)) ? (x) : (y))
#endif

static BOOL _fgOsdYCbCr709 = true;
/* region config */
OSD_REGION_NODE_T _rAllRgnNode[OSD_RGN_REG_NUM];
/* plane config */
BOOL _afgPlaneEnableStatus[OSD_PLANE_MAX_NUM];
/* scaler config */
static OSD_SCALE_INFO _arOsdScaleInfo[OSD_SCALER_MAX_NUM];
BOOL _afgCutEdge[OSD_SCALER_MAX_NUM];
INT32 _ai4OsdPosOfst[OSD_PLANE_MAX_NUM];

void osd_region_init(void)
{
	INT32 i4Index;

	for (i4Index = 0; i4Index < OSD_RGN_REG_NUM; i4Index++)
		memset(&_rAllRgnNode, 0, sizeof(OSD_REGION_NODE_T));

	_osd_region_init();
}

INT32 osd_get_free_region(UINT32 u4Plane, UINT32 *u4region)
{
	UINT32 u4Index;
	UINT32 u4MaxRgnByPlane;
	UINT32 u4RgnPlane;

	u4RgnPlane = osd_plane_map_region(u4Plane);
	u4Index = u4RgnPlane * OSD_RGN_REG_LIST;
	u4MaxRgnByPlane = OSD_MAX_NUM_RGN_LIST + u4Index;

	if (u4MaxRgnByPlane >= OSD_RGN_REG_NUM)
		return -OSD_RET_INV_ARG;

	for (; u4Index < u4MaxRgnByPlane; u4Index++) {
		if (_rAllRgnNode[u4Index].eOsdRgnState == OSD_RGN_STATE_IDLE) {
			*u4region = u4Index;
			_rAllRgnNode[u4Index].eOsdRgnState = OSD_RGN_STATE_ACTIVE;
			return OSD_RET_OK;
		}
	}
	return -OSD_RET_INV_ARG;
}

void osd_set_region_state(UINT32 u4Plane)
{
	UINT32 u4Index;
	UINT32 u4MaxRgnByPlane;
	UINT32 u4RgnPlane;

	u4RgnPlane = osd_plane_map_region(u4Plane);
	u4Index = u4RgnPlane * OSD_RGN_REG_LIST;
	u4MaxRgnByPlane = OSD_MAX_NUM_RGN_LIST + u4Index;

	OSD_PRINTF(OSD_CONFIG_SW_LOG,
		   "[Osd]osd_set_region_state u4Plane = %d,u4RgnPlane=%d ,u4Index = %d\n",
		   u4Plane, u4RgnPlane, u4Index);
	for (; u4Index < u4MaxRgnByPlane; u4Index++) {
		if (_rAllRgnNode[u4Index].eOsdRgnState == OSD_RGN_STATE_ACTIVE) {
			_rAllRgnNode[u4Index].eOsdRgnState = OSD_RGN_STATE_USED;
			OSD_PRINTF(OSD_CONFIG_SW_LOG,
				   "[Osd]OSD_RGN_STATE_USED pu4Addr = 0x%lx ,u4Index = %d\n",
				   _rAllRgnNode[u4Index].pu4_vAddr, u4Index);
		} else if (_rAllRgnNode[u4Index].eOsdRgnState == OSD_RGN_STATE_USED) {
			_rAllRgnNode[u4Index].eOsdRgnState = OSD_RGN_STATE_READ_DONE;
			OSD_PRINTF(OSD_CONFIG_SW_LOG,
				   "[Osd]OSD_RGN_STATE_READ_DONE pu4Addr = 0x%lx ,u4Index = %d\n",
				   _rAllRgnNode[u4Index].pu4_vAddr, u4Index);
		} else {
			_rAllRgnNode[u4Index].eOsdRgnState = OSD_RGN_STATE_IDLE;
			OSD_PRINTF(OSD_CONFIG_SW_LOG,
				   "[Osd]OSD_RGN_STATE_IDLE pu4Addr = 0x%lx ,u4Index = %d\n",
				   _rAllRgnNode[u4Index].pu4_vAddr, u4Index);
		}
	}

}

static int osd_base_reconfig_all(uint32_t plane, uint32_t path, HDMI_VIDEO_RESOLUTION osd_res_mode)
{
	unsigned int osd_hstart, osd_veven, osd_vodd;
	unsigned int mix_layer_id;
	/* adjust osd screen display postion when change OSD path */
	if (plane == OSD_PLANE_1)
		mix_layer_id = DISP_PATH_OSD1;/*UHD*/
	else
		mix_layer_id = DISP_PATH_OSD2;/*FHD*/

	/* disp_path_set_delay(mix_layer_id, osd_res_mode); */
	/* re-set osd vsync pulse after osd start again with sdr2hdr */
	disp_path_set_delay_by_shift(mix_layer_id, osd_res_mode, true, 26, false, 0);
	disp_path_get_active_zone(mix_layer_id, osd_res_mode, &osd_hstart,
		&osd_vodd, &osd_veven);

	if (plane == OSD_PLANE_1)
		_OSD_BASE_SetScrnHStartOsd2(plane, osd_hstart);
	else
		_OSD_BASE_SetScrnHStartOsd3(plane, osd_hstart);

	_OSD_BASE_SetScrnVStartBotMain(plane, osd_veven);
	_OSD_BASE_SetScrnVStartTopMain(plane, osd_vodd);/*odd is top*/

	_OSD_BASE_SetVsEdge(plane, 1);
	_OSD_BASE_SetHsEdge(plane, 0);

	_OSD_BASE_SetUpdate(plane, true);

	return OSD_RET_OK;
}


static int osd_get_scrn_size(struct osd_resolution_change *res_chg,
			     UINT32 u4Path, UINT32 *pu4HSize, UINT32 *pu4VSize)
{
	bool fgInterlace = 0;

	*pu4HSize = res_chg->width;
	*pu4VSize = res_chg->height;
	fgInterlace = !res_chg->is_progressive;

	if (!(res_chg->is_hd) && fgInterlace)	/* sd interlace */
		*pu4HSize <<= 1;

	return OSD_RET_OK;
}

static int osd_get_res_total_size(struct osd_resolution_change *res_chg,
				  UINT32 u4Path, UINT32 *pu4HTotal, UINT32 *pu4VTotal)
{

	bool fgInterlace = 0;

	*pu4HTotal = res_chg->htotal;
	*pu4VTotal = res_chg->vtotal;
	fgInterlace = !res_chg->is_progressive;

	if (!(res_chg->is_hd) && fgInterlace)	/* sd interlace */
		*pu4HTotal <<= 1;

	return OSD_RET_OK;
}

#if 0
static int osd_sc_reconfig_all(uint32_t plane, struct osd_resolution_change *osd_res)
{
	UINT32 enable, src_w, src_h, dst_w, dst_h;

	OSDDBG("osd_sc_reconfig_all\n");
	osd_get_scrn_size(osd_res, OSD_MAIN_PATH, &dst_w, &dst_h);
	enable = _arOsdScaleInfo[plane].u4Enable;
	src_w = _arOsdScaleInfo[plane].u4SrcW;
	src_h = _arOsdScaleInfo[plane].u4SrcH;

	_arOsdScaleInfo[plane].u4DispMode = (uint32_t) osd_res->osd_res_mode;

	if ((src_w == 0) || (src_h == 0)) {
		OSDDBG("osd_sc: invalid src\n");
		_OSD_SC_GetSrcHSize(plane, &src_w);
		_OSD_SC_GetSrcVSize(plane, &src_h);
		_OSD_SC_GetScEn(plane, &enable);
	}

	OSD_SC_Scale(plane, enable, src_w, src_h, 0, 0, dst_w, dst_h);
	return (INT32) OSD_RET_OK;
}
#endif

INT32 osd_reset_plane_region_state(UINT32 u4Plane)
{
	UINT32 u4Index;
	UINT32 u4MaxRgnByPlane;
	UINT32 u4RgnPlane;

	u4RgnPlane = osd_plane_map_region(u4Plane);
	u4Index = u4RgnPlane * OSD_RGN_REG_LIST;
	u4MaxRgnByPlane = OSD_MAX_NUM_RGN_LIST + u4Index;
	OSD_PRINTF(OSD_CONFIG_SW_LOG,
		   "osd_reset_plane_region_state Plane=0x%x,Index=%d\n", u4Plane, u4Index);
	for (; u4Index < u4MaxRgnByPlane; u4Index++) {
		if (_rAllRgnNode[u4Index].eOsdRgnState != OSD_RGN_STATE_READ_DONE) {
			_rAllRgnNode[u4Index].eOsdRgnState = OSD_RGN_STATE_IDLE;
			OSD_PRINTF(OSD_CONFIG_SW_LOG,
				   "osd_reset_plane_region_state Addr=0x%lx,Index=%d\n",
				   _rAllRgnNode[u4Index].pu4_vAddr, u4Index);
		}
	}
	return OSD_RET_OK;
}

INT32 OSD_RGN_Get(UINT32 u4Region, INT32 i4Cmd, UINT32 *pu4Value)
{
	if (pu4Value == NULL)
		return -(INT32) OSD_RET_INV_ARG;

	OSD_VERIFY_REGION(u4Region);
	if (u4Region >= OSD_RGN_REG_NUM) {
		OSD_LOG_I("OSD_RGN_Get: u4Region is %d, Invalid\n", u4Region);
		return -(INT32) OSD_RET_INV_ARG;
	}
#if 0
	switch (i4Cmd) {
	case OSD_RGN_POS_X:
		return _OSD_RGN_GetOutputPosX(u4Region, pu4Value);
	case OSD_RGN_POS_Y:
		return _OSD_RGN_GetOutputPosY(u4Region, pu4Value);
	case OSD_RGN_BMP_W:
		return _OSD_RGN_GetInputWidth(u4Region, pu4Value);
	case OSD_RGN_BMP_H:
		return _OSD_RGN_GetInputHeight(u4Region, pu4Value);
	case OSD_RGN_DISP_W:
	case OSD_RGN_OUT_W:
		return _OSD_RGN_GetOutputWidth(u4Region, pu4Value);
	case OSD_RGN_DISP_H:
	case OSD_RGN_OUT_H:
		return _OSD_RGN_GetOutputHeight(u4Region, pu4Value);
	case OSD_RGN_COLORMODE:
		return _OSD_RGN_GetColorMode(u4Region, pu4Value);
	case OSD_RGN_ALPHA:
		return _OSD_RGN_GetAlpha(u4Region, pu4Value);
	case OSD_RGN_BMP_ADDR:
		return _OSD_RGN_GetDataAddr(u4Region, pu4Value);
	case OSD_RGN_BMP_PITCH:
		{
			UINT32 u4LineSize;

			_OSD_RGN_GetLineSize(u4Region, &u4LineSize);
			*pu4Value = (u4LineSize << 4);
			break;
		}
	case OSD_RGN_CLIP_V:
		return _OSD_RGN_GetVClip(u4Region, pu4Value);
	case OSD_RGN_CLIP_H:
		return _OSD_RGN_GetHClip(u4Region, pu4Value);
	case OSD_RGN_STEP_V:
		return _OSD_RGN_GetVStep(u4Region, pu4Value);
	case OSD_RGN_STEP_H:
		return _OSD_RGN_GetHStep(u4Region, pu4Value);
	case OSD_RGN_COLOR_KEY:
		return _OSD_RGN_GetColorKey(u4Region, pu4Value);
	case OSD_RGN_COLOR_KEY_EN:
		return _OSD_RGN_GetColorKeyEnable(u4Region, pu4Value);
	case OSD_RGN_MIX_SEL:
		return _OSD_RGN_GetBlendMode(u4Region, pu4Value);
	case OSD_RGN_ALPHA_SEL:
		return _OSD_RGN_GetASel(u4Region, pu4Value);
	case OSD_RGN_YR_SEL:
		return _OSD_RGN_GetYrSel(u4Region, pu4Value);
	case OSD_RGN_UG_SEL:
		return _OSD_RGN_GetUgSel(u4Region, pu4Value);
	case OSD_RGN_VB_SEL:
		return _OSD_RGN_GetVbSel(u4Region, pu4Value);
	case OSD_RGN_NEXT_EN:
		return _OSD_RGN_GetNextEnable(u4Region, pu4Value);
	case OSD_RGN_NEXT_HDR_ADDR:
		return _OSD_RGN_GetNextRegion(u4Region, pu4Value);
	case OSD_RGN_FIFO_EX:
		return _OSD_RGN_GetFifoEx(u4Region, pu4Value);
	default:
		return -(INT32) OSD_RET_INV_ARG;
	}
#endif
	return (INT32) OSD_RET_OK;
}

INT32 OSD_RGN_Set(UINT32 u4Region, INT32 i4Cmd, UINT32 u4Value)
{
	OSD_VERIFY_REGION(u4Region);

	if (u4Region >= OSD_RGN_REG_NUM)
		return -(INT32) OSD_RET_INV_ARG;

	switch (i4Cmd) {
#if 0
	case OSD_RGN_POS_X:
		return _OSD_RGN_SetOutputPosX(u4Region, u4Value);

	case OSD_RGN_POS_Y:
		return _OSD_RGN_SetOutputPosY(u4Region, u4Value);

	case OSD_RGN_BMP_W:
		return _OSD_RGN_SetInputWidth(u4Region, u4Value);

	case OSD_RGN_BMP_H:
		return _OSD_RGN_SetInputHeight(u4Region, u4Value);

	case OSD_RGN_DISP_W:
		return OSD_RGN_SetDisplayWidth(u4Region, u4Value);

	case OSD_RGN_DISP_H:
		return OSD_RGN_SetDisplayHeight(u4Region, u4Value);

	case OSD_RGN_OUT_W:
		return _OSD_RGN_SetOutputWidth(u4Region, u4Value);

	case OSD_RGN_OUT_H:
		return _OSD_RGN_SetOutputHeight(u4Region, u4Value);

	case OSD_RGN_COLORMODE:
		return _OSD_RGN_SetColorMode(u4Region, u4Value);

	case OSD_RGN_ALPHA:
		return _OSD_RGN_SetAlpha(u4Region, u4Value);

	case OSD_RGN_BMP_ADDR:
		return _OSD_RGN_SetDataAddr(u4Region, u4Value);

	case OSD_RGN_BMP_PITCH:
		IGNORE_RET(_OSD_RGN_SetLineSize(u4Region, (u4Value >> 4) & 0x7ff));
		break;

	case OSD_RGN_CLIP_V:
		return _OSD_RGN_SetVClip(u4Region, u4Value);

	case OSD_RGN_CLIP_H:
		return _OSD_RGN_SetHClip(u4Region, u4Value);

	case OSD_RGN_STEP_V:
		return _OSD_RGN_SetVStep(u4Region, u4Value);

	case OSD_RGN_STEP_H:
		return _OSD_RGN_SetHStep(u4Region, u4Value);

	case OSD_RGN_COLOR_KEY:
		return _OSD_RGN_SetColorKey(u4Region, u4Value);

	case OSD_RGN_COLOR_KEY_EN:
		return _OSD_RGN_SetColorKeyEnable(u4Region, u4Value);

	case OSD_RGN_MIX_SEL:
		return _OSD_RGN_SetBlendMode(u4Region, u4Value);

	case OSD_RGN_BIG_ENDIAN:
		return OSD_RGN_SetBigEndian(u4Region, u4Value);

	case OSD_RGN_SELECT_BYTE_EN:
		return _OSD_RGN_SetSelectByteEn(u4Region, u4Value);

	case OSD_RGN_ACS_FRAME_MODE:
		return _OSD_RGN_SetFrameMode(u4Region, u4Value);

	case OSD_RGN_ACS_TOP:
		return _OSD_RGN_SetTopField(u4Region, u4Value);

	case OSD_RGN_ACS_AUTO:
		return _OSD_RGN_SetAutoMode(u4Region, u4Value);
#endif

	default:
		return -(INT32) OSD_RET_INV_ARG;
	}

	return (INT32) OSD_RET_OK;
}

INT32 OSD_RGN_SetDisplayWidth(UINT32 u4Region, UINT32 u4Width)
{
#if 0
	UINT32 src_w;
	UINT32 step;
	INT32 ignore;

	OSD_VERIFY_REGION(u4Region);

	if (u4Width == 0)
		return -(INT32) OSD_RET_INV_ARG;

	ignore = _OSD_RGN_GetInputWidth(u4Region, &src_w);
	ignore = _OSD_RGN_SetOutputWidth(u4Region, u4Width);
	step = (src_w == u4Width) ? 0x1000 : ((src_w << 12) / u4Width);
	ignore = _OSD_RGN_SetHStep(u4Region, (step > 0xffff) ? 0xffff : step);
#endif
	return (INT32) OSD_RET_OK;
}

INT32 OSD_RGN_SetDisplayHeight(UINT32 u4Region, UINT32 u4Height)
{
#if 0
	UINT32 src_h;
	UINT32 step;
	INT32 ignore;

	OSD_VERIFY_REGION(u4Region);

	if (u4Height == 0)
		return -(INT32) OSD_RET_INV_ARG;

	ignore = _OSD_RGN_GetInputHeight(u4Region, &src_h);
	ignore = _OSD_RGN_SetOutputHeight(u4Region, u4Height);
	step = (src_h == u4Height) ? 0x1000 : ((src_h << 12) / u4Height);
	ignore = _OSD_RGN_SetVStep(u4Region, (step > 0xffff) ? 0xffff : step);
#endif
	return (INT32) OSD_RET_OK;
}

INT32 OSD_RGN_SetBigEndian(UINT32 u4Region, BOOL fgBE)
{
	OSD_VERIFY_REGION(u4Region);
#if 0
	if (fgBE) {
		IGNORE_RET(_OSD_RGN_SetASel(u4Region, 0));
		IGNORE_RET(_OSD_RGN_SetYrSel(u4Region, 1));
		IGNORE_RET(_OSD_RGN_SetUgSel(u4Region, 2));
		IGNORE_RET(_OSD_RGN_SetVbSel(u4Region, 3));
	} else {
		IGNORE_RET(_OSD_RGN_SetASel(u4Region, 3));
		IGNORE_RET(_OSD_RGN_SetYrSel(u4Region, 2));
		IGNORE_RET(_OSD_RGN_SetUgSel(u4Region, 1));
		IGNORE_RET(_OSD_RGN_SetVbSel(u4Region, 0));
	}
#endif
	return (INT32) OSD_RET_OK;
}

INT32 OSD_RGN_Create_EX(UINT32 u4Region, UINT32 u4SrcDispX, UINT32 u4SrcDispY,
			UINT32 s_width, UINT32 s_height,
			unsigned int pvBitmap, UINT32 eColorMode, UINT32 u4BmpPitch,
			UINT32 u4DispX, UINT32 u4DispY,
			UINT32 u4DispW, UINT32 u4DispH,
			UINT32 ui4Plane, uint32_t index, UINT32 u4AlphaEn, uint32_t alpha, uint32_t fmt_order)
{
	INT32 i4Ret;
	UINT32 u4VStep;
	UINT32 u4HStep;
	int BPP = 0;

	OSD_PRINTF(OSD_CONFIG_SW_LOG, "Create region: region=%d,index =%d, addr=0x%x,CM=%d,u4BmpPitch=%d\n",
		   u4Region, index, pvBitmap, eColorMode, u4BmpPitch);
	OSD_PRINTF(OSD_CONFIG_SW_LOG, "Create region: w=%d,h=%d,ox=%d,oy=%d,ow=%d,oh=%d\n",
		   s_width, s_height, u4DispX, u4DispY, u4DispW, u4DispH);

	if ((u4Region >= OSD_RGN_REG_NUM) || (pvBitmap == 0) ||
	    ((pvBitmap & 0xf) != 0) || (s_width == 0) ||
	    (s_height == 0) || (u4DispW == 0) || (u4DispH == 0) ||
	    ((u4BmpPitch & 0xf) != 0) || ((u4BmpPitch >> 15) != 0)) {
		OSD_LOG_I("region Attr Err pvbitmap=%x!!\n", pvBitmap);
		OSD_LOG_I("Create region: region=%d,index =%d, addr=0x%x,CM=%d,u4BmpPitch=%d\n",
			   u4Region, index, pvBitmap, eColorMode, u4BmpPitch);
		OSD_LOG_I("Create region: w=%d,h=%d,ox=%d,oy=%d,ow=%d,oh=%d\n",
			   s_width, s_height, u4DispX, u4DispY, u4DispW, u4DispH);
		return -(INT32) OSD_RET_INV_ARG;
	}

	if ((eColorMode == (UINT32) OSD_CM_AYCBCR8888_DIRECT32) ||
		(eColorMode == (UINT32) OSD_CM_ARGB8888_DIRECT32))
		BPP = 4;
	else
		BPP = 2;

	if (u4SrcDispX >= 16) {
		pvBitmap += ((u4SrcDispX / 16) * 16 * BPP);
		u4SrcDispX %= 16;
	}

	i4Ret = _OSD_RGN_SetNextRegion(u4Region, index, 0);
	i4Ret = _OSD_RGN_SetNextEnable(u4Region, index, 0);
	i4Ret = _OSD_RGN_SetColorMode(u4Region, index, eColorMode);
	i4Ret = _OSD_RGN_SetDataAddr(u4Region, index, pvBitmap);
	i4Ret = _OSD_RGN_SetInputWidth(u4Region, index, s_width);
	i4Ret = _OSD_RGN_SetInputHeight(u4Region, index, s_height);
	i4Ret = _OSD_RGN_SetOutputWidth(u4Region, index, u4DispW);
	i4Ret = _OSD_RGN_SetOutputHeight(u4Region, index, u4DispH);
	i4Ret = _OSD_RGN_SetOutputPosX(u4Region, index, u4DispX);
	i4Ret = _OSD_RGN_SetOutputPosY(u4Region, index, u4DispY);
	i4Ret = _OSD_RGN_SetColorKeyEnable(u4Region, index, 0);
	i4Ret = _OSD_RGN_SetColorKey(u4Region, index, 0);
	i4Ret = _OSD_RGN_SetHClip(u4Region, index, u4SrcDispX);
	i4Ret = _OSD_RGN_SetVClip(u4Region, index, u4SrcDispY);
	i4Ret = _OSD_RGN_SetAutoMode(u4Region, index, 1);
	i4Ret = _OSD_RGN_SetMixSel(u4Region, index, 1);

	i4Ret = _OSD_RGN_SetAlpha(u4Region, index, (uint32_t) 0xFF);
	if (u4AlphaEn)
		i4Ret = _OSD_RGN_SetMixSel(u4Region, index, 1);
	else
		i4Ret = _OSD_RGN_SetMixSel(u4Region, index, 2);

	/*todo: change this cmd*/
	/*i4Ret = OSD_RGN_Set(u4Region, (INT32) OSD_RGN_BMP_PITCH, u4BmpPitch);*/
	_OSD_RGN_SetLineSize(u4Region, index, (u4BmpPitch >> 4) & 0x7ff);
	u4HStep = (s_width == u4DispW) ? 0x1000 : ((s_width << 12) / u4DispW);
	i4Ret = _OSD_RGN_SetHStep(u4Region, index, u4HStep);
	u4VStep = (s_height == u4DispH) ? 0x1000 : ((s_height << 12) / u4DispH);
	i4Ret = _OSD_RGN_SetVStep(u4Region, index, u4VStep);
	OSD_PRINTF(OSD_CONFIG_SW_LOG, "Create region:HStep =0x%x,VStep =0x%x\n", u4HStep, u4VStep);

	if (fmt_order == 3) {
		i4Ret = _OSD_RGN_SetASel(u4Region, index, 3);
		i4Ret = _OSD_RGN_SetYrSel(u4Region, index, 2);
		i4Ret = _OSD_RGN_SetUgSel(u4Region, index, 1);
		i4Ret = _OSD_RGN_SetVbSel(u4Region, index, 0);
	} else if (fmt_order == 2) {
		i4Ret = _OSD_RGN_SetASel(u4Region, index, 3);
		i4Ret = _OSD_RGN_SetYrSel(u4Region, index, 0);
		i4Ret = _OSD_RGN_SetUgSel(u4Region, index, 1);
		i4Ret = _OSD_RGN_SetVbSel(u4Region, index, 2);
	}
	return (INT32) OSD_RET_OK;
}

int osd_create_multi_region(UINT32 u4Region, UINT32 u4SrcDispX, UINT32 u4SrcDispY,
			UINT32 s_width, UINT32 s_height,
			unsigned int pvBitmap, UINT32 eColorMode, UINT32 u4BmpPitch,
			UINT32 u4DispX, UINT32 u4DispY,
			UINT32 u4DispW, UINT32 u4DispH, UINT32 ui4Plane, uint32_t index)
{

	/*get parameter*/

	/*multi-region setting*/
	return 0;
}


/***********************************************************************************************************/
int i4Osd_BaseSetFmt(uint32_t plane, uint32_t u4Path, struct osd_resolution_change *res_chg, bool fgFlip)
{
	uint32_t u4HSize, u4VSize, u4HTotal, u4VTotal;
	uint32_t u4DisplayMode;

	u4DisplayMode = res_chg->osd_res_mode;
	OSDDBG("i4Osd_BaseSetFmt u4Path=%d, u4DM=%d, fgFlip=%d\n",
	  u4Path, u4DisplayMode, fgFlip);

	osd_get_scrn_size(res_chg, u4Path, &u4HSize, &u4VSize);
	osd_get_res_total_size(res_chg, u4Path, &u4HTotal, &u4VTotal);

	/*_OSD_BASE_SetForceUnupdate(plane, true);*/
	_OSD_BASE_SetOsdPrgs(plane, 1);/*use P2I to realize Interlace*/

	IGNORE_RET(_OSD_BASE_SetOvtMain(plane, u4VTotal));
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(plane, 6));
	IGNORE_RET(_OSD_BASE_SetHsWidthMain(plane, 1));
	IGNORE_RET(_OSD_BASE_SetOhtMain(plane, u4HTotal));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(plane, u4HSize));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(plane, u4VSize));

	IGNORE_RET(_OSD_BASE_SetAutoSwEn(plane, 1));
	/* reconfig all related osd progress flag */
	/* scl reconfig must before plane reconfig */
	if (fgFlip) {
		osd_base_reconfig_all(plane, u4Path, res_chg->osd_res_mode);
		/*osd_sc_reconfig_all(plane, res_chg);*/
	}
	if (plane == OSD_PLANE_2) { /*plane cnt 4ms before osd vsync*/
		if (res_chg->frequency == 60)
			IGNORE_RET(osd_pla_set_sync_threshold(plane, u4VTotal * 76 / 100));
		else if (res_chg->frequency == 50)
			IGNORE_RET(osd_pla_set_sync_threshold(plane, u4VTotal * 4 / 5));
		else if (res_chg->frequency == 30)
			IGNORE_RET(osd_pla_set_sync_threshold(plane, u4VTotal * 88 / 100));
		else /*less than 30*/
			IGNORE_RET(osd_pla_set_sync_threshold(plane, u4VTotal * 90 / 100));

		osd_plane_set_sync_threshold_update(plane, 1);
	}

#if 0
	if (plane == OSD_PLANE_1) {
		IGNORE_RET(_OSD_PLA_SetBurst8_Enable(plane, 0x1));
		osd_pla_set_auto_ultra(plane, 0x100080FF);
		IGNORE_RET(_OSD_PLA_SetFifoSize(plane, 0xC));
	}
#else
	if (plane == OSD_PLANE_1)
		osd_pla_set_auto_ultra(plane, 0x100080FF);
#endif

	_OSD_BASE_SetAlphaSel(plane, 0x1f);
	_OSD_BASE_SetUpdate(plane, true);
	_OSD_BASE_SetAlwaysUpdate(plane, false);
	return OSD_RET_OK;
}

INT32 _OSD_BASE_SetOsdPrgs(UINT32 u4Plane, UINT32 u4ProgressFlag)
{
	switch (u4Plane) {
	case OSD_PLANE_1:
		IGNORE_RET(_OSD_BASE_SetOsd2Prgs(u4Plane, u4ProgressFlag));
		break;
	case OSD_PLANE_2:
		IGNORE_RET(_OSD_BASE_SetOsd3Prgs(u4Plane, u4ProgressFlag));
		break;
	default:
		break;
	}

	_OSD_BASE_SetUpdate(u4Plane, true);
	return OSD_RET_OK;

}

/************************************************************************************************************/
BOOL OSD_Is_PLA_Enabled(UINT32 u4Plane)
{
	UINT32 u4CoreRg_00;

	switch (u4Plane) {
	case OSD_PLANE_1:
		_OSD_PLA_GetEnable(OSD_PLANE_1, &u4CoreRg_00);
		break;
	case OSD_PLANE_2:
		_OSD_PLA_GetEnable(OSD_PLANE_2, &u4CoreRg_00);
		break;
	default:
		u4CoreRg_00 = 0;
	}

	if (0 == (u4CoreRg_00 & 1))
		return false;
	else
		return true;
}

INT32 OSD_PLA_Reset(UINT32 u4Plane)
{
	OSD_VERIFY_PLANE(u4Plane);

	/* reset plane's hardware state */
	/* IGNORE_RET(_OSD_PLA_SetReg(u4Plane, NULL)); */
	/* IGNORE_RET(_OSD_PLA_SetEnable(u4Plane, 0));*/
#if 0
	IGNORE_RET(_OSD_PLA_SetHFilter(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetBlending(u4Plane, 0xff));
	IGNORE_RET(_OSD_PLA_SetFading(u4Plane, 0xff));

	IGNORE_RET(_OSD_PLA_SetFakeHdr(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetPrngEn(u4Plane, 0));

	IGNORE_RET(_OSD_PLA_SetOutRngColorMode(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetColorExpSel(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetAlphaRatioEn(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetNonAlphaShift(u4Plane, 1));
	IGNORE_RET(_OSD_PLA_SetContReqLmt(u4Plane, 0xF));
	IGNORE_RET(_OSD_PLA_SetFifoSize(u4Plane, 0x3F));
	IGNORE_RET(_OSD_PLA_SetPauseCnt(u4Plane, 0x3));
	IGNORE_RET(_OSD_PLA_SetContReqLmt0(u4Plane, 0xf));
	IGNORE_RET(_OSD_PLA_SetBurstDis(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetRgbMode(u4Plane, 0));
	IGNORE_RET(_OSD_PLA_SetVacancyThr(u4Plane, 0x7));
#endif
	if (_fgOsdYCbCr709)
		_OSD_PLA_SetYCbCr709En((UINT32) u4Plane, 1);
	else
		_OSD_PLA_SetYCbCr709En((UINT32) u4Plane, 0);
	_OSD_PLA_SetRgbMode((UINT32) u4Plane, 1);

#if 0
	if (u4Plane == OSD_PLANE_1) {
		IGNORE_RET(_OSD_PLA_SetBurst8_Enable(u4Plane, 0x1));
		osd_pla_set_auto_ultra(u4Plane, 0x100080FF);
		IGNORE_RET(_OSD_PLA_SetFifoSize(u4Plane, 0xC));
	}
#else
	if (u4Plane == OSD_PLANE_1)
		osd_pla_set_auto_ultra(u4Plane, 0x100080FF);
#endif

	return (INT32) OSD_RET_OK;
}

UINT32 osd_plane_map_region(UINT32 u4Plane)
{
	UINT32 u4RgnPlane = 0;

	switch (u4Plane) {
	case OSD_PLANE_2:
		u4RgnPlane = 0;
		break;
	default:
		u4RgnPlane = 1;
		break;
	}
	return u4RgnPlane;
}

INT32 OSDSetColorSpace(UINT8 ui1ClrSpa)
{
	UINT32 u4Plane;

	if (ui1ClrSpa == OSD_COLORSPACE_601) {
		_fgOsdYCbCr709 = false;
		for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++)
			_OSD_PLA_SetYCbCr709En((UINT32) u4Plane, 0);
	} else {
		_fgOsdYCbCr709 = true;
		for (u4Plane = OSD_PLANE_1; u4Plane < OSD_PLANE_MAX_NUM; u4Plane++)
			_OSD_PLA_SetYCbCr709En((UINT32) u4Plane, 1);
	}
	return (INT32) OSD_RET_OK;
}

INT32 OSD_PLA_Update(uint32_t plane)
{
	uint32_t update;

	if (_OSD_PLA_GetUpdateStatus(plane)) {
		/*OSDDBG("[Osd %ld]OSD_PLA_Update plane =  0x%x\n ", vsync_cnt, plane);*/
		_OSD_PLA_Update(plane);
		_OSD_PLA_SetUpdateStatus(plane, false);
	}


	osd_plane_get_extend_update(plane, &update);
	if (update) {
		osd_plane_update_extend(plane);
		osd_plane_set_extend_update(plane, false);
	}

	osd_plane_get_sync_threshold_update(plane, &update);
	if (update) {
		osd_plane_update_sync_threshold(plane);
		osd_plane_set_sync_threshold_update(plane, false);
	}
	return (INT32) OSD_RET_OK;
}


INT32 OSDGetColorSpace(UINT8 *ui1ClrSpa)
{
	if (ui1ClrSpa != NULL) {
		if (_fgOsdYCbCr709)
			*ui1ClrSpa = OSD_COLORSPACE_709;
		else
			*ui1ClrSpa = OSD_COLORSPACE_601;
	}

	return (INT32) OSD_RET_OK;
}

void OSD_PLA_DisableAllPlane(uint32_t i)
{
	if (OSD_Is_PLA_Enabled(i)) {
		OSD_LOG_I("Before change res, osd plane %d is enabled\n", i);
		_afgPlaneEnableStatus[i] = true;

		IGNORE_RET(_OSD_PLA_SetEnable(i, (UINT32) false));
		VERIFYED(_OSD_PLA_UpdateHwReg(i) == (INT32) OSD_RET_OK);
	}

	//_Osd_PustReset_Plane(OSD_PLANE_MAX_NUM);
}

void OSD_PLA_EnablePlane(void)
{
	UINT32 i;

	OSD_LOG_I("En all plane closed before change resolution\n");

	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		if (_afgPlaneEnableStatus[i]) {
			_afgPlaneEnableStatus[i] = false;
			OSD_LOG_I("After change res, enable osd plane %d\n", i);
			IGNORE_RET(_OSD_PLA_SetEnable(i, (UINT32) true));
			VERIFYED(_OSD_PLA_UpdateHwReg(i) == (INT32) OSD_RET_OK);
		}

		//_Osd_ReleaseReset_Plane(i);
	}

}
void osd_premix_set_mix_del_sel(unsigned int plane, bool enable)
{
	if (plane == OSD_PLANE_1) { /*UHD*/
		if (enable)
			osd_pmx_set_mix2_de_sel(1);
		else
			osd_pmx_set_mix2_de_sel(0);
	} else if (plane == OSD_PLANE_2) {
		if (enable)
			osd_pmx_set_mix1_de_sel(1);
		else
			osd_pmx_set_mix1_de_sel(0);
	}
	_OSD_BASE_SetPremixUpdate(plane, 1);
}

void osd_plane_enable(unsigned int plane, bool enable)
{
	if (enable ==  true) {
		IGNORE_RET(_OSD_PLA_SetEnable(plane, (UINT32) true));
		VERIFYED(_OSD_PLA_UpdateHwReg(plane) == (INT32) OSD_RET_OK);

		/*change premix de for dolby*/
		/*osd_premix_set_mix_del_sel(plane, enable);*/
	} else {
		if (OSD_Is_PLA_Enabled(plane)) {
			OSD_LOG_I("disable osd plane %d\n", plane);

			IGNORE_RET(_OSD_PLA_SetEnable(plane, (UINT32) false));
			VERIFYED(_OSD_PLA_UpdateHwReg(plane) == (INT32) OSD_RET_OK);

			/*change premix de for dolby*/
			/*osd_premix_set_mix_del_sel(plane, enable);*/
		}
	}
}

INT32 OSD_PLA_SetBlendLevel(UINT32 u4Plane, UINT8 u1BlendLevel)
{
	OSD_VERIFY_PLANE(u4Plane);
	_OSD_PLA_SetBlending(u4Plane, u1BlendLevel);
	_OSD_PLA_UpdateHwReg(u4Plane);
	return (INT32) OSD_RET_OK;
}

INT32 OSD_PLA_GetFading(UINT32 u4Plane, UINT8 *pu1Fading)
{
	UINT32 u4Fading;

	if (pu1Fading == NULL)
		return -(INT32) OSD_RET_INV_ARG;

	OSD_VERIFY_PLANE(u4Plane);
	VERIFYED(_OSD_PLA_GetFading(u4Plane, &u4Fading) == (INT32) OSD_RET_OK);
	*pu1Fading = (UINT8) u4Fading;

	return (INT32) OSD_RET_OK;
}

INT32 OSD_PLA_SetFading(UINT32 u4Plane, UINT8 u1Fading)
{
	OSD_VERIFY_PLANE(u4Plane);
	VERIFYED(_OSD_PLA_SetFading(u4Plane, u1Fading) == (INT32) OSD_RET_OK);
	VERIFYED(_OSD_PLA_UpdateHwReg(u4Plane) == (INT32) OSD_RET_OK);
	return (INT32) OSD_RET_OK;
}

INT32 OSD_PLA_SetHFilter(UINT32 u4Plane, BOOL fgEnable)
{
	OSD_VERIFY_PLANE(u4Plane);
	VERIFYED(_OSD_PLA_SetHFilter(u4Plane, fgEnable) == (INT32) OSD_RET_OK);
	VERIFYED(_OSD_PLA_UpdateHwReg(u4Plane) == (INT32) OSD_RET_OK);
	return (INT32) OSD_RET_OK;
}

void osd_alpha_detect_enable(UINT32 u4Plane, BOOL fgEnable, unsigned int bmp_height)
{
	unsigned int region_height = 0;

	region_height = bmp_height/(OSD_RGN_CLUSTER_NUM - 1);

	osd_pla_set_region_width(u4Plane, region_height);
	osd_pla_set_alpha_detect_threshold(u4Plane, 0);
	osd_pla_set_alpha_detect_en(u4Plane, fgEnable);
	/*_OSD_PLA_UpdateHwReg(u4Plane);*/
	osd_plane_set_extend_update(u4Plane, true);
}

INT32 OsdFlipPlaneRightNow(UINT32 u4Plane, BOOL fgValidReg, UINT32 u4Region, unsigned int idx)
{
	OSD_VERIFY_PLANE(u4Plane);

	if (fgValidReg)
		OSD_PLA_FlipTo(u4Plane, u4Region, idx);
	else {
		OSD_PLA_FlipToNone(u4Plane);
	}

	return (INT32) OSD_RET_OK;
}

INT32 OSD_PLA_FlipTo(UINT32 u4Plane, UINT32 u4Region, unsigned int idx)
{
	UINT32 header_addr;

	/*todo*/
	_OSD_RGN_GetAddress(u4Region, idx, &header_addr);
	OSD_PRINTF(OSD_CONFIG_SW_LOG, "OSD_PLA_FlipTo,u4FirstRegionAddr 0x%x\n", header_addr);

	/* ASSERT((header_addr & 0xf) == 0);*/
	_OSD_PLA_SetHeaderAddr(u4Plane, header_addr);
	osd_plane_enable(u4Plane, true);
	return OSD_RET_OK;
}

INT32 OSD_PLA_FlipToNone(UINT32 u4Plane)
{
	osd_plane_enable(u4Plane, false);
	return (INT32) OSD_RET_OK;
}

INT32 OSD_SC_CheckCapability(UINT32 u4SrcW, UINT32 u4SrcH, UINT32 u4DstW, UINT32 u4DstH)
{
	return OSD_RET_OK;
}

INT32 osd_sc_check_same_scaler_info(UINT32 u4Scaler, UINT32 u4SrcWidth,
		   UINT32 u4SrcHeight, UINT32 uDstX, UINT32 uDstY, UINT32 u4DstWidth, UINT32 u4DstHeight)
{
	if (u4SrcWidth != _arOsdScaleInfo[u4Scaler].u4SrcW)
		return -OSD_RET_INV_ARG;

	if (u4SrcHeight != _arOsdScaleInfo[u4Scaler].u4SrcH)
		return -OSD_RET_INV_ARG;

	if (u4DstWidth != _arOsdScaleInfo[u4Scaler].u4DstW)
		return -OSD_RET_INV_ARG;

	if (u4DstHeight != _arOsdScaleInfo[u4Scaler].u4DstH)
		return -OSD_RET_INV_ARG;

	if (uDstX != _arOsdScaleInfo[u4Scaler].dst_x)
		return -OSD_RET_INV_ARG;

	if (uDstY != _arOsdScaleInfo[u4Scaler].dst_y)
		return -OSD_RET_INV_ARG;

	return OSD_RET_OK;
}

INT32 OSD_SC_Scale(UINT32 u4Scaler, UINT32 u4Enable, UINT32 u4SrcWidth,
		   UINT32 u4SrcHeight, UINT32 uDstX, UINT32 uDstY, UINT32 u4DstWidth, UINT32 u4DstHeight)
{
	UINT32 u4Tmp = 0, u4MaxWidth = 0, u4MaxHeight = 0, fgPrgs = 0, u4Path = OSD_MAIN_PATH;
	/*BOOL  fgHDInterlaceScale = false; */
	UINT32 u4VTopOfst = 0, u4VBotOfst = 0;
	UINT32 u4SrcV = u4SrcHeight, u4DstV = u4DstHeight;

	OSD_VERIFY_SCALER(u4Scaler);

	OSD_PRINTF(OSD_FLOW_LOG, "OSD_SC_Scale:id=%d,en=%d,src=(%d,%d),dst=(%d,%d)\n", u4Scaler,
		u4Enable, u4SrcWidth, u4SrcHeight, u4DstWidth, u4DstHeight);

#if 0
	if (osd_sc_check_same_scaler_info(u4Scaler, u4SrcWidth, u4SrcHeight,
		uDstX, uDstY, u4DstWidth, u4DstHeight) == OSD_RET_OK)
		return (INT32) OSD_RET_OK;
#endif

	_arOsdScaleInfo[u4Scaler].u4Enable = u4Enable;
	_arOsdScaleInfo[u4Scaler].u4SrcW = u4SrcWidth;
	_arOsdScaleInfo[u4Scaler].u4SrcH = u4SrcHeight;
	_arOsdScaleInfo[u4Scaler].u4DstW = u4DstWidth;
	_arOsdScaleInfo[u4Scaler].u4DstH = u4DstHeight;
	_arOsdScaleInfo[u4Scaler].dst_x = uDstX;
	_arOsdScaleInfo[u4Scaler].dst_y = uDstY;
	_OSD_SC_SetUpdateStatus(u4Scaler, false);

	if (u4Path == OSD_MAIN_PATH) {
		IGNORE_RET(_OSD_BASE_GetScrnHSizeMain(u4Scaler, &u4MaxWidth));
		IGNORE_RET(_OSD_BASE_GetScrnVSizeMain(u4Scaler, &u4MaxHeight));
	} else
		OSD_PRINTF(OSD_CONFIG_SW_LOG, "Only support main path now.\n");

	if ((u4MaxWidth == 0) || (u4MaxHeight) == 0) {
		u4MaxWidth = 0x780;
		u4MaxHeight = 0x438;
		if (u4Path == OSD_MAIN_PATH) {
			IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(u4Scaler, u4MaxWidth));
			IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(u4Scaler, u4MaxHeight));
		}
	}

	if ((uDstX + u4DstWidth) > u4MaxWidth)
		OSD_LOG_W("uDstX =0x%x, u4DstWidth =0x%x\n", uDstX, u4DstWidth);

	if ((uDstY + u4DstHeight) > u4MaxHeight)
		OSD_LOG_W("uDstY =0x%x, u4DstHeight =0x%x\n", uDstY, u4DstHeight);

	switch (u4Scaler) {
	case (UINT32) OSD_SCALER_1:
		IGNORE_RET(_OSD_BASE_SetOsd2HStart(u4Scaler, uDstX));
		IGNORE_RET(_OSD_BASE_SetOsd2VStart(u4Scaler, uDstY));
		IGNORE_RET(_OSD_BASE_GetOsd2Prgs(u4Scaler, &fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd2HStart(u4Scaler, &u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd2VStart(u4Scaler, &u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;
	case (UINT32) OSD_SCALER_2:
		IGNORE_RET(_OSD_BASE_SetOsd3HStart(u4Scaler, uDstX));
		IGNORE_RET(_OSD_BASE_SetOsd3VStart(u4Scaler, uDstY));
		IGNORE_RET(_OSD_BASE_GetOsd3Prgs(u4Scaler, &fgPrgs));
		IGNORE_RET(_OSD_BASE_GetOsd3HStart(u4Scaler, &u4Tmp));
		u4MaxWidth -= u4Tmp;
		IGNORE_RET(_OSD_BASE_GetOsd3VStart(u4Scaler, &u4Tmp));
		u4MaxHeight -= u4Tmp;
		break;
	default:
		break;
	}

	if (u4SrcWidth == 0)
		u4SrcWidth = u4MaxWidth;

	if (u4DstWidth == 0)
		u4DstWidth = u4MaxWidth;

	if (u4SrcHeight == 0)
		u4SrcHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);

	if (u4DstHeight == 0)
		u4DstHeight = (fgPrgs) ? u4MaxHeight : (u4MaxHeight << 1);


	/* modify for MW's API require */
	IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, u4Enable));
	/* to cut non-necessary src input */
	if (u4DstWidth > u4MaxWidth) {
		u4SrcWidth = (u4MaxWidth * u4SrcWidth) / u4DstWidth;
		u4DstWidth = u4MaxWidth;
		u4SrcWidth = MAX(u4SrcWidth, 1);
		u4DstWidth = MAX(u4DstWidth, 2);
	}

	if (u4SrcWidth == u4DstWidth) {
		IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
		IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4SrcWidth));
		/* clear */
		IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, 0));

	} else {

		if (u4SrcWidth < u4DstWidth) {
			/* horizontal scaling up */
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetHuscOfst(u4Scaler, 0));

			if (_afgCutEdge[u4Scaler]) {
				IGNORE_RET(_OSD_SC_SetHuscAlphaEdgeElt(u4Scaler, 1));
				IGNORE_RET(_OSD_SC_SetHdscAlphaEdgeElt(u4Scaler, 0));
			}

			u4Tmp = ((u4SrcWidth - 1) << OSD_SC_STEP_BIT) / (u4DstWidth - 1);
			IGNORE_RET(_OSD_SC_SetHuscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));
			if (u4SrcWidth == 1280)
				_OSD_SC_SetHuscOfst(u4Scaler, 0x1800);
		} else {
			/* horizontal scaling down */
			IGNORE_RET(_OSD_SC_SetHdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetHuscEn(u4Scaler, 0));

			if (_afgCutEdge[u4Scaler]) {
				IGNORE_RET(_OSD_SC_SetHdscAlphaEdgeElt(u4Scaler, 1));
				IGNORE_RET(_OSD_SC_SetHuscAlphaEdgeElt(u4Scaler, 0));
			}
			u4Tmp = ((u4DstWidth << OSD_SC_STEP_BIT) / u4SrcWidth) +
			    (((u4DstWidth << OSD_SC_STEP_BIT) % u4SrcWidth) ? 1 : 0);
			IGNORE_RET(_OSD_SC_SetHdscOfst(u4Scaler, u4Tmp));
			IGNORE_RET(_OSD_SC_SetHdscStep(u4Scaler, u4Tmp));

			IGNORE_RET(_OSD_SC_SetSrcHSize(u4Scaler, u4SrcWidth));
			IGNORE_RET(_OSD_SC_SetVscHSize(u4Scaler, u4DstWidth));
			IGNORE_RET(_OSD_SC_SetDstHSize(u4Scaler, u4DstWidth));
		}
	}

	if (!fgPrgs)
		u4DstHeight >>= 1;


	if (u4SrcHeight == u4DstHeight) {
		IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));

		if (fgPrgs) {
			if (u4DstHeight > u4MaxHeight)
				u4DstHeight = u4MaxHeight;

			u4SrcV = u4DstHeight;
			u4DstV = u4DstHeight;
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4DstHeight));
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
		} else {
			if (u4DstHeight > u4MaxHeight)
				u4DstHeight = u4MaxHeight;

			u4SrcV = u4DstHeight;
			u4DstV = u4DstHeight;
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4DstHeight));
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));
		}

		/* clear */
		u4VTopOfst = 0;
		u4VBotOfst = 0;

		IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
		IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, 0));

		/* to choose osd clock as output clock */
		/* OSD_BASE_SetClock(OSD_CK_OCLK); */
		/* ??? */
		/* IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS)); */
	} else {
		if (u4DstHeight > u4MaxHeight) {
			u4SrcHeight = (u4MaxHeight * u4SrcHeight) / u4DstHeight;
			u4DstHeight = u4MaxHeight;
			u4SrcHeight = MAX(u4SrcHeight, 1);
			u4DstHeight = MAX(u4DstHeight, 2);
		}

		/* "=" only happen when src=2*dst in interlaced mode */
		if (u4SrcHeight <= u4DstHeight) {
			/* vertical scaling up */
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 0));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 1));

			if (_afgCutEdge[u4Scaler]) {
				IGNORE_RET(_OSD_SC_SetVuscAlphaEdgeElt(u4Scaler, 1));
				IGNORE_RET(_OSD_SC_SetVdscAlphaEdgeElt(u4Scaler, 0));
			}

			if (fgPrgs) {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) / (u4DstHeight - 1);

				u4VTopOfst = 0;
				u4VBotOfst = 0;

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, 0));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, 0));
			} else {
				u4Tmp = ((u4SrcHeight - 1) << OSD_SC_STEP_BIT) / (u4DstHeight);
				if ((u4Tmp % (1 << OSD_SC_STEP_BIT)) == 0)
					u4Tmp--;

				u4VTopOfst = u4Tmp >> 2;
				u4VBotOfst = ((3 * u4Tmp) >> 2) & 0x3ffc;

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4VTopOfst));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4VBotOfst));
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));

			u4SrcV = u4SrcHeight;
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			/* if (fgPrgs || fgHDInterlaceScale) */
			u4DstV = u4DstHeight;
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));

			/* to choose osd clock as output clock */
			/* OSD_BASE_SetClock(OSD_CK_OCLK); */
			/* ??? */
			/* IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS)); */
		} else {


			if (OSD_SC_CheckCapability(u4SrcWidth, u4SrcHeight,
						   u4DstWidth, u4DstHeight) != (INT32) OSD_RET_OK)
				OSD_LOG_I("OSD_SC_CheckCapability: fail\n");

			/* vertical scaling down */
			IGNORE_RET(_OSD_SC_SetVdscEn(u4Scaler, 1));
			IGNORE_RET(_OSD_SC_SetVuscEn(u4Scaler, 0));

			if (_afgCutEdge[u4Scaler]) {
				IGNORE_RET(_OSD_SC_SetVdscAlphaEdgeElt(u4Scaler, 1));
				IGNORE_RET(_OSD_SC_SetVuscAlphaEdgeElt(u4Scaler, 0));
			}

			if (fgPrgs) {
				u4Tmp = ((u4DstHeight << OSD_SC_STEP_BIT) / u4SrcHeight) +
				    (((u4DstHeight << OSD_SC_STEP_BIT) % u4SrcHeight) ? 1 : 0);

				u4VTopOfst = u4Tmp;
				u4VBotOfst = u4Tmp;

				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4VTopOfst));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4VBotOfst));
			} else {
				u4Tmp = ((u4DstHeight + 1) << OSD_SC_STEP_BIT) / u4SrcHeight;
				if (u4Tmp == (1 << (OSD_SC_STEP_BIT - 1))) {
					u4VTopOfst = (1 << OSD_SC_STEP_BIT) - 1;
					u4VBotOfst = (1 << (OSD_SC_STEP_BIT - 1)) - 1;
				} else if (u4Tmp < (1 << (OSD_SC_STEP_BIT - 1))) {
					u4VTopOfst = u4Tmp + (1 << (OSD_SC_STEP_BIT - 1));
					u4VBotOfst = u4Tmp;
				} else {
					u4VTopOfst = u4Tmp;
					u4VBotOfst = u4Tmp - (1 << (OSD_SC_STEP_BIT - 1));
				}
				IGNORE_RET(_OSD_SC_SetVscOfstTop(u4Scaler, u4VTopOfst));
				IGNORE_RET(_OSD_SC_SetVscOfstBot(u4Scaler, u4VBotOfst));
			}

			IGNORE_RET(_OSD_SC_SetVscStep(u4Scaler, u4Tmp));

			u4SrcV = u4SrcHeight;
			IGNORE_RET(_OSD_SC_SetSrcVSize(u4Scaler, u4SrcHeight));

			/* if (fgPrgs || fgHDInterlaceScale) */
			u4DstV = u4DstHeight;
			IGNORE_RET(_OSD_SC_SetDstVSize(u4Scaler, u4DstHeight));

			/* to choose osd clock as syspll_d2_ck */
			/* ??? */
			/* IGNORE_RET(OSD_BASE_SetClock(OSD_CK_SYS)); */
		}
	}

	_OSD_SC_SetUpdateStatus(u4Scaler, true);
	osd_base_set_start_update(u4Scaler, true);
	/*_OSD_BASE_SetUpdate(u4Scaler, true);*/

	/* SHOULD NOT do reflip plane in scaler functions!! */
	/* OSD_PLA_Reflip(u4Scaler); */

	return (INT32) OSD_RET_OK;
}

INT32 OSD_SC_SetLpfInfo(UINT32 u4Scaler, UINT32 u4Enable, INT16 i2C1,
			INT16 i2C2, INT16 i2C3, INT16 i2C4, INT16 i2C5)
{
	UINT16 tmp = 0;

	OSD_VERIFY_SCALER(u4Scaler);
	IGNORE_RET(_OSD_SC_SetScLpfEn(u4Scaler, u4Enable));
	IGNORE_RET(_OSD_SC_SetScLpfC3(u4Scaler, tmp));
	IGNORE_RET(_OSD_SC_SetScLpfC4(u4Scaler, tmp));

	if ((i2C5 > 127) || (i2C5 < 0))
		return -(INT32) OSD_RET_INV_ARG;

	tmp = (UINT16) i2C5;
	IGNORE_RET(_OSD_SC_SetScLpfC5(u4Scaler, tmp));
	if (u4Enable)
		IGNORE_RET(_OSD_SC_SetScEn(u4Scaler, true));


	IGNORE_RET(_OSD_SC_UpdateHwReg(u4Scaler));

	return (INT32) OSD_RET_OK;
}

INT32 OSD_SC_SetLpf(UINT32 u4Scaler, UINT32 u4Enable)
{
	return OSD_SC_SetLpfInfo(u4Scaler, u4Enable, OSD_DEFAULT_LPF_C1,
				 OSD_DEFAULT_LPF_C2, OSD_DEFAULT_LPF_C3,
				 OSD_DEFAULT_LPF_C4, OSD_DEFAULT_LPF_C5);
}

INT32 OSD_SC_Update(uint32_t plane)
{
	if (_OSD_SC_GetUpdateStatus(plane)) {
		_OSD_SC_Update(plane);
		_OSD_SC_SetUpdateStatus(plane, false);
	}

	return (INT32) OSD_RET_OK;
}

int osd_premix_config(struct osd_resolution_change *res_chg)
{
	uint32_t u4HTotal, u4VTotal;

	osd_get_res_total_size(res_chg, OSD_MAIN_PATH, &u4HTotal, &u4VTotal);

	osd_pmx_set_shadow_en(1);

	/*get vtotal, htotal*/
	/*osd_pmx_set_rg_htotal(u4HTotal); */
	/*osd_pmx_set_rg_vtotal(u4VTotal); */

	osd_pmx_set_rg_pln0_max_a_sel(1);
	osd_pmx_set_rg_pln1_max_a_sel(1);
	osd_pmx_set_rg_pln2_max_a_sel(1);

	osd_pmx_set_rg_max_a_sel(1);
	osd_pmx_set_rg_max_a_sel_div(1);

	osd_pmx_set_mix1_de_sel(1); /*default fhd*/
	osd_pmx_set_pln0_msk(1);

	/* osd_premix_rgb2bgr(1); */

	osd_pmx_set_shadow_update(1);
	return 0;
}

void osd_premix_rgb2bgr(uint32_t rgb2bgr)
{
	if (rgb2bgr) {
		osd_pmx_set_plane0_y_sel(1);  /*y-> cb*/
		osd_pmx_set_plane0_cb_sel(2); /*cb-> cr*/
		osd_pmx_set_plane0_cr_sel(0); /*cr-> y*/

		osd_pmx_set_plane1_y_sel(1);  /*y-> cb*/
		osd_pmx_set_plane1_cb_sel(2); /*cb-> cr*/
		osd_pmx_set_plane1_cr_sel(0); /*cr-> y*/

		osd_pmx_set_plane2_y_sel(1);  /*y-> cb*/
		osd_pmx_set_plane2_cb_sel(2); /*cb-> cr*/
		osd_pmx_set_plane2_cr_sel(0); /*cr-> y*/
	} else {
		osd_pmx_set_plane0_y_sel(0);
		osd_pmx_set_plane0_cb_sel(1);
		osd_pmx_set_plane0_cr_sel(2);

		osd_pmx_set_plane1_y_sel(0);
		osd_pmx_set_plane1_cb_sel(1);
		osd_pmx_set_plane1_cr_sel(2);

		osd_pmx_set_plane2_y_sel(0);
		osd_pmx_set_plane2_cb_sel(1);
		osd_pmx_set_plane2_cr_sel(2);
	}
}

void osd_premix_window_config(struct disp_osd_layer_config *buffer)
{
	unsigned int i;

	for (i = 0; i < OSD_PLANE_MAX_WINDOW; i++) {
		if (buffer->win[i].active == true) {
			if (i == 0) {
				osd_pmx_set_rg_win1_en(1);
				osd_pmx_set_rg_win1_hst(buffer->win[i].x);
				osd_pmx_set_rg_win1_vst(buffer->win[i].y);
				osd_pmx_set_rg_win1_hend(buffer->win[i].x + buffer->win[i].width - 1);
				osd_pmx_set_rg_win1_vend(buffer->win[i].y + buffer->win[i].height - 1);
			} else {
				osd_pmx_set_rg_win2_en(1);
				osd_pmx_set_rg_win2_hst(buffer->win[i].x);
				osd_pmx_set_rg_win2_vst(buffer->win[i].y);
				osd_pmx_set_rg_win2_hend(buffer->win[i].x + buffer->win[i].width - 1);
				osd_pmx_set_rg_win2_vend(buffer->win[i].y + buffer->win[i].height - 1);
			}
		} else {/*no window*/
			if (i == 0) {
				osd_pmx_set_rg_win1_en(0);
				osd_pmx_set_rg_win1_hst(0);
				osd_pmx_set_rg_win1_vst(0);
				osd_pmx_set_rg_win1_hend(0);
				osd_pmx_set_rg_win1_vend(0);
			} else {
				osd_pmx_set_rg_win2_en(0);
				osd_pmx_set_rg_win2_hst(0);
				osd_pmx_set_rg_win2_vst(0);
				osd_pmx_set_rg_win2_hend(0);
				osd_pmx_set_rg_win2_vend(0);
			}
		}
		/*_OSD_BASE_SetPremixUpdate(buffer->layer_id, 1);*/
		osd_pmx_set_window_update(buffer->layer_id, true);
	}
}

