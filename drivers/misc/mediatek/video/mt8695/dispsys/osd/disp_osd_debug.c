#define LOG_TAG "OSD_DEBUG"
#include <linux/types.h>
#include <linux/kthread.h>

#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>


#include "disp_osd_debug.h"

#include "disp_osd_log.h"
#include "disp_osd_if.h"
#include "osd_hw.h"
#include "disp_reg.h"
#pragma GCC optimize("O0")

/*#include "ddp_reg.h"*/
/*#include "ddp_debug.h"*/
#define OSD_TRACE_BUFFER_SIZE               0x200000
#define OSD_TRACE_BUFFER_IDLE               (OSD_TRACE_BUFFER_SIZE - \
			(_arOsdTraceInfo.pcWritePtr - _arOsdTraceInfo.pcTraceBuf))

unsigned int osd_dbg_level;
unsigned int osd_irq_level;

static int osd_debug_init;
static struct dentry *osd_debugfs;
static char osd_dbg_buf[2048];
static char cmd_buf[512];
static char STR_HELP[] =
	"USAGE:\n"
	"       echo [ACTION]>/d/osd\n"
	"ACTION:\n";


void set_osd_log_level(unsigned int level, unsigned on)
{
	OSDMSG("set osd_dbg_level level %d  on %d\n", level, on);
	if (on) {
		osd_dbg_level |= (1 << level);
		OSDMSG("set osd_dbg_level  0x%x\n", osd_dbg_level);

	} else {
		osd_dbg_level &= 0;
		OSDMSG("set osd_dbg_level  0x%x\n", osd_dbg_level);
	}

	OSDMSG("set osd_dbg_level  0x%x\n", osd_dbg_level);
}

void set_osd_debug_irq_log_level(unsigned int level, unsigned on)
{
	OSDMSG("set set_osd_debug_irq_log_level  %d  on %d\n", level, on);
	if (on) {
		osd_irq_level |= (1 << level);
		OSDMSG("set osd_irq_level  0x%x\n", osd_irq_level);

	} else {
		osd_irq_level &= 0;
		OSDMSG("set osd_irq_level  0x%x\n", osd_irq_level);
	}

	OSDMSG("set osd_irq_level  0x%x\n", osd_irq_level);
}

unsigned int osd_dbg_log_level(void)
{
	return osd_dbg_level;
}

unsigned int osd_irq_log_level(void)
{
	return osd_irq_level;
}

typedef struct {
	UINT16 bfType;
	UINT32 bfSize;
	UINT16 bfReserved1;
	UINT16 bfReserved2;
	UINT32 bfOffBits;
	UINT32 biSize;
	UINT32 biWidth;
	UINT32 biHeight;
	UINT16 biPlanes;
	UINT16 biBitCount;
	UINT32 biCompression;
	UINT32 biSizeImage;
	UINT32 biXPelsPerMeter;
	UINT32 biYPelsPerMeter;
	UINT32 biClrUsed;
	UINT32 biClrImportant;
} BITMAPINFOHEADER;

