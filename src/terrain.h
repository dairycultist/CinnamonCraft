#ifndef TERRAIN_H
#define TERRAIN_H

// exposes terrain manipulation (i.e. the blocks) as globally positioned and chunk-agnostic
// abstracts away chunks and the internal format of block types

#include "io.h"
#include "entities.h"

// if this value is larger than the narrowest block collider, there will be significant gaps in AABB colliders
// if this value is too small, it will impact performance
#define AABB_COLLISION_DS 0.2

// only 256 texture indices and 256 block types can exist
typedef unsigned char block_t;
typedef unsigned char atlas_index_t;

void initialize_terrain();
void draw_chunks(const Transform *camera);

// block manipulation
block_t get_block_at(int x, int y, int z);
void set_block_at(int x, int y, int z, block_t block);
int is_block_fullblock(block_t block);
atlas_index_t get_block_atlas_index(block_t block, int i);
unsigned short get_block_ticks_to_break(block_t block);

// remeshing
void set_delay_remesh_block_at(int x, int y, int z, block_t block);
void remesh_delayed_chunks();

// collision
int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(AABB *aabb);
int would_aabb_intersect_block_at(int x, int y, int z, block_t block, AABB *aabb);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif