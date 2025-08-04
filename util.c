#ifndef UTIL_DEFINED

#define UTIL_DEFINED

#define TRUE 1
#define FALSE 0
#define DEG2RAD (M_PI / 180)

static unsigned int rng_state = 1; // uint32_t? time(NULL)?

// stole this from nash so I don't have to use rand(). it's deterministic!
unsigned int random_uint(unsigned int bound) {
	
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state % bound;
}

unsigned char random_unsigned_char() {

	return (unsigned char) random_uint(256);
}

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