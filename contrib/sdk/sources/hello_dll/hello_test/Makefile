CC = kos32-gcc
LD = kos32-ld 

SDK_DIR:= $(abspath ../../../)

LDFLAGS = -static -S -Tapp-dynamic.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib

SOURCES = hello.c	\
	  $(NULL)
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

default: hello

hello: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o hello $(OBJECTS) -lgcc -lc.dll -lhello.dll
	objcopy hello -O binary

%.o : %.c Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
