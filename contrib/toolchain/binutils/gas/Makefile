
LIB_DIR:= $(SDK_DIR)/lib

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wmissing-prototypes -Wno-format
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES= -I. -I../bfd -I./config -I../include -I../
INCLUDES+= -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/zlib 

DEFINES=  -DHAVE_CONFIG_H -DLOCALEDIR='"/home/autobuild/tools/win32/share/locale"'

LIBS= -lopcodes -lbfd -liberty -lz -lgcc -lc.dll -lapp

LIBPATH:= -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 12582912 -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native


SRCS = \
	app.c as.c atof-generic.c compress-debug.c	\
	cond.c depend.c	dwarf2dbg.c dw2gencfi.c ecoff.c	\
	ehopt.c expr.c flonum-copy.c flonum-konst.c	\
	flonum-mult.c frags.c hash.c input-file.c	\
	input-scrub.c listing.c	literal.c macro.c	\
	messages.c output-file.c read.c remap.c sb.c	\
	stabs.c subsegs.c symbols.c write.c		\
	config/atof-ieee.c config/obj-coff.c		\
	config/tc-i386.c

OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: as

as: $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJS) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

 