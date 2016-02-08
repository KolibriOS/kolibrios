@erase lang.inc
@echo lang fix sp >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause