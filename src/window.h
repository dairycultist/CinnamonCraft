#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TRUE 1
#define FALSE 0

// https://pixelbrush.dev/beta-wiki/networking/packets/
#define pid_KeepAlive 0x00
#define pid_Login 0x01
#define pid_PreLogin 0x02

void send_pid(unsigned char pid);
void send_string16(const char *string);
void send_integer(int value);
void send_long(long value);
void send_byte(char value);

unsigned char read_pid();

#endif