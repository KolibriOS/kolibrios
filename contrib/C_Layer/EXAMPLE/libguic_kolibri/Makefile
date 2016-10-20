NAME=boardmsg
CC = kos32-gcc
LD = kos32-ld

SDK_DIR:= $(abspath ../../../sdk)
CLAYER:= $(abspath ../../)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/lib/app-dynamic.lds \
          --image-base 0 -lgcc -ldll -lc.dll 

CFLAGS = -g -U_Win32 -U_WIN32 -U__MINGW32__ -std=gnu90 -mno-ms-bitfields

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(CLAYER)/INCLUDE
LIBPATH:= -L $(SDK_DIR)/lib 

OBJPATH = $(CLAYER)/OBJ

SOURCES =    boardmsg.c option_menu.c

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))

all: boardmsg option_menu scroll_progress dbutton_files editor_tree_msgbox

editor_tree_msgbox: editor_tree_msgbox.o
	$(LD) $(LIBPATH) --subsystem windows -o $@ $^ $(OBJPATH)/loadboxlib.obj $(OBJPATH)/loadlibimg.obj $(OBJPATH)/loadmsgbox.obj $(LDFLAGS) -Map=editor_tree_msgbox.map
	kos32-objcopy $@ -O binary

scroll_progress: scroll_progress.o
	$(LD) $(LIBPATH) --subsystem windows -o $@ $^ $(OBJPATH)/loadboxlib.obj $(LDFLAGS)
	kos32-objcopy $@ -O binary


option_menu: option_menu.o
	$(LD) $(LIBPATH) --subsystem native -o $@ $^ $(OBJPATH)/loadboxlib.obj $(LDFLAGS)
	kos32-objcopy $@ -O binary

boardmsg: boardmsg.o
	$(LD) $(LIBPATH) --subsystem native -o $@ $^ $(OBJPATH)/loadboxlib.obj $(LDFLAGS) -Map=boardmsg.map
#	strip $@
	kos32-objcopy $@ -O binary

dbutton_files: dbutton_files.o
	$(LD) $(LIBPATH) --subsystem native -o $@ $^ $(OBJPATH)/loadboxlib.obj $(OBJPATH)/loadproclib.obj $(OBJPATH)/loadlibimg.obj $(LDFLAGS) 
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	-rm -f *.o
