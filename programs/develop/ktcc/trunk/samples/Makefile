all:
	../bin/kos32-tcc asm_ex.c -lck -o asm_ex.kex -I ../libc/include
	../bin/kos32-tcc consoleio.c -lck -o consoleio.kex -I ../libc/include
	../bin/kos32-tcc files.c -lck -o files.kex -I ../libc/include
	../bin/kos32-tcc winbasics.c -lck -o winbasics.kex -I ../libc/include
	../bin/kos32-tcc dynamic.c -lck -lhttp -linputbox -o dynamic.kex -I ../libc/include
	../bin/kos32-tcc load_coff.c -o load_coff.kex -lck -I ../libc/include
	../bin/kos32-tcc clayer/msgbox.c  -lck -lmsgbox -o clayer/msgbox.kex -I ../libc/include
	../bin/kos32-tcc graphics.c -lck -lgb -o graphics.kex -I ../libc/include
	../bin/kos32-tcc clayer/rasterworks.c -lck -lrasterworks -o clayer/rasterworks.kex -I ../libc/include
	../bin/kos32-tcc clayer/boxlib.c -lck -lbox -o clayer/boxlib.kex -I ../libc/include
	../bin/kos32-tcc clayer/libimg.c -lck -limg -o clayer/libimg.kex -I ../libc/include
	../bin/kos32-tcc console/console.c -lck -limg -o console/console.kex -I ../libc/include
	../bin/kos32-tcc dir_example.c -lck -o dir_example.kex -I ../libc/include
	../bin/kos32-tcc net/tcpsrv_demo.c -lck -o net/tcpsrv_demo.kex -I ../libc/include
	../bin/kos32-tcc net/nslookup.c -lck -lnetwork -o net/nslookup.kex -I ../libc/include
	../bin/kos32-tcc getopt_ex.c -lck -o getopt_ex.kex -I ../libc/include

clean:
	rm *.kex
	rm clayer/*.kex
	rm console/*.kex
	rm net/*.kex
