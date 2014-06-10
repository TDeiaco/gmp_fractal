# makefile for gmp_fractal renderer
# Taylor Deiaco

CC=g++
CFLAGS=-std=c++11 -I"/usr/include"

OBJECTS= main.o SDL_Helper.o fractal.o

LIBS =  -L"/usr/lib"\
	-lSDL -lgmp -lSDL_image

all: gmp_renderer run

gmp_renderer: $(OBJECTS)
	$(CC) -g -o  gmp_renderer $(OBJECTS) $(LIBS)

%.o : %.cpp
	$(CC) $(CFLAGS) -g -c $<

clean: 
	-rm -f *.o

run: 
	./gmp_renderer
