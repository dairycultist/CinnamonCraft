#ifndef IO_H
#define IO_H

// exposes helpers for receiving player input, model manipulation and rendering, etc

// abstracts away all that hardware-specific stuff (graphical context/renderer, the
// file system, what physical inputs map to which game inputs, etc)

// if you want to port this game to something that doesn't support SDL/OpenGL, or
// otherwise handles rendering, input, file loading, etc differently, you should
// only have to reimplement io.c

typedef struct {

    int camera_dx, camera_dy;

    int left, right, forward, backward, up, down;
    int attack, use; // maybe change to primary and secondary

} Input;

typedef struct {

	float x, y, z;
	float pitch, yaw;

} Transform;

typedef void *Texture;
typedef void *Mesh;

void initialize_io();

int game_is_running();
void populate_input(Input *input);
void present();

// textures
Texture load_texture(const char *path);
void free_texture(Texture texture);

// 3D meshes
Mesh create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture texture);
Mesh create_mesh_from_obj(const char *obj_path, Texture texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh mesh);
void remesh_mesh(Mesh mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);

// sky meshes
Mesh create_sky_mesh();
void draw_sky_mesh(const Transform *camera, const Mesh mesh);

// 2D meshes
// u/v [-1, 1] are the screen UV where the anchor should be aligned
// anchor_u/anchor_v [0, 1] are the sprite UV of the anchor
// h_pixels is height in pixels (width is calculated automatically)
Mesh create_sprite_mesh(float u, float v, float anchor_u, float anchor_v, int h_pixels, Texture texture);
void draw_sprite_mesh(const Mesh mesh);

// misc
void mesh_set_texture(Mesh mesh, Texture texture);
void free_mesh(Mesh mesh);

#endif