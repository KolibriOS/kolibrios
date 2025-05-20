@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm rtfread.asm rtfread
@kpack rtfread
@erase lang.inc
@pause
