#ifndef __HDR_CLI_H__
#define __HDR_CLI_H__


#include "disp_hw_mgr.h"

struct hdr_cli_info_struct {
	uint32_t logLevel; /* define log level */
	uint32_t gainValue; /* sdr2hdr gain value [0~256], if set to 0xffffffff, use TV gain value */
	int updateHDR; /* set to 1 update HDR HW reigster */
	bool disable_path_clock_control; /* disable path & clock new function control. */
	uint32_t bypassModule; /* set 0: not take effect, BT2020:0x001, SDR2HDR:0x010, HDR2SDR:0x100 */
};

struct hdr_irq_info_struct {
	int irqid;
	int hightrigger;
};

void hdr_cli_init(void);
struct hdr_cli_info_struct *hdr_cli_get_info(void);
int _hdr_core_handle_disp_config(struct mtk_disp_buffer *pConfig, struct disp_hw_common_info *pInfo);


#endif /* endof __HDR_CLI_H__ */

