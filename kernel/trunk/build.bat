@echo off

set languages=en ru ge
set drivers=unisound sis infinity ati2d

set opt_ok=0
for %%a in (%languages%) do if %%a==%1 set opt_ok=1
if %opt_ok%==0 goto :Check_Targets

echo building kernel with language %1 ...

if not exist bin mkdir bin
echo lang fix %1 > lang.inc
fasm kernel.asm bin\kernel.mnt
if not %errorlevel%==0 goto :Error_FasmFailed
erase lang.inc
goto :Exit_OK

:Check_Targets

for %%a in (all drivers skins clean) do if %%a==%1 set opt_ok=1
if %opt_ok%==0 goto :Error_WrongOption
goto :Target_%1

:Target_all

echo building all ...

if not exist bin mkdir bin
for %%a in (%languages%) do (
  echo lang fix %%a > lang.inc
  fasm kernel.asm bin\kernel_%%a.mnt
  if not %errorlevel%==0 goto :Error_FasmFailed
)
erase lang.inc
call :Target_drivers
call :Target_skins
exit :Exit_OK

:Target_drivers

echo building drivers ...

if not exist bin\drivers mkdir bin\drivers
cd drivers
for %%a in (%drivers%) do (
  fasm %%a.asm ..\bin\drivers\%%a.obj
  if not %errorlevel%==0 goto :Error_FasmFailed
)
cd ..
goto :Exit_OK

:Target_skins

echo building skins ...

if not exist bin\skins mkdir bin\skins
cd skin
fasm default.asm ..\bin\skins\default.skn
if not %errorlevel%==0 goto :Error_FasmFailed
cd ..
goto :Exit_OK

:Target_clean

echo cleaning ...

del /Q bin\drivers\*.*
del /Q bin\skins\*.*
del /Q bin\*.*
rmdir bin\drivers
rmdir bin\skins
rmdir bin
goto :Exit_OK

:Error_WrongOption

echo error: specified option is incorrect - '%1'
exit 1

:Error_FasmFailed

echo error: fasm execution failed
erase lang.inc
exit 1

:Exit_OK
