CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= /home/programist/KOS_SVN/contrib/sdk
CLAYER:= /home/programist/KOS_SVN/contrib/C_Layer

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app-dynamic.lds \
          --image-base 0 -lquirc -lgcc -ldll -lc.dll 

# caution: without -mno-ms-bitfields below, pagefault occurs (why ? idk)
CFLAGS = -g -U_Win32 -U_WIN32 -U__MINGW32__ -std=c99 -mno-ms-bitfields

INCLUDES= -I. -Ilib -I$(SDK_DIR)/sources/newlib/libc/include -I$(CLAYER)/INCLUDE
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L.

OBJPATH = $(CLAYER)/OBJ

SOURCES = qr_tool.c

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

all: qr_tool

qr_tool: $(OBJECTS)
	$(LD) $(LIBPATH) --subsystem windows -o $@ $^ $(OBJPATH)/loadboxlib.obj $(OBJPATH)/loadproclib.obj $(OBJPATH)/loadlibimg.obj $(LDFLAGS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o qr_tool
