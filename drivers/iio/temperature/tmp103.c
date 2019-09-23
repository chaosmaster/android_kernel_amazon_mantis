/*
 * tmp103 Temperature sensor driver file
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 * Author: Steven King <sfking@fdwdc.com>
 * Author: Sabatier, Sebastien" <s-sabatier1@ti.com>
 * Author: Mandrenko, Ievgen" <ievgen.mandrenko@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/stddef.h>
#include <linux/sysfs.h>
#include <linux/err.h>
#include <linux/reboot.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/thermal_framework.h>

#ifdef CONFIG_AMAZON_METRICS_LOG
#include <linux/metricslog.h>
#define TMP103_METRICS_STR_LEN 128
#endif

#define TMP103_TEMP_REG    0x00
#define TMP103_CONF_REG    0x01

#define TMP103_CONF_M0     0x0001
#define TMP103_CONF_M1     0x0002
#define TMP103_CONF_LC     0x0004
#define TMP103_CONF_FL     0x0008
#define TMP103_CONF_FH     0x0010
#define TMP103_CONF_CR0    0x0020
#define TMP103_CONF_CR1    0x0040
#define TMP103_CONF_ID     0x0080

#define TMP103_TLOW_REG    0x02
#define TMP103_THIGH_REG   0x03

#define TMP103_SHUTDOWN    0x01
#define TMP103_MAX_TEMP    127000

/*
 * omap_temp_sensor structure
 * @iclient - I2c client pointer
 * @dev - device pointer
 * @sensor_mutex - Mutex for sysfs, irq and PM
 */
struct tmp103_temp_sensor {
	struct i2c_client *iclient;
	struct device *dev;
	struct mutex sensor_mutex;
	u16 config_orig;
	u16 config_current;
	unsigned long last_update;
	int temp[3];
	int debug_temp;
};

