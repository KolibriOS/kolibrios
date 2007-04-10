@echo off
set drivers=sound sis infinity ati2d

mkdir bin
mkdir bin\drivers
mkdir bin\skins
:Target_kernel
echo building kernel

   echo lang fix en > lang.inc
   fasm -m 65536 kernel.asm bin\kernel.mnt


:Target_drivers
echo building drivers ...

   for %%a in (%drivers%) do (
     fasm -m 65536 drivers\%%a.asm bin\drivers\%%a.obj
   )


:Target_skins
echo building skins ...
cd skin
   fasm -m 65536 default.asm ..\bin\skins\default.skn
cd ..
pause





