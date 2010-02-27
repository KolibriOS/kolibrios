@echo lang fix ru >lang.inc
@fasm freecell.asm freecell
@erase lang.inc
@pause