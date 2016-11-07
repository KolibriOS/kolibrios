@erase lang.inc
@echo lang fix en >lang.inc
@fasm runOD.asm run
@erase lang.inc
@pause