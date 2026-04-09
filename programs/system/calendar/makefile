#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=calendar
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang fix en_US" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
et:
	echo "lang fix et_EE" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
de:
	echo "lang fix de_DE" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
ru:
	echo "lang fix ru_RU" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
it:
	echo "lang fix it_IT" > ${LANG}
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
