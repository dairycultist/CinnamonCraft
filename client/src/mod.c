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

static Transform camera;
static float vertical_velocity;

static Transform chunk_transform;
static Transform miku_transform;

static Mesh *miku_mesh;

static Chunk chunk;

static int left     = FALSE;
static int right    = FALSE;
static int forward  = FALSE;
static int backward = FALSE;
static int up       = FALSE;
static int down     = FALSE;

void on_start() {

	register_block_type((BlockType) { 0b00000001, 240, 240, 240 });
	register_block_type((BlockType) { 0b00000001, 241, 241, 241 });
	
	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.z = -2;
	camera.y = 11;

	// create a mesh for testing
	miku_mesh = create_mesh_from_obj("client/res/miku.obj", load_texture("client/res/dirt.png"));
	miku_transform.x = 4;
	miku_transform.z = -4;
	miku_transform.y = 6.45;

	// create a chunk for testing
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				chunk.blocks[x][y][z] = (sin(x * 0.5) / 4 + 0.5) > (1 - y / 16.) ? 0 : (y < 5 ? 2 : 1);

	remesh_chunk(&chunk, load_texture("client/res/blockmap.png"));
}

void on_terminate() {

	free(miku_mesh);
}

void process_tick() {

	miku_transform.yaw += 0.01;

	// move in direction of input
	// if colliding, step in opposite direction in small increments (10) until no longer collision (or completely undid movement)
	// doesn't allow sliding against walls ugh

	#define PLAYER_WL 0.2
	#define PLAYER_H 1.8
	#define PLAYER_CAM_H 1.5

	if (left) {

		camera.z -= sin(camera.yaw) * 0.1;
		camera.x -= cos(camera.yaw) * 0.1;

		for (int i=0; is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H) && i < 10; i++) {

			camera.z += sin(camera.yaw) * 0.01;
			camera.x += cos(camera.yaw) * 0.01;
		}

	} else if (right) {

		camera.z += sin(camera.yaw) * 0.1;
		camera.x += cos(camera.yaw) * 0.1;

		for (int i=0; is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H) && i < 10; i++) {

			camera.z -= sin(camera.yaw) * 0.01;
			camera.x -= cos(camera.yaw) * 0.01;
		}
	}

	if (forward) {

		camera.z -= cos(camera.yaw) * 0.1;
		camera.x += sin(camera.yaw) * 0.1;

		for (int i=0; is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H) && i < 10; i++) {

			camera.z += cos(camera.yaw) * 0.01;
			camera.x -= sin(camera.yaw) * 0.01;
		}

	} else if (backward) {

		camera.z += cos(camera.yaw) * 0.1;
		camera.x -= sin(camera.yaw) * 0.1;

		for (int i=0; is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H) && i < 10; i++) {

			camera.z -= cos(camera.yaw) * 0.01;
			camera.x += sin(camera.yaw) * 0.01;
		}
	}

	camera.y += vertical_velocity;

	if (is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H)) {

		for (int i=0; is_aabb_inside_chunk(&chunk, camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H) && i < 10; i++) {

			camera.y -= vertical_velocity / 10;
		}

		vertical_velocity = -0.01;

		// jump
		if (up)
			vertical_velocity = 0.2;

	} else {

		// gravity
		vertical_velocity -= 0.01;
	}

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