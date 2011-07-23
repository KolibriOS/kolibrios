@echo off

   if not exist bin mkdir bin
   fasm -m 65536 kernel.asm bin\kernel.mnt

pause
exit 0