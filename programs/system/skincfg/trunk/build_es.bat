@erase lang.inc
@echo lang fix es_ES >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause
