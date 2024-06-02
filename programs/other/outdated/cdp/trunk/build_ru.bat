@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm cdp.asm cdp
@erase lang.inc
@pause
