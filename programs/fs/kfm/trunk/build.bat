@echo lang fix en >lang.inc
@fasm -m 16384 kfm.asm kfm 
@erase lang.inc
@kpack kfm
@pause
