@echo off

set NAME=%1
set NAMEEXE=%NAME%.exe
set NAMEKEX=%NAME%.kex

set BUILD=-FUbuild
set UNTS=-Fu..\units

fpc %NAME%.pp -n -Twin32 -Se5 -XXs -Sg -O3pPENTIUM3 -CfSSE -WB0 %BUILD% %UNTS%
if errorlevel 1 goto error

..\exe2kos\exe2kos.exe %NAMEEXE% %NAMEKEX%
del %NAMEEXE%
move %NAMEKEX% bin
goto end

:error
echo An error occured while building %NAME%

:end
