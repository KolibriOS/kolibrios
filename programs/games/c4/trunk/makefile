#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
NASM=/usr/bin/nasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=c4
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang equ 'en'" > lang.inc
	${NASM} -f bin -o ${OUT} ${SOURCE}

it:
	echo "lang equ 'it'" > lang.inc
	${NASM} -f bin -o ${OUT} ${SOURCE}

def:
	${NASM} -f bin -o ${OUT} ${SOURCE}

clean:
	rm -f ${OUT}
