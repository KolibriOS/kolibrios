@echo off
@cl /c /O2 /nologo main.cpp kosSyst.cpp mcsmemm.cpp formats\pcx.cpp formats\bmp.cpp formats\tga.cpp dlgopen.cpp
@link /nologo /entry:crtStartUp /subsystem:native /base:0 /fixed /align:16 /nodefaultlib main.obj kosSyst.obj mcsmemm.obj pcx.obj bmp.obj tga.obj dlgopen.obj
@tools\pe2kos main.exe imgview.kos PARAM
@erase main.obj,kosSyst.obj,mcsmemm.obj,pcx.obj,bmp.obj,tga.obj,KosFile.obj,main.exe,dlgopen.obj
pause