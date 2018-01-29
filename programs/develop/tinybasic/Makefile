CC = kos32-gcc
LD = kos32-ld 

SDK_DIR:= $(abspath ../../../contrib/sdk)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SOURCES = TinyBasic.c	\
	  $(NULL)
 
OUTFILE= TinyBasic

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

default: hello

hello: $(OBJECTS) 
	$(LD) $(LDFLAGS) $(LIBPATH) -o $(OUTFILE) $(OBJECTS) -lgcc -lc.dll -lapp -lgcc_eh  -Map=$(OUTFILE).map
	kos32-strip $(OUTFILE)
	kos32-objcopy $(OUTFILE) -O binary

%.o : %.c $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
