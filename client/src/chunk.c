#include "header.h"
#include "chunk.h"

#include <math.h>

typedef struct { // remember, only this file can access this struct

	Mesh mesh;
	unsigned char blocks[CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS]; // array of bytes indexing into block_types

} Chunk;

static BlockType block_types[256] = { 0b00000000, 0, 0, 0, 0 }; // first block is always air
static unsigned char block_type_count = 1;

static Texture *blockmap_texture;

static Chunk *chunks[WORLD_DIM_IN_CHUNKS][WORLD_DIM_IN_CHUNKS][WORLD_DIM_IN_CHUNKS]; // finite for now

static EZArray delayed_remesh_chunks; // when you want to set a bunch of blocks, remeshing after each is slow and redundant, so you save them to remesh once at the end

static void (*populator)(int x, int y, int z);

void register_block_type(BlockType block_type) {

	block_types[block_type_count++] = block_type;
}

// this function determines what mesh/UV a block gets (including face culling)
static void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, const unsigned char blocks[CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS], int block_x, int block_y, int block_z) {

	#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.
	#define BT_AT(x, y, z) (block_types[blocks[x][y][z]])

	if (blocks[block_x][block_y][block_z] == 0) { return; }

	BlockType *block_type = &BT_AT(block_x, block_y, block_z);

	float u_sml, v_sml, u_big, v_big;

	// get UV for sides
	GET_SPRITEMAP_UV(block_type->tex_side, u_sml, v_sml, u_big, v_big);

	// -x face
	if (block_x == 0 || !BT_IS_SOLID(BT_AT(block_x - 1, block_y, block_z))) {

		float full_block_data[] = {
			block_x, block_y, block_z,			-1, 0, 0,	u_big, v_sml,
			block_x, block_y, block_z + 1,		-1, 0, 0,	u_sml, v_sml,
			block_x, block_y + 1, block_z,		-1, 0, 0,	u_big, v_big,
			block_x, block_y + 1, block_z + 1,	-1, 0, 0,	u_sml, v_big,
			block_x, block_y + 1, block_z,		-1, 0, 0,	u_big, v_big,
			block_x, block_y, block_z + 1,		-1, 0, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +x face
	if (block_x == 15 || !BT_IS_SOLID(BT_AT(block_x + 1, block_y, block_z))) {

		float full_block_data[] = {
			block_x + 1, block_y, block_z,			1, 0, 0,	u_sml, v_sml,
			block_x + 1, block_y + 1, block_z,		1, 0, 0,	u_sml, v_big,
			block_x + 1, block_y, block_z + 1,		1, 0, 0,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z + 1,	1, 0, 0,	u_big, v_big,
			block_x + 1, block_y, block_z + 1,		1, 0, 0,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z,		1, 0, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -z face
	if (block_z == 0 || !BT_IS_SOLID(BT_AT(block_x, block_y, block_z - 1))) {

		float full_block_data[] = {
			block_x, block_y, block_z,			0, 0, -1,	u_sml, v_sml,
			block_x, block_y + 1, block_z,		0, 0, -1,	u_sml, v_big,
			block_x + 1, block_y, block_z,		0, 0, -1,	u_big, v_sml,
			block_x + 1, block_y + 1, block_z,	0, 0, -1,	u_big, v_big,
			block_x + 1, block_y, block_z,		0, 0, -1,	u_big, v_sml,
			block_x, block_y + 1, block_z,		0, 0, -1,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +z face
	if (block_z == 15 || !BT_IS_SOLID(BT_AT(block_x, block_y, block_z + 1))) {

		float full_block_data[] = {
			block_x, block_y, block_z + 1,			0, 0, 1,	u_big, v_sml,
			block_x + 1, block_y, block_z + 1,		0, 0, 1,	u_sml, v_sml,
			block_x, block_y + 1, block_z + 1,		0, 0, 1,	u_big, v_big,
			block_x + 1, block_y + 1, block_z + 1,	0, 0, 1,	u_sml, v_big,
			block_x, block_y + 1, block_z + 1,		0, 0, 1,	u_big, v_big,
			block_x + 1, block_y, block_z + 1,		0, 0, 1,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -y face
	if (block_y == 0 || !BT_IS_SOLID(BT_AT(block_x, block_y - 1, block_z))) {

		// get UV for bottom
		GET_SPRITEMAP_UV(block_type->tex_bottom, u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			block_x, block_y, block_z,			0, -1, 0,	u_sml, v_sml,
			block_x + 1, block_y, block_z,		0, -1, 0,	u_sml, v_big,
			block_x, block_y, block_z + 1,		0, -1, 0,	u_big, v_sml,
			block_x + 1, block_y, block_z + 1,	0, -1, 0,	u_big, v_big,
			block_x, block_y, block_z + 1,		0, -1, 0,	u_big, v_sml,
			block_x + 1, block_y, block_z,		0, -1, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +y face
	if (block_y == 15 || !BT_IS_SOLID(BT_AT(block_x, block_y + 1, block_z))) {

		// get UV for top
		GET_SPRITEMAP_UV(block_type->tex_top, u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			block_x, block_y + 1, block_z,			0, 1, 0,	u_big, v_sml,
			block_x, block_y + 1, block_z + 1,		0, 1, 0,	u_sml, v_sml,
			block_x + 1, block_y + 1, block_z,		0, 1, 0,	u_big, v_big,
			block_x + 1, block_y + 1, block_z + 1,	0, 1, 0,	u_sml, v_big,
			block_x + 1, block_y + 1, block_z,		0, 1, 0,	u_big, v_big,
			block_x, block_y + 1, block_z + 1,		0, 1, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}
}

// remeshes based on the chunk's internal blocks
// TODO should probably "free" chunk->mesh.vertex_array, in however way that works in OpenGL
static void remesh_chunk(const Chunk *chunk) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < CHUNK_DIM_IN_BLOCKS; x++)
		for (int y = 0; y < CHUNK_DIM_IN_BLOCKS; y++)
			for (int z = 0; z < CHUNK_DIM_IN_BLOCKS; z++)
				append_block_to_mesh(&mesh_data, &vertex_count, chunk->blocks, x, y, z);

	Mesh *mesh = create_mesh(mesh_data.data, mesh_data.bytecount, vertex_count, blockmap_texture);

	memcpy((void *) &chunk->mesh, mesh, sizeof(Mesh));
	free(mesh);
}

void initialize_chunk_system(void (*chunk_populator)(int x, int y, int z)) {

	populator = chunk_populator;

	blockmap_texture = load_texture("client/res/blockmap.png");
	
	for (int x = 0; x < WORLD_DIM_IN_CHUNKS; x++)
		for (int y = 0; y < WORLD_DIM_IN_CHUNKS; y++)
			for (int z = 0; z < WORLD_DIM_IN_CHUNKS; z++)
				chunks[x][y][z] = malloc(sizeof(Chunk));
	
	// populate chunks (finite)
	for (int cx = 0; cx < WORLD_DIM_IN_CHUNKS; cx++) {
		for (int cy = 0; cy < WORLD_DIM_IN_CHUNKS; cy++) {
			for (int cz = 0; cz < WORLD_DIM_IN_CHUNKS; cz++) {

				// populate single chunk
				for (int bx = 0; bx < CHUNK_DIM_IN_BLOCKS; bx++) {
					for (int by = 0; by < CHUNK_DIM_IN_BLOCKS; by++) {
						for (int bz = 0; bz < CHUNK_DIM_IN_BLOCKS; bz++) {

							populator(
								cx * CHUNK_DIM_IN_BLOCKS + bx,
								cy * CHUNK_DIM_IN_BLOCKS + by,
								cz * CHUNK_DIM_IN_BLOCKS + bz
							);
						}
					}
				}

				// remesh the chunk
				remesh_delayed_chunks();
			}
		}
	}
}

void draw_chunks(const Transform *camera) {

	int cx, cy, cz;
	Transform chunk_transform = {};

	for (cx = 0; cx < WORLD_DIM_IN_CHUNKS; cx++) {
		for (cy = 0; cy < WORLD_DIM_IN_CHUNKS; cy++) {
			for (cz = 0; cz < WORLD_DIM_IN_CHUNKS; cz++) {

				chunk_transform.x = cx * CHUNK_DIM_IN_BLOCKS;
				chunk_transform.y = cy * CHUNK_DIM_IN_BLOCKS;
				chunk_transform.z = cz * CHUNK_DIM_IN_BLOCKS;

				draw_mesh(camera, &chunk_transform, &chunks[cx][cy][cz]->mesh);
			}
		}
	}
}

static Chunk *get_chunk_of_block(int x, int y, int z) {

	if (
		x < 0 ||
		y < 0 ||
		z < 0 ||
		x >= WORLD_DIM_IN_CHUNKS * CHUNK_DIM_IN_BLOCKS ||
		y >= WORLD_DIM_IN_CHUNKS * CHUNK_DIM_IN_BLOCKS ||
		z >= WORLD_DIM_IN_CHUNKS * CHUNK_DIM_IN_BLOCKS
	)
		return NULL;
	
	return chunks[x / CHUNK_DIM_IN_BLOCKS][y / CHUNK_DIM_IN_BLOCKS][z / CHUNK_DIM_IN_BLOCKS];
}

BlockType *get_block_type(unsigned char id) {

	return &block_types[id];
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
	
	return chunk->blocks[x % CHUNK_DIM_IN_BLOCKS][y % CHUNK_DIM_IN_BLOCKS][z % CHUNK_DIM_IN_BLOCKS];
}

void set_block_at(int x, int y, int z, unsigned char block) {
	
	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % CHUNK_DIM_IN_BLOCKS][y % CHUNK_DIM_IN_BLOCKS][z % CHUNK_DIM_IN_BLOCKS] = block;

	remesh_chunk(chunk);
}

void set_delay_remesh_block_at(int x, int y, int z, unsigned char block) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % CHUNK_DIM_IN_BLOCKS][y % CHUNK_DIM_IN_BLOCKS][z % CHUNK_DIM_IN_BLOCKS] = block;

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

	return BT_IS_SOLID(block_types[get_block_at((int) x, (int) y, (int) z)]);
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