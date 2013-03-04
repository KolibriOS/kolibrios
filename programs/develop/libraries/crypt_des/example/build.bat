if not exist bin mkdir bin
@fasm.exe -m 16384 crypt_files.asm bin\crypt_files.kex
@kpack bin\crypt_files.kex
if not exist bin\toolbar.png @copy toolbar.png bin\toolbar.png
if not exist bin\font8x9.bmp @copy ..\..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
@fasm.exe -m 16384 ..\trunk\crypt_des.asm bin\crypt_des.obj
@kpack bin\crypt_des.obj
pause