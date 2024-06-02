@echo lang fix ru_RU >lang.inc
@fasm freecell.asm freecell
@erase lang.inc
@pause
