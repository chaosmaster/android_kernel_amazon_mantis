#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/ioctl.h>

#include "disp_cli.h"




static struct proc_dir_entry *gCliProc;
static bool global_cli_is_ready;

struct global_cli_info_struct {
	struct list_head gCliListHead;
	struct mutex lock;
};
struct global_cli_info_struct gCliInfo;

bool is_space(char c)
{
	if (c == ' ' || c == '\t' || c == '\n')
		return true;
	return false;
}

int32_t cli_register(char *module_name, CLI_EXEC_T item[])
{
	int i = 0;

	if (!global_cli_is_ready)
		return 0;

	for (i = 0; item[i].function != NULL; i++) {
		struct module_item_struct *pItem = vmalloc(sizeof(struct module_item_struct));

		snprintf(pItem->module_name, sizeof(pItem->module_name), "%s", module_name);
		pItem->module_case = item[i];
		list_add_tail(&pItem->list, &gCliInfo.gCliListHead);
	}
	return 0;
}

enum CLI_STATUS cli_core_fill_items(struct cli_items_struct *pItem)
{
	struct module_item_struct *pTempItem = NULL;
	struct module_item_user_struct *pUserItem = NULL;
	int module_index = 0;
	int i = 0;

	if (!global_cli_is_ready)
		return 0;

	pItem->module_count = 0;
	list_for_each_entry(pTempItem, &gCliInfo.gCliListHead, list) {
		for (i = 0; i < pItem->module_count; i++) {
			if (strcmp(pTempItem->module_name, pItem->module[i].module_name) == 0) {
				module_index = i;
				break;
			}
		}
		if (i == pItem->module_count) { /* find new module */
			module_index = i;
			pItem->module_count++;
			if (pItem->module_count >=  MAX_MODULE_COUNT) {
				CLI_ERR("module count is larger than max module count\n");
				return CLI_STATUS_TOO_MANY_MODULE;
			}
			snprintf(pItem->module[module_index].module_name,
				sizeof(pItem->module[module_index].module_name),
				"%s", pTempItem->module_name);
			pItem->module[module_index].item_count = 0;
		}
		/* find module index */
		pUserItem = &(pItem->module[module_index].items[pItem->module[module_index].item_count]);
		snprintf(pUserItem->name, sizeof(pUserItem->name), "%s", pTempItem->module_case.name);
		snprintf(pUserItem->module_name, sizeof(pUserItem->module_name), "%s", pTempItem->module_name);
		snprintf(pUserItem->detail, sizeof(pUserItem->detail), "%s", pTempItem->module_case.detail);
		pItem->module[module_index].item_count++;
	}

	return CLI_STATUS_OK;
}


enum CLI_STATUS cli_parse_command(char *pCommand, int *pWordCount, char *argv[], int size)
{
	int i = 0;
	char *pBeg = pCommand;
	char *pEnd = pCommand;
	*pWordCount = 0;
	while (pEnd < pCommand + strlen(pCommand)) {
		/* find first non space char */
		while (is_space(*pEnd) && (pEnd < pCommand + strlen(pCommand)))
			pEnd++;

		if (pEnd == pCommand + strlen(pCommand))
			break;
		pBeg = pEnd;

		/* find first space char */
		while (!is_space(*pEnd) && (pEnd < pCommand + strlen(pCommand)))
			pEnd++;

		/* find a word, [pBeg,pEnd) */
		memset(argv[*pWordCount], 0, size);
		for (i = 0; (i < pEnd - pBeg) && (i < size - 1); i++)
			argv[*pWordCount][i] = pBeg[i];

		(*pWordCount)++;
		if (*pWordCount >= MAX_PARAM_COUNT) {
			CLI_ERR("param count too many larger than %d\n", MAX_PARAM_COUNT);
			return CLI_STATUS_PARSE_CMD_FAIL;
		}

		pBeg = pEnd - 1;
	}

	return CLI_STATUS_OK;
}

enum CLI_STATUS cli_handle_command(struct cli_exec_command_struct *pCommand)
{
	struct module_item_struct *pTempItem = NULL;
	int32_t status = CLI_STATUS_ITEM_NOT_FOUND;
	int argc = 0;
	int i = 0;
	char *argv[MAX_PARAM_COUNT] = {NULL};

	if (!global_cli_is_ready)
		return 0;

