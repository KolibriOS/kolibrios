CC = kos32-gcc
LD = kos32-ld
SDK_DIR = ../../../sdk


CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wno-format -Werror
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES=  -I. -I../include -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/zlib 

DEFINES=  -DHAVE_CONFIG_H -DHAVE_i386_pe_vec -DHAVE_i386_pei_vec -DHAVE_i386_elf32_vec
DEFINES+= -DHAVE_elf32_le_vec -DHAVE_elf32_be_vec
DEFINES+= -DBINDIR="/home/autobuild/tools/win32/bin"
TDEFS=    -DDEFAULT_VECTOR=i386_pe_vec "-DSELECT_VECS=&i386_pe_vec,&i386_pei_vec,&i386_elf32_vec,&elf32_le_vec,&elf32_be_vec"
TDEFS+=   "-DSELECT_ARCHITECTURES=&bfd_i386_arch" 
SRCS = \
	archive.c archures.c bfd.c bfdio.c	\
	binary.c cache.c coff-bfd.c coffgen.c cofflink.c	\
	compress.c corefile.c cpu-i386.c	\
	dwarf1.c dwarf2.c elf.c	elf32.c		\
	elf32-gen.c elf32-i386.c elf-attrs.c	\
	elf-eh-frame.c elf-ifunc.c elflink.c	\
	elf-nacl.c elf-strtab.c elf-vxworks.c	\
	format.c hash.c ihex.c init.c libbfd.c	\
	linker.c merge.c opncls.c pe-i386.c	\
	peigen.c pei-i386.c reloc.c section.c	\
	simple.c srec.c stabs.c stab-syms.c	\
	syms.c targets.c tekhex.c verilog.c


OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: libbfd.a

libbfd.a : $(OBJS) Makefile
	$(AR) crs libbfd.a $(OBJS)
	mv -f libbfd.a $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

dwarf2.o : dwarf2.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -DDEBUGDIR="\"/home/autobuild/tools/win32/lib/debug\"" $(INCLUDES) -o $@ $<


targets.o : targets.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(TDEFS) $(INCLUDES) -o $@ $<

archures.o : archures.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(TDEFS) $(INCLUDES) -o $@ $<
 
