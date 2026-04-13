/*
 * Creates the window. Other files still need to know about SDL (rendering, keycodes, etc).
 */

#include "window.h"
#include "render.h"
#include "logic.h"

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
	SDL_Window *window = SDL_CreateWindow("CinnamonCraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!window) {
        log_error("Could not create window");
		return 1;
    }

	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	// enable depth buffer
	glEnable(GL_DEPTH_TEST);

	// enable backface culling
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	// initialize rendering
	initialize_shaders();
	initialize_perspective(2.0);

	// logical start
	on_start();

	// process events until window is closed
	SDL_Event event;
	int running = TRUE;

	Sint32 mouse_dx = 0.0,
	       mouse_dy = 0.0;

	static int left = FALSE, right = FALSE, forward = FALSE, backward = FALSE, up = FALSE, down = FALSE;
	static int attack = FALSE, use = FALSE;

	while (running) {

		mouse_dx = 0.0;
		mouse_dy = 0.0;

		use = FALSE; // TODO replace with logical cooldown (for block placement)

		// input
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {

				running = FALSE;

			} else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

				glViewport(0, 0, event.window.data1, event.window.data2);
				initialize_perspective(event.window.data1 / (float) event.window.data2);
			
			} else if (event.type == SDL_MOUSEMOTION) {

				mouse_dx = event.motion.xrel;
				mouse_dy = event.motion.yrel;

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

		// tick
		process_tick(mouse_dx, mouse_dy, left, right, forward, backward, up, down, attack, use);

		SDL_GL_SwapWindow(window);
		SDL_Delay(1000 / 60);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// free everything
	on_terminate();

	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}