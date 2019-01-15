if not exist bin mkdir bin

@fasm.exe -m 16384 te_syntax.asm bin\te_syntax.kex
@kpack bin\te_syntax.kex

pause