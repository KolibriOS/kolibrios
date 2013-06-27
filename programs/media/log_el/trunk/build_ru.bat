if not exist bin mkdir bin
if not exist bin\font6x9.bmp @copy ..\..\..\fs\kfar\trunk\font6x9.bmp bin\font6x9.bmp
@copy *.png bin\*.png
@copy *.txt bin\*.txt
@fasm.exe -m 16384 log_el.asm bin\log_el.kex
@kpack bin\log_el.kex
pause
