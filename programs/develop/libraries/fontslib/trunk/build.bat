@fasm -m 16384 font_.asm font01.ksf
@fasm -m 65536 fonts_lib.asm
@fasm -m 65536 font_ex.asm font_ex.kex
@kpack font_ex.kex
@kpack fonts_lib.obj