#include "header.h"

void append_ezarray(EZArray *array, void *data, int data_length) {

	if (!array->data) {

		// create a data allocation
		array->data = malloc(data_length * 8);
		array->datasize = data_length * 8;

	} else if (array->datasize < array->bytecount + data_length) {

		// increase the size of the data allocation
		array->data = realloc(array->data, (array->bytecount + data_length) * 2);
		array->datasize = (array->bytecount + data_length) * 2;
	}

	memcpy(&array->data[array->bytecount], data, data_length);
	array->bytecount += data_length;
}

void clear_ezarray(EZArray *array) {

	array->bytecount = 0;
}