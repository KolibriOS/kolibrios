
CC = gcc
AS = as

DRV_TOPDIR   = $(CURDIR)/..
DRV_INCLUDES = $(DRV_TOPDIR)/include

INCLUDES = 	-I$(DRV_INCLUDES) -I$(DRV_INCLUDES)/linux -I$(DRV_INCLUDES)/linux/asm
DEFINES  = 	-DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 
CFLAGS = 	-c -O2 $(INCLUDES) $(DEFINES) -fomit-frame-pointer -fno-builtin-printf 

NAME:=	  libddk

CORE_SRC=	core.S


NAME_SRCS:=  				\
		debug/dbglog.c		\
           	io/create.c		\
           	io/finfo.c		\
           	io/ssize.c		\
           	io/write.c		\
		linux/idr.c		\
		linux/firmware.c	\
		linux/list_sort.c	\
		linux/dmapool.c		\
		malloc/malloc.c		\
		stdio/icompute.c	\
		stdio/vsprintf.c	\
		stdio/doprnt.c		\
		stdio/chartab.c		\
           	string/_memmove.S	\
           	string/_strncat.S	\
           	string/_strncmp.S 	\
           	string/_strncpy.S 	\
           	string/_strnlen.S 	\
           	string/memcpy.S   	\
           	string/memcmp.S   	\
           	string/memset.S   	\
           	string/strcat.S		\
           	string/strchr.S   	\
           	string/strcpy.S   	\
           	string/strncpy.S  	\
           	string/strncmp.S  	\
           	string/strlen.S



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
	$(AS) -o $@ $<

%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

