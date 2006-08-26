FASM=fasm
KSRC=kernel.asm
KOUT=kernel.mnt

en: kernel.asm
	rm -f lang.inc
	echo lang fix en > lang.inc
	$(FASM) $(KSRC) $(KOUT)
ru: kernel.asm
	rm -f lang.inc
	echo lang fix ru > lang.inc
	$(FASM) $(KSRC) $(KOUT)

clean:
	rm -f $(KOUT)
	rm -f lang.inc
