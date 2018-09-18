@echo lang fix ru >lang.inc
@fasm.exe -m 16384 @HOTANGLES.asm @HOTANGLES
@erase lang.inc
@kpack @HOTANGLES
@pause