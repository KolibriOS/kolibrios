@echo off
rem #### CONFIG SECTION ####
set LIBNAME=melibc.a
set INCLUDE=include
set CC=gcc
set CFLAGS=-c -nostdinc -DGNUC -I"%cd%\%INCLUDE%"
set AR=ar
set ASM=fasm
set dirs=file mem mesys string
rem #### END OF CONFIG SECTION ####

set objs=
set target=%1
if not "%1"=="clean" set target=all

set INCLUDE="%cd%"
call :Target_%target%

if ERRORLEVEL 0 goto Exit_OK

echo Probably at runing has been created error
echo For help send a report...
pause
goto :eof

:Compile_C
   %CC% %CFLAGS% %1 -o "%~dpn1.o"
   if not %errorlevel%==0 goto Error_Failed
   set objs=%objs% "%~dpn1.o"
goto :eof

:Compile_Asm
   %ASM% %1 "%~dpn1.o"
   if not %errorlevel%==0 goto Error_Failed
   set objs=%objs% "%~dpn1.o"
goto :eof

:Target_clean
   echo cleaning ...
   for %%a in (%dirs%) do del /Q "%%a\*.o"
goto :Exit_OK

:Target_all
   echo building all ...
   for %%a in (%dirs%) do (
      for %%f in ("%%a\*.asm") do call :Compile_Asm "%%f"
      for %%f in ("%%a\*.c") do call :Compile_C "%%f"
   )
   %AR% -ru %LIBNAME% %objs%
   if not %errorlevel%==0 goto Error_Failed
goto Exit_OK

:Error_Failed
echo error: execution failed
pause
exit 1

:Exit_OK
echo all operations has been done...
echo for cleaning run this script with param " clean"
pause
exit 0