CC = gcc
DEPS = bmp.h
SOURCE = bmp.c
OBJ = bmp.o

bmplib.a: $(OBJ)
	ar rs bmplib.a $(OBJ)

$(OBJ): $(SOURCE) $(DEPS)
	$(CC) -Wall -c $(SOURCE)

clean: 
	rm $(OBJ) bmplib.a