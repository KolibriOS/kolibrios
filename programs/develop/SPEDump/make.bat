Set NAME=SPEDump

: KOLIBRIOS_PAS - path to KolibriOS.pas
Set KOLIBRIOS_PAS=

: KOLIBRIOS_LIB - path to KolibriOS.lib
Set KOLIBRIOS_LIB=

dcc32 -J -U%KOLIBRIOS_PAS% %NAME%.pas
omf2d %NAME%.obj
link -edit %NAME%.obj
LD -T LScript.x %NAME%.obj -o %NAME%.kex -L %KOLIBRIOS_LIB% -l KolibriOS
objcopy -O binary -j .all %NAME%.kex

Del %NAME%.obj
Del %NAME%.dcu

Pause