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

player_state_t current_state[3];

static gpio_num_t gpio_step[3] = {CONFIG_STEP_GPIO_A, CONFIG_STEP_GPIO_B, CONFIG_STEP_GPIO_C};
static gpio_num_t gpio_dir[3] = {CONFIG_DIR_GPIO_A,  CONFIG_DIR_GPIO_B,  CONFIG_DIR_GPIO_C};
	
static void play_task(void* arg);
static void position_floppy();

void player_init(BaseType_t core) {
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		gpio_pad_select_gpio(gpio_step[i]);
		gpio_pad_select_gpio(gpio_dir[i]);
		gpio_set_direction(gpio_step[i], GPIO_MODE_OUTPUT);
    	gpio_set_direction(gpio_dir[i], GPIO_MODE_OUTPUT);
	}

    position_floppy();

    xTaskCreatePinnedToCore(play_task, "play_task", 2048, NULL, 10, NULL, core);
}

/* TODO : return which drive is playing */
void player_play(int note) {
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		if(current_state[i].isPlaying == false) {
			current_state[i].isPlaying = true;
			current_state[i].note = note;
			current_state[i].period = period(note);
			return;
		}
	}
}

void player_stop(int note) {
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		if(current_state[i].isPlaying == true && current_state[i].note == note) {
			current_state[i].isPlaying = false;
			return;
		}
	}
}

void player_set_position(int drive, int position) {
	current_state[drive].goalPosition = position;
}


void position_floppy() {
	int i;
	int j;
	for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_dir[j], 0); }
	
	for(i=0;i<CONFIG_STEP_LENGTH;i++) {
		for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_step[j], 1); }
		vTaskDelay(10 / portTICK_PERIOD_MS);
		for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_step[j], 0); }
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	
	for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_dir[j], 1); }
	
	for(i=0;i<CONFIG_STEP_LENGTH/2;i++) {
		for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_step[j], 1); }
		vTaskDelay(10 / portTICK_PERIOD_MS);
		for(j=0;j<CONFIG_DRIVE_NB;j++) { gpio_set_level(gpio_step[j], 0); }
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

	for(j=0;j<CONFIG_DRIVE_NB;j++) { 
		current_state[j].position = CONFIG_STEP_LENGTH/2;
		current_state[j].goalPosition = CONFIG_STEP_LENGTH/2;
		gpio_set_level(gpio_step[j], current_state[j].dir ? 1 : 0);
	}
}

void play_task(void* arg) {
	while(1) {
		for(int j=0;j<CONFIG_DRIVE_NB;j++) { 
			if(current_state[j].isPlaying) {
				if(current_state[j].nextTimeUS <= esp_timer_get_time()) {
					gpio_set_level(gpio_step[j], current_state[j].isPinHigh ? 0 : 1);
					current_state[j].nextTimeUS += current_state[j].period;
					current_state[j].isPinHigh = !current_state[j].isPinHigh;

					if(current_state[j].isPinHigh) {
						if( (current_state[j].dir == true && current_state[j].position > current_state[j].goalPosition)
						 || (current_state[j].dir == false && current_state[j].position <= current_state[j].goalPosition) ) {
							current_state[j].dir = !current_state[j].dir;
							gpio_set_level(gpio_dir[j], current_state[j].dir ? 1 : 0);
						}
					} else {
						current_state[j].position += current_state[j].dir ? 1 : -1;
						if(current_state[j].position < 0) current_state[j].position = 0;
						if(current_state[j].position > CONFIG_STEP_LENGTH) current_state[j].position = CONFIG_STEP_LENGTH; 
					}
				}
			}
		}
	}
}