#SHS
../tcc asm_ex.c -lck -o /tmp0/1/asm_ex
../tcc consoleio.c -lck -o /tmp0/1/consoleio
../tcc files.c -lck -o /tmp0/1/files
../tcc winbasics.c -lck -o /tmp0/1/winbasics
../tcc dynamic.c -lck -lhttp -linputbox -o /tmp0/1/dynamic
../tcc load_coff.c -o /tmp0/1/load_coff -lck
../tcc clayer/msgbox.c  -lck -lmsgbox -o /tmp0/1/msgbox
../tcc graphics.c -lck -lgb -o /tmp0/1/graphics
../tcc clayer/rasterworks.c -lck -lrasterworks -o /tmp0/1/rasterworks
../tcc clayer/boxlib.c -lck -lbox -o /tmp0/1/boxlib_ex
../tcc clayer/libimg.c -lck -limg -o /tmp0/1/libimg_ex
cp clayer/kolibrios.jpg /tmp0/1/kolibrios.jpg
../tcc clayer/dialog.c -lck -ldialog -o /tmp0/1/dialog_ex
../tcc dir_example.c -lck -o /tmp0/1/dir_example
../tcc net/tcpsrv_demo.c -lck -o /tmp0/1/tcpsrv_demo
../tcc net/nslookup.c -lck -lnetwork -o /tmp0/1/nslookup
../tcc net/http_tcp_demo.c -lck -lnetwork -o /tmp0/1/http_tcp_demo
../tcc getopt_ex.c -lck -o /tmp0/1/getopt_ex
../tcc tinygl/fps.c tinygl/gears.c -o /tmp0/1/gears -ltinygl -lck
exit