static inline int tmp103_read_reg(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static inline int tmp103_write_reg(struct i2c_client *client, u8 reg, u16 val)
{
	return i2c_smbus_write_byte_data(client, reg, val);
}

static inline int tmp103_reg_to_mC(u8 val)
{
	/*Negative numbers */
	if (val & 0x80) {
		val = ~val + 1;
		return -(val * 1000);
	}
	return val * 1000;
}

/* convert milliCelsius to 8-bit TMP103 register value */
static inline u8 tmp103_mC_to_reg(int val)
{
	return (val / 1000);
}

static const u8 tmp103_reg[] = {
	TMP103_TEMP_REG,
	TMP103_TLOW_REG,
	TMP103_THIGH_REG,
};

static int tmp103_read_current_temp(struct device *dev)
{
	int index = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct tmp103_temp_sensor *tmp103 = i2c_get_clientdata(client);

	//pr_debug("%s: client->name:%s client->addr:%d\n", __func__, client->name, client->addr);

	mutex_lock(&tmp103->sensor_mutex);
	if (time_after(jiffies, tmp103->last_update + HZ / 3)) {
		int status = tmp103_read_reg(client, tmp103_reg[index]);
		if (status > -1) {
			//pr_debug("%s: status:%d\n", __func__, status);
			tmp103->temp[index] = tmp103_reg_to_mC(status);
		}
		tmp103->last_update = jiffies;
	}
	mutex_unlock(&tmp103->sensor_mutex);

	return tmp103->temp[index];
}

#ifdef CONFIG_THERMAL_VIRTUAL_SENSOR
#define MAX_RETRY 5		/* retry count when PCB temp reading is 127 which could be a faulty read */

/*
 * if tmp103_get_temp returns saved value straight 50 times,
 * then return faulty one so system can thermal shutdown.
 */
#define MAX_FAULTY_RETURN_COUNT 50
static int tmp103_get_temp(struct device *dev, int data, int *temp)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct tmp103_temp_sensor *tmp103 = platform_get_drvdata(pdev);
	int current_temp;
	static int saved_temp;
	int count = 1;
	struct i2c_client *client = tmp103->iclient;
#ifdef CONFIG_AMAZON_METRICS_LOG
	char *tmp103_metric_prefix = "pcbsensor:def";
	char buf[TMP103_METRICS_STR_LEN];
#endif
	static int fail_count;

	current_temp = tmp103_read_current_temp(dev);

	if (unlikely(current_temp >= TMP103_MAX_TEMP)) {
#ifdef CONFIG_AMAZON_METRICS_LOG
		/* Log in metrics */
		snprintf(buf, TMP103_METRICS_STR_LEN, "%s:abnormal_temp=%d;CT;1:NR",
			 tmp103_metric_prefix,
			 current_temp);
		log_to_metrics(ANDROID_LOG_INFO, "ThermalEvent", buf);
#endif
		pr_info("TMP103 reads abnormal temperature %d", current_temp);

		/* switch to shutdown mode, then swtich back to previous mode ( contious conversion mode) */
		tmp103_write_reg(client, TMP103_CONF_REG, 0);	/* shutdown mode: TMP103_CONF_M1=0, TMP103_CONF_M0=0 */
		tmp103_write_reg(client, TMP103_CONF_REG, tmp103->config_current);	/* start conversion */

		/* retry */
		do {
			current_temp = tmp103_read_current_temp(dev);
#ifdef CONFIG_AMAZON_METRICS_LOG
			/* Log in metrics */
			snprintf(buf, TMP103_METRICS_STR_LEN,
				 "%s:pcbtemp=%d;CT;1,retry=%d;CT;1:NR",
				 tmp103_metric_prefix, current_temp, count);
			log_to_metrics(ANDROID_LOG_INFO, "ThermalEvent", buf);
#endif
			pr_info("TMP103 reads temperature %d, retrying... %d ", current_temp,
				count);
		} while ((count++ < MAX_RETRY)
			 && (!current_temp || (current_temp >= TMP103_MAX_TEMP)));
	}

	if (unlikely(current_temp >= TMP103_MAX_TEMP)) {
		if (++fail_count > MAX_FAULTY_RETURN_COUNT) {
			pr_info("FATAL: TMP103 locked down, return faulty temperature %d,"
				" system will shutdown....", current_temp);
			return -EINVAL;
		}
		current_temp = saved_temp;
#ifdef CONFIG_AMAZON_METRICS_LOG
		/* Log in metrics */
		snprintf(buf, TMP103_METRICS_STR_LEN,
			 "%s:saved_temp=%d;CT;1,use_saved_temp=1;CT;1:NR",
			 tmp103_metric_prefix, saved_temp);
		log_to_metrics(ANDROID_LOG_INFO, "ThermalEvent", buf);
#endif
		pr_info("WARNING: TMP103 retry failed, return last saved temperature %d",
			saved_temp);
	} else {		/* less than TMP103_MAX_TEMP read */
		fail_count = 0;
		saved_temp = current_temp;
	}

	saved_temp = current_temp;
	*temp = current_temp;
	return 0;
}
static struct thermal_dev_ops tmp103_fops = {
	.get_temp = tmp103_get_temp,
};

#endif //CONFIG_THERMAL_VIRTUAL_SENSOR

/*
 * sysfs hook functions
 */
static ssize_t tmp103_show_temp_user_space(struct device *dev,
					   struct device_attribute *devattr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tmp103_temp_sensor *tmp103 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", tmp103->debug_temp);
}

static ssize_t tmp103_set_temp_user_space(struct device *dev,
					  struct device_attribute *devattr, const char *buf,
					  size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tmp103_temp_sensor *tmp103 = i2c_get_clientdata(client);
	long val;

	if (kstrtol(buf, 10, &val)) {
		count = -EINVAL;
		goto out;
	}

	/* Set new temperature */
	tmp103->debug_temp = (int)val;

	/* Send a kobj_change */
	kobject_uevent(&tmp103->dev->kobj, KOBJ_CHANGE);

 out:
	return count;
}

static ssize_t tmp103_temp_sensor_read_temp(struct device *dev,
					struct device_attribute *devattr, char *buf)
{
	int temp = tmp103_read_current_temp(dev);

	return sprintf(buf, "%d\n", temp);
}

static DEVICE_ATTR(debug_user, S_IWUSR | S_IRUGO, tmp103_show_temp_user_space,
		   tmp103_set_temp_user_space);
static DEVICE_ATTR(temp1_input, S_IRUGO, tmp103_temp_sensor_read_temp, NULL);

