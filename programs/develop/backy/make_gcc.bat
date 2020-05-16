gcc -m32 -c -fomit-frame-pointer -IC:\Users\Boppan\Documents\KolibriOS\contrib/sdk/sources/newlib/libc/include Backy.c -o Backy.o -Wall -Wextra
kos32-ld Backy.o -o Backy.gcc -static -S -nostdlib -T C:\Users\Boppan\Documents\KolibriOS\contrib/sdk/sources/newlib/app.lds --image-base 0 -L "C:\Program Files (x86)\kos32-msys-5.4.0\win32\lib" -lgcc -lapp -lc.dll
kos32-objcopy Backy.gcc -O binary
@pause