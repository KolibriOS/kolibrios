@erase lang.inc
@echo lang fix ru >lang.inc
@fasm tanks.asm tanks
@fasm leveledit.asm leveledit
@erase lang.inc
@pause