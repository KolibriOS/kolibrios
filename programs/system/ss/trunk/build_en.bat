@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 @ss.asm @ss
@kpack @ss
@erase lang.inc
@pause