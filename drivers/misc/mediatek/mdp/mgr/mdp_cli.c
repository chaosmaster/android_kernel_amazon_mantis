#include <linux/kthread.h>
#include <linux/ratelimit.h>
#include <linux/printk.h>
#include "mdp_api.h"


#include "disp_cli.h"
#include "mdp_cli.h"
#include "mdp_log.h"
#include "mdp_driver.h"

struct mdp_cli_setting_struct mdp_cli = {
	.dump_imgresz_setting = false,
	.enable_log = false,
	.enable_mmp_debug = false,
	.print_record = false,
	.print_message = false,
};

struct mdp_cli_setting_struct *mdp_cli_get(void)
{
	return &mdp_cli;
}


int32_t mdp_cli_thread(void *p)
{
	return 0;
}


static int mdp_cli_dump_imgresz_setting(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli mdp.setting 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	mdp_cli.dump_imgresz_setting = (value != 0);
	pr_err_ratelimited("set mdp dump setting:%d\n", mdp_cli.dump_imgresz_setting);

	return 0;
}

static int mdp_cli_enable_log(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli mdp.log 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	mdp_cli.enable_log = (value != 0);
	pr_err_ratelimited("set mdp log_enable:%d\n", mdp_cli.enable_log);

	return 0;
}

static int mdp_cli_enable_mmp_debug(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli mdp.mmp 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	mdp_cli.enable_mmp_debug = (value != 0);
	pr_err_ratelimited("set mdp mmp:%d\n", mdp_cli.enable_mmp_debug);

	return 0;
}

static int mdp_cli_print_mdp_record(int argc, const char **argv)
{
	pr_err_ratelimited("enable MDP record debug\n");
	mdp_cli.print_record = true;
	return 0;
}

static int mdp_cli_debug(int argc, const char **argv)
{
	int value = 0;

	pr_err_ratelimited("mdp cli debug\n");
	if (argc != 2) {
		pr_err_ratelimited("usage: cli mdp.debug 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	mdp_driver_set_suspend(value);
	return 0;
}

static int mdp_cli_print_mdp_message(int argc, const char **argv)
{
	int value = 0;

	if (argc != 2) {
		pr_err_ratelimited("usage: cli mdp.print 1\n");
		return 0;
	}
	if (kstrtoint(argv[1], 0, &value) != 0) {
		pr_err_ratelimited("invalid input:%s\n", argv[1]);
		return 0;
	}

	mdp_cli.print_message = value;
	pr_err_ratelimited("print mdp message:%d\n", value);

	return 0;
}


static CLI_EXEC_T items[] = {
	{"setting",  NULL, mdp_cli_dump_imgresz_setting, NULL,  "MDP: dump imgresz setting", CLI_GUEST},
	{"log",  NULL, mdp_cli_enable_log, NULL,  "MDP: enable mdp log", CLI_GUEST},
	{"mmp",  NULL, mdp_cli_enable_mmp_debug, NULL,  "MDP: enable mmp debug", CLI_GUEST},
	{"record",  NULL, mdp_cli_print_mdp_record, NULL,  "MDP: enable mdp record print", CLI_GUEST},
	{"debug",  NULL, mdp_cli_debug, NULL,  "MDP: debug", CLI_GUEST},
	{"print",  NULL, mdp_cli_print_mdp_message, NULL,  "MDP: print message", CLI_GUEST},
	{NULL, NULL, NULL, NULL, NULL, CLI_GUEST},
};

void mdp_cli_init(void)
{
	cli_register("mdp", items);
}

