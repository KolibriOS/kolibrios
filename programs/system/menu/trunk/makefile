#Makefile per la compilazione

#Macro
FASM=/opt/bin/fasm
KPACK=/opt/bin/kpack
LANG=lang.inc
FILE=menu
SOURCE=${FILE}.asm

en:
	echo "lang fix en_US" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

ru:
	echo "lang fix ru_RU" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

it:
	echo "lang fix it_IT" > lang.inc
	${FASM} ${SOURCE} @menu
	${KPACK} @menu
	rm ${LANG}

clean:
	rm -f ${LANG} \@menu
