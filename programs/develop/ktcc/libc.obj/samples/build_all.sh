#SHS
mkdir /tmp0/1/tcc_samples
../tcc file_io.c -o /tmp0/1/tcc_samples/file_io
../tcc whois.c -o /tmp0/1/tcc_samples/whois -lnetwork
../tcc stdio_test.c -o /tmp0/1/tcc_samples/stdio_test
../tcc basic_gui.c -o /tmp0/1/tcc_samples/basic_gui
../tcc consoleio.c -o /tmp0/1/tcc_samples/consoleio
../tcc dir_example.c -o /tmp0/1/tcc_samples/ls_dir
../tcc http_tcp_demo.c -o /tmp0/1/tcc_samples/http_tcp_demo -lnetwork
../tcc math_test.c -o /tmp0/1/tcc_samples/math_test
../tcc string_test.c -o /tmp0/1/tcc_samples/string_test
../tcc tmpdisk_work.c -o /tmp0/1/tcc_samples/tmpdisk_work
../tcc clayer/boxlib.c -o /tmp0/1/tcc_samples/boxlib  -lbox_lib
../tcc clayer/dialog.c -o /tmp0/1/tcc_samples/dialog  -ldialog
cp     clayer/logo.png /tmp0/1/tcc_samples/logo.png 
../tcc clayer/libimg.c -o /tmp0/1/tcc_samples/libimg  -limg
../tcc clayer/msgbox.c -o /tmp0/1/tcc_samples/msgbox  -lmsgbox
../tcc clayer/rasterworks.c -o /tmp0/1/tcc_samples/rasterworks  -lrasterworks
../tcc thread_work.c -o /tmp0/1/tcc_samples/thread_work
../tcc -I../include/SDL sdltest.c -o /tmp0/1/tcc_samples/sdltest -lSDL -lsound
../tcc shell_test.c -o /tmp0/1/tcc_samples/shell_test -lshell
../tcc libc_test.c -o /tmp0/1/tcc_samples/libc_test
../tcc defgen.c -o /tmp0/1/tcc_samples/defgen
../tcc pipe.c -o /tmp0/1/tcc_samples/pipe
../tcc futex.c -o /tmp0/1/tcc_samples/futex
"/sys/File managers/Eolite" /tmp0/1/tcc_samples
exit
