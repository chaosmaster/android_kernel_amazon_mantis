#ifndef __DISP_VDP_CLI_H__
#define __DISP_VDP_CLI_H__
#include "disp_info.h"
struct vdp_cli_tgt_area_struct {
	bool enable;
	int layer_id;
	struct mtk_disp_range range;
};

struct vdp_cli_setting_struct {
	bool enable_mva_debug;
	bool enable_force_use_mdp;
	bool use_vdo3;
    bool enable_pts_debug;
	struct vdp_cli_tgt_area_struct target_area;
	int slow;
	int debug;
};

void vdp_cli_init(void);
struct vdp_cli_setting_struct *vdp_cli_get(void);
extern int debug_frame_count[2];	/* vdp show fps */
extern int config_frame_count[2];	/* vdp valid config */
extern int config_buffer_count[2];	/* vdp total buffer(contain invalid buffer)*/
extern int mdp_config_frame_count;	/* mdp config buffer */
extern int mdp_release_frame_count;	/* mdp handle done buffer */



#endif /* endof __DISP_VDP_CLI_H__ */