int disp_osd_dump_plane(unsigned int plane)
{
	char file_name[512] = "osd.raw";
	unsigned int p_bitmap;
	unsigned int width, height, pitch;
	unsigned int size;
	unsigned int p_value;
	OSD_RGN_UNION_T region;
	struct file *filp;
	int i4WriteSize = 0;
	int ret = 0;
	BITMAPINFOHEADER t_bmpinfo;
	char *p_temp = NULL;

	_OSD_PLA_GetHeaderAddr(plane, &p_value);

	memcpy((void *)(&region), (void *)&p_value, sizeof(OSD_RGN_UNION_T));

	p_bitmap = (((region.au4Reg[1] & 0xFFFFFF) << 4) |
	((region.au4Reg[7]>>23 & 0x3) << 28) |
	((region.au4Reg[6]>>2 & 0x3) << 30));

	width = region.au4Reg[9] & 0x1fff;
	height = (region.au4Reg[9] & 0xfff0000) >> 16;
	pitch = (region.au4Reg[2] & 0x7ff) << 4;

	size = pitch * height;

	OSDMSG("[dump] p_bitmap:0x%x,width:%d,height:%d,size:0x%x\n", p_bitmap, width, height, size);

	filp = filp_open(file_name, O_CREAT | O_RDWR, 0);
	if (IS_ERR(filp)) {
		OSDMSG("[dump] <1.open file fail> handle !!\n");
		ret = -1;
	} else if (filp->f_op == NULL) {
		filp_close(filp, NULL);
		OSDMSG("[dump] <2.open file fail> handle op !!\n");
		ret = -1;
	} else {
		OSDMSG("[dump] Write file: %s, address=0x%x, len=0x%x\n", &file_name[0], p_bitmap, size);
		/*p_temp = VIRTUAL(p_bitmap);*/
		i4WriteSize = filp->f_op->write(filp, p_temp, size, &filp->f_pos);
		OSDMSG("[dump] iWriteSize =%d\n", i4WriteSize);
		filp_close(filp, NULL);
		OSDMSG("[dump] Close file:\t%s\n", &file_name[0]);
	}

	{
		UINT32 ui4_header_len = 0x36;

		t_bmpinfo.bfType = (UINT16)0x4d42;	/*bmp*/
		t_bmpinfo.bfReserved1 = 0;
		t_bmpinfo.bfReserved2 = 0;
		t_bmpinfo.biSize  = 0x28;
		t_bmpinfo.biWidth  = width;
		t_bmpinfo.biHeight	 = height;
		t_bmpinfo.biPlanes	 = 1;
		t_bmpinfo.biBitCount  = 32;
		t_bmpinfo.biCompression = 0;
		t_bmpinfo.biSizeImage  = width*height;
		t_bmpinfo.biXPelsPerMeter = 0;
		t_bmpinfo.biYPelsPerMeter = 0;
		t_bmpinfo.biClrUsed  = 0;
		t_bmpinfo.biClrImportant = 0;

		t_bmpinfo.bfSize = size + ui4_header_len;
		t_bmpinfo.bfOffBits = ui4_header_len;

		strcpy(file_name, "osd.bmp");
		filp = filp_open(file_name, O_CREAT | O_RDWR, 0);

		if (IS_ERR(filp)) {
			OSDMSG("[dump] <1.open file fail> handle !!\n");
			filp_close(filp, NULL);
			ret = -1;
		} else if (filp->f_op == NULL) {
			OSDMSG("[dump] <2.open file fail> handle op!!\n");
			filp_close(filp, NULL);
			ret = -1;
		} else {
			INT32 i4Idx;

			i4WriteSize = filp->f_op->write(filp, (UINT8 *)&t_bmpinfo, 2, &filp->f_pos);
			if (i4WriteSize != 2) {
				OSDMSG("[DRAM DUMP] Write header fail!!\n");
				filp_close(filp, NULL);
				ret = -1;
			}

			i4WriteSize = filp->f_op->write(filp, (UINT8 *)((&t_bmpinfo) + 4),
					ui4_header_len - 2, &filp->f_pos);
			if (i4WriteSize != ui4_header_len - 2) {
				OSDMSG("[DRAM DUMP] Write header fail!!\n");
				filp_close(filp, NULL);
				ret = -1;
			}

			OSDMSG("[dump] Write BMP header done!! current offset: 0x%x\n", (unsigned int)filp->f_pos);
			/*p_temp = VIRTUAL(p_bitmap);*/
			for (i4Idx = height - 1; i4Idx >= 0 ; i4Idx--) {
				i4WriteSize = filp->f_op->write(filp, (p_temp + i4Idx * pitch), pitch, &filp->f_pos);
				if (i4WriteSize != pitch) {
					OSDMSG("[DRAM DUMP] Write data fail!! index=%d\n", i4Idx);
					filp_close(filp, NULL);
					ret = -1;
				}
			}
		}
		filp_close(filp, NULL);
	}

	return ret;
}


int disp_osd_dump_base_info(void)
{
	return 0;
}

