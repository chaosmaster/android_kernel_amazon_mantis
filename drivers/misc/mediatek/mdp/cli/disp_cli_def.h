#ifndef __DISP_CLI_DEF_H__
#define __DISP_CLI_DEF_H__
#include <linux/types.h>
#define MAX_SHORT_STRING_LEN 20
#define MAX_STRING_LEN 50
#define MAX_LONG_STRING_LEN 300

#define MAX_ITME_COUNT 100
#define MAX_PARAM_COUNT 30
#define MAX_MODULE_COUNT 50

struct module_item_user_struct {
	char name[MAX_SHORT_STRING_LEN];
	char detail[MAX_STRING_LEN];
	char module_name[MAX_SHORT_STRING_LEN];
};

struct module_user_struct {
	struct module_item_user_struct items[MAX_ITME_COUNT];
	int item_count;
	char module_name[MAX_SHORT_STRING_LEN];
};


/* save all cli items */
struct cli_items_struct {
	int module_count;
	struct module_user_struct module[MAX_MODULE_COUNT];
};

/* execute a command */
struct cli_exec_command_struct {
	struct module_item_user_struct item;
	char param_string[MAX_LONG_STRING_LEN];
};

/* ioctl for userspace */
#if 0
#define CLI_IOCTL_MAGIC_NUMBER 'j'
#define CLI_GET_ITMES           (_IOW(CLI_IOCTL_MAGIC_NUMBER, 1, struct cli_items_struct))
#define CLI_EXEC_COMMAND (_IOW(CLI_IOCTL_MAGIC_NUMBER, 2, struct cli_exec_command_struct))
#else
#define CLI_GET_ITMES 233
#define CLI_EXEC_COMMAND 234
#endif

#endif /* endof __DISP_CLI_DEF_H__ */

