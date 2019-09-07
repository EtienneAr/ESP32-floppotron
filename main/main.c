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

void check_potentiometer() {
	int position, prev_position;

	adc1_config_width(ADC_WIDTH_BIT_9); //minimum resolution
  	adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); //Largest scale : 3.9V

	prev_position = 0;
	do {
		position = adc1_get_raw(ADC1_CHANNEL_6) * PLAYER_MAX_POSITION / (1<<9) ; //position is coded on  bits.
		if(position - 2 > prev_position || position + 2 < prev_position) {
			player_set_position(position);
			prev_position = position;
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	} while(1);
}

static const int RX_BUF_SIZE = 512;

#define TXD_PIN (GPIO_NUM_26)
#define RXD_PIN (GPIO_NUM_25)

void uart_task() {
    const uart_config_t uart_config = {
        .baud_rate = 31250,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    
    uint8_t buff_bytes[3];
    size_t buff_len;
    while(true) {
    	uart_get_buffered_data_len(UART_NUM_1, &buff_len);
    	if(buff_len > 2) {
    		uart_read_bytes(UART_NUM_1, buff_bytes, 3, 1000 / portTICK_RATE_MS);
    		printf("cmd : %d, note : %d, velo : %d\n", buff_bytes[0], buff_bytes[1], buff_bytes[2]);
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

  /*
  for(int i=12;i<72;i++) {
  	player_play(i);
  	vTaskDelay(250 / portTICK_PERIOD_MS);
  }
  player_stop();
  */
  player_play(50);
}
