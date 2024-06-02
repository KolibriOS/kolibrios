@erase lang.inc
@echo lang fix it_IT >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause
