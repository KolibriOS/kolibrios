@erase lang.inc
@echo lang fix ru >lang.inc
@fasm 3dcube2.asm 3dcube2
@erase lang.inc
@kpack 3dcube2
@pause