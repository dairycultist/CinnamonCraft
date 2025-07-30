// isolation/abstraction is good :)

#ifndef UTIL_DEFINED

#define UTIL_DEFINED

#define TRUE 1
#define FALSE 0
#define DEG2RAD (M_PI / 180)

static uint rng_state = 1; // uint32_t? time(NULL)?

// stole this from nash so I don't have to use rand(). it's deterministic!
uint random_uint(uint bound) {
	
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state % bound;
}

void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

typedef struct {

	char data[65536 * 64];
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length) {

	memcpy(&array->data[array->bytecount], data, data_length);
	array->bytecount += data_length;
}

#endif