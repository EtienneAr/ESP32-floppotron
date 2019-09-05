#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "pitch.h"
#include "player.h"

static int is_playing;
static int current_note;
static long current_period;
static long next_time_us = 0;
static bool current_pin_state = false;
static bool current_dir;

static void play_task(void* arg);
static void position_floppy();

void player_init(BaseType_t core) {
	gpio_pad_select_gpio(CONFIG_STEP_GPIO);
    gpio_pad_select_gpio(CONFIG_DIR_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(CONFIG_STEP_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_DIR_GPIO, GPIO_MODE_OUTPUT);

    position_floppy();

    xTaskCreatePinnedToCore(play_task, "play_task", 2048, NULL, 10, NULL, core);
}

void player_play(int note) {
	is_playing = true;
	current_note = note;
	current_period = period(note);
}

void player_stop() {
	is_playing = false;
}

static inline void change_dir() {
	gpio_set_level(CONFIG_DIR_GPIO, current_dir ? 1 : 0);
	current_dir = !current_dir;
}

void position_floppy() {
	int i;
	for(i=0;i<CONFIG_STEP_LENGTH;i++) {
		gpio_set_level(CONFIG_STEP_GPIO, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_STEP_GPIO, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	change_dir();
	for(i=0;i<CONFIG_STEP_LENGTH/2;i++) {
		gpio_set_level(CONFIG_STEP_GPIO, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_STEP_GPIO, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void play_task(void* arg) {
	while(1) {
		if(is_playing) {
			if(next_time_us <= esp_timer_get_time()) {
				gpio_set_level(CONFIG_STEP_GPIO, current_pin_state ? 1 : 0);
				next_time_us += current_period;
				current_pin_state = !current_pin_state;
				if(current_pin_state) {
					change_dir();
				}
			}
		}
	}
}