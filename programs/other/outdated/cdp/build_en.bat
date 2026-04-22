@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm cdp.asm cdp
@erase lang.inc
@pause
