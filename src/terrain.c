#include "main.h"
#include "terrain.h"
#include "ez_array.h"
#include "append_block_to_mesh.h"

#include <stdlib.h>
#include <math.h>

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
	atlas_index_t atlas_indices[4]; // how these are actually rendered onto the block is determined by append_block_to_mesh

} BlockType;

typedef struct { // only this file knows Chunks even exist

	Mesh mesh;
	block_t blocks[16][128][16]; // array of bytes indexing into block_types

	int chunk_x, chunk_z;

} Chunk;

static Texture blockmap_texture;

static Chunk *chunks[WORLD_SIZE_IN_CHUNKS * WORLD_SIZE_IN_CHUNKS];

static EZArray delayed_remesh_chunks; // when you want to set a bunch of blocks, remeshing after each is slow and redundant, so you save them to remesh once at the end

static int full_block_collider(float dx, float dy, float dz) {

	return 1;
}

// block type registry
static BlockType block_types[256] = {
	(BlockType) { NULL, NULL, 0b00000000 },											// air
	(BlockType) { ABTM_grass, full_block_collider, 0b00000011, 60, { 0, 1, 2 } },	// grass
	(BlockType) { ABTM_block, full_block_collider, 0b00000011, 20, { 3, 3, 3 } }	// stone
};

static void populator(int x, int y, int z) {

	set_delay_remesh_block_at(
		x, y, z,
		sin(x * 0.1) * 16 + 16 < y ? 0 : (y < 20 ? 2 : 1)
	);
}

// remeshes based on the chunk's internal blocks
static void remesh_chunk(Chunk *chunk) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 128; y++)
			for (int z = 0; z < 16; z++)

				// add mesh of this block (given it has a meshing function)
				if (block_types[chunk->blocks[x][y][z]].append_block_to_mesh != NULL)
					block_types[chunk->blocks[x][y][z]].append_block_to_mesh(
						&mesh_data,
						&vertex_count,
						x + chunk->chunk_x * 16,
						y,
						z + chunk->chunk_z * 16
					);

	// create mesh
	remesh_mesh(chunk->mesh, mesh_data.data, mesh_data.bytecount, vertex_count);
}

void initialize_terrain() {

	blockmap_texture = load_texture("res/blockmap.png");
	
	// initialize chunks
	for (int chunk_x = 0; chunk_x < WORLD_SIZE_IN_CHUNKS; chunk_x++) {
		for (int chunk_z = 0; chunk_z < WORLD_SIZE_IN_CHUNKS; chunk_z++) {

			int i = chunk_x + WORLD_SIZE_IN_CHUNKS * chunk_z;

			chunks[i] = malloc(sizeof(Chunk));
			chunks[i]->mesh = create_mesh(NULL, 0, 0, blockmap_texture);

			chunks[i]->chunk_x = chunk_x;
			chunks[i]->chunk_z = chunk_z;
		}
	}
	
	// populate chunks (finite)
	for (int i = 0; i < WORLD_SIZE_IN_CHUNKS * WORLD_SIZE_IN_CHUNKS; i++) {

		// populate single chunk
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 128; y++) {
				for (int z = 0; z < 16; z++) {

					populator(
						x + chunks[i]->chunk_x * 16,
						y,
						z + chunks[i]->chunk_z * 16
					);
				}
			}
		}
	}

	// remesh every chunk at once (can't do right after populating because it needs
	// its neighbor to be loaded to be able to remesh at its chunk boundary properly)
	remesh_delayed_chunks();
}

void draw_chunks(const Transform *camera) {

	int chunk_x, chunk_z;
	Transform chunk_transform = {};

	for (int i = 0; i < WORLD_SIZE_IN_CHUNKS * WORLD_SIZE_IN_CHUNKS; i++) {

		chunk_transform.x = chunks[i]->chunk_x * 16;
		chunk_transform.y = 0.0;
		chunk_transform.z = chunks[i]->chunk_z * 16;

		draw_mesh(camera, &chunk_transform, chunks[i]->mesh);
	}
}

static Chunk *get_chunk_of_block(int x, int y, int z) {

	if (y < 0 || y >= 128) // we include y so that OOB y will return NULL
		return NULL;
	
	for (int i = 0; i < WORLD_SIZE_IN_CHUNKS * WORLD_SIZE_IN_CHUNKS; i++) {

		if (chunks[i]->chunk_x == (int) floor(x / 16.0) && chunks[i]->chunk_z == (int) floor(z / 16.0))
			return chunks[i];
	}

	return NULL;
}

block_t get_block_at(int x, int y, int z) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return 0;
	
	return chunk->blocks[x % 16][y][z % 16];
}

