@erase lang.inc
@echo lang fix ru >lang.inc
@fasm sysxtree.asm sysxtree
@erase lang.inc
@pause