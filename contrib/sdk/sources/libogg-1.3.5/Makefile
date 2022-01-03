CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

LIBNAME=libogg

SDK_DIR:= $(abspath ../../)

OBJS = src/bitwise.o src/framing.o

CFLAGS = -c -O2 -mpreferred-stack-boundary=2 -fno-ident -fomit-frame-pointer -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -fno-exceptions -fno-asynchronous-unwind-tables -ffast-math -mno-ms-bitfields -march=pentium-mmx -UWIN32 -U_Win32 -U_WIN32 -U__MINGW32__ -I../newlib/libc/include -Iinclude -DHAVE_CONFIG

all: $(LIBNAME).a 

$(LIBNAME).a: $(OBJS)
	$(AR) -crs $(SDK_DIR)/lib/$(LIBNAME).a $(OBJS)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f */*.o \ rm *.o \ rm */*/*.o
