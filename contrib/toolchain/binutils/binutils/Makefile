NAME= objcopy-new

LIB_DIR:= $(SDK_DIR)/lib

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wmissing-prototypes -Wno-format
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES= -I. -I../bfd -I../include -I$(SDK_DIR)/sources/newlib/libc/include 

DEFINES=  -DHAVE_CONFIG_H -DLOCALEDIR='"/home/autobuild/tools/win32/share/locale"'
DEFINES+= -Dbin_dummy_emulation=bin_vanilla_emulation

LIBS= -lbfd -liberty -lz -lgcc -lc.dll -lapp

LIBPATH:= -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 12582912 -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0


SRCS = \
	objcopy.c not-strip.c rename.c	\
	rddbg.c debug.c stabs.c ieee.c	\
	rdcoff.c wrstabs.c bucomm.c	\
	version.c filemode.c



OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: $(NAME)

$(NAME): $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJS) $(LIBS)
	kos32-objcopy $@ kos32-objcopy -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

 