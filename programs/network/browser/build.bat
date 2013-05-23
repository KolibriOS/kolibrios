@erase lang.inc
@echo lang fix en >lang.inc
@fasm browser.asm browser
@kpack browser
@erase lang.inc
@pause
