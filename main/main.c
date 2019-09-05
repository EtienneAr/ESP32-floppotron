/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "player.h"

void app_main()
{
  printf("Main running on core : %d\n", xPortGetCoreID());
  player_init(1);
  for(int i=12;i<72;i++) {
  	player_play(i);
  	vTaskDelay(250 / portTICK_PERIOD_MS);
  }
  player_stop();
}
