CC = kos32-gcc
LD = kos32-ld
SDK_DIR = ../../../sdk

NAME= ld-new

LIB_DIR:= $(SDK_DIR)/lib

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wmissing-prototypes -Wno-format -Werror
ELF_CFLAGS = -DELF_LIST_OPTIONS=FALSE -DELF_SHLIB_LIST_OPTIONS=FALSE
ELF_CFLAGS+= -DELF_PLT_UNWIND_LIST_OPTIONS=FALSE
CFLAGS   = -c -save-temps $(CFLAGS_OPT) $(ELF_CFLAGS)

INCLUDES= -I. -I../bfd -I../include -I$(SDK_DIR)/sources/newlib/libc/include 

DEFINES=  -DHAVE_CONFIG_H -DLOCALEDIR='"/home/autobuild/tools/win32/share/locale"'
FDEFS=    -DSCRIPTDIR='"/home/autobuild/tools/win32/mingw32/lib"' -DBINDIR='"/home/autobuild/tools/win32/bin"'
FDEFS+=   -DTOOLBINDIR='"/home/autobuild/tools/win32/mingw32/bin"'
MDEFS=    -DDEFAULT_EMULATION='"i386pe"' -DBINDIR='"/home/autobuild/tools/win32/bin"'
MDEFS+=   -DTOOLBINDIR='"/home/autobuild/tools/win32/mingw32/bin"' -DTARGET='"i686-pc-mingw32"'
MDEFS+=   -DTARGET_SYSTEM_ROOT='"/home/autobuild/tools/win32"' -DTARGET_SYSTEM_ROOT_RELOCATABLE

LIBS= -lc -lbfd -liberty -lz -lgcc -lc.dll

LIBPATH:= -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 12582912 -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native


SRCS = \
	deffilep.c ei386pe.c ldbuildid.c\
	ldcref.c ldctor.c ldemul.c	\
	ldexp.c  ldfile.c ldgram.c	\
	ldlang.c ldlex-wrapper.c	\
	ldmain.c ldmisc.c  ldver.c	\
	ldwrite.c lexsup.c mri.c	\
	pe-dll.c



OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: $(NAME)

$(NAME): $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJS) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

ldfile.o : ldfile.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(FDEFS) $(INCLUDES) -o $@ $<

ldmain.o : ldmain.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(MDEFS) $(INCLUDES) -o $@ $<
 
