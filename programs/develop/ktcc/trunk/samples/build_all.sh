#SHS
../tcc asm_ex.c -lck -o asm_ex
../tcc consoleio.c -lck -o consoleio
../tcc files.c -lck -o files
../tcc winbasics.c -lck -o winbasics
../tcc dynamic.c -lck -lhttp -linputbox -o dynamic
../tcc load_coff.c -o load_coff -lck
../tcc clayer/msgbox.c  -lck -lmsgbox -o msgbox
../tcc graphics.c -lck -lgb -o graphics
../tcc clayer/rasterworks.c -lck -lrasterworks -o clayer/rasterworks
../tcc clayer/boxlib.c -lck -lbox -o clayer/boxlib
../tcc clayer/libimg.c -lck -limg -o clayer/libimg

../tcc console/console.c -lck -limg -o console/console
exit
