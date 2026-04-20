@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm -m 16384 kfar.asm kfar
@erase lang.inc
@kpack kfar
@pause
