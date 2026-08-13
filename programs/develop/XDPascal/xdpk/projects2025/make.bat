@echo off

if exist bin rmdir /s /q bin
mkdir bin

For /R %%i In (*.pas) Do (
    ..\xdpk "%%i"
)

:: Переносим все скомпилированные файлы .kex из подпапок в общую папку bin
move /y *.kex bin\
:: Если они сохраняются прямо рядом с исходниками в тех же папках, то лучше так:
:: For /R %%i In (*.kex) Do move /y "%%i" bin\

pause