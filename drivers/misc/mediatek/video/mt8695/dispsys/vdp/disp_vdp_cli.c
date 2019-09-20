#include <linux/gfp.h>
#include <linux/dma-mapping.h>
#include <linux/ratelimit.h>
#include <linux/printk.h>

#define LOG_TAG "VDP_CLI"
#include "disp_cli.h"
#include "disp_vdp_cli.h"
#include "disp_hw_mgr.h"
#include "disp_hw_log.h"
#include "disp_vdp_sec.h"

struct vdp_cli_setting_struct cli_setting = {
	.enable_mva_debug = false,
	.enable_force_use_mdp = false,
	.use_vdo3 = true,
	.target_area.enable = false,
	.slow = 1,
	.debug = 0,
};

struct vdp_cli_setting_struct *vdp_cli_get(void)
{
	return &cli_setting;
}

static struct timer_list timer;

void timer_callback_function(unsigned long data)
{
	mod_timer(&timer, jiffies + msecs_to_jiffies(1000));
	pr_err_ratelimited("debug mdp config:%d release:%d, vdp config0:%d valid0:%d fps0:%d, config1:%d valid1:%d fps1:%d\n",
		mdp_config_frame_count,
		mdp_release_frame_count,
		config_buffer_count[0],
		config_frame_count[0],
		debug_frame_count[0],
		config_buffer_count[1],
		config_frame_count[1],
		debug_frame_count[1]);

	mdp_config_frame_count = 0;
	mdp_release_frame_count = 0;
	memset(config_buffer_count, 0, sizeof(config_buffer_count));
	memset(config_frame_count, 0, sizeof(config_frame_count));
	memset(debug_frame_count, 0, sizeof(debug_frame_count));
}

/* int vdp_set_timer(void); */
static int _vdp_cli_set_timer(int argc, const char **argv)
{
	static bool start_timer_flag;

	if (start_timer_flag)
		return 0;
	start_timer_flag = true;

	setup_timer(&timer, timer_callback_function, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(1000));
	return 0;
}

static int _vdp_cli_read_normal(int argc, const char **argv)
{
	int i = 0;
	uint32_t registerPA = 0;
	uint32_t registerBase = 0;
	void __iomem *iomapVA = NULL;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.read 0x15001000\n");
		return 0;
	}
	if (kstrtouint(argv[1], 0, &registerPA) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	/* print 4K region in around reigsterPA */
	registerBase = registerPA & ~(PAGE_SIZE - 1);
	iomapVA = ioremap(registerBase, PAGE_SIZE);

	i = (registerPA - registerBase) / 4;

	pr_err_ratelimited("reg 0x%x: 0x%08x\t 0x%08x\t 0x%08x\t 0x%08x\n",
		registerBase + (i * 4),
		*((uint32_t *)iomapVA + i),
		*((uint32_t *)iomapVA + i + 1),
		*((uint32_t *)iomapVA + i + 2),
		*((uint32_t *)iomapVA + i + 3));

	iounmap(iomapVA);

	return 0;
}

