@erase lang.inc
@echo lang fix es_ES >lang.inc
@fasm magnify.asm magnify
@kpack magnify
@erase lang.inc
@pause
