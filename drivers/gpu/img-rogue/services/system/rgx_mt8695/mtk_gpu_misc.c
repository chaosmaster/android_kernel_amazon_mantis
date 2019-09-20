#include "pvrsrv_device.h"
#include "rgxdevice.h"
#include "rgxinit.h"
#include "pvrsrv.h"
#include "syscommon.h"
#include "sysconfig.h"
#include "physheap.h"
#if defined(SUPPORT_ION)
#include "ion_support.h"
#endif
#include "mtk_mfg.h"
#include "mtk_gpu_misc.h"

#if defined(CONFIG_OF)
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/platform_device.h>
#endif
#include <linux/proc_fs.h>
#include <linux/devfreq.h>
#include <linux/seq_file.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include "process_stats.h"

static PVRSRV_DEVICE_CONFIG *pvr_cfg;
static struct device *pvr_os_dev;
static PVRSRV_DVFS *pvr_dvfs;

static unsigned int limited_power = INT_MAX;
static unsigned int limited_freq = 0;

static PVRSRV_DEVICE_NODE *get_pvr_dev_node(void)
{
	PVRSRV_DATA *psPVRSRVData = PVRSRVGetPVRSRVData();
	IMG_UINT32 i;
	/* This may be null when caller using too early..*/
	if (!psPVRSRVData)
		return NULL;
	for (i = 0; i < psPVRSRVData->ui32RegisteredDevices; i++) {
		PVRSRV_DEVICE_NODE *psDeviceNode = &psPVRSRVData->psDeviceNodeList[i];

		if (psDeviceNode && psDeviceNode->psDevConfig)
			return psDeviceNode;

	}
	return NULL;
}


#ifdef CONFIG_PROC_FS
static int freq_seq_show(struct seq_file *m, void *v)
{
	u32 freq = mtk_mfg_get_freq();
	int volt = mtk_mfg_get_volt();
	u64 high, low, blk, total;
	u32 loading  = mtk_gpu_misc_get_util(&high, &low, &blk, &total);

	seq_printf(m, "volt:%d, freq:%d, Loading %d\n[ H %lld, L %lld, B %lld / %lld ]\n",
		volt, freq, loading, high, low, blk, total);
	return 0;
}

static int freq_seq_open(struct inode *in, struct file *file)
{
	return single_open(file, freq_seq_show, NULL);
}

static ssize_t freq_seq_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	char desc[32];
	int len = 0;
	u32 volt;
	u32 freq;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
		return 0;

	desc[len] = '\0';

	if (sscanf(desc, "%d %d", &volt, &freq) == 2) {
		/*
		 * We don't handle too much flow such as volt or freq first.
		 * This is just for debug, keep this simple. User should set
		 * this while GPU is idle.
		 */
		if (freq > mtk_mfg_get_freq()) {
			mtk_mfg_set_volt(volt);
			mtk_mfg_set_freq(freq, true);
		} else {
			mtk_mfg_set_freq(freq, true);
			mtk_mfg_set_volt(volt);
		}
	}

	return count;
}

static const struct file_operations freq_proc_fops = {
	.open = freq_seq_open,
	.read = seq_read,
	.write = freq_seq_write,
	.release = single_release,
};


static int regs_seq_show(struct seq_file *m, void *v)
{
	/*TODO: Enable power first?? */
	mfg_dump_regs_seq(m, "MFG ");
	return 0;
}

static int regs_seq_open(struct inode *in, struct file *file)
{
	return single_open(file, regs_seq_show, NULL);
}

static ssize_t regs_seq_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	char desc[32];
	int len = 0;
	u32 off;
	u32 val;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
		return 0;

	desc[len] = '\0';

	if (sscanf(desc, "%x %x", &off, &val) == 2)
		mfg_write(off, val);

	return count;

}

static const struct file_operations regs_proc_fops = {
	.open = regs_seq_open,
	.read = seq_read,
	.write = regs_seq_write,
	.release = single_release,
};


static int dvfs_seq_show(struct seq_file *m, void *v)
{
#if defined(PVR_DVFS)
	int i;
	int count;
	struct dev_pm_opp *opp;
	unsigned long freq = 0;
	unsigned long volt;

	IMG_DVFS_DEVICE *dvfs_dev = &pvr_dvfs->sDVFSDevice;

	struct devfreq_dev_status *stat;

	stat = &dvfs_dev->psDevFreq->last_status;

	if (dvfs_dev->psDevFreq->stop_polling)
		seq_printf(m, "Disabled by devfreq, current freq %ld, volt %d\n",
		mtk_mfg_get_freq(), mtk_mfg_get_volt());
	else if (!dvfs_dev->bEnabled)
		seq_printf(m, "Disabled by PVR DVFS, current freq %ld, volt %d\n",
		mtk_mfg_get_freq(), mtk_mfg_get_volt());
	else
		seq_puts(m, "Enabled\n");

	seq_puts(m, "    volt \t freq [power]\n");

	rcu_read_lock();
	count = dev_pm_opp_get_opp_count(pvr_os_dev);
	for (i = 0; i < count; i++) {
		opp = dev_pm_opp_find_freq_ceil(pvr_os_dev, &freq);
		freq = dev_pm_opp_get_freq(opp);
		volt =	dev_pm_opp_get_voltage(opp);

		if (freq > dvfs_dev->psDevFreq->max_freq)
			seq_puts(m, "X");
		else
			seq_puts(m, " ");

		if (freq == stat->current_frequency)
			seq_puts(m, " * ");
		else
			seq_puts(m, "   ");

		seq_printf(m, "%ld \t %ld [%ld]\n", volt, freq, mtk_mfg_calculate_power(volt, freq));
		freq++;/* next freq greater*/
	}
	rcu_read_unlock();

	/*
	 * We show the loading bounds instead of orignal data structure.
	 */
	seq_printf(m, "up:%d, down:%d, last:%ld\n", dvfs_dev->data.upthreshold,
		dvfs_dev->data.upthreshold - dvfs_dev->data.downdifferential,
		(unsigned long)(div_u64(stat->busy_time * 100UL, stat->total_time)));

#else
	seq_puts(m, "NO DVFS\n");
#endif

	return 0;
}

static int dvfs_seq_open(struct inode *in, struct file *file)
{
	return single_open(file, dvfs_seq_show, NULL);
}

static ssize_t dvfs_seq_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{

	char desc[32];
	int len = 0;
	IMG_DVFS_DEVICE *psDVFSDev = &pvr_dvfs->sDVFSDevice;


	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
		return 0;

	desc[len] = '\0';
	if (strncmp(desc, "disable", 7) == 0)
		devfreq_suspend_device(psDVFSDev->psDevFreq);
	else if (strncmp(desc, "enable", 6) == 0)
		devfreq_resume_device(psDVFSDev->psDevFreq);
	else {
		unsigned long limit;

		if (!kstrtol(desc, 10, &limit))
			mt_gpufreq_thermal_protect(limit);
	}

	return count;
}

static const struct file_operations dvfs_proc_fops = {
	.open = dvfs_seq_open,
	.read = seq_read,
	.write = dvfs_seq_write,
	.release = single_release,
};


static int bw_ultra_seq_show(struct seq_file *m, void *v)
{
	if (mtk_mfg_is_bw_ultra())
		seq_puts(m, "Enabled\n");
	else
		seq_puts(m, "Disabled\n");
	return 0;
}

static int bw_ultra_seq_open(struct inode *in, struct file *file)
{
	return single_open(file, bw_ultra_seq_show, NULL);
}

static ssize_t bw_ultra_seq_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	char desc[32];
	int len = 0;

	len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
	if (copy_from_user(desc, buffer, len))
		return 0;

	desc[len] = '\0';
	if (strncmp(desc, "disable", 7) == 0)
		mtk_mfg_disable_bw_ultra();
	else if (strncmp(desc, "enable", 6) == 0)
		mtk_mfg_enable_bw_ultra();

	return count;
}

static const struct file_operations bw_ultra_proc_fops = {
	.open = bw_ultra_seq_open,
	.read = seq_read,
	.write = bw_ultra_seq_write,
	.release = single_release,
};


static int install_cmds(void)
{
	struct proc_dir_entry *pentry = proc_mkdir("mtk-gpu", NULL);

	if (pentry) {
		proc_create_data("freq", 0, pentry, &freq_proc_fops, NULL);
		proc_create_data("regs", 0, pentry, &regs_proc_fops, NULL);
		proc_create_data("dvfs", 0, pentry, &dvfs_proc_fops, NULL);
		proc_create_data("bw_ultra", 0644, pentry, &bw_ultra_proc_fops, NULL);
	}

	return 0;
}
#else
static int install_cmds(void)
{
	return 0;
}
#endif

/*
 * TODO: Helper functions for MET..
 */
static IMG_HANDLE util_user;

