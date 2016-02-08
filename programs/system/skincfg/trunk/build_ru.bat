@erase lang.inc
@echo lang fix ru >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause