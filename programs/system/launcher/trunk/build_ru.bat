@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm launcher.asm launcher
@kpack launcher
@erase lang.inc
@pause
