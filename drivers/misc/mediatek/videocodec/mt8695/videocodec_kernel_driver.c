/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <asm/page.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
/* #include <mach/x_define_irq.h> */
#include <linux/wait.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
/* #include <linux/earlysuspend.h> */
#include "sync_write.h"
/* #include "mach/mt_reg_base.h" */
#ifdef CONFIG_OF
#include <linux/clk.h>
#else
#include "mach/mt_clkmgr.h"
#endif
#ifdef CONFIG_MTK_HIBERNATION
#include <mtk_hibernate_dpm.h>
#endif

#include "videocodec_kernel_driver.h"
#include <asm/cacheflush.h>
#include <linux/io.h>
#include <asm/sizes.h>
#include "val_types_private.h"
#include "hal_types_private.h"
#include "val_api_private.h"
#include "val_log.h"
#include "drv_api.h"

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/pm_runtime.h>
#if IS_ENABLED(CONFIG_COMPAT)
#include <linux/uaccess.h>
#include <linux/compat.h>
#endif
#include "mtk_iommu.h"
#include "smi.h"

#define VDO_HW_WRITE(ptr, data)     mt_reg_sync_writel(data, ((void __iomem *)ptr))
#define VDO_HW_READ(ptr)           (*((volatile unsigned int * const)(ptr)))

#define VCODEC_DEVNAME     "Vcodec"
#define VDECDISP_DEVNAME "VDecDisp"
#define MT8695_VCODEC_DEV_MAJOR_NUMBER 160	/* 189 */
/* #define VENC_USE_L2C */
#define VDEC_LARB_COUNT 2

#define VDEC_CLOSE_DEBUG_CODE 0
static dev_t vcodec_devno = MKDEV(MT8695_VCODEC_DEV_MAJOR_NUMBER, 0);
static struct cdev *vcodec_cdev;
static struct class *vcodec_class;
static struct device *vcodec_device;
static struct platform_device *my_pdev;
static struct platform_device *pimudev[VDEC_LARB_COUNT];

static DEFINE_MUTEX(IsOpenedLock);
static DEFINE_MUTEX(PWRLock);
static DEFINE_MUTEX(VdecHWLock);
static DEFINE_MUTEX(VdecHWLatLock);
static DEFINE_MUTEX(L2CLock);
static DEFINE_MUTEX(DecEMILock);
static DEFINE_MUTEX(DriverOpenCountLock);
static DEFINE_MUTEX(DecCoreHWLockEventTimeoutLock);
static DEFINE_MUTEX(DecLatHWLockEventTimeoutLock);

static DEFINE_MUTEX(VdecLatPWRLock);
static DEFINE_MUTEX(VdecCorePWRLock);

static DEFINE_SPINLOCK(DecIsrLock);
static DEFINE_SPINLOCK(DecIsrLatLock);
#if VDEC_CLOSE_DEBUG_CODE
static DEFINE_SPINLOCK(LockDecHWCountLock);
static DEFINE_SPINLOCK(DecISRCountLock);
#endif

static VAL_MUTEX_T DecHWLockMutex;
static VAL_MUTEX_T DecLatHWLockMutex;


static VAL_EVENT_T DecHWLockEvent;	/* mutex : HWLockEventTimeoutLock */
static VAL_EVENT_T DecHWLatLockEvent;	/* mutex : HWLockEventTimeoutLock for LAX */
static VAL_EVENT_T DecIsrEvent;	/* mutex : HWLockEventTimeoutLock */
static VAL_EVENT_T DecLatIsrEvent;	/* mutex : HWLockEventTimeoutLock */
static VAL_INT32_T MT8695Driver_Open_Count;	/* mutex : DriverOpenCountLock */
static VAL_UINT32_T gu4PWRCounter;	/* mutex : PWRLock */
static VAL_UINT32_T gu4DecEMICounter;	/* mutex : DecEMILock */
static VAL_UINT32_T gu4L2CCounter;	/* mutex : L2CLock */
static VAL_BOOL_T bIsOpened = VAL_FALSE;	/* mutex : IsOpenedLock */

static VAL_UINT32_T gu4VdecPWRCounter;	/* mutex : VdecCorePWRLock */
static VAL_UINT32_T gu4VdecLatPWRCounter;	/* mutex : VdecLatPWRLock */

static VAL_UINT32_T gu4VdecLockThreadId;

/*#define MT8695_VCODEC_DEBUG*/
#ifdef MT8695_VCODEC_DEBUG
#undef VCODEC_DEBUG
#define VCODEC_DEBUG printk
#undef MODULE_MFV_LOGD
#define MODULE_MFV_LOGD  printk
#else
#define VCODEC_DEBUG(...)
#undef MODULE_MFV_LOGD
#define MODULE_MFV_LOGD(...)
#endif

/* VDEC virtual base address */
#define VDEC_BASE_PHY   0x16000000
#define VDEC_REGION     0x40000

#define HW_BASE         0x16000000
#define HW_REGION       0x40000

#define INFO_BASE       0x16000000
#define INFO_REGION     0x1000

VAL_ULONG_T KVA_VDEC_LAT_MISC_BASE, KVA_VDEC_MISC_BASE, KVA_VDEC_VLD_BASE, KVA_VDEC_BASE;
VAL_ULONG_T VDEC_SOC_GCON_BASE, VDEC_CORE_GCON_BASE;
VAL_UINT32_T VDEC_IRQ_ID, VDEC_LAT_IRQ_ID;

#ifdef CONFIG_OF
static struct clk *clk_vdec_sel;
static struct clk *clk_vdec_slow_sel;
static struct clk *clk_vdec_lae_sel;
static struct clk *clk_vdec_pll;
static struct clk *clk_osd_pll_d2;
#endif

static void enable_soc_core_gcon(void)
{
	VDO_HW_WRITE(VDEC_CORE_GCON_BASE + 0 * 4, 0x1);
	VDO_HW_WRITE(VDEC_SOC_GCON_BASE + 40 * 4, 0x1);
	VDO_HW_WRITE(VDEC_SOC_GCON_BASE + 42 * 4, 0x1);
}
void vdec_power_on_for_smi(void)
{
#ifdef CONFIG_OF
	pm_runtime_get_sync(&my_pdev->dev);
	/* clk_vdec_sys */
	clk_prepare_enable(clk_vdec_sel);
	clk_prepare_enable(clk_vdec_slow_sel);
	clk_prepare_enable(clk_vdec_lae_sel);
	enable_soc_core_gcon();
	mtk_smi_larb_get(&pimudev[0]->dev);
	mtk_smi_larb_get(&pimudev[1]->dev);
#endif

}

void vdec_power_on(void)
{
	mutex_lock(&VdecCorePWRLock);
	gu4VdecPWRCounter++;
	mutex_unlock(&VdecCorePWRLock);

#ifdef CONFIG_OF
	/* clk_vdec_sys */
	clk_prepare_enable(clk_vdec_sel);
	clk_prepare_enable(clk_vdec_slow_sel);
	clk_prepare_enable(clk_vdec_lae_sel);
	enable_soc_core_gcon();
	mtk_smi_larb_get(&pimudev[0]->dev);
	mtk_smi_larb_get(&pimudev[1]->dev);
#endif

}

void vdec_lat_power_on(void)
{
	mutex_lock(&VdecLatPWRLock);
	gu4VdecLatPWRCounter++;
	mutex_unlock(&VdecLatPWRLock);
#ifdef CONFIG_OF
	/* clk_lae_sys */
	clk_prepare_enable(clk_vdec_sel);
	clk_prepare_enable(clk_vdec_slow_sel);
	clk_prepare_enable(clk_vdec_lae_sel);
	enable_soc_core_gcon();
	mtk_smi_larb_get(&pimudev[0]->dev);
	mtk_smi_larb_get(&pimudev[1]->dev);
#endif

}

void vdec_power_off_for_smi(void)
{
#ifdef CONFIG_OF
		/* clk_vdec_sys */
	mtk_smi_larb_put(&pimudev[0]->dev);
	mtk_smi_larb_put(&pimudev[1]->dev);
	clk_disable_unprepare(clk_vdec_sel);
	clk_disable_unprepare(clk_vdec_slow_sel);
	clk_disable_unprepare(clk_vdec_lae_sel);
	pm_runtime_put_sync(&my_pdev->dev);
#endif
}

void vdec_power_off(void)
{
	VAL_UINT32_T u4TempCounter = 0;

	mutex_lock(&VdecCorePWRLock);
	u4TempCounter =  gu4VdecPWRCounter;
	if (gu4VdecPWRCounter != 0)  {
		gu4VdecPWRCounter--;
	}
	mutex_unlock(&VdecCorePWRLock);

	if (u4TempCounter != 0)  {
#ifdef CONFIG_OF
		/* clk_vdec_sys */
		mtk_smi_larb_put(&pimudev[0]->dev);
		mtk_smi_larb_put(&pimudev[1]->dev);
		clk_disable_unprepare(clk_vdec_sel);
		clk_disable_unprepare(clk_vdec_slow_sel);
		clk_disable_unprepare(clk_vdec_lae_sel);
#endif
	}
}

void vdec_lat_power_off(void)
{
	VAL_UINT32_T u4TempCounter = 0;

	mutex_lock(&VdecLatPWRLock);
	u4TempCounter = gu4VdecLatPWRCounter;
	if (gu4VdecLatPWRCounter != 0)  {
		gu4VdecLatPWRCounter--;
	}
	mutex_unlock(&VdecLatPWRLock);

	if (u4TempCounter != 0)  {
#ifdef CONFIG_OF
		/* Central power off */
		mtk_smi_larb_put(&pimudev[0]->dev);
		mtk_smi_larb_put(&pimudev[1]->dev);
		clk_disable_unprepare(clk_vdec_sel);
		clk_disable_unprepare(clk_vdec_slow_sel);
		clk_disable_unprepare(clk_vdec_lae_sel);
#endif
	}
}

void dec_isr(void)
{
	VAL_RESULT_T eValRet;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlags;
	VAL_ULONG_T ulFlagsISR;
	VAL_UINT32_T u4TempDecISRCount = 0;

	VAL_ULONG_T ulFlagsLockHW;
	VAL_UINT32_T u4TempLockDecHWCount = 0;
#endif
	/*VAL_UINT32_T u4CgStatus = 0;*/
	VAL_UINT32_T u4DecDoneStatus = 0;
	/*
	 *u4CgStatus = VDO_HW_READ(KVA_VDEC_GCON_BASE);
	 *if ((u4CgStatus & 0x10) != 0) {
	 *	MODULE_MFV_LOGE("[MFV][ERROR] DEC ISR, VDEC active is not 0x0 (0x%08x)", u4CgStatus);
	 *	return;
	 *}
	 */
	u4DecDoneStatus = VDO_HW_READ(KVA_VDEC_MISC_BASE + 0xA4);
	if ((u4DecDoneStatus & (0x1 << 16)) != 0x10000) {
		MODULE_MFV_LOGE("[MFV][ERROR] DEC ISR, Decode done status is not 0x1 (0x%08x)",
			 u4DecDoneStatus);
		return;
	}


#if VDEC_CLOSE_DEBUG_CODE
	spin_lock_irqsave(&DecISRCountLock, ulFlagsISR);
	gu4DecISRCount++;
	u4TempDecISRCount = gu4DecISRCount;
	spin_unlock_irqrestore(&DecISRCountLock, ulFlagsISR);

	spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
	u4TempLockDecHWCount = gu4LockDecHWCount;
	spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);
	/*
	 *if (u4TempDecISRCount != u4TempLockDecHWCount) {
	 *	MODULE_MFV_LOGE("[INFO] Dec ISRCount: 0x%x, LockHWCount:0x%x\n",
	 *		u4TempDecISRCount, u4TempLockDecHWCount);
	 *}
	 */
