
CC = kos32-gcc
AS = kos32-as
LD = kos32-ld 

DRV_TOPDIR   = $(CURDIR)/..
DRV_INCLUDES = $(DRV_TOPDIR)/include


INCLUDES =	-I$(DRV_INCLUDES)	\
		-I$(DRV_INCLUDES)/asm 	\
		-I$(DRV_INCLUDES)/uapi

DEFINES = -DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_DMI -DCONFIG_TINY_RCU
DEFINES+= -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE
DEFINES+= -DCONFIG_PRINTK

CFLAGS = 	-c -Os $(INCLUDES) $(DEFINES) -march=i686 -fomit-frame-pointer -fno-builtin-printf \
-mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -fno-ident

NAME:=	  libddk

CORE_SRC=	core.S


NAME_SRCS:=  				\
		debug/dbglog.c		\
		debug/chkstk.S		\
		dma/dma_alloc.c		\
		dma/fence.c		\
           	io/create.c		\
           	io/finfo.c		\
           	io/ssize.c		\
           	io/write.c		\
		linux/bitmap.c		\
		linux/ctype.c		\
		linux/div64.c		\
		linux/dmapool.c		\
		linux/dmi.c		\
		linux/fbsysfs.c		\
		linux/find_next_bit.c	\
		linux/firmware.c	\
		linux/gcd.c		\
		linux/hdmi.c		\
		linux/hexdump.c		\
		linux/idr.c		\
		linux/interval_tree.c	\
		linux/kasprintf.c	\
		linux/kmap.c		\
		linux/list_sort.c	\
		linux/mutex.c		\
		linux/rbtree.c		\
		linux/scatterlist.c	\
		linux/string.c		\
		linux/time.c		\
		linux/workqueue.c	\
		linux/msr.c		\
		malloc/malloc.c		\
		stdio/vsprintf.c	\
		string/strstr.c		\
		string/_memmove.S 	\
		string/_strncat.S 	\
		string/_strncmp.S 	\
		string/_strncpy.S 	\
		string/_strnlen.S 	\
		string/bcmp.S 		\
		string/bcopy.S 		\
		string/bzero.S 		\
		string/index.S 		\
		string/memchr.S 	\
		string/memcmp.S 	\
		string/memcpy.S 	\
		string/memmove.S 	\
		string/memset.S 	\
		string/rindex.S 	\
		string/strcat.S 	\
		string/strchr.S 	\
		string/strcmp.S 	\
		string/strcpy.S 	\
		string/strlen.S 	\
		string/strncat.S 	\
		string/strncmp.S 	\
		string/strncpy.S 	\
		string/strnlen.S 	\
		string/strrchr.S 



NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
             $(patsubst %.c, %.o, $(NAME_SRCS))))


TARGET  = $(NAME).a 

all: $(TARGET) libcore.a


$(TARGET): $(NAME_OBJS) $(NAME_SRC) Makefile
	$(AR) cvrs $@ $(NAME_OBJS)


libcore.a: core.S Makefile
	$(AS) -o core.o $<
	$(LD) -shared -s --out-implib $@ --output-def core.def -o core.dll core.o
	
%.o: %.S Makefile
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c Makefile ../include/*.h ../include/linux/*.h
	$(CC) $(CFLAGS) -o $@ $<

clean: 
	-rm -f */*.o


