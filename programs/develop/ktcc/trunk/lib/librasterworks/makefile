all:
	fasm __lib__.asm
	fasm charsFit.asm
	fasm cntUTF-8.asm
	fasm drawText.asm
	fasm  strWidth.asm
	ar -crs librasterworks.a *.o
clean:
	rm -f *.o