#endif
	/* Clear interrupt */
	VDO_HW_WRITE(KVA_VDEC_MISC_BASE + 41 * 4, VDO_HW_READ(KVA_VDEC_MISC_BASE + 41 * 4) | 0x11);
	VDO_HW_WRITE(KVA_VDEC_MISC_BASE + 41 * 4, VDO_HW_READ(KVA_VDEC_MISC_BASE + 41 * 4) & ~0x10);


#if VDEC_CLOSE_DEBUG_CODE
	spin_lock_irqsave(&DecIsrLock, ulFlags);
#endif
	eValRet = eVideoSetEvent(&DecIsrEvent, sizeof(VAL_EVENT_T));
	if (eValRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] Core ISR set DecIsrEvent error\n");
#if VDEC_CLOSE_DEBUG_CODE
	spin_unlock_irqrestore(&DecIsrLock, ulFlags);
#endif
}

void dec_lat_isr(void)
{
	VAL_RESULT_T eValRet;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlags;
	VAL_ULONG_T ulFlagsISR;
	VAL_UINT32_T u4TempDecISRCount = 0;

	VAL_ULONG_T ulFlagsLockHW;
	VAL_UINT32_T u4TempLockDecHWCount = 0;
#endif
	/*VAL_UINT32_T u4CgStatus = 0;*/
	VAL_UINT32_T u4DecDoneStatus = 0;
	/*
	 *u4CgStatus = VDO_HW_READ(KVA_VDEC_GCON_BASE);
	 *if ((u4CgStatus & 0x10) != 0) {
	 *	MODULE_MFV_LOGE("[MFV][ERROR] DEC ISR, VDEC active is not 0x0 (0x%08x)", u4CgStatus);
	 *	return;
	 *}
	 */
	u4DecDoneStatus = VDO_HW_READ(KVA_VDEC_LAT_MISC_BASE + 0xA4);
	if ((u4DecDoneStatus & (0x1 << 16)) != 0x10000) {
		MODULE_MFV_LOGE("[MFV][ERROR] LAT DEC ISR, Decode done status is not 0x1 (0x%08x)",
			 u4DecDoneStatus);
		return;
	}

#if VDEC_CLOSE_DEBUG_CODE
	spin_lock_irqsave(&DecISRCountLock, ulFlagsISR);
	gu4DecISRCount++;
	u4TempDecISRCount = gu4DecISRCount;
	spin_unlock_irqrestore(&DecISRCountLock, ulFlagsISR);

	spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
	u4TempLockDecHWCount = gu4LockDecHWCount;
	spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);
	/*
	 *if (u4TempDecISRCount != u4TempLockDecHWCount) {
	 *	MODULE_MFV_LOGE("[INFO] Dec ISRCount: 0x%x, LockHWCount:0x%x\n",
	 *		u4TempDecISRCount, u4TempLockDecHWCount);
	 *}
	 */
#endif
	/* Clear interrupt */
	VDO_HW_WRITE(KVA_VDEC_LAT_MISC_BASE + 41 * 4, VDO_HW_READ(KVA_VDEC_LAT_MISC_BASE + 41 * 4) | 0x11);
	VDO_HW_WRITE(KVA_VDEC_LAT_MISC_BASE + 41 * 4, VDO_HW_READ(KVA_VDEC_LAT_MISC_BASE + 41 * 4) & ~0x10);

#if VDEC_CLOSE_DEBUG_CODE
	spin_lock_irqsave(&DecIsrLatLock, ulFlags);
#endif
	eValRet = eVideoSetEvent(&DecLatIsrEvent, sizeof(VAL_EVENT_T));
	if (eValRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] LAT ISR set DecLatIsrEvent error\n");
#if VDEC_CLOSE_DEBUG_CODE
	spin_unlock_irqrestore(&DecIsrLatLock, ulFlags);
#endif
}

static irqreturn_t video_intr_dlr(int irq, void *dev_id)
{
	dec_isr();
	return IRQ_HANDLED;
}

static irqreturn_t video_lat_intr_dlr(int irq, void *dev_id)
{
	dec_lat_isr();
	return IRQ_HANDLED;
}

static long vcodec_lockhw_dec_fail(VAL_HW_LOCK_T rHWLock, VAL_UINT32_T FirstUseDecHW)
{
	MODULE_MFV_LOGE("[ERROR] VCODEC_LOCKHW, DecHWLockEvent TimeOut, CurrentTID = %d\n", current->pid);
	if (FirstUseDecHW != 1) {
		mutex_lock(&VdecHWLock);
		if (grVcodecDecHWLock.pvHandle == 0) {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			MODULE_MFV_LOGE("[WARNING] VCODEC_LOCKHW, maybe mediaserver restart before, please check!!\n");
		} else {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			MODULE_MFV_LOGE("[WARNING] VCODEC_LOCKHW, someone use HW, and check timeout value!!\n");
		}
		mutex_unlock(&VdecHWLock);
	}

	return 0;
}

static long vcodec_lockhw_lat_fail(VAL_HW_LOCK_T rHWLock, VAL_UINT32_T FirstUseDecHW)
{
	MODULE_MFV_LOGE("[ERROR] VCODEC_LAT_LOCKHW, DecHWLockEvent TimeOut, CurrentTID = %d\n", current->pid);
	if (FirstUseDecHW != 1) {
		mutex_lock(&VdecHWLatLock);
		if (grVcodecDecHWLaxLock.pvHandle == 0) {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			MODULE_MFV_LOGE("[WARNING] VCODEC_LAT_LOCKHW, maybe mediaserver restart before, please check!!\n");
		} else {
			/* Add one line comment for avoid kernel coding style, WARNING:BRACES: */
			MODULE_MFV_LOGE("[WARNING] VCODEC_LAT_LOCKHW, someone use HW, and check timeout value!!\n");
		}
		mutex_unlock(&VdecHWLatLock);
	}

	return 0;
}

