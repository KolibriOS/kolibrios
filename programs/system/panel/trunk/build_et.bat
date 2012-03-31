@erase lang.inc
@echo lang fix et >lang.inc
@fasm -m 16384 @panel.asm @panel
@erase lang.inc
@kpack @panel
@pause