#include "main.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>
#include <math.h>

// player
static Transform camera;
static float vertical_velocity;

static int looking_at_block;
static int look_block_x, look_block_y, look_block_z;
static unsigned short look_block_ticks_to_break;

// misc
static Transform miku_transform;
static Mesh miku_mesh;
static Mesh sprite_mesh;
static Mesh sky_mesh;

void initialize_player() {

	// position player
	camera.x = 8;
	camera.z = 8;
	camera.y = 30;

	// create a mesh for testing
	miku_mesh = create_mesh_from_obj("res/miku.obj", load_texture("res/dirt.png"));
	miku_transform.x = 8;
	miku_transform.z = 8;
	miku_transform.y = 30;

	// create a sprite mesh for testing
	sprite_mesh = create_sprite_mesh(0.0f, 0.0f, 0.4f, load_texture("res/dirt.png"));

	sky_mesh = create_sky_mesh();
}

void player_process_tick(Input *input) {

	miku_transform.yaw += 0.01;

	// player camera control
	camera.pitch += input->camera_dy * 0.01;
	camera.yaw += input->camera_dx * 0.01;

	// clamp camera pitch
	if (camera.pitch > M_PI / 2) {
		camera.pitch = M_PI / 2;
	} else if (camera.pitch < -M_PI / 2) {
		camera.pitch = -M_PI / 2;
	}

	// placing blocks
	if (input->use) {

		int hit_x, hit_y, hit_z;

		if (raycast_blocks(&camera, 5.0, 1, &hit_x, &hit_y, &hit_z))
			set_block_at(hit_x, hit_y, hit_z, 1);
	}

	// player control
	#define PLAYER_RADIUS 0.4
	#define PLAYER_HEIGHT 1.7
	#define PLAYER_CAM_H 1.4

	#define PLAYER_IS_COLLIDING does_aabb_intersect_blocks(camera.x, camera.y - PLAYER_CAM_H, camera.z, PLAYER_RADIUS, PLAYER_HEIGHT)

	// move in direction of input (crucially, splitting movement into its components to allow for sliding)
	// if colliding, step in opposite direction in small increments until no longer collision (or completely undid movement + a little to prevent float-error related stuckage)
	if (input->left) {

		camera.z -= sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z += sin(camera.yaw) * 0.01;

		camera.x -= cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x += cos(camera.yaw) * 0.01;

	} else if (input->right) {

		camera.z += sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z -= sin(camera.yaw) * 0.01;

		camera.x += cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x -= cos(camera.yaw) * 0.01;
	}

	if (input->forward) {

		camera.z -= cos(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.z += cos(camera.yaw) * 0.01;

		camera.x += sin(camera.yaw) * 0.1;

		for (int i=0; PLAYER_IS_COLLIDING && i < 11; i++)
			camera.x -= sin(camera.yaw) * 0.01;

	} else if (input->backward) {

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
		if (input->up && vertical_velocity < 0) {
			vertical_velocity = 0.2;
		} else {
			vertical_velocity = -0.01;
		}

	} else {

		// gravity
		vertical_velocity -= 0.01;
	}

	// breaking blocks
	if (looking_at_block && input->attack) {

		printf("%d\n", look_block_ticks_to_break);

		if (look_block_ticks_to_break == 0) {

			set_block_at(look_block_x, look_block_y, look_block_z, 0);

		} else {

			look_block_ticks_to_break--;
		}
	}

	// update look block every tick (since any movement, i.e. camera turning, running,
	// being knocked back, etc, and also having broken a block, can influence it)
	int prev_look_block_x = look_block_x;
	int prev_look_block_y = look_block_y;
	int prev_look_block_z = look_block_z;

	looking_at_block = raycast_blocks(&camera, 5.0, 0, &look_block_x, &look_block_y, &look_block_z);

	if (looking_at_block && (prev_look_block_x != look_block_x || prev_look_block_y != look_block_y || prev_look_block_z != look_block_z)) {

		look_block_ticks_to_break = get_block_type(get_block_at(look_block_x, look_block_y, look_block_z))->ticks_to_break;
	}

	// render the sky as just a sprite covering the whole screen
	draw_sky_mesh(&camera, sky_mesh);

	// draw worldly stuff
	draw_mesh(&camera, &miku_transform, miku_mesh);
	draw_chunks(&camera);

	// draw UI
	draw_sprite_mesh(sprite_mesh);
}