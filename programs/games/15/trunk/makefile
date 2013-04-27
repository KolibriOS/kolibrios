#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=15
SOURCE=${FILE}.ASM
OUT=${FILE}.bin

en:
	echo "lang fix en" > lang.inc
	${FASM} ${SOURCE} ${OUT}

ru:
	echo "lang fix ru" > lang.inc
	${FASM} ${SOURCE} ${OUT}

de:
	echo "lang fix de" > ${LANG}
	${FASM} ${SOURCE} ${OUT}

it:
	echo "lang fix it" > ${LANG}
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