int disp_osd_dump_plane_info(unsigned int plane)
{
	if ((plane != OSD_PLANE_1) && (plane != OSD_PLANE_2)) {
		OSDERR("invalid plane:%d\n", plane);
		return -1;
	}

	OSDMSG("0xf0020000:%x,%x,%x,%x\n",
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[plane])),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[plane] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[plane] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_fmt_reg_base[plane] + 0xc)));

	OSDMSG("0xf0020x00+osd%d:%x,%x,%x,%x\n", (plane+1),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane])),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0xc)));

	OSDMSG("0xf0020x00+osd%d:%x,%x,%x,%x\n", (plane+1),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x20)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x24)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x28)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x2c)));

	OSDMSG("0xf0020x00+osd%d:%x,%x,%x,%x\n", (plane+1),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x30)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x34)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x38)),
		*((unsigned int *)(osd.osd_reg.osd_pln_reg_base[plane] + 0x3c)));

	OSDMSG("0xf0020x00+scl%d:%x,%x,%x,%x\n", (plane+1),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[plane])),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[plane] + 0x4)),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[plane] + 0x8)),
		*((unsigned int *)(osd.osd_reg.osd_scl_reg_base[plane] + 0xc)));

	return 0;
}

static const char dump_filename[] = "osd_trace.txt";

typedef struct {
	bool            fg_trace_en;
	char            *tracebuf;
	char            *write_pc;
	struct file     *ptrfile;
	spinlock_t      trace_spinlock;
} OSD_TRACE_INFO;

static OSD_TRACE_INFO osd_trace_info;
struct task_struct *osd_debug_task;

static int disp_osd_debug_thread(void *data)
{
	int lock = -1;
	unsigned int use_trace_size = 0;

	while (osd_trace_info.fg_trace_en) {
		lock = spin_is_locked(&osd_trace_info.trace_spinlock);
		if ((lock == 0) && (osd_trace_info.write_pc != osd_trace_info.tracebuf)) {
			mm_segment_t old_fs;
			int i4WriteSize;

			old_fs = get_fs(); /* get old fs, avoid USER/KERNEL file system conflict*/
			set_fs(KERNEL_DS);

			use_trace_size = (osd_trace_info.write_pc - osd_trace_info.tracebuf);

			i4WriteSize = osd_trace_info.ptrfile->f_op->write(
										osd_trace_info.ptrfile,
										osd_trace_info.tracebuf,
										use_trace_size,
										&(osd_trace_info.ptrfile->f_pos));
			osd_trace_info.write_pc = osd_trace_info.tracebuf;
			set_fs(old_fs);
		} else {
			OSDMSG("trace buffer -> locked, write file in next loop\n");
		}
		msleep(30);
	}
	OSDMSG("trace -> thread exit!\n");
	return 0;
}

int disp_osd_dump_byframe_init(unsigned int plane)
{
	struct file *filp;
	mm_segment_t old_fs;

	filp = filp_open(dump_filename, O_CREAT | O_RDWR, 0);
	if (IS_ERR(filp) || (filp->f_op == NULL)) {
		OSDERR("<1.Create file fail> handle!!\n");
		OSDERR("Try to open it!\n");
		filp = filp_open(dump_filename, O_RDWR, 0);

		if (IS_ERR(filp)) {
			OSDERR("<2.Open file fail> handle!!\n");
			return -1;
		} else if (filp->f_op == NULL) {
			OSDERR("<3.open file fail> filp->f_op!!\n");
			return -1;
		}
	}

	OSDMSG("Open file \"%s\" successfully!\n", dump_filename);

	osd_trace_info.ptrfile = filp;

    /*get old fs, avoid USER/KERNEL file system conflict*/
	old_fs = get_fs();
	set_fs(KERNEL_DS);

    /*seek relative to beginning of the file*/
	filp->f_op->llseek(filp, 0, SEEK_SET);

	set_fs(old_fs);
	spin_lock_init(&osd_trace_info.trace_spinlock);

	if (osd_trace_info.tracebuf == NULL) {
		osd_trace_info.tracebuf = vmalloc(OSD_TRACE_BUFFER_SIZE);
		if (osd_trace_info.tracebuf) {
			OSDERR("Allocate trace buffer failed!!\n");
			filp_close(filp, NULL);
			return -1;
		}
		memset(osd_trace_info.tracebuf, 0x0, OSD_TRACE_BUFFER_SIZE);
		osd_trace_info.write_pc = osd_trace_info.tracebuf;
	} else {
		memset(osd_trace_info.tracebuf, 0x0, OSD_TRACE_BUFFER_SIZE);
		osd_trace_info.write_pc = osd_trace_info.tracebuf;
		OSDMSG("trace buffer already allocate\n");
	}

	OSDMSG("alloc trace buffer addr=0x%p\n", osd_trace_info.tracebuf);

	if (!osd_debug_task) {
		osd_debug_task = kthread_create(disp_osd_debug_thread,
						 NULL, "disp_osd_debug_thread");
		if (osd_debug_task == NULL) {
			OSDERR("create debug thread fail\n");
			vfree(osd_trace_info.tracebuf);
			filp_close(filp, NULL);
			return -1;
		}
		wake_up_process(osd_debug_task);
		OSD_LOG_I("create disp_osd_debug_thread\n");
	}

	return 0;
}

