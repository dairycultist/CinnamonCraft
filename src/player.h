#ifndef PLAYER_H
#define PLAYER_H

// basically just conveniently groups together player code (drawing their perspective, handling their input, etc)

#include "io.h"

void initialize_player();
void player_process_tick(Input *input);

void set_player_position(float x, float y, float z);
void set_player_rotation(float yaw, float pitch);

// parameters can be ignored by setting them to NULL
void get_player_information(float *x, float *y, float *z, float *camera_y, float *yaw, float *pitch, int *grounded_out);

#endif