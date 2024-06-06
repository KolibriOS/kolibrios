@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm runOD.asm run
@erase lang.inc
@pause
