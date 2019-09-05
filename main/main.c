/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ESP_INTR_FLAG_DEFAULT 0

#include "play.h"

void app_main()
{
  printf("Main running on core : %d\n", xPortGetCoreID());
  init_play(1);
  play(24);
}
