/*
 * mtk_gpu_thermal.c
 *
 * gpufreq throttling initialization
 *
 * Created on: May 4, 2018
 *      Author: Amazon
 */

#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/pm.h>
#include <linux/of.h>
#include <linux/module.h>            /* kernel module definitions */
#include <linux/slab.h>
#ifdef CONFIG_GPU_THERMAL
#include <linux/gpu_cooling.h>
#endif

#include "mtk_gpu_misc.h"
#include "mtk_gpu_thermal.h"

struct mtk_pvr_gpufreq_data{
	struct mtk_gpu_power_info *power_table;
	int power_table_avail;
	int gpu_opp_num;
	int max_state;
	int scale_max;
	int scale_max_freq;
	unsigned int cur_state;
	unsigned int cur_freq;
};

//static struct mtk_gpu_power_info *mtk_gpu_power;
//static int Num_of_GPU_OPP;
static struct mtk_pvr_gpufreq_data g_pvr_gpufreq = {
			.power_table_avail = 0,
};

static struct gpufreq_cooling_device *g_gcdev;

/* gpufreq  power_table has smaller frequencey level while index increase */
static int pvr_get_freq_level(int freq)
{
	int level = 0;

	if (!g_pvr_gpufreq.power_table) {
		pr_err("%s: gpu power table not ready\n", __func__);
		return -EINVAL;
	}

	for (level = 0; level <= g_pvr_gpufreq.max_state; level++) {
		if (freq == g_pvr_gpufreq.power_table[level].gpufreq_khz)
			return level;

		if (freq > g_pvr_gpufreq.power_table[level+1].gpufreq_khz &&
				freq < g_pvr_gpufreq.power_table[level].gpufreq_khz)
			return (level + 1);

		/* check freq outside of available frequency later
		   because ideally this should not happen */
		if (freq > g_pvr_gpufreq.power_table[0].gpufreq_khz) {
			pr_info("%s: frequency is larger than freq max, set to level 0\n",
					__func__);
			return 0;
		}

		if (freq < g_pvr_gpufreq.power_table[g_pvr_gpufreq.max_state].gpufreq_khz) {
			pr_info("%s: frequency is less than freq min , set to level max:%d\n",
					__func__, g_pvr_gpufreq.max_state);
			return g_pvr_gpufreq.max_state;
		}
	}

	pr_err("%s: cannot find frequency level for freq:%d\n", __func__, freq);

	return -1;
}

static unsigned int pvr_get_max_level(void)
{
    return g_pvr_gpufreq.max_state;
}

#if 0
int get_gpu_max_clk_level(void)
{
    return (g_pvr_gpufreq.power_table[g_pvr_gpufreq.max_state].gpufreq_khz);
}
#endif

static void pvr_set_scale_max(unsigned int idx)
{
	if (!g_pvr_gpufreq.power_table) {
		pr_err("%s: gpu power table not ready\n", __func__);
		return -EINVAL;
	}

	if (idx > g_pvr_gpufreq.max_state || idx < 0) {
		pr_err("%s: scale clock idx is out of range\n", __func__);
        return;
	}

	g_pvr_gpufreq.scale_max = idx;
	g_pvr_gpufreq.scale_max_freq = g_pvr_gpufreq.power_table[idx].gpufreq_khz;
    mt_gpufreq_set_thermal_limit_freq(g_pvr_gpufreq.scale_max_freq);
}

static int pvr_get_scale_max(void)
{
    return g_pvr_gpufreq.scale_max;
}

int mtk_gpufreq_register(struct mtk_gpu_power_info *freqs, int num)
{
	int i = 0;

	pr_info("mtk_gpufreq_register\n");

	g_pvr_gpufreq.power_table = kzalloc((num) * sizeof(struct mtk_gpu_power_info),
										GFP_KERNEL);
	if (!g_pvr_gpufreq.power_table)
		return -ENOMEM;

	for (i = 0; i < num; i++) {
		g_pvr_gpufreq.power_table[i].gpufreq_khz = freqs[i].gpufreq_khz;
		g_pvr_gpufreq.power_table[i].gpufreq_power = freqs[i].gpufreq_power;

		pr_info("[%d].gpufreq_khz=%d, .gpufreq_power=%d\n",
				i, g_pvr_gpufreq.power_table[i].gpufreq_khz,
				g_pvr_gpufreq.power_table[i].gpufreq_power);
	}

	g_pvr_gpufreq.gpu_opp_num = num;	/* GPU OPP count */
	g_pvr_gpufreq.max_state = num - 1;
	g_pvr_gpufreq.power_table_avail = 1;

	return 0;
}
EXPORT_SYMBOL(mtk_gpufreq_register);

int mtk_gpufreq_power_table_avail(void)
{
	return (g_pvr_gpufreq.power_table_avail == 1) ? 1 : 0;
}
EXPORT_SYMBOL(mtk_gpufreq_power_table_avail);

void mtk_gpu_thermal_init(void)
{
    int err;

    g_gcdev = gpufreq_cooling_alloc();
    //register_gpu_freq_info(get_current_frequency);
    if (IS_ERR(g_gcdev))
		pr_err("%s: malloc gpu cooling buffer error!!\n", __func__);
    else if (!g_gcdev)
        printk("%s: system does not enable thermal driver\n", __func__);
    else {
        g_gcdev->get_gpu_freq_level = pvr_get_freq_level;
        g_gcdev->get_gpu_max_level = pvr_get_max_level;
        g_gcdev->set_gpu_freq_idx = pvr_set_scale_max;
        g_gcdev->get_gpu_current_max_level = pvr_get_scale_max;

#ifdef CONFIG_GPU_THERMAL
		err = gpufreq_cooling_register(g_gcdev);
        if (err < 0)
            printk("register GPU  cooling error\n");
#endif
        pr_info("gpu cooling register okay with err=%d\n",err);
    }

}
EXPORT_SYMBOL(mtk_gpu_thermal_init);

