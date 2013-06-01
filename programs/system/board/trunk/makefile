#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=board
SOURCE=${FILE}.asm
OUT=${FILE}.bin

ru:
	echo "lang fix ru" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
ge:
	echo "lang fix ge" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
en:
	echo "lang fix en" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
it:
	echo "lang fix it" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
