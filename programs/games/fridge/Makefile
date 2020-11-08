CC = kos32-gcc
LD = kos32-ld 

CONTRIB_DIR:= $(abspath ../../../contrib)
SDK_DIR:= $(CONTRIB_DIR)/sdk
C_LAYER:= $(CONTRIB_DIR)/C_Layer

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include -I $(C_LAYER)/INCLUDE
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SOURCES = fridge.c	\
	  $(NULL)
 

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

default: fridge.kex

fridge.kex: $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) --subsystem native -o fridge.kex $(OBJECTS) $(C_LAYER)/OBJ/loadlibimg.obj -lgcc -lc.dll
	objcopy fridge.kex -O binary
	rm fridge.o

%.o : %.c Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
