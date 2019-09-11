#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "pitch.h"
#include "player.h"

typedef struct player_state {
	int  isPlaying;
	int  note;
	long period;
	long nextTimeUS;
	bool isPinHigh;
	bool dir;
	int  position;
	int  goalPosition;
} player_state_t;

player_state_t current_state;

static void play_task(void* arg);
static void position_floppy();

void player_init(BaseType_t core) {
	gpio_pad_select_gpio(CONFIG_STEP_GPIO_A);
    gpio_pad_select_gpio(CONFIG_DIR_GPIO_A);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(CONFIG_STEP_GPIO_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONFIG_DIR_GPIO_A, GPIO_MODE_OUTPUT);

    position_floppy();

    xTaskCreatePinnedToCore(play_task, "play_task", 2048, NULL, 10, NULL, core);
}

void player_play(int note) {
	current_state.isPlaying = true;
	current_state.note = note;
	current_state.period = period(note);
}

void player_stop() {
	current_state.isPlaying = false;
}

void player_set_position(int position) {
	current_state.goalPosition = position;
}

static inline void change_dir() {
	
	current_state.dir = !current_state.dir;
}

void position_floppy() {
	int i;
	gpio_set_level(CONFIG_DIR_GPIO_A, 0);
	for(i=0;i<CONFIG_STEP_LENGTH;i++) {
		gpio_set_level(CONFIG_STEP_GPIO_A, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_STEP_GPIO_A, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	gpio_set_level(CONFIG_DIR_GPIO_A, 1);
	for(i=0;i<CONFIG_STEP_LENGTH/2;i++) {
		gpio_set_level(CONFIG_STEP_GPIO_A, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(CONFIG_STEP_GPIO_A, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	current_state.position = CONFIG_STEP_LENGTH/2;
	current_state.goalPosition = CONFIG_STEP_LENGTH/2;
	gpio_set_level(CONFIG_DIR_GPIO_A, current_state.dir ? 1 : 0);
}

void play_task(void* arg) {
	while(1) {
		if(current_state.isPlaying) {
			if(current_state.nextTimeUS <= esp_timer_get_time()) {
				gpio_set_level(CONFIG_STEP_GPIO_A, current_state.isPinHigh ? 0 : 1);
				current_state.nextTimeUS += current_state.period;
				current_state.isPinHigh = !current_state.isPinHigh;

				if(current_state.isPinHigh) {
					if( (current_state.dir == true && current_state.position > current_state.goalPosition)
					 || (current_state.dir == false && current_state.position <= current_state.goalPosition) ) {
						current_state.dir = !current_state.dir;
						gpio_set_level(CONFIG_DIR_GPIO_A, current_state.dir ? 1 : 0);
					}
				} else {
					current_state.position += current_state.dir ? 1 : -1;
					if(current_state.position < 0) current_state.position = 0;
					if(current_state.position > CONFIG_STEP_LENGTH) current_state.position = CONFIG_STEP_LENGTH; 
				}
			}
		}
	}
}