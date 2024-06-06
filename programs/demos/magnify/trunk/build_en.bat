@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm magnify.asm magnify
@kpack magnify
@erase lang.inc
@pause
