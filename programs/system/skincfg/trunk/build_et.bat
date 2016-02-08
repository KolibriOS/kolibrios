@erase lang.inc
@echo lang fix et >lang.inc
@fasm skincfg.asm skincfg
@kpack skincfg
@erase lang.inc
@pause