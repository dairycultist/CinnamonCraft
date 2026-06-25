#include "window.h"
#include "renderer.h"
#include "terrain.h"
#include "player.h"

static void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

int main() {

	printf("Starting CinnamonCraft\n");

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_error("Could not initialize SDL");
		return 1;
	}

	// init OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// create the window
	SDL_Window *window = SDL_CreateWindow("CinnamonCraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (!window) {
        log_error("Could not create window");
		return 1;
    }

	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glClearColor(1.0f, 0.188f, 0.647f, 1.0f); // since the sky is rendered as a mesh, set the clear color to hot pink so it's obvious

	SDL_SetRelativeMouseMode(SDL_TRUE);

	// initialize our stuff
	initialize_renderer();
	initialize_terrain();
	initialize_player();

	// process events until window is closed
	SDL_Event event;
	int running = TRUE;

	Sint32 camera_dx = 0.0,
	       camera_dy = 0.0;

	static int left = FALSE, right = FALSE, forward = FALSE, backward = FALSE, up = FALSE, down = FALSE;
	static int attack = FALSE, use = FALSE;

	while (running) {

		camera_dx = 0.0;
		camera_dy = 0.0;

		use = FALSE; // TODO replace with logical cooldown (for block placement)

		// input
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {

				running = FALSE;
			
			} else if (event.type == SDL_MOUSEMOTION) {

				camera_dx = event.motion.xrel;
				camera_dy = event.motion.yrel;

			} else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {

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

			} else if (event.type == SDL_KEYUP) {

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

			} else if (event.type == SDL_MOUSEBUTTONDOWN) {

				if (event.button.button == 1) { // LMB
					attack = TRUE;
				} else if (event.button.button == 3) { // RMB
					use = TRUE;
				}
			}

			else if (event.type == SDL_MOUSEBUTTONUP) {

				if (event.button.button == 1) { // LMB
					attack = FALSE;
				} else if (event.button.button == 3) { // RMB
					use = FALSE;
				}
			}
		}
		
		// player stuff (drawing their perspective, handling their input, etc)
		player_process_tick(camera_dx, camera_dy, left, right, forward, backward, up, down, attack, use);

		SDL_GL_SwapWindow(window);
		SDL_Delay(1000 / 60);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// exit (everything frees automatically, so)
	return 0;
}