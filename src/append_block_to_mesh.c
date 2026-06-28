#include "main.h"
#include "terrain.h"
#include "ez_array.h"
#include "append_block_to_mesh.h"

// stole this from nash so I don't have to use rand(). it's deterministic!
unsigned int r_hash(unsigned int seed) {
	
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.

//                                                                                              [ -x, +x, -z, +z, -y (bottom), +y (top) ]
static void helper_append_fullblock(EZArray *mesh_data, int *vertex_count, int x, int y, int z, atlas_index_t faces[6]) {

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

// all the functions below serve as possible values for BlockType's function pointer append_block_to_mesh

// atlas_indices [ top side bottom ]
void ABTM_block(EZArray *mesh_data, int *vertex_count, int x, int y, int z) {

	block_t block = get_block_at(x, y, z);

	helper_append_fullblock(mesh_data, vertex_count, x, y, z, (atlas_index_t[6]) {
		get_block_atlas_index(block, 1),
		get_block_atlas_index(block, 1),
		get_block_atlas_index(block, 1),
		get_block_atlas_index(block, 1),
		get_block_atlas_index(block, 2),
		get_block_atlas_index(block, 0)
	});
}

void ABTM_grass(EZArray *mesh_data, int *vertex_count, int x, int y, int z) {

	if (get_block_at(x, y + 1, z) == 0) {

		// grass top
		ABTM_block(mesh_data, vertex_count, x, y, z);

		// randomly place tall grass
		if (r_hash(x * 108 + z * 4878) % 3 != 0) {

			float local_x = x % 16 + (r_hash(x * 51 + z * 12) % 30) * 0.01;
			float local_y = y      - (r_hash(x * 7 + z * 5) % 30) * 0.01;
			float local_z = z % 16 + (r_hash(x * 19 + z * 154) % 30) * 0.01;

			float u_sml, v_sml, u_big, v_big;

			GET_SPRITEMAP_UV(4, u_sml, v_sml, u_big, v_big);

			float full_block_data[] = {
				local_x,	 local_y + 1, 	local_z,		0, 1, 0,	u_big, v_sml,
				local_x + .7,local_y + 1, 	local_z + .7,	0, 1, 0,	u_sml, v_sml,
				local_x,	 local_y + 2, 	local_z,		0, 1, 0,	u_big, v_big,
				local_x + .7,local_y + 2, 	local_z + .7,	0, 1, 0,	u_sml, v_big,
				local_x,	 local_y + 2, 	local_z,		0, 1, 0,	u_big, v_big,
				local_x + .7,local_y + 1, 	local_z + .7,	0, 1, 0,	u_sml, v_sml,

				local_x,	 local_y + 1, 	local_z,		0, 1, 0,	u_big, v_sml,
				local_x,	 local_y + 2, 	local_z,		0, 1, 0,	u_big, v_big,
				local_x + .7,local_y + 1, 	local_z + .7,	0, 1, 0,	u_sml, v_sml,
				local_x + .7,local_y + 2, 	local_z + .7,	0, 1, 0,	u_sml, v_big,
				local_x + .7,local_y + 1, 	local_z + .7,	0, 1, 0,	u_sml, v_sml,
				local_x,	 local_y + 2, 	local_z,		0, 1, 0,	u_big, v_big,

				local_x,		local_y + 1, 	local_z + .7,	0, 1, 0,	u_big, v_sml,
				local_x + .7,	local_y + 1, 	local_z,		0, 1, 0,	u_sml, v_sml,
				local_x,		local_y + 2, 	local_z + .7,	0, 1, 0,	u_big, v_big,
				local_x + .7,	local_y + 2, 	local_z,		0, 1, 0,	u_sml, v_big,
				local_x,		local_y + 2, 	local_z + .7,	0, 1, 0,	u_big, v_big,
				local_x + .7,	local_y + 1, 	local_z,		0, 1, 0,	u_sml, v_sml,

				local_x,		local_y + 1, 	local_z + .7,	0, 1, 0,	u_big, v_sml,
				local_x,		local_y + 2, 	local_z + .7,	0, 1, 0,	u_big, v_big,
				local_x + .7,	local_y + 1, 	local_z,		0, 1, 0,	u_sml, v_sml,
				local_x + .7,	local_y + 2, 	local_z,		0, 1, 0,	u_sml, v_big,
				local_x + .7,	local_y + 1, 	local_z,		0, 1, 0,	u_sml, v_sml,
				local_x,		local_y + 2, 	local_z + .7,	0, 1, 0,	u_big, v_big,
			};

			append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 24);
			*vertex_count += 24;
		}

	} else {

		// dirt top
		block_t block = get_block_at(x, y, z);

		helper_append_fullblock(mesh_data, vertex_count, x, y, z, (atlas_index_t[6]) {
			get_block_atlas_index(block, 2),
			get_block_atlas_index(block, 2),
			get_block_atlas_index(block, 2),
			get_block_atlas_index(block, 2),
			get_block_atlas_index(block, 2),
			get_block_atlas_index(block, 2)
		});
	}
}