PROGRAM=main
SOURCES=syscalls.o colors.go.o colors.gox os.go.o os.gox $(PROGRAM).go.o
LIBOBJ=os.go.o syscalls.o colors.go.o
GOFLAGS= -nostdlib -nostdinc -fno-stack-protector -fno-split-stack -static -m32 -g -I.
GO=gccgo
ASFLAGS= -felf
NASM= nasm $(ASFLAGS)
OBJCOPY=objcopy
 
LDFLAGS=-T static.lds -n -m elf_i386
 
 
all: $(SOURCES) link
clean:
	rm *.o *.gox  $(PROGRAM).kex
link:
	ld $(LDFLAGS) -o $(PROGRAM).kex $(SOURCES)
	$(OBJCOPY) $(PROGRAM).kex -O binary
%.gox: %.go.o
	$(OBJCOPY) -j .go_export $< $@
%.go.o: %.go
	$(GO)   $(GOFLAGS) -o $@ -c $<
%.o: %.s
	$(NASM) $<
