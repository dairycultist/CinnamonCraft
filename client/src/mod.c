#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "engine.h"
#include "mod.h"

#include "resources.c" // binary, automatically updated with new OBJ on Make (ought to replace with external loading since better for modding + more intuitive)

#define TRUE 1
#define FALSE 0

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

Transform camera;
Transform chunk_transform;
Transform mesh_transform;

Mesh *miku;

Chunk chunk;

int left     = FALSE;
int right    = FALSE;
int forward  = FALSE;
int backward = FALSE;
int up       = FALSE;
int down     = FALSE;

void on_start() {
	
	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.z = 2;

	// create a mesh for testing
	miku = create_mesh(miku_mesh_data, miku_mesh_bytecount, miku_mesh_vertcount, load_texture("client/res/dirt.ppm"));

	// create a chunk for testing
	float heightmap[16][16];
	populate_2D_noise(16, 16, 20, (float *) heightmap);

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				chunk.blocks[x][y][z] = heightmap[x][z] > (1 - y / 16.) ? 0 : 1;

	remesh_chunk(&chunk, load_texture("client/res/minecraft_block_spritemap.ppm"));
}

void on_terminate() {

	free(miku);
}

void process_tick() {

	// move in direction of input
	// if colliding, step in opposite direction in small increments (10) until no longer collision (or completely undid movement)
	// doesn't allow sliding against walls ugh

	const float size = 0.2;

	if (left) {

		camera.z -= sin(camera.yaw) * 0.1;
		camera.x -= cos(camera.yaw) * 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.z += sin(camera.yaw) * 0.01;
				camera.x += cos(camera.yaw) * 0.01;
			}
		}

	} else if (right) {

		camera.z += sin(camera.yaw) * 0.1;
		camera.x += cos(camera.yaw) * 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.z -= sin(camera.yaw) * 0.01;
				camera.x -= cos(camera.yaw) * 0.01;
			}
		}
	}

	if (forward) {

		camera.z -= cos(camera.yaw) * 0.1;
		camera.x += sin(camera.yaw) * 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.z += cos(camera.yaw) * 0.01;
				camera.x -= sin(camera.yaw) * 0.01;
			}
		}

	} else if (backward) {

		camera.z += cos(camera.yaw) * 0.1;
		camera.x -= sin(camera.yaw) * 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.z -= cos(camera.yaw) * 0.01;
				camera.x += sin(camera.yaw) * 0.01;
			}
		}
	}

	if (up) {

		camera.y += 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.y -= 0.01;
			}
		}

	} else if (down) {

		camera.y -= 0.1;

		if (is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_inside_chunk(&chunk, camera.x, camera.y, camera.z, size); i++) {

				camera.y += 0.01;
			}
		}
	}

	mesh_transform.yaw += 0.01;

	draw_mesh(&camera, &mesh_transform, miku);
	draw_mesh(&camera, &chunk_transform, &chunk.mesh);
}

void process_event(SDL_Event event) {

	if (event.type == SDL_MOUSEMOTION) {

		camera.pitch += event.motion.yrel * 0.01;
		camera.yaw += event.motion.xrel * 0.01;

		// clamp camera pitch
		if (camera.pitch > M_PI / 2) {
			camera.pitch = M_PI / 2;
		} else if (camera.pitch < -M_PI / 2) {
			camera.pitch = -M_PI / 2;
		}
	}

	else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {

		if (event.key.keysym.scancode == SDL_SCANCODE_A) {
			left = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
			right = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
			forward = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
			backward = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			up = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
			down = TRUE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());
		}
	}

	else if (event.type == SDL_KEYUP) {

		if (event.key.keysym.scancode == SDL_SCANCODE_A) {
			left = FALSE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
			right = FALSE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
			forward = FALSE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
			backward = FALSE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			up = FALSE;
		} else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
			down = FALSE;
		}
	}
}