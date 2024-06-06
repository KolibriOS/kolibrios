@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause
