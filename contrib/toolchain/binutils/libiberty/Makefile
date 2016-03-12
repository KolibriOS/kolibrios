
CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wwrite-strings -Wc++-compat -Wstrict-prototypes
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES=  -I. -I../include -I$(SDK_DIR)/sources/newlib/libc/include

DEFINES=  -DHAVE_CONFIG_H

SRCS = \
	alloca.c argv.c	asprintf.c bcmp.c bcopy.c bzero.c	\
	choose-temp.c concat.c cp-demangle.c cp-demint.c 	\
	cplus-dem.c crc32.c d-demangle.c dwarfnames.c dyn-string.c fdmatch.c	\
	ffs.c fibheap.c filename_cmp.c floatformat.c fnmatch.c	\
	fopen_unlocked.c getopt.c getopt1.c getpwd.c		\
	getruntime.c hashtab.c hex.c index.c insque.c		\
	lbasename.c lrealpath.c	make-relative-prefix.c		\
	make-temp-file.c md5.c memmem.c mempcpy.c		\
	objalloc.c obstack.c partition.c physmem.c random.c	\
	regex.c rindex.c safe-ctype.c setenv.c setproctitle.c	\
	sha1.c sigsetmask.c simple-object.c simple-object-coff.c\
	simple-object-elf.c simple-object-mach-o.c		\
	simple-object-xcoff.c sort.c spaces.c splay-tree.c	\
	stack-limit.c stpcpy.c stpncpy.c strcasecmp.c strerror.c\
	strncasecmp.c strndup.c strnlen.c strverscmp.c		\
	timeval-utils.c	unlink-if-ordinary.c vasprintf.c vprintf-support.c	\
	xatexit.c xexit.c xmalloc.c xmemdup.c xstrdup.c		\
	xstrerror.c xstrndup.c

OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: libiberty.a

libiberty.a : $(OBJS) MAkefile
	$(AR) crs libiberty.a $(OBJS)
	mv -f libiberty.a $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

 