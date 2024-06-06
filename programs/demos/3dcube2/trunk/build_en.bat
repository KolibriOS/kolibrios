@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm 3dcube2.asm 3dcube2
@erase lang.inc
@kpack 3dcube2
@pause
