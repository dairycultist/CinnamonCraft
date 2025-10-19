#ifndef ENGINE_H
#define ENGINE_H

/*
 * mesh stuff
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
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh);
// destroy_mesh when

void initialize_shader();
void initialize_perspective(const float aspectRatio);

/*
 * chunk stuff
 */
typedef struct {

	Mesh mesh;
	unsigned char blocks[16][16][16]; // array of bytes indexing into block_types
	
	// int chunk_x;
	// int chunk_y;
	// int chunk_z;

} Chunk;

typedef struct {

	unsigned char mesh_type; // 0:empty,1:cube,...slope?
	unsigned char tex_top;
	unsigned char tex_side;
	unsigned char tex_bottom;

} BlockType;

void remesh_chunk(const Chunk *chunk, const Texture *blocksheet_texture);
int is_point_inside_chunk(const Chunk *chunk, float x, float y, float z);
int is_aabb_inside_chunk(const Chunk *chunk, float x, float y, float z, float size);

#endif