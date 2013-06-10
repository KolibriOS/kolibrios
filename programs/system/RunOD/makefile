#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=RunOD
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang fix en" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
it:
	echo "lang fix it" > ${LANG}
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
