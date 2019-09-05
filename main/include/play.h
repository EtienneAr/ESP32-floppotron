#ifndef PLAY_H
#define PLAY_H

#define STEP_GPIO 13
#define DIR_GPIO 27
#define INPUT_GPIO 25

static int is_playing;
static int current_note;
static int current_position;
static long current_period;
static long next_time_us = 0;
static bool current_pin_state = false;
static bool current_dir;

void play_task(void* arg);

void init_play(BaseType_t core);
void play(int note);
void stop();
void change_dir();

#endif