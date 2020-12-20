CC = kos32-gcc

SDK_DIR = ../../../../contrib/sdk
CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I .

SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	ar rcs libmujs.a *.o
	rm *.o

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