static long vcodec_alloc_non_cache_buffer(unsigned long arg)
{
	VAL_LONG_T ret;
	VAL_UINT8_T *user_data_addr;
	VAL_MEMORY_T rTempMem;

	MODULE_MFV_LOGE("VCODEC_ALLOC_NON_CACHE_BUFFER + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_ALLOC_NON_CACHE_BUFFER, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	rTempMem.u4ReservedSize /*kernel va */  =
	    (VAL_ULONG_T) dma_alloc_coherent(0, rTempMem.u4MemSize, (dma_addr_t *) &rTempMem.pvMemPa, GFP_KERNEL);
	if ((rTempMem.u4ReservedSize == 0) || (rTempMem.pvMemPa == 0)) {
		MODULE_MFV_LOGE("[ERROR] dma_alloc_coherent fail in VCODEC_ALLOC_NON_CACHE_BUFFER\n");
		return -EFAULT;
	}

	MODULE_MFV_LOGD("kernel va = 0x%lx, kernel pa = 0x%lx, memory size = %lu\n",
		 (VAL_ULONG_T) rTempMem.u4ReservedSize,
		 (VAL_ULONG_T) rTempMem.pvMemPa,
		 (VAL_ULONG_T) rTempMem.u4MemSize);

	/* mutex_lock(&NonCacheMemoryListLock);
	  * Add_NonCacheMemoryList(rTempMem.u4ReservedSize,
	  * (VAL_UINT32_T)rTempMem.pvMemPa, (VAL_UINT32_T)rTempMem.u4MemSize, 0, 0);
	  * mutex_unlock(&NonCacheMemoryListLock);
	 */

	ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_ALLOC_NON_CACHE_BUFFER, copy_to_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGE("VCODEC_ALLOC_NON_CACHE_BUFFER - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_free_non_cache_buffer(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_MEMORY_T rTempMem;
	VAL_LONG_T ret;

	MODULE_MFV_LOGE("VCODEC_FREE_NON_CACHE_BUFFER + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rTempMem, user_data_addr, sizeof(VAL_MEMORY_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_FREE_NON_CACHE_BUFFER, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	dma_free_coherent(0, rTempMem.u4MemSize, (void *)rTempMem.u4ReservedSize,
			  (dma_addr_t) rTempMem.pvMemPa);

	/* mutex_lock(&NonCacheMemoryListLock); */
	/* Free_NonCacheMemoryList(rTempMem.u4ReservedSize, (VAL_UINT32_T)rTempMem.pvMemPa); */
	/* mutex_unlock(&NonCacheMemoryListLock); */

	rTempMem.u4ReservedSize = 0;
	rTempMem.pvMemPa = NULL;

	ret = copy_to_user(user_data_addr, &rTempMem, sizeof(VAL_MEMORY_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_FREE_NON_CACHE_BUFFER, copy_to_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGE("VCODEC_FREE_NON_CACHE_BUFFER - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_lockhw(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_HW_LOCK_T rHWLock;
	VAL_RESULT_T eValRet;
	VAL_LONG_T ret;
	VAL_BOOL_T bLockedHW = VAL_FALSE;
	VAL_UINT32_T FirstUseDecHW = 0;
	VAL_TIME_T rCurTime;
	VAL_UINT32_T u4TimeInterval;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlagsLockHW;
#endif
	MODULE_MFV_LOGD("VCODEC_LOCKHW + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rHWLock, user_data_addr, sizeof(VAL_HW_LOCK_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_LOCKHW, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGD("LOCKHW eDriverType = %d\n", rHWLock.eDriverType);
	eValRet = VAL_RESULT_INVALID_ISR;
	if (rHWLock.eDriverType == VAL_DRIVER_TYPE_MP4_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_MP1_MP2_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VC1_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VC1_ADV_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP9_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP8_DEC) {
		while (bLockedHW == VAL_FALSE) {
			mutex_lock(&DecCoreHWLockEventTimeoutLock);
			if (DecHWLockEvent.u4TimeoutMs == 1) {
				MODULE_MFV_LOGE("[NOT ERROR][VCODEC_LOCKHW] First Use Dec HW!!\n");
				FirstUseDecHW = 1;
			} else {
				FirstUseDecHW = 0;
			}
			mutex_unlock(&DecCoreHWLockEventTimeoutLock);

			if (FirstUseDecHW == 1)
				eValRet = eVideoWaitEvent(&DecHWLockEvent, sizeof(VAL_EVENT_T));

			eVideoWaitMutex(&DecHWLockMutex, sizeof(VAL_MUTEX_T));
			/*mutex_lock(&DecCoreHWLockEventTimeoutLock);*/
			if (DecHWLockEvent.u4TimeoutMs != 1000) {
				DecHWLockEvent.u4TimeoutMs = 1000;
				FirstUseDecHW = 1;
			} else {
				FirstUseDecHW = 0;
			}

/*
			mutex_lock(&VdecHWLock);

			// one process try to lock twice
			if (grVcodecDecHWLock.pvHandle ==
				(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T)rHWLock.pvHandle)) {
				MODULE_MFV_LOGE("[WARNING] one decoder instance try to lock twice\n");
				MODULE_MFV_LOGE("may cause lock HW timeout!! instance = 0x%lx, CurrentTID = %d\n",
				(VAL_ULONG_T) grVcodecDecHWLock.pvHandle, current->pid);
			}
			mutex_unlock(&VdecHWLock);
*/
			if (FirstUseDecHW == 0) {
				MODULE_MFV_LOGD("VCODEC_LOCKHW, not first time use HW, timeout = %d\n",
					DecHWLockEvent.u4TimeoutMs);
				eValRet = eVideoWaitEvent(&DecHWLockEvent, sizeof(VAL_EVENT_T));
			}
			/*mutex_unlock(&DecCoreHWLockEventTimeoutLock);*/
			eVideoReleaseMutex(&DecHWLockMutex, sizeof(VAL_MUTEX_T));


			if (eValRet == VAL_RESULT_INVALID_ISR) {
				ret = vcodec_lockhw_dec_fail(rHWLock, FirstUseDecHW);
				if (ret) {
					MODULE_MFV_LOGE("[ERROR] vcodec_lockhw_dec_fail failed: %lu\n", ret);
					return -EFAULT;
				}
			} else if (eValRet == VAL_RESULT_RESTARTSYS) {
				MODULE_MFV_LOGE("[WARNING] VAL_RESULT_RESTARTSYS return when HWLock!!\n");
				return -ERESTARTSYS;
			}

			mutex_lock(&VdecHWLock);
			if (grVcodecDecHWLock.pvHandle == 0) {	/* No one holds dec hw lock now */
				gu4VdecLockThreadId = current->pid;
				grVcodecDecHWLock.pvHandle =
				(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T)rHWLock.pvHandle);

				grVcodecDecHWLock.eDriverType = rHWLock.eDriverType;
				eVideoGetTimeOfDay(&grVcodecDecHWLock.rLockedTime, sizeof(VAL_TIME_T));

				MODULE_MFV_LOGD("No process use dec HW, so current process can use HW\n");
				MODULE_MFV_LOGD("LockInstance = 0x%lx CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
				(VAL_ULONG_T) grVcodecDecHWLock.pvHandle,
					 current->pid,
					 grVcodecDecHWLock.rLockedTime.u4Sec,
					 grVcodecDecHWLock.rLockedTime.u4uSec);

				bLockedHW = VAL_TRUE;
				if (eValRet == VAL_RESULT_INVALID_ISR && FirstUseDecHW != 1) {
					MODULE_MFV_LOGE("[WARNING] reset power/irq when HWLock!!\n");
					vdec_power_off();
					disable_irq(VDEC_IRQ_ID);
				}
				vdec_power_on();

#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT	/* Morris Yang moved to TEE */
				if (rHWLock.bSecureInst == VAL_FALSE) {
					if (request_irq(VDEC_IRQ_ID,
						(irq_handler_t) video_intr_dlr,
						IRQF_TRIGGER_LOW, VCODEC_DEVNAME, NULL) < 0) {
						MODULE_MFV_LOGE("[ERROR] error to request dec irq\n");
					} else {
						MODULE_MFV_LOGD("success to request dec irq\n");
					}
					/* enable_irq(VDEC_IRQ_ID); */
				}
#else
				enable_irq(VDEC_IRQ_ID);
#endif
			} else {	/* Another one holding dec hw now */
				MODULE_MFV_LOGE("[NOT ERROR][VCODEC_LOCKHW] E\n");
				eVideoGetTimeOfDay(&rCurTime, sizeof(VAL_TIME_T));
				u4TimeInterval = (((((rCurTime.u4Sec - grVcodecDecHWLock.rLockedTime.u4Sec) * 1000000)
					+ rCurTime.u4uSec) - grVcodecDecHWLock.rLockedTime.u4uSec) / 1000);

				MODULE_MFV_LOGD("[VCODEC_LOCKHW] someone use dec HW, and check timeout value\n");
				MODULE_MFV_LOGD("TimeInterval(ms) = %d, TimeOutValue(ms)) = %d\n",
					u4TimeInterval, rHWLock.u4TimeoutMs);

				MODULE_MFV_LOGE("Lock Instance = 0x%lx, Lock TID = %d, CurrentTID = %d\n",
				(VAL_ULONG_T) grVcodecDecHWLock.pvHandle, gu4VdecLockThreadId, current->pid);

				MODULE_MFV_LOGE("rLockedTime(%d s, %d us), rCurTime(%d s, %d us)\n",
				grVcodecDecHWLock.rLockedTime.u4Sec, grVcodecDecHWLock.rLockedTime.u4uSec,
				rCurTime.u4Sec, rCurTime.u4uSec);

				/* 2012/12/16. Cheng-Jung Never steal hardware lock */
			}
			mutex_unlock(&VdecHWLock);
#if VDEC_CLOSE_DEBUG_CODE
			spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
			gu4LockDecHWCount++;
			spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);
#endif
		}
	}  else {
		MODULE_MFV_LOGE("[VCODEC_LOCKHW] [WARNING] Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("[VCODEC_LOCKHW] - Tid = %d\n", current->pid);

	return 0;
}

static long vcodec_lat_lockhw(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_HW_LOCK_T rHWLock;
	VAL_RESULT_T eValRet;
	VAL_LONG_T ret;
	VAL_BOOL_T bLockedHW = VAL_FALSE;
	VAL_UINT32_T FirstUseDecHW = 0;
	VAL_TIME_T rCurTime;
	VAL_UINT32_T u4TimeInterval;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlagsLockHW;
#endif
	MODULE_MFV_LOGD("VCODEC_LAT_LOCKHW + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rHWLock, user_data_addr, sizeof(VAL_HW_LOCK_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_LAT_LOCKHW, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGD("LOCKHW eDriverType = %x\n", rHWLock.eDriverType);
	eValRet = VAL_RESULT_INVALID_ISR;
	if (rHWLock.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP9_DEC) {
		while (bLockedHW == VAL_FALSE) {
			mutex_lock(&DecLatHWLockEventTimeoutLock);
			if (DecHWLatLockEvent.u4TimeoutMs == 1) {
				MODULE_MFV_LOGE("[NOT ERROR][VCODEC_LAT_LOCKHW] First Use Dec HW!!\n");
				FirstUseDecHW = 1;
			} else {
				FirstUseDecHW = 0;
			}
			mutex_unlock(&DecLatHWLockEventTimeoutLock);

			if (FirstUseDecHW == 1)
				eValRet = eVideoWaitEvent(&DecHWLatLockEvent, sizeof(VAL_EVENT_T));

			/*mutex_lock(&DecLatHWLockEventTimeoutLock);*/
			eVideoWaitMutex(&DecLatHWLockMutex, sizeof(VAL_MUTEX_T));

			if (DecHWLatLockEvent.u4TimeoutMs != 1000) {
				DecHWLatLockEvent.u4TimeoutMs = 1000;
				FirstUseDecHW = 1;
			} else {
				FirstUseDecHW = 0;
			}

/*
			mutex_lock(&VdecHWLatLock);

			// one process try to lock twice
			if (grVcodecDecHWLaxLock.pvHandle ==
				(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T)rHWLock.pvHandle)) {
				MODULE_MFV_LOGE("[WARNING] lat decoder instance try to lock twice\n");
				MODULE_MFV_LOGE("lax may cause lock HW timeout!! instance = 0x%lx, CurrentTID = %d\n",
				(VAL_ULONG_T) grVcodecDecHWLaxLock.pvHandle, current->pid);
			}
			mutex_unlock(&VdecHWLatLock);
*/
			if (FirstUseDecHW == 0) {
				MODULE_MFV_LOGD("VCODEC_LAT_LOCKHW, not first time use HW, timeout = %d\n",
					DecHWLatLockEvent.u4TimeoutMs);
				eValRet = eVideoWaitEvent(&DecHWLatLockEvent, sizeof(VAL_EVENT_T));
			}

			/*mutex_unlock(&DecLatHWLockEventTimeoutLock);*/
			eVideoReleaseMutex(&DecLatHWLockMutex, sizeof(VAL_MUTEX_T));

			if (eValRet == VAL_RESULT_INVALID_ISR) {
				ret = vcodec_lockhw_lat_fail(rHWLock, FirstUseDecHW);
				if (ret) {
					MODULE_MFV_LOGE("[ERROR] vcodec_lockhw_dec_fail failed: %lu\n", ret);
					return -EFAULT;
				}
			} else if (eValRet == VAL_RESULT_RESTARTSYS) {
				MODULE_MFV_LOGE("[WARNING] VAL_RESULT_RESTARTSYS return when HWLock!!\n");
				return -ERESTARTSYS;
			}

			mutex_lock(&VdecHWLatLock);
			if (grVcodecDecHWLaxLock.pvHandle == 0) {	/* No one holds dec hw lock now */
				gu4VdecLockThreadId = current->pid;
				grVcodecDecHWLaxLock.pvHandle =
				(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T)rHWLock.pvHandle);

				grVcodecDecHWLaxLock.eDriverType = rHWLock.eDriverType;
				eVideoGetTimeOfDay(&grVcodecDecHWLaxLock.rLockedTime, sizeof(VAL_TIME_T));

				MODULE_MFV_LOGD("No process use dec HW, so current process can use HW\n");
				MODULE_MFV_LOGD("LockInstance = 0x%lx CurrentTID = %d, rLockedTime(s, us) = %d, %d\n",
				(VAL_ULONG_T) grVcodecDecHWLaxLock.pvHandle,
					 current->pid,
					 grVcodecDecHWLaxLock.rLockedTime.u4Sec,
					 grVcodecDecHWLaxLock.rLockedTime.u4uSec);

				bLockedHW = VAL_TRUE;
				if (eValRet == VAL_RESULT_INVALID_ISR && FirstUseDecHW != 1) {
					MODULE_MFV_LOGE("[WARNING] reset power/irq when HWLock!!\n");
					vdec_power_off();
					disable_irq(VDEC_LAT_IRQ_ID);
				}
				vdec_lat_power_on();
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT	/* Morris Yang moved to TEE */
				if (rHWLock.bSecureInst == VAL_FALSE) {
					if (request_irq(VDEC_LAT_IRQ_ID,
						(irq_handler_t) video_lat_intr_dlr,
						IRQF_TRIGGER_LOW, VCODEC_DEVNAME, NULL) < 0) {
						MODULE_MFV_LOGE("[ERROR] error to request lax irq\n");
					} else {
						MODULE_MFV_LOGD("success to request lax irq\n");
					}
					/* enable_irq(VDEC_IRQ_ID); */
				}
#else
				enable_irq(VDEC_LAT_IRQ_ID);
#endif
			} else {	/* Another one holding dec hw now */
				MODULE_MFV_LOGE("[NOT ERROR][VCODEC_LAT_LOCKHW] E\n");
				eVideoGetTimeOfDay(&rCurTime, sizeof(VAL_TIME_T));
				u4TimeInterval = (((((rCurTime.u4Sec - grVcodecDecHWLaxLock.rLockedTime.u4Sec)
						* 1000000) + rCurTime.u4uSec) - grVcodecDecHWLaxLock.rLockedTime.u4uSec)
						/ 1000);

				MODULE_MFV_LOGD("[VCODEC_LAX_LOCKHW] someone use dec HW, and check timeout value\n");
				MODULE_MFV_LOGD("TimeInterval(ms) = %d, TimeOutValue(ms)) = %d\n",
					u4TimeInterval, rHWLock.u4TimeoutMs);

				MODULE_MFV_LOGE("Lock Instance = 0x%lx, Lock TID = %d, CurrentTID = %d\n",
				(VAL_ULONG_T) grVcodecDecHWLaxLock.pvHandle, gu4VdecLockThreadId, current->pid);

				MODULE_MFV_LOGE("rLockedTime(%d s, %d us), rCurTime(%d s, %d us)\n",
				grVcodecDecHWLaxLock.rLockedTime.u4Sec, grVcodecDecHWLaxLock.rLockedTime.u4uSec,
				rCurTime.u4Sec, rCurTime.u4uSec);

				/* 2012/12/16. Cheng-Jung Never steal hardware lock */
			}
			mutex_unlock(&VdecHWLatLock);
#if VDEC_CLOSE_DEBUG_CODE
			spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
			gu4LockDecHWCount++;
			spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);
#endif
		}
	} else {
		MODULE_MFV_LOGE("[VCODEC_LAT_LOCKHW] [WARNING] Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("[VCODEC_LAT_LOCKHW] - Tid = %d\n", current->pid);

	return 0;
}

static long vcodec_unlockhw(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_HW_LOCK_T rHWLock;
	VAL_RESULT_T eValRet;
	VAL_LONG_T ret;

	MODULE_MFV_LOGD("VCODEC_UNLOCKHW + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rHWLock, user_data_addr, sizeof(VAL_HW_LOCK_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_UNLOCKHW, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGD("UNLOCKHW eDriverType = %d\n", rHWLock.eDriverType);
	eValRet = VAL_RESULT_INVALID_ISR;
	if (rHWLock.eDriverType == VAL_DRIVER_TYPE_MP4_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_MP1_MP2_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VC1_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VC1_ADV_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP9_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP8_DEC) {
		mutex_lock(&VdecHWLock);
		if (grVcodecDecHWLock.pvHandle ==
			(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T) rHWLock.pvHandle)) {
			grVcodecDecHWLock.pvHandle = 0;
			grVcodecDecHWLock.eDriverType = VAL_DRIVER_TYPE_NONE;
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT	/* Morris Yang moved to TEE */
			if (rHWLock.bSecureInst == VAL_FALSE) {
				/* disable_irq(VDEC_IRQ_ID); */

				free_irq(VDEC_IRQ_ID, NULL);
			}
#else
			disable_irq(VDEC_IRQ_ID);
#endif
			/* TODO: check if turning power off is ok */
			vdec_power_off();
		} else {	/* Not current owner */

			MODULE_MFV_LOGD("[ERROR] Not owner trying to unlock dec hardware 0x%lx\n",
				 pmem_user_v2p_video((VAL_ULONG_T) rHWLock.pvHandle));
			mutex_unlock(&VdecHWLock);
			return -EFAULT;
		}
		mutex_unlock(&VdecHWLock);
		eValRet = eVideoSetEvent(&DecHWLockEvent, sizeof(VAL_EVENT_T));
	}  else {
		MODULE_MFV_LOGE("[WARNING] VCODEC_UNLOCKHW Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("VCODEC_UNLOCKHW - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_lat_unlockhw(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_HW_LOCK_T rHWLock;
	VAL_RESULT_T eValRet;
	VAL_LONG_T ret;

	MODULE_MFV_LOGD("VCODEC_LAT_UNLOCKHW + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&rHWLock, user_data_addr, sizeof(VAL_HW_LOCK_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_LAT_UNLOCKHW, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	MODULE_MFV_LOGD("UNLOCKHW eDriverType = %x\n", rHWLock.eDriverType);
	eValRet = VAL_RESULT_INVALID_ISR;
	if (rHWLock.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		rHWLock.eDriverType == VAL_DRIVER_TYPE_VP9_DEC) {
		mutex_lock(&VdecHWLatLock);
		if (grVcodecDecHWLaxLock.pvHandle ==
			(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T) rHWLock.pvHandle)) {
			grVcodecDecHWLaxLock.pvHandle = 0;
			grVcodecDecHWLaxLock.eDriverType = VAL_DRIVER_TYPE_NONE;
#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT	/* Morris Yang moved to TEE */
			if (rHWLock.bSecureInst == VAL_FALSE) {
				/* disable_irq(VDEC_IRQ_ID); */

				free_irq(VDEC_LAT_IRQ_ID, NULL);
			}
#else
			disable_irq(VDEC_LAT_IRQ_ID);
#endif
			/* TODO: check if turning power off is ok */
			vdec_lat_power_off();
		} else {	/* Not current owner */

			MODULE_MFV_LOGD("[ERROR] Not owner trying to unlock dec hardware 0x%lx\n",
				 pmem_user_v2p_video((VAL_ULONG_T) rHWLock.pvHandle));
			mutex_unlock(&VdecHWLatLock);
			return -EFAULT;
		}
		mutex_unlock(&VdecHWLatLock);
		eValRet = eVideoSetEvent(&DecHWLatLockEvent, sizeof(VAL_EVENT_T));
	} else {
		MODULE_MFV_LOGE("[WARNING] VCODEC_LAX_UNLOCKHW Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("VCODEC_LAT_UNLOCKHW - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_waitisr(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_ISR_T val_isr;
	VAL_BOOL_T bLockedHW = VAL_FALSE;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlags;
#endif
	VAL_LONG_T ret;
	VAL_RESULT_T eValRet;

	MODULE_MFV_LOGD("VCODEC_WAITISR + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&val_isr, user_data_addr, sizeof(VAL_ISR_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_WAITISR, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	if (val_isr.eDriverType == VAL_DRIVER_TYPE_MP4_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_MP1_MP2_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_VC1_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_VC1_ADV_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_VP8_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_VP9_DEC) {
		mutex_lock(&VdecHWLock);
		if (grVcodecDecHWLock.pvHandle ==
			(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T) val_isr.pvHandle)) {
			bLockedHW = VAL_TRUE;
		} else {
		}
		mutex_unlock(&VdecHWLock);

		if (bLockedHW == VAL_FALSE) {
			MODULE_MFV_LOGE("[ERROR] DO NOT have HWLock, so return fail\n");
			return -EFAULT;
		}

#if VDEC_CLOSE_DEBUG_CODE
		spin_lock_irqsave(&DecIsrLock, ulFlags);
#endif
		DecIsrEvent.u4TimeoutMs = val_isr.u4TimeoutMs;
#if VDEC_CLOSE_DEBUG_CODE
		spin_unlock_irqrestore(&DecIsrLock, ulFlags);
#endif
		eValRet = eVideoWaitEvent(&DecIsrEvent, sizeof(VAL_EVENT_T));
		if (eValRet == VAL_RESULT_INVALID_ISR) {
			return -2;
		} else if (eValRet == VAL_RESULT_RESTARTSYS) {
			MODULE_MFV_LOGE("[WARNING] VAL_RESULT_RESTARTSYS return when WAITISR!!\n");
			return -ERESTARTSYS;
		}
	}  else {
		MODULE_MFV_LOGE("[WARNING] VCODEC_WAITISR Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("VCODEC_WAITISR - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_wait_lat_isr(unsigned long arg)
{
	VAL_UINT8_T *user_data_addr;
	VAL_ISR_T val_isr;
	VAL_BOOL_T bLockedHW = VAL_FALSE;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlags;
#endif
	VAL_LONG_T ret;
	VAL_RESULT_T eValRet;

	MODULE_MFV_LOGD("VCODEC_WAIT_LAT_ISR + tid = %d\n", current->pid);

	user_data_addr = (VAL_UINT8_T *) arg;
	ret = copy_from_user(&val_isr, user_data_addr, sizeof(VAL_ISR_T));
	if (ret) {
		MODULE_MFV_LOGE("[ERROR] VCODEC_WAIT_LAT_ISR, copy_from_user failed: %lu\n", ret);
		return -EFAULT;
	}

	if (val_isr.eDriverType == VAL_DRIVER_TYPE_HEVC_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_H264_DEC ||
		val_isr.eDriverType == VAL_DRIVER_TYPE_VP9_DEC) {
		mutex_lock(&VdecHWLatLock);
		if (grVcodecDecHWLaxLock.pvHandle ==
			(VAL_VOID_T *) pmem_user_v2p_video((VAL_ULONG_T) val_isr.pvHandle)) {
			bLockedHW = VAL_TRUE;
		} else {
		}
		mutex_unlock(&VdecHWLatLock);

		if (bLockedHW == VAL_FALSE) {
			MODULE_MFV_LOGE("[ERROR_LAX] DO NOT have HW_LaxLock, so return fail\n");
			return -EFAULT;
		}

#if VDEC_CLOSE_DEBUG_CODE
		spin_lock_irqsave(&DecIsrLatLock, ulFlags);
#endif
		DecLatIsrEvent.u4TimeoutMs = val_isr.u4TimeoutMs;
#if VDEC_CLOSE_DEBUG_CODE
		spin_unlock_irqrestore(&DecIsrLatLock, ulFlags);
#endif
		eValRet = eVideoWaitEvent(&DecLatIsrEvent, sizeof(VAL_EVENT_T));
		if (eValRet == VAL_RESULT_INVALID_ISR) {
			MODULE_MFV_LOGE("[ERROR] VAL_RESULT_INVALID_ISR return !!\n");
			return -2;
		} else if (eValRet == VAL_RESULT_RESTARTSYS) {
			MODULE_MFV_LOGE("[WARNING] VAL_RESULT_RESTARTSYS return when WAIT_LAX_ISR!!\n");
			return -ERESTARTSYS;
		}
	}  else {
		MODULE_MFV_LOGE("[WARNING] VCODEC_WAIT_LAT_ISR Unknown instance\n");
		return -EFAULT;
	}
	MODULE_MFV_LOGD("VCODEC_WAIT_LAT_ISR - tid = %d\n", current->pid);

	return 0;
}

static long vcodec_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	VAL_LONG_T ret;
	VAL_UINT8_T *user_data_addr;
	VAL_VCODEC_CORE_LOADING_T rTempCoreLoading;
	VAL_VCODEC_CPU_OPP_LIMIT_T rCpuOppLimit;
	VAL_INT32_T temp_nr_cpu_ids;
	VAL_POWER_T rPowerParam;

	switch (cmd) {
	case VCODEC_SET_THREAD_ID:
		MODULE_MFV_LOGE("[8695] VCODEC_SET_THREAD_ID [EMPTY] + tid = %d\n", current->pid);

		MODULE_MFV_LOGE("[8695] VCODEC_SET_THREAD_ID [EMPTY] - tid = %d\n", current->pid);
		break;

	case VCODEC_ALLOC_NON_CACHE_BUFFER:
		{
			ret = vcodec_alloc_non_cache_buffer(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_ALLOC_NON_CACHE_BUFFER failed! %lu\n", ret);
				return ret;
			}
		}
		break;

	case VCODEC_FREE_NON_CACHE_BUFFER:
		{
			ret = vcodec_free_non_cache_buffer(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_FREE_NON_CACHE_BUFFER failed! %lu\n", ret);
				return ret;
			}
		}
		break;

	case VCODEC_INC_DEC_EMI_USER:
		MODULE_MFV_LOGD("VCODEC_INC_DEC_EMI_USER + tid = %d\n", current->pid);

		mutex_lock(&DecEMILock);
		gu4DecEMICounter++;
		MODULE_MFV_LOGE("DEC_EMI_USER = %d\n", gu4DecEMICounter);
		user_data_addr = (VAL_UINT8_T *) arg;
		ret = copy_to_user(user_data_addr, &gu4DecEMICounter, sizeof(VAL_UINT32_T));
		if (ret) {
			MODULE_MFV_LOGE("[ERROR] VCODEC_INC_DEC_EMI_USER, copy_to_user failed: %lu\n",
				 ret);
			mutex_unlock(&DecEMILock);
			return -EFAULT;
		}
		mutex_unlock(&DecEMILock);

		MODULE_MFV_LOGD("VCODEC_INC_DEC_EMI_USER - tid = %d\n", current->pid);
		break;

	case VCODEC_DEC_DEC_EMI_USER:
		MODULE_MFV_LOGD("VCODEC_DEC_DEC_EMI_USER + tid = %d\n", current->pid);

		mutex_lock(&DecEMILock);
		gu4DecEMICounter--;
		MODULE_MFV_LOGE("DEC_EMI_USER = %d\n", gu4DecEMICounter);
		user_data_addr = (VAL_UINT8_T *) arg;
		ret = copy_to_user(user_data_addr, &gu4DecEMICounter, sizeof(VAL_UINT32_T));
		if (ret) {
			MODULE_MFV_LOGE("[ERROR] VCODEC_DEC_DEC_EMI_USER, copy_to_user failed: %lu\n",
				 ret);
			mutex_unlock(&DecEMILock);
			return -EFAULT;
		}
		mutex_unlock(&DecEMILock);

		MODULE_MFV_LOGD("VCODEC_DEC_DEC_EMI_USER - tid = %d\n", current->pid);
		break;

	case VCODEC_LOCKHW:
		{
			ret = vcodec_lockhw(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_LOCKHW failed! %lu\n", ret);
				return ret;
			}
		}
		break;
	case VCODEC_LAT_LOCKHW:
		{
			ret = vcodec_lat_lockhw(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_LAX_LOCKHW failed! %lu\n", ret);
				return ret;
			}
		}
		break;

	case VCODEC_UNLOCKHW:
		{
			ret = vcodec_unlockhw(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_UNLOCKHW failed! %lu\n", ret);
				return ret;
			}
		}
		break;
	case VCODEC_LAT_UNLOCKHW:
		{
			ret = vcodec_lat_unlockhw(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_LAX_UNLOCKHW failed! %lu\n", ret);
				return ret;
			}
		}
		break;

	case VCODEC_INC_PWR_USER:
		{
			MODULE_MFV_LOGD("VCODEC_INC_PWR_USER + tid = %d\n", current->pid);
			user_data_addr = (VAL_UINT8_T *) arg;
			ret = copy_from_user(&rPowerParam, user_data_addr, sizeof(VAL_POWER_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_INC_PWR_USER, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			MODULE_MFV_LOGD("INC_PWR_USER eDriverType = %d\n", rPowerParam.eDriverType);

			MODULE_MFV_LOGD("VCODEC_INC_PWR_USER - tid = %d\n", current->pid);
		}
		break;

	case VCODEC_DEC_PWR_USER:
		{
			MODULE_MFV_LOGD("VCODEC_DEC_PWR_USER + tid = %d\n", current->pid);
			user_data_addr = (VAL_UINT8_T *) arg;
			ret = copy_from_user(&rPowerParam, user_data_addr, sizeof(VAL_POWER_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_DEC_PWR_USER, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			MODULE_MFV_LOGD("DEC_PWR_USER eDriverType = %d\n", rPowerParam.eDriverType);

			MODULE_MFV_LOGD("[8695] VCODEC_DEC_PWR_USER - tid = %d\n", current->pid);
		}
		break;

	case VCODEC_WAITISR:
		{
			ret = vcodec_waitisr(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_WAITISR failed! %lu\n", ret);
				return ret;
			}
		}
		break;
	case VCODEC_WAIT_LAT_ISR:
		{
			ret = vcodec_wait_lat_isr(arg);
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_WAIT_LAT_ISR failed! %lu\n", ret);
				return ret;
			}
		}
		break;

	case VCODEC_INITHWLOCK:
		{
			MODULE_MFV_LOGE("VCODEC_INITHWLOCK [EMPTY] + - tid = %d\n", current->pid);
			MODULE_MFV_LOGE("VCODEC_INITHWLOCK [EMPTY] - - tid = %d\n", current->pid);
		}
		break;

	case VCODEC_DEINITHWLOCK:
		{
			MODULE_MFV_LOGE("VCODEC_DEINITHWLOCK [EMPTY] + - tid = %d\n", current->pid);
			MODULE_MFV_LOGE("VCODEC_DEINITHWLOCK [EMPTY] - - tid = %d\n", current->pid);
		}
		break;

	case VCODEC_GET_CPU_LOADING_INFO:
		{
			VAL_UINT8_T *user_data_addr;
			VAL_VCODEC_CPU_LOADING_INFO_T _temp;

			MODULE_MFV_LOGD("VCODEC_GET_CPU_LOADING_INFO +\n");
			user_data_addr = (VAL_UINT8_T *) arg;
			/* TODO: */
			ret = copy_to_user(user_data_addr, &_temp, sizeof(VAL_VCODEC_CPU_LOADING_INFO_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_GET_CPU_LOADING_INFO, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}

			MODULE_MFV_LOGD("VCODEC_GET_CPU_LOADING_INFO -\n");
		}
		break;

	case VCODEC_GET_CORE_LOADING:
		{
			MODULE_MFV_LOGD("VCODEC_GET_CORE_LOADING + - tid = %d\n", current->pid);

			user_data_addr = (VAL_UINT8_T *) arg;
			ret = copy_from_user(&rTempCoreLoading, user_data_addr, sizeof(VAL_VCODEC_CORE_LOADING_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_GET_CORE_LOADING, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			/* tempory remark, must enable after function check-in */
			/* rTempCoreLoading.Loading = get_cpu_load(rTempCoreLoading.CPUid); */
			ret = copy_to_user(user_data_addr, &rTempCoreLoading, sizeof(VAL_VCODEC_CORE_LOADING_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_GET_CORE_LOADING, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}

			MODULE_MFV_LOGD("VCODEC_GET_CORE_LOADING - - tid = %d\n", current->pid);
		}
		break;

	case VCODEC_GET_CORE_NUMBER:
		{
			MODULE_MFV_LOGD("VCODEC_GET_CORE_NUMBER + - tid = %d\n", current->pid);

			user_data_addr = (VAL_UINT8_T *) arg;
			temp_nr_cpu_ids = nr_cpu_ids;
			ret = copy_to_user(user_data_addr, &temp_nr_cpu_ids, sizeof(int));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_GET_CORE_NUMBER, copy_to_user failed: %lu\n", ret);
				return -EFAULT;
			}
			MODULE_MFV_LOGD("VCODEC_GET_CORE_NUMBER - - tid = %d\n", current->pid);
		}
		break;
	case VCODEC_SET_CPU_OPP_LIMIT:
		{
			MODULE_MFV_LOGE("VCODEC_SET_CPU_OPP_LIMIT [EMPTY] + - tid = %d\n", current->pid);
			user_data_addr = (VAL_UINT8_T *) arg;
			ret = copy_from_user(&rCpuOppLimit, user_data_addr, sizeof(VAL_VCODEC_CPU_OPP_LIMIT_T));
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] VCODEC_SET_CPU_OPP_LIMIT, copy_from_user failed: %lu\n", ret);
				return -EFAULT;
			}
			MODULE_MFV_LOGE("+VCODEC_SET_CPU_OPP_LIMIT (%d, %d, %d), tid = %d\n",
				 rCpuOppLimit.limited_freq, rCpuOppLimit.limited_cpu,
				 rCpuOppLimit.enable, current->pid);
			/* TODO: Check if cpu_opp_limit is available */
			/* ret = cpu_opp_limit(EVENT_VIDEO,
			*rCpuOppLimit.limited_freq,
			*rCpuOppLimit.limited_cpu,
			*rCpuOppLimit.enable);
			*/
			if (ret) {
				MODULE_MFV_LOGE("[ERROR] cpu_opp_limit failed: %lu\n", ret);
				return -EFAULT;
			}
			MODULE_MFV_LOGE("-VCODEC_SET_CPU_OPP_LIMIT tid = %d, ret = %lu\n", current->pid, ret);
			MODULE_MFV_LOGE("VCODEC_SET_CPU_OPP_LIMIT [EMPTY] - - tid = %d\n", current->pid);
		}
		break;
	case VCODEC_MB:
		{
			mb();
		}
		break;
	default:
		MODULE_MFV_LOGE("========[ERROR] vcodec_ioctl default case======== %u\n", cmd);
		break;
	}
	return 0xFF;
}

#if IS_ENABLED(CONFIG_COMPAT)

typedef enum {
	VAL_HW_LOCK_TYPE = 0,
	VAL_POWER_TYPE,
	VAL_ISR_TYPE,
	VAL_MEMORY_TYPE
} STRUCT_TYPE;

typedef enum {
	COPY_FROM_USER = 0,
	COPY_TO_USER,
} COPY_DIRECTION;

typedef struct COMPAT_VAL_HW_LOCK {
	compat_uptr_t pvHandle;	/* /< [IN]     The video codec driver handle */
	compat_uint_t u4HandleSize;	/* /< [IN]     The size of video codec driver handle */
	compat_uptr_t pvLock;	/* /< [IN/OUT] The Lock discriptor */
	compat_uint_t u4TimeoutMs;	/* /< [IN]     The timeout ms */
	compat_uptr_t pvReserved;	/* /< [IN/OUT] The reserved parameter */
	compat_uint_t u4ReservedSize;	/* /< [IN]     The size of reserved parameter structure */
	compat_uint_t eDriverType;	/* /< [IN]     The driver type */
	char bSecureInst;	/* /< [IN]     True if this is a secure instance // MTK_SEC_VIDEO_PATH_SUPPORT */
} COMPAT_VAL_HW_LOCK_T;

typedef struct COMPAT_VAL_POWER {
	compat_uptr_t pvHandle;	/* /< [IN]     The video codec driver handle */
	compat_uint_t u4HandleSize;	/* /< [IN]     The size of video codec driver handle */
	compat_uint_t eDriverType;	/* /< [IN]     The driver type */
	char fgEnable;		/* /< [IN]     Enable or not. */
	compat_uptr_t pvReserved;	/* /< [IN/OUT] The reserved parameter */
	compat_uint_t u4ReservedSize;	/* /< [IN]     The size of reserved parameter structure */
	/* VAL_UINT32_T        u4L2CUser;              ///< [OUT]    The number of power user right now */
} COMPAT_VAL_POWER_T;

typedef struct COMPAT_VAL_ISR {
	compat_uptr_t pvHandle;	/* /< [IN]     The video codec driver handle */
	compat_uint_t u4HandleSize;	/* /< [IN]     The size of video codec driver handle */
	compat_uint_t eDriverType;	/* /< [IN]     The driver type */
	compat_uptr_t pvIsrFunction;	/* /< [IN]     The isr function */
	compat_uptr_t pvReserved;	/* /< [IN/OUT] The reserved parameter */
	compat_uint_t u4ReservedSize;	/* /< [IN]     The size of reserved parameter structure */
	compat_uint_t u4TimeoutMs;	/* /< [IN]     The timeout in ms */
	compat_uint_t u4IrqStatusNum;	/* /< [IN]     The num of return registers when HW done */
	compat_uint_t u4IrqStatus[IRQ_STATUS_MAX_NUM];	/* /< [IN/OUT] The value of return registers when HW done */
} COMPAT_VAL_ISR_T;

typedef struct COMPAT_VAL_MEMORY {
	compat_uint_t eMemType;	/* /< [IN]     The allocation memory type */
	compat_ulong_t u4MemSize;	/* /< [IN]     The size of memory allocation */
	compat_uptr_t pvMemVa;	/* /< [IN/OUT] The memory virtual address */
	compat_uptr_t pvMemPa;	/* /< [IN/OUT] The memory physical address */
	compat_uint_t eAlignment;	/* /< [IN]     The memory byte alignment setting */
	compat_uptr_t pvAlignMemVa;	/* /< [IN/OUT] The align memory virtual address */
	compat_uptr_t pvAlignMemPa;	/* /< [IN/OUT] The align memory physical address */
	compat_uint_t eMemCodec;	/* /< [IN]     The memory codec for VENC or VDEC */
	compat_uint_t i4IonShareFd;
	compat_uptr_t pIonBufhandle;
	compat_uptr_t pvReserved;	/* /< [IN/OUT] The reserved parameter */
	compat_ulong_t u4ReservedSize;	/* /< [IN]     The size of reserved parameter structure */
} COMPAT_VAL_MEMORY_T;

static int compat_copy_struct(STRUCT_TYPE eType,
			      COPY_DIRECTION eDirection, void __user *data32, void __user *data)
{
	compat_uint_t u;
	compat_ulong_t l;
	compat_uptr_t p;
	VAL_VOID_T *ptr;
	char c;
	int err = 0;

	switch (eType) {
	case VAL_HW_LOCK_TYPE:
		{
			if (eDirection == COPY_FROM_USER) {
				COMPAT_VAL_HW_LOCK_T __user *from32 =
				    (COMPAT_VAL_HW_LOCK_T *) data32;
				VAL_HW_LOCK_T __user *to = (VAL_HW_LOCK_T *) data;

				err = get_user(p, &(from32->pvHandle));
				err |= put_user(compat_ptr(p), &(to->pvHandle));
				err |= get_user(u, &(from32->u4HandleSize));
				err |= put_user(u, &(to->u4HandleSize));
				err |= get_user(p, &(from32->pvLock));
				err |= put_user(compat_ptr(p), &(to->pvLock));
				err |= get_user(u, &(from32->u4TimeoutMs));
				err |= put_user(u, &(to->u4TimeoutMs));
				err |= get_user(p, &(from32->pvReserved));
				err |= put_user(compat_ptr(p), &(to->pvReserved));
				err |= get_user(u, &(from32->u4ReservedSize));
				err |= put_user(u, &(to->u4ReservedSize));
				err |= get_user(u, &(from32->eDriverType));
				err |= put_user(u, &(to->eDriverType));
				err |= get_user(c, &(from32->bSecureInst));
				err |= put_user(c, &(to->bSecureInst));
			} else {
				COMPAT_VAL_HW_LOCK_T __user *to32 = (COMPAT_VAL_HW_LOCK_T *) data32;
				VAL_HW_LOCK_T __user *from = (VAL_HW_LOCK_T *) data;

				err = get_user(ptr, &(from->pvHandle));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvHandle));
				err |= get_user(u, &(from->u4HandleSize));
				err |= put_user(u, &(to32->u4HandleSize));
				err |= get_user(ptr, &(from->pvLock));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvLock));
				err |= get_user(u, &(from->u4TimeoutMs));
				err |= put_user(u, &(to32->u4TimeoutMs));
				err |= get_user(ptr, &(from->pvReserved));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvReserved));
				err |= get_user(u, &(from->u4ReservedSize));
				err |= put_user(u, &(to32->u4ReservedSize));
				err |= get_user(u, &(from->eDriverType));
				err |= put_user(u, &(to32->eDriverType));
				err |= get_user(c, &(from->bSecureInst));
				err |= put_user(c, &(to32->bSecureInst));
			}
		}
		break;
	case VAL_POWER_TYPE:
		{
			if (eDirection == COPY_FROM_USER) {
				COMPAT_VAL_POWER_T __user *from32 = (COMPAT_VAL_POWER_T *) data32;
				VAL_POWER_T __user *to = (VAL_POWER_T *) data;

				err = get_user(p, &(from32->pvHandle));
				err |= put_user(compat_ptr(p), &(to->pvHandle));
				err |= get_user(u, &(from32->u4HandleSize));
				err |= put_user(u, &(to->u4HandleSize));
				err |= get_user(u, &(from32->eDriverType));
				err |= put_user(u, &(to->eDriverType));
				err |= get_user(c, &(from32->fgEnable));
				err |= put_user(c, &(to->fgEnable));
				err |= get_user(p, &(from32->pvReserved));
				err |= put_user(compat_ptr(p), &(to->pvReserved));
				err |= get_user(u, &(from32->u4ReservedSize));
				err |= put_user(u, &(to->u4ReservedSize));
			} else {
				COMPAT_VAL_POWER_T __user *to32 = (COMPAT_VAL_POWER_T *) data32;
				VAL_POWER_T __user *from = (VAL_POWER_T *) data;

				err = get_user(ptr, &(from->pvHandle));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvHandle));
				err |= get_user(u, &(from->u4HandleSize));
				err |= put_user(u, &(to32->u4HandleSize));
				err |= get_user(u, &(from->eDriverType));
				err |= put_user(u, &(to32->eDriverType));
				err |= get_user(c, &(from->fgEnable));
				err |= put_user(c, &(to32->fgEnable));
				err |= get_user(ptr, &(from->pvReserved));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvReserved));
				err |= get_user(u, &(from->u4ReservedSize));
				err |= put_user(u, &(to32->u4ReservedSize));
			}
		}
		break;
	case VAL_ISR_TYPE:
		{
			int i = 0;

			if (eDirection == COPY_FROM_USER) {
				COMPAT_VAL_ISR_T __user *from32 = (COMPAT_VAL_ISR_T *) data32;
				VAL_ISR_T __user *to = (VAL_ISR_T *) data;

				err = get_user(p, &(from32->pvHandle));
				err |= put_user(compat_ptr(p), &(to->pvHandle));
				err |= get_user(u, &(from32->u4HandleSize));
				err |= put_user(u, &(to->u4HandleSize));
				err |= get_user(u, &(from32->eDriverType));
				err |= put_user(u, &(to->eDriverType));
				err |= get_user(p, &(from32->pvIsrFunction));
				err |= put_user(compat_ptr(p), &(to->pvIsrFunction));
				err |= get_user(p, &(from32->pvReserved));
				err |= put_user(compat_ptr(p), &(to->pvReserved));
				err |= get_user(u, &(from32->u4ReservedSize));
				err |= put_user(u, &(to->u4ReservedSize));
				err |= get_user(u, &(from32->u4TimeoutMs));
				err |= put_user(u, &(to->u4TimeoutMs));
				err |= get_user(u, &(from32->u4IrqStatusNum));
				err |= put_user(u, &(to->u4IrqStatusNum));
				for (; i < IRQ_STATUS_MAX_NUM; i++) {
					err |= get_user(u, &(from32->u4IrqStatus[i]));
					err |= put_user(u, &(to->u4IrqStatus[i]));
				}
			} else {
				COMPAT_VAL_ISR_T __user *to32 = (COMPAT_VAL_ISR_T *) data32;
				VAL_ISR_T __user *from = (VAL_ISR_T *) data;

				err = get_user(ptr, &(from->pvHandle));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvHandle));
				err |= get_user(u, &(from->u4HandleSize));
				err |= put_user(u, &(to32->u4HandleSize));
				err |= get_user(u, &(from->eDriverType));
				err |= put_user(u, &(to32->eDriverType));
				err |= get_user(ptr, &(from->pvIsrFunction));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvIsrFunction));
				err |= get_user(ptr, &(from->pvReserved));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvReserved));
				err |= get_user(u, &(from->u4ReservedSize));
				err |= put_user(u, &(to32->u4ReservedSize));
				err |= get_user(u, &(from->u4TimeoutMs));
				err |= put_user(u, &(to32->u4TimeoutMs));
				err |= get_user(u, &(from->u4IrqStatusNum));
				err |= put_user(u, &(to32->u4IrqStatusNum));
				for (; i < IRQ_STATUS_MAX_NUM; i++) {
					err |= get_user(u, &(from->u4IrqStatus[i]));
					err |= put_user(u, &(to32->u4IrqStatus[i]));
				}
			}
		}
		break;
	case VAL_MEMORY_TYPE:
		{
			if (eDirection == COPY_FROM_USER) {
				COMPAT_VAL_MEMORY_T __user *from32 = (COMPAT_VAL_MEMORY_T *) data32;
				VAL_MEMORY_T __user *to = (VAL_MEMORY_T *) data;

				err = get_user(u, &(from32->eMemType));
				err |= put_user(u, &(to->eMemType));
				err |= get_user(l, &(from32->u4MemSize));
				err |= put_user(l, &(to->u4MemSize));
				err |= get_user(p, &(from32->pvMemVa));
				err |= put_user(compat_ptr(p), &(to->pvMemVa));
				err |= get_user(p, &(from32->pvMemPa));
				err |= put_user(compat_ptr(p), &(to->pvMemPa));
				err |= get_user(u, &(from32->eAlignment));
				err |= put_user(u, &(to->eAlignment));
				err |= get_user(p, &(from32->pvAlignMemVa));
				err |= put_user(compat_ptr(p), &(to->pvAlignMemVa));
				err |= get_user(p, &(from32->pvAlignMemPa));
				err |= put_user(compat_ptr(p), &(to->pvAlignMemPa));
				err |= get_user(u, &(from32->eMemCodec));
				err |= put_user(u, &(to->eMemCodec));
				err |= get_user(u, &(from32->i4IonShareFd));
				err |= put_user(u, &(to->i4IonShareFd));
				err |= get_user(p, &(from32->pIonBufhandle));
				err |= put_user(compat_ptr(p), &(to->pIonBufhandle));
				err |= get_user(p, &(from32->pvReserved));
				err |= put_user(compat_ptr(p), &(to->pvReserved));
				err |= get_user(l, &(from32->u4ReservedSize));
				err |= put_user(l, &(to->u4ReservedSize));
			} else {
				COMPAT_VAL_MEMORY_T __user *to32 = (COMPAT_VAL_MEMORY_T *) data32;
				VAL_MEMORY_T __user *from = (VAL_MEMORY_T *) data;

				err = get_user(u, &(from->eMemType));
				err |= put_user(u, &(to32->eMemType));
				err |= get_user(l, &(from->u4MemSize));
				err |= put_user(l, &(to32->u4MemSize));
				err |= get_user(ptr, &(from->pvMemVa));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvMemVa));
				err |= get_user(ptr, &(from->pvMemPa));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvMemPa));
				err |= get_user(u, &(from->eAlignment));
				err |= put_user(u, &(to32->eAlignment));
				err |= get_user(ptr, &(from->pvAlignMemVa));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvAlignMemVa));
				err |= get_user(ptr, &(from->pvAlignMemPa));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvAlignMemPa));
				err |= get_user(u, &(from->eMemCodec));
				err |= put_user(u, &(to32->eMemCodec));
				err |= get_user(u, &(from->i4IonShareFd));
				err |= put_user(u, &(to32->i4IonShareFd));
				err |= get_user(ptr, &(from->pIonBufhandle));
				err |= put_user(ptr_to_compat(ptr), &(to32->pIonBufhandle));
				err |= get_user(ptr, &(from->pvReserved));
				err |= put_user(ptr_to_compat(ptr), &(to32->pvReserved));
				err |= get_user(l, &(from->u4ReservedSize));
				err |= put_user(l, &(to32->u4ReservedSize));
			}
		}
		break;
	default:
		break;
	}

	return err;
}


