#!gmake

# Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc

en:
	echo "lang fix ru" > ${LANG}
	${FASM} \@clip.ASM \@clip
	${FASM} cliptest.ASM cliptest
	${FASM} test2.asm test2
	${KPACK} \@clip

clean:
	rm -f ${LANG} \@clip test2 cliptest

