@erase lang.inc
@echo lang fix fr >lang.inc
@fasm template.asm template
@erase lang.inc
@pause