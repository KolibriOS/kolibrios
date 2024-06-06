@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm launcher.asm launcher
@kpack launcher
@erase lang.inc
@pause
