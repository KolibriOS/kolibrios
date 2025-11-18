@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm sysxtree.asm sysxtree
@erase lang.inc
@pause
