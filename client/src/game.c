Transform *camera;
Mesh *mesh1;
Mesh *mesh2;
Mesh *sky;

int left     = FALSE;
int right    = FALSE;
int forward  = FALSE;
int backward = FALSE;
int up       = FALSE;
int down     = FALSE;

char *get_title() {

	return "Boobcraft";
}

void on_start() {
	
	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	camera = calloc(sizeof(Transform), 1);

	mesh1 = import_mesh("res/miku.obj", "res/miku.ppm", MESH_SHADED);
	mesh2 = import_mesh("res/block.obj", "res/block.ppm", MESH_SHADED);
	sky = import_mesh("res/sky.obj", "res/sky.ppm", MESH_SKY);

	mesh1->transform.z = -2.0;
	mesh1->transform.yaw = M_PI * -0.2;

	mesh2->transform.z = -2.0;
	mesh2->transform.y = -2.3;
}

void on_terminate() {

	free(camera);
	free(mesh1);
	free(mesh2);
	free(sky);
}

void process_tick() {

	if (left) {
		camera->z -= sin(camera->yaw) * 0.1;
		camera->x -= cos(camera->yaw) * 0.1;
	} else if (right) {
		camera->z += sin(camera->yaw) * 0.1;
		camera->x += cos(camera->yaw) * 0.1;
	}

	if (forward) {
		camera->z -= cos(camera->yaw) * 0.1;
		camera->x += sin(camera->yaw) * 0.1;
	} else if (backward) {
		camera->z += cos(camera->yaw) * 0.1;
		camera->x -= sin(camera->yaw) * 0.1;
	}

	if (up) {
		camera->y += 0.1;
	} else if (down) {
		camera->y -= 0.1;
	}

	draw_mesh(camera, sky);
	draw_mesh(camera, mesh1);
	draw_mesh(camera, mesh2);
}

void process_event(SDL_Event event) {

	if (event.type == SDL_MOUSEMOTION) {

		camera->pitch += event.motion.yrel * 0.01;
		camera->yaw += event.motion.xrel * 0.01;

		// clamp camera pitch
		if (camera->pitch > M_PI / 2) {
			camera->pitch = M_PI / 2;
		} else if (camera->pitch < -M_PI / 2) {
			camera->pitch = -M_PI / 2;
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