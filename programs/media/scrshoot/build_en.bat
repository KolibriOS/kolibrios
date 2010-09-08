@erase lang.inc
@echo lang fix en >lang.inc
@fasm scrshoot.asm scrshoot
@kpack scrshoot
@erase lang.inc
@pause