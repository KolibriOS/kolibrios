@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 scrshoot.asm scrshoot
@kpack scrshoot
@erase lang.inc
@pause