static long vcodec_unlocked_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_unlocked_compat_ioctl: 0x%x\n", cmd);
	switch (cmd) {
	case VCODEC_ALLOC_NON_CACHE_BUFFER:
	case VCODEC_FREE_NON_CACHE_BUFFER:
		{
			COMPAT_VAL_MEMORY_T __user *data32;
			VAL_MEMORY_T __user *data;
			int err;

			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(VAL_MEMORY_T));
			if (data == NULL)
				return -EFAULT;

			err =
			    compat_copy_struct(VAL_MEMORY_TYPE, COPY_FROM_USER, (void *)data32,
					       (void *)data);
			if (err)
				return err;

			ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)data);

			err =
			    compat_copy_struct(VAL_MEMORY_TYPE, COPY_TO_USER, (void *)data32,
					       (void *)data);

			if (err)
				return err;
			return ret;
		}
		break;
	case VCODEC_LOCKHW:
	case VCODEC_LAT_LOCKHW:
	case VCODEC_UNLOCKHW:
	case VCODEC_LAT_UNLOCKHW:
		{
			COMPAT_VAL_HW_LOCK_T __user *data32;
			VAL_HW_LOCK_T __user *data;
			int err;

			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(VAL_HW_LOCK_T));
			if (data == NULL)
				return -EFAULT;

			err =
			    compat_copy_struct(VAL_HW_LOCK_TYPE, COPY_FROM_USER, (void *)data32,
					       (void *)data);
			if (err)
				return err;

			ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)data);

			err =
			    compat_copy_struct(VAL_HW_LOCK_TYPE, COPY_TO_USER, (void *)data32,
					       (void *)data);

			if (err)
				return err;
			return ret;
		}
		break;

	case VCODEC_INC_PWR_USER:
	case VCODEC_DEC_PWR_USER:
		{
			COMPAT_VAL_POWER_T __user *data32;
			VAL_POWER_T __user *data;
			int err;

			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(VAL_POWER_T));
			if (data == NULL)
				return -EFAULT;

			err =
			    compat_copy_struct(VAL_POWER_TYPE, COPY_FROM_USER, (void *)data32,
					       (void *)data);

			if (err)
				return err;

			ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)data);

			err =
			    compat_copy_struct(VAL_POWER_TYPE, COPY_TO_USER, (void *)data32,
					       (void *)data);

			if (err)
				return err;
			return ret;
		}
		break;

	case VCODEC_WAITISR:
	case VCODEC_WAIT_LAT_ISR:
		{
			COMPAT_VAL_ISR_T __user *data32;
			VAL_ISR_T __user *data;
			int err;

			data32 = compat_ptr(arg);
			data = compat_alloc_user_space(sizeof(VAL_ISR_T));
			if (data == NULL)
				return -EFAULT;

			err =
			    compat_copy_struct(VAL_ISR_TYPE, COPY_FROM_USER, (void *)data32,
					       (void *)data);
			if (err)
				return err;

			/*ret = file->f_op->unlocked_ioctl(file, VCODEC_WAITISR, (unsigned long)data);*/
			ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)data);

			err =
			    compat_copy_struct(VAL_ISR_TYPE, COPY_TO_USER, (void *)data32,
					       (void *)data);

			if (err)
				return err;
			return ret;
		}
		break;

	default:
		{
			return vcodec_unlocked_ioctl(file, cmd, arg);
		}
		break;
	}
	return 0;
}
#else
#define vcodec_unlocked_compat_ioctl NULL
#endif
static int vcodec_open(struct inode *inode, struct file *file)
{
	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_open\n");

	mutex_lock(&DriverOpenCountLock);
	MT8695Driver_Open_Count++;

	pm_runtime_get_sync(&my_pdev->dev);
	MODULE_MFV_LOGE("vcodec_open pid = %d, MT8695Driver_Open_Count %d\n", current->pid,
		 MT8695Driver_Open_Count);
	mutex_unlock(&DriverOpenCountLock);


	/* TODO: Check upper limit of concurrent users? */

	return 0;
}


