all: raycaster

raycaster: raycaster.c
	cc -g -Ofast -o raycaster soil/*.c raycaster.c -framework OpenGL `sdl2-config --cflags --libs` -lSDL2_image -lSOIL -lGLU