void set_block_at(int x, int y, int z, block_t block) {
	
	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % 16][y][z % 16] = block;

	remesh_chunk(chunk);

	// if block was at the edge of a chunk, also remesh the adjacent chunk(s)
	if (x % 16 == 0) {
		chunk = get_chunk_of_block(x - 1, y, z);
		if (chunk)
			remesh_chunk(chunk);
	}
	if (x % 16 == 15) {
		chunk = get_chunk_of_block(x + 1, y, z);
		if (chunk)
			remesh_chunk(chunk);
	}
	if (z % 16 == 0) {
		chunk = get_chunk_of_block(x, y, z - 1);
		if (chunk)
			remesh_chunk(chunk);
	}
	if (z % 16 == 15) {
		chunk = get_chunk_of_block(x, y, z + 1);
		if (chunk)
			remesh_chunk(chunk);
	}
}

int is_block_fullblock(block_t block) {

	return BT_IS_FULLBLOCK(block_types[block]);
}

atlas_index_t get_block_atlas_index(block_t block, int i) {

	return block_types[block].atlas_indices[i];
}

unsigned short get_block_ticks_to_break(block_t block) {

	return block_types[block].ticks_to_break;
}

void set_delay_remesh_block_at(int x, int y, int z, block_t block) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % 16][y][z % 16] = block;

	// save chunk for delayed remeshing if it's not already saved
	if (!contains_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *)))
		append_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *));
}

void remesh_delayed_chunks() {

	Chunk **chunks = (Chunk **) delayed_remesh_chunks.data;

	for (int i = 0; i < delayed_remesh_chunks.bytecount / sizeof(Chunk *); i++)
		remesh_chunk(chunks[i]);

	clear_ezarray(&delayed_remesh_chunks);
}

int does_point_intersect_blocks(float x, float y, float z) {

	BlockType *block_type = &block_types[get_block_at(floor(x), floor(y), floor(z))];

	return BT_IS_COLLIDABLE(*block_type) && block_type->does_local_point_collide(
		fmod(fmod(x, 1.0) + 1.0, 1.0),
		fmod(fmod(y, 1.0) + 1.0, 1.0),
		fmod(fmod(z, 1.0) + 1.0, 1.0)
	);
}

// the AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h) {

	wl /= 2;

	for (float block_x = x - wl; block_x <= x + wl; block_x += AABB_COLLISION_DS) {
	for (float block_z = z - wl; block_z <= z + wl; block_z += AABB_COLLISION_DS) {
	for (float block_y = y;      block_y <= y + h;  block_y += AABB_COLLISION_DS) {

		if (does_point_intersect_blocks(block_x, block_y, block_z))
			return 1;
	}}}

	return 0;
}

int would_aabb_intersect_block_at(int x, int y, int z, block_t block, float aabb_x, float aabb_y, float aabb_z, float aabb_wl, float aabb_h) {

	BlockType *block_type = &block_types[block];

	if (!BT_IS_COLLIDABLE(*block_type))
		return 0;

	for (float block_x = fmax(x, aabb_x - aabb_wl); block_x <= fmin(x + 1, aabb_x + aabb_wl); block_x += AABB_COLLISION_DS) {
	for (float block_z = fmax(z, aabb_z - aabb_wl); block_z <= fmin(z + 1, aabb_z + aabb_wl); block_z += AABB_COLLISION_DS) {
	for (float block_y = fmax(y, aabb_y);           block_y <= fmin(y + 1, aabb_y + aabb_h);  block_y += AABB_COLLISION_DS) {

		if (block_type->does_local_point_collide(
				fmod(fmod(block_x, 1.0) + 1.0, 1.0),
				fmod(fmod(block_y, 1.0) + 1.0, 1.0),
				fmod(fmod(block_z, 1.0) + 1.0, 1.0)
			))
			return 1;
	}}}

	return 0;
}

// returns true if it hit a block, in which case it populates the output parameters with the position of the block
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z) {

	#define STEP_SIZE 0.1

	float x = origin->x, y = origin->y, z = origin->z;
	float dx, dy, dz;

	dx = STEP_SIZE *  sin(origin->yaw) * cos(origin->pitch);
	dz = STEP_SIZE * -cos(origin->yaw) * cos(origin->pitch);

	dy = STEP_SIZE * -sin(origin->pitch);

	for (float dist = 0.0; dist < max_dist; dist += STEP_SIZE) {
		
		if (does_point_intersect_blocks(x, y, z)) {

			if (return_surface) {

				x -= dx;
				y -= dy;
				z -= dz;
			}

			*out_x = floor(x);
			*out_y = floor(y);
			*out_z = floor(z);

			return 1;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	return 0;
}