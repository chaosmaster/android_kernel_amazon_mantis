#ifndef _MTK_GPU_TEHRMAL_H_
#define _MTK_GPU_THERMAL_H_


static int pvr_set_freq_level(int freq);
static unsigned int pvr_set_max_level(void);
//static int get_gpu_max_clk_level(void);
static void pvr_set_scale_max(unsigned int idx);
int mtk_gpufreq_register(struct mtk_gpu_power_info *freqs, int num);
void mtk_gpu_thermal_init(void);
int mtk_gpufreq_power_table_avail(void);
extern void mt_gpufreq_set_thermal_limit_freq(unsigned long freq);
int mtk_gpufreq_register(struct mtk_gpu_power_info *freqs, int num);
#endif
