@erase lang.inc
@echo lang fix it >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause