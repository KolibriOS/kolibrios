#!/bin/sh
fpc -Tlinux exe2kos.pp &&
rm *.o &&
rm *.ppu &&
mv exe2kos ../../bin