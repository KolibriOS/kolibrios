#SHS
mkdir /tmp0/1/tcc_samples
../tcc file_io.c -o /tmp0/1/tcc_samples/file_io -lc.obj
../tcc whois.c -o /tmp0/1/tcc_samples/whois -ltcc -lnetwork -lc.obj
../tcc stdio_test.c -o /tmp0/1/tcc_samples/stdio_test -lc.obj
../tcc basic_gui.c -o /tmp0/1/tcc_samples/basic_gui -ltcc -lc.obj
../tcc consoleio.c -o /tmp0/1/tcc_samples/consoleio -lc.obj
../tcc dir_example.c -o /tmp0/1/tcc_samples/ls_dir -lc.obj
../tcc http_tcp_demo.c -o /tmp0/1/tcc_samples/http_tcp_demo -ltcc -lnetwork -lc.obj
../tcc math_test.c -o /tmp0/1/tcc_samples/math_test -ltcc -lc.obj
../tcc string_test.c -o /tmp0/1/tcc_samples/string_test -ltcc -lc.obj
../tcc tmpdisk_work.c -o /tmp0/1/tcc_samples/tmpdisk_work -ltcc -lc.obj
../tcc clayer/boxlib.c -o /tmp0/1/tcc_samples/boxlib -ltcc -lbox -lc.obj
../tcc clayer/dialog.c -o /tmp0/1/tcc_samples/dialog -ltcc -ldialog -lc.obj
cp     clayer/logo.png /tmp0/1/tcc_samples/logo.png
../tcc clayer/libimg.c -o /tmp0/1/tcc_samples/libimg -ltcc -limg -lc.obj
../tcc clayer/msgbox.c -o /tmp0/1/tcc_samples/msgbox -ltcc -lmsgbox -lc.obj
../tcc clayer/rasterworks.c -o /tmp0/1/tcc_samples/rasterworks -ltcc -lrasterworks -lc.obj
../tcc thread_work.c -o /tmp0/1/tcc_samples/thread_work -ltcc -lc.obj
../tcc -I../include/SDL sdltest.c -o /tmp0/1/tcc_samples/sdltest -lSDL -lsound -ltcc -lc.obj
../tcc shell_test.c -o /tmp0/1/tcc_samples/shell_test -lshell -ltcc -lc.obj
../tcc libc_test.c -o /tmp0/1/tcc_samples/libc_test -ltcc -lc.obj
