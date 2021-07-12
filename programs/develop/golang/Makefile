PROGRAM=example
SOURCES=syscalls.o colors.go.o colors.gox os.go.o os.gox $(PROGRAM).go.o
LIBOBJ=os.go.o syscalls.o colors.go.o
GOFLAGS = -m32 -c -nostdlib -nostdinc -fno-stack-protector -fno-split-stack -static -fno-leading-underscore -fno-common -fno-pie -g -I.
GO = gccgo
GCC = gcc
ASFLAGS= -g -f elf32 -F dwarf
NASM= nasm $(ASFLAGS)
OBJCOPY=objcopy

LDFLAGS=-n -T static.lds -m elf_i386 --no-ld-generated-unwind-info


all: $(SOURCES) link
clean:
	rm -f *.o *.gox $(PROGRAM).kex
link:
	ld $(LDFLAGS) -o $(PROGRAM).kex $(SOURCES)
	$(OBJCOPY) $(PROGRAM).kex -O binary
%.gox: %.go.o
	$(OBJCOPY) -j .go_export $< $@
%.go.o: %.go
	$(GO)   $(GOFLAGS) -o $@ -c $<
%.o: %.s
	$(NASM) $<
