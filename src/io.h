#ifndef IO_H
#define IO_H

typedef struct {

    int camera_dx, camera_dy;

    int left, right, forward, backward, up, down;
    int attack, use; // maybe change to primary and secondary

} Input;

void initialize_io();
int io_keep_program_alive();
void io_populate_input(Input *input);
void io_present();

#endif