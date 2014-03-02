@erase lang.inc
@echo lang fix ru >lang.inc
@fasm launcher.asm launcher
@kpack launcher
@erase lang.inc
@pause