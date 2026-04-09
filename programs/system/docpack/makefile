#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=docpack
SOURCE=${FILE}.asm
OUT=${FILE}.bin

ru:
	echo "lang fix ru_RU" > ${LANG}
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
