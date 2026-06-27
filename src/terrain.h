#ifndef TERRAIN_H
#define TERRAIN_H

#include "io.h"
#include "ez_array.h"

// only 256 texture indices and 256 block types can exist

#define BT_IS_FULLBLOCK(block_type) ((block_type).flags & 0b00000001)
#define BT_IS_COLLIDABLE(block_type) ((block_type).flags & 0b00000010)

typedef struct {

	// this function determines what mesh/UV a block gets (including face culling)
	void (*append_block_to_mesh)(EZArray *mesh_data, int *vertex_count, int x, int y, int z);

	// TODO stuff like:
	// - mining level
	// - dropped item
	// - orientation (used by append_block_to_mesh)
	// - vanilla metadata

	// 1 "fullblock": adjacent blocks will cull the faces that touch it
	// 2 "collidable": has full-block collision
	// 4
	// 8
	// 16
	// 32
	// 64
	// 128
	unsigned char flags;
	unsigned short ticks_to_break;
	unsigned char tex_indices[4]; // how these are actually rendered onto the block is determined by append_block_to_mesh

} BlockType;

void initialize_terrain();
BlockType *get_block_type(unsigned char id);

void draw_chunks(const Transform *camera);

// The following functions are globally positioned. Other files needn't
// consider where chunk boundaries are or that chunks exist at all.

// TODO int is_chunk_loaded(int cx, int cy, int cz)

unsigned char get_block_at(int x, int y, int z);
void set_block_at(int x, int y, int z, unsigned char block);
void set_delay_remesh_block_at(int x, int y, int z, unsigned char block);
void remesh_delayed_chunks();

int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif