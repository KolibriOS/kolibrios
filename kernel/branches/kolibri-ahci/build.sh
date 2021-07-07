#!/bin/sh
# Compile the KolibriOS kernel on Linux
# 2017, The KolibriOS team

KERPACK=$HOME/kolibrios/programs/other/kpack/kerpack_linux/kerpack
KOLIBRI_IMG=$HOME/nightly/kolibri.img

replace=0; # Replace kernel in the image file?
echo 'lang fix en' > lang.inc
fasm -m 65536 bootbios.asm bootbios.bin
fasm -m 65536 kernel.asm kernel.mnt
$KERPACK kernel.mnt kernel.mnt

[[ $replace -eq 1 ]] && {
    mntpt=$(mktemp -d)

    sudo mount -o loop $KOLIBRI_IMG $mntpt
    sudo mount -o remount,rw $mntpt
    sudo cp kernel.mnt ${mntpt}/KERNEL.MNT
    sudo umount $mntpt
    rmdir $mntpt
}
