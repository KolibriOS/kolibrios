@echo off

set LIBPREFIX=lib
set LIBEXT=obj
set TESTPREFIX=test

echo building libraries...

if exist bin rmdir /S /Q bin
mkdir bin

for %%i in (gfx img ini io) do (
    echo   %LIBPREFIX%%%i
    cd %LIBPREFIX%%%i
    fasm %LIBPREFIX%%%i.asm ..\bin\%LIBPREFIX%%%i.%LIBEXT% >nul
    cd ..
)

echo building tests...

mkdir bin\.test

for %%i in (001 002) do (
    echo   %%i
    cd .test\%%i
    fasm %TESTPREFIX%%%i.asm ..\..\bin\.test\%TESTPREFIX%%%i >nul
    cd ..\..
)