int disp_osd_dump_byframe_uninit(unsigned int plane)
{
	/*make sure all log output*/

	/*close file*/

	/*free trace buffer*/

	/*destroy thread*/

	return 0;
}

int disp_osd_dump_reg_info_by_frame(unsigned int plane, char by_frame_en)
{
	if (by_frame_en)
		disp_osd_dump_byframe_init(plane);/*init*/
	else
		disp_osd_dump_byframe_uninit(plane);/*deinit*/

	osd_trace_info.fg_trace_en = by_frame_en;

	return 0;
}



int osd_trace_log(void)
{

	/*keep output all log*/
	return 0;
}

int osd_trace_write(void)
{
	/*dump reg into file*/
	char config_file[512] = "plane_config.txt";
	struct file *filp;
	int i4WriteSize;
	int ret = 0;

	filp = filp_open(config_file, O_CREAT | O_RDWR, 0);
	if (IS_ERR(filp)) {
		OSDMSG("<1.open file fail> handle !!\n");
		filp_close(filp, NULL);
		ret = -1;
	} else if (filp->f_op == NULL) {
		OSDMSG("<2.open file fail> handle op!!\n");
		filp_close(filp, NULL);
		ret = -1;
	} else {
		unsigned char *posd_reg_base;
		unsigned int size = 0x100;

		/*fmt  info*/
		posd_reg_base = (unsigned char *)osd.osd_reg.osd_fmt_reg_base;
		i4WriteSize = filp->f_op->write(filp, (posd_reg_base), size, &filp->f_pos);
		if (i4WriteSize != size) {
			OSDMSG("Write fmt fail!!\n");
			filp_close(filp, NULL);
			ret = -1;
		}
		/*header & scaler info*/
		posd_reg_base = (unsigned char *)osd.osd_reg.osd_pln_reg_base[OSD_PLANE_1];
		i4WriteSize = filp->f_op->write(filp, (posd_reg_base), size, &filp->f_pos);
		if (i4WriteSize != size) {
			OSDMSG("Write header fail!!\n");
			filp_close(filp, NULL);
			ret = -1;
		}

		posd_reg_base = (unsigned char *)osd.osd_reg.osd_scl_reg_base[OSD_PLANE_1];
		i4WriteSize = filp->f_op->write(filp, (posd_reg_base), 0x30, &filp->f_pos);
		if (i4WriteSize != size) {
			OSDMSG("Write header fail!!\n");
			filp_close(filp, NULL);
			ret = -1;
		}

		posd_reg_base = (unsigned char *)osd.osd_reg.osd_pln_reg_base[OSD_PLANE_2];
		i4WriteSize = filp->f_op->write(filp, (posd_reg_base), size, &filp->f_pos);
		if (i4WriteSize != size) {
			OSDMSG("Write header fail!!\n");
			filp_close(filp, NULL);
			ret = -1;
		}

		posd_reg_base = (unsigned char *)osd.osd_reg.osd_scl_reg_base[OSD_PLANE_2];
		i4WriteSize = filp->f_op->write(filp, (posd_reg_base), 0x30, &filp->f_pos);
		if (i4WriteSize != size) {
			OSDMSG("Write header fail!!\n");
			filp_close(filp, NULL);
			ret = -1;
		}
	}
	/*dump bmp into file + plus plane config info*/

	return 0;
}

