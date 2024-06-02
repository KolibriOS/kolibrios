@echo lang fix en_US >lang.inc
@fasm freecell.asm freecell
@erase lang.inc
@pause