static int _vdp_cli_write_normal(int argc, const char **argv)
{
	uint32_t registerPA = 0;
	uint32_t value = 0;
	uint32_t registerBase = 0;
	void __iomem *iomapVA = NULL;
	uint32_t *registerVA = NULL;

	if (argc != 3) {
		pr_err_ratelimited("usage: cli vdp.write 0x15001000 0x1\n");
		return 0;
	}
	if (kstrtouint(argv[1], 0, &registerPA) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	if (kstrtouint(argv[2], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[2]);
		return 0;
	}

	/* print 4K region in around reigsterPA */
	registerBase = registerPA & ~(PAGE_SIZE - 1);
	iomapVA = ioremap(registerBase, PAGE_SIZE);

	registerVA = (uint32_t *)((char *)iomapVA + (registerPA - registerBase));

	pr_err_ratelimited("write register:0x%08x value:0x%08x\n", registerPA, value);
	*registerVA = value;

	iounmap(iomapVA);


	return 0;
}

static int _vdp_cli_alloc_memory(int argc, const char **argv)
{
	struct device *pDev = disp_hw_mgr_get_dev();
	static bool allocated;
	static char *pYAddr;
	static char *pCAddr;
	static char *pYlenAddr;
	static char *pClenAddr;
	static dma_addr_t yPA;
	static dma_addr_t cPA;
	static dma_addr_t ylenPA;
	static dma_addr_t clenPA;
	int size = 4096 * 2176;

	if (!allocated) {
		pYAddr = dma_alloc_coherent(pDev, size, &yPA, GFP_KERNEL);
		pCAddr = dma_alloc_coherent(pDev, size, &cPA, GFP_KERNEL);
		pYlenAddr = dma_alloc_coherent(pDev, size/256, &ylenPA, GFP_KERNEL);
		pClenAddr = dma_alloc_coherent(pDev, size/256, &clenPA, GFP_KERNEL);
		allocated = true;
	}
	pr_err_ratelimited("allocated buffer info:\n");
	pr_err_ratelimited("Y:\tVA[%p] PA[0x%08x]\n", pYAddr, (uint32_t)yPA);
	pr_err_ratelimited("C:\tVA[%p] PA[0x%08x]\n", pCAddr, (uint32_t)cPA);
	pr_err_ratelimited("Ylen:\tVA[%p] PA[0x%08x]\n", pYlenAddr, (uint32_t)ylenPA);
	pr_err_ratelimited("Clen:\tVA[%p] PA[0x%08x]\n", pClenAddr, (uint32_t)clenPA);

	return 0;
}


static int _vdp_cli_debug_force_mdp(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.force_mdp 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	cli_setting.enable_force_use_mdp = (value != 0);
	pr_err_ratelimited("set force_mdp:%d\n", cli_setting.enable_force_use_mdp);
	return 0;
}

static int _vdp_cli_use_vdo3(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.use_vdo3 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	cli_setting.use_vdo3 = (value != 0);
	pr_err_ratelimited("set use_vdo3:%d\n", cli_setting.use_vdo3);
	return 0;
}

static int _vdp_cli_debug_set_disp_area(int argc, const char **argv)
{
	int value[6] = {0};
	int i = 0;

	if (argc != 7) {
		pr_err_ratelimited("usage: cli vdp.show layer_id x y width height pitch\n");
		return 0;
	}
	for (i = 0; i < 6; i++)
		if (kstrtoint(argv[i+1], 0, &value[i]) != 0) {
			pr_err_ratelimited("invalid input:%s\n", argv[i+1]);
			return 0;
		}

	if ((value[3] == 0) ||
		(value[4] == 0) ||
		(value[5] == 0))
		cli_setting.target_area.enable = false;
	else
		cli_setting.target_area.enable = true;

	cli_setting.target_area.layer_id = value[0];
	cli_setting.target_area.range.x = value[1];
	cli_setting.target_area.range.y = value[2];
	cli_setting.target_area.range.width = value[3];
	cli_setting.target_area.range.height = value[4];
	cli_setting.target_area.range.pitch = value[5];
	pr_err_ratelimited("set show: enable[%d] layer_id[%d] x[%d] y[%d] width[%d] height[%d] pitch[%d]\n",
		cli_setting.target_area.enable,
		cli_setting.target_area.layer_id,
		cli_setting.target_area.range.x,
		cli_setting.target_area.range.y,
		cli_setting.target_area.range.width,
		cli_setting.target_area.range.height,
		cli_setting.target_area.range.pitch);
	return 0;
}

static int _vdp_cli_set_secure_loglevel(int argc, const char **argv)
{
	int level = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.secure_loglevel 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &level) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	pr_err_ratelimited("set secure loglevel:%d\n", level);
	disp_vdp_sec_set_log_level(level);

	return 0;
}

