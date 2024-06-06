# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=bnc
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang fix en_US" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
ru:
	echo "lang fix ru_RU" > ${LANG}
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
