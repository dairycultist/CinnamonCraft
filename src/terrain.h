#ifndef TERRAIN_H
#define TERRAIN_H

// exposes terrain manipulation (i.e. the blocks) as globally positioned and chunk-agnostic
// abstracts away chunks and the internal format of block types

#include "io.h"
#include "entities.h"

#define BLOCK_AIR 0
#define BLOCK_GRASS 1
#define BLOCK_STONE 2

// if this value is larger than the narrowest block collider, there will be significant gaps in AABB colliders
// if this value is too small, it will impact performance
#define AABB_COLLISION_DS 0.2

// only 256 block types can exist
typedef unsigned char block_t;

void initialize_terrain();
void draw_chunks(const Transform *camera);

// chunk creation/deletion
void create_chunk_at(int chunk_x, int chunk_z);
void destroy_chunk_at(int chunk_x, int chunk_z);

// block info
block_t get_block_at(int x, int y, int z);
unsigned short get_block_ticks_to_break(block_t block);

// block setting/remeshing
void set_block_at(int x, int y, int z, block_t block);
void set_delay_remesh_block_at(int x, int y, int z, block_t block);
void remesh_delayed_chunks();

// collision
int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(AABB *aabb);
int would_aabb_intersect_block_at(int x, int y, int z, block_t block, AABB *aabb);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif