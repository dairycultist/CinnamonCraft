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
 * chunk.c
 */

typedef struct {

	Mesh mesh;
	unsigned char blocks[16][16][16]; // array of bytes indexing into block_types
	
	// int chunk_x;
	// int chunk_y;
	// int chunk_z;

} Chunk;

#define BT_IsSolid(block_type) (block_type.flags & 0b00000001)

typedef struct {

	// right now flags is just the first bit representing "solid," which means adjacent blocks will cull the faces that touch it
	unsigned char flags;
	unsigned char tex_top;
	unsigned char tex_side;
	unsigned char tex_bottom;

} BlockType;

void register_block_type(BlockType block_type);
void remesh_chunk(const Chunk *chunk, const Texture *blocksheet_texture);
int is_point_inside_chunk(const Chunk *chunk, float x, float y, float z);
int is_aabb_inside_chunk(const Chunk *chunk, float x, float y, float z, float wl, float h);

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