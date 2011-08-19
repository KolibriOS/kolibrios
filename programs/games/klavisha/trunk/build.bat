@echo lang fix en >lang.inc
@fasm -m 16384 klavisha.asm klavisha
@erase lang.inc
@kpack klavisha
@pause