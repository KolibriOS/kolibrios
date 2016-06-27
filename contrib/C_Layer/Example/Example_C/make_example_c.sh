#Please set up kolibrios sources here : /home/<username>/kolibrios

rm *.o
rm *.obj
rm *.map
rm example_c

kos32-gcc -c -I${HOME}/kolibrios/contrib/sdk/sources/newlib/libc/include -g -U_Win32 -U_WIN32 -U__MINGW32__ example_c.c -o example_c.o

fasm loadboxlib.asm loadboxlib.obj
fasm loadkmenu.asm loadkmenu.obj
fasm loadlibimg.asm loadlibimg.obj

kos32-ld *.o *.obj -T${HOME}/kolibrios/contrib/sdk/sources/newlib/libc/app.lds -nostdlib -static --image-base 0 -lgcc -L/home/autobuild/tools/win32/mingw32/lib /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a -static -o example_c -Map=example_c.map

objcopy -O binary example_c

echo "If everything went well, boardxmsg should be your binary!"
