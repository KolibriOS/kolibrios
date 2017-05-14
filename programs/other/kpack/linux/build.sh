#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the kpack program, hopefully ;-)

fasm -m 16384 kpack.asm kpack.o
gcc -m32 -s -nostdlib -march=i386 kpack.o -o kpack -lc
strip -R .comment -R .gnu.version kpack
rm -f kpack.o
