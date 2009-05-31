@echo off
cls
set languages=en ru ge et
set drivers=sound sis infinity ensoniq ps2mouse com_mouse uart ati2d vmode
set targets=all kernel drivers skins clean

call :Check_Target %1
for %%a in (all kernel) do if %%a==%target% call :Check_Lang %2
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
   fasm -m 65536 kernel.asm bin\kernel.mnt
   if not %errorlevel%==0 goto :Error_FasmFailed
   erase lang.inc
goto :eof


:Target_all
   call :Target_kernel
   call :Target_drivers
   call :Target_skins
goto :eof


:Target_drivers
   echo *** building drivers ...

   if not exist bin\drivers mkdir bin\drivers
   cd drivers
   for %%a in (%drivers%) do (
     fasm -m 65536 %%a.asm ..\bin\drivers\%%a.obj
     if not %errorlevel%==0 goto :Error_FasmFailed
   )
   cd ..
   move bin\drivers\vmode.obj bin\drivers\vmode.mdr


kpack >nul 2>&1

if %errorlevel%==9009 goto :Error_KpackFailed

echo *
echo ##############################################
echo *
echo Kpack KolibriOS drivers?
echo *    

set /P res=[y/n]?

if "%res%"=="y" (

  echo *
  echo Compressing system

  echo *
  for %%a in (bin\drivers\*.obj) do (
    echo ================== kpack %%a
    kpack %%a
    if not %errorlevel%==0 goto :Error_KpackFailed
  )

)
goto :eof


:Target_skins
   echo *** building skins ...

   if not exist bin\skins mkdir bin\skins
   cd skin
   fasm -m 65536 default.asm ..\bin\skins\default.skn
   if not %errorlevel%==0 goto :Error_FasmFailed
   cd ..
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

:Error_KpackFailed
echo   *** NOTICE ***
echo If you want pack all applications you may 
echo place "kpack" in accessible directory or system %PATH%.
echo You can download that tool from http://diamond.kolibrios.org/.
pause
exit 1

:Exit_OK
echo.
echo all operations has been done
pause
exit 0
