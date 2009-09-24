SRC = libGUI.c
TARGET = libGUI.obj
CFLAGS = -O2 -nostdinc -nostdlib -march=pentium -fomit-frame-pointer -fno-builtin -fno-builtin-printf
CC = gcc

all:
	$(CC) -c $(SRC) $(CFLAGS) -o $(TARGET)
	strip -x -S $(TARGET)
	$(CC) -S $(SRC) $(CFLAGS)
clean:
	rm $(TARGET) 
