PROGRAM=example
OBJS=../syscalls.o colors.go.o colors.gox ../kos.go.o ../kos.gox $(PROGRAM).go.o
GOFLAGS=-m32 -c -nostdlib -nostdinc -fno-stack-protector -fno-split-stack -static -fno-leading-underscore -fno-common -fno-pie -g -I.
GO=gccgo
ASFLAGS=-g -f elf32 -F dwarf
NASM=nasm $(ASFLAGS)
OBJCOPY=objcopy
LDFLAGS=-n -T static.lds -m elf_i386 --no-ld-generated-unwind-info

all: $(OBJS) link

clean:
	rm -f $(OBJS) $(PROGRAM).kex

link:
	ld $(LDFLAGS) -o $(PROGRAM).kex $(OBJS)
	$(OBJCOPY) $(PROGRAM).kex -O binary

%.gox: %.go.o
	$(OBJCOPY) -j .go_export $< $@

%.go.o: %.go
	$(GO)   $(GOFLAGS) -o $@ -c $<

%.o: %.asm
	$(NASM) $<
