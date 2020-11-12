CC = kos32-gcc
LD = kos32-ld 

SDK_DIR = $(abspath ../..)

LDFLAGS = -call_shared -nostdlib -T $(SDK_DIR)/sources/newlib/app-dynamic.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/libstdc++-v3/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SOURCES = hello.cpp   \
      $(NULL)

OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES))

default: hello

hello: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem console -o hello.kex $(OBJECTS) -lc.dll -lstdc++ -lsupc++ -lgcc -lc
	objcopy hello.kex -O binary
	rm *.o

%.o : %.cpp Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
