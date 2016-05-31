all: raycaster

raycaster: raycaster.c
	cc -g -o raycaster soil/*.c raycaster.c -framework OpenGL `sdl2-config --cflags --libs` -lSOIL
