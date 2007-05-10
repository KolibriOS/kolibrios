@erase lang.inc
@echo lang fix ru >lang.inc
@fasm cdp.asm cdp
@erase lang.inc
@pause