@erase lang.inc
@echo lang fix en >lang.inc
@fasm tftpa.asm tftpa
@erase lang.inc
@pause