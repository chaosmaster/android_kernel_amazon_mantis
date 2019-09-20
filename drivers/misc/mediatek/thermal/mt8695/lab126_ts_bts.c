/*
 * drivers/misc/mediatek/thermal/mt8695/lab126_ts_bts.c
 *
 * ADC NTC BTS driver
 *
 * Copyright (C) 2018 Amazon
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.

 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/of.h>
#include "inc/tmp_bts.h"
#include <linux/thermal_framework.h>
#include <linux/iio/consumer.h>
#include <linux/iio/iio.h> /* for dereferncing struct iio_dev */
#include "board_id.h"

#ifdef CONFIG_AMAZON_METRICS_LOG
#include <linux/metricslog.h>
#include <linux/vmalloc.h>
#define METRICSCOUNT 900
#ifndef THERMO_METRICS_STR_LEN
#define THERMO_METRICS_STR_LEN 128
#endif
static int metrics_cnt;
#endif

#ifdef CONFIG_IIO
#define USE_AUXADC_API 0
#else
#define USE_AUXADC_API 1
#endif

#define PRIOR_HVT_PULLUP_R 33000
/* use CONFIG_BOARD_ID first, if not, fall back to use idme if exist */
#ifdef CONFIG_BOARD_ID
extern unsigned int get_board_id(void);
#else
#ifdef CONFIG_IDME
extern unsigned int idme_get_board_rev(void);
#define Proto_MANTIS_BOARD_REV (0x00)
#define HVT_MANTIS_BOARD_REV (0x10)
#define EVT_MANTIS_BOARD_REV (0x20)
#define DVT_MANTIS_BOARD_REV (0x30)
#define PVT_MANTIS_BOARD_REV (0x40)
#endif /* CONFIG_IDME */
#endif /* CONFIG_BAORD_ID */


#if !USE_AUXADC_API
struct gadc_thermal_info {
	struct device *dev;
	struct iio_channel *channel;
};

static struct gadc_thermal_info *gti_ntc;
static int gadc_thermal_get_temp(void *data, int index, int *temp);
#endif


#ifndef INT32
typedef int INT32;
typedef unsigned int UINT32;
typedef short INT16;
#endif

typedef struct{
    INT32 BTS_Temp;
    INT32 TemperatureR;
}BTS_TEMPERATURE;

/* Should map to how many channels used in device tree */
#define AUX_CHANNEL_NUM 3 /* number of channels */

/* use by mtk_ts_cpu.c */
int bts_cur_temp = 1;

struct _ntc_bts_channel_param {
	int g_RAP_pull_up_R;
	int g_TAP_over_critical_low;
	int g_RAP_pull_up_voltage;
	int g_RAP_ntc_table;
	int g_RAP_ADC_channel;
	int g_AP_TemperatureR;
};

static struct _ntc_bts_channel_param bts_channel_param[AUX_CHANNEL_NUM];

static BTS_TEMPERATURE BTS_Temperature_Table[] = {
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}, {0,0},
	{0,0}, {0,0}, {0,0}, {0,0}
};

/*AP_NTC_BL197 */
BTS_TEMPERATURE BTS_Temperature_Table1[] = {
	{-40,74354}, {-35,74354}, {-30,74354}, {-25,74354}, {-20,74354},
	{-15,57626}, {-10,45068}, { -5,35548}, {  0,28267}, {  5,22650},
	{ 10,18280}, { 15,14855}, { 20,12151}, { 25,10000},/*10K*/ { 30,8279},
	{ 35,6892},  { 40,5768},  { 45,4852},  { 50,4101},  { 55,3483},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970}
};

/*AP_NTC_TSM_1*/
BTS_TEMPERATURE BTS_Temperature_Table2[] = {
	{-40,70603}, {-35,70603}, {-30,70603}, {-25,70603}, {-20,70603},
	{-15,55183}, {-10,43499}, { -5,34569}, {  0,27680}, {  5,22316},
	{ 10,18104}, { 15,14773}, { 20,12122}, { 25,10000},/*10K*/ { 30,8294},
	{ 35,6915},  { 40,5795},  { 45,4882},  { 50,4133},  { 55,3516},
	{ 60,3004},  { 60,3004},  { 60,3004},  { 60,3004},  { 60,3004},
	{ 60,3004},  { 60,3004},  { 60,3004},  { 60,3004},  { 60,3004},
	{ 60,3004},  { 60,3004},  { 60,3004},  { 60,3004}
};

