@erase lang.inc
@echo lang fix en >lang.inc
@fasm magnify.asm magnify
@kpack magnify
@erase lang.inc
@pause