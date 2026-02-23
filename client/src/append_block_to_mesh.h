#ifndef ABTM_H
#define ABTM_H

void ABTM_block(EZArray *mesh_data, int *vertex_count, const unsigned char blocks[CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS][CHUNK_DIM_IN_BLOCKS], int block_x, int block_y, int block_z);

#endif