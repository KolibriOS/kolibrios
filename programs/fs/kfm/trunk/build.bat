@echo lang fix en_US >lang.inc
@fasm -m 16384 kfm.asm kfm 
@kpack kfm
@erase lang.inc
@pause
