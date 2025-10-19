client_app: client/src/* client/res/*
	@gcc -o client_app client/src/main.c client/src/mod.c -lGLEW -framework OpenGL $(shell pkg-config --cflags --libs sdl2 SDL2_image)

# server_app next

run: client_app
	@./client_app

clean:
	rm client_app