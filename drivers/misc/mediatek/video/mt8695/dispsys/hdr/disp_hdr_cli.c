#include <linux/vmalloc.h>
#include <linux/ratelimit.h>
#include <linux/printk.h>
#include "disp_cli.h"
#include "disp_hdr_util.h"
#include "disp_hdr_device.h"
#include "disp_hdr_sec.h"
#include "disp_hdr_core.h"
#include "disp_hw_mgr.h"

static struct hdr_cli_info_struct gCliInfo = {
	.logLevel = HDR_LOG_LEVEL_ERR | HDR_LOG_LEVEL_LOG | HDR_LOG_LEVEL_DBG,
	.gainValue = 0xFFFFFFFF,
	.updateHDR = 0,
	.bypassModule = 0,
	.disable_path_clock_control = true,
};

static int _hdr_cli_read_register(int argc, const char **argv)
{
	/* cli hdr.read register_address
	** example: cli hdr.read 0xf001c00
	*/
	uint32_t registerPA = 0;
	uint32_t value;
	char *registerVA = NULL;

	if (argc != 2) {
		pr_err_ratelimited("argc = %d, usage: cli hdr.read register_address\n", argc);
		return 0;
	}

	if (kstrtouint(argv[1], 0, &registerPA) != 0) {
		pr_err_ratelimited("invalid register:%s\n", argv[1]);
		return 0;
	}

	hdr_device_map_PA_2_VA(registerPA, &registerVA);
	value = hdr_read_register(registerVA);
	pr_err_ratelimited("read register[0x%08x] value[0x%08x]\n", registerPA, value);
	return 0;
}

static int _hdr_cli_write_register(int argc, const char **argv)
{
	/* cli hdr.write register_address value
	** example: cli hdr.write 0xf001c00 0x1
	*/
	uint32_t registerPA = 0;
	uint32_t value;
	char *registerVA = NULL;

	if (argc != 3) {
		pr_err_ratelimited("argc = %d, usage: cli hdr.write register_address value\n", argc);
		return 0;
	}

	if (kstrtouint(argv[1], 0, &registerPA) != 0) {
		pr_err_ratelimited("invalid register:%s\n", argv[1]);
		return 0;
	}

	if (kstrtoint(argv[2], 0, &value) != 0) {
		pr_err_ratelimited("invalid value:%s\n", argv[2]);
		return 0;
	}

	hdr_device_map_PA_2_VA(registerPA, &registerVA);
	hdr_write_register(registerVA, value, true);
	return 0;
}

int hdr_cli_set_log_level(int argc, const char **argv)
{
	/*
	** usage:
	** cli hdr.log set loglevel  force clear or set loglevel
	** cli hdr.log enable loglevel  add one loglevel
	** cli hdr.log disable loglevel  disable one loglevel
	*/
	uint32_t value = 0;

	if (argc != 3) {
		pr_err_ratelimited("---- set log level usage: ----\n");
		pr_err_ratelimited("loglevel bit ERR[0] DGB[1] LOG[2] TONE_MAP[3]\n");
		pr_err_ratelimited("cli hdr.log set loglevel\n");
		pr_err_ratelimited("cli hdr.log enable loglevel\n");
		pr_err_ratelimited("cli hdr.log disable loglevel\n");
		return 0;
	}
	if (strcmp(argv[1], "set") == 0) {
		if (kstrtoint(argv[2], 0, &value) != 0) {
			pr_err_ratelimited("invalid level input:%s\n", argv[2]);
			return 0;
		}
		gCliInfo.logLevel = value;
	} else if (strcmp(argv[1], "enable") == 0) {
		if (kstrtoint(argv[2], 0, &value) != 0) {
			pr_err_ratelimited("invalid level input:%s\n", argv[2]);
			return 0;
		}
		gCliInfo.logLevel = gCliInfo.logLevel | value;
	} else if (strcmp(argv[1], "disable") == 0) {
		if (kstrtoint(argv[2], 0, &value) != 0) {
			pr_err_ratelimited("invalid level input:%s\n", argv[2]);
			return 0;
		}
		gCliInfo.logLevel = gCliInfo.logLevel & ~value;
	} else {
		pr_err_ratelimited("invalid usage for %s\n", argv[1]);
		pr_err_ratelimited("---- set log level usage: ----\n");
		pr_err_ratelimited("loglevel bit ERR[0] DGB[1] LOG[2] TONE_MAP[3]\n");
		pr_err_ratelimited("cli hdr.log set loglevel\n");
		pr_err_ratelimited("cli hdr.log enable loglevel\n");
		pr_err_ratelimited("cli hdr.log disable loglevel\n");
		return 0;
	}

	do {
		enum HDR_LOG_LEVEL level = 0;

		pr_err_ratelimited("test HDR log level\n");
		for (level = 1 << 0; level < HDR_LOG_LEVEL_MAX; level = level << 1)
			HDR_PRINTF(level, "hdr test log level:%s\n", hdr_print_log_level(level));
	} while (0);

	return 0;
}

