all: raycaster

raycaster: raycaster.c
	cc -g -o raycaster raycaster.c -framework OpenGL `sdl2-config --cflags --libs`
