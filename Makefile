client_app: client/src/* client/res/*
	@gcc -o client/res/temp client/src/resloader.c # create/update resources.c
	@cd client/res/; ./temp # need to be cd'd into the res folder so that the resloader has correct relative access to resource files
	@rm -f client/res/temp
	@gcc -o client_app client/src/main.c client/src/mod.c -lGLEW -framework OpenGL $(shell pkg-config --cflags --libs sdl2 SDL2_image)

# server_app next

run: client_app
	@./client_app

clean:
	rm client_app