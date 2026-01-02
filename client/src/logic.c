#include "header.h"
#include "chunk.h"

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

static Transform miku_transform;

static Mesh *miku_mesh;

static int left     = FALSE;
static int right    = FALSE;
static int forward  = FALSE;
static int backward = FALSE;
static int up       = FALSE;
static int down     = FALSE;
static int attack   = FALSE;

static int BOOL_look_block;
static int look_block_x, look_block_y, look_block_z;
static unsigned short look_block_ticks_to_break;

static void chunk_populator(int x, int y, int z) {

	set_delay_remesh_block_at(
		x, y, z,
		sin(x * 0.1) * 16 + 16 < y ? 0 : (y < 20 ? 2 : 1)
	);
}

void on_start() {

	register_block_type((BlockType) { 0b00000001, 240, 240, 240, 50 });
	register_block_type((BlockType) { 0b00000001, 241, 241, 241, 100 });

	initialize_chunk_system(chunk_populator);
	
	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera.x = 8;
	camera.z = 8;
	camera.y = 30;

	// create a mesh for testing
	miku_mesh = create_mesh_from_obj("client/res/miku.obj", load_texture("client/res/dirt.png"));
	miku_transform.x = 8;
	miku_transform.z = 8;
	miku_transform.y = 30;
}

void on_terminate() {

	free(miku_mesh);
}

void process_tick() {

	miku_transform.yaw += 0.01;

	// player control
	#define PLAYER_WL 0.4
	#define PLAYER_H 1.7
	#define PLAYER_CAM_H 1.4

	#define PLAYER_IS_COLLIDING does_aabb_intersect_blocks(camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_WL, PLAYER_H)

	// move in direction of input (crucially, splitting movement into its components to allow for sliding)
	// if colliding, step in opposite direction in small increments until no longer collision (or completely undid movement + a little to prevent float-error related stuckage)
	if (left) {

		camera.z -= sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z += sin(camera.yaw) * 0.01;

		camera.x -= cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x += cos(camera.yaw) * 0.01;

	} else if (right) {

		camera.z += sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z -= sin(camera.yaw) * 0.01;

		camera.x += cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x -= cos(camera.yaw) * 0.01;
	}

	if (forward) {

		camera.z -= cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z += cos(camera.yaw) * 0.01;

		camera.x += sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x -= sin(camera.yaw) * 0.01;

	} else if (backward) {

		camera.z += cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z -= cos(camera.yaw) * 0.01;

		camera.x -= sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x += sin(camera.yaw) * 0.01;
	}

	// vertical movement
	camera.y += vertical_velocity;

	if (PLAYER_IS_COLLIDING) {

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++) {

			camera.y -= vertical_velocity / 10;
		}

		// jump (only when grounded)
		if (up && vertical_velocity < 0) {
			vertical_velocity = 0.2;
		} else {
			vertical_velocity = -0.01;
		}

	} else {

		// gravity
		vertical_velocity -= 0.01;
	}

	// breaking blocks
	if (BOOL_look_block && attack) {

		printf("%d\n", look_block_ticks_to_break);

		if (look_block_ticks_to_break == 0) {

			set_block_at(look_block_x, look_block_y, look_block_z, 0);

		} else {

			look_block_ticks_to_break--;
		}
	}

	// update look block every tick (since any movement, i.e. mouse, running,
	// being knocked back, etc, and also having broken a block, can influence it)
	int prev_look_block_x = look_block_x;
	int prev_look_block_y = look_block_y;
	int prev_look_block_z = look_block_z;

	BOOL_look_block = raycast_blocks(&camera, 5.0, FALSE, &look_block_x, &look_block_y, &look_block_z);

	if (BOOL_look_block && (prev_look_block_x != look_block_x || prev_look_block_y != look_block_y || prev_look_block_z != look_block_z)) {

		look_block_ticks_to_break = get_block_type(get_block_at(look_block_x, look_block_y, look_block_z))->ticks_to_break;
	}

	// draw everything
	draw_mesh(&camera, &miku_transform, miku_mesh);
	draw_chunks(&camera);
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

	else if (event.type == SDL_MOUSEBUTTONDOWN) {

		if (event.button.button == 1) { // LMB
			attack = TRUE;
		}

		else if (event.button.button == 3) { // RMB

			int hit_x, hit_y, hit_z;

			if (raycast_blocks(&camera, 5.0, TRUE, &hit_x, &hit_y, &hit_z)) {

				set_block_at(hit_x, hit_y, hit_z, 1);
			}
		}
	}

	else if (event.type == SDL_MOUSEBUTTONUP) {

		if (event.button.button == 1) { // LMB
			attack = FALSE;
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