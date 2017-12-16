@echo off
cls
set languages=en ru ge et sp
set targets=kernel clean

call :Check_Target %1
for %%a in (kernel) do if %%a==%target% call :Check_Lang %2
call :Target_%target%

if ERRORLEVEL 0 goto Exit_OK

echo There was an error executing script.
echo For any help, please send a report.
pause
goto :eof




:Check_Lang
   set res=%1
  :Check_Lang_loop
   for %%a in (%languages%) do if %%a==%res% set lang=%res%
   if defined lang goto :eof

   echo Language '%res%' is incorrect
   echo Enter valid language [ %languages% ]:

   set /P res=">
   goto Check_Lang_loop
goto :eof

:Check_Target
   set res=%1
  :Check_Target_loop
   for %%a in (%targets%) do if %%a==%res% set target=%res%
   if defined target goto :eof

   echo Target '%res%' is incorrect
   echo Enter valid target [ %targets% ]:

   set /P res=">
   goto Check_Target_loop
goto :eof


:Target_kernel
   echo *** building kernel with language '%lang%' ...

   if not exist bin mkdir bin
   echo lang fix %lang% > lang.inc
   fasm -m 65536 bootbios.asm bootbios.bin
   fasm -m 65536 kernel.asm bin\kernel.mnt
   fasm -m 65536 kernel.asm bin\kernel.bin -dUEFI=1
   if not %errorlevel%==0 goto :Error_FasmFailed
   erase lang.inc
goto :eof


:Target_clean
   echo *** cleaning ...
   rmdir /S /Q bin
goto :Exit_OK


:Error_FasmFailed
echo error: fasm execution failed
erase lang.inc >nul 2>&1
echo.
pause
exit 1

:Exit_OK
echo.
echo all operations have been done
pause
exit 0