static struct attribute *tmp103_temp_sensor_attributes[] = {
	&dev_attr_temp1_input.attr,
	&dev_attr_debug_user.attr,
	NULL
};

static const struct attribute_group tmp103_temp_sensor_attr_group = {
	.attrs = tmp103_temp_sensor_attributes,
};


static int tmp103_temp_sensor_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct tmp103_temp_sensor *tmp103;
	const char *str; // points to existing string
	int ret = 0;
	int new;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev, "adapter doesn't support SMBus word " "transactions\n");
		return -ENODEV;
	}

	tmp103 = kzalloc(sizeof(struct tmp103_temp_sensor), GFP_KERNEL);
	if (!tmp103)
		return -ENOMEM;

	mutex_init(&tmp103->sensor_mutex);

	tmp103->iclient = client;
	tmp103->dev = &client->dev;

	kobject_uevent(&client->dev.kobj, KOBJ_ADD);
	i2c_set_clientdata(client, tmp103);

	/* somehow the device_name is not set automatically so add it here
	   so that virtual sensor can use it to find
	 */
	ret = of_property_read_string(np, "device_name", &str);
	if (!ret){
		dev_info(&client->dev, "set name to %s\n", str);
		dev_set_name(&client->dev, str);
	}

	ret = tmp103_read_reg(client, TMP103_CONF_REG);
	if (ret < 0) {
		dev_err(&client->dev, "error reading config register\n");
//		goto free_err;
	}
	tmp103->config_orig = ret;

	/* continuous conversions, M1=1, so no need to clear */
	/* Conversion rate settings */
	/* By default, it is set to 4s. Align it to 250ms as used on TI mainline. */
	new = ret & ~0x62;
	new |= 0x42;

	if (ret != new) {
		ret = tmp103_write_reg(client, TMP103_CONF_REG, new);
		if (ret < 0) {
			dev_err(&client->dev, "error writing config register\n");
//			goto restore_config_err;
		}
	}

	tmp103->config_current = new;
	tmp103->last_update = jiffies - HZ;
	mutex_init(&tmp103->sensor_mutex);

	ret = sysfs_create_group(&client->dev.kobj, &tmp103_temp_sensor_attr_group);
	if (ret)
		goto sysfs_create_err;

#ifdef CONFIG_THERMAL_VIRTUAL_SENSOR
	ret = virtual_sensor_dev_register(tmp103->dev, &tmp103_fops, 0);
	if (ret) {
		pr_err("%s: Error registering thermal device\n", __func__);
		return -EINVAL;
	}
#endif

	dev_info(&client->dev, "client %s initialized\n", client->name);

	return 0;

 sysfs_create_err:
 //restore_config_err:
	tmp103_write_reg(client, TMP103_CONF_REG, tmp103->config_orig);
 //free_err:
	mutex_destroy(&tmp103->sensor_mutex);
	kfree(tmp103);

	return ret;
}

static int tmp103_temp_sensor_remove(struct i2c_client *client)
{
	struct tmp103_temp_sensor *tmp103 = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &tmp103_temp_sensor_attr_group);

	tmp103_write_reg(client, TMP103_CONF_REG, tmp103->config_orig);
	kfree(tmp103);

	return 0;
}


static const struct i2c_device_id tmp103_id[] = {
	{"tmp103_temp_sensor", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, tmp103_id);

static const struct of_device_id tmp103_of_id[] = {
	{ .compatible = "ti,tmp103", },
	{}
};
MODULE_DEVICE_TABLE(of, tmp103_of_id);

static struct i2c_driver tmp103_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = tmp103_temp_sensor_probe,
	.remove = tmp103_temp_sensor_remove,
	.driver = {
		   .name = "tmp103_temp_sensor",
		   .of_match_table = tmp103_of_id,
		   },
	.id_table = tmp103_id,
};

static int __init tmp103_init(void)
{
	return i2c_add_driver(&tmp103_driver);
}
module_init(tmp103_init);

static void __exit tmp103_exit(void)
{
	i2c_del_driver(&tmp103_driver);
}
module_exit(tmp103_exit);

MODULE_DESCRIPTION("tmp103 Temperature Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_AUTHOR("Texas Instruments Inc");
