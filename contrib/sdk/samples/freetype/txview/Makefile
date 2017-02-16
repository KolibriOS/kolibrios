
CC = kos32-gcc
LD = kos32-ld 
AR = kos32-ar

SDK_DIR:= $(abspath ../../..)
CONTRIB_DIR:= $(abspath ../../../..)

LDFLAGS = -static --subsystem native -Tapp-dynamic.lds -Map txview.map --image-base 0

CFLAGS = -c -O2 -msse2 -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32

INCLUDES= -I./winlib -I./pxdraw -I $(SDK_DIR)/sources/newlib/libc/include -I $(SDK_DIR)/sources/freetype/include
INCLUDES+= -I $(CONTRIB_DIR)/toolchain/binutils/bfd -I $(CONTRIB_DIR)/toolchain/binutils/include
LIBPATH:= -L./ -L $(SDK_DIR)/lib

LIB_SRCS= 			\
	  pxdraw/context.c	\
	  pxdraw/dutils.c	\
	  pxdraw/region.c	\
	  winlib/button.c	\
	  winlib/winlib.c	\
	  $(NULL)

SOURCES = main.c	\
	  fontlib.c	\
	  tview.c	\
	  $(NULL)
 
LIB_OBJS =  $(patsubst %.c, %.o, $(LIB_SRCS))

OBJECTS =  $(patsubst %.c, %.o, $(SOURCES))


default: txview

libwin.a: $(LIB_OBJS) Makefile
	$(AR) crs $@ $(LIB_OBJS)

txview: $(OBJECTS) libwin.a Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o txview.dll $(OBJECTS) -lfreetype.dll -lpixlib3 -lwin -lgcc -lc.dll
#	objdump -d txview.dll > txview.lst
	objcopy txview.dll txview -O binary

clean:
	/bin/rm -rf *.o txview

%.o : %.c Makefile $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<
