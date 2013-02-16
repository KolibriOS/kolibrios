@echo off
fasm test.asm test_asm
kpack test_asm
copy test_asm bin\eng\
move test_asm bin\rus\
pause