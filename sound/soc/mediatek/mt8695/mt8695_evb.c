/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <sound/soc.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>

enum PINCTRL_PIN_STATE {
	PIN_STATE_DEFAULT = 0,
	PIN_STATE_MAX
};

struct mt8695_priv {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pin_states[PIN_STATE_MAX];
	int dac_rst_gpio;
	int tdmadc_rst_gpio;
};

static const char *const mt8695_pinctrl_pin_str[PIN_STATE_MAX] = {
	"audio_default_pins",
};

static int tdmin_capture_startup(struct snd_pcm_substream *substream)
{

	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct mt8695_priv *card_data = snd_soc_card_get_drvdata(card);

	/* default enable AK5558VN power */
	if (gpio_is_valid(card_data->tdmadc_rst_gpio))
		gpio_set_value(card_data->tdmadc_rst_gpio, 1);
	return 0;
}

static void tdmin_capture_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct mt8695_priv *card_data = snd_soc_card_get_drvdata(card);

	/* default enable AK5558VN power */
	if (gpio_is_valid(card_data->tdmadc_rst_gpio))
		gpio_set_value(card_data->tdmadc_rst_gpio, 0);
}

static struct snd_soc_ops tdmin_capture_ops = {
	.startup = tdmin_capture_startup,
	.shutdown = tdmin_capture_shutdown,
};

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link mt8695_evb_dais[] = {
	/* Front End DAI Links */
	{
		.name = "HDMI_OUTPUT",
		.stream_name = "HDMI_Playback",
		.cpu_dai_name = "HDMI",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
		.name = "SPDIF_OUTPUT",
		.stream_name = "SPDIF_Playback",
		.cpu_dai_name = "SPDIF",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
		.name = "I2S_OUTPUT",
		.stream_name = "DL1_Playback",
		.cpu_dai_name = "DL1",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
		.name = "TDM_INPUT",
		.stream_name = "TDM_Capture",
		.cpu_dai_name = "TDM_IN",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
		.ops = &tdmin_capture_ops,
	},
	{
		.name = "DMIC_INPUT",
		.stream_name = "DMIC_Capture",
		.cpu_dai_name = "DMIC",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
		.name = "BTSCO_OUTPUT",
		.stream_name = "DL2_Playback",
		.cpu_dai_name = "DL2",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
		.name = "BTSCO_INPUT",
		.stream_name = "UL5_Capture",
		.cpu_dai_name = "UL5",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	/* Back End DAI Links */
	{
		.name = "HDMI BE",
		.cpu_dai_name = "8CH_I2S_OUT",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
				SND_SOC_DAIFMT_CBS_CFS,
		.dpcm_playback = 1,
	},
	{
		.name = "SPDIF BE",
		.cpu_dai_name = "IEC",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_playback = 1,
	},
	{
		.name = "I2S BE",
		.cpu_dai_name = "I2S",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dai_fmt = SND_SOC_DAIFMT_LEFT_J | SND_SOC_DAIFMT_NB_NF |
				SND_SOC_DAIFMT_CBS_CFS,
		.dpcm_playback = 1,
	},
	{
		.name = "TDM IN BE",
		.cpu_dai_name = "TDM_IN_IO",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
				SND_SOC_DAIFMT_CBS_CFS,
		.dpcm_capture = 1,
	},
	{
		.name = "DMIC BE",
		.cpu_dai_name = "DMIC_IO",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_capture = 1,
	},
	{
		.name = "BTSCO BE",
		.cpu_dai_name = "BTSCO_IO",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_playback = 1,
		.dpcm_capture = 1,
	},
};

static struct snd_soc_card mt8695_evb_card = {
	.name = "mt-snd-card",
	.owner = THIS_MODULE,
	.dai_link = mt8695_evb_dais,
	.num_links = ARRAY_SIZE(mt8695_evb_dais),
};

static int mt8695_evb_dev_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &mt8695_evb_card;
	struct device *dev = &pdev->dev;
	struct device_node *platform_node, *np;
	int ret, i;
	struct mt8695_priv *card_data;

	dev_dbg(dev, "mt8695_evb_dev_probe\n");
	platform_node = of_parse_phandle(dev->of_node, "mediatek,platform", 0);
	if (!platform_node) {
		dev_info(dev, "Property 'platform' missing or invalid\n");
		return -EINVAL;
	}

	for (i = 0; i < card->num_links; i++) {
		if (mt8695_evb_dais[i].platform_name)
			continue;
		mt8695_evb_dais[i].platform_of_node = platform_node;
	}
	card->dev = dev;

	card_data = devm_kzalloc(dev, sizeof(struct mt8695_priv), GFP_KERNEL);

	if (!card_data) {
		ret = -ENOMEM;
		dev_info(dev, "%s allocate card private data fail %d\n", __func__, ret);
		return ret;
	}
	snd_soc_card_set_drvdata(card, card_data);

	/* Init GPIOs */
	card_data->pinctrl = devm_pinctrl_get(card->dev);
	if (IS_ERR(card_data->pinctrl)) {
		ret = PTR_ERR(card_data->pinctrl);
		dev_info(card->dev, "%s pinctrl_get failed %d\n", __func__, ret);
	}

	if (!IS_ERR(card_data->pinctrl)) {
		for (i = 0; i < PIN_STATE_MAX; i++) {
			card_data->pin_states[i] =
				pinctrl_lookup_state(card_data->pinctrl, mt8695_pinctrl_pin_str[i]);
			if (IS_ERR(card_data->pin_states[i])) {
				ret = PTR_ERR(card_data->pin_states[i]);
				dev_info(card->dev, "%s Can't find pinctrl state %s %d\n",
					__func__, mt8695_pinctrl_pin_str[i], ret);
			}
		}
	}

	if (!IS_ERR(card_data->pin_states[PIN_STATE_DEFAULT])) {
		ret = pinctrl_select_state(card_data->pinctrl, card_data->pin_states[PIN_STATE_DEFAULT]);

	if (ret)
		dev_info(card->dev, "%s failed to select state %d\n", __func__, ret);
	}

	np = card->dev->of_node;
	card_data->dac_rst_gpio = of_get_named_gpio(np, "dac-rst-gpio", 0);
	if (!gpio_is_valid(card_data->dac_rst_gpio))
		dev_warn(card->dev, "%s get invalid dac_rst_gpio %d\n",
			__func__, card_data->dac_rst_gpio);
	/* Init AK5558VN RESET PIN*/
	card_data->tdmadc_rst_gpio = of_get_named_gpio(np, "tdmadc-rst-gpio", 0);
	if (!gpio_is_valid(card_data->tdmadc_rst_gpio))
		dev_warn(card->dev, "%s get invalid tdmadc_rst_gpio %d\n",
			__func__, card_data->tdmadc_rst_gpio);
	/* default enable AK5558VN power */
	if (gpio_is_valid(card_data->tdmadc_rst_gpio)) {
		ret = devm_gpio_request_one(card->dev, card_data->tdmadc_rst_gpio, GPIOF_OUT_INIT_LOW, "tdm adc reset");
		if (ret < 0)
			dev_err(card->dev, "%s failed to init tdm adc reset gpio %d\n",
				__func__, ret);
			gpio_set_value(card_data->tdmadc_rst_gpio, 1);
	}
	/* default enable AK4438VN power */
	if (gpio_is_valid(card_data->dac_rst_gpio)) {
		ret = devm_gpio_request_one(card->dev, card_data->dac_rst_gpio, GPIOF_OUT_INIT_LOW, "DAC RESET");
		if (ret < 0)
			dev_err(card->dev, "%s failed to init DAC reset gpio %d\n",
					__func__, ret);
		gpio_set_value(card_data->dac_rst_gpio, 1);
	}
	/* End Init GPIOs */

	ret = devm_snd_soc_register_card(dev, card);
	if (ret)
		dev_info(dev, "%s snd_soc_register_card fail %d\n", __func__, ret);

	return ret;
}


static const struct of_device_id mt8695_evb_machine_dt_match[] = {
	{.compatible = "mediatek,mt8695-audio-card",},
	{}
};

MODULE_DEVICE_TABLE(of, mt8695_evb_machine_dt_match);

static struct platform_driver mt8695_evb_machine_driver = {
	.driver = {
			.name = "mt8695-audio-card",
			.owner = THIS_MODULE,
			.of_match_table = mt8695_evb_machine_dt_match,
#ifdef CONFIG_PM
			.pm = &snd_soc_pm_ops,
#endif
			},
	.probe = mt8695_evb_dev_probe,
};

module_platform_driver(mt8695_evb_machine_driver);

/* Module information */
MODULE_DESCRIPTION("ASoC driver for MT8695 EVB");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mt8695-audio-card");