/*AP_NTC_10_SEN_1*/
BTS_TEMPERATURE BTS_Temperature_Table3[] = {
	{-40,74354}, {-35,74354}, {-30,74354}, {-25,74354}, {-20,74354},
	{-15,57626}, {-10,45068}, { -5,35548}, {  0,28267}, {  5,22650},
	{ 10,18280}, { 15,14855}, { 20,12151}, { 25,10000},/*10K*/ { 30,8279},
	{ 35,6892},  { 40,5768},  { 45,4852},  { 50,4101},  { 55,3483},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},  { 60,2970},
	{ 60,2970},  { 60,2970},  { 60,2970},  { 60,2970}
};

/*AP_NTC_10(TSM0A103F34D1RZ)/ NTCG103JF103F */
BTS_TEMPERATURE BTS_Temperature_Table4[] = {
	{-40,188500}, {-35,144290}, {-30,111330}, {-25,86560}, {-20,67790},
	{-15,53460},  {-10,42450},  { -5,33930},  {  0,27280}, {  5,22070},
	{ 10,17960},  { 15,14700},  { 20,12090},  { 25,10000},/*10K*/ { 30,8310},
	{ 35,6940},   { 40,5830},   { 45,4910},   { 50,4160},  { 55,3540},
	{ 60,3020},   { 65,2590},   { 70,2230},   { 75,1920},  { 80,1670},
	{ 85,1450},   { 90,1270},   { 95,1110},   { 100,975},  { 105,860},
	{ 110,760},   { 115,674},   { 120,599},   { 125,534}
};

/*AP_NTC_47*/
BTS_TEMPERATURE BTS_Temperature_Table5[] = {
	{-40,483954}, {-35,483954}, {-30,483954}, {-25,483954}, {-20,483954},
	{-15,360850}, {-10,271697}, { -5,206463}, {  0,158214}, {  5,122259},
	{ 10,95227},  { 15,74730},  { 20,59065},  { 25,47000},/*47K*/ { 30,37643},
	{ 35,30334},  { 40,24591},  { 45,20048},  { 50,16433},  { 55,13539},
	{ 60,11210},  { 60,11210},  { 60,11210},  { 60,11210},  { 60,11210},
	{ 60,11210},  { 60,11210},  { 60,11210},  { 60,11210},  { 60,11210},
	{ 60,11210},  { 60,11210},  { 60,11210},  { 60,11210}
};


/*NTCG104EF104F(100K)*/
BTS_TEMPERATURE BTS_Temperature_Table6[] = {
	{-40,4251000}, {-35,3005000}, {-30,2149000}, {-25,1554000}, {-20,1135000},
	{-15,837800},  {-10,624100},  { -5,469100},  {  0,355600},  {  5,271800},
	{ 10,209400},  { 15,162500},  { 20,127000},  { 25,100000},/*100K*/ { 30,79230},
	{ 35,63180},   { 40,50680},   { 45,40900},   { 50,33190},   { 55,27090},
	{ 60,22220},   { 65,18320},   { 70,15180},   { 75,12640},   { 80,10580},
	{ 85, 8887},   { 90, 7500},   { 95, 6357},   { 100,5410},   { 105,4623},
	{ 110,3965},   { 115,3415},   { 120,2951},   { 125,2560}
};

/*NCP15XH103F03RC*/
BTS_TEMPERATURE BTS_Temperature_Table7[] = {
	{-40,195652}, {-35,148171}, {-30,113347}, {-25,87559}, {-20,68237},
	{-15,53650},  {-10,42506},  { -5,33892},  {  0,27219},  {  5,22021},
	{ 10,17926},  { 15,14674},  { 20,12080},  { 25,10000},/*100K*/ { 30,8315},
	{ 35,6948},  { 40,5833},  { 45,4917},   { 50,4161},  { 55,3535},
	{ 60,3014},   { 65,2586},   { 70,2228},   { 75,1925},  { 80,1669},
	{ 85,1452},   { 90,1268},   { 95,1110},   { 100,974},  { 105,858},
	{ 110,758},   { 115,671},   { 120,596},    { 125,531}
};

