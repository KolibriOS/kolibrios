#SHS
/sys/@notify 'Build in progress...\nYou will find binaries in /tmp0/1/tccbin' -I
mkdir /tmp0/1/tccbin
../tcc asm_ex.c -lck -o /tmp0/1/tccbin/asm_ex
../tcc consoleio.c -lck -o /tmp0/1/tccbin/consoleio
../tcc files.c -lck -o /tmp0/1/tccbin/files
../tcc winbasics.c -lck -o /tmp0/1/tccbin/winbasics
../tcc dynamic.c -lck -lhttp -linputbox -o /tmp0/1/tccbin/dynamic
../tcc load_coff.c -o /tmp0/1/tccbin/load_coff -lck
../tcc clayer/msgbox.c  -lck -lmsgbox -o /tmp0/1/tccbin/msgbox
../tcc graphics.c -lck -lgb -o /tmp0/1/tccbin/graphics
../tcc clayer/rasterworks.c -lck -lrasterworks -o /tmp0/1/tccbin/rasterworks
../tcc clayer/boxlib.c -lck -lbox -o /tmp0/1/tccbin/boxlib_ex
../tcc clayer/libimg.c -lck -limg -o /tmp0/1/tccbin/libimg_ex
cp clayer/logo.png /tmp0/1/tccbin/logo.png
../tcc clayer/dialog.c -lck -ldialog -o /tmp0/1/tccbin/dialog_ex
../tcc dir_example.c -lck -o /tmp0/1/tccbin/dir_example
../tcc net/tcpsrv_demo.c -lck -o /tmp0/1/tccbin/tcpsrv_demo
../tcc net/nslookup.c -lck -lnetwork -o /tmp0/1/tccbin/nslookup
../tcc net/http_tcp_demo.c -lck -lnetwork -o /tmp0/1/tccbin/http_tcp_demo
../tcc getopt_ex.c -lck -o /tmp0/1/tccbin/getopt_ex
../tcc tinygl/fps.c tinygl/gears.c -o /tmp0/1/tccbin/gears -ltinygl -lck
exit
