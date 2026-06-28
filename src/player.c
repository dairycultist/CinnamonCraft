#include "main.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>
#include <math.h>

#define PLAYER_RADIUS 0.4
#define PLAYER_HEIGHT 1.7
#define PLAYER_CAM_H 1.5

// player (aabb determines position, camera just follows that; camera determines rotation)
static Transform camera;
static AABB aabb = { 0.0, 0.0, 0.0, PLAYER_RADIUS, PLAYER_HEIGHT };

static float dsway;  // sway is the local left-right axis
static float dsurge; // surge is the local forward-backward axis
static float dheave; // heave is the local up-down axis

static int looking_at_block;
static int lookblock_x, lookblock_y, lookblock_z;
static unsigned short lookblock_breakticks_left;
static unsigned short lookblock_breakticks_total;

// misc
static Mesh crosshair_mesh;
static Texture crosshair_textures[9];
static Mesh sky_mesh;

void initialize_player() {

	// position player
	aabb.x = 8;
	aabb.y = 30;
	aabb.z = 8;

	// create the crosshair
	crosshair_textures[0] = load_texture("res/crosshair_0.png");
	crosshair_textures[1] = load_texture("res/crosshair_1.png");
	crosshair_textures[2] = load_texture("res/crosshair_2.png");
	crosshair_textures[3] = load_texture("res/crosshair_3.png");
	crosshair_textures[4] = load_texture("res/crosshair_4.png");
	crosshair_textures[5] = load_texture("res/crosshair_5.png");
	crosshair_textures[6] = load_texture("res/crosshair_6.png");
	crosshair_textures[7] = load_texture("res/crosshair_7.png");
	crosshair_textures[8] = load_texture("res/crosshair_8.png");
	crosshair_mesh = create_sprite_mesh(0.0f, 0.0f, 0.5f, 0.5f, 64, crosshair_textures[0]);

	sky_mesh = create_sky_mesh();
}

void player_process_tick(Input *input) {

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

		if (raycast_blocks(&camera, 5.0, 1, &hit_x, &hit_y, &hit_z) && !would_aabb_intersect_block_at(hit_x, hit_y, hit_z, 1, &aabb))
			set_block_at(hit_x, hit_y, hit_z, 1);
	}

	// breaking blocks
	if (looking_at_block && input->attack) {

		if (lookblock_breakticks_left == 0) {

			set_block_at(lookblock_x, lookblock_y, lookblock_z, 0);

		} else {

			lookblock_breakticks_left--;
			mesh_set_texture(crosshair_mesh, crosshair_textures[8 - 8 * lookblock_breakticks_left / lookblock_breakticks_total]);
		}

	} else {

		lookblock_breakticks_left = lookblock_breakticks_total;
		mesh_set_texture(crosshair_mesh, crosshair_textures[0]);
	}

	// move in direction of input (splitting movement into its components to allow for sliding)
	// if colliding, step in opposite direction in small increments until no longer collision
	// (or completely undid movement + a little to prevent float-error related stuckage)
	dsway  = (dsway  * 4.0 + (input->right    - input->left)    * 0.1) / 5.0;
	dsurge = (dsurge * 4.0 + (input->backward - input->forward) * 0.1) / 5.0;
	
	// when dsway/dsurge are really small, dsway/dsurge / 10.0 == 0.0, so we end up clipping ever-so-slightly into the wall
	// so prevent them from being really small
	
	// sway
	if (fabs(dsway) < 0.01) {

		dsway = 0.0;
	
	} else {

		aabb.z += dsway * sin(camera.yaw);
		for (int i = 0; does_aabb_intersect_blocks(&aabb) && i <= 10; i++)
			aabb.z -= dsway * sin(camera.yaw) / 10.0;

		aabb.x += dsway * cos(camera.yaw);
		for (int i = 0; does_aabb_intersect_blocks(&aabb) && i <= 10; i++)
			aabb.x -= dsway * cos(camera.yaw) / 10.0;
	}

	// surge
	if (fabs(dsurge) < 0.01) {

		dsurge = 0.0;

	} else {
		
		aabb.z += dsurge * cos(camera.yaw);
		for (int i = 0; does_aabb_intersect_blocks(&aabb) && i <= 10; i++)
			aabb.z -= dsurge * cos(camera.yaw) / 10.0;

		aabb.x -= dsurge * sin(camera.yaw);
		for (int i = 0; does_aabb_intersect_blocks(&aabb) && i <= 10; i++)
			aabb.x += dsurge * sin(camera.yaw) / 10.0;
	}

	// heave
	aabb.y += dheave;

	if (does_aabb_intersect_blocks(&aabb)) {

		for (int i = 0; does_aabb_intersect_blocks(&aabb) && i <= 10; i++)
			aabb.y -= dheave / 10.0;

		// jump (only when grounded)
		if (input->up && dheave < 0) {
			dheave = 0.2;
		} else {
			dheave = -0.01;
		}

	} else {

		// gravity
		dheave -= 0.01;
	}

	// update camera position
	camera.x = aabb.x;
	camera.y = aabb.y + PLAYER_CAM_H;
	camera.z = aabb.z;

	// update look block every tick (since any movement, i.e. camera turning, running,
	// being knocked back, etc, and also having broken a block, can influence it)
	int prev_lookblock_x = lookblock_x;
	int prev_lookblock_y = lookblock_y;
	int prev_lookblock_z = lookblock_z;

	looking_at_block = raycast_blocks(&camera, 5.0, 0, &lookblock_x, &lookblock_y, &lookblock_z);

	if (looking_at_block && (prev_lookblock_x != lookblock_x || prev_lookblock_y != lookblock_y || prev_lookblock_z != lookblock_z)) {

		lookblock_breakticks_total = get_block_ticks_to_break(get_block_at(lookblock_x, lookblock_y, lookblock_z));
		lookblock_breakticks_left = lookblock_breakticks_total;
	}

	// render the sky as just a sprite covering the whole screen
	draw_sky_mesh(&camera, sky_mesh);

	// draw worldly stuff
	draw_chunks(&camera);
	draw_entities(&camera);

	// draw UI
	draw_sprite_mesh(crosshair_mesh);
}