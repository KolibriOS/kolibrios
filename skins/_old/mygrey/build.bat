@echo off
set fasm=c:\kolibri\usr\dos\fasm.exe
set file=default
set srcext=asm
set binext=skn

%fasm% %file%.%srcext% %file%.%binext%
pause
echo.