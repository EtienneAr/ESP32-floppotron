/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/adc.h>
#include "driver/uart.h"

#include "player.h"
#include "gpioToADC.h"

static adc1_channel_t adc_channels[3] = { ADC_CHAN_POTA, ADC_CHAN_POTB, ADC_CHAN_POTC };

void check_potentiometer() {
	int position[3], prev_position[3];
  int j;

	adc1_config_width(ADC_WIDTH_BIT_9); //minimum resolution
  for(j=0;j<3;j++) {
    adc1_config_channel_atten(adc_channels[j],ADC_ATTEN_DB_11); //Largest scale : 3.9V
    prev_position[j] = 0;
  }

	do {
    for(j=0;j<3;j++) {
  		position[j] = adc1_get_raw(adc_channels[j]) * PLAYER_MAX_POSITION / (1<<9) ; //position is coded on  bits.
  		if(position[j] - 2 > prev_position[j] || position[j] + 2 < prev_position[j]) {
  			player_set_position(j, position[j]);
  			prev_position[j] = position[j];
  		}
    }
		vTaskDelay(50 / portTICK_PERIOD_MS);
	} while(1);
}

static const int RX_BUF_SIZE = 512;

#define RXD_PIN (GPIO_NUM_27)

void uart_task() {
    const uart_config_t uart_config = {
        .baud_rate = 31250,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    
    uint8_t buff_bytes[3];
    size_t buff_len;
    while(true) {
    	uart_get_buffered_data_len(UART_NUM_1, &buff_len);
    	if(buff_len > 2) {
    		uart_read_bytes(UART_NUM_1, buff_bytes, 3, 1000 / portTICK_RATE_MS);
    		//printf("cmd : %d, note : %d, velo : %d\n", buff_bytes[0], buff_bytes[1], buff_bytes[2]);
        if(buff_bytes[0] >> 4 == 0x9) player_play(buff_bytes[1], buff_bytes[0] & ((1<<4)-1));
        if(buff_bytes[0] >> 4 == 0x8) player_stop(buff_bytes[1], buff_bytes[0] & ((1<<4)-1));
    	}
    	vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
  printf("Main running on core : %d\n", xPortGetCoreID());

  player_init((BaseType_t) 1);
  xTaskCreatePinnedToCore(check_potentiometer, "potentiometer_task", 2048, NULL, 10, NULL, 0);
  xTaskCreatePinnedToCore(uart_task, "uart_task", 2048, NULL, 10, NULL, 0);
}
