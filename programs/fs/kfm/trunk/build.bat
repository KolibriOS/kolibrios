@echo lang fix en >lang.inc
@fasm -m 16384 kfm.asm kfm 
@kpack kfm
@erase lang.inc
@pause
