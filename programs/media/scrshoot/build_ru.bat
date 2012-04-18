@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 scrshoot.asm scrshoot
@kpack scrshoot
@erase lang.inc
@pause