u32 mtk_gpu_misc_get_util(u64 *high, u64 *low, u64 *blk, u64 *total)
{
	PVRSRV_DEVICE_NODE *pvr_node;
	PVRSRV_RGXDEV_INFO *pvr_info;

	pvr_node = get_pvr_dev_node();
	if (!util_user) {
		*high = *low = *blk = *total = 0;
		return 0;
	}
	if (pvr_node) {
		pvr_info = (PVRSRV_RGXDEV_INFO *)pvr_node->pvDevice;

		if (pvr_info && pvr_info->pfnGetGpuUtilStats) {
			RGXFWIF_GPU_UTIL_STATS util_stats = {0};

			pvr_info->pfnGetGpuUtilStats(pvr_info->psDeviceNode, util_user, &util_stats);
			if (util_stats.bValid) {
				u64 loading;

				loading =
					div_u64((util_stats.ui64GpuStatActiveHigh +
						util_stats.ui64GpuStatActiveLow) * 100UL,
					util_stats.ui64GpuStatCumulative);

				if (high)
					*high =	util_stats.ui64GpuStatActiveHigh;

				if (low)
					*low = util_stats.ui64GpuStatActiveLow;

				if (blk)
					*blk = util_stats.ui64GpuStatBlocked;

				if (total)
					*total = util_stats.ui64GpuStatCumulative;

				return (u32)loading;
			}
		}
	}
	return 0;
}

/*
 * For MET and other drivers which has 'static' reference to
 * gpu utils.
 */
static unsigned long sample_time;
static unsigned int snap_loading;
static u64 snap_block;
static u64 snap_high;
static u64 snap_low;
static u64 snap_total;
static void met_sample_update(void)
{
	if (time_after(jiffies, sample_time + 20)) {
		sample_time = jiffies;
		snap_loading = mtk_gpu_misc_get_util(
			&snap_high,
			&snap_low,
			&snap_block,
			&snap_total);
	}
}

bool mtk_get_gpu_loading(unsigned int *pLoading)
{
	met_sample_update();
	if (pLoading)
		*pLoading = snap_loading;
	return true;
}
EXPORT_SYMBOL(mtk_get_gpu_loading);

bool mtk_get_gpu_block(unsigned int *pLoading)
{
	met_sample_update();
	if (pLoading)
		*pLoading = snap_block;
	return true;
}
EXPORT_SYMBOL(mtk_get_gpu_block);

bool mtk_get_gpu_idle(unsigned int *pLoading)
{
	met_sample_update();
	if (pLoading)
		*pLoading = snap_total - snap_high - snap_low;
	return true;
}
EXPORT_SYMBOL(mtk_get_gpu_idle);

bool mtk_get_gpu_memory_usage(unsigned int *pLoading)
{
	if (pLoading)
		*pLoading = PVRMTKGetTotalMemory();
	return true;
}
EXPORT_SYMBOL(mtk_get_gpu_memory_usage);

bool mtk_get_gpu_power_loading(unsigned int *pLoading)
{
	if (pLoading)
		*pLoading = 0;
	return true;
}
EXPORT_SYMBOL(mtk_get_gpu_power_loading);

unsigned int mt_gpufreq_get_cur_freq(void)
{
	return mtk_mfg_get_snap_freq();
}
EXPORT_SYMBOL(mt_gpufreq_get_cur_freq);

unsigned int mt_gpufreq_get_cur_volt(void)
{
	return mtk_mfg_get_snap_volt();
}
EXPORT_SYMBOL(mt_gpufreq_get_cur_volt);

unsigned int mt_gpufreq_get_thermal_limit_freq(void)
{
	return limited_power;
}
EXPORT_SYMBOL(mt_gpufreq_get_thermal_limit_freq);

unsigned int mt_gpufreq_get_thermal_limit_freq_Khz(void)
{
	return limited_freq;
}
EXPORT_SYMBOL(mt_gpufreq_get_thermal_limit_freq_Khz);

void mt_gpufreq_set_thermal_limit_freq(unsigned long freq)
{
	struct dev_pm_opp *opp;
	unsigned long freq_hz;

	freq_hz = freq*1000;
	rcu_read_lock();
	opp = dev_pm_opp_find_freq_ceil(pvr_os_dev, &freq_hz);
	limited_freq = freq_hz;
	rcu_read_unlock();

	if (freq_hz) {
		mutex_lock(&pvr_dvfs->sDVFSDevice.psDevFreq->lock);
		pvr_dvfs->sDVFSDevice.psDevFreq->max_freq = freq_hz;
		mutex_unlock(&pvr_dvfs->sDVFSDevice.psDevFreq->lock);
		devfreq_update_stats(pvr_dvfs->sDVFSDevice.psDevFreq);
	} else {
		pr_err("%s: cannot find any limited freq %ld\n", __func__, freq_hz);
	}
}
EXPORT_SYMBOL(mt_gpufreq_set_thermal_limit_freq);