/*=============================================================
 *Weak functions
 *=============================================================
 */
int __attribute__ ((weak))
IMM_GetOneChannelValue(int dwChannel, int data[4], int *rawdata)
{
	pr_err("E_WF: %s doesn't exist\n", __func__);
	return -1;
}
/*=============================================================*/

/* =========== bts temp read ========== */

/* convert register to temperature  */
static INT32 mtkts_bts_thermistor_conver_temp(INT32 Res)
{
	int i=0;
	int asize=0;
	INT32 RES1=0,RES2=0;
	INT32 TAP_Value=-200,TMP1=0,TMP2=0;

	asize = (sizeof(BTS_Temperature_Table)/sizeof(BTS_TEMPERATURE));
	if(Res >= BTS_Temperature_Table[0].TemperatureR)
	{
		TAP_Value = -40; /* min */
	}
	else if(Res <= BTS_Temperature_Table[asize-1].TemperatureR)
	{
		TAP_Value = 125; /* max */
	}
	else
	{
		RES1 = BTS_Temperature_Table[0].TemperatureR;
		TMP1 = BTS_Temperature_Table[0].BTS_Temp;

		for(i=0; i < asize; i++)
		{
			if(Res >= BTS_Temperature_Table[i].TemperatureR)
			{
				RES2 = BTS_Temperature_Table[i].TemperatureR;
				TMP2 = BTS_Temperature_Table[i].BTS_Temp;
				break;
			}
			else
			{
				RES1 = BTS_Temperature_Table[i].TemperatureR;
				TMP1 = BTS_Temperature_Table[i].BTS_Temp;
			}
		}
		TAP_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))*1000/(RES1-RES2);
	}

	return TAP_Value;
}

/* convert ADC_AP_temp_volt to register */
/* Volt to Temp formula same with 6589  */
static INT32 mtkts_bts_volt_to_temp(int index, UINT32 dwVolt)

{
    INT32 TRes;
    INT32 dwVCriAP = 0;
    INT32 BTS_TMP = -100;

    /* SW workaround-----------------------------------------------------
      dwVCriAP = (TAP_OVER_CRITICAL_LOW * 1800) / (TAP_OVER_CRITICAL_LOW + 39000);
      dwVCriAP = (TAP_OVER_CRITICAL_LOW * RAP_PULL_UP_VOLT) / (TAP_OVER_CRITICAL_LOW + RAP_PULL_UP_R);
    */
    dwVCriAP = (bts_channel_param[index].g_TAP_over_critical_low * bts_channel_param[index].g_RAP_pull_up_voltage) /
			(bts_channel_param[index].g_TAP_over_critical_low + bts_channel_param[index].g_RAP_pull_up_R);

    if(dwVolt > dwVCriAP)
    {
        TRes = bts_channel_param[index].g_TAP_over_critical_low;
    }
    else
    {
        TRes = (bts_channel_param[index].g_RAP_pull_up_R*dwVolt) / (bts_channel_param[index].g_RAP_pull_up_voltage-dwVolt);
    }

    bts_channel_param[index].g_AP_TemperatureR = TRes;

    /* convert register to temperature */
    BTS_TMP = mtkts_bts_thermistor_conver_temp(TRes);
    pr_debug("Thermal %s: TRes = %d, BTS_TMP=%d\n", __func__, TRes, BTS_TMP);

    return BTS_TMP;
}

