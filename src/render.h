#ifndef RENDER_H
#define RENDER_H

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

// 3D
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, const Texture *texture);
Mesh *create_mesh_from_obj(const char *obj_path, const Texture *texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh);
void remesh_mesh(Mesh *mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);
void retexture_mesh(Mesh *mesh, const Texture *texture);

// 2D
Mesh *create_sprite_mesh(float u, float v, float w, float h, const Texture *texture); // TODO only pass w, have h be dynamically calculated (including after window resize)
void draw_sprite_mesh(const Mesh *mesh);

// both 2D and 3D
void free_mesh(Mesh *mesh);

void initialize_shaders();
void initialize_perspective(const float aspectRatio);

#endif