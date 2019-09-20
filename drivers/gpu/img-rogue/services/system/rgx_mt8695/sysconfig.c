/*************************************************************************/
/*!
*@File
*@Title          System Configuration
*@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
*@Description    System Configuration functions
*@License        Dual MIT/GPLv2
*
*The contents of this file are subject to the MIT license as set out below.
*
*Permission is hereby granted, free of charge, to any person obtaining a copy
*of this software and associated documentation files (the "Software"), to deal
*in the Software without restriction, including without limitation the rights
*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*copies of the Software, and to permit persons to whom the Software is
*furnished to do so, subject to the following conditions:
*
*The above copyright notice and this permission notice shall be included in
*all copies or substantial portions of the Software.
*
*Alternatively, the contents of this file may be used under the terms of
*the GNU General Public License Version 2 ("GPL") in which case the provisions
*of GPL are applicable instead of those above.
*
*If you wish to allow use of your version of this file only under the terms of
*GPL, and not to allow others to use your version of this file under the terms
*of the MIT license, indicate your decision by deleting the provisions above
*and replace them with the notice and other provisions required by GPL as set
*out in the file called "GPL-COPYING" included in this distribution. If you do
*not delete the provisions above, a recipient may use your version of this file
*under the terms of either the MIT license or GPL.
*
*This License is also included in this distribution in the file called
*"MIT-COPYING".
*
*EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
*PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
*BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
*PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
*COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
*IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
*CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**************************************************************************/

#include "interrupt_support.h"
#include "pvrsrv_device.h"
#include "syscommon.h"
#include "sysconfig.h"
#include "physheap.h"
#if defined(SUPPORT_ION)
#include "ion_support.h"
#endif
#include "mtk_mfg.h"
#include "mtk_gpu_misc.h"

#if defined(CONFIG_OF)
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/platform_device.h>
struct platform_device *gpsPVRLDMDev;
#endif
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/syscalls.h>
#define RGX_CR_ISP_GRIDOFFSET	(0x0FA0U)


static RGX_TIMING_INFORMATION	gsRGXTimingInfo;
static RGX_DATA			gsRGXData;
static PVRSRV_DEVICE_CONFIG	gsDevices[1];

static IMG_UINT32 gauiBIFTilingHeapXStrides[RGXFWIF_NUM_BIF_TILING_CONFIGS] = {
	0, /* BIF tiling heap 1 x-stride */
	1, /* BIF tiling heap 2 x-stride */
	2, /* BIF tiling heap 3 x-stride */
	3  /* BIF tiling heap 4 x-stride */
};

static PHYS_HEAP_FUNCTIONS	gsPhysHeapFuncs;
static PHYS_HEAP_CONFIG		gsPhysHeapConfig;

#if defined(SUPPORT_PDVFS) || defined(PVR_DVFS)

static const IMG_OPP asOPPTable[] = {
	{ 800000, 375000000},
	{ 800000, 450000000},
	{ 800000, 525000000},
	{ 800000, 600000000},
	{ 800000, 650000000},
	{ 900000, 700000000},
	{ 900000, 750000000}
};

static const IMG_OPP asOPPTableLowVolt[] = {
	{ 800000, 375000000},
	{ 800000, 450000000},
	{ 800000, 525000000},
	{ 800000, 600000000},
	{ 800000, 650000000}
};

#endif


/* CPU to Device physcial address translation */
static
void UMAPhysHeapCpuPAddrToDevPAddr(IMG_HANDLE hPrivData,
				   IMG_UINT32 ui32NumOfAddr,
				   IMG_DEV_PHYADDR *psDevPAddr,
				   IMG_CPU_PHYADDR *psCpuPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psDevPAddr[0].uiAddr = psCpuPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1) {
		IMG_UINT32 ui32Idx;

		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
			psDevPAddr[ui32Idx].uiAddr = psCpuPAddr[ui32Idx].uiAddr;
	}
}

/* Device to CPU physcial address translation */
static
void UMAPhysHeapDevPAddrToCpuPAddr(IMG_HANDLE hPrivData,
				   IMG_UINT32 ui32NumOfAddr,
				   IMG_CPU_PHYADDR *psCpuPAddr,
				   IMG_DEV_PHYADDR *psDevPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psCpuPAddr[0].uiAddr = psDevPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1) {
		IMG_UINT32 ui32Idx;

		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
			psCpuPAddr[ui32Idx].uiAddr = psDevPAddr[ui32Idx].uiAddr;
	}
}

/* SysCreateConfigData */
static PHYS_HEAP_REGION gsHeapRegionsLocal[] = {
	/* sStartAddr, sCardBase, uiSize */
	{ { 0 }, { 0 }, 0, },
};

