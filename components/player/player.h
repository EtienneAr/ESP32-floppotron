#ifndef PLAY_H
#define PLAY_H

#define PLAYER_MAX_POSITION CONFIG_STEP_LENGTH

void player_init(BaseType_t core);
void player_play(int note, int mask);
void player_stop(int note, int mask);
void player_set_position(int drive, int position);

#endif