@erase lang.inc
@echo lang fix ru >lang.inc
@fasm iconmngr.asm iconmngr
@erase lang.inc
@kpack iconmngr
@pause