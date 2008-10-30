
CC = gcc
FASM = fasm.exe

INCLUDE = include/

DEFS = -DUSE_SMP -DCONFIG_DEBUG

CFLAGS = -c -O2 -I $(INCLUDE) -fomit-frame-pointer -fno-builtin
LDFLAGS = -shared -s -Map kernel.map --image-base 0x100000 --file-alignment 32 

KERNEL_SRC:=				\
		kernel.asm		\
		data32.inc		\
		core/memory.inc		\
		core/heap.inc		\
		core/malloc.inc		\
		core/taskman.inc	\
		core/v86.inc		\
		core/sys32.inc		\
		core/dll.inc		\
		core/exports.inc	\
		fs/ntfs.inc		\
		gui/event.inc		\
		video/cursors.inc


PE_SRC:=			\
		init.c		\
		mm.c		\
		slab.c		\
		heap.c		\
		dll.c		\
		spinlock.c	\
		boot/boot.asm	\
		boot/start.asm

#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>
	   	
H_SRC:=					\
		include/types.h		\
		include/atomic.h	\
		include/spinlock.h	\
		include/link.h		\
		include/core.h		\
		include/mm.h		\
		include/slab.h

PE_OBJS = $(patsubst %.s, bin/%.obj, $(patsubst %.asm, bin/%.obj,\
            $(patsubst %.c, bin/%.obj, $(PE_SRC))))


all: kernel.gz

kernel.gz :kernel.mnt
	7z a -tgzip kernel.gz kernel.mnt   

kernel.mnt: kernel.obj $(PE_OBJS) Makefile ld.x
	ld $(LDFLAGS) -T ld.x -o $@ kernel.obj $(PE_OBJS) 

bin/%.obj : core/%.c $(H_SRC) Makefile
	$(CC) $(CFLAGS) -o $@ $<
	
bin/%.obj: %.asm
	$(FASM) $< $@

kernel.obj: $(KERNEL_SRC)
	$(FASM) kernel.asm

all: $(SUBDIRS)

.PHONY: all
