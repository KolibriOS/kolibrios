if not exist bin mkdir bin
@fasm.exe -m 16384 life3.asm bin\life3.kex
@kpack bin\life3.kex
pause
