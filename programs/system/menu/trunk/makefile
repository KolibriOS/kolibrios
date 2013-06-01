#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=menu
SOURCE=${FILE}.asm
OUT=${FILE}.bin

en:
	echo "lang fix de" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

ru:
	echo "lang fix ru" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

it:
	echo "lang fix it" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

clean:
	rm ${LANG} \@menu
