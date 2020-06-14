@echo off
gcc -c libvorbis.c -o stb_vorbis.obj -O3 -march=i586 -mfpmath=387 -ffast-math -fwhole-program -nostdlib
pause