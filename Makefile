UNAME := $(shell uname)

ifeq ($(UNAME), Darwin) # macOS
	GCCFLAGS = -lGLEW -framework OpenGL
else ifeq ($(UNAME), Linux)
	GCCFLAGS = -lGLEW -lGL -lm
else
	$(error Unsupported OS: $(UNAME))
endif

client_app: client/src/* client/res/*
	@gcc -o client_app client/src/*.c $(GCCFLAGS) $(shell pkg-config --cflags --libs sdl2 SDL2_image)

# server_app next

run: client_app
	@./client_app

clean:
	rm client_app