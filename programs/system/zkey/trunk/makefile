#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=ZKEY
SOURCE=${FILE}.ASM
OUT=${FILE}.bin

en:
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}

