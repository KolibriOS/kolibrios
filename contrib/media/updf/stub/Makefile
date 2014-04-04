include osrules.mak

.SUFFIXES: .asm;

OBJS = crt0.o

all: $(OBJS)

ifdef ON_WINDOWS
crt0.o: crt0_$(STUBFMT).asm
	fasm crt0_$(STUBFMT).asm crt0.o
else
crt0.o: crt0_$(STUBFMT)_nounderscores.asm
	fasm crt0_$(STUBFMT)_nounderscores.asm crt0.o
endif

clean:
	$(RM) $(OBJS)
