#ifndef __MDP_CLI_H__
#define __MDP_CLI_H__

struct mdp_cli_setting_struct {
	bool dump_imgresz_setting;
	bool enable_log;
	bool enable_mmp_debug;
	bool print_record;
	bool print_message;
};

void mdp_cli_init(void);
struct mdp_cli_setting_struct *mdp_cli_get(void);

#endif /* endof __MDP_CLI_H__ */