#if USE_AUXADC_API
static int get_hw_bts_temp(int index, int *temp)
{

	int ret = 0, i, ret_temp = 0, ret_value = 0, output;
	int times=2, Channel = bts_channel_param[index].g_RAP_ADC_channel;
	int data[4];
	//int v_pullup = bts_channel_param[index].g_RAP_pull_up_voltage;
	static int valid_temp;

	i = times;
	while (i--)
	{
		ret_value = IMM_GetOneChannelValue(Channel, data, &ret_temp);
		if (ret_value == -1) {/* AUXADC is busy */
			ret_temp = valid_temp;
		} else {
			valid_temp = ret_temp;
		}
		ret += ret_temp;
		pr_debug("%s:IMM_GetOneChannelValue(channel:%d)]: ret_temp=%d\n", __func__,
				Channel, ret_temp);
		msleep(5); /* from saradc_ntc_bts.c */
	}

	if(ret == 0)
	{
		pr_err("ADC channel %d reading error\n", Channel);
		return -EINVAL;
	}

	/* Mt_auxadc_hal.c */
	/* #define VOLTAGE_FULL_RANGE  1500 // VA voltage */
	/* #define AUXADC_PRECISE      4096 // 12 bits */
	ret = ret * 1500 / 4096;
	//ret = ret*v_pullup/adc_range; /* 82's ADC power */
	pr_debug("Channel = %d\n", Channel);
	pr_debug("adc reading 0x%x, APtery output mV = %d\n",ret_temp, ret);
	output = mtkts_bts_volt_to_temp(index, ret);
	*temp = output;
	pr_debug("BTS output temperature = %d\n",output);

	return 0;
}
#endif

static DEFINE_MUTEX(BTS_lock);
int mtkts_bts_get_hw_temp(int index, int *temp)
{
	int t_ret=0, ret=0;
#ifdef CONFIG_AMAZON_METRICS_LOG
	char buf[THERMO_METRICS_STR_LEN + 1];
	char *thermal_metric_prefix = "tmon:def";
#endif


	mutex_lock(&BTS_lock);

#if USE_AUXADC_API
	ret = get_hw_bts_temp(index, &t_ret);
#else
	ret = gadc_thermal_get_temp(gti_ntc, index, &t_ret);
#endif
	mutex_unlock(&BTS_lock);

	if (ret != 0) return ret;

	bts_cur_temp = t_ret;

	if (t_ret > 60000){ /* abnormal high temp */
		pr_info("[Power/BTS_Thermal] T_AP=%d\n", t_ret);
	}
	/* Print warning when it's < -10degC */
	if (t_ret < -10000) { /* abnormal low temp */
		pr_info("[Power/BTS_Thermal] T_AP=%d\n", t_ret);
		ret = -EINVAL;
	}


#ifdef CONFIG_AMAZON_METRICS_LOG
	if (!index)
		metrics_cnt++;
	if ((METRICSCOUNT <= metrics_cnt) && (metrics_cnt < METRICSCOUNT+AUX_CHANNEL_NUM)){
		snprintf(buf, THERMO_METRICS_STR_LEN,
				"%s:thermistor%d=%d;CT;1:NR",
				thermal_metric_prefix, index, t_ret);
		log_to_metrics(ANDROID_LOG_INFO, "ThermalEvent", buf);
		metrics_cnt++;
	}
	if(metrics_cnt == METRICSCOUNT+AUX_CHANNEL_NUM)
		metrics_cnt = 0;
#endif
	pr_debug("[ntc_bts_get_hw_temp] index %d T_AP, %d\n", index, t_ret);
	*temp = t_ret;

	return ret;
}

/* sysfs to read temperature.
   Usage:
   p212:/ # cat sys/bus/platform/devices/thermistor/temp
   channel 0 temperature 87
   p212:/ # cat sys/bus/platform/devices/thermistor1/temp
   channel 1 temperature 109
*/
static ssize_t ntc_bts_show_temp(struct device *dev,
				 struct device_attribute *devattr, char *buf)
{
	struct _ntc_bts_channel_param *param = dev_get_drvdata(dev);
	int index, ret, temp;

	if (!param) return -EINVAL;
	//index = (param == &bts_channel_param[0]) ? 0 : 1;
	index = param->g_RAP_ADC_channel;
#if USE_AUXADC_API
	ret = get_hw_bts_temp(index, &temp);
#else
	ret = gadc_thermal_get_temp(gti_ntc, index, &temp);
#endif
	if (ret)
		return sprintf(buf, "channel %d temperature read error, ret=%d\n", index, ret);
	else
		return sprintf(buf, "channel %d temperature %d\n", index, temp);
}

