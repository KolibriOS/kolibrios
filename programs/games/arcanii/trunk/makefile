#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=arcanii
SOURCE=${FILE}.asm
OUT=${FILE}.bin

ru:
	echo "lang fix ru" > lang.inc
	${FASM} ${SOURCE} ${OUT}

en:
	echo "lang fix en" > lang.inc
	${FASM} ${SOURCE} ${OUT}

it:
	echo "lang fix it" > lang.inc
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
