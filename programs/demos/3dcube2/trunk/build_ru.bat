@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm 3dcube2.asm 3dcube2
@erase lang.inc
@kpack 3dcube2
@pause
