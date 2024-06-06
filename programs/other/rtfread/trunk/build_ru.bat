@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm rtfread.asm rtfread
@kpack rtfread
@erase lang.inc
@pause
