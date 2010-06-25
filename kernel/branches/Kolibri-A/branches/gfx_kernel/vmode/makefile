FASM=fasm
FENV=fasminc=/usr/fasm/include
KSRC=vmode.asm
KOUT=vmode.mdr

en: $(KSRC)
	$(FENV) $(FASM) $(KSRC) $(KOUT)
ru: $(KSRC)
	$(FENV) $(FASM) $(KSRC) $(KOUT)

clean:
	rm -f $(KOUT)
