CC = kos32-gcc
LD = kos32-ld

SDK_DIR = $(abspath ../../sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0

INCLUDES = -I $(SDK_DIR)/sources/newlib/libc/include
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib


# Only selected
SRC = RAM.c icache.c MMU.c pxa255_PwrClk.c pxa255_IC.c pxa255_GPIO.c callout_RAM.c rt.c pxa255_RTC.c SoC.c pxa255_UART.c pxa255_TIMR.c mem.c dcache.c pxa255_LCD.c cp15.c pxa255_DMA.c math64.c CPU.c main_pc.c pxa255_DSP.c

# All .c files
# SRC = $(notdir $(wildcard *.c))

OBJECTS = $(patsubst %.c, %.o, $(SRC))

default: $(patsubst %.c,%.o,$(SRC))
	kos32-ld $(LDFLAGS) $(LIBPATH) --subsystem console -o uARM $(OBJECTS) -lgcc -lc.dll
	objcopy uARM -O binary

%.o : %.c Makefile $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm *.o
