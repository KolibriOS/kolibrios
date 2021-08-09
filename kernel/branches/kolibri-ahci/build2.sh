#!/bin/sh
KERPACK=/home/programist/KOS_SVN/programs/other/kpack/kerpack_linux/kerpack
KOLIBRI_IMG=kolibri_test2.img

echo 'lang fix en' > lang.inc
fasm -m 65536 bootbios.asm bootbios.bin
fasm -m 65536 kernel.asm kernel.mnt
$KERPACK kernel.mnt kernel.mnt
mcopy -D o -i $KOLIBRI_IMG kernel.mnt ::kernel.mnt
cp $KOLIBRI_IMG kolibri.img
