@echo off
cls

call :Target_kernel

if ERRORLEVEL 0 goto Exit_OK

echo There was an error executing script.
echo For any help, please send a report.
pause
goto :eof

:Target_kernel
   rem valid languages: en ru ge et sp
   set lang=en

   echo *** building kernel with language '%lang%' ...

   echo lang fix %lang% > lang.inc
   fasm -m 65536 bootbios.asm bootbios.bin
   fasm -m 65536 kernel.asm kernel.mnt
   fasm -m 65536 kernel.asm kernel.bin -dUEFI=1
   if not %errorlevel%==0 goto :Error_FasmFailed
   erase lang.inc
goto :eof


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
