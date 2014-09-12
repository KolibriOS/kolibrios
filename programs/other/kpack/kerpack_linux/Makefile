CFLAGS=-O3
all: kpack kerpack
kpack: kpack64.o LZMAEncoder-kpack.o MatchFinder.o RangeCoder.o
	gcc -nostartfiles -o kpack kpack64.o LZMAEncoder-kpack.o MatchFinder.o RangeCoder.o
kpack64.o: kpack64.asm
	fasm kpack64.asm kpack64.o
kerpack: kerpack64.o LZMAEncoder-kerpack.o MatchFinder.o RangeCoder.o
	gcc -nostartfiles -o kerpack kerpack64.o LZMAEncoder-kerpack.o MatchFinder.o RangeCoder.o
kerpack64.o: kerpack64.asm
	fasm kerpack64.asm kerpack64.o
LZMAEncoder-kpack.o: lzma_c/LZMAEncoder.c lzma_c/LZMAEncoder.h lzma_c/MatchFinder.h lzma_c/lzma.h lzma_c/RangeCoder.h lzma_c/RangeCoderBit.h lzma_c/RangeCoderBitTree.h lzma_c/common.h
	gcc -c $(CFLAGS) -o LZMAEncoder-kpack.o lzma_c/LZMAEncoder.c
LZMAEncoder-kerpack.o: lzma_c/LZMAEncoder.c lzma_c/LZMAEncoder.h lzma_c/MatchFinder.h lzma_c/lzma.h lzma_c/RangeCoder.h lzma_c/RangeCoderBit.h lzma_c/RangeCoderBitTree.h lzma_c/common.h
	gcc -c -DFOR_KERPACK $(CFLAGS) -o LZMAEncoder-kerpack.o lzma_c/LZMAEncoder.c
MatchFinder.o: lzma_c/MatchFinder.c lzma_c/MatchFinder.h lzma_c/common.h
	gcc -c $(CFLAGS) -o MatchFinder.o lzma_c/MatchFinder.c
RangeCoder.o: lzma_c/RangeCoder.c lzma_c/RangeCoder.h lzma_c/RangeCoderBit.h lzma_c/RangeCoderBitTree.h lzma_c/lzma.h lzma_c/common.h
	gcc -c $(CFLAGS) -o RangeCoder.o lzma_c/RangeCoder.c

clean:
	rm *.o kpack kerpack
