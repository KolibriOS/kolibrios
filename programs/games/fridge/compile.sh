#!/bin/bash

app_name=fridge
dir_path=/usr/local/kos32
# 1 - static, 2 - dynamic.
compile_mode=2
# for app with gui (native, windows, posix).
# for app with cui (console).
subsys_arg='native'


gcc_keys="-m32 -march=native -mtune=generic -std=c99 -c -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
ld_keys1="-static -nostdlib --subsystem $subsys_arg --image-base 0"
ld_keys2="-call_shared -nostdlib --subsystem $subsys_arg --image-base 0"

path_to_headers="$dir_path/sdk/sources/newlib/libc/include"
path_to_ldscript1="$dir_path/sdk/sources/newlib/static.lds"
path_to_ldscript2="$dir_path/sdk/sources/newlib/app-dynamic.lds"
path_to_libc="$dir_path/lib"
path_to_libgcc="$dir_path/sdk/lib"

kos32-gcc  $gcc_keys -I $path_to_headers -o $app_name.o $app_name.c

case $compile_mode in
  1) kos32-ld  $ld_keys1 -T $path_to_ldscript1 -Map=$app_name.map -L $path_to_libgcc -L $path_to_libc -o $app_name.kex $app_name.o -lc -lgcc -lc  ;;
  2) kos32-ld  $ld_keys2 -T $path_to_ldscript2 -Map=$app_name.map -L $path_to_libgcc -L $path_to_libc  -o $app_name.kex $app_name.o -lgcc -lc.dll ;;
  *) exit 1 ;;
esac

kos32-strip  -s $app_name.kex -o $app_name.kex
kos32-objcopy  $app_name.kex -O binary
rm  $app_name.o
rm  $app_name.map

exit 0
