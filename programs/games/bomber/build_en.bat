if not exist bin mkdir bomber_eng_bin
@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 bomber.asm bomber_eng_bin\bomber
@erase lang.inc
@kpack bomber_eng_bin\bomber
@fasm -m 16384 sounds\bomberdata.asm bomber_eng_bin\bomberdata.bin
@copy ackack.bmp bomber_eng_bin\ackack.bmp
@copy bomb.bmp bomber_eng_bin\bomb.bmp
@copy plane.bmp bomber_eng_bin\plane.bmp
@copy tile.bmp bomber_eng_bin\tile.bmp
@pause