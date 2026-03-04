#ifndef EZ_ARRAY_H
#define EZ_ARRAY_H

typedef struct {

	unsigned char *data;
	int datasize;
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length);
int contains_ezarray(EZArray *array, void *data, int data_length);
void clear_ezarray(EZArray *array);

#endif