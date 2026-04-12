#ifndef ABTM_H
#define ABTM_H

void ABTM_block(EZArray *mesh_data, int *vertex_count, int x, int y, int z); // xyz are global block coordinates
void ABTM_grass(EZArray *mesh_data, int *vertex_count, int x, int y, int z);

#endif