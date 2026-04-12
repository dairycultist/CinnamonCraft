#include "ez_array.h"
#include <stdlib.h>
#include <string.h>

void append_ezarray(EZArray *array, void *data, int data_length) {

	if (!array->data) {

		// create a data allocation
		array->data = malloc(data_length * 8);
		array->datasize = data_length * 8;
		array->bytecount = 0;

	} else if (array->datasize < array->bytecount + data_length) {

		// increase the size of the data allocation
		array->data = realloc(array->data, (array->bytecount + data_length) * 2);
		array->datasize = (array->bytecount + data_length) * 2;
	}

	memcpy(array->data + array->bytecount, data, data_length);
	array->bytecount += data_length;
}

int contains_ezarray(EZArray *array, void *data, int data_length) {

	for (int i = 0; i < array->bytecount; i += data_length)
		if (!memcmp(data, array->data + i, data_length))
			return 1;

	return 0;
}

void clear_ezarray(EZArray *array) {

	array->bytecount = 0;
}