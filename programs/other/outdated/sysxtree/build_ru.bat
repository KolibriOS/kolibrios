@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm sysxtree.asm sysxtree
@erase lang.inc
@pause
