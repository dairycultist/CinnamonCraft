#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "resources.c" // binary, automatically updated with new resources on Make (might replace with external loading since modding fun yay)
#include "../../util.c"
#include "3D.c"
#include "game.c"

void log_error(const char *msg) {
	
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
	glFrontFace(GL_CW);

	// initialize 3D
	initialize_shader();
	initialize_perspective(2.0);
	
	// let programmer initialize stuff
	on_start();

	// process events until window is closed
	SDL_Event event;
	int running = TRUE;

	while (running) {

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				running = FALSE;
			} else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

				glViewport(0, 0, event.window.data1, event.window.data2);
				initialize_perspective(event.window.data1 / (float) event.window.data2);
			
			} else {
				process_event(event);
			}
		}

		process_tick();

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