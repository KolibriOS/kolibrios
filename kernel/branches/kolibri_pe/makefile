
CC = gcc
FASM = fasm.exe

INCLUDE = include/

DEFS = -DUSE_SMP -DCONFIG_DEBUG

CFLAGS = -c -O2  -I $(INCLUDE) -fomit-frame-pointer -fno-builtin-printf -masm=intel
LDFLAGS = -shared -s -Map kernel.map --image-base 0x100000 --file-alignment 32 

KERNEL_SRC:=				\
		kernel.asm		\
		core/memory.inc		\
		core/heap.inc		\
		core/taskman.inc	\
		core/sys32.inc		\
		core/dll.inc

PE_SRC:=			\
		init.c		\
		mm.c		\
		slab.c		\
		heap.c		\
		spinlock.c	\
		boot/boot.asm	\
		boot/start.asm
	   	

PE_OBJS = $(patsubst %.s, bin/%.obj, $(patsubst %.asm, bin/%.obj,\
            $(patsubst %.c, bin/%.obj, $(PE_SRC))))


all: kernel.gz

kernel.gz :kernel.mnt
	7z a -tgzip kernel.gz kernel.mnt   

kernel.mnt: kernel.obj $(PE_OBJS) Makefile ld.x
	ld $(LDFLAGS) -T ld.x -o $@ kernel.obj $(PE_OBJS) 

bin/%.obj : core/%.c Makefile
	$(CC) $(CFLAGS) -o $@ $<
	
bin/%.obj: %.asm
	$(FASM) $< $@

kernel.obj: $(KERNEL_SRC)
	$(FASM) kernel.asm

all: $(SUBDIRS)

.PHONY: all
