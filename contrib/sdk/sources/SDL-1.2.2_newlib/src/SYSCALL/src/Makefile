CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../../../)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I ../include -I $(SDK_DIR)/sources/newlib/libc/include

SYSCALL_SRC := $(notdir $(wildcard *.c))
OBJECTS =  $(patsubst %.c, %.o, $(SYSCALL_SRC))

default: $(patsubst %.c,%.o,$(SYSCALL_SRC))

%.o : %.c Makefile $(SYSCALL_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
