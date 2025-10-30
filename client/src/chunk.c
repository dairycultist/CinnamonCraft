#include "header.h"

#include <math.h>

static BlockType block_types[256] = { 0b00000000, 0, 0, 0 }; // first block is always air
static unsigned char block_type_count = 1;

void register_block_type(BlockType block_type) {

	block_types[block_type_count++] = block_type;
}

// this function determines what mesh/UV a block gets (including face culling)
static void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, const unsigned char blocks[16][16][16], int block_x, int block_y, int block_z) {

	#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.
	#define BT_AT(x, y, z) (block_types[blocks[x][y][z]])

	if (blocks[block_x][block_y][block_z] == 0) { return; }

	BlockType *block_type = &block_types[blocks[block_x][block_y][block_z]];

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
void remesh_chunk(const Chunk *chunk, const Texture *blocksheet_texture) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				append_block_to_mesh(&mesh_data, &vertex_count, chunk->blocks, x, y, z);

	Mesh *mesh = create_mesh(mesh_data.data, mesh_data.bytecount, vertex_count, blocksheet_texture);

	memcpy((void *) &chunk->mesh, mesh, sizeof(Mesh));
	free(mesh);
}

// TODO should be global and account for chunk position and such but whatever
int is_point_inside_chunk(const Chunk *chunk, float x, float y, float z) {

	if (x < 0 || y < 0 || z > 0 || x >= 16 || y >= 16 || z <= -16)
		return FALSE;

	return chunk->blocks[(int) x][(int) y][(int) -z];
}

// the AABB is a rectangular prism with a square base centered on x,y,z (extruding up)
int is_aabb_inside_chunk(const Chunk *chunk, float x, float y, float z, float wl, float h) {

	wl /= 2;

	for (int block_x = floor(x - wl); block_x <= floor(x + wl); block_x++) {

		for (int block_z = ceil(z - wl); block_z <= ceil(z + wl); block_z++) {

			for (int block_y = floor(y); block_y <= floor(y + h); block_y++) {

				if (is_point_inside_chunk(chunk, block_x, block_y, block_z))
					return TRUE;
			}
		}
	}

	return FALSE;
}