@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause
