client_app: client/src/* client/res/*
	@gcc -o client/res/temp client/res/resloader.c # create/update resources.c
	@cd client/res/; ./temp # need to be cd'd into the res folder so that the resloader has correct relative access to resource files
	@ rm -f client/res/temp
	@gcc -o client_app client/src/main.c client/GLEW/glew.o -framework OpenGL -lSDL2

# server_app next

run: client_app
	@./client_app

clean:
	rm client_app