boobcraft: src/*
	@gcc -o boobcraft src/main.c \
	GLEW/glew.o -framework OpenGL \
	-lSDL2

run: boobcraft
	@./boobcraft

clean:
	rm boobcraft