@erase lang.inc
@echo lang fix ru >lang.inc
@fasm -m 16384 @ss.asm @ss
@kpack @ss
@erase lang.inc
@pause