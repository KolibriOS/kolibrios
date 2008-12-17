cl /c /O2 /nologo kosilka.cpp kosFile.cpp kosSyst.cpp mcsmemm.cpp
link /nologo /entry:crtStartUp /subsystem:native /base:0 /fixed /align:16 /nodefaultlib kosilka.obj kosFile.obj kosSyst.obj mcsmemm.obj
pe2kos kosilka.exe kosilka
del kosilka.exe
pause