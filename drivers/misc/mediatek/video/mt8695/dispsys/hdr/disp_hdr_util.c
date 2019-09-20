#include "disp_hdr_util.h"
#include "disp_hdr_device.h"


unsigned int hdr_util_translate_s3p13(int data)
{
	unsigned int result = 0;

	if (data >= 0)
		result = ((data/8192) << 13) | ((data%8192) << 0);
	else
		result = (((65536 + data)/8192) << 13) | (((65536 + data)%8192) << 0);

	return result;
}

enum HDR_STATUS hdr_write_register(char *pAddr, uint32_t value, bool printLog)
{
	if (pAddr == NULL) {
		HDR_ERR("hdr_write_register NULL pointer error\n");
		return HDR_STATUS_NULL_POINTER;
	}

	*(volatile uint32_t *)(pAddr) = value;
	if (printLog) {
		uint32_t registerPA = 0;

		hdr_device_map_VA_2_PA(pAddr, &registerPA);
		HDR_LOG("WRITE REG:0x%08x VALUE:0x%08x\n", registerPA, value);
	}
	return HDR_STATUS_OK;
}

/* mask: if you want to modify one bit, set this bit to 1 */
enum HDR_STATUS hdr_write_register_with_mask(char *pAddr, uint32_t value, uint32_t mask, bool printLog)
{
	if (pAddr == NULL) {
		HDR_ERR("hdr_write_register NULL pointer error\n");
		return HDR_STATUS_NULL_POINTER;
	}

	*(volatile uint32_t *)(pAddr) = (value & mask) | (*(volatile uint32_t *)(pAddr) & ~mask);
	if (printLog) {
		uint32_t registerPA = 0;

		hdr_device_map_VA_2_PA(pAddr, &registerPA);
		HDR_LOG("WRITE REG:0x%x VALUE:0x%x mask:0x%x\n", registerPA, value, mask);
	}
	return HDR_STATUS_OK;
}

uint32_t hdr_read_register(char *pAddr)
{
	return *(volatile uint32_t *)(pAddr);
}

const char *hdr_print_log_level(enum HDR_LOG_LEVEL level)
{
	switch (level) {
	case HDR_LOG_LEVEL_ERR:
		return "HDR_ERR";
	case HDR_LOG_LEVEL_DBG:
		return "HDR_DBG";
	case HDR_LOG_LEVEL_LOG:
		return "HDR_LOG";
	case HDR_LOG_LEVEL_TONE_MAP:
		return "HDR_TONE_MAP";
	default:
		return NULL;
	}
	return NULL;
}

#if 0
char *hdr_util_map_PA_2_VA(uint32_t registerPA)
{
	return __va(registerPA);
}

uint32_t hdr_util_map_VA_2_PA(char *registerVA)
{
	return __pa(registerVA);
}
#endif
