all:
	kos32-gcc -c -fomit-frame-pointer -I/usr/local/kos32/sdk/sources/newlib/libc/include ./algorithms/md5.c -o ./algorithms/md5.o -Wall -Wextra
	kos32-gcc -c -fomit-frame-pointer -I/usr/local/kos32/sdk/sources/newlib/libc/include ./algorithms/sha1.c -o ./algorithms/sha1.o -Wall -Wextra
	kos32-gcc -c -fomit-frame-pointer -I/usr/local/kos32/sdk/sources/newlib/libc/include ./algorithms/sha256.c -o ./algorithms/sha256.o -Wall -Wextra
	kos32-gcc -c -fomit-frame-pointer -I/usr/local/kos32/sdk/sources/newlib/libc/include thashview.c -o thashview.o -Wall -Wextra
	kos32-ld  -call_shared -nostdlib --subsystem native --image-base 0 -T /usr/local/kos32/sdk/sources/newlib/app-dynamic.lds -Map=thashview.map -L /usr/local/kos32/lib/ -L /usr/local/kos32/sdk/lib/  -o thashview ./algorithms/md5.o ./algorithms/sha1.o ./algorithms/sha256.o thashview.o -lgcc -lc.dll
	kos32-strip thashview -o thashview
	kos32-objcopy thashview -O binary
clean:
	rm -f *.o
	rm -f algorithms/*.o


