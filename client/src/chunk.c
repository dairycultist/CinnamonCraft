#include "../../util.c"

typedef struct {

	Mesh mesh;
	unsigned char blocks[16][16][16]; // array of bytes representing blockstates
	
	// int chunk_x;
	// int chunk_y;
	// int chunk_z;

} Chunk;

// TODO allow initialization by mod using register_block function
static unsigned char block_types[256 * 4] = { // 4 bytes: block mesh (0:empty,1:cube,...slope?) | top texture index | side texture index | bottom texture index
	0, 0, 0, 0,
	1, 98, 243, 242
};

#define BLOCK_MESH_EMPTY 0
#define BLOCK_MESH_CUBE 1

#define BLOCK_GET_MESH_TYPE(block) (block_types[block * 4])
#define BLOCK_GET_TOP(block) (block_types[block * 4 + 1])
#define BLOCK_GET_SIDE(block) (block_types[block * 4 + 2])
#define BLOCK_GET_BOTTOM(block) (block_types[block * 4 + 3])
#define BLOCK_HAS_PASSTHROUGH(block) (BLOCK_GET_MESH_TYPE(block) == 0) // "passthrough" means adjacent blocks aren't able to cull the faces that touch it

#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.;

void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, const unsigned char blocks[16][16][16], int block_x, int block_y, int block_z) {

	// this function determines what mesh/UV a block gets (including considering its environment)

	unsigned char block = blocks[block_x][block_y][block_z];

	if (BLOCK_GET_MESH_TYPE(block) == BLOCK_MESH_EMPTY) { return; }

	float u_sml, v_sml, u_big, v_big;

	// get UV for sides
	GET_SPRITEMAP_UV(BLOCK_GET_SIDE(block), u_sml, v_sml, u_big, v_big)

	// -x face
	if (block_x == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x-1][block_y][block_z])) {

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
	if (block_x == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x+1][block_y][block_z])) {

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
	if (block_z == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y][block_z-1])) {

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
	if (block_z == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y][block_z+1])) {

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
	if (block_y == 0 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y-1][block_z])) {

		// get UV for bottom
		GET_SPRITEMAP_UV(BLOCK_GET_BOTTOM(block), u_sml, v_sml, u_big, v_big)

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
	if (block_y == 15 || BLOCK_HAS_PASSTHROUGH(blocks[block_x][block_y+1][block_z])) {

		// get UV for top
		GET_SPRITEMAP_UV(BLOCK_GET_TOP(block), u_sml, v_sml, u_big, v_big)

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
void remesh_chunk(const Chunk *chunk) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				append_block_to_mesh(&mesh_data, &vertex_count, chunk->blocks, x, y, z);

	Mesh *mesh = create_mesh(mesh_data.data, mesh_data.bytecount, vertex_count, block_spritemap_data, 256, 256);

	memcpy((void *) &chunk->mesh, mesh, sizeof(Mesh));
	free(mesh);
}

// TODO should be global and account for chunk position and such but whatever
int is_point_intersecting(const Chunk *chunk, float x, float y, float z) {

	if (x < 0 || y < 0 || z > 0 || x > 16 || y > 16 || z < -16)
		return FALSE;

	return chunk->blocks[(int) x][(int) y][(int) -z];
}

int is_aabb_cube_intersecting(const Chunk *chunk, float x, float y, float z, float size) {

	// gonna use unit aabb for now
	return is_point_intersecting(chunk, x - size, y - size, z - size)
	    || is_point_intersecting(chunk, x - size, y - size, z + size)
		|| is_point_intersecting(chunk, x - size, y + size, z - size)
		|| is_point_intersecting(chunk, x - size, y + size, z + size)
		|| is_point_intersecting(chunk, x + size, y - size, z - size)
		|| is_point_intersecting(chunk, x + size, y - size, z + size)
		|| is_point_intersecting(chunk, x + size, y + size, z - size)
		|| is_point_intersecting(chunk, x + size, y + size, z + size);
}