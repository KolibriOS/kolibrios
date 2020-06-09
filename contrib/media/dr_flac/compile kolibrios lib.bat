@echo off
gcc -c dr_flac.c -o dr_flac.obj -O3 -march=i586 -mfpmath=387 -ffast-math -fwhole-program -nostdlib
pause