static DEVICE_ATTR(temp, S_IRUGO, ntc_bts_show_temp, NULL);

/* ========= bts table/tzbts_param handling =========== */

void ntc_bts_copy_table(BTS_TEMPERATURE *des,BTS_TEMPERATURE *src)
{
	int i=0;
	int j=0;

	j = (sizeof(BTS_Temperature_Table)/sizeof(BTS_TEMPERATURE));
	for(i=0; i<j; i++)
	{
		des[i] = src[i];
	}
}

void ntc_bts_prepare_table(int table_num)
{
	pr_info("Thermal %s with %d\n", __func__, table_num);

	switch(table_num)
	{
		case 1://AP_NTC_BL197
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table1);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table1));
			break;
		case 2://AP_NTC_TSM_1
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table2);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table2));
			break;
		case 3://AP_NTC_10_SEN_1
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table3);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table3));
			break;
		case 4://AP_NTC_10
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table4);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table4));
			break;
		case 5://AP_NTC_47
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table5);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table5));
			break;
		case 6://NTCG104EF104F
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table6);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table6));
			break;
                case 7://NCP15XH103F03RC
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table7);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table7));
                        break;

		default://AP_NTC_10
			ntc_bts_copy_table(BTS_Temperature_Table,BTS_Temperature_Table4);
			BUG_ON(sizeof(BTS_Temperature_Table)!=sizeof(BTS_Temperature_Table4));
			break;
	}
}

/* ========= virtual sensor handling =========== */
#ifdef CONFIG_THERMAL_VIRTUAL_SENSOR

static int ntc_bts_read_temp(struct device *dev, int index, int *temp)
{
	if (index < 0 || index > AUX_CHANNEL_NUM) {
		pr_err("%s bad channel index %d\n", __func__, index);
		return -EINVAL;
	}

	return mtkts_bts_get_hw_temp(index, temp);
}
static struct thermal_dev_ops ntc_bts_fops = {
	.get_temp = ntc_bts_read_temp,
};
#endif

/* ========= bts device/driver handling =========== */

