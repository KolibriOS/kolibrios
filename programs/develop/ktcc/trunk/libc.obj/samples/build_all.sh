#SHS
mkdir /tmp0/1/tcc_samples
/kolibrios/develop/tcc/tcc file_io.c -o /tmp0/1/tcc_samples/file_io -lc.obj
/kolibrios/develop/tcc/tcc whois.c -o /tmp0/1/tcc_samples/whois -ltcc -lnetwork -lc.obj
/kolibrios/develop/tcc/tcc stdio_test.c -o /tmp0/1/tcc_samples/stdio_test -lc.obj
/kolibrios/develop/tcc/tcc basic_gui.c -o /tmp0/1/tcc_samples/basic_gui -ltcc -lc.obj
/kolibrios/develop/tcc/tcc consoleio.c -o /tmp0/1/tcc_samples/consoleio -lc.obj
/kolibrios/develop/tcc/tcc dir_example.c -o /tmp0/1/tcc_samples/ls_dir -lc.obj
/kolibrios/develop/tcc/tcc http_tcp_demo.c -o /tmp0/1/tcc_samples/http_tcp_demo -ltcc -lnetwork -lc.obj
/kolibrios/develop/tcc/tcc math_test.c -o /tmp0/1/tcc_samples/math_test -ltcc -lc.obj
/kolibrios/develop/tcc/tcc string_test.c -o /tmp0/1/tcc_samples/string_test -ltcc -lc.obj
/kolibrios/develop/tcc/tcc tmpdisk_work.c -o /tmp0/1/tcc_samples/tmpdisk_work -ltcc -lc.obj
/kolibrios/develop/tcc/tcc clayer/boxlib.c -o /tmp0/1/tcc_samples/boxlib -ltcc -lbox -lc.obj
/kolibrios/develop/tcc/tcc clayer/dialog.c -o /tmp0/1/tcc_samples/dialog -ltcc -ldialog -lc.obj
cp /kolibrios/develop/tcc/samples/clayer/logo.png /tmp0/1/tcc_samples/logo.png
/kolibrios/develop/tcc/tcc clayer/libimg.c -o /tmp0/1/tcc_samples/libimg -ltcc -limg -lc.obj
/kolibrios/develop/tcc/tcc clayer/msgbox.c -o /tmp0/1/tcc_samples/msgbox -ltcc -lmsgbox -lc.obj
/kolibrios/develop/tcc/tcc clayer/rasterworks.c -o /tmp0/1/tcc_samples/rasterworks -ltcc -lrasterworks -lc.obj
/kolibrios/develop/tcc/tcc thread_work.c -o /tmp0/1/tcc_samples/thread_work -ltcc -lc.obj
/kolibrios/develop/tcc/tcc -I/kolibrios/develop/tcc/include/SDL sdltest.c -o /tmp0/1/tcc_samples/sdltest -lSDL -lsound -ltcc -lc.obj
