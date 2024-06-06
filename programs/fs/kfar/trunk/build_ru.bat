@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm -m 16384 kfar.asm kfar
@erase lang.inc
@kpack kfar
@pause
