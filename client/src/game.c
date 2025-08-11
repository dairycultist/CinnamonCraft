#include <math.h>

Transform camera;

Model *model_test;

ChunkModel chunk;

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

	// create a model for testing
	model_test = create_model(miku_mesh, miku_mesh_bytecount, miku_mesh_vertcount, dirt_texture, 16, 16);

	// create a chunk for testing
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			for (int z = 0; z < 16; z++)
				chunk.blocks[x][y][z] = random_uint(5) ? 0 : 1;

	remesh_chunk(&chunk);
}

void on_terminate() {

	free(model_test);
}

int is_point_inside_block(float x, float y, float z) {

	if (x < 0 || y < 0 || z > 0 || x > 16 || y > 16 || z < -16)
		return FALSE;

	return chunk.blocks[(int) floor(x)][(int) floor(y)][(int) floor(-z)];
}

int is_aabb_cube_inside_block(float x, float y, float z, float size) {

	// gonna use unit aabb for now
	return is_point_inside_block(x - size, y - size, z - size)
	    || is_point_inside_block(x - size, y - size, z + size)
		|| is_point_inside_block(x - size, y + size, z - size)
		|| is_point_inside_block(x - size, y + size, z + size)
		|| is_point_inside_block(x + size, y - size, z - size)
		|| is_point_inside_block(x + size, y - size, z + size)
		|| is_point_inside_block(x + size, y + size, z - size)
		|| is_point_inside_block(x + size, y + size, z + size);
}

void process_tick() {

	// move in direction of input
	// if colliding, step in opposite direction in small increments (10) until no longer collision (or completely undid movement)
	// doesn't allow sliding against walls ugh

	const float size = 0.2;

	if (left) {

		camera.z -= sin(camera.yaw) * 0.1;
		camera.x -= cos(camera.yaw) * 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.z += sin(camera.yaw) * 0.01;
				camera.x += cos(camera.yaw) * 0.01;
			}
		}

	} else if (right) {

		camera.z += sin(camera.yaw) * 0.1;
		camera.x += cos(camera.yaw) * 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.z -= sin(camera.yaw) * 0.01;
				camera.x -= cos(camera.yaw) * 0.01;
			}
		}
	}

	if (forward) {

		camera.z -= cos(camera.yaw) * 0.1;
		camera.x += sin(camera.yaw) * 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.z += cos(camera.yaw) * 0.01;
				camera.x -= sin(camera.yaw) * 0.01;
			}
		}

	} else if (backward) {

		camera.z += cos(camera.yaw) * 0.1;
		camera.x -= sin(camera.yaw) * 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.z -= cos(camera.yaw) * 0.01;
				camera.x += sin(camera.yaw) * 0.01;
			}
		}
	}

	if (up) {

		camera.y += 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.y -= 0.01;
			}
		}

	} else if (down) {

		camera.y -= 0.1;

		if (is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size)) {

			for (int i=0; i<10 && is_aabb_cube_inside_block(camera.x, camera.y, camera.z, size); i++) {

				camera.y += 0.01;
			}
		}
	}

	model_test->transform.yaw += 0.01;

	draw_model(&camera, model_test);
	draw_model(&camera, &chunk.model);
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