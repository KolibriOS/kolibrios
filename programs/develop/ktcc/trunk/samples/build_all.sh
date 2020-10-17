#SHS
../tcc asm_ex.c /kolibrios/develop/tcc/lib/libck.a -o asm_ex
../tcc consoleio.c /kolibrios/develop/tcc/lib/libck.a -o consoleio
../tcc files.c /kolibrios/develop/tcc/lib/libck.a -o files
../tcc winbasics.c /kolibrios/develop/tcc/lib/libck.a -o winbasics
../tcc dynamic.c -lconsole -lhttp -linputbox -o dynamic
../tcc load_coff.c -o load_coff -lck
../tcc clayer/msgbox.c  -lck -lmsgbox -o msgbox
../tcc graphics.c -lck -lgb -o graphics
../tcc clayer/rasterworks.c -lck -lrasterworks -o rasterworks
../tcc clayer/boxlib.c -lck -lbox -o boxlib_ex
exit
