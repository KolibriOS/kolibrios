CC=../bin/kos32-tcc
CFLAGS= -I ../libc/include

all:
	$(CC) $(CFLAGS) asm_ex.c -lck -o asm_ex.kex
	$(CC) $(CFLAGS) consoleio.c -lck -o consoleio.kex 
	$(CC) $(CFLAGS) files.c -lck -o files.kex 
	$(CC) $(CFLAGS) winbasics.c -lck -o winbasics.kex 
	$(CC) $(CFLAGS) dynamic.c -lck -lhttp -linputbox -o dynamic.kex 
	$(CC) $(CFLAGS) load_coff.c -o load_coff.kex -lck
	$(CC) $(CFLAGS) graphics.c -lck -lgb -o graphics.kex
	$(CC) $(CFLAGS) dir_example.c -lck -o dir_example.kex 
	$(CC) $(CFLAGS) getopt_ex.c -lck -o getopt_ex.kex 
	
	$(CC) $(CFLAGS) clayer/msgbox.c  -lck -lmsgbox -o clayer/msgbox.kex 
	$(CC) $(CFLAGS) clayer/rasterworks.c -lck -lrasterworks -o clayer/rasterworks.kex 
	$(CC) $(CFLAGS) clayer/boxlib.c -lck -lbox -o clayer/boxlib.kex 
	$(CC) $(CFLAGS) clayer/libimg.c -lck -limg -o clayer/libimg.kex
	$(CC) $(CFLAGS) clayer/dialog.c -lck -ldialog -o clayer/dialog.kex
	
	$(CC) $(CFLAGS) net/tcpsrv_demo.c -lck -o net/tcpsrv_demo.kex 
	$(CC) $(CFLAGS) net/nslookup.c -lck -lnetwork -o net/nslookup.kex
	$(CC) $(CFLAGS) net/http_tcp_demo.c -lck -lnetwork -o net/http_tcp_demo.kex
	
	$(CC) $(CFLAGS) tinygl/fps.c tinygl/gears.c -o tinygl/gears.kex -ltinygl -lck

clean:
	rm *.kex clayer/*.kex net/*.kex tinygl/*.kex
