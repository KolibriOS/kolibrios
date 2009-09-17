SRC = libGUI.c
TARGET = libGUI.obj
CFLAGS = -O2 -nostdinc -nostdlib -march=pentium -fomit-frame-pointer -fno-builtin
CC = gcc

all:
	$(CC) -c $(SRC) $(CFLAGS) -o $(TARGET)
	strip -X --strip-unneeded $(TARGET)
clean:
	rm *.obj 
	rm $(TARGET) 
