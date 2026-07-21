#ifndef EZ_ARRAY_H
#define EZ_ARRAY_H

// helper data structure

typedef struct {

	unsigned char *data;
	int datasize;
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length);
int index_of_ezarray(EZArray *array, void *data, int data_length);
void clear_ezarray(EZArray *array);

#define INDEX_EZARRAY(array, type, i) (*(((type*) (array).data) + i))

#endif