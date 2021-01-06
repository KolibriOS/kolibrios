@erase lang.inc
@echo lang fix en >lang.inc
@fasm sysxtree.asm sysxtree
@erase lang.inc
@pause