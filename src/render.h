#ifndef RENDER_H
#define RENDER_H

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
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, const Texture *texture);
Mesh *create_mesh_from_obj(const char *obj_path, const Texture *texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh *mesh);

void remesh_mesh(Mesh *mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);
void retexture_mesh(Mesh *mesh, const Texture *texture); // rename to mesh_bind_texture

// 2D
Mesh *create_sprite_mesh(float u, float v, float h, const Texture *texture);
void draw_sprite_mesh(const Mesh *mesh);

// both 2D and 3D
void free_mesh(Mesh *mesh);

void initialize_shaders();

#endif