static PVRSRV_ERROR MTKDevPrePowerState(IMG_HANDLE hSysData, PVRSRV_DEV_POWER_STATE eNewPowerState,
				 PVRSRV_DEV_POWER_STATE eCurrentPowerState,
				 IMG_BOOL bForced)
{
	if ((eNewPowerState == PVRSRV_DEV_POWER_STATE_OFF) &&
	    (eCurrentPowerState == PVRSRV_DEV_POWER_STATE_ON)) {
		mtk_mfg_disable_gpu();
		mtk_gpu_misc_met_nfy_power_state(0);
	}
	return PVRSRV_OK;
}

static PVRSRV_ERROR MTKDevPostPowerState(IMG_HANDLE hSysData, PVRSRV_DEV_POWER_STATE eNewPowerState,
				  PVRSRV_DEV_POWER_STATE eCurrentPowerState,
				  IMG_BOOL bForced)
{
	if ((eCurrentPowerState == PVRSRV_DEV_POWER_STATE_OFF) &&
	    (eNewPowerState == PVRSRV_DEV_POWER_STATE_ON)) {
		mtk_mfg_enable_gpu();
		mtk_gpu_misc_met_nfy_power_state(1);
	}
	return PVRSRV_OK;
}


static void MTKSetFreq(IMG_UINT32 ui32Freq)
{
	mtk_mfg_set_freq(ui32Freq, true);
}

static void MTKSetVolt(IMG_UINT32 ui32Volt)
{
	mtk_mfg_set_volt(ui32Volt);
}

PVRSRV_ERROR SysDevInit(void *pvOSDevice, PVRSRV_DEVICE_CONFIG **ppsDevConfig)
{
	PVRSRV_ERROR err = PVRSRV_OK;

	if (!mtk_mfg_is_ready()) {
		pr_info("mfg not ready - defer");
		return PVRSRV_ERROR_PROBE_DEFER;
	}

	gsPhysHeapFuncs.pfnCpuPAddrToDevPAddr = UMAPhysHeapCpuPAddrToDevPAddr;
	gsPhysHeapFuncs.pfnDevPAddrToCpuPAddr = UMAPhysHeapDevPAddrToCpuPAddr;

	gsPhysHeapConfig.ui32PhysHeapID = 0;
	gsPhysHeapConfig.pszPDumpMemspaceName = "SYSMEM";
	gsPhysHeapConfig.eType = PHYS_HEAP_TYPE_UMA;
	gsPhysHeapConfig.psMemFuncs = &gsPhysHeapFuncs;
	gsPhysHeapConfig.hPrivData = (IMG_HANDLE)&gsDevices[0];

	gsPhysHeapConfig.pasRegions = &gsHeapRegionsLocal[0];

	gsDevices[0].pvOSDevice = pvOSDevice;
	gsDevices[0].pasPhysHeaps = &gsPhysHeapConfig;
	gsDevices[0].ui32PhysHeapCount = sizeof(gsPhysHeapConfig) / sizeof(PHYS_HEAP_CONFIG);


	gsDevices[0].eBIFTilingMode = RGXFWIF_BIFTILINGMODE_256x16;
	gsDevices[0].pui32BIFTilingHeapConfigs = gauiBIFTilingHeapXStrides;
	gsDevices[0].ui32BIFTilingHeapCount = IMG_ARR_NUM_ELEMS(gauiBIFTilingHeapXStrides);

	/*
	 * Setup RGX specific timing data
	 */
	gsRGXTimingInfo.ui32CoreClockSpeed        = asOPPTable[0].ui32Freq;

#if MTK_PM_SUPPORT
	gsRGXTimingInfo.bEnableActivePM           = true;
	gsRGXTimingInfo.ui32ActivePMLatencyms     = SYS_RGX_ACTIVE_POWER_LATENCY_MS,
#else
	gsRGXTimingInfo.bEnableActivePM           = false;
#endif

	/*  define HW APM */
	gsRGXTimingInfo.bEnableRDPowIsland        = mtk_mfg_is_hwapm();

	/*
	 * Setup RGX specific data
	 */
	gsRGXData.psRGXTimingInfo = &gsRGXTimingInfo;

	/*
	 * Setup RGX device
	 */
	gsDevices[0].pszName	= "RGX";
	gsDevices[0].pszVersion = NULL;

	/* Device setup information */
#if defined(CONFIG_OF)
	/* MTK: using device tree */
	{
		struct resource *irq_res;
		struct resource *reg_res;

		gpsPVRLDMDev = to_platform_device((struct device *)pvOSDevice);
		irq_res = platform_get_resource(gpsPVRLDMDev, IORESOURCE_IRQ, 0);

		if (irq_res) {
			gsDevices[0].ui32IRQ				= irq_res->start;
		} else {
			PVR_DPF((PVR_DBG_ERROR, "irq_res = NULL!"));
err_out:
			return PVRSRV_ERROR_INIT_FAILURE;
		}

		reg_res = platform_get_resource(gpsPVRLDMDev, IORESOURCE_MEM, 0);

		if (reg_res) {
			gsDevices[0].sRegsCpuPBase.uiAddr	= reg_res->start;
			gsDevices[0].ui32RegsSize			= resource_size(reg_res);
		} else {
			PVR_DPF((PVR_DBG_ERROR, "reg_res = NULL!"));
			goto err_out;
		}
	}
#else
	gsDevices[0].sRegsCpuPBase.uiAddr   = SYS_MTK_RGX_REGS_SYS_PHYS_BASE;
	gsDevices[0].ui32RegsSize           = SYS_MTK_RGX_REGS_SIZE;
	gsDevices[0].ui32IRQ                = SYS_MTK_RGX_IRQ;
#endif

	/*  power management on  HW system */
#if MTK_PM_SUPPORT
	gsDevices[0].pfnPrePowerState       = MTKDevPrePowerState;
	gsDevices[0].pfnPostPowerState      = MTKDevPostPowerState;
#else
	gsDevices[0].pfnPrePowerState       = NULL;
	gsDevices[0].pfnPostPowerState      = NULL;
#endif

	/*  clock frequency  */
	gsDevices[0].pfnClockFreqGet        = NULL;
	gsDevices[0].hDevData               = &gsRGXData;
	gsDevices[0].eCacheSnoopingMode = PVRSRV_DEVICE_SNOOP_NONE;

#if defined(PVR_DVFS) || defined(SUPPORT_PDVFS)
	/*
	 * If target platform does not support volt changing, we use low volt
	 * table.
	 */
	if (mtk_mfg_can_set_volt()) {
		gsDevices[0].sDVFS.sDVFSDeviceCfg.pasOPPTable = asOPPTable;
		gsDevices[0].sDVFS.sDVFSDeviceCfg.ui32OPPTableSize = ARRAY_SIZE(asOPPTable);
	} else {
		gsDevices[0].sDVFS.sDVFSDeviceCfg.pasOPPTable = asOPPTableLowVolt;
		gsDevices[0].sDVFS.sDVFSDeviceCfg.ui32OPPTableSize = ARRAY_SIZE(asOPPTableLowVolt);
	}
	gsDevices[0].sDVFS.sDVFSDeviceCfg.pfnSetFrequency = MTKSetFreq;
	gsDevices[0].sDVFS.sDVFSDeviceCfg.pfnSetVoltage = MTKSetVolt;
	gsDevices[0].sDVFS.sDVFSDeviceCfg.bIdleReq = mtk_mfg_dvfs_idle(); /* Use false after hooping ready */
#endif
#if defined(PVR_DVFS)
	gsDevices[0].sDVFS.sDVFSDeviceCfg.ui32PollMs = 100;
	gsDevices[0].sDVFS.sDVFSGovernorCfg.ui32UpThreshold = 25;
	gsDevices[0].sDVFS.sDVFSGovernorCfg.ui32DownDifferential = 20;
#endif

	/* Setup other system specific stuff */
#if defined(SUPPORT_ION)
	IonInit(NULL);
#endif

	gsDevices[0].pvOSDevice = pvOSDevice;

	*ppsDevConfig = &gsDevices[0];

	mtk_gpu_misc_init(&gsDevices[0]);

	return err;
}

