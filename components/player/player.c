#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "pitch.h"
#include "player.h"

#define QUEUE_LEN_MAX 3

typedef struct player_state {
	volatile int  isPlaying;
	volatile int  note;
	volatile long period;
	volatile long nextTimeUS;
	volatile bool isPinHigh;
	volatile bool dir;
	volatile int  position;
	volatile int  goalPosition;

	/* TODO : Create special function to handle queue*/
	volatile int queue[QUEUE_LEN_MAX];
	volatile int queueLen;
} player_state_t;

volatile player_state_t current_state[CONFIG_DRIVE_NB];

static gpio_num_t gpio_step[3] = {CONFIG_STEP_GPIO_A, CONFIG_STEP_GPIO_B, CONFIG_STEP_GPIO_C};
static gpio_num_t gpio_dir[3] = {CONFIG_DIR_GPIO_A,  CONFIG_DIR_GPIO_B,  CONFIG_DIR_GPIO_C};
static gpio_num_t gpio_led[3] = {CONFIG_LED_GPIO_A,  CONFIG_LED_GPIO_B,  CONFIG_LED_GPIO_C};
	
static void play_task(void* arg);
static void position_floppy();
static void player_updateLeds();

void player_init(BaseType_t core) {
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		gpio_pad_select_gpio(gpio_step[i]);
		gpio_pad_select_gpio(gpio_dir[i]);
		gpio_pad_select_gpio(gpio_led[i]);
		gpio_set_direction(gpio_step[i], GPIO_MODE_OUTPUT);
    	gpio_set_direction(gpio_dir[i], GPIO_MODE_OUTPUT);
    	gpio_set_direction(gpio_led[i], GPIO_MODE_OUTPUT);

    	gpio_set_level(gpio_led[i], 1);

    	current_state[i].queueLen = 0;
	}

    position_floppy();

    player_updateLeds();

    xTaskCreatePinnedToCore(play_task, "play_task", 2048, NULL, 10, NULL, core);
}

void player_updateLeds() {
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		gpio_set_level(gpio_led[i], current_state[i].isPlaying ? 1 : 0);
	}
}

void player_play(int note, int mask) {
	int smallestQueue = QUEUE_LEN_MAX;
	int smallestQueue_i = -1;

	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		if((mask&(1<<i)) != 0) {
			//If no note is playing, play on this one !
			if(current_state[i].isPlaying == false) {
				smallestQueue = 0;
				smallestQueue_i = i;
				break;
			}
			//Otherwise, compare to find the less busy drive
			if(current_state[i].queueLen < smallestQueue) {
				smallestQueue_i = i;
				smallestQueue = current_state[i].queueLen;
			}
		}
	}
	if(smallestQueue < QUEUE_LEN_MAX) {
		if(current_state[smallestQueue_i].isPlaying) {
			//if the drive already plays, add the current note to queue
			for(int i=current_state[smallestQueue_i].queueLen;i>0;i--) {
				current_state[smallestQueue_i].queue[i] = current_state[smallestQueue_i].queue[i-1];
			}
			current_state[smallestQueue_i].queue[0] = current_state[smallestQueue_i].note;
			current_state[smallestQueue_i].queueLen++;
		} else {
			//The drive was not playing already
			current_state[smallestQueue_i].nextTimeUS = -1; //To tell the drive to start playing immediately
		}
		//Play the new note
		current_state[smallestQueue_i].isPlaying = true;
		current_state[smallestQueue_i].note = note;
		current_state[smallestQueue_i].period = period(note);
	}

	player_updateLeds();
}

void player_stop(int note, int mask) {
	//Special case : if the note is 0, kill the channel
	if(note == 0) {
		for(int i=0;i<CONFIG_DRIVE_NB;i++) {
			if((mask&(1<<i)) != 0) {
				//Reset the queue
				current_state[i].queueLen = 0;
				//Stop the drive
				current_state[i].isPlaying = false;
			}
		}
	}


	//First check if the note to stop is currently played
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		if((mask&(1<<i)) != 0) {
			if(current_state[i].isPlaying == true) {
				if(current_state[i].note == note) {
					//Check if there is another note to play in the queue
					if(current_state[i].queueLen > 0) {
						//Pop the queue
						current_state[i].note = current_state[i].queue[0];
						current_state[i].period = period(current_state[i].note);
						current_state[i].queueLen--;
						for(int j=0;j<current_state[i].queueLen;j++) {
							current_state[i].queue[j] = current_state[i].queue[j+1];
						}
					} else {
						//If the queue is empty, then stops the drive
						current_state[i].isPlaying = false;
					}
					player_updateLeds();
					return;
				}
			}
		}
	}

	//If nothing found, search in queues
	for(int i=0;i<CONFIG_DRIVE_NB;i++) {
		if((mask&(1<<i)) != 0) {
			int isNoteFound = false;
			for(int j=0;j<current_state[i].queueLen;j++) {
				if(current_state[i].queue[j] == note) {
					current_state[i].queueLen--;
					isNoteFound = true;
				}
				if(isNoteFound) {
					current_state[i].queue[j] = current_state[i].queue[j+1];
				}
			}
		}
	}

	player_updateLeds();
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
	for( ;; ) {
		for(int j=0;j<CONFIG_DRIVE_NB;j++) { 
			if(current_state[j].isPlaying) {
				if(current_state[j].nextTimeUS < 0 || current_state[j].nextTimeUS <= esp_timer_get_time()) {
					gpio_set_level(gpio_step[j], current_state[j].isPinHigh ? 0 : 1);
					
					if(current_state[j].nextTimeUS < 0) current_state[j].nextTimeUS = esp_timer_get_time();
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