static int ntc_bts_probe(struct platform_device *pdev)
{
	int ret = 0;
	static int serial = 0; // count of bts devices

#ifdef CONFIG_BOARD_ID
	int board_rev = 0;
#else
#ifdef CONFIG_IDME
	unsigned int board_rev = 0;
#endif
#endif
	/* For both thermistor devices: pdev->id is -1 pdev->dev.id is 0 */

	if (!pdev->dev.of_node) {
		pr_err("%s: Error No of_node\n", __func__);
		return -EINVAL;
	}

	if (serial >= AUX_CHANNEL_NUM) {
		pr_err("%s: serial num %d larger than max\n", __func__, serial);
		return -EINVAL;
	}

	ret |= of_property_read_u32(pdev->dev.of_node, "pull_up_resistor",  &bts_channel_param[serial].g_RAP_pull_up_R);
	ret |= of_property_read_u32(pdev->dev.of_node, "over_critical_low", &bts_channel_param[serial].g_TAP_over_critical_low);
	ret |= of_property_read_u32(pdev->dev.of_node, "pull_up_voltage",   &bts_channel_param[serial].g_RAP_pull_up_voltage);
	ret |= of_property_read_u32(pdev->dev.of_node, "ntc_table",         &bts_channel_param[serial].g_RAP_ntc_table);
	ret |= of_property_read_u32(pdev->dev.of_node, "adc_channel",       &bts_channel_param[serial].g_RAP_ADC_channel);
	pr_info("%s, reading one bts device, pull_up_resistor %d, pull_up_voltage %d over_critical_low %d ntc_table %d channel %d\n",
			__func__, bts_channel_param[serial].g_RAP_pull_up_R, bts_channel_param[serial].g_RAP_pull_up_voltage,
			bts_channel_param[serial].g_TAP_over_critical_low, bts_channel_param[serial].g_RAP_ntc_table,
			bts_channel_param[serial].g_RAP_ADC_channel);
	if (ret) {
		pr_err("%s: Error bts params not complete\n", __func__);
		return -EINVAL;
	}

	/*HVT and before have 39K pullup*/
#ifdef CONFIG_BOARD_ID
	board_rev = get_board_id();
	if (board_rev == -1) {
		pr_info("%s: board_rev not ready, defer probe\n", __func__);
		return -EPROBE_DEFER;
	}

	switch (board_rev) {
	case M_DOE:
	case M_PROTO:
		bts_channel_param[serial].g_RAP_pull_up_R = PRIOR_HVT_PULLUP_R;
		break;
	case M_HVT:
	case M_EVT:
	case M_DVT:
	case M_PVT:
		break;
	default:
		pr_info("%s: invalid board_rev:%d\n", __func__, board_rev);
		break;
	}
	pr_info("bts device adjusted pullup R: %d\n", bts_channel_param[serial].g_RAP_pull_up_R);

#else
#ifdef CONFIG_IDME
	board_rev = idme_get_board_rev();
	pr_info("IDME board_rev = 0x%x\n", board_rev);
	if (board_rev < 0 || board_rev > PVT_MANTIS_BOARD_REV) {
		pr_err("%s: board_rev:0x%x out of range\n", __func__, board_rev);
	}
	if (board_rev >= 0 && board_rev < HVT_MANTIS_BOARD_REV ) {
		bts_channel_param[serial].g_RAP_pull_up_R = PRIOR_HVT_PULLUP_R;
	}
	pr_info("bts device adjusted pullup R: %d\n", bts_channel_param[serial].g_RAP_pull_up_R);

#endif /* CONFIG_IDME */
#endif /* CONFIG_BOARD_ID */
	// Note all channels should have same table otherwise overwritten
	ntc_bts_prepare_table(bts_channel_param[serial].g_RAP_ntc_table);

#ifdef CONFIG_THERMAL_VIRTUAL_SENSOR
	ret = virtual_sensor_dev_register(&(pdev->dev), &ntc_bts_fops, serial);
	if (ret) {
		pr_err("%s: Error registering thermal device\n", __func__);
		return -EINVAL;
	}
#endif

	dev_set_drvdata(&pdev->dev, &bts_channel_param[serial]);

	ret = device_create_file(&pdev->dev, &dev_attr_temp);
	if (ret)
		pr_err("%s Failed to create params attr\n", __func__);

	serial++;
#ifdef CONFIG_AMAZON_METRICS_LOG
	metrics_cnt = 0;
#endif

	return 0;
}

