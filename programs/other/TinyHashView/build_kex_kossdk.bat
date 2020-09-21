kos32-gcc -c -fomit-frame-pointer -ID:\KOSSDK\newlib/libc/include algorithms\md5.c -o algorithms\md5.o -Wall -Wextra
kos32-gcc -c -fomit-frame-pointer -ID:\KOSSDK\newlib/libc/include algorithms\sha1.c -o algorithms\sha1.o -Wall -Wextra
kos32-gcc -c -fomit-frame-pointer -ID:\KOSSDK\newlib/libc/include algorithms\sha256.c -o algorithms\sha256.o -Wall -Wextra
kos32-gcc -c -fomit-frame-pointer -ID:\KOSSDK\newlib/libc/include thashview.c -o thashview.o -Wall -Wextra

kos32-ld thashview.o algorithms\md5.o algorithms\sha1.o algorithms\sha256.o -o thashview -call_shared -nostdlib --subsystem native -T D:\KOSSDK\newlib/app-dynamic.lds --image-base 0 -L "D:\KOSSDK\kos32-msys-5.4.0\win32\lib" -lgcc -lapp -lc.dll
kos32-objcopy thashview -O binary
@pause

