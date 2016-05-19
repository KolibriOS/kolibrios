@echo off
echo ####################################################
echo #           test libc builder                      #
echo #  usage: build [clean]                            #
echo ####################################################
rem #### CONFIG SECTION ####
set LIBNAME=libck.a
set INCLUDE=include
set CC=D:\VSProjects\msys-kos32-4.8.2\ktcc\trunk\libc\kos32-tcc.exe 
set CFLAGS=-I"%cd%\%INCLUDE%" -Wall
set AR=kos32-ar
set ASM=fasm
set dirs=.
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
   %CC% %CFLAGS% %1 -o "%~dpn1.kex" -lck
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
   for %%a in (%dirs%) do del /Q "%%a\*.kex"
goto :Exit_OK

:Target_all
   echo building all ...
   for %%a in (%dirs%) do (
      for %%f in ("%%a\*.asm") do call :Compile_Asm "%%f"
      for %%f in ("%%a\*.c") do call :Compile_C "%%f"
   )
::   %AR% -ru %LIBNAME% %objs%
::   if not %errorlevel%==0 goto Error_Failed
goto Exit_OK

:Error_Failed
echo error: execution failed
pause
exit 1

:Exit_OK
echo ####################################################
echo # All operations has been done...                  #
echo ####################################################
pause
exit 0