/*************************************************************************/

static void disp_osd_process_dbg_opt(const char *opt)
{
	int ret = 0;
	char *p;

	if (strncmp(opt, "log_level:", 10) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 10;
		ret = kstrtoul(p, 10, (unsigned long int *)&value);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (value != 0)
			set_osd_log_level(value, 1);
		else
			set_osd_log_level(value, 0);
	} else if (strncmp(opt, "enable:", 7) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 7;
		ret = kstrtoul(p, 7, (unsigned long int *)&value);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (value == 1) { /*get status*/
			unsigned int pla_en[MAX_OSD_INPUT_CONFIG];

			_OSD_PLA_GetEnable(OSD_PLANE_1, &pla_en[OSD_PLANE_1]);
			_OSD_PLA_GetEnable(OSD_PLANE_2, &pla_en[OSD_PLANE_2]);
			OSDMSG("plane enable %x,%x.\n", pla_en[OSD_PLANE_1],
			pla_en[OSD_PLANE_2]);
		} else if (value == 2) { /*clk disable*/
			osd_engine_clk_enable(1, false);
		} else if (value == 3) { /*clk enable*/
			osd_engine_clk_enable(1, true);
		} else if (value == 4) { /*close plane 0*/
			_OSD_PLA_SetEnable(OSD_PLANE_1, 0);
			_OSD_PLA_SetUpdateStatus(OSD_PLANE_1, true);
			_OSD_AlwaysUpdateReg(OSD_PLANE_1, true);
			fg_debug_update =  true;
		} else if (value == 5) { /*enable plane 0*/
			_OSD_PLA_SetEnable(OSD_PLANE_2, 1);
			_OSD_PLA_SetUpdateStatus(OSD_PLANE_2, true);
			_OSD_AlwaysUpdateReg(OSD_PLANE_2, true);
			fg_debug_update =  true;
		} else if (value == 6) {
			disp_osd_start(NULL, OSD_PLANE_2);
			OSDMSG("disp_osd_start finish\n");
		} else if (value == 7) {
			disp_osd_stop(OSD_PLANE_2);
			OSDMSG("disp_osd_stop finish\n");
		}
	} else if (strncmp(opt, "reg:", 4) == 0) {
		unsigned int plane = 0;

		p = (char *)opt + 4;
		ret = kstrtoul(p, 4, (unsigned long int *)&plane);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_osd_dump_base_info();
		disp_osd_dump_plane_info(plane);
	} else if (strncmp(opt, "en_chksum", 9) == 0) {
		unsigned int en = 0;
		unsigned int i = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 9, (unsigned long int *)&en);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (en == 1) {
			fg_debug_update =  true;
			for (i = 0; i < OSD_PLANE_MAX_NUM; i++) {
				_OSD_BASE_SetOsdChkSumEn(i, true);
				_OSD_BASE_SetUpdate(i, true);
				_OSD_AlwaysUpdateReg(i, true);
			}
		} else if (en == 2) {
			fg_debug_update =  false;
			for (i = 0; i < OSD_PLANE_MAX_NUM; i++) {
				_OSD_BASE_SetOsdChkSumEn(i, false);
				_OSD_AlwaysUpdateReg(i, false);
			}
		} else {
			fg_debug_update =  false;
		}
	} else if (strncmp(opt, "get_chksum", 10) == 0) {
		uint32_t OsdChkSum, OsdScChkSum;

		_OSD_BASE_GetOsd3ChkSum(OSD_PLANE_1, &OsdChkSum);
		_OSD_BASE_GetOsdSc3ChkSum(OSD_PLANE_1, &OsdScChkSum);
		OSDMSG("0xf0020078:%x,%x\n", OsdChkSum, OsdScChkSum);

		_OSD_BASE_GetOsd2ChkSum(OSD_PLANE_2, &OsdChkSum);
		_OSD_BASE_GetOsdSc2ChkSum(OSD_PLANE_2, &OsdScChkSum);
		OSDMSG("0xf0020080:%x,%x\n", OsdChkSum, OsdScChkSum);
	} else if (strncmp(opt, "dump:", 5) == 0) {
		unsigned int value = 0;

		p = (char *)opt + 5;
		ret = kstrtoul(p, 5, (unsigned long int *)&value);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_osd_dump_plane(value);
	} else if (strncmp(opt, "dump_f_reg:", 11) == 0) {
		unsigned int plane = 0;
		unsigned int by_frame_en = false;

		p = (char *)opt + 11;
		ret = kstrtoul(p, 11, (unsigned long int *)&plane);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		p = (char *)opt + 13;
		ret = kstrtoul(p, 13, (unsigned long int *)&by_frame_en);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_osd_dump_reg_info_by_frame(plane, by_frame_en);
	} else if (strncmp(opt, "a_det_en:", 9) == 0) {
		unsigned int en = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 9, (unsigned long int *)&en);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		fg_osd_alpha_detect_en = en;
	} else if (strncmp(opt, "a_cls_en:", 9) == 0) {
		unsigned int en = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 9, (unsigned long int *)&en);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		fg_osd_always_update_frame[OSD_PLANE_1] = en;
		fg_osd_always_update_frame[OSD_PLANE_2] = en;
	} else if (strncmp(opt, "a_dmp_en:", 9) == 0) {
		unsigned int en = 0;

		p = (char *)opt + 9;
		ret = kstrtoul(p, 9, (unsigned long int *)&en);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}

		if (en  == 1) {
			fg_osd_debug_dump_en[OSD_PLANE_1] = true;
		} else if (en == 2) {
			fg_osd_debug_dump_en[OSD_PLANE_2] = true;
		} else {
			fg_osd_debug_dump_en[OSD_PLANE_1] = false;
			fg_osd_debug_dump_en[OSD_PLANE_2] = false;
		}
	} else if (strncmp(opt, "cnt:", 4) == 0) {
		unsigned int idx = 0;

		p = (char *)opt + 4;
		ret = kstrtoul(p, 4, (unsigned long int *)&idx);
		if (ret) {
			OSDMSG("%s: errno %d\n", __func__, ret);
			goto Error;
		}
		disp_osd_vsync_stastic(idx);
	}
	return;

