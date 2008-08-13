
CC = gcc
FASM = fasm.exe

INCLUDE = include/

DEFS = -DUSE_SMP

CFLAGS = -c -O2 -DCONFIG_DEBUG -I $(INCLUDE) -fomit-frame-pointer -fno-builtin-printf -masm=intel
LDFLAGS = -shared -s -Map kernel.map --image-base 0x100000 --file-alignment 32 


KERNEL_SRC:=			\
		kernel.asm	\
		init.c		\
		mm.c		\
		spinlock.c		\
		boot/boot.asm	\
		boot/start.asm
	   	

KERNEL_OBJS = $(patsubst %.s, bin/%.obj, $(patsubst %.asm, bin/%.obj,\
            $(patsubst %.c, bin/%.obj, $(KERNEL_SRC))))


all: kernel.gz

kernel.gz :kernel.mnt
	7z a -tgzip kernel.gz kernel.mnt   

kernel.mnt: $(KERNEL_OBJS) Makefile ld.x
	ld $(LDFLAGS) -T ld.x -o $@ $(KERNEL_OBJS) 

bin/%.obj : core/%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<
	
bin/%.obj: %.asm
	$(FASM) $< $@

all: $(SUBDIRS)

.PHONY: all
