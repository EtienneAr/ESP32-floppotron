/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define DIR_GPIO 27
#define INPUT_GPIO 25

#define ESP_INTR_FLAG_DEFAULT 0

int n = 0;
int cnt = 0;
bool my_bool = false;

static void IRAM_ATTR isr_handler(void* arg) {
  my_bool = true;
  cnt++;
}

void app_main()
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_pad_select_gpio(DIR_GPIO);
    gpio_pad_select_gpio(INPUT_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIR_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(INPUT_GPIO, GPIO_MODE_INPUT);

    ESP_ERROR_CHECK(gpio_intr_enable(INPUT_GPIO));
    ESP_ERROR_CHECK(gpio_set_intr_type(INPUT_GPIO, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));
    ESP_ERROR_CHECK(gpio_isr_handler_add(INPUT_GPIO, isr_handler, NULL));
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(15 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(15 / portTICK_PERIOD_MS);
        if(++n%CONFIG_STEP_LENGTH == 0) gpio_set_level(DIR_GPIO, n%(2*CONFIG_STEP_LENGTH) == 0 ? 1 : 0);
        if(my_bool) {
          printf("Hello %d\n", cnt);
          my_bool = false;
        }
    }
}
