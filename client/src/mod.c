#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "engine.h"
#include "mod.h"

#define TRUE 1
#define FALSE 0

// static unsigned int rng_state = 1; // uint32_t? time(NULL)?

// // stole this from nash so I don't have to use rand(). it's deterministic!
// unsigned int random_uint(unsigned int bound) {
	
//     rng_state ^= rng_state << 13;
//     rng_state ^= rng_state >> 17;
//     rng_state ^= rng_state << 5;
//     return rng_state % bound;
// }

Transform camera;
Transform chunk_transform;
Transform miku_transform;

Mesh *miku_mesh;

Chunk chunk;

int left     = FALSE;
int right    = FALSE;
int forward  = FALSE;
int backward = FALSE;
int up       = FALSE;
int down     = FALSE;

void on_start() {

	register_block_type((BlockType) { 0b00000001, 240, 240, 240 });
	
	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.z = 2;

	// create a mesh for testing
	miku_mesh = create_mesh_from_obj("client/res/miku.obj", load_texture("client/res/dirt.png"));
	miku_transform.x = 4;
	miku_transform.z = -4;
	miku_transform.y = 6.45;

	// create a chunk for testing
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				chunk.blocks[x][y][z] = (sin(x * 0.5) / 4 + 0.5) > (1 - y / 16.) ? 0 : 1;

	remesh_chunk(&chunk, load_texture("client/res/blockmap.png"));
}

void on_terminate() {

	free(miku_mesh);
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

	miku_transform.yaw += 0.01;

	draw_mesh(&camera, &miku_transform, miku_mesh);
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