static int vcodec_flush(struct file *file, fl_owner_t id)
{
	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_flush, curr_tid =%d\n", current->pid);
	MODULE_MFV_LOGE("vcodec_flush pid = %d, MT8695Driver_Open_Count %d\n", current->pid, MT8695Driver_Open_Count);

	pm_runtime_put_sync(&my_pdev->dev);
	return 0;
}

static int vcodec_release(struct inode *inode, struct file *file)
{
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlagsLockHW;
	VAL_ULONG_T ulFlagsISR;
#endif

	/* dump_stack(); */
	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_release, curr_tid =%d\n", current->pid);
	mutex_lock(&DriverOpenCountLock);
	MODULE_MFV_LOGE("vcodec_flush pid = %d, MT8695Driver_Open_Count %d\n", current->pid,
		MT8695Driver_Open_Count);
	MT8695Driver_Open_Count--;

	if (MT8695Driver_Open_Count == 0) {

		mutex_lock(&VdecHWLock);
		gu4VdecLockThreadId = 0;
		grVcodecDecHWLock.pvHandle = 0;
		grVcodecDecHWLock.eDriverType = VAL_DRIVER_TYPE_NONE;
		grVcodecDecHWLock.rLockedTime.u4Sec = 0;
		grVcodecDecHWLock.rLockedTime.u4uSec = 0;
		mutex_unlock(&VdecHWLock);

		mutex_lock(&VdecHWLatLock);
		grVcodecDecHWLaxLock.pvHandle = 0;
		grVcodecDecHWLaxLock.eDriverType = VAL_DRIVER_TYPE_NONE;
		grVcodecDecHWLaxLock.rLockedTime.u4Sec = 0;
		grVcodecDecHWLaxLock.rLockedTime.u4uSec = 0;
		mutex_unlock(&VdecHWLatLock);

		mutex_lock(&DecEMILock);
		gu4DecEMICounter = 0;
		mutex_unlock(&DecEMILock);

		mutex_lock(&PWRLock);
		gu4PWRCounter = 0;
		mutex_unlock(&PWRLock);

#ifdef VENC_USE_L2C
		mutex_lock(&L2CLock);
		if (gu4L2CCounter != 0) {
			MODULE_MFV_LOGE("vcodec_flush pid = %d, L2 user = %d, force restore L2 settings\n",
				 current->pid, gu4L2CCounter);
			if (config_L2(1))
				MODULE_MFV_LOGE("restore L2 settings failed\n");
		}
		gu4L2CCounter = 0;
		mutex_unlock(&L2CLock);
#endif
#if VDEC_CLOSE_DEBUG_CODE
		spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
		gu4LockDecHWCount = 0;
		spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);

		spin_lock_irqsave(&DecISRCountLock, ulFlagsISR);
		gu4DecISRCount = 0;
		spin_unlock_irqrestore(&DecISRCountLock, ulFlagsISR);
#endif

	}
	mutex_unlock(&DriverOpenCountLock);

	return 0;
}

