# Note that kobra_st.o MUST be first and kobra.o last.
OBJECTS=kobra_st.o heap.o malloc.o stdlib.o kolibri.o kobra.o

# Set real path to kolibri.img if you want to use install option
KOLIBRI_IMG=../../../../../../0.7.x-night/kolibri.img

# Path to kobra.kex on floppy (directory should be created before installing)
DESTDIR=kobra/

CC=gcc
AS=gcc
LD=ld
OBJCOPY=objcopy
READELF=readelf
MCOPY=mcopy
RM=rm -rf

CFLAGS=-c -m32 -O2
ASFLAGS=-c -m32
LDFLAGS=-nostdlib -T kolibri.ld -melf_i386
OBJCOPYFLAGS=-O binary
READELFFLAGS=--syms
MCOPYFLAGS=-D o

.c.o:
	$(CC) $(CFLAGS) $<

.s.o:
	$(AS) $(ASFLAGS) $<

all: $(OBJECTS)
	$(LD) $(LDFLAGS) -o kobra.elf $(OBJECTS)
	$(OBJCOPY) $(OBJCOPYFLAGS) kobra.elf kobra.kex

debug: all
	$(READELF) $(READELFFLAGS) kobra.elf | ./symbols2dbg.py >kobra.dbg

install: kobra.kex $(KOLIBRI_IMG)
	@$(MCOPY) $(MCOPYFLAGS) -i $(KOLIBRI_IMG) kobra.kex ::/$(DESTDIR) && echo kobra.kex copied
	@ls kobra.dbg >/dev/null && $(MCOPY) $(MCOPYFLAGS) -i $(KOLIBRI_IMG) kobra.dbg ::/$(DESTDIR) && echo kobra.dbg copied || echo no kobra.dbg

clean:
	$(RM) *.o kobra.kex kobra.dbg
	

help:
	@echo "Available options:"
	@echo "  all - make kobra.kex"
	@echo "  debug - same + kobra.dbg"
	@echo "  install - same + copying them to image"
	@echo "You should gcc, ld & objcopy (or other C compiler and linker, but you may need correct Makefile to use them)."
	@echo "If you want to use debug option you should have readelf. If you want to use install you should have shell"
	@echo "(sh, bash, etc) & mtools."
