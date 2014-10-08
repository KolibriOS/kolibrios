include $(MENUETDEV)/osrules.mak

.SUFFIXES: .asm;

OBJS = crt0.o crt0_dynstack.o

all: $(OBJS)

crt0.o: crt0_coff.asm
	fasm crt0_coff.asm crt0.o
crt0_dynstack.o: crt0_coff_dynstack.asm
	fasm crt0_coff_dynstack.asm crt0_dynstack.o

clean:
	$(RM) $(OBJS)