#if 0
static int _vdp_cli_set_playback_slow(int argc, const char **argv)
{
	int slow_rate = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.slow 50\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &slow_rate) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	pr_err_ratelimited("set playback rate:1/%d\n", slow_rate);
	cli_setting.slow = slow_rate;

	return 0;
}
#endif

static int _vdp_cli_debug(int argc, const char **argv)
{
	uint32_t debug[10] = {0};
	int i = 0;

	for (i = 0; i < (argc < 11 ? (argc - 1) : 10); i++)
		if (kstrtouint(argv[i+1], 0, &debug[i]) != 0) {
			pr_err_ratelimited("invalid input:%s\n", argv[i+1]);
			return 0;
		}

	cli_setting.debug = debug[0];

	if (debug[0] == 1) {
		disp_clock_enable(DISP_CLK_OSD_PREMIX, true);
		disp_vdp_set_osd_premix(true);
	} else {
		disp_clock_enable(DISP_CLK_OSD_PREMIX, false);
		disp_vdp_set_osd_premix(false);
	}

	return 0;
}

extern void disp_sys_hal_set_main_sub_swap(uint32_t swap);
extern int32_t fmt_hal_set_output_444(uint32_t fmt_id, bool is_444);
extern struct disp_hw *disp_vdp_get_drv(void);
static int _vdp_cli_swap(int argc, const char **argv)
{
	uint32_t value = 0;
	
	if (argc != 2) {
		DISP_LOG_E("usage: cli vdp.swap 1\n");
		return 0;
	}
	if (kstrtouint(argv[1], 0, &value) != 0) {
		DISP_LOG_E("invalid input:%s\n", argv[1]);
		return 0;
	}

	value = !!value;

	/* swap main/sub video path 0x15000014[19] */
	disp_sys_hal_set_main_sub_swap(value);
	/* disable vdo4 output to pre-mix */
	value = !value;
	disp_vdp_get_drv()->drv_call(DISP_CMD_OSD_PREMIX_ENABLE_VDO4, &value);

	/* disable YUV444 */
	fmt_hal_set_output_444(1, value);

	return 0;
}


static int _vdp_cli_pts(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli vdp.pts 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	cli_setting.enable_pts_debug = (value != 0);
	pr_err_ratelimited("set ptsdebug enable:%d\n", cli_setting.enable_pts_debug);
	return 0;
}

static CLI_EXEC_T items[] = {
	{"timer",  NULL, _vdp_cli_set_timer, NULL,  "VDP: test timer function", CLI_GUEST},
	{"read",  NULL, _vdp_cli_read_normal, NULL,  "VDP: read normal register", CLI_GUEST},
	{"write",  NULL, _vdp_cli_write_normal, NULL,  "VDP: write normal register", CLI_GUEST},
	{"use_vdo3",  NULL, _vdp_cli_use_vdo3, NULL,  "VDP: use vdo3", CLI_GUEST},
	{"alloc",  NULL, _vdp_cli_alloc_memory, NULL,  "VDP: alloc memory for debug", CLI_GUEST},
	{"force_mdp",  NULL, _vdp_cli_debug_force_mdp, NULL,  "VDP: force use MDP", CLI_GUEST},
	{"show",  NULL, _vdp_cli_debug_set_disp_area, NULL,  "VDP: set display area", CLI_GUEST},
	{"secure_loglevel",  NULL, _vdp_cli_set_secure_loglevel, NULL,  "VDP: set secure loglevel", CLI_GUEST},
	{"debug",  NULL, _vdp_cli_debug, NULL,  "VDP: debug", CLI_GUEST},
	{"swap",  NULL, _vdp_cli_swap, NULL,  "VDP: swap", CLI_GUEST},
	{"pts",  NULL, _vdp_cli_pts, NULL,  "VDP: pts", CLI_GUEST},
	{NULL, NULL, NULL, NULL, NULL, CLI_GUEST},
};

void vdp_cli_init(void)
{
	cli_register("vdp", items);
}

