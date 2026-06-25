#ifndef PLAYER_H
#define PLAYER_H

void initialize_player();
void player_process_tick(Sint32 mouse_dx, Sint32 mouse_dy, int left, int right, int forward, int backward, int up, int down, int attack, int use);

#endif