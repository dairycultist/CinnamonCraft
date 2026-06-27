#ifndef IO_H
#define IO_H

typedef struct {

    int camera_dx, camera_dy;

    int left, right, forward, backward, up, down;
    int attack, use; // maybe change to primary and secondary

} Input;

typedef struct {

	float x;
	float y;
	float z;
	float pitch;
	float yaw;

} Transform;

typedef void *Texture;
typedef void *Mesh;

void initialize_io();
int io_keep_program_alive();
void io_populate_input(Input *input);
void io_present();

Texture load_texture(const char *path);
// TODO free_texture

// 3D
Mesh create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture texture);
Mesh create_mesh_from_obj(const char *obj_path, Texture texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh mesh);
void remesh_mesh(Mesh mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);

// sky
Mesh create_sky_mesh();
void draw_sky_mesh(const Transform *camera, const Mesh mesh);

// 2D
Mesh create_sprite_mesh(float u, float v, float h, Texture texture);
void draw_sprite_mesh(const Mesh mesh);

// both 2D and 3D
void mesh_set_texture(Mesh mesh, Texture texture);
void free_mesh(Mesh mesh);

void initialize_renderer();

#endif