Error:
	OSDMSG("parse command error!\n%s", STR_HELP);
}


static void disp_osd_process_dbg_cmd(char *cmd)
{
	char *tok;

	OSDMSG("cmd: %s\n", cmd);
	memset(osd_dbg_buf, 0, sizeof(osd_dbg_buf));
	while ((tok = strsep(&cmd, " ")) != NULL)
		disp_osd_process_dbg_opt(tok);
}


/*************************************************************************/

static ssize_t disp_osd_debug_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	if (strlen(osd_dbg_buf))
		return simple_read_from_buffer(ubuf, count, ppos, osd_dbg_buf, strlen(osd_dbg_buf));
	else
		return simple_read_from_buffer(ubuf, count, ppos, STR_HELP, strlen(STR_HELP));

}

static ssize_t disp_osd_debug_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	const int debug_bufmax = sizeof(cmd_buf) - 1;
	size_t ret;

	ret = count;

	if (count > debug_bufmax)
		count = debug_bufmax;

	if (copy_from_user(&cmd_buf, ubuf, count))
		return -EFAULT;

	cmd_buf[count] = 0;

	disp_osd_process_dbg_cmd(cmd_buf);

	return ret;
}

static int disp_osd_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static const struct file_operations osd_debug_fops = {
	.read = disp_osd_debug_read,
	.write = disp_osd_debug_write,
	.open = disp_osd_debug_open,
};


void disp_osd_debug_init(void)
{
	if (!osd_debug_init) {
		osd_debug_init = 1;
		osd_debugfs = debugfs_create_file("osd",
					      S_IFREG | S_IRUGO, NULL, (void *)0, &osd_debug_fops);

		OSDDBG("disp osd debug init, fs= %p\n", osd_debugfs);

	}
}

void disp_osd_debug_deinit(void)
{
	debugfs_remove(osd_debugfs);
}


