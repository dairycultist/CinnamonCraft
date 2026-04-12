/*
 * Abstraction layer for terrain. Outside files (ideally) won't have to know about chunks at all.
 */

#include "header.h"
#include "terrain.h"

#include <math.h>

typedef struct { // remember, only this file can access this struct

	Mesh mesh;
	unsigned char blocks[16][128][16]; // array of bytes indexing into block_types

	int chunk_x, chunk_z;

} Chunk;

static Texture *blockmap_texture;

static Chunk *chunks[1][1]; // finite for now

static EZArray delayed_remesh_chunks; // when you want to set a bunch of blocks, remeshing after each is slow and redundant, so you save them to remesh once at the end

static void (*populator)(int x, int y, int z);

// block type registry
static BlockType block_types[256] = { (BlockType) { NULL, 0b00000000, 0, 0, 0, 0 } }; // first block is always air
static unsigned char block_type_count = 1;

void register_block_type(BlockType block_type) {

	block_types[block_type_count++] = block_type;
}

BlockType *get_block_type(unsigned char id) {

	return &block_types[id];
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
	remesh_mesh(&chunk->mesh, mesh_data.data, mesh_data.bytecount, vertex_count);
}

void initialize_chunk_system(void (*chunk_populator)(int x, int y, int z)) {

	populator = chunk_populator;

	blockmap_texture = load_texture("res/blockmap.png");
	
	for (int chunk_x = 0; chunk_x < 1; chunk_x++)
		for (int chunk_z = 0; chunk_z < 1; chunk_z++) {

			chunks[chunk_x][chunk_z] = malloc(sizeof(Chunk));
			
			retexture_mesh(&chunks[chunk_x][chunk_z]->mesh, blockmap_texture);

			chunks[chunk_x][chunk_z]->chunk_x = chunk_x;
			chunks[chunk_x][chunk_z]->chunk_z = chunk_z;
		}
	
	// populate chunks (finite)
	for (int chunk_x = 0; chunk_x < 1; chunk_x++) {
		for (int chunk_z = 0; chunk_z < 1; chunk_z++) {

			// populate single chunk
			for (int x = 0; x < 16; x++) {
				for (int y = 0; y < 128; y++) {
					for (int z = 0; z < 16; z++) {

						populator(
							x + chunk_x * 16,
							y,
							z + chunk_z * 16
						);
					}
				}
			}

			// remesh the chunk
			remesh_delayed_chunks();
		}
	}
}

void draw_chunks(const Transform *camera) {

	int chunk_x, chunk_z;
	Transform chunk_transform = {};

	for (chunk_x = 0; chunk_x < 1; chunk_x++) {
		for (chunk_z = 0; chunk_z < 1; chunk_z++) {

			chunk_transform.x = chunk_x * 16;
			chunk_transform.y = 0.0;
			chunk_transform.z = chunk_z * 16;

			draw_mesh(camera, &chunk_transform, &chunks[chunk_x][chunk_z]->mesh);
		}
	}
}

static Chunk *get_chunk_of_block(int x, int y, int z) { // we include y so that OOB y will return NULL

	if (
		x < 0 ||
		y < 0 ||
		z < 0 ||
		x >= 1 * 16 ||
		y >= 128 ||
		z >= 1 * 16
	)
		return NULL;
	
	return chunks[x / 16][z / 16];
}

/*
 * The following functions (from chunk.h!) are globally positioned!
 * No need for other files to consider where chunk boundaries are.
 */

// TODO int is_chunk_loaded(int cx, int cy, int cz)

unsigned char get_block_at(int x, int y, int z) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return 0;
	
	return chunk->blocks[x % 16][y][z % 16];
}

void set_block_at(int x, int y, int z, unsigned char block) {
	
	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % 16][y][z % 16] = block;

	remesh_chunk(chunk);
}

void set_delay_remesh_block_at(int x, int y, int z, unsigned char block) {

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

	return BT_IS_COLLIDABLE(block_types[get_block_at((int) x, (int) y, (int) z)]);
}

// the AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h) {

	wl /= 2;

	for (int block_x = floor(x - wl); block_x <= floor(x + wl); block_x++) {

		for (int block_z = floor(z - wl); block_z <= floor(z + wl); block_z++) {

			for (int block_y = floor(y); block_y <= floor(y + h); block_y++) {

				if (does_point_intersect_blocks(block_x, block_y, block_z))
					return TRUE;
			}
		}
	}

	return FALSE;
}

// returns TRUE if it hit a block, in which case it populates the output parameters with the position of the block
int raycast_blocks(const Transform *origin, float max_dist, int bool_surface, int *out_x, int *out_y, int *out_z) {

	#define STEP_SIZE 0.1

	float x = origin->x, y = origin->y, z = origin->z;
	float dx, dy, dz;

	dx = STEP_SIZE *  sin(origin->yaw) * cos(origin->pitch);
	dz = STEP_SIZE * -cos(origin->yaw) * cos(origin->pitch);

	dy = STEP_SIZE * -sin(origin->pitch);

	for (float dist = 0.0; dist < max_dist; dist += STEP_SIZE) {
		
		if (does_point_intersect_blocks(x, y, z)) {

			if (bool_surface) {

				x -= dx;
				y -= dy;
				z -= dz;
			}

			*out_x = floor(x);
			*out_y = floor(y);
			*out_z = floor(z);

			return TRUE;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	return FALSE;
}