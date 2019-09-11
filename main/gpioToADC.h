#ifndef GPIOTOADC
#define GPIOTOADC

#if CONFIG_POT_GPIO_A == 32
	#define ADC_CHAN_POTA ADC1_CHANNEL_4
#elif CONFIG_POT_GPIO_A == 33
	#define ADC_CHAN_POTA ADC1_CHANNEL_5
#elif CONFIG_POT_GPIO_A == 34
	#define ADC_CHAN_POTA ADC1_CHANNEL_6
#elif CONFIG_POT_GPIO_A == 35
	#define ADC_CHAN_POTA ADC1_CHANNEL_7
#elif CONFIG_POT_GPIO_A == 36
	#define ADC_CHAN_POTA ADC1_CHANNEL_0
#elif CONFIG_POT_GPIO_A == 37
	#error adc_A gpio not accessible
#elif CONFIG_POT_GPIO_A == 38
	#error adc_A gpio not accessible
#elif CONFIG_POT_GPIO_A == 39
	#define ADC_CHAN_POTA ADC1_CHANNEL_3
#endif

#if CONFIG_POT_GPIO_B == 32
	#define ADC_CHAN_POTB ADC1_CHANNEL_4
#elif CONFIG_POT_GPIO_B == 33
	#define ADC_CHAN_POTB ADC1_CHANNEL_5
#elif CONFIG_POT_GPIO_B == 34
	#define ADC_CHAN_POTB ADC1_CHANNEL_6
#elif CONFIG_POT_GPIO_B == 35
	#define ADC_CHAN_POTB ADC1_CHANNEL_7
#elif CONFIG_POT_GPIO_B == 36
	#define ADC_CHAN_POTB ADC1_CHANNEL_0
#elif CONFIG_POT_GPIO_B == 37
	#error adc_B gpio not accessible
#elif CONFIG_POT_GPIO_B == 38
	#error adc_B gpio not accessible
#elif CONFIG_POT_GPIO_B == 39
	#define ADC_CHAN_POTB ADC1_CHANNEL_3
#endif

#if CONFIG_POT_GPIO_C == 32
	#define ADC_CHAN_POTC ADC1_CHANNEL_4
#elif CONFIG_POT_GPIO_C == 33
	#define ADC_CHAN_POTC ADC1_CHANNEL_5
#elif CONFIG_POT_GPIO_C == 34
	#define ADC_CHAN_POTc ADC1_CHANNEL_6
#elif CONFIG_POT_GPIO_C == 35
	#define ADC_CHAN_POTC ADC1_CHANNEL_7
#elif CONFIG_POT_GPIO_C == 36
	#define ADC_CHAN_POTC ADC1_CHANNEL_0
#elif CONFIG_POT_GPIO_C == 37
	#error adc_C gpio not accessible
#elif CONFIG_POT_GPIO_C == 38
	#error adc_C gpio not accessible
#elif CONFIG_POT_GPIO_C == 39
	#define ADC_CHAN_POTC ADC1_CHANNEL_3
#endif

#endif