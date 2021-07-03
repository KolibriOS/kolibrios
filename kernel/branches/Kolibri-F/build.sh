#!/bin/sh
# Compile the KolibriOS kernel on Linux
# 2017, The KolibriOS team

KERPACK=./tools/kerpack
KOLIBRI_IMG=kolibri.img

echo 'lang fix en' > lang.inc
fasm -m 65536 bootbios.asm bootbios.bin
fasm -m 65536 -s kernel.fas kernel.asm kernel.mnt
$KERPACK kernel.mnt kernel.mnt
mcopy -D o -i $KOLIBRI_IMG kernel.mnt ::kernel.mnt