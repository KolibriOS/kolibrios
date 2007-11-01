@echo off

rem Для сборки игры необходимо в переменной UNITS (определена ниже)
rem указать расположение папки, в которой находятся откомпилированные модули
rem RTL для KolibriOS. Например, если исходники RTL находятся в папке my/rtl,
rem то собранные модули RTL - скорее всего в my/units. Может оказаться
rem достаточным просто перенести эту папку (lrl) в директорию my.

rem Так же, для сборки, вам понадобится утилита exe2kos.exe и FreePascal 2.2.0.


set NAME=lrl
set NAMEEXE=%NAME%.exe
set NAMEKEX=%NAME%.kex

set BUILD=-FUbuild
set UNITS=-Fu../units

fpc %NAME%.pp -n -Twin32 -Se5 -XXs -Sg -O3pPENTIUM3 -CfSSE -WB0 %BUILD% %UNITS%
if errorlevel 1 goto error

exe2kos.exe %NAMEEXE% %NAMEKEX%
del %NAMEEXE%
move %NAMEKEX% bin
goto end

:error
echo An error occured while building %NAME%

:end
