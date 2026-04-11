@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm template.asm template
@erase lang.inc
@pause
