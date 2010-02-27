@echo lang fix en >lang.inc
@fasm freecell.asm freecell
@erase lang.inc
@pause