client_app: client/src/*
	@gcc -o client_app client/src/main.c \
	client/GLEW/glew.o -framework OpenGL \
	-lSDL2

# server_app next

run: client_app
	@./client_app

clean:
	rm client_app