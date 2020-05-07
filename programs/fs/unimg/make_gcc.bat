gcc -m32 -c -fomit-frame-pointer -IC:\Users\Boppan\Documents\KolibriOS\contrib/sdk/sources/newlib/libc/include fat12.c -o unimg.o -Wall -Wextra
kos32-ld unimg.o -o unimg -static -S -nostdlib -T C:\Users\Boppan\Documents\KolibriOS\contrib/sdk/sources/newlib/app.lds --image-base 0 -L "C:\Program Files (x86)\kos32-msys-5.4.0\win32\lib" -lgcc -lapp -lc.dll
kos32-objcopy unimg -O binary
@pause