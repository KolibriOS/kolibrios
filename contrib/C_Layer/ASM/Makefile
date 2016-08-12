
FASM = fasm

all: $(patsubst   %.asm,%.obj,$(wildcard   *.asm))
	mv $^ ../OBJ
%.obj: %.asm
	$(FASM) $< $%
