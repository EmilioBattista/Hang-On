CFLAGS = -Wall -std=c99 -pedantic -Werror -g
LDIBS = -lm

all: hangon

hangon: main.o imagen.o pixel.o paleta.o fondo.o ruta.o
	gcc main.o imagen.o pixel.o paleta.o fondo.o ruta.o -I include -L lib -l SDL2-2.0.0 -o hangon $(CFLAGS) $(LDIBS)

main.o: main.c imagen.h paleta.h fondo.h pixel.h
	gcc -c main.c $(CFLAGS)

imagen.o: imagen.c imagen.h pixel.h
	gcc -c imagen.c $(CFLAGS)

pixel.o: pixel.c pixel.h
	gcc -c pixel.c $(CFLAGS)

paleta.o: paleta.c paleta.h
	gcc -c paleta.c $(CFLAGS)

fondo.o: fondo.c fondo.h
	gcc -c fondo.c $(CFLAGS)

ruta.o: ruta.c ruta.h
	gcc -c ruta.c $(CFLAGS)

clean:
	rm -f *.o
	rm -f hangon