static int _hdr_cli_config_sdr2hdr_gain(int argc, const char **argv)
{
	uint32_t value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli hdr.gain value\n");
		return 0;
	}

	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input %s\n", argv[1]);
		return 0;
	}
	pr_err_ratelimited("set gain value:0x%x\n", value);
	gCliInfo.gainValue = value;
	return 0;
}

static int _hdr_cli_debug(int argc, const char **argv)
{
	/* temp debug add here */
	return 0;
}


static int _hdr_cli_update_HDR(int argc, const char **argv)
{
	int updateHDR;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli hdr.update 1\n");
		return 0;
	}

	if (kstrtoint(argv[1], 0, &updateHDR) != 0) {
		pr_err_ratelimited("invalid printCount[%s]\n", argv[1]);
		return 0;
	}

	gCliInfo.updateHDR = updateHDR;

	pr_err_ratelimited("set updateHDR = %d\n", updateHDR);

	if (gCliInfo.updateHDR == 0)
		hdr_core_handle_disp_stop(0);
	return 0;
}

static int _hdr_cli_stop_video(int argc, const char **argv)
{
	hdr_core_handle_disp_stop(0);
	return 0;
}



static int _hdr_cli_disableModule(int argc, const char **argv)
{
	uint32_t bypassModule;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli hdr.disable 0x111\n");
		return 0;
	}

	if (kstrtouint(argv[1], 0, &bypassModule) != 0) {
		pr_err_ratelimited("invalid bypassModule[%s]\n", argv[1]);
		return 0;
	}

	gCliInfo.bypassModule = bypassModule;

	pr_err_ratelimited("set bypassModule = %d\n", bypassModule);
	return 0;
}