static int ntc_bts_remove(struct platform_device *pdev)
{
	struct virtual_thermal_dev *vthermal_dev = dev_get_drvdata(&pdev->dev);

	if (vthermal_dev) kfree(vthermal_dev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ntc_bts_dt_match[] = {
	{	.compatible = "mtk,thermistor",},
	{},
};
#else
#define ntc_bts_dt_match NULL
#endif

static struct platform_driver ntc_bts_driver = {
	.probe = ntc_bts_probe,
	.remove = ntc_bts_remove,
	.driver     = {
		.name   = "thermistor",
        .owner = THIS_MODULE,
		.of_match_table = ntc_bts_dt_match,
	},
};

static int __init ntc_bts_init(void)
{
	int ret;
	ret = platform_driver_register(&ntc_bts_driver);
	if (ret) {
		pr_err("Unable to register ntc_bts thermal driver (%d)\n", ret);
		return ret;
	}
	return 0;
}

static void __exit ntc_bts_exit(void)
{
	platform_driver_unregister(&ntc_bts_driver);
}

late_initcall(ntc_bts_init);
module_exit(ntc_bts_exit);


#if !USE_AUXADC_API
int adc_cod_read(int index, int *val)
{
	struct gadc_thermal_info *gti = gti_ntc;
	struct iio_channel *channel;
	int ret = 0;

	if (gti == NULL)
		return ret;

	if (index >= AUX_CHANNEL_NUM) {
		dev_err(gti->dev, "%s: index:%d is more than defined. AUX_CHANNEL_NUM:%d, \
				should match how many defined in device tree\n",
				__func__, index, AUX_CHANNEL_NUM);
		return -EINVAL;
	}

	channel = &gti->channel[index];
	if (!channel)
		return -EINVAL;

	/* ret here is 1, seems like only negative number is error?
	 * @read_raw:function to request a value from the device.
	 *			mask specifieswhich value. Note 0 means a reading of
	 *			the channel in question.  Return value will specify the
	 *			type of value returned by the device. val and val2 will
	 *			contain the elements making up the returned value.
	 */
	ret = iio_read_channel_processed(channel, val);
	if (ret < 0) {
		dev_err(gti->dev, "IIO channel read failed %d\n", ret);
		return ret;
	}
	return 0;
}
static int gadc_thermal_get_temp(void *data, int index, int *temp)
{
	struct gadc_thermal_info *gti = data;
	struct iio_channel *channel;
	int val;
	int ret = 0;

	if (gti == NULL)
		return ret;

	if (index >= AUX_CHANNEL_NUM) {
		dev_err(gti->dev, "%s: index:%d is more than defined. AUX_CHANNEL_NUM:%d, \
				should match how many defined in device tree\n",
				__func__, index, AUX_CHANNEL_NUM);
		return -EINVAL;
	}

	channel = &gti->channel[index];
	if (!channel)
		return -EINVAL;

	/* ret here is 1, seems like only negative number is error?
	 * @read_raw:function to request a value from the device.
	 *			mask specifieswhich value. Note 0 means a reading of
	 *			the channel in question.  Return value will specify the
	 *			type of value returned by the device. val and val2 will
	 *			contain the elements making up the returned value.
	 */
	ret = iio_read_channel_processed(channel, &val);
	if (ret < 0) {
		dev_err(gti->dev, "IIO channel read failed %d\n", ret);
		return ret;
	}
	/* pr_debug("%s: gti->channel[%d] from dt, maps to channel:%d from iio adc device\n",
			__func__, index, channel->channel->channel); */
	val = val * 1500 / 4096;
	*temp = mtkts_bts_volt_to_temp(index, val);

	/* see above comments */
	return 0;
}

static int gadc_thermal_probe(struct platform_device *pdev)
{
	struct gadc_thermal_info *gti;
	int ret;

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "Only DT based supported\n");
		return -ENODEV;
	}

	gti = devm_kzalloc(&pdev->dev, sizeof(*gti), GFP_KERNEL);
	if (!gti)
		return -ENOMEM;
	gti_ntc = gti;

	gti->dev = &pdev->dev;
	platform_set_drvdata(pdev, gti);

	gti->channel = iio_channel_get_all(&pdev->dev);

	/* gti->channel = iio_channel_get(&pdev->dev, "sensor-channel"); */
	if (IS_ERR(gti->channel)) {
		ret = PTR_ERR(gti->channel);
		dev_err(&pdev->dev, "IIO channel not found: %d\n", ret);
		return ret;
	}

	/* pr_debug("%s: iio adc device has total: %d channels\n", __func__,  gti->channel->indio_dev->num_channels); */

	pr_err("%s OK\n", __func__);

	return 0;

#if 0
sensor_fail:
	iio_channel_release(gti->channel);

	return ret;
#endif
}

static int gadc_thermal_remove(struct platform_device *pdev)
{
	struct gadc_thermal_info *gti = platform_get_drvdata(pdev);

	iio_channel_release(gti->channel);

	return 0;
}

static const struct of_device_id of_adc_thermal_match[] = {
	{ .compatible = "generic-adc-thermal", },
	{},
};
MODULE_DEVICE_TABLE(of, of_adc_thermal_match);

static struct platform_driver gadc_thermal_driver = {
	.driver = {
		.name = "generic-adc-thermal",
		.of_match_table = of_adc_thermal_match,
	},
	.probe = gadc_thermal_probe,
	.remove = gadc_thermal_remove,
};

static int __init gadc_thermal_init(void)
{
	return platform_driver_register(&gadc_thermal_driver);
}

static void __exit gadc_thermal_exit(void)
{
	return platform_driver_unregister(&gadc_thermal_driver);
}

late_initcall(gadc_thermal_init);
module_exit(gadc_thermal_exit);

#endif