void vcodec_vma_open(struct vm_area_struct *vma)
{
	MODULE_MFV_LOGD("vcodec VMA open, virt %lx, phys %lx\n", vma->vm_start,
		 vma->vm_pgoff << PAGE_SHIFT);
}

void vcodec_vma_close(struct vm_area_struct *vma)
{
	MODULE_MFV_LOGD("vcodec VMA close, virt %lx, phys %lx\n", vma->vm_start,
		 vma->vm_pgoff << PAGE_SHIFT);
}

static struct vm_operations_struct vcodec_remap_vm_ops = {
	.open = vcodec_vma_open,
	.close = vcodec_vma_close,
};

static int vcodec_mmap(struct file *file, struct vm_area_struct *vma)
{
	VAL_UINT32_T u4I = 0;
	VAL_ULONG_T length;
	VAL_ULONG_T pfn;

	length = vma->vm_end - vma->vm_start;
	pfn = vma->vm_pgoff << PAGE_SHIFT;

	if (((length > VDEC_REGION) || (pfn < VDEC_BASE_PHY) || (pfn > VDEC_BASE_PHY + VDEC_REGION))) {
		VAL_ULONG_T ulAddr, ulSize;

		for (u4I = 0; u4I < VCODEC_MULTIPLE_INSTANCE_NUM_x_10; u4I++) {
			if ((grNonCacheMemoryList[u4I].ulKVA != -1L)
			    && (grNonCacheMemoryList[u4I].ulKPA != -1L)) {
				ulAddr = grNonCacheMemoryList[u4I].ulKPA;
				ulSize =
				    (grNonCacheMemoryList[u4I].ulSize + 0x1000 - 1) & ~(0x1000 - 1);
				if ((length == ulSize) && (pfn == ulAddr)) {
					MODULE_MFV_LOGD(" cache idx %d\n", u4I);
					break;
				}
			}
		}

		if (u4I == VCODEC_MULTIPLE_INSTANCE_NUM_x_10) {
			MODULE_MFV_LOGE("[ERROR] mmap region error: Length(0x%lx), pfn(0x%lx)\n",
				 (VAL_ULONG_T) length, pfn);
			return -EAGAIN;
		}
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	MODULE_MFV_LOGE("[mmap] vma->start 0x%lx, vma->end 0x%lx, vma->pgoff 0x%lx\n",
		 (VAL_ULONG_T) vma->vm_start, (VAL_ULONG_T) vma->vm_end,
		 (VAL_ULONG_T) vma->vm_pgoff);
	if (remap_pfn_range
	    (vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	vma->vm_ops = &vcodec_remap_vm_ops;
	vcodec_vma_open(vma);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void vcodec_early_suspend(struct early_suspend *h)
{
	mutex_lock(&PWRLock);
	MODULE_MFV_LOGE("vcodec_early_suspend, tid = %d, PWR_USER = %d\n", current->pid, gu4PWRCounter);
	mutex_unlock(&PWRLock);

	MODULE_MFV_LOGD("vcodec_early_suspend - tid = %d\n", current->pid);
}

static void vcodec_late_resume(struct early_suspend *h)
{
	mutex_lock(&PWRLock);
	MODULE_MFV_LOGE("vcodec_late_resume, tid = %d, PWR_USER = %d\n", current->pid, gu4PWRCounter);
	mutex_unlock(&PWRLock);

	MODULE_MFV_LOGD("vcodec_late_resume - tid = %d\n", current->pid);
}

static struct early_suspend vcodec_early_suspend_handler = {
	.level = (EARLY_SUSPEND_LEVEL_DISABLE_FB - 1),
	.suspend = vcodec_early_suspend,
	.resume = vcodec_late_resume,
};
#endif

static const struct file_operations vcodec_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = vcodec_unlocked_ioctl,
	.open = vcodec_open,
	.flush = vcodec_flush,
	.release = vcodec_release,
	.mmap = vcodec_mmap,
#if IS_ENABLED(CONFIG_COMPAT)
	.compat_ioctl = vcodec_unlocked_compat_ioctl,
#endif
};

static int vcodec_probe(struct platform_device *pdev)
{
	int ret, i;
	struct device_node *np;

	MODULE_MFV_LOGD("+vcodec_probe\n");

	for (i = 0; i < VDEC_LARB_COUNT ; i++) {
		np = of_parse_phandle(pdev->dev.of_node, "larbs", i);
		if (!np) {
			MODULE_MFV_LOGE("[VCODEC_DEBUG][ERROR] parse larb device failed\n");
			return 0;
		}
		pimudev[i] = of_find_device_by_node(np);
		of_node_put(np);
	}

	mutex_lock(&DecEMILock);
	gu4DecEMICounter = 0;
	mutex_unlock(&DecEMILock);

	mutex_lock(&PWRLock);
	gu4PWRCounter = 0;
	mutex_unlock(&PWRLock);

	mutex_lock(&L2CLock);
	gu4L2CCounter = 0;
	mutex_unlock(&L2CLock);

	/* retrieve clock node from dtsi */
#ifdef CONFIG_OF
	clk_vdec_sel = devm_clk_get(&pdev->dev, "vdecsel");
	WARN_ON(IS_ERR(clk_vdec_sel));
	clk_vdec_slow_sel = devm_clk_get(&pdev->dev, "vdecslowsel");
	WARN_ON(IS_ERR(clk_vdec_slow_sel));
	clk_vdec_lae_sel = devm_clk_get(&pdev->dev, "laesel");
	WARN_ON(IS_ERR(clk_vdec_lae_sel));
	clk_vdec_pll = devm_clk_get(&pdev->dev, "vdecpll");
	WARN_ON(IS_ERR(clk_vdec_pll));
	clk_osd_pll_d2 = devm_clk_get(&pdev->dev, "osdpll_d2");
	WARN_ON(IS_ERR(clk_osd_pll_d2));

	clk_set_parent(clk_vdec_sel, clk_vdec_pll);
	clk_prepare_enable(clk_osd_pll_d2);
	clk_set_parent(clk_vdec_slow_sel, clk_osd_pll_d2);
	clk_disable_unprepare(clk_osd_pll_d2);
	clk_set_parent(clk_vdec_lae_sel, clk_vdec_pll);
#endif

	ret = register_chrdev_region(vcodec_devno, 1, VCODEC_DEVNAME);
	if (ret)
		MODULE_MFV_LOGE("[VCODEC_DEBUG][ERROR] Can't Get Major number for VCodec Device\n");

	vcodec_cdev = cdev_alloc();
	vcodec_cdev->owner = THIS_MODULE;
	vcodec_cdev->ops = &vcodec_fops;

	ret = cdev_add(vcodec_cdev, vcodec_devno, 1);
	if (ret)
		MODULE_MFV_LOGE("[VCODEC_DEBUG][ERROR] Can't add Vcodec Device\n");

	vcodec_class = class_create(THIS_MODULE, VCODEC_DEVNAME);
	if (IS_ERR(vcodec_class)) {
		ret = PTR_ERR(vcodec_class);
		MODULE_MFV_LOGE("Unable to create class, err = %d", ret);
		return ret;
	}

	vcodec_device = device_create(vcodec_class, NULL, vcodec_devno, NULL, VCODEC_DEVNAME);

#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
#else
	if (request_irq
	    (VDEC_IRQ_ID, (irq_handler_t) video_intr_dlr, IRQF_TRIGGER_LOW, VCODEC_DEVNAME,
	     NULL) < 0) {
		MODULE_MFV_LOGD("[VCODEC_DEBUG][ERROR] error to request dec irq\n");
	} else {
		MODULE_MFV_LOGD("[VCODEC_DEBUG] success to request dec irq: %d\n", VDEC_IRQ_ID);
	}

	if (request_irq
		(VDEC_LAT_IRQ_ID, (irq_handler_t) video_lat_intr_dlr, IRQF_TRIGGER_LOW, VCODEC_DEVNAME,
		 NULL) < 0) {
		MODULE_MFV_LOGD("[VCODEC_DEBUG][ERROR] error to request lat irq\n");
	} else {
		MODULE_MFV_LOGD("[VCODEC_DEBUG] success to request lat irq: %d\n", VDEC_LAT_IRQ_ID);
	}

#endif

#ifdef CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT
#else
	/* disable_irq(MT_VDEC_IRQ_ID); */
	disable_irq(VDEC_IRQ_ID);
	disable_irq(VDEC_LAT_IRQ_ID);
	/* disable_irq(MT_VENC_IRQ_ID); */

#endif
	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_probe Done\n");

	pm_runtime_enable(&pdev->dev);
	my_pdev = pdev;
	return 0;
}

static int vcodec_remove(struct platform_device *pdev)
{
	MODULE_MFV_LOGD("vcodec_remove\n");
	pm_runtime_disable(&pdev->dev);
	return 0;
}

#ifdef CONFIG_OF
/* VDEC main device */
static const struct of_device_id vcodec_of_ids[] = {
	{.compatible = "mediatek,mt8695-vcodec-dec",},
	{}
};

static struct platform_driver VCodecDriver = {
	.probe = vcodec_probe,
	.remove = vcodec_remove,
	.driver = {
		   .name = VCODEC_DEVNAME,
		   .owner = THIS_MODULE,
		   .of_match_table = vcodec_of_ids,
		   }
};
#endif

#ifdef CONFIG_MTK_HIBERNATION
static int vcodec_pm_restore_noirq(struct device *device)
{
	/* vdec : IRQF_TRIGGER_LOW */
	mt_irq_set_sens(VDEC_IRQ_ID, MT_LEVEL_SENSITIVE);
	mt_irq_set_polarity(VDEC_IRQ_ID, MT_POLARITY_LOW);

	return 0;
}
#endif

static int __init vcodec_driver_init(void)
{
	VAL_RESULT_T eValHWLockRet;
	VAL_ULONG_T ulFlags;
#if VDEC_CLOSE_DEBUG_CODE
	VAL_ULONG_T ulFlagsISR;
	VAL_ULONG_T ulFlagsLockHW;
#endif
	MODULE_MFV_LOGD("+vcodec_init !!\n");

	mutex_lock(&DriverOpenCountLock);
	MT8695Driver_Open_Count = 0;
	mutex_unlock(&DriverOpenCountLock);

	{
		struct device_node *node = NULL;

		node = of_find_compatible_node(NULL, NULL, "mediatek,mt8695-vcodec-dec");
		KVA_VDEC_BASE = (VAL_ULONG_T) of_iomap(node, 0);/*0x16000000*/
		VDEC_IRQ_ID = irq_of_parse_and_map(node, 0);
		VDEC_LAT_IRQ_ID = irq_of_parse_and_map(node, 1);
		KVA_VDEC_MISC_BASE = KVA_VDEC_BASE + 0x25000;
		KVA_VDEC_LAT_MISC_BASE = KVA_VDEC_BASE + 0x10000; /*lat0 misc*/
		KVA_VDEC_VLD_BASE = KVA_VDEC_BASE + 0x20000;
		VDEC_SOC_GCON_BASE = KVA_VDEC_BASE + 0x0f000;
		VDEC_CORE_GCON_BASE = KVA_VDEC_BASE + 0x2f000;
	}

#if VDEC_CLOSE_DEBUG_CODE
	spin_lock_irqsave(&LockDecHWCountLock, ulFlagsLockHW);
	gu4LockDecHWCount = 0;
	spin_unlock_irqrestore(&LockDecHWCountLock, ulFlagsLockHW);

	spin_lock_irqsave(&DecISRCountLock, ulFlagsISR);
	gu4DecISRCount = 0;
	spin_unlock_irqrestore(&DecISRCountLock, ulFlagsISR);
#endif

	mutex_lock(&VdecLatPWRLock);
	gu4VdecLatPWRCounter = 0;
	mutex_unlock(&VdecLatPWRLock);

	mutex_lock(&VdecCorePWRLock);
	gu4VdecPWRCounter = 0;
	mutex_unlock(&VdecCorePWRLock);

	mutex_lock(&IsOpenedLock);
	if (bIsOpened == VAL_FALSE) {
		bIsOpened = VAL_TRUE;
#ifdef CONFIG_OF
	platform_driver_register(&VCodecDriver);
#else
	vcodec_probe(NULL);
#endif
	}
	mutex_unlock(&IsOpenedLock);

	mutex_lock(&VdecHWLock);
	gu4VdecLockThreadId = 0;
	grVcodecDecHWLock.pvHandle = 0;
	grVcodecDecHWLock.eDriverType = VAL_DRIVER_TYPE_NONE;
	grVcodecDecHWLock.rLockedTime.u4Sec = 0;
	grVcodecDecHWLock.rLockedTime.u4uSec = 0;
	mutex_unlock(&VdecHWLock);

	mutex_lock(&VdecHWLatLock);
	grVcodecDecHWLaxLock.pvHandle = 0;
	grVcodecDecHWLaxLock.eDriverType = VAL_DRIVER_TYPE_NONE;
	grVcodecDecHWLaxLock.rLockedTime.u4Sec = 0;
	grVcodecDecHWLaxLock.rLockedTime.u4uSec = 0;
	mutex_unlock(&VdecHWLatLock);

	mutex_lock(&DecCoreHWLockEventTimeoutLock);
	DecHWLockEvent.pvHandle = "DECHWLOCK_EVENT";
	DecHWLockEvent.u4HandleSize = sizeof("DECHWLOCK_EVENT") + 1;
	DecHWLockEvent.u4TimeoutMs = 1;
	DecHWLockMutex.pvHandle = "DECHWLOCK_MUTEX";
	DecHWLockMutex.u4HandleSize = sizeof("DECHWLOCK_MUTEX") + 1;
	DecHWLockMutex.u4TimeoutMs = 1000;
	mutex_unlock(&DecCoreHWLockEventTimeoutLock);

	mutex_lock(&DecLatHWLockEventTimeoutLock);
	DecHWLatLockEvent.pvHandle = "DECHWLATLOCK_EVENT";
	DecHWLatLockEvent.u4HandleSize = sizeof("DECHWLATLOCK_EVENT") + 1;
	DecHWLatLockEvent.u4TimeoutMs = 1;
	DecLatHWLockMutex.pvHandle = "DECLATHWLOCK_MUTEX";
	DecLatHWLockMutex.u4HandleSize = sizeof("DECLATHWLOCK_MUTEX") + 1;
	DecLatHWLockMutex.u4TimeoutMs = 1000;
	mutex_unlock(&DecLatHWLockEventTimeoutLock);

	eValHWLockRet = eVideoCreateEvent(&DecHWLockEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create dec hwlock event error\n");

	eValHWLockRet = eVideoCreateMutex(&DecHWLockMutex, sizeof(VAL_MUTEX_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create dec hwlock mutex error\n");

	eValHWLockRet = eVideoCreateEvent(&DecHWLatLockEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create lat hwlock event error\n");

	eValHWLockRet = eVideoCreateMutex(&DecLatHWLockMutex, sizeof(VAL_MUTEX_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create lat hwlock mutex error\n");

	/* MT8697_IsrEvent part */
	spin_lock_irqsave(&DecIsrLock, ulFlags);
	DecIsrEvent.pvHandle = "DECISR_EVENT";
	DecIsrEvent.u4HandleSize = sizeof("DECISR_EVENT") + 1;
	DecIsrEvent.u4TimeoutMs = 1;
	spin_unlock_irqrestore(&DecIsrLock, ulFlags);

	spin_lock_irqsave(&DecIsrLatLock, ulFlags);
	DecLatIsrEvent.pvHandle = "DECLATISR_EVENT";
	DecLatIsrEvent.u4HandleSize = sizeof("DECLATISR_EVENT") + 1;
	DecLatIsrEvent.u4TimeoutMs = 1;
	spin_unlock_irqrestore(&DecIsrLatLock, ulFlags);

	eValHWLockRet = eVideoCreateEvent(&DecIsrEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create dec isr event error\n");

	eValHWLockRet = eVideoCreateEvent(&DecLatIsrEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] create dec lat isr event error\n");

	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_driver_init Done\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&vcodec_early_suspend_handler);
#endif

#ifdef CONFIG_MTK_HIBERNATION
	register_swsusp_restore_noirq_func(ID_M_VCODEC, vcodec_pm_restore_noirq, NULL);
#endif

	return 0;
}

static void __exit vcodec_driver_exit(void)
{
	VAL_RESULT_T eValHWLockRet;

	MODULE_MFV_LOGD("[VCODEC_DEBUG] vcodec_driver_exit\n");

	mutex_lock(&IsOpenedLock);
	if (bIsOpened == VAL_TRUE) {
		MODULE_MFV_LOGD("+vcodec_driver_exit remove device !!\n");
#ifdef CONFIG_OF
		platform_driver_unregister(&VCodecDriver);

#else
		bIsOpened = VAL_FALSE;
#endif
		MODULE_MFV_LOGD("+vcodec_driver_exit remove done !!\n");
	}
	mutex_unlock(&IsOpenedLock);

	cdev_del(vcodec_cdev);
	unregister_chrdev_region(vcodec_devno, 1);

	/* [TODO] free IRQ here */
	/* free_irq(MT_VDEC_IRQ_ID, NULL); */
	free_irq(VDEC_IRQ_ID, NULL);


	/* MT8695_HWLockEvent part */
	eValHWLockRet = eVideoCloseEvent(&DecHWLockEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close dec hwlock event error\n");

	eValHWLockRet = eVideoCloseMutex(&DecHWLockMutex, sizeof(VAL_MUTEX_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close dec hwlock mutex error\n");

	eValHWLockRet = eVideoCloseEvent(&DecHWLatLockEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close lat hwlock event error\n");

	eValHWLockRet = eVideoCloseMutex(&DecLatHWLockMutex, sizeof(VAL_MUTEX_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close lat hwlock mutex error\n");

	/* MT8695_IsrEvent part */
	eValHWLockRet = eVideoCloseEvent(&DecIsrEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close dec isr event error\n");
	eValHWLockRet = eVideoCloseEvent(&DecLatIsrEvent, sizeof(VAL_EVENT_T));
	if (eValHWLockRet != VAL_RESULT_NO_ERROR)
		MODULE_MFV_LOGE("[MFV][ERROR] close lat isr event error\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&vcodec_early_suspend_handler);
#endif

#ifdef CONFIG_MTK_HIBERNATION
	unregister_swsusp_restore_noirq_func(ID_M_VCODEC);
#endif
}

module_init(vcodec_driver_init);
module_exit(vcodec_driver_exit);
MODULE_AUTHOR("Legis, Lu <legis.lu@mediatek.com>");
MODULE_DESCRIPTION("8695 Vcodec Driver");
MODULE_LICENSE("GPL");
