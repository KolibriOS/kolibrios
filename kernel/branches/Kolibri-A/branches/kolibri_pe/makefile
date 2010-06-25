
CC = gcc
FASM = fasm.exe

INCLUDE = include/

DEFS = -DUSE_SMP -DCONFIG_DEBUG

CFLAGS = -c -O2 $(DEFS) -I $(INCLUDE) -fomit-frame-pointer -fno-builtin-printf
LDFLAGS = -shared -s -Map kernel.map --image-base 0x100000 --file-alignment 32 

KERNEL_SRC:=				\
		kernel.asm		    \
		const.inc		\
		data32.inc		    \
		core/memory.inc		\
		core/heap.inc		\
		core/malloc.inc		\
		core/taskman.inc	\
		core/v86.inc		\
		core/sys32.inc		\
		core/syscall.inc	\
		core/dll.inc		\
		core/exports.inc	\
		fs/ntfs.inc		    \
		gui/window.inc		\
		gui/event.inc		\
		video/vesa20.inc	\
		video/cursors.inc	\
		hid/mousedrv.inc


PE_SRC:=			\
		init.asm		\
		mbi.c			\
		heap.c			\
		slab.c		\
		frame.c			\
		pe.c		\
		dll.c		\
		spinlock.c	\
		thread.c		\
		syscall.asm		\
		boot/boot.asm	\
		boot/start.asm

   	
H_SRC:=					\
		include/types.h		\
		include/atomic.h	\
		include/spinlock.h	\
		include/link.h		\
		include/core.h		\
		include/mm.h		\
		include/pe.h		\
		include/slab.h

PE_OBJS = $(patsubst %.s, bin/%.obj, $(patsubst %.asm, bin/%.obj,\
            $(patsubst %.c, bin/%.obj, $(PE_SRC))))


all: kernel.gz

kernel.gz :kernel.mnt
	7z a -tgzip kernel.gz kernel.mnt   

kernel.mnt: kernel.obj bin/export.obj $(PE_OBJS) Makefile ld.x
	ld $(LDFLAGS) -T ld.x -o $@ kernel.obj bin/export.obj $(PE_OBJS)

bin/%.obj : core/%.c $(H_SRC) Makefile
	$(CC) $(CFLAGS) -o $@ $<

bin/%.obj : gui/%.c $(H_SRC) Makefile
	$(CC) $(CFLAGS) -o $@ $<

bin/%.obj: core/%.asm Makefile
	$(FASM) $< $@
	
bin/%.obj: %.asm
	$(FASM) $< $@

bin/export.obj: core/export.asm
	as  -o $@ $< 

kernel.obj: $(KERNEL_SRC)
	$(FASM) kernel.asm

all: $(SUBDIRS)

.PHONY: all
