CC = wine bcc32
MSVC = wine link
LD = kos32-ld

SDK_DIR = $(abspath ../../../contrib/sdk)

LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

SRC = main.c

OBJ = $(patsubst %.c, %.obj, $(SRC))

default: $(patsubst %.c,%.obj,$(SRC))
	$(MSVC) -edit $(OBJ)
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem console -o main $(OBJ) -lgcc -lc.dll
	objcopy main -O binary

%.obj : %.c Makefile $(SRC)
	$(CC) -c $(INCLUDES) $<

clean:
	rm *.o