void SysDevDeInit(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
#if defined(SUPPORT_ION)
	IonDeinit();
#endif
	psDevConfig->pvOSDevice = NULL;
}

PVRSRV_ERROR SysInstallDeviceLISR(IMG_HANDLE hSysData,
				  IMG_UINT32 ui32IRQ,
				  const IMG_CHAR *pszName,
				  PFN_LISR pfnLISR,
				  void *pvData,
				  IMG_HANDLE *phLISRData)
{
	IMG_UINT32 ui32IRQFlags = SYS_IRQ_FLAG_TRIGGER_LOW; /*SYS_IRQ_FLAG_TRIGGER_DEFAULT;*/

	PVR_UNREFERENCED_PARAMETER(hSysData);

#if defined(PVRSRV_GPUVIRT_MULTIDRV_MODEL)
	ui32IRQFlags |= SYS_IRQ_FLAG_SHARED;
#endif

	return OSInstallSystemLISR(phLISRData, ui32IRQ, pszName, pfnLISR, pvData,
							   ui32IRQFlags);
}

PVRSRV_ERROR SysUninstallDeviceLISR(IMG_HANDLE hLISRData)
{
	return OSUninstallSystemLISR(hLISRData);
}


PVRSRV_ERROR SysDebugInfo(PVRSRV_DEVICE_CONFIG *psDevConfig, DUMPDEBUG_PRINTF_FUNC *pfnDumpDebugPrintf,
			  void *pvDumpDebugFile)
{
	PVR_UNREFERENCED_PARAMETER(psDevConfig);
	PVR_UNREFERENCED_PARAMETER(pfnDumpDebugPrintf);
	return PVRSRV_OK;
}




/******************************************************************************
* End of file (sysconfig.c)
******************************************************************************/
