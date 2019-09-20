#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/vmalloc.h>
#include<linux/seq_file.h>
#include<linux/proc_fs.h>

#include "mdp_proc_print.h"
#include "mdp_def.h"
#include "mdp_log.h"
#include "mdp_cli.h"



static DEFINE_MUTEX(MDP_PROC_PRINT_MUTEX);
struct mdp_proc_print_struct global_mdp_proc_print_list;

void mdp_print(char *pBuffer)
{
#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->print_message)
		return;
#else
	return;
#endif

	mdp_proc_add_error("%s", pBuffer);
}

void mdp_printf(char *fmt, ...)
{
	va_list arg;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->print_message)
		return;
#else
	return;
#endif

	va_start(arg, fmt);
	mdp_proc_add_error(fmt, arg);
	va_end(arg);
}
enum MDP_TASK_STATUS mdp_proc_add_error(char *fmt, ...)
{
	unsigned int len = 512;
	va_list arg;

	if (!global_mdp_proc_print_list.error_ready_to_use)
			return MDP_TASK_STATUS_OK;

	mutex_lock(&MDP_PROC_PRINT_MUTEX);
	if (global_mdp_proc_print_list.error->count + len >
			global_mdp_proc_print_list.error->bufferSize) {
		char *pNewBuffer = vmalloc(global_mdp_proc_print_list.error->bufferSize + PAGESIZE);

		if (!pNewBuffer)
			return MDP_TASK_STATUS_ALLOCATE_ERROR_BUFFER_FAIL;
		memset(pNewBuffer, 0, global_mdp_proc_print_list.error->bufferSize + PAGESIZE);
		memcpy(pNewBuffer,
			global_mdp_proc_print_list.error->printBuffer,
			global_mdp_proc_print_list.error->count);
		vfree(global_mdp_proc_print_list.error->printBuffer);
		global_mdp_proc_print_list.error->printBuffer = pNewBuffer;
		global_mdp_proc_print_list.error->bufferSize += PAGESIZE;
	}
	va_start(arg, fmt);
	global_mdp_proc_print_list.error->count +=
		vsnprintf(global_mdp_proc_print_list.error->printBuffer + global_mdp_proc_print_list.error->count,
		len, fmt, arg);
	va_end(arg);
	mutex_unlock(&MDP_PROC_PRINT_MUTEX);
	return 0;
}

enum MDP_TASK_STATUS mdp_proc_add_time_item(struct mdp_task_struct *pTask)
{
	struct task_time_item_struct *pTime = NULL;

#ifdef CONFIG_MTK_CLI_DEBUG_SUPPORT
	if (!mdp_cli_get()->print_record)
		return MDP_TASK_STATUS_OK;
#else
	return MDP_TASK_STATUS_OK;
#endif

	if (!global_mdp_proc_print_list.record_ready_to_use)
				return MDP_TASK_STATUS_OK;

	pTime = vmalloc(sizeof(struct task_time_item_struct));

	if (pTime == NULL)
		return MDP_TASK_STATUS_ALLOCATE_TIME_ITEM_FAIL;

	memcpy(pTime, &pTask->time, sizeof(struct task_time_item_struct));
	snprintf(pTime->taskString, sizeof(pTime->taskString), "%s", pTask->mdp_command.taskString);

	mutex_lock(&global_mdp_proc_print_list.record->lock);
	if (global_mdp_proc_print_list.record->record_count > SHOW_RECORD_COUNT) {
		struct task_time_item_struct *pTempTime;

		pTempTime = list_first_entry(&global_mdp_proc_print_list.record->listHead,
			struct task_time_item_struct, list);
		list_del_init(&pTempTime->list);
		vfree(pTempTime);
		global_mdp_proc_print_list.record->record_count--;
	}
	list_add_tail(&pTime->list,  &global_mdp_proc_print_list.record->listHead);
	global_mdp_proc_print_list.record->record_count++;
	mutex_unlock(&global_mdp_proc_print_list.record->lock);
	return 0;
}

int64_t mdp_proc_get_time_in_us(uint64_t start, uint64_t end)
{
	int64_t _duration = end - start;

	do_div(_duration, 1000);
	return _duration;
}

