/********************************************************************************************
*LEGALDISCLAIMER
*
*(HeaderofMediaTekSoftware/FirmwareReleaseorDocumentation)
*
*BYOPENINGORUSINGTHISFILE, BUYERHEREBYUNEQUIVOCALLYACKNOWLEDGESANDAGREES
*THATTHESOFTWARE/FIRMWAREANDITSDOCUMENTATIONS("MEDIATEKSOFTWARE")RECEIVED
*FROMMEDIATEKAND/ORITSREPRESENTATIVESAREPROVIDEDTOBUYERONAN"AS-IS"BASIS
*ONLY.MEDIATEKEXPRESSLYDISCLAIMSANYANDALLWARRANTIES, EXPRESSORIMPLIED,
*INCLUDINGBUTNOTLIMITEDTOTHEIMPLIEDWARRANTIESOFMERCHANTABILITY, FITNESSFOR
*APARTICULARPURPOSEORNONINFRINGEMENT.NEITHERDOESMEDIATEKPROVIDEANYWARRANTY
*WHATSOEVERWITHRESPECTTOTHESOFTWAREOFANYTHIRDPARTYWHICHMAYBEUSEDBY,
*INCORPORATEDIN, ORSUPPLIEDWITHTHEMEDIATEKSOFTWARE, ANDBUYERAGREESTOLOOK
*ONLYTOSUCHTHIRDPARTYFORANYWARRANTYCLAIMRELATINGTHERETO.MEDIATEKSHALLALSO
*NOTBERESPONSIBLEFORANYMEDIATEKSOFTWARERELEASESMADETOBUYER'SSPECIFICATION
*ORTOCONFORMTOAPARTICULARSTANDARDOROPENFORUM.
*
*BUYER'SSOLEANDEXCLUSIVEREMEDYANDMEDIATEK'SENTIREANDCUMULATIVELIABILITYWITH
*RESPECTTOTHEMEDIATEKSOFTWARERELEASEDHEREUNDERWILLBE, ATMEDIATEK'SOPTION,
*TOREVISEORREPLACETHEMEDIATEKSOFTWAREATISSUE, ORREFUNDANYSOFTWARELICENSE
*FEESORSERVICECHARGEPAIDBYBUYERTOMEDIATEKFORSUCHMEDIATEKSOFTWAREATISSUE.
*
*THETRANSACTIONCONTEMPLATEDHEREUNDERSHALLBECONSTRUEDINACCORDANCEWITHTHELAWS
*OFTHESTATEOFCALIFORNIA, USA, EXCLUDINGITSCONFLICTOFLAWSPRINCIPLES.
************************************************************************************************/


#ifndef _PP_DRV_H_
#define _PP_DRV_H_
#define CONFIG_SUSPEND_TO_DRAM0

#include "disp_pp_if.h"
#include "hdmitx.h"


/****************************************************************************
*GeneralConstants
****************************************************************************/
void pp_enable(bool enable);
extern int32_t pp_drv_init(void);
extern int32_t pp_drv_uninit(void);
extern void pp_hal_set_path(uint8_t path);
extern void pp_hal_color_proc_bypass(int16_t mode);
extern void pp_hal_set_sharp_lti_band_param(int16_t mode, int16_t _gain);
extern void pp_hal_set_sharp_band1_param(int8_t gain, bool enable);

/***AdaptiveLuma***/
extern void pp_hal_auto_con_enable(uint8_t b_on_off);
extern void pp_auto_con_init(void);
extern void pp_auto_contrast(void);
extern void pp_get_hist_info(void);
extern void pp_update_luma_curve(void);
extern void pp_set_luma_curve(void);

/*----AdaptiveSCE-----*/
extern void pp_hal_auto_sat(void);
extern void pp_auto_con_set_dft(void);
extern uint8_t pp_get_norm_hist(uint32_t *p_hist);
extern uint8_t pp_get_apl_value(void);
extern uint8_t pp_get_luma_max(void);
extern uint8_t pp_get_luma_min(void);
extern void pp_hal_set_bs_param(uint8_t item, uint8_t value);
extern void pp_hal_set_ws_param(uint8_t item, uint8_t value);
extern void pp_hal_set_al_param(uint8_t item, uint8_t value);

