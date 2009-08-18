echo off
cls
echo ** bulding after win loader ** 
@fasm -m 65535 after_win/kordldr.win.asm after_win/kordldr.win
echo ==============================
echo ** building first loader for cd/dvd **
@fasm -m 65535 cdfs/bootsect.asm cdfs/bootsect.bin
echo ==============================
echo ** building first loader for fat12/fat16 **
@fasm -m 65535 fat1x/bootsect.asm fat1x/bootsect.bin
@fasm -m 65535 fat1x/kordldr.f1x.asm fat1x/kordldr.f1x
echo ==============================
echo ** building firs loader for fat32 **
@fasm -m 65535 fat32/bootsect.asm fat32/bootsect.bin
@fasm -m 65535 fat32/kordldr.f1x.asm fat32/kordldr.f1x
@pause
