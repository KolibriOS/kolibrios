CC = kos32-gcc
LD = kos32-ld 

CONTRIB_DIR = $(abspath ../../../contrib)
SDK_DIR = $(CONTRIB_DIR)/sdk
C_LAYER = $(CONTRIB_DIR)/C_Layer

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
LDFLAGS = -static -S -Tapp-dynamic.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I $(C_LAYER)/INCLUDE
LIBPATH = -L $(SDK_DIR)/lib

SOURCES = fridge.c
OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

default: hello

hello: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o fridge $(OBJECTS) $(C_LAYER)/OBJ/loadlibimg.obj -lgcc -lc.dll
	strip -s fridge -o fridge
	objcopy fridge -O binary

%.o : %.c Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
