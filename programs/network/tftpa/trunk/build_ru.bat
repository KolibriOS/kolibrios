@erase lang.inc
@echo lang fix ru >lang.inc
@fasm tftpa.asm tftpa
@erase lang.inc
@pause