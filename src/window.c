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
#define USERNAME "test"

static int sock;

void send_pid(unsigned char pid) {

	send(sock, &pid, 1, 0);
}

void send_string16(const char *string) {

	unsigned char zero = 0x00;
	int len = strlen(string);
	unsigned char len_lower = (unsigned char) len;

	// send length of string in characters (unsigned short, big-endian)
	send(sock, &zero, 1, 0);
	send(sock, &len_lower, 1, 0);

	// UCS-2 (16-bit words) is just ascii for the first 256 values, convenient
	for (int i = 0; i < len; i++) {
		
		send(sock, "\00", 1, 0);
		send(sock, string + i, 1, 0);
	}
}

void send_integer(int value) {

}

void send_long(long value) {

}

void send_byte(char value) {

}

// void read_packet() { // into buffer
	
// 	// char buffer[1024] = {0};
// 	// read(sock, buffer, 1024);
// }

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

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log_error("Could not connect to server");
        return 1;
    }
	
	// log in
	send_pid(pid_PreLogin);
	send_string16(USERNAME);

	send_pid(pid_Login);
	send(sock, "\00\00\00\14", 4, 0);
	send_string16(USERNAME);
	send(sock, "\00\00\00\00\00\00\00\00\00", 9, 0);

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