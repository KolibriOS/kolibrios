# KolibriOS Image Unpacker
## Summary

Extracts files from FAT12 KolibriOS image to specified folder.

## How to use

unimg path/to/img [output/folder] [-e]

If output folder is skipped, the image will be unpacked at /TMP0/1/KOLIBRI.IMG

Options:
-e: Exit on success

## How to build

kos32-tcc fat12.c -lck -o unimg.kex

## Toolchain

Default toolchain for TCC on Kolibri, got from KolibriISO/develop/tcc

## Authors

- Magomed Kostoev (Boppan, mkostoevr): FAT12 file system, driver.

## Contributors

- Kirill Lypatov (Leency): Coding style, driver working protocol.