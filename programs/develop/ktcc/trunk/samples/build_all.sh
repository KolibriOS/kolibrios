#SHS
../tcc asm_ex.c /kolibrios/develop/tcc/lib/libck.a -o asm_ex
../tcc consoleio.c /kolibrios/develop/tcc/lib/libck.a -o consoleio
../tcc files.c /kolibrios/develop/tcc/lib/libck.a -o files
../tcc winbasics.c /kolibrios/develop/tcc/lib/libck.a -o winbasics
../tcc dynamic.c -lconsole -lhttp -linputbox -o dynamic
../tcc load_coff.c -o load_coff -lck
../tcc msgbox.c  -lck -lmsgbox -o msgbox
exit
