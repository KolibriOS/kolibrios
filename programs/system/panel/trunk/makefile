#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=@PANEL
SOURCE=${FILE}.ASM
OUT=${FILE}.bin

en:
	echo "lang fix en" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
et:
	echo "lang fix et" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
ru:
	echo "lang fix ru" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}
it:
	echo "lang fix it" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
