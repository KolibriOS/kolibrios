fasm pcirom.asm
fasm step1.asm
fasm step2.asm
ren step2.bin pcirom.vhd
del step1.bin
del pcirom.bin
echo off
echo ********************************
echo     (C) Artem Jerdev, 2014
echo PCI ROM VHDL Template generated
pause