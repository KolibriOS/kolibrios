#Please set up kolibrios sources here : /home/<username>/kolibrios

kos32-gcc -c -I${HOME}/kolibrios/contrib/sdk/sources/newlib/libc/include -I../../INCLUDE -g -U_Win32 -U_WIN32 -U__MINGW32__ boardmsg.c -o boardmsg.o

kos32-ld *.o ../../OBJ/loadboxlib.obj -T${HOME}/kolibrios/contrib/sdk/sources/newlib/libc/app.lds -nostdlib -static --image-base 0 -lgcc -L/home/autobuild/tools/win32/mingw32/lib /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libc.dll.a -static -o boardxmsg -Map=boardxmsg.map

objcopy -O binary boardxmsg

echo "If everything went well, boardxmsg should be your binary!"
