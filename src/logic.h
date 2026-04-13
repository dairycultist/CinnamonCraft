#ifndef LOGIC_H
#define LOGIC_H

void on_start();
void on_terminate();

void process_tick(Sint32 mouse_dx, Sint32 mouse_dy);
void process_event(SDL_Event event);

#endif