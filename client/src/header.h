#ifndef HEADER
#define HEADER

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define TRUE 1
#define FALSE 0

/*
 * logic.c
 */

void on_start();
void on_terminate();

void process_tick();
void process_event(SDL_Event event);

/*
 * mesh.c
 */

typedef struct {

	float x;
	float y;
	float z;
	float pitch;
	// no one needs roll
	float yaw;

} Transform;

typedef struct {

	GLuint vertex_array; // "VAO"
	uint vertex_count;
	GLuint texture;

} Mesh;

typedef struct {

	SDL_Surface *surface;

} Texture;

Texture *load_texture(const char *path);
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, const Texture *texture);
Mesh *create_mesh_from_obj(const char *obj_path, const Texture *texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh);
// destroy_mesh when

void initialize_shader();
void initialize_perspective(const float aspectRatio);

/*
 * ez_array.c
 */
typedef struct {

	unsigned char *data;
	int datasize;
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length);

#endif