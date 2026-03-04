#ifndef CHUNK_H
#define CHUNK_H

// only 256 texture indices and 256 block types can exist right now

/*
 * Abstraction layer for all block/chunk related actions.
 */

#define BT_IS_FULLBLOCK(block_type) ((block_type).flags & 0b00000001)
#define BT_IS_COLLIDABLE(block_type) ((block_type).flags & 0b00000010)

#define WORLD_DIM_IN_CHUNKS 8
#define CHUNK_DIM_IN_BLOCKS 16

typedef struct {

	// other stuff like mining level, dropped item, maybe function pointer for append_block_to_mesh

	// this function determines what mesh/UV a block gets (including face culling)
	void (*append_block_to_mesh)(EZArray *mesh_data, int *vertex_count, const unsigned char blocks[CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS], int block_x, int block_y, int block_z);

	// TODO add flipx, flipy, flipz (used by append_block_to_mesh)
	// 1 "fullblock": adjacent blocks will cull the faces that touch it
	// 2 "collidable": has full-block collision
	// 4
	// 8
	// 16
	// 32
	// 64
	// 128
	unsigned char flags;

	unsigned char tex_top;
	unsigned char tex_side;
	unsigned char tex_bottom;
	unsigned short ticks_to_break;

} BlockType;

void initialize_chunk_system(void (*chunk_populator)(int x, int y, int z)); // chunk populator doesn't need to account for remeshes
void register_block_type(BlockType block_type);
BlockType *get_block_type(unsigned char id);

void draw_chunks(const Transform *camera);

unsigned char get_block_at(int x, int y, int z);
void set_block_at(int x, int y, int z, unsigned char block);
void set_delay_remesh_block_at(int x, int y, int z, unsigned char block);
void remesh_delayed_chunks();

int does_point_intersect_blocks(float x, float y, float z);
int does_aabb_intersect_blocks(float x, float y, float z, float wl, float h);
int raycast_blocks(const Transform *origin, float max_dist, int bool_surface, int *out_x, int *out_y, int *out_z);

#endif