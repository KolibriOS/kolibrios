@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 pcidev.asm pcidev
@kpack pcidev
@erase lang.inc
@pause