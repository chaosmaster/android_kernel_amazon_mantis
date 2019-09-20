#include <linux/slab.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/cpu_cooling.h>
#include <linux/cpucore_cooling.h>
#include <linux/gpu_cooling.h>
#include <linux/wifi_cooling.h>
#include "thermal_core.h"
//#include <linux/opp.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/thermal_framework.h>

struct amzn_thermal {
	int cool_dev_num;
	//int min_exist;
	struct amzn_cool_dev *cool_devs;
	struct thermal_zone_device	*tzd;
};

enum amzn_cool_dev_type {
	COOL_DEV_TYPE_CPU_FREQ = 0,
	COOL_DEV_TYPE_CPU_CORE,
	COOL_DEV_TYPE_GPU_FREQ,
	COOL_DEV_TYPE_GPU_CORE,
	COOL_DEV_TYPE_WIFI,
	COOL_DEV_TYPE_MAX,
};

struct amzn_cool_dev {
	int min_state;
	int cluster_id;
	int coeff;
	enum amzn_cool_dev_type device_type;
	struct device_node *np;
	struct thermal_cooling_device *cooling_dev;
};

static struct amzn_thermal amznt;

extern struct thermal_cooling_device *wifi_cooling_register(struct device_node *np, int id);
extern void mtk_gpu_thermal_init(void);
extern int mtk_gpufreq_power_table_avail(void);

static int get_cool_dev_type(const char *type)
{
	if (!strcmp(type, "cpufreq"))
		return COOL_DEV_TYPE_CPU_FREQ;
	if (!strcmp(type, "cpucore"))
		return COOL_DEV_TYPE_CPU_CORE;
	if (!strcmp(type, "gpufreq"))
		return COOL_DEV_TYPE_GPU_FREQ;
	if (!strcmp(type, "gpucore"))
		return COOL_DEV_TYPE_GPU_CORE;
	if (!strcmp(type, "wifi"))
		return COOL_DEV_TYPE_WIFI;
	return COOL_DEV_TYPE_MAX;
}

static int amzn_register_cool_dev(struct amzn_cool_dev *cool)
{
	switch (cool->device_type) {
	case COOL_DEV_TYPE_CPU_CORE:
		cool->cluster_id = 0;
		cool->cooling_dev = cpucore_cooling_register(cool->np,
							cool->cluster_id);
		break;

	case COOL_DEV_TYPE_CPU_FREQ:
		cool->cooling_dev = of_cpufreq_cooling_register(cool->np,
							cpu_present_mask);
		break;

	/* GPU is KO, just save these parameters */
	case COOL_DEV_TYPE_GPU_FREQ:
		/* if (of_property_read_u32(cool->np, "num_of_pp", &pp))
		   pr_err("thermal: read num_of_pp failed\n"); */
		/* pp(max_pp) is not used on, set to 0. */
		save_gpu_cool_para(cool->coeff, cool->np, 0);
		mtk_gpu_thermal_init();
		break;

	case COOL_DEV_TYPE_GPU_CORE:
		return 0;

	case COOL_DEV_TYPE_WIFI:
		cool->cooling_dev = wifi_cooling_register(cool->np, 0);
		return 0;

	default:
		pr_err("thermal: unknown cooling device type:%d\n", cool->device_type);
		return -EINVAL;
	}

	if (IS_ERR(cool->cooling_dev)) {
		pr_err("thermal: register cooling device type %d failed\n", cool->device_type);
		return -EINVAL;
	}

	return 0;
}

int amzn_parse_register_cool_device(struct device_node *np, struct amzn_cool_dev *cool)
{
	int temp, ret = 0;
	struct device_node *node, *child = NULL;
	const char *str;

	while ((child = of_get_next_child(np, child)) != NULL) {
		if (of_property_read_string(child, "device_type", &str))
			pr_err("thermal: read device_type failed\n");
		else
			cool->device_type = get_cool_dev_type(str);

		if (of_property_read_u32(child, "min_state", &temp))
			pr_err("thermal: read min_state failed\n");
		else
			cool->min_state = temp;

		if (cool->device_type == COOL_DEV_TYPE_CPU_FREQ ||
			cool->device_type == COOL_DEV_TYPE_CPU_CORE ||
			cool->device_type == COOL_DEV_TYPE_GPU_FREQ) {
			if (of_property_read_u32(child, "dyn_coeff", &temp))
				pr_err("thermal: read dyn_coeff failed\n");
			else
				cool->coeff = temp;
		}

		if (of_property_read_string(child, "node_name", &str))
			pr_err("thermal: read node_name failed\n");
		else {
			node = of_find_node_by_name(NULL, str);
			if (!node)
				pr_err("thermal: can't find node :%s\n", str);
			cool->np = node;
		}
		if (cool->np) {
			ret |= amzn_register_cool_dev(cool);
			pr_info("thermal: register cooling device type %d ret %d\n", cool->device_type, ret);
		}
		cool++;

	}
	return ret;
}

