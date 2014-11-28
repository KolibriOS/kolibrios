
CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wno-format -Werror
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES=  -I. -I../include -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/zlib 

DEFINES=  -DHAVE_CONFIG_H -DHAVE_i386pe_vec -DHAVE_i386pei_vec -DHAVE_bfd_elf32_i386_vec -DHAVE_bfd_elf32_little_generic_vec
DEFINES+= -DHAVE_bfd_elf32_big_generic_vec -DBINDIR="/home/autobuild/tools/win32/bin"

SRCS = \
	archive.c archures.c bfd.c bfdio.c	\
	binary.c cache.c coffgen.c cofflink.c	\
	compress.c corefile.c cpu-i386.c	\
	dwarf1.c dwarf2.c elf.c	elf32.c		\
	elf32-gen.c elf32-i386.c elf-attrs.c	\
	elf-eh-frame.c elf-ifunc.c elflink.c	\
	elf-nacl.c elf-strtab.c elf-vxworks.c	\
	format.c hash.c ihex.c init.c libbfd.c	\
	linker.c merge.c opncls.c pe-i386.c	\
	peigen.c reloc.c section.c simple.c	\
	srec.c stabs.c stab-syms.c syms.c	\
	targets.c tekhex.c verilog.c


OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: libbfd.a

libbfd.a : $(OBJS) MAkefile
	$(AR) crs libbfd.a $(OBJS)
#	mv -f libbfd.a $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

dwarf2.o : dwarf2.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -DDEBUGDIR=\"/home/autobuild/tools/win32/lib/debug\" $(INCLUDES) -o $@ $<


 