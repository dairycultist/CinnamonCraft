#ifndef UTIL_DEFINED

#define UTIL_DEFINED

#define TRUE 1
#define FALSE 0
#define DEG2RAD (M_PI / 180)

typedef struct {

	unsigned char *data;
	int datasize;
	int bytecount;

} EZArray;

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

#endif