void mt_gpufreq_thermal_protect(unsigned int power)
{
	int i;
	int count;
	struct dev_pm_opp *opp;
	unsigned long freq = 0;
	unsigned long volt;
	unsigned long calc_power;
	unsigned long last_freq = 0;
	unsigned long last_volt = 0;

	if (power == 0)
		limited_power = INT_MAX;
	else
		limited_power = power;

	rcu_read_lock();
	count = dev_pm_opp_get_opp_count(pvr_os_dev);
	for (i = 0; i < count; i++) {
		opp = dev_pm_opp_find_freq_ceil(pvr_os_dev, &freq);
		freq = dev_pm_opp_get_freq(opp);
		volt =	dev_pm_opp_get_voltage(opp);
		calc_power = mtk_mfg_calculate_power(volt, freq);
		if (calc_power > limited_power)
			break;

		last_freq = freq;
		last_volt = volt;
		freq++;/* next freq greater*/
	}
	rcu_read_unlock();

	if (last_freq) {
		mutex_lock(&pvr_dvfs->sDVFSDevice.psDevFreq->lock);
		pvr_dvfs->sDVFSDevice.psDevFreq->max_freq = last_freq;
		mutex_unlock(&pvr_dvfs->sDVFSDevice.psDevFreq->lock);
		devfreq_update_stats(pvr_dvfs->sDVFSDevice.psDevFreq);
	} else {
		pr_err("limited power %d, but cannot find any limited freq\n", power);
	}
}
EXPORT_SYMBOL(mt_gpufreq_thermal_protect);


void mtk_gpu_thermal_setup(void)
{
	int i;
	int count;
	struct dev_pm_opp *opp;
	unsigned long freq = 0;
	unsigned long volt;
	unsigned long calc_power;
	struct mtk_gpu_power_info *info;

	rcu_read_lock();
	count = dev_pm_opp_get_opp_count(pvr_os_dev);
	info = kcalloc(count, sizeof(struct mtk_gpu_power_info), GFP_NOWAIT);
	for (i = 0; i < count; i++) {
		opp = dev_pm_opp_find_freq_ceil(pvr_os_dev, &freq);
		freq = dev_pm_opp_get_freq(opp);
		volt =	dev_pm_opp_get_voltage(opp);
		calc_power = mtk_mfg_calculate_power(volt, freq);

		info[count - i - 1].gpufreq_khz = freq/1000;
		info[count - i - 1].gpufreq_power = calc_power;

		freq++;/* next freq greater*/
	}
	rcu_read_unlock();

	mtk_gpufreq_register(info, count);

	kfree(info);
}


#define MFG_POWER_NOTIFY_COUNT 5
met_gpu_power_change_notify_fp met_power_nfy[MFG_POWER_NOTIFY_COUNT];
char met_power_nfy_name[MFG_POWER_NOTIFY_COUNT][64];

static DEFINE_MUTEX(nfy_lock);

bool mtk_register_gpu_power_change(const char *name, met_gpu_power_change_notify_fp callback)
{
	int i;

	mutex_lock(&nfy_lock);
	for (i = 0; i < MFG_POWER_NOTIFY_COUNT; i++) {
		if (met_power_nfy[i] == NULL) {
			met_power_nfy[i] = callback;
			strncpy(met_power_nfy_name[i], name, 63);
			mutex_unlock(&nfy_lock);
			return true;
		}
	}
	mutex_unlock(&nfy_lock);
	return false;
}
EXPORT_SYMBOL(mtk_register_gpu_power_change);

bool mtk_unregister_gpu_power_change(const char *name)
{
	int i;

	mutex_lock(&nfy_lock);
	for (i = 0; i < MFG_POWER_NOTIFY_COUNT; i++) {
		if (strncmp(met_power_nfy_name[i], name, 63)) {
			met_power_nfy[i] = NULL;
			met_power_nfy_name[i][0] = '\0';
			mutex_unlock(&nfy_lock);
			return true;
		}
	}
	mutex_unlock(&nfy_lock);
	return false;
}
EXPORT_SYMBOL(mtk_unregister_gpu_power_change);

void mtk_gpu_misc_met_nfy_power_state(int power_on)
{
	int i;

	mutex_lock(&nfy_lock);
	for (i = 0; i < MFG_POWER_NOTIFY_COUNT; i++) {
		if (met_power_nfy[i] != NULL)
			met_power_nfy[i](power_on);
	}
	mutex_unlock(&nfy_lock);
}

static void met_nfy_init(void)
{
	int i;

	for (i = 0; i < MFG_POWER_NOTIFY_COUNT; i++) {
		met_power_nfy[i] = NULL;
		met_power_nfy_name[i][0] = '\0';
	}
}

void mtk_gpu_misc_init(PVRSRV_DEVICE_CONFIG *cfg)
{
	pvr_cfg = cfg;
	pvr_os_dev = cfg->pvOSDevice;
	pvr_dvfs = &cfg->sDVFS;

	RGXRegisterGpuUtilStats(&util_user);

	met_nfy_init();

	install_cmds();
}
