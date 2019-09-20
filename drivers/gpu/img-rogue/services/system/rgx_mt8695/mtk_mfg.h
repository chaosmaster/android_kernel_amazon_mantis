#ifndef _MTK_MFG_H_
#define _MTK_MFG_H_


void mfg_write(u32 offset, u32 val);
void mfg_dump_regs_seq(struct seq_file *m, const char *prefix);


/*
 * Called by GPU DDK for power mgt.
 */
void mtk_mfg_enable_gpu(void);
void mtk_mfg_disable_gpu(void);

void mtk_mfg_dump_regs(const char *prefix);

unsigned long mtk_mfg_get_freq(void);
unsigned long mtk_mfg_get_snap_freq(void);
void mtk_mfg_set_freq(unsigned long freq, bool hopping);
bool mtk_mfg_dvfs_idle(void);

int mtk_mfg_get_volt(void);
int mtk_mfg_get_snap_volt(void);
int mtk_mfg_set_volt(int volt);

unsigned long  mtk_mfg_calculate_power(unsigned long  volt, unsigned long freq);

bool mtk_mfg_is_ready(void);
bool mtk_mfg_is_hwapm(void);
bool mtk_mfg_can_set_volt(void);

void mtk_mfg_disable_bw_ultra(void);
void mtk_mfg_enable_bw_ultra(void);
bool mtk_mfg_is_bw_ultra(void);

int mtk_mfg_init(void);

#endif
