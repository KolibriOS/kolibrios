cl /c /O2 /Ox /Os /GL /Gr /Oi /nologo /GS- /GR- kosilka.cpp kosFile.cpp kosSyst.cpp mcsmemm.cpp
link /section:.bss,E /nologo /ltcg /map /entry:crtStartUp /subsystem:native /base:0 /fixed:no /nodefaultlib /merge:.data=.text /merge:.rdata=.text kosilka.obj kosFile.obj kosSyst.obj mcsmemm.obj
fasm doexe2.asm kosilka