int mdp_proc_print_show_record(struct seq_file *seq, void *v)
{
	struct task_time_item_struct *pTime;

	if (!global_mdp_proc_print_list.record_ready_to_use)
		return MDP_TASK_STATUS_OK;

	mutex_lock(&global_mdp_proc_print_list.record->lock);
	list_for_each_entry(pTime, &global_mdp_proc_print_list.record->listHead, list) {
		seq_printf(seq, "task %s start[0x%llx] interval[%lld]us trigger[0x%llx] interval[%lld]us gotIRQ[0x%llx]",
				pTime->taskString, pTime->start,
				mdp_proc_get_time_in_us(pTime->start, pTime->trigger),
				pTime->trigger,
				mdp_proc_get_time_in_us(pTime->trigger, pTime->gotIRQ),
				pTime->gotIRQ);
		seq_printf(seq, " interval[%lld]us release[0x%llx] interval[%lld]us done[0x%llx]",
			mdp_proc_get_time_in_us(pTime->gotIRQ, pTime->releaseFence),
			pTime->releaseFence,
			mdp_proc_get_time_in_us(pTime->releaseFence, pTime->done),
			pTime->done);
		seq_printf(seq, " fence_time[%lld]us total[%lld]us",
			mdp_proc_get_time_in_us(pTime->start, pTime->releaseFence),
			mdp_proc_get_time_in_us(pTime->start, pTime->done));
		seq_putc(seq, '\n');
	}
	mutex_unlock(&global_mdp_proc_print_list.record->lock);
	return 0;
}

int mdp_proc_print_show_error(struct seq_file *seq, void *v)
{
	struct proc_debug *pDebug = global_mdp_proc_print_list.error;

	if (!global_mdp_proc_print_list.error_ready_to_use)
		return 0;

	seq_printf(seq, "\n%s\n", pDebug->name);
	seq_printf(seq, "%s\n", pDebug->printBuffer);
	seq_putc(seq, '\n');

	return 0;
}

int mdp_proc_print_open_error(struct inode *node, struct file *file)
{
	return single_open(file, mdp_proc_print_show_error, node->i_private);
}

int mdp_proc_print_open_record(struct inode *node, struct file *file)
{
	return single_open(file, mdp_proc_print_show_record, node->i_private);
}

enum MDP_TASK_STATUS mdp_proc_init_error(void)
{
	global_mdp_proc_print_list.error = vmalloc(sizeof(struct proc_debug));
	if (!global_mdp_proc_print_list.error)
		return MDP_TASK_STATUS_ALLOCATE_ERROR_STRUCT_FAIL;

	memset(global_mdp_proc_print_list.error, 0, sizeof(struct proc_debug));
	global_mdp_proc_print_list.error->printBuffer = vmalloc(PAGESIZE);
	if (!global_mdp_proc_print_list.error->printBuffer)
		return MDP_TASK_STATUS_ALLOCATE_ERROR_BUFFER_FAIL;
	memset(global_mdp_proc_print_list.error->printBuffer, 0, PAGESIZE);

	global_mdp_proc_print_list.error->bufferSize = PAGESIZE;
	snprintf(global_mdp_proc_print_list.error->name,
		sizeof(global_mdp_proc_print_list.error->name),
		"%s", "MDP_ERR");

	return MDP_TASK_STATUS_OK;
}

enum MDP_TASK_STATUS mdp_proc_init_record(void)
{
	global_mdp_proc_print_list.record = vmalloc(sizeof(struct mdp_proc_record_struct));
	if (global_mdp_proc_print_list.record == NULL) {
		MDP_ERR("create mdp_proc_record_struct fail\n");
		return MDP_TASK_STATUS_ALLOCATE_TIME_ITEM_FAIL;
	}
	memset(global_mdp_proc_print_list.record, 0, sizeof(struct mdp_proc_record_struct));
	INIT_LIST_HEAD(&global_mdp_proc_print_list.record->listHead);
	mutex_init(&global_mdp_proc_print_list.record->lock);

	return MDP_TASK_STATUS_OK;
}

enum MDP_TASK_STATUS mdp_proc_print_init(void)
{
	enum MDP_TASK_STATUS status = MDP_TASK_STATUS_OK;

	do {
		status = mdp_proc_init_error();
		if (status != MDP_TASK_STATUS_OK)
			break;
		global_mdp_proc_print_list.error_ready_to_use = true;

		status = mdp_proc_init_record();
		if (status != MDP_TASK_STATUS_OK)
			break;
		global_mdp_proc_print_list.record_ready_to_use = true;
	} while (0);

	return status;
}

void mdp_proc_print_exit(void)
{
}