static int _hdr_cli_secure(int argc, const char **argv)
{
#ifdef HDR_SECURE_SUPPORT
	if (argc != 4) {
		pr_err_ratelimited("usage: cli hdr.secure plane use_hdr\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &hdr_sec_get_share_memory()->plane) != 0)
		return 0;
	if (kstrtoint(argv[2], 0, &hdr_sec_get_share_memory()->use_hdr) != 0)
		return 0;
	if (kstrtoint(argv[3], 0, &hdr_sec_get_share_memory()->use_hdr_sub_path) != 0)
		return 0;

	pr_err_ratelimited("share memory: plane[%d] use_hdr_main[%d] use_hdr_sub_path[%d]\n",
		hdr_sec_get_share_memory()->plane,
		hdr_sec_get_share_memory()->use_hdr,
		hdr_sec_get_share_memory()->use_hdr_sub_path);
	hdr_sec_service_call(SERVICE_CALL_CMD_SEND_CLI_INFO,
		SERVICE_CALL_DIRECTION_INPUT,
		(void *)&gCliInfo, sizeof(gCliInfo));
#endif
	return 0;
}

static int _hdr_irqinfo_secure(int argc, const char **argv)
{
#ifdef HDR_SECURE_SUPPORT
	struct hdr_irq_info_struct IrqInfo;

	if (argc != 3) {
		pr_err_ratelimited("usage: cli irqid.secure use_hdr\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &IrqInfo.irqid) != 0)
		return 0;

	if (kstrtoint(argv[2], 0, &IrqInfo.hightrigger) != 0)
		return 0;

	pr_err_ratelimited("share memory: irqid[%d] hightrigger[%d]\n", IrqInfo.irqid, IrqInfo.hightrigger);
	hdr_sec_service_call(SERVICE_CALL_CMD_REGISTER_IRQ,
		SERVICE_CALL_DIRECTION_INPUT,
		(void *)&IrqInfo, sizeof(IrqInfo));
#endif
	return 0;
}


static int  _hdr_cli_disable_new_path_function(int argc, const char **argv)
{
	int disable_new_path;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli hdr.disable_new_path 1\n");
		return 0;
	}

	if (kstrtoint(argv[1], 0, &disable_new_path) != 0) {
		pr_err_ratelimited("invalid param[%s]\n", argv[1]);
		return 0;
	}
	pr_err_ratelimited("set disable_new_path = %d\n", disable_new_path);

	gCliInfo.disable_path_clock_control = disable_new_path;
	return 0;
}

static int _hdr_cli_early_porting_bt2020(int argc, const char **argv)
{
	int path = 0;
	int convert_bt2020_709 = 0;
	struct mtk_disp_buffer dispBuffer;
	struct disp_hw_common_info hwInfo;

	memset(&dispBuffer, 0, sizeof(dispBuffer));
	memset(&hwInfo, 0, sizeof(hwInfo));
	hwInfo.resolution = vmalloc(sizeof(struct disp_hw_resolution));
	/*
	** path = 0 1 2 for main / sub / osd path
	** convert_bt2020_709 = 0 / 1 / 2 for 709->BT2020 / BT2020 -> 709 / constant BT2020 -> 709.
	*/
	if (argc != 3) {
		pr_err_ratelimited("usage: cli hdr.bt2020 1 1\n");
		return 0;
	}
	if ((kstrtouint(argv[1], 0, &path) != 0) || (kstrtouint(argv[2], 0, &convert_bt2020_709) != 0)) {
		pr_err_ratelimited("invalid input argv[1][%s] argv[2][%s]\n", argv[1], argv[2]);
		return 0;
	}

	dispBuffer.hdr_info.colorPrimaries = 0;
	dispBuffer.hdr_info.transformCharacter = 0; /* 16 for ST2084 18 for HLG */
	dispBuffer.hdr_info.matrixCoeffs = 0;	/* 10 for constant BT2020 */

	switch (path) {
	case 0: /* main path */
		dispBuffer.layer_id = 0;
		dispBuffer.type = DISP_LAYER_VDP;
		break;
	case 1: /* sub path */
		dispBuffer.layer_id = 1;
		dispBuffer.type = DISP_LAYER_VDP;
		break;
	case 2: /* osd path */
		dispBuffer.type = DISP_LAYER_OSD;
		break;
	default:
		pr_err_ratelimited("invalid path:%d\n", path);
	}

	switch (convert_bt2020_709) {
	case 0: /* convert from BT709 to BT2020 */
		dispBuffer.is_bt2020 = 0;
		dispBuffer.video_type = VIDEO_YUV_BT709;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 0;
		hwInfo.tv.is_support_bt2020 = 1;
		break;
	case 1: /* convert from BT2020 to BT709 */
		dispBuffer.is_bt2020 = 1;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 1;
		hwInfo.tv.is_support_bt2020 = 0;
		break;
	case 2: /* convert from constant BT2020 to BT709 */
		dispBuffer.is_bt2020 = 1;
		dispBuffer.hdr_info.matrixCoeffs = 10;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 1;
		hwInfo.tv.is_support_bt2020 = 0;
		break;
	default:
		pr_err_ratelimited("invalid param:%d\n", convert_bt2020_709);
	}
	dispBuffer.is_hdr = 0;
	hwInfo.tv.is_support_hdr = 0;

	_hdr_core_handle_disp_config(&dispBuffer, &hwInfo);
	return 0;
}

static int _hdr_cli_early_porting_sdr2hdr(int argc, const char **argv)
{
	struct mtk_disp_buffer dispBuffer;
	struct disp_hw_common_info hwInfo;

	memset(&dispBuffer, 0, sizeof(dispBuffer));
	memset(&hwInfo, 0, sizeof(hwInfo));
	hwInfo.resolution = vmalloc(sizeof(struct disp_hw_resolution));

	dispBuffer.hdr_info.colorPrimaries = 0;
	dispBuffer.hdr_info.transformCharacter = 0; /* 16 for ST2084 18 for HLG */
	dispBuffer.hdr_info.matrixCoeffs = 0;	/* 10 for constant BT2020 */

	/* osd path */
	dispBuffer.type = DISP_LAYER_OSD;

	dispBuffer.is_bt2020 = 0;
	dispBuffer.video_type = VIDEO_YUV_BT709;

	hwInfo.tv.is_support_601 = 0;
	hwInfo.tv.is_support_709 = 1;
	hwInfo.tv.is_support_bt2020 = 0;

	dispBuffer.is_hdr = 0;
	hwInfo.tv.is_support_hdr = 1;

	_hdr_core_handle_disp_config(&dispBuffer, &hwInfo);
	return 0;
}

static int _hdr_cli_early_porting_hdr2sdr(int argc, const char **argv)
{
	int path = 0;
	int convert_bt2020_709 = 0;
	struct mtk_disp_buffer dispBuffer;
	struct disp_hw_common_info hwInfo;

	memset(&dispBuffer, 0, sizeof(dispBuffer));
	memset(&hwInfo, 0, sizeof(hwInfo));
	hwInfo.resolution = vmalloc(sizeof(struct disp_hw_resolution));
	/*
	** path = 0 1 2 for main / sub / osd path
	** convert_bt2020_709 = 0 / 1 / 2 for 709->BT2020 / BT2020 -> 709 / constant BT2020 -> 709.
	*/
	if (argc != 3) {
		pr_err_ratelimited("usage: cli hdr.hdr2sdr 1 1\n");
		return 0;
	}
	if ((kstrtouint(argv[1], 0, &path) != 0) || (kstrtouint(argv[2], 0, &convert_bt2020_709) != 0)) {
		pr_err_ratelimited("invalid input argv[1][%s] argv[2][%s]\n", argv[1], argv[2]);
		return 0;
	}

	dispBuffer.hdr_info.colorPrimaries = 0;
	dispBuffer.hdr_info.transformCharacter = 18; /* 16 for ST2084 18 for HLG */
	dispBuffer.hdr_info.matrixCoeffs = 0;	/* 10 for constant BT2020 */

	switch (path) {
	case 0: /* main path */
		dispBuffer.layer_id = 0;
		dispBuffer.type = DISP_LAYER_VDP;
		break;
	case 1: /* sub path */
		dispBuffer.layer_id = 1;
		dispBuffer.type = DISP_LAYER_VDP;
		break;
	case 2: /* osd path */
		dispBuffer.type = DISP_LAYER_OSD;
		break;
	default:
		pr_err_ratelimited("invalid path:%d\n", path);
	}

	switch (convert_bt2020_709) {
	case 0: /* convert from BT709 to BT2020 */
		dispBuffer.is_bt2020 = 0;
		dispBuffer.video_type = VIDEO_YUV_BT709;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 0;
		hwInfo.tv.is_support_bt2020 = 1;
		break;
	case 1: /* convert from BT2020 to BT709 */
		dispBuffer.is_bt2020 = 1;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 1;
		hwInfo.tv.is_support_bt2020 = 0;
		break;
	case 2: /* convert from constant BT2020 to BT709 */
		dispBuffer.is_bt2020 = 1;
		dispBuffer.hdr_info.matrixCoeffs = 10;

		hwInfo.tv.is_support_601 = 0;
		hwInfo.tv.is_support_709 = 1;
		hwInfo.tv.is_support_bt2020 = 0;
		break;
	default:
		pr_err_ratelimited("invalid param:%d\n", convert_bt2020_709);
	}

	dispBuffer.is_hdr = 1;
	hwInfo.tv.is_support_hdr = 0;

	_hdr_core_handle_disp_config(&dispBuffer, &hwInfo);
	return 0;
}

static CLI_EXEC_T items[] = {
	{"log",  NULL, hdr_cli_set_log_level, NULL,  "HDR: set log level", CLI_GUEST},
	{"read",  NULL, _hdr_cli_read_register, NULL,  "HDR: read register", CLI_GUEST},
	{"write",  NULL, _hdr_cli_write_register, NULL,  "HDR: write register", CLI_GUEST},
	{"gain",  NULL, _hdr_cli_config_sdr2hdr_gain, NULL,  "HDR: config sdr2hdr gain value", CLI_GUEST},
	{"update",  NULL, _hdr_cli_update_HDR, NULL,  "HDR: update HDR HW", CLI_GUEST},
	{"stop",  NULL, _hdr_cli_stop_video, NULL,  "HDR: stop video", CLI_GUEST},
	{"debug",  NULL, _hdr_cli_debug, NULL,  "HDR: debug", CLI_GUEST},
	{"disable",  NULL, _hdr_cli_disableModule, NULL,  "HDR: disable module", CLI_GUEST},
	{"secure",  NULL, _hdr_cli_secure, NULL,  "HDR: secure debug", CLI_GUEST},
	{"irqid",  NULL, _hdr_irqinfo_secure, NULL,  "HDR: trustzone irqinfo debug", CLI_GUEST},
	{"disable_new_path",  NULL, _hdr_cli_disable_new_path_function, NULL,
			"HDR: disable new path function(path/clock)", CLI_GUEST},
	{"bt2020",  NULL, _hdr_cli_early_porting_bt2020, NULL,  "HDR: early porting BT2020", CLI_GUEST},
	{"sdr2hdr",  NULL, _hdr_cli_early_porting_sdr2hdr, NULL,  "HDR: early porting SDR2HDR", CLI_GUEST},
	{"hdr2sdr",  NULL, _hdr_cli_early_porting_hdr2sdr, NULL,  "HDR: early porting HDR2SDR", CLI_GUEST},
	{NULL, NULL, NULL, NULL, NULL, CLI_GUEST},
};

void hdr_cli_init(void)
{
	cli_register("hdr", items);
}

struct hdr_cli_info_struct *hdr_cli_get_info(void)
{
	return &gCliInfo;
}

