call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"  

@cl /c /O2 /nologo /GS- /GR- /fp:fast rforces.cpp kosFile.cpp kosSyst.cpp mcsmemm.cpp
@link /nologo /manifest:no /entry:crtStartUp /subsystem:native /base:0 /fixed /align:16 /nodefaultlib rforces.obj kosFile.obj kosSyst.obj mcsmemm.obj
@pe2kos rforces.exe rforces
@del rforces.exe
@del *.obj
pause