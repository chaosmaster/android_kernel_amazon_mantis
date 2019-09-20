/*
 * Current overdraw detection logic
 */

#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/delay.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/printk.h>
#include "hdmi_tx_cod.h"
#include "../../../thermal/mt8695/inc/tmp_bts.h"

#define COD_RATE 2*HZ
#define COD_CHANNEL 2
#define COD_HDMIV_LIMIT 2094

static int cnt, det, state;
static struct iio_channel *channels;
struct workqueue_struct *hdmi_wq;
struct delayed_work work_cod;

static struct switch_dev sd = {
        .name = "hdmi_cod",
        .value =0,
};

static int get_hdmiv(unsigned int ch)
{
	int ret = 0, val = 0;

	ret = adc_cod_read(COD_CHANNEL, &val);
	if (ret < 0) {
		pr_err("IIO channel read failed %d\n", ret);
		return ret;
	}

	return val;
}

static void work_cod_handler(struct work_struct *work)
{
	int mv;
	mv = get_hdmiv(COD_CHANNEL);

	// Spec allows up to 50mA to be drawn by the sink
	if (state ? (mv >= COD_HDMIV_LIMIT) : (mv < COD_HDMIV_LIMIT)) {
		det++;
	}
	sd.value = mv;
	if (++cnt < 10) {
		if (det > 0) {
			state = !state;
			pr_warn("current overdraw detection: %d\n", state);
			switch_set_state(&sd, state ? mv : 0);
			cnt = det = 0;
		}
	} else {
		cnt = det = 0;
	}

	queue_delayed_work(hdmi_wq, &work_cod, COD_RATE);
}

void cod_init(struct platform_device *pdev)
{

	channels = iio_channel_get_all(&pdev->dev);
	if (IS_ERR(channels))
		pr_err("get all iio channel failed!\n");

	hdmi_wq = alloc_workqueue("cod", WQ_HIGHPRI | WQ_CPU_INTENSIVE, 0);

	switch_dev_register(&sd);
	INIT_DELAYED_WORK(&work_cod, work_cod_handler);
}

void cod_test(void)
{
	cancel_delayed_work_sync(&work_cod);
	switch_set_state(&sd, 0);
	cnt = det = state = 0;
	queue_delayed_work(hdmi_wq, &work_cod, COD_RATE);
}

void cod_fini(void)
{
	iio_channel_release(channels);

	cancel_delayed_work_sync(&work_cod);
	switch_dev_unregister(&sd);
}

