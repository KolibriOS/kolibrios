SRC = libGUI.c
TARGET = libGUI.obj
CFLAGS = -O2 -nostdinc -nostdlib -march=pentium -fomit-frame-pointer -fno-builtin
CC = gcc
AR=ar

all:
	$(CC) -c $(SRC) $(CFLAGS) -o $(TARGET)
clean:
	rm *.obj 
	rm $(TARGET) 
