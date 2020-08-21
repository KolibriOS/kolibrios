#!/bin/bash

app_name=fil

kos32-gcc -c -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -I/usr/local/kos32/sdk/sources/newlib/libc/include -I/usr/local/kos32/sdk/sources/libstdc++-v3/include -o solitaire.o solitaire.cpp

kos32-ld -static -nostdlib -subsystem consolitairee -image-base 0 -T /usr/local/kos32/sdk/sources/newlib/static.lds -Map=solitaire.map -L /usr/local/kos32/sdk/lib -L /usr/local/kos32/lib -L /usr/local/kos32/mingw32/lib -o solitaire solitaire.o -lc -lgcc -lstdc++ -lsupc++ -lgcc -lc

kos32-objcopy solitaire -O binary

exit 0
