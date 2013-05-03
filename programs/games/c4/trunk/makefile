#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
NASM=/usr/bin/nasm
KPACK=/opt/bin/kpack
LANG=lang_nasm.inc
FILE=c4
SOURCE=${FILE}.asm
OUT=${FILE}.bin

all:en

en:
	echo "lang equ 'en'" > lang_nasm.inc
	${NASM} -f bin -o ${OUT} ${SOURCE}

it:
	echo "lang equ 'it'" > lang_nasm.inc
	${NASM} -f bin -o ${OUT} ${SOURCE}

clean:
	rm -f ${OUT} lang_nasm.inc
