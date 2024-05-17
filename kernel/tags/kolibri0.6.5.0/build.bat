@echo off

set languages=en ru ge et
set drivers=unisound sis infinity ati2d
set targets=all kernel drivers skins clean

call :Check_Target %1
for %%a in (all kernel) do if %%a==%target% call :Check_Lang %2
call :Target_%target%

if ERRORLEVEL 0 goto Exit_OK

echo Error encountered while running!
echo For help send a report...
pause
goto :eof




:Check_Lang
   set res=%1
  :Check_Lang_loop
   for %%a in (%languages%) do if %%a==%res% set lang=%res%
   if defined lang goto :eof

   echo Language "%res%" is not founded
   echo Enter valide languege
   echo     [%languages%]

   set /P res=">
   goto Check_Lang_loop
goto :eof

:Check_Target
   set res=%1
  :Check_Target_loop
   for %%a in (%targets%) do if %%a==%res% set target=%res%
   if defined target goto :eof

   echo Target "%res%" is not valide
   echo Enter valide target
   echo     [%targets%]

   set /P res=">
   goto Check_Target_loop
goto :eof


:Target_kernel
   echo building kernel with language %lang% ...

   if not exist bin mkdir bin
   echo lang fix %lang% > lang.inc
   fasm kernel.asm bin\kernel.mnt
   if not %errorlevel%==0 goto :Error_FasmFailed
   erase lang.inc
goto :eof


:Target_all
   echo building all ...
   call :Target_kernel
   call :Target_drivers
   call :Target_skins
goto :eof


:Target_drivers
   echo building drivers ...

   if not exist bin\drivers mkdir bin\drivers
   cd drivers
   for %%a in (%drivers%) do (
     fasm %%a.asm ..\bin\drivers\%%a.obj
     if not %errorlevel%==0 goto :Error_FasmFailed
   )
   cd ..
goto :eof


:Target_skins
   echo building skins ...

   if not exist bin\skins mkdir bin\skins
   cd skin
   fasm default.asm ..\bin\skins\default.skn
   if not %errorlevel%==0 goto :Error_FasmFailed
   cd ..
goto :eof


:Target_clean
   echo cleaning ...

   del /Q bin\drivers\*.*
   del /Q bin\skins\*.*
   del /Q bin\*.*
   rmdir bin\drivers
   rmdir bin\skins
   rmdir bin
goto :Exit_OK


:Error_FasmFailed
echo error: fasm execution failed
erase lang.inc
pause
exit 1

:Exit_OK
echo all operations have been done
pause
exit 0
