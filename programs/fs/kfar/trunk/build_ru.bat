@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 kfar.asm kfar
@erase lang.inc
@kpack kfar
@pause