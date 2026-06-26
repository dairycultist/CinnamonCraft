// literally just an abstraction layer for SDL + OpenGL

// if you want to port this game to something that doesn't support SDL/OpenGL, or
// otherwise handles rendering, input, file loading, etc differently, you should
// only have to reimplement this file

// TODO merge renderer.c/h into this (load, create, draw meshes)

#include "main.h"
#include "io.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static SDL_Window *window;
static int running = TRUE;

static void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

void initialize_io() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_error("Could not initialize SDL");
		exit(1);
	}

	// init OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// create the window
	window = SDL_CreateWindow("CinnamonCraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	if (!window) {
        log_error("Could not create window");
		exit(1);
    }

	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glClearColor(1.0f, 0.188f, 0.647f, 1.0f); // since the sky is rendered as a mesh, set the clear color to hot pink so it's obvious

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

int io_keep_program_alive() {

    return running;
}

void io_populate_input(Input *input) {

    input->camera_dx = 0.0;
    input->camera_dy = 0.0;

    input->use = FALSE; // TODO replace with logical cooldown (for block placement)

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {

            running = FALSE;
        
        } else if (event.type == SDL_MOUSEMOTION) {

            input->camera_dx = (int) event.motion.xrel; // Sint32
            input->camera_dy = (int) event.motion.yrel;

        } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {

            if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                input->left = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                input->right = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                input->forward = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                input->backward = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                input->up = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
                input->down = TRUE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());
            }

        } else if (event.type == SDL_KEYUP) {

            if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                input->left = FALSE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                input->right = FALSE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
                input->forward = FALSE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
                input->backward = FALSE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                input->up = FALSE;
            } else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
                input->down = FALSE;
            }

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {

            if (event.button.button == 1) { // LMB
                input->attack = TRUE;
            } else if (event.button.button == 3) { // RMB
                input->use = TRUE;
            }
        }

        else if (event.type == SDL_MOUSEBUTTONUP) {

            if (event.button.button == 1) { // LMB
                input->attack = FALSE;
            } else if (event.button.button == 3) { // RMB
                input->use = FALSE;
            }
        }
    }
}

void io_present() {

    SDL_GL_SwapWindow(window);
    SDL_Delay(1000 / 60);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}