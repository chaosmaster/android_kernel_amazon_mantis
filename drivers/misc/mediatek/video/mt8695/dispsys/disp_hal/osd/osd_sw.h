#ifndef __DRV_OSD_SW_H__
#define __DRV_OSD_SW_H__

#include "disp_osd_env.h"
#include "osd_hw.h"
#include "disp_def.h"

extern struct device *mtkfb_dev;

#ifdef CONFIG_OF
extern struct clk *osd_clk_map;
#endif

/* region function */
void osd_region_init(void);
void osd_region_uninit(void);
INT32 osd_get_free_region(UINT32 u4Plane, UINT32 *u4region);
void osd_set_region_state(UINT32 u4Plane);
INT32 osd_reset_plane_region_state(UINT32 u4Plane);
INT32 OSD_RGN_Get(UINT32 u4Region, INT32 i4Cmd, UINT32 *pu4Value);
INT32 OSD_RGN_Set(UINT32 u4Region, INT32 i4Cmd, UINT32 u4Value);
INT32 OSD_RGN_Create_EX(UINT32 u4Region, UINT32 u4SrcDispX,
			UINT32 u4SrcDispY, UINT32 s_width, UINT32 s_height,
			unsigned int pvBitmap, UINT32 eColorMode, UINT32 u4BmpPitch,
			UINT32 u4DispX, UINT32 u4DispY,
			UINT32 u4DispW, UINT32 u4DispH,
			UINT32 ui4Plane, uint32_t index, UINT32 u4AlphaEn, uint32_t alpha, uint32_t fmt_order);
INT32 OSD_RGN_SetDisplayWidth(UINT32 u4Region, UINT32 u4Width);
INT32 OSD_RGN_SetDisplayHeight(UINT32 u4Region, UINT32 u4Height);
INT32 OSD_RGN_SetBigEndian(UINT32 u4Region, BOOL fgBE);


/*INT32 i4Osd_BaseSetOsdPath(UINT32 u4Plane, UINT32 u4Path);*/
int i4Osd_BaseSetFmt(uint32_t plane, uint32_t u4Path, struct osd_resolution_change *res_chg, bool fgFlip);
INT32 _OSD_BASE_GetPathPrgs(UINT32 u4DisplayMode, BOOL *pfgProgressive);
INT32 _OSD_BASE_SetOsdPrgs(UINT32 u4Plane, UINT32 u4ProgressFlag);


/* plane function */
void OSD_PLA_DisableAllPlane(uint32_t i);
BOOL OSD_Is_PLA_Enabled(UINT32 u4Plane);
INT32 OSD_PLA_Reset(UINT32 u4Plane);
UINT32 osd_plane_map_region(UINT32 u4Plane);
INT32 OSDSetColorSpace(UINT8 ui1ClrSpa);
INT32 OSDGetColorSpace(UINT8 *ui1ClrSpa);
INT32 OSD_PLA_Update(uint32_t plane);
void OSD_PLA_EnablePlane(void);
void osd_plane_enable(unsigned int plane, bool enable);
INT32 OsdFlipPlaneRightNow(UINT32 u4Plane, BOOL fgValidReg, UINT32 u4Region, unsigned int idx);
INT32 OSD_PLA_FlipTo(UINT32 u4Plane, UINT32 u4Region, unsigned int idx);
INT32 OSD_PLA_FlipToNone(UINT32 u4Plane);
void osd_alpha_detect_enable(UINT32 u4Plane, BOOL fgEnable, unsigned int bmp_height);


/* scaler function */
INT32 OSD_SC_CheckCapability(UINT32 u4SrcW, UINT32 u4SrcH, UINT32 u4DstW, UINT32 u4DstH);

INT32 OSD_SC_Scale(UINT32 u4Scaler, UINT32 u4Enable, UINT32 u4SrcWidth,
		   UINT32 u4SrcHeight, UINT32 uDstX, UINT32 uDstY, UINT32 u4DstWidth, UINT32 u4DstHeight);
INT32 OSD_SC_ReCfgAll(VOID);
INT32 OSD_SC_SetLpfInfo(UINT32 u4Scaler, UINT32 u4Enable, INT16 i2C1,
			INT16 i2C2, INT16 i2C3, INT16 i2C4, INT16 i2C5);

INT32 OSD_SC_SetLpf(UINT32 u4Scaler, UINT32 u4Enable);

INT32 OSD_SC_Update(uint32_t plane);


/* just for check patch */
int disp_osd_config_kthread(void *data);
int disp_osd_irq_kthread(void *data);

int osd_config_sw_register(struct disp_osd_layer_config *osd_overlay_buffer_info);
int osd_reconfig_sw_register(uint32_t plane_id, uint32_t region_id,
		struct disp_osd_layer_config *osd_overlay_buffer_info);

int disp_osd_update_register(uint32_t plane);
extern OSD_REGION_NODE_T _rAllRgnNode[];
/*extern BOOL fg_change_resolution;*/

int osd_premix_config(struct osd_resolution_change *res_chg);
void osd_premix_rgb2bgr(uint32_t rgb2bgr);
void osd_premix_window_config(struct disp_osd_layer_config *buffer);

#endif