/***CTI***/
extern void pp_hal_cti_enable(uint8_t b_on_off);
extern void pp_hal_cti_update(void);

/***2DSHARP***/
extern void pp_hal_sharp_enable(uint8_t b_on_off);
extern void pp_hal_update_sharp(void);

/***CDS***/
extern void pp_hal_cds_enable(uint8_t b_on_off);
extern void pp_hal_cds_update(void);

/***CONTRAST***/
extern void pp_hal_update_main_con(uint8_t value);

/***BRIGHTNESS***/
extern void pp_hal_update_main_bri(uint8_t value);

/***SATURATION***/
extern void pp_hal_update_main_sat(uint8_t value);

/***HUE***/
extern void pp_hal_update_main_hue(uint8_t value);

/***SCE***/
extern void pp_hal_main_sec_on_off(uint8_t b_on_off);
extern void pp_hal_set_sce_table(enum POST_VIDEO_MODE_ENUM emode);
extern void pp_hal_sce_init(void);

/*****************************************************************************************/
/*************************************LTI&HLTI*******************************************/
/*****************************************************************************************/
void pp_hal_lti_init(HDMI_VIDEO_RESOLUTION e_res);

/*****************************************************************************************/
/*************************************HVBand*******************************************/
/*****************************************************************************************/
void pp_hal_hv_band_init(HDMI_VIDEO_RESOLUTION e_res);

/*****************************************************************************************/
/*************************************Adap*******************************************/
/*****************************************************************************************/
void pp_hal_adap_init(HDMI_VIDEO_RESOLUTION e_res);

/***Misc***/
extern void pp_hal_yc_proc_init(void);
extern void pp_hal_chg_res(HDMI_VIDEO_RESOLUTION e_res);
extern void pp_hal_set_scered_y4(uint8_t value);
extern void pp_hal_set_scered_y5(uint8_t value);
extern void pp_hal_set_scered_s4(uint8_t value);
extern void pp_hal_set_scered_s5(uint8_t value);
extern void pp_hal_set_scered_h4(uint8_t value);
extern void pp_hal_set_scered_h5(uint8_t value);
extern void pp_hal_set_scegreen_y10(uint8_t value);
extern void pp_hal_set_scegreen_y11(uint8_t value);
extern void pp_hal_set_scegreen_s10(uint8_t value);
extern void pp_hal_set_scegreen_s11(uint8_t value);
extern void pp_hal_set_scegreen_h10(uint8_t value);
extern void pp_hal_set_scegreen_h11(uint8_t value);
extern void pp_hal_set_sceblue_y14(uint8_t value);
extern void pp_hal_set_sceblue_y15(uint8_t value);
extern void pp_hal_set_sceblue_s14(uint8_t value);
extern void pp_hal_set_sceblue_s15(uint8_t value);
extern void pp_hal_set_sceblue_h14(uint8_t value);
extern void pp_hal_set_sceblue_H15(uint8_t value);
extern void pp_hal_set_sceyellow_y7(uint8_t value);
extern void pp_hal_set_sceyellow_y8(uint8_t value);
extern void pp_hal_set_sceyellow_s7(uint8_t value);
extern void pp_hal_set_sceyellow_s8(uint8_t value);
extern void pp_hal_set_sceyellow_h7(uint8_t value);
extern void pp_hal_set_sceyellow_h8(uint8_t value);
extern void pp_hal_set_scecyan_y12(uint8_t value);
extern void pp_hal_set_scecyan_y13(uint8_t value);
extern void pp_hal_set_scecyan_s12(uint8_t value);
extern void pp_hal_set_scecyan_s13(uint8_t value);
extern void pp_hal_set_scecyan_h12(uint8_t value);
extern void pp_hal_set_scecyan_h13(uint8_t value);
extern void pp_hal_set_scemagenta_y2(uint8_t value);
extern void pp_hal_set_scemagenta_y3(uint8_t value);
extern void pp_hal_set_scemagenta_s2(uint8_t value);
extern void pp_hal_set_scemagenta_s3(uint8_t value);
extern void pp_hal_set_scemagenta_h2(uint8_t value);
extern void pp_hal_set_scemagenta_h3(uint8_t value);

#endif				/*#ifndef _PP_T ASK_H_ */
