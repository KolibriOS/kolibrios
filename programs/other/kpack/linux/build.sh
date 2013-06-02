#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

fasm -m 16384 kpack.asm kpack.o
gcc -s -nostdlib kpack.o -o kpack -lc
strip -R .comment -R .gnu.version kpack
exit 0




