NAME=kmenu_example
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= $(abspath ../../../sdk)
CLAYER:= $(abspath ../../)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds \
          --image-base 0 -lgcc -ldll -lc.dll 

CFLAGS = -g -U_Win32 -U_WIN32 -U__MINGW32__  -std=gnu90

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(CLAYER)/INCLUDE
LIBPATH:= -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

OBJPATH = $(CLAYER)/OBJ

SOURCES =    kmenu_example.c

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

all:$(NAME)

$(NAME): $(OBJECTS) Makefile
	$(LD) $(LIBPATH) --subsystem native -o $@ $(OBJECTS) $(OBJPATH)/loadboxlib.obj $(OBJPATH)/loadkmenu.obj $(LDFLAGS) -Map $(NAME).map
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	-rm -f *.o *.map
