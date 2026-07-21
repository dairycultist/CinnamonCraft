#include "main.h"
#include "terrain.h"
#include "ez_array.h"

#include <stdlib.h>
#include <math.h>

// stole this from nash so I don't have to use rand(). it's deterministic!
unsigned int r_hash(unsigned int seed) {
	
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

#define MASK_FULLBLOCK 0b00000001
#define MASK_COLLIDABLE 0b00000010

#define BLOCK_AIR 0
#define BLOCK_GRASS 1
#define BLOCK_STONE 2

// block mesh types
#define MESH_EMPTY 0
#define MESH_TOP_AND_BOTTOM 1

typedef struct {

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

	// first element is always the mesh type
	// the use of latter elements is determined by the mesh type but is usually atlas indices
	unsigned char mesh_data[4];

} BlockType;

typedef struct { // only this file knows Chunks even exist

	Mesh mesh;
	block_t blocks[16][128][16]; // array of bytes indexing into block_types

	int chunk_x, chunk_z;

} Chunk;

static Texture blockmap_texture;

// both of these store Chunk *
static EZArray chunks; // first-come, first-serve; empty spots are NULL
static EZArray delayed_remesh_chunks; // when you want to set a bunch of blocks, remeshing after each is slow and redundant, so you save them to remesh once at the end

#define MAX_CHUNK_COUNT (chunks.bytecount / sizeof(Chunk *))

static int full_block_collider(float dx, float dy, float dz) {

	return 1;
}

// block type registry
static BlockType block_types[256] = {

	[ BLOCK_AIR ]	= (BlockType) { 0 },
	[ BLOCK_GRASS ]	= (BlockType) { full_block_collider, 0b00000011, 60, { MESH_TOP_AND_BOTTOM, 0, 1, 2 } },
	[ BLOCK_STONE ]	= (BlockType) { full_block_collider, 0b00000011, 20, { MESH_TOP_AND_BOTTOM, 3, 3, 3 } }
};

static inline int is_block_fullblock(block_t block) {

	return block_types[block].flags & MASK_FULLBLOCK;
}

static inline unsigned char get_block_mesh_data(block_t block, int i) {

	return block_types[block].mesh_data[i];
}

#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.

//                                                                                              [ -x, +x, -z, +z, -y (bottom), +y (top) ]
static void helper_append_fullblock(EZArray *mesh_data, int *vertex_count, int x, int y, int z, unsigned char faces[6]) {

	int local_x = x % 16;
	int local_y = y;
	int local_z = z % 16;

	float u_sml, v_sml, u_big, v_big;

	// -x face
	if (!is_block_fullblock(get_block_at(x - 1, y, z))) {

		GET_SPRITEMAP_UV(faces[0], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x, local_y, local_z,			-1, 0, 0,	u_big, v_sml,
			local_x, local_y, local_z + 1,		-1, 0, 0,	u_sml, v_sml,
			local_x, local_y + 1, local_z,		-1, 0, 0,	u_big, v_big,
			local_x, local_y + 1, local_z + 1,	-1, 0, 0,	u_sml, v_big,
			local_x, local_y + 1, local_z,		-1, 0, 0,	u_big, v_big,
			local_x, local_y, local_z + 1,		-1, 0, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +x face
	if (!is_block_fullblock(get_block_at(x + 1, y, z))) {

		GET_SPRITEMAP_UV(faces[1], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x + 1, local_y, local_z,			1, 0, 0,	u_sml, v_sml,
			local_x + 1, local_y + 1, local_z,		1, 0, 0,	u_sml, v_big,
			local_x + 1, local_y, local_z + 1,		1, 0, 0,	u_big, v_sml,
			local_x + 1, local_y + 1, local_z + 1,	1, 0, 0,	u_big, v_big,
			local_x + 1, local_y, local_z + 1,		1, 0, 0,	u_big, v_sml,
			local_x + 1, local_y + 1, local_z,		1, 0, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -z face
	if (!is_block_fullblock(get_block_at(x, y, z - 1))) {

		GET_SPRITEMAP_UV(faces[2], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x, local_y, local_z,			0, 0, -1,	u_sml, v_sml,
			local_x, local_y + 1, local_z,		0, 0, -1,	u_sml, v_big,
			local_x + 1, local_y, local_z,		0, 0, -1,	u_big, v_sml,
			local_x + 1, local_y + 1, local_z,	0, 0, -1,	u_big, v_big,
			local_x + 1, local_y, local_z,		0, 0, -1,	u_big, v_sml,
			local_x, local_y + 1, local_z,		0, 0, -1,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +z face
	if (!is_block_fullblock(get_block_at(x, y, z + 1))) {

		GET_SPRITEMAP_UV(faces[3], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x, local_y, local_z + 1,			0, 0, 1,	u_big, v_sml,
			local_x + 1, local_y, local_z + 1,		0, 0, 1,	u_sml, v_sml,
			local_x, local_y + 1, local_z + 1,		0, 0, 1,	u_big, v_big,
			local_x + 1, local_y + 1, local_z + 1,	0, 0, 1,	u_sml, v_big,
			local_x, local_y + 1, local_z + 1,		0, 0, 1,	u_big, v_big,
			local_x + 1, local_y, local_z + 1,		0, 0, 1,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -y face
	if (!is_block_fullblock(get_block_at(x, y - 1, z))) {

		GET_SPRITEMAP_UV(faces[4], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x, local_y, local_z,			0, -1, 0,	u_sml, v_sml,
			local_x + 1, local_y, local_z,		0, -1, 0,	u_sml, v_big,
			local_x, local_y, local_z + 1,		0, -1, 0,	u_big, v_sml,
			local_x + 1, local_y, local_z + 1,	0, -1, 0,	u_big, v_big,
			local_x, local_y, local_z + 1,		0, -1, 0,	u_big, v_sml,
			local_x + 1, local_y, local_z,		0, -1, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +y face
	if (!is_block_fullblock(get_block_at(x, y + 1, z))) {

		GET_SPRITEMAP_UV(faces[5], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			local_x, 		local_y + 1, local_z,			0, 1, 0,	u_big, v_sml,
			local_x, 		local_y + 1, local_z + 1,		0, 1, 0,	u_sml, v_sml,
			local_x + 1, 	local_y + 1, local_z,			0, 1, 0,	u_big, v_big,
			local_x + 1, 	local_y + 1, local_z + 1,		0, 1, 0,	u_sml, v_big,
			local_x + 1, 	local_y + 1, local_z,			0, 1, 0,	u_big, v_big,
			local_x, 		local_y + 1, local_z + 1,		0, 1, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}
}

static void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, int x, int y, int z) {

	block_t block = get_block_at(x, y, z);

	switch (get_block_mesh_data(block, 0)) {

		case MESH_EMPTY: return;
		case MESH_TOP_AND_BOTTOM: helper_append_fullblock(mesh_data, vertex_count, x, y, z, (unsigned char[6]) {
				get_block_mesh_data(block, 2),
				get_block_mesh_data(block, 2),
				get_block_mesh_data(block, 2),
				get_block_mesh_data(block, 2),
				get_block_mesh_data(block, 3),
				get_block_mesh_data(block, 1)
			});
			return;

	}
}

// remeshes based on the chunk's internal blocks
static void remesh_chunk(Chunk *chunk) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 128; y++)
			for (int z = 0; z < 16; z++)
				append_block_to_mesh(
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
}

void draw_chunks(const Transform *camera) {

	Transform chunk_transform = {};

	for (int i = 0; i < MAX_CHUNK_COUNT; i++) {

		if (!INDEX_EZARRAY(chunks, Chunk *, i))
			continue;

		chunk_transform.x = INDEX_EZARRAY(chunks, Chunk *, i)->chunk_x * 16;
		chunk_transform.y = 0.0;
		chunk_transform.z = INDEX_EZARRAY(chunks, Chunk *, i)->chunk_z * 16;

		draw_mesh(camera, &chunk_transform, INDEX_EZARRAY(chunks, Chunk *, i)->mesh);
	}
}

static int get_chunk_index_at(int chunk_x, int chunk_z) {

	for (int i = 0; i < MAX_CHUNK_COUNT; i++)
		if (INDEX_EZARRAY(chunks, Chunk *, i) && INDEX_EZARRAY(chunks, Chunk *, i)->chunk_x == chunk_x && INDEX_EZARRAY(chunks, Chunk *, i)->chunk_z == chunk_z)
			return i;

	return -1;
}

static int get_free_chunk_index() {

	for (int i = 0; i < MAX_CHUNK_COUNT; i++)
		if (!INDEX_EZARRAY(chunks, Chunk *, i))
			return i;

	return -1;
}

void create_chunk_at(int chunk_x, int chunk_z) {

	int i = get_chunk_index_at(chunk_x, chunk_z);

	if (i != -1)
		return; // chunk already exists at that position

	Chunk *chunk = malloc(sizeof(Chunk));
	chunk->mesh = create_mesh(NULL, 0, 0, blockmap_texture);
	chunk->chunk_x = chunk_x;
	chunk->chunk_z = chunk_z;

	i = get_free_chunk_index();

	if (i == -1) { // append chunk

		append_ezarray(&chunks, &chunk, sizeof(Chunk *));

	} else { // insert chunk

		INDEX_EZARRAY(chunks, Chunk *, i) = chunk;
	}
}

void destroy_chunk_at(int chunk_x, int chunk_z) {

	int i = get_chunk_index_at(chunk_x, chunk_z);

	free_mesh(INDEX_EZARRAY(chunks, Chunk *, i)->mesh);
	free(INDEX_EZARRAY(chunks, Chunk *, i));

	INDEX_EZARRAY(chunks, Chunk *, i) = NULL;

	// TODO should make sure that this chunk isn't stored in delayed_remesh_chunks
}

static Chunk *get_chunk_of_block(int x, int y, int z) {

	if (y < 0 || y >= 128) // we include y so that OOB y will return NULL
		return NULL;
	
	int i = get_chunk_index_at((int) floor(x / 16.0), (int) floor(z / 16.0));

	return i == -1 ? NULL : INDEX_EZARRAY(chunks, Chunk *, i);
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

unsigned short get_block_ticks_to_break(block_t block) {

	return block_types[block].ticks_to_break;
}

void set_delay_remesh_block_at(int x, int y, int z, block_t block) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[x % 16][y][z % 16] = block;

	// save chunk for delayed remeshing if it's not already saved
	if (index_of_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *)) == -1)
		append_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *));
}

void remesh_delayed_chunks() {

	for (int i = 0; i < delayed_remesh_chunks.bytecount / sizeof(Chunk *); i++)
		remesh_chunk(INDEX_EZARRAY(delayed_remesh_chunks, Chunk *, i));

	clear_ezarray(&delayed_remesh_chunks);
}

int does_point_intersect_blocks(float x, float y, float z) {

	BlockType *block_type = &block_types[get_block_at(floor(x), floor(y), floor(z))];

	return (block_type->flags & MASK_COLLIDABLE) && block_type->does_local_point_collide(
		fmod(fmod(x, 1.0) + 1.0, 1.0),
		fmod(fmod(y, 1.0) + 1.0, 1.0),
		fmod(fmod(z, 1.0) + 1.0, 1.0)
	);
}

int does_aabb_intersect_blocks(AABB *aabb) {

	//                                                                     V for float imprecision from        V this
	for (float block_x = aabb->x - aabb->r; block_x <= aabb->x + aabb->r + AABB_COLLISION_DS * 0.5; block_x += AABB_COLLISION_DS) {
	for (float block_z = aabb->z - aabb->r; block_z <= aabb->z + aabb->r + AABB_COLLISION_DS * 0.5; block_z += AABB_COLLISION_DS) {
	for (float block_y = aabb->y;           block_y <= aabb->y + aabb->h + AABB_COLLISION_DS * 0.5; block_y += AABB_COLLISION_DS) {

		if (does_point_intersect_blocks(block_x, block_y, block_z))
			return 1;
	}}}

	return 0;
}

int would_aabb_intersect_block_at(int x, int y, int z, block_t block, AABB *aabb) {

	BlockType *block_type = &block_types[block];

	if (!(block_type->flags & MASK_COLLIDABLE))
		return 0;

	float aabb_x = aabb->x - x;
	float aabb_y = aabb->y - y;
	float aabb_z = aabb->z - z;

	for (float dx = fmax(0, aabb_x - aabb->r); dx <= fmin(1, aabb_x + aabb->r); dx += AABB_COLLISION_DS) {
	for (float dz = fmax(0, aabb_z - aabb->r); dz <= fmin(1, aabb_z + aabb->r); dz += AABB_COLLISION_DS) {
	for (float dy = fmax(0, aabb_y);           dy <= fmin(1, aabb_y + aabb->h); dy += AABB_COLLISION_DS) {

		if (block_type->does_local_point_collide(dx, dy, dz))
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