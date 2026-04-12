/*
 * Creates the window and server connection. For now, only the server connection is abstracted away from
 * other files; Other files still need to know about SDL (rendering, keycodes, etc).
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "header.h"
#include "logic.h"

static void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 25565

static int sock;

void send_packet() {

}

void read_packet() { // into buffer
	
	// char buffer[1024] = {0};
	// read(sock, buffer, 1024);
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
	
	// let programmer initialize stuff
	on_start();

	// initialize server connection
	sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
		log_error("Could not create socket");
        return 1;
    }

	struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
		log_error("Invalid address");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        log_error("Could not connect to server");
        return 1;
    }

	// MVP client
	// 1. Pre login packet https://pixelbrush.dev/beta-wiki/networking/packets/002-pre-login
	// 2. Login packet https://pixelbrush.dev/beta-wiki/networking/packets/001-login
	// 3. Send KeepAlives forever https://pixelbrush.dev/beta-wiki/networking/packets/000-keep-alive
	
	send(sock, "\02\00\01\00\46", 5, 0);
	send(sock, "\01\00\00\00\14\00\01\00\46\00\00\00\00\00\00\00\00\00", 18, 0);

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
	close(sock);

	on_terminate();

	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}