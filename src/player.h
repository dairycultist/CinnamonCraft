#ifndef PLAYER_H
#define PLAYER_H

// basically just conveniently groups together player code (drawing their perspective, handling their input, etc)

#include "io.h"

void initialize_player();
void player_process_tick(Input *input);

#endif