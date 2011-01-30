"C:\Program Files\Microsoft Visual Studio 8\VC\bin\cl" /c /O2 /nologo /GS- /GR- /fp:fast rforces.cpp kosFile.cpp kosSyst.cpp mcsmemm.cpp
"C:\Program Files\Microsoft Visual Studio 8\VC\bin\link" /nologo /manifest:no /entry:crtStartUp /subsystem:native /base:0 /fixed /align:16 /nodefaultlib rforces.obj kosFile.obj kosSyst.obj mcsmemm.obj
pe2kos rforces.exe rforces
pause