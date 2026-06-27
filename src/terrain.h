#ifndef TERRAIN_H
#define TERRAIN_H

// exposes terrain manipulation (i.e. the blocks)
// abstracts away chunks and the internal format of blocks

#include "io.h"

typedef unsigned char block_t;

// if this value is larger than the narrowest block collider, there will be significant gaps in AABB colliders
// if this value is too small, it will impact performance
#define AABB_COLLISION_DS 0.2

void initialize_terrain();
void draw_chunks(const Transform *camera);

// the following functions are globally positioned and chunk-agnostic

// block manipulation
block_t get_block_at(int x, int y, int z);
void set_block_at(int x, int y, int z, block_t block);
int is_block_fullblock(block_t block);
unsigned char get_block_atlas_index(block_t block, int i);
unsigned short get_block_ticks_to_break(block_t block);

// remeshing
void set_delay_remesh_block_at(int x, int y, int z, block_t block);
void remesh_delayed_chunks();

// collision
int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif