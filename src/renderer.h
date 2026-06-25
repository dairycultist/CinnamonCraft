#ifndef RENDERER_H
#define RENDERER_H

typedef struct {

	float x;
	float y;
	float z;
	float pitch;
	float yaw;

} Transform;

typedef struct {

	int w, h;
	GLuint texture;

} Texture;

typedef struct {

	GLuint vertex_array; // "VAO"
	uint vertex_count;
	Texture *texture;

} Mesh;

Texture *load_texture(const char *path);

// 3D
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture *texture);
Mesh *create_mesh_from_obj(const char *obj_path, Texture *texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh);
void remesh_mesh(Mesh *mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);

// sky
Mesh *create_sky_mesh();
void draw_sky_mesh(const Transform *camera, const Mesh *mesh);

// 2D
Mesh *create_sprite_mesh(float u, float v, float h, Texture *texture);
void draw_sprite_mesh(const Mesh *mesh);

// both 2D and 3D
void mesh_set_texture(Mesh *mesh, Texture *texture);
void free_mesh(Mesh *mesh);

void initialize_renderer();

#endif