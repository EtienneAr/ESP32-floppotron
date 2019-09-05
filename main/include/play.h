#ifndef PLAY_H
#define PLAY_H

#define STEP_GPIO 13
#define DIR_GPIO 27

void init_player(BaseType_t core);
void play(int note);
void stop();

#endif