	for (i = 0; i < MAX_PARAM_COUNT; i++) {
		argv[i] = vmalloc(MAX_STRING_LEN);
		if (argv[i] == NULL) {
			/* allocat fail, need to release all allcated buffer and return */
			while (--i >= 0)
				vfree(argv[i]);
			return CLI_STATUS_ALLOCATE_BUFFER_FAIL;
		}
	}
	list_for_each_entry(pTempItem, &gCliInfo.gCliListHead, list) {
		if (strcmp(pTempItem->module_name, pCommand->item.module_name) == 0 &&
			strcmp(pTempItem->module_case.name, pCommand->item.name) == 0) {
			/* since pCommand->param_string is parse from userspace,
			** param_string may not end with \0.
			** kernel needs to make sure pCommand->param_string ends with '\0' before use it.
			*/
			pCommand->param_string[MAX_LONG_STRING_LEN - 1] = '\0';

			/* find the execute item. parse & execute. */
			status = cli_parse_command(pCommand->param_string, &argc, argv, MAX_STRING_LEN);
			if (status != 0) {
				CLI_ERR("cli_parse_command fail:%d\n", status);
				break;
			}
			status = pTempItem->module_case.function(argc, (const char **)argv);
			if (status != 0) {
				CLI_ERR("exeute function return non-zeor:%d\n", status);
				status = CLI_STATUS_CALLBACK_ERROR;
				break;
			}
			status = CLI_STATUS_OK;
			break;
		}
	}

	for (i = 0; i < MAX_PARAM_COUNT; i++)
		vfree(argv[i]);
	return status;
}

static long cli_ioctl(struct file *pFile, unsigned int code, unsigned long param)
{
	enum CLI_STATUS status = CLI_STATUS_OK;
	struct cli_items_struct *pItems = vmalloc(sizeof(struct cli_items_struct));
	struct cli_exec_command_struct *pCommand = vmalloc(sizeof(struct cli_exec_command_struct));

	if (!global_cli_is_ready)
		return 0;
	pItems = vmalloc(sizeof(struct cli_items_struct));
	if (pItems == NULL)
		return 0;
	pCommand = vmalloc(sizeof(struct cli_exec_command_struct));
	if (pCommand == NULL)
		return 0;
	switch (code) {
	case CLI_GET_ITMES:
		status = cli_core_fill_items(pItems);
		if (status != CLI_STATUS_OK)
			break;
		if (copy_to_user((void *)param, (void *)pItems, sizeof(struct cli_items_struct))) {
			CLI_ERR("copy to user fail\n");
			status = CLI_STATUS_COPY_TO_USER_FAIL;
		}
		break;
	case CLI_EXEC_COMMAND:
		if (copy_from_user(pCommand, (void *)param, sizeof(struct cli_exec_command_struct))) {
			CLI_ERR("copy from user fail\n");
			status = CLI_STATUS_COPY_FROM_USER_FAIL;
			break;
		}
		status = cli_handle_command(pCommand);
		if (status != CLI_STATUS_OK)
			CLI_ERR("exec command fail:%d\n", status);

		break;
	default:
		CLI_ERR("unrecognised ioctl 0x%x\n", code);
		status = CLI_STATUS_UNRECOGNIZED_IOCTL;
		break;
	}
	return status;
}

#ifdef CONFIG_COMPAT
static long cli_ioctl_compat(struct file *pFile, unsigned int code, unsigned long param)
{
	switch (code) {
	case CLI_GET_ITMES:
	case CLI_EXEC_COMMAND:
		return cli_ioctl(pFile, code, param);
	default:
		CLI_ERR("undefined compat ioctl:0x%x\n", code);
		return -2;
	}

	return 0;
}
#endif


static const struct file_operations cli_fop = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = cli_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = cli_ioctl_compat,
#endif
};

enum CLI_STATUS cli_create_fs(void)
{
	enum CLI_STATUS status = CLI_STATUS_OK;

	do {
		gCliProc = proc_create("cli", 0644, NULL, &cli_fop);
		if (gCliProc == NULL) {
			CLI_ERR("create /proc/cli fail\n");
			status = CLI_STATUS_CREATE_PROC_CLI_FAIL;
			break;
		}
	} while (0);

	return status;
}

static int __init cli_init(void)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT

	/* for userdebug load add cli driver */
	enum CLI_STATUS status = CLI_STATUS_OK;

	INIT_LIST_HEAD(&gCliInfo.gCliListHead);
	mutex_init(&gCliInfo.lock);


	status = cli_create_fs();
	if (status != CLI_STATUS_OK)
		return (int)status;

	global_cli_is_ready = true;
#endif
	return 0;
}

static void __exit cli_exit(void)
{

}

subsys_initcall(cli_init);
module_exit(cli_exit);

MODULE_DESCRIPTION("MTK CLI driver");
MODULE_AUTHOR("Harry<jiaguang.zhang@mediatek.com>");
MODULE_LICENSE("GPL");
