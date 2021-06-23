CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -DSQLITE_OS_OTHER=1 -DHAVE_UNISTD_H=0 -D_NO_STDERR -DSQLITE_OMIT_POPEN -DSQLITE_THREADSAFE=0 -D_KOLIBRI -U__linux__ -DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.36.0\" -DPACKAGE_STRING=\"sqlite\ 3.36.0\" -DPACKAGE_BUGREPORT=\"http://www.sqlite.org\" -DPACKAGE_URL=\"\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.36.0\"
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include -I..
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib -L..


# Only selected
SRC =  shell.c stub.c
# All .c files
# SRC = $(notdir $(wildcard *.c))

OBJECTS = $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem console -o sqlite3 $(OBJECTS) -lgcc -lsqlite3.dll -lc.dll
	strip -S sqlite3
	objcopy sqlite3 -O binary

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
