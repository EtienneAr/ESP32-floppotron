#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "pitch.h"
#include "play.h"

void init_play(BaseType_t core) {
	gpio_pad_select_gpio(STEP_GPIO);
    gpio_pad_select_gpio(DIR_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(STEP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIR_GPIO, GPIO_MODE_OUTPUT);

    xTaskCreatePinnedToCore(play_task, "play_task", 2048, NULL, 10, NULL, core);
}

void play(int note) {
	is_playing = true;
	current_note = note;
	current_period = period(note);
}

void stop() {
	is_playing = false;
}

void change_dir() {
	gpio_set_level(DIR_GPIO, current_dir ? 1 : 0);
	current_dir = !current_dir;
}

void play_task(void* arg) {
	while(1) {
		if(is_playing) {
			if(next_time_us <= esp_timer_get_time()) {
				gpio_set_level(STEP_GPIO, current_pin_state ? 1 : 0);
				next_time_us += current_period;
				current_pin_state = !current_pin_state;
			}
		}
	}
}