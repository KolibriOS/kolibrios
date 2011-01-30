del kerpack
del kerpack.obj
del kerpack.exe
fasm memset.asm
fasm kerpack.asm
"C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\link.exe" /section:.bss,E /fixed:no /subsystem:native /merge:.data=.text /merge:.rdata=.text /nologo /entry:start /out:kerpack.exe /ltcg kerpack.obj /nodefaultlib lzmapack.lib memset.obj
fasm doexe2.asm kerpack
