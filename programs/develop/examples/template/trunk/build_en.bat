@erase lang.inc
@echo lang fix en >lang.inc
@fasm template.asm template
@erase lang.inc
@pause