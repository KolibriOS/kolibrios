CC = kos32-gcc
LD = kos32-ld 

CONTRIB_DIR = $(abspath ../../..)
SDK_DIR = $(CONTRIB_DIR)/sdk

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -Wno-pointer-arith
LDFLAGS = -static -S -Tapp-dynamic.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/libstdc++-v3/include
LIBPATH = -L $(SDK_DIR)/lib

SOURCES = hello.cpp
OBJECTS =  $(patsubst %.cpp, %.o, $(SOURCES))

default: hello

hello: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem console -o hello $(OBJECTS) -lstdc++ -lsupc++ -lgcc -lc.dll
	strip -s hello -o hello
	objcopy hello -O binary

%.o : %.cpp Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
