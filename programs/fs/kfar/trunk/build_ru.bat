@erase lang.inc
@echo lang fix ru >lang.inc
@fasm kfar.asm kfar
mtappack kfar
@erase lang.inc
@kpack kfar
@pause