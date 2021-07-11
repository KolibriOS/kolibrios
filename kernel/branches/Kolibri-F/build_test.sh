#!/bin/sh
set -e
KERPACK=kerpack
KOLIBRI_IMG=kolibri.img

gcc -m32 -c -nostdinc -I../../../programs/develop/ktcc/trunk/libc.obj/source/ -I../../../programs/develop/ktcc/trunk/libc.obj/include  -DGNUC -D_BUILD_LIBC -fno-common -Os -fno-builtin -fno-leading-underscore -fno-stack-protector -fno-pie libc/libc.c -o libc/libc.o
ld -m elf_i386 -nostdlib -s -Map libc/libc.map -T libc/bin.x libc/libc.o -o libc/libc.bin

echo 'lang fix en' > lang.inc
fasm -m 65536 bootbios.asm bootbios.bin
fasm -m 65536 -s debug_bochs/kernel.fas kernel.asm kernel.mnt 
wine debug_bochs/symbols.exe debug_bochs/kernel.fas debug_bochs/kernel.txt
python3 debug_bochs/fastxt2bochs_map.py debug_bochs/kernel.txt

$KERPACK kernel.mnt kernel.mnt
wget http://builds.kolibrios.org/eng/latest-img.7z
7z x -y latest-img.7z
rm -f latest-img.7z
mcopy -D o -i $KOLIBRI_IMG kernel.mnt ::kernel.mnt
qemu-system-i386 -m 256 -fda kolibri.img -boot a  -enable-kvm

