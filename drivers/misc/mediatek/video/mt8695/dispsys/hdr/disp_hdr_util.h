#ifndef __HDR_UTIL_H__
#define __HDR_UTIL_H__
#include "disp_hdr_def.h"
#include "disp_hdr_cli.h"
#include "disp_hdr_device.h"


uint32_t hdr_util_translate_s3p13(int data);

enum HDR_STATUS hdr_write_register(char *pAddr, uint32_t value, bool printLog);
enum HDR_STATUS hdr_write_register_with_mask(char *pAddr, uint32_t value, uint32_t mask, bool printLog);
uint32_t hdr_read_register(char *pAddr);


/* HDR LOG related */
enum HDR_LOG_LEVEL {
	HDR_LOG_LEVEL_ERR = 1 << 0,
	HDR_LOG_LEVEL_DBG = 1 << 1,
	HDR_LOG_LEVEL_LOG = 1 << 2,
	HDR_LOG_LEVEL_TONE_MAP = 1 << 3,
	HDR_LOG_LEVEL_MAX = 1 << 4, /* note to increase when add new log level */
};
const char *hdr_print_log_level(enum HDR_LOG_LEVEL level);

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
#define HDR_PRINTF(level, format, ...) do { \
	if (hdr_cli_get_info()->logLevel & level) \
		pr_err("%s: "format, hdr_print_log_level(level), ##__VA_ARGS__); \
} while (0)
#else
#define HDR_PRINTF(level, format, ...) do { \
	if (HDR_LOG_LEVEL_ERR & level) \
		pr_err("%s: "format, hdr_print_log_level(level), ##__VA_ARGS__); \
} while (0)
#endif

#define HDR_ERR(format, ...) HDR_PRINTF(HDR_LOG_LEVEL_ERR, format, ##__VA_ARGS__)
#define HDR_DBG(format, ...) HDR_PRINTF(HDR_LOG_LEVEL_DBG, format, ##__VA_ARGS__)
#define HDR_LOG(format, ...) HDR_PRINTF(HDR_LOG_LEVEL_LOG, format, ##__VA_ARGS__)
#define HDR_TONE_LOG(format, ...) HDR_PRINTF(HDR_LOG_LEVEL_TONE_MAP, format, ##__VA_ARGS__)

#if 0
char *hdr_util_map_PA_2_VA(uint32_t registerPA);
uint32_t hdr_util_map_VA_2_PA(char *registerVA);
#endif


#endif /* endof __HDR_UTIL_H__ */