static struct amzn_cool_dev *get_cool_dev_by_node(struct device_node *np)
{
	int i;
	struct amzn_cool_dev *dev;

	if (!np)
		return NULL;
	for (i = 0; i < amznt.cool_dev_num; i++) {
		dev = &amznt.cool_devs[i];
		if (dev->np == np)
			return dev;
	}
	return NULL;
}
/* Currently does not make any effect, was used to update ins->upper for IPA governor */
int amzn_thermal_min_update(struct thermal_cooling_device *cdev)
{
	//struct gpufreq_cooling_device *gf_cdev;
	//struct thermal_instance *ins;
	struct amzn_cool_dev *cool;
	long min_state;
	int cpu = 0;

	cool = get_cool_dev_by_node(cdev->np);
	if (!cool)
		return -ENODEV;

	if (cool->cooling_dev == NULL)
		cool->cooling_dev = cdev;

	if (cool->min_state == 0)
		return 0;

	switch (cool->device_type) {
	case COOL_DEV_TYPE_CPU_CORE:
		//cool->cooling_dev->ops->get_max_state(cdev, &min_state);
		//min_state = min_state - cool->min_state;
		break;

	case COOL_DEV_TYPE_CPU_FREQ:
		min_state = cpufreq_cooling_get_level(cpu, cool->min_state);
		break;

	case COOL_DEV_TYPE_GPU_FREQ:
		//gf_cdev = (struct gpufreq_cooling_device *)cdev->devdata;
		//min_state = gf_cdev->get_gpu_freq_level(cool->min_state);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL(amzn_thermal_min_update);

static int amzn_thermal_probe(struct platform_device *pdev)
{
	int i;
	struct device_node *np, *child;
	struct amzn_cool_dev *cool;


	if (!cpufreq_frequency_get_table(0)) {
		dev_info(&pdev->dev,
			"Frequency table not initialized. Deferring probe...\n");
		return -EPROBE_DEFER;
	}

	if (!mtk_gpufreq_power_table_avail()) {
		dev_info(&pdev->dev,
				"GPU Power table not initialized. Deferring probe. \n");
		return -EPROBE_DEFER;
	}

	np = pdev->dev.of_node;
	child = of_get_child_by_name(np, "cooling_devices");
	if (child == NULL) {
		pr_err("thermal: can't found cooling_devices\n");
		return -EINVAL;
	}
	amznt.cool_dev_num = of_get_child_count(child);
	pr_info("%s: found %d cooling devices defined\n", __func__, amznt.cool_dev_num);

	amznt.cool_devs = kzalloc(sizeof(struct amzn_cool_dev)*(amznt.cool_dev_num), GFP_KERNEL);
	if (amznt.cool_devs == NULL) {
		pr_err("thermal: alloc mem failed\n");
		return -ENOMEM;
	}

	if (amzn_parse_register_cool_device(child, &amznt.cool_devs[0]))
		return -EINVAL;

	/* update min state for each device */
	for (i = 0; i < amznt.cool_dev_num; i++) {
		cool = &amznt.cool_devs[i];
		if (cool->cooling_dev)
			amzn_thermal_min_update(cool->cooling_dev);
	}


	return 0;
}

static int amzn_thermal_remove(struct platform_device *pdev)
{
	kfree(amznt.cool_devs);
	return 0;
}

static struct of_device_id amzn_thermal_of_match[] = {
	{ .compatible = "amazon,amzn-thermal" },
	{},
};

static struct platform_driver amzn_thermal_platdrv = {
	.driver = {
		.name		= "amzn-thermal",
		.owner		= THIS_MODULE,
		.of_match_table = amzn_thermal_of_match,
	},
	.probe	= amzn_thermal_probe,
	.remove	= amzn_thermal_remove,
};


static int __init amzn_thermal_platdrv_init(void)
{
	 return platform_driver_register(&(amzn_thermal_platdrv));
}
late_initcall(amzn_thermal_platdrv_init);
