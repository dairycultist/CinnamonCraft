#ifndef TERRAIN_H
#define TERRAIN_H

// exposes terrain manipulation (i.e. the blocks)
// abstracts away chunks

#include "io.h"
#include "ez_array.h"

// if it's larger than the narrowest block collider, there will be significant gaps in AABB colliders
// if it's too small, it will impact performance
#define AABB_COLLISION_DS 0.2

// only 256 texture indices and 256 block types can exist

#define BT_IS_FULLBLOCK(block_type) ((block_type).flags & 0b00000001)
#define BT_IS_COLLIDABLE(block_type) ((block_type).flags & 0b00000010)

typedef struct {

	// this function determines what mesh/UV a block gets (including face culling)
	void (*append_block_to_mesh)(EZArray *mesh_data, int *vertex_count, int x, int y, int z);

	// dx, dy, and dz can be assumed to ALWAYS have the range [0, 1]
	int (*does_local_point_collide)(float dx, float dy, float dz);

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

// the following functions are globally positioned and chunk-agnostic

// block manipulation
unsigned char get_block_at(int x, int y, int z);
void set_block_at(int x, int y, int z, unsigned char block);

// remeshing
void set_delay_remesh_block_at(int x, int y, int z, unsigned char block);
void remesh_delayed_chunks();

// collision
int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif