#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=vrr_m
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	${FASM} ${SOURCE} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
