CC=gcc
AS=gcc
LD=ld
OBJCOPY=objcopy
READELF=readelf
MCOPY=mcopy
RM=rm -rf

CFLAGS=-c -m32 -O2 -I$(ROOT)/include/
ASFLAGS=-c -m32
LDFLAGS=-nostdlib -T kolibri.ld -melf_i386
OBJCOPYFLAGS=-O binary
READELFFLAGS=--syms
MCOPYFLAGS=-D o

.c.o:
	$(CC) $(CFLAGS) -o $*.o $<

.s.o:
	$(AS) $(ASFLAGS) $*.o $<
