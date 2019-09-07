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

#include "player.h"

void check_potentiometer() {
	int position, prev_position;

	adc1_config_width(ADC_WIDTH_BIT_9); //minimum resolution
  	adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11); //Largest scale : 3.9V

	prev_position = 0;
	do {
		position = adc1_get_raw(ADC1_CHANNEL_6) * CONFIG_STEP_LENGTH / (1<<9);
		if(position != prev_position) {
			//player_position(position);
			printf("Pos : %d\n", position);
			prev_position = position;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	} while(1);
}

void app_main()
{
  printf("Main running on core : %d\n", xPortGetCoreID());

  player_init((BaseType_t) 1);
  xTaskCreatePinnedToCore(check_potentiometer, "potentiometer_task", 2048, NULL, 10, NULL, 0);

  for(int i=12;i<72;i++) {
  	player_play(i);
  	vTaskDelay(250 / portTICK_PERIOD_MS);
  }
  player_stop();
}
