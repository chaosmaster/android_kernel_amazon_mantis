/*
 * SYSFS infrastructure specific to Lab126 product
 */
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>

#define MAXBUF 16
static char usb_charge_type[MAXBUF];

static struct kobject *amazon_kobj;

static int __init usb_charge_type_setup(char * str)
{
	snprintf(usb_charge_type, MAXBUF, "%s\n", str);

	return 1;
}

__setup("usb_extconn=", usb_charge_type_setup);

static ssize_t
usb_charge_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%s", usb_charge_type);

	return strlen(usb_charge_type);
}

static DEVICE_ATTR(usb_charge_type, 0444, usb_charge_type_show, NULL);


static struct attribute *amazon_attrs[] = {
	&dev_attr_usb_charge_type.attr,
	NULL,
};

static struct attribute_group amazon_attr_group = {
	.attrs = amazon_attrs,
};

static int __init amazon_sysfs_init(void)
{
	amazon_kobj = kobject_create_and_add("amazon", NULL);
	if (!amazon_kobj)
		return -ENOMEM;
	return sysfs_create_group(amazon_kobj, &amazon_attr_group);
}

static void __exit amazon_sysfs_exit(void)
{
	sysfs_remove_group(amazon_kobj, &amazon_attr_group);
}

module_init(amazon_sysfs_init);
module_exit(amazon_sysfs_exit);
