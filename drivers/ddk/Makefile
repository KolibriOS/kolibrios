
CC = gcc
AS = as

DRV_TOPDIR   = $(CURDIR)/..
DRV_INCLUDES = $(DRV_TOPDIR)/include

INCLUDES = 	-I$(DRV_INCLUDES) -I$(DRV_INCLUDES)/linux -I$(DRV_INCLUDES)/linux/asm
DEFINES  = 	-DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 
CFLAGS = 	-c -Os $(INCLUDES) $(DEFINES) -march=i486 -fomit-frame-pointer -fno-builtin-printf \
-mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2

NAME:=	  libddk

CORE_SRC=	core.S


NAME_SRCS:=  				\
		debug/dbglog.c		\
		debug/chkstk.S		\
           	io/create.c		\
           	io/finfo.c		\
           	io/ssize.c		\
           	io/write.c		\
		linux/idr.c		\
		linux/firmware.c	\
		linux/list_sort.c	\
		linux/dmapool.c		\
		linux/ctype.c		\
		linux/string.c		\
		malloc/malloc.c		\
		stdio/vsprintf.c	\
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

