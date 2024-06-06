@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm -m 16384 mousemul.asm mousemul
@kpack mousemul
@erase lang.inc
@pause
