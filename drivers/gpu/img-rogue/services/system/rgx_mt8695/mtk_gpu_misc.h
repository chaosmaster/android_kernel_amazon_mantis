#ifndef _MTK_GPU_MISC_H_
#define _MTK_GPU_MISC_H_

#include "pvrsrv_device.h"

void mtk_gpu_misc_init(PVRSRV_DEVICE_CONFIG *pvr_cfg);
u32 mtk_gpu_misc_get_util(u64 *high, u64 *low, u64 *blk, u64 *total);

bool mtk_get_gpu_loading(unsigned int *pLoading);

bool mtk_get_gpu_block(unsigned int *pLoading);

bool mtk_get_gpu_idle(unsigned int *pLoading);

bool mtk_get_gpu_memory_usage(unsigned int *pLoading);

bool mtk_get_gpu_power_loading(unsigned int *pLoading);

unsigned int mt_gpufreq_get_cur_freq(void);

unsigned int mt_gpufreq_get_cur_volt(void);

unsigned int mt_gpufreq_get_thermal_limit_freq(void);

void mt_gpufreq_thermal_protect(unsigned int power);

typedef void (*met_gpu_power_change_notify_fp)(int power_on);

bool mtk_register_gpu_power_change(const char *name, met_gpu_power_change_notify_fp callback);

bool mtk_unregister_gpu_power_change(const char *name);

void mtk_gpu_misc_met_nfy_power_state(int power_on);
void mt_gpufreq_thermal_protect(unsigned int power);
struct mtk_gpu_power_info {
	unsigned int gpufreq_khz;
	unsigned int gpufreq_power;
};
extern int mtk_gpufreq_register(struct mtk_gpu_power_info *freqs, int num);
void mt_gpufreq_set_thermal_limit_freq(unsigned long freq);
unsigned int mt_gpufreq_get_thermal_limit_freq_Khz(void);


void mtk_gpu_thermal_setup(void);
#endif
