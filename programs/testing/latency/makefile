#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
FILE=latency
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	${FASM} ${SOURCE} ${OUT}
	${KPACK} ${OUT}

clean:
	rm -f ${LANG} ${OUT}
