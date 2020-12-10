CC = kos32-gcc
LD = kos32-ld
SDK_DIR = ../../../sdk

LIB_DIR:= $(SDK_DIR)/lib

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wmissing-prototypes -Wno-format
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES= -I. -I../bfd -I../include -I$(SDK_DIR)/sources/newlib/libc/include 

DEFINES=  -DHAVE_CONFIG_H -DLOCALEDIR='"/home/autobuild/tools/win32/share/locale"'
DEFINES+= -Dbin_dummy_emulation=bin_vanilla_emulation

LIBS= -lc -lbfd -liberty -lz -lgcc -lc.dll

LIBPATH:= -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 12582912 -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native


SRCS = \
	ar.c arlex.c arparse.c arsup.c	\
	binemul.c bucomm.c debug.c	\
	emul_vanilla.c filemode.c ieee.c\
	is-strip.c not-ranlib.c		\
	not-strip.c objcopy.c rdcoff.c	\
	rddbg.c rename.c stabs.c	\
	version.c wrstabs.c

OCOPY = \
	objcopy.o not-strip.o rename.o	\
	rddbg.o debug.o stabs.o ieee.o	\
	rdcoff.o wrstabs.o bucomm.o	\
	version.o filemode.o

OAR = \
	arparse.o arlex.o ar.o 		\
	not-ranlib.o arsup.o rename.o	\
	binemul.o emul_vanilla.o 	\
	bucomm.o version.o filemode.o

OSTRIP = \
	objcopy.o is-strip.o rename.o	\
	rddbg.o debug.o stabs.o ieee.o	\
	rdcoff.o wrstabs.o bucomm.o	\
	version.o filemode.o

# targets

all: objcopy ar strip

objcopy: $(OCOPY) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OCOPY) $(LIBS)
	kos32-objcopy $@ -O binary

ar: $(OAR) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OAR) $(LIBS)
	kos32-objcopy $@ -O binary

strip: $(OSTRIP) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OSTRIP) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

 
