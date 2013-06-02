#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=gomoku
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang fix en" > lang.inc
	${FASM} ${SOURCE} ${OUT}

ru:
	echo "lang fix ru" > lang.inc
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${OUT} ${LANG}
