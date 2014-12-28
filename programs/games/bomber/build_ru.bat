if not exist bin mkdir bomber_rus_bin
@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 bomber.asm bomber_rus_bin\bomber
@erase lang.inc
@kpack bomber_rus_bin\bomber
@fasm -m 16384 sounds\bomberdata.asm bomber_rus_bin\bomberdata.bin
@copy ackack.bmp bomber_rus_bin\ackack.bmp
@copy bomb.bmp bomber_rus_bin\bomb.bmp
@copy plane.bmp bomber_rus_bin\plane.bmp
@copy tile.bmp bomber_rus_bin\tile.bmp
@pause