SOURCES=int40.o os.go.o os.gox main.go.o

GOFLAGS= -nostdlib -nostdinc -fno-stack-protector -fno-split-stack -static -m32 -g -I.
GO=gccgo
ASFLAGS= -felf
NASM= nasm $(ASFLAGS)
OBJCOPY=objcopy

LDFLAGS=-T static.lds -n -m elf_i386
 

all: $(SOURCES) link

clean: 
	rm *.o *.gox  main 

link:
	ld $(LDFLAGS) -o main.kex $(SOURCES)
	$(OBJCOPY) *.kex -O binary

%.gox: %.go.o
		$(OBJCOPY) -j .go_export $< $@

%.go.o: %.go
	$(GO)	$(GOFLAGS) -o $@ -c $<

%.o: %.s
	$(NASM) $<


