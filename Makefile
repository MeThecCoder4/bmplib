bmplib.a: bmp.o
	ar rs bmplib.a bmp.o

bmp.o: bmp.c
	gcc -Wall -c bmp.c

clean: 
	rm bmp.o bmplib.a