#SHS
../tcc asm_ex.c -lck -o asm_ex
../tcc consoleio.c -lck -o consoleio
../tcc files.c -lck -o files
../tcc winbasics.c -lck -o winbasics
../tcc dynamic.c -lhttp -linputbox -o dynamic
../tcc load_coff.c -o load_coff -lck
../tcc clayer/msgbox.c  -lck -lmsgbox -o msgbox
../tcc graphics.c -lck -lgb -o graphics
../tcc clayer/rasterworks.c -lck -lrasterworks -o rasterworks
../tcc clayer/boxlib.c -lck -lbox -o boxlib
../tcc clayer/libimg.c -lck -limg -o libimg
exit
