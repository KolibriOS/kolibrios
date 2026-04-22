@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm cmd.asm cmd
@erase lang.inc
@pause
