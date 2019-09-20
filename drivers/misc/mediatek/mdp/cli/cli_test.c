#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "disp_cli.h"


int init_cli_test(int argc, const char **argv)
{
	int i = 0;

	for (i = 0; i < argc; i++)
		pr_debug("init cli test argv[%d] is %s\n", i, argv[i]);
	return 0;
}
int cli_test_param(int argc, const char **argv)
{
	int i = 0;

	for (i = 0; i < argc; i++)
		pr_debug("cli_test_param argv[%d] is %s\n", i, argv[i]);
	return 0;
}

CLI_EXEC_T items[] = {
	{"init",  NULL, init_cli_test, NULL,  "cli Test: Init cli test", CLI_GUEST},
	{"testparam",  NULL, cli_test_param, NULL,  "cli Test: test param", CLI_GUEST},
	{NULL, NULL, NULL, NULL, NULL, CLI_GUEST},
};

static int __init cli_test_init(void)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	cli_register("cli_test", items);
#endif
	return 0;
}

static void __exit cli_test_exit(void)
{

}



module_init(cli_test_init);
module_exit(cli_test_exit);
MODULE_LICENSE("GPL");
