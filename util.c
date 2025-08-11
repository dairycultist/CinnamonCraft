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

unsigned char random_uchar() {

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

void populate_2D_noise(int width, int height, int smoothness, float *buffer) {

	// these comments were made for 1D noise and I kinda just extrapolated the code to 2D the best I could

	// generate array of random values
	for (int i = 0; i < width * height; i++) {
		buffer[i] = random_uint(10000) * 0.0001;
	}

	// smoothing step (must repeat this ~20 times or until smooth)
	for (int i = 0; i < smoothness; i++) {

		// for every element except the last one, average it with the element following it
		for (int x = 0; x < width - 1; x++) {
			for (int y = 0; y < height - 1; y++) {
			
				buffer[y * width + x] = (buffer[y * width + x] + buffer[y * width + x + 1] + buffer[(y + 1) * width + x] + buffer[(y + 1) * width + x + 1]) / 4;
			}
		}

		// finally set the last element to the first element
		for (int x = 0; x < width - 1; x++) {

			buffer[(height - 1) * width + x] = buffer[x];
		}

		for (int y = 0; y < height; y++) {

			buffer[y * width + (width - 1)] = buffer[y * width];
		}
	}
}

#endif