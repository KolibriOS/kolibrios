@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 pcidev.asm pcidev
@kpack pcidev
@erase